;-------------------------------------------------------------------------------
; DEBUG BOARD v1.0.1 for APPLICATIONS and KERNEL DEVELOPMENT
; See f63
; Compile with FASM for KolibriOS
;-------------------------------------------------------------------------------
	use32
	org	0
	db	"MENUET01"
	dd	1
	dd	START
	dd	I_END
	dd	MEM_END
	dd	STACK_TOP
	dd	PARAMS_STRING
	dd	0
;-------------------------------------------------------------------------------
	include	"lang.inc"
	include	"../../../macros.inc"

	purge	newline
MAX_STRINGS = 45
STRING_LENGTH = 80	;must be a multiple of 4
LINE_HEIGHT = 16
WINDOW_W = 60*8+5*2
WINDOW_W_MIN = 330
WORK_AREA_H_MIN = 60
WINDOW_W_PRCNT = 40
WINDOW_H_PRCNT = 75
TEXTFIELD_H = MAX_STRINGS*LINE_HEIGHT
FONT_TYPE = 0x50000000
;-------------------------------------------------------------------------------
START:
	mcall	68, 11	;init heap
	
	call	parse_params_string
	test	eax, eax
	jnz	.no_params
	call	check_and_fix_params

.no_params:
	call	STATE_handler

	mov	ecx, MAX_STRINGS*(STRING_LENGTH/4)
	mov	edi, text1
	mov	eax, "    "
	push	ecx
	rep	stosd

	pop	ecx
	mov	edi, text2
	rep	stosd

	cmp	[params.daemon_mode], 1
	je	.create_log_file
	
	call	calc_window_size
	mov	ax, word [xstart+2]
	mov	[prev_win_x], ax
	mov	ax, word [ystart+2]
	mov	[prev_win_y], ax
	mov	ax, word [xstart]
	mov	[prev_win_w], ax
	mov	ax, word [ystart]
	mov	[prev_win_h], ax
	
	cmp	[fullscreen_mode], 0
	je	@f
	call	calc_fullscreen_window_size
	
@@:
	mcall	48, 3, sc, sizeof.system_colors
	
	xor	eax, eax
	mov	[vmode], eax
	mov	dword [text1-4], eax
	mov	dword [text1-8], eax
	mov	dword [text2-4], eax
	mov	dword [text2-8], eax
	mov	[krnl_cnt], eax
	mov	[hl_line_count], eax

.create_log_file:
	xor	eax, eax
	mov	byte [buffer_length], al
	mov	[filepos], eax

	cmp	[params.logging_mode], 0
	je	@f
	
	mov	esi, [log_filename]
	call	create_file
	jnc	@f
	mov	esi, default_log_filename
	mov	[log_filename], default_log_filename
	call	create_file
	
@@:
	cmp	[params.daemon_mode], 0
	je	.red
	mcall	40, 0	;set event mask (do not receive any event codes)

.red:
	cmp	[params.daemon_mode], 1
	je	.still
	mcall	9, procinfo, -1			;get info about board process
	mov	eax, dword [procinfo+42]	;width of board window
	mov	edx, WINDOW_W_MIN
	cmp	eax, edx
	jae	@f
	mcall	67, -1, -1, WINDOW_W_MIN, -1	;resize window
	mov	dword [procinfo+42], edx 

@@:
	mcall	48, 4				; get window header height
	mov	[window_header_height], eax
	mov	esi, WORK_AREA_H_MIN
	add	esi, eax
	mov	eax, dword [procinfo+46]	;height of board window
	cmp	eax, esi
	jae	@f
	mcall	67, -1, -1, -1 ;resize window
	mov	dword [procinfo+46], esi 

@@:	
	call	draw_window

.still:
	cmp	byte [buffer_length], 0
	je	@f
	cmp	[params.logging_mode], 0
	je	@f
	call	write_buffer
	
@@:
	cmp	[params.daemon_mode], 0
	je	@f
	mcall	5, 50				;pause for 0.5 sec
	jmp	.read_byte
