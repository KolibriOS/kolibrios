;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
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
        lodsb                           ; get one character of the string
        test    al, al                  ; end of string?
        jz      .almost_done
        cmp     al, ' '                 ; it's a space! remember its position
        je      .space
        cmp     al, 13                  ; we already inserted a newline once, make it a space again
        je      .soft_nl
        cmp     al, 10                  ; it's a newline, continue onto the next line
        jne     .more
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

; When you set the direction flag before calling, you can also scan for previous line!
; in: esi
; out:esi
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
        dec     ecx
        jnz     .loop
  .done:

        ret


print_text:                             ; eax = start ptr
                                        ; dl = end char
        pusha
  ptr2:
        mov     bl, [eax]

        cmp     bl, dl
        je      .done
        test    bl, bl
        jz      .done
        call    print_character

        inc     eax
        jmp     ptr2

  .done:
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


print_character:

        push    esi edi
        mov     esi, [window_print]
        mov     edi, [esi + window.text_write]
        mov     byte[edi], bl
        inc     edi
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
        mov     edx, [scroll2.position]
        test    edx, edx
        jz      .yesscroll
        add     edx, [scroll2.cur_area]
        sub     edx, [scroll2.max_area]
        jne     .noscroll
  .yesscroll:
; Scrollbar was at lowest position, scroll down automatically when new text arrived.
        mov     edx, [edi + window.text_lines]
        sub     edx, [textbox_height]
        jle     .noscroll                               ; There are less lines of text than fit into the window, dont scroll..
        sub     edx, [edi + window.text_line_print]
        je      .noscroll                               ; We are already at the bottom pos, dont scroll..
  .scroll_to_pos:               ; edx = number of lines to go up/down (flags must indicate direction)
        pushf
        add     [edi + window.text_line_print], edx
        mov     esi, [edi + window.text_print]
        popf
        ja      .loop_forward
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
        mov     eax, [textbox_height]   ; max number of lines to draw
        add     ebx, TEXT_X shl 16 + TEXT_Y
        mov     ecx, [colors.work_text] ; default text color
        mov     edx, [edi + window.text_print]
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
        mcall   13                      ; draw rectangle
        popa

        mov     esi, edx
        add     esi, [textbox_width]
  .line:
        cmp     byte[edx], 0
        je      .end_of_text

        cmp     byte[edx], 13
        je      .newline_soft

        cmp     byte[edx], 10
        je      .newline_hard

        push    esi eax
        cmp     byte[edx], 3            ; escape code for mIRC colors
        jne     .no_colors
        inc     edx
        call    dec_to_esi
        jz      .no_colors
        mov     ecx, [irc_colors + 4*esi]

        cmp     byte[edx], ','          ; background color?
        jne     .no_colors
        inc     edx
        call    dec_to_esi
        jz      .no_colors
        mov     edi, [irc_colors + 4*esi]
        or      ecx, 0x40000000
  .no_colors:

  .draw:
        mov     esi, 1
        mcall   4                       ; draw text
        add     ebx, FONT_WIDTH shl 16
        inc     edx
        pop     eax esi
        cmp     edx, esi
        jb      .line
        jmp     .line_full

  .newline_hard:
        mov     ecx, [colors.work_text]
  .newline_soft:
        inc     edx
  .line_full:
        and     ebx, 0x0000ffff
        add     ebx, TEXT_X shl 16 + FONT_HEIGHT
        dec     eax
        jnz     .drawloop
  .end_of_text:

; Update and draw scrollbar when nescessary
        mov     edi, [window_active]
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