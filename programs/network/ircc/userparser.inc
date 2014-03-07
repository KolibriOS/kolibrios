;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


user_parser:

        push    [window_active]   ; print to the current window
        pop     [window_print]

        mov     eax, [edit1.size]
        test    eax, eax
        jz      .ret                                    ; ignore empty commands
        mov     word[usercommand + eax], 0x0a0d         ; terminate the line

        cmp     byte[usercommand], '/'                  ; is it a server command ?
        je      server_command

; Ignore data commands when not connected.
        cmp     [status], STATUS_CONNECTED
        jne     .notconnected

; Ok, we said something, print it to our textbox
; TODO: dont send if it's a server window?

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     bl, '<'
        call    print_character

        mov     esi, user_nick
        call    print_text2

        mov     bl, '>'
        call    print_character
        mov     bl, ' '
        call    print_character

        mov     eax, [edit1.size]
        mov     byte[usercommand + eax],0

        mov     esi, usercommand
        call    print_text2

        mov     bl, 10
        call    print_character

; and now send it to the server
        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '

        mov     esi, [window_active]
        add     esi, window.name
        mov     edi, packetbuf+8
        mov     ecx, MAX_WINDOWNAME_LEN
  .loop:
        lodsb
        test    al, al
        jz      .done
        stosb
        dec     ecx
        jnz     .loop
  .done:

        mov     ax, ' :'
        stosw

        mov     esi, usercommand
        mov     ecx, [edit1.size]
        inc     ecx
        call    recode

; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0
  .ret:

        ret

  .notconnected:
        mov     esi, str_notconnected
        call    print_text2

        ret



user_commands:
        dd      'nick', cmd_usr_nick
        dd      'real', cmd_usr_real
        dd      'serv', cmd_usr_server
        dd      'help', cmd_usr_help
        dd      'code', cmd_usr_code

        .number2 = ($ - user_commands) / 8

; All following commands require a connection to the server.
        dd      'quer', cmd_usr_quer
        dd      'quit', cmd_usr_quit
        dd      'part', cmd_usr_part
        dd      'ctcp', cmd_usr_ctcp
        dd      'msg ', cmd_usr_msg

        .number = ($ - user_commands) / 8



server_command:

        mov     eax, dword[usercommand+1]       ; skip '/'
        or      eax, 0x20202020                 ; convert to lowercase

        mov     edi, user_commands
        mov     ecx, user_commands.number
        cmp     [status], STATUS_CONNECTED
        jne     .loop
        mov     ecx, user_commands.number2
  .loop:
        scasd
        je      .got_cmd
        add     edi, 4
        dec     ecx
        jnz     .loop

        cmp     [status], STATUS_CONNECTED
        jne     .notconnected

        jmp     cmd_usr_send                    ; If none of the previous commands, just send to server

  .got_cmd:
        jmp     dword[edi]

  .notconnected:
        mov     esi, str_notconnected
        call    print_text2

        ret



cmd_usr_msg:

        lea     esi, [usercommand+5]

        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '
        lea     edi, [packetbuf+8]

  @@:
        lodsb
        test    al, al
        jz      .fail
        cmp     al, 10
        je      .fail
        cmp     al, 13
        je      .fail
        stosb
        cmp     al, ' '
        jne     @r

        mov     al, ':'
        stosb

        push    edi
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 10
        je      @f
        cmp     al, 13
        je      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw
        mov     byte[edi], 0

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

; now print to the window
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, msg_header
        call    print_text2

        mov     eax, packetbuf+8
        mov     dl, ' '
        call    print_text

        mov     bl, '*'
        call    print_character

        mov     bl, ' '
        call    print_character

        pop     esi
        call    print_text2

  .fail:
        ret



cmd_usr_quit:
        mov     esi, quit_msg

        cmp     byte[usercommand+5], ' '
        jne     .with_message
        lea     esi, [usercommand+6]
  .with_message:
        call    cmd_usr_quit_server

        mcall   close, [socketnum]
        mov     [status], STATUS_DISCONNECTED

        mov     ecx, MAX_WINDOWS
        mov     edi, windows
  .loop:
        mov     [window_print], edi
        push    edi ecx
        call    window_close
        pop     ecx edi
        add     edi, sizeof.window
        dec     ecx
        jnz     .loop

        ret



; esi = quit message
cmd_usr_quit_server:

; User wants to close a channel, send PART command to server
        mov     dword[packetbuf], 'QUIT'
        mov     word[packetbuf+4], ' :'
        lea     edi, [packetbuf+6]
; Append our quit msg
  @@:
        lodsb
        cmp     al, 13
        je      @f
        test    al, al
        jz      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

        ret




cmd_usr_nick:

        cmp     [edit1.size], 5
        je      .dontsend
        cmp     byte[usercommand+5], ' '
        jne     .fail
        cmp     [socketnum], 0
        je      .dontsend

; Request nickname change to server
        mov     dword[usercommand+1], 'NICK'
        mov     esi, [edit1.size]
        mov     word[usercommand + esi], 0x0a0d
        inc     esi
        mcall   send, [socketnum], usercommand+1, , 0

  .fail:

        ret

; We arent logged in yet, directly change user_nick field and print notification to user.
  .dontsend:
        mov     ecx, MAX_NICK_LEN
        mov     esi, usercommand+6
        mov     edi, user_nick
  @@:
        lodsb
        cmp     al, 13
        je      @f
        stosb
        dec     ecx
        jnz     @r
  @@:
        xor     al, al
        stosb

        mov     esi, str_nickchange
        call    print_text2
        mov     esi, user_nick
        call    print_text2
        mov     esi, str_dotnewline
        call    print_text2

        ret



cmd_usr_real:

        cmp     byte[usercommand+5], ' '
        jne     cmd_usr_send

        mov     ecx, MAX_REAL_LEN
        mov     esi, usercommand+6
        mov     edi, user_real_name
  .loop:
        lodsb
        cmp     al, 13
        je      .done
        stosb
        dec     ecx
        jnz     .loop
  .done:
        xor     al, al
        stosb

        mov     esi, str_realchange
        call    print_text2
        mov     esi, user_real_name
        call    print_text2
        mov     esi, str_dotnewline
        call    print_text2

        ret



cmd_usr_server:

        mov     eax, dword[usercommand+5]       ; check for 'er ', we only checked 'serv'
        or      eax, 0x00002020
        and     eax, 0x00ffffff
        cmp     eax, 'er '
        jne     cmd_usr_send

        mov     ecx, [edit1.size]               ; ok now set the address
        sub     ecx, 8

        mov     esi, usercommand+8
  .now:
        push    esi
        mov     edi, irc_server_name
  .loop:                                        ; copy until zero byte, or ecx reaches zero.
        lodsb
        stosb
        test    al, al
        jz      .done
        dec     ecx
        jnz     .loop
        xor     al, al
        stosb
  .done:
        pop     esi

; set it also in window name
        mov     ebx, [window_print]
        call    window_set_name

; now connect
        call    socket_connect

        ret


cmd_usr_quer:

        mov     esi, usercommand+7
        call    window_open

        ret



cmd_usr_help:

        mov     esi, str_help
        call    print_text2

        ret



cmd_usr_code:

        ; TODO

        ret



; User typed a part command
cmd_usr_part:

        cmp     byte[usercommand+5], 13         ; parameters given?
        jne     cmd_usr_send                    ; yes, send command straight to server

; close active window
cmd_usr_close_window:

        mov     esi, [window_active]
        mov     [window_print], esi
        cmp     [esi + window.type], WINDOWTYPE_SERVER
        je      .not_channel

        lea     esi, [esi + window.name]
        call    cmd_usr_part_channel
        call    window_close
        ret

  .not_channel:
        cmp     [esi + window.type], WINDOWTYPE_CHAT
        jne     .not_chat

        call    window_close
  .not_chat:

        ret



; Send part command to server
; esi must point to channel name (ASCIIZ)
cmd_usr_part_channel:

; User wants to close a channel, send PART command to server
        mov     dword[packetbuf], 'PART'
        mov     byte[packetbuf+4], ' '
        lea     edi, [packetbuf+5]
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 13
        je      @f
        cmp     al, 10
        je      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

        ret



cmd_usr_ctcp:

        cmp     byte[usercommand+5], ' '
        jne     cmd_usr_send

        mov     esi, usercommand+6
; prepare a 'PRIVMSG '
        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '
        lea     edi, [packetbuf+8]

; append the destination (nickname/channel)
  @@:
        lodsb
        test    al, al
        jz      .fail
        cmp     al, ' '
        je      @f
        stosb
        jmp     @r
  @@:

        mov     ax, ' :'
        stosw
        mov     al, 0x01
        stosb

; copy the message itself
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 13
        je      @f
        cmp     al, 10
        je      @f
        stosb
        jmp     @r
  @@:

; end of message
        mov     al, 0x01
        stosb
        mov     ax, 0x0a0d
        stosw

; now send it away
        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

;; TODO: print to window

  .fail:

        ret



; The user typed some undefined command, just recode it and send to the server
cmd_usr_send:

        mov     esi, usercommand+1
        mov     ecx, [edit1.size]
        inc     ecx
        mov     edi, packetbuf
        call    recode

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

        ret