@@:
	mcall	23, 50				; wait here for event
	dec	eax				; redraw request ?
	jz	.red
	
	dec	eax				; key in buffer ?
	jz	.key
	
	dec	eax				; button in buffer ?
	jz	.button

.read_byte:
	mcall	63, 2				;read a byte from the debug buffer
	cmp	ebx, 1
	je	.new_data
	jne	.still
	
.key:
	mcall	2
	cmp	ah, " "
	jne	@f
	not	[vmode]
	and	[vmode], 1
	jmp	.red
@@:
	cmp	ah, 51 ; F2
	je	.open_boardlog
	cmp	ah, 0x1B		;[Esc]?
	je	terminate_board
	cmp	ah, 0x44		;[F11]?
	jne	.still
	cmp	[fullscreen_mode], 0
	jne	@f
	mov	[fullscreen_mode], 1
	mcall	9, procinfo, -1		;get info about process
	mov	ax, word [ebx+34]
	mov	[prev_win_x], ax
	mov	ax, word [ebx+38]
	mov	[prev_win_y], ax
	mov	ax, word [ebx+42]
	mov	[prev_win_w], ax
	mov	ax, word [ebx+46]
	mov	[prev_win_h], ax
	call	calc_fullscreen_window_size
	movzx	ebx, word [xstart+2]
	movzx	ecx, word [ystart+2]
	movzx	edx, word [xstart]
	movzx	esi, word [ystart]
	mcall	67			;change window size & position
	jmp	.still
	
@@:
	mov	[fullscreen_mode], 0
	mcall	71, 2, title, 1			;set window caption
	mcall	18, 25, 2, -1, 0		;set window position (usual)
	mov	[already_on_top], 0
	movzx	ebx, word [prev_win_x]
	movzx	ecx, word [prev_win_y]
	movzx	edx, word [prev_win_w]
	movzx	esi, word [prev_win_h]
	mcall	67				;change window size & position
	
@@:
	jmp	.still
	
.open_boardlog:
	mov	eax, [log_filename]
	mov	[open_log_in_tinypad+8], eax
	mcall 70, open_log_in_tinypad
	jmp	.red
	
.button:
	mcall	17			; get id
	cmp	ah, 1			; button id=1 ?
	jne	.button.noclose
	
.exit:
	jmp	terminate_board
.button.noclose:
	cmp	ah, 3
	jne	@f
	mov	[vmode], 0
	jmp	.red
@@:
	cmp	ah, 4
	jne	@f
	mov	[vmode], 1
@@:
	jmp	.red

.new_data:
	cmp	byte [buffer_length], 255
	jne	@f
	cmp	[params.logging_mode], 0
	je	@f
	call	write_buffer
	
@@:
	movzx	ebx, byte [buffer_length]
	mov	[ebx+tmp], al
	inc	byte [buffer_length]
	cmp	[params.daemon_mode], 1
	je	.read_byte
	mov	ebp, [targ]
	cmp	al, 10
	jz	.new_line
	cmp	al, 13
	jz	.new_check
	jmp	.char

.new_line:
	and	[ebp-8], dword 0
	inc	dword [ebp-4]
	cmp	[ebp-4], dword MAX_STRINGS
	jb	.noypos
	mov	[ebp-4], dword MAX_STRINGS-1
	lea	esi, [ebp+STRING_LENGTH]
	mov	edi, ebp
	mov	ecx, (MAX_STRINGS-1)*(STRING_LENGTH/4)
	cld
	rep	movsd

	mov	esi, [ebp-4]
	imul	esi, STRING_LENGTH
	add	esi, [ebp-8]
	add	esi, ebp
	mov	ecx, STRING_LENGTH/4
	mov	eax, "    "
	rep	stosd
	
.noypos:
	mov	[targ], text1
	and	[krnl_cnt], 0
	jmp	.new_check

.char:
	cmp	ebp, text2
	je	.add2
	mov	ecx, [krnl_cnt]
	cmp	al, [krnl_msg+ecx]
	jne	.nokrnl
	inc	[krnl_cnt]
	cmp	[krnl_cnt], 4
	jne	.new_check
	mov	[targ], text2
