; Copyright (c) 2008-2009, diamond
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

restore_usa:
; Update Sequence Array restore
; in: ds:bx -> USA-protected structure
	push	bx
	lea	di, [bx+1feh]
	mov	cx, [bx+6]
	add	bx, [bx+4]
	dec	cx
@@:
	mov	ax, [bx+2]
	mov	[di], ax
	inc	bx
	inc	bx
	add	di, 200h
	loop	@b
	pop	bx
	ret

find_attr:
; in: ds:di->file record, ax=attribute
; out: ds:di->attribute or di=0 if not found
	add	di, [di+14h]
.1:
; attributes' codes are formally dwords, but all of them fit in word
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
; in: ds:si->attribute, es:di->buffer
; out: es:di->buffer end
	pushad
	pop	di
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
	cmp	di, 0x8000 - 12
	jbe	.loop
..attr_overflow:
	mov	si, fragmented_string
	jmp	find_error_si
.done:
	xor	ax, ax
	stosw
	stosw
	push	di
	popad
	ret

load_attr:
; in: ax=attribute, ds:bx->base record
; out: if found: CF=0, attribute loaded to [freeattr], [freeattr] updated,
;	edx=size of attribute in bytes
; out: if not found: CF=1
	mov	di, [bp + freeattr - dat]
	push	ss
	pop	es
	mov	byte [es:di], 1
	inc	di
	cmp	di, 0x8000 - 12
	ja	..attr_overflow
	or	edx, -1		; file size is not known yet
; scan for attribute
	push	di
	mov	di, bx
	add	di, [di+14h]
@@:
	call	find_attr.1
	test	di, di
	jz	.notfound1
	cmp	byte [di+8], 0
	jnz	.nonresident
	mov	si, di
	pop	di
	push	ds
	jmp	.resident
.aux_resident:
	mov	ax, ds
	mov	si, di
	pop	di ds bx ds edx
	push	ss
	pop	es
	push	ds
	mov	ds, ax
; resident attribute
.resident:
	dec	di
	mov	al, 0
	stosb
	mov	ax, [si+10h]
	stosw
	push	di
	add	di, ax
	cmp	di, 0x8000 - 12
	pop	di
	ja	..attr_overflow
	movzx	edx, ax		; length of attribute
	xchg	ax, cx
	add	si, [si+14h]
	rep	movsb
	mov	[bp + freeattr - dat], di
	pop	ds
	ret
.nonresident:
; nonresident attribute
	cmp	dword [di+10h], 0
	jnz	@b
; read start of data
	mov	si, di
	mov	edx, [di+30h]	; size of attribute
	pop	di
	call	process_mcb_nonres
	sub	di, 4
	push	di
.notfound1:
	pop	di
	push	edx
; $ATTRIBUTE_LIST is always in base file record
	cmp	ax, 20h
	jz	.nofragmented
; try to load $ATTRIBUTE_LIST = 20h
	push	ax
	mov	ax, 20h
	push	[bp + freeattr - dat]
	mov	[bp + freeattr - dat], di
	push	di
	call	load_attr
	pop	di
	pop	[bp + freeattr - dat]
	pop	ax
	jc	.nofragmented
	push	ds bx
	pusha
	mov	si, di
	push	ss
	pop	ds
	push	0x8100
	pop	es
	xor	ecx, ecx
	mov	cl, 0x78
	xor	bx, bx
	push	es
	call	read_file_chunk
	pop	ds
	jc	..found_disk_error
	test	cx, cx
	jz	..attr_overflow
	popa
	push	ss
	pop	es
	xor	bx, bx
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
	dec	di
	cmp	di, [bp + freeattr - dat]
	lea	di, [di+1]
	jnz	.continue1
@@:
	push	ds di
	push	ax
	mov	eax, [bx+10h]
	mov	ecx, [bx+8]
	call	read_file_record
	pop	ax
	mov	di, [14h]
.2:
	call	find_attr.1
	cmp	byte [di+8], 0
	jz	.aux_resident
	cmp	dword [di+10h], ecx
	jnz	.2
	mov	si, di
	mov	di, sp
	cmp	dword [ss:di+8], -1
	jnz	@f
	push	dword [si+30h]	; size of attribute
	pop	dword [ss:di+8]
