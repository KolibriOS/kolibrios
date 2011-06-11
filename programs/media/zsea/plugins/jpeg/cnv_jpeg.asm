;*****************************************************************************
; JPEG to RAW convert plugin - for zSea image viewer
; Copyright (c) 2008, 2009, Evgeny Grechnikov aka Diamond
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Evgeny Grechnikov ''AS IS'' AND ANY
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
; Some small changes (c) 2011 Marat Zakiyanov aka Mario79, aka Mario
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16

START:
	pushad
	finit
	mov	eax,dword [esp+36]
	mov	esi, [eax]	; esi -> JPEG data
	mov	ebp, [eax+12]	; ebp = file size
	mov	[_esp], esp
; initialize constant tables, if not yet
	cmp	[color_table_1+4], 0
	jnz	@f
	call	initialize_color_table
@@:
	xor	ebx, ebx	; ebx -> RAW data, not allocated yet
	mov	[dct_buffer], ebx
	mov	[_ebx], ebx
; check for SOI [Start-Of-Image] marker
	call	get_marker
	jc	.end
	cmp	al, 0xD8	; SOI?
	jz	.soi_ok
.end:
; general exit from the function
; for progressive mode: convert loaded DCT coefficients to image
	call	handle_progressive
; convert full-color images to RGB
	call	convert_to_rgb
	xor	eax, eax
	test	ebx, ebx
	jnz	@f
	inc	eax	; ebx=0 => bad image
@@:
.ret:
	mov	ecx, [esp+28]
	mov	[ecx+4], ebx	; save RAW data ptr
	mov	[ecx+8], eax	; save result
	popad
	ret	4
.soi_ok:
	mov	[restart_interval], ebx
	mov	[adobe_ycck], 0
; loop until start of frame (real data), parse markers
.markers_loop:
	call	get_marker
	jc	.end
; markers RSTn do not have parameters
; N.B. They can not exist in this part of JPEG, but let's be liberal :)
	cmp	al, 0xD0
	jb	@f
	cmp	al, 0xD8
	jb	.markers_loop
@@:
	cmp	al, 0xD9	; EOI? [invalid here]
	jz	.end
; ok, this is marker segment
; first word is length of the segment
	cmp	ebp, 2
	jb	.end
	xor	edx, edx
	mov	dl, [esi+1]
	mov	dh, [esi]	; edx = marker length, al = marker value
	sub	ebp, edx
	jb	.end
	cmp	al, 0xDB	; DQT?
	jz	.dqt
	cmp	al, 0xC4	; DHT?
	jz	.dht
	cmp	al, 0xCC	; DAC? [ignored - no arithmetic coding]
	jz	.next_marker
	cmp	al, 0xDD	; DRI?
	jz	.dri
	cmp	al, 0xDA	; SOS?
	jz	.sos
	cmp	al, 0xC0
	jb	@f
	cmp	al, 0xD0
	jb	.sofn
@@:
	cmp	al, 0xEE	; APP14?
	jz	.app14
; unrecognized marker; let's skip it and hope for the best
.next_marker:
	add	esi, edx
	jmp	.markers_loop
.app14:
; check for special Adobe marker
	cmp	dx, 14
	jb	.next_marker
	cmp	byte [esi+2], 'A'
	jnz	.next_marker
	cmp	dword [esi+3], 'dobe'
	jnz	.next_marker
	cmp	byte [esi+13], 2
	setz	[adobe_ycck]
	jmp	.next_marker
.dqt:
; DQT marker found
; length: 2 bytes for length field + 65 bytes per table
	sub	edx, 2
	jc	.end
	lodsw
.dqt_loop:
	test	edx, edx
	jz	.markers_loop
	sub	edx, 1+64
	jc	.end
	lodsb
; 8-bit DCT-based process shall not use a 16-bit precision quantization table.
	test	al, 0xF0
	jnz	.end
	and	eax, 3
	mov	[eax+quant_tables_defined], 1
	shl	eax, 8
	lea	edi, [eax+quant_tables]
	xor	ecx, ecx
@@:
	xor	eax, eax
	lodsb
	push	eax
	fild	dword [esp]
	pop	eax
	movzx	eax, byte [zigzag+ecx]
	add	eax, eax
	push	eax
	and	eax, 7*4
	fmul	dword [idct_pre_table+eax]
	pop	eax
	push	eax
	shr	eax, 3
	and	eax, 7*4
	fmul	dword [idct_pre_table+eax]
	pop	eax
	fstp	dword [edi+eax]
	inc	ecx
	cmp	ecx, 64
	jb	@b
	jmp	.dqt_loop
.dri:
; DRI marker found
	cmp	edx, 4		; length must be 4
	jnz	.end2
	movzx	eax, word [esi+2]
	xchg	al, ah
	mov	[restart_interval], eax
	jmp	.next_marker
.dht:
; DHT marker found
	sub	edx, 2
	jc	.end2
	lodsw
.dht_loop:
	test	edx, edx
	jz	.markers_loop
	sub	edx, 17
	jc	.end2
; next Huffman table; find place for it
	lodsb
	mov	edi, eax
	and	eax, 0x10
	and	edi, 3
	shr	eax, 2
	or	edi, eax
	mov	[dc_huffman_defined+edi], 1
;	shl	edi, 11
	imul	edi, max_hufftable_size
	add	edi, dc_huffman	; edi -> destination table
; get table size
	xor	eax, eax
	push	16
	pop	ecx
@@:
	add	al, [esi]
	adc	ah, 0
	inc	esi
	loop	@b
	cmp	ax, 0x100
	ja	.end2
	sub	edx, eax
	jc	.end2
; construct Huffman tree
	push	ebx edx
	; lea	eax, [edi+256*8]
	; push	eax
	; push	16
	; mov	edx, esi
; @@:
	; cmp	byte [edx-1], 0
	; jnz	@f
	; dec	edx
	; dec	dword [esp]
	; jmp	@b
; @@:
	; sub	edx, [esp]
	; lea	eax, [edi+8]
	; push	2
	; pop	ecx
; .lenloop:
	; mov	bl, byte [edx]
	; test	bl, bl
	; jz	.len1done
	; push	eax
	; xor	eax, eax
; .len1loop:
	; dec	ecx
	; js	.dhterr
	; cmp	edi, [esp+8]
	; jae	.dhterr
	; lodsb
	; stosd
	; dec	bl
	; jnz	.len1loop
	; pop	eax
; .len1done:
	; jecxz	.len2done
	; push	ecx
; .len2loop:
	; cmp	eax, [esp+8]
	; jb	@f
	; or	eax, -1
; @@:
	; cmp	edi, [esp+8]
	; jae	.dhterr
	; stosd
	; add	eax, 8
	; jnb	@f
	; or	eax, -1
; @@:
	; loop	.len2loop
	; pop	ecx
; .len2done:
	; add	ecx, ecx
	; inc	edx
	; dec	dword [esp]
	; jnz	.lenloop
	; pop	eax
	; pop	eax
	; sub	eax, edi
	; shr	eax, 2
	; cmp	eax, ecx
	; ja	@f
	; mov	ecx, eax
; @@:
	; or	eax, -1
	; rep	stosd
	; pop	edx ebx
	; jmp	.dht_loop
; .dhterr:
	; ;pop	eax eax eax edx ebx
	; add	esp, 5*4
	lea	eax, [edi+256*2]
	push	eax
	lea	edx, [esi-16]
	mov	ah, 1
	mov	ecx, 128
.dht_l1:
	movzx	ebx, byte [edx]
	inc	edx
	test	ebx, ebx
	jz	.dht_l3
.dht_l2:
	cmp	edi, [esp]
	jae	.dhterr1
	lodsb
	xchg	al, ah
	push	ecx
	rep	stosw
	pop	ecx
	xchg	al, ah
	dec	ebx
	jnz	.dht_l2
.dht_l3:
	inc	ah
	shr	ecx, 1
	jnz	.dht_l1
	push	edi
	mov	edi, [esp+4]
	push	edi
	mov	eax, 0x00090100
	mov	cl, 8
.dht_l4:
	movzx	ebx, byte [edx]
	inc	edx
	test	ebx, ebx
	jz	.dht_l6
.dht_l5:
	cmp	edi, [esp]
	jb	@f
	mov	edi, [esp+4]
	rol	eax, 16
	cmp	edi, [esp+8]
	jae	.dhterr2
	stosw
	inc	ah
	mov	[esp+4], edi
	pop	edi
	push	edi
	rol	eax, 16
	add	dword [esp], 16*2
@@:
	lodsb
	xchg	al, ah
	push	ecx
	rep	stosw
	pop	ecx
	xchg	al, ah
	dec	ebx
	jnz	.dht_l5
