;ˆ‘‚€ˆ… ˜€ˆŠ€,  ‡€„€Œ“ ID ŠŠˆ (AX)
paint_ball:
	pushad

	movzx	bx, byte [eax*2 + lineCoord - 4]
	movzx	cx, byte [eax*2 + lineCoord - 3]

	movzx	eax, byte [eax + lineBall - 2]
	test	eax, eax
	jz	@F

	mov	edx,[tableColor+eax*4]

	imul	bx, bx, 37
	add	bx, 11
	shl	ebx, 16
	mov	bx, 19		; ebx=[x start]*65536+[x size]

	imul	cx, cx, 37
	add	cx, 57
	shl	ecx, 16
	mov	cx, 19		; ecx=[y start]*65536+[y size]

	mcall	13
	@@:
	popad
	ret


;ˆ‘‚Š€ €ŒŠˆ ‚Š“ƒ ‚›€ƒ ˜€ˆŠ€ [current]
choice_ball:
	pushad
	mov	eax, [current]
	movzx	bx, byte [eax*2 + lineCoord - 4]
	movzx	cx, byte [eax*2 + lineCoord - 3]

	imul	bx, bx, 37
	add	bx, 3
	shl	ebx, 16
	mov	bx, 34		; ebx=[x start]*65536+[x size]

	imul	cx, cx, 37
	add	cx, 49
	shl	ecx, 16
	mov	cx, 34		; ecx=[y start]*65536+[y size]

	mov	edx,0x0
	mcall	13
	popad
	ret

;……ˆ‘‚Š€ Ÿ—…‰Šˆ (eax)
redraw_cell:
	pushad
	or	eax, 0x80000000
	mov	edx,eax
	mcall	8

	and	edx, 0x7FFFFFFF
	movzx	bx, byte [edx*2 + lineCoord - 4]
	movzx	cx, byte [edx*2 + lineCoord - 3]

	imul	bx, bx, 37
	add	bx, 3
	shl	ebx, 16
	mov	bx, 34		; ebx=[x start]*65536+[x size]

	imul	cx, cx, 37
	add	cx, 49
	shl	ecx, 16
	mov	cx, 34		; ecx=[y start]*65536+[y size]

	mov	esi, 0x00B8C2D3
	mcall	8

	popad
	ret

;–…„“€ ‚…Šˆ „‹ˆ› ‹ˆˆ‰ ˆ ‘ˆ‚€ˆ… ˜€ˆŠ‚
vanish_balls:

	mov	[countVanishBall], 0
	mov	al, [findUpDown]
	cmp	al, 4
	jb	@F

	add	[countVanishBall], al
	mov	ah, 1
	call	zero_vanish_matrix
	mov	ah, 2
	call	zero_vanish_matrix
@@:
	mov	al, [findRightLeft]
	cmp	al, 4
	jb	@F

	add	[countVanishBall], al
	mov	ah, 3
	call	zero_vanish_matrix
	mov	ah, 4
	call	zero_vanish_matrix
@@:
	mov	al, [findUpRight_DownLeft]
	cmp	al, 4
	jb	@F

	add	[countVanishBall], al
	mov	ah, 7
	call	zero_vanish_matrix
	mov	ah, 8
	call	zero_vanish_matrix
@@:
	mov	al, [findUpLeft_DownRight]
	cmp	al, 4
	jb	@F

	add	[countVanishBall], al
	mov	ah, 5
	call	zero_vanish_matrix
	mov	ah, 6
	call	zero_vanish_matrix
@@:
	ret

;®¡­ã«ï¥â ¬ âà¨æã ¯® á®®â¢ §­ ç¥­¨î ah
zero_vanish_matrix:

	mov	ecx, 81
	mov	edi, lineBall
	xor	al, al

	.cycle_zeros:
	   mov	dl, [edi]
  	   shr	dl, 4
	   cmp	dl, 0x0F
	   je	@F
	   cmp	dl, ah
	   je	@F
	   inc	edi
	   jmp	.temp
	   @@:
	   stosb
	   push	eax
	   mov	eax, edi
	   sub	eax, lineBall - 1
	   call	redraw_cell
	   pop	eax
	   .temp:
	   loop	.cycle_zeros
	ret

;ˆ‘“…Œ ‚›… –‚…’€
paint_new_color:
	movzx	eax, [newColor1]
	mov	edx, [tableColor + eax*4]
	mov	ebx, 123*65536 + 19
	mov	ecx, 15*65536 + 19
	mcall	13
	movzx	eax, [newColor2]
	mov	edx, [tableColor + eax*4]
	mov	ebx, 160*65536 + 19
	mov	ecx, 15*65536 + 19
	mcall	13
	movzx	eax, [newColor3]
	mov	edx, [tableColor + eax*4]
	mov	ebx, 197*65536 + 19
	mov	ecx, 15*65536 + 19
	mcall	13
	ret

;ˆ‘“…Œ —Šˆ
redraw_score:
	mov	ebx, 8*65536 + 100
	mov	ecx, 20*65536 + 20
	mov	edx, 0x0
	mcall	13

	mov	ebx, 0xB0000
	mov	ecx, [score]
	mov	edx, 15*65536 + 27
	mov	esi, 0x10ffffff
	mcall	47
	ret