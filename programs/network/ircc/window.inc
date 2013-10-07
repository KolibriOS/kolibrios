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


window_create:

; allocate the window data block
        mcall   68, 12, sizeof.window_data
        test    eax, eax
        jz      .fail

; fill it with all zeros
        push    eax
        mov     edi, eax
        mov     ecx, (sizeof.window_data+3)/4
        xor     eax, eax
        rep     stosd
        pop     eax

  .fail:
        ret


window_set_name:    ; esi = ptr to name, ebx = window ptr

        pusha

; Skip heading spaces
  .spaceloop:
        cmp     byte[esi], ' '
        jne     .done
        inc     esi
        jmp     .spaceloop
  .done:

; Now copy it
        lea     edi, [ebx + window.name]
        mov     ecx, MAX_WINDOWNAME_LEN
  .loop:
        lodsb
        cmp     al, 0x21
        jbe     .addzero
        stosb
        dec     ecx
        jnz     .loop
  .addzero:
        xor     al, al
        stosb

        call    draw_windowtabs         ; redraw it

        popa

        ret



window_refresh:

; set the correct buffer pointers       ; FIXME: what is it good for?
        mov     eax, [textbox_width]    ;
        imul    eax, 11                 ;
        mov     [pos], eax              ;

        mov     eax, [window_print]
        mov     eax, [eax + window.data_ptr]
        add     eax, window_data.text
        mov     [text_start], eax

        ret



window_is_updated:

        mov     edi, [window_print]
        test    [edi + window.flags], FLAG_UPDATED
        jnz     .skip

        or      [edi + window.flags], FLAG_UPDATED

; now play a sound :)

        call    draw_windowtabs         ; highlight updated tabs
  .skip:

        ret



window_close:

; If current window is a channel, send part command to server
        mov     esi, [window_active]
        cmp     [esi + window.type], WINDOWTYPE_CHANNEL
        jne     .not_channel

        lea     esi, [esi + window.name]
        call    cmd_usr_part_channel
  .not_channel:

; Remove the window (overwrite current structure with trailing ones)
        mov     edi, [window_active]
        push    [edi + window.data_ptr]         ; remember data ptr so we can free it later
        lea     esi, [edi + sizeof.window]
        mov     ecx, windows + MAX_WINDOWS*sizeof.window
        sub     ecx, esi
        rep     movsb

; Completely zero the trailing window block (there will always be one!)
        mov     ecx, sizeof.window
        xor     al, al
        rep     stosb

; free the window data block
        pop     ecx
        mcall   68, 13

; We closed this window so we need to show another
        mov     edi, [window_active]
        cmp     [edi + window.data_ptr], 0
        jne     @f
        sub     edi, sizeof.window
        mov     [window_active], edi
        mov     [window_print], edi
  @@:

; At last, redraw everything
        call    draw_window

        ret




; open a window with a given name, if it does not exist, create it
; This procedure only affects window_print ptr, not window_active!
;
; esi = ptr to ASCIIZ windowname
window_open:

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
        mov     [window_print], ebx
        call    window_refresh

        call    draw_windowtabs
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


print_text:                             ; eax = start ptr
                                        ; dl = end char
        pusha
  ptr2:
        mov     bl, [eax]

        cmp     bl, dl
        je      ptr_ret
        cmp     bl, 0
        je      ptr_ret
        call    print_character

        inc     eax
        jmp     ptr2

  ptr_ret:
        popa
        ret


print_text2:                            ; esi = ptr to ASCIIZ string

        pusha
  .loop:
        lodsb
        test    al, al
        jz      .done
        mov     bl, al
        call    print_character
        jmp     .loop

  .done:
        popa
        ret


if TIMESTAMP
print_timestamp:

        pusha
        mcall   3                       ; get system time

        mov     bl, '['
        call    print_character
        mov     ecx, TIMESTAMP
  .loop:
        mov     bl, al
        shr     bl, 4
        add     bl, '0'
        call    print_character

        mov     bl, al
        and     bl, 0x0f
        add     bl, '0'
        call    print_character

        dec     ecx
        jz      .done

        mov     bl, ':'
        call    print_character
        shr     eax, 8
        jmp     .loop
  .done:
        mov     bl, ']'
        call    print_character
        mov     bl, ' '
        call    print_character

        popa
        ret
end if