.dht_l6:
	inc	ah
	shr	ecx, 1
	jnz	.dht_l4
	push	edi
	movzx	ebx, byte [edx]
	add	ebx, ebx
	add	bl, [edx+1]
	adc	bh, 0
	add	ebx, ebx
	add	bl, [edx+2]
	adc	bh, 0
	add	ebx, ebx
	add	bl, [edx+3]
	adc	bh, 0
	add	ebx, 15
	shr	ebx, 4
	mov	cl, 8
	lea	ebx, [edi+ebx*2]
	sub	ebx, [esp+12]
	add	ebx, 31
	shr	ebx, 5
	mov	edi, ebx
	shl	edi, 5
	add	edi, [esp+12]
	xor	ebx, 9
	shl	ebx, 16
	xor	eax, ebx
	push	edi
.dht_l7:
	movzx	ebx, byte [edx]
	inc	edx
	test	ebx, ebx
	jz	.dht_l10
.dht_l8:
	cmp	edi, [esp]
	jb	.dht_l9
	mov	edi, [esp+4]
	cmp	edi, [esp+8]
	jb	@f
	mov	edi, [esp+12]
	cmp	edi, [esp+16]
	jae	.dhterr3
	mov	al, 9
	stosb
	rol	eax, 8
	stosb
	inc	eax
	ror	eax, 8
	mov	[esp+12], edi
	mov	edi, [esp+8]
	add	dword [esp+8], 16*2
@@:
	mov	al, 9
	stosb
	rol	eax, 16
	stosb
	inc	eax
	ror	eax, 16
	mov	[esp+4], edi
	pop	edi
	push	edi
	add	dword [esp], 16*2
.dht_l9:
	lodsb
	xchg	al, ah
	push	ecx
	rep	stosw
	pop	ecx
	xchg	al, ah
	dec	ebx
	jnz	.dht_l8
.dht_l10:
	inc	ah
	shr	ecx, 1
	jnz	.dht_l7
	push	-1
	pop	eax
	pop	ecx
	sub	ecx, edi
	rep	stosb
	pop	edi
	pop	ecx
	sub	ecx, edi
	rep	stosb
	pop	edi
	pop	ecx
	sub	ecx, edi
	rep	stosb
	pop	edx ebx
	jmp	.dht_loop
.dhterr3:
	pop	eax eax
.dhterr2:
	pop	eax eax
.dhterr1:
	pop	eax
	pop	edx ebx
.end2:
	jmp	.end
.sofn:
; SOFn marker found
	test	ebx, ebx
	jnz	.end2	; only one frame is allowed
; only SOF0 [baseline sequential], SOF1 [extended sequential], SOF2 [progressive]
; nobody supports other compression methods
	cmp	al, 0xC2
	ja	.end2
	setz	[progressive]
; Length must be at least 8
	sub	edx, 8
	jb	.end2
; Sample precision in JFIF must be 8 bits
	cmp	byte [esi+2], 8
	jnz	.end2
; Color space in JFIF is either YCbCr (color images, 3 components)
;                        or Y (grey images, 1 component)
	movzx	eax, byte [esi+7]
	cmp	al, 1
	jz	@f
	cmp	al, 3
	jz	@f
; Adobe products sometimes use YCCK color space with 4 components
	cmp	al, 4
	jnz	.end2
	cmp	[adobe_ycck], 0
	jz	.end2
@@:
	mov	edi, eax	; edi = number of components
	lea	eax, [eax*3]
	sub	edx, eax
	jnz	.end2
; get width and height
; width must be nonzero
; height must be nonzero - nobody supports DNL markers
	mov	ah, [esi+3]
	mov	al, [esi+4]	; eax = height
	test	eax, eax
	jz	.end2
	xor	ecx, ecx
	mov	ch, [esi+5]
	mov	cl, [esi+6]	; ecx = width
	jecxz	.end3
	push	eax ecx
	imul	ecx, eax
	cmp	ecx, 4000000h
	jb	@f
	pop	ecx eax
.end3:
	jmp	.end2
@@:
	imul	ecx, edi
	push	ecx
	add	ecx, 44+1
; for grayscale images, allocate additional memory for palette
	cmp	edi, 1
	jnz	@f
	add	ecx, 256*4-1
@@:
	push	68
	pop	eax
	push	12
	pop	ebx
	int	0x40
	mov	ebx, eax
	test	eax, eax
	jnz	@f
	pop	ecx
	mov	al, 2
	jmp	.ret
@@:
; OS zeroes all allocated memory
;	push	edx edi
;	mov	edi, eax
;	mov	edx, ecx
;	shr	ecx, 2
;	xor	eax, eax
;	rep	stosd
;	mov	ecx, edx
;	and	ecx, 3
;	rep	stosb
;	mov	ecx, edx
;	pop	edi edx
	mov	[_ebx], ebx
	pop	dword [ebx+32]		; size of pixels area
	push	44
	pop	eax
	mov	dword [ebx+28], eax	; pointer to RGB data
; create grayscale palette if needed
	cmp	edi, 1
	jnz	.no_create_palette
	mov	dword [ebx+20], eax
	mov	ecx, 256*4
	mov	dword [ebx+24], ecx
	add	dword [ebx+28], ecx
	push	edi
	shr	ecx, 2
	xor	eax, eax
	lea	edi, [ebx+44]
@@:
	stosd
	add	eax, 0x010101
	loop	@b
	pop	edi
.no_create_palette:
; other image characteristics
	pop	ecx eax
	mov	dword [ebx], 'RAW '	; signature
	mov	dword [ebx+4], ecx	; width
	mov	dword [ebx+8], eax	; height
	mov	eax, edi
	shl	eax, 3
	mov	dword [ebx+12], eax	; total pixel size
	mov	byte [ebx+16], 8	; 8 bits per component
	mov	word [ebx+18], di	; number of components
	mov	[delta_x], eax
	mov	[pixel_size], edi
	;mov	eax, edi
	imul	eax, ecx
	mov	[delta_y], eax
	shr	eax, 3
	mov	[line_size], eax
;	and	dword [ebx+36], 0	; transparency data pointer = NULL
;	and	dword [ebx+40], 0	; transparency data size
	add	esi, 8
	mov	ecx, edi
	mov	edi, components
	xor	eax, eax
	xor	edx, edx
.sof_parse_comp:
	movsb	; db ComponentIdentifier
	lodsb
	mov	ah, al
	and	al, 0xF
	jz	.end3
	shr	ah, 4
	jz	.end3
	stosd	; db V, db H, db ?, db ? (will be filled later)
	cmp	dl, al
	ja	@f
	mov	dl, al
@@:
	cmp	dh, ah
	ja	@f
	mov	dh, ah
@@:
	movsb	; db QuantizationTableID
	loop	.sof_parse_comp
	mov	word [max_v], dx
	movzx	eax, dh
	movzx	edx, dl
	push	eax edx
	shl	eax, 3
	shl	edx, 3
	mov	[block_width], eax
	mov	[block_height], edx
	pop	edx eax
	push	eax edx
	imul	eax, [delta_x]
	mov	[block_delta_x], eax
	imul	edx, [delta_y]
	mov	[block_delta_y], edx
	mov	eax, [ebx+4]
	add	eax, [block_width]
	dec	eax
	xor	edx, edx
	div	[block_width]
	mov	[x_num_blocks], eax
	mov	eax, [ebx+8]
	add	eax, [block_height]
	dec	eax
	xor	edx, edx
	div	[block_height]
	mov	[y_num_blocks], eax
	mov	cl, [ebx+18]
	pop	edx
	mov	edi, components
@@:
	mov	eax, edx
	div	byte [edi+1]	; VMax / V_i = VFactor_i
	mov	byte [edi+3], al	; db VFactor
	pop	eax
	push	eax
	div	byte [edi+2]	; HMax / H_i = HFactor_i
	mov	byte [edi+4], al	; db HFactor
	add	edi, 6
	loop	@b
	pop	eax
	cmp	[progressive], 0
	jz	.sof_noprogressive
	mov	eax, [x_num_blocks]
	mul	[block_width]
	mul	[y_num_blocks]
	mul	[block_height]
	add	eax, eax
	mov	[dct_buffer_size], eax
	mul	[pixel_size]
	xchg	eax, ecx
	push	ebx
	push	68
	pop	eax
	push	12
	pop	ebx
	int	0x40
	pop	ebx
	test	eax, eax
	jnz	@f
	mov	ecx, ebx
	push	68
	pop	eax
	push	13
	pop	ebx
	int	0x40
	xor	ebx, ebx
	jmp	.end
@@:
	mov	[dct_buffer], eax
.sof_noprogressive:
	jmp	.markers_loop
.sos:
; SOS marker found
; frame must be already opened
	test	ebx, ebx
	jz	.end3
	cmp	edx, 6
	jb	.end3
