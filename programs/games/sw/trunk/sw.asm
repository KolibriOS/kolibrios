;v. 0.4:  20.10.2009 staper@inbox.ru

use32
	org	0x0
	db	'MENUET01'
	dd	0x1
	dd	START
	dd	I_END
	dd	(I_END+600) and not 3
	dd	(I_END+600) and not 3
	dd	0x0,0x0

DEBUG equ 0

use_new_logic equ 1	;фюсрты хЄ яЁютхЁъє ъыхЄъш эр "юъЁєц╕ээюёЄ№", яЁюуЁрььр т√сшЁрхЄ эршсюыхх ётюсюфэє■
use_ext_logic equ 1	;ъ use_new_logic, фюяюыэшЄхы№эр  яЁютхЁър  ўххъ тюъЁєу чрфрээющ ъыхЄъш

include '../../../macros.inc'
include 'lang.inc'

;include 'debug.inc'

macro dbg_dec num
{pushad
newline
debug_print_dec num
popad}


START:

	mcall	40,100111b
	mcall	3
	mov	[rsx1],ax
	shr	eax,16
	mov	[rsx2],ax
	jmp	new_game

redraw_all:
	mcall	12,1
	mcall	0,100*65536+432,100*65536+260,0x34FFFFFF,,title
	mcall	38,1*65536+421,27*65536+27,0x00b0b0b0
	mov	edi,10
  @@:	add	ecx,20*65536+20
	mcall
	dec	edi
	jnz	@b
	mcall	,1*65536+1,27*65536+226,;000000
	mov	edi,21
  @@:	add	ebx,20*65536+20
	mcall
	dec	edi
	jnz	@b
	mcall	13,202*65536+19,27*65536+202,0xFFFFFF
	mcall	4,40*65536+12,0x80000000,text_user
	mcall	,350*65536+12,,text_cpu
	mcall	8,120*65536+70,7*65536+15,2,0xFFFFFF
	mcall	,230*65536+70,,3,
	mcall	4,135*65536+12,0x80000000,but_new
	mcall	,240*65536+12,,but_auto
	mcall	12,2
ret

draw_pole:
;	mcall	0,100*65536+432,100*65536+260,0x74FFFFFF,,title
	if DEBUG
	call	SysMsgBoardNum	;show esp
	mcall	8,200*65536+10,7*65536+15,4,0xFFFFFF
	endf



	mov	esi,table1
	mov	edi,90
	mov	ebp,10
	mov	ebx,1*65536+21
	mov	ecx,27*65536+47
	call	draw_bots

	mov	esi,table5
	mov	edi,90
	mov	ebp,10
	mov	ebx,221*65536+241
	mov	ecx,27*65536+47
	call	draw_bots
	cmp	[u_kill],10
	jne	@f
	mcall	4,69*65536+12,0x80ff0000,text_win
	bts	[flags],5
  @@:	cmp	[c_kill],10
	jne	@f
	mcall	4,372*65536+12,0x80ff0000,text_win
	bts	[flags],5
  @@:	mcall	12,2
	cmp	[number_bots],10
	jne	still
	bts	[flags],0
still:
	mcall	10

	cmp	eax,6
	je	mouse
	dec	eax
	jnz	@f
	call	redraw_all
	jmp	draw_pole
@@:	dec	eax
	jz	key
	dec	eax
	jz	button
	jmp	still

key:
	mcall	2
	jmp	still

button:
	mcall	17
	dec	ah
	jnz	@f
	mcall	-1

  @@:	mov	ecx,(344)/4
	mov	esi,table1
    .3: mov	dword [esi],0
	add	esi,4
	loop	.3

	dec	ah
	jz	@f

	mov	[flags],1001b	;auto fill
	mov	[number_bots],0

	call	set_bots
	mov	[number_bots],0
	bts	[flags],2
	call	set_bots
	call	redraw_all
	btc	[flags2],0
	jc	second_pole.comp
	jmp	draw_pole

  @@:				;new game
	mov	[number_bots],0
new_game:
	mov	[flags],1100b
	call	set_bots
	btr	[flags],0
	btr	[flags],3
	btr	[flags],2
	mov	[number_bots],0
	call	redraw_all
	jmp	draw_pole

draw_bots:
	cmp	byte [esi],1
	je	.one
	cmp	byte [esi],2
	je	.two
	cmp	byte [esi],3
	je	.three
	cmp	byte [esi],4
	jne	.null
	bts	[flags],4
	jmp	.three

.end:
	inc	esi
	dec	ebp
	jnz	draw_bots
	test	edi,edi
	jz	@f
	sub	edi,10
	mov	ebp,10
	add	ebx,-200*65536-200
	add	ecx,20*65536+20
	jmp	draw_bots
  @@:
ret

.one:
	push	ebx ecx
	call	.0
	mov	edx,0x00aa66
	mcall	13
	pop	ecx ebx
.null:
	add	ebx,20*65536+20
	jmp	.end
.two:
	push	ebx ecx
	add	ebx,8*65536-6
	add	ecx,12*65536-12
	mcall	38,,,0x508cec
	add	ecx,-6*65536+6
	mcall
	pop	ecx ebx
	jmp	.null
.three:
	push	ebx ecx
	call	.0
	mov	edx,0x00FF0000
	bt	[flags],4
	jnc	@f
	mov	edx,0x00555555
	btr	[flags],4
  @@:	mcall	13
	pop	ecx ebx
	xor	edx,edx
	mcall	38
	add	ecx,20*65536-20
	mcall
	add	ecx,-20*65536+20
	jmp	.null

.0:
	shr	ebx,16
	inc	bx
	shl	ebx,16
	add	ebx,19
	shr	ecx,16
	inc	cx
	shl	ecx,16
	add	ecx,19
ret

mouse:
	bt	[flags],5
	jc	still
	mcall	37,2
	test	eax,eax
	jz	still
	cmp	al,2
	jg	still
	;1 - левая кнопка, 2 - правая
	dec	al
	jnz	.mouse_1
	btr	[flags],1
	jmp	@f
  .mouse_1:
	bts	[flags],1
  @@:
	mcall	37,1
	mov	dx,ax
	shr	eax,16
	cmp	dx,27
	jbe	still
	cmp	dx,227
	jge	still
	cmp	ax,1
	jbe	still
	cmp	ax,201
	jge	second_pole
			;первое поле
	bt	[flags],0
	jc	still
	sub	ax,1
	push	dx
	xor	edx,edx
	mov	cx,20
	div	cx
	pop	dx
	mov	[oX],al
	push	ax
	sub	dx,27
	mov	ax,dx
	xor	dx,dx
	div	cx
	mov	[oY],al
	xor	dx,dx
	mov	cx,10
	mul	cx
	pop	dx
	add	ax,dx
	cmp	ax,100
	jg	still

set_bot:
	mov	esi,table1
	bt	[flags],2
	jnc	@f
	mov	esi,table2
  @@:	mov	edi,esi
	add	edi,100
	cmp	[number_bots],6
	jnb	.1paluba
	cmp	[number_bots],3
	jnb	.2paluba
	cmp	[number_bots],0
	je	.4paluba
	jmp	.3paluba

.4paluba:
	bt	[flags],1	;четырёхпалубный
	jnc	@f
	cmp	[oX],6
	jg	_still
	jmp	.41
  @@:	cmp	[oY],6
	jg	_still
  .41:	inc	[number_bots]
	mov	[edi],al
	bt	[flags],1
	jc	@f
	add	al,10
	mov	[edi+1],al
	add	al,10
	mov	[edi+2],al
	add	al,10
	mov	[edi+3],al
	jmp	init_table
  @@:	inc	al
	mov	[edi+1],al
	inc	al
	mov	[edi+2],al
	inc	al
	mov	[edi+3],al
	jmp	init_table


.3paluba:				;трёхпалубный
	bt	[flags],1
	jnc	@f
	cmp	[oX],7
	jg	_still
	jmp	.31
  @@:	cmp	[oY],7
	jg	_still
  .31:
	call	find_near

	bt	[flags],1
	jc	@f
	add	al,10
	call	find_near
	add	al,10
	call	find_near
	sub	al,20
	jmp	.32
  @@:	inc	al
	call	find_near
	inc	al
	call	find_near
	sub	al,2
  .32:
	inc	[number_bots]
	mov	ecx,4
	mov	dl,[number_bots]
	sub	dl,2
	jz	@f
	add	ecx,3
  @@:
	mov	[edi+ecx],al
	bt	[flags],1
	jc	@f
	add	al,10
	mov	[edi+ecx+1],al
	add	al,10
	mov	[edi+ecx+2],al
jmp	init_table
  @@:	inc	al
	mov	[edi+ecx+1],al
	inc	al
	mov	[edi+ecx+2],al
jmp	init_table



.2paluba:				;двухпалубный
	bt	[flags],1
	jnc	@f
	cmp	[oX],8
	jg	_still
	jmp	.21
  @@:	cmp	[oY],8
	jg	_still
  .21:
	call	find_near
	bt	[flags],1
	jc	@f
	add	al,10
	call	find_near
	sub	al,10
	jmp	.22
  @@:	inc	al
	call	find_near
	dec	al
  .22:
	inc	[number_bots]
	mov	ecx,8
	mov	dl,[number_bots]
	sub	dl,4
	jz	@f
	add	ecx,2
	dec	dl
	jz	@f
	add	ecx,2
  @@:	add	ecx,2
	mov	[edi+ecx],al
	bt	[flags],1
	jc	@f
	add	al,10
	mov	[edi+ecx+1],al
jmp	init_table
  @@:	inc	al
	mov	[edi+ecx+1],al
jmp	init_table


.1paluba:				;однопалубный
	call	find_near
	inc	[number_bots]
	xor	edx,edx
	movzx	ecx,[number_bots]
	add	ecx,9
	mov	[edi+ecx],al
jmp	init_table

find_near:
	push	ax
	call	f_near	;0

	cmp	al,10
	jb	@f
	sub	al,10	;-10
	call	f_near

@@:	cmp	al,11
	jb	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,0
	je	@f

	sub	al,11	;-11
	call	f_near

@@:	cmp	al,9
	jb	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,9
	je	@f

	sub	al,9	;-9
	call	f_near

@@:	cmp	al,1
	jb	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,0
	je	@f

	dec	al	;-1
	call	f_near

@@:	cmp	al,99
	jg	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,9
	je	@f

	inc	al	;+1
	call	f_near

@@:	cmp	al,90
	jge	@f
	add	al,10	;+10
	call	f_near

@@:	cmp	al,91
	jge	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,0
	je	@f

	add	al,9	;+9
	call	f_near

@@:	cmp	al,89
	jge	@f

	xor	edx,edx
	mov	ecx,10
	div	cx
	mov	ax,[esp]
	cmp	dx,9
	je	@f
	add	al,11	;+11
	call	f_near

@@:	pop	ax
;	xor	bl,bl
;	bt	[flags],6
;	jnc	@f
;	bt	[flags],9
;	jnc	@f
;	inc	[p_pov]
;	cmp	[p_pov],20
;	jb	@f
;	bts	[flags],9
;  @@:
	btr	[flags],6
	clc
ret

f_near:
	bt	[flags],6
	jnc	.2
;	bt	[flags],9
;	jc	@f
;	cmp	byte [esi+eax],2
;	jge	@f
;  @@:
	cmp	byte [esi+eax],4
	je	@f
	mov	ax,[esp+4]
	ret
  @@:;	add	esp,4
;	jmp	second_pole.cpu
	mov	ax,[esp+4]
	mov	cx,ax
	add	esp,6
	btr	[flags],6
	stc
;	mov	bl,1
	ret

 .2:	movzx	edx, byte [smeshenie]
  @@:	cmp	al,[edi+edx]
	je	.end
	dec	dl
	jnz	@b
	cmp	al,[edi]
	je	.end
	mov	ax,[esp+4]
	ret
  .end: add	esp,10
	jmp	_still



init_table:
;       xor     eax,eax
	movzx	ecx, byte [number_bots]
	mov	edx,3
	dec	cl
	jz	.i_t

	mov	al,2
.2:	add	dl,3
	dec	cl
	jz	.i_t
	dec	al
	jnz	.2

	mov	al,3
.3:	add	dl,2
	dec	cl
	jz	.i_t
	dec	al
	jnz	.3

	mov	al,4
.4:	inc	dl
	dec	cl
	jz	.i_t
	dec	al
	jnz	.4

.i_t:	mov	[smeshenie],dl
@@:	movzx	eax, byte [edi+edx]
	mov	byte [esi+eax],1
	dec	dl
	jnz	@b
	movzx	eax, byte [edi]
	mov	byte [esi+eax],1
	bt	[flags],3
	jc	set_bots
;	mcall	12,1
	jmp	draw_pole


second_pole:
	cmp	ax,221
	jbe	still
	cmp	ax,421
	jge	still
			;второе поле
	bt	[flags],0
	jnc	still
	sub	ax,221
	push	dx
	xor	edx,edx
	mov	cx,20
	div	cx
	pop	dx
	push	ax
	sub	dx,27
	mov	ax,dx
	xor	dx,dx
	div	cx
	xor	dx,dx
	mov	cx,10
	mul	cx
	pop	dx
	add	ax,dx
	cmp	ax,100
	jg	still

	mov	edi,table2
	mov	esi,table5
	cmp	byte [esi+eax],2
	jge	still
	mov	bl, byte [edi+eax]
	add	bl,2
	mov	ecx,table5
	call	if_bot_killed
	mov	[esi+eax],bl
	cmp	bl,3
	jge	.m_end
.comp:	mov	esi,table1



 .cpu:
;        jmp     .rndm
	xor	eax,eax
;       mov     edi,4
	btr	[flags],8
	xor	ecx,ecx
	mov	edx,tbl_ranen
	mov	dword [edx],0
    @@: mov	al,[esi+ecx]	;поиск "подраненного корабля"
	cmp	al,3
	je	.0
	cmp	cl,99
	jge	.1
	inc	cl
	jmp	@b
    .0: mov	[edx],cl
if DEBUG
dbg_dec ecx
endf
	inc	edx
	inc	cl
	jmp	@b

    .1:
	dec	edx
	mov	cl,[edx]
	cmp	edx,tbl_ranen
	jb	.rndm
	je	@f
	sub	cl,[edx-1]
	cmp	cl,1
	jne	.2
	add	cl,[edx-1]
	jmp	.12
       .2:
	add	cl,[edx-1]
	jmp	.13

    @@: bts	[flags],8
	mov	eax,4
	call	random
	dec	al
	jz	.11
	dec	al
	jz	.12
	dec	al
	jz	.13
	jmp	.14
    .11:
if DEBUG
dbg_dec 100
endf
	cmp	cl,10
	jge	@f
	bts	[flags],8
	jmp	.12
    @@: sub	cl,10
	cmp	byte [esi+ecx],2
	jge	@f
	mov	eax,ecx
	bts	[flags],6
	call	find_near
;	test	bl,bl
;	jz	.20
	jnc	.20
    @@: bts	[flags],8
	add	cl,10
	bts	[flags],8
    .12:
if DEBUG
dbg_dec 200
endf
	movzx	eax,cl
	push	cx
	mov	ch,10
	div	ch
	pop	cx
	cmp	ah,9
	jb	@f
	.121:
	bt	[flags],8
	jc	.13
	bts	[flags],8
	mov	edx,tbl_ranen
	mov	cl,[edx]
	jmp	.14
    @@: inc	cl
	cmp	byte [esi+ecx],2
	jge	@f
	mov	eax,ecx
	bts	[flags],6
	call	find_near
;	test	bl,bl
;	jz	.20
	jnc	.20
    @@: dec	cl
	jmp	.121

    .13:
if DEBUG
dbg_dec 300
endf
	cmp	cl,90
	jb	@f
	.131:
	bt	[flags],8
	jc	.14
	bts	[flags],8
	mov	edx,tbl_ranen
	mov	cl,[edx]
	jmp	.11
    @@: add	cl,10
	cmp	byte [esi+ecx],2
	jge	@f
	mov	eax,ecx
	bts	[flags],6
	call	find_near
;	test	bl,bl
;	jz	.20
	jnc	.20
    @@: sub	cl,10
	jmp	.131

    .14:
if DEBUG
dbg_dec 400
endf
	movzx	eax,cl
	push	cx
	mov	ch,10
	div	ch
	pop	cx
	cmp	ah,0
	jne	@f
	.141:
	bts	[flags],8
	jmp	.11;cpu;rndm
    @@: dec	cl
	cmp	byte [esi+ecx],2
	jge	@f
	mov	eax,ecx
	bts	[flags],6
	call	find_near
;	test	bl,bl
;	jz	.20
	jnc	.20
    @@: inc	cl
	jmp	.141

.rndm:

	mov	eax,100
	call	random
	cmp	eax,100
	jb	@f
	shr	eax,1
@@:

if use_new_logic
	btc	[flags],10
	jnc	@f
	btc	[flags],9
	jnc	@f
	call	check_freedom
endf

@@:	cmp	byte [esi+eax],2
	jb	@f;    .rndm
	;поиск свободной клеточки перед/после выбранной,
	;иначе random иногда входит в бесконечный цикл
	bt	ax,0
	jnc	 .r_1
    .r_0:
	cmp	al,0
	je	.r_1
	dec	al

	cmp	byte [esi+eax],2
	jb	@f
	jmp	.r_0
    .r_1:
	cmp	al,99
	je	.r_0
	inc	al

	cmp	byte [esi+eax],2
	jb	@f
	jmp	.r_1
@@:
	bts	[flags],6
	call	find_near
;	test	bl,bl
;	jnz	.cpu	
	jc	.cpu;20

.20:
	mov	bl,[esi+eax]
	add	bl,2
	mov	ecx,esi
	mov	edi,esi
	call	if_bot_killed
	mov	[esi+eax],bl
;	cmp	[u_kill],10
;	je	.m_end
	cmp	[c_kill],10
	je	.m_end
	cmp	bl,3
	jge	.cpu

.m_end:;	mcall	12,1
jmp	draw_pole

if_bot_killed:
	push	ebx
	add	edi,100
	mov	ebx,19
  @@:	cmp	al,[edi+ebx]
	je	@f
	dec	ebx
	jnz	@b
	cmp	al,[edi+ebx]
	jne	.end
  @@:
	cmp	ebx,16
	jnb	.end_i

  @@:	cmp	bl,10
	jb	.2
	btc	bx,0
	mov	bl, [edi+ebx]
	cmp	byte [ecx+ebx],3
	jne	.end
	inc	byte [ecx+ebx]
	jmp	.end_i

  .2:	cmp	bl,4
	jb	.3
	add	edi,4
	cmp	bl,7
	jb	.3
	add	edi,3
  .3:	movzx	edx, byte [edi]
	cmp	dl,al
	je	@f
	cmp	byte [ecx+edx],3
	jne	.end
  @@:	mov	dl,[edi+1]
	cmp	dl,al
	je	@f
	cmp	byte [ecx+edx],3
	jne	.end
  @@:	mov	dl,[edi+2]
	cmp	dl,al
	je	@f
	cmp	byte [ecx+edx],3
	jne	.end

  @@:	cmp	bl,4
	jnb	@f
	mov	dl,[edi+3]
	cmp	dl,al
	je	@f
	cmp	byte [ecx+edx],3
	jne	.end


  @@:	mov	dl,[edi]
	cmp	dl,al
	je	@f
	inc	byte [ecx+edx]
  @@:	mov	dl,[edi+1]
	cmp	dl,al
	je	@f
	inc	byte [ecx+edx]
  @@:	mov	dl,[edi+2]

	cmp	bl,4
	jnb	.9;@f

;	cmp	dl,al
;	je	.end_i
;	inc	byte [ecx+edx]
;	jmp	.end_i


  @@:	cmp	dl,al
	je	@f
	inc	byte [ecx+edx]
  @@:	mov	dl,[edi+3]
.9:	cmp	dl,al
	je	.end_i
	inc	byte [ecx+edx]
  .end_i:
	pop	ebx
	inc	ebx
	cmp	ecx,table5
	jne	@f
	inc	[u_kill]
	ret
  @@:	inc	[c_kill]
	ret
  .end: pop	ebx
ret




_still:
	bt	[flags],3
	jnc	still

set_bots:
	cmp	[number_bots],10
	jne	@f
	ret
  @@:	xor	edx,edx
	mov	ecx,10
	cmp	[number_bots],6
	jb	@f
	push	dword 10
	jmp	.1
  @@:	cmp	[number_bots],3
	jb	@f
	push	dword 9
	jmp	.1
  @@:	cmp	[number_bots],2
	jb	@f
	push	dword 8
	jmp	.1
  @@:	push	dword 7
  .1:
	mov	eax,[esp]
	call	random
	mov	[oX],al
	mul	cl
	mov	cl,al
	mov	eax,[esp]
	call	random
	mov	[oY],al
	add	cl,al

	cmp	cl,100
	jge	.1

	add	esp,4
	mov	eax,2
	call	random
	cmp	al,1
	jne	@f
	bts	[flags],1
	mov	al,cl
	jmp	set_bot
  @@:	btr	[flags],1
	mov	al,cl
	jmp	set_bot






;процедура отлавливает "наиболее свободную" из клеток для атаки

if use_new_logic
check_freedom:
;       mov     esi,table1
	push	ax
	push	word 100
	xor	eax,eax
	mov	ebx,0x900

.1:	bts	[flags],6
	call	find_near
	jc	.11
	cmp	byte [esi+eax],2	;0
	jb	@f
.11:	inc	bl

@@:	cmp	al,10			;-10
	jb	@f
	push	ax

	sub	al,10
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.21
	 endf
	cmp	byte [esi+eax],2
	jb	.22
.21:	inc	bl
.22:	pop	ax

@@:	cmp	al,11
	jb	@f
	push	ax

	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax

	cmp	dx,0
	je	.32

	sub	al,11
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.31
	 endf
	cmp	byte [esi+eax],2	;-11
	jb	.32
.31:	inc	bl
.32:	pop	ax

@@:	cmp	al,9
	jb	@f
	push	ax

	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax
	cmp	dx,9
	je	.42

	sub	eax,9
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.41
	 endf
	cmp	byte [esi+eax],2	;-9
	jb	.42
.41:	inc	bl
.42:	pop	ax

@@:	cmp	al,1
	jb	@f
	push	ax

	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax
	cmp	dx,0
	je	.52

	dec	al
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.51
	 endf
	cmp	byte [esi+eax],2	;-1
	jb	.52
.51:	inc	bl
.52:	pop	ax

@@:	cmp	al,99
	jg	@f
	push	ax

	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax
	cmp	dx,9
	je	.62

	inc	al
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.61
	 endf
	cmp	byte [esi+eax],2	;+1
	jb	.62
.61:	inc	bl
.62:	pop	ax

@@:	cmp	al,90
	jge	@f
	push	ax
	add	al,10
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.71
	 endf
	cmp	byte [esi+eax],2	;+10
	jb	.72
.71:	inc	bl
.72:	pop	ax

@@:	cmp	al,91
	jge	@f
	push	ax
	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax
	cmp	dx,0
	je	.82

	add	al,9
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.81
	 endf
	cmp	byte [esi+eax],2	;+9
	jb	.82
.81:	inc	bl
.82:	pop	ax

@@:	cmp	al,89
	jge	@f
	push	ax

	xor	edx,edx
	mov	ecx,10
	push	ax
	div	cx
	pop	ax

	cmp	dx,9
	je	.92

	add	al,11
	 if use_ext_logic
		bts	[flags],6
		call	find_near
		jc	.91
	 endf
	cmp	byte [esi+eax],2	;+11
	jb	.92
.91:	inc	bl
.92:	pop	ax

@@:	bt	[flags],7
	jc	.ret
	cmp	byte [esi+eax],1
	jg	@f
	cmp	bl,bh
	jg	@f

;	shl	ebx,8
	bts	[flags],6
	call	find_near
;	xchg	cl,bl
;	shr	ebx,8
;	test	cl,cl
;	jnz	@f
	jc	@f

	shl	bx,8
	pop	dx	;dh - ближайшая слева свободнейшая клетка, dl - справа
	pop	cx	;cl - рандомно выбранная клетка
;       cmp     al,dl
;       jg      .5
;       cmp     al,cl
;       jb      .4
;       mov     dl,al
;       jmp     .5
;.4:    cmp     al,dh
;       jb      .5
;       mov     dh,al
	cmp	al,dh
	jb	.5
	cmp	al,cl
	jg	.4
	mov	dh,al
	jmp	.5
.4:	cmp	al,dl
	jg	.5
	mov	dl,al
.5:	push	cx dx
@@:	xor	bl,bl
	inc	al
	cmp	al,100
	jb	.1
	pop	dx ax

if DEBUG
dbg_dec eax
dbg_dec edx
endf
	cmp	dl,100
	jb	@f
	mov	al,dh
	ret
@@:	mov	al,dh
	push	dx
	bts	[flags],7
	call	.1
	shl	bx,8
	mov	al,[esp]
	call	.1
	pop	dx
	btr	[flags],7
	mov	al,dl
	test	bl,bh
	jbe	.ret
	mov	al,dh
.ret:
ret

endf







align 4
rsx1 dw ?;0x4321
rsx2 dw ?;0x1234
random: 	; из ASCL
	push ecx edx esi
	mov cx,ax
	mov ax,[rsx1]
	mov bx,[rsx2]
	mov si,ax
	mov di,bx
	mov dl,ah
	mov ah,al
	mov al,bh
	mov bh,bl
	xor bl,bl
	rcr dl,1
	rcr ax,1
	rcr bx,1
	add bx,di
	adc ax,si
	add bx,0x62e9
	adc ax,0x3619
	mov [rsx1],bx
	mov [rsx2],ax
	xor dx,dx
	cmp ax,0
	je nodiv
	cmp cx,0
	je nodiv
	div cx
nodiv:
	mov ax,dx
	pop esi edx ecx
	and eax,0000ffffh
ret


if DEBUG
SysMsgBoardNum: ;warning: destroys eax,ebx,ecx,esi
	mov	ebx,esp
	mov	ecx,8
	mov	esi,(number_to_out+1)
.1:
	mov	eax,ebx
	and	eax,0xF
	cmp	al,10
	sbb	al,69h
	das
	mov	[esi+ecx],al
	shr	ebx,4
	loop	.1
	dec	esi
	mcall	71,1,number_to_out
ret

number_to_out	db '0x00000000',13,10,0
endf




text_cpu db 'CPU',0
text_user db 'USER',0
if lang eq ru
title db 'Морской бой 0.4',0
but_auto db 'Расставить',0
but_new db 'Новая',0
text_win db 'Выиграл!',0
else
title db 'Sea war 0.4',0
but_auto db 'Auto fill',0
but_new db 'New',0
text_win db 'Won!',0
endf

I_END:
; t1,t3,t5: 0-непомеченная клеточка, 1-занятая, 2-"промах", 3-"горит", 4-"потоплен"
; t2,t4: таблицы расположения элементов, 1 элемент по 4 клетки, 2x3, 3x2, 4x1
align 16
table1	rb 100	;левое поле
table3	rb 20
table2	rb 100	;правое
table4	rb 20
table5	rb 100
u_kill	rb 1
c_kill	rb 1
p_pov	rb 1
smeshenie rb 1
number_bots rb 1
oX rb 1
oY rb 1
tbl_ranen rb 4

flags rw 1
flags2 rw 1
;бит 0: 0/1 - заполнение таблиц /игра
;1: нажата левая кнопка мыши (вертикаль) /правая (горизонталь)
;2: заполняется левое поле /правое
;3: ручное заполнение /автозаполнение
;4: в .three - потопленный корабль
;5: игра окончена
;6: в second_pole - вызов find_near
;7: в check_freedom
;8: зарезервировано
;9: зарезервировано
;10: зарезервировано