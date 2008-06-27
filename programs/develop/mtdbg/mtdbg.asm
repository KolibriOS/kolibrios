format binary
include '../../macros.inc'
use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	used_mem
	dd	used_mem
	dd	i_param
	dd	0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; GUI ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

data_width equ 80
data_x_pos equ 12
data_x_size equ data_width*6

title_x_pos equ 30
title_y_pos equ 32
title_y_size equ 10

registers_x_pos equ data_x_pos
registers_y_pos equ (title_y_pos + title_y_size)
registers_y_size equ 30

dump_y_pos equ (registers_y_pos + registers_y_size + 5)
dump_height equ 4
dump_y_size equ (dump_height*10)

disasm_y_pos equ (dump_y_pos + dump_y_size + 4)
disasm_height equ 16
disasm_y_size equ (disasm_height*10)

messages_width equ data_width
messages_height equ 12
messages_x_pos equ data_x_pos
messages_y_pos equ (disasm_y_pos + disasm_y_size + 4)
messages_x_size equ messages_width*6
messages_y_size equ messages_height*10

cmdline_width equ data_width
cmdline_x_pos equ data_x_pos
cmdline_y_pos equ (messages_y_pos + messages_y_size + 10)
cmdline_x_size equ messages_x_size
cmdline_y_size equ 10

wnd_x_size equ (data_x_pos + messages_x_size + data_x_pos)
wnd_y_size equ (cmdline_y_pos + cmdline_y_size + data_x_pos)

start:
	mcall	68, 11
	mov	edi, messages
	mov	ecx, messages_width*messages_height
	mov	al, ' '
	rep	stosb
	xor	eax, eax
	mov	[messages_pos], eax
	mov	[cmdline_len], eax
	mov	[cmdline_pos], eax
	mov	edi, needzerostart
	mov	ecx, (needzeroend-needzerostart+3)/4
	rep	stosd
	mov	esi, begin_str
	call	put_message_nodraw
; set event mask - default events and debugging events
	push	40
	pop	eax
	mov	ebx, 0x107
	mcall
; set debug messages buffer
	mov	ecx, dbgbufsize
	mov	dword [ecx], 256
	xor	ebx, ebx
	mov	[ecx+4], ebx
	mov	al, 69
	mcall
	mov	esi, i_param
	call	skip_spaces
	test	al, al
	jz	dodraw
	push	esi
	call	draw_window
	pop	esi
	call	OnLoadInit
	jmp	waitevent
dodraw:
	call	draw_window
waitevent:
	push	10
	pop	eax
	mcall
	cmp	al, 9
	jz	debugmsg
	dec	eax
	jz	dodraw
	dec	eax
	jz	keypressed
	dec	eax
	jnz	waitevent
; button pressed - we have only one button (close)
	push	-1
	pop	eax
	mcall
keypressed:
	mov	al, 2
	mcall
	shr	eax, 8
	cmp	al, 8
	jz	.backspace
	cmp	al, 0xB0
	jz	.left
	cmp	al, 0xB3
	jz	.right
	cmp	al, 0x0D
	jz	.enter
	cmp	al, 0xB6
	jz	.del
	cmp	al, 0xB4
	jz	.home
	cmp	al, 0xB5
	jz	.end
	cmp	al, 0xB1
	jz	.down
	cmp	al, 0xB2
	jz	.up
	cmp	al, 0xD8
	jz	CtrlF7
	cmp	al, 0xD9
	jz	CtrlF8
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
.enter:
	mov	ecx, [cmdline_len]
	test	ecx, ecx
	jz	waitevent
	mov	esi, cmdline
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
; now esi points to command
	push	esi
	mov	esi, prompt
	call	put_message_nodraw
	pop	esi
	push	esi
	call	put_message_nodraw
z1:	mov	esi, newline
	call	put_message
	pop	esi
	push	esi
	call	get_arg
	mov	[curarg], esi
	pop	edi
	mov	esi, commands
	call	find_cmd
	mov	eax, aUnknownCommand
	jc	.x11
; check command requirements
; flags field:
; &1: command may be called without parameters
; &2: command may be called with parameters
; &4: command may be called without loaded program
; &8: command may be called with loaded program
	mov	eax, [esi+8]
	mov	ecx, [curarg]
	cmp	byte [ecx], 0
	jz	.noargs
	test	byte [esi+16], 2
	jz	.x11
	jmp	@f
.noargs:
	test	byte [esi+16], 1
	jz	.x11
@@:
	cmp	[debuggee_pid], 0
	jz	.nodebuggee
	mov	eax, aAlreadyLoaded
	test	byte [esi+16], 8
	jz	.x11
	jmp	.x9
.nodebuggee:
	mov	eax, need_debuggee
	test	byte [esi+16], 4
	jnz	.x9
.x11:
	xchg	esi, eax
	call	put_message
.x10:
	jmp	waitevent
.x9:
	call	dword [esi+4]
	jmp	.x10

find_cmd:
; all commands are case-insensitive
	push	edi
.x4:
	mov	al, [edi]
	cmp	al, 0
	jz	.x5
	cmp	al, 'A'
	jb	@f
	cmp	al, 'Z'
	ja	@f
	or	al, 20h
@@:
	stosb
	jmp	.x4
.x5:
; find command
	pop	edi
.x6:
	cmp	dword [esi], 0
	jz	.x7
	push	esi
	mov	esi, [esi]
	lodsb
	movzx	ecx, al
	push	edi
	repz	cmpsb
	pop	edi
	pop	esi
	jz	.x8
	add	esi, 17
	jmp	.x6
.x7:
	stc
.x8:
	ret

get_arg:
	lodsb
	cmp	al, ' '
	ja	get_arg
	mov	byte [esi-1], 0
	cmp	al, 0
	jnz	skip_spaces
	dec	esi
skip_spaces:
	lodsb
	cmp	al, 0
	jz	@f
	cmp	al, ' '
	jbe	skip_spaces
@@:	dec	esi
	ret

clear_cmdline_end:
	mov	ebx, [cmdline_pos]
	mov	ecx, [cmdline_len]
	sub	ecx, ebx
	push	13
	pop	eax
	imul	ebx, 6
	imul	ecx, 6
	inc	ecx
	add	ebx, cmdline_x_pos
	shl	ebx, 16
	or	ebx, ecx
	mov	ecx, cmdline_y_pos*10000h + cmdline_y_size
	mov	edx, 0xFFFFFF
	mcall
	ret

draw_cmdline:
	xor	ebx, ebx
	jmp	@f
draw_cmdline_end:
	mov	ebx, [cmdline_pos]
@@:
	mov	esi, [cmdline_len]
	sub	esi, ebx
	push	4
	pop	eax
	xor	ecx, ecx
	lea	edx, [cmdline+ebx]
	imul	ebx, 6
	add	ebx, cmdline_x_pos
	shl	ebx, 16
	or	ebx, cmdline_y_pos+1
	mcall
	ret

put_message_nodraw:
; in: esi->ASCIZ message
	mov	edx, [messages_pos]
.m:
	lea	edi, [messages+edx]
.l:
	lodsb
	cmp	al, 0
	jz	.done
	call	test_scroll
	cmp	al, 10
	jz	.newline
	cmp	al, '%'
	jnz	@f
	cmp	dword [esp], z1
	jnz	.format
@@:
	stosb
	inc	edx
	jmp	.l
.newline:
	push	edx
	mov	ecx, messages_width
	xor	eax, eax
	xchg	eax, edx
	div	ecx
	xchg	eax, edx
	pop	edx
	test	eax, eax
	jz	.m
	sub	edx, eax
	add	edx, ecx
	jmp	.m
.done:
	mov	[messages_pos], edx
	ret
.format:
; at moment all format specs must be %<digit>X
	lodsb	; get <digit>
	sub	al, '0'
	movzx	ecx, al
	lodsb
	pop	eax
	pop	ebp
	push	eax
; write number in ebp with ecx digits
	dec	ecx
	shl	ecx, 2
.writenibble:
	push	ecx
	call	test_scroll
	pop	ecx
	mov	eax, ebp
	shr	eax, cl
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	inc	edx
	sub	ecx, 4
	jns	.writenibble
	jmp	.l

test_scroll:
	cmp	edx, messages_width*messages_height
	jnz	.ret
	push	esi
	mov	edi, messages
	lea	esi, [edi+messages_width]
	mov	ecx, (messages_height-1)*messages_width/4
	rep	movsd
	push	eax
	mov	al, ' '
	push	edi
	push	messages_width
	pop	ecx
	sub	edx, ecx
	rep	stosb
	pop	edi
	pop	eax
	pop	esi
.ret:	ret

put_message:
	call	put_message_nodraw

draw_messages:
	push	13
	pop	eax
	mov	edx, 0xFFFFFF
	mov	ebx, messages_x_pos*10000h+messages_x_size
	mov	ecx, messages_y_pos*10000h+messages_y_size
	mcall
	mov	edx, messages
	push	messages_width
	pop	esi
	xor	ecx, ecx
	mov	al, 4
	mov	ebx, messages_x_pos*10000h+messages_y_pos
@@:
	mcall
	add	edx, esi
	add	ebx, 10
	cmp	edx, messages+messages_width*messages_height
	jb	@b
	ret

draw_cursor:
	push	38
	pop	eax
	mov	ecx, cmdline_y_pos*10001h+cmdline_y_size-1
	mov	ebx, [cmdline_pos]
	imul	ebx, 6
	add	ebx, cmdline_x_pos
	mov	edx, ebx
	shl	ebx, 16
	or	ebx, edx
	xor	edx, edx
	mcall
	ret
hide_cursor:
	mov	ebx, [cmdline_pos]
	push	13
	pop	eax
	imul	ebx, 6
	add	ebx, cmdline_x_pos
	shl	ebx, 16
	inc	ebx
	mov	ecx, cmdline_y_pos*10000h + cmdline_y_size
	mov	edx, 0xFFFFFF
	mcall
	mov	ebx, [cmdline_pos]
	cmp	ebx, [cmdline_len]
	jae	.ret
	mov	al, 4
	xor	ecx, ecx
	lea	edx, [cmdline+ebx]
	imul	ebx, 6
	add	ebx, cmdline_x_pos
	shl	ebx, 16
	or	ebx, cmdline_y_pos+1
	push	1
	pop	esi
	mcall
.ret:
	ret

redraw_title:
	push	13
	pop	eax
	mov	edx, 0xFFFFFF
	mov	ebx, title_x_pos*10000h + data_x_pos+data_x_size-title_x_pos
	mov	ecx, title_y_pos*10000h + title_y_size
	mcall
draw_title:
	mov	al, 38
	mov	ebx, (data_x_pos-2)*10000h + title_x_pos-5
	mov	ecx, (title_y_pos+5)*10001h
	xor	edx, edx
	mcall
	push	NoPrgLoaded_len
	pop	esi
	cmp	[debuggee_pid], 0
	jz	@f
	mov	esi, [prgname_len]
@@:	imul	ebx, esi, 6
	add	ebx, title_x_pos+4
	shl	ebx, 16
	mov	bx, data_x_pos+data_x_size-10-5-6*7
	cmp	[bSuspended], 0
	jz	@f
	add	ebx, 6
@@:
	mcall
	mov	ebx, (data_x_pos+data_x_size-10+4)*0x10000 + data_x_pos+data_x_size+2
	mcall
	mov	al, 4
	mov	ebx, title_x_pos*10000h+title_y_pos
	xor	ecx, ecx
	mov	edx, NoPrgLoaded_str
	cmp	[debuggee_pid], 0
	jz	@f
	mov	edx, [prgname_ptr]
@@:
	mcall
	cmp	[debuggee_pid], 0
	jz	.nodebuggee
	mov	ebx, (data_x_pos+data_x_size-10-6*7)*10000h + title_y_pos
	mov	edx, aRunning
	push	7
	pop	esi
	cmp	[bSuspended], 0
	jz	@f
	add	ebx, 6*10000h
	mov	edx, aPaused
	dec	esi
@@:
	mcall
	ret
.nodebuggee:
	mov	al, 38
	mov	ebx, (data_x_pos+data_x_size-10-6*7-5)*0x10000 + data_x_pos+data_x_size+2
	mov	ecx, (title_y_pos+5)*10001h
	xor	edx, edx
	jmp	@b

draw_register:
; in: esi->value, edx->string, ecx=string len, ebx=coord
	push	edx
	push	ecx
	push	esi
	mov	eax, esi
	mov	esi, ecx
; color
	mov	ecx, 808080h
	cmp	[debuggee_pid], 0
	jz	.cd
	cmp	[bSuspended], 0
	jz	.cd
	xor	ecx, ecx
	mov	edi, [eax]
	cmp	dword [eax+oldcontext-context], edi
	jz	.cd
	mov	ecx, 0x00AA00
.cd:
	push	4
	pop	eax
	mcall
	imul	esi, 60000h
	lea	edx, [ebx+esi]
	mov	al, 47
	mov	ebx, 80101h
	mov	esi, ecx
	pop	ecx
	mcall
	lea	ebx, [edx+60000h*18]
	mov	esi, ecx
	pop	ecx
	pop	edx
	add	edx, ecx
	ret
draw_flag:
	movzx	edi, byte [edx+7]
	bt	[_eflags], edi
	jc	.on
	or	byte [edx], 20h
	jmp	.onoff
.on:
	and	byte [edx], not 20h
.onoff:
	mov	ecx, 808080h
	cmp	[debuggee_pid], 0
	jz	.doit
	cmp	[bSuspended], 0
	jz	.doit
	xor	ecx, ecx
	bt	[_eflags], edi
	lahf
	bt	dword [_eflags + oldcontext - context], edi
	rcl	ah, 1
	test	ah, 3
	jp	.doit
	mov	ecx, 0x00AA00
.doit:
	mov	ah, 0
	mcall
	ret

redraw_registers:
	push	13
	pop	eax
	mov	edx, 0xFFFFFF
	mov	ebx, data_x_pos*10000h + data_x_size
	mov	ecx, registers_y_pos*10000h + registers_y_size
	mcall
draw_registers:
	mov	esi, _eax
	push	4
	pop	ecx
	mov	edx, regs_strs
	mov	ebx, registers_x_pos*10000h+registers_y_pos
	call	draw_register
	add	esi, _ebx-_eax
	call	draw_register
	add	esi, _ecx-_ebx
	call	draw_register
	add	esi, _edx-_ecx
	call	draw_register
	mov	ebx, registers_x_pos*10000h+registers_y_pos+10
	add	esi, _esi-_edx
	call	draw_register
	add	esi, _edi-_esi
	call	draw_register
	add	esi, _ebp-_edi
	call	draw_register
	add	esi, _esp-_ebp
	call	draw_register
	mov	ebx, registers_x_pos*10000h+registers_y_pos+20
	add	esi, _eip-_esp
	call	draw_register
	mov	cl, 7
	add	esi, _eflags-_eip
	call	draw_register
	mov	al, 4
	mov	ecx, 808080h
	cmp	[debuggee_pid], 0
	jz	@f
	cmp	[bSuspended], 0
	jz	@f
	xor	ecx, ecx
@@:
	mov	edx, aColon
	xor	esi, esi
	inc	esi
	mov	ebx, (registers_x_pos+37*6)*10000h + registers_y_pos+20
	mcall
	mov	edx, flags
@@:
	add	ebx, 2*6*10000h
	call	draw_flag
	inc	edx
	cmp	dl, flags_bits and 0xFF
	jnz	@b
	ret

redraw_dump:
	push	13
	pop	eax
	mov	edx, 0xFFFFFF
	mov	ebx, data_x_pos*10000h + data_x_size
	mov	ecx, dump_y_pos*10000h + dump_y_size
	mcall
draw_dump:
; addresses
	mov	al, 47
	mov	ebx, 80100h
	mov	edx, data_x_pos*10000h + dump_y_pos
	mov	ecx, [dumppos]
	mov	esi, 808080h
	cmp	[debuggee_pid], 0
	jz	@f
	cmp	[bSuspended], 0
	jz	@f
	xor	esi, esi
@@:
	mcall
	add	ecx, 10h
	add	edx, 10
	cmp	dl, dump_y_pos + dump_y_size
	jb	@b
; hex dump of data
	mov	ebx, 20101h
	mov	ecx, dumpdata
	push	ecx
	xor	edi, edi
	mov	edx, (data_x_pos+12*6)*10000h + dump_y_pos
	cmp	[dumpread], edi
	jz	.hexdumpdone1
.hexdumploop1:
	mcall
	add	edx, 3*6*10000h
	inc	ecx
	inc	edi
	test	edi, 15
	jz	.16
	test	edi, 7
	jnz	@f
	add	edx, 2*6*10000h - 10 + 6*(3*10h+2)*10000h
.16:
	add	edx, 10 - 6*(3*10h+2)*10000h
@@:
	cmp	edi, [dumpread]
	jb	.hexdumploop1
.hexdumpdone1:
	mov	al, 4
	mov	ecx, esi
	mov	ebx, edx
	push	2
	pop	esi
	mov	edx, aQuests
.hexdumploop2:
	cmp	edi, dump_height*10h
	jae	.hexdumpdone2
	mcall
	add	ebx, 3*6*10000h
	inc	edi
	test	edi, 15
	jz	.16x
	test	edi, 7
	jnz	.hexdumploop2
	add	ebx, 2*6*10000h - 10 + 6*(3*10h+2)*10000h
.16x:
	add	ebx, 10 - 6*(3*10h+2)*10000h
	jmp	.hexdumploop2
.hexdumpdone2:
	dec	esi
; colon, minus signs
	mov	ebx, (data_x_pos+8*6)*10000h + dump_y_pos
	mov	edx, aColon
@@:
	mcall
	add	ebx, 10
	cmp	bl, dump_y_pos+dump_height*10
	jb	@b
	mov	ebx, (data_x_pos+(12+3*8)*6)*10000h + dump_y_pos
	mov	edx, aMinus
@@:
	mcall
	add	ebx, 10
	cmp	bl, dump_y_pos+dump_height*10
	jb	@b
; ASCII data
	mov	ebx, (data_x_pos+(12+3*10h+2+2)*6)*10000h + dump_y_pos
	mov	edi, dump_height*10h
	pop	edx
.asciiloop:
	push	edx
	cmp	byte [edx], 20h
	jae	@f
	mov	edx, aPoint
@@:
	mcall
	pop	edx
	inc	edx
	add	ebx, 6*10000h
	dec	edi
	jz	.asciidone
	test	edi, 15
	jnz	.asciiloop
	add	ebx, 10 - 6*10h*10000h
	jmp	.asciiloop
.asciidone:
	ret