.nokrnl:
	mov	ebp, [targ]
	jecxz	.add
	push	eax
	mov	esi, krnl_msg
.l1:
	lodsb
	call	add_char
	loop	.l1
	pop	eax
.add:
	and	[krnl_cnt], 0
.add2:
	call	add_char

.new_check:
	mcall	63, 2			;read a byte from the debug buffer
	cmp	ebx, 1
	je	.new_data
	
	cmp	[params.daemon_mode], 1
	je	.still
	call	draw_text
	
	jmp	.still
;-------------------------------------------------------------------------------

;------------------------------------------------------------------------------
add_char:
	push	esi
	mov	esi, [ebp-4]
	imul	esi, STRING_LENGTH
	add	esi, [ebp-8]
	mov	[ebp+esi], al
	cmp	dword [ebp-8], STRING_LENGTH-1
	je	.ok
	inc	dword [ebp-8]
.ok:
	pop	esi
	ret
;-------------------------------------------------------------------------------

calc_fullscreen_window_size:
;-------------------------------------------------------------------------------
	push	eax
	
	mcall	14			;get screen size
;	inc	ax
	mov	word [ystart], ax
	shr	eax, 16
;	inc	ax
	mov	word [xstart], ax
	xor	eax, eax
	mov	word [xstart+2], ax
	mov	word [ystart+2], ax
	
	pop	eax
	ret
;-------------------------------------------------------------------------------

calc_window_size:
;-------------------------------------------------------------------------------
	pusha

	mcall	48, 5			;get screen working area size
	mov	[ystart], ebx
	ror	eax, 16
	ror	ebx, 16
	mov	[xstart], eax
	movzx	ecx, ax
	movzx	edx, bx
	shr	eax, 16
	shr	ebx, 16
	sub	eax, ecx
	sub	ebx, edx
	inc	eax
	inc	ebx
	
	mov	ecx, 100
	mov	edx, WINDOW_W_PRCNT
	mul	edx
	xor	edx, edx
	div	ecx
	cmp	ax, WINDOW_W_MIN
	jae	@f
	mov	ax, WINDOW_W_MIN
@@:
	mov	word [xstart], ax
	sub	word [xstart+2], ax
	mov	eax, ebx
	mov	edx, WINDOW_H_PRCNT
	mul	edx
	xor	edx, edx
	div	ecx
	mov	dx, ax
	mov	cx, WORK_AREA_H_MIN
	mcall	48, 4				; get window header height
	add	ax, cx
	cmp	dx, ax
	jae	@f
	mov	dx, ax
@@:
	mov	word [ystart], dx
	
	popa
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
write_buffer:
	pusha
	
	mov	edx, tmp
	movzx	ecx, byte [buffer_length]
	mov	esi, [log_filename]
	mov	ebx, ecx
	
.write_to_logfile:
	call	write_to_file
	cmp	eax, 5
	jne	@f
	mov	esi, [log_filename]
	mov	[filepos], 0
	call	create_file
	jnc	.write_to_logfile
;	mov	[params.logging_mode], 0
	jmp	.ret
@@:
	add	[filepos], ebx
	xor	eax, eax
	mov	byte [buffer_length], al
	
.ret:
	popa
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
parse_params_string:
;result: eax = 0 (OK), eax = -1 (no params string), -2 (wrong params string)
	xor	eax, eax
	pusha
	
	mov	esi, PARAMS_STRING
	mov	ecx, 256
	
	cmp	byte [esi], 0
	je	.no_params_string
	
@@:
	call	skip_spaces
	call	check_param
	inc	eax
	jz	.no_params_string
	inc	eax
	jz	.wrong_params_string
	cmp	byte [esi], 0
	je	.ok
	loop	@b

.ok:
	popa
	
	ret
	
.no_params_string:
	popa

	dec	eax
	
	ret

.wrong_params_string:
	popa
	
	dec	eax
	dec	eax
	
	ret
	
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
skip_spaces:
	push	eax
	cld
.0:
	lodsb
	cmp	al, " "
	je	@f
	cmp	al, 9	;skip tabs too
	jne	.ret
@@:
	loop	.0
	
.ret:
	dec	esi
	
	pop	eax
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
check_param:
	push	ebx

	test	ecx, ecx
	jz	.empty
	
	cld
	lodsb
	test	al, al
	jnz	@f
	dec	esi
	jmp	.empty
	
@@:
	cmp	al, "-"
	jne	.not_option
	dec	ecx
	lodsb
	test	al, al
	jz	.wrong_option
	cmp	al, " "
	je	.wrong_option
	cmp	al, 9
	je	.wrong_option
	
	dec	esi
	call	check_option
	inc	eax
	jnz	.ok

.wrong_option:
	lea	esi, [esi-2]
	jmp	.wrong
	
.not_option:
	cmp	[params.log_filename_found], 1
	je	.wrong
	mov	[params.log_filename], esi
	cmp	al, '"'
	je	.quoted
	dec	[params.log_filename]
	xor	ebx, ebx
	inc	ebx
	dec	ecx
	
@@:
	lodsb
	test	al, al
	jz	@f
	cmp	al, " "
	je	@f
	cmp	al, 9
	je	@f
	inc	ebx
	loop	@b
	dec	ebx
	
@@:
	dec	esi
	mov	[params.log_filename_found], 1
	mov	[params.log_filename.size], ebx
	jmp	.ok
	
.quoted:
	xor	ebx, ebx
	dec	ecx
.quoted.0:
	lodsb
	test	al, al
	jnz	@f
	lea	ecx, [ecx+ebx+1]
	lea	esi, [esi-2]
	sub	esi, ebx
	jmp	.wrong
@@:
	cmp	al, '"'
	je	@f
	inc	ebx
	loop	.quoted.0
	lea	ecx, [ecx+ebx+1]
	lea	esi, [esi-2]
	sub	esi, ebx
	jmp	.wrong
	
@@:
	mov	al, byte [esi]
	test	al, al
	jz	@f
	cmp	al, " "
	je	@f
	cmp	al, 9
	jne	.wrong		;there must be spaces or null after closing (final) quote mark
@@:
	mov	[params.log_filename_found], 1
	mov	[params.log_filename.size], ebx
	

.ok:
	xor	eax, eax
	
	pop	ebx
	ret
	
.empty:
	xor	eax, eax
	dec	eax
	
	pop	ebx
	ret
	
.wrong:
	xor	eax, eax
	dec	eax
	dec	eax
	
	pop	ebx
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
check_option:
	push	ebx edi
	lea	ebx, [esi-1]
	cmp	byte [esi], "-"
	je	.long_option
	
	cld
@@:
	lodsb
	test	al, al
	jz	.ok
	cmp	al, " "
	je	.ok
	cmp	al, 9
	je	.ok
	call	.subcheck
	test	eax, eax
	jz	.wrong
	loop	@b

.subcheck:
	cmp	al, "d"
	jne	@f
	mov	[params.daemon_mode], 1
	ret

@@:
	cmp	al, "s"
	jne	@f
	mov	[params.stop_daemon_flag], 1
	ret
	
@@:	
	cmp	al, "n"
	jne	@f
	mov	[params.logging_mode], 0
	ret
	
@@:	
	cmp	al, "r"
	jne	@f
	mov	[params.rewrite_log_flag], 1
	ret
	
@@:
	cmp	al, "f"
	jne	@f
	mov	[fullscreen_mode], 1
	ret
	
		


@@:
	xor	eax, eax
	ret

	
.long_option:
	lodsb
	dec	ecx
	jz	.wrong
	mov	edi, esi
	push	esi
	cmp	ecx, params.options.d.size-1
	jl	@f
	mov	esi, params.options.d
	call	cmp_str
	jnz	@f
	pop	esi
	lea	ecx, [ecx-params.options.d.size-1]
	lea	esi, [esi+params.options.d.size-1]
	mov	[params.daemon_mode], 1
	jmp	.long_option.0

