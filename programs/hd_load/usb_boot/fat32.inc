fat32_parse_dir:
; in: eax=directory cluster
; out: eax=entry cluster
	xor	bx, bx
	mov	di, bx
	push	eax
	call	read_cluster
	movzx	cx, byte [7C0Dh]
	shl	cx, 4
.scan_cluster:
	pop	eax
	cmp	byte [es:di], 0
	jz	file_not_found
	mov	si, [esp+2]
	push	eax
	call	fat_compare_name
	jz	.file_found
	and	di, not 1Fh
	add	di, 20h
	loop	.scan_cluster
	pop	eax
	call	next_cluster
	jnc	file_not_found
	jc	fat32_parse_dir
.file_found:
	pop	eax
	mov	si, [esp+2]
	mov	[cur_obj], si
	and	di, not 1Fh
	mov	si, directory_string
	mov	ax, [es:di+14h]
	and	ax, 0xFFF
	shl	eax, 10h
	mov	ax, [es:di+1Ah]
	test	eax, eax
	mov	si, nodata_string
	jz	find_error_si
	ret	2

fat_compare_name:
	push	cx
	mov	cx, 9
.scan:
	lodsb
	cmp	al, '.'
	jz	.ext
	cmp	al, 0
	jz	.nameend
	cmp	al, 'a'
	jb	.notletter
	cmp	al, 'z'
	ja	.notletter
	or	byte [es:di], 20h
.notletter:
	scasb
	loopz	.scan
.notfound:
	inc	cx	; to clear ZF flag
	pop	cx
	ret
.ext:
	mov	al, ' '
	dec	cx
	repz	scasb
	jnz	.notfound
	test	di, 1
	jnz	.notfound
	mov	cx, 4
	jmp	.scan
.nameend:
	mov	al, ' '
	dec	cx
	repz	scasb
	jnz	.notfound
	test	di, 1
	jnz	.file_found
	mov	cx, 3
	repz	scasb
	jnz	.notfound
.file_found:
	xor	cx, cx	; to set ZF flag
	pop	cx
	ret