redraw_disasm:
	push	13
	pop	eax
	mov	edx, 0xFFFFFF
	mov	ebx, data_x_pos*10000h + data_x_size
	mov	ecx, (disasm_y_pos-1)*10000h + (disasm_y_size+1)
	mcall
draw_disasm:
	mov	eax, [disasm_start_pos]
	mov	[disasm_cur_pos], eax
	and	[disasm_cur_str], 0
.loop:
	mov	eax, [disasm_cur_pos]
	call	find_symbol
	jc	.nosymb
	mov	ebx, [disasm_cur_str]
	imul	ebx, 10
	add	ebx, (data_x_pos+6*2)*10000h + disasm_y_pos
	mov	edx, esi
@@:	lodsb
	test	al, al
	jnz	@b
	mov	byte [esi-1], ':'
	sub	esi, edx
	xor	ecx, ecx
	push	4
	pop	eax
	mcall
	mov	byte [esi+edx-1], 0
	inc	[disasm_cur_str]
	cmp	[disasm_cur_str], disasm_height
	jae	.loopend
.nosymb:
	push	[disasm_cur_pos]
	call	disasm_instr
	pop	ebp
	jc	.loopend
	xor	esi, esi	; default color: black
	mov	ebx, data_x_pos*10000h + data_x_size
	mov	ecx, [disasm_cur_str]
	imul	ecx, 10*10000h
	add	ecx, (disasm_y_pos-1)*10000h + 10
	mov	eax, ebp
	pushad
	call	find_enabled_breakpoint
	popad
	jnz	.nored
	push	13
	pop	eax
	mov	edx, 0xFF0000
	mcall
.nored:
	mov	eax, [_eip]
	cmp	eax, ebp
	jnz	.noblue
	push	13
	pop	eax
	mov	edx, 0x0000FF
	mcall
	mov	esi, 0xFFFFFF	; on blue bgr, use white color
.noblue:
	push	47
	pop	eax
	mov	ebx, 80100h
	mov	edx, [disasm_cur_str]
	imul	edx, 10
	add	edx, data_x_pos*10000h + disasm_y_pos
	mov	ecx, ebp
	mcall
	mov	al, 4
	lea	ebx, [edx+8*6*10000h]
	mov	ecx, esi
	push	1
	pop	esi
	mov	edx, aColon
	mcall
	push	9
	pop	edi
	lea	edx, [ebx+2*6*10000h]
	mov	esi, ecx
	mov	al, 47
	mov	ebx, 20101h
	mov	ecx, ebp
	sub	ecx, [disasm_start_pos]
	add	ecx, disasm_buffer
.drawhex:
	mcall
	add	edx, 6*3*10000h
	inc	ecx
	inc	ebp
	cmp	ebp, [disasm_cur_pos]
	jae	.hexdone
	dec	edi
	jnz	.drawhex
	push	esi
	mov	esi, [disasm_cur_pos]
	dec	esi
	cmp	esi, ebp
	pop	esi
	jbe	.drawhex
	mov	al, 4
	lea	ebx, [edx-6*10000h]
	mov	ecx, esi
	push	3
	pop	esi
	mov	edx, aDots
	mcall
	mov	esi, ecx
.hexdone:
	xor	eax, eax
	mov	edi, disasm_string
	mov	edx, edi
	or	ecx, -1
	repnz	scasb
	not	ecx
	dec	ecx
	xchg	ecx, esi
	mov	ebx, [disasm_cur_str]
	imul	ebx, 10
	add	ebx, (data_x_pos+6*40)*10000h+disasm_y_pos
	mov	al, 4
	mcall
	inc	[disasm_cur_str]
	cmp	[disasm_cur_str], disasm_height
	jb	.loop
.loopend:
	ret

update_disasm_eip:
; test if instruction at eip is showed
	mov	ecx, disasm_height
	mov	eax, [disasm_start_pos]
	mov	[disasm_cur_pos], eax
.l:
	mov	eax, [disasm_cur_pos]
	call	find_symbol
	jc	@f
	dec	ecx
	jz	.m
@@:
	cmp	[_eip], eax
	jz	redraw_disasm
	push	ecx
	call	disasm_instr
	pop	ecx
	jc	.m
	loop	.l
.m:
update_disasm_eip_force:
	mov	eax, [_eip]
	mov	[disasm_start_pos], eax
update_disasm:
	cmp	[debuggee_pid], 0
	jz	.no
	push	69
	pop	eax
	push	6
	pop	ebx
	mov	ecx, [debuggee_pid]
	mov	edi, disasm_buffer
	mov	edx, 256
	mov	esi, [disasm_start_pos]
	mcall
	cmp	eax, -1
	jnz	@f
	mov	esi, read_mem_err
	call	put_message
.no:
	xor	eax, eax
@@:
	mov	[disasm_buf_size], eax
	call	restore_from_breaks
	jmp	redraw_disasm

draw_window:
; start redraw
	push	12
	pop	eax
	push	1
	pop	ebx
	mcall
; define window
	xor	eax, eax
	mov	ebx, wnd_x_size
	mov	ecx, wnd_y_size
	mov	edx, 14FFFFFFh
	mov	edi, caption_str
	mcall
; messages frame
	mov	al, 38
	mov	ebx, (messages_x_pos-2)*10000h + (messages_x_pos+messages_x_size+2)
	push	ebx
	mov	ecx, (messages_y_pos-2)*10001h
	xor	edx, edx
	mcall
	mov	ecx, (messages_y_pos+messages_y_size+2)*10001h
	mcall
	mov	ebx, (messages_x_pos-2)*10001h
	push	ebx
	mov	ecx, (messages_y_pos-2)*10000h + (messages_y_pos+messages_y_size+2)
	mcall
	mov	ebx, (messages_x_pos+messages_x_size+2)*10001h
	push	ebx
	mcall
; command line frame
	mov	ecx, (cmdline_y_pos-2)*10000h + (cmdline_y_pos+cmdline_y_size+2)
	pop	ebx
	mcall
	pop	ebx
	mcall
	pop	ebx
	mov	ecx, (cmdline_y_pos+cmdline_y_size+2)*10001h
	mcall
	mov	ecx, (cmdline_y_pos-2)*10001h
	mcall
; messages
	call	draw_messages
; command line & cursor
	call	draw_cmdline
	call	draw_cursor
; title & registers & dump & disasm
	mov	al, 38
	mov	ebx, (data_x_pos-2)*10001h
	mov	ecx, (title_y_pos+5)*10000h + (messages_y_pos-2)
	mcall
	mov	ebx, (data_x_pos+data_x_size+2)*10001h
	mcall
	mov	ebx, (data_x_pos-2)*10000h + (data_x_pos+data_x_size+2)
	mov	ecx, (dump_y_pos-3)*10001h
	mcall
	mov	ecx, (disasm_y_pos-4)*10001h
	mcall
	call	draw_title
	call	draw_registers
	call	draw_dump
	call	draw_disasm
; end redraw
	push	12
	pop	eax
	push	2
	pop	ebx
	mcall
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DEBUGGING ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OnHelp:
	mov	esi, help_msg
	mov	edi, [curarg]
	cmp	byte [edi], 0
	jz	.x
	mov	esi, help_groups
	call	find_cmd
	jc	.nocmd
	mov	esi, [esi+12]
.x:
	jmp	put_message
.nocmd:
	mov	esi, aUnknownCommand
	jmp	.x

OnQuit:
	push	-1
	pop	eax
	mcall

get_new_context:
	mov	esi, context
	mov	edi, oldcontext
	mov	ecx, 10
	rep	movsd
get_context:
	push	1
	pop	ebx
	push	69
	pop	eax
	mov	ecx, [debuggee_pid]
	mov	esi, context
	push	28h
	pop	edx
	mcall
	ret
set_context:
	push	2
	pop	ebx
	push	69
	pop	eax
	mov	ecx, [debuggee_pid]
	mov	esi, context
	push	28h
	pop	edx
	mcall
	ret

get_dump:
	mov	edi, dumpdata
	mov	esi, [edi-4]
	mov	edx, dump_height*10h
	mov	ecx, edx
	xor	eax, eax
	push	edi
	rep	stosb
	pop	edi
	mov	ecx, [debuggee_pid]
	mov	al, 69
	push	6
	pop	ebx
	mcall
	cmp	eax, -1
	jnz	@f
	mov	esi, read_mem_err
	call	put_message
	xor	eax, eax
@@:
	mov	[edi-8], eax
;	call	restore_from_breaks
;	ret
restore_from_breaks:
; in: edi=buffer,eax=size,esi=address
	mov	ebx, breakpoints
@@:
	test	byte [ebx+4], 1
	jz	.cont		; ignore invalid
	test	byte [ebx+4], 2 or 8
	jnz	.cont		; ignore disabled and memory breaks
	mov	ecx, [ebx]
	sub	ecx, esi
	cmp	ecx, eax
	jae	.cont
	mov	dl, [ebx+5]
	mov	[edi+ecx], dl
.cont:
	add	ebx, 6
	cmp	ebx, breakpoints+breakpoints_n*6
	jb	@b
	ret

OnLoad:
	mov	esi, [curarg]
OnLoadInit:
	mov	edi, loadname
	or	[prgname_len], -1
	mov	[prgname_ptr], edi
.copyname:
	lodsb
	stosb
	inc	[prgname_len]
	cmp	al, '/'
	jnz	@f
	or	[prgname_len], -1
	mov	[prgname_ptr], edi
@@:
	cmp	al, ' '
	ja	.copyname
	mov	byte [edi-1], 0
	and	[load_params], 0
	dec	esi
	call	skip_spaces
	cmp	al, 0
	jz	@f
	mov	[load_params], esi
@@:
	and	[dumppos], 0
	mov	ecx, [symbols]
	jecxz	do_reload
	mcall	68, 13
	and	[symbols], 0
	and	[num_symbols], 0
do_reload:
	push	18
	pop	eax
	push	7
	pop	ebx
	mcall
	mov	[dbgwnd], eax
	xchg	ecx, eax
	push	70
	pop	eax
	mov	ebx, fn70_load_block
	mcall
	test	eax, eax
	jns	.load_ok
.load_err:
	push	eax
	mov	esi, load_err_msg
	call	put_message
	pop	eax
	not	eax
	cmp	eax, 0x20
	jae	.unk_err
	mov	esi, [load_err_msgs+eax*4]
	test	esi, esi
	jnz	put_message
.unk_err:
	mov	esi, unk_err_msg
	inc	eax
	push	eax
	call	put_message_nodraw
	jmp	draw_messages
.load_ok:
	mov	[debuggee_pid], eax
	mov	[bSuspended], 1
	push	ecx
	call	get_context
	mov	edi, oldcontext
	mov	ecx, 10
	rep	movsd
; activate debugger window
	pop	ecx
	mov	bl, 3
	push	18
	pop	eax
	mcall
	call	redraw_title
	call	redraw_registers
	call	get_dump
	call	redraw_dump
	call	update_disasm_eip_force
	mov	esi, load_succ_msg
	push	[debuggee_pid]
	call	put_message_nodraw
	call	draw_messages
; try to load symbols
	mov	esi, loadname
	mov	edi, symbolsfile
	push	edi
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	lea	ecx, [edi-1]
@@:
	dec	edi
	cmp	edi, symbolsfile
	jb	@f
	cmp	byte [edi], '/'
	jz	@f
	cmp	byte [edi], '.'
	jnz	@b
	mov	ecx, edi
@@:
	mov	dword [ecx], '.dbg'
	mov	byte [ecx+4], 0
	pop	esi
	mov	ebp, esi
	call	OnLoadSymbols.silent
; now test for packed progs
	cmp	[disasm_buf_size], 100h
	jz	@f
	ret
@@:
	mov	esi, mxp_nrv_sig
	mov	ebp, disasm_buffer
	mov	edi, ebp
	push	3
	pop	ecx
	repz	cmpsb
	jnz	.not_mxp_nrv
	cmpsb
	mov	cl, mxp_nrv_sig_size-4
	repz	cmpsb
	mov	esi, mxp_nrv_name
	jz	.packed
.not_mxp_nrv:
	mov	esi, mxp_sig
	mov	edi, ebp
	mov	cl, mxp_sig_size
	repz	cmpsb
	mov	esi, mxp_name
	jz	.packed
.not_mxp:
	mov	esi, mxp_lzo_sig1
	mov	edi, ebp
	mov	cl, mxp_lzo_sig1_size
	repz	cmpsb
	mov	esi, mxp_lzo_name
	jz	.packed
	mov	esi, mxp_lzo_sig2
	mov	edi, ebp
	mov	cl, 8
	repz	cmpsb
	jnz	.not_mxp_lzo
	cmpsb
	mov	cl, mxp_lzo_sig2_size - 9
	repz	cmpsb
	mov	esi, mxp_lzo_name
	jz	.packed
.not_mxp_lzo:
	mov	esi, mtappack_name
	cmp	dword [ebp], 0xBF5E246A
	jnz	.not_mtappack
	cmp	dword [ebp+8], 0xEC4E8B57
	jnz	.not_mtappack1
	cmp	dword [ebp+12], 0x8D5EA4F3
	jnz	.not_mtappack1
	cmp	byte [ebp+12h], 0xE9
	jz	.packed
.not_mtappack1:
	cmp	word [ebp+8], 0xB957
	jnz	.not_mtappack
	cmp	dword [ebp+14], 0x575EA4F3
	jnz	.not_mtappack2
	cmp	byte [ebp+17h], 0xE9
	jz	.packed
.not_mtappack2:
	cmp	dword [ebp+14], 0x5F8DA4F3
	jnz	.not_mtappack3
	cmp	word [ebp+18], 0xE9FC
	jz	.packed
.not_mtappack3:
	cmp	word [ebp+14], 0xA4F3
	jnz	.not_mtappack
	cmp	byte [ebp+15h], 0xE9
	jz	.packed
.not_mtappack:
	ret
.packed:
	push	esi
	mov	esi, aPacked1
	call	put_message_nodraw
	pop	esi
	call	put_message_nodraw
	mov	esi, aPacked2
	call	put_message
	call	hide_cursor
	push	40
	pop	eax
	push	7
	pop	ebx
	mcall
.wait:
	push	10
	pop	eax
	mcall
	dec	eax
	jz	.redraw
	dec	eax
	jz	.key
	or	eax, -1
	mcall
.redraw:
	call	draw_window
	call	hide_cursor
	jmp	.wait
.key:
	mov	al, 2
	mcall
	cmp	ah, 'y'
	jz	.yes
	cmp	ah, 'Y'
	jz	.yes
	cmp	ah, 0xD
	jz	.yes
	cmp	ah, 'n'
	jz	.no
	cmp	ah, 'N'
	jnz	.wait
.no:
	push	40
	pop	eax
	mov	ebx, 0x107
	mcall
	call	draw_cursor
	mov	esi, aN_str
	jmp	put_message
.yes:
	push	40
	pop	eax
	mov	ebx, 0x107
	mcall
	call	draw_cursor
	mov	esi, aY_str
	call	put_message
	call	OnUnpack
	ret

mxp_nrv_sig:
	xor	eax, eax
	mov	ecx, 0x95	; 0xA1 for programs with parameters
	mov	[eax], ecx
	add	ecx, [eax+24h]
	push	40h
	pop	esi
	mov	edi, [eax+20h]
	push	edi
	rep	movsb
	jmp	dword [esp]
	pop	esi
	add	esi, [eax]
	xor	edi, edi
mxp_nrv_sig_size = $ - mxp_nrv_sig

mxp_sig:
	mov	ecx, 1CBh
	push	46h
	pop	esi
	mov	edi, [20h]
	rep	movsb
	mov	ecx, [24h]
	rep	movsb
	jmp	dword [20h]
	mov	eax, [20h]
	add	eax, 1CBh
	push	eax
	push	dword [24h]
	push	0
	push	8
	call	$+0x25
mxp_sig_size = $ - mxp_sig

mxp_lzo_sig1:
	xor	eax, eax
	mov	ebp, 0FFh
	mov	ecx, 175h
	mov	[eax], ecx
	add	ecx, [eax+24h]
	push	45h
	pop	esi
	mov	edi, [eax+20h]
	push	edi
	rep	movsb
	jmp	dword [esp]
	pop	ebx
	add	ebx, [eax]
	xor	edi, edi
	cmp	byte [ebx], 11h
	jbe	$+0x1A
mxp_lzo_sig1_size = $ - mxp_lzo_sig1
mxp_lzo_sig2:
	xor	eax, eax
	mov	ebp, 0FFh
	mov	ecx, 188h	; or 177h
	mov	[eax], ecx
	add	ecx, [eax+24h]
	push	44h
	pop	esi
	mov	edi, [eax+20h]
	rep	movsb
	jmp	dword [eax+20h]
	mov	ebx, [eax+20h]
	add	ebx, [eax]
mxp_lzo_sig2_size = $ - mxp_lzo_sig2

OnReload:
	cmp	[debuggee_pid], 0
	jnz	terminate_reload
	mov	esi, need_debuggee
	cmp	byte [loadname], 0
	jnz	do_reload
	jz	put_message
terminate_reload:
	mov	[bReload], 1
OnTerminate:
	mov	ecx, [debuggee_pid]
	push	8
	pop	ebx
	push	69
	pop	eax
	mcall
	ret

AfterSuspend:
	mov	[bSuspended], 1
	call	get_new_context
	call	get_dump
	call	redraw_title
	call	redraw_registers
	call	redraw_dump
	call	update_disasm_eip
	ret

OnSuspend:
	mov	ecx, [debuggee_pid]
	push	4
	pop	ebx
	push	69
	pop	eax
	mcall
	call	AfterSuspend
	mov	esi, aSuspended
	jmp	put_message
DoResume:
	mov	ecx, [debuggee_pid]
	push	5
	pop	ebx
	push	69
	pop	eax
	mcall
	mov	[bSuspended], 0
	ret
OnResume:
	mov	esi, [curarg]
	cmp	byte [esi], 0
	jz	GoOn
	call	calc_expression
	jc	.ret
	mov	eax, ebp
	push	eax
	call	find_enabled_breakpoint
	pop	eax
	jz	GoOn
	mov	bl, 5	; valid enabled one-shot
	call	add_breakpoint
	jnc	GoOn
	mov	esi, aBreakpointLimitExceeded
	call	put_message
.ret:
	ret
GoOn:
; test for enabled breakpoint at eip
	mov	eax, [_eip]
	call	find_enabled_breakpoint
	jnz	.nobreak
; temporarily disable breakpoint, make step, enable breakpoint, continue
	inc	eax
	mov	[temp_break], eax
	mov	[bAfterGo], 1
	dec	eax
	call	disable_breakpoint
	call	get_context
	or	byte [_eflags+1], 1		; set TF
	call	set_context
	and	byte [_eflags+1], not 1
	call	DoResume
	ret
.nobreak:
	call	DoResume
	call	redraw_title
	call	redraw_registers
	call	redraw_dump
	ret
OnDetach:
	mov	ecx, [debuggee_pid]
	push	3
	pop	ebx
	push	69
	pop	eax
	mcall
	and	[debuggee_pid], 0
	call	redraw_title
	call	redraw_registers
	call	redraw_dump
	call	free_symbols
	mov	esi, aContinued
	jmp	put_message

after_go_exception:
	push	eax
	mov	eax, [temp_break]
	dec	eax
	push	esi
	call	enable_breakpoint
; in any case, clear TF and RF
	call	get_new_context
	and	[_eflags], not 10100h		; clear TF,RF
	call	set_context
	xor	edx, edx
	mov	[temp_break], edx
	xchg	dl, [bAfterGo]
	pop	esi
	pop	eax
	cmp	dl, 2
	jnz	@f
	lodsd
	push	esi
	call	get_dump
	jmp	exception.done
@@:	test	eax, eax
	jz	.notint1
; if exception is result of single step, simply ignore it and continue
	test	dword [esi], 0xF
	jnz	dbgmsgstart.5
	lodsd
	push	esi
	mov	esi, oldcontext
	mov	edi, context
	mov	ecx, 28h/4
	rep	movsd
	call	DoResume
	jmp	dbgmsgend
.notint1:
; in other case, work as without temp_break
	lodsd
	push	esi
	push	eax
	jmp	exception.4
.notour:

debugmsg:
	neg	[dbgbufsize]
	mov	esi, dbgbuf
dbgmsgstart:
	lodsd
;	push	eax esi
;	push	dword [esi]
;	mov	esi, dbgmsg_str
;	call	put_message_nodraw
;	pop	esi eax
	add	esi, 4
	dec	eax
	jz	exception
	dec	eax
	jz	terminated
	mov	[bSuspended], 1
	cmp	[bAfterGo], 0
	jnz	after_go_exception
	push	esi
	call	get_new_context
	and	[_eflags], not 10100h		; clear TF,RF
	call	set_context
	pop	esi
.5:
	push	esi
	call	get_dump
	pop	esi
	lodsd
	xor	ecx, ecx
.6:
	bt	eax, ecx
	jnc	.7
	mov	ebx, [drx_break+ecx*4]
	test	ebx, ebx
	jz	.7
	pushad
	dec	ebx
	push	ebx
	mov	esi, aBreakStop
	call	put_message_nodraw
	popad
.7:
	inc	ecx
	cmp	cl, 4
	jb	.6
	push	esi
	jmp	exception.done_draw
terminated:
	push	esi
	mov	esi, terminated_msg
	call	put_message
	and	[debuggee_pid], 0
	and	[temp_break], 0
	mov	[bAfterGo], 0
	xor	eax, eax
	mov	ecx, breakpoints_n*6/4+4
	mov	edi, breakpoints
	rep	stosd
	cmp	[bReload], 1
	sbb	[bReload], -1
	jnz	exception.done
	call	free_symbols
	jmp	exception.done
exception:
	mov	[bSuspended], 1
	cmp	[bAfterGo], 0
	jnz	after_go_exception
	lodsd
	push	esi
	push	eax
	call	get_new_context
	and	[_eflags], not 10100h		; clear TF,RF
	call	set_context
.4:
	call	get_dump
	pop	eax
; int3 command generates exception 0D, #GP
	push	eax
	cmp	al, 0Dh
	jnz	.notdbg
; check for 0xCC byte at eip
	push	0
	push	69
	pop	eax
	push	6
	pop	ebx
	mov	ecx, [debuggee_pid]
	mov	edi, esp
	mov	esi, [_eip]
	push	1
	pop	edx
	mcall
	pop	eax
	cmp	al, 0xCC
	jnz	.notdbg
; this is either dbg breakpoint or int3 cmd in debuggee
	mov	eax, [_eip]
	call	find_enabled_breakpoint
	jnz	.user_int3
; dbg breakpoint; clear if one-shot
	pop	ecx
	push	eax
	mov	esi, aBreakStop
	test	byte [edi+4], 4
	jz	.put_msg_eax
	pop	ecx
	call	clear_breakpoint
	jmp	.done
.user_int3:
	mov	eax, [_eip]
	inc	[_eip]
	pop	ecx
	push	eax
	call	set_context
	mov	esi, aUserBreak
	jmp	.put_msg_eax
.notdbg:
	mov	esi, aException
.put_msg_eax:
	call	put_message_nodraw
.done_draw:
	call	draw_messages
.done:
	push	18
	pop	eax
	push	3
	pop	ebx
	mov	ecx, [dbgwnd]
	mcall	; activate dbg window
	call	redraw_title
	call	redraw_registers
	call	redraw_dump
	call	update_disasm_eip
dbgmsgend:
	pop	esi
	mov	ecx, [dbgbuflen]
	add	ecx, dbgbuf
	cmp	esi, ecx
	jnz	dbgmsgstart
	and	[dbgbuflen], 0
	neg	[dbgbufsize]
	cmp	[bReload], 2
	jnz	@f
	mov	[bReload], 0
	call	do_reload
@@:
	jmp	waitevent

CtrlF7:
	cmp	[debuggee_pid], 0
	jz	.no
	call	OnStep
.no:
	jmp	waitevent
CtrlF8:
	cmp	[debuggee_pid], 0
	jz	CtrlF7.no
	call	OnProceed
	jmp	CtrlF7.no

OnStep:
	cmp	[bSuspended], 0
	jz	.running
	call	get_context
	or	byte [_eflags+1], 1		; set TF
	call	set_context
	and	byte [_eflags+1], not 1
; if instruction at eip is "int xx", set one-shot breakpoint immediately after
	mov	eax, [_eip]
	call	find_enabled_breakpoint
	jnz	@f
	cmp	byte [edi+5], 0xCD
	jz	.int
@@:
	push	0
	push	69
	pop	eax
	push	6
	pop	ebx
	mov	ecx, [debuggee_pid]
	push	3
	pop	edx
	mov	edi, esp
	mov	esi, [_eip]
	mcall
	cmp	eax, edx
	pop	eax
	jnz	.doit
	cmp	al, 0xCD
	jz	.int
	cmp	ax, 0x050F
	jz	.syscall
	cmp	ax, 0x340F
	jz	.sysenter
; resume process
.doit:
	call	GoOn
	cmp	[bAfterGo], 0
	jz	@f
	mov	[bAfterGo], 2
@@:
	ret
.sysenter:	; return address is [ebp-4]
	push	0
	push	69
	pop	eax
	inc	edx	; read 4 bytes
	mov	esi, [_ebp]
	sub	esi, 4
	mcall
	cmp	eax, edx
	pop	eax
	jnz	.syscall
	push	eax
	and	byte [_eflags+1], not 1
	call	set_context
	pop	eax
	jmp	@f
.syscall:
	and	byte [_eflags+1], not 1	; clear TF - avoid system halt (!)
	call	set_context
.int:
	mov	eax, [_eip]
	inc	eax
	inc	eax
@@:
	push	eax
	call	find_enabled_breakpoint
	pop	eax
	jz	.doit
; there is no enabled breakpoint yet; set temporary breakpoint
	mov	bl, 5
	call	add_breakpoint
	jmp	.doit
.running:
	mov	esi, aRunningErr
	jmp	put_message

OnProceed:
	cmp	[bSuspended], 0
	jz	OnStep.running
	mov	esi, [_eip]
@@:
	call	get_byte_nobreak
	jc	OnStep
	inc	esi
; skip prefixes
	call	is_prefix
	jz	@b
	cmp	al, 0xE8	; call
	jnz	@f
	add	esi, 4
	jmp	.doit
@@:	; A4,A5 = movs, A6,A7=cmps
	cmp	al, 0xA4
	jb	@f
	cmp	al, 0xA8
	jb	.doit
@@:	; AA,AB=stos, AC,AD=lods, AE,AF=scas
	cmp	al, 0xAA
	jb	@f
	cmp	al, 0xB0
	jb	.doit
@@:	; E0=loopnz,E1=loopz,E2=loop
	cmp	al, 0xE0
	jb	.noloop
	cmp	al, 0xE2
	ja	.noloop
	inc	esi
	jmp	.doit
.noloop:	; FF /2 = call
	cmp	al, 0xFF
	jnz	OnStep
	call	get_byte_nobreak
	jc	OnStep
	inc	esi
	mov	cl, al
	and	al, 00111000b
	cmp	al, 00010000b
	jnz	OnStep
; skip instruction
	mov	al, cl
	and	eax, 7
	shr	cl, 6
	jz	.mod0
	jp	.doit
	cmp	al, 4
	jnz	@f
	inc	esi
@@:
	inc	esi
	dec	cl
	jz	@f
	add	esi, 3
@@:
	jmp	.doit
.mod0:
	cmp	al, 4
	jnz	@f
	call	get_byte_nobreak
	jc	OnStep
	inc	esi
	and	al, 7
@@:
	cmp	al, 5
	jnz	.doit
	add	esi, 4
.doit:
; insert one-shot breakpoint at esi and resume
	call	get_byte_nobreak
	jc	OnStep
	mov	eax, esi
	call	find_enabled_breakpoint
	jz	.ret
	mov	eax, esi
	mov	bl, 5
	call	add_breakpoint
	jmp	OnStep.doit
.ret:
	ret

get_byte_nobreak:
	mov	eax, esi
	call	find_enabled_breakpoint
	jnz	.nobreak
	mov	al, [edi+5]
	clc
	ret
.nobreak:
	push	69
	pop	eax
	push	6
	pop	ebx
	mov	ecx, [debuggee_pid]
	xor	edx, edx
	push	edx
	inc	edx
	mov	edi, esp
	mcall
	dec	eax
	clc
	jz	@f
	stc
@@:	pop	eax
	ret

is_prefix:
	cmp	al, 0x64	; fs:
	jz	.ret
	cmp	al, 0x65	; gs:
	jz	.ret
	cmp	al, 0x66	; use16/32
	jz	.ret
	cmp	al, 0x67	; addr16/32
	jz	.ret
	cmp	al, 0xF0	; lock
	jz	.ret
	cmp	al, 0xF2	; repnz
	jz	.ret
	cmp	al, 0xF3	; rep(z)
	jz	.ret
	cmp	al, 0x2E	; cs:
	jz	.ret
	cmp	al, 0x36	; ss:
	jz	.ret
	cmp	al, 0x3E	; ds:
	jz	.ret
	cmp	al, 0x26	; es:
.ret:	ret

token_end	equ	1
token_reg	equ	2
token_hex	equ	3
token_add	equ	4
token_sub	equ	5
token_mul	equ	6
token_div	equ	7
token_lp	equ	8
token_rp	equ	9
token_err	equ	-1

is_hex_digit:
	cmp	al, '0'
	jb	.no
	cmp	al, '9'
	jbe	.09
	cmp	al, 'A'
	jb	.no
	cmp	al, 'F'
	jbe	.AF
	cmp	al, 'a'
	jb	.no
	cmp	al, 'f'
	jbe	.af
.no:
	stc
	ret
.09:
	sub	al, '0'
;	clc
	ret
.AF:
	sub	al, 'A'-10
;	clc
	ret
.af:
	sub	al, 'a'-10
;	clc
	ret

find_reg:
	mov	edi, reg_table
.findreg:
	movzx	ecx, byte [edi]
	stc
	jecxz	.regnotfound
	inc	edi
	push	esi edi ecx
@@:
	lodsb
	or	al, 20h
	scasb
	loopz	@b
	pop	ecx edi esi
	lea	edi, [edi+ecx+1]
	jnz	.findreg
	movzx	edi, byte [edi-1]
	add	esi, ecx
.regnotfound:
	ret

expr_get_token:
	lodsb
	cmp	al, 0
	jz	.end_token
	cmp	al, ' '
	jbe	expr_get_token
	cmp	al, '+'
	jz	.add
	cmp	al, '-'
	jz	.sub
	cmp	al, '*'
	jz	.mul
	cmp	al, '/'
	jz	.div
	cmp	al, '('
	jz	.lp
	cmp	al, ')'
	jnz	.notsign
.rp:
	mov	al, token_rp
	ret
.div:
	mov	al, token_div
	ret
.end_token:
	mov	al, token_end
	ret
.add:
	mov	al, token_add
	ret
.sub:
	mov	al, token_sub
	ret
.mul:
	mov	al, token_mul
	ret
.lp:
	mov	al, token_lp
	ret
.notsign:
	dec	esi
	call	find_reg
	jc	.regnotfound
	mov	al, token_reg
	ret
.regnotfound:
; test for symbol
	push	esi
@@:
	lodsb
	cmp	al, ' '
	ja	@b
	push	eax
	mov	byte [esi], 0
	xchg	esi, [esp+4]
	call	find_symbol_name
	mov	edi, eax
	pop	eax
	xchg	esi, [esp]
	mov	byte [esi], al
	jc	@f
	add	esp, 4
	mov	al, token_hex
	ret
@@:
	pop	esi
; test for hex number
	xor	ecx, ecx
	xor	edi, edi
	xor	eax, eax
@@:
	lodsb
	call	is_hex_digit
	jc	@f
	shl	edi, 4
	or	edi, eax
	inc	ecx
	jmp	@b
@@:
	dec	esi
	jecxz	.err
	cmp	ecx, 8
	ja	.err
	mov	al, token_hex
	ret
.err:
	mov	al, token_err
	mov	esi, aParseError
	ret

expr_read2:
	cmp	al, token_hex
	jz	.hex
	cmp	al, token_reg
	jz	.reg
	cmp	al, token_lp
	jz	.lp
	mov	al, token_err
	mov	esi, aParseError
	ret
.hex:
	mov	ebp, edi
.ret:
	jmp	expr_get_token
.reg:
	cmp	edi, 24
	jz	.eip
	sub	edi, 4
	jb	.8lo
	sub	edi, 4
	jb	.8hi
	sub	edi, 8
	jb	.16
	mov	ebp, [_eax+edi*4]
	jmp	.ret
.16:
	movzx	ebp, word [_eax+(edi+8)*4]
	jmp	.ret
.8lo:
	movzx	ebp, byte [_eax+(edi+4)*4]
	jmp	.ret
.8hi:
	movzx	ebp, byte [_eax+(edi+4)*4+1]
	jmp	.ret
.eip:
	mov	ebp, [_eip]
	jmp	.ret
.lp:
	call	expr_get_token
	call	expr_read0
	cmp	al, token_err
	jz	@f
	cmp	al, token_rp
	jz	expr_get_token
	mov	al, token_err
	mov	esi, aParseError
@@:	ret

expr_read1:
	call	expr_read2
.1:
	cmp	al, token_mul
	jz	.mul
	cmp	al, token_div
	jz	.div
	ret
.mul:
	push	ebp
	call	expr_get_token
	call	expr_read2
	pop	edx
; ebp := edx*ebp
	imul	ebp, edx
	jmp	.1
.div:
	push	ebp
	call	expr_get_token
	call	expr_read2
	pop	edx
; ebp := edx/ebp
	test	ebp, ebp
	jz	.div0
	push	eax
	xor	eax, eax
	xchg	eax, edx
	div	ebp
	xchg	eax, ebp
	pop	eax
	jmp	.1
.div0:
	mov	al, token_err
	mov	esi, aDivByZero
	ret

expr_read0:
	xor	ebp, ebp
	cmp	al, token_add
	jz	.add
	cmp	al, token_sub
	jz	.sub
	call	expr_read1
.1:
	cmp	al, token_add
	jz	.add
	cmp	al, token_sub
	jz	.sub
	ret
.add:
	push	ebp
	call	expr_get_token
	call	expr_read1
	pop	edx
; ebp := edx+ebp
	add	ebp, edx
	jmp	.1
.sub:
	push	ebp
	call	expr_get_token
	call	expr_read1
	pop	edx
; ebp := edx-ebp
	xchg	edx, ebp
	sub	ebp, edx
	jmp	.1

calc_expression:
; in: esi->expression
; out: CF=1 if error
;      CF=0 and ebp=value if ok
	call	expr_get_token
	call	expr_read0
	cmp	al, token_end
	jz	.end
	cmp	al, token_err
	jz	@f
	mov	esi, aParseError
@@:
	call	put_message
	stc
	ret
.end:
	clc
	ret

OnCalc:
	mov	esi, [curarg]
	call	calc_expression
	jc	.ret
	push	ebp
	mov	esi, calc_string
	call	put_message_nodraw
	jmp	draw_messages
.ret:
	ret

OnDump:
	mov	esi, [curarg]
	cmp	byte [esi], 0
	jnz	.param
	add	[dumppos], dump_height*10h
	jmp	.doit
.param:
	call	calc_expression
	jc	.ret
	mov	[dumppos], ebp
.doit:
	call	get_dump
	call	redraw_dump
.ret:
	ret

OnUnassemble:
	mov	esi, [curarg]
	cmp	byte [esi], 0
	jnz	.param
	mov	eax, [disasm_start_pos]
	mov	ecx, disasm_height
	mov	[disasm_cur_pos], eax
.l:
	mov	eax, [disasm_cur_pos]
	call	find_symbol
	jc	@f
	dec	ecx
	jz	.m
@@:
	push	ecx
	call	disasm_instr
	pop	ecx
	jc	.err
	loop	.l
.m:
	mov	eax, [disasm_cur_pos]
	jmp	.doit
.param:
	call	calc_expression
	jc	.ret
	mov	eax, ebp
.doit:
	push	eax
	push	[disasm_start_pos]
	mov	[disasm_start_pos], eax
	call	update_disasm
	pop	[disasm_start_pos]
	pop	eax
	cmp	[disasm_cur_str], 0
	jz	@f
	mov	[disasm_start_pos], eax
.ret:
	ret
@@:
	call	update_disasm
.err:
	mov	esi, aInvAddr
	jmp	put_message

OnReg:
	mov	esi, [curarg]
	call	skip_spaces
	call	find_reg
	jnc	@f
.err:
	mov	esi, RSyntax
	jmp	put_message
@@:
	call	skip_spaces
	test	al, al
	jz	.err
	cmp	al, '='
	jnz	@f
	inc	esi
	call	skip_spaces
	test	al, al
	jz	.err
@@:
	push	edi
	call	calc_expression
	pop	edi
	jc	.ret
; now edi=register id, ebp=value
	cmp	[bSuspended], 0
	mov	esi, aRunningErr
	jz	put_message
	xchg	eax, ebp
	cmp	edi, 24
	jz	.eip
	sub	edi, 4
	jb	.8lo
	sub	edi, 4
	jb	.8hi
	sub	edi, 8
	jb	.16
	mov	[_eax+edi*4], eax
	jmp	.ret
.16:
	mov	word [_eax+(edi+8)*4], ax
	jmp	.ret
.8lo:
	mov	byte [_eax+(edi+4)*4], al
	jmp	.ret
.8hi:
	mov	byte [_eax+(edi+4)*4+1], al
	jmp	.ret
.eip:
	mov	[_eip], eax
	call	update_disasm_eip
.ret:
	call	set_context
	jmp	redraw_registers

; Breakpoints manipulation
OnBp:
	mov	esi, [curarg]
	call	calc_expression
	jc	.ret
	xchg	eax, ebp
	push	eax
	call	find_breakpoint
	inc	eax
	pop	eax
	jz	.notfound
	mov	esi, aDuplicateBreakpoint
	jmp	.sayerr
.notfound:
	mov	bl, 1
	call	add_breakpoint
	jnc	.ret
	mov	esi, aBreakpointLimitExceeded
.sayerr:
	call	put_message
.ret:
	jmp	redraw_disasm

OnBpmb:
	mov	dh, 0011b
	jmp	DoBpm
OnBpmw:
	mov	dh, 0111b
	jmp	DoBpm
OnBpmd:
	mov	dh, 1111b
DoBpm:
	mov	esi, [curarg]
	cmp	byte [esi], 'w'
	jnz	@f
	and	dh, not 2
	inc	esi
@@:
	push	edx
	call	calc_expression
	pop	edx
	jnc	@f
	ret
@@:
; ebp=expression, dh=flags
	movzx	eax, dh
	shr	eax, 2
	test	ebp, eax
	jz	@f
	mov	esi, aUnaligned
	jmp	put_message
@@:
	mov	eax, ebp
	mov	bl, 0Bh
	call	add_breakpoint
	jnc	@f
	mov	esi, aBreakpointLimitExceeded
	jmp	put_message
@@:
; now find index
	push	eax
	xor	ecx, ecx
.l1:
	cmp	[drx_break+ecx*4], 0
	jnz	.l2
	push	69
	pop	eax
	push	ecx
	mov	dl, cl
	mov	ecx, [debuggee_pid]
	mov	esi, ebp
	push	9
	pop	ebx
	mcall
	test	eax, eax
	jz	.ok
	pop	ecx
.l2:
	inc	ecx
	cmp	ecx, 4
	jb	.l1
	pop	eax
	call	clear_breakpoint
	mov	esi, aBreakpointLimitExceeded
	jmp	put_message
.ok:
	pop	ecx
	pop	eax
	and	byte [edi], not 2	; breakpoint is enabled
	shl	dl, 6
	or	dl, dh
	mov	byte [edi+1], dl
	inc	eax
	mov	[drx_break+ecx*4], eax
	ret

OnBc:
	mov	esi, [curarg]
@@:	call	get_hex_number
	jc	OnBp.ret
	call	clear_breakpoint
	jmp	@b

OnBd:
	mov	esi, [curarg]
@@:	call	get_hex_number
	jc	OnBp.ret
	call	disable_breakpoint
	jmp	@b

OnBe:
	mov	esi, [curarg]
@@:	call	get_hex_number
	jc	OnBp.ret
	push	eax
	call	find_enabled_breakpoint
	pop	eax
	jz	.err
	call	enable_breakpoint
	jmp	@b
.err:
	mov	esi, OnBeErrMsg
	jmp	put_message

get_hex_number:
	call	skip_spaces
	xor	ecx, ecx
	xor	edx, edx
@@:
	lodsb
	call	is_hex_digit
	jc	.ret
	shl	edx, 4
	or	dl, al
	inc	ecx
	jmp	@b
.ret:
	dec	esi
	cmp	ecx, 1
	xchg	eax, edx
	ret

OnBl:
	mov	esi, [curarg]
	cmp	byte [esi], 0
	jz	.listall
	call	get_hex_number
	jc	.ret
	cmp	eax, breakpoints_n
	jae	.err
	push	eax
	add	eax, eax
	lea	edi, [breakpoints + eax + eax*2]
	pop	eax
	test	byte [edi+4], 1
	jz	.err
	call	show_break_info
.ret:
	ret
.err:
	mov	esi, aInvalidBreak
	jmp	put_message
.listall:
	mov	edi, breakpoints
	xor	eax, eax
@@:
	test	byte [edi+4], 1
	jz	.cont
	push	edi eax
	call	show_break_info
	pop	eax edi
.cont:
	add	edi, 6
	inc	eax
	cmp	eax, breakpoints_n
	jb	@b
	ret

show_break_info:
	push	edi
	test	byte [edi+4], 8
	jnz	.dr
	push	dword [edi]
	push	eax
	mov	esi, aBreakNum
	call	put_message_nodraw
	jmp	.cmn
.dr:
	push	eax
	mov	esi, aMemBreak1
	call	put_message_nodraw
	pop	edi
	push	edi
	mov	esi, aMemBreak2
	test	byte [edi+5], 2
	jz	@f
	mov	esi, aMemBreak3
@@:
	call	put_message_nodraw
	pop	edi
	push	edi
	mov	esi, aMemBreak6
	test	byte [edi+5], 8
	jnz	@f
	mov	esi, aMemBreak5
	test	byte [edi+5], 4
	jnz	@f
	mov	esi, aMemBreak4
@@:
	call	put_message_nodraw
	pop	edi
	push	edi
	push	dword [edi]
	mov	esi, aMemBreak7
	call	put_message_nodraw
.cmn:
	pop	edi
	test	byte [edi+4], 2
	jz	@f
	push	edi
	mov	esi, aDisabled
	call	put_message_nodraw
	pop	edi
@@:
	test	byte [edi+4], 4
	jz	@f
	mov	esi, aOneShot
	call	put_message_nodraw
@@:
	mov	esi, newline
	jmp	put_message

add_breakpoint:
; in: eax=address, bl=flags
; out: CF=1 => error, CF=0 => eax=breakpoint number
	xor	ecx, ecx
	mov	edi, breakpoints
@@:
	test	byte [edi+4], 1
	jz	.found
	add	edi, 6
	inc	ecx
	cmp	ecx, breakpoints_n
	jb	@b
	stc
	ret
.found:
	stosd
	xchg	eax, ecx
	mov	[edi], bl
	test	bl, 2
	jnz	@f
	or	byte [edi], 2
	push	eax
	call	enable_breakpoint
	pop	eax
@@:
	clc
	ret

clear_breakpoint:
	cmp	eax, breakpoints_n
	jae	.ret
	mov	ecx, 4
	inc	eax
.1:
	cmp	[drx_break-4+ecx*4], eax
	jnz	@f
	and	[drx_break-4+ecx*4], 0
@@:	loop	.1
	dec	eax
	push	eax
	add	eax, eax
	lea	edi, [breakpoints + eax + eax*2 + 4]
	test	byte [edi], 1
	pop	eax
	jz	.ret
	push	edi
	call	disable_breakpoint
	pop	edi
	mov	byte [edi], 0
.ret:
	ret

disable_breakpoint:
	cmp	eax, breakpoints_n
	jae	.ret
	add	eax, eax
	lea	edi, [breakpoints + eax + eax*2 + 5]
	test	byte [edi-1], 1
	jz	.ret
	test	byte [edi-1], 2
	jnz	.ret
	or	byte [edi-1], 2
	test	byte [edi-1], 8
	jnz	.dr
	push	esi
	push	7
	pop	ebx
	push	69
	pop	eax
	mov	ecx, [debuggee_pid]
	xor	edx, edx
	inc	edx
	mov	esi, [edi-5]
	mcall
	pop	esi
.ret:
	ret
.dr:
	mov	dl, [edi]
	shr	dl, 6
	mov	dh, 80h
	push	69
	pop	eax
	push	9
	pop	ebx
	mov	ecx, [debuggee_pid]
	mcall
	ret

enable_breakpoint:
	push	esi
	cmp	eax, breakpoints_n
	jae	.ret
	add	eax, eax
	lea	edi, [breakpoints + eax + eax*2 + 5]
	test	byte [edi-1], 1
	jz	.ret
	test	byte [edi-1], 2
	jz	.ret
	and	byte [edi-1], not 2
	test	byte [edi-1], 8
	jnz	.dr
	push	6
	pop	ebx
	push	69
	pop	eax
	mov	esi, [edi-5]
	mov	ecx, [debuggee_pid]
	xor	edx, edx
	inc	edx
	mcall
	dec	eax
	jnz	.err
	mov	al, 69
	push	0xCC
	mov	edi, esp
	inc	ebx
	mcall
	pop	eax
.ret:
	pop	esi
	ret
.err:
	or	byte [edi-1], 2
	mov	esi, aBreakErr
	call	put_message
	pop	esi
	ret
.dr:
	push	9
	pop	ebx
	push	69
	pop	eax
	mov	esi, [edi-5]
	mov	ecx, [debuggee_pid]
	mov	dl, [edi]
	shr	dl, 6
	mov	dh, [edi]
	and	dh, 0xF
	mcall
	test	eax, eax
	jnz	.err
	pop	esi
	ret

find_breakpoint:
	xor	ecx, ecx
	xchg	eax, ecx
	mov	edi, breakpoints
@@:
	test	byte [edi+4], 1
	jz	.cont
	test	byte [edi+4], 8
	jnz	.cont
	cmp	[edi], ecx
	jz	.found
.cont:
	add	edi, 6
	inc	eax
	cmp	eax, breakpoints_n
	jb	@b
	or	eax, -1
.found:
	ret

find_enabled_breakpoint:
	xor	ecx, ecx
	xchg	eax, ecx
	mov	edi, breakpoints
@@:
	test	byte [edi+4], 1
	jz	.cont
	test	byte [edi+4], 2 or 8
	jnz	.cont
	cmp	[edi], ecx
	jz	.found
.cont:
	add	edi, 6
	inc	eax
	cmp	eax, breakpoints_n
	jb	@b
	or	eax, -1
.found:
	ret

OnUnpack:
; program must be loaded - checked when command was parsed
; program must be stopped
	mov	esi, aRunningErr
	cmp	[bSuspended], 0
	jz	put_message
; all breakpoints must be disabled
	mov	edi, breakpoints
@@:
	test	byte [edi+4], 1
	jz	.cont
	test	byte [edi+4], 2
	jnz	.cont
	mov	esi, aEnabledBreakErr
	jmp	put_message
.cont:
	add	edi, 6
	cmp	edi, breakpoints+breakpoints_n*6
	jb	@b
; ok, now do it
; set breakpoint on 0xC dword access
	push	9
	pop	ebx
	mov	ecx, [debuggee_pid]
	mov	dx, 1111b*256
	push	0xC
	pop	esi
@@:
	push	69
	pop	eax
	mcall
	test	eax, eax
	jz	.breakok
	inc	edx
	cmp	dl, 4
	jb	@b
.breakok:
	call	GoOn
; now wait for event
.wait:
	push	10
	pop	eax
	mcall
	dec	eax
	jz	.redraw
	dec	eax
	jz	.key
	dec	eax
	jnz	.debug
; button; we have only one button, close
	or	eax, -1
	mcall
.redraw:
	call	draw_window
	jmp	.wait
.key:
	mov	al, 2
	mcall
	cmp	ah, 3	; Ctrl+C
	jnz	.wait
.userbreak:
	mov	esi, aInterrupted
.x1:
	push	edx esi
	call	put_message
	pop	esi edx
	or	dh, 80h
	push	69
	pop	eax
	push	9
	pop	ebx
	mov	ecx, [debuggee_pid]
	mcall
	cmp	esi, aUnpacked
	jnz	OnSuspend
	jmp	AfterSuspend
.debug:
	cmp	[dbgbuflen], 4*3
	jnz	.notour
	cmp	dword [dbgbuf], 3
	jnz	.notour
	test	byte [dbgbuf+8], 1
	jnz	.our
.notour:
	mov	esi, aInterrupted
	push	edx
	call	put_message
	pop	edx
	or	dh, 80h
	push	69
	pop	eax
	push	9
	pop	ebx
	mov	ecx, [debuggee_pid]
	mcall
	jmp	debugmsg
.our:
	and	[dbgbuflen], 0
	push	edx
	call	get_context
	push	eax
	mov	al, 69
	mov	bl, 6
	mov	ecx, [debuggee_pid]
	mov	edi, esp
	push	4
	pop	edx
	push	0xC
	pop	esi
	mcall
	pop	eax
	pop	edx
	cmp	eax, [_eip]
	jz	.done
	call	DoResume
	jmp	.wait
.done:
	mov	esi, aUnpacked
	jmp	.x1

include 'sort.inc'
compare:
	cmpsd
	jnz	@f
	cmp	esi, edi
@@:	ret
compare2:
	cmpsd
@@:
	cmpsb
	jnz	@f
	cmp	byte [esi], 0
	jnz	@b
	cmp	esi, edi
@@:
	ret

free_symbols:
	mov	ecx, [symbols]
	jecxz	@f
	mcall	68, 13
	and	[symbols], 0
	and	[num_symbols], 0
@@:
	ret

OnLoadSymbols.fileerr:
	test	ebp, ebp
	jz	@f
	mcall	68, 13, edi
	ret
@@:
	push	eax
	mcall	68, 13, edi
	mov	esi, aCannotLoadFile
	call	put_message_nodraw
	pop	eax
	cmp	eax, 0x20
	jae	.unk
	mov	esi, [load_err_msgs + eax*4]
	test	esi, esi
	jnz	put_message
.unk:
	mov	esi, unk_err_msg2
	jmp	put_message

OnLoadSymbols:
	xor	ebp, ebp
; load input file
	mov	esi, [curarg]
	call	free_symbols
.silent:
	xor	edi, edi
	cmp	[num_symbols], edi
	jz	@f
	ret
@@:
	mov	ebx, fn70_attr_block
	mov	[ebx+21], esi
	mcall	70
	test	eax, eax
	jnz	.fileerr
	cmp	dword [fileattr+36], edi
	jnz	.memerr
	mov	ecx, dword [fileattr+32]
	mcall	68, 12
	test	eax, eax
	jz	.memerr
	mov	edi, eax
	mov	ebx, fn70_read_block
	mov	[ebx+12], ecx
	mov	[ebx+16], edi
	mov	[ebx+21], esi
	mcall	70
	test	eax, eax
	jnz	.fileerr
; calculate memory requirements
	lea	edx, [ecx+edi-1]	; edx = EOF-1
	mov	esi, edi
	xor	ecx, ecx
.calcloop:
	cmp	esi, edx
	jae	.calcdone
	cmp	word [esi], '0x'
	jnz	.skipline
	inc	esi
	inc	esi
@@:
	cmp	esi, edx
	jae	.calcdone
	lodsb
	or	al, 20h
	sub	al, '0'
	cmp	al, 9
	jbe	@b
	sub	al, 'a'-'0'-10
	cmp	al, 15
	jbe	@b
	dec	esi
@@:
	cmp	esi, edx
	ja	.calcdone
	lodsb
	cmp	al, 20h
	jz	@b
	jb	.calcloop
	cmp	al, 9
	jz	@b
	add	ecx, 12+1
	inc	[num_symbols]
@@:
	inc	ecx
	cmp	esi, edx
	ja	.calcdone
	lodsb
	cmp	al, 0xD
	jz	.calcloop
	cmp	al, 0xA
	jz	.calcloop
	jmp	@b
.skipline:
	cmp	esi, edx
	jae	.calcdone
	lodsb
	cmp	al, 0xD
	jz	.calcloop
	cmp	al, 0xA
	jz	.calcloop
	jmp	.skipline
.calcdone:
	mcall	68, 12
	test	eax, eax
	jnz	.memok
	inc	ebx
	mov	ecx, edi
	mov	al, 68
	mcall
.memerr:
	mov	esi, aNoMemory
	jmp	put_message
.memok:
	mov	[symbols], eax
	mov	ebx, eax
	push	edi
	mov	esi, edi
	mov	edi, [num_symbols]
	lea	ebp, [eax+edi*4]
	lea	edi, [eax+edi*8]
; parse input data, esi->input, edx->EOF, ebx->ptrs, edi->names
.readloop:
	cmp	esi, edx
	jae	.readdone
	cmp	word [esi], '0x'
	jnz	.readline
	inc	esi
	inc	esi
	xor	eax, eax
	xor	ecx, ecx
@@:
	shl	ecx, 4
	add	ecx, eax
	cmp	esi, edx
	jae	.readdone
	lodsb
	or	al, 20h
	sub	al, '0'
	cmp	al, 9
	jbe	@b
	sub	al, 'a'-'0'-10
	cmp	al, 15
	jbe	@b
	dec	esi
@@:
	cmp	esi, edx
	ja	.readdone
	lodsb
	cmp	al, 20h
	jz	@b
	jb	.readloop
	cmp	al, 9
	jz	@b
	mov	dword [ebx], edi
	add	ebx, 4
	mov	dword [ebp], edi
	add	ebp, 4
	mov	dword [edi], ecx
	add	edi, 4
	stosb
@@:
	xor	eax, eax
	stosb
	cmp	esi, edx
	ja	.readdone
	lodsb
	cmp	al, 0xD
	jz	.readloop
	cmp	al, 0xA
	jz	.readloop
	mov	byte [edi-1], al
	jmp	@b
.readline:
	cmp	esi, edx
	jae	.readdone
	lodsb
	cmp	al, 0xD
	jz	.readloop
	cmp	al, 0xA
	jz	.readloop
	jmp	.readline
.readdone:
	pop	ecx
	mcall	68, 13
	mov	ecx, [num_symbols]
	mov	edx, [symbols]
	mov	ebx, compare
	call	sort
	mov	ecx, [num_symbols]
	lea	edx, [edx+ecx*4]
	mov	ebx, compare2
	call	sort
	mov	esi, aSymbolsLoaded
	call	put_message
	jmp	redraw_disasm

find_symbol:
; in: eax=address
; out: esi, CF
	cmp	[num_symbols], 0
	jnz	@f
.ret0:
	xor	esi, esi
	stc
	ret
@@:
	push	ebx ecx edx
	xor	edx, edx
	mov	esi, [symbols]
	mov	ecx, [num_symbols]
	mov	ebx, [esi]
	cmp	[ebx], eax
	jz	.donez
	jb	@f
	pop	edx ecx ebx
	jmp	.ret0
@@:
; invariant: symbols_addr[edx] < eax < symbols_addr[ecx]
.0:
	push	edx
.1:
	add	edx, ecx
	sar	edx, 1
	cmp	edx, [esp]
	jz	.done2
	mov	ebx, [esi+edx*4]
	cmp	[ebx], eax
	jz	.done
	ja	.2
	mov	[esp], edx
	jmp	.1
.2:
	mov	ecx, edx
	pop	edx
	jmp	.0
.donecont:
	dec	edx
.done:
	test	edx, edx
	jz	@f
	mov	ebx, [esi+edx*4-4]
	cmp	[ebx], eax
	jz	.donecont
@@:
	pop	ecx
.donez:
	mov	esi, [esi+edx*4]
	add	esi, 4
	pop	edx ecx ebx
	clc
	ret
.done2:
	lea	esi, [esi+edx*4]
	pop	ecx edx ecx ebx
	stc
	ret

find_symbol_name:
; in: esi->name
; out: if found: CF clear, eax=value
;      otherwise CF set
	cmp	[num_symbols], 0
	jnz	@f
.stc_ret:
	stc
	ret
@@:
	push	ebx ecx edx edi
	push	-1
	pop	edx
	mov	ebx, [symbols]
	mov	ecx, [num_symbols]
	lea	ebx, [ebx+ecx*4]
; invariant: symbols_name[edx] < name < symbols_name[ecx]
.0:
	push	edx
.1:
	add	edx, ecx
	sar	edx, 1
	cmp	edx, [esp]
	jz	.done2
	call	.cmp
	jz	.done
	jb	.2
	mov	[esp], edx
	jmp	.1
.2:
	mov	ecx, edx
	pop	edx
	jmp	.0
.done:
	pop	ecx
.donez:
	mov	eax, [ebx+edx*4]
	mov	eax, [eax]
	pop	edi edx ecx ebx
	clc
	ret
.done2:
	pop	edx edi edx ecx ebx
	stc
	ret

.cmp:
	mov	edi, [ebx+edx*4]
	push	esi
	add	edi, 4
@@:
	cmpsb
	jnz	@f
	cmp	byte [esi-1], 0
	jnz	@b
@@:
	pop	esi
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DISASSEMBLER ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

disasm_get_byte:
; out: al=byte
	push	ecx
	mov	ecx, [disasm_cur_pos]
	sub	ecx, [disasm_start_pos]
	cmp	ecx, [disasm_buf_size]
	jae	disasm_err
	mov	al, [disasm_buffer+ecx]
	pop	ecx
	inc	[disasm_cur_pos]
	ret
disasm_get_word:
	push	ecx
	mov	ecx, [disasm_cur_pos]
	sub	ecx, [disasm_start_pos]
	inc	ecx
	cmp	ecx, [disasm_buf_size]
	jae	disasm_err
	mov	ax, word [disasm_buffer-1+ecx]
	pop	ecx
	add	[disasm_cur_pos], 2
	ret
disasm_get_dword:
	push	ecx
	mov	ecx, [disasm_cur_pos]
	sub	ecx, [disasm_start_pos]
	add	ecx, 3
	cmp	ecx, [disasm_buf_size]
	jae	disasm_err
	mov	eax, dword [disasm_buffer-3+ecx]
	pop	ecx
	add	[disasm_cur_pos], 4
	ret

disasm_err:
	mov	esp, ebp
stc_ret:
	stc
	ret
disasm_ret:
	mov	esp, ebp
	and	byte [edi], 0
	ret

disasm_instr:
	mov	ebp, esp
	cmp	[debuggee_pid], 0
	jz	stc_ret
	mov	edi, disasm_string
	xor	ecx, ecx
; ecx=flags
disasm_loop1:
	xor	eax, eax
	call	disasm_get_byte
	jmp	dword [disasm_table_1 + eax*4]

cop0:
clock:
csegcs:
csegds:
cseges:
csegss:
csegfs:
cseggs:
	mov	esi, cmd1
iglobal
cmd1:
	db	0x2E,3,'cs:'
	db	0x36,3,'ss:'
	db	0x3E,3,'ds:'
	db	0x26,3,'es:'
	db	0x64,3,'fs:'
	db	0x65,3,'gs:'
	db	0x06,10,'push    es'
	db	0x07,10,'pop     es'
	db	0x0E,10,'push    cs'
	db	0x16,10,'push    ss'
	db	0x17,10,'pop     ss'
	db	0x1E,10,'push    ds'
	db	0x1F,10,'pop     ds'
	db	0x27,3,'daa'
	db	0x2F,3,'das'
	db	0x37,3,'aaa'
	db	0x3F,3,'aas'
	db	0x60,6,0,'pusha'
	db	0x61,5,0,'popa'
	db	0x90,3,'nop'
	db	0x9B,5,'fwait'
	db	0x9C,6,0,'pushf'
	db	0x9D,5,0,'popf'
	db	0x9E,4,'sahf'
	db	0x9F,4,'lahf'
	db	0xA4,5,'movsb'
	db	0xA5,5,0,'movs'
	db	0xA6,5,'cmpsb'
	db	0xA7,5,0,'cmps'
	db	0xAA,5,'stosb'
	db	0xAB,5,0,'stos'
	db	0xAC,5,'lodsb'
	db	0xAD,5,0,'lods'
	db	0xAE,5,'scasb'
	db	0xAF,5,0,'scas'
	db	0xC3,3,'ret'
	db	0xC9,5,'leave'
	db	0xCC,4,'int3'
	db	0xF0,4,'lock'
	db	0xF5,3,'cmc'
	db	0xF8,3,'clc'
	db	0xF9,3,'stc'
	db	0xFA,3,'cli'
	db	0xFB,3,'sti'
	db	0xFC,3,'cld'
	db	0xFD,3,'std'
cmd2:
	db	0x05,7,'syscall'
	db	0x06,4,'clts'
	db	0x31,5,'rdtsc'
	db	0x34,8,'sysenter'
	db	0xA2,5,'cpuid'
	db	0x77,4,'emms'
endg
	jmp	@f
ccpuid:
crdtsc:
cemms:
cop0_F:
	mov	esi, cmd2
@@:
	cmp	al, [esi]
	jz	.found
	inc	esi
	movzx	edx, byte [esi]
	inc	esi
	add	esi, edx
	jmp	@b
.found:
	inc	esi
	lodsb
	cmp	byte [esi], 0
	jz	@f
	movzx	ecx, al
disasm_1:
	rep	movsb
	and	byte [edi], 0
	ret
@@:
	mov	dl, ch
	movzx	ecx, al
	dec	ecx
	inc	esi
	rep	movsb
	test	dl, 1
	mov	al, 'w'
	jnz	@f
	mov	al, 'd'
@@:	stosb
	and	byte [edi], 0
	ret

c67:
	or	ch, 2
	jmp	disasm_loop1
c66:
	or	ch, 1
	jmp	disasm_loop1

cxlat:
cunk:
cerr:
	mov	eax, '???'
	stosd
	clc
	ret

cF:
	call	disasm_get_byte
	jmp	dword [disasm_table_2 + eax*4]

crep:
	push	[disasm_cur_pos]
	call	disasm_get_byte
	cmp	al, 0x0F
	jz	.sse
	mov	dl, al
	mov	eax, 'rep '
	stosd
	mov	al, dl
@@:
	and	eax, not 1
	cmp	al, 0x66
	jnz	@f
	call	disasm_get_byte
	mov	dl, al
	jmp	@b
@@:
	cmp	al, 0xA6
	jz	.repz
	cmp	al, 0xAE
	jz	.repz
	cmp	al, 0xA4
	jz	.prefix
	cmp	al, 0xAA
	jz	.prefix
	cmp	al, 0xAC
	jz	.prefix
	cmp	al, 0x6C
	jz	.prefix
	cmp	al, 0x6E
	jz	.prefix
.noprefix:
	pop	[disasm_cur_pos]
	and	byte [edi-1], 0
	ret
.repz:
	mov	byte [edi-1], 'z'
	mov	al, ' '
	stosb
.prefix:
	pop	[disasm_cur_pos]
	jmp	disasm_loop1
.sse:
	pop	eax
	call	disasm_get_byte
iglobal
rep_sse_cmds:
	db	0x58,3,'add'
	db	0xC2,3,'cmp'
	db	0,0
endg
	mov	esi, rep_sse_cmds+1
@@:
	movzx	edx, byte [esi]
	cmp	al, [esi-1]
	jz	@f
	lea	esi, [esi+edx+2]
	cmp	byte [esi], 0
	jnz	@b
	sub	[disasm_cur_pos], 2
	mov	eax, 'rep'
	stosd
	ret
@@:
	push	ecx
	mov	ecx, edx
	inc	esi
	rep	movsb
	pop	ecx
	mov	al, 's'
	stosb
	jmp	rep_sse_final

crepnz:
	call	disasm_get_byte
	cmp	al, 0x0F
	jz	.sse
	mov	dl, al
	mov	eax, 'repn'
	stosd
	mov	al, 'z'
	stosb
	mov	al, ' '
	stosb
	movzx	eax, dl
	cmp	al, 0x6C
	jb	crep.noprefix
	cmp	al, 0x6F
	jbe	.prefix
	cmp	al, 0xA4
	jb	crep.noprefix
	cmp	al, 0xA7
	jbe	.prefix
	cmp	al, 0xAA
	jb	crep.noprefix
	cmp	al, 0xAF
	ja	crep.noprefix
.prefix:
	jmp	cop0
.sse:
	call	disasm_get_byte
	mov	esi, rep_sse_cmds+1
@@:
	movzx	edx, byte [esi]
	cmp	al, [esi-1]
	jz	.found0
	lea	esi, [esi+edx+2]
	cmp	byte [esi], 0
	jnz	@b
	mov	esi, sse_cmds2+1
@@:
	movzx	edx, byte [esi]
	cmp	al, [esi-1]
	jz	.found1
	lea	esi, [esi+edx+2]
	cmp	byte [esi], 0
	jnz	@b
	sub	[disasm_cur_pos], 2
	mov	eax, 'repn'
	stosd
	mov	al, 'z'
	stosb
	and	byte [edi], 0
	ret
.found0:
	push	ecx
	mov	ecx, edx
	inc	esi
	rep	movsb
	pop	ecx
	mov	al, 's'
	stosb
	mov	al, 'd'
	jmp	rep_sse_final
.found1:
	push	ecx
	mov	ecx, edx
	inc	esi
	rep	movsb
	pop	ecx
	mov	al, 'p'
	stosb
	mov	al, 's'
rep_sse_final:
	stosb
	push	ecx
	push	5
	pop	ecx
	sub	ecx, edx
	adc	ecx, 1
	mov	al, ' '
	rep	stosb
	pop	ecx
	or	ch, 1
	jmp	disasm_mmx1

macro disasm_set_modew
{
	test	al, 1
	jz	@f
	or	ch, 80h
@@:
}

cmov2:
	disasm_set_modew
; mov r/m,i
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	test	al, 00111000b
	jnz	cunk
	mov	eax, 'mov '
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	xor	eax, eax
	test	ch, 80h
	jnz	.1
	call	disasm_get_byte
	jmp	.3
.1:
	test	ch, 1
	jnz	.2
	call	disasm_get_dword
	jmp	.3
.2:
	call	disasm_get_word
.3:
	call	disasm_write_num
	and	byte [edi], 0
	ret

cret2:
	mov	eax, 'ret '
	stosd
	mov	eax, '    '
	stosd
	xor	eax, eax
	jmp	cmov2.2

disasm_write_num:
	push	esi
	cmp	eax, 0x80
	jl	.nosymb
	lea	esi, [eax-1]
	test	eax, esi
	jz	.nosymb
	call	find_symbol
	jc	.nosymb
@@:
	lodsb
	test	al, al
	jz	@f
	stosb
	jmp	@b
@@:
	pop	esi
	ret
.nosymb:
	pop	esi
	push	ecx eax
	inc	edi
@@:
	mov	ecx, eax
	shr	eax, 4
	jz	@f
	inc	edi
	jmp	@b
@@:
	pop	eax
	cmp	ecx, 10
	jb	@f
	inc	edi
@@:
	push	edi eax
@@:
	mov	ecx, eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	dec	edi
	mov	[edi], al
	mov	eax, ecx
	shr	eax, 4
	jnz	@b
	cmp	ecx, 10
	jb	@f
	mov	byte [edi-1], '0'
@@:
	pop	eax edi ecx
	cmp	eax, 10
	jb	@f
	mov	byte [edi], 'h'
	inc	edi
@@:
	ret

iglobal
label disasm_regs32 dword
label disasm_regs dword
	db	'eax',0
	db	'ecx',0
	db	'edx',0
	db	'ebx',0
	db	'esp',0
	db	'ebp',0
	db	'esi',0
	db	'edi',0
disasm_regs16	dw	'ax','cx','dx','bx','sp','bp','si','di'
disasm_regs8	dw	'al','cl','dl','bl','ah','ch','dh','bh'
disasm_scale	db	'1248'
endg
disasm_readrmop:
	call	disasm_get_byte
	test	ch, 40h
	jnz	.skip_size
	push	eax
	and	al, 0xC0
	cmp	al, 0xC0
	pop	eax
	jz	.skip_size
	test	ch, 80h
	jz	.byte
	test	ch, 1
	jnz	.word
	mov	dword [edi], 'dwor'
	mov	byte [edi+4], 'd'
	inc	edi
	jmp	@f
.byte:
	test	ch, 20h
	jz	.qb
	mov	byte [edi], 't'
	inc	edi
.qb:
	mov	dword [edi], 'byte'
	jmp	@f
.word:
	test	ch, 20h
	jz	.qw
	mov	byte [edi], 'q'
	inc	edi
.qw:
	mov	dword [edi], 'word'
@@:
	mov	byte [edi+4], ' '
	add	edi, 5
.skip_size:
	test	ch, 2
	jnz	disasm_readrmop16
	push	ecx
	movzx	ecx, al
	and	eax, 7
	shr	ecx, 6
	jz	.vmod0
	jp	.vmod3
	mov	byte [edi], '['
	inc	edi
	cmp	al, 4
	jz	.sib1
	mov	eax, [disasm_regs+eax*4]
	stosd
	dec	edi
	jmp	@f
.sib1:
	call	.parse_sib
@@:
	mov	al, '+'
	stosb
	dec	ecx
	jz	.vmod1
	call	disasm_get_dword
	jmp	@f
.vmod1:
	call	disasm_get_byte
	movsx	eax, al
@@:
	test	eax, eax
	jns	.2
	neg	eax
	mov	byte [edi-1], '-'
.2:
	call	disasm_write_num
.2a:
	mov	al, ']'
	stosb
	pop	ecx
	ret
.vmod3:
	pop	ecx
	test	ch, 10h
	jnz	.vmod3_mmi
	test	ch, 80h
	jz	.vmod3_byte
	test	ch, 1
	jnz	.vmod3_word
	test	ch, 20h
	jnz	.vmod3_sti
	mov	eax, [disasm_regs32+eax*4]
	stosd
	dec	edi
	ret
.vmod3_byte:
	mov	ax, [disasm_regs8+eax*2]
@@:
	stosw
	ret
.vmod3_word:
	mov	ax, [disasm_regs16+eax*2]
	jmp	@b
.vmod3_sti:
	mov	word [edi], 'st'
	add	al, '0'
	mov	byte [edi+2], al
	add	edi, 3
	ret
.vmod3_mmi:
disasm_write_mmreg = $
	test	ch, 1
	jz	@f
	mov	byte [edi], 'x'
	inc	edi
@@:
	mov	word [edi], 'mm'
	add	al, '0'
	mov	byte [edi+2], al
	add	edi, 3
	ret
.vmod0:
	mov	byte [edi], '['
	inc	edi
	cmp	al, 4
	jz	.sib2
	cmp	al, 5
	jz	.ofs32
	mov	eax, [disasm_regs+eax*4]
	stosd
	mov	byte [edi-1], ']'
	pop	ecx
	ret
.ofs32:
	call	disasm_get_dword
	jmp	.2
.sib2:
	call	.parse_sib
	mov	al, ']'
	stosb
	pop	ecx
	ret
.parse_sib:
	call	disasm_get_byte
	push	edx
	mov	dl, al
	mov	dh, 0
	and	eax, 7
	cmp	al, 5
	jnz	@f
	jecxz	.sib0
@@:
	mov	eax, [disasm_regs+eax*4]
	stosd
	dec	edi
	mov	dh, 1
.sib0:
	mov	al, dl
	shr	eax, 3
	and	eax, 7
	cmp	al, 4
	jz	.sibret
	test	dh, dh
	jz	@f
	mov	byte [edi], '+'
	inc	edi
@@:
	mov	eax, [disasm_regs+eax*4]
	stosd
	dec	edi
	shr	dl, 6
	jz	@f
	mov	al, '*'
	stosb
	movzx	eax, dl
	mov	al, [disasm_scale+eax]
	stosb
@@:
.sibret:
	test	dh, dh
	jnz	.sibret2
	call	disasm_get_dword
	cmp	byte [edi-1], '['
	jz	@f
	mov	byte [edi], '+'
	test	eax, eax
	jns	.sibns
	neg	eax
	mov	byte [edi], '-'
.sibns:
	inc	edi
@@:
	call	disasm_write_num
.sibret2:
	pop	edx
	ret

iglobal
disasm_rm16_1	dd	'bxsi','bxdi','bpsi','bpdi'
disasm_rm16_2	dw	'si','di','bp','bx'
endg
disasm_readrmop16:
	push	ecx
	movzx	ecx, al
	and	eax, 7
	shr	ecx, 6
	jz	.vmod0
	jp	disasm_readrmop.vmod3	; mod=3 is the same in 16- and 32-bit code
; 1 or 2
	mov	byte [edi], '['
	inc	edi
	cmp	al, 4
	jae	@f
	mov	eax, [disasm_rm16_1+eax*4]
	stosw
	mov	al, '+'
	stosb
	shr	eax, 16
	jmp	.1
@@:
	mov	eax, dword [disasm_rm16_2+eax*2-4*2]
.1:
	stosw
	mov	al, '+'
	stosb
	xor	eax, eax
	dec	ecx
	jnz	.2
	call	disasm_get_byte
	cbw
	jmp	@f
.2:
	call	disasm_get_word
@@:
	test	ax, ax
	jns	@f
	mov	byte [edi-1], '-'
	neg	ax
@@:
	call	disasm_write_num
.done1:
	mov	al, ']'
	stosb
	pop	ecx
	ret
.vmod0:
	mov	byte [edi], '['
	inc	edi
	cmp	al, 6
	jz	.ofs16
	cmp	al, 4
	jae	@f
	mov	eax, [disasm_rm16_1+eax*4]
	stosw
	mov	al, '+'
	stosb
	shr	eax, 16
	jmp	.3
@@:
	mov	eax, dword [disasm_rm16_2+eax*2-4*2]
.3:
	stosw
	jmp	.done1
.ofs16:
	xor	eax, eax
	call	disasm_get_word
	call	disasm_write_num
	jmp	.done1

cpush21:
	mov	eax, 'push'
	stosd
	mov	eax, '    '
	stosd
disasm_i32:
	call	disasm_get_dword
	call	disasm_write_num
	and	byte [edi], 0
	ret

cpush22:
	mov	eax, 'push'
	stosd
	mov	eax, '    '
	stosd
	call	disasm_get_byte
	movsx	eax, al
@@:
	call	disasm_write_num
	and	byte [edi], 0
	ret

center:
	mov	eax, 'ente'
	stosd
	mov	eax, 'r   '
	stosd
	xor	eax, eax
	call	disasm_get_word
	call	disasm_write_num
	mov	al, ','
	stosb
	mov	al, ' '
	stosb
	xor	eax, eax
	call	disasm_get_byte
	jmp	@b

cinc1:
; inc reg32
cdec1:
; dec reg32
cpush1:
; push reg32
cpop1:
; pop reg32
cbswap:
; bswap reg32
	mov	edx, eax
	and	edx, 7
	shr	eax, 3
	sub	al, 8
	mov	esi, 'inc '
	jz	@f
	mov	esi, 'dec '
	dec	al
	jz	@f
	mov	esi, 'push'
	dec	al
	jz	@f
	mov	esi, 'pop '
	dec	al
	jz	@f
	mov	esi, 'bswa'
@@:
	xchg	eax, esi
	stosd
	mov	eax, '    '
	jz	@f
	mov	al, 'p'
@@:
	stosd
	xchg	eax, edx
	call	disasm_write_reg1632
	and	byte [edi], 0
	ret

cxchg1:
; xchg eax,reg32
	and	eax, 7
	xchg	eax, edx
	mov	eax, 'xchg'
	stosd
	mov	eax, '    '
	stosd
	xor	eax, eax
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	xchg	eax, edx
	call	disasm_write_reg1632
	and	byte [edi], 0
	ret

cint:
	mov	eax, 'int '
	stosd
	mov	eax, '    '
	stosd
disasm_i8u:
	xor	eax, eax
	call	disasm_get_byte
	call	disasm_write_num
	and	byte [edi], 0
	ret

cmov11:
; mov r8,i8
	mov	ecx, eax
	mov	eax, 'mov '
	stosd
	mov	eax, '    '
	stosd
	and	ecx, 7
	mov	ax, [disasm_regs8+ecx*2]
	stosw
	mov	ax, ', '
	stosw
	jmp	disasm_i8u

cmov12:
; mov r32,i32
	xchg	eax, edx
	mov	eax, 'mov '
	stosd
	mov	eax, '    '
	stosd
	xchg	eax, edx
	and	eax, 7
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	jmp	cmov2.1

iglobal
disasm_shifts	dd	'rol ','ror ','rcl ','rcr ','shl ','shr ','sal ','sar '
endg
cshift2:
; shift r/m,1 = D0/D1
cshift3:
; shift r/m,cl = D2/D3
	disasm_set_modew
	mov	dl, al
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	mov	eax, [disasm_shifts+eax*4]
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	cmp	dl, 0xD2
	jb	.s1
	mov	eax, ', cl'
	stosd
	and	byte [edi], 0
	ret
.s1:
	mov	eax, ', 1'
	stosd
	clc
	ret

cshift1:
; shift r/m,i8 = C0/C1
	disasm_set_modew
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	mov	eax, [disasm_shifts+eax*4]
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	jmp	disasm_i8u

caam:
	mov	eax, 'aam '
	jmp	@f
caad:
	mov	eax, 'aad '
@@:
	stosd
	mov	eax, '    '
	stosd
	xor	eax, eax
	call	disasm_get_byte
	cmp	al, 10
	jz	@f
	call	disasm_write_num
@@:
	and	byte [edi], 0
	ret

cmov3:
; A0: mov al,[ofs32]
; A1: mov ax/eax,[ofs32]
; A2: mov [ofs32],al
; A3: mov [ofs32],ax/eax
	mov	edx, 'mov '
	xchg	eax, edx
	stosd
	mov	eax, '    '
	stosd
	test	dl, 2
	jnz	.1
	call	.write_acc
	mov	ax, ', '
	stosw
	call	.write_ofs32
	jmp	.2
.1:
	call	.write_ofs32
	mov	ax, ', '
	stosw
	call	.write_acc
.2:	and	byte [edi], 0
	ret
.write_acc:
	test	dl, 1
	jz	.8bit
	test	ch, 1
	jnz	.16bit
	mov	eax, 'eax'
	stosd
	dec	edi
	ret
.16bit:
	mov	ax, 'ax'
	stosw
	ret
.8bit:
	mov	ax, 'al'
	stosw
	ret
.write_ofs32:
	mov	al, '['
	stosb
	call	disasm_get_dword
	call	disasm_write_num
	mov	al, ']'
	stosb
	ret

disasm_write_reg:
	test	ch, 80h
	jnz	disasm_write_reg1632
	mov	ax, [disasm_regs8+eax*2]
	stosw
	ret
disasm_write_reg1632:
	test	ch, 1
	jnz	@f
	mov	eax, [disasm_regs32+eax*4]
	stosd
	dec	edi
	ret
@@:
	mov	ax, [disasm_regs16+eax*2]
	stosw
	ret

cmovzx:		; 0F B6/B7
cmovsx:		; 0F BE/BF
	mov	edx, eax
	disasm_set_modew
	mov	eax, 'movz'
	cmp	dl, 0xB8
	jb	@f
	mov	eax, 'movs'
@@:
	stosd
	mov	eax, 'x   '
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	or	ch, 1	; 2nd operand - 8 or 16 bits
	call	disasm_readrmop
	and	byte [edi], 0
	ret

iglobal
disasm_op2cmds	dd 'add ','or  ','adc ','sbb ','and ','sub ','xor ','cmp '
endg
cop21:
	disasm_set_modew
	mov	esi, 'test'
	cmp	al, 0A8h
	jae	@f
	shr	al, 3
	and	eax, 7
	mov	esi, [disasm_op2cmds+eax*4]
@@:
	xchg	eax, esi
	stosd
	mov	eax, '    '
	stosd
	test	ch, 80h
	jnz	.1632
	mov	eax, 'al, '
	stosd
	jmp	disasm_i8u
.1632:
	test	ch, 1
	jnz	.16
	mov	eax, 'eax,'
	stosd
	mov	al, ' '
	stosb
	call	disasm_get_dword
	jmp	.x
.16:
	mov	eax, 'ax, '
	stosd
	xor	eax, eax
	call	disasm_get_word
.x:
	call	disasm_write_num
	and	byte [edi], 0
	ret

carpl:
	xor	edx, edx
	or	ch, 0C1h
	mov	eax, 'arpl'
	jmp	cop22.d2

ccmpxchg:
	xor	edx, edx
	disasm_set_modew
	or	ch, 40h
	mov	eax, 'cmpx'
	stosd
	mov	eax, 'chg '
	jmp	cop22.d1

cbsf:
cbsr:
	or	ch, 80h

cop22:
	disasm_set_modew
	or	ch, 40h
	mov	edx, eax
	mov	esi, 'lea '
	cmp	al, 8Dh
	jz	@f
	mov	esi, 'imul'
	cmp	al, 0xAF
	jz	@f
	mov	esi, 'bsf '
	cmp	al, 0BCh
	jz	@f
	mov	esi, 'bsr '
	cmp	al, 0BDh
	jz	@f
	mov	esi, 'mov '
	cmp	al, 88h
	jae	@f
	mov	esi, 'xchg'
	cmp	al, 86h
	jae	@f
	mov	esi, 'test'
	cmp	al, 84h
	jae	@f
	shr	al, 3
	and	eax, 7
	mov	esi, [disasm_op2cmds+eax*4]
@@:
	xchg	eax, esi
.d2:
	stosd
	mov	eax, '    '
.d1:
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	cmp	dl, 0x8D
	jz	@f
	cmp	dl, 0x86
	jz	@f
	cmp	dl, 0x87
	jz	@f
	cmp	dl, 0xBC
	jz	@f
	cmp	dl, 0xBD
	jz	@f
	test	dl, 2
	jz	.d0
@@:
	call	disasm_write_reg
	mov	ax, ', '
	stosw
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.d0:
	push	eax
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	pop	eax
	call	disasm_write_reg
	and	byte [edi], 0
	ret

cbound:
	mov	edx, eax
	mov	eax, 'boun'
	stosd
	mov	eax, 'd   '
	or	ch, 0xC0
	jmp	cop22.d1

cop23:
	disasm_set_modew
	xchg	eax, edx
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	eax, 3
	and	eax, 7
	mov	eax, [disasm_op2cmds+eax*4]
ctest:
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	test	ch, 80h
	jz	.i8
	cmp	dl, 83h
	jz	.i8
	test	ch, 1
	jnz	.i16
	call	disasm_get_dword
	jmp	.ic
.i8:
	xor	eax, eax
	call	disasm_get_byte
	cmp	dl, 83h
	jnz	.ic
	movsx	eax, al
	jmp	.ic
.i16:
	xor	eax, eax
	call	disasm_get_word
.ic:
	call	disasm_write_num
	and	byte [edi], 0
	ret

cmovcc:
	or	ch, 0C0h
	and	eax, 0xF
	mov	ax, [disasm_jcc_codes + eax*2]
	mov	dword [edi], 'cmov'
	add	edi, 4
	stosw
	mov	ax, '  '
	stosw
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	eax, 3
	and	eax, 7
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	call	disasm_readrmop
	and	byte [edi], 0
	ret

cbtx1:
; btx r/m,i8 = 0F BA
	or	ch, 80h
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	cmp	al, 4
	jb	cunk
	mov	eax, [btx1codes+eax*4-4*4]
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	jmp	disasm_i8u
iglobal
btx1codes	dd	'bt  ','bts ','btr ','btc '
endg
cbtx2:
; btx r/m,r = 0F 101xx011 (A3,AB,B3,BB)
	shr	al, 3
	and	eax, 3
	mov	eax, [btx1codes+eax*4]
	stosd
	mov	eax, '    '
	stosd
	or	ch, 0xC0
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	push	eax
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	pop	eax
	call	disasm_write_reg1632
	and	byte [edi], 0
	ret

csetcc:
	and	eax, 0xF
	mov	ax, [disasm_jcc_codes + eax*2]
	mov	dword [edi], 'setc'
	add	edi, 3
	stosw
	mov	ax, '  '
	stosw
	stosb
	call	disasm_readrmop
	and	byte [edi], 0
	ret

iglobal
disasm_jcc_codes dw 'o ','no','b ','ae','z ','nz','be','a ','s ','ns','p ','np','l ','ge','le','g '
endg
cjcc1:
cjmp2:
	cmp	al, 0xEB
	jz	.1
	and	eax, 0xF
	mov	ax, [disasm_jcc_codes + eax*2]
	jmp	.2
.1:
	mov	ax, 'mp'
.2:
	mov	byte [edi], 'j'
	inc	edi
	stosw
	mov	eax, '    '
	stosb
	stosd
	call	disasm_get_byte
	movsx	eax, al
disasm_rva:
	add	eax, [disasm_cur_pos]
	call	disasm_write_num
	and	byte [edi], 0
	ret

ccall1:
cjmp1:
cjcc2:
	mov	edx, 'call'
	cmp	al, 0xE8
	jz	@f
	mov	edx, 'jmp '
	cmp	al, 0xE9
	jz	@f
	mov	edx, '    '
	and	eax, 0xF
	mov	dx, [disasm_jcc_codes+eax*2]
	shl	edx, 8
	mov	dl, 'j'
@@:
	xchg	eax, edx
	stosd
	mov	eax, '    '
	stosd
	test	ch, 1
	jnz	@f
	call	disasm_get_dword
	jmp	disasm_rva
@@:
	call	disasm_get_word
	add	eax, [disasm_cur_pos]
	and	eax, 0xFFFF
	call	disasm_write_num
	and	byte [edi], 0
	ret

ccallf:
	mov	eax, 'call'
	stosd
	mov	eax, '    '
	stosd
	mov	al, 'd'
	test	ch, 1
	jnz	@f
	mov	al, 'p'
@@:
	stosb
	mov	eax, 'word'
	stosd
	mov	al, ' '
	stosb
	test	ch, 1
	jnz	.1
	call	disasm_get_dword
	jmp	.2
.1:
	xor	eax, eax
	call	disasm_get_word
.2:
	push	eax
	xor	eax, eax
	call	disasm_get_word
	call	disasm_write_num
	mov	al, ':'
	stosb
	pop	eax
	call	disasm_write_num
	and	byte [edi], 0
	ret

iglobal
op11codes	dd	'test',0,'not ','neg ','mul ','imul','div ','idiv'
op12codes	dd	'inc ','dec ','call',0,'jmp ',0,'push',0
endg
cop1:
	disasm_set_modew
	xchg	eax, edx
	call	disasm_get_byte
	movzx	esi, al
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	cmp	dl, 0xFE
	jnz	@f
	cmp	al, 1
	jbe	@f
.0:
	inc	[disasm_cur_pos]
	jmp	cunk
@@:
	and	edx, 8
	add	eax, edx
	cmp	al, 11
	jz	.callfar
	cmp	al, 13
	jz	.jmpfar
	mov	eax, [op11codes+eax*4]
	test	eax, eax
	jz	.0
	cmp	eax, 'test'
	jz	ctest
.2:
	stosd
	mov	eax, '    '
	stosd
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.callfar:
	mov	eax, 'call'
.1:
	cmp	esi, 0xC0
	jae	.0
	stosd
	mov	eax, '    '
	stosd
	mov	eax, 'far '
	stosd
	mov	al, 'd'
	test	ch, 1
	jnz	@f
	mov	al, 'p'
@@:
	stosb
	or	ch, 1
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.jmpfar:
	mov	eax, 'jmp '
	jmp	.1

cpop2:
	or	ch, 80h
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	test	al, 00111000b
	jnz	cunk
	mov	eax, 'pop '
	jmp	cop1.2

cloopnz:
	mov	eax, 'loop'
	stosd
	mov	eax, 'nz  '
	test	ch, 2
	jz	@f
	mov	ah, 'w'
@@:	jmp	cloop.cmn
cloopz:
	mov	eax, 'loop'
	stosd
	mov	eax, 'z   '
	test	ch, 2
	jz	@f
	mov	eax, 'zw  '
@@:	jmp	cloop.cmn

cjcxz:
cloop:
	cmp	al, 0xE2
	jz	.loop
	test	ch, 2
	jnz	.jcxz
	mov	eax, 'jecx'
	stosd
	mov	eax, 'z   '
	jmp	.cmn
.jcxz:
	mov	eax, 'jcxz'
	stosd
	mov	eax, '    '
	jmp	.cmn
.loop:
	mov	eax, 'loop'
	stosd
	mov	eax, '    '
	test	ch, 2
	jz	.cmn
	mov	al, 'w'
.cmn:
	stosd
	call	disasm_get_byte
	movsx	eax, al
	add	eax, [disasm_cur_pos]
	test	ch, 1
	jz	@f
	and	eax, 0xFFFF
@@:
disasm_write_num_done:
	call	disasm_write_num
	and	byte [edi], 0
	ret

cimul1:
; imul r,r/m,i
	or	ch, 80h		; 32bit operation
	xchg	eax, edx
	mov	eax, 'imul'
	stosd
	mov	eax, '    '
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	test	ch, 1
	jnz	.16
	cmp	dl, 0x69
	jz	.op32
	call	disasm_get_byte
	movsx	eax, al
	jmp	disasm_write_num_done
.op32:
	call	disasm_get_dword
	jmp	disasm_write_num_done
.16:
	cmp	dl, 0x69
	jz	.op16
	call	disasm_get_byte
	cbw
	jmp	disasm_write_num_done
.op16:
	xor	eax, eax
	call	disasm_get_word
	jmp	disasm_write_num_done

cshld:
cshrd:
	mov	edx, 'shld'
	test	al, 8
	jz	@f
	mov	edx, 'shrd'
@@:
	xchg	eax, edx
	stosd
	mov	eax, '    '
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	push	eax
	or	ch, 80h
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	pop	eax
	call	disasm_write_reg1632
	mov	ax, ', '
	stosw
	test	dl, 1
	jz	disasm_i8u
	mov	ax, 'cl'
	stosw
	and	byte [edi], 0
	ret

ccbw:
	mov	eax, 'cbw '
	test	ch, 1
	jnz	@f
	mov	eax, 'cwde'
@@:	stosd
	and	byte [edi], 0
	ret
ccwd:
	mov	eax, 'cwd '
	test	ch, 1
	jnz	@b
	mov	eax, 'cdq '
	jmp	@b

ccmpxchg8b:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	cerr
	shr	al, 3
	and	al, 7
	cmp	al, 1
	jnz	cerr
	dec	[disasm_cur_pos]
	mov	eax, 'cmpx'
	stosd
	mov	eax, 'chg8'
	stosd
	mov	al, 'b'
	stosb
	mov	al, ' '
	stosb
	or	ch, 40h
	call	disasm_readrmop
	and	byte [edi], 0
	ret

iglobal
fpuD8	dd	'add ','mul ','com ','comp','sub ','subr','div ','divr'
endg

cD8:
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	push	eax
	shr	al, 3
	and	eax, 7
	mov	byte [edi], 'f'
	inc	edi
	xchg	eax, edx
	mov	eax, [fpuD8+edx*4]
	stosd
	mov	ax, '  '
	stosw
	stosb
	pop	eax
	cmp	dl, 2
	jb	.1
	cmp	dl, 3
	jbe	.2
.1:
	cmp	al, 0xC0
	jb	.2
	mov	eax, 'st0,'
	stosd
	mov	al, ' '
	stosb
.2:
	or	ch, 80h or 20h
	and	ch, not 1
	call	disasm_readrmop
	and	byte [edi], 0
	ret

iglobal
fpuD9_2:
	dq	'fchs    ','fabs    ',0,0,'ftst    ','fxam    ',0,0
	db	'fld1    fldl2t  fldl2e  fldpi   fldlg2  fldln2  fldz    '
	dq	0
	db	'f2xm1   fyl2x   fptan   fpatan  fxtract fprem1  fdecstp fincstp '
	db	'fprem   fyl2xp1 fsqrt   fsincos frndint fscale  fsin    fcos    '
fpuD9_fnop	db	'fnop    '
endg
cD9:
	call	disasm_get_byte
	sub	al, 0xC0
	jae	.l1
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	cmp	al, 7
	jnz	@f
	mov	eax, 'fnst'
	stosd
	mov	eax, 'cw  '
	jmp	.x1
@@:
	cmp	al, 5
	jnz	@f
	mov	eax, 'fldc'
	stosd
	mov	eax, 'w   '
.x1:
	stosd
	or	ch, 0C1h
	jmp	.cmn
@@:
	mov	edx, 'fld '
	test	al, al
	jz	@f
	mov	edx, 'fst '
	cmp	al, 2
	jz	@f
	mov	edx, 'fstp'
	cmp	al, 3
	jnz	cunk
@@:
	xchg	eax, edx
	stosd
	mov	eax, '    '
	stosd
	or	ch, 80h
	and	ch, not 1
.cmn:
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.l1:
	cmp	al, 10h
	jae	.l2
	mov	edx, 'fld '
	cmp	al, 8
	jb	@f
	mov	edx, 'fxch'
@@:
	xchg	eax, edx
	stosd
	mov	eax, '    '
	stosd
	xchg	eax, edx
	and	al, 7
	add	al, '0'
	shl	eax, 16
	mov	ax, 'st'
	stosd
	clc
	ret
.l2:
	cmp	al, 0x10
	jnz	@f
	mov	esi, fpuD9_fnop
	jmp	.l3
@@:
	sub	al, 0x20
	jb	cerr
	lea	esi, [fpuD9_2+eax*8]
	cmp	byte [esi], 0
	jz	cerr
.l3:
	movsd
	movsd
	and	byte [edi-1], 0
	ret

cDA:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	cunk
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	mov	word [edi], 'fi'
	inc	edi
	inc	edi
	mov	eax, [fpuD8+eax*4]
	stosd
	mov	ax, '  '
	stosw
	or	ch, 80h
	and	ch, not 1	; 32-bit operand
	call	disasm_readrmop
	and	byte [edi], 0
	ret

iglobal
fpuDB	dd	'ild ',0,'ist ','istp',0,'ld  ',0,'stp '
endg
cDB:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	.1
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	xchg	eax, edx
	mov	eax, [fpuDB+edx*4]
	test	eax, eax
	jz	cerr
	mov	byte [edi], 'f'
	inc	edi
	stosd
	mov	ax, '  '
	stosw
	stosb
	or	ch, 80h
	and	ch, not 1	; 32-bit operand
	cmp	dl, 4
	jb	@f
	or	ch, 20h
	and	ch, not 80h	; 80-bit operand
@@:
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.1:
	cmp	al, 0xE3
	jnz	cunk
	mov	eax, 'fnin'
	stosd
	mov	eax, 'it'
	stosd
	dec	edi
	ret		; CF cleared

iglobal
fpuDC	dd	'add ','mul ',0,0,'subr','sub ','divr','div '
endg
cDC:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	.1
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	mov	byte [edi], 'f'
	inc	edi
	mov	eax, [fpuD8+eax*4]
	stosd
	mov	ax, '  '
	stosw
	stosb
	or	ch, 0A1h	; qword
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.1:
	mov	dl, al
	shr	al, 3
	and	eax, 7
	mov	eax, [fpuDC+eax*4]
	test	eax, eax
	jz	cerr
	mov	byte [edi], 'f'
	inc	edi
	stosd
	mov	eax, '   s'
	stosd
	mov	al, 't'
	stosb
	and	edx, 7
	lea	eax, [edx+'0']
	stosb
	mov	eax, ', st'
	stosd
	mov	ax, '0'
	stosw
	ret	; CF cleared

iglobal
fpuDD	dd	'fld ',0,'fst ','fstp',0,0,0,0
fpuDD_2	dq	'ffree   ',0,'fst     ','fstp    ','fucom   ','fucomp  ',0,0
endg
cDD:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	.1
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	xchg	eax, edx
	mov	eax, [fpuDD+edx*4]
	test	eax, eax
	jz	cunk
	stosd
	mov	eax, '    '
	stosd
	or	ch, 0A1h	; qword operand
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.1:
	push	eax
	shr	al, 3
	and	eax, 7
	xchg	eax, edx
	mov	eax, dword [fpuDD_2+edx*8]
	test	eax, eax
	jz	cerr
	stosd
	mov	eax, dword [fpuDD_2+4+edx*8]
	stosd
	mov	ax, 'st'
	stosw
	pop	eax
	and	al, 7
	add	al, '0'
	stosb
	and	byte [edi], 0
	ret

iglobal
fpuDE	dd	'add ','mul ',0,0,'subr','sub ','divr','div '
endg
cDE:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	.1
	dec	[disasm_cur_pos]
	mov	word [edi], 'fi'
	inc	edi
	inc	edi
	shr	al, 3
	and	eax, 7
	mov	eax, [fpuD8+eax*4]
	stosd
	mov	ax, '  '
	stosw
	or	ch, 81h		; force 16-bit
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.1:
	push	eax
	shr	al, 3
	and	eax, 7
	xchg	eax, edx
	mov	eax, [fpuDE+edx*4]
	test	eax, eax
	jz	.fcompp
	mov	byte [edi], 'f'
	inc	edi
	stosd
	mov	al, 'p'
	cmp	byte [edi-1], ' '
	jnz	@f
	mov	byte [edi-1], al
	mov	al, ' '
@@:	stosb
	mov	eax, '  st'
	stosd
	pop	eax
	and	al, 7
	add	al, '0'
	stosb
	mov	ax, ', '
	stosw
	mov	eax, 'st0'
	stosd
	ret	; CF cleared
.fcompp:
	pop	eax
	cmp	al, 0xD9
	jnz	cerr
	mov	eax, 'fcom'
	stosd
	mov	ax, 'pp'
	stosw
	and	byte [edi], 0
	ret

iglobal
fpuDF	dd	'ild ',0,'ist ','istp','bld ','ild ','bstp','istp'
endg

cDF:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	.1
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	xchg	eax, edx
	mov	eax, [fpuDF+edx*4]
	test	eax, eax
	jz	cerr
	mov	byte [edi], 'f'
	inc	edi
	stosd
	mov	ax, '  '
	stosw
	stosb
	or	ch, 81h		; force 16-bit operand
	cmp	dl, 4
	jb	@f
	or	ch, 20h
	test	dl, 1
	jnz	@f
	or	ch, 40h
@@:
	call	disasm_readrmop
	and	byte [edi], 0
	ret
.1:
	cmp	al, 0xE0
	jnz	cunk
	mov	eax, 'fnst'
	stosd
	mov	eax, 'sw  '
	stosd
	mov	ax, 'ax'
	stosw
	and	byte [edi], 0
	ret

cmovd1:
	mov	eax, 'movd'
	stosd
	mov	eax, '    '
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	call	disasm_write_mmreg
	mov	ax, ', '
	stosw
	or	ch, 0C0h
	and	ch, not 1
	call	disasm_readrmop
	and	byte [edi], 0
	ret
cmovd2:
	mov	eax, 'movd'
	stosd
	mov	eax, '    '
	stosd
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	push	eax ecx
	or	ch, 0C0h
	and	ch, not 1
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	pop	ecx eax
	call	disasm_write_mmreg
	and	byte [edi], 0
	ret

cmovq1:
	test	ch, 1
	jz	.mm
	mov	eax, 'movd'
	stosd
	mov	eax, 'qa  '
	stosd
	jmp	disasm_mmx1
.mm:
	mov	eax, 'movq'
	stosd
	mov	eax, '    '
	stosd
	jmp	disasm_mmx1
cmovq2:
	test	ch, 1
	jz	.mm
	mov	eax, 'movd'
	stosd
	mov	eax, 'qa  '
	stosd
	jmp	disasm_mmx3
.mm:
	mov	eax, 'movq'
disasm_mmx2:
	stosd
	mov	eax, '    '
	stosd
disasm_mmx3:
	or	ch, 50h
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	push	eax
	call	disasm_readrmop
	mov	ax, ', '
	stosw
	pop	eax
	shr	al, 3
	and	eax, 7
	call	disasm_write_mmreg
	and	byte [edi], 0
	ret

iglobal
mmx_cmds:
	db	0x60,'unpcklbw'
	db	0x61,'unpcklwd'
	db	0x62,'unpckldq'
	db	0x63,'packsswb'
	db	0x64,'pcmpgtb '
	db	0x65,'pcmpgtw '
	db	0x66,'pcmpgtd '
	db	0x67,'packuswb'
	db	0x68,'unpckhbw'
	db	0x69,'unpckhwd'
	db	0x6A,'unpckhdq'
	db	0x6B,'packssdw'
	db	0x74,'pcmpeqb '
	db	0x75,'pcmpeqw '
	db	0x76,'pcmpeqd '
	db	0xD4,'paddq   '
	db	0xD5,'pmullw  '
	db	0xD8,'psubusb '
	db	0xD9,'psubusw '
	db	0xDA,'pminub  '
	db	0xDB,'pand    '
	db	0xDC,'paddusb '
	db	0xDD,'paddusw '
	db	0xDE,'pmaxub  '
	db	0xDF,'pandn   '
	db	0xE0,'pavgb   '
	db	0xE3,'pavgw   '
	db	0xE4,'pmulhuw '
	db	0xE5,'pmulhw  '
	db	0xE8,'psubsb  '
	db	0xE9,'psubsw  '
	db	0xEA,'pminsw  '
	db	0xEB,'por     '
	db	0xEC,'paddsb  '
	db	0xED,'paddsw  '
	db	0xEE,'pmaxsw  '
	db	0xEF,'pxor    '
	db	0xF4,'pmuludq '
	db	0xF5,'pmaddwd '
	db	0xF6,'psadbw  '
	db	0xF8,'psubb   '
	db	0xF9,'psubw   '
	db	0xFA,'psubd   '
	db	0xFB,'psubq   '
	db	0xFC,'paddb   '
	db	0xFD,'paddw   '
	db	0xFE,'paddd   '
endg
cpcmn:
	mov	esi, mmx_cmds
@@:
	cmp	al, [esi]
	jz	@f
	add	esi, 9
	jmp	@b
@@:
	inc	esi
	mov	al, 'p'
	cmp	byte [esi], al
	jz	@f
	stosb
@@:
	movsd
	movsd
	cmp	byte [edi-1], ' '
	jz	@f
	mov	al, ' '
	stosb
@@:

disasm_mmx1:
	or	ch, 50h
	call	disasm_get_byte
	dec	[disasm_cur_pos]
	shr	al, 3
	and	eax, 7
	call	disasm_write_mmreg
	mov	ax, ', '
	stosw
	call	disasm_readrmop
	cmp	word [disasm_string], 'cm'
	jz	.cmp
	and	byte [edi], 0
	ret
.cmp:
	call	disasm_get_byte
	and	eax, 7
	mov	dx, 'eq'
	dec	eax
	js	@f
	mov	dx, 'lt'
	jz	@f
	mov	dh, 'e'
	dec	eax
	jnz	.no2
@@:
	xchg	dx, word [disasm_string+3]
	mov	word [disasm_string+5], dx
	and	byte [edi], 0
	ret
.no2:
	dec	eax
	jnz	@f
	add	edi, 2
	push	edi
	lea	esi, [edi-3]
	lea	ecx, [esi-(disasm_string+8)+2]
	std
	rep	movsb
	cld
	mov	cx, word [esi-3]
	mov	dword [esi-3], 'unor'
	mov	byte [esi+1], 'd'
	mov	word [esi+2], cx
	pop	edi
	and	byte [edi+1], 0
	ret
@@:
	mov	edx, 'neq'
	dec	eax
	jz	@f
	mov	edx, 'nlt'
	dec	eax
	jz	@f
	mov	edx, 'nle'
	dec	eax
	jz	@f
	mov	edx, 'ord'
@@:
	push	edi
	lea	esi, [edi-1]
	lea	ecx, [esi-(disasm_string+8)+2]
	std
	rep	movsb
	cld
	mov	cx, word [esi-3]
	mov	dword [esi-3], edx
	mov	word [esi], cx
	pop	edi
	and	byte [edi+1], 0
	ret

cpsrlw:
	mov	eax, 'psrl'
	jmp	@f
cpsraw:
	mov	eax, 'psra'
	jmp	@f
cpsllw:
	mov	eax, 'psll'
@@:
	stosd
	mov	eax, 'w   '
	stosd
	jmp	disasm_mmx1
cpsrld:
	mov	eax, 'psrl'
	jmp	@f
cpsrad:
	mov	eax, 'psra'
	jmp	@f
cpslld:
	mov	eax, 'psll'
@@:
	stosd
	mov	eax, 'd   '
	stosd
	jmp	disasm_mmx1
cpsrlq:
	mov	eax, 'psrl'
	jmp	@f
cpsllq:
	mov	eax, 'psll'
@@:
	stosd
	mov	eax, 'q   '
	stosd
	jmp	disasm_mmx1

csse1:
iglobal
sse_cmds1:
	db	0x2F,4,'comi'
	db	0x54,3,'and'
	db	0x55,4,'andn'
	db	0x58,3,'add'
	db	0xC2,3,'cmp'
endg
	mov	esi, sse_cmds1+1
.1:
@@:
	movzx	edx, byte [esi]
	cmp	al, [esi-1]
	jz	@f
	lea	esi, [esi+edx+2]
	jmp	@b
@@:
	push	ecx
	mov	ecx, edx
	inc	esi
	rep	movsb
	pop	ecx
	mov	al, 's'
	cmp	byte [edi-1], 'i'
	jz	@f
	mov	al, 'p'
@@:
	stosb
	mov	al, 'd'
	test	ch, 1
	jnz	@f
	mov	al, 's'
@@:
	stosb
	push	ecx
	push	5
	pop	ecx
	sub	ecx, edx
	adc	ecx, 1
	mov	al, ' '
	rep	stosb
	pop	ecx
	or	ch, 1		; force XMM reg
	jmp	disasm_mmx1

csse2:
iglobal
sse_cmds2:
	db	0xD0,6,'addsub'
	db	0,0
endg
	test	ch, 1
	jz	cerr
	mov	esi, sse_cmds2+1
	jmp	csse1.1

cpshift:
	mov	dl, al
	mov	ax, 'ps'
	stosw
	call	disasm_get_byte
	push	eax
	and	al, 0xC0
	cmp	al, 0xC0
	jnz	.pop_cunk
	pop	eax
	push	eax
	shr	al, 3
	and	eax, 7
	cmp	al, 2
	jz	.rl
	cmp	al, 4
	jz	.ra
	cmp	al, 6
	jz	.ll
.pop_cunk:
	pop	eax
	jmp	cunk
.ll:
	mov	ax, 'll'
	jmp	@f
.rl:
	mov	ax, 'rl'
	jmp	@f
.ra:
	cmp	dl, 0x73
	jz	.pop_cunk
	mov	ax, 'ra'
@@:
	stosw
	mov	al, 'w'
	cmp	dl, 0x71
	jz	@f
	mov	al, 'd'
	cmp	dl, 0x72
	jz	@f
	mov	al, 'q'
@@:
	stosb
	mov	ax, '  '
	stosw
	stosb
	pop	eax
	and	eax, 7
	call	disasm_write_mmreg
	mov	ax, ', '
	stosw
	xor	eax, eax
	call	disasm_get_byte
	call	disasm_write_num
	and	byte [edi], 0
	ret

iglobal
grp15c1	dq	'fxsave  ','fxrstor ','ldmxcsr ','stmxcsr ',0,0,0,'clflush '
endg
cgrp15:
	call	disasm_get_byte
	cmp	al, 0xC0
	jae	cunk
	shr	al, 3
	and	eax, 7
	mov	edx, eax
	mov	eax, dword [grp15c1+eax*8]
	test	eax, eax
	jz	cerr
	dec	[disasm_cur_pos]
	stosd
	mov	eax, dword [grp15c1+4+edx*8]
	stosd
	or	ch, 40h
	call	disasm_readrmop
	and	byte [edi], 0
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DATA ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

caption_str db 'Kolibri Debugger',0
caption_len = $ - caption_str
begin_str db	'Kolibri Debugger, version 0.31',10
	db	'Hint: type "help" for help, "quit" for quit'
newline	db	10,0
prompt	db	'> ',0

help_groups:
	dd	aControl, 0, 0, help_control_msg
	db	0
	dd	aData, 0, 0, help_data_msg
	db	0
	dd	aBreakpoints, 0, 0, help_breaks_msg
	db	0
; flags field:
; &1: command may be called without parameters
; &2: command may be called with parameters
; &4: command may be called without loaded program
; &8: command may be called with loaded program
commands:
	dd	_aH, OnHelp, HelpSyntax, HelpHelp
	db	0Fh
	dd	aHelp, OnHelp, HelpSyntax, HelpHelp
	db	0Fh
	dd	aQuit, OnQuit, QuitSyntax, QuitHelp
	db	0Dh
	dd	aLoad, OnLoad, LoadSyntax, LoadHelp
	db	6
	dd	aReload, OnReload, ReloadSyntax, ReloadHelp
	db	0Dh
	dd	aTerminate, OnTerminate, TerminateSyntax, TerminateHelp
	db	9
	dd	aDetach, OnDetach, DetachSyntax, DetachHelp
	db	9
	dd	aSuspend, OnSuspend, SuspendSyntax, SuspendHelp
	db	9
	dd	aResume, OnResume, ResumeSyntax, ResumeHelp
	db	0Bh
	dd	aStep, OnStep, StepSyntax, StepHelp
	db	9
	dd	aProceed, OnProceed, ProceedSyntax, ProceedHelp
	db	9
	dd	aCalc, OnCalc, CalcSyntax, CalcHelp
	db	0Eh
	dd	aDump, OnDump, DumpSyntax, DumpHelp
	db	0Bh
	dd	aUnassemble, OnUnassemble, UnassembleSyntax, UnassembleHelp
	db	0Bh
	dd	aBp, OnBp, BpSyntax, BpHelp
	db	0Ah
	dd	aBpm, OnBpmb, BpmSyntax, BpmHelp
	db	0Ah
	dd	aBpmb, OnBpmb, BpmSyntax, BpmHelp
	db	0Ah
	dd	aBpmw, OnBpmw, BpmSyntax, BpmHelp
	db	0Ah
	dd	aBpmd, OnBpmd, BpmSyntax, BpmHelp
	db	0Ah
	dd	aBl, OnBl, BlSyntax, BlHelp
	db	0Bh
	dd	aBc, OnBc, BcSyntax, BcHelp
	db	0Ah
	dd	aBd, OnBd, BdSyntax, BdHelp
	db	0Ah
	dd	aBe, OnBe, BeSyntax, BeHelp
	db	0Ah
	dd	aReg, OnReg, RSyntax, RHelp
	db	0Ah
	dd	aUnpack, OnUnpack, UnpackSyntax, UnpackHelp
	db	9
	dd	aLoadSymbols, OnLoadSymbols, LoadSymbolsSyntax, LoadSymbolsHelp
	db	0Ah
	dd	0
aHelp	db	5,'help',0
_aH	db	2,'h',0
HelpHelp db	'Help on specified function',10
HelpSyntax db	'Usage: h or help [group | command]',10,0

help_msg db	'List of known command groups:',10
	db	'"help control"     - display list of control commands',10
	db	'"help data"        - display list of commands concerning data',10
	db	'"help breakpoints" - display list of commands concerning breakpoints',10,0
aControl db	8,'control',0
help_control_msg db	'List of control commands:',10
	db	'h = help             - help',10
	db	'quit                 - exit from debugger',10
	db	'load <name> [params] - load program for debugging',10
	db	'reload               - reload debugging program',10
	db	'load-symbols <name>  - load information on symbols for program',10
	db	'terminate            - terminate loaded program',10
	db	'detach               - detach from debugging program',10
	db	'stop                 - suspend execution of debugging program',10
	db	'g [<expression>]     - go on (resume execution of debugging program)',10
	db	's = <Ctrl+F7>        - program step',10
	db	'p = <Ctrl+F8>        - program wide step',10
	db	'unpack               - try to bypass unpacker code (heuristic)',10,0
aData	db	5,'data',0
help_data_msg db	'List of data commands:',10
	db	'? <expression>       - calculate value of expression',10
	db	'd [<expression>]     - dump data at given address',10
	db	'u [<expression>]     - unassemble instructions at given address',10
	db	'r <register> <expression> or',10
	db	'r <register>=<expression> - set register value',10,0
aBreakpoints db 12,'breakpoints',0
help_breaks_msg db	'List of breakpoints commands:',10
	db	'bp <expression>      - set breakpoint on execution',10
	db	'bpm[b|w|d] <type> <expression> - set breakpoint on memory access',10
	db	'bl [<number>]        - breakpoint(s) info',10
	db	'bc <number>...       - clear breakpoint',10
	db	'bd <number>...       - disable breakpoint',10
	db	'be <number>...       - enable breakpoint',10,0

aQuit	db	5,'quit',0
QuitHelp db	'Quit from debugger',10
QuitSyntax db	'Usage: quit',10,0

aLoad	db	5,'load',0
LoadHelp db	'Load program for debugging',10
LoadSyntax db	'Usage: load <program-name> [parameters]',10,0

aReload db	7,'reload',0
ReloadHelp db	'Reload debugging program (restart debug session)',10
ReloadSyntax db	'Usage: reload',10,0

aTerminate db	10,'terminate',0
TerminateHelp db 'Terminate debugged program',10
TerminateSyntax db 'Usage: terminate',10,0

aDetach	db	7,'detach',0
DetachHelp db	'Detach from debugged program',10
DetachSyntax db	'Usage: detach',10,0

aSuspend db	5,'stop',0
SuspendHelp db	'Suspend execution of debugged program',10
SuspendSyntax db 'Usage: stop',10,0

aResume db	2,'g',0
ResumeHelp db	'Go (resume execution of debugged program)',10
ResumeSyntax db	'Usage: g',10
	db	'   or: g <expression> - wait until specified address is reached',10,0

aStep	db	2,'s',0
StepHelp db	'Make step in debugged program',10
StepSyntax db	'Usage: s',10,0

aProceed db	2,'p',0
ProceedHelp db	'Make wide step in debugged program (step over CALL, REPxx, LOOP)',10
ProceedSyntax db 'Usage: p',10,0

aDump	db	2,'d',0
DumpHelp db	'Dump data of debugged program',10
DumpSyntax db	'Usage: d <expression> - dump data at specified address',10
	db	'   or: d              - continue current dump',10,0

aCalc	db	2,'?',0
CalcHelp db	'Calculate value of expression',10
CalcSyntax db	'Usage: ? <expression>',10,0

aUnassemble db	2,'u',0
UnassembleHelp db 'Unassemble',10
UnassembleSyntax:
	db	'Usage: u <expression> - unassemble instructions at specified address',10
	db	'   or: u              - continue current unassemble screen',10,0

aReg	db	2,'r',0
RHelp	db	'Set register value',10
RSyntax:
	db	'Usage: r <register> <expression>',10
	db	'   or: r <register>=<expression> - set value of <register> to <expression>',10,0

aBp	db	3,'bp',0
BpHelp	db	'set BreakPoint on execution',10
BpSyntax db	'Usage: bp <expression>',10,0

aBpm	db	4,'bpm',0
aBpmb	db	5,'bpmb',0
aBpmw	db	5,'bpmw',0
aBpmd	db	5,'bpmd',0
BpmHelp	db	'set BreakPoint on Memory access',10
	db	'Maximum 4 breakpoints of this type are allowed',10
	db	'Note that for this breaks debugger is activated after access',10
BpmSyntax db	'Usage: bpmb [w] <expression>',10
	db	'       bpmw [w] <expression>',10
	db	'       bpmd [w] <expression>',10
	db	'       bpm is synonym for bpmd',10
	db	'"w" means break only on writes (default is on read/write)',10,0

aBl	db	3,'bl',0
BlHelp	db	'Breakpoint List',10
BlSyntax db	'Usage: bl          - list all breakpoints',10
	db	'       bl <number> - display info on particular breakpoint',10,0

aBc	db	3,'bc',0
BcHelp	db	'Breakpoint Clear',10
BcSyntax db	'Usage: bc <number-list>',10
	db	'Examples: bc 2',10
	db	'          bc 1 3 4 A',10,0

aBd	db	3,'bd',0
BdHelp	db	'Breakpoint Disable',10
BdSyntax db	'Usage: bd <number-list>',10
	db	'Examples: bd 2',10
	db	'          bd 1 3 4 A',10,0

aBe	db	3,'be',0
BeHelp	db	'Breakpoint Enable',10
BeSyntax db	'Usage: be <number-list>',10
	db	'Examples: be 2',10
	db	'          be 1 3 4 A',10,0

aUnpack	db	7,'unpack',0
UnpackHelp db	'Try to bypass unpacker code',10
UnpackSyntax db	'Usage: unpack',10,0

aLoadSymbols db	13,'load-symbols',0
LoadSymbolsHelp db 'Load symbolic information for executable',10
LoadSymbolsSyntax db 'Usage: load-symbols <symbols-file-name>',10,0

aUnknownCommand db 'Unknown command',10,0

load_err_msg	db	'Cannot load program. ',0
unk_err_msg	db	'Unknown error code -%4X',10,0
aCannotLoadFile	db	'Cannot load file. ',0
unk_err_msg2	db	'Unknown error code %4X.',10,0
load_err_msgs:
	dd	.1, 0, .3, 0, .5, .6, 0, 0, .9, .A, 0, 0, 0, 0, 0, 0
	dd	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .1E, .1F, .20
.1		db	'HD undefined.',10,0
.3		db	'Unknown FS.',10,0
.5		db	'File not found.',10,0
.6		db	'Unexpected EOF.',10,0
.9		db	'FAT table corrupted.',10,0
.A		db	'Access denied.',10,0
.1E		db	'No memory.',10,0
.1F		db	'Not Menuet/Kolibri executable.',10,0
.20		db	'Too many processes.',10,0
load_succ_msg	db	'Program loaded successfully! PID=%4X. Use "g" to run.',10,0
need_debuggee	db	'No program loaded. Use "load" command.',10,0
aAlreadyLoaded	db	'Program is already loaded. Use "terminate" or "detach" commands',10,0
terminated_msg	db	'Program terminated.',10,0
aException	db	'Debugged program caused an exception %2X. '
aSuspended	db	'Suspended',10,0
aContinued	db	'Continuing',10,0
aRunningErr	db	'Program is running',10,0
read_mem_err	db	'ERROR: cannot read process memory!!!',10,0
aBreakpointLimitExceeded db 'Breakpoint limit exceeded',10,0
aBreakErr	db	'Cannot activate breakpoint, it will be disabled',10,0
aDuplicateBreakpoint db	'Duplicate breakpoint',10,0
aInvalidBreak	db	'Invalid breakpoint number',10,0
OnBeErrMsg	db	'There is already enabled breakpoint on this address',10,0
aBreakNum	db	'%2X: at %8X',0
aMemBreak1	db	'%2X: on ',0
aMemBreak2	db	'read from ',0
aMemBreak3	db	'access of ',0
aMemBreak4	db	'byte',0
aMemBreak5	db	'word',0
aMemBreak6	db	'dword',0
aMemBreak7	db	' at %8X',0
aOneShot	db	', one-shot',0
aDisabled	db	', disabled',0
aBreakStop	db	'Breakpoint #%2X',10,0
aUserBreak	db	'int3 command at %8X',10,0
;dbgmsg_str	db	'Debug message for process %4X.',10,0
aInvAddr	db	'Invalid address',10,0
NoPrgLoaded_str	db	'No program loaded'
NoPrgLoaded_len = $ - NoPrgLoaded_str
aRunning	db	'Running'
aPaused		db	'Paused'
aPoint		db	0x1C
aMinus		db	'-'
aColon		db	':'
aQuests		db	'??'
aDots		db	'...'
aParseError	db	'Parse error',10,0
aDivByZero	db	'Division by 0',10,0
calc_string	db	'%8X',10,0
aNoMemory	db	'No memory',10,0
aSymbolsLoaded	db	'Symbols loaded',10,0
aUnaligned	db	'Unaligned address',10,0
aEnabledBreakErr db	'Enabled breakpoints are not allowed',10,0
aInterrupted	db	'Interrupted',10,0
aUnpacked	db	'Unpacked successful!',10,0
aPacked1	db	'Program is probably packed with ',0
aPacked2	db	'.',10,'Try to unpack automatically? [y/n]: ',0
aY_str		db	'y',10,0
aN_str		db	'n',10,0
mxp_nrv_name	db	'mxp_nrv',0
mxp_name	db	'mxp',0
mxp_lzo_name	db	'mxp_lzo',0
mtappack_name	db	'mtappack',0
flags		db	'CPAZSDO'
flags_bits	db	0,2,4,6,7,10,11
regs_strs:
	db	'EAX='
	db	'EBX='
	db	'ECX='
	db	'EDX='
	db	'ESI='
	db	'EDI='
	db	'EBP='
	db	'ESP='
	db	'EIP='
	db	'EFLAGS='

debuggee_pid	dd	0
bSuspended	db	0
bAfterGo	db	0
temp_break	dd	0

disasm_table_1:
	dd	cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0		; 0x
	dd	cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cF
	dd	cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0		; 1x
	dd	cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0
	dd	cop22, cop22, cop22, cop22, cop21, cop21, cseges,cop0		; 2x
	dd	cop22, cop22, cop22, cop22, cop21, cop21, csegcs,cop0
	dd	cop22, cop22, cop22, cop22, cop21, cop21, csegss,cop0		; 3x
	dd	cop22, cop22, cop22, cop22, cop21, cop21, csegds,cop0
	dd	cinc1, cinc1, cinc1, cinc1, cinc1, cinc1, cinc1, cinc1		; 4x
	dd	cdec1, cdec1, cdec1, cdec1, cdec1, cdec1, cdec1, cdec1
	dd	cpush1,cpush1,cpush1,cpush1,cpush1,cpush1,cpush1,cpush1		; 5x
	dd	cpop1, cpop1, cpop1, cpop1, cpop1, cpop1, cpop1, cpop1
	dd	cop0,  cop0,  cbound,carpl, csegfs,cseggs,c66,   c67		; 6x
	dd	cpush21,cimul1,cpush22,cimul1,cunk,cunk,  cunk,  cunk
	dd	cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1		; 7x
	dd	cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1
	dd	cop23, cop23, cop23, cop23, cop22, cop22, cop22, cop22		; 8x
	dd	cop22, cop22, cop22, cop22, cunk,  cop22, cunk,  cpop2
	dd	cop0,  cxchg1,cxchg1,cxchg1,cxchg1,cxchg1,cxchg1,cxchg1		; 9x
	dd	ccbw,  ccwd,  ccallf,cop0,  cop0,  cop0,  cop0,  cop0
	dd	cmov3, cmov3, cmov3, cmov3, cop0,  cop0,  cop0,  cop0		; Ax
	dd	cop21, cop21, cop0,  cop0,  cop0,  cop0,  cop0,  cop0
	dd	cmov11,cmov11,cmov11,cmov11,cmov11,cmov11,cmov11,cmov11		; Bx
	dd	cmov12,cmov12,cmov12,cmov12,cmov12,cmov12,cmov12,cmov12
	dd	cshift1,cshift1,cret2,cop0, cunk,  cunk,  cmov2, cmov2		; Cx
	dd	center,cop0,  cunk,  cunk,  cop0,  cint,  cunk,  cunk
	dd	cshift2,cshift2,cshift3,cshift3,caam,caad,cunk,  cxlat		; Dx
	dd	cD8,   cD9,   cDA,   cDB,   cDC,   cDD,   cDE,   cDF
	dd	cloopnz,cloopz,cloop,cjcxz, cunk,  cunk,  cunk,  cunk		; Ex
	dd	ccall1,cjmp1, cunk,  cjmp2, cunk,  cunk,  cunk,  cunk
	dd	clock, cunk,  crepnz,crep,  cunk,  cop0,  cop1,  cop1		; Fx
	dd	cop0,  cop0,  cop0,  cop0,  cop0,  cop0,  cop1,  cop1

disasm_table_2:
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cop0_F,cop0_F,cunk		; 0x
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk		; 1x
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk		; 2x
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  csse1
	dd	cunk,  crdtsc,cunk,  cunk,  cop0_F,cunk,  cunk,  cunk		; 3x
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
	dd	cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc		; 4x
	dd	cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc
	dd	cunk,  cunk,  cunk,  cunk,  csse1, csse1, cunk,  cunk		; 5x
	dd	csse1, cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
	dd	cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn		; 6x
	dd	cpcmn, cpcmn, cpcmn, cpcmn, cunk,  cunk,  cmovd1,cmovq1
	dd	cunk,  cpshift,cpshift,cpshift,cpcmn,cpcmn,cpcmn,cemms		; 7x
	dd	cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cmovd2,cmovq2
	dd	cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2		; 8x
	dd	cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2
	dd	csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc		; 9x
	dd	csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc
	dd	cunk,  cunk,  ccpuid,cbtx2, cshld, cshld, cunk,  cunk		; Ax
	dd	cunk,  cunk,  cunk,  cbtx2, cshrd, cshrd, cgrp15,cop22
	dd	ccmpxchg,ccmpxchg,cunk,cbtx2,cunk, cunk,  cmovzx,cmovzx		; Bx
	dd	cunk,  cunk,  cbtx1, cbtx2, cbsf,  cbsr,  cmovsx,cmovsx
	dd	cunk,  cunk,  csse1, cunk,  cunk,  cunk,  cunk,  ccmpxchg8b	; Cx
	dd	cbswap,cbswap,cbswap,cbswap,cbswap,cbswap,cbswap,cbswap
	dd	csse2, cpsrlw,cpsrlw,cpsrlq,cpcmn, cpcmn, cunk,  cunk		; Dx
	dd	cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn
	dd	cpcmn, cpsraw,cpsrad,cpcmn, cpcmn, cpcmn, cunk,  cunk		; Ex
	dd	cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn
	dd	cunk,  cpsllw,cpslld,cpsllq,cpcmn, cpcmn, cpcmn, cunk		; Fx
	dd	cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cunk

reg_table:
	db	2,'al',0
	db	2,'cl',1
	db	2,'dl',2
	db	2,'bl',3
	db	2,'ah',4
	db	2,'ch',5
	db	2,'dh',6
	db	2,'bh',7
	db	2,'ax',8
	db	2,'cx',9
	db	2,'dx',10
	db	2,'bx',11
	db	2,'sp',12
	db	2,'bp',13
	db	2,'si',14
	db	2,'di',15
	db	3,'eax',16
	db	3,'ecx',17
	db	3,'edx',18
	db	3,'ebx',19
	db	3,'esp',20
	db	3,'ebp',21
	db	3,'esi',22
	db	3,'edi',23
	db	3,'eip',24
	db	0

IncludeIGlobals

fn70_read_block:
	dd	0
	dq	0
	dd	?
	dd	?
	db	0
	dd	?

fn70_attr_block:
	dd	5
	dd	0,0,0
	dd	fileattr
	db	0
	dd	?

fn70_load_block:
	dd	7
	dd	1
load_params dd	0
	dd	0
	dd	0
i_end:
loadname:
	db	0
	rb	255

symbolsfile	rb	260

prgname_ptr dd ?
prgname_len dd ?

IncludeUGlobals

dbgwnd		dd	?

messages	rb	messages_height*messages_width
messages_pos	dd	?

cmdline		rb	cmdline_width+1
cmdline_len	dd	?
cmdline_pos	dd	?
curarg		dd	?

was_temp_break	db	?

dbgbufsize	dd	?
dbgbuflen	dd	?
dbgbuf		rb	256

fileattr	rb	40

needzerostart:

context:
_eip	dd	?
_eflags	dd	?
_eax	dd	?
_ecx	dd	?
_edx	dd	?
_ebx	dd	?
_esp	dd	?
_ebp	dd	?
_esi	dd	?
_edi	dd	?

oldcontext rb $-context

dumpread dd	?
dumppos dd	?
dumpdata rb	dump_height*10h

; breakpoint structure:
; dword +0: address
; byte +4: flags
; bit 0: 1 <=> breakpoint valid
; bit 1: 1 <=> breakpoint disabled
; bit 2: 1 <=> one-shot breakpoint
; bit 3: 1 <=> DRx breakpoint
; byte +5: overwritten byte
;          for DRx breaks: flags + (index shl 6)
breakpoints_n = 256
breakpoints	rb	breakpoints_n*6
drx_break	rd	4

disasm_buf_size		dd	?

symbols		dd	?
num_symbols	dd	?

bReload			db	?

needzeroend:

disasm_buffer		rb	256
disasm_start_pos	dd	?
disasm_cur_pos		dd	?
disasm_cur_str		dd	?
disasm_string		rb	256

i_param		rb	256

; stack
	align	400h
	rb	400h
used_mem:
