;http://sources.ru/pascal/gamestxt/go-moku.zip

N equ 19		; Size of the board

use32
	org	0x0
	db	'MENUET01'
	dd	0x1
	dd	START
	dd	I_END
	dd	(I_END+200+13*N*N) and not 3
	dd	(I_END+200+13*N*N) and not 3
	dd	0x0,0x0

include '../../../macros.inc'
include 'lang.inc'

AttackFactor dw 1	; Importance of attack (1..16)

START:

	mcall	40,100111b
	mcall	40,100111b
	mcall	3
	mov	[rsx1],ax
	shr	eax,16
	mov	[rsx2],ax

redraw_all:
	mcall	12,1
	mcall	48,4
	xchg	eax,ecx
	add	ecx,100*65536+(16*N+26)
	mcall	0,100*65536+(16*N+12),,0x34FFFFFF,,title
	mcall	38,2*65536+(16*N),20*65536+20,0x00a0a0a0;000000
	mov	edi,N
  @@:	add	ecx,16*65536+16
	mcall
	dec	edi
	jnz	@b
	push	cx
	mov	ecx,20*65536
	pop	cx
	mcall	,1*65536+1
	mov	edi,N
  @@:	add	ebx,16*65536+16
	mcall
	dec	edi
	jnz	@b

	mcall	8,3*65536+40,3*65536+12,2,0xFFFFFF
	mcall	,50*65536+40,,3,
	mcall	4,7*65536+5,0x80000000,txt_buttons
	mcall	12,2


draw_pole:
;	mcall	48,4
;	xchg	eax,ecx
;	add	ecx,100*65536+(16*N+26)
;	mcall	0,100*65536+(16*N+12),,0x74FFFFFF,,title

	mov	esi,Board
	mov	edi,N*N-N
	mov	ebp,N
	mov	ebx,(1+5)*65536+(21+5-1)
	call	print_board
	bt	[flags],0
	jnc	@f
	mcall	4,100*65536+6,0x800000ff,txt_go
@@:	bt	[flags],3
	jnc	@f
	mcall	4,100*65536+6,0x800000ff,txt_tie
	jmp	still
@@:
	bt	[flags],2
	jnc	still
	ret
still:
	mcall	10

	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jz	button
	sub	al,3
	jz	mouse
	jmp	still

key:
	mcall	2
	btr	[flags],2
	cmp	ah,97
	jne	@f
.auto:	bts	[flags],2
	jmp	mouse.auto
@@:	cmp	ah,110
	je	button.new_game
	jmp	still

button:
	mcall	17
	cmp	ah,1
	jne	@f
	mcall	-1
@@:	cmp	ah,2
	jne	key.auto
.new_game:
	mov	[TotalLines],2 * 2 * (N * (N - 4) + (N - 4) * (N - 4))
	mov	[WinningLine],0x0
	mov	[X],(N + 1)/2
	mov	[Y],(N + 1)/2
	mov	[flags],0
	mov	edi,Board
	mov	ecx,(13*N*N/4)+1
	xor	eax,eax
	cld
@@:	stosd
	loop	@b
	jmp	redraw_all

print_board:
	cmp	byte [esi],0	;яєёЄю
	je	.null
	cmp	byte [esi],1	;X
	je	.one
	cmp	byte [esi],2	;O
	je	.two
	bts	[flags],4
	cmp	byte [esi],3	;╒ т√шуЁры
	je	.one
	jmp	.two		;0 т√шуЁры

.end:
	inc	esi
	dec	ebp
	jnz	print_board
	test	edi,edi
	jz	@f
	sub	edi,N
	mov	ebp,N
	add	ebx,-N*16*65536+16
	jmp	print_board
@@:	ret

.one:
	mov	ecx,0xd04ba010
	bt	[flags],4
	jnc	@f
	mov	ecx,0xd0ff0000
	btr	[flags],4
@@:	push	edi
	mcall	4,,,txt_x,,0xffffff
	pop	edi
.null:
	add	ebx,16*65536;+16
	jmp	.end
.two:
	mov	ecx,0xd000459a
	bt	[flags],4
	jnc	@f
	mov	ecx,0xd0ff0000
	btr	[flags],4
@@:	push	edi
	mcall	4,,,txt_o,,0xffffff
	pop	edi
	jmp	.null


draw_one_symbol:
	movzx	eax,[X]
	mov	ebx,16
	mul	ebx
	shl	eax,16
	add	eax,(1+5)*65536;
	mov	ax,[Y]
	mov	ecx,16
	mul	cx
	add	ax,(21+5-1)
	xchg	eax,ebx

	movzx	eax,[Y]
	push	ebx
	mov	ebx,N
	mul	bx
	mov	bx,[X]
	add	ax,bx
	pop	ebx
	mov	esi,Board
	add	esi,eax
	mov	edi,0
	mov	ebp,1
	call	print_board
ret


mouse:
	bt	[flags],5
	jc	still
	mcall	37,2
	test	eax,eax
	jz	still
	mcall	37,1
	mov	dx,ax
	shr	eax,16
	cmp	dx,20
	jbe	still
	cmp	dx,(16*N+20)
	jge	still
	cmp	ax,1
	jbe	still
	cmp	ax,(16*N)
	jge	still

	bt	[flags],0
	jc	still
	bt	[flags],3
	jc	still
	sub	ax,1
	push	dx
	xor	edx,edx
	mov	cx,16
	div	cx
	pop	dx
	mov	[X],ax
	push	ax
	sub	dx,20
	mov	ax,dx
	xor	dx,dx
	div	cx
	mov	[Y],ax
	xor	dx,dx
	mov	cx,N
	mul	cx
	pop	dx
	add	ax,dx
	cmp	ax,N*N
	jge	still
	mov	esi,Board

	cmp	byte [esi+eax],0
	jne	still

.auto:	bt	[flags],0
	jc	.end
	bt	[flags],3
	jc	.end

	btr	[flags],1	;0 - ход делает игрок
	bt	[flags],2
	jnc	@f
	call	FindMove
@@:	call	MakeMove
	call	draw_one_symbol
	bt	[flags],0
	jc	.end

	bts	[flags],1	;1 - ход делает cpu
	call	FindMove
	call	MakeMove
	call	draw_one_symbol
.end:	bt	[flags],0
	jnc	@f
	call	BlinkRow
	btr	[flags],2
@@:;	mcall	12,1
	bt	[flags],3
	jc	@f
	bt	[flags],2
	jnc	@f
	call	draw_pole
	jmp	.auto
@@:	jmp	draw_pole




winline: dw 1,0, 1,1, 1,-1, 0,1  ;X,Y
BlinkRow:
	movzx	ecx,[WinningLine]
	mov	eax,[winline+(ecx-1)*4]
	push	ax	;Dx
	shr	eax,16
	push	ax	;Dy
	movzx	eax,[Y]
	mov	si,N
	mul	si
	add	ax,[X]
	mov	cl,[Board+eax]
@@:	movzx	eax,[Y]
	add	ax,[esp]
	mov	[Y],ax
	test	eax,eax
	jz	.ret
	cmp	eax,N-1
	jg	.ret
	movzx	ebx,[X]
	add	bx,[esp+2]
	mov	[X],bx
	test	ebx,ebx
	jz	.ret
	cmp	ebx,N-1
	jg	.ret
	mov	si,N
	mul	si
	add	ax,bx
	cmp	byte [Board+eax],cl
	je	@b

.ret:	mov	edi,5
	mov	esi,N
@@:	movzx	eax,[Y]
	sub	ax,[esp]
	mov	[Y],ax
	mul	si
	movzx	ebx,[X]
	sub	bx,[esp+2]
	mov	[X],bx
	add	ax,bx
	cmp	byte [Board+eax],cl
	jne	.1
	add	byte [Board+eax],2
.1:	dec	edi
	jnz	@b
	add	esp,4
ret



Max dw ?

FindMove:
	mov	[Max],0
	mov	[X],((N+1) / 2)
	mov	[Y],((N+1) / 2)
	movzx	eax,[Y]
	mov	ah,N
	mul	ah
	add	ax,[X]
	cmp	byte [Board+eax],0
	jne	@f
	mov	[Max],4
@@:	xor	ecx,ecx
.loop:
	cmp	byte [Board+ecx],0
	jne	.check_loop
		movzx	eax, word [Value+ecx*2]
		bt	[flags],1
		jc	@f
		movzx	eax, word [Value+(N*N+ecx)*2]
		@@:
		mov	ebx,16
		add	bx,[AttackFactor]
		mul	bx
		shr	eax,4 ;div 16
		mov	bx,[Value+2*(N*N+ecx)]
		bt	[flags],1
		jc	@f
		mov	bx,[Value+2*(ecx)]
		@@:
		add	bx,ax
		mov	eax,4
		call	random
		add	bx,ax
		cmp	bx,[Max]
		jbe	.check_loop
			mov	[Max],bx
			xor	edx,edx
			mov	eax,ecx
			mov	ebx,N
			div	ebx
			mov	[X],dx
			mov	[Y],ax
.check_loop:
	inc	ecx
	cmp	ecx,N*N
	jb	.loop
ret


MakeMove:
	xor	eax,eax
	mov	esi,N

.1:	movzx	ecx,[X] ;ecx=X1, eax=K, edx=Y1
	inc	cl
	movzx	edx,[Y]
	inc	dl
	sub	cl,al
	xor	edi,edi
	test	ecx,ecx
	jz	.1_
	cmp	ecx,N-4
	jg	.1_
		dec	cl
		dec	dl
		push	eax edx
		mov	eax,edx
		mul	esi
		add	eax,ecx
		call	.Add
		bt	[flags],0
		jnc	.11
		cmp	[WinningLine],0x0
		jne	.11
		mov	[WinningLine],1
	.11:	mov	eax,[esp];edx
		mul	esi
		add	eax,ecx
		push	eax
		mov	eax,[esp+4];edx
		mul	esi
		add	eax,edi
		add	eax,ecx
		mov	ebx,eax
		pop	eax
		call	.Update
		inc	edi
		cmp	edi,4
		jbe	.11
		pop	edx eax
.1_:	inc	eax
	cmp	eax,4
	jbe	.1

	xor	eax,eax

.2:	movzx	ecx,[X]
	inc	cl
	movzx	edx,[Y]
	inc	dl
	xor	edi,edi
	sub	cl,al
	sub	dl,al
	test	ecx,ecx
	jz	.2_
	cmp	ecx,N-4
	jg	.2_
	test	edx,edx
	jz	.2_
	cmp	edx,N-4
	jg	.2_
		dec	cl
		dec	dl
		push	eax edx
		mov	eax,edx
		mul	esi
		add	eax,ecx
		add	eax,1*N*N
		call	.Add
		bt	[flags],0
		jnc	.21
		cmp	[WinningLine],0x0
		jne	.21
		mov	[WinningLine],2
	.21:	mov	eax,[esp];edx
		mul	esi
		add	eax,ecx
		add	eax,1*N*N
		push	eax
		mov	eax,[esp+4];edx
		add	eax,edi
		mul	esi
		add	eax,edi
		add	eax,ecx
		mov	ebx,eax
		pop	eax
		call	.Update
		inc	edi
		cmp	edi,4
		jbe	.21
		pop	edx eax
.2_:	inc	eax
	cmp	eax,4
	jbe	.2

	xor	eax,eax

.3:	movzx	ecx,[X]
	inc	cl
	movzx	edx,[Y]
	inc	dl
	xor	edi,edi
	add	cl,al
	sub	dl,al
	cmp	ecx,5
	jb	.3_
	cmp	ecx,N
	jg	.3_
	test	edx,edx
	jz	.3_
	cmp	edx,N-4
	jg	.3_
		dec	cl
		dec	dl
		push	eax edx
		mov	eax,edx
		mul	esi
		add	eax,ecx
		add	eax,3*N*N
		call	.Add
		bt	[flags],0
		jnc	.31
		cmp	[WinningLine],0
		jne	.31
		mov	[WinningLine],3
	.31:	mov	eax,[esp];edx
		mul	esi
		add	eax,ecx
		add	eax,3*N*N
		push	eax
		mov	eax,[esp+4];edx
		add	eax,edi
		mul	esi
		add	eax,ecx
		sub	eax,edi
		mov	ebx,eax
		pop	eax
		call	.Update
		inc	edi
		cmp	edi,4
		jbe	.31
		pop	edx eax
.3_:	inc	eax
	cmp	eax,4
	jbe	.3

	xor	eax,eax

