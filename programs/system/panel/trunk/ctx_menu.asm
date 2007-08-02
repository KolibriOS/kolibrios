  context_menu_start:

    mov        eax, 40
    mov        ebx, 00100111b
    int        0x40

    ;call       draw_ctx_menu

    mov        ebp, 2		; 2 часто используется.

    xor        ecx, ecx
    mov        edx, [ctx_menu_PID]
  find_slot:
    inc        ecx

    mov        eax, 9
    mov        ebx, process_info_buffer
    int        0x40

    cmp        dword [process_info_buffer + 30], edx
    jne        find_slot

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
    jne        ctx_menu_still
    mov        eax, 18
    mov        ebx, ebp 	; mov     eax, 2
    mov        ecx, [n_slot]

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
    cmp        ax, 41
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
    sub        ecx, 41
    shl        ecx, 16
    add        ecx, 41
    mov        edx, [system_colours + 20]    ; sc.work
    mov        esi, [system_colours + 4]     ; sc.grab
    or	       esi, 0x81000000
    mov        edi, [system_colours]	     ; sc.frame
    int        0x40

    mov        eax, 8
    mov        ebx, 0 * 65536 + 133
    mov        ecx, 22 * 65536 + 18
    mov        edx, 0x40000001
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
    xor        ecx, ecx 	; mov        ecx, 0x00000000
    mov        edx, ctx_menu_text
    mov        esi, ctx_menu_text_end - ctx_menu_text
    int        0x40

    mov        eax, 12
    mov        ebx, ebp 	; mov     ebx, 2
    int        0x40

    ret

endf

x_coord rw	1
y_coord rw	1
n_slot	rd	1
lsz ctx_menu_text,\
  ru, "X ‡ Єалвм    Alt + F4",\
  en, "X Close      Alt + F4",\
  et, "X Fine       Alt + F4"	   ; May be its not correct

ctx_menu_text_end:
ctx_menu_PID	rd	1

ctx_menu_title:
	db	'KolibriOS'
ctx_menu_title_end: