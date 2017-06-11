;    dh_gex.inc - Diffie Hellman Group exchange
;
;    Copyright (C) 2015-2016 Jeffrey Amelynck
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.

; https://www.ietf.org/rfc/rfc4419.txt

; TODO: dont convert mpints to little endian immediately.
; Or maybe even better, not at all.

proc dh_gex

;----------------------------------------------
; >> Send Diffie-Hellman Group Exchange Request

        DEBUGF  2, "Sending GEX\n"
        stdcall ssh_send_packet, con, ssh_gex_req, ssh_gex_req.length, 0
        cmp     eax, -1
        je      .socket_err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Group

        stdcall ssh_recv_packet, con, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_KEX_DH_GEX_GROUP
        jne     proto_err
        DEBUGF  2, "Received GEX group\n"

        mov     esi, con.rx_buffer+sizeof.ssh_packet_header
        mov     edi, con.dh_p
        DEBUGF  1, "DH modulus (p): "
        call    mpint_to_little_endian
        stdcall mpint_print, con.dh_p

        DEBUGF  1, "DH base (g): "
        mov     edi, con.dh_g
        call    mpint_to_little_endian
        stdcall mpint_print, con.dh_g

;-------------------------------------------
; >> Send Diffie-Hellman Group Exchange Init

; generate a random number x, where 1 < x < (p-1)/2
        mov     edi, con.dh_x+4
        mov     [con.dh_x], DH_PRIVATE_KEY_SIZE/8
        mov     ecx, DH_PRIVATE_KEY_SIZE/8/4
  @@:
        push    ecx
        call    MBRandom
        pop     ecx
        stosd
        dec     ecx
        jnz     @r

; If the highest bit is set, add a zero byte
        shl     eax, 1
        jnc     @f
        mov     byte[edi], 0
        inc     dword[con.dh_x]
  @@:

; Fill remaining bytes with zeros       ; TO BE REMOVED ?
if ((MAX_BITS-DH_PRIVATE_KEY_SIZE) > 0)
        mov     ecx, (MAX_BITS-DH_PRIVATE_KEY_SIZE)/8/4
        xor     eax, eax
        rep stosd
end if

        DEBUGF  1, "DH x: "
        stdcall mpint_print, con.dh_x

; Compute e = g^x mod p
        stdcall mpint_modexp, con.dh_e, con.dh_g, con.dh_x, con.dh_p

        DEBUGF  1, "DH e: "
        stdcall mpint_print, con.dh_e

; Create group exchange init packet
        mov     edi, con.tx_buffer.message_code
        mov     al, SSH_MSG_KEX_DH_GEX_INIT
        stosb
        mov     esi, con.dh_e
        call    mpint_to_big_endian

        DEBUGF  2, "Sending GEX init\n"
        mov     ecx, dword[con.tx_buffer.message_code+1]
        bswap   ecx
        add     ecx, 5
        stdcall ssh_send_packet, con, con.tx_buffer.message_code, ecx, 0
        cmp     eax, -1
        je      .socket_err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Reply

        stdcall ssh_recv_packet, con, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_KEX_DH_GEX_REPLY
        jne     .proto_err

        DEBUGF  2, "Received GEX Reply\n"

;--------------------------------
; HASH: string K_S, the host key
        mov     esi, con.rx_buffer+sizeof.ssh_packet_header
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        lea     ebx, [esi+edx]
        push    ebx
        invoke  sha256_update, con.temp_ctx, esi, edx

;--------------------------------------------------------------------------
; HASH: uint32 min, minimal size in bits of an acceptable group
;       uint32 n, preferred size in bits of the group the server will send
;       uint32 max, maximal size in bits of an acceptable group
        invoke  sha256_update, con.temp_ctx, ssh_gex_req+sizeof.ssh_packet_header-ssh_packet_header.message_code, 12

;----------------------------
; HASH: mpint p, safe prime
        mov     esi, con.dh_p
        mov     edi, con.mpint_tmp
        call    mpint_to_big_endian
        lea     edx, [eax+4]
        invoke  sha256_update, con.temp_ctx, con.mpint_tmp, edx

;----------------------------------------
; HASH: mpint g, generator for subgroup
        mov     esi, con.dh_g
        mov     edi, con.mpint_tmp
        call    mpint_to_big_endian
        lea     edx, [eax+4]
        invoke  sha256_update, con.temp_ctx, con.mpint_tmp, edx

;---------------------------------------------------
; HASH: mpint e, exchange value sent by the client
        mov     esi, con.tx_buffer+sizeof.ssh_packet_header
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        invoke  sha256_update, con.temp_ctx, esi, edx

