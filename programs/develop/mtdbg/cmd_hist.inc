init_cmd_hist_buffer:
;-------------------------------------------------------------------------------
	push	eax ebx ecx

	mcall	68, 12, 4096	;allocate 4KB block
	mov	[cmd_hist_buffer.ptr], eax
	mov	[cmd_hist_buffer.size], ecx

	mov	edi, eax
	xor	eax, eax
	stosd
	stosd
	mov	[cmd_hist_buffer.new_node_offset], eax
	mov	[cmd_hist_buffer.last_node_offset], eax
	mov	[cmd_hist_buffer.cur_node_offset], eax
	mov	[cmd_hist_buffer.tmp_line_flag], al
	mov	[cmd_hist_buffer.n_flag], al

	pop	ecx ebx eax
	ret
;-------------------------------------------------------------------------------

enlarge_cmd_hist_buffer:
;-------------------------------------------------------------------------------
	pusha

	mov	ecx, [cmd_hist_buffer.size]
	add	ecx, 4096
	mov	edx, [cmd_hist_buffer.ptr]
	mcall	68, 20		;reallocate the buffer (prev. size + 4KB)
	test	eax, eax
	jz	.err
	mov	[cmd_hist_buffer.ptr], eax
	mov	[cmd_hist_buffer.size], ecx

.ok:
	clc
	popa
	ret

.err:
	stc
	popa
	ret
;-------------------------------------------------------------------------------

add_cmd_hist_line:
;-------------------------------------------------------------------------------
;in:
;eax = ptr to ASCIIZ string of line
;-------------------------------------------------------------------------------
	pusha

	mov	esi, eax
	mov	edx, eax
	xor	ecx, ecx

@@:
	inc	ecx
	lodsb
	test	al, al
	jz	@f
	jmp	@b

@@:
	dec	ecx
	jz	.ok

	inc	ecx

	mov	eax, [cmd_hist_buffer.new_node_offset]
	lea	eax, [eax+ecx+2*4]
	cmp	[cmd_hist_buffer.size], eax
	jae	@f
	call	enlarge_cmd_hist_buffer
	jc	.err

@@:
	mov	edi, [cmd_hist_buffer.new_node_offset]
	mov	eax, [cmd_hist_buffer.ptr]
	add	edi, eax
	cmp	eax, edi		;if eax = edi, then it's the first node
	jne	@f
	xor	eax, eax
	dec	eax
	stosd				;save previous node link = -1 for the first node
	stosd				;save next node link = -1 for the first node
	jmp	.cp_str

@@:
	mov	ebx, [cmd_hist_buffer.last_node_offset]
	mov	[edi], ebx		;save previous node link for a new node
	add	ebx, eax
	mov	eax, [cmd_hist_buffer.new_node_offset]
	mov	[ebx+4], eax		;update next node link for a last node
	mov	[cmd_hist_buffer.last_node_offset], eax
	mov	[cmd_hist_buffer.cur_node_offset], eax
	add	edi, 4
	xor	eax, eax
	dec	eax
	stosd

.cp_str:
	mov	esi, edx

	push	ecx
	rep	movsb
	pop	ecx

	add	ecx, 2*4
	add	[cmd_hist_buffer.new_node_offset], ecx


.ok:
	clc

.err:
	mov	[cmd_hist_buffer.tmp_line_flag], 0

	popa
	ret
;-------------------------------------------------------------------------------

add_cmd_hist_tmp_line:
;-------------------------------------------------------------------------------
;in:
;eax = ptr to ASCIIZ string of line
;-------------------------------------------------------------------------------
	pusha

	mov	esi, eax
	mov	edx, eax
	xor	ecx, ecx

@@:
	inc	ecx
	lodsb
	test	al, al
	jz	@f
	jmp	@b

@@:
	mov	eax, [cmd_hist_buffer.new_node_offset]
	lea	eax, [eax+ecx+2*4]
	cmp	[cmd_hist_buffer.size], eax
	jae	@f
	call	enlarge_cmd_hist_buffer
	jc	.err

@@:
	mov	edi, [cmd_hist_buffer.new_node_offset]
	mov	[cmd_hist_buffer.tmp_line_flag], 1
	mov	eax, [cmd_hist_buffer.ptr]
	add	edi, eax
	xor	eax, eax
	stosd
	stosd		;two zeroes - marker (prefix) of tmp line

.cp_str:
	mov	esi, edx

	rep	movsb

.ok:
	clc

.err:
	popa
	ret
;-------------------------------------------------------------------------------

get_cmd_hist_line:
;-------------------------------------------------------------------------------
;out:
;eax = ptr to ASCIIZ string of current line
;-------------------------------------------------------------------------------
	push	ebx

	mov	eax, [cmd_hist_buffer.ptr]
	mov	ebx, [cmd_hist_buffer.cur_node_offset]

	lea	eax, [eax + ebx + 2*4]

	pop	ebx
	ret
;-------------------------------------------------------------------------------

get_cmd_hist_tmp_line:
;-------------------------------------------------------------------------------
;out:
;eax = ptr to ASCIIZ string of current line or 0 (no tmp line)
;-------------------------------------------------------------------------------
	cmp	[cmd_hist_buffer.tmp_line_flag], 0
	je	@f

	mov	eax, [cmd_hist_buffer.new_node_offset]
	add	eax, [cmd_hist_buffer.ptr]
	add	eax, 2*4

@@:
	ret
;-------------------------------------------------------------------------------

prev_cmd_hist_line:
;-------------------------------------------------------------------------------
;out:
;CF = 0 - OK, CF = 1 (no previous lines)
;-------------------------------------------------------------------------------
	push	eax ebx

	mov	eax, [cmd_hist_buffer.ptr]
	mov	ebx, [cmd_hist_buffer.cur_node_offset]

	lea	eax, [eax+ebx]
	mov	eax, [eax]
	inc	eax
	jz	.ret_cf
	dec	eax
	mov	[cmd_hist_buffer.cur_node_offset], eax

	clc
	pop	ebx eax
	ret

.ret_cf:
	stc
	pop	ebx eax
	ret
;-------------------------------------------------------------------------------

next_cmd_hist_line:
;-------------------------------------------------------------------------------
;out:
;CF = 0 - OK, CF = 1 (no next lines)
;-------------------------------------------------------------------------------
	push	eax ebx

	mov	eax, [cmd_hist_buffer.ptr]
	mov	ebx, [cmd_hist_buffer.cur_node_offset]

	lea	eax, [eax+ebx]
	mov	eax, [eax+4]
	inc	eax
	jz	.ret_cf
	dec	eax
	mov	[cmd_hist_buffer.cur_node_offset], eax

	clc
	pop	ebx eax
	ret

.ret_cf:
	stc
	pop	ebx eax
	ret
;-------------------------------------------------------------------------------

