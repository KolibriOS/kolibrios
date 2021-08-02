;    sshlib_host.inc - SSH remote host authentication
;
;    Copyright (C) 2021 Jeffrey Amelynck
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

; https://datatracker.ietf.org/doc/html/rfc4253#section-6.6
; https://datatracker.ietf.org/doc/html/rfc3447

proc sshlib_host_verify  con_ptr, str_host_key, str_signature, message, message_len

locals
        known_key_sz rb MAX_PUBLIC_KEY_SIZE
endl

        mov     eax, [con_ptr]
        cmp     [eax+sshlib_connection.algo_hostkey], SSHLIB_HOSTKEY_RSA
        je      .rsa
        ; ..add more here
        mov     eax, SSHLIB_ERR_HKEY_NO_ALGO
        ret

  .rsa:
        stdcall sshlib_host_verify_rsa, [str_host_key], [str_signature], [message], [message_len]
        test    eax, eax
        jnz     .err

  .lookup:
;        lea     eax, [known_key_sz]
;        mov     ebx, [con_ptr]
;        lea     ebx, [ebx + sshlib_connection.hostname_sz]
;        invoke  ini_get_str, known_hosts_file, ebx, ssh_rsa_sz, eax, MAX_PUBLIC_KEY_SIZE, null_sz
;        test    eax, eax
;        jnz     .unknown

; TODO: verify cached host key
;        jne     .mismatch

        jmp     .unknown        ; FIXME

        xor     eax, eax
        ret

  .mismatch:
        lea     eax, [known_key_sz]
        stdcall sshlib_callback_hostkey_problem, [con_ptr], SSHLIB_HOSTKEY_PROBLEM_MISMATCH, eax
        cmp     eax, SSHLIB_HOSTKEY_ACCEPT
        je      .store
        ret

  .unknown:
        lea     eax, [known_key_sz]
        stdcall sshlib_callback_hostkey_problem, [con_ptr], SSHLIB_HOSTKEY_PROBLEM_UNKNOWN, eax
        cmp     eax, SSHLIB_HOSTKEY_ACCEPT
        je      .store
        ret

  .store:
; TODO: write to know_hosts file and fall-through

        ret

  .err:
        ret

endp


; https://datatracker.ietf.org/doc/html/rfc3447#section-8.2.2
; RSASSA-PKCS1-V1_5-VERIFY
proc sshlib_host_verify_rsa str_host_key, str_signature, M, message_len

locals
        h_ctx                   dd ?

; Signer's RSA public key
        mpint_e                 dd ?    ; public exponent
        mpint_n                 dd ?    ; modulus

        mpint_m                 dd ?

        EM                      dd ?
        EM_accent               dd ?

        mpint_s                 dd ?    ; rsa_signature_blob


;        k       dd ?    ; length of RSA modulus n

endl

        DEBUGF  3, "SSH: Performing RSA verification\n"

        mcall   68, 12, sizeof.crash_ctx + 5*(MAX_BITS/8+4)
        test    eax, eax
        jz      .err_nomem
        mov     [h_ctx], eax
        add     eax, sizeof.crash_ctx
        mov     [mpint_e], eax
        add     eax, MAX_BITS/8+4
        mov     [mpint_n], eax
        add     eax, MAX_BITS/8+4
        mov     [mpint_m], eax
        add     eax, MAX_BITS/8+4
        mov     [EM], eax
        add     eax, MAX_BITS/8+4
        mov     [EM_accent], eax
        add     eax, MAX_BITS/8+4
        mov     [mpint_s], eax
;        add     eax, MAX_BITS/8+4

; Host key
        mov     esi, [str_host_key]
        mov     ecx, [esi]
        bswap   ecx
        cmp     ecx, MAX_PUBLIC_KEY_SIZE
        ja      .err_key
; Host key type (string)
        cmp     dword[esi+4], 0x07000000
        jne     .err_key
        cmp     dword[esi+8], 'ssh-'
        jne     .err_key
        cmp     dword[esi+11], '-rsa'
        jne     .err_key
        add     esi, 4+4+7
; mpint e
        stdcall mpint_to_little_endian, [mpint_e], esi
        add     esi, eax
        add     esi, 4
; mpint n
        stdcall mpint_to_little_endian, [mpint_n], esi
;        mov     [k], eax             ;; HMMMM FIXME, 0-byte..

; Signature
        mov     esi, [str_signature]
        mov     ecx, [esi]
        bswap   ecx             ; TODO: check length
; Host key type (string)
        cmp     dword[esi+4], 0x07000000
        jne     .err_signature
        cmp     dword[esi+8], 'ssh-'
        jne     .err_signature
        cmp     dword[esi+11], '-rsa'
        jne     .err_signature
        add     esi, 4+4+7
; RSA signature blob
        stdcall mpint_to_little_endian, [mpint_s], esi
;        cmp     eax, [k]
 ;;;       jne     .err_signature

; RSAVP1
        stdcall mpint_modexp, [mpint_m], [mpint_s], [mpint_e], [mpint_n]
; I2OSP
        stdcall mpint_shrink, [mpint_m]
        stdcall mpint_grow, [mpint_m], 256
        stdcall mpint_to_big_endian, [EM], [mpint_m]

; EMSA-PKCS1-v1_5
        invoke  sha1_init, [h_ctx]
        invoke  sha1_update, [h_ctx], [M], [message_len]
        invoke  sha1_final, [h_ctx]

        mov     edi, [EM_accent]
        mov     al, 0x00
        stosb
        mov     al, 0x01
        stosb
        mov     ecx, 256 - (rsa_sha1_t.len + 3 + SHA1_HASH_SIZE)
        mov     al, 0xff
        rep stosb
        mov     al, 0x00
        stosb
        mov     esi, rsa_sha1_t
        mov     ecx, rsa_sha1_t.len
        rep movsb
        mov     esi, [h_ctx]
        mov     ecx, SHA1_HASH_SIZE
        rep movsb

; Compare EM with EM_accent
        mov     esi, [EM]
        add     esi, 4
        mov     edi, [EM_accent]
        mov     ecx, 256/4
        xor     eax, eax
  .ct_cmp_loop:
        mov     ebx, [esi]
        xor     ebx, [edi]
        or      eax, ebx
        lea     esi, [esi+4]
        lea     edi, [edi+4]
        dec     ecx
        jnz     .ct_cmp_loop

        push    eax
        mcall   68, 13, [h_ctx]
        pop     eax

        test    eax, eax
        jnz     .fail

        DEBUGF  3, "SSH: RSA verification OK!\n"

        ret

  .fail:
        DEBUGF  3, "SSH: RSA verification failed!\n"
        mov     eax, SSHLIB_ERR_HKEY_VERIFY_FAIL
        ret

  .err_nomem:
        mov     eax, SSHLIB_ERR_NOMEM
        ret

  .err_signature:
        mov     eax, SSHLIB_ERR_HKEY_SIGNATURE
        ret

  .err_key:
        mov     eax, SSHLIB_ERR_HKEY_PUBLIC_KEY
        ret

endp


iglobal

        rsa_sha1_t      db 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x04, 0x14
        .len = $ - rsa_sha1_t

endg

