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
;        je      op_ok

        cmp     dword[s], "331 "
        je      pass

;        cmp     dword[s], "421 "
;        je      timeout

        cmp     dword[s], "530 "         ; password incorrect
        je      welcome

        cmp     dword[s], "550 "
        je      close_datacon

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

        cmp     [operation], OPERATION_STOR
        je      .stor

; we are receiving data
        mcall   recv, [datasocket], buffer_ptr2, BUFFERSIZE, 0
        test    ebx, ebx
        jnz     .done
        mov     byte[buffer_ptr2 + eax], 0

        cmp     [operation], OPERATION_RETR
        je      .retr

; not retreiving, just print to console
        invoke  con_write_asciiz, buffer_ptr2
        jmp     data_ok

; retreiving, save to file
  .retr:
        mov     [filestruct.ptr], buffer_ptr2
        mov     [filestruct.size], eax
        push    eax
        mcall   70, filestruct
        pop     eax
        add     [filestruct.offset], eax
        jmp     data_ok

; storing, send all data
  .stor:
        mcall   70, filestruct
        cmp     eax, 6          ; end of file
        je      .last_call
        test    eax, eax        ; error
;        jne     .fileerror
        add     [filestruct.offset], ebx
        mov     esi, ebx
        mcall   send, [datasocket], buffer_ptr2, , 0
        jmp     .stor

  .last_call:
        mov     esi, ebx
        mcall   send, [datasocket], buffer_ptr2, , 0

  .done:
        mcall   close, [datasocket]
        jmp     wait_for_servercommand



close_datacon:
        mcall   close, [datasocket]
        jmp     wait_for_usercommand



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