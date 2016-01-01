;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; TODO: work correctly with user prefixes.
; use first byte of nick for prefix ONLY (use a space for those that do not have special powers..)

user_prefixes db '~&@%+ ', 0    ; in descending order

; TODO: update selected user if needed
; esi is ptr to nick
; ebx is ptr to window
align 4
user_add:

        cmp     [ebx + window.users], MAX_USERS
        jae     .fail

        mov     edi, [ebx + window.data_ptr]
        add     edi, window_data.names
        mov     ebp, [ebx + window.users]
        inc     ebp             ; CHECKME

        push    esi edi
  .restart:
        mov     ecx, MAX_NICK_LEN
  .loop1:
        lodsb
        cmp     al, '@'
        jne     @f
        mov     al, ' '         ; give @ highest priority
  @@:
        cmp     al, 'A'
        jb      @f
        cmp     al, 'Z'
        ja      @f
        add     al, 'a' - 'A'   ; convert to lowercase
  @@:
        dec     ecx
        jz      .got_it

  .loop2:
        mov     dl, [edi]
        cmp     dl, 0
        je      .got_it
        cmp     dl, '@'
        jne     @f
        mov     dl, ' '         ; give @ highest priority
  @@:
        cmp     dl, 'A'
        jb      @f
        cmp     dl, 'Z'
        ja      @f
        add     dl, 'a' - 'A'   ; convert to lowercase
  @@:
        cmp     al, dl
        jb      .got_it
        je      .check_next

        pop     edi esi
        add     edi, MAX_NICK_LEN
        push    esi edi

        dec     ebp
        jnz     .restart

  .check_next:
        inc     edi
        jmp     .loop1

  .got_it:
        pop     edi esi

; OK, insert it here..

; mov all trailing usernames by MAX_NICK_LEN bytes
        push    esi edi
        mov     esi, [ebx + window.data_ptr]
        add     esi, window_data.names + MAX_NICK_LEN * (MAX_USERS - 1) - 4     ; -4 because we're copying backward, dword wise
        mov     ecx, esi
        sub     ecx, edi
        add     ecx, MAX_NICK_LEN
        shr     ecx, 2
        lea     edi, [esi + MAX_NICK_LEN]
        std
        rep     movsd
        cld
        pop     edi esi

; Now insert our new username
        mov     ecx, MAX_NICK_LEN-1
  .fill:
        lodsb
        cmp     al, ' '
        je      .done
        cmp     al, '!'
        je      .done
        cmp     al, 13
        je      .done
        cmp     al, 10
        je      .done
        stosb
        loop    .fill
  .done:
        xor     al, al
        stosb

        inc     [ebx + window.users]
        or      [ebx + window.flags], FLAG_UPDATED
  .fail:
        ret




; TODO: update selected user if needed
; esi is ptr to nick
; ebx is ptr to window
align 4
user_remove:

        call    user_find
        jz      .fail

        lea     esi, [edi + MAX_NICK_LEN]
        mov     ecx, [ebx + window.data_ptr]
        add     ecx, window_data.names + MAX_NICK_LEN * MAX_USERS
        sub     ecx, esi
        shr     ecx, 2
        rep     movsd

        dec     [ebx + window.users]
        or      [ebx + window.flags], FLAG_UPDATED
  .fail:
        ret



; IN:
; esi is ptr to nick
; ebx is ptr to window
; OUT:
; edi is ptr to nick in userlist
align 4
user_find:

        mov     eax, [ebx + window.users]
        test    eax, eax
        jz      .fail
        mov     edi, [ebx + window.data_ptr]
        add     edi, window_data.names

  .loop:
        push    esi edi
        mov     ecx, MAX_NICK_LEN
        repe    cmpsb
        cmp     byte[edi-1], 0
        je      .got_it
        ; TODO: check byte[esi] too!
        pop     edi esi
        add     edi, MAX_NICK_LEN
        dec     eax
        jnz     .loop
        jmp     .fail

  .got_it:
        pop     edi esi
        test    edi, edi        ; to clear zero flag
        ret

  .fail:
        xor     edi, edi
        ret



user_remove_all:

        xor     eax, eax
        mov     edi, [window_print]
        mov     [edi + window.users], eax
        mov     [edi + window.selected], eax
        mov     edi, [edi + window.data_ptr]
        add     edi, window_data.names
        mov     ecx, MAX_NICK_LEN * MAX_USERS / 4
        rep stosd

        ret