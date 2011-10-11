use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	mem
	dd	mem
	dd	0
	dd	0

; field size in items
FieldWidth = 10
FieldHeight = 6

; size of one picture in pixels
ImageWidth = 32
ImageHeight = 32

; size of one button in pixels
ButtonWidth = ImageWidth + 6
ButtonHeight = ImageHeight + 6

SpaceWidth = ButtonWidth + 4
SpaceHeight = ButtonHeight + 4

; size of window in pixels
WindowWidth = 434
WindowHeight = 291	; excluding skin height

if FieldWidth*FieldHeight mod 2
error field size must be an even number!
end if

draw_frame:
; in: ebx=[xstart]*65536+[xsize],ecx=[ystart]*65536+[ysize],edx=color
;	ystart is relative to SkinHeight
; out: nothing
; destroys: eax,ecx
	push	13
	pop	eax
	ror	ecx, 16
	add	ecx, [SkinHeight]
	push	ecx
	shl	ecx, 16
	inc	ecx
	int	0x40	; ebx=[xstart]*65536+[xsize], ecx=[ystart]*65536+1
	ror	ecx, 16
	add	cx, [esp+2]
	ror	ecx, 16
	int	0x40	; ebx=[xstart]*65536+[xsize], ecx=([ystart]+[ysize])*65536+1
	pop	ecx
	ror	ecx, 16
	push	ebx
	mov	bx, 1
	int	0x40	; ebx=[xstart]*65536+1, ecx=[ystart]*65536+[ysize]
	ror	ebx, 16
	add	bx, [esp]
	ror	ebx, 16
	inc	ecx
	int	0x40	; ebx=([xstart]+[xsize])*65536+1, ecx=[ystart]*65536+([ysize]+1)
	dec	ecx
	pop	ebx
	ret

draw_button:
; in: ebx=[xstart]*65536+[xsize],ecx=[ystart]*65536+[ysize],edx=button color,esi=id
;	ystart is relative to SkinHeight
; out: nothing
; destroys: eax
	push	edx
	mov	edx, 0x94AECE
	call	draw_frame
	mov	edx, 0xFFFFFF
	push	ebx
	add	ebx, 0xFFFF
	push	ecx
	add	ecx, 0x10000
	mov	cx, 1
	int	0x40	; ebx=([xstart]+1)*65536+([xsize]-1), ecx=([ystart]+1)*65536+1
	pop	ecx
	mov	edx, 0xC0C0C0
	dec	ebx
	push	ecx
	shr	ecx, 16
	add	ecx, [esp]
	dec	ecx
	shl	ecx, 16
	inc	ecx
	int	0x40	; ebx=([xstart]+1)*65536+([xsize]-2), ecx=([ystart]+[ysize]-1)*65536+1
	pop	ecx
	mov	edx, 0xFFFFFF
	push	ecx
	add	ecx, 0xFFFF
	push	ebx
	mov	bx, 1
	int	0x40	; ebx=([xstart]+1)*65536+1, ecx=([ystart]+1)*65536+([ysize]-1)
	pop	ebx
	push	ebx
	shr	ebx, 16
	add	ebx, [esp]
	shl	ebx, 16
	inc	ebx
	mov	edx, 0xC0C0C0
	add	ecx, 0xFFFF
	int	0x40	; ebx=([xstart]+[xsize]-1)*65536+1, ecx=([ystart]+2)*65536+([ysize]-2)
	pop	ebx
	pop	ecx
	pop	ebx
	pop	edx
	push	ebx ecx
	add	ebx, 20000h - 3
	add	ecx, 20000h - 3
	int	0x40
	pop	ecx ebx
	push	edx
	mov	edx, esi
	or	edx, 0x80000000
	mov	al, 8
	int	0x40
	mov	edx, esi
	or	edx, 0x40000000
	int	0x40
	pop	edx
	ret

;digits_ptr	dd	0
;digits_str	rb	11

;images:
;	file	'mblocks.raw'
images:
	file	'mblocksi.raw'
palette:
	file	'mblocksp.raw'