; parse marker
	movzx	eax, byte [esi+2]	; number of components in this scan
	test	eax, eax
	jz	.end3		; must be nonzero
	cmp	al, [ebx+18]
	ja	.end3		; must be <= total number of components
;	mov	[ns], eax
	cmp	al, 1
	setz	[not_interleaved]
	lea	ecx, [6+eax+eax]
	cmp	edx, ecx
	jnz	.end3
	mov	ecx, eax
	mov	edi, cur_components
	add	esi, 3
.sos_find_comp:
	lodsb	; got ComponentID, look for component info
	push	ecx esi
	mov	cl, [ebx+18]
	mov	esi, components
	and	dword [edi+48], 0
	and	dword [edi+52], 0
@@:
	cmp	[esi], al
	jz	@f
	inc	dword [edi+52]
	add	esi, 6
	loop	@b
@@:
	mov	eax, [esi+1]
	mov	dl, [esi+5]
	pop	esi ecx
	jnz	.end3	; bad ComponentID
	cmp	[not_interleaved], 0
	jz	@f
	mov	ax, 0x0101
@@:
	stosd		; db V, db H, db VFactor, db HFactor
	xor	eax, eax
	mov	al, byte [edi-1]	; get HFactor
	mul	byte [ebx+18]		; number of components
	stosd			; HIncrement_i = HFactor_i * sizeof(pixel)
	mov	al, byte [edi-4-2]	; get VFactor
	mul	byte [ebx+18]		; number of components
	imul	eax, [ebx+4]		; image width
	stosd			; VIncrement_i = VFactor_i * sizeof(row)
	xchg	eax, edx
	and	eax, 3
	cmp	[quant_tables_defined+eax], 0
	jz	.end3
	shl	eax, 8
	add	eax, quant_tables
	stosd		; dd QuantizationTable
	lodsb
	movzx	eax, al
	mov	edx, eax
	shr	eax, 4
	and	edx, 3
	and	eax, 3
	cmp	[dc_huffman_defined+eax], 0
	jnz	.dc_table_ok
	cmp	[progressive], 0
	jz	.end3
	xor	eax, eax
	jmp	.dc_table_done
.dc_table_ok:
;	shl	eax, 11
	imul	eax, max_hufftable_size
	add	eax, dc_huffman
.dc_table_done:
	cmp	[ac_huffman_defined+edx], 0
	jnz	.ac_table_ok
	cmp	[progressive], 0
	jz	.end3
	xor	edx, edx
	jmp	.ac_table_done
.ac_table_ok:
;	shl	edx, 11
	imul	edx, max_hufftable_size
	add	edx, ac_huffman
.ac_table_done:
	stosd		; dd DCTable
	xchg	eax, edx
	stosd		; dd ACTable
	push	ecx
	mov	eax, [ebx+4]
	movzx	ecx, byte [edi-21]	; get HFactor
	cdq	; edx:eax = width (width<0x10000, so as dword it is unsigned)
	div	ecx
	stosd		; dd width / HFactor_i
	stosd
	xchg	eax, ecx
	inc	eax
	sub	eax, edx
	stosd		; dd HFactor_i+1 - (width % HFactor_i)
	mov	eax, [ebx+8]
	movzx	ecx, byte [edi-34]	; get VFactor
	cdq
	div	ecx
	stosd		; dd height / VFactor_i
	stosd
	xchg	eax, ecx
	inc	eax
	sub	eax, edx
	stosd		; dd VFactor_i+1 - (height % VFactor_i)
	pop	ecx
	scasd		; dd DCPrediction
	cmp	dword [edi], 0
	setnp	al
	ror	al, 1
	mov	byte [edi-1], al
	scasd		; dd ComponentOffset
	dec	ecx
	jnz	.sos_find_comp
	mov	[cur_components_end], edi
	mov	edi, ScanStart
	movsb
	cmp	byte [esi], 63
	ja	.end3
	movsb
	lodsb
	push	eax
	and	al, 0xF
	stosb
	pop	eax
	shr	al, 4
	stosb
; now unpack data
	call	init_limits
	and	[decoded_MCUs], 0
	mov	[cur_rst_marker], 7
	and	[huffman_bits], 0
	cmp	[progressive], 0
	jz	.sos_noprogressive
; progressive mode - only decode DCT coefficients
; initialize pointers to coefficients data
; zero number of EOBs for AC coefficients
; redefine HIncrement and VIncrement
	mov	edi, cur_components
.coeff_init:
	mov	eax, [dct_buffer_size]
	mul	dword [edi+52]
	add	eax, [dct_buffer]
	mov	[edi+12], eax
	and	dword [edi+52], 0
	cmp	[ScanStart], 0
	jz	.scan_dc
	cmp	dword [edi+20], 0
	jz	.end3
	jmp	@f
.scan_dc:
	cmp	dword [edi+16], 0
	jz	.end3
@@:
	movzx	eax, byte [edi+1]
	shl	eax, 7
	mov	[edi+4], eax
	mov	eax, [edi+28]
	mov	cl, [edi+3]
	cmp	cl, [edi+32]
	sbb	eax, -7-1
	shr	eax, 3
	shl	eax, 7
	mov	[edi+8], eax
	add	edi, 56
	cmp	edi, [cur_components_end]
	jb	.coeff_init
; unpack coefficients
; N.B. Speed optimization has sense here.
	push	ebx
.coeff_decode_loop:
	mov	edx, cur_components
.coeff_components_loop:
	mov	edi, [edx+12]
	movzx	ecx, byte [edx]
	push	dword [edx+40]
	push	edi
.coeff_y_loop:
	push	ecx
	movzx	eax, byte [edx+1]
	push	dword [edx+28]
	push	edi
.coeff_x_loop:
	cmp	dword [edx+40], 0
	jl	@f
	cmp	dword [edx+28], 0
	jge	.realdata
@@:
	cmp	[not_interleaved], 0
	jnz	.norealdata
	push	eax edi
	mov	edi, dct_coeff
	call	decode_progressive_coeff
	pop	edi eax
	jmp	.norealdata
.realdata:
	push	eax
	call	decode_progressive_coeff
	add	edi, 64*2
	pop	eax
.norealdata:
	sub	dword [edx+28], 8
	sub	eax, 1
	jnz	.coeff_x_loop
	pop	edi
	pop	dword [edx+28]
	add	edi, [edx+8]
	pop	ecx
	sub	dword [edx+40], 8
	sub	ecx, 1
	jnz	.coeff_y_loop
	movzx	eax, byte [edx+1]
	shl	eax, 3
	pop	edi
	add	edi, [edx+4]
	pop	dword [edx+40]
	sub	[edx+28], eax
	mov	[edx+12], edi
	add	edx, 56
	cmp	edx, [cur_components_end]
	jnz	.coeff_components_loop
	call	next_MCU
	jc	.norst
	sub	[cur_x], 1
	jnz	.coeff_decode_loop
	call	next_line
	mov	edx, cur_components
@@:
	mov	eax, [max_x]
	imul	eax, [edx+4]
	sub	[edx+12], eax
	movzx	eax, byte [edx]
	imul	eax, [edx+8]
	add	[edx+12], eax
	add	edx, 56
	cmp	edx, [cur_components_end]
	jnz	@b
	sub	[cur_y], 1
	jnz	.coeff_decode_loop
	pop	ebx
	jmp	.markers_loop
.norst:
	pop	ebx
	jmp	.end4
.sos_noprogressive:
; normal mode - unpack JPEG image
	mov	edi, [ebx+28]
	add	edi, ebx
	mov	[cur_out_ptr], edi
; N.B. Speed optimization has sense here.
.decode_loop:
	call	decode_MCU
	call	next_MCU
	jc	.end4
	sub	[cur_x], 1
	jnz	.decode_loop
	call	next_line
	sub	[cur_y], 1
	jnz	.decode_loop
	jmp	.markers_loop
.end4:
	jmp	.end3
;---------------------------------------------------------------------
get_marker:
; in: esi -> data
; out: CF=0, al=marker value - ok
;      CF=1 - no marker
	sub	ebp, 1
	jc	.ret
	lodsb
if 1
	cmp	al, 0xFF
	jae	@f
; Some stupid men, which do not read specifications and manuals,
; sometimes create markers with length field two less than true
; value (in JPEG length of marker = length of data INCLUDING
; length field itself). To open such files, allow 2 bytes
; before next marker.
	cmp	ebp, 2
	jb	.ret
	lodsb
	lodsb
end if
	cmp	al, 0xFF
	jb	.ret
@@:
	sub	ebp, 1
	jc	.ret
	lodsb
	cmp	al, 0xFF
	jz	@b
	clc
.ret:
	ret
;---------------------------------------------------------------------
align 16
decode_MCU:
	mov	edx, cur_components
.components_loop:
; decode each component
	push	[cur_out_ptr]
	movzx	ecx, byte [edx]
	push	dword [edx+40]
; we have H_i * V_i blocks of packed data, decode them
.y_loop_1:
	push	[cur_out_ptr]
	push	ecx
	movzx	eax, byte [edx+1]
	push	dword [edx+28]
.x_loop_1:
	push	eax
	call	decode_data_unit
	cmp	dword [edx+40], 0
	jl	.nocopyloop
	cmp	dword [edx+28], 0
	jl	.nocopyloop
; now we have decoded block 8*8 in decoded_data
; H_i * V_i packed blocks 8*8 make up one block (8*HMax) * (8*VMax)
; so each pixel in packed block corresponds to HFact * VFact pixels
	movzx	ecx, byte [edx+2]
	push	esi ebp
	mov	edi, [cur_out_ptr]
	add	edi, [edx+52]
.y_loop_2:
	push	ecx edi
	cmp	ecx, [edx+44]
	mov	ecx, [edx+40]
	sbb	ecx, 8-1
	sbb	eax, eax
	and	ecx, eax
	add	ecx, 8
	jz	.skip_x_loop_2
	movzx	eax, byte [edx+3]
.x_loop_2:
	push	eax ecx edi
	cmp	eax, [edx+32]
	mov	eax, [edx+28]
	sbb	eax, 8-1
	sbb	ebp, ebp
	and	eax, ebp
	mov	ebp, .copyiter_all
	mov	esi, decoded_data
	sub	ebp, eax
	sub	ebp, eax
	sub	ebp, eax
	mov	eax, [edx+4]
	sub	eax, 1
.copyloop:
	push	esi edi
	jmp	ebp
.copyiter_all:
	movsb
repeat 7
	add	edi, eax
	movsb
end repeat
	nop
	nop
	pop	edi esi
	add	edi, [edx+8]
	add	esi, 8
	sub	ecx, 1
	jnz	.copyloop
	pop	edi ecx eax
	add	edi, [pixel_size]
	sub	eax, 1
	jnz	.x_loop_2
.skip_x_loop_2:
	pop	edi ecx
	add	edi, [line_size]
	sub	ecx, 1
	jnz	.y_loop_2
	pop	ebp esi
.nocopyloop:
	mov	eax, [delta_x]
	add	[cur_out_ptr], eax
	pop	eax
	sub	dword [edx+28], 8
	sub	eax, 1
	jnz	.x_loop_1
	pop	dword [edx+28]
	pop	ecx
	pop	eax
	sub	dword [edx+40], 8
	add	eax, [delta_y]
	mov	[cur_out_ptr], eax
	sub	ecx, 1
	jnz	.y_loop_1
	movzx	eax, byte [edx+1]
	pop	dword [edx+40]
	shl	eax, 3
	pop	[cur_out_ptr]
	sub	dword [edx+28], eax
	add	edx, 56
	cmp	edx, [cur_components_end]
	jb	.components_loop
	mov	eax, [cur_block_dx]
	add	[cur_out_ptr], eax
	ret

align 16
next_MCU:
	add	[decoded_MCUs], 1
	mov	eax, [restart_interval]
	test	eax, eax
	jz	.no_restart
	cmp	[decoded_MCUs], eax
	jb	.no_restart
	and	[decoded_MCUs], 0
	and	[huffman_bits], 0
	cmp	[cur_x], 1
	jnz	@f
	cmp	[cur_y], 1
	jz	.no_restart
@@:
; restart marker must be present
	sub	ebp, 2
	js	.error
	cmp	byte [esi], 0xFF
	jnz	.error
	mov	al, [cur_rst_marker]
	inc	eax
	and	al, 7
	mov	[cur_rst_marker], al
	add	al, 0xD0
	cmp	[esi+1], al
	jnz	.error
	add	esi, 2
; handle restart marker - zero all DC predictions
	mov	edx, cur_components
@@:
	and	word [edx+48], 0
	add	edx, 56
	cmp	edx, [cur_components_end]
	jb	@b
.no_restart:
	clc
	ret
.error:
	stc
	ret

next_line:
	mov	eax, [max_x]
	mov	[cur_x], eax
	mul	[cur_block_dx]
	sub	eax, [cur_block_dy]
	sub	[cur_out_ptr], eax
	mov	edx, cur_components
@@:
	mov	eax, [edx+24]
	mov	[edx+28], eax
	movzx	eax, byte [edx]
	shl	eax, 3
	sub	[edx+40], eax
	add	edx, 56
	cmp	edx, [cur_components_end]
	jb	@b
	ret

init_limits:
	push	[x_num_blocks]
	pop	[max_x]
	push	[y_num_blocks]
	pop	[max_y]
	push	[block_delta_x]
	pop	[cur_block_dx]
	push	[block_delta_y]
	pop	[cur_block_dy]
	cmp	[not_interleaved], 0
	jz	@f
	mov	eax, dword [cur_components+28]
	movzx	ecx, byte [cur_components+3]
	cmp	cl, [cur_components+32]
	sbb	eax, -7-1
	shr	eax, 3
	mov	[max_x], eax
	mov	eax, dword [cur_components+40]
	movzx	edx, byte [cur_components+2]
	cmp	dl, [cur_components+44]
	sbb	eax, -7-1
	shr	eax, 3
	mov	[max_y], eax
	imul	ecx, [delta_x]
	mov	[cur_block_dx], ecx
	imul	edx, [delta_y]
	mov	[cur_block_dy], edx
@@:
	push	[max_x]
	pop	[cur_x]
	push	[max_y]
	pop	[cur_y]
	ret

