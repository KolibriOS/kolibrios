resolve:

        ; parse command line parameters of type
        ; ftp://user:password@server.com:port/folder/subfolder type urls

        ; check ftp://
        xor     edx, edx
        mov     esi, buf_cmd
        mov     edi, str_ftp
        xor     ecx, ecx
  @@:
        mov     al, byte [esi+edx]
        mov     bl, byte [edi+edx]
        inc     edx
        cmp     al, bl
        jne     main
        cmp     byte [edi+edx], '/'
        jne     @b
        inc     ecx
        cmp     ecx, 2
        jne     @b

        ; parse user
        mov     dword[param_user], "USER"
        mov     byte[param_user+4], " "
        inc     edx
        add     esi, edx
        lea     edi, [param_user+5]
  .user:
        lodsb
        cmp     al, 0
        je      .error
        stosb
        cmp     byte [esi], ':'
        jne     .user
        mov     word [edi], 0x0a0d
        mov     byte [edi+2], 0

        ; parse password
        inc     esi
        mov     dword[param_password], "PASS"
        mov     byte[param_password+4], " "
        lea     edi, [param_password+5]
  .pass:   
        lodsb
        cmp     al, 0
        je      .error
        stosb
        cmp     byte [esi], '@'
        jne     .pass
        mov     word [edi], 0x0a0d
        mov     byte [edi+2], 0
  
        ; parse server address
        inc     esi
        mov     edi, param_server_addr
  .addr:
        lodsb
        cmp     al, 0
        je      .error
        stosb
        cmp     byte [esi], ':'
        jne     .addr
        mov     byte [edi], 0

        ; parse port
        inc     esi
        xor     eax, eax
        xor     ebx, ebx
        mov     [sockaddr1.port], 21 shl 8
  .port:
        lodsb
        cmp     al, 0
        je      .error
        cmp     al, '/'
        jbe     @f
        sub     al, '0'
        jb      error_hostname
        cmp     al, 9
        ja      error_hostname
        lea     ebx, [ebx*4 + ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .port
  @@: ; port done
        xchg    bl, bh
        mov     [sockaddr1.port], bx

        ; parse path
        mov     dword[param_path], "CWD "
        lea     edi, [param_path+4]
  .path:   
        lodsb
        stosb
        cmp     byte [esi], 0
        jne     .path
        mov     word [edi], 0x0a0d
        mov     byte [edi+2], 0

        ; parameters resolved successfully
        mov     [use_params], 1

        ; copy server address to buf_cmd
        mov     esi, param_server_addr
        mov     edi, buf_cmd
  .copy:
        lodsb
        stosb
        cmp     byte [edi], 0
        jne     .copy
        mov     byte [esi], 0

        jmp     no_resolve.done

  .error:
        jmp     main ; parameter resolution failed. get input from user