@@:
	cmp	ecx, params.options.s.size-1
	jl	@f
	mov	esi, params.options.s
	call	cmp_str
	jnz	@f
	pop	esi
	lea	ecx, [ecx-params.options.s.size-1]
	lea	esi, [esi+params.options.s.size-1]
	mov	[params.stop_daemon_flag], 1
	jmp	.long_option.0
	
@@:
	cmp	ecx, params.options.n.size-1
	jl	@f
	mov	esi, params.options.n
	call	cmp_str
	jnz	@f
	pop	esi
	lea	ecx, [ecx-params.options.n.size-1]
	lea	esi, [esi+params.options.n.size-1]
	mov	[params.logging_mode], 0
	jmp	.long_option.0
	
@@:
	cmp	ecx, params.options.r.size-1
	jl	@f
	mov	esi, params.options.r
	call	cmp_str
	jnz	@f
	pop	esi
	lea	ecx, [ecx-params.options.r.size-1]
	lea	esi, [esi+params.options.r.size-1]
	mov	[params.rewrite_log_flag], 1
	jmp	.long_option.0
	
@@:
	cmp	ecx, params.options.f.size-1
	jl	@f
	mov	esi, params.options.f
	call	cmp_str
	jnz	@f
	pop	esi
	lea	ecx, [ecx-params.options.f.size-1]
	lea	esi, [esi+params.options.f.size-1]
	mov	[fullscreen_mode], 1
	jmp	.long_option.0
	
@@:
	pop	esi
	jmp	.wrong

.long_option.0:
	mov	al, byte [esi]
	test	al, al
	jz	@f
	cmp	al, " "
	je	@f
	cmp	al, 9
	jne	.wrong		;there must be spaces or null after long option 
@@:
	inc	esi
	
.ok:
	dec	esi
	xor	eax, eax
	
	pop	edi ebx
	ret
	
.wrong:
	xor	eax, eax
	dec	eax
	pop	edi esi
	ret
;-------------------------------------------------------------------------------

check_and_fix_params:
;-------------------------------------------------------------------------------
	push	eax
	
	cmp	[params.daemon_mode], 1
	jne	@f
	mov	[params.stop_daemon_flag], 0
	mov	[params.logging_mode], 1
	
@@:	
	mov	eax, [params.log_filename]
	test	eax, eax
	jz	@f
	mov	[log_filename], eax
	add	ebx, [params.log_filename.size]
	mov	byte [eax+ebx], 0
	
@@:
	pop	eax
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;************************	WINDOW DEFINITIONS AND DRAW ************************
;-------------------------------------------------------------------------------
draw_window:
	mcall	12, 1			; start of draw
	xor	edx, edx
	mov	dl, 0x13
	cmp	[fullscreen_mode], 0
	je	@f
	mov	dl, 1			;set fullscreen mode window
	
@@:
	shl	edx, 24
	or	edx, 0xFFFFFF
	xor	esi, esi
	mcall	0,[xstart],[ystart],,,title

	cmp	[fullscreen_mode], 0
	je	.l0
	
	cmp	[already_on_top], 1
	je	@f
	mcall	18, 25, 2, -1, 1			;set window position (always on top)
	mov	[already_on_top], 1

@@:
	mcall	13, [xstart], [ystart], 0xFFFFFF	;draw a rectangle (fill the window)
	
.l0:
	mov	ebx, 15 shl 16+(8*4+5)
	xor	ecx, ecx
	cmp	[fullscreen_mode], 1
	je	@f
	mov	ecx, [window_header_height]

@@:
	add	ecx, 5
	shl	ecx, 16
	mov	cx, LINE_HEIGHT+5
	mov	eax, [sc.work_button]
	cmp	[vmode], 0
	jne	@f
	call	lighten_color	;make button lighter if it's active
@@:
	mov	esi, eax
	mcall	8,,,3	;'User' button
	
	push	ebx ecx
	
	shr	ecx, 16
	mov	bx, cx
	rol	ebx, 16
	add	bx, 3
	ror	ebx, 16
	add	bx, 4
	mov	ecx, 0x90
	shl	ecx, 24
	add	ecx, [sc.work_button_text]
	mcall	4,,, u_button_text
	cmp	[vmode], 0
	jne	@f
	add	ebx, 1 shl 16
	mcall			;make text bolder if the button is active

@@:
	pop	ecx ebx
	
	add	ebx, (8*4+10) shl 16
	mov	bx, (8*6+5+5)
	mov	eax, [sc.work_button]
	cmp	[vmode], 1
	jne	@f
	call	lighten_color	;make button lighter if it's active
@@:
	mov	esi, eax
	mcall	8,,,4	;'Kernel' button
	
	shr	ecx, 16
	mov	bx, cx
	rol	ebx, 16
	add	bx, 3
	ror	ebx, 16
	add	bx, 4
	mov	ecx, 0x90
	shl	ecx, 24
	add	ecx, [sc.work_button_text]
	mcall	4,,, k_button_text
	cmp	[vmode], 1
	jne	@f
	add	ebx, 1 shl 16
	mcall			;make text bolder if the button is active

@@:
	call	draw_text
	mcall	12, 2			; 2, end of draw
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
draw_text:
	mov	ebx, 15 shl 16 + 0
	xor	bx, bx
	cmp	[fullscreen_mode], 1
	je	@f
	mov	bx, word [window_header_height] 

@@:
	add	bx, 5+(LINE_HEIGHT+5)+5			;space between window header and textfield
	mov	ecx, FONT_TYPE
	mov	edi, 0xFFFFFF
	mov	edx, text2
	cmp	[vmode], 1
	je	.kern
	mov	edx, text1
.kern:
	push	ebx edx
	
	
	
	mov	eax, dword [procinfo+42]		;x-size of board window
	sub	eax, 15*2
	xor	edx, edx
	mov	ebx, 8
	div	ebx
	
@@:
	cmp	[vmode], 1
	jne	@f
	add	eax, 4

@@:
	mov	esi, STRING_LENGTH
	cmp	eax, esi
	ja	@f
	mov	esi, eax

@@:
	mov	eax, dword [procinfo+46]		;y-size of board window
	cmp	[fullscreen_mode], 1
	je	@f
	sub	eax, [window_header_height]

@@:
	sub	eax, 5+(LINE_HEIGHT+5)+5+5
	xor	edx, edx
	mov	ebx, 16
	div	ebx
	
	pop	edx ebx
	
	mov	ebp, [edx-4]	;[edx-4] = count of strings
	cmp	eax, ebp
	ja	@f
	mov	ebp, eax

@@:
	test	ebp, ebp
	jz	.ret
	mov	eax, 4
.newline:
	cmp	[edx], dword "K : "
	jne	.no_k_prefix
	
	push	edx esi
	add	edx, 4
	sub	esi, 4
	
	push	ecx esi edi
	mov	ecx, STRING_LENGTH-4
	mov	edi, edx
	
	mov	esi, proc_fors_term
	call	cmp_str
	jz	.highlight

	mov	esi, proc_fors_term_sp
	call	cmp_str
	jnz	.no_fors_term

.highlight:
	lea	edi, [edx+STRING_LENGTH*7]
	mov	esi, unexp_end_of_stack
	call	cmp_str
	jnz	@f
	mov	[hl_line_count], 8
	jmp	.no_fors_term
@@:
	mov	[hl_line_count], 10

.no_fors_term:
	pop	edi esi ecx
	push	ecx
	cmp	[hl_line_count], 0
	je	@f
	and	ecx, 0xFF000000
	or	ecx, 0xCC0000		;red
	dec	[hl_line_count]
@@:
	mcall				; draw info text with function 4
	pop	ecx
	pop	esi edx
	jmp	@f
.no_k_prefix:
	mcall
@@:
	add	ebx, LINE_HEIGHT
	add	edx, STRING_LENGTH
	dec	ebp
	jnz	.newline
.ret:
	ret
;-------------------------------------------------------------------------------

cmp_str:
;-------------------------------------------------------------------------------
	pusha

@@:
	lodsb
	test	al, al
	jz	.ret
	scasb
	loope	@b
	xor	eax, eax
	inc	eax
	
.ret:
	popa
	ret
