fat32_parse_dir:
; in: eax=directory cluster
; out: eax=entry cluster
	mov	bx, 900h
	mov	di, bx
	push	eax
	call	read_cluster
	mov	cx, word [cluster_size]
	shr	cx, 5		; div 20h
.scan_cluster:
	pop	eax
	cmp	byte [di], 0
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
	mov	ax, [di+14h]
	shl	eax, 10h
	mov	ax, [di+1Ah]
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
	or	byte [di], 20h
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

read_cluster:
; in: eax=cluster,bx->buffer
	and	eax, 0FFFFFFFh
	movzx	ecx, byte [50Dh]	; sects_per_clust
	mul	ecx
	add	eax, [data_start]
;	call	read
;	ret
	jmp	relative_read
next_cluster:
	mov	bx, 700h
; sector is 200h bytes long, one entry in FAT occupies 4 bytes => 80h entries in sector
	push	eax
	shr	eax, 7		; div 80h
	cmp	eax, [fat_cur_sector]
	jz	@f
	mov	[fat_cur_sector], eax
	add	eax, [fat_start]
	mov	cx, 1
	call	relative_read
@@:
	pop	eax
	and	eax, 7Fh
	mov	eax, [700h+eax*4]
	and	eax, 0FFFFFFFh
	cmp	eax, 0FFFFFF7h
	mov	si, bad_cluster_string
	jz	find_error_si
	ret
