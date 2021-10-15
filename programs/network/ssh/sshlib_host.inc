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
; https://datatracker.ietf.org/doc/html/rfc4716
; https://datatracker.ietf.org/doc/html/rfc8017

proc sshlib_host_verify  con_ptr, str_host_key, str_signature, message, message_len

locals
        current_hkb64           rb MAX_PUBLIC_KEY_SIZE*4        ; Current Host key in Base64
        cached_hkb64            rb MAX_PUBLIC_KEY_SIZE*4        ; Cached Host key in Base64
        key_name_sz             dd ?
        hostname_sz             dd ?
        current_hk64_end        dd ?
endl

        mov     eax, [con_ptr]
        lea     ebx, [eax + sshlib_connection.hostname_sz]
        mov     [hostname_sz], ebx
        cmp     [eax+sshlib_connection.algo_hostkey], SSHLIB_HOSTKEY_RSA
        je      .rsa
        ; ..add more here
        mov     eax, SSHLIB_ERR_HKEY_NO_ALGO
        ret

  .rsa:
        stdcall sshlib_host_verify_rsa, [str_host_key], [str_signature], [message], [message_len]
        test    eax, eax
        jnz     .err
        mov     [key_name_sz], ssh_rsa_sz

  .lookup:
; Convert the current host key to base64
        mov     esi, [str_host_key]
        mov     ecx, [esi]
        bswap   ecx
        add     esi, 4
        lea     edi, [current_hkb64]
        call    base64_encode
        mov     [current_hk64_end], edi

; Try to read the cached key for this host and key type
        lea     edi, [cached_hkb64]
        invoke  ini_get_str, known_hostsfile, [hostname_sz], [key_name_sz], edi, MAX_PUBLIC_KEY_SIZE*4, 0
        test    eax, eax
        jnz     .unknown
; If the cached key is empty, return SSHLIB_HOSTKEY_PROBLEM_UNKNOWN
        lea     esi, [cached_hkb64]
        cmp     byte[esi], 0
        je      .unknown
; Else, compare it to the current one
        lea     edi, [current_hkb64]
        mov     ecx, MAX_PUBLIC_KEY_SIZE*4
  .cmploop:
        lodsb
        scasb
        jne     .mismatch
        test    al, al
        jz      .match
        dec     ecx
        jnz     .cmploop
        jmp     .mismatch

  .match:
        xor     eax, eax
        ret

  .mismatch:
int3
        lea     eax, [current_hkb64]
        stdcall sshlib_callback_hostkey_problem, [con_ptr], SSHLIB_HOSTKEY_PROBLEM_MISMATCH, eax
        cmp     eax, SSHLIB_HOSTKEY_ACCEPT
        je      .store
        ret

  .unknown:
        lea     eax, [current_hkb64]
        stdcall sshlib_callback_hostkey_problem, [con_ptr], SSHLIB_HOSTKEY_PROBLEM_UNKNOWN, eax
        cmp     eax, SSHLIB_HOSTKEY_ACCEPT
        je      .store
        ret

  .store:
        lea     esi, [current_hkb64]
        mov     ecx, [current_hk64_end]
        sub     ecx, esi
        invoke  ini_set_str, known_hostsfile, [hostname_sz], [key_name_sz], esi, ecx
        xor     eax, eax
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

        k                       dd ?    ; Key length

endl

        DEBUGF  3, "SSH: Performing RSA verification\n"

        mcall   68, 12, LIBCRASH_CTX_LEN + 5*(MAX_BITS/8+4)
        test    eax, eax
        jz      .err_nomem
        mov     [h_ctx], eax
        add     eax, LIBCRASH_CTX_LEN
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
        and     eax, not (32-1)         ; CHECKME
        mov     [k], eax

; Signature
        mov     esi, [str_signature]
        mov     ecx, [esi]
        bswap   ecx                     ; TODO: check length

; Host key type (string)
        cmp     dword[esi+4], 0x07000000
        jne     .not_ssh_rsa
        cmp     dword[esi+8], 'ssh-'
        jne     .not_ssh_rsa
        cmp     dword[esi+11], '-rsa'
        je      .sha1

  .not_ssh_rsa:
        cmp     dword[esi+4], 0x0c000000
        jne     .not_sha2
        cmp     dword[esi+8], 'rsa-'
        jne     .not_sha2
        cmp     dword[esi+12], 'sha2'
        jne     .not_sha2
        cmp     dword[esi+16], '-256'
        je      .sha2_256
        cmp     dword[esi+16], '-512'
        je      .sha2_512

  .not_sha2:
        jmp     .err_signature

  .sha1:
        DEBUGF  3, "SSH: Using RSA with SHA1 hash\n"
        add     esi, 4+4+7
        push    esi

; EMSA-PKCS1-v1_5
        invoke  sha1_init, [h_ctx]
        invoke  sha1_update, [h_ctx], [M], [message_len]
        invoke  sha1_finish, [h_ctx]

        mov     edi, [EM_accent]
        mov     al, 0x00
        stosb
        mov     al, 0x01
        stosb
        mov     ecx, [k]
        sub     ecx, (rsa_sha1_T.len + 3 + SHA1_LEN)
        jl      .err_key
        jz      @f
        mov     al, 0xff
        rep stosb
  @@:
        mov     al, 0x00
        stosb
        mov     esi, rsa_sha1_T
        mov     ecx, rsa_sha1_T.len
        rep movsb
        mov     esi, [h_ctx]
        mov     ecx, SHA1_LEN
        rep movsb

        pop     esi
        jmp     .rsa

  .sha2_256:
        DEBUGF  3, "SSH: Using RSA with SHA2-256 hash\n"
        add     esi, 4+4+12
        push    esi

; EMSA-PKCS1-v1_5
        invoke  sha2_256_init, [h_ctx]
        invoke  sha2_256_update, [h_ctx], [M], [message_len]
        invoke  sha2_256_finish, [h_ctx]

        mov     edi, [EM_accent]
        mov     al, 0x00
        stosb
        mov     al, 0x01
        stosb
        mov     ecx, [k]
        sub     ecx, (rsa_sha256_T.len + 3 + SHA2_256_LEN)
        jl      .err_key
        jz      @f
        mov     al, 0xff
        rep stosb
  @@:
        mov     al, 0x00
        stosb
        mov     esi, rsa_sha256_T
        mov     ecx, rsa_sha256_T.len
        rep movsb
        mov     esi, [h_ctx]
        mov     ecx, SHA2_256_LEN
        rep movsb

        pop     esi
        jmp     .rsa

  .sha2_512:
        DEBUGF  3, "SSH: Using RSA with SHA2-512 hash\n"
        add     esi, 4+4+12
        push    esi

; EMSA-PKCS1-v1_5
        invoke  sha2_512_init, [h_ctx]
        invoke  sha2_512_update, [h_ctx], [M], [message_len]
        invoke  sha2_512_finish, [h_ctx]

        mov     edi, [EM_accent]
        mov     al, 0x00
        stosb
        mov     al, 0x01
        stosb
        mov     ecx, [k]
        sub     ecx, (rsa_sha512_T.len + 3 + SHA2_512_LEN)
        jl      .err_key
        jz      @f
        mov     al, 0xff
        rep stosb
  @@:
        mov     al, 0x00
        stosb
        mov     esi, rsa_sha512_T
        mov     ecx, rsa_sha512_T.len
        rep movsb
        mov     esi, [h_ctx]
        mov     ecx, SHA2_512_LEN
        rep movsb

        pop     esi
        jmp     .rsa

  .rsa:
; RSA signature blob
        stdcall mpint_to_little_endian, [mpint_s], esi
;        cmp     eax, [k]
;        jne     .err_signature

; RSAVP1
        stdcall mpint_modexp, [mpint_m], [mpint_s], [mpint_e], [mpint_n]
; I2OSP
        stdcall mpint_shrink, [mpint_m]
        stdcall mpint_grow, [mpint_m], [k]
        stdcall mpint_to_big_endian, [EM], [mpint_m]

; Compare EM with EM_accent
        mov     esi, [EM]
        add     esi, 4
        mov     edi, [EM_accent]
        mov     ecx, [k]
        shr     ecx, 2
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


base64_encode:

        xor     ebx, ebx
  .loop:
        lodsb
        call    .byte
        dec     ecx
        jnz     .loop

  .final:
        mov     al, 0
        test    ebx, ebx
        jz      .f000
        call    .byte
        test    ebx, ebx
        jz      .f001
        call    .byte
        mov     byte[edi-2], '='

  .f001:
        mov     byte[edi-1], '='

  .f000:
        mov     byte[edi], 0
        ret

  .byte:
        inc     ebx
        shl     edx, 8
        mov     dl, al
        cmp     ebx, 3
        je      .b001
        ret

  .b001:
        shl     edx, 8
        inc     ebx

  .b002:
        rol     edx, 6
        xor     eax, eax
        xchg    al, dl
        mov     al, [base64_table+eax]
        stosb
        dec     ebx
        jnz     .b002
        ret




iglobal

        known_hostsfile db '/sys/settings/known_hosts.ini', 0
        base64_table    db 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
        rsa_sha1_T      db 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x04, 0x14
        .len = $ - rsa_sha1_T
        rsa_sha256_T    db 0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
        .len = $ - rsa_sha256_T
        rsa_sha512_T    db 0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40
        .len = $ - rsa_sha512_T
        ssh_rsa_sz      db 'ssh-rsa', 0

endg

