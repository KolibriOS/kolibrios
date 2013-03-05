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

        dd      '322 ', cmd_322         ; RPL_LIST
        dd      '323 ', cmd_323         ; RPL_LISTEND
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


align 4
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

align 4
skip_nick:

; First: skip the NICK (maybe we should verify it?)
  .nick:
        lodsb
        cmp     al, ' '
        je      .skip
        cmp     al, ':'
        je      .skip
        jmp     .nick

; skip all leading spaces and semicolons
  .skip:
        lodsb
        cmp     al, ' '
        je      .skip
        cmp     al, ':'
        je      .skip
        dec     esi

        ret


align 4
find_window:                            ; esi is ptr to windowname

        push    esi

        mov     edi, esi
        call    compare_to_nick
        jne     .nochat

        mov     esi, servercommand+1
  .nochat:

; now search for window in list
        mov     ebx, windows
        mov     [window_print], ebx     ; set first window (server window) as default output window
  .scanloop:
        cmp     [ebx + window.data_ptr], 0
        je      .create_it
        push    esi
        lea     edi, [ebx + window.name]
        mov     ecx, MAX_WINDOWNAME_LEN
        repe    cmpsb
        pop     esi
        cmp     byte[edi-1], 0
        je      .got_it
        add     ebx, sizeof.window
        ; TODO: check buffer limits ?
        jmp     .scanloop

; create channel window - search for empty slot
  .create_it:
        mov     ebx, windows
        mov     ecx, MAX_WINDOWS
  .scanloop2:
        cmp     [ebx + window.data_ptr], 0
        je      .free_found
        add     ebx, sizeof.window
        dec     ecx
        jnz     .scanloop2
; Error: no more available windows!
        jmp     .just_skip

  .free_found:
        push    ebx
        call    window_create
        pop     ebx
        test    eax, eax
        jz      .just_skip
        mov     [ebx + window.data_ptr], eax
        mov     [ebx + window.type], WINDOWTYPE_CHAT
        mov     [ebx + window.flags], 0

        call    window_set_name

        mov     [window_open], ebx
        mov     [window_print], ebx
        call    window_refresh

        call    draw_windownames
        jmp     .just_skip

; found it!
  .got_it:
        mov     [window_print], ebx
        call    window_refresh

  .just_skip:
        pop     esi
  .skip1:
; skip text
        lodsb
        test    al, al
        jz      .quit
        cmp     al, ' '
        jne     .skip1
        dec     esi
; now skip trailing spaces and semicolons
  .skip2:
        lodsb
        test    al, al
        jz      .quit
        cmp     al, ' '
        je      .skip2
        cmp     al, ':'
        je      .skip2
        dec     esi

  .quit:
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

        add     esi, 8          ; skip 'PRIVMSG '
        call    find_window     ; esi now points to end of destination name

        cmp     byte[esi], 1
        je      cmd_ctcp

        cmp     dword[esi], 'ACTI'      ; Action?
        je      .action

; nope, just plain old privmsg
if TIMESTAMP
        call    print_timestamp
end if

        push    esi
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
        add     esi, 8
        push    esi
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
        call    print_text2

        mov     bl, 10
        call    print_character

        ret

cmd_ctcp:
        inc     esi

        cmp     dword[esi], 'VERS'
        je      .version

        cmp     dword[esi], 'TIME'
        je      .time

        cmp     dword[esi], 'PING'
        je      .ping

        ret

  .time:
        mov     byte [esi+4], ' '
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
        push    esi
        call    skip_nick
        call    find_window
        pop     esi

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

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_remove

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
        call    find_window

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

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_add

        ret




cmd_nick:
; NOTE: This command applies to a user, and thus has no specific channel
        add     esi, 5  ; skip 'NICK '

        cmp     byte[esi], ':'          ; TODO: skip all spaces and semicolons?
        jne     @f
        inc     esi
       @@:

; Change the nick in the current userlist. TODO: check other channels too!
        push    esi
        mov     ebx, [window_print]

        mov     esi, servercommand+1
        call    user_remove

        mov     esi, [esp]
        call    user_add

        call    redraw_channel_list

; Is it me who changed nick?
        mov     edi, servercommand+1
        call    compare_to_nick
        pop     esi
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

; Now print a message on the current channel
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
        call    skip_nick
        call    find_window

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

        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_remove

        ret



cmd_quit:
; NOTE: This command applies to a user, and thus has no specific channel

        mov     esi, action_header
        call    print_text2

        mov     eax, servercommand+1
        mov     dl, '!'
        call    print_text

        mov     esi, has_quit_irc
        call    print_text2

; TODO: check other channels on same server too!
        mov     ebx, [window_print]
        mov     esi, servercommand+1
        call    user_remove

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


cmd_353:                ; channel usernames reply

        add     esi, 4  ; skip '353 '
        call    skip_nick
        inc     esi     ; channel type '*', '=' or '@'
        inc     esi     ; ' '
        call    find_window

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
        call    redraw_channel_list

        ret





cmd_366:        ; channel usernames end

        add     esi, 4          ; skip '366 '
        call    skip_nick
        call    find_window

        mov     ebx, [window_print]
        and     [ebx + window.flags], not FLAG_RECEIVING_NAMES

        ret




cmd_topic:

        add     esi, 4          ; skip '332 '
        call    skip_nick
        call    find_window

        push    esi
        mov     esi, action_header
        call    print_text2

        mov     esi, str_topic
        call    print_text2

        pop     esi
        call    print_text2

        mov     esi, str_newline
        call    print_text2

        ret


cmd_333:

        add     esi, 4          ; skip '333 '
        call    skip_nick               ;;;;
        call    find_window

;        mov     ecx, 2  ; number of spaces to find    ;;; CHECKME
;  .loop:
;        lodsb
;        test    al, al
;        je      .fail
;        cmp     al, ' '
;        jne     .loop
;        dec     ecx
;        jnz     .loop   ; find some more spaces

        push    esi
        mov     esi, action_header
        call    print_text2

        mov     esi, str_setby
        call    print_text2

;        pop     esi
;        call    print_text2

        pop     eax
        mov     dl, '!'
        call    print_text

        mov     esi, str_newline
        call    print_text2

  .fail:
        ret

cmd_322:
        add     esi, 4

        call    skip_nick

        call    print_text2

        mov     esi, str_newline
        call    print_text2

        ret

cmd_323:

        ret