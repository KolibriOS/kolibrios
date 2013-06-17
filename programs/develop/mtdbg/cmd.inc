; TODO: add both visual and command modes

; scan and build command line
scan_cmdline:
	pusha
	cmp	[cmdline_len], cmdline_width
	jae	waitevent
	push	eax
	call	clear_cmdline_end
	pop	eax
	mov	edi, cmdline
	mov	ecx, [cmdline_len]
	add	edi, ecx
	lea	esi, [edi-1]
	sub	ecx, [cmdline_pos]
	std
	rep	movsb
	cld
	stosb
	inc	[cmdline_len]
	call	draw_cmdline_end
	inc	[cmdline_pos]
	call	draw_cursor
	jmp	waitevent
.backspace:
	cmp	[cmdline_pos], 0
	jz	waitevent
	dec	[cmdline_pos]
.delchar:
	call	clear_cmdline_end
	mov	edi, [cmdline_pos]
	dec	[cmdline_len]
	mov	ecx, [cmdline_len]
	sub	ecx, edi
	add	edi, cmdline
	lea	esi, [edi+1]
	rep	movsb
	call	draw_cmdline_end
	call	draw_cursor
	jmp	waitevent
.del:
	mov	eax, [cmdline_pos]
	cmp	eax, [cmdline_len]
	jae	waitevent
	jmp	.delchar
.left:
	cmp	[cmdline_pos], 0
	jz	waitevent
	call	hide_cursor
	dec	[cmdline_pos]
	call	draw_cursor
	jmp	waitevent
.right:
	mov	eax, [cmdline_pos]
	cmp	eax, [cmdline_len]
	jae	waitevent
	call	hide_cursor
	inc	[cmdline_pos]
	call	draw_cursor
	jmp	waitevent
.home:
	call	hide_cursor
	and	[cmdline_pos], 0
	call	draw_cursor
	jmp	waitevent
.end:
	call	hide_cursor
	mov	eax, [cmdline_len]
	mov	[cmdline_pos], eax
	call	draw_cursor
.up:
.down:
	jmp	waitevent
;; We also trying to execute previous command, if empty command_line
.enter:
	mov	ecx, [cmdline_len]
	cmp ecx, 0
	jg	.exec_cur
	mov cl, byte [cmdline_prev]
	cmp cl, 0
	jz waitevent
.exec_prev:
	mov	esi, cmdline_prev
	jmp .exec
.exec_cur:
	mov	esi, cmdline
.exec:
	mov	byte [esi+ecx], 0
	and	[cmdline_pos], 0
	push	esi
	call	clear_cmdline_end
	call	draw_cursor
	pop	esi
	and	[cmdline_len], 0
; skip leading spaces
	call	skip_spaces
	cmp	al, 0
	jz	waitevent

; vim: ft= fasm

