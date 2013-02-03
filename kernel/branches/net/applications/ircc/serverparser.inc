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

        dd      '328 ', cmd_328
        dd      '332 ', cmd_topic
        dd      '333 ', cmd_333         ; nickname and time of topic
        dd      '353 ', cmd_353         ; name reply
        dd      '366 ', cmd_366         ; end of names list
        dd      '372 ', cmd_372         ; motd
        dd      '375 ', cmd_375         ; start of motd
        dd      '376 ', cmd_376         ; end of motd
        dd      '421 ', cmd_421         ; unknown command
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

compare_to_nick:

        push    esi
        mov     ecx, MAX_NICK_LEN
        mov     esi, user_nick
  .loop:
        lodsb
        cmp     al, ' '
        jbe     .done
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



find_window:

;        mov     [window_print],

        ret



cmd_328:
cmd_421:
cmd_372:
cmd_375:
cmd_376:
        add     esi, 4
        jmp     cmd_notice.loop

cmd_notice:

        cmp     byte[servercommand], ':'
        jne     .gogogo

        mov     byte [esi-1], 0
        push    esi
        mov     esi, str_1
        call    print_text2
        mov     esi, servercommand+1
        call    print_text2
        mov     esi, str_2
        call    print_text2
        pop     esi

  .gogogo:
        add     esi, 6

  .loop:
        inc     esi
        cmp     byte [esi], 0
        je      .fail
;        cmp     byte [esi], 10  ; newline
;        je      server_parser.parse
        cmp     byte [esi], ' '
        jne     .loop

  .loop2:
        inc     esi
        cmp     byte [esi], 0
        je      .fail
        cmp     byte [esi], ' '
        je      .loop2
        cmp     byte [esi], ':'
        je      .loop2

        call    print_text2
        mov     esi, str_newline
        call    print_text2

  .fail:

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

        add     esi, 8  ; skip 'PRIVMSG '

; Check if it was destined for me privately
        mov     edi, servercommand+1
        call    compare_to_nick
;;;        je      .private

; If not, find the correct window ???

; now find the end of nick
        mov     edi, esi
  .loop:
        inc     edi
        cmp     byte [edi], 0
        je      .fail
        cmp     byte [edi], ' '
        jne     .loop

  .loop2:
        inc     edi
        cmp     byte [edi], 0
        je      .fail
        cmp     byte [edi], ' '
        je      .loop2
        cmp     byte [edi], ':'
        je      .loop2
        cmp     byte [edi], 1
        je      cmd_ctcp

; Action?
        cmp     dword[edi+1], 'ACTI'
        je      .action

; nope, just plain old privmsg
if TIMESTAMP
        call    print_timestamp
end if

        push    edi
        mov     bl, '<'
        call    print_character

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     bl, '>'
        call    print_character

        mov     bl, ' '
        call    print_character

        pop     esi
        call    print_text2

        mov     bl, 10
        call    print_character

  .fail:
        ret

  .action:
        push    edi
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, action_header_short
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, ' '
        call    print_text

        mov     bl, ' '
        call    print_character

        pop     esi
        add     esi, 8
        call    print_text2

        mov     bl, 10
        call    print_character

        ret

cmd_ctcp:

        cmp     dword[edi+1], 'VERS'
        je      .version

        cmp     dword[edi+1], 'TIME'
        je      .time

        cmp     dword[edi+1], 'PING'
        je      .ping

        ret

  .time:
        lea     esi, [edi+1]
        mov     byte [edi+5], ' '
        add     edi, 6

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
        call    print_text2

        mov     esi, servercommand+1
        call    print_text2

        mov     esi, ctcp_time
        call    print_text2

        ret

  .version:
        mov     esi, str_version
        call    ctcp_reply

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_text2

        mov     esi, servercommand+1
        call    print_text2

        mov     esi, ctcp_version
        call    print_text2

        ret

  .ping:
        lea     esi, [edi+1]
        call    ctcp_reply

        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_text2

        mov     esi, servercommand+1
        call    print_text2

        mov     esi, ctcp_ping
        call    print_text2

        ret



ctcp_reply:

        push    esi

        mov     dword [usercommand], 'NOTI'
        mov     dword [usercommand+4], 'CE  '

        mov     esi, servercommand+1
        mov     edi, usercommand+7
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

        lea     esi, [edi - usercommand]
        mcall   send, [socketnum], usercommand, , 0
  .fail:
        ret



cmd_part:
        add     esi, 5  ; skip 'PART '

; Is it me who parted?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .dont_close

; yes, close the window
        mov     edi, [window_print]
        mov     [edi + window.flags], FLAG_UPDATED + FLAG_CLOSE

        ret

; somebody else parted, just print message
  .dont_close:
        push    esi
        mov     esi, action_header
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        mov     cl, ' '
        call    print_text

        mov     esi, has_left_channel
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

;;; TODO: dec [window.users], remove username from the userlist


        ret



cmd_join:
        add     esi, 5  ; skip 'JOIN '

; compare nick: did we join a channel?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .no_new_window

