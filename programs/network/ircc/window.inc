;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; in:  window ptr in ebx
; out: eax = 0 on error
window_create:

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
        rep     stosd
        pop     eax

        mov     [ebx + window.data_ptr], eax
        mov     [ebx + window.flags], 0

        add     eax, window_data.text+1         ; let text begin at offset 1, this way the text will be prepended with a 0
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



window_is_updated:

        mov     edi, [window_print]
        cmp     edi, [window_active]
        je      .skip
        test    [edi + window.flags], FLAG_UPDATED
        jnz     .skip
        or      [edi + window.flags], FLAG_UPDATED

; TODO: make some noise

        call    draw_windowtabs         ; highlight updated tabs
  .skip:

        ret



window_close:   ; closes the 'print' window

; Remove the window (overwrite current structure with trailing ones)
        mov     edi, [window_print]
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
        mov     [window_print], edi  ;;;;;;;;
  @@:

; At last, redraw everything
        call    draw_window

        ret



window_find:    ; esi = window name
; search for window in list
        mov     ebx, windows
        mov     [window_print], ebx     ; set first window (server window) as default output window
        mov     eax, MAX_WINDOWS
  .scanloop:
        push    esi
        cmp     [ebx + window.type], WINDOWTYPE_NONE
        je      .try_next
        lea     edi, [ebx + window.name]
        mov     ecx, MAX_WINDOWNAME_LEN
        repe    cmpsb
        cmp     byte[edi-1], 0
        jne     .try_next
        cmp     byte[esi-1], 0
        je      .got_it
        cmp     byte[esi-1], 10
        je      .got_it
        cmp     byte[esi-1], 13
        je      .got_it
        cmp     byte[esi-1], ' '
        je      .got_it
  .try_next:
        pop     esi
        add     ebx, sizeof.window
        dec     eax
        jnz     .scanloop

        xor     ebx, ebx
        ret

  .got_it:
        pop     esi                 ;;; TODO: dont reset ESI  ?
        mov     [window_print], ebx
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

        call    window_find
        test    ebx, ebx
        jne     .got_it

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
        jmp     .got_it ; TODO: return error

  .free_found:
        call    window_create
        test    eax, eax
        jz      .got_it ; TODO: return error
        mov     [ebx + window.type], WINDOWTYPE_CHAT    ; FIXME: let caller handle this ?

        call    window_set_name
        mov     [window_print], ebx

        call    draw_windowtabs

  .got_it:
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