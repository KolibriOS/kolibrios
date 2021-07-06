virtual at 0
editor_data:
	.hPlugin	dd	?
	.hFile		dd	?
if (.hPlugin <> viewer_data.hPlugin) | (.hFile <> viewer_data.hFile)
error in viewer_IsHandleUsed/editor_IsHandleUsed
end if
	.memsize	dd	?
	.encoding	db	?
	.flags		db	?	; & 0x80: modified
					; & 0x40: file is locked
					; & 0x20: next key as a symbol
					; & 0x10: replace mode (vs insert)
	.eol		db	?
			rb	1
	.first_block	dd	?
	.last_block	dd	?
	.numfree	dd	?
	.freeblocks	dd	?
	.numlines	dd	?
	.curline	dd	?
	.curcol		dd	?
	;.curpos_block	dd	?
	;.curpos_offs	dd	?
	.cur_block	dd	?
	.cur_offs	dd	?
	.cur_delta	dd	?
	.cursor_x	dd	?
	.cursor_y	dd	?
	align 200h
	.filename	rb	1024
	.hostname	rb	1024
if (.filename <> viewer_data.filename) | (.hostname <> viewer_data.hostname)
error in viewer_getname/editor_getname
end if
	.buf		rb	16384	; all I/O operations use this buffer
	.basesize = $
	.linedata_start:
end virtual

virtual at 0
editor_line:
	.block		dd	?
	.offs		dw	?
	;.length		dd	?
	.plugdata:
end virtual

; when a file is loaded into the editor, this file is fragmented to blocks
; each block has RESERVE_IN_BLOCK empty bytes to allow quick inserting
; must be dword-aligned!
edit.RESERVE_IN_BLOCK = 16
edit.eol_dos = 1	; DOS/Win EOLn style (0D 0A)
edit.eol_unix = 2	; Unix EOLn style (0A)
edit.eol_mac = 3	; MacOS EOLn style (0D)

virtual at 0
edit_block_header:
	.next		dd	?
	.prev		dd	?
	.limit		dd	?
	.size = $	; must be dword-aligned
end virtual

edit_file:
	mov	eax, [ebp + PanelData.files]
	mov	ecx, [eax+ecx*4]
	test	byte [ecx], 10h
	jz	.file
	ret
.file:
; calculate required memory size
	cmp	dword [ecx+36], 0
	jnz	.nomemory
; block size = 4096
; block header: edit_block_header.size bytes
; some plugin-specific data can follow
; reserve RESERVE_IN_BLOCK free bytes in the end of block
	mov	ebx, 4096
	mov	eax, [EditPlugInfo]
	add	eax, edit_block_header.size
	mov	[EditBlockStart], eax
	sub	ebx, eax
	sub	ebx, edit.RESERVE_IN_BLOCK
	mov	[EditBlockSize], ebx
; now ebx = size of file data in each block
	mov	eax, [ecx+32]
; if eax == 0, set eax = 1
	sub	eax, 1
	adc	eax, 1
	xor	edx, edx
	div	ebx
	sub	edx, 1
	sbb	eax, -1
	add	eax, [EditDataSize]
; eax = number of blocks + memory for editor_data structure
	cmp	eax, 0x80000000 shr 12
	jb	.memok
.nomemory:
	push	aEditNoMemory
	mov	eax, esp
	push	ContinueBtn
	push	1
	push	eax
	push	1
	call	SayErr
	pop	eax
	ret
.memok:
	lea	esi, [ebp + PanelData.dir]
	push	eax
	push	ecx
	mov	ecx, eax
	shl	ecx, 12
	mov	edx, editor_vtable
	call	new_screen
	pop	ecx
	pop	ebx
	test	eax, eax
	jnz	@f
	ret
@@:
	mov	[ebp + editor_data.memsize], ebx
	mov	al, [EditEOLStyle]
	mov	[ebp + editor_data.eol], al
	mov	eax, dword [esi + panel1.hPlugin - panel1.dir]
	mov	[ebp + editor_data.hPlugin], eax
	test	eax, eax
	jz	.nocopyhostname
	lea	edi, [ebp + editor_data.hostname]
	push	esi
	mov	eax, dword [esi + panel1.parents - panel1.dir]
	mov	esi, dword [esi + panel1.parents_sz - panel1.dir]
	add	esi, eax
@@:
	dec	esi
	cmp	byte [esi-1], 0
	jz	@f
	cmp	byte [esi-1], '/'
	jnz	@b
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	pop	esi
.nocopyhostname:
	mov	eax, dword [esi + panel1.hFile - panel1.dir]
	mov	[ebp + editor_data.hFile], eax
	mov	[ebp + editor_data.encoding], encodings.cp866
	xor	eax, eax
	mov	[ebp + editor_data.flags], al
	inc	eax
	mov	[ebp + editor_data.numlines], eax
	lea	edi, [ebp + editor_data.filename]
	mov	ebx, readinfo
	mov	[ebx+21], edi
@@:
	lodsb
	test	al, al
	jz	@f
	stosb
	jmp	@b
@@:
	lea	esi, [ecx+40]
	mov	al, '/'
	cmp	byte [edi-1], al
	jz	@f
	stosb
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
; load file into memory
	mov	esi, [ebp + editor_data.memsize]
	mov	edi, [EditDataSize]
	sub	esi, edi	; esi = number of blocks
	shl	edi, 12
	;mov	[ebp + editor_data.curpos_block], edi
	mov	[ebp + editor_data.first_block], edi
	mov	[ebp + editor_data.cur_block], edi
	add	edi, ebp	; edi -> first block
	mov	[ebp + editor_data.linedata_start + editor_line.block], edi
	xor	eax, eax
	mov	[ebx+4], eax
	mov	[ebx+8], eax
	mov	dword [ebx+12], 16384
	lea	eax, [ebp + editor_data.buf]
	mov	[ebx+16], eax
	mov	edx, [ebp + editor_data.hPlugin]
	test	edx, edx
	jz	@f
	pushad
	push	O_READ
	push	dword [ebx+21]
	push	[ebp + editor_data.hFile]
	call	[edx + PluginInfo.open]
	mov	[esp+1Ch], eax
	popad
	test	eax, eax
	jz	..openerr_in_screen
	mov	ebx, eax
@@:
	mov	ecx, [EditBlockSize]	; bytes rest in the current block
	add	edi, [EditBlockStart]
.readloop:
	mov	edx, [ebp + editor_data.hPlugin]
	test	edx, edx
	jz	.readnative
	pushad
	push	16384
	push	[readinfo.data]
	push	ebx
	call	[edx + PluginInfo.read]
	mov	[esp+1Ch], eax
	popad
	cmp	eax, -1
	jnz	.readok
; let us hope that plugin says error itself
	push	ebp
	push	ebx
	call	[edx + PluginInfo.close]
	pop	ebp
	jmp	.readfailed
.readnative:
	push	ebx
	push	70
	pop	eax
	int	40h
	mov	edx, ebx
	xchg	eax, edx
	pop	ebx
	add	dword [ebx+4], eax
	adc	dword [ebx+8], 0
	test	edx, edx
	jz	.readok
	cmp	edx, 6
	jz	.readok
	push	dword [ebx+21]
	push	aCannotReadFile
	xchg	eax, edx
	call	get_error_msg
	push	eax
	mov	eax, esp
	push	RetryOrCancelBtn
	push	2
	push	eax
	push	3
	call	SayErr
	add	esp, 3*4
	test	eax, eax
	jz	.readnative
.readfailed:
	jmp	delete_active_screen
.readok:
; eax = number of bytes read
	test	eax, eax
	jnz	@f
	push	edi
	sub	edi, ebp
	cmp	edi, [ebp + editor_data.first_block]
	pop	edi
	jnz	.readdone
@@:
	push	eax ebx
	mov	ebx, [readinfo.data]
.loadloop:
	test	ecx, ecx
	jnz	.hasplace
	push	eax
	dec	esi
	jns	.hasblock
	add	[ebp + editor_data.memsize], 8
	add	esi, 8
	mov	ecx, [ebp + editor_data.memsize]
	cmp	ecx, 80000000h shr 12
	jb	@f
.nomemory2:
	pop	eax ebx eax
	call	.nomemory
	jmp	.readfailed
@@:
	sub	edi, ebp
	shl	ecx, 12
	mov	edx, ebp
	call	xpgrealloc
	test	eax, eax
	jz	.nomemory2
	add	edi, eax
	xchg	ebp, eax
.hasblock:
	push	edi
	and	edi, not 0xFFF
	lea	eax, [edi + 0x1000]
	sub	eax, ebp
	stosd	; edit_block_header.next
	sub	eax, 0x2000
	stosd	; edit_block_header.prev
	pop	eax
	sub	eax, edi
	add	eax, 8
	stosd	; edit_block_header.limit
	mov	ecx, [EditPlugInfo]
	inc	ecx
	jz	@f
	dec	ecx
