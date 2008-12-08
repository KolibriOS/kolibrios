;======================================================================
;===============  WINDOW DEFINITIONS AND DRAW  ========================
;======================================================================
macro draw_window {
	mcall	12, 1	;начинаем перерисоку окна

	mov	ebx, 100*65536 + 347	; [x start] *65536 + [x size]
	mov	ecx, 100*65536 + 465	; [y start] *65536 + [y size]
	mov	edx, 0x33D0D6C9		; color of work area RRGGBB,8->color gl
	mov	edi, szTitle		; заголовок
	mcall	0			; нарисовать окно


	mov	ebx, 12*65536 + 7	;РИСУЕМ ОЧКИ
	mov	ecx, 0x90000000
	mov	edx, szScore
	mcall	4
	call	redraw_score

	mov	ebx, 234*65536 + 7	;РИСУЕМ РЕКОРД
	mov	ecx, 0x90000000
	mov	edx, szRecord
	mcall	4

	mov	ebx, 230*65536 + 100
	mov	ecx, 20*65536 + 20
	mov	edx, 0x0
	mcall	13

	mov	ebx, 0xB0000
	mov	ecx, [record]
	mov	edx, 237*65536 + 27
	mov	esi, 0x10ffffff
	mcall	47

	mov	ebx, 115*65536 + 34	;РИСУЕМ МЕСТО ПОД НОВЫЕ ЦВЕТА
	mov	ecx, 7*65536 + 34
	mov	edx, 0x00B8C2D3
   @@:
	mcall	13
	add	ebx, 37*65536
	cmp	ebx, 189*65536 + 34
	jbe	@B

	call	paint_new_color

	mov	ebx, 20*65536 + 400	;РИСУЕМ УПРАВЛЯЮЩИЕ КНОПКИ СНИЗУ
	mov	ecx, 0x90000000
	mov	edx, szNewGame
	mcall	4

	mov	ecx,9			;РИСУЕМ ОСНОВНЫЕ КНОПКИ
	mov	edx, 82

	.drm1:
	   mov	ebx,ecx
	   mov	ecx,9
	   .drm2:
		mov	eax, edx
		call	redraw_cell
		cmp	eax, [current]
		jne	@F
		call	choice_ball
		@@:
		call	paint_ball
		dec	edx
	   loop	.drm2
	   mov	ecx,ebx
	loop 	.drm1
	mcall	12,2
	}