@@:
	pop	di
	call	process_mcb_nonres
	sub	di, 4
	pop	ds
.continue1:
	add	bx, [bx+4]
	cmp	bx, dx
	jb	.1
	pop	bx ds
.nofragmented:
	pop	edx
	dec	di
	cmp	di, [bp + freeattr - dat]
	jnz	@f
	stc
	ret
@@:
	inc	di
	xor	ax, ax
	stosw
	stosw
	mov	[bp + freeattr - dat], di
	ret

read_file_record:
; in: eax = index of record
; out: ds:0 -> record
; find place in cache
	push	di
	push	si
	mov	si, cache1head
	call	cache_lookup
	pop	si
	pushf
	sub	di, 3400h
	shl	di, 10-3
	add	di, 0x6000
	mov	ds, di
	popf
	pop	di
	jnc	.noread
; read file record <eax> to ds:0
	pushad
	push	ds
	push	es
	movzx	ecx, [bp + frs_size - dat]
	shr	cx, 9
	mul	ecx
	push	ds
	pop	es
	push	ss
	pop	ds
	mov	si, 0x4000
	xor	bx, bx
	push	[bp + cur_obj - dat]
	mov	[bp + cur_obj - dat], mft_string
	push	es
	call	read_attr
; initialize cache for $INDEX_ALLOCATION for this record
	pop	si
	push	si
	sub	si, 0x6000
	mov	ax, si
	shr	si, 10-3
	shr	ax, 2
	add	si, 3480h
	add	ax, 3500h
	mov	[si], si
	mov	[si+2], si
	mov	[si+4], ax
	pop	ds
	call	restore_usa
	pop	[bp + cur_obj - dat]
	pop	es
	pop	ds
	popad
.noread:
	ret

read_attr:
; in: eax = offset in sectors, ecx = size in sectors (<10000h), es:bx -> buffer, ds:si -> attribute
	push	invalid_read_request_string
	cmp	byte [si], 0
	jnz	.nonresident
	cmp	eax, 10000h shr 9
	jae	find_error_sp
	shl	ax, 9
	shl	cx, 9
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
	inc	si
.loop:
	mov	edx, dword [si]
	add	si, 8
	test	edx, edx
	jz	find_error_sp
	imul	edx, [bp + sect_per_clust - dat]
	sub	eax, edx
	jnc	.loop
	add	eax, edx
	sub	edx, eax
	push	cx
	cmp	ecx, edx
	jb	@f
	mov	cx, dx
@@:
	push	bx
	mov	ebx, [si-4]
	imul	ebx, [bp + sect_per_clust - dat]
	add	eax, ebx
	pop	bx
	call	read
	jc	..found_disk_error
	mov	dx, cx
	pop	cx
	xor	eax, eax
	sub	cx, dx
	jnz	.loop
	pop	ax
	ret

load_file_ntfs:
; in: ss:bp = 0:dat
; in: es:bx = address to load file
; in: ds:si -> ASCIIZ name
; in: cx = limit in sectors
; out: bx = status: bx=0 - ok, bx=1 - file is too big, only part has been loaded, bx=2 - file not found
; out: dx:ax = file size (0xFFFFFFFF if file not found)
	push	es bx cx
	mov	eax, 5		; root cluster
	mov	[bp + cur_obj - dat], root_string
.parse_dir_loop:
	push	ds si
	call	read_file_record
; find attributes $INDEX_ROOT, $INDEX_ALLOCATION, $BITMAP
	mov	ax, [bp + freeattr - dat]
	mov	[bp + index_root - dat], ax
	mov	ax, 90h		; $INDEX_ROOT
	xor	bx, bx
	call	load_attr
	mov	si, noindex_string
	jc	find_error_si
	mov	ax, [bp + freeattr - dat]
	mov	[bp + index_alloc - dat], ax
	mov	ax, 0A0h	; $INDEX_ALLOCATION
	call	load_attr
	jnc	@f
	mov	[bp + index_alloc - dat], bx
@@:
	push	ds