; create channel window - search for empty slot
        mov     ebx, windows
        mov     ecx, MAX_WINDOWS
  .loop:
        cmp     [ebx + window.data_ptr], 0
        je      .free_found
        add     ebx, sizeof.window
        dec     ecx
        jnz     .loop
; Error: no more available windows!! ;;;;; TODO
  .fail:
        ret

  .free_found:
        push    ebx
        call    window_create
        pop     ebx
        test    eax, eax
        jz      .fail
        mov     [ebx + window.data_ptr], eax
        mov     [ebx + window.type], WINDOWTYPE_CHANNEL
        mov     [ebx + window.flags], 0

        call    window_set_name

        mov     [window_open], ebx
        mov     [window_print], ebx
        call    window_refresh

        push    esi
        mov     esi, action_header
        call    print_text2

        mov     esi, str_talking
        call    print_text2

        pop     eax
        mov     dl, ' '
        call    print_text

        mov     esi, str_dotnewline
        call    print_text2

        call    draw_window

        ret

  .no_new_window:
        push    esi
        call    window_set_name

        mov     esi, action_header
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, joins_channel
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

;;; TODO: inc [window.users], add username to the userlist

        ret




cmd_nick:

        add     esi, 5  ; skip 'NICK '

        push    esi
; Is it me who changed nick?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .not_me

        mov     ecx, MAX_NICK_LEN-1
        push    esi
  .copyloop:
        lodsb
        test    al, al
        jz      .copydone
        cmp     al, ' '
        je      .copydone
        stosb
        dec     ecx
        jnz     .copyloop
  .copydone:
        xor     al, al
        stosb
        pop     esi

  .not_me:

; TODO: if we reach here: change nick in userlist(s)

        push    esi
        mov     esi, action_header_short
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, is_now_known_as
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

        ret




cmd_kick:
        add     esi, 5  ; skip 'KICK '
; Is it me who got kicked?
        mov     edi, servercommand+1
        call    compare_to_nick
        jne     .not_me

; TODO: mark channel as disconnected

  .not_me:
; find the channel user has been kicked from
        push    esi
        mov     esi, action_header_short
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, kicked
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

;;; TODO: dec [window.users], remove username from the userlist

        ret



cmd_quit:

        mov     esi, action_header
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, has_quit_irc
        call    print_text2

;;; TODO: dec [window.users], remove username from the userlist

        ret



cmd_mode:

        add     esi, 5  ; skip 'MODE '

        push    esi
        mov     esi, action_header_short
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, ' '
        call    print_text

        mov     esi, sets_mode
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

;;; TODO: change username if needed

        ret


cmd_353:        ; channel usernames reply

        add     esi, 4  ; skip '353 '

; first, find the channel name
  .loop1:
        lodsb
        cmp     al, '#'
        je      .got_channel
        test    al, al
        jnz     .loop1
        ret

  .got_channel:

; now find the semicolon separating channelname and usernames
  .loop2:
        lodsb
        cmp     al, ':'
        je      .got_list
        test    al, al
        jnz     .loop2
        ret

  .got_list:

; now find window ptr and check if this is the first 353 message
        mov     ebx, [window_print]
        test    [ebx + window.flags], FLAG_RECEIVING_NAMES
        jnz     .start

        or      [ebx + window.flags], FLAG_RECEIVING_NAMES
        mov     [ebx + window.users], 0

  .start:
        mov     eax, [ebx + window.users]
        xor     edx, edx
        mov     ecx, MAX_NICK_LEN
        mul     ecx
        ; eax is now offset
        add     eax, [ebx + window.data_ptr]
        lea     edi, [eax + window_data.names]
        mov     edx, edi

        lea     ecx, [eax + window_data.names]
        add     ecx, MAX_NICK_LEN + MAX_USERS

  .newname:
        cmp     edi, ecx                ; check buffer overflow
        jae     .done
        inc     [ebx + window.users]
  .namesloop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '                 ; names list is separated with spaces
        je      .next
        stosb
        jmp     .namesloop
  .next:
        add     edx, MAX_NICK_LEN
        mov     edi, edx
  .loop3:
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '
        je      .loop3
        stosb
        jmp     .newname

  .done:
        call    redraw_channel_list

        ret





cmd_366:        ; channel usernames end

        mov     ebx, [window_print]
        and     [ebx + window.flags], not FLAG_RECEIVING_NAMES


        ret


cmd_topic:

        add     esi, 4  ; skip '332 '

  .loop:
        lodsb
        test    al, al
        je      .fail
        cmp     al, ':'
        jne     .loop

        push    esi
        mov     esi, action_header
        call    print_text2

        mov     esi, str_topic
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

  .fail:
        ret


cmd_333:

        add     esi, 4  ; skip '333 '

; TODO: check channelname and change pointer accordingly

        mov     ecx, 3  ; number of spaces to find
  .loop:
        lodsb
        test    al, al
        je      .fail
        cmp     al, ' '
        jne     .loop
        dec     ecx
        jnz     .loop   ; find some more spaces

        push    esi
        mov     esi, action_header
        call    print_text2

        mov     esi, str_setby
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

  .fail:
        ret

