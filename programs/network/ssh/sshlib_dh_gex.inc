;    sshlib_dh_gex.inc - Diffie Hellman Group exchange
;
;    Copyright (C) 2015-2021 Jeffrey Amelynck
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

proc sshlib_dh_gex con_ptr

locals

        mpint_tmp               dd ?

        mpint_p                 dd ?
        mpint_g                 dd ?
        mpint_x                 dd ?
        mpint_e                 dd ?
        mpint_f                 dd ?
        mpint_K_big             dd ?

        k_h_ctx                 dd ?
        temp_ctx                dd ?

        H                       dd ?    ; exchange hash
        K_length                dd ?

        session_id_x            rb SHA2_256_LEN+1

        str_K_S                 dd ?    ; server public host key and certificates (K_S)
        mpint_f_big             dd ?    ; pointer to original
        str_s_of_H              dd ?    ; signature of H

endl

; Allocate memory for temp variables

        mov     ecx, 7*(MAX_BITS/8+4) + 2*LIBCRASH_CTX_LEN + 1*SHA2_256_LEN
        mcall   68, 12
        test    eax, eax
        jz      .err_nomem

; Init pointers for temp variables

        mov     [mpint_tmp], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_p], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_g], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_x], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_e], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_f], eax
        add     eax, (MAX_BITS/8+4)
        mov     [mpint_K_big], eax
        add     eax, (MAX_BITS/8+4)

        mov     [k_h_ctx], eax
        add     eax, LIBCRASH_CTX_LEN
        mov     [temp_ctx], eax
        add     eax, LIBCRASH_CTX_LEN

        mov     [H], eax
        add     eax, SHA2_256_LEN

; Copy the partial exchange hash to our temporary one

        mov     esi, [con_ptr]
        lea     esi, [esi+sshlib_connection.part_ex_hash_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd

;----------------------------------------------
; >> Send Diffie-Hellman Group Exchange Request

        DEBUGF  2, "Sending GEX\n"
        stdcall sshlib_send_packet, [con_ptr], ssh_msg_gex_req, ssh_msg_gex_req.length, 0
        cmp     eax, 0
        jl      .err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Group

        stdcall sshlib_recv_packet, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     ebx, [con_ptr]
        cmp     [ebx + sshlib_connection.rx_buffer.message_code], SSH_MSG_KEX_DH_GEX_GROUP
        jne     .err_proto
        DEBUGF  2, "Received GEX group\n"

        lea     esi, [ebx + sshlib_connection.rx_buffer + sizeof.ssh_packet_header]
        stdcall mpint_to_little_endian, [mpint_p], esi
        add     esi, 4
        add     esi, eax
        DEBUGM  1, "DH modulus (p): ", [mpint_p]

        stdcall mpint_to_little_endian, [mpint_g], esi
        add     esi, 4
        add     esi, eax
        DEBUGM  1, "DH base (g): ", [mpint_g]

;-------------------------------------------
; >> Send Diffie-Hellman Group Exchange Init

; generate a random number x, where 1 < x < (p-1)/2
        mov     edi, [mpint_x]
        mov     dword[edi], DH_PRIVATE_KEY_SIZE/8
        add     edi, 4
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
        mov     eax, [mpint_x]
        inc     dword[eax]
  @@:
        DEBUGM  1, "DH private key (x): ", [mpint_x]

; Compute e = g^x mod p
        stdcall mpint_modexp, [mpint_e], [mpint_g], [mpint_x], [mpint_p]
        stdcall mpint_shrink, [mpint_e]
        DEBUGM  1, "DH public key (e): ", [mpint_e]

; Create group exchange init packet
        mov     byte[ebx + sshlib_connection.tx_buffer.message_code], SSH_MSG_KEX_DH_GEX_INIT
        lea     edi, [ebx + sshlib_connection.tx_buffer.message_code+1]
        stdcall mpint_to_big_endian, edi, [mpint_e]

        DEBUGF  2, "Sending GEX init\n"
        mov     ecx, dword[ebx + sshlib_connection.tx_buffer.message_code+1]         ;;;; dword[edi]
        bswap   ecx
        add     ecx, 5
        lea     esi, [ebx + sshlib_connection.tx_buffer.message_code]
        stdcall sshlib_send_packet, [con_ptr], esi, ecx, 0
        cmp     eax, 0
        jl      .err

;---------------------------------------------
; << Parse Diffie-Hellman Group Exchange Reply

        stdcall sshlib_recv_packet, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     ebx, [con_ptr]
        cmp     [ebx + sshlib_connection.rx_buffer.message_code], SSH_MSG_KEX_DH_GEX_REPLY
        jne     .err_proto

        DEBUGF  2, "Received GEX Reply\n"

;--------------------------------
; HASH: string K_S, the host key
        lea     esi, [ebx + sshlib_connection.rx_buffer + sizeof.ssh_packet_header]
        mov     [str_K_S], esi
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        lea     eax, [esi+edx]
        mov     [mpint_f_big], eax
        invoke  sha2_256_update, [temp_ctx], esi, edx

;--------------------------------------------------------------------------
; HASH: uint32 min, minimal size in bits of an acceptable group
;       uint32 n, preferred size in bits of the group the server will send
;       uint32 max, maximal size in bits of an acceptable group
        invoke  sha2_256_update, [temp_ctx], ssh_msg_gex_req+sizeof.ssh_packet_header-ssh_packet_header.message_code, 12

;----------------------------
; HASH: mpint p, safe prime
        stdcall mpint_shrink, [mpint_p]
        stdcall mpint_to_big_endian, [mpint_tmp], [mpint_p]
        add     eax, 4
        invoke  sha2_256_update, [temp_ctx], [mpint_tmp], eax

;----------------------------------------
; HASH: mpint g, generator for subgroup
        stdcall mpint_shrink, [mpint_g]
        stdcall mpint_to_big_endian, [mpint_tmp], [mpint_g]
        add     eax, 4
        invoke  sha2_256_update, [temp_ctx], [mpint_tmp], eax

;---------------------------------------------------
; HASH: mpint e, exchange value sent by the client
        mov     ebx, [con_ptr]
        lea     esi, [ebx + sshlib_connection.tx_buffer + sizeof.ssh_packet_header]
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        invoke  sha2_256_update, [temp_ctx], esi, edx

;---------------------------------------------------
; HASH: mpint f, exchange value sent by the server
        mov     esi, [mpint_f_big]
        mov     edx, [esi]
        bswap   edx
        add     edx, 4
        invoke  sha2_256_update, [temp_ctx], esi, edx

        stdcall mpint_to_little_endian, [mpint_f], [mpint_f_big]
        mov     esi, [mpint_f_big]
        add     esi, eax
        add     esi, 4
        mov     [str_s_of_H], esi
        DEBUGM  1, "DH exchange value (f): ", [mpint_f]

;--------------------------------------
; Calculate shared secret K = f^x mod p
        stdcall mpint_modexp, [mpint_tmp], [mpint_f], [mpint_x], [mpint_p]
        stdcall mpint_shrink, [mpint_tmp]
        DEBUGM  1, "DH shared secret (K): ", [mpint_tmp]

; We always need it in big endian order, so store it as such.
        stdcall mpint_to_big_endian, [mpint_K_big], [mpint_tmp]
        mov     [K_length], eax

;-----------------------------------
; HASH: mpint K, the shared secret
        add     eax, 4
        invoke  sha2_256_update, [temp_ctx], [mpint_K_big], eax

;-------------------------------
; Finalize the exchange hash (H)
        invoke  sha2_256_finish, [temp_ctx]
        mov     esi, [temp_ctx]
        mov     edi, [H]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

        DEBUGF  1, "Exchange hash H: "
        stdcall dump_hex, [H], SHA2_256_LEN/4

;--------------------------
; Set or get the session id

        mov     eax, [con_ptr]
        cmp     [eax + sshlib_connection.status], SSHLIB_CON_STAT_KEX_DONE
        jae     @f

; If first KEX, verify host public key
        stdcall sshlib_host_verify, [con_ptr], [str_K_S], [str_s_of_H], [H], SHA2_256_LEN
        test    eax, eax
        jnz     .err

        mov     eax, [con_ptr]
        mov     esi, [H]
        lea     edi, [eax + sshlib_connection.session_id]
        mov     ecx, SHA2_256_LEN/4
        rep movsd
  @@:

        lea     esi, [eax + sshlib_connection.session_id]
        lea     edi, [session_id_x+1]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;-------------------------------------
; << Parse Diffie-Hellman New Keys MSG

        stdcall sshlib_recv_packet, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     ebx, [con_ptr]
        cmp     [ebx + sshlib_connection.rx_buffer.message_code], SSH_MSG_NEWKEYS
        jne     .err_proto

        DEBUGF  2, "Received New Keys\n"

;----------------------------------------------------
; >> Reply with New Keys message (Using the old keys)

        stdcall sshlib_send_packet, [con_ptr], ssh_msg_new_keys, ssh_msg_new_keys.length, 0
        cmp     eax, 0
        jl      .err

;---------------
; Calculate keys

; First, calculate partial hash of K and H so we can re-use it for every key.

        invoke  sha2_256_init, [k_h_ctx]

        mov     ecx, [K_length]
        add     ecx, 4
        invoke  sha2_256_update, [k_h_ctx], [mpint_K_big], ecx
        invoke  sha2_256_update, [k_h_ctx], [H], SHA2_256_LEN

;---------------------------------------------------------------
; Initial IV client to server: HASH(K || H || "A" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'A'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]
        mov     ebx, [con_ptr]
        lea     edi, [ebx + sshlib_connection.tx_iv]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;---------------------------------------------------------------
; Initial IV server to client: HASH(K || H || "B" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'B'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]
        mov     ebx, [con_ptr]
        lea     edi, [ebx + sshlib_connection.rx_iv]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;-------------------------------------------------------------------
; Encryption key client to server: HASH(K || H || "C" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'C'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]

        mov     ebx, [con_ptr]
        lea     edi, [ebx+sshlib_connection.tx_enc_key]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        mov     ebx, [con_ptr]
        lea     edi, [ebx+sshlib_connection.tx_enc_key]
        invoke  sha2_256_update, [temp_ctx], edi, SHA2_256_LEN
        invoke  sha2_256_finish, [temp_ctx]

        mov     ebx, [con_ptr]
        add     edi, 256/8
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;-------------------------------------------------------------------
; Encryption key server to client: HASH(K || H || "D" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'D'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]

        mov     ebx, [con_ptr]
        lea     edi, [ebx+sshlib_connection.rx_enc_key]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        mov     ebx, [con_ptr]
        lea     edi, [ebx+sshlib_connection.rx_enc_key]
        invoke  sha2_256_update, [temp_ctx], edi, SHA2_256_LEN
        invoke  sha2_256_finish, [temp_ctx]

        mov     ebx, [con_ptr]
        add     edi, 256/8
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;------------------------------------------------------------------
; Integrity key client to server: HASH(K || H || "E" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'E'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]
        mov     ebx, [con_ptr]
        lea     edi, [ebx + sshlib_connection.tx_int_key]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

;------------------------------------------------------------------
; Integrity key server to client: HASH(K || H || "F" || session_id)

        mov     esi, [k_h_ctx]
        mov     edi, [temp_ctx]
        mov     ecx, LIBCRASH_CTX_LEN/4
        rep movsd
        lea     edx, [session_id_x]
        mov     byte[edx], 'F'
        invoke  sha2_256_update, [temp_ctx], edx, SHA2_256_LEN+1
        invoke  sha2_256_finish, [temp_ctx]
        mov     ebx, [con_ptr]
        lea     edi, [ebx + sshlib_connection.rx_int_key]
        mov     esi, [temp_ctx]
        mov     ecx, SHA2_256_LEN/4
        rep movsd

        mov     ebx, [con_ptr]
        mov     [ebx + sshlib_connection.status], SSHLIB_CON_STAT_KEX_DONE
        xor     eax, eax

  .err:
        push    eax
        xor     eax, eax
        mov     ecx, (7*(MAX_BITS/8+4) + 2*LIBCRASH_CTX_LEN + 1*SHA2_256_LEN )/4
        mov     edi, [mpint_tmp]
        rep stosd

        mcall   68, 13, [mpint_tmp]
        pop     eax
        ret

  .err_nomem:
        DEBUGF  3, "Out of memory during key exchange!\n"
        mov     eax, SSHLIB_ERR_NOMEM
        ret

  .err_proto:
        DEBUGF  3, "Protocol error during key exchange!\n"
        mov     eax, SSHLIB_ERR_PROTOCOL
        jmp     .err

endp