; search for entry
	mov	si, [bp + index_root - dat]
	push	ss
	pop	ds
	push	0x8100
	pop	es
	xor	ecx, ecx
	mov	cl, 0x78
	xor	bx, bx
	push	es
	call	read_file_chunk
	pop	ds
	jc	..found_disk_error
	test	cx, cx
	jz	..attr_overflow
	mov	si, invalid_read_request_string
	cmp	word [bx+10], 0
	jnz	find_error_si
; calculate number of items in cache
	mov	di, [bx+8]	; subnode_size
	mov	ax, 0x4000
	sub	ax, word [bp + frs_size - dat]
	cwd
	div	di
	test	ax, ax
	jz	find_error_si
	mov	si, invalid_volume_msg
	test	di, 0x1FF
	jnz	find_error_si
	pop	cx
	mov	[bp + cur_index_seg - dat], cx
	shl	ax, 3
	sub	cx, 6000h
	mov	si, cx
	shr	cx, 2
	shr	si, 10-3
	add	cx, ax
	add	si, 3480h
	mov	[bp + cur_index_cache - dat], si
	add	cx, 3500h
	mov	[ss:si+6], cx
	mov	dx, di
	add	bx, 10h
.scan_record:
	add	bx, [bx]
.scan:
	test	byte [bx+0Ch], 2
	jnz	.look_child
	movzx	cx, byte [bx+50h]	; namelen
	lea	di, [bx+52h]		; name
	push	ds
	pop	es
	pop	si ds
	push	ds si
	xor	ax, ax
.1:
	lodsb
	cmp	al, '/'
	jnz	@f
	mov	al, 0
@@:
	cmp	al, 'A'
	jb	.nocapital
	cmp	al, 'Z'
	ja	.nocapital
	or	al, 20h
.nocapital:
	cmp	al, 'a'
	jb	.notletter
	cmp	al, 'z'
	ja	.notletter
	or	byte [es:di], 20h
.notletter:
	scasw
	loopz	.1
	jb	.look_child
	ja	@f
	cmp	byte [si], 0
	jz	.file_found
	cmp	byte [si], '/'
	jz	.file_found
@@:
	push	es
	pop	ds
	add	bx, [bx+8]
	jmp	.scan
.look_child:
	push	es
	pop	ds
	test	byte [bx+0Ch], 1
	jz	.not_found
	mov	si, [bp + index_alloc - dat]
	test	si, si
	jz	.not_found
	add	bx, [bx+8]
	mov	eax, [bx-8]
	mov	es, [bp + cur_index_seg - dat]
	push	si
	mov	si, [bp + cur_index_cache - dat]
	call	cache_lookup
	pop	si
	pushf
	mov	bx, di
	mov	bh, 0
	shr	bx, 3
	imul	bx, dx
	add	bx, [bp + frs_size - dat]
	popf
	jnc	.noread
	push	es
	push	dx
	push	ss
	pop	ds
	movzx	ecx, dx
	shr	cx, 9
	mul	[bp + sect_per_clust - dat]
	call	read_attr
	pop	dx
	pop	es
	push	es
	pop	ds
	call	restore_usa
.noread:
	push	es
	pop	ds
	add	bx, 18h
	jmp	.scan_record
.not_found:
	pop	[bp + cur_obj - dat]
	mov	si, error_not_found
	jmp	find_error_si
.file_found:
	pop	[bp + cur_obj - dat]
	pop	cx
	mov	ax, [bp + index_root - dat]
	mov	[bp + freeattr - dat], ax
	mov	eax, [es:bx]
	test	byte [es:bx+48h+3], 10h
	jz	.regular_file
	cmp	byte [si], 0
	jz	..directory_error
	inc	si
	jmp	.parse_dir_loop
.regular_file:
	cmp	byte [si], 0
	jnz	..notdir_error
; read entry
	call	read_file_record
	xor	bx, bx
	mov	ax, 80h
	call	load_attr
	mov	si, nodata_string
	jc	find_error_si
	mov	si, [bp + index_root - dat]
	mov	[bp + freeattr - dat], si
	push	ss
	pop	ds
	jmp	load_file_common_end