@@:
	xor	eax, eax
	rep	stosb	; info for plugins: zeroed
	add	edi, 0x1000
	mov	ecx, [EditBlockSize]
	pop	eax
.hasplace:
	push	ecx
	cmp	eax, ecx
	ja	@f
	mov	ecx, eax
@@:
	push	ecx
	push	eax
	push	esi edi
	mov	esi, ebx
	mov	edx, ecx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	mov	ebx, esi
	mov	ecx, edx
	pop	esi
; calculate number of lines in this block
	test	ecx, ecx
	jz	.4
	mov	al, [esi - 1]
	mov	edx, esi
	sub	edx, [EditBlockStart]
	test	edx, 0xFFF
	jnz	.0
	mov	al, 0
	sub	edx, ebp
	cmp	edx, [ebp + editor_data.first_block]
	jz	.0
	sub	edx, 1000h
	add	edx, ebp
	add	edx, [edx + edit_block_header.limit]
	mov	al, [edx - 1]
.0:
	xor	edx, edx
	cmp	al, 13
	jnz	.1
	mov	dl, 10
.1:
	mov	al, [esi]
	add	esi, 1
	cmp	al, 13
	jz	@f
	cmp	al, 10
	jz	@f
	mov	dl, 0
	sub	ecx, 1
	jnz	.1
	jmp	.4
@@:
	cmp	al, dl
	mov	dl, 0
	jz	.3
	add	[ebp + editor_data.numlines], 1
	cmp	al, 10
	jz	.3
	mov	dl, 10
.3:
	sub	ecx, 1
	jnz	.1
.4:
	pop	esi
	pop	eax
	pop	ecx
	sub	[esp], ecx
	sub	eax, ecx
	pop	ecx
	jnz	.loadloop
	pop	ebx eax
	cmp	eax, 16384
	jz	.readloop
.readdone:
	push	edi
	and	edi, not 0xFFF
	xor	eax, eax
	stosd	; editor_block_header.next
	lea	eax, [edi - 4 - 0x1000]
	sub	eax, ebp
	stosd	; editor_block_header.prev
	pop	eax
	sub	eax, edi
	add	eax, 8
	stosd	; editor_block_header.limit
	mov	ecx, [EditPlugInfo]
	inc	ecx
	jz	@f
	dec	ecx
@@:
	xor	eax, eax
	rep	stosb
	and	edi, not 0xFFF
	lea	ecx, [edi + 0x1000]
	sub	ecx, ebp
	mov	edx, ecx
	shr	ecx, 12
	sub	ecx, [ebp + editor_data.memsize]
	neg	ecx
	mov	[ebp + editor_data.numfree], ecx
	jz	.nofree
	mov	[ebp + editor_data.freeblocks], edx
	push	edi
.addfree:
	add	edi, 1000h
	add	edx, 1000h
	mov	[edi], edx
	loop	.addfree
	mov	[edi], eax
	pop	edi
.nofree:
	sub	edi, ebp
	mov	[ebp + editor_data.last_block], edi
	mov	ecx, [EditDataSize]
	shl	ecx, 12
	mov	[ecx + ebp + edit_block_header.prev], eax
	mov	[ebp + editor_data.curline], eax
	mov	[ebp + editor_data.curcol], eax
	mov	[ebp + editor_data.cursor_x], eax
	inc	eax
	mov	[ebp + editor_data.cursor_y], eax
	mov	eax, [EditBlockStart]
	;mov	[ebp + editor_data.curpos_offs], eax
	mov	[ebp + editor_data.linedata_start + editor_line.offs], ax
	mov	[ebp + editor_data.cur_offs], eax
	mov	ecx, [ebp + editor_data.first_block]
	cmp	[ecx + edit_block_header.limit], eax
	setz	cl
	movzx	ecx, cl
	dec	ecx
	mov	[ebp + editor_data.cur_delta], ecx
	call	editor_init_lines
editor_OnRedraw:
	mov	eax, [ebp + editor_data.cursor_x]
	mov	[cursor_x], eax
	mov	eax, [ebp + editor_data.cursor_y]
	mov	[cursor_y], eax
	test	[ebp + editor_data.flags], 10h
	jz	@f
	mov	[cursor_size], cursor_big_size
@@:
	call	editor_test_cursor_x
	call	editor_test_cursor_y
	call	editor_set_keybar
	call	editor_draw_text
	ret

editor_save:
	cmp	[ebp + editor_data.hPlugin], 0
	jz	.native
	push	aCannotSaveToPlugin
	mov	eax, esp
	push	ContinueBtn
	push	1
	push	eax
	push	1
	call	SayErr
	pop	eax
	ret
.native:
	call	editor_calc_filesize
	mov	ebx, writeinfo
	mov	[ebx+4], eax
	xor	eax, eax
	mov	[ebx+8], eax
	mov	[ebx+12], eax
	mov	[ebx+16], eax
	lea	eax, [ebp + editor_data.filename]
	mov	[ebx+21], eax
.setsize_retry:
	mov	byte [ebx], 4
	push	70
	pop	eax
	push	ebx
	int	0x40
	pop	ebx
	mov	byte [ebx], 3
	test	eax, eax
	jz	.sizeok
	push	dword [ebx+21]
	push	aCannotWriteFile
	call	get_error_msg
	push	eax
	mov	eax, esp
	push	RetryOrCancelBtn
	push	2
	push	eax
	push	3
	call	SayErr
	add	esp, 12
	test	eax, eax
	jz	.setsize_retry
.ret:
	ret
.sizeok:
	and	dword [ebx+4], 0
	mov	esi, [ebp + editor_data.first_block]
	add	esi, ebp
	mov	ebx, [EditBlockStart]
	call	editor_normalize_offs
	jnc	.writeok
	lea	edi, [ebp + editor_data.buf]
.loop:
	mov	ecx, 16384
	call	editor_get_data
	test	eax, eax
	jz	.done
	push	ebx
	mov	ebx, writeinfo
	mov	[ebx+12], eax
	mov	[ebx+16], edi
.write_retry:
	push	70
	pop	eax
	push	ebx
	int	0x40
	pop	ebx
	test	eax, eax
	jz	.writeok
	push	dword [ebx+21]
	push	aCannotWriteFile
	call	get_error_msg
	push	eax
	mov	eax, esp
	push	RetryOrCancelBtn
	push	2
	push	eax
	push	3
	call	SayErr
	add	esp, 12
	test	eax, eax
	jz	.write_retry
	ret
.writeok:
	mov	eax, [ebx+12]
	add	[ebx+4], eax
	adc	dword [ebx+8], 0
	pop	ebx
	jmp	.loop
.done:
	and	[ebp + editor_data.flags], not 0x80
	ret

editor_calc_filesize:
	xor	eax, eax
	push	esi
	mov	esi, [ebp + editor_data.first_block]
@@:
	add	esi, ebp
	add	eax, [esi + edit_block_header.limit]
	sub	eax, [EditBlockStart]
	mov	esi, [esi + edit_block_header.next]
	test	esi, esi
	jnz	@b
	pop	ebx
	ret

editor_get_data:
	push	edi
	test	esi, esi
	jz	.ret
.loop:
	mov	edx, [esi + edit_block_header.limit]
	sub	edx, ebx
	push	ecx
	cmp	ecx, edx
	jb	@f
	mov	ecx, edx
@@:
	push	esi
	add	esi, ebx
	add	ebx, ecx
	add	eax, ecx
	rep	movsb
	pop	esi
	pop	ecx
	sub	ecx, edx
	jb	.ret
	mov	esi, [esi + edit_block_header.next]
	mov	ebx, [EditBlockStart]
	test	esi, esi
	jz	.ret
	add	esi, ebp
	jmp	.loop
.ret:
	mov	eax, edi
	pop	edi
	sub	eax, edi
	ret

editor_get_pos:
	;mov	esi, [ebp + editor_data.curpos_block]
	;mov	ebx, [ebp + editor_data.curpos_offs]
	mov	esi, [ebp + editor_data.linedata_start + editor_line.block]
	sub	esi, ebp
	movzx	ebx, [ebp + editor_data.linedata_start + editor_line.offs]
@@:
	test	esi, esi
	jz	@f
	add	esi, ebp
	cmp	ebx, [esi + edit_block_header.limit]
	jb	@f
	mov	esi, [esi + edit_block_header.next]
	mov	ebx, [EditBlockStart]
	jmp	@b
@@:
	ret

editor_curline_start:
	mov	edx, [EditPlugInfo]
	add	edx, editor_line.plugdata
	mov	eax, [ebp + editor_data.cursor_y]
	dec	eax
	imul	eax, edx
	lea	edi, [ebp + eax + editor_data.linedata_start]
	mov	esi, [edi + editor_line.block]
	movzx	ebx, [edi + editor_line.offs]
	ret

