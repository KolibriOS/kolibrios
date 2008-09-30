context_menu_start:

    mov        eax, 40
    mov        ebx, 00100111b
    int        0x40

    ;call       draw_ctx_menu

    mov        ebp, 2		; 2 часто используется.
; // Alver 26.08.2007 // {
;    xor        ecx, ecx
;    mov        edx, [ctx_menu_PID]
;  find_slot:
;    inc        ecx
;    mov        eax, 9
;    mov        ebx, process_info_buffer
;    int        0x40
;    cmp        dword [process_info_buffer + 30], edx
;    jne        find_slot
    mov        ecx, [ctx_menu_PID]
    mcall      18,21
    mov        ecx, eax
; } \\ Alver \\
    mov        eax, 18
    mov        ebx, 3
    ;mov       ecx, ecx         ; :)
    int        0x40

    call       draw_ctx_menu

  ctx_menu_still:
    mov        eax, 10
    int        0x40

    cmp        eax, ebp 	; cmp     eax, 2
    jz	       ctx_menu_key
    cmp        eax, 3
    jz	       ctx_menu_button
    cmp        eax, 6
    jz	       ctx_menu_mouse

    call       draw_ctx_menu

    jmp        ctx_menu_still

  ctx_menu_key:
    mov        eax, ebp 	; mov     eax, 2
    int        0x40

  ctx_menu_button:
    mov        eax, 17
    int        0x40

    cmp        ah, 1
; // Alver 26.08.2007 // {
;    jne        ctx_menu_still
    jne        @f
; } \\ Alver \\
    mov        eax, 18
    mov        ebx, ebp 	; mov     eax, 2
    mov        ecx, [n_slot]
; // Alver 26.08.2007 // {
    jmp        .lllxxx
@@:
    cmp        ah, 2
    jne        ctx_menu_still
    mov        eax, 18
    mov        ebx, 22
    mov        edx, [n_slot]
    xor        ecx, ecx
; \begin{diamond}[20.09.2007]
    test       byte [procinfo_for_detect+70], 2
    setnz      cl
    add        cl, cl
; \end{diamond}[20.09.2007]
  .lllxxx:
; } \\ Alver \\
    int        0x40
    jmp        ctx_menu_exit

  ctx_menu_mouse:
    mov        eax, 37
    mov        ebx, ebp 	; mov     ebx, 2
    int        0x40

    xchg       eax, ecx 	; cmp     eax, 0  Если не одна из кнопок не нажата возвращаемся
				;                 в главный цикл потока
    jecxz      ctx_menu_still

    mov        eax, 37
    xor        ebx, ebx 	; mov     ebx, 1
    inc        ebx
    int        0x40

    cmp        ax, 0		; Тут проверяем произошёл-ли клик за пределами окна контекстного
    jb	       ctx_menu_exit	; меню, если за пределами то закрываем контекстное меню
    cmp        ax, 60           ; 41
    ja	       ctx_menu_exit
    shr        eax, 16
    cmp        ax, 0
    jb	       ctx_menu_exit
    cmp        ax, 133
    ja	       ctx_menu_exit

    jmp        ctx_menu_still

  ctx_menu_exit:

    xor        eax, eax
    dec        eax		; mov        eax, -1
    int        0x40

func  draw_ctx_menu

    mov        eax, 12
    xor        ebx, ebx 	; mov        ebx, 1
    inc        ebx
    int        0x40

    xor        eax, eax 	; mov        eax, 0
    movzx      ebx, [x_coord]
    shl        ebx, 16
    add        ebx, 133
    movzx      ecx, [y_coord]
    sub        ecx, 60         ; 41
    shl        ecx, 16
    add        ecx, 60         ; 41
    mov        edx, [system_colours + 20]    ; sc.work
    mov        esi, [system_colours + 4]     ; sc.grab
    or	       esi, 0x81000000
    mov        edi, [system_colours]	     ; sc.frame
    int        0x40

    mov        eax, 8
    mov        ebx, 0 * 65536 + 133
    mov        ecx, 22 * 65536 + 16
    mov        edx, 0x40000001
    int        0x40

    mov        eax, 8
    mov        ebx, 0 * 65536 + 133
    mov        ecx, 40 * 65536 + 18
    mov        edx, 0x40000002
    int        0x40

    shr        eax, 1	; mov   eax, 4
    mov        ebx, 36 * 65536 + 7
    mov        ecx, [system_colours + 16]    ; sc.grab_text
    or	       ecx, 0x10000000

    mov        edx, ctx_menu_title

    mov        esi, ctx_menu_title_end - ctx_menu_title
    int        0x40

    add        ebx, 1 * 65536
    int        0x40

    mov        ebx, 4 * 65536 + 28
    mov        ecx, 0x80000000
    mov        edx, ctx_menu_text
; // Alver 26.08.2007 // {
    int        0x40
    add        bx, 18
    mov        edx, ctx_menu_text2
; \begin{diamond}[20.09.2007]
    test       byte [procinfo_for_detect+70], 2
    jz         @f
    mov        edx, ctx_menu_text3
@@:
; \end{diamond}[20.09.2007]
    int        0x40
; } \\ Alver \\
    mov        eax, 12
    mov        ebx, ebp 	; mov     ebx, 2
    int        0x40

    ret

endf

x_coord rw	1
y_coord rw	1
n_slot	rd	1
lsz ctx_menu_text,\
  ru, <"X Закрыть    Alt + F4",0>,\
  en, <"X Close      Alt + F4",0>,\
  et, <"X Sulge      Alt + F4",0>      ; Now correct
; // Alver 26.08.2007 // {
lsz ctx_menu_text2,\
  ru, <25," Свернуть           ",0>,\
  en, <25," Minimize           ",0>,\
; } \\ Alver \\
; \begin{diamond}[20.09.2007]
lsz ctx_menu_text3,\
  ru, <24," Восстановить       ",0>,\
  en, <24," Restore            ",0>
; \end{diamond}[20.09.2007]
ctx_menu_PID	rd	1

ctx_menu_title:
	db	'KolibriOS'
ctx_menu_title_end:
