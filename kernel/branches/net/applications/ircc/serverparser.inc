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

        ret



cmd_join:
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
        mov     dl, ' '
        call    print_text

        mov     esi, joins_channel
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

        ret




cmd_nick:       ; FIXME

        push    esi
; test for change of my nick
        mov     esi, servercommand+1
        mov     edi, user_nick
        mov     ecx, MAX_NICK_LEN
        rep     cmpsb
        cmp     byte[edi-1], 0
        jne     .notmy
        cmp     byte[esi-1], '!'
        jne     .notmy

; yes, this is my nick, set to new
        pop     esi
        or      ecx, -1
        mov     edi, esi
        xor     eax, eax
        repne   scasb
        neg     ecx
        cmp     ecx, MAX_NICK_LEN
        jb      @f
        mov     ecx, MAX_NICK_LEN
      @@:
        mov     edi, user_nick
        rep     movsb
  .notmy:

; replace nick in all lists of users
        mov     ebx, windows
  .channels:
        mov     esi, [ebx + window.data_ptr]
        lea     esi, [esi + window_data.names]
;;;;;        mov     edx, [esi + window_data.nameslen]
        add     edx, esi
  .nicks:
        mov     edi, servercommand+1
        cmp     byte[esi], '@'
        jne     @f
        inc     esi

  @@:
        cmp     esi, edx
        jae     .srcdone
        lodsb
        cmp     al, ' '
        je      .srcdone
        scasb
        je      @b

  @@:
        cmp     esi, edx
        jae     .nextchannel
        lodsb
        cmp     al, ' '
        jne     @b

  .nextnick:
        cmp     esi, edx
        jae     .nextchannel
        lodsb
        cmp     al, ' '
        jz      .nextnick
        dec     esi
        jmp     .nicks


  .srcdone:
        cmp     byte [edi], '!'
        jne     .nextnick

; here we have esi -> end of nick which must be replaced to [servercommand_position]+6
        lea     edx, [edi-servercommand-1]
        sub     esi, edx
        or      ecx, -1
        xor     eax, eax
;        mov     edi, [servercommand_position]        ;;;;; FIXME
        repnz   scasb
        not     ecx
        dec     ecx
        push    ecx
        cmp     ecx, edx
        jb      .decrease
        jz      .copy
  .increase:

; new nick is longer than the old
        push    esi
        lea     edi, [ebx+120*10] ;;;;;;
        lea     esi, [edi+edx]
        sub     esi, ecx
        mov     ecx, esi
        sub     ecx, [esp]
        dec     esi
        dec     edi
        std
        rep     movsb
        cld
        pop     esi
        jmp     .copy
  .decrease:
; new nick is shorter than the old
        push    esi
        lea     edi, [esi+ecx]
        add     esi, edx
        lea     ecx, [ebx+120*10]
        sub     ecx, edi
        rep     movsb
        pop     esi
  .copy:
; copy nick
        mov     edi, esi
        dec     edi
;        mov     esi, [servercommand_position]        ;;;;; FIXME
        pop     ecx
        sub     edx, ecx
        sub     [ebx-4], edx
        rep     movsb
        mov     al, ' '
        stosb

  .nextchannel:
        add     ebx, sizeof.window
        cmp     ebx, windows + sizeof.window*MAX_WINDOWS
        jb      .channels

;        mov  [text_start], window_text + 120*80
 new_all_channels3:

        mov     esi, action_header_short
        call    print_text2

        mov     eax, servercommand+1
        mov     dl,'!'
        call    print_text

        mov     esi,is_now_known_as
        call    print_text2

;       mov     esi,[servercommand_position]        ;;;;; FIXME
        call    print_text2

;;;        call    notify_channel_thread

; go to next window (and check if its a channel!)

;        add     [text_start], 120*80
;        cmp     [text_start], I_END+120*80*20
;        jb      new_all_channels3

        ret




cmd_kick:
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
        mov     dl,'!'
        call    print_text

        mov     esi, kicked
        call    print_text2

        pop     esi
        call    print_text2

        ret



cmd_quit:

        mov     esi, action_header
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, has_quit_irc
        call    print_text2

        ret



cmd_mode:

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

        ret


cmd_353:        ; channel usernames reply

; TODO: mark a bit that we are receiving names

; first, find the channel name
  .loop1:
        lodsb
        cmp     al, '#'
        je      .got_channel
        test    al, al
        jnz     .loop1

        ret

  .got_channel:
;        call    find_channel   ;;;; ASSUME current channel for now
        mov     ebx, [window_print]
        mov     ebx, [ebx + window.data_ptr]
        lea     edi, [ebx + window_data.names]
        lea     edx, [edi + MAX_NICK_LEN]

; now find the semicolon separating channelname and usernames
  .loop2:
        lodsb
        cmp     al, ':'
        je      .namesloop
        test    al, al
        jnz     .loop2

        ret

  .namesloop:
; now the names list begins, separated with spaces
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '
        jz      .next
        stosb
        jmp     .namesloop

  .next:
        mov     edi, edx
        add     edx, MAX_NICK_LEN

;;;        cmp     edi, ..      ; Check for buffer overflow
        jmp     .namesloop

  .done:
        call    print_channel_list

        ret




cmd_366:        ; channel usernames end

; TODO: clear the bit that we are receiving names


        ret


cmd_topic:

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

