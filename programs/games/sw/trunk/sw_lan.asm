;10.11.2009 staper@inbox.ru

use32
org	0x0
db	'MENUET01'
dd	0x1, START, I_END, (I_END+600) and not 3, (I_END+600) and not 3, 0x0, 0x0

remote_ip db 010,010,010,002
server_port dd 0x9876
client_port dd 0x6789
socknum dd ?

DEBUG equ 1

include 'macros.inc'
include 'lang.inc'
include 'debug.inc'

macro dbg_dec num
{pushad
newline
debug_print_dec num
popad}

OP_SOC equ 0 ; UDP only
CL_SOC equ 1
WR_SOC equ 4

macro free_socket {
mcall 53,1,[socknum]
 if CL_SOC no equ 1
 mcall 53,CL_SOC,[socknum]
 endf
btr [flags],11
btr [flags],12}


START:
	mcall	40,10100111b ;сеть,мышь
	mcall	3
	mov	[rsx1],ax
	shr	eax,16
	mov	[rsx2],ax
	call	redraw_all
	jmp	draw_pole

redraw_all:
	mcall	12,1
	mcall	0,100*65536+432,100*65536+280,0x34FFFFFF,,title
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

	mcall	8,120*65536+70,7*65536+15,3,0xFFFFFF	;auto_fill
	mcall	,230*65536+70,,2,			;new_game
	mov	ecx,234*65536+15
	bt	[flags],11
	jc	@f
	mcall	,2*65536+70,,4, 			;listen
@@:	bt	[flags],12
	jc	@f
	mcall	,74*65536+70,,5,			;connect
@@:	mcall	,146*65536+70,,6,			;disconnect
	mcall	4,135*65536+12,0x80000000,but_new
	mcall	,240*65536+12,,but_auto
	mcall	,15*65536+240,,but_lis_con

	mcall	12,2
ret

draw_pole:
	if DEBUG
	call	SysMsgBoardNum	;show esp
	endf

	mov	esi,table1
	mov	edi,90
	mov	ebp,10
	mov	ebx,1*65536+21
	mov	ecx,27*65536+47
	call	draw_bots

	mov	esi,table2
	mov	edi,90
	mov	ebp,10
	mov	ebx,221*65536+241
	mov	ecx,27*65536+47
	call	draw_bots
	cmp	[loc_kill],10
	jne	@f
	mcall	4,69*65536+12,0x80ff0000,text_win
	bts	[flags],5
  @@:	cmp	[rem_kill],10
	jne	@f
	mcall	4,372*65536+12,0x80ff0000,text_win
	bts	[flags],5
  @@:
	cmp	[number_bots],10
	jne	still
	bts	[flags],0
still:
	mcall	23,10
	test	eax,eax
	jz	listen
;        mcall   10

	cmp	eax,6
	je	mouse
;        cmp     eax,8
;        je      still
	dec	eax
	jnz	@f
	call	redraw_all
	jmp	draw_pole
@@:	dec	eax
	jz	key
	dec	eax
	jz	button
	jmp	still

listen:
	mcall	53,2,[socknum]
	test	eax,eax
	jz	still
	xor	edx,edx
	call	check_header
	jc	still
	mcall	53,2,[socknum]
	test	eax,eax
	jz	still
	mcall	53,3,[socknum]
	push	ebx
	mcall	53,2,[socknum]
	pop	ebx
if DEBUG
debug_print_dec eax
push eax
movzx eax,bl
debug_print_dec eax
pop eax
endf
	cmp	bl,1	;получено сообщение от сервера с рандомным числом
	je	.get_first
	cmp	bl,2	;сервер получил ответ, определяющий порядок хода
	je	.set_first
	cmp	bl,3	;отмена соединения
	je	.close_connect
	cmp	bl,4	;генерация новой игры, очистка карт
	je	.new_game
	cmp	bl,5	;окончание игры (выигрыш оппонента)
	je	.end_game
	cmp	bl,6	;poluchena koordinata ataki
	je	.get_koord
	cmp	bl,7	;v otvet posyla'utsya ismen'ennue elementy v table2, max 12 byte = zagolovok 3 bytes + kol-vo elementov + el-ty (max 4*2 bytes)
	je	.get_elem
.free_cash:
	call	check_header.free
	jmp	still
.get_first:
;	cmp	eax,2
;	jne	.free_cash
	mcall	53,2,[socknum]
@@:	mcall	53,3,[socknum]
	mov	dl,bl
	shl	edx,8
	mcall	53,2,[socknum]
	test	eax,eax
	jnz	@b
	shr	edx,8
	cmp	dx,[who_first]
	jge	@f
	mov	word [message+2],0x0002 ;I'm first
	mcall	53,WR_SOC,[socknum],4,message
	inc	eax
	jz	button.error
	bts	[flags],13	;moi hod
	jmp	still
@@:	mov	word [message+2],0x0102 ;He is first
	mcall	53,WR_SOC,[socknum],4,message
	inc	eax
	jz	button.error
	btr	[flags],13
	jmp	still
.set_first:
;	cmp	eax,1
;	jne	.free_cash
	mcall	53,2,[socknum]
@@:	mcall	53,3,[socknum]
	dec	bl
	jz	@f
	btr	[flags],13
	jmp	still
@@:	bts	[flags],13
	jmp	still
.close_connect:
	free_socket
	jmp	still
.new_game:
	mov	esi,table2
	mov	ecx,100/4
   @@:	mov	dword [esi],0
	add	esi,4
	loop	@b
	jmp	still
.end_game:
	mov	[rem_kill],10
	bts	[flags],13
	call	redraw_all
	jmp	draw_pole
.buf dw 0xffff
.get_koord:
;        cmp     eax,1
;        jne     .free_cash
	mcall	53,2,[socknum]
;       test    eax,eax
;       jz      still
	mcall	53,3,[socknum]
	movzx	eax,bl
	cmp	ax,[.buf]
	je	still
	mov	[.buf],ax
	mov	esi,table1
	mov	edi,table1
	mov	bl, byte [edi+eax]
	add	bl,2
	mov	ecx,table1
	call	if_bot_killed

	movzx	edx, byte [message+3]
	mov	byte [message+3+1+edx*2],al
	mov	byte [message+3+1+edx*2+1],bl
	inc	byte [message+3]

	mov	[table1+eax],bl
	push	ebx
	mov	byte [message+2],7
	mov	edi,5
   .@u: movzx	edx,byte [message+3]
	lea	edx,[edx*2+4]
	mcall	53,WR_SOC,[socknum],,message
	mcall	5,10
	dec	edi
	jnz	.@u
	inc	eax
	jz	button.error
	pop	ebx
	bts	[flags],13	;ostavliaem hod za soboi
	cmp	bl,3
	jb	@f
	btr	[flags],13	;peredaem hod
	cmp	bl,4
	jne	@f
	inc	[rem_kill]
	cmp	[rem_kill],10
	jne	draw_pole
	call	redraw_all
@@:	jmp	draw_pole
.buf2 dw 0xffff
.get_elem:
	mcall	53,2,[socknum]
;       test    eax,eax
;       jz      still
	mcall	53,3,[socknum]
	movzx	edi,bl		;chislo elementov
	test	edi,edi
	jz	.free_cash
@@:	mcall	53,2,[socknum]
	mcall	53,3,[socknum]
	movzx	eax,bl
	push	eax
	mcall	53,2,[socknum]
	mcall	53,3,[socknum]
	pop	eax
	cmp	bl,byte [.buf2+1]	;проверка на повторный пакет
	jne	@f
	cmp	al,byte [.buf2]
	je	.free_cash
@@:	mov	byte [.buf2],al
	mov	byte [.buf2+1],bl
	jmp	.00
@@:	mcall	53,3,[socknum]
	movzx	eax,bl
	push	eax
	mcall	53,2,[socknum]
	mcall	53,3,[socknum]
	pop	eax
.00	mov	[table2+eax],bl
	dec	edi
	jz	@f
	mcall	53,2,[socknum]
	test	eax,eax
	jnz	@b
