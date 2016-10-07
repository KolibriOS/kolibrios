;;================================================================================================;;
parse_args: ;/////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? parses command line arguments of type                                                          ;;
;? ftp://user:password@server:port/folder/subfolder type urls and attempts to connect             ;;
;? automatically                                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;> buf_cmd = pointer to command line arguments                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

        cmp     dword[buf_cmd], 'ftp:'
        jne     @f
        mov     esi, buf_cmd
        jmp     .start
  @@:
        mov     esi, buf_cmd+5

  .start:
        ; check ftp://
        cmp     dword[esi], 'ftp:'
        jne     .error
        cmp     word[esi+4], '//'
        jne     .error

        ; parse user
        lea     edi, [param_user]
  @@:
        lodsb
        test    al, al
        jz      .error
        stosb
        cmp     byte [esi], ':'
        jne     @b
        mov     byte [edi], 0

        ; parse password
        inc     esi
        lea     edi, [param_password]
  @@:   
        lodsb
        test    al, al
        jz      .error
        stosb
        cmp     byte [esi], '@'
        jne     @b
        mov     byte [edi], 0
  
        ; parse server address
        inc     esi
        mov     edi, param_server_addr
  @@:
        lodsb
        test    al, al
        jz      .error
        stosb
        cmp     byte [esi], ':'
        jne     @b
        mov     byte [edi], 0

        ; parse port
        inc     esi
        lea     edi, [param_port]
  @@:
        lodsb
        test    al, al
        jz      .error
        stosb
        cmp     byte [esi], '/'
        jne     @b
        mov     byte [edi], 0        

        ; parse path
        inc     esi
        lea     edi, [param_path]
  @@:   
        movsb
        cmp     byte [esi], 0
        jne     @b
        mov     byte [edi], 0

        ; parameters parsed successfully
        jmp     arg_handler.connect

  .error:
        mov     eax, str_err_params
        jmp     error
