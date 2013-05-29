;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


user_parser:

        mov     eax, [edit1.size]
        test    eax, eax
        jz      sdts_ret                                ; ignore empty commands
        mov     word [usercommand + eax], 0x0a0d        ; terminate the line

        cmp     byte[usercommand], '/'                  ; is it a server command ?
        je      server_command

; Ignore data commands when not connected.
        cmp     [status], STATUS_CONNECTED
        jne     sdts_ret

; Ok, we said something, print it to our textbox

; TODO: dont send if it's a server window?

        push    [window_open]   ; print to the current window
        pop     [window_print]
        call    window_refresh

        if      TIMESTAMP
        call    print_timestamp
        end     if

        mov     bl, '<'
        call    print_character

        mov     esi, user_nick
        call    print_text2

        mov     bl,'>'
        call    print_character
        mov     bl,' '
        call    print_character

        mov     eax, [edit1.size]
        mov     byte[usercommand + eax],0

        mov     esi, usercommand
        call    print_text2

        mov     bl, 10
        call    print_character

; and now send it to the server

        mov     dword[packetbuf], 'priv'
        mov     dword[packetbuf+4], 'msg '

        mov     esi, [window_open]
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

        mov     al, 10
        stosb

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

sdts_ret:

        ret



user_commands:
        dd      'nick', cmd_usr_nick
        dd      'real', cmd_usr_real
        dd      'serv', cmd_usr_server
        dd      'help', cmd_usr_help
        dd      'code', cmd_usr_code
; TODO: All other commands require a connection to the server.
        dd      'quer', cmd_usr_quer
        dd      'quit', cmd_usr_quit

        .number = ($ - user_commands) / 8



server_command:

        mov     eax, dword[usercommand+1]
        or      eax, 0x20202020

        mov     edi, user_commands
        mov     ecx, user_commands.number
  .loop:
        scasd
        je      .got_cmd
        add     edi, 4
        dec     ecx
        jnz     .loop
        jmp     cmd_usr_send            ; If none of the previous commands, just send to server

  .got_cmd:
        jmp     dword[edi]





cmd_usr_quit:

        cmp     [edit1.size], 5
        je      .ok
        jb      cmd_usr_send
        cmp     byte[usercommand+5], ' '
        jne     cmd_usr_send

  .ok:
        call    cmd_usr_send

        mcall   close, [socketnum]

        mov     ecx, MAX_WINDOWS
        mov     edi, windows
  .loop:
        mov     [edi + window.flags], FLAG_CLOSE
        add     edi, sizeof.window
        dec     ecx
        jnz     .loop

        ret




cmd_usr_nick:

        cmp     [edit1.size], 5
        je      .justprint
        cmp     byte[usercommand+5], ' '
        jne     cmd_usr_send

        mov     ecx, MAX_NICK_LEN
        mov     esi, usercommand+6
        mov     edi, user_nick
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

        cmp     [socketnum], 0
        je      .justprint

        lea     esi, [edi - usercommand]
        mcall   send, [socketnum], usercommand+1, , 0

  .justprint:
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

        mov     ecx, [edit1.size]         ; ok now set the address
        sub     ecx, 8

        mov     esi, usercommand+8
        push    esi
        mov     edi, irc_server_name
        rep     movsb
        xor     al, al
        stosb
        pop     esi

; set it also in window name
        mov     ebx, [window_print]
        call    window_set_name

; now connect
        call    socket_connect

        ret


cmd_usr_quer:

        mov     ecx, MAX_WINDOWS
        mov     ebx, windows
  .loop:
        cmp     [ebx + window.data_ptr], 0
        je      .found
        add     ebx, sizeof.window
        dec     ecx
        jnz     .loop

; error: no available channels ! FIXME

        ret


  .found:
        call    window_create
        test    eax, eax
        jz      .error
        mov     [ebx + window.data_ptr], eax

        mov     esi, usercommand+7
        call    window_set_name

        mov     [ebx + window.type], WINDOWTYPE_CHAT
        mov     [ebx + window.flags], 0

  .error:

        ret



cmd_usr_help:

        mov     esi, str_help
        call    print_text2

        ret



cmd_usr_code:

        ; TODO

        ret



cmd_usr_send:

        mov     esi, usercommand+1
        mov     ecx, [edit1.size]
        inc     ecx
        mov     edi, packetbuf
        call    recode

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

        ret