;-------------------------------------------------------------------------------
	
;-------------------------------------------------------------------------------
lighten_color:
	ror	eax, 16
	add	al, 20
	rol	eax, 8
	add	al, 20
	rol	eax, 8
	add	al, 20
	ret
;-------------------------------------------------------------------------------
	
;-------------------------------------------------------------------------------
;*	input:	esi = pointer to the file name	*
;-------------------------------------------------------------------------------
create_file:
	pusha
	
	cmp	[params.rewrite_log_flag], 1
	je	.create
	xor	eax, eax
	mov	dword [f70_structure+0], 5		; get file info
	mov	dword [f70_structure+4], eax		; reserved
	mov	dword [f70_structure+8], eax		; reserved
	mov	dword [f70_structure+12], eax		; reserved
	mov	dword [f70_structure+16], file_info	; file info buf pointer
	mov	dword [f70_structure+20], eax		; reserved
	mov	dword [f70_structure+21], esi 		; pointer to the file name
	mcall	70, f70_structure
	cmp	eax, 5
	je	.create			;create file if it doesn't exist
	test	eax, eax
	jnz	.create
	
	cmp	dword [file_info+36], 0
	jne	.create
	mov	eax, dword [file_info+32]
	mov	[filepos], eax

	jmp	.ret
	
.create:
	xor	eax, eax
	mov	dword [f70_structure+0], 2	; create file
	mov	dword [f70_structure+4], eax	; reserved
	mov	dword [f70_structure+8], eax	; reserved
	mov	dword [f70_structure+12], eax	; 0 bytes to write (just create)
	mov	dword [f70_structure+16], eax	; NULL data pointer (no data)
	mov	dword [f70_structure+20], eax	; reserved
	mov	dword [f70_structure+21], esi ; pointer to the file name
	mcall	70, f70_structure
	test	eax, eax
	jz	.ret
	stc
.ret:
	popa
	ret
;-------------------------------------------------------------------------------






;-------------------------------------------------------------------------------
;*	input:	esi = pointer to the file name	*
;*		edx = pointer to data buffer	*
;*		ecx = data length		*
;-------------------------------------------------------------------------------
write_to_file:
	push	ebx
	mov	dword [f70_structure+0], 3	; write to file
	mov	eax,	[filepos]
	mov	dword [f70_structure+4], eax ; lower position addr
	mov	dword [f70_structure+8], 0	; upper position addr (0 for FAT)
	mov	dword [f70_structure+12], ecx ; number of bytes to write
	mov	dword [f70_structure+16], edx ; pointer to data buffer
	mov	dword [f70_structure+20], 0	; reserved
	mov	dword [f70_structure+21], esi ; pointer to the file name
	mcall	70, f70_structure
	clc
	test	eax, eax
	jz	.out
	stc
.out:
	pop	ebx
	ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
STATE_handler:
	mcall	68, 22, STATE_shm_block.name,, 0x00	;open "BOARDAPP:STATE" shared memory block (ro access)
	test	eax, eax
	jz	.create_STATE_shm_block
	
	mov	ecx, 20*10	;timeout = 20 sec
.0:
	mov	ebx, [eax]
	test	ebx, ebx
	jz	@f
	mcall	5, 10		;wait for 0.1 sec
	loop	.0
	jmp	terminate_board
	
@@:	
	mov	ebx, [eax+4]
	test	ebx, ebx
	jnz	terminate_board
	cmp	dword [eax+12], 0
	jne	@f
	mov	ecx, [eax+16]
	mcall	18, 21		;get slot number by PID
	test	eax, eax
	jz	terminate_board
	mov	ecx, eax
	mcall	18, 3		;activate board window
	jmp	terminate_board
	
@@:
	cmp	[params.stop_daemon_flag], 1
	jne	terminate_board
	cmp	dword [eax+12], 1
	jne	terminate_board
	mov	ecx, [eax+16]
	mcall	18, 18		;terminate daemonized board
	jmp	terminate_board
	
.create_STATE_shm_block:
	cmp	[params.stop_daemon_flag], 1
	je	terminate_board

	mcall	68,,,STATE_shm_block.size, 0x08		;create "BOARDAPP:STATE" shared memory block (ro access for other processes)
	test	eax, eax
	jz	terminate_board
	
	mov	edi, eax
	mov	dword [edi], 1
	mov	dword [edi+4], STATE_shm_block.version
	mov	dword [edi+8], STATE_shm_block.size
	movzx	eax, byte [params.daemon_mode]
	mov	dword [edi+12], eax
	mcall	9, procinfo, -1
	mov	eax, [ebx+30]				;PID of board
	mov	dword [edi+16], eax
	mov	dword [edi], 0
	
.ret:
	ret
;-------------------------------------------------------------------------------

terminate_board:
	mcall	-1
;-------------------------------------------------------------------------------
; DATA

	if lang eq ru
		title	db	"Доска отладки и сообщений v1.0.1", 0
	else if lang eq it
		title	db	"Notifiche e informazioni generiche per il debug v1.0.1", 0
	else if lang eq ge
		title	db	"Allgemeines debug- & nachrichtenboard v1.0.1", 0
	else
		title	db	"General debug & message board v1.0.1", 0
	end if

STATE_shm_block:
;structure of BOARDAPP:STATE shared memory block:
;+0	dd	mutex (0 - unlocked, block was already written by board, 1 - locked, board is writing into block)
;+4	dd	version of block structure (current is 0)
;+8	dd	size of block (min. 4096 byte for version 0)
;+12	dd	board's state (0 - usual windowed app, 1 - daemonized)
;+16	dd	board's PID
;next bytes are reserved

	.name	db	"BOARDAPP:STATE", 0
	.size = 4*1024
	.version = 0

default_log_filename	db	"/tmp0/1/boardlog.txt", 0
krnl_msg	db	"K : "
k_button_text	db	"Kernel", 0
u_button_text	db	"User", 0

proc_fors_term		db	"Process - forced terminate PID: ", 0
proc_fors_term_sp	db	"Proceso - terminado forzado PID: ", 0
unexp_end_of_stack	db	"Unexpected end of the stack", 0

log_filename	dd	default_log_filename

filepos			dd	0

already_on_top	db	0


params:
	.log_filename		dd	0
	.log_filename.size	dd	0
	.log_filename_found	db	0
	.daemon_mode		db	0	;0 - normal mode, 1 - daemon mode
	.stop_daemon_flag	db	0
	.logging_mode		db	1	;0 - don't log, 1 - log
	.rewrite_log_flag	db	0
	.options.d		db	"daemonize", 0
	.options.d.size = $-.options.d
	.options.s		db	"stop-daemon", 0
	.options.s.size = $-.options.s
	.options.n		db	"no-log", 0
	.options.n.size = $-.options.n
	.options.r		db	"rewrite-log", 0
	.options.r.size = $-.options.r
	.options.f		db	"fullscreen", 0
	.options.f.size = $-.options.f

vmode			dd	1
fullscreen_mode		db	0


align 4

targ	dd	text1

open_log_in_tinypad:
	dd	7
	dd	0
	dd	0
	dd	0
	dd	0
	db	"/sys/tinypad", 0

I_END:

	prev_win_x	dw	?
prev_win_y	dw	?
prev_win_w	dw	?
prev_win_h	dw	?

f70_structure:
	dd	?	; subfunction number
	dd	?	; 
	dd	?	; 
	dd	?	; 
	dd	?	; 
	db	?
	dd	?	; pointer to the filename

window_header_height	dd	?
buffer_length		dd	?
krnl_cnt		dd	?
xstart			dd	?
ystart			dd	?

hl_line_count	rd	1

sc	system_colors

	rd	2
text1	rb	STRING_LENGTH*MAX_STRINGS	;'User' messages

	rd	2
text2	rb	STRING_LENGTH*MAX_STRINGS	;'Kernel' messages

tmp			rb	256
PARAMS_STRING		rb	256
procinfo		rb	1024
file_info		rb	40

	align	4
	rb	512
STACK_TOP:

MEM_END:
