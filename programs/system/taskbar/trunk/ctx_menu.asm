;------------------------------------------------------------------------------ 
align 4
context_menu_start:
        mcall   40, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE  ;00100111b
        mov     ecx, [ctx_menu_PID]
        mcall   18,21
        mov     ecx, eax
        mcall   18,3
        call    draw_ctx_menu
;------------------------------------------------------------------------------ 
align 4
ctx_menu_still:
        mcall   10
        cmp     eax, EV_KEY
        jz      ctx_menu_key

        cmp     eax, EV_BUTTON
        jz      ctx_menu_button

        cmp     eax, EV_MOUSE
        jz      ctx_menu_mouse

        call    draw_ctx_menu
        jmp     ctx_menu_still
;------------------------------------------------------------------------------ 
align 4
ctx_menu_mouse:
        mcall   37,2
        xchg    eax,ecx ; Если не одна из кнопок не нажата возвращаемся
                        ; в главный цикл потока
        jecxz   ctx_menu_still

        mcall   37,1

        cmp     ax, 0           ; Тут проверяем произошёл-ли клик за пределами окна контекстного
        jb      ctx_menu_exit   ; меню, если за пределами то закрываем контекстное меню

        cmp     ax, 60           ; 41
        ja      ctx_menu_exit

        shr     eax, 16
        cmp     ax, 0
        jb      ctx_menu_exit

        cmp     ax, 133
        ja      ctx_menu_exit

        jmp     ctx_menu_still
;------------------------------------------------------------------------------ 
align 4
ctx_menu_key:
        mcall   2
;--------------------------------------
align 4
ctx_menu_button:
        mcall   17
        cmp     ah, 1
        jne     @f

;       mov     eax, 18
;       mov     ebx, 2
;       mov     ecx, [n_slot]
;       jmp     .lllxxx
        mcall   18,3,[n_slot]
        mcall   72,1,3,1
        mcall   68,1
        jmp     ctx_menu_exit
;--------------------------------------
align 4
@@:
        cmp     ah, 2
        jne     ctx_menu_still
        mov     eax, 18
        mov     ebx, 22
        mov     edx, [n_slot]
        xor     ecx, ecx

        test    [procinfo_for_detect.wnd_state],byte 2
        setnz   cl
        add     cl, cl
;--------------------------------------
align 4
.lllxxx:
        mcall
        jmp     ctx_menu_exit
;--------------------------------------
align 4
ctx_menu_exit:
        or      eax,-1
        mcall
;------------------------------------------------------------------------------ 
align 4
;func  draw_ctx_menu
draw_ctx_menu:
        mcall   12, 1

        xor     eax, eax
        movzx   ebx, [x_coord]
        shl     ebx, 16
        add     ebx, 133
        movzx   ecx, [y_coord]
        sub     ecx, 60         ; 41
        shl     ecx, 16
        add     ecx, 60         ; 41
        mov     esi, [system_colours.grab]     ; sc.grab
        or      esi, 0x81000000
        mcall   ,,,[system_colours.work],,[system_colours.work_dark]

        mcall   8,<0,133>,<22,17>,0x40000001

        inc     edx
        mcall   ,,<40,17>
        
        mov     ecx, [system_colours.grab_text]    ; sc.grab_text
        or      ecx, 0x10000000
        mcall   4,<32,4>,,ctx_menu_title,ctx_menu_title_end - ctx_menu_title

        add     ebx, 1 * 65536
        mcall

        mov     ecx, [system_colours.work_text]    ; sc.work_text
        or      ecx, 0x80000000
        mcall   ,<4,28>,,ctx_menu_text

        mov     edx, ctx_menu_text2

        test    byte [procinfo_for_detect.wnd_state], 2
        jz      @f
        mov     edx, ctx_menu_text3
;--------------------------------------
align 4
@@:
        add     bx, 18
        mcall

        mcall   12,2
        ret
;endf
;------------------------------------------------------------------------------ 
align 4
x_coord rw      1
y_coord rw      1
n_slot  rd      1
ctx_menu_PID    rd      1
;------------------------------------------------------------------------------ 
lsz ctx_menu_text,\
  ru, <"X Закрыть    Alt + F4",0>,\
  en, <"X Close      Alt + F4",0>,\
  et, <"X Sulge      Alt + F4",0>,\
;------------------------------------------------------------------------------ 
lsz ctx_menu_text2,\
  ru, <25," Свернуть           ",0>,\
  en, <25," Minimize           ",0>,\
  et, <25," Minimeeri          ",0>,\
;------------------------------------------------------------------------------ 
lsz ctx_menu_text3,\
  ru, <24," Восстановить       ",0>,\
  en, <24," Restore            ",0>,\
  et, <24," Taasta             ",0>
;------------------------------------------------------------------------------ 
ctx_menu_title:
        db 'KolibriOS'
ctx_menu_title_end:
;------------------------------------------------------------------------------ 