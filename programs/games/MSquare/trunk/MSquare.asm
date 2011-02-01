; 10.06.2010 staper@inbox.ru

;Магический квадрат - это набор таких целых чисел, что их суммы
;в строках, столбцах (реже в диагоналях) равны.

;Пример:

; 16 3  2  13
; 5  10 11 8
; 9  6  7  12
; 4  15 14 1

use32
org	0x0
 db	'MENUET01'
 dd	0x1, START, I_END, (D_END+100) and not 3,  (D_END+100) and not 3, 0, 0

;Цвета:
Bckgrd_clr	equ 0xffffff	;фон
Brdr_line_clr	equ 0xb0b0b0	;линии по границам
Inter_line_clr	equ 0xb0b0b0	;внутренние линии
Square_clr	equ 0xdddddd	;цвет курсора
Fix_nmb_clr	equ 0x83459	;статичное значение
Chg_nmb_clr	equ 0;x008d8d	;переменное значение
Text_clr	equ 0x000000	;текст
Message_clr	equ 0x0000ff	;сообщения

max_dif equ 10
min_dif equ 2
len_kub equ 30	;длина стороны квадратика
beg_x	equ 40	;координата Х левого верхнего угла
beg_y	equ 50	;координата Y левого верхнего угла
sym_x equ 20
sym_y equ 20
win_x equ 400	;ширина окна
win_y equ 400	;высота окна


DEBUG equ 0

macro dbg_dec num
{pushad
newline
debug_print_dec num
popad
}

include '../../../macros.inc'
;include 'debug.inc'
include 'editbox_ex.mac'
include 'lang.inc'


START:
	;mcall	40,7

redraw_all:
	mcall	12,1
	mcall	48,4
	add	eax,100*65536+win_x
	mov	ecx,eax
	mcall	0,100*65536+win_y,,(0x34000000+Bckgrd_clr),,title
	movzx	eax,[Difficult]
	mov	ebx,len_kub
	xor	edx,edx
	mul	ebx
	add	eax,beg_x*65536+beg_x
	xchg	eax,ebx
	mcall	38,,beg_y*65536+beg_y,Brdr_line_clr
	mov	edx,Inter_line_clr
	movzx	esi,[Difficult]
	dec	esi
  @@:	add	ecx,len_kub*65536+len_kub
	mcall
	dec	esi
	jnz	@b
	add	ecx,len_kub*65536+len_kub
	mcall	,,,Brdr_line_clr

	movzx	eax,[Difficult]
	mov	ecx,len_kub
	xor	edx,edx
	mul	ecx
	add	eax,(beg_y+1)*65536+beg_y-1
	xchg	eax,ecx
	mov	ebx,beg_x*65536+beg_x
	mcall	38,,,Brdr_line_clr
	mov	edx,Inter_line_clr
	movzx	esi,[Difficult]
	dec	esi

  @@:	add	ebx,len_kub*65536+len_kub
	mcall
	dec	esi
	jnz	@b
	add	ebx,len_kub*65536+len_kub
	mcall	,,,Brdr_line_clr

	mcall	8,<3,78>,<3,13>,2,0xbbbbbb
	mcall	4,<7,5>,(0x80000000+Text_clr),txt.clear
	mcall	,<105,5>,,txt.dif


	call	show_level

	push	dword Map;esi;  mov     esi,Map
	mcall	12,2
draw_pole:
	if DEBUG
	call	SysMsgBoardNum	;show esp
	endf

	movzx	eax,[Y]
	dec	al
	movzx	ebx,[Difficult]
	mul	bl
	mov	bl,[X]
	add	al,bl
	pop	esi	;       mov     esi,Map
	push	eax	;курсорчик
	movzx	eax,[Difficult]
	mov	ebx,eax
	mul	eax
	sub	eax,ebx
	mov	edi,eax
	pop	eax
	push	eax
;	mov	edi,81-9
	movzx	ebp,[Difficult]
	mov	ebx,beg_x*65536+beg_x+sym_x
	mov	ecx,beg_y*65536+beg_y+sym_y
	call	out_numbers
	pop	eax
	call	out_sum

still:
	mcall	10

	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jnz	still
;button:
	mcall	17
	cmp	ah,1
	jne	@f
	mcall	-1
.clear:
@@:	mov	ecx,max_dif*max_dif
	mov	esi,Map
@@:	mov	byte [esi+ecx-1],0
	loop	@b
	jmp	redraw_all

key:
	mcall	2

.43:	cmp	ah,43		;+
	jne	.45
	cmp	[Difficult],max_dif
	je	still
	inc	[Difficult]
	jmp	redraw_all
	jmp	still
.45:				;-
	cmp	ah,45
	jne	.99
	cmp	[Difficult],min_dif
	je	still
	dec	[Difficult]
	jmp	redraw_all
	jmp	still

.99:	cmp	ah,0x58
	je	@f
	cmp	ah,0x78
	jne	.39
@@:	jmp	still.clear

.39:	cmp	ah,0x39
	ja	.110
	cmp	ah,0x30
	jb	still
	sub	ah,0x30
	mov	cl,ah

	movzx	eax,[Y]
	dec	al
	mov	ebx,max_dif;[Difficult]
	mul	bl
	mov	bl,[X]
	dec	bl
	add	al,bl

	mov	esi,Map
	bt	[flags],9
	jnc	@f
	mov	bl,[esi+eax]
	push	eax
	mov	ax,10
	xor	dl,dl
	mul	bl
	add	cl,al
	pop	eax
	mov	[esi+eax],cl
	btr	[flags],9
	jmp	.onedraw
   @@:	mov	[esi+eax],cl
	bts	[flags],9
	jmp	.onedraw


.110:	cmp	ah,110		;n
	jne	.176
.new_game:
	jmp	redraw_all

.176:	cmp	ah,176 ;курсоры
	jne	.177
	call	draw_one_symbol
	dec	[X]
	cmp	[X],1
	jge	@f
	push	eax
	movzx	eax,[Difficult]
	mov	[X],al
	pop	eax
@@:	btr	[flags],9
	jmp	.onedraw
.177:	cmp	ah,177
	jne	.178
	call	draw_one_symbol
	inc	[Y]
	push	eax
	movzx	eax,[Difficult]
	cmp	[Y],al
	jbe	@f
	mov	[Y],1
@@:	pop	eax
	btr	[flags],9
	jmp	.onedraw
.178:	cmp	ah,178
	jne	.179
	call	draw_one_symbol
	dec	[Y]
	cmp	[Y],1
	jge	@f
	push	eax
	movzx	eax,[Difficult]
	mov	[Y],al
	pop	eax
@@:	btr	[flags],9
	jmp	.onedraw
.179:	cmp	ah,179
	jne	still
	call	draw_one_symbol
	inc	[X]
	push	eax
	movzx	eax,[Difficult]
	cmp	[X],al
	jbe	@f
	mov	[X],1
@@:	btr	[flags],9
	pop	eax
.onedraw:
	bts	[flags],4
	call	draw_one_symbol
	call	out_one_sum
	jmp	still ;.todraw

show_level:
	movzx	ecx,[Difficult]
	mcall	47,0x10000,,<205,5>,(0x50000000+Text_clr),Bckgrd_clr
ret


draw_one_symbol:
	movzx	eax,[X]
	mov	ebx,len_kub*65536+len_kub
	mul	ebx
	xchg	eax,ebx
	add	ebx,(beg_x*65536+beg_y-len_kub*65536+len_kub)
	movzx	eax,[Y]
	mov	ecx,len_kub*65536+len_kub
	mul	ecx
	xchg	eax,ecx
	add	ecx,(beg_y*65536+beg_y+sym_y-len_kub*65536+len_kub)
	movzx	eax,[Y]
	dec	al
	push	ebx
	movzx	ebx,[Difficult]
	mul	bl
	mov	bl,[X]
	add	al,bl
	dec	al
	pop	ebx
	mov	esi,Map
	add	esi,eax
	push	dword 0 ;не курсор
	bt	[flags],4
	jnc	@f
	mov	dword [esp],1 ;курсор
	btr	[flags],4
@@:	mov	edi,0
	mov	ebp,1
	call	out_numbers
	pop	eax
ret


out_numbers:
	push	ebx ecx esi
	shr	ebx,16
	inc	bx
	shl	ebx,16
	add	ebx,len_kub-1
	shr	ecx,16
	inc	cx
	shl	ecx,16
	add	ecx,len_kub-1
	mov	edx,Bckgrd_clr
	push	ebp
	dec	dword [esp+4*5]
	jnz	@f
	mov	edx,Square_clr
@@:	mcall	13
	pop	ebp
	pop	esi

	push	ebx edx
	mov	eax,esi
	sub	eax,Map-1
	xor	edx,edx
	movzx	ebx,[Difficult]
	div	ebx
	push	edx
	xor	edx,edx
	mov	ebx,max_dif
	mul	ebx
	pop	edx
	test	edx,edx
	jnz	@f
	movzx	edx,[Difficult]
	sub	eax,max_dif
@@:	add	eax,edx
	mov	al,[eax+Map-1]
	pop	edx ebx
	test	al,al
 
;	cmp	byte [esi],0
	jnz	.changeable_number
	jmp	.null
.end:
	inc	esi
	dec	ebp
	jnz	out_numbers
	test	edi,edi
	jz	@f
	push	ebx edx
	movzx	eax,[Difficult]
	sub	edi,eax
	mov	ebp,eax
	mov	ebx,len_kub*65536
	xor	edx,edx
	mul	ebx
	push	eax
	movzx	eax,[Difficult]
	mov	ebx,len_kub
	mul	ebx
	pop	ebx
	add	eax,ebx
	pop	edx
	pop	ebx
	sub	ebx,eax
;	add	ebx,-9*24*65536-9*24
	add	ecx,len_kub*65536+len_kub
	jmp	out_numbers
  @@:
ret


.null:
	pop	ecx ebx
	add	ebx,len_kub*65536+len_kub
	jmp	.end
.changeable_number:
	push	esi
	shr	ebx,16
	shr	ecx,16
	mov	dx,bx
	shl	edx,16
	mov	dx,cx
	add	edx,8*65536+4

	push	ebx edx
	mov	eax,esi
	sub	eax,Map-1
	xor	edx,edx
	movzx	ebx,[Difficult]
	div	ebx
	push	edx
	xor	edx,edx
	mov	ebx,max_dif
	mul	ebx
	pop	edx
	test	edx,edx
	jnz	@f
	movzx	edx,[Difficult]
	sub	eax,max_dif
@@:	add	eax,edx
	pop	edx ebx
	
	movzx	ebx,byte [eax+Map-1];[esi]


	push	esi ebp edi edx
	sub	edx,2*65536-3
	push	edx
	xchg	eax,ebx
	mov	ebx,10
	xor	edx,edx
	div	ebx
	mov	ebx,edx
	pop	edx
	
	push	ebx

	test	eax,eax
	jz	.only_first_num

;	bt	[flags],9
;	jc	.only_first_num

	xchg	eax,ebx

	shl	bx,4
	add	ebx,FONT
	mov	ecx,8*65536+16

	mov	edi,Pltr.ch
	cmp	dword [esp+4*9],0
	jne	@f
	mov	edi,Pltr.chk
@@:	mov	esi,1
	mov	ebp,0
	mcall	65

.only_first_num:
	pop	ebx
	shl	bl,4

	add	ebx,FONT
	mov	ecx,8*65536+16
	add	edx,8*65536
	mov	edi,Pltr.ch
	cmp	dword [esp+4*8],0
	jne	@f
	mov	edi,Pltr.chk
@@:	mov	esi,1
	mov	ebp,0
	mcall	65
	pop	edx edi ebp esi
	pop	esi ecx ebx
	add	ebx,len_kub*65536+len_kub
	jmp	.end

out_sum:
	movzx	eax,[Difficult]
	push	eax
	mov	esi,Map
	mov	edi,out_sum_buf
.1:	xor	ebx,ebx
	xor	ecx,ecx
	xor	edx,edx
.2:	mov	dl,[ebx+esi]
	add	ecx,edx
	inc	ebx
	cmp	ebx,[esp]
	jne	.2
	mov	word [edi],cx
	add	edi,2
	add	esi,max_dif
	dec	eax
	jnz	.1

	push	out_sum_buf
	movzx	eax,[Difficult]
	mov	ebx,len_kub*65536
	xor	edx,edx
	mul	ebx
	add	eax,40*65536
	push	eax
	mov	edx,(beg_x-30)*65536+beg_y+10
	mov	ebx,0x30000
	mov	esi,0x50000000+Fix_nmb_clr
	mov	edi,Bckgrd_clr

.3:	mov	ecx,[esp+4]
	mov	cx,[ecx]
	shl	ecx,16
	shr	ecx,16
	mcall	47
	push	edx
	add	edx,[esp+4]
	mcall	
	pop	edx
	add	edx,len_kub
	add	dword [esp+4],2
	dec	dword [esp+8]
	jnz	.3
	pop	eax
	pop	eax
	pop	eax


	movzx	eax,[Difficult]
	push	eax
	mov	esi,Map
	mov	edi,out_sum_buf
	xor	ebx,ebx
.4:	xor	ecx,ecx
	xor	edx,edx
	mov	ebx,[esp]
.5:	mov	dl,[esi]
	add	ecx,edx
	add	esi,max_dif
	dec	ebx
	jnz	.5
	mov	word [edi],cx
	add	edi,2
	push	eax
	mov	eax,max_dif
	xor	edx,edx
	mul	dword [esp+4]
	dec	eax
	sub	esi,eax
	pop	eax
	dec	eax
	jnz	.4

	push	out_sum_buf
	movzx	eax,[Difficult]
	mov	ebx,len_kub
	xor	edx,edx
	mul	ebx
	add	eax,35
	push	eax
	mov	edx,(beg_x+5)*65536+beg_y-20
	mov	ebx,0x30000
	mov	esi,0x50000000+Fix_nmb_clr
	mov	edi,Bckgrd_clr

.6:	mov	ecx,[esp+4]
	mov	ecx,[ecx]
	shl	ecx,16
	shr	ecx,16
	mcall	47
	push	edx
	add	edx,[esp+4]
	mcall	
	pop	edx
	add	edx,len_kub*65536;[esp]
	add	dword [esp+4],2
	dec	dword [esp+8]
	jnz	.6
	pop	eax
	pop	eax
	pop	eax
ret



out_one_sum:
	movzx	ecx,[Difficult]
	mov	esi,Map
	xor	edx,edx

	movzx	eax,[Y]
	dec	al
	mov	ebx,max_dif
	mul	bl
	add	esi,eax
@@:	mov	al,[esi]
	inc	esi
	add	edx,eax
	loop	@b

	mov	ecx,edx
	movzx	eax,[Difficult]
	mov	ebx,len_kub*65536
	xor	edx,edx
	mul	ebx
	add	eax,40*65536
	push	eax
	movzx	eax,[Y]
	dec	eax
	mov	ebx,len_kub
	xor	edx,edx
	mul	ebx
	mov	edx,(beg_x-30)*65536+beg_y+10
	add	edx,eax
	mov	ebx,0x30000
	mov	esi,0x50000000+Fix_nmb_clr
	mov	edi,Bckgrd_clr

	mcall	47
	add	edx,[esp]
	mcall	
	pop	eax




	movzx	ecx,[Difficult]
	mov	esi,Map
	xor	edx,edx

	movzx	eax,[X]
	dec	al
	add	esi,eax
@@:	mov	al,[esi]
	add	esi,max_dif
	add	edx,eax
	loop	@b

	mov	ecx,edx
	movzx	eax,[Difficult]
	mov	ebx,len_kub
	xor	edx,edx
	mul	ebx
	add	eax,35
	push	eax
	movzx	eax,[X]
	dec	eax
	mov	ebx,len_kub*65536
	xor	edx,edx
	mul	ebx
	mov	edx,eax
	add	edx,(beg_x+5)*65536+beg_y-20
	mov	ebx,0x30000
	mov	esi,0x50000000+Fix_nmb_clr
	mov	edi,Bckgrd_clr

	mcall	47
	add	edx,[esp]
	mcall	

	pop	edx

ret




if DEBUG
SysMsgBoardNum: ;warning: destroys eax,ebx,ecx,esi
	mov	ebx,esp
	mov	ecx,8
	mov	esi,(number_to_out+1)
.1:
	mov	eax,ebx
	and	eax,0xF
	add	al,'0'
	cmp	al,(10+'0')
	jb	@f
	add	al,('A'-'0'-10)
@@:
	mov	[esi+ecx],al
	shr	ebx,4
	loop	.1
	dec	esi
	mcall	71,1,number_to_out
ret

number_to_out	db '0x00000000',13,10,0
endf





if lang eq ru
title db 'Магический квадрат',0
txt:
.dif db "Сложность (+/-):",0
.clear db 'Очистить (x)',0
else
title db 'Magical square',0
txt:
.dif db "Difficult (+/-)",0
.clear db 'Clear (x)',0
endf




align 4
;Палитры:
Pltr:
.ch dd Bckgrd_clr,Chg_nmb_clr
.chk dd Square_clr,Chg_nmb_clr
.fx dd Bckgrd_clr,Fix_nmb_clr
.fxk dd Square_clr,Fix_nmb_clr

FONT file "MSquare.fnt"

X db 1
Y db 1

Difficult db 3

I_END:
align 16
Map	rb max_dif*max_dif
out_sum_buf rw max_dif

flags rw 1

D_END:
;бит 0: см. перед draw_pole
;2: в draw_pole и key
;4: in draw_one_symbol
;9: введёна первая цифра числа