aNewGame_ru	db	'Новая игра(F2)',0
aCount_ru	db	' Счет:',0
aRu		db	'RU',0
aNewGame_en	db	'New  game (F2)',0
aCount_en	db	'Count:',0
aEn		db	'EN',0

CurLanguage	db	0	; 0=russian, 1=english

field_status	rb	60	; status of field elements: 0=closed, 1=temporarily opened, 2=permanently opened
field_items	rb	60	; items in field

count		dd	0

start:
	push	26
	pop	eax
	push	5
	pop	ebx
	int	0x40
	cmp	eax, 4
	setnz	[CurLanguage]
	call	generate
; get skin height
	push	48
	pop	eax
	push	4
	pop	ebx
	int	0x40
	mov	[SkinHeight], eax
; no previous click
	mov	eax, 0xBAD
	mov	[FirstClick], eax
	mov	[SecondClick], eax
; draw window
redraw:
	call	draw_window
; events loop
evloop:
	push	10
	pop	eax
	int	0x40
	dec	eax
	jz	redraw
	dec	eax
	jz	key
	mov	al, 17
	int	0x40
	shr	eax, 8
;	cmp	eax, 6
;	jz	set_lang
	cmp	eax, 5
	jz	new_game
	cmp	eax, 1
	jnz	field_pressed
	push	-1
	pop	eax
	int	0x40
key:
	mov	al, 2
	int	0x40
	cmp	ah, '3'		; F2?
	jnz	evloop
new_game:
	mov	edi, field_status
	mov	ecx, FieldWidth*FieldHeight/2
	xor	eax, eax
	rep	stosd
	mov	[count], eax
	mov	eax, 0xBAD
	mov	[FirstClick], eax
	mov	[SecondClick], eax
	call	generate
	call	draw_field
@@:
	call	draw_aux
	jmp	evloop
;set_lang:
;	xor	[CurLanguage], 1
;	jmp	@b
field_pressed:
	sub	eax, 100
	cmp	[field_status+eax], 0
	jnz	cont
	mov	ebx, [FirstClick]
	mov	ecx, [SecondClick]
	cmp	ebx, 0xBAD
	jz	first_click
	cmp	ecx, 0xBAD
	jz	second_click
	mov	dl, [field_items+ecx]
	cmp	[field_items+ebx], dl
	jnz	dont_match
	mov	[field_status+ebx], 2
	mov	[field_status+ecx], 2
	jmp	@f
dont_match:
	mov	[field_status+ebx], 0
	mov	[field_status+ecx], 0
@@:
	push	eax ecx
	mov	eax, ebx
	call	draw_field_item
	pop	eax
	call	draw_field_item
	mov	[SecondClick], 0xBAD
	pop	eax
	mov	[FirstClick], eax
	jmp	@f
second_click:
	cmp	[FirstClick], eax
	jz	cont
	mov	[SecondClick], eax
@@:
	mov	[field_status+eax], 1
	call	draw_field_item
	inc	[count]
cont:
	call	draw_count
	jmp	evloop
first_click:
	mov	[FirstClick], eax
	jmp	@b

draw_window:
	mov eax, 48
	mov ebx, 3
	mov ecx, color_table
	mov edx, 40
	int	0x40 	; get color table

	mov eax, 12
	mov ebx, 1
	int	0x40	; start redraw
	
	xor	eax, eax
	mov	ebx, 100*65536 + WindowWidth
	mov	ecx, 100*65536 + WindowHeight
	add	ecx, [SkinHeight]
	mov	edx, 0x14C0C0C0
	mov	edi, caption
	int	0x40
	
	call	draw_aux
	call	draw_field
	
	mov eax, 12
	mov ebx, 2
	int	0x40	; end redraw
	
	ret

caption	db	'Memory Blocks L&V Edition',0

generate:
; generate random field
	xor	edi, edi
.loop:
	call	random
	cmp	[field_items+edx], 0
	jnz	.loop
	mov	eax, edi
	shr	eax, 1
	inc	eax
	mov	[field_items+edx], al
	inc	edi
	cmp	edi, FieldWidth*FieldHeight
	jb	.loop
	ret

draw_field_item:
; in: eax=field item
; out: nothing
; destroys: eax,ebx,ecx,edx,esi
	mov	esi, eax
	lea	edx, [eax+0x80000000+100]
	push	8
	pop	eax
	int	0x40	; delete old button
	mov	ebx, [xstart+esi*4]
	shl	ebx, 16
	mov	bl, ButtonWidth
	mov	ecx, [ystart+esi*4]
	add	ecx, [SkinHeight]
	shl	ecx, 16
	mov	cl, ButtonHeight
	lea	edx, [esi+0x40000000+100]
	int	0x40	; define new button
	ror	ecx, 16
	sub	ecx, [SkinHeight]
	ror	ecx, 16
	mov	al, [field_status+esi]
	dec	eax
	js	draw_closed_item
	jz	draw_current_item
draw_opened_item:
	mov	edx, 0x94AECE
	call	draw_frame
	mov	edx, 0xEFEBEF
	add	ebx, 0xFFFF
	add	ecx, 0xFFFF
	mov	al, 13
	int	0x40
	jmp	draw_item_image
draw_current_item:
	push	ebx ecx
	mov	edx, 0x94AECE
	call	draw_frame
	pop	ecx ebx
	add	ebx, 10000h
	add	ecx, 10000h
	push	ebx ecx
	mov	bl, ButtonWidth-2
	mov	cl, ButtonHeight-2
	mov	edx, 0x94DB00
	call	draw_frame
	pop	ecx ebx
	add	ebx, 10000h
	add	ecx, 10000h
	mov	bl, ButtonWidth-4
	mov	cl, ButtonHeight-4
	mov	edx, 0x94DB00
	call	draw_frame
	add	ebx, 10000h
	add	ecx, 10000h
	mov	bl, ButtonWidth-5
	mov	cl, ButtonHeight-5
	mov	edx, 0xEFEBEF
	int	0x40
draw_item_image:
;	push	7
;	pop	eax
;	movzx	ebx, [field_items+esi]
;	dec	ebx
;	imul	ebx, ImageWidth*ImageHeight*3
;	add	ebx, images
;	mov	ecx, ImageWidth*10000h+ImageHeight
;	mov	edx, [xstart+esi*4]
;	shl	edx, 16
;	add	edx, [ystart+esi*4]
;	add	edx, 3*10001h
;	add	edx, [SkinHeight]
;	int	0x40
	push	65
	pop	eax
	movzx	ebx, [field_items+esi]
	dec	ebx
	imul	ebx, ImageWidth*ImageHeight
	add	ebx, images
	mov	ecx, ImageWidth*10000h+ImageHeight
	mov	edx, [xstart+esi*4]
	shl	edx, 16
	add	edx, [ystart+esi*4]
	add	edx, 3*10001h
	add	edx, [SkinHeight]
	push	8
	pop	esi
	mov	edi, palette
	xor	ebp, ebp
	int	0x40
	ret
draw_closed_item:
	mov	edx, 0x94AECE
	call	draw_frame
	push	38
	pop	eax
	shr	ebx, 16
	inc	ebx
	imul	ebx, 0x10001
	push	ebx
	add	ebx, ButtonWidth-2
	shr	ecx, 16
	inc	ecx
	imul	ecx, 0x10001
	mov	edx, 0xFFFFFF
	int	0x40
	pop	ebx
	push	ecx
	add	ecx, ButtonHeight-3
	int	0x40
	pop	ecx
	push	ebx ecx
	add	ebx, ButtonWidth-2
	shr	ecx, 16
	add	ecx, ButtonHeight-2
	imul	ecx, 0x10001
	mov	edx, 0xDEDEDE
	int	0x40
	pop	ecx
	push	ecx
	movzx	ebx, bx
	imul	ebx, 0x10001
	add	ecx, 10000h+ButtonHeight-3
	int	0x40
	pop	ecx ebx
	add	ebx, 10000h
	mov	bx, ButtonWidth-3
	add	ecx, 10000h
	mov	cx, ButtonHeight-3
	mov	al, 13
	mov	edx, 0xBDC7D6
	int	0x40
	ret

