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



; Character in bl
print_character:

        pusha
        mov     ecx, TEXTBOX_LINES
        imul    ecx, [textbox_width]
        mov     esi, [text_start]

        cmp     bl, 10                  ; line down
        je      .linefeed

        mov     eax, [text_pos]
        mov     byte[esi + eax], bl     ; write the byte
        inc     [text_pos]

        cmp     [text_pos], ecx
        jb      .done

  .linefeed:
; scroll all text one line to the top
        mov     edi, esi
        add     esi, [textbox_width]
        rep     movsb

        mov     ecx, TEXTBOX_LINES - 1
        imul    ecx, [textbox_width]
        mov     [text_pos], ecx

  .done:
        call    window_is_updated

        popa
        ret



draw_channel_text:                      ; edx = pointer to text

        pusha

        mov     ebx, TEXT_X shl 16 + TEXT_Y
        mov     ecx, TEXTBOX_LINES

  .drawloop:
        pusha
        mov     cx, bx
        shl     ecx, 16
        mov     cx, 9                   ; character height
        mov     ebx, TEXT_X shl 16
        mov     bx, word[textbox_width]
        imul    bx, 6                   ; character width
        mov     edx, [colors.work]
        mcall   13                      ; draw rectangle
        popa

        push    ecx
        mov     ecx, [colors.work_text]

  .draw:
        mov     esi, [textbox_width]
        mcall   4                       ; draw text
        add     edx, [textbox_width]
        add     ebx, 10                 ; height distance between lines

        pop     ecx
        loop    .drawloop

        mov     eax, [window_active]
        and     [eax + window.flags], not FLAG_UPDATED  ; clear the 'window is updated' flag

        popa
        ret

