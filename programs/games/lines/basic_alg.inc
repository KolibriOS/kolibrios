;ПРОЦЕДУРА ПРОВЕРКИ СУЩЕСТВОВАНИЯ ПУТИ ДО КНОПКИ [dest] из EAX
test_path:
	pushad
	mov	cx, [lineCoord + eax*2 - 4]
	mov	edx, eax
	cmp	eax, [current]
	je	.recurs_call

	cmp	eax, [dest]
	je	.end_yes

	cmp	byte [eax + lineBall - 2], 0
	jne	.end_no

	or	byte [eax + lineBall - 2], 0x8	;4-й бит - 1 (cheked)

	.recurs_call:
	;тупой вызов для соседних клеток
	test	cl, cl
	jz	@F
	sub	eax, 9
	call	test_path
	jc	.end_yes

	@@:
	test	cl, 8
	jnz	@F
	mov	eax, edx
	add	eax, 9
	call	test_path
	jc	.end_yes

	@@:
	test	ch, ch
	jz	@F
	mov	eax, edx
	dec	eax
	call	test_path
	jc	.end_yes

	@@:
	test	ch, 8
	jnz	.end_no
	mov	eax, edx
	inc	eax
	call	test_path
	jc	.end_yes

	.end_no:
	clc
	popad
	ret
	.end_yes:
	stc
	popad
	ret

;ПРОЦЕДУРА ПОИСКА СБИТЫХ ЛИНИЙ ([dest] - ID кнопки для поиска)
find_line:
	mov	dword [findRightLeft], 0	;обнуляем начальные значения

	mov	eax, [dest]
	add	eax, lineBall - 2
	mov	dh, [eax]
	mov	[baseAddr], eax
	or	byte [eax], 0xF0

	mov	ecx, 8		;в ecx - счетчик
	.cycle_find:
	   lea	eax, [ecx*4 + ecx]
	   shl	eax, 1				;eax = ecx*10
	   mov	edi, [eax + findTable - 4]	;edi - куды записывать рез-ты
	   mov	ebp, [eax + findTable - 8]	;ebp - прибавление адреса
	   mov	bx, [eax + findTable - 10]	;bx - прибавление координат
	   mov	dl, cl
	   shl	dl, 4
	   mov	esi, [baseAddr]
	   mov	eax, [dest]
	   mov	ax, [eax*2 + lineCoord - 4]
	   .small_cycle_find:
		add	al, bh
		cmp	al, 8		;в [0..8] ?
		ja	@F
		add	ah, bl
		cmp	ah, 8
		ja	@F

		add	esi, ebp
		cmp	byte [esi], dh
		jne	@F

		or	byte [esi], dl
		inc	byte [edi]
		jmp	.small_cycle_find
	   @@:
	   loop	.cycle_find
	ret

findRightLeft		db ?
findUpRight_DownLeft	db ?
findUpDown		db ?
findUpLeft_DownRight	db ?

findTable:
   Up:
	db	 -1, 0
	dd	-1, findUpDown
   Down:
	db	 +1, 0
	dd	+1, findUpDown
   Left:
	db	0, -1
	dd	-9, findRightLeft
   Right:
	db	0, +1
	dd	+9, findRightLeft
   UpLeft:
	db	-1, -1
	dd	-10, findUpLeft_DownRight
   DownRight:
	db	+1, +1
	dd	+10, findUpLeft_DownRight
   UpRight:
	db	-1, +1
	dd	+8, findUpRight_DownLeft
   DownLeft:
	db	+1, -1
	dd	-8, findUpRight_DownLeft