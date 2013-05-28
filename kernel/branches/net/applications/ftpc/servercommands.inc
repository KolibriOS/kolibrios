server_parser:

; Commands are always 3 numbers and followed by a space
; If a server decides it needs multiline output,
; first lines will have a dash instead of space after numbers,
; thus they are simply ignored.

        cmp     dword[s], "150 "
        je      data_ok

        cmp     dword[s], "220 "
        je      welcome

        cmp     dword[s], "227 "
        je      pasv_ok

        cmp     dword[s], "230 "
        je      login_ok

        cmp     dword[s], "331 "
        je      pass

        ret


welcome:

        mov     [status], STATUS_CONNECTED
        ret


pass:

        mov     [status], STATUS_NEEDPASSWORD
        ret


login_ok:

        mov     [status], STATUS_LOGGED_IN
        ret


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
        mov     byte[sockaddr2.port+1], bl
        call    ascii_dec
        mov     byte[sockaddr2.port+0], bl

        push    str_open
        call    [con_write_asciiz]

        mcall   connect, [datasocket], sockaddr2, 18

  .fail:
        ret


data_ok:

        mcall   recv, [datasocket], buffer_ptr, BUFFERSIZE, 0   ; fixme: use other buffer
        inc     eax
        jz      .fail
        dec     eax
        jz      .fail

        mov     byte[buffer_ptr + eax], 0
        pushd   buffer_ptr
        call    [con_write_asciiz]

  .fail:
        ret


ascii_dec:

        xor     ebx, ebx
        mov     cl, 3
  .loop:
        lodsb
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx*4+ebx]
        shl     ebx, 1
        add     bl, al
        dec     cl
        jnz     .loop

  .done:
        ret