.4:	movzx	ecx,[X]
	inc	cl
	movzx	edx,[Y]
	inc	dl
	xor	edi,edi
	sub	dl,al
	test	edx,edx
	jz	.4_
	cmp	edx,N-4
	jg	.4_
		dec	cl
		dec	dl
		push	eax edx
		mov	eax,edx
		mul	esi
		add	eax,ecx
		add	eax,2*N*N
		call	.Add
		bt	[flags],0
		jnc	.41
		cmp	[WinningLine],0
		jne	.41
		mov	[WinningLine],4
	.41:	mov	eax,[esp];edx
		mul	esi
		add	eax,ecx
		add	eax,2*N*N
		push	eax
		mov	eax,[esp+4];edx
		add	eax,edi
		mul	esi
		add	eax,ecx
		mov	ebx,eax
		pop	eax
		call	.Update
		inc	edi
		cmp	edi,4
		jbe	.41
		pop	edx eax
.4_:	inc	eax
	cmp	eax,4
	jbe	.4

	movzx	eax,[Y]
	mul	esi
	add	ax,[X]
	bt	[flags],1
	jc	@f
	mov	byte [Board+eax],1
	jmp	.end
@@:
	mov	byte [Board+eax],2
.end:	cmp	[TotalLines],0
	jne	@f
	bts	[flags],3
@@:
ret

.Add:
	bt	[flags],1
	jnc	.Add_player
	inc	byte [Line+eax]
	cmp	byte [Line+eax],1
	jne	@f
		dec	[TotalLines]
@@:	cmp	byte [Line+eax],5
	jb	@f
	bts	[flags],0	;игра окончена
@@:
ret
.Add_player:
	inc	byte [Line+eax+4*N*N]
	cmp	byte [Line+eax+4*N*N],1
	jne	@f
		dec	[TotalLines]
@@:	cmp	byte [Line+eax+4*N*N],5
	jb	@f
	bts	[flags],0	;игра окончена
@@:
ret

.Update:
;eax первый параметр, ebx второй
	push	edi
	bt	[flags],1
	jnc	.Update_player
	cmp	byte [Line+eax+4*N*N],0
	jne	.else_cpu
		push	eax
		movzx	edi, byte [Line+eax]
		mov	ax, word [edi*2+2+Weight]
		sub	ax, word [edi*2+Weight]
		add	[Value+ebx*2],ax
		pop	eax
	jmp	.Update_end
	.else_cpu:
	cmp	byte [Line+eax],1
	jne	.Update_end
		push	eax
		movzx	edi, byte [Line+eax+4*N*N]
		mov	ax, word [edi*2+2+Weight]
		sub	[Value+ebx*2+N*N*2],ax
		pop	eax
	jmp	.Update_end
.Update_player:
	cmp	byte [Line+eax],0
	jne	.else_player
		push	eax
		movzx	edi, byte [Line+eax+4*N*N]
		mov	ax, word [edi*2+2+Weight]
		mov	di, word [edi*2+Weight]
		sub	ax,di
		add	[Value+ebx*2+2*N*N],ax
		pop	eax
	jmp	.Update_end
	.else_player:
	cmp	byte [Line+eax+4*N*N],1
	jne	.Update_end
		push	eax
		movzx	edi, byte [Line+eax]
		mov	ax, word [edi*2+2+Weight]
		sub	[Value+ebx*2],ax
		pop	eax
.Update_end:
	pop	edi
ret


align 4
rsx1 dw ?;0x4321
rsx2 dw ?;0x1234
random: 	; из ASCL
	push ecx ebx edi edx
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
	pop edx edi ebx ecx
	and eax,0000ffffh
ret



txt_x db 'X',0
txt_o db 'O',0
if lang eq ru
title db 'Гомоку',0
txt_buttons db 'Новая   Авто',0
txt_go db 'Игра окончена',0
txt_tie db 'Нет ходов',0
else
title db 'Gomoku',0
txt_go db 'Game over',0
txt_tie db 'Tie game',0
txt_buttons db 'New     Auto',0
endf


Weight dw 0,0,4,20,100,500,0


WinningLine db 0
TotalLines dw 0

X dw 0
Y dw 0

flags rw 1
;бит 0: игра окончена
;1: 0-ход игрока, 1-цпу
;2: autoplay
;3: ходы исчерпаны
;4: в print_board - выделение красным цветом 5-ти в ряд клеток

I_END:
align 16
Board	rb N*N
Value	rw N*N*2	;первая половина - для компа, вторая - для игрока
Line	rb 4*N*N*2


