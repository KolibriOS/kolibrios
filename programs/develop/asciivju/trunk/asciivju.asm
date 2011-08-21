use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0


BUTTON_SIDE		equ	16		; button are squares
BUTTON_SPACE		equ	19		; space between cols and rows


start:
	call	get_skin_height

still:
	mov	eax, 10
	int	0x40

	dec	eax
	jnz	@f
	call	redraw
    @@:	dec	eax
	jnz	@f
	call	key
    @@:	dec	eax
	jnz	@f
	call	button
    @@:	jmp	still


get_skin_height:

	mov	eax, 48
	mov	ebx, 4
	int	0x40

	mov	[skin_height], eax
	ret


redraw:
	mov	eax, 12
	mov	ebx, 1
	int	0x40

	mov	eax, 0
	mov	ebx, 300*0x10000+317
	mov	ecx, 300*0x10000+184
	add	ecx, [skin_height]
	mov	edx, 0x33F0F0F0			; skinned, resizeable
	mov	esi, 0x80808080			; title color
	mov	edi, window_title
	int	0x40

	call	draw_table
	call	draw_codes
	call	draw_page_switcher

	mov	eax, 12
	mov	ebx, 2
	int	0x40
	ret


key:
	mov	eax, 2
	int	0x40
	ret

button:
	mov	eax, 17
	int	0x40
	cmp	eax, 1
	jnz	@f
	ret
    @@:	shr	eax, 8
	cmp	eax, 1
	jz	quit
	cmp	ax, 0xFFAA
	jnz	.change_focus
	xor	bx, bx
	mov	bl, [symbol_start]
	add	bx, 2
	mov	cx, bx
	add	cx, 128
    @@:	mov	eax, 8
	mov	edx, 0x80000000
	add	dx, bx
	int	0x40
	cmp	bx, cx
	jz	@f
	inc	bx
	jmp	@b

    @@:	add	[symbol_start], 128		; change page
	add	[symbol_focused], 128
	call	draw_table
	call	draw_codes
	mov	[redraw_flag], 1		; for page_switcher this means visual blink (color changing)
	call	draw_page_switcher
	mov	eax, 5
	mov	ebx, 10
	int	0x40
	mov	[redraw_flag], 0
	call	draw_page_switcher
	ret

  .change_focus:
	mov	bl, [symbol_focused]
	mov	[symbol_unfocused], bl
	shl	eax, 8
	sub	ah, 2
	mov	[symbol_focused], ah
	mov	[redraw_flag], 1		; for table this means redraw only focused and unfocused buttons
	call	draw_table
	call	draw_codes
	mov	[redraw_flag], 0
	ret


draw_table:

	mov	al, [symbol_start]
	mov	[symbol_current], al

  .next_button:
	push	[button_x]
	pop	[button_cur_x]
	push	[button_y]
	pop	[button_cur_y]

	mov	al, [symbol_current]
	mov	[button_cur_id], al

	mov	[is_active], 0

	mov	al, [symbol_focused]
	cmp	[symbol_current], al
	jnz	@f
	mov	byte[is_active], 1
    @@:	cmp	[redraw_flag], 1
	jnz	.draw				; if redraw_flag is zero, we should redraw the whole table
	mov	al, [symbol_focused]
	cmp	[symbol_current], al
	jz	.draw
	mov	al, [symbol_unfocused]
	cmp	[symbol_current], al
	jz	.draw
	jmp	.skip				; skip button if it isn't (un)focused

  .draw:
	call	draw_button
  .skip:
	mov	al, [symbol_start]
	add	al, 127				; end of current page
	cmp	[symbol_current], al		; the last on page?
	jb	@f
	mov	[button_x], 2
	mov	[button_y], 2
	ret
    @@:	inc	[symbol_current]
	add	[button_x], BUTTON_SPACE
	cmp	[button_x], 306			; the last in row?
	jae	@f
	jmp	.next_button
    @@:	add	[button_y], BUTTON_SPACE	; next row
	mov	[button_x], 2
	jmp	.next_button


	ret