draw_field:
	xor	ebx, ebx
	xor	ecx, ecx
	mov	edi, xstart
.loop:
	mov	eax, ebx
	imul	eax, SpaceWidth
	add	eax, 9
	stosd
	mov	eax, ecx
	imul	eax, SpaceHeight
	add	eax, 4
	mov	[edi+ystart-xstart-4], eax
	lea	eax, [edi-xstart-4]
	shr	eax, 2
	push	ebx ecx edi
	call	draw_field_item
	pop	edi ecx ebx
	inc	ecx
	cmp	ecx, FieldHeight
	jb	.loop
	xor	ecx, ecx
	inc	ebx
	cmp	ebx, FieldWidth
	jb	.loop
	ret

draw_aux:
	push	13
	pop	eax
	mov	ebx, 5*65536 + 425
	mov	ecx, [SkinHeight]
	shl	ecx, 16
	add	ecx, 256*65536 + 1
	mov	edx, 0xFFFFFF
	int	0x40
	add	ecx, 1*65536 + 29
	mov	edx, [color_table+5*4]
	int	0x40
	push	8
	pop	eax
	mov	ebx, 14*65536 + 107
	add	ecx, 3*65536 - 7
	push	5
	pop	edx
	mov	esi, [color_table+6*4]
	int	0x40
	push	4
	pop	eax
	mov	ebx, 27*65536 + 268
	add	ebx, [SkinHeight]
	mov	ecx, [color_table+7*4]
	or	ecx, 0x80000000
	mov	edx, aNewGame_ru
	cmp	[CurLanguage], 0
	jz	@f
	mov	edx, aNewGame_en
@@:
	int	0x40
	mov	ebx, 155*65536 + 268
	add	ebx, [SkinHeight]
	mov	ecx, [color_table+8*4]
	or	ecx, 0x80000000
	mov	edx, aCount_ru
	cmp	[CurLanguage], 0
	jz	@f
	mov	edx, aCount_en
@@:
	int	0x40
	call	draw_count
	mov	al, 13
	mov	ecx, [SkinHeight]
	shl	ecx, 16
	mov	ebx, 405*65536 + 2
	add	ecx, 265*65536 + 2
	mov	edx, 0xBDCBDE
	int	0x40
	mov	ebx, 403*65536 + 6
	add	ecx, (270*65536 + 6) - (265*65536 + 2)
	int	0x40
	mov	ebx, 411*65536 + 6
	add	ecx, (275*65536 + 6) - (270*65536 + 6)
	int	0x40
	mov	ebx, 412*65536 + 9
	add	ecx, (262*65536 + 8) - (275*65536 + 6)
	mov	edx, 0xD6D7CE
	int	0x40
	ret

draw_count:
	mov	edi, string_for_number+15
	mov	eax, [count]
	push	10
	pop	ecx
@@:
	xor	edx, edx
	div	ecx
	dec	edi
	add	dl, '0'
	mov	[edi], dl
	test	eax, eax
	jnz	@b
	mov	al, 4
	mov	ebx, [SkinHeight]
	add	ebx, 195*65536 + 268
	mov	ecx, [color_table+8*4]
	or	ecx, 0xC0000000
	mov	edx, edi
	mov	edi, [color_table+5*4]
	int	0x40
	ret

random:
; in: nothing
; out: edx = random value in [0..FieldWidth*FieldHeight-1]
; destroys: eax,ecx,edx
	rdtsc
	xor	edx, eax
	not	edx
	mov	eax, [RandSeed]
	ror	eax, 3
	xor	eax, 0DEADBEEFh
	add	eax, edx
	mov	[RandSeed], eax
	add	eax, edx
	xor	edx, edx
	push	FieldWidth*FieldHeight
	pop	ecx
	div	ecx
	ret

i_end:

SkinHeight	dd	?
color_table	rd	10
RandSeed	dd	?
FirstClick	dd	?
SecondClick	dd	?
string_for_number	rb	16
xstart		rd	60
ystart		rd	60

align 1000h
stk	rb	1000h
mem:
