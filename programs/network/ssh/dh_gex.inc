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

        DEBUGF  1, "Sending GEX\n"
        stdcall ssh_send_packet, [socketnum], ssh_gex_req, ssh_gex_req.length, 0
        cmp     eax, -1
        je      .socket_err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Group

        stdcall ssh_recv_packet, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [rx_buffer+ssh_header.message_code], SSH_MSG_KEX_DH_GEX_GROUP
        jne     proto_err
        DEBUGF  1, "Received GEX group\n"

        mov     esi, rx_buffer+sizeof.ssh_header
        mov     edi, dh_p
        DEBUGF  1, "DH modulus (p): "
        call    mpint_to_little_endian
        stdcall mpint_print, dh_p

        DEBUGF  1, "DH base (g): "
        mov     edi, dh_g
        call    mpint_to_little_endian
        stdcall mpint_print, dh_g

;-------------------------------------------
; >> Send Diffie-Hellman Group Exchange Init

; generate a random number x, where 1 < x < (p-1)/2
        mov     edi, dh_x+4
        mov     [dh_x], DH_PRIVATE_KEY_SIZE/8
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
        inc     dword[dh_x]
  @@:

; Fill remaining bytes with zeros       ; TO BE REMOVED ?
if ((MAX_BITS-DH_PRIVATE_KEY_SIZE) > 0)
        mov     ecx, (MAX_BITS-DH_PRIVATE_KEY_SIZE)/8/4
        xor     eax, eax
        rep     stosd
end if

        DEBUGF  1, "DH x: "
        stdcall mpint_length, dh_x;;;;;;;;;;;;;
        stdcall mpint_print, dh_x

; Compute e = g^x mod p
        stdcall mpint_modexp, dh_e, dh_g, dh_x, dh_p
        stdcall mpint_length, dh_e

        DEBUGF  1, "DH e: "
        stdcall mpint_print, dh_e

; Create group exchange init packet
        mov     edi, tx_buffer+ssh_header.message_code
        mov     al, SSH_MSG_KEX_DH_GEX_INIT
        stosb
        mov     esi, dh_e
        call    mpint_to_big_endian

        DEBUGF  1, "Sending GEX init\n"
        mov     ecx, dword[tx_buffer+ssh_header.message_code+1]
        bswap   ecx
        add     ecx, 5
        stdcall ssh_send_packet, [socketnum], tx_buffer+ssh_header.message_code, ecx, 0
        cmp     eax, -1
        je      .socket_err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Reply

        stdcall ssh_recv_packet, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [rx_buffer+ssh_header.message_code], SSH_MSG_KEX_DH_GEX_REPLY
        jne     .proto_err

        DEBUGF  1, "Received GEX Reply\n"

;--------------------------------
; HASH: string  K_S, the host key
        mov     esi, rx_buffer+sizeof.ssh_header
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        lea     ebx, [esi+edx]
        push    ebx
        call    sha256_update

;--------------------------------------------------------------------------
; HASH: uint32  min, minimal size in bits of an acceptable group
;       uint32  n, preferred size in bits of the group the server will send
;       uint32  max, maximal size in bits of an acceptable group
        mov     esi, ssh_gex_req+sizeof.ssh_header-ssh_header.message_code
        mov     edx, 12
        call    sha256_update

;----------------------------
; HASH: mpint   p, safe prime
        mov     esi, dh_p
        mov     edi, mpint_tmp
        call    mpint_to_big_endian
        lea     edx, [eax+4]
        mov     esi, mpint_tmp
        call    sha256_update

;----------------------------------------
; HASH: mpint   g, generator for subgroup
        mov     esi, dh_g
        mov     edi, mpint_tmp
        call    mpint_to_big_endian
        lea     edx, [eax+4]
        mov     esi, mpint_tmp
        call    sha256_update

;---------------------------------------------------
; HASH: mpint   e, exchange value sent by the client
        mov     esi, tx_buffer+sizeof.ssh_header
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        call    sha256_update

;---------------------------------------------------
; HASH: mpint   f, exchange value sent by the server
        mov     esi, [esp]
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        call    sha256_update
        pop     esi

        mov     edi, dh_f
        call    mpint_to_little_endian

        DEBUGF  1, "DH f: "
        stdcall mpint_print, dh_f

        mov     edi, dh_signature
        call    mpint_to_little_endian

        DEBUGF  1, "DH signature: "
        stdcall mpint_print, dh_signature

;--------------------------------------
; Calculate shared secret K = f^x mod p
        stdcall mpint_modexp, rx_buffer, dh_f, dh_x, dh_p
        stdcall mpint_length, rx_buffer

        DEBUGF  1, "DH K: "
        stdcall mpint_print, rx_buffer

; We always need it in big endian order, so store it as such.
        mov     edi, dh_K
        mov     esi, rx_buffer
        call    mpint_to_big_endian
        mov     [dh_K.length], eax

;-----------------------------------
; HASH: mpint   K, the shared secret
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update

;-------------------------------
; Finalize the exchange hash (H)
        mov     edi, dh_H
        call    sha256_final

        DEBUGF  1, "Exchange hash H: "
        stdcall dump_256bit_hex, dh_H

; TODO: skip this block when re-keying
        mov     esi, dh_H
        mov     edi, session_id
        mov     ecx, 32/4
        rep movsd

;---------------
; Calculate keys

; TODO: re-use partial hash of K and H

;---------------------------------------------------------------
; Initial IV client to server: HASH(K || H || "A" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_A
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, tx_iv
        call    sha256_final

        DEBUGF  1, "Remote IV: "
        stdcall dump_256bit_hex, tx_iv

;---------------------------------------------------------------
; Initial IV server to client: HASH(K || H || "B" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_B
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, rx_iv
        call    sha256_final

        DEBUGF  1, "Local IV: "
        stdcall dump_256bit_hex, rx_iv

;-------------------------------------------------------------------
; Encryption key client to server: HASH(K || H || "C" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_C
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, tx_enc_key
        call    sha256_final

        DEBUGF  1, "Remote key: "
        stdcall dump_256bit_hex, tx_enc_key

;-------------------------------------------------------------------
; Encryption key server to client: HASH(K || H || "D" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_D
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, rx_enc_key
        call    sha256_final

        DEBUGF  1, "Local key: "
        stdcall dump_256bit_hex, rx_enc_key

;------------------------------------------------------------------
; Integrity key client to server: HASH(K || H || "E" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_E
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, tx_int_key
        call    sha256_final

        DEBUGF  1, "Remote Integrity key: "
        stdcall dump_256bit_hex, tx_int_key

;------------------------------------------------------------------
; Integrity key server to client: HASH(K || H || "F" || session_id)

        call    sha256_init
        mov     edx, [dh_K.length]
        add     edx, 4
        mov     esi, dh_K
        call    sha256_update
        mov     edx, 32
        mov     esi, dh_H
        call    sha256_update
        mov     edx, 1
        mov     esi, str_F
        call    sha256_update
        mov     edx, 32
        mov     esi, session_id
        call    sha256_update
        mov     edi, rx_int_key
        call    sha256_final

        DEBUGF  1, "Local Integrity key: "
        stdcall dump_256bit_hex, rx_int_key

;-------------------------------------
; << Parse Diffie-Hellman New Keys MSG

        stdcall ssh_recv_packet, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      .socket_err

        cmp     [rx_buffer+ssh_header.message_code], SSH_MSG_NEWKEYS
        jne     .proto_err

        DEBUGF  1, "Received New Keys\n"

;-------------------------------
; >> Reply with New Keys message

        stdcall ssh_send_packet, [socketnum], ssh_new_keys, ssh_new_keys.length, 0

        xor     eax, eax
        ret

  .socket_err:
        DEBUGF  2, "Socket error during key exchange!\n"
        mov     eax, 1
        ret

  .proto_err:
        DEBUGF  2, "Protocol error during key exchange!\n"
        mov     eax, 2
        ret

endp

proc dump_256bit_hex _ptr
        pushad

        mov     esi, [_ptr]
        mov     ecx, 8
  .next_dword:
        lodsd
        bswap   eax
        DEBUGF  1,'%x',eax
        loop    .next_dword
        DEBUGF  1,'\n'

        popad
        ret
endp

iglobal

        str_A   db 'A'
        str_B   db 'B'
        str_C   db 'C'
        str_D   db 'D'
        str_E   db 'E'
        str_F   db 'F'

endg