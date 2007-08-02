; Detecting mouse right-clicks.

detect_start:

	mov	eax, 40
	mov	ebx, 00100000b
	int	0x40

   detect_still:
	;mov     eax, 10        ; Работает не совсем корректно, почему хз.
	mov	eax, 23
	mov	ebx, 4
	int	0x40
	;cmp     eax, 6
	;jne     detect_still

	mov	eax, 37
	mov	ebx, 2		; Опрашиваем кнопки мыши
	int	0x40

	test	eax, ebx	; test    eax, 00000010b Интересует только правая кнопка
	jz	detect_still	; Нет - ? Возвращаемся в главный цикл потока

   mouse_btn_up:
	mov	eax, 37
	mov	ebx, 2
	int	0x40

	test	eax, ebx
	jnz	mouse_btn_up

	mov	eax, 37
	xor	ebx, ebx	; mov     ebx, 0
	int	0x40

	mov	ecx, [panel_y_pos]
	shr	ecx, 16
	inc	ecx

	cmp	ax, cx
	jb	detect_still
	add	ecx, 15
	cmp	ax, cx
	ja	detect_still

	shr	eax, 16
	xor	edx, edx	; mov     edx, 1
	inc	edx

   detect_button:
	mov	ebx, edx
	imul	ebx, 6 * 10
	add	ebx, 4

	cmp	eax, ebx
	jb	detect_still

	add	ebx, 60 - 1
	cmp	eax, ebx
	ja	@f

	shl	edx, 2
	mov	ecx, [app_list + edx - 4]
	cmp	ecx, -1
	jz	detect_still

	mov	[x_coord], ax

	mov	eax, 37
	xor	ebx, ebx	; mov     ebx, 0
	int	0x40

	mov	[y_coord], ax

	mov	[n_slot], ecx

	mov	eax, 51
	mov	ebx, 1
	mov	ecx, context_menu_start
	mov	edx, ctx_menu_stack
	int	0x40

	mov	[ctx_menu_PID], eax

	jmp	detect_still

   @@:
	cmp	edx, [max_applications]
	jae	detect_still
	inc	edx
	jmp	detect_button