;macro get_bit
;{
;local .l1,.l2,.marker
;	add	cl, cl
;	jnz	.l1
;	sub	ebp, 1
;	js	decode_data_unit.eof
;	mov	cl, [esi]
;	cmp	cl, 0xFF
;	jnz	.l2
;.marker:
;	add	esi, 1
;	sub	ebp, 1
;	js	decode_data_unit.eof
;	cmp	byte [esi], 0xFF
;	jz	.marker
;	cmp	byte [esi], 0
;	jnz	decode_data_unit.eof
;.l2:
;	sub	esi, -1
;	adc	cl, cl
;.l1:
;}
macro get_bit
{
local .l1,.l2,.marker
	sub	cl, 1
	jns	.l1
	sub	ebp, 1
	js	.eof
	mov	ch, [esi]
	cmp	ch, 0xFF
	jnz	.l2
.marker:
	add	esi, 1
	sub	ebp, 1
	js	.eof
	cmp	byte [esi], 0xFF
	jz	.marker
	cmp	byte [esi], 0
	jnz	.eof
.l2:
	add	esi, 1
	mov	cl, 7
.l1:
	add	ch, ch
}
macro get_bits restore_edx
{
local .l1,.l2,.l3,.marker2
	movzx	eax, ch
	mov	dl, cl
	shl	eax, 24
	neg	cl
	push	ebx
	add	cl, 24
.l1:
	cmp	bl, dl
	jbe	.l2
	sub	bl, dl
	sub	ebp, 1
	js	.eof
	mov	ch, [esi]
	cmp	ch, 0xFF
	jnz	.l3
.marker2:
	add	esi, 1
	sub	ebp, 1
	js	.eof
	cmp	byte [esi], 0xFF
	jz	.marker2
	cmp	byte [esi], 0
	jnz	.eof
.l3:
	movzx	edx, ch
	add	esi, 1
	shl	edx, cl
	sub	cl, 8
	or	eax, edx
	mov	dl, 8
	jmp	.l1
.l2:
	mov	cl, bl
	sub	dl, bl
	shl	ch, cl
	pop	ebx
	cmp	eax, 80000000h
	rcr	eax, 1
	mov	cl, 31
	sub	cl, bl
	sar	eax, cl
	mov	cl, dl
if restore_edx eq true
	pop	edx
end if
	add	eax, 80000000h
	adc	eax, 80000000h
}
; macro get_huffman_code
; {
; local .l1
	; xor	ebx, ebx
; .l1:
	; get_bit
	; adc	ebx, ebx
	; mov	eax, [eax+4*ebx]
	; xor	ebx, ebx
	; cmp	eax, -1
	; jz	.eof
	; cmp	eax, 0x1000
	; jae	.l1
	; mov	ebx, eax
; }
macro get_huffman_code
{
local .l1,.l2,.l3,.l4,.l5,.l6,.nomarker1,.marker1,.nomarker2,.marker2,.nomarker3,.marker3,.done
; 1. (First level in Huffman table) Does the current Huffman code fit in 8 bits
; and have we got enough bits?
	movzx	ebx, ch
	cmp	byte [eax+ebx*2], cl
	jbe	.l1
; 2a. No; load next byte
	sub	ebp, 1
	js	.eof
	mov	ch, [esi]
	movzx	edx, ch
	cmp	ch, 0xFF
	jnz	.nomarker1
.marker1:
	add	esi, 1
	sub	ebp, 1
	js	.eof
	cmp	byte [esi], 0xFF
	jz	.marker1
	cmp	byte [esi], 0
	jnz	.eof
.nomarker1:
	shr	edx, cl
	add	esi, 1
	or	ebx, edx
; 3a. (First level in Huffman table, >=8 bits known) Does the current Huffman code fit in 8 bits?
	cmp	byte [eax+ebx*2], 8
	jbe	.l2
	jl	.eof
; 4aa. No; go to next level
	movzx	ebx, byte [eax+ebx*2+1]
	mov	dl, ch
	shl	ebx, 5
	ror	edx, cl
	lea	ebx, [eax+ebx+0x200]
	shr	edx, 24
	push	edx
	shr	edx, 4
; 5aa. (Second level in Huffman table) Does the current Huffman code fit in 12 bits
; and have we got enough bits?
	cmp	byte [ebx+edx*2], cl
	jbe	.l3
; 6aaa. No; have we got 12 bits?
	cmp	cl, 4
	jae	.l4
; 7aaaa. No; load next byte
	pop	edx
	sub	ebp, 1
	js	.eof
	mov	ch, [esi]
	cmp	ch, 0xFF
	jnz	.nomarker2
.marker2:
	add	esi, 1
	sub	ebp, 1
	js	.eof
	cmp	byte [esi], 0xFF
	jz	.marker2
	cmp	byte [esi], 0
	jnz	.eof
.nomarker2:
	push	ecx
	shr	ch, cl
	add	esi, 1
	or	dl, ch
	pop	ecx
	push	edx
	shr	edx, 4
; 8aaaa. (Second level in Huffman table) Does the current Huffman code fit in 12 bits?
	cmp	byte [ebx+edx*2], 4
	jbe	.l5
	jl	.eof
; 9aaaaa. No; go to next level
	movzx	ebx, byte [ebx+edx*2+1]
	pop	edx
	shl	ebx, 5
	and	edx, 0xF
	lea	ebx, [eax+ebx+0x200]
; 10aaaaa. Get current code length and value
	sub	cl, [ebx+edx*2]
	movzx	eax, byte [ebx+edx*2+1]
	neg	cl
	shl	ch, cl
	neg	cl
	add	cl, 8
	jmp	.done
.l5:
; 9aaaab. Yes; get current code length and value
	sub	cl, [ebx+edx*2]
	movzx	eax, byte [ebx+edx*2+1]
	neg	cl
	pop	edx
	shl	ch, cl
	neg	cl
	add	cl, 8
	jmp	.done
.l4:
; 7aaab. Yes; go to next level
	movzx	ebx, byte [ebx+edx*2+1]
	pop	edx
	shl	ebx, 5
	and	edx, 0xF
	lea	ebx, [eax+ebx+0x200]
; 8aaab. (Third level in Huffman table) Have we got enough bits?
	cmp	[ebx+edx*2], cl
	jbe	.l6
; 9aaaba. No; load next byte
	sub	ebp, 1
	js	.eof
	mov	ch, [esi]
	cmp	ch, 0xFF
	jnz	.nomarker3
.marker3:
	add	esi, 1
	sub	ebp, 1
	js	.eof
	cmp	byte [esi], 0xFF
	jz	.marker3
	cmp	byte [esi], 0
	jnz	.eof
.nomarker3:
	push	ecx
	shr	ch, cl
	add	esi, 1
	or	dl, ch
	pop	ecx
; 10aaaba. Get current code length and value
	sub	cl, [ebx+edx*2]
	movzx	eax, byte [ebx+edx*2+1]
	neg	cl
	shl	ch, cl
	neg	cl
	add	cl, 8
	jmp	.done
.l3:
; 6aab. Yes; get current code length and value
	pop	eax
.l6:
; 9aaabb. Yes; get current code length and value
	sub	cl, [ebx+edx*2]
	movzx	eax, byte [ebx+edx*2+1]
	xor	cl, 7
	shl	ch, cl
	xor	cl, 7
	add	ch, ch
	jmp	.done
.l2:
; 3ab. Yes; get current code length and value
	sub	cl, [eax+ebx*2]
	movzx	eax, byte [eax+ebx*2+1]
	neg	cl
	shl	ch, cl
	neg	cl
	add	cl, 8
	jmp	.done
.l1:
; 3b. Yes; get current code length and value
	mov	dl, [eax+ebx*2]
	movzx	eax, byte [eax+ebx*2+1]
	xchg	cl, dl
	sub	dl, cl
	shl	ch, cl
	mov	cl, dl
.done:
	mov	ebx, eax
}
; Decode DCT coefficients for one 8*8 block in progressive mode
; from input stream, given by pointer esi and length ebp
; N.B. Speed optimization has sense here.
align 16
decode_progressive_coeff:
	mov	ecx, [huffman_bits]
	cmp	[ScanStart], 0
	jnz	.ac
; DC coefficient
	cmp	[ApproxPosHigh], 0
	jz	.dc_first
; DC coefficient, subsequent passes
	xor	eax, eax
	get_bit
	adc	eax, eax
	mov	[huffman_bits], ecx
	mov	cl, [ApproxPosLow]
	shl	eax, cl
	or	[edi], ax
	ret
.dc_first:
; DC coefficient, first pass
	mov	eax, [edx+16]
	push	edx
	get_huffman_code
	get_bits true
	add	eax, [edx+48]
	mov	[edx+48], ax
	mov	[huffman_bits], ecx
	mov	cl, [ApproxPosLow]
	shl	eax, cl
	mov	[edi], ax
	ret
.ac:
; AC coefficients
	movzx	eax, [ScanStart]
	cmp	al, [ScanEnd]
	ja	.ret
	cmp	dword [edx+52], 0
	jnz	.was_eob
.acloop:
	push	edx
	push	eax
	mov	eax, [edx+20]
	get_huffman_code
	pop	eax
	test	ebx, 15
	jz	.band
	push	eax ebx
	and	ebx, 15
	get_bits false
	pop	ebx
	xchg	eax, [esp]
	shr	ebx, 4
.zeroloop1:
	push	eax ebx
	movzx	eax, byte [zigzag+eax]
	xor	ebx, ebx
	cmp	word [edi+eax], bx
	jz	.zeroloop2
	get_bit
	jnc	@f
	push	ecx
	mov	cl, [ApproxPosLow]
	xor	ebx, ebx
	cmp	byte [edi+eax+1], 80h
	adc	ebx, 0
	add	ebx, ebx
	sub	ebx, 1
	shl	ebx, cl
	pop	ecx
	add	[edi+eax], bx
@@:
	pop	ebx eax
@@:
	add	eax, 1
	cmp	al, [ScanEnd]
	ja	decode_data_unit.eof
	jmp	.zeroloop1
.zeroloop2:
	pop	ebx eax
	sub	ebx, 1
	jns	@b
.nozero1:
	pop	ebx
	test	ebx, ebx
	jz	@f
	push	eax
	movzx	eax, byte [zigzag+eax]
	push	ecx
	mov	cl, [ApproxPosLow]
	shl	ebx, cl
	pop	ecx
	mov	[edi+eax], bx
	pop	eax
@@:
	add	eax, 1
	pop	edx
	cmp	al, [ScanEnd]
	jbe	.acloop
	mov	[huffman_bits], ecx
.ret:
	ret
.eof:
	jmp	decode_data_unit.eof
.band:
	shr	ebx, 4
	cmp	ebx, 15
	jnz	.eob
	push	0
	jmp	.zeroloop1
.eob:
	pop	edx
	push	eax
	mov	eax, 1
	test	ebx, ebx
	jz	.eob0
@@:
	get_bit
	adc	eax, eax
	sub	ebx, 1
	jnz	@b
.eob0:
	mov	[edx+52], eax
	pop	eax
.was_eob:
	sub	dword [edx+52], 1
	cmp	al, [ScanEnd]
	ja	.ret2
.zeroloop3:
	push	eax
	movzx	eax, byte [zigzag+eax]
	xor	ebx, ebx
	cmp	word [edi+eax], bx
	jz	@f
	get_bit
	jnc	@f
	push	ecx
	mov	cl, [ApproxPosLow]
	xor	ebx, ebx
	cmp	byte [edi+eax+1], 80h
	adc	ebx, 0
	add	ebx, ebx
	sub	ebx, 1
	shl	ebx, cl
	pop	ecx
	add	[edi+eax], bx
@@:
	pop	eax
	add	eax, 1
	cmp	al, [ScanEnd]
	jbe	.zeroloop3
.ret2:
	mov	[huffman_bits], ecx
	ret

handle_progressive:
	cmp	[dct_buffer], 0
	jnz	@f
	ret
@@:
; information for all components
	mov	esi, components
	xor	ebp, ebp
	movzx	ecx, byte [ebx+18]
.next_component:
	mov	edi, cur_components
	lodsb	; ComponentID
	lodsd
	mov	ax, 0x0101
	stosd	; db V, db H, db VFactor, db HFactor
	xor	eax, eax
	mov	al, byte [edi-1]	; get HFactor
	mul	byte [ebx+18]		; number of components
	stosd			; HIncrement_i = HFactor_i * sizeof(pixel)
	mov	al, byte [edi-4-2]	; get VFactor
	mul	byte [ebx+18]		; number of components
	mul	dword [ebx+4]		; image width
	stosd			; VIncrement_i = VFactor_i * sizeof(row)
	lodsb
	and	eax, 3
	cmp	[quant_tables_defined+eax], 0
	jz	.error
	shl	eax, 8
	add	eax, quant_tables
	stosd		; dd QuantizationTable
	stosd		; dd DCTable - ignored
	mov	eax, ebp
	mul	[dct_buffer_size]
	add	eax, [dct_buffer]
	stosd		; instead of dd ACTable - pointer to current DCT coefficients
	push	ecx
	mov	eax, [ebx+4]
	movzx	ecx, byte [edi-21]	; get HFactor
;	cdq	; edx = 0 as a result of previous mul
	div	ecx
	stosd		; dd width / HFactor_i
	stosd
	xchg	eax, ecx
	inc	eax
	sub	eax, edx
	stosd		; dd HFactor_i+1 - (width % HFactor_i)
	mov	eax, [ebx+8]
	movzx	ecx, byte [edi-34]	; get VFactor
	cdq
	div	ecx
	stosd		; dd height / VFactor_i
	stosd
	xchg	eax, ecx
	inc	eax
	sub	eax, edx
	stosd		; dd VFactor_i+1 - (height % VFactor_i)
	pop	ecx
	xor	eax, eax
	test	ebp, ebp
	setnp	al
	ror	eax, 1
	stosd		; dd DCPrediction
	mov	eax, ebp
	stosd		; dd ComponentOffset
	inc	ebp
	push	ecx
	mov	[cur_components_end], edi
	lea	edx, [edi-56]
; do IDCT and unpack
	mov	edi, [ebx+28]
	add	edi, ebx
	mov	[cur_out_ptr], edi
	mov	[not_interleaved], 1
	call	init_limits
.decode_loop:
	call	decode_MCU
	sub	[cur_x], 1
	jnz	.decode_loop
	call	next_line
	sub	[cur_y], 1
	jnz	.decode_loop
	pop	ecx
	dec	ecx
	jnz	.next_component
; image unpacked, return
.error:
	push	ebx
	push	68
	pop	eax
	push	13
	pop	ebx
	mov	ecx, [dct_buffer]
	int	0x40
	pop	ebx
	ret

; Support for YCbCr -> RGB conversion
; R = Y                          + 1.402 * (Cr - 128)
; G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128)
; B = Y +   1.772 * (Cb - 128)
; When converting YCbCr -> RGB, we need to do some multiplications;
; to be faster, we precalculate the table for all 256 possible values
; Also we approximate fractions with N/65536, this gives sufficient precision
initialize_color_table:
; 1.402 = 1 + 26345/65536, -0.71414 = -46802/65536
; -0.34414 = -22554/65536, 1.772 = 1 + 50594/65536
	mov	edi, color_table_1
	mov	ecx, 128
; 1. Cb -> 1.772*Cb
	xor	eax, eax
	mov	dx, 8000h
.l1:
	push	ecx
@@:
	stosd
	add	dx, 50594
	adc	eax, 1
	loop	@b
	neg	dx
	adc	eax, -1
	neg	eax
	pop	ecx
	jnz	.l1
; 2. Cb -> -0.34414*Cb
	mov	ax, dx
.l2:
	push	ecx
@@:
	stosd
	sub	eax, 22554
	loop	@b
	neg	eax
	pop	ecx
	cmp	ax, dx
	jnz	.l2
	xor	eax, eax
; 3. Cr -> -0.71414*Cr
.l3:
	push	ecx
@@:
	stosd
	sub	eax, 46802
	loop	@b
	neg	eax
	pop	ecx
	jnz	.l3
; 4. Cr -> 1.402*Cr
.l4:
	push	ecx
@@:
	stosd
	add	dx, 26345
	adc	eax, 1
	loop	@b
	neg	dx
	adc	eax, -1
	neg	eax
	pop	ecx
	jnz	.l4
	ret

; this function is called in the end of image loading
convert_to_rgb:
; some checks
	test	ebx, ebx	; image exists?
	jz	.ret
	cmp	byte [ebx+18], 3	; full-color image?
	jz	.ycc2rgb
	cmp	byte [ebx+18], 4
	jz	.ycck2rgb
.ret:
	ret
.ycc2rgb:
; conversion is needed
	mov	esi, [ebx+4]
	imul	esi, [ebx+8]
	lea	edi, [ebx+44]
	push	ebx
; N.B. Speed optimization has sense here.
align 16
.loop:
;	mov	ebx, [edi]
;	mov	edx, ebx
;	mov	ecx, ebx
;	movzx	ebx, bl		; ebx = Y
;	shr	edx, 16
;	mov	eax, ebx
;	movzx	edx, dl		; edx = Cr
;	movzx	ecx, ch		; ecx = Cb
	movzx	ebx, byte [edi]
	movzx	ecx, byte [edi+1]
	mov	eax, ebx
	movzx	edx, byte [edi+2]
; B = Y + color_table_1[Cb]
	add	eax, [color_table_1+ecx*4]
	mov	ebp, [color_table_2+ecx*4]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	add	ebp, [color_table_3+edx*4]
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	sar	ebp, 16
	or	eax, ecx
	mov	[edi], al
; G = Y + color_table_2[Cb] + color_table_3[Cr]
	lea	eax, [ebx+ebp]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	or	eax, ecx
	mov	[edi+1], al
; R = Y + color_table_4[Cr]
	mov	eax, ebx
	add	eax, [color_table_4+edx*4]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	or	eax, ecx
	mov	[edi+2], al
	add	edi, 3
	sub	esi, 1
	jnz	.loop
	pop	ebx
	ret
.ycck2rgb:
; conversion is needed
	mov	esi, [ebx+4]
	imul	esi, [ebx+8]
	push	ebx
	push	esi
	lea	edi, [ebx+44]
	mov	esi, edi
; N.B. Speed optimization has sense here.
align 16
.kloop:
;	mov	ebx, [esi]
;	mov	edx, ebx
;	mov	ecx, ebx
;	movzx	ebx, bl		; ebx = Y
;	shr	edx, 16
;	mov	eax, ebx
;	movzx	edx, dl		; edx = Cr
;	movzx	ecx, ch		; ecx = Cb
	movzx	ebx, byte [esi]
	movzx	ecx, byte [esi+1]
	mov	eax, ebx
	movzx	edx, byte [esi+2]
; B = Y + color_table_1[Cb]
	add	eax, [color_table_1+ecx*4]
	mov	ebp, [color_table_2+ecx*4]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	add	ebp, [color_table_3+edx*4]
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	sar	ebp, 16
	or	eax, ecx
	xor	al, 0xFF
	mul	byte [esi+3]
	add	al, ah
	adc	ah, 0
	add	al, 80h
	adc	ah, 0
	mov	byte [edi], ah
; G = Y + color_table_2[Cb] + color_table_3[Cr]
	lea	eax, [ebx+ebp]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	or	eax, ecx
	xor	al, 0xFF
	mul	byte [esi+3]
	add	al, ah
	adc	ah, 0
	add	al, 80h
	adc	ah, 0
	mov	byte [edi+1], ah
; R = Y + color_table_4[Cr]
	mov	eax, ebx
	add	eax, [color_table_4+edx*4]
	cmp	eax, 80000000h
	sbb	ecx, ecx
	and	eax, ecx
	cmp	eax, 0x100
	sbb	ecx, ecx
	not	ecx
	or	eax, ecx
	xor	al, 0xFF
	mul	byte [esi+3]
	add	al, ah
	adc	ah, 0
	add	al, 80h
	adc	ah, 0
	mov	byte [edi+2], ah
	add	esi, 4
	add	edi, 3
	sub	dword [esp], 1
	jnz	.kloop
	pop	eax
	pop	ebx
; now correct values in RAW header
	mov	byte [ebx+12], 24
	mov	byte [ebx+18], 3
	mov	ecx, [ebx+32]
	shr	ecx, 2
	lea	ecx, [ecx*3]
	mov	[ebx+22], ecx
; release some memory - must succeed because we decrease size
	add	ecx, 44+1
	mov	edx, ebx
	push	68
	pop	eax
	push	20
	pop	ebx
	int	0x40
	mov	ebx, eax
	ret

; Decodes one data unit, that is, 8*8 block,
; from input stream, given by pointer esi and length ebp
; N.B. Speed optimization has sense here.
align 16
decode_data_unit:
; edx -> component data
	cmp	[progressive], 0
	jz	@f
	mov	edi, [edx+20]
	add	dword [edx+20], 64*2
	jmp	.coeff_decoded
@@:
	mov	edi, dct_coeff
	mov	ecx, 64*2/4
	xor	eax, eax
	rep	stosd
	mov	edi, zigzag+1
	mov	ecx, [huffman_bits]
; read DC coefficient
	mov	eax, [edx+16]
	push	edx
	get_huffman_code
	get_bits true
	add	eax, [edx+48]
	mov	[dct_coeff], ax
	mov	[edx+48], ax
; read AC coefficients
@@:
	mov	eax, [edx+20]
	push	edx
	get_huffman_code
	shr	eax, 4
	and	ebx, 15
	jz	.band
	add	edi, eax
	cmp	edi, zigzag+64
	jae	.eof
	get_bits true
	movzx	ebx, byte [edi]
	mov	[dct_coeff+ebx], ax
	add	edi, 1
	cmp	edi, zigzag+64
	jb	@b
	jmp	.do_idct
.band:
	pop	edx
	cmp	al, 15
	jnz	.do_idct
	add	edi, 16
	cmp	edi, zigzag+64
	jb	@b
;	jmp	.eof
.do_idct:
	mov	edi, dct_coeff
	mov	[huffman_bits], ecx
; coefficients loaded, now IDCT
.coeff_decoded:
	mov	eax, [edx+12]
	mov	ebx, idct_tmp_area
.idct_loop1:
	mov	cx, word [edi+1*16]
repeat 6
	or	cx, word [edi+(%+1)*16]
end repeat
	jnz	.real_transform
	fild	word [edi]
	fmul	dword [eax]
	fstp	dword [ebx]
	mov	ecx, [ebx]
repeat 7
	mov	[ebx+%*32], ecx
end repeat
	jmp	.idct_next1
.real_transform:
; S0,...,S7 - transformed values, s0,...,s7 - sought-for values
; S0,...,S7 are dequantized;
; dequantization table elements were multiplied to [idct_pre_table],
; so S0,S1,... later denote S0/2\sqrt{2},S1*\cos{\pi/16}/2,...
; 	sqrt2 = \sqrt{2}, cos = 2\cos{\pi/8},
; 	cos_sum = -2(\cos{\pi/8}+\cos{3\pi/8}), cos_diff = 2(\cos{\pi/8}-\cos{3\pi/8})
; Now formulas:
; s0 = ((S0+S4)+(S2+S6)) + ((S1+S7)+(S3+S5))
; s7 = ((S0+S4)+(S2+S6)) - ((S1+S7)+(S3+S5))
; val0 = ((cos-1)S1-(cos+cos_sum+1)S3+(cos+cos_sum-1)S5-(cos+1)S7)
; s1 = ((S0-S4)+((sqrt2-1)S2-(sqrt2+1)S6)) + val0
; s6 = ((S0-S4)+((sqrt2-1)S2-(sqrt2+1)S6)) - val0
; val1 = (S1+S7-S3-S5)sqrt2 - val0
; s2 = ((S0-S4)-((sqrt2-1)S2-(sqrt2+1)S6)) + val1
; s5 = ((S0-S4)-((sqrt2-1)S2-(sqrt2+1)S6)) - val1
; val2 = (S1-S7)cos_diff - (S1-S3+S5-S7)cos + val1
; s3 = ((S0+S4)-(S2+S6)) - val2
; s4 = ((S0+S4)-(S2+S6)) + val2
	fild	word [edi+3*16]
	fmul	dword [eax+3*32]
	fild	word [edi+5*16]
	fmul	dword [eax+5*32]	; st0=S5,st1=S3
	fadd	st1,st0
	fadd	st0,st0
	fsub	st0,st1		; st0=S5-S3,st1=S5+S3
	fild	word [edi+1*16]
	fmul	dword [eax+1*32]
	fild	word [edi+7*16]
	fmul	dword [eax+7*32]	; st0=S7,st1=S1
	fsub	st1,st0
	fadd	st0,st0
	fadd	st0,st1		; st0=S1+S7,st1=S1-S7,st2=S5-S3,st3=S5+S3
	fadd	st3,st0
	fadd	st0,st0
	fsub	st0,st3		; st0=S1-S3-S5+S7,st1=S1-S7,st2=S5-S3,st3=S1+S3+S5+S7
	fmul	[idct_sqrt2]
	fld	st2
	fadd	st0,st2
	fmul	[idct_cos]	; st0=(S1-S3+S5-S7)cos,st1=(S1-S3-S5+S7)sqrt2,
				; st2=S1-S7,st3=S5-S3,st4=S1+S3+S5+S7
	fxch	st2
	fmul	[idct_cos_diff]
	fsub	st0,st2		; st0=(S1-S7)cos_diff - (S1-S3+S5-S7)cos
	fxch	st3
	fmul	[idct_cos_sum]
	fadd	st0,st2		; st0=(S5-S3)cos_sum+(S1-S3+S5-S7)cos
	fsub	st0,st4		; st0=val0
	fsub	st1,st0		; st0=val0,st1=val1,st2=(S1-S3+S5-S7)cos,
				; st3=(S1-S7)cos_diff-(S1-S3+S5-S7)cos,st4=S1+S3+S5+S7
	fxch	st2
	fstp	st0
	fadd	st2,st0		; st0=val1,st1=val0,st2=val2,st3=S1+S3+S5+S7

	fild	word [edi+0*16]
	fmul	dword [eax+0*32]
	fild	word [edi+4*16]
	fmul	dword [eax+4*32]	; st0=S4,st1=S0
	fsub	st1,st0
	fadd	st0,st0
	fadd	st0,st1		; st0=S0+S4,st1=S0-S4
	fild	word [edi+6*16]
	fmul	dword [eax+6*32]
	fild	word [edi+2*16]
	fmul	dword [eax+2*32]	; st0=S2,st1=S6
	fadd	st1,st0
	fadd	st0,st0
	fsub	st0,st1		; st0=S2-S6,st1=S2+S6
	fmul	[idct_sqrt2]
	fsub	st0,st1
	fsub	st3,st0
	fadd	st0,st0
	fadd	st0,st3		; st0=(S0-S4)+((S2-S6)sqrt2-(S2+S6))
				; st3=(S0-S4)-((S2-S6)sqrt2-(S2+S6))
	fxch	st1
	fsub	st2,st0
	fadd	st0,st0
	fadd	st0,st2		; st0=(S0+S4)+(S2+S6),st1=(S0-S4)+((S2-S6)sqrt2-(S2+S6)),
				; st2=(S0+S4)-(S2+S6),st3=(S0-S4)-((S2-S6)sqrt2-(S2+S6))
				; st4=val1,st5=val0,st6=val2,st7=S1+S3+S5+S7
	fsubr	st7,st0
	fadd	st0,st0
	fsub	st0,st7
	fstp	dword [ebx+0*32]
	fsubr	st4,st0
	fadd	st0,st0
	fsub	st0,st4
	fstp	dword [ebx+1*32]
	fadd	st4,st0
	fadd	st0,st0
	fsub	st0,st4
	fstp	dword [ebx+3*32]
	fsubr	st1,st0
	fadd	st0,st0
	fsub	st0,st1
	fstp	dword [ebx+2*32]
	fstp	dword [ebx+5*32]
	fstp	dword [ebx+6*32]
	fstp	dword [ebx+4*32]
	fstp	dword [ebx+7*32]
.idct_next1:
	add	ebx, 4
	add	edi, 2
	add	eax, 4
	cmp	ebx, idct_tmp_area+8*4
	jb	.idct_loop1
	sub	ebx, 8*4
.idct_loop2:
	fld	dword [ebx+3*4]
	fld	dword [ebx+5*4]
	fadd	st1,st0
	fadd	st0,st0
	fsub	st0,st1		; st0=S5-S3,st1=S5+S3
	fld	dword [ebx+1*4]
	fld	dword [ebx+7*4]
	fsub	st1,st0
	fadd	st0,st0
	fadd	st0,st1		; st0=S1+S7,st1=S1-S7,st2=S5-S3,st3=S5+S3
	fadd	st3,st0
	fadd	st0,st0
	fsub	st0,st3		; st0=S1-S3-S5+S7,st1=S1-S7,st2=S5-S3,st3=S1+S3+S5+S7
	fmul	[idct_sqrt2]
	fld	st2
	fadd	st0,st2
	fmul	[idct_cos]	; st0=(S1-S3+S5-S7)cos,st1=(S1-S3-S5+S7)sqrt2,
				; st2=S1-S7,st3=S5-S3,st4=S1+S3+S5+S7
	fxch	st2
	fmul	[idct_cos_diff]
	fsub	st0,st2		; st0=(S1-S7)cos_diff - (S1-S3+S5-S7)cos
	fxch	st3
	fmul	[idct_cos_sum]
	fadd	st0,st2		; st0=(S5-S3)cos_sum+(S1-S3+S5-S7)cos
	fsub	st0,st4		; st0=val0
	fsub	st1,st0		; st0=val0,st1=val1,st2=(S1-S3+S5-S7)cos,
				; st3=(S1-S7)cos_diff-(S1-S3+S5-S7)cos,st4=S1+S3+S5+S7
	fxch	st2
	fstp	st0
	fadd	st2,st0		; st0=val1,st1=val0,st2=val2,st3=S1+S3+S5+S7

	fld	dword [ebx+0*4]
	fld	dword [ebx+4*4]
	fsub	st1,st0
	fadd	st0,st0
	fadd	st0,st1		; st0=S0+S4,st1=S0-S4
	fld	dword [ebx+6*4]
	fld	dword [ebx+2*4]
	fadd	st1,st0
	fadd	st0,st0
	fsub	st0,st1		; st0=S2-S6,st1=S2+S6
	fmul	[idct_sqrt2]
	fsub	st0,st1
	fsub	st3,st0
	fadd	st0,st0
	fadd	st0,st3		; st0=(S0-S4)+((S2-S6)sqrt2-(S2+S6))
				; st3=(S0-S4)-((S2-S6)sqrt2-(S2+S6))
	fxch	st1
	fsub	st2,st0
	fadd	st0,st0
	fadd	st0,st2		; st0=(S0+S4)+(S2+S6),st1=(S0-S4)+((S2-S6)sqrt2-(S2+S6)),
				; st2=(S0+S4)-(S2+S6),st3=(S0-S4)-((S2-S6)sqrt2-(S2+S6))
				; st4=val1,st5=val0,st6=val2,st7=S1+S3+S5+S7
	fsubr	st7,st0
	fadd	st0,st0
	fsub	st0,st7
	fistp	dword [ebx+0*4]
	fsubr	st4,st0
	fadd	st0,st0
	fsub	st0,st4
	fistp	dword [ebx+1*4]
	fadd	st4,st0
	fadd	st0,st0
	fsub	st0,st4
	fistp	dword [ebx+3*4]
	fsubr	st1,st0
	fadd	st0,st0
	fsub	st0,st1
	fistp	dword [ebx+2*4]
	fistp	dword [ebx+5*4]
	fistp	dword [ebx+6*4]
	fistp	dword [ebx+4*4]
	fistp	dword [ebx+7*4]

	add	ebx, 32
	cmp	ebx, idct_tmp_area+32*8
	jb	.idct_loop2

	mov	ecx, idct_tmp_area
	mov	edi, decoded_data - 1
.idct_loop3:
	mov	eax, [ecx]
	add	ecx, 4
	add	eax, 80h
	cmp	eax, 80000000h
	sbb	ebx, ebx
	add	edi, 1
	and	eax, ebx
	cmp	eax, 100h
	sbb	ebx, ebx
	not	ebx
	or	eax, ebx
	sub	al, [edx+51]
	cmp	ecx, idct_tmp_area+64*4
	mov	[edi], al
	jb	.idct_loop3
; done
	mov	ebx, [_ebx]
	ret

.eof:
; EOF or incorrect data during scanning
	mov	esp, [_esp]
	mov	ebx, [_ebx]
	jmp	START.end
;---------------------------------------------------------------------
check_header:
	pushad
	mov	eax,dword [esp+36]
	push	eax
	and	dword [eax+8], 0
	mov	esi, [eax]
	mov	ebp, [eax+12]
	call	get_marker
	jc	.err
	cmp	al, 0xD8
	jnz	.err
	pop	eax
	popad
	ret	4
.err:
	pop	eax
	inc	dword [eax+8]
	popad	
	ret	4
;---------------------------------------------------------------------
zigzag:
; (x,y) -> 2*(x+y*8)
repeat 8
	.cur = %
	if .cur and 1
		repeat %
			db	2*((%-1) + (.cur-%)*8)
		end repeat
	else
		repeat %
			db	2*((.cur-%) + (%-1)*8)
		end repeat
	end if
end repeat
repeat 7
	.cur = %
	if .cur and 1
		repeat 8-%
			db	2*((%+.cur-1) + (8-%)*8)
		end repeat
	else
		repeat 8-%
			db	2*((8-%) + (%+.cur-1)*8)
		end repeat
	end if
end repeat

align 4
idct_pre_table:
; c_0 = 1/(2\sqrt{2}), c_i = cos(i*\pi/16)/2
	dd	0.35355339, 0.49039264, 0.461939766, 0.41573481
	dd	0.35355339, 0.27778512, 0.19134172, 0.09754516
idct_sqrt2	dd	1.41421356	; \sqrt{2}
idct_cos	dd	1.847759065	; 2\cos{\pi/8}
idct_cos_sum	dd	-2.61312593	; -2(\cos{\pi/8} + \cos{3\pi/8})
idct_cos_diff	dd	1.08239220	; 2(\cos{\pi/8} - \cos{3\pi/8})
;---------------------------------------------------------------------
Associations:
dd  Associations.end - Associations
db 'JPEG',0
db 'JPG',0
db 'JPE',0
.end:
db 0
;---------------------------------------------------------------------
align 4
EXPORTS:
	dd	szStart,	START
	dd	szVersion,	0x00010002
	dd	szCheck,	check_header
	dd	szAssoc,	Associations
	dd	0

szStart		db 'START',0
szVersion	db 'version',0
szCheck		db 'Check_Header',0
szAssoc		db 'Associations',0

section '.data' data readable writable align 16

; up to 4 quantization tables
quant_tables		rd	4*64
quant_tables_defined	rb	4

; Huffman tables
dc_huffman_defined	rb	4
ac_huffman_defined	rb	4
; up to 4 DC Huffman tables
;dc_huffman		rd	4*256*2
; up to 4 AC Huffman tables
;ac_huffman		rd	4*256*2
max_hufftable_size = (256 + (9+128)*16)*2
dc_huffman		rb	4*max_hufftable_size
ac_huffman		rb	4*max_hufftable_size

; restart interval
restart_interval	dd	?
decoded_MCUs		dd	?

; base esp,ebx values
_esp			dd	?
_ebx			dd	?

; components information, up to 4 components
; db ComponentIdentifier, db V, db H, db VFactor, db HFactor, db QuantizationTable
components		rb	4*6
max_v			db	?
max_h			db	?
cur_rst_marker		db	?
			db	?
huffman_bits		dd	?
block_width	dd	?
block_height	dd	?
block_delta_x	dd	?
block_delta_y	dd	?
cur_block_dx	dd	?
cur_block_dy	dd	?
x_num_blocks	dd	?
y_num_blocks	dd	?
delta_x		dd	?
delta_y		dd	?
pixel_size	dd	?
line_size	dd	?
cur_x		dd	?
cur_y		dd	?
max_x		dd	?
max_y		dd	?
cur_out_ptr	dd	?
dct_buffer	dd	?
dct_buffer_size	dd	?
;ns			dd	?
; +0: db V, db H, db VFactor, db HFactor, dd HIncrement, dd VIncrement,
; +12: dd QuantizationTable, dd DCTable, dd ACTable,
; +24: dd width/HFactor, dd width/HFactor-8k, dd HFactor+1-(width%HFactor),
; +36: dd height/VFactor, dd height/VFactor-8m, dd VFactor+1-(height%VFactor),
; +48: dw DCPrediction, db ?, db (0 for Y, 80h for Cb,Cr), dd ComponentOffset
cur_components		rb	4*56
cur_components_end	dd	?
; progressive JPEG?
progressive		db	?
; one component in the scan?
not_interleaved		db	?
; Adobe YCCK file?
adobe_ycck		db	?
			rb	1
; parameters for progressive scan
ScanStart		db	?
ScanEnd			db	?
ApproxPosLow		db	?
ApproxPosHigh		db	?
; Fourier coefficients
dct_coeff		rw	64
; Temporary space for IDCT
idct_tmp_area		rd	64
; decoded block 8*8
decoded_data		rb	8*8
; data for YCbCr -> RGB translation
color_table_1		rd	256
color_table_2		rd	256
color_table_3		rd	256
color_table_4		rd	256
