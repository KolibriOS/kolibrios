;------------------------------------------------------------------------------
; Display Test for KolibriOS
;------------------------------------------------------------------------------
; version:	0.41
; last update:  17/03/2012
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      some optimisations and code refactoring
;------------------------------------------------------------------------------
; compiler:        FASM 1.50
; name:            Display Test
; version:         0.4
; original author: barsuk

; <--- include all MeOS stuff --->
include "lang.inc"
include "../../../macros.inc"

; <--- start of MenuetOS application --->
MEOS_APP_START

;include "..\..\..\debug.inc"


; <--- start of code --->
CODE
	mcall	37,4,cursor,2
	or      eax, eax
	jz      exit
	mov     [cursorID], eax
;------------------------------------------------------------------------------
redraw:
	call	draw_window		  ; at first create and draw the window
;------------------------------------------------------------------------------
wait_event:				  ; main cycle
	xor	ebx, ebx
	mcall	10

	cmp	eax, 1			  ;   if event == 1
	je	 redraw 		  ;     jump to redraw handler
	cmp	eax, 2			  ;   else if event == 2
	je	 key				;       jump to key handler
	cmp	eax, 3			  ;   else if event == 3
	je	 button 		  ;     jump to button handler

	jmp	wait_event		   ;   else return to the start of main cycle
;------------------------------------------------------------------------------
key:						; key event handler
	mcall	2			  ;   get key code

	cmp	ah, 27
	jz	exit

	cmp	ah, 0x20
	jz	next_test

	cmp	ah, 179 		; ->
	jz	next_test

	cmp	ah, 176 		; <-
	jz	prev_test

	cmp	ah, 'i'
	jz	toggle_info

	cmp	ah, 'I' 		; а вдруг у чела КАПСЛОК ))
	jz	toggle_info

	cmp	ah, 'c'
	jz	toggle_cursor

	cmp	ah, 'C'
	jz	toggle_cursor

	cmp	ah, 'd'
	jz	redraw

	cmp	ah, 'D'
	jz	redraw

	jmp	wait_event
;------------------------------------------------------------------------------
next_test:
	cmp	dword [test_done], 1
	jz	wait_event

	inc	dword [test_num]
	call	draw_window
	jmp	wait_event
;------------------------------------------------------------------------------
prev_test:
	cmp	dword [test_num], ebx
	jz	wait_event

	dec	dword [test_num]
	mov	dword [test_done], ebx
	call	draw_window
	jmp	wait_event
;------------------------------------------------------------------------------
toggle_info:
	xor	dword [show_info], 1
	call	draw_window
	jmp	wait_event
;------------------------------------------------------------------------------
toggle_cursor:
	mov	eax, cursorVisible
	cmp	dword [eax], 0
	jz	.no_cursor

	mov	dword [eax], 0
	mov	ecx, [cursorID]
	jmp	.set
;--------------------------------------
.no_cursor:
	mov	dword [eax], 1
	xor	ecx, ecx
;--------------------------------------
.set:
	mcall	37,5
	mcall	18,15			; чтобы обновился
	jmp	wait_event
;------------------------------------------------------------------------------
button: 				 ; button event handler
	mcall	17		 ;   get button identifier
	cmp	ah, 1
	jne	wait_event		   ;   return if button id != 1
;--------------------------------------
exit:
	or	 eax, -1			 ;   exit application
	mcall
;------------------------------------------------------------------------------
draw_window:
	mcall	12,1

	; курсор
	;mov     eax, 37
	;mov     ebx, 5
	;mov     ecx, cursorID
	;int     0x40

	mcall	14 		; screen size

	mov	ebx, eax
	shr	ebx, 16
	mov	ecx, eax
	and	ecx, 0xffff
	mov	[screenx], ebx
	mov	[screeny], ecx

	inc	ebx
	inc	ecx
	xor	eax, eax			  ; create and draw the window
	mov	edx, 0x01000000
	mov	esi, edx
	mcall
	; стереть границы окна
	xor	edx, edx
	mcall	13 		; грубо так

	dec	ebx
	dec	ecx

	mov	eax, [test_num]
	mov	eax, [test_proc + eax*4]
	or	eax, eax
	jz	end_of_test
	call	eax
	jmp	exit_draw
;--------------------------------------
end_of_test:
	mcall	4,<8,8>,0xffffff,test_finish,test_finish.size
	mov	dword [test_done], 1
	jmp	no_info
;--------------------------------------
exit_draw:
	cmp	dword [show_info], 1
	jnz	no_info
; снова размеры экрана
	mov	ebx, [screenx]
	mov	ecx, [screeny]
