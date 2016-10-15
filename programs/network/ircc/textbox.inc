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


text_insert_newlines:                   ; esi = ASCIIZ string

        xor     edx, edx                ; number of lines of text
        cmp     byte[esi], 0
        je      .done
  .next_line:
        xor     ebx, ebx
        mov     ecx, [textbox_width]
        inc     ecx
  .more:
        dec     ecx
        jz      .end_of_line
  .next_byte:
        lodsb                           ; get one character of the string
        test    al, al                  ; end of string?
        jz      .almost_done
        cmp     al, ' '                 ; it's a space! remember its position
        je      .space
        cmp     al, 13                  ; we already inserted a newline once, make it a space again
        je      .soft_nl
        cmp     al, 10                  ; it's a newline, continue onto the next line
        je      .newline
        and     al, 0xc0                ; Is it a multi byte UTF8 char?
        cmp     al, 0x80
        je      .next_byte
        jmp     .more
  .newline:
        inc     edx
        jmp     .next_line
  .soft_nl:
        mov     byte[esi-1], ' '
  .space:
        mov     ebx, esi                ; last detected space
        jmp     .more
  .end_of_line:
        inc     edx
        test    ebx, ebx                ; did we detect any spaces on this line?
        jz      .next_line              ; no:   just continue onto the next line
        mov     byte[ebx-1], 13         ; yes:  replace last space on line with a soft newline
        mov     esi, ebx                ;       and continue parsing just after last space
        jmp     .next_line              ;
  .almost_done:
        dec     esi
  .done:

        ret


;----------------------------------
; scan untill next line is reached
;
; When you set the direction flag before calling, you can also scan for previous line!
; IN:   esi
; OUT:  esi
;----------------------------------
text_nextline:

        mov     ecx, [textbox_width]
  .loop:
        lodsb
        test    al, al
        jz      .done
        cmp     al, 10
        je      .done
        cmp     al, 13
        je      .done
        and     al, 0xc0
        cmp     al, 0x80
        je      .loop           ; This byte is the second, third or fourth byte of a multi-byte UTF8 char
        dec     ecx
        jnz     .loop
  .done:
        ret

;----------------------------------
; print string
;
; IN:   esi = ptr to string
;       bl = char which marks end of string
; OUT:  esi = ptr to end of str
;----------------------------------
print_string:

        push    eax
  .loop:
        lodsb
        cmp     al, bl
        je      .done
        cmp     al, 13
        je      .loop
        test    al, al
        jz      .done
        call    print_char
        jmp     .loop
  .done:
        pop     eax

        ret


;----------------------------------
; print ASCIIZ string
;
; IN:   esi = ptr to ASCIIZ string
; OUT:  esi = ptr to end of str
;----------------------------------
print_asciiz:

        push    eax
  .loop:
        lodsb
        test    al, al
        jz      .done
        call    print_char
        jmp     .loop
  .done:
        pop     eax

        ret


;----------------------------------
; print character
;
; IN:   al = char to print
; OUT:  /
;----------------------------------
print_char:

        push    esi edi
        mov     esi, [window_print]
        mov     edi, [esi + window.text_write]
        stosb
        cmp     edi, [esi + window.text_end]
        jae     .uh_ow
        mov     [esi + window.text_write], edi
  .continue:
        or      [esi + window.flags], FLAG_UPDATED
        pop     edi esi

        ret

  .uh_ow:
        pusha
        mov     edi, [esi + window.text_start]
        mov     [esi + window.text_print], edi
        lea     esi, [edi + TEXT_BUFFERSIZE/2]
        call    text_nextline
        mov     ecx, TEXT_BUFFERSIZE/8
        rep     movsd
        mov     esi, edi
        call    text_insert_newlines

        mov     ebx, [window_print]
        mov     [ebx + window.text_lines], edx
        mov     [ebx + window.text_scanned], esi
        mov     [ebx + window.text_write], esi
        mov     [ebx + window.text_line_print], 0
        popa

        jmp     .continue



draw_channel_text:

        mov     edi, [window_active]
        and     [edi + window.flags], not FLAG_UPDATED  ; clear the 'window is updated' flag

; Scan new text for newlines
        mov     esi, [edi + window.text_scanned]
        call    text_insert_newlines
        add     [edi + window.text_lines], edx
        mov     [edi + window.text_scanned], esi

; Is scrollbar at lowest position?
        test    [edi + window.flags], FLAG_SCROLL_LOW
        jnz     .yesscroll                              ; Yes
        cmp     [scroll2.all_redraw], 1                 ; No
        jnz      .noscroll
        mov     edx, [textbox_height]
        add     edx, [edi + window.text_line_print]
    cmp edx, [edi + window.text_lines]
        jl      .noscroll
  .yesscroll:
; Scrollbar was at lowest position, scroll down automatically when new text arrived.
        mov     edx, [edi + window.text_lines]
        sub     edx, [textbox_height]
        jg      @f
        mov     [edi + window.text_line_print], 0
        jmp     .noscroll                               ; There are less lines of text than fit into the window, dont scroll..
  @@:
        sub     edx, [edi + window.text_line_print]
        je      .noscroll                               ; We are already at the bottom pos, dont scroll..
  .scroll_to_pos:               ; edx = number of lines to go up/down (flags must indicate direction)
        pushf
        add     [edi + window.text_line_print], edx
        mov     esi, [edi + window.text_print]
        popf
        jg      .loop_forward
        std                     ; set direction flag so we can scan backwards
        dec     esi
        dec     esi             ; move our cursor just in front of newline, for scanning backwards
  .loop_backward:
        call    text_nextline
        inc     edx
        jnz     .loop_backward
        inc     esi
        inc     esi             ; move the cursor just after last newline
        cld
        jmp     .ok

  .loop_forward:
        call    text_nextline
        dec     edx
        jnz     .loop_forward
  .ok:
        mov     [edi + window.text_print], esi
  .noscroll:

; Update and draw scrollbar when nescessary
        mov     edx, [edi + window.text_lines]
        cmp     edx, [textbox_height]
        jbe     .scroll_done

        mov     [scroll2.max_area], edx
        mov     eax, [edi + window.text_line_print]
        mov     [scroll2.position], eax

        push    dword scroll2                   ; redraw scrollbar
        call    [scrollbar_draw]
        mov     [scroll2.all_redraw], 0         ; next time, dont redraw it completely
  .scroll_done:

; Calculate start offset coordinates (align text to bottom)
        mov     ebx, [textbox_height]
        sub     ebx, [edi + window.text_lines]
        jb      .no_offset
        imul    ebx, FONT_HEIGHT
        push    [edi + window.text_start]
        pop     [edi + window.text_print]
        jmp     .draw_text
  .no_offset:
        xor     ebx, ebx
  .draw_text:

; Prepare to actually draw some text
        add     ebx, TEXT_X shl 16 + TEXT_Y     ; text coordinates
        mov     ecx, [colors.work_text]         ; default text color
        or      ecx, 0x30000000
        mov     edx, [edi + window.text_print]  ; start of text to print

; Scan backwards on line for color escape codes
        mov     esi, edx
        push    edx
        std
  @@:
        lodsb
        cmp     al, 0           ; end of text
        je      @f
        cmp     al, 10          ; hard newline
        je      @f
        cmp     al, 3           ; mIRC escape code
        jne     @b

        cld
        lea     edx, [esi+2]
        call    dec_to_esi
        jz      @f
        mov     ecx, [irc_colors + 4*esi]
        or      ecx, 0x30000000                 ; UTF-8 text

        cmp     byte[edx], ','                  ; background color?
        jne     @f
        inc     edx
        call    dec_to_esi
        jz      @f
        mov     edi, [irc_colors + 4*esi]
        or      ecx, 0x40000000                 ; enable background color
  @@:
        cld

        pop     edx
        mov     eax, [textbox_height]           ; max number of lines to draw
  .drawloop:
        cmp     byte[edx], 0
        je      .end_of_text

; Clear one row of characters
        pusha
        mov     cx, bx
        shl     ecx, 16
        mov     cx, FONT_HEIGHT
        mov     ebx, TEXT_X shl 16
        mov     bx, word[textbox_width]
        imul    bx, FONT_WIDTH
        mov     edx, [colors.work]
        mcall   13                              ; draw rectangle
        popa

        push    eax
        mov     esi, [textbox_width]
  .line:
        cmp     byte[edx], 0
        je      .end_of_text
        cmp     byte[edx], 13
        je      .newline_soft
        cmp     byte[edx], 10
        je      .newline_hard

        push    esi
        cmp     byte[edx], 3                    ; escape code for mIRC colors
        jne     .no_colors
        inc     edx
        call    dec_to_esi
        jz      .no_colors
        mov     ecx, [irc_colors + 4*esi]
        or      ecx, 0x30000000

        cmp     byte[edx], ','                  ; background color?
        jne     .no_colors
        inc     edx
        call    dec_to_esi
        jz      .no_colors
        mov     edi, [irc_colors + 4*esi]
        or      ecx, 0x40000000
  .no_colors:

        mov     esi, 1
        mcall   4                               ; draw text

        mov     esi, 1
        mov     al, byte[edx]
        test    al, 10000000b
        jz      @f
        mov     esi, 4
        and     al, 11111000b
        cmp     al, 11110000b
        je      @f
        dec     esi
        and     al, 11110000b
        cmp     al, 11100000b
        je      @f
        dec     esi
  @@:

        add     ebx, FONT_WIDTH shl 16
        add     edx, esi
        pop     esi
        dec     esi
        jnz     .line
        jmp     .line_full

  .newline_hard:
        mov     ecx, [colors.work_text]
        or      ecx, 0x30000000
  .newline_soft:
        inc     edx
  .line_full:
        and     ebx, 0x0000ffff
        add     ebx, TEXT_X shl 16 + FONT_HEIGHT
        pop     eax
        dec     eax
        jnz     .drawloop
  .end_of_text:

        ret




dec_to_esi:

        xor     esi, esi
  .loop:
        movzx   eax, byte[edx]
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        inc     edx
        lea     esi, [esi*4 + esi]      ; esi * 5
        lea     esi, [esi*2 + eax]      ; esi * 2 + eax
        jmp     .loop
  .done:
        cmp     esi, 16
        jae     .fail
        ret

  .fail:
        xor     esi, esi
        ret



if TIMESTAMP
print_timestamp:

        pusha
        mcall   3                       ; get system time
        mov     ebx, eax

        mov     al, '['
        call    print_char
        mov     ecx, TIMESTAMP
  .loop:
        mov     al, bl
        shr     al, 4
        add     al, '0'
        call    print_char

        mov     al, bl
        and     al, 0x0f
        add     al, '0'
        call    print_char

        dec     ecx
        jz      .done

        mov     al, ':'
        call    print_char
        shr     ebx, 8
        jmp     .loop
  .done:
        mov     al, ']'
        call    print_char
        mov     al, ' '
        call    print_char

        popa
        ret
end if