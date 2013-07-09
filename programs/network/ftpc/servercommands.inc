server_parser:

; Commands are always 3 numbers and followed by a space
; If a server decides it needs multiline output,
; first lines will have a dash instead of space after numbers,
; thus they are simply ignored in this simple command parser.

        cmp     dword[s], "150 "
        je      data_ok

        cmp     dword[s], "220 "
        je      welcome

;        cmp     dword[s], "226 "
;        je      list_ok

        cmp     dword[s], "227 "
        je      pasv_ok

        cmp     dword[s], "230 "
        je      login_ok

;        cmp     dword[s], "250"
;        je      dir_ok

        cmp     dword[s], "331 "
        je      pass

;        cmp     dword[s], "421 "
;        je      timeout

        cmp     dword[s], "530"         ; password incorrect
        je      welcome

        jmp     wait_for_usercommand


welcome:

        mov     [status], STATUS_CONNECTED
        jmp     wait_for_usercommand


pass:

        mov     [status], STATUS_NEEDPASSWORD
        jmp     wait_for_usercommand


login_ok:

        mov     [status], STATUS_LOGGED_IN
        jmp     wait_for_usercommand


pasv_ok:

        sub     ecx, 5
        jb      .fail
        mov     al, "("
        mov     edi, s + 5
        repne   scasb

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      fail
        mov     [datasocket], eax

        mov     esi, edi
        call    ascii_dec
        mov     byte[sockaddr2.ip+0], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+1], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+2], bl
        call    ascii_dec
        mov     byte[sockaddr2.ip+3], bl

        call    ascii_dec
        mov     byte[sockaddr2.port+0], bl
        call    ascii_dec
        mov     byte[sockaddr2.port+1], bl

        invoke  con_write_asciiz, str_open
        mcall   connect, [datasocket], sockaddr2, 18

  .fail:
        jmp     wait_for_servercommand


data_ok:

        mcall   recv, [datasocket], buffer_ptr2, BUFFERSIZE, MSG_DONTWAIT
        inc     eax
        jz      .fail
        dec     eax
        jz      .fail
        mov     byte[buffer_ptr2 + eax], 0

        invoke  con_write_asciiz, buffer_ptr2

  .fail:
        mcall   close, [datasocket]
        jmp     wait_for_servercommand


ascii_dec:

        xor     ebx, ebx
        mov     cl, 4                   ; max length is 3 digits + 1 separator
  .loop:
        lodsb
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx*4+ebx]        ; ebx *5
        shl     ebx, 1                  ; ebx *2
        add     bl, al
        dec     cl
        jnz     .loop

  .done:
        ret