@@:	call	check_header.free
	cmp	byte [.buf2+1],2
	jbe	@f
	bts	[flags],13
	cmp	byte [.buf2+1],4
	jb	draw_pole
	inc	[loc_kill]
	cmp	[loc_kill],10
	jb	draw_pole
	call	redraw_all
	jmp	draw_pole
@@:	btr	[flags],13
	jmp	draw_pole




check_header:
	mov	esi,2
@@:	mcall	53,3,[socknum]
	mov	dl,bl
	dec	esi
	jz	@f
	shl	edx,8
	mcall	53,2,[socknum]
	test	eax,eax
	jnz	@b
@@:	xchg	dh,dl
	cmp	dx, word [message]
	jne	.free
if DEBUG
newline
debug_print 'XO packet:'
endf
	clc
	ret
.free:
@@:	mcall	53,3,[socknum]
	mcall	53,2,[socknum]
	test	eax,eax
	jnz	@b
if DEBUG
newline
debug_print 'packet cleared'
endf
	stc
ret




key:
	mcall	2
	jmp	still

close_prog:
	free_socket	;-1 ошибка
	mcall	-1

button:
	mcall	17
	dec	ah
	jz	close_prog

	dec	ah
	jnz	 @f
	mov	ecx,(344)/4
	mov	esi,table1
   .@0: mov	dword [esi],0
	add	esi,4
	loop	.@0
	mov	word [listen.buf],0xffff
	mov	word [listen.buf2],0xffff
;        mov     [flags],1100b   ;auto fill
	btr	[flags],0
	btr	[flags],2
	bts	[flags],3
	mov	[number_bots],0
	call	set_bots
	bts	[flags],0
  .a0:	btr	[flags],5
	bt	[flags],12
	jc	.a1
	bt	[flags],11
	jnc	.a2
  .a1:	mov	byte [message+3],4
;        mcall   53,WR_SOC,[sicknum],3,message
  .a2:	call	redraw_all
	jmp	draw_pole
  @@:	dec	ah		;new game
	jnz	 @f
.new_game:
	mov	ecx,(344)/4
	mov	esi,table1
   .@1: mov	dword [esi],0
	add	esi,4
	loop	.@1
	mov	word [listen.buf],0xffff
	mov	word [listen.buf2],0xffff
	mov	[number_bots],0
;        mov     [flags],1100b
	btr	[flags],0
	btr	[flags],2
	btr	[flags],3
	jmp	.a0

  @@:	dec	ah
	jnz	 @f
	bt	[flags],11
	jc	still
	bt	[flags],12
	jc	still
	mcall	53,OP_SOC,[client_port],[server_port],<dword [remote_ip]>    ;listen
if DEBUG
push eax
dbg_dec eax
debug_print ':socket:listen...'
pop eax
endf
	mov	[socknum],eax	;-1 ошибка
	inc	eax
	jz	.error
	bts	[flags],11
	xor	eax,eax
	or	ax,-1
	call	random
	mov	[who_first],ax		;progi soediniaias' sveria'ut eto znachenie, u kogo bol'she, tot nachinaet pervyi
	bts	[flags],14
	call	redraw_all
	jmp	draw_pole

  @@:	dec	ah
	jnz	 @f
	bt	[flags],11
	jc	still
	bt	[flags],12
	jc	still
	mcall	53,OP_SOC,[server_port],[client_port],<dword [remote_ip]>    ;connect
if DEBUG
push eax
dbg_dec eax
debug_print ':socket:connect...'
pop eax
endf
	mov	[socknum],eax	;-1 ошибка
	inc	eax
	jz	.error
	bts	[flags],12
	xor	eax,eax
	or	ax,-1
	call	random
	mov	esi,message+2
	mov	byte [esi],1
	mov	[esi+1],ax
	mcall	53,WR_SOC,[socknum],5,message ;-1 ошибка
	inc	eax
	jz	.error
	bts	[flags],14
	call	redraw_all
	jmp	draw_pole

  @@:	free_socket	;-1 ошибка      ;disconnect
	call	redraw_all
	jmp	draw_pole

.error:
if DEBUG
debug_print 'error on open/sent'
endf
	jmp	still

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
	clc
ret

f_near:
movzx	edx, byte [smeshenie]
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
;       mcall   12,1
	jmp	draw_pole


second_pole:
	bt	[flags],0
	jnc	still
	bt	[flags],13
	jnc	still
	cmp	ax,221
	jbe	still
	cmp	ax,421
	jge	still
			;второе поле
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

;       mov     edi,table1      ;nashe pole
	mov	esi,table2	;opponent
	cmp	byte [esi+eax],2
	jge	still
	add	byte [esi+eax],2
	mov	byte [message+3],al
	mov	byte [message+2],6
	mov	edi,5
@@:	mcall	5,10
	mcall	53,WR_SOC,[socknum],4,message
	dec	edi
	jnz	@b
	jmp	draw_pole



if_bot_killed:
	mov	byte [message+3],0

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

	movzx	edx, byte [message+3]
	mov	byte [message+3+1+edx*2],bl
	mov	bl,[ecx+ebx]
	mov	byte [message+3+1+edx*2+1],bl
	inc	byte [message+3]

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

	push	edx ebx
	movzx	ebx, byte [message+3]
	mov	byte [message+3+1+ebx*2],dl
	mov	dl,[ecx+edx]
	mov	byte [message+3+1+ebx*2+1],dl
	inc	byte [message+3]
	pop	ebx edx


  @@:	mov	dl,[edi+1]
	cmp	dl,al
	je	@f

	inc	byte [ecx+edx]

	push	edx ebx
	movzx	ebx, byte [message+3]
	mov	byte [message+3+1+ebx*2],dl
	mov	dl,[ecx+edx]
	mov	byte [message+3+1+ebx*2+1],dl
	inc	byte [message+3]
	pop	ebx edx

  @@:	mov	dl,[edi+2]

	cmp	bl,4
	jnb	.9
	cmp	dl,al
	je	@f

	inc	byte [ecx+edx]

	push	edx ebx
	movzx	ebx, byte [message+3]
	mov	byte [message+3+1+ebx*2],dl
	mov	dl,[ecx+edx]
	mov	byte [message+3+1+ebx*2+1],dl
	inc	byte [message+3]
	pop	ebx edx

  @@:	mov	dl,[edi+3]
  .9:	cmp	dl,al
	je	.end_i

	inc	byte [ecx+edx]

	push	edx ebx
	movzx	ebx, byte [message+3]
	mov	byte [message+3+1+ebx*2],dl
	mov	dl,[ecx+edx]
	mov	byte [message+3+1+ebx*2+1],dl
	inc	byte [message+3]
	pop	ebx edx

  .end_i:
	pop	ebx
	inc	ebx
;	inc	[rem_kill]
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
but_lis_con db 'Listen     Connect     Disconnect',0
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

who_first dw 0

message db 'XO1234567890'

I_END:
; t1,t3,t5: 0-непомеченная клеточка, 1-занятая, 2-"промах", 3-"горит", 4-"потоплен"
; t2,t4: таблицы расположения элементов, 1 элемент по 4 клетки, 2x3, 3x2, 4x1
align 16
table1	rb 100	;левое поле, local
table3	rb 20
table2	rb 100	;правое, remote
table4	rb 20
rb 100
rem_kill rb 1	;remote
loc_kill rb 1	;local
p_pov	rb 1
smeshenie rb 1
number_bots rb 1
oX rb 1
oY rb 1
tbl_ranen rb 4

flags rw 1
;flags2 rw 1
;бит 0: 0/1 - заполнение таблиц /игра
;1: нажата левая кнопка мыши (вертикаль) /правая (горизонталь)
;2: заполняется левое поле /правое
;3: ручное заполнение /автозаполнение
;4: в .three - потопленный корабль
;5: игра окончена
;11: ogidat' server
;12: server gdet otveta
;13; 1 - мой ход, 0 - оппонента