;---------------------------------------------------
; HASH: mpint f, exchange value sent by the server
        mov     esi, [esp]
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        invoke  sha256_update, con.temp_ctx, esi, edx
        pop     esi

        mov     edi, con.dh_f
        call    mpint_to_little_endian

        DEBUGF  1, "DH f: "
        stdcall mpint_print, con.dh_f

        mov     edi, con.dh_signature
        call    mpint_to_little_endian

        DEBUGF  1, "DH signature: "
        stdcall mpint_print, con.dh_signature

;--------------------------------------
; Calculate shared secret K = f^x mod p
        stdcall mpint_modexp, con.rx_buffer, con.dh_f, con.dh_x, con.dh_p

        DEBUGF  1, "DH K: "
        stdcall mpint_print, con.rx_buffer

; We always need it in big endian order, so store it as such.
        mov     edi, con.dh_K
        mov     esi, con.rx_buffer
        call    mpint_to_big_endian
        mov     [con.dh_K_length], eax

;-----------------------------------
; HASH: mpint K, the shared secret
        mov     edx, [con.dh_K_length]
        add     edx, 4
        invoke  sha256_update, con.temp_ctx, con.dh_K, edx

;-------------------------------
; Finalize the exchange hash (H)
        invoke  sha256_final, con.temp_ctx
        mov     esi, con.temp_ctx.hash
        mov     edi, con.dh_H
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Exchange hash H: "
        stdcall dump_hex, con.dh_H, 8

; TODO: skip this block when re-keying
        mov     esi, con.dh_H
        mov     edi, con.session_id
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

;---------------
; Calculate keys

; First, calculate partial hash of K and H so we can re-use it for every key.

        invoke  sha256_init, con.k_h_ctx

        mov     edx, [con.dh_K_length]
        add     edx, 4
        invoke  sha256_update, con.k_h_ctx, con.dh_K, edx
        invoke  sha256_update, con.k_h_ctx, con.dh_H, 32

;---------------------------------------------------------------
; Initial IV client to server: HASH(K || H || "A" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep movsd
        mov     [con.session_id_prefix], 'A'
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx.hash
        mov     edi, con.tx_iv
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Remote IV: "
        stdcall dump_hex, con.tx_iv, 8

;---------------------------------------------------------------
; Initial IV server to client: HASH(K || H || "B" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep     movsd
        inc     [con.session_id_prefix]
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx
        mov     edi, con.rx_iv
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Local IV: "
        stdcall dump_hex, con.rx_iv, 8

;-------------------------------------------------------------------
; Encryption key client to server: HASH(K || H || "C" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep     movsd
        inc     [con.session_id_prefix]
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx
        mov     edi, con.tx_enc_key
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Remote key: "
        stdcall dump_hex, con.tx_enc_key, 8

;-------------------------------------------------------------------
; Encryption key server to client: HASH(K || H || "D" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep     movsd
        inc     [con.session_id_prefix]
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx
        mov     edi, con.rx_enc_key
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Local key: "
        stdcall dump_hex, con.rx_enc_key, 8

;------------------------------------------------------------------
; Integrity key client to server: HASH(K || H || "E" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep     movsd
        inc     [con.session_id_prefix]
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx
        mov     edi, con.tx_int_key
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Remote Integrity key: "
        stdcall dump_hex, con.tx_int_key, 8

;------------------------------------------------------------------
; Integrity key server to client: HASH(K || H || "F" || session_id)

        mov     esi, con.k_h_ctx
        mov     edi, con.temp_ctx
        mov     ecx, sizeof.ctx_sha224256/4
        rep     movsd
        inc     [con.session_id_prefix]
        invoke  sha256_update, con.temp_ctx, con.session_id_prefix, 32+1
        invoke  sha256_final, con.temp_ctx
        mov     edi, con.rx_int_key
        mov     esi, con.temp_ctx
        mov     ecx, SHA256_HASH_SIZE/4
        rep movsd

        DEBUGF  1, "Local Integrity key: "
        stdcall dump_hex, con.rx_int_key, 8

;-------------------------------------
; << Parse Diffie-Hellman New Keys MSG

        stdcall ssh_recv_packet, con, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_NEWKEYS
        jne     .proto_err

        DEBUGF  2, "Received New Keys\n"

;-------------------------------
; >> Reply with New Keys message

        stdcall ssh_send_packet, con, ssh_new_keys, ssh_new_keys.length, 0

        xor     eax, eax
        ret

  .socket_err:
        DEBUGF  3, "Socket error during key exchange!\n"
        mov     eax, 1
        ret

  .proto_err:
        DEBUGF  3, "Protocol error during key exchange!\n"
        mov     eax, 2
        ret

endp
