;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2018. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; window_create_textbox
; Initialises the data structure for our multiline textbox
;
; in:   window ptr in ebx
; out:  eax = 0 on error
;       ecx, edi = destroyed

window_create_textbox:

        push    ebx
; allocate the window data block
        mcall   68, 12, sizeof.window_data
        test    eax, eax
        pop     ebx
        jz      .fail

; fill it with all zeros
        push    eax
        mov     edi, eax
        mov     ecx, (sizeof.window_data+3)/4
        xor     eax, eax
        rep stosd
        pop     eax

        mov     [ebx + window.data_ptr], eax
        mov     [ebx + window.flags], FLAG_SCROLL_LOW

        add     eax, window_data.text+2         ; let text begin at offset 2, this way the text will be prepended with two null bytes
        mov     [ebx + window.text_start], eax
        mov     [ebx + window.text_print], eax
        mov     [ebx + window.text_write], eax
        mov     [ebx + window.text_scanned], eax
        mov     [ebx + window.text_lines], 0
        mov     [ebx + window.text_line_print], 0
        add     eax, TEXT_BUFFERSIZE-1
        mov     [ebx + window.text_end], eax

  .fail:
        ret


; window_set_name
; Fills in the window name in window structure
;
; IN:   esi = ptr to name
;       ebx = window ptr
; OUT:  esi = ptr to next parameter
;       ebx = window ptr
;       eax, ecx, edx, edi = destroyed

window_set_name:

        lea     edi, [ebx + window.name]
        mov     ecx, MAX_WINDOWNAME_LEN
  .loop:
        lodsb
        cmp     al, 0x21        ; name ends with 0, space or !
        jbe     .addzero
        stosb
        dec     ecx
        jnz     .loop
  .addzero:
        xor     al, al
        stosb

        push    esi ebx
        call    draw_window_tabs
        pop     ebx esi

        ret

window_close_all:

  .loop:
        call    window_close

        cmp     [window_print], 0
        jne     .loop

        ret


window_close:   ; closes the 'print' window

; Remove the window (overwrite current structure with trailing ones)
        mov     edi, [window_print]
        push    [edi + window.data_ptr]         ; remember data ptr so we can free it later
        lea     esi, [edi + sizeof.window]
        mov     ecx, windows + MAX_WINDOWS*sizeof.window
        sub     ecx, esi
        rep movsb

; Completely zero the trailing window block (there will always be one!)
        mov     ecx, sizeof.window
        xor     al, al
        rep stosb

; Clear the window data block to erase the chat history and nicknames 
        mov     edi, [esp]
        mov     ecx, sizeof.window_data         ; TEXT_BUFFERSIZE + MAX_NICK_LEN * MAX_USERS
        rep stosb

; And now free the window data block
        pop     ecx
        mcall   68, 13

; We closed this window so we need to show another
        ; Is there still a window in the active position?
        mov     edi, [window_active]
        cmp     [edi + window.data_ptr], 0
        jne     .redraw
        ; Did we just close the last window?
        cmp     edi, windows
        je      .closed_last
        ; Nope, move one window to the left
        sub     edi, sizeof.window
        mov     [window_active], edi
        mov     [window_print], edi
        jmp     .redraw

  .closed_last:
        xor     edi, edi
        mov     [window_active], edi
        mov     [window_print], edi

; At last, redraw everything
  .redraw:
        call    draw_window

        ret


; window_find:
; search for a window with given name in the window list
;
; IN:   esi = ptr to start of window name
; OUT:  ebx = window ptr, or 0 if none found
;       esi = ptr to end of window name, if window was found

window_find:

        mov     ebx, windows
        mov     eax, MAX_WINDOWS
  .scanloop:
        push    esi
        cmp     [ebx + window.type], WINDOWTYPE_NONE
        je      .try_next
        lea     edi, [ebx + window.name]
        mov     ecx, MAX_WINDOWNAME_LEN
        repe    cmpsb
        cmp     byte[edi-1], 0          ; last equall character was null? yes, the strings match!
        je      .got_it
        cmp     byte[edi], 0            ; we're at the end of string1.. ?
        jne     .try_next
        cmp     byte[esi], 0x21         ; and the end of string2? yes!
        jbe     .got_it
  .try_next:
        pop     esi
        add     ebx, sizeof.window
        dec     eax
        jnz     .scanloop

        xor     ebx, ebx
        ret

  .got_it:
        add     esp, 4
        ret



; window_open:
; open a window with a given name, if it does not exist, create it
; This procedure only affects window_print ptr, not window_active!
;
; IN:   esi = ptr to ASCIIZ windowname
; OUT:  esi = ptr to next parameter
;       ebx = window ptr/0 on error

window_open:

; Skip heading spaces
        lodsb
        cmp     al, ' '
        je      window_open
        cmp     al, ':'
        je      window_open
        dec     esi

        call    window_find
        test    ebx, ebx
        jnz     .got_it

; create channel window - search for empty slot
  .create_it:
        mov     ebx, windows
        mov     ecx, MAX_WINDOWS
  .scanloop2:
        cmp     [ebx + window.type], WINDOWTYPE_NONE
        je      .free_found
        add     ebx, sizeof.window
        dec     ecx
        jnz     .scanloop2
        jmp     .error

  .free_found:
        call    window_create_textbox
        test    eax, eax
        jz      .error
        mov     [ebx + window.type], WINDOWTYPE_CHAT    ; FIXME: let caller handle this ?

        call    window_set_name

  .got_it:
        lodsb
        cmp     al, ' '
        je      .got_it
        cmp     al, ':'
        je      .got_it
        dec     esi

        mov     [window_print], ebx
        ret

  .error:
        xor     ebx, ebx
        ret