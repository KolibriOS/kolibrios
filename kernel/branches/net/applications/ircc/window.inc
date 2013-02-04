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
        cmp     al, 0x20
        jbe     .addzero
        stosb
        dec     ecx
        jnz     .loop
  .addzero:
        xor     al, al
        stosb

        call    draw_windownames        ; redraw it

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