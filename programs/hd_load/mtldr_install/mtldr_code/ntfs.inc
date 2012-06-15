restore_usa:
; Update Sequence Array restore
	mov	bx, [di+4]
	mov	cx, [di+6]
	inc	bx
	add	bx, di
	inc	bx
	add	di, 1feh
	dec	cx
@@:
	mov	ax, [bx]
	stosw
	inc	bx
	inc	bx
	add	di, 1feh
	loop	@b
	ret

find_attr:
; in: di->file record, ax=attribute
; out: di->attribute or di=0 if not found
	add	di, [di+14h]
.1:
; attributes codes are formally dwords, but all they fit in word
	cmp	word [di], -1
	jz	.notfound
	cmp	word [di], ax
	jnz	.continue
; for $DATA attribute, scan only unnamed
	cmp	ax, 80h
	jnz	.found
	cmp	byte [di+9], 0
	jz	.found
.continue:
	add	di, [di+4]
	jmp	.1
.notfound:
	xor	di, di
.found:
	ret

process_mcb_nonres:
; in: si->attribute, es:di->buffer
; out: di->buffer end
	add	si, [si+20h]
	xor	ebx, ebx
.loop:
	lodsb
	test	al, al
	jz	.done
	push	invalid_read_request_string
	movzx	cx, al
	shr	cx, 4
	jz	find_error_sp
	xchg	ax, dx
	and	dx, 0Fh
	jz	find_error_sp
	add	si, cx
	add	si, dx
	pop	ax
	push	si
	dec	si
	movsx	eax, byte [si]
	dec	cx
	jz	.l1e
.l1:
	dec	si
	shl	eax, 8
	mov	al, [si]
	loop	.l1
.l1e:
	xchg	ebp, eax
	dec	si
	movsx	eax, byte [si]
	mov	cx, dx
	dec	cx
	jz	.l2e
.l2:
	dec	si
	shl	eax, 8
	mov	al, byte [si]
	loop	.l2
.l2e:
	pop	si
	add	ebx, ebp
; eax=length, ebx=disk block
	stosd
	mov	eax, ebx
	stosd
	jmp	.loop
.done:
	xor	eax, eax
	stosd
	ret

load_attr:
; in: ax=attribute, es:bx->buffer, di->base record
; out: bx->buffer end; CF set if not found
	push	di
	push	ax
	mov	byte [es:bx], 1
	inc	bx
	push	bx
	mov	[ofs], bx
; scan for attrubute
	add	di, [di+14h]
@@:
	call	find_attr.1
	test	di, di
	jz	.notfound1
	cmp	byte [di+8], 0
	jnz	.nonresident
	jmp	.resident
.aux_resident:
	push	di
	popad
; resident attribute
.resident:
	mov	si, di
	pop	di
	dec	di
	mov	al, 0
	stosb
	mov	ax, [si+10h]
	stosw
	xchg	ax, cx
	add	si, [si+14h]
	rep	movsb
	mov	bx, di
	pop	ax
	pop	di
	ret
.nonresident:
; nonresident attribute
	cmp	dword [di+10h], 0
	jnz	@b
; read start of data
	mov	si, di
	pop	di
	call	process_mcb_nonres
	sub	di, 4
	push	di
.notfound1:
; $ATTRIBUTE_LIST is always in base file record
	cmp	word [esp+2], 20h
	jz	.nofragmented
; scan for $ATTRIBUTE_LIST = 20h
	mov	di, [esp+4]
	mov	ax, 20h
	call	find_attr
	test	di, di
	jz	.nofragmented
; load $ATTRIBUTE_LIST itself
	push	es
	mov	bx, 0C000h
	mov	di, [esp+6]
	push	bx
	push	[ofs]
	push	ds
	pop	es
	call	load_attr
	pop	[ofs]
	pop	si
	mov	bx, 8000h
	push	bx
	push	si
	call	read_attr_full
	pop	si
	pop	bx
	add	dx, bx
	mov	ax, [esp+4]
	pop	es
.1:
	cmp	[bx], ax
	jnz	.continue1
; only unnamed $DATA attributes!
	cmp	ax, 80h
	jnz	@f
	cmp	byte [bx+6], 0
	jnz	.continue1
@@:
	cmp	dword [bx+10h], 0
	jz	.continue1
	cmp	dword [bx+8], 0
	jnz	@f
	push	ax
	mov	ax, [esp+2]
	cmp	ax, [ofs]
	pop	ax
	jnz	.continue1
@@:
	pushad
	mov	eax, [bx+10h]
	mov	bx, dx
	push	[ofs]
	push	es
	push	ds
	pop	es
	call	read_file_record
	pop	es
	pop	[ofs]
	popad
	pushad
	pop	di
	mov	di, dx
	add	di, [di+14h]
.2:
	call	find_attr.1
	cmp	byte [di+8], 0
	jz	.aux_resident
	mov	eax, [bx+8]
	cmp	eax, [di+10h]
	jnz	.2
	mov	si, di
	mov	di, [esp+1Eh]
	call	process_mcb_nonres
	sub	di, 4
	mov	[esp+1Eh], di
	push	di
	popad
.continue1:
	add	bx, [bx+4]
	cmp	bx, dx
	jb	.1
.nofragmented:
	pop	bx
	pop	ax
	pop	di
	cmp	bx, [ofs]
	jnz	@f
	dec	bx
	stc
	ret
@@:
	add	bx, 4
	ret

read_attr_full:
; in: si->decoded attribute data, bx->buffer
; out: edx=length in bytes
	lodsb
	cmp	al, 0
	jnz	.nonresident
; resident
	lodsw
	movzx	edx, ax
	xchg	ax, cx
	mov	di, bx
	rep	movsb
	ret
.nonresident:
; nonresident :-)
	xor	edx, edx
.loop:
	lodsd
	xchg	ecx, eax
	jecxz	.loopend
	lodsd
	xchg	edi, eax
; read ecx clusters from cluster edi to es:bx
.intloop:
	push	ecx
; read 1 cluster from physical cluster edi to es:bx
	mov	ecx, [cluster_size]
	mov	eax, edi
	mul	ecx
	push	bx
	call	relative_read
	pop	bx
	pop	ecx
	inc	edi
	mov	eax, [cluster_size]
	add	edx, eax
	shr	eax, 4
	mov	bp, es
	add	bp, ax
	mov	es, bp
	loop	.intloop
	jmp	.loop
.loopend:
	mov	es, cx
	ret

read_file_record:
; in: eax=index of record, bx=buffer
	mov	si, 700h
	mov	ecx, [frs_size]
	mul	ecx
	push	bx
	push	[cur_obj]
	mov	[cur_obj], mft_string
	call	read_attr
	pop	[cur_obj]
	pop	di
	call	restore_usa
	ret
read_attr:
; in: edx:eax=offset in bytes, ecx=size in bytes, bx=buffer, si=attribute
	push	invalid_read_request_string
	cmp	byte [si], 0
	jnz	.nonresident
	test	edx, edx
	jnz	find_error_sp
	cmp	eax, 10000h
	jae	find_error_sp
	cmp	ecx, 10000h
	jae	find_error_sp
	cmp	ax, [si+2]
	jae	find_error_sp
	cmp	cx, [si+2]
	ja	find_error_sp
	add	si, 3
	add	si, ax
	mov	di, bx
	rep	movsb
	pop	ax
	ret
.nonresident:
	mov	edi, [cluster_size]
	div	edi
	mov	[ofs], dx
	add	cx, dx
	push	eax
	xchg	eax, ecx
	xor	edx, edx
	dec	eax
	div	edi
	inc	eax
	xchg	eax, ecx
	pop	eax
	add	si, 1
	xor	edx, edx
	push	bx
; eax=offset in clusters, ecx=size in clusters
.scan:
	mov	ebx, [si]
	test	ebx, ebx
	jz	.notfound
	add	edx, ebx
	add	si, 8
	cmp	eax, edx
	jae	.scan
	mov	edi, [si-4]
; now edx=end of block, ebx=length of block, edi=start of block on disk
; eax=required offset, ecx=required length
	push	edx
	push	edi
	sub	edx, eax
	add	edi, ebx
	sub	edi, edx
	cmp	edx, ecx
	jb	@f
	mov	edx, ecx
@@:
; read (edx) clusters from (edi=disk offset in clusters) to ([esp+8])
	cmp	[ofs], 0
	jnz	.ofs_read
.cont:
	pushad
	movzx	ebx, byte [50Dh]
;       xchg    eax, edx
;       mul     ebx
	xchg	ax, dx
	mul	bx
	xchg	cx, ax
	xchg	eax, edi
	mul	ebx
	mov	bx, [esp+8+20h]
	call	relative_read
	mov	[esp+8+20h], bx
	popad
.cont2:
	add	eax, edx
	sub	ecx, edx
.cont3:
	pop	edi
	pop	edx
	jnz	.scan
	pop	bx
	pop	ax
	ret
.ofs_read:
	push	ecx
	movzx	ecx, byte [50Dh]	; bpb_sects_per_clust
	mov	eax, edi
	push	edx
	mul	ecx
	push	1000h
	pop	es
	xor	bx, bx
	call	relative_read
	mov	cx, bx
	push	si
	push	di
	mov	si, [ofs]
	mov	di, [esp+8+12]
	sub	cx, si
	push	ds
	push	es
	pop	ds
	pop	es
	rep	movsb
	mov	[esp+8+12], di
	push	es
	pop	ds
	pop	di
	pop	si
	pop	edx
	pop	ecx
	inc	edi
	mov	[ofs], 0
	inc	eax
	dec	ecx
	jz	.cont3
	dec	edx
	jnz	.cont
	jmp	.cont2
.notfound:
	mov	si, invalid_read_request_string
	jmp	find_error_si

ntfs_parse_dir:
; in: eax=directory iRecord, [word sp+2]=filename
; out: si=$DATA attribute of file
	mov	bx, [free]
	mov	[dir], bx
	push	bx
	call	read_file_record
	mov	ax, word [frs_size]
	add	[free], ax
	pop	di
; find attributes $INDEX_ROOT, $INDEX_ALLOCATION, $BITMAP
	mov	ax, 90h 	; $INDEX_ROOT
	push	di
	mov	bx, [free]
	mov	[index_root], bx
	call	load_attr
	mov	si, noindex_string
	jc	find_error_si
	mov	[free], bx
	pop	di
	mov	ax, 0A0h	; $INDEX_ALLOCATION
	mov	bx, [free]
	mov	[index_alloc], bx
	call	load_attr
	jnc	@f
	mov	[index_alloc], 0
@@:
	mov	[free], bx
; search for entry
	mov	si, [index_root]
	mov	bx, [free]
	call	read_attr_full
	mov	ebp, [bx+8]	; subnode_size
	add	bx, 10h
.scan_record:
	add	bx, [bx]
.scan:
	test	byte [bx+0Ch], 2
	jnz	.not_found
	mov	si, [esp+2]
	movzx	cx, byte [bx+50h]	; namelen
	lea	di, [bx+52h]		; name
	xor	ax, ax
@@:
	lodsb
	cmp	al, 'a'
	jb	.notletter
	cmp	al, 'z'
	ja	.notletter
	or	byte [di], 20h
.notletter:
	scasw
	loopz	@b
	jb	.not_found
	ja	@f
	cmp	byte [esi], 0
	jz	.file_found
@@:
	add	bx, [bx+8]
	jmp	.scan
.not_found:
	test	byte [bx+0Ch], 1
	jz	file_not_found
	cmp	[index_alloc], 0
	jz	file_not_found
	add	bx, [bx+8]
	mov	eax, [bx-8]
	mul	[cluster_size]
	mov	si, [index_alloc]
	mov	ecx, ebp
	mov	bx, [free]
	call	read_attr
	mov	di, [free]
	call	restore_usa
	mov	bx, [free]
	add	bx, 18h
	jmp	.scan_record
.file_found:
	mov	si, [esp+2]
	mov	[cur_obj], si
	cmp	byte [esp+4], 0
	jz	.need_file
	mov	si, notdir_string
	test	byte [bx+48h+3], 10h
	jz	find_error_si
	mov	eax, [bx]
	mov	bx, [dir]
	mov	[free], bx
	ret	2
.need_file:
	mov	si, directory_string
	test	byte [bx+48h+3], 10h	; directory?
	jnz	find_error_si
; read entry
	mov	eax, [bx]
	mov	bx, [dir]
	mov	[free], bx
	mov	bx, 4000h
	push	bx
	call	read_file_record
	pop	di
	mov	ax, 80h
	push	2000h
	pop	es
	xor	bx, bx
	call	load_attr
	mov	si, nodata_string
	jz	find_error_si
	mov	[free], bx
	ret	2
