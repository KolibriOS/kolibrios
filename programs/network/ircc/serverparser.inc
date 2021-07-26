;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


server_parser:

        mov     esi, servercommand

        cmp     byte [esi], ':'
        jne     .parse

  .spaceloop:
        lodsb
        test    al, al
        jz      .fail
        cmp     al, ' '
        jne     .spaceloop

  .parse:
        mov     eax, [esi]
        or      eax, 0x20202020
        mov     edi, server_commands
        mov     ecx, server_commands.number

  .loop:
        scasd
        je      .got_cmd
        add     edi, 4
        dec     ecx
        jnz     .loop

  .fail:
        ret

  .got_cmd:
        jmp     dword[edi]


server_commands:

        dd      '001 ', cmd_welcome
        dd      '002 ', cmd_justprint
        dd      '003 ', cmd_justprint
        dd      '004 ', cmd_justprint
        dd      '005 ', cmd_justprint

        dd      '250 ', cmd_justprint
        dd      '251 ', cmd_justprint
        dd      '252 ', cmd_justprint
        dd      '253 ', cmd_justprint
        dd      '254 ', cmd_justprint
        dd      '255 ', cmd_justprint

        dd      '265 ', cmd_justprint
        dd      '266 ', cmd_justprint

        dd      '311 ', cmd_justprint   ; RPL_WHOISUSER
        dd      '312 ', cmd_justprint   ; RPL_WHOISSERVER
        dd      '317 ', cmd_justprint   ; RPL_WHOISIDLE
        dd      '318 ', cmd_justprint   ; RPL_ENDOFWHOIS
        dd      '319 ', cmd_justprint   ; RPL_WHOISCHANNELS
        dd      '322 ', cmd_322         ; RPL_LIST
        dd      '323 ', cmd_323         ; RPL_LISTEND
        dd      '324 ', cmd_justprint   ; RPL_CHANNELMODEIS
        dd      '328 ', cmd_justprint   ; RPL_CHANNEL_URL
        dd      '329 ', cmd_justprint   ; RPL_CREATIONTIME
        dd      '330 ', cmd_justprint
        dd      '332 ', cmd_topic       ; topic
        dd      '333 ', cmd_333         ; nickname and time of topic
        dd      '338 ', cmd_justprint   ; RPL_CHANPASSOK
        dd      '353 ', cmd_353         ; name reply
        dd      '366 ', cmd_366         ; end of names list
        dd      '372 ', cmd_justprint   ; motd
        dd      '375 ', cmd_justprint   ; start of motd
        dd      '376 ', cmd_justprint   ; end of motd
        dd      '421 ', cmd_justprint   ; unknown command
        dd      '432 ', cmd_justprint   ; erroneous nickname
        dd      '433 ', cmd_justprint   ; nickname already in use
        dd      '436 ', cmd_justprint   ; nickname collision
        dd      '671 ', cmd_justprint   ; RPL_WHOISSECURE

        dd      'join', cmd_join
        dd      'kick', cmd_kick
        dd      'mode', cmd_mode
        dd      'nick', cmd_nick
        dd      'part', cmd_part
        dd      'ping', cmd_ping
        dd      'priv', cmd_privmsg
        dd      'quit', cmd_quit
        dd      'noti', cmd_notice

        .number = ($ - server_commands) / 8


align 4
compare_to_nick:

        push    esi
        mov     ecx, MAX_NICK_LEN
        mov     esi, user_nick
  .loop:
        lodsb
        cmp     al, ' '
        jbe     .done
        test    al, al
        jz      .done
        cmp     al, 'a'
        jb      .ok
        cmp     al, 'z'
        ja      .ok
        sub     al, 0x20
  .ok:

        mov     bl, byte[edi]
        cmp     bl, 'a'
        jb      .ok2
        cmp     bl, 'z'
        ja      .ok2
        sub     bl, 0x20
  .ok2:
        cmp     bl, al
        jne     .not_equal
        inc     edi
        dec     ecx
        jnz     .loop

  .done:
        xor     eax, eax
        pop     esi
        ret

  .not_equal:
        or      eax, -1
        pop     esi
        ret



align 4
skip_parameter:

; First: skip the parameter (scan untill space or colon)
  .part1:
        lodsb
        cmp     al, ' '
        je      .part2
        cmp     al, ':'
        jne     .part1

; Skip all trailing spaces
  .part3:
        lodsb
        cmp     al, ' '
        je      .part3
        dec     esi
        ret

; Now, skip all trailing spaces and first semicolon
  .part2:
        lodsb
        cmp     al, ' '
        je      .part2
        cmp     al, ':'
        je      .part3
        dec     esi
        ret



cmd_welcome:

        mov     [status], STATUS_LOGGED_IN

cmd_justprint:

        add     esi, 4
        call    skip_parameter          ; our nickname

        call    print_asciiz

        mov     al, 10
        call    print_char

        ret



cmd_notice:

        if TIMESTAMP
        call    print_timestamp
        end if

        cmp     byte[servercommand], ':'
        jne     .gogogo

        mov     byte [esi-1], 0
        push    esi
        mov     esi, str_1
        call    print_asciiz
        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string
        mov     esi, str_2
        call    print_asciiz
        pop     esi

  .gogogo:
        add     esi, 6
        call    skip_parameter
        call    skip_parameter
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret



cmd_ping:

; Just change PING to PONG
        mov     dword[esi], 'PONG'

; Find the end of the command
        lea     edi, [esi + 5]
        xor     al, al
        repne   scasb

; Now send it back
        mov     edx, esi
        mov     esi, edi
        mov     word [esi], 0x0d0a
        inc     esi
        inc     esi
        sub     esi, edx
        mcall   send, [socketnum], , , 0

        ret



cmd_privmsg:

        mov     eax, dword[esi+4]
        or      eax, 0x20202020
        cmp     eax, 'msg '
        jne     .fail
        add     esi, 8          ; skip 'PRIVMSG '

        mov     edi, esi
        call    compare_to_nick
        jne     .channel

; private chat message
        push    esi
        mov     esi, servercommand+1
        call    window_open
        test    ebx, ebx
        jz      .fail2
        pop     esi
        call    skip_parameter  ; our own nickname
        jmp     .print

  .channel:
        call    window_open
        test    ebx, ebx
        jz      .fail

  .print:
        cmp     byte[esi], 1    ; Client to Client protocol?
        je      cmd_ctcp

        if TIMESTAMP
        call    print_timestamp
        end if

        push    esi
        mov     al, '<'
        call    print_char

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     al, '>'
        call    print_char

        mov     al, ' '
        call    print_char

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret

  .fail2:
        pop     esi
  .fail:
        ret




cmd_ctcp:

        inc     esi
        mov     eax, dword[esi]
        or      eax, 0x20202020

        cmp     eax, 'vers'
        je      .version
        cmp     eax, 'time'
        je      .time
        cmp     eax, 'ping'
        je      .ping
        cmp     eax, 'acti'
        je      .action
        cmp     eax, 'dcc '    ; TODO
        je      cmd_dcc

; Unknown CTCP command - just print to window

  .just_print:

        push    esi

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header_recv
        call    print_asciiz

        mov     al, '<'
        call    print_char

        mov     esi, servercommand+1    ; print nickname
        mov     bl, '!'
        call    print_string

        mov     al, '>'
        call    print_char

        mov     al, ' '
        call    print_char

        pop     esi
        mov     bl, 1
        call    print_string

        mov     al, 10
        call    print_char

        ret

  .time:
        mov     byte[esi+4], ' '
        lea     edi, [esi+5]

        ; TODO: add system date (fn 29) in human readable format

        mcall   3                       ; get system time

        mov     ecx, 3
  .timeloop:
        mov     bl, al
        shr     al, 4
        add     al, '0'
        stosb

        mov     al, bl
        and     al, 0x0f
        add     al, '0'
        stosb

        dec     ecx
        jz      .timedone

        mov     al, ':'
        stosb
        shr     eax, 8
        jmp     .timeloop

  .timedone:
        xor     al, al
        stosb
        call    ctcp_reply

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_asciiz

        mov     esi, servercommand+1
        call    print_asciiz

        mov     esi, ctcp_time
        call    print_asciiz

        ret

  .version:
        mov     esi, str_version
        call    ctcp_reply

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_asciiz

        mov     esi, servercommand+1
        call    print_asciiz

        mov     esi, ctcp_version
        call    print_asciiz

        ret

  .ping:
        call    ctcp_reply

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_asciiz

        mov     esi, servercommand+1
        call    print_asciiz

        mov     esi, ctcp_ping
        call    print_asciiz

        ret

  .action:
        add     esi, 7
        push    esi

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, action_header
        call    print_asciiz

        mov     esi, servercommand+1    ; print nickname
        mov     bl, '!'
        call    print_string

        mov     al, ' '
        call    print_char

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret


cmd_dcc:
        add     esi, 4
        mov     eax, dword[esi]
        or      eax, 0x202020

        cmp     eax, 'send'
        je      .send

        ret

  .send:
        call    window_open
        test    ebx, ebx
        jz      .fail
        mov     [ebx + window.type], WINDOWTYPE_DCC

  .fail:

        ret



ctcp_reply:

        push    esi
        mov     dword[user_command], 'NOTI'
        mov     dword[user_command+4], 'CE  '

        mov     esi, servercommand+1
        mov     edi, user_command+7
  .nickloop:
        lodsb
        cmp     al, '!'
        je      .done
        cmp     al, ' '
        je      .done
        test    al, al
        je      .fail
        stosb
        jmp     .nickloop
  .done:
        mov     byte [esi-1], 0
        mov     ax, ' :'
        stosw
        mov     al, 1
        stosb

        pop     esi
  .replyloop:
        lodsb
        cmp     al, 1
        jbe     .done2
        stosb
        jmp     .replyloop
  .done2:

        mov     al, 1
        stosb
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - user_command]
        mcall   send, [socketnum], user_command, , 0
  .fail:
        ret



cmd_part:

        cmp     byte [esi+4], ' '
        jne     .fail
        add     esi, 5  ; skip 'PART '

; Is it me who parted?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .not_me

; yes, close the window (if its open)
        call    window_find
        test    ebx, ebx
        jz      @f
        call    window_close
  @@:
  .fail:

        ret


; somebody else parted, just print message
  .not_me:
        push    esi
        call    window_open
        test    ebx, ebx
        jz      .fail2

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, part_header
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     esi, has_left_channel
        call    print_asciiz

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_remove

        ret

  .fail2:
        pop     esi

        ret



cmd_join:

        cmp     byte[esi+4], ' '
        jne     .fail
        add     esi, 5  ; skip 'JOIN '

; did we join a channel?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .not_me

        push    esi
        call    window_open
        test    ebx, ebx
        jz      .fail
        mov     [ebx + window.type], WINDOWTYPE_CHANNEL
        mov     [window_active], ebx

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, join_header
        call    print_asciiz

        mov     esi, str_talking
        call    print_asciiz

        pop     esi
        mov     bl, ' '
        call    print_string

        mov     al, 10
        call    print_char

        call    draw_window

        ret

  .not_me:
        push    esi
        call    window_open
        test    ebx, ebx
        jz      .fail

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, join_header
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     esi, joins_channel
        call    print_asciiz

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_add

        ret

  .fail:
        pop     esi
        ret




cmd_nick:

        cmp     byte[esi+4], ' '
        jne     .fail
        add     esi, 5          ; skip 'NICK '
        cmp     byte[esi], ':'
        jne     @f
        inc     esi
  @@:

; Is it me who changed nick?
        push    esi
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .not_me

; Yup, update user_nick
        mov     ecx, MAX_NICK_LEN-1
        mov     esi, [esp]
        mov     edi, user_nick
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ' '
        je      @f
        cmp     al, 10
        je      @f
        cmp     al, 13
        je      @f
        cmp     al, ':'
        je      @r
        stosb
        dec     ecx
        jnz     @r
  @@:
        xor     al, al
        stosb

; Print a message on the server window
        mov     [window_print], windows

        mov     esi, str_nickchange
        call    print_asciiz

        mov     esi, user_nick
        call    print_asciiz

        mov     al, 10
        call    print_char

  .not_me:

; Update in userlist
        mov     ebx, windows
        mov     ecx, MAX_WINDOWS
  .window_loop:
        push    ecx ebx
        cmp     [ebx + window.type], WINDOWTYPE_CHANNEL
        jne     .next_window

        mov     esi, servercommand+1
        call    user_remove
        test    edi, edi
        jz      .next_window

        mov     esi, [esp + 8]
        call    user_add

; And print a notification in the channel
        mov     [window_print], ebx

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, nick_header
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     esi, is_now_known_as
        call    print_asciiz

        mov     esi, [esp + 8]
        call    print_asciiz

        mov     al, 10
        call    print_char

; Now do this for all open windows
  .next_window:
        pop     ebx ecx
        add     ebx, sizeof.window
        dec     ecx
        jnz     .window_loop

        pop     esi

  .fail:

        ret




cmd_kick:

        cmp     byte [esi+4], ' '
        jne     .fail
        add     esi, 5  ; skip 'KICK '

; TODO: Is it me who got kicked?
; if so, mark channel as disconnected

  .not_me:
; find the channel user has been kicked from
        push    esi
        call    window_open
        test    ebx, ebx
        jz      .fail
        push    esi

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, kick_header
        call    print_asciiz

        pop     esi
        mov     bl, ' '
        call    print_string

        mov     esi, str_kicked
        call    print_asciiz

        pop     esi
        mov     bl, ' '
        call    print_string

        mov     esi, str_by
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     al, 10
        call    print_char

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_remove

        ret

  .fail:
        pop     esi

        ret



cmd_quit:

        cmp     byte [esi+4], ' '
        jne     .fail

        mov     ebx, windows
        mov     ecx, MAX_WINDOWS

  .window_loop:
        push    ecx
        cmp     [ebx + window.type], WINDOWTYPE_CHANNEL
        jne     .next_window

        mov     esi, servercommand+1
        call    user_remove
        test    edi, edi
        jz      .next_window

        push    ebx
        mov     [window_print], ebx

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, quit_header
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     esi, has_quit_irc
        call    print_asciiz

; TODO: check if quit message was given, and print it to the window
        pop     ebx
  .next_window:
        pop     ecx
        add     ebx, sizeof.window
        dec     ecx
        jnz     .window_loop

  .fail:


        ret



cmd_mode:

        cmp     byte [esi+4], ' '
        jne     .fail
        add     esi, 5  ; skip 'MODE '
        push    esi
        call    window_find
        test    ebx, ebx
        jz      .user
        mov     [esp], esi
        mov     [window_print], ebx

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, mode_header
        call    print_asciiz

        mov     esi, servercommand+1
        mov     bl, '!'
        call    print_string

        mov     esi, sets_mode
        call    print_asciiz

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

; TODO: keep track of user modes in username list

  .fail:
        ret


  .user:
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, mode_header
        call    print_asciiz

        mov     esi, [esp]
        mov     bl, ' '
        call    print_string

        mov     esi, sets_mode
        call    print_asciiz

        pop     esi
        call    skip_parameter
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret


cmd_353:                ; channel usernames reply

        add     esi, 4  ; skip '353 '
        call    skip_parameter
        inc     esi     ; channel type '*', '=' or '@'
        inc     esi     ; ' '
        call    window_open
        test    ebx, ebx
        jz      .fail

; now find window ptr and check if this is the first 353 message
        mov     ebx, [window_print]
        test    [ebx + window.flags], FLAG_RECEIVING_NAMES
        jnz     .add

        or      [ebx + window.flags], FLAG_RECEIVING_NAMES
;        mov     [ebx + window.users], 0
        ; TODO: remove all users?

  .add:
        push    esi
        call    user_add
        pop     esi

  .namesloop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '                 ; names list is separated with spaces
        jne     .namesloop
        jmp     .add

  .done:
        call    draw_user_list
  .fail:

        ret





cmd_366:        ; channel usernames end

        add     esi, 4          ; skip '366 '
        call    skip_parameter
        call    window_open
        test    ebx, ebx
        jz      .fail
        and     [ebx + window.flags], not FLAG_RECEIVING_NAMES
  .fail:

        ret




cmd_topic:

        add     esi, 4          ; skip '332 '
        call    skip_parameter
        call    window_open
        test    ebx, ebx
        jz      .fail

        if TIMESTAMP
        call    print_timestamp
        end if

        push    esi
        mov     esi, topic_header
        call    print_asciiz

        mov     esi, str_topic
        call    print_asciiz

        pop     esi
        call    print_asciiz

        mov     esi, str_topic_end
        call    print_asciiz

  .fail:

        ret


cmd_333:

        add     esi, 4          ; skip '333 '
        call    skip_parameter
        call    window_open
        test    ebx, ebx
        jz      .fail

        if TIMESTAMP
        call    print_timestamp
        end if

        push    esi
        mov     esi, topic_header
        call    print_asciiz

        mov     esi, str_setby
        call    print_asciiz

        pop     esi
        mov     bl, '!'
        call    print_string

        mov     al, 10
        call    print_char

  .fail:

        ret



cmd_322:        ; LIST

        add     esi, 4
        call    skip_parameter

        push    esi
        mov     esi, str_list
        call    window_open
        test    ebx, ebx
        jz      .fail

        mov     [window_active], ebx
        call    draw_window_tabs
        pop     esi
        call    print_asciiz
        mov     al, 10
        call    print_char

        ret

  .fail:
        pop     esi

        ret

cmd_323:        ; LIST END

        ret