editor_step_forward:
; in: esi = block (must be nonzero), ebx = offset in block
; out: esi = block (zero iff EOF reached), ebx = offset in block
; out: CF=1 iff EOF NOT reached
	inc	ebx
editor_normalize_offs:
	cmp	ebx, [esi + edit_block_header.limit]
	jb	@f
	mov	esi, [esi + edit_block_header.next]
	mov	ebx, [EditBlockStart]
	test	esi, esi
	jz	@f
	add	esi, ebp
	stc
@@:
	ret

editor_step_backward:
; in: esi = block (must be nonzero), ebx = offset in block
; out: esi = block (zero iff input was at the beginning), ebx = offset in block
; out: CF=1 iff start of file reached
	dec	ebx
	cmp	ebx, [EditBlockStart]
	jae	@f
	mov	esi, [esi + edit_block_header.prev]
	test	esi, esi
	stc
	jz	@f
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
	dec	ebx
@@:
	ret

editor_get_string:
; read string up to the end of line
; in: esi = block, ebx = offset in block
; in: edi = destination, ecx = maximum number of bytes to copy, edx = number of bytes to skip
; out: esi = block, ebx = offset in block
; out: ecx = number of rest bytes (ecx_in - ecx_out = number of copied characters)
	mov	ah, [edit_normal_color]
	cmp	ebx, [esi + edit_block_header.limit]
	jz	.retnp
	push	0	; current position in line
.loop:
	test	ecx, ecx
	jz	.ret
	mov	al, [esi + ebx]
	cmp	al, 13
	jz	.ret
	cmp	al, 10
	jz	.ret
	cmp	al, 9
	jz	.tab
	inc	dword [esp]
	dec	edx
	jns	.4
	xor	edx, edx
	stosw
	dec	ecx
.4:
	call	editor_step_forward
	jc	.loop
.ret:
	pop	edx
.retnp:
	ret
.tab:
	push	eax edx
	mov	eax, [esp+8]
	xor	edx, edx
	div	[editor_tabsize]
	sub	edx, [editor_tabsize]
	neg	edx
	add	[esp+8], edx
	sub	[esp], edx
	pop	edx eax
	jns	.4
	mov	al, ' '
@@:
	stosw
	dec	ecx
	jz	.ret
	inc	edx
	jnz	@b
	jmp	.4

editor_find_newline:
; in: esi = block, ebx = offset in block
; out: esi = block, ebx = offset in block, ecx = line length
	xor	ecx, ecx
	test	esi, esi
	jz	.ret0
	cmp	ebx, [esi + edit_block_header.limit]
	jb	.1
	xor	esi, esi
.ret0:
	ret
.1:
	mov	al, [esi + ebx]
	inc	ecx
	call	editor_step_forward
	cmp	al, 13
	jz	.2
	cmp	al, 10
	jz	.2
	test	esi, esi
	jnz	.1
.ret1:
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
	ret
.2:
	dec	ecx
	test	esi, esi
	jz	.ret1
	cmp	al, 13
	jnz	.ret
	cmp	byte [esi + ebx], 10
	jnz	.ret
	call	editor_step_forward
	jnc	.ret1
.ret:
	ret

editor_prev_newline:
	xor	ecx, ecx
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
@@:
	call	editor_step_backward
	jc	.ret
	mov	al, [esi + ebx]
	call	editor_step_backward
	jc	.ret
	cmp	al, 10
	jnz	.1
	cmp	byte [esi + ebx], 13
	jnz	.1
@@:
	call	editor_step_backward
	jc	.ret
.1:
	inc	ecx
	mov	al, [esi + ebx]
	cmp	al, 13
	jz	@f
	cmp	al, 10
	jnz	@b
@@:
	dec	ecx
	inc	ebx
	cmp	ebx, [esi + edit_block_header.limit]
	jb	.ret
	mov	esi, [esi + edit_block_header.next]
	mov	ebx, [EditBlockStart]
	test	esi, esi
	jz	.ret
	add	esi, ebp
.ret:
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.first_block]
	mov	ebx, [EditBlockStart]
	add	esi, ebp
@@:
	ret

editor_init_lines:
	call	editor_get_pos
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
@@:
	mov	ecx, max_height
	lea	edi, [ebp + editor_data.linedata_start]
.1:
	mov	eax, esi
	stosd	; editor_line.block
	mov	eax, ebx
	stosw	; editor_line.offs
	push	ecx
	call	editor_find_newline
	;mov	eax, ecx
	;stosd	; editor_line.length
	xor	eax, eax
	mov	ecx, [EditPlugInfo]
	rep	stosb
	pop	ecx
	loop	.1
	ret

editor_draw_status:
	lea	edi, [ebp + editor_data.buf]
	mov	ah, [edit_status_color]
	mov	ecx, [cur_width]
	sub	ecx, 56
	cmp	ecx, 25
	jge	@f
	push	25
	pop	ecx
@@:
	call	viewedit_draw_filename
	inc	ecx
	rep	stosw
	test	[ebp + editor_data.flags], 80h
	jz	@f
	mov	al, '*'
@@:
	stosw
	mov	al, ' '
	test	[ebp + editor_data.flags], 40h
	jz	@f
	mov	al, '-'
@@:
	stosw
	mov	al, ' '
	test	[ebp + editor_data.flags], 20h
	jz	@f
	mov	al, '"'
@@:
	stosw
	mov	al, ' '
	mov	cl, 10
        rep     stosw
        movzx   esi, [ebp+editor_data.encoding]
        lea     esi, [encodings.names+esi*8]
        push    edi esi
        dec     edi
        dec     edi
        std
        add     esi, 8
@@:
        dec     esi
        cmp     byte [esi], ' '
        jz      @b
@@:
        lodsb
        stosw
        cmp     esi, [esp]
        jae     @b
        cld
        pop     esi edi
	mov	esi, aLine
	mov	cl, 8
@@:
	lodsb
	stosw
	loop	@b
	mov	cl, 13
	mov	al, ' '
	rep	stosw
	std
	push	edi
	dec	edi
	dec	edi
	push	eax
	mov	eax, [ebp + editor_data.numlines]
	mov	cl, 10
@@:
	xor	edx, edx
	div	ecx
	xchg	eax, edx
	add	al, '0'
	mov	ah, [edit_status_color]
	stosw
	xchg	eax, edx
	test	eax, eax
	jnz	@b
	mov	al, '/'
	mov	ah, [edit_status_color]
	stosw
	mov	eax, [ebp + editor_data.curline]
	add	eax, [ebp + editor_data.cursor_y]
@@:
	xor	edx, edx
	div	ecx
	xchg	eax, edx
	add	al, '0'
	mov	ah, [edit_status_color]
	stosw
	xchg	eax, edx
	test	eax, eax
	jnz	@b
	pop	eax
	stosw
	cld
	pop	edi
	mov	ah, [edit_status_color]
	mov	esi, aCol
	mov	cl, 7
@@:
	lodsb
	stosw
	loop	@b
	mov	eax, [ebp + editor_data.curcol]
	add	eax, [ebp + editor_data.cursor_x]
	inc	eax
	mov	cl, 10
	push	-'0'
@@:
	xor	edx, edx
	div	ecx
	push	edx
	test	eax, eax
	jnz	@b
@@:
	pop	eax
	mov	ah, [edit_status_color]
	add	al, '0'
	jz	@f
	stosw
	jmp	@b
@@:
	mov	al, ' '
	mov	cl, 13
	rep	stosw
	xor	eax, eax
	xor	edx, edx
	call	get_console_ptr
	lea	esi, [ebp + editor_data.buf]
	mov	ecx, [cur_width]
	shr	ecx, 1
	rep	movsd
	adc	ecx, ecx
	rep	movsw
	cmp	[ebp + editor_data.cur_delta], -1
	jnz	.a
	mov	al, ' '
	mov	byte [edi-4*2], al
	mov	byte [edi-3*2], al
	mov	byte [edi-2*2], al
	mov	byte [edi-1*2], al
	mov	eax, [ebp + editor_data.cur_block]
	add	eax, ebp
	add	eax, [ebp + editor_data.cur_offs]
	movzx	eax, byte [eax]
	mov	cl, 100
	;xor	edx, edx	; edx=0 already
	div	ecx
	test	al, al
	jz	@f
	add	al, '0'
	mov	[edi-3*2], al
@@:
	xchg	eax, edx
	aam
	test	ah, ah
	jnz	.b
	cmp	byte [edi-3*2], ' '
	jz	@f
.b:
	add	ah, '0'
	mov	[edi-2*2], ah
@@:
	add	al, '0'
	mov	[edi-1*2], al
.a:
	ret

editor_draw_line:
	push	ecx
	mov	ecx, [cur_width]
	test	esi, esi
	jz	.2
	lea	edi, [ebp + editor_data.buf]
	push	edx edi
	mov	edx, [ebp + editor_data.curcol]
	call	editor_get_string
	mov	al, ' '
	rep	stosw
	pop	esi edx
	xor	eax, eax
	call	get_console_ptr
	mov	ecx, [cur_width]
	shr	ecx, 1
	rep	movsd
	adc	ecx, ecx
	rep	movsw
	pop	ecx
	ret
.2:
	xor	eax, eax
	call	get_console_ptr
	mov	al, ' '
	mov	ah, [edit_normal_color]
	rep	stosw
	pop	ecx
	ret

editor_draw_text:
	call	editor_draw_status
	push	1
	pop	edx
	lea	ecx, [ebp + editor_data.linedata_start]
.1:
	mov	esi, [ecx + editor_line.block]
	movzx	ebx, [ecx + editor_line.offs]
	add	ecx, editor_line.plugdata
	add	ecx, [EditPlugInfo]
	call	editor_draw_line
	inc	edx
	mov	eax, [cur_height]
	dec	eax
	cmp	edx, eax
	jb	.1
	jmp	draw_image

editor_set_keybar:
	mov	eax, keybar_editor
	movzx	esi, [ebp+editor_data.encoding]
	dec	esi
	jz	@f
	push	1
	pop	esi
@@:
	lea	esi, [encodings.names+esi*8]
	lea	edi, [eax+keybar_cp2-keybar_editor]
	movsd
	movsw
	jmp	draw_keybar

editor_up_scroll:
	push	ecx
	sub	[ebp + editor_data.curline], ecx
	mov	edx, [EditPlugInfo]
	add	edx, editor_line.plugdata
	imul	eax, edx, max_height
	imul	ecx, edx
	sub	ecx, eax
	neg	ecx
	lea	esi, [ebp + ecx + editor_data.linedata_start - 4]
	lea	edi, [ebp + eax + editor_data.linedata_start - 4]
	shr	ecx, 2
	std
	rep	movsd
	cld
	jnc	@f
	mov	cx, [esi+2]
	mov	[edi+2], cx
	sub	edi, 2
@@:
	pop	ecx
	add	edi, 4
	movzx	ebx, [edi + editor_line.offs]
	mov	esi, [edi + editor_line.block]
@@:
	push	ecx
	call	editor_prev_newline
	sub	edi, edx
	push	edi
	mov	eax, esi
	stosd	; editor_line.offs
	mov	eax, ebx
	stosw	; editor_line.block
	;mov	eax, ecx
	;stosd	; editor_line.length
	mov	ecx, [EditPlugInfo]
	xor	eax, eax
	rep	stosb
	pop	edi
	pop	ecx
	loop	@b
; fall through to editor_update_cur

editor_update_cur:
	mov	ecx, [ebp + editor_data.cursor_x]
	add	ecx, [ebp + editor_data.curcol]
	call	editor_curline_start
	xor	edx, edx	; current position in the line
	cmp	ebx, [esi + edit_block_header.limit]
	jz	.notfound
.scan:
	mov	al, [esi+ebx]
	cmp	al, 13
	jz	.notfound
	cmp	al, 10
	jz	.notfound
	test	ecx, ecx
	jz	.found
	cmp	al, 9
	jz	.tab
	inc	edx
	dec	ecx
	call	editor_step_forward
	jc	.scan
.notfound:
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
@@:
	mov	[ebp + editor_data.cur_delta], ecx
	xor	eax, eax
	jmp	.reta
.found:
	xor	eax, eax
.founda:
	or	[ebp + editor_data.cur_delta], -1
.reta:
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
	ret
.tab:
	push	edx
	mov	eax, edx
	xor	edx, edx
	div	[editor_tabsize]
	sub	edx, [editor_tabsize]
	neg	edx
	add	[esp], edx
	cmp	ecx, edx
	jb	.curintab
	sub	ecx, edx
	pop	edx
	call	editor_step_forward
	jnc	.notfound
	test	ecx, ecx
	jz	.found
	jmp	.scan
.curintab:
	sub	[esp], edx
	pop	edx
	cmp	edx, [ebp + editor_data.curcol]
	setc	al
	jae	@f
	mov	[ebp + editor_data.curcol], edx
@@:
	sub	edx, [ebp + editor_data.curcol]
	mov	[ebp + editor_data.cursor_x], edx
	mov	[cursor_x], edx
	jmp	.founda

editor_down_scroll:
	push	ecx
	lea	edi, [ebp + editor_data.linedata_start]
	mov	edx, [EditPlugInfo]
	add	edx, editor_line.plugdata
	imul	ecx, edx
	lea	esi, [edi + ecx]
	imul	eax, edx, max_height
	sub	eax, ecx
	mov	ecx, eax
	shr	ecx, 2
	rep	movsd
	adc	ecx, ecx
	rep	movsw
@@:
	mov	esi, edi
	sub	esi, edx
	movzx	ebx, [esi + editor_line.offs]
	mov	esi, [esi + editor_line.block]
	pop	ecx
	jecxz	.ret
@@:
	push	ecx
	call	editor_find_newline
	mov	eax, esi
	stosd	; editor_line.block
	mov	eax, ebx
	stosw	; editor_line.offs
	;mov	eax, ecx
	;stosd	; editor_line.length
	mov	ecx, [EditPlugInfo]
	xor	eax, eax
	rep	stosb
	pop	ecx
	loop	@b
.ret:
	ret

editor_end_scroll:
	call	editor_curline_start
; calculate visible length of the line (it differs from the real length if tabulations are present)
	xor	ecx, ecx
	cmp	ebx, [esi + edit_block_header.limit]
	jz	.calcdone
.calcloop:
	mov	al, [esi+ebx]
	cmp	al, 10
	jz	.calcdone
	cmp	al, 13
	jz	.calcdone
	cmp	al, 9
	jz	.calctab
	inc	ecx
.calcnext:
	call	editor_step_forward
	jc	.calcloop
	jmp	.calcdone
.calctab:
	mov	eax, ecx
	xor	edx, edx
	div	[editor_tabsize]
	sub	edx, [editor_tabsize]
	sub	ecx, edx
	jmp	.calcnext
.calcdone:
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
@@:
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
	and	[ebp + editor_data.cur_delta], 0
; ecx = number of symbols in the line
; calculate new position of view range
	mov	eax, [ebp + editor_data.curcol]
	cmp	ecx, eax
	jb	.toleft
	add	eax, [cur_width]
	cmp	ecx, eax
	jae	.toright
	sub	ecx, [ebp + editor_data.curcol]
	mov	[ebp + editor_data.cursor_x], ecx
	mov	[cursor_x], ecx
	ret
.toleft:
	mov	[ebp + editor_data.curcol], ecx
	and	[ebp + editor_data.cursor_x], 0
	and	[cursor_x], 0
	stc
	ret
.toright:
	mov	eax, [cur_width]
	dec	eax
	sub	ecx, eax
	mov	[ebp + editor_data.curcol], ecx
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
	stc
	ret

editor_move_linestart:
; in: esi = block, ebx = start offset, ecx = delta
	lea	edi, [ebp + editor_data.linedata_start]
	mov	edx, max_height
.0:
	cmp	[edi + editor_line.block], esi
	jnz	.1
	movzx	eax, [edi + editor_line.offs]
	cmp	eax, ebx
	ja	.2
;	push	eax
;	add	eax, [edi + editor_line.length]
;	cmp	eax, ebx
;	jb	@f
;	add	[edi + editor_line.length], ecx
;@@:
;	pop	eax
	cmp	eax, [esi + edit_block_header.limit]
	jb	.1
	push	esi ebx
	mov	ebx, eax
	jmp	.3
.2:
	push	esi ebx
	mov	ebx, eax
	add	ebx, ecx
.3:
	cmp	ebx, [esi + edit_block_header.limit]
	jb	.4
	cmp	[esi + edit_block_header.next], 0
	jz	.4
	sub	ebx, [esi + edit_block_header.limit]
	mov	esi, [esi + edit_block_header.next]
	add	esi, ebp
	add	ebx, [EditBlockStart]
	jmp	.3
.4:
	mov	[edi + editor_line.block], esi
	mov	[edi + editor_line.offs], bx
	pop	ebx esi
.1:
	add	edi, editor_line.plugdata
	add	edi, [EditPlugInfo]
	dec	edx
	jnz	.0
	;lea	edx, [ebp + editor_data.curpos_block]
	;call	.5
	;add	edx, editor_data.cur_block - editor_data.curpos_block
	lea	edx, [ebp + editor_data.cur_block]
;	call	.5
;	ret
.5:
	mov	eax, [edx]
	add	eax, ebp
	cmp	eax, esi
	jnz	.6
	cmp	[edx+4], ebx
	jbe	@f
	add	[edx+4], ecx
@@:
	mov	eax, [edx+4]
	sub	eax, [esi + edit_block_header.limit]
	jb	.6
	push	ecx
	mov	ecx, [esi + edit_block_header.next]
	add	ecx, ebp
	add	eax, [EditBlockStart]
	mov	[edx], ecx
	mov	[edx+4], eax
	pop	ecx
.6:
	ret

editor_reserve_symbol:
	push	1
	pop	ecx

editor_reserve_space:
; Вставляет пустое место внутрь цепочки блоков
; in: ecx = number of symbols to add, esi = block, ebx = offset
	mov	eax, [esi + edit_block_header.limit]
	add	eax, ecx
	cmp	eax, 4096
	ja	.add
	mov	[esi + edit_block_header.limit], eax
	push	esi ecx
	sub	eax, ecx
	lea	esi, [esi + eax - 1]
	lea	edi, [esi + ecx]
	sub	eax, ebx
	mov	ecx, eax
	std
	rep	movsb
	cld
	pop	ecx esi
	call	editor_move_linestart
	clc
	ret
.add:
	push	ecx
	mov	eax, [esi + edit_block_header.limit]
	sub	eax, [EditBlockStart]
	add	eax, ecx
	push	eax
	xor	edx, edx
	div	[EditBlockSize]
	push	eax
	sub	eax, [ebp + editor_data.numfree]
	jbe	.norealloc
	mov	edx, [ebp + editor_data.memsize]
	push	eax edx
	add	eax, edx
	mov	ecx, eax
	shl	ecx, 12
	mov	edx, ebp
	call	xpgrealloc
	pop	edx ecx
	test	eax, eax
	jz	.nomem
	sub	eax, ebp
	add	ebp, eax
	add	esi, eax
	add	[ebp + editor_data.memsize], ecx
	push	ecx
	mov	ecx, max_height
	lea	edi, [ebp + editor_data.linedata_start]
@@:
	add	[edi + editor_line.block], eax
	add	edi, editor_line.plugdata
	add	edi, [EditPlugInfo]
	loop	@b
	pop	ecx
	shl	edx, 12
	add	[ebp + editor_data.numfree], ecx
	lea	edi, [edx + ebp]
	mov	eax, [ebp + editor_data.freeblocks]
	mov	[ebp + editor_data.freeblocks], edx
@@:
	add	edx, 1000h
	mov	[edi], edx
	add	edi, 1000h
	loop	@b
	mov	[edi-1000h], eax
.norealloc:
	pop	edx
	push	[esi + edit_block_header.next]
	push	esi
	sub	[ebp + editor_data.numfree], edx
	mov	edi, [ebp + editor_data.freeblocks]
@@:
	mov	[esi + edit_block_header.next], edi
	add	edi, ebp
	push	edi
	push	dword [edi]
	stosd	; edit_block_header.next - will be filled later
	mov	eax, esi
	sub	eax, ebp
	stosd	; edit_block_header.prev
	mov	eax, 4096 - edit.RESERVE_IN_BLOCK
	stosd	; edit_block_header.limit
	mov	eax, [EditBlockSize]
	sub	[esp+16], eax
	xor	eax, eax
	mov	ecx, [EditPlugInfo]
	rep	stosb
	pop	edi
	pop	esi
	dec	edx
	jnz	@b
	mov	[ebp + editor_data.freeblocks], edi
	mov	edi, esi
	pop	esi
	pop	[edi + edit_block_header.next]
	pop	ecx
	add	ecx, [EditBlockSize]
	mov	edx, ecx
	shr	edx, 1;2
	mov	eax, ecx
	sub	eax, edx
	cmp	eax, [EditBlockSize]
	jb	@f
	mov	eax, [EditBlockSize]
	mov	edx, ecx
	sub	edx, eax
@@:
	add	eax, [EditBlockStart]
	add	edx, [EditBlockStart]
	mov	ecx, [esi + edit_block_header.limit]
	mov	[esi + edit_block_header.limit], eax
	mov	[edi + edit_block_header.limit], edx
	sub	ecx, ebx
	push	ecx
	push	esi edi ecx
	add	esi, ecx
	add	esi, ebx
	push	edx
	sub	edx, [EditBlockStart]
	cmp	ecx, edx
	jb	@f
	mov	ecx, edx
@@:
	pop	edx
	sub	[esp], ecx
	add	edi, edx
	sub	esi, ecx
	sub	edi, ecx
	rep	movsb
	pop	ecx edi esi
	push	esi edi
	lea	edi, [esi + eax - 1]
	add	esi, ebx
	lea	esi, [esi + ecx - 1]
	std
	rep	movsb
	cld
	pop	edi esi
	pop	ecx
	mov	ecx, ebx
	sub	ecx, eax
	jb	@f
	push	esi edi
	add	esi, eax
	add	edi, [EditBlockStart]
	rep	movsb
	pop	edi esi
@@:
	push	esi edi
	sub	esi, ebp
	sub	edi, ebp
	cmp	[ebp + editor_data.last_block], esi
	jnz	@f
	mov	[ebp + editor_data.last_block], edi
@@:
	pop	edi esi
	pop	ecx
	call	editor_move_linestart
	cmp	ebx, [esi + edit_block_header.limit]
	jb	@f
	sub	ebx, [esi + edit_block_header.limit]
	mov	esi, [esi + edit_block_header.next]
	add	esi, ebp
	add	ebx, [EditBlockStart]
@@:
	clc
	ret
.nomem:
	pop	eax
	pop	ecx
	pop	ecx
	add	esi, ebp
	stc
	ret

editor_delete_symbol:
	push	1
	pop	ecx

editor_delete_space:
; Удаляет ecx байт из текста, начиная с esi:ebx
; ecx, esi, ebx разрушаются
	mov	eax, [esi + edit_block_header.limit]
	sub	eax, ebx
	cmp	eax, ecx
	jb	.more1
	push	esi
	sub	[esi + edit_block_header.limit], ecx
	sub	eax, ecx
	lea	edi, [esi+ebx]
	lea	esi, [edi+ecx]
	mov	ecx, eax
	rep	movsb
	pop	esi
.done:
	call	.collapse_prev
	call	.collapse_next
	call	editor_init_lines
	jmp	editor_update_cur
.more1:
	mov	[esi + edit_block_header.limit], ebx
	sub	ecx, eax
@@:
	mov	esi, [esi + edit_block_header.next]
	add	esi, ebp
	mov	eax, [esi + edit_block_header.limit]
	sub	eax, [EditBlockStart]
	sub	ecx, eax
	jb	@f
	call	.delete_block
	jmp	@b
@@:
	add	ecx, eax
	push	esi
	mov	eax, [esi + edit_block_header.limit]
	sub	[esi + edit_block_header.limit], ecx
	add	eax, esi
	add	esi, [EditBlockStart]
	mov	edi, esi
	add	esi, ecx
	sub	eax, esi
	mov	ecx, esi
	rep	movsb
	pop	esi
	call	.collapse_next
	mov	esi, [esi + edit_block_header.prev]
	add	esi, ebp
	jmp	.done

.delete_block:
	mov	eax, [esi + edit_block_header.next]
	mov	edx, [esi + edit_block_header.prev]
	test	eax, eax
	jz	.dbfirst
	mov	[eax + ebp + edit_block_header.prev], edx
	jmp	@f
.dbfirst:
	mov	[ebp + editor_data.first_block], edx
@@:
	test	edx, edx
	jz	.dblast
	mov	[edx + ebp + edit_block_header.next], eax
	jmp	@f
.dblast:
	mov	[ebp + editor_data.last_block], eax
@@:
	mov	eax, [ebp + editor_data.freeblocks]
	mov	[esi], eax
	sub	esi, ebp
	mov	[ebp + editor_data.freeblocks], esi
	inc	[ebp + editor_data.numfree]
	ret

.collapse_prev:
	mov	eax, [esi + edit_block_header.prev]
	test	eax, eax
	jz	.cpno
	add	eax, ebp
	mov	edx, [esi + edit_block_header.limit]
	sub	edx, [EditBlockStart]
	add	edx, [eax + edit_block_header.limit]
	cmp	edx, 4096 - edit.RESERVE_IN_BLOCK
	ja	.cpno
	mov	edi, eax
.collapse:	; (edi) + (esi) -> (edi)
	push	edi
	push	esi
	mov	ecx, [esi + edit_block_header.limit]
	sub	ecx, [EditBlockStart]
	add	esi, [EditBlockStart]
	mov	eax, [edi + edit_block_header.limit]
	add	[edi + edit_block_header.limit], ecx
	add	edi, eax
	rep	movsb
	pop	esi
	call	.delete_block
	pop	esi
.cpno:
	ret
.collapse_next:
	mov	eax, [esi + edit_block_header.next]
	test	eax, eax
	jz	.cpno
	add	eax, ebp
	mov	edx, [esi + edit_block_header.limit]
	sub	edx, [EditBlockStart]
	add	edx, [eax + edit_block_header.limit]
	cmp	edx, 4096 - edit.RESERVE_IN_BLOCK
	ja	.cpno
	mov	edi, esi
	mov	esi, eax
	jmp	.collapse

editor_OnKey:
	test	al, 80h
	jnz	.ret
	test	[ebp + editor_data.flags], 20h
	jnz	.symbol
	mov	esi, editor_ctrlkeys
	call	process_ctrl_keys
	jnc	.ret
.symbol:
	and	[ebp + editor_data.flags], not 20h
	test	[ebp + editor_data.flags], 40h
	jnz	.ret
	or	[ebp + editor_data.flags], 80h
	movzx	eax, al
	call	get_ascii_char
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	cmp	al, 10
	jz	.insert_newline
	cmp	al, 13
	jz	.insert_newline
	cmp	[ebp + editor_data.cur_delta], -1
	jnz	.insert_after_eol
	test	[ebp + editor_data.flags], 10h
	jnz	.replace_symbol
	push	eax
	call	editor_reserve_symbol
	pop	eax
	jc	.ret
.replace_symbol:
	cmp	ebx, [esi + edit_block_header.limit]
	jnz	@f
	mov	esi, [esi + edit_block_header.next]
	add	esi, ebp
	mov	ebx, [EditBlockStart]
@@:
	mov	[esi + ebx], al
.symb_inserted:
	call	.redraw_curline
	jmp	.right
.redraw_curline:
	call	editor_curline_start
	mov	edx, [cursor_y]
	call	editor_draw_line
.ret:
	ret
.insert_after_eol:
	mov	ecx, [ebp + editor_data.cur_delta]
	inc	ecx
	push	eax
	call	editor_reserve_space
	pop	eax
	jc	.ret
	push	eax
	mov	edx, ecx
.2:
	mov	ecx, [esi + edit_block_header.limit]
	sub	ecx, ebx
	lea	edi, [esi + ebx]
	cmp	ecx, edx
	jb	@f
	mov	ecx, edx
@@:
	add	ebx, ecx
	sub	edx, ecx
	mov	al, ' '
	rep	stosb
	jz	@f
	mov	esi, [esi + edit_block_header.next]
	add	esi, ebp
	mov	ebx, [EditBlockStart]
	jmp	.2
@@:
	pop	eax
	mov	[edi-1], al
	dec	ebx
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
	or	[ebp + editor_data.cur_delta], -1
	jmp	.symb_inserted
.insert_newline:
	push	1
	pop	ecx
	cmp	[ebp + editor_data.eol], 2
	adc	ecx, 0
	call	editor_reserve_space
	jc	.ret
; Просто так вставлять символ новой строки чревато - из-за существования двухбайтовых комбинаций перевода строки
; старый символ может слиться с новым. В таком случае придётся заниматься корректировкой.
; Проблема бывает в двух случаях:
; если редактор настроен на Unix-стиль и новый перевод строки вставляется непосредственно после
;	перевода строки в Mac-стиле;
; если редактор настроен на Mac-стиль и новый перевод строки вставляется непосредственно перед
;	переводом строки в Unix-стиле.
; В этих случаях форсируем перевод строки в текущем стиле файла.
	mov	al, [ebp + editor_data.eol]
	cmp	al, edit.eol_dos
	jz	.insert_eol_dos
	cmp	al, edit.eol_mac
	jz	.insert_eol_mac
	mov	al, 10	; Unix-style for end-of-line
	push	esi ebx
	call	editor_step_backward
	jc	@f
	cmp	byte [esi+ebx], 13
	jnz	@f
	mov	al, 13	; force Mac-style to avoid collapse
@@:
	pop	ebx esi
	mov	[esi+ebx], al
	jmp	.eol_correct
.insert_eol_mac:
	mov	al, 13	; Mac-style for end-of-line
	push	esi ebx
	call	editor_step_forward
	jnc	@f
	cmp	byte [esi+ebx], 10
	jnz	@f
	mov	al, 10	; force Unix-style to avoid collapse
@@:
	pop	ebx esi
	mov	[esi+ebx], al
	jmp	.eol_correct
.insert_eol_dos:
	mov	byte [esi+ebx], 13
	call	editor_step_forward
	mov	byte [esi+ebx], 10
.eol_correct:
	call	editor_step_forward
	test	esi, esi
	jnz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
@@:
	and	[ebp + editor_data.cur_delta], 0
	cmp	ebx, [esi + edit_block_header.limit]
	jz	@f
	mov	al, [esi+ebx]
	cmp	al, 10
	jz	@f
	cmp	al, 13
	jz	@f
	dec	[ebp + editor_data.cur_delta]
@@:
; ну что же, в текст нужные символы вставили, теперь надо скорректировать информацию о начале строк на экране
	mov	edi, [ebp + editor_data.cursor_y]
	mov	ecx, max_height-1
	sub	ecx, edi
	dec	edi
	mov	eax, [EditPlugInfo]
	add	eax, editor_line.plugdata
	imul	edi, eax, max_height
	imul	ecx, eax
	lea	edi, [ebp + edi + editor_data.linedata_start - 2]
	push	esi
	mov	esi, edi
	sub	esi, eax
	shr	ecx, 1
	std
	rep	movsw
	cld
	pop	esi
	add	edi, 2
	sub	edi, eax
	push	esi
	mov	esi, edi
	sub	esi, eax
	pop	eax
	stosd	; editor_line.block
	sub	eax, ebp
	mov	[ebp + editor_data.cur_block], eax
	mov	eax, ebx
	stosw	; editor_line.offs
	mov	[ebp + editor_data.cur_offs], eax
	;mov	eax, ecx
	;stosd	; editor_line.length
	mov	ecx, [EditPlugInfo]
	xor	eax, eax
	rep	stosb
; на одну строку стало больше
	inc	[ebp + editor_data.numlines]
; курсор окажется в начале строки
	mov	[ebp + editor_data.cursor_x], eax
	mov	[ebp + editor_data.curcol], eax
	mov	[cursor_x], eax
; и передвинется на строку ниже
	mov	eax, [ebp + editor_data.cursor_y]
	inc	eax
	mov	edx, [cur_height]
	dec	edx
	cmp	eax, edx
	jae	.down_scroll
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	jmp	editor_draw_text

.exit_confirm:
	test	[ebp + editor_data.flags], 80h
	jz	.exit
	push	aFileModified
	mov	eax, esp
	push	EditorExitBtn
	push	3
	push	eax
	push	1
	push	aEditorTitle
	call	SayErrTitle
	pop	ecx
	test	eax, eax
	jz	.save
	dec	eax
	jz	.exit
	ret
.exit_save:
	test	[ebp + editor_data.flags], 80h
	jz	.exit
.save:
	call	editor_save
.exit:
	call	editor_OnExit
	jmp	delete_active_screen
.f2:
	call	editor_save
	jmp	.redraw_status

.up_scroll:
	cmp	[ebp + editor_data.curline], 0
	jz	.ret
	push	1
	pop	ecx
	call	editor_up_scroll
	jmp	editor_draw_text
.up:
	mov	eax, [ebp + editor_data.cursor_y]
	dec	eax
	jz	.up_scroll
.set_cursor_y:
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	call	editor_update_cur
	test	al, al
	jnz	editor_draw_text
.redraw_status:
	call	editor_draw_status
	jmp	draw_image
.down:
	mov	eax, [ebp + editor_data.cursor_y]
	inc	eax
	push	eax
	add	eax, [ebp + editor_data.curline]
	cmp	eax, [ebp + editor_data.numlines]
	pop	eax
	ja	.ret
	mov	edx, [cur_height]
	dec	edx
	cmp	eax, edx
	jnz	.set_cursor_y
.down_scroll:
	inc	[ebp + editor_data.curline]
	lea	edi, [ebp + editor_data.linedata_start]
	mov	eax, [EditPlugInfo]
	add	eax, editor_line.plugdata
	lea	esi, [edi + eax]
	imul	ecx, eax, max_height-1
	shr	ecx, 2
	rep	movsd
	adc	ecx, ecx
	rep	movsw
	sub	esi, eax
	sub	esi, eax
	movzx	ebx, [esi + editor_line.offs]
	mov	esi, [esi + editor_line.block]
	call	editor_find_newline
	mov	eax, esi
	stosd	; editor_line.block
	mov	eax, ebx
	stosw	; editor_line.offs
	;mov	eax, ecx
	;stosd	; editor_line.length
	mov	ecx, [EditPlugInfo]
	xor	eax, eax
	rep	stosb
	jmp	.ret2

.pgup:
	mov	ecx, [cur_height]
	sub	ecx, 3
	mov	edx, [ebp + editor_data.curline]
	mov	eax, edx
	push	edx
	sub	edx, ecx
	jnc	@f
	xor	edx, edx
@@:
	mov	[ebp + editor_data.curline], edx
	add	eax, [ebp + editor_data.cursor_y]
	dec	eax
	sub	eax, ecx
	jnc	@f
	xor	eax, eax
@@:
	sub	eax, edx
	inc	eax
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	pop	ecx
	sub	ecx, edx
	push	ecx
	mov	edx, [EditPlugInfo]
	add	edx, editor_line.plugdata
	imul	ecx, edx
	imul	eax, edx, max_height
	lea	edi, [ebp + editor_data.linedata_start + eax - 2]
	mov	esi, edi
	sub	esi, ecx
	sub	eax, ecx
	mov	ecx, eax
	shr	ecx, 1
	std
	rep	movsw
	cld
	pop	ecx
	jecxz	.ret2
	inc	edi
	inc	edi
	mov	esi, [edi + editor_line.block]
	movzx	ebx, [edi + editor_line.offs]
@@:
	push	ecx
	call	editor_prev_newline
	sub	edi, edx
	push	edi
	mov	eax, esi
	stosd	; editor_line.block
	mov	eax, ebx
	stosw	; editor_line.offs
	;mov	eax, ecx
	;stosd	; editor_line.length
	mov	ecx, [EditPlugInfo]
	xor	eax, eax
	rep	stosb
	pop	edi
	pop	ecx
	loop	@b
.ret2:
	call	editor_update_cur
	jmp	editor_draw_text
.pgdn:
	mov	edx, [cur_height]
	sub	edx, 2
	mov	ecx, [ebp + editor_data.curline]
	push	ecx
	lea	ecx, [edx + ecx - 1]
	mov	eax, [ebp + editor_data.numlines]
	sub	eax, edx
	jnc	@f
	xor	eax, eax
@@:
	cmp	ecx, eax
	jb	@f
	mov	ecx, eax
@@:
	mov	[ebp + editor_data.curline], ecx
	pop	eax
	push	eax
	add	eax, [ebp + editor_data.cursor_y]
	lea	eax, [eax + edx - 1]
	cmp	eax, [ebp + editor_data.numlines]
	jb	@f
	mov	eax, [ebp + editor_data.numlines]
@@:
	sub	eax, ecx
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	pop	edx
	sub	ecx, edx
	call	editor_down_scroll
	jmp	.ret2

.left:
	call	editor_cursor_left
	jnc	.redraw_status
	jmp	editor_draw_text
.ret3:
	ret

.right:
	cmp	[ebp + editor_data.cur_delta], -1
	jz	.right_in_text
	cmp	[ebp + editor_data.curcol], 0x80000000 - 0x1000
	jae	.ret3
	inc	[ebp + editor_data.cur_delta]
	push	1
	pop	edx
	jmp	.right_char
.right_in_text:
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	mov	al, [esi + ebx]
	push	eax
	call	editor_step_forward
	test	esi, esi
	jz	.right_eol0
	mov	al, [esi + ebx]
	cmp	al, 10
	jz	.right_eol
	cmp	al, 13
	jz	.right_eol
	jmp	.right_ok
.right_eol0:
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
.right_eol:
	inc	[ebp + editor_data.cur_delta]
.right_ok:
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
	pop	eax
	push	1
	pop	edx
	cmp	al, 9
	jnz	.right_char
	mov	eax, [ebp + editor_data.curcol]
	add	eax, [ebp + editor_data.cursor_x]
	xor	edx, edx
	div	[editor_tabsize]
	sub	edx, [editor_tabsize]
	neg	edx
.right_char:
	mov	eax, [ebp + editor_data.cursor_x]
	add	eax, edx
	cmp	eax, [cur_width]
	jb	.set_cursor_x
	add	eax, [ebp + editor_data.curcol]
	mov	edx, [cur_width]
	dec	edx
	sub	eax, edx
	mov	[ebp + editor_data.cursor_x], edx
	mov	[cursor_x], edx
	mov	[ebp + editor_data.curcol], eax
	jmp	editor_draw_text
.set_cursor_x:
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
	jmp	.redraw_status

.home:
	call	editor_curline_start
	and	[ebp + editor_data.cur_delta], 0
	cmp	ebx, [esi + edit_block_header.limit]
	jz	@f
	mov	al, [esi+ebx]
	cmp	al, 10
	jz	@f
	cmp	al, 13
	jz	@f
	dec	[ebp + editor_data.cur_delta]
@@:
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
	xor	eax, eax
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
	xchg	eax, [ebp + editor_data.curcol]
	test	eax, eax
	jnz	editor_draw_text
	jmp	.redraw_status

.end:
	call	editor_end_scroll
	jc	editor_draw_text
	jmp	.redraw_status

.ins:
	xor	[ebp + editor_data.flags], 10h
	mov	eax, cursor_normal_size
	test	[ebp + editor_data.flags], 10h
	jz	@f
	mov	eax, cursor_big_size
@@:
	mov	[cursor_size], eax
	jmp	draw_image

.backspace:
	cmp	[ebp + editor_data.cur_delta], -1
	jz	@f
	cmp	[ebp + editor_data.cur_delta], 0
	jnz	.left
@@:
	test	[ebp + editor_data.flags], 40h
	jnz	.ret3
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	call	editor_step_backward
	jc	.ret3
	push	esi ebx
	call	editor_cursor_left
	pop	ebx esi
	setc	al
	push	eax
	push	esi ebx
	cmp	byte [esi+ebx], 10
	jnz	@f
	call	editor_step_backward
	jc	@f
	cmp	byte [esi+ebx], 13
	jnz	@f
	pop	eax eax
	jmp	.del_char
@@:
	pop	ebx esi
	jmp	.del_char
.del:
	test	[ebp + editor_data.flags], 40h
	jnz	.ret3
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	call	editor_normalize_offs
	jnc	.ret3
	push	0
.del_char:
	or	[ebp + editor_data.flags], 80h
	mov	al, [esi+ebx]
	push	1
	pop	ecx
	push	eax
	push	esi ebx
	cmp	al, 13
	jnz	@f
	call	editor_step_forward
	jnc	@f
	cmp	byte [esi+ebx], 10
	jnz	@f
	inc	ecx
@@:
	pop	ebx esi
	call	editor_delete_space
	pop	eax
	cmp	al, 13
	jz	@f
	cmp	al, 10
	jz	@f
	pop	eax
	test	al, al
	jnz	.del1
	call	.redraw_curline
	jmp	.redraw_status
@@:
	pop	eax
	dec	[ebp + editor_data.numlines]
	call	editor_test_cursor_y
.del1:
	jmp	editor_draw_text

.f7:
	call	find_in_file_dlg
	jz	.shift_f7
.f7.ret:
	ret
.shift_f7:
; search string SearchString in file starting from current cursor position
	mov	ebx, SearchString
	xor	eax, eax
	cmp	byte [ebx], al
	jz	.f7.ret
	mov	esi, tolower_table
	test	[find_in_file_dlgdata.case.flags], 10h
	jz	@f
	mov	esi, identical_table
@@:
	test	[find_in_file_dlgdata.whole.flags], 10h
	setnz	al
	push	eax
	push	dword [ebp+editor_data.encoding]; always cp866 for now
				; needs to be revisited after Unicode support in editor
	call	search_string_pre
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	call	editor_normalize_offs
	jnc	.f7.notfound
	xor	edi, edi
	push	ebx esi
	push	edi
	push	edi
	test	[find_in_file_dlgdata.whole.flags], 10h
	jnz	@f
	movzx	eax, byte [esi+ebx]
	jmp	.search_loop_next
@@:
	mov	edi, edx
.search_loop:
; edx -> FSM, ecx = last state, esi:ebx -> current data,
; edi = current state
; [esp] = row, [esp+4] = delta in lines
; get current symbol
	movzx	eax, byte [esi+ebx]
; calculate next state
	movzx	edi, byte [edi+eax]
; done?
	cmp	edi, ecx
	jz	.f7.found
.search_loop_next:
; no; proceed to next symbol
	add	ebx, 1
	shl	edi, 8
	add	dword [esp], 1
	add	edi, edx
	cmp	ebx, [esi + edit_block_header.limit]
	jae	.f7.nextblock
.f7.nonextblock:
	cmp	al, 10
	jz	.f7.newline
	cmp	al, 13
	jnz	.search_loop
	cmp	byte [esi+ebx], 10
	jnz	.f7.newline
	call	editor_step_forward
	jnc	.f7.notfound_pop
.f7.newline:
	mov	dword [esp], 0
	add	dword [esp+4], 1
	mov	dword [esp+8], esi
	mov	dword [esp+12], ebx
	jmp	.search_loop
.f7.nextblock:
	call	editor_normalize_offs
	jc	.f7.nonextblock
.f7.notfound_pop:
; last chance - if we are looking for a whole word, EOF is ok for last symbol
	test	[find_in_file_dlgdata.whole.flags], 10h
	jz	@f
	mov	esi, [ebp + editor_data.last_block]
	add	esi, ebp
	mov	ebx, [esi + edit_block_header.limit]
	movzx	edi, byte [edi+' ']
	cmp	edi, ecx
	jz	.f7.found
@@:
	add	esp, 10h
.f7.notfound:
	jmp	search_failed
.f7.found:
	or	[ebp + editor_data.cur_delta], -1
	sub	ebx, ecx
	inc	ebx
	test	[find_in_file_dlgdata.whole.flags], 10h
	jz	@f
	inc	ebx
@@:
	cmp	ebx, [EditBlockStart]
	jge	@f
	sub	ebx, [EditBlockStart]
	mov	esi, [esi + edit_block_header.prev]
	add	esi, ebp
	add	ebx, [esi + edit_block_header.limit]
	jmp	@b
@@:
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	add	esi, ebp
	mov	[ebp + editor_data.cur_offs], ebx
	push	ecx
	mov	ecx, edx
	call	pgfree
	pop	ecx
	pop	eax
	pop	edi
; esi:ebx -> last symbol of match, eax = row, edi = delta in lines
	pop	esi ebx
	test	[find_in_file_dlgdata.whole.flags], 10h
	jz	@f
	dec	ecx
@@:
	push	ebx esi
	sub	eax, ecx
	lea	edx, [eax+1]
	mov	eax, [ebp + editor_data.curcol]
	add	eax, [ebp + editor_data.cursor_x]
	test	edi, edi
	jz	@f
	xor	eax, eax
@@:
	test	edx, edx
	jz	.f7.foundpos1
.f7.findpos1:
	cmp	byte [ebx+esi], 9
	jz	.f7.findpos1.tab
	inc	eax
	call	editor_step_forward
	dec	edx
	jnz	.f7.findpos1
	jmp	.f7.foundpos1
.f7.findpos1.tab:
	push	edx eax
	xor	edx, edx
	div	[editor_tabsize]
	pop	eax
	sub	edx, [editor_tabsize]
	sub	eax, edx
	call	editor_step_forward
	pop	edx
	dec	edx
	jnz	.f7.findpos1
.f7.foundpos1:
	pop	esi ebx
	push	eax
	cmp	eax, [ebp + editor_data.curcol]
	jb	.f7.scrollleft
	sub	eax, [ebp + editor_data.curcol]
	sub	eax, [cur_width]
	jb	.f7.xset
	inc	eax
	inc	edx
	add	[ebp + editor_data.curcol], eax
	jmp	.f7.xset
.f7.scrollleft:
	inc	edx
	mov	[ebp + editor_data.curcol], eax
.f7.xset:
	pop	eax
	push	edx
	sub	eax, [ebp + editor_data.curcol]
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
	add	edi, [ebp + editor_data.cursor_y]
	push	edi
	inc	edi
	cmp	edi, [cur_height]
	pop	edi
	jae	.f7.newview
	mov	[ebp + editor_data.cursor_y], edi
	mov	[cursor_y], edi
	jmp	.f7.yset
.f7.newview:
	dec	edi
	mov	[ebp + editor_data.linedata_start + editor_line.block], esi
	mov	[ebp + editor_data.linedata_start + editor_line.offs], bx
	add	[ebp + editor_data.curline], edi
	xor	eax, eax
	inc	eax
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	call	editor_init_lines
	call	editor_test_cursor_y
	inc	dword [esp]
.f7.yset:
	pop	eax
	test	eax, eax
	jz	.redraw_status
	jmp	editor_draw_text

editor_cursor_left:
	cmp	[ebp + editor_data.cur_delta], -1
	jz	.in_text
	dec	[ebp + editor_data.cur_delta]
	cmp	[ebp + editor_data.cur_delta], -1
	jnz	.left_cursor
.in_text:
	mov	esi, [ebp + editor_data.cur_block]
	add	esi, ebp
	mov	ebx, [ebp + editor_data.cur_offs]
	call	editor_step_backward
	jc	.ret_clc
	mov	al, [esi + ebx]
	cmp	al, 13
	jz	.eol
	cmp	al, 10
	jz	.eol2
	cmp	al, 9
	jz	.tab
	sub	esi, ebp
	mov	[ebp + editor_data.cur_block], esi
	mov	[ebp + editor_data.cur_offs], ebx
.left_cursor:
	mov	eax, [ebp + editor_data.cursor_x]
	test	eax, eax
	jz	@f
	dec	eax
.set_cursor_x:
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
.ret_clc:
	clc
	ret
@@:
	dec	[ebp + editor_data.curcol]
	jmp	.ret_stc
.tab:
	mov	eax, [ebp + editor_data.cursor_x]
	test	eax, eax
	jz	@f
	dec	eax
	mov	[ebp + editor_data.cursor_x], eax
	mov	[cursor_x], eax
	call	editor_update_cur
	test	al, al
	jz	.ret_clc
.ret_stc:
	stc
	ret
@@:
	dec	[ebp + editor_data.curcol]
	call	editor_update_cur
	jmp	.ret_stc
.eol2:
	push	esi ebx
	call	editor_step_backward
	jc	@f
	cmp	byte [esi + ebx], 13
	jnz	@f
	pop	eax eax
	push	esi ebx
@@:
	pop	ebx esi
.eol:
	mov	eax, [ebp + editor_data.cursor_y]
	dec	eax
	jz	.scroll_left_toup
	push	0	; no full redraw
	mov	[ebp + editor_data.cursor_y], eax
	mov	[cursor_y], eax
	jmp	.scroll_left_toend
.scroll_left_toup:
	push	1
	pop	ecx
	call	editor_up_scroll
	push	1	; full redraw required
.scroll_left_toend:
	call	editor_end_scroll
	pop	eax
	adc	al, 0
	jnz	.ret_stc
	jmp	.ret_clc

editor_test_cursor_x:
	mov	ecx, [cur_width]
	dec	ecx
	mov	eax, [ebp + editor_data.cursor_x]
	sub	eax, ecx
	jbe	@f
	add	[ebp + editor_data.curcol], eax
	mov	[ebp + editor_data.cursor_x], ecx
	mov	[cursor_x], ecx
@@:
	ret

editor_test_cursor_y:
	mov	ecx, [ebp + editor_data.cursor_y]
	mov	edx, [cur_height]
	dec	edx
	dec	edx
	sub	ecx, edx
	ja	.scroll_down
	mov	ecx, [ebp + editor_data.curline]
	add	ecx, edx
	sub	ecx, [ebp + editor_data.numlines]
	ja	.scroll_up
.clc_ret:
	clc
	ret
.scroll_down:
	add	[ebp + editor_data.curline], ecx
	sub	[ebp + editor_data.cursor_y], ecx
	sub	[cursor_y], ecx
	call	editor_down_scroll
	stc
	ret
.scroll_up:
	cmp	ecx, [ebp + editor_data.curline]
	jb	@f
	mov	ecx, [ebp + editor_data.curline]
@@:
	jecxz	.clc_ret
	add	[ebp + editor_data.cursor_y], ecx
	add	[cursor_y], ecx
	call	editor_up_scroll
	stc
	ret

editor_OnExit:
	mov	edx, [ebp+editor_data.hPlugin]
	test	edx, edx
	jz	@f
	mov	ebx, [ebp+editor_data.hFile]
	call	close_handle_if_unused
@@:
	ret

find_in_file_dlg:
        mov     ebx, find_in_file_dlgdata
        mov     eax, [cur_width]
        sub     eax, 12
        mov     [ebx + dlgtemplate.width], eax
        dec     eax
        dec     eax
        mov     [ebx + FindInFileDlgdata.efind.x2], eax
        shr     eax, 1
        dec     eax
        dec     eax
        mov     [ebx + FindInFileDlgdata.search.x2], eax
        sub     eax, aSearchBLength-1
        mov     [ebx + FindInFileDlgdata.search.x1], eax
        add     eax, aSearchBLength+3
        mov     [ebx + FindInFileDlgdata.cnl.x1], eax
        add     eax, aCancelBLength - 1
        mov     [ebx + FindInFileDlgdata.cnl.x2], eax
        mov     byte [ebx + FindInFileDlgdata.efind.flags], 0xC
        and     byte [ebx + FindInFileDlgdata.search.flags], not 4
        and     byte [ebx + FindInFileDlgdata.cnl.flags], not 4
	and	byte [ebx + FindInFileDlgdata.case.flags], not 4
	and	byte [ebx + FindInFileDlgdata.whole.flags], not 4
        push    ebx
        call    DialogBox
	cmp	eax, find_in_file_dlgdata.search
	ret