; сгенерить строчку с разрешением экрана. НЕ нужно, потому что ппц
	; прямоугольник 200х40 с инфой
	mov	edx, 200
	sub	ebx, edx
	shl	ebx, 15
	mov	bx, dx
	mov	edx, 40
	sub	ecx, edx
	shl	ecx, 15
	mov	cx, dx
	mcall	13,,,0xffffff

	xor	edx, edx
	add	ebx, 0x0000fffe 	; очень удобно :))))
	add	ecx, 0x0000fffe
	mcall
; текст
	shr	ecx, 16
	mov	bx, cx
	add	ebx, 0x00010001
	mov	ecx, 0x80ffffff
	mov	edx, [test_num]
	mov	edx, [test_info + edx*4]
	mcall	4

	add	ebx, 8
	mcall	,,,press_space

	add	ebx, 8
	mcall	,,,press_i

	add	ebx, 8
	mcall	,,,press_c
;--------------------------------------
no_info:
	mcall	12,2
	ret
;------------------------------------------------------------------------------
; <---- procedures for various tests of display ----->
; in: ebx = screen_width, ecx = screen_height
;------------------------------------------------------------------------------
lsz i_image_size, ru, "Image Size and Placement"
db	0
;------------------------------------------------------------------------------
t_image_size:
	mov	esi, ebx
	mov	edi, ecx
; 6 отрезков
	xor	ecx, ecx
	mcall	38,,,0xffffff

	mov	ecx, edi
	shl	ecx, 16
	xor	ebx, ebx
	mcall

	mov	ebx, esi
	shl	ebx, 16
	add	ecx, edi
	mcall
	
	sub	ecx, edi
	add	ebx, esi
	mcall
; рамка готова
	mov	ebx, esi
	shl	ebx, 16
	mov	ecx, edi
	shl	ecx, 15
	mov	cx, di
	shr	cx, 1
	mcall

	shr	ebx, 1
	mov	bx, si
	shr	bx, 1
	mov	ecx, edi
	shl	ecx, 16
	mcall
	ret
;------------------------------------------------------------------------------
lsz i_grid, ru, "Grid"
db	0
;------------------------------------------------------------------------------
t_grid:
;       сетка размером в 64 пиксела
	mov	eax, 38
	inc	ebx
	inc	ecx
	mov	esi, ebx
	mov	edi, ecx
	mov	edx, 0xffffff
	mov	ebp, 0x00400040
;       горизонтальные линии
	shl	ebx, 16
	xor	ecx, ecx
;--------------------------------------
grid_next_y:
	mcall

	add	ecx, ebp
	cmp	cx, di
	jnae	grid_next_y
	sub	ecx, 0x00010001
	mcall
;       вертикальные линии
	mov	ecx, edi
	shl	ecx, 16
	xor	ebx, ebx
;--------------------------------------
grid_next_x:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jnae	grid_next_x
	sub	ebx, 0x00010001
	mcall
	ret
;------------------------------------------------------------------------------
lsz i_horstr, ru, "Horizontal Straightness"
db	0
;------------------------------------------------------------------------------
t_horstr:
	mov	eax, 38
	mov	edi, ecx
	mov	edx, 0xffffff
	mov	esi, ecx
	inc	esi
	shr	esi, 3
	mov	ebp, esi
	shl	ebp, 16
	mov	bp, si
;       горизонтальные линии
	shl	ebx, 16
	mov	ecx, ebp
	shr	ecx, 1
	mov	cx, bp
	shr	cx, 1
;--------------------------------------
hor_next_y:
	mcall
	add	ecx, ebp
	cmp	cx, di
	jnae	hor_next_y
	ret
;------------------------------------------------------------------------------
lsz i_vertstr, ru, "Vertical Straightness",0
db	0
;------------------------------------------------------------------------------
t_vertstr:
	mov	eax, 38
	mov	edx, 0xffffff
	mov	esi, ebx
	shl	ecx, 16
	mov	edi, esi
	shr	edi, 3
	mov	ebp, edi
	shl	ebp, 16
	mov	bp, di
	mov	ebx, ebp
	shr	ebx, 1
	mov	bx, bp
	shr	bx, 1
;--------------------------------------
vert_next_x:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jnae	vert_next_x
	ret
;------------------------------------------------------------------------------
lsz i_distort, ru, "Distortion",0
db	0
;------------------------------------------------------------------------------
t_distort:
	mov	edx, 0xffffff
	mov	esi, ebx
	mov	edi, ecx
	mov	ebp, 3
	xor	ebx, ebx
;--------------------------------------
dist_next:
	push	ebp
	mov	ebp, ebx
	shl	ebx, 16
	or	ebx, ebp

	mov	ecx, edi
	shl	ecx, 16
	or	ecx, ebp
	mcall	38

	mov	ebx, esi
	shl	ebx, 16
	mov	bx, si
	mcall

	mov	bx, bp
	mov	ecx, ebp
	shl	ecx, 16
	or	ecx, ebp
	mcall

	mov	ecx, edi
	shl	ecx, 16
	mov	cx, di
	mcall

	mov	eax, 30
	sub	esi, eax
	sub	edi, eax
	mov	ebx, ebp
	add	ebx, eax
	pop	ebp
	dec	ebp
	jnz	dist_next
	ret
;------------------------------------------------------------------------------
lsz i_horres, ru, "Horizontal Resolution",0
db	0
;------------------------------------------------------------------------------
t_horres:
	mov	eax, 38
	mov	edx, 0xffffff
	mov	edi, ecx
	shl	ecx, 16
	mov	esi, ebx
	xor	ebx, ebx
	mov	edi, 0x003A003A
	mov	ebp, 0x00030003
;--------------------------------------
horres_next:
	add	ebx, edi
	mcall

	add	ebx, ebp
	mcall

	add	ebx, ebp
	mcall

	add	ebx, ebp
	mcall

	add	ebx, ebp
	mcall

	cmp	bx, si
	jna	horres_next
	ret
;------------------------------------------------------------------------------
lsz i_vertres, ru, "Vertical Resolution",0
db	0
;------------------------------------------------------------------------------
t_vertres:
	mov	eax, 38
	mov	edx, 0xffffff
;       mov     esi, ebx
	shl	ebx, 16
	mov	edi, ecx
	xor	ecx, ecx
	mov	ebp, 0x00030003
	mov	esi, 0x002A002A
;--------------------------------------
vertres_next:
	add	ecx, esi
	mcall

	add	ecx, ebp
	mcall

	add	ecx, ebp
	mcall

	add	ecx, ebp
	mcall

	add	ecx, ebp
	mcall

	add	ecx, 0x00540054
	cmp	cx, di
	jna	vertres_next
	ret
;------------------------------------------------------------------------------
lsz i_moire, ru, "Moire Patterns",0
db	0
;------------------------------------------------------------------------------
t_moire:
	mov	eax, 38
	mov	edx, 0xffffff
	mov	edi, ecx
	shl	ecx, 16
	mov	esi, ebx
	xor	ebx, ebx
	mov	ebp, 0x00030003
;--------------------------------------
moire_next:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jna	moire_next
	ret
;------------------------------------------------------------------------------
lsz i_revsharp, ru, "Reverse Video Sharpness",0
db	0
;------------------------------------------------------------------------------
t_revsharp:
	mov	esi, ebx
	mov	edi, ecx
	shr	ecx, 1
	mcall	13,,,0xffffff
; а теперь - инверсные линии
	mov	eax, 38
	mov	ecx, edi
	mov	edx, 0x01000000
	xor	ebx, ebx
	mov	ebp, 0x00010001
	mov	edi, 0x003F003F
;--------------------------------------
revsharp_next:
	add	ebx, edi
	mcall

	add	ebx, ebp
	mcall

	add	ebx, ebp
	mcall

	add	ebx, edi
	sub	ebx, ebp
	mcall

	cmp	bx, si
	jna	revsharp_next
	ret
;------------------------------------------------------------------------------
lsz i_flicker, ru, "Flicker Severity",0
db	0
;------------------------------------------------------------------------------
t_flicker:
	mcall	13,,,0xffffff
	ret
;------------------------------------------------------------------------------
lsz i_glare, ru, "Glare Severity",0
db	0
;------------------------------------------------------------------------------
t_glare:		; оптимизировать нечего
	ret
;------------------------------------------------------------------------------
lsz i_interlace, ru, "Interlacing Detection",0
db	0
;------------------------------------------------------------------------------
t_interlace:
	mov	edi, ecx
	mov	eax, 38
	mov	edx, 0xffffff
	xor	ecx, ecx
	mov	ebp, 0x00020002
;--------------------------------------
interlace_next:
	add	ecx, ebp
	mcall
	cmp	cx, di
	jna	interlace_next
	ret
;------------------------------------------------------------------------------
lsz i_scrreg, ru, "Screen Regulation",0
db	0
;------------------------------------------------------------------------------
t_scrreg:
	add	ebx, 0x0018FFCD ; +25 к началу и -50 от длины
	shr	ecx, 1
	add	ecx, 0x0013FFEC ; +19 к началу и -19 от длины
	mcall	13,,,0xffffff
	ret
;------------------------------------------------------------------------------
lsz i_pricol, ru, "Primary Color Purity"
db	0
;------------------------------------------------------------------------------
t_pricol:
	mov	esi, ebx
	mov	edi, ecx

	shr	ebx, 4	; /16
	mov	ebp, ebx
	shl	ebx, 16
	mov	bx, bp
	shl	ebp, 16
	lea	ebp, [ebp + ebp * 4]		; ebp *= 5

	mov	ecx, 0x00280000
	mov	cx, di
	sub	cx, 80
	;shr     cx, 1

	shl	bx, 2
	mcall	13,,,0xff0000

	add	ebx, ebp
	shr	edx, 8
	mcall

	add	ebx, ebp
	shr	edx, 8
	mcall
	ret
;------------------------------------------------------------------------------
lsz i_colint, ru, "Color Intensity Gradient"
db	0
;------------------------------------------------------------------------------
t_colint:

	mov	esi, ebx
	mov	edi, ecx

;        mov     eax, ecx
;        shr     ecx, 2          ; end y coord
;        and     ecx, 0xffffff80         ; это not 7F
;        shr     eax, 7                  ; / 128
;        mov     ebp, eax
;        mov     edx, eax
;        lea     eax, [eax + eax * 2]    ; eax *= 5
;        shl     ebp, 4
;        add     eax, ebp

;        shl     eax, 16
;        add     ecx, eax
;        mov     edx, ebp
;        shl     ebp, 16
;        mov     bp, dx          ; йа больной

	; я не понял, что там делалось, и решил написать снова о_О

	; надо здесь сгенерить ecx (начальный сдвиг) и ebp (шаг по у)

	mov	eax, edi
	lea	eax, [eax + 2 * eax]
	shr	eax, 5			; eax = 3/32 высоты
	mov	ebp, eax
	shl	ebp, 16
	mov	bp, ax			; ebp = ax в обоих словах

	mov	ebx, eax		; сохраним это значение

	mov	eax, edi
	inc	eax
	shr	eax, 4		; 3/16 высоты - начальное значение
				; всего полосы займут 3/4 высоты, итого по 3/32 высоты на полосу (для ровного счета)
	lea	eax, [eax + eax * 2]
	mov	ecx, eax
	shl	ecx, 16
	shr	ebx, 2
	lea	ebx, [ebx + ebx * 2]	; ebx = 3/4 ebx, т.е. 3/4 высоты полосы
	add	eax, ebx
	mov	cx, ax

	xor	edx, edx
	mov	eax, 0xffff
	div	esi
	mov	edi, eax	; edi = 64K/width

	mov	dword [color_index], 0
	jmp	colint_next
;------------------------------------------------------------------------------
color_table	dd	0x00ff0000, 0x0000ff00, 0x00ffff00, \
			0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
color_index	dd	0
;------------------------------------------------------------------------------
colint_next:
	xor	edx, edx
	xor	ebx, ebx
	mov	eax, 38
;--------------------------------------
colint_next_line:
	push	edx
	push	eax
	movzx	eax, dh
	shl	eax, 16
	mov	dl, dh
	or	edx, eax
	mov	eax, [color_index]
	mov	eax, [color_table + eax * 4]
	and	edx, eax
	pop	eax
	mcall
	pop	edx
	add	ebx, 0x00010001
	add	edx, edi
	cmp	bx, si
	jna	colint_next_line

	add	ecx, ebp
	inc	dword [color_index]
	cmp	dword [color_index], 7
	jb	colint_next
	ret
;------------------------------------------------------------------------------
lsz i_colalign, ru, "Color Alignment Grid"
db	0
;------------------------------------------------------------------------------
t_colalign:
; красная сетка
	inc	ebx		; так нужно
	inc	ecx
	mov	esi, ebx
	mov	edi, ecx
	mov	edx, 0xff0000
;       горизонтальные линии
	shl	ebx, 16
	xor	ecx, ecx
	push	edi
	shr	edi, 3
	mov	ebp, edi
	shl	ebp, 16
	mov	bp, di
	pop	edi
	mov	[yshift], ebp
	mov	eax, 38
;--------------------------------------
cgrid_next_y:
	mcall
	add	ecx, ebp
	cmp	cx, di
	jnae	cgrid_next_y
	; последняя линия:
	sub	ecx, 0x00010001
	mcall

;       вертикальные линии
	mov	ecx, edi
	shl	ecx, 16
	xor	ebx, ebx
	push	esi
	shr	esi, 3
	mov	ebp, esi
	shl	ebp, 16
	mov	bp, si
	mov	[xshift], ebp
	pop	esi
	mov	eax, 38
;--------------------------------------
cgrid_next_x:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jnae	cgrid_next_x
	; последняя линия
	sub	ebx, 0x00010001
	mcall
	jmp	cgrid_green
;------------------------------------------------------------------------------
	xshift	dd	0
	yshift	dd	0
	shift	dd	0
;------------------------------------------------------------------------------
cgrid_green:
; зеленые линии: горизонтальные
	mov	edx, esi
	shr	edx, 5
	lea	eax, [edx + edx * 2]
	shl	edx, 16
	or	edx, eax
	mov	[shift], edx
	mov	eax, 38
	mov	edx, 0x00ff00
	xor	ecx, ecx
	mov	ebp, [xshift]
;--------------------------------------
ggrid_next_yy:
	mov	ebx, [shift]
;--------------------------------------
ggrid_next_yx:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jnae	ggrid_next_yx
	sub	ebx, 0x00010001
	mcall

	add	ecx, [yshift]
	cmp	cx, di
	jnae	ggrid_next_yy
	; last row of lines
	mov	ebx, [shift]
	dec	ecx
;--------------------------------------
ggrid_last_yx:
	mcall
	add	ebx, ebp
	cmp	bx, si
	jnae	ggrid_last_yx

; и вертикальные
	mov	edx, edi
	shr	edx, 5
	lea	eax, [edx + edx * 2]
	shl	edx, 16
	or	edx, eax
	mov	[shift], edx

	mov	eax, 38
	mov	edx, 0x00ff00
	mov	ecx, [shift]
	mov	ebp, [xshift]
;--------------------------------------
ggrid_next_xy:
	xor	ebx, ebx
;--------------------------------------
ggrid_next_xx:
	mcall

	add	ebx, ebp
	cmp	bx, si
	jnae	ggrid_next_xx
	sub	ebx, 0x00010001
	mcall

	add	ecx, [yshift]
	cmp	cx, di
	jnae	ggrid_next_xy
	xor	ebx, ebx
	dec	ecx
;--------------------------------------
ggrid_last_xy:
	;int     0x40
	;add     ebx, ebp
	;cmp     bx, si
	;jnae    ggrid_last_xy
	ret
;------------------------------------------------------------------------------
lsz i_colsyn, ru, "Color Synchronization"
db	0
;------------------------------------------------------------------------------
t_colsyn:
	inc	ebx
	inc	ecx
	mov	esi, ebx
	mov	edi, ecx

	shr	ebx, 5
	mov	eax, ebx
	lea	ebx, [ebx + ebx * 4]
	shl	ebx, 1			; 10/32
	mov	ebp, ebx
	shl	eax, 16
	or	ebx, eax
	shl	ebp, 16

	mov	edi, 0x0000ffff
	add	ecx, 0x003FFF7F
	mov	edx, edi
	mcall	13

	mov	edx, 0x00ff0000
	add	ebx, ebp
	mcall

	mov	edx, edi
	add	ebx, ebp
	mcall
	ret
;------------------------------------------------------------------------------
; <--- initialised data --->
DATA
	screenx 	dd	0
	screeny 	dd	0

	test_num	dd	0
	test_done	dd	0
	show_info	dd	1
	test_proc	dd	t_image_size, t_grid, t_horstr, t_vertstr,\
		t_distort, t_horres, t_vertres, t_moire, t_revsharp, \
		t_flicker, t_glare, t_interlace, t_scrreg, t_pricol, \
		t_colint, t_colalign, t_colsyn, 0
	test_info	dd	i_image_size, i_grid, i_horstr, i_vertstr, \
		i_distort, i_horres, i_vertres, i_moire, i_revsharp, \
		i_flicker, i_glare, i_interlace, i_scrreg, i_pricol, \
		i_colint, i_colalign, i_colsyn, 0

	lsz press_space, ru, "Нажмите пробел для продолжения,"
	db	0
	lsz press_i, ru, "I для переключения сведений,"
	db	0
	lsz press_c, ru, "и C для переключения курсора"
	db	0
	lsz header, ru, "Тест монитора"
	lsz test_finish, ru, "Конец теста. Нажмите ESC."

	cursor  dd 32*32 dup(0x00000000)	; все равно сожмется

	cursorVisible	dd	1
	cursorID	dd	0
;------------------------------------------------------------------------------
; <--- uninitialised data --->
UDATA
;------------------------------------------------------------------------------
MEOS_APP_END
; <--- end of MenuetOS application --->
;------------------------------------------------------------------------------