draw_button:
	mov	eax, 8
	mov	ebx, [button_cur_x]
	shl	ebx, 16
	add	ebx, BUTTON_SIDE
	mov	ecx, [button_cur_y]
	shl	ecx, 16
	add	ecx, BUTTON_SIDE
	mov	edx, 0x80000000
	add	dl, [button_cur_id]
	add	edx, 2
	mov	eax, 8
	int	0x40

	or	edx, 0x20000000
	add	edx, 0x80000000
	mov	esi, 0x00F0F0F0			; button color
	int	0x40

	cmp	byte[is_active], 1
	jz	@f
  .symbol:
	mov	eax, 4
	mov	ebx, [button_cur_x]
	add	ebx, 6
	shl	ebx, 16
	add	ebx, [button_cur_y]
	add	ebx, 5
	xor	ecx, ecx
	mov	edx, symbol_current
	mov	esi, 1
	int	0x40	
	ret
    @@:						; draw a blue square (selection), 8 lines
	mov	eax, 38
	mov	ebx, [button_cur_x]
	shl	ebx, 16
	add	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE-1
	mov	ecx, [button_cur_y]
	shl	ecx, 16
	add	ecx, [button_cur_y]
	mov	edx, 0x000080FF			; square color
	int	0x40

	mov	ebx, [button_cur_x]
	shl	ebx, 16
	add	ebx, [button_cur_x]
	mov	ecx, [button_cur_y]
	shl	ecx, 16
	add	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE-1
	int	0x40

	mov	ebx, [button_cur_x]
	inc	ebx
	shl	ebx, 16
	add	ebx, [button_cur_x]
	inc	ebx
	add	ebx, BUTTON_SIDE-1
	mov	ecx, [button_cur_y]
	inc	ecx
	shl	ecx, 16
	add	ecx, [button_cur_y]
	inc	ecx
	int	0x40

	mov	ebx, [button_cur_x]
	inc	ebx
	shl	ebx, 16
	add	ebx, [button_cur_x]
	inc	ebx
	mov	ecx, [button_cur_y]
	inc	ecx
	shl	ecx, 16
	add	ecx, [button_cur_y]
	inc	ecx
	add	ecx, BUTTON_SIDE-2
	int	0x40

	mov	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE-1
	shl	ebx, 16
	add	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE-1
	mov	ecx, [button_cur_y]
	inc	ecx
	shl	ecx, 16
	add	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE-1
	int	0x40

	mov	ebx, [button_cur_x]
	inc	ebx
	shl	ebx, 16
	add	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE-2
	mov	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE-1
	shl	ecx, 16
	add	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE-1
	int	0x40

	mov	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE
	shl	ebx, 16
	add	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE
	mov	ecx, [button_cur_y]
	shl	ecx, 16
	add	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE-1
	int	0x40

	mov	ebx, [button_cur_x]
	shl	ebx, 16
	add	ebx, [button_cur_x]
	add	ebx, BUTTON_SIDE
	mov	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE
	shl	ecx, 16
	add	ecx, [button_cur_y]
	add	ecx, BUTTON_SIDE
	int	0x40

	jmp	.symbol


draw_page_switcher:

	mov	eax, 8
	mov	edx, 0x8000FFAA
	int	0x40

	mov	esi, 0x00F0F0F0
	cmp	[redraw_flag], 1
	jnz	@f
	mov	esi, 0x00808080
    @@:	mov	eax, 8
	mov	ebx, 2*0x10000+60
	mov	ecx, 157*0x10000+19
	mov	edx, 0x2000FFAA
	int	0x40

	cmp	[symbol_start], 0
	jnz	@f
	mov	eax, 4
	mov	ebx, 11*0x10000+164
	mov	ecx, 0x80000000
	mov	edx, string_000_127
	int	0x40
	ret

    @@:	mov	eax, 4
	mov	ebx, 11*0x10000+164
	mov	ecx, 0x80000000
	mov	edx, string_128_255
	int	0x40
	ret


draw_codes:

	mov	eax, 13
	mov	ebx, 80*0x10000+220
	mov	ecx, 164*0x10000+9
	mov	edx, 0x00F0F0F0
	int	0x40

	mov	eax, 4
	mov	ebx, 80*0x10000+164
	mov	ecx, 0x80000000
	mov	edx, string_ASCII_CODE
	int	0x40

	mov	ebx, 180*0x10000+164
	mov	ecx, 0x80000000
	mov	edx, string_ASCII_HEX_CODE
	int	0x40

	mov	eax, 47
	mov	ebx, 0x00030000			; 3 digits, dec
	xor	ecx, ecx
	add	cl, [symbol_focused]
	mov	edx, 152*0x10000+164
	xor	esi, esi
	int	0x40

	mov	ebx, 0x00020100			; 2 digits, hex
	mov	edx, 276*0x10000+164
	int	0x40

	ret


quit:
	mov	eax, -1
	int	0x40

window_title		db 'ASCIIVju v0.3 R3',0

string_000_127		db '000-127',0
string_128_255		db '128-255',0
string_ASCII_CODE	db 'ASCII Code:    ',0
string_ASCII_HEX_CODE	db 'ASCII Hex-Code:   ',0

skin_height		dd 0

button_x		dd 2
button_y		dd 2

symbol_current		db 0
symbol_start		db 0
button_cur_x		dd 0
button_cur_y		dd 0
button_cur_id		db 0

db 0,0,0					; unused

is_active		dd 0			; is current symbol selected?

symbol_unfocused	db 0
symbol_focused		db 0
redraw_flag		dd 0

db 0,0,0,0					; unused
i_end:

rb 0x500					;stack
e_end:
