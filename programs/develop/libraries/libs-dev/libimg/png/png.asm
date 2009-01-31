;;================================================================================================;;
;;//// png.asm //// (c) diamond, 2009 ////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;

;;================================================================================================;;
;;proc img.is.png _data, _length ;////////////////////////////////////////////////////////////////;;
img.is.png:
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in PNG format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
; test 1 (length of data)
	cmp	dword [esp+8], 8
	jb	.nope
; test 2: signature
	mov	eax, [esp+4]
	cmp	dword [eax], 0x474E5089
	jne	.nope
	cmp	dword [eax+4], 0x0A1A0A0D
	je	.yep

  .nope:
	xor	eax, eax
	ret	8

  .yep:
	xor	eax, eax
	inc	eax
	ret	8
;endp

;;================================================================================================;;
;;proc img.decode.png _data, _length ;////////////////////////////////////////////////////////////;;
img.decode.png:
	xor	eax, eax	; .image = 0
	pushad
	mov	ebp, esp
.localsize = 15*4
virtual at ebp - .localsize
.width		dd	?
.height		dd	?
.bit_depth	dd	?
.color_type	dd	?
.bytes_per_pixel dd	?
.scanline_len	dd	?
.cur_chunk_ptr	dd	?
.cur_chunk_size	dd	?
.paeth_a	dd	?
.paeth_b	dd	?
.paeth_c	dd	?
.paeth_pa	dd	?
.paeth_pb	dd	?
.paeth_pc	dd	?
.idat_read	dd	?
	rb	1Ch
.image		dd	?
	rd	1
.data		dd	?
.length		dd	?
end virtual
	push	0	; .idat_read = 0
	sub	esp, .localsize-4
; load deflate unpacker, if not yet
; acquire mutex
@@:
	push	1
	pop	eax
	xchg	[deflate_loader_mutex], eax	; 'xchg' has an implicit 'lock' prefix
	test	eax, eax
	jz	@f
	mcall	5, 1
	jmp	@b
@@:
	cmp	[deflate_unpack2], __deflate_unpack2_import_name__
	jnz	.deflate_loaded
; do loading
	invoke	dll.load, @IMPORT
	test	eax, eax
	jz	.deflate_loaded
	add	esp, .localsize
	popad
	mov	[deflate_loader_mutex], eax
	ret
.deflate_loaded:
; release mutex
	mov	[deflate_loader_mutex], 0
; ok, continue
	mov	esi, [.data]		; esi -> data
	mov	ecx, [.length]		; ecx = length
; the signature has been already checked in img.is.png
	lodsd
	lodsd
	sub	ecx, 8
	xor	ebx, ebx	; no image allocated
.chunks_loop:
	sub	ecx, 12
	jc	.eof
	lodsd	; chunk length
	bswap	eax
	sub	ecx, eax
	jc	.eof
	push	ecx	; save length of data rest
	xchg	eax, ecx	; ecx = size of data in the chunk
	lodsd	; chunk type
	cmp	eax, 'IHDR'
	jz	.ihdr
	cmp	eax, 'IDAT'
	jz	.idat
	cmp	eax, 'IEND'
	jz	.iend
	cmp	eax, 'PLTE'
	jz	.palette
; unrecognized chunk, ignore
	lea	esi, [esi+ecx+4]
	pop	ecx
	jmp	.chunks_loop
; IHDR chunk
.ihdr:
	cmp	ecx, 13
	jnz	.invalid_chunk
	cmp	[.image], 0
	jnz	.invalid_chunk
; read image characteristics
	lodsd
	bswap	eax
	mov	[.width], eax
	lodsd
	bswap	eax
	mov	[.height], eax
	xor	eax, eax
	lea	ebx, [eax+1]
	lodsb
	cmp	al, 16
	ja	.invalid_chunk
	test	al, al
	jz	.invalid_chunk
	lea	edx, [eax-1]
	test	al, dl
	jnz	.invalid_chunk
	mov	[.bit_depth], eax
	lodsb
	test	al, not 7
	jnz	.invalid_chunk
	mov	[.color_type], eax
	lodsb
	test	al, al
	jnz	.invalid_chunk	; only compression method 0 is defined
	lodsb
	test	al, al
	jnz	.invalid_chunk	; only filtering method 0 is defined
	lodsb
	test	al, al
	jnz	.invalid_chunk	; progressive PNGs are not supported yet
; check for correctness and calculate bytes_per_pixel and scanline_len
	mov	eax, [.bit_depth]
	mov	edx, [.color_type]
	dec	edx
	js	.grayscale1
	dec	edx
	jz	.rgb1
	dec	edx
	jz	.palette1
	dec	edx
	jz	.grayscale_alpha1
	dec	edx
	dec	edx
	jnz	.invalid_chunk
.rgb_alpha1:
	inc	ebx
.rgb1:
	inc	ebx
.grayscale_alpha1:
	inc	ebx
	cmp	al, 8
	jb	.invalid_chunk
	jmp	@f
.palette1:
	cmp	al, 8
	ja	.invalid_chunk
.grayscale1:
@@:
	mul	ebx
	push	eax
	add	eax, 7
	shr	eax, 3
	mov	[.bytes_per_pixel], eax
	pop	eax
	mul	[.width]
	add	eax, 7
	shr	eax, 3
	mov	[.scanline_len], eax
; allocate image
	push	Image.bpp24
	pop	eax
	cmp	[.color_type], 2
	jz	@f
	mov	al, Image.bpp32
	cmp	[.color_type], 6
	jz	@f
	mov	al, Image.bpp8
@@:
	stdcall	img.create, [.width], [.height], eax
	test	eax, eax
	jz	.invalid_chunk
	mov	[.image], eax
	jmp	.next_chunk
.invalid_chunk:
.iend:
	pop	ecx
.eof:
	add	esp, .localsize
	popad
	ret
; PLTE chunk
.palette:
	mov	eax, [.image]
	test	eax, eax
	jz	.invalid_chunk
	cmp	[.color_type], 3
	jz	.copy_palette
.ignore_chunk:
	add	esi, ecx
.next_chunk:
	lodsd
	pop	ecx
	jmp	.chunks_loop
.copy_palette:
	mov	edi, [eax + Image.Palette]
	xor	eax, eax
	cmp	ecx, 256*3
	ja	.next_chunk
@@:
	sub	ecx, 3
	jz	@f
	js	.invalid_chunk
	lodsd
	dec	esi
	bswap	eax
	shr	eax, 8
	stosd
	jmp	@b
@@:
	lodsd
	dec	esi
	bswap	eax
	shr	eax, 8
	stosd
	jmp	.next_chunk
.idat:
	jecxz	.next_chunk
	cmp	[.idat_read], 0
	jnz	@f
	lodsb
	inc	[.idat_read]
	and	al, 0xF
	cmp	al, 8
	jnz	.invalid_chunk
	dec	ecx
	jz	.next_chunk
@@:
	cmp	[.idat_read], 1
	jnz	@f
	lodsb
	inc	[.idat_read]
	test	al, 20h
	jnz	.invalid_chunk
	dec	ecx
	jz	.next_chunk
@@:
	mov	[.cur_chunk_ptr], esi
	mov	[.cur_chunk_size], ecx
	pop	[.length]
	push	eax
	push	esp
	push	ebp
	push	.deflate_callback
	call	[deflate_unpack2]
	pop	ecx
	test	eax, eax
	jz	.invalid_chunk
; convert PNG unpacked data to RAW data
	mov	esi, eax
	push	eax ecx
; unfilter
	mov	edx, [.height]
.unfilter_loop_e:
	mov	ebx, [.scanline_len]
	sub	ecx, 1
	jc	.unfilter_done
	sub	ecx, ebx
	jc	.unfilter_done
	movzx	eax, byte [esi]
	add	esi, 1
	cmp	eax, 4
	ja	.next_scanline
	jmp	dword [@f + eax*4]
align 4
@@:
	dd	.unfilter_none
	dd	.unfilter_sub
	dd	.unfilter_up
	dd	.unfilter_average
	dd	.unfilter_paeth
.unfilter_sub:
	mov	edi, [.bytes_per_pixel]
	add	esi, edi
	sub	ebx, edi
	jbe	.next_scanline
	neg	edi
@@:
	mov	al, [esi+edi]
	add	[esi], al
	add	esi, 1
	sub	ebx, 1
	jnz	@b
	jmp	.next_scanline
.unfilter_up:
	cmp	edx, [.height]
	jz	.unfilter_none
	lea	edi, [ebx+1]
	neg	edi
@@:
	mov	al, [esi+edi]
	add	[esi], al
	add	esi, 1
	sub	ebx, 1
	jnz	@b
	jmp	.next_scanline
.unfilter_average:
	mov	edi, [.bytes_per_pixel]
	cmp	edx, [.height]
	jz	.unfilter_average_firstline
	push	edx
	lea	edx, [ebx+1]
	neg	edx
	sub	ebx, edi
@@:
	mov	al, [esi+edx]
	shr	al, 1
	add	[esi], al
	add	esi, 1
	sub	edi, 1
	jnz	@b
	mov	edi, [.bytes_per_pixel]
	neg	edi
	test	ebx, ebx
	jz	.unfilter_average_done
@@:
	mov	al, [esi+edx]
	add	al, [esi+edi]
	rcr	al, 1
	add	[esi], al
	add	esi, 1
	sub	ebx, 1
	jnz	@b
.unfilter_average_done:
	pop	edx
	jmp	.next_scanline
.unfilter_average_firstline:
	mov	edi, [.bytes_per_pixel]
	add	esi, edi
	sub	ebx, edi
	jbe	.next_scanline
	neg	edi
@@:
	mov	al, [esi+edi]
	shr	al, 1
	add	[esi], al
	add	esi, 1
	sub	ebx, 1
	jnz	@b
	jmp	.unfilter_none
.unfilter_paeth:
	cmp	edx, [.height]
	jz	.unfilter_sub
	push	edx
	lea	edx, [ebx+1]
	mov	edi, [.bytes_per_pixel]
	neg	edx
	sub	ebx, edi
@@:
	mov	al, [esi+edx]
	add	[esi], al
	add	esi, 1
	sub	edi, 1
	jnz	@b
	mov	edi, [.bytes_per_pixel]
	neg	edi
	test	ebx, ebx
	jz	.unfilter_paeth_done
	push	ecx
@@:
	push	ebx
; PaethPredictor(Raw(x-bpp) = a, Prior(x) = b, Prior(x-bpp) = c)
	movzx	eax, byte [esi+edi]
	mov	[.paeth_a], eax
	movzx	ecx, byte [esi+edx]
	add	edi, edx
	mov	[.paeth_b], ecx
	add	ecx, eax
	movzx	eax, byte [esi+edi]
	mov	[.paeth_c], eax
	sub	ecx, eax	; ecx = a + b - c = p
; calculate pa = abs(p-a), pb = abs(p-b), pc = abs(p-c)
	mov	ebx, ecx
	sub	ebx, eax	; ebx = p - c
	cmp	ebx, 80000000h
	sbb	eax, eax	; eax = (p < c) ? 0 : 0xFFFFFFF
	not	eax		; eax = (p < c) ? 0xFFFFFFFF : 0
	and	eax, ebx	; eax = (p < c) ? p - c : 0
	sub	ebx, eax
	sub	ebx, eax	; ebx = abs(p-c)
	mov	[.paeth_pc], ebx
	mov	ebx, ecx
	sub	ebx, [.paeth_a]
	cmp	ebx, 80000000h
	sbb	eax, eax
	not	eax
	and	eax, ebx
	sub	ebx, eax
	sub	ebx, eax
	mov	[.paeth_pa], ebx
	mov	ebx, ecx
	sub	ebx, [.paeth_b]
	cmp	ebx, 80000000h
	sbb	eax, eax
	not	eax
	and	eax, ebx
	sub	ebx, eax
	sub	ebx, eax
	;mov	[.paeth_pb], ebx
; select closest value
	push	edx
	mov	edx, [.paeth_b]
	sub	edx, [.paeth_a]
	sub	ebx, [.paeth_pa]
	sbb	ecx, ecx	; ecx = (pa > pb) ? 0xFFFFFFFF : 0
	sbb	eax, eax	; eax = (pa > pb) ? 0xFFFFFFFF : 0
	and	ecx, ebx	; ecx = (pa > pb) ? pb - pa : 0
	and	eax, edx	; eax = (pa > pb) ? b - a : 0
	add	ecx, [.paeth_pa]	; ecx = (pa > pb) ? pb : pa = min(pa,pb)
	add	eax, [.paeth_a]		; eax = (pa > pb) ? b : a
	mov	edx, [.paeth_c]
	sub	edx, eax
	sub	[.paeth_pc], ecx
	sbb	ebx, ebx	; ebx = (min(pa,pb) <= pc) ? 0 : 0xFFFFFFFF
	and	ebx, edx	; ebx = (min(pa,pb) <= pc) ? 0 : c - eax
	add	eax, ebx
	pop	edx
	add	[esi], al
	pop	ebx
	sub	edi, edx
	add	esi, 1
	sub	ebx, 1
	jnz	@b
	pop	ecx
.unfilter_paeth_done:
	pop	edx
	jmp	.next_scanline
.unfilter_none:
	add	esi, ebx
.next_scanline:
	sub	edx, 1
	jnz	.unfilter_loop_e
.unfilter_done:
; unfiltering done, now convert to raw data
	pop	ebx esi
	push	esi
	mov	edx, [.height]
	mov	eax, [.image]
	mov	edi, [eax + Image.Data]
	cmp	[.color_type], 0
	jz	.grayscale2
	cmp	[.color_type], 2
	jz	.rgb2
	cmp	[.color_type], 3
	jz	.palette2
	cmp	[.color_type], 4
	jz	.grayscale_alpha2
.rgb_alpha2:
	cmp	[.bit_depth], 16
	jz	.rgb_alpha2_16bit
.rgb_alpha2.next:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
@@:
	mov	al, [esi+2]
	mov	[edi], al
	mov	al, [esi+1]
	mov	[edi+1], al
	mov	al, [esi]
	mov	[edi+2], al
	mov	al, [esi+3]
	mov	[edi+3], al
	sub	ecx, 4
	jnz	@b
	sub	edx, 1
	jnz	.rgb_alpha2.next
	jmp	.convert_done
.rgb_alpha2_16bit:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
.rgb_alpha2.loop:

; convert 16 bit sample to 8 bit sample
macro convert_16_to_8
{
local .l1,.l2
	xor	ah, 0x80
	js	.l1
	cmp	al, ah
	adc	al, 0
	jmp	.l2
.l1:
	cmp	ah, al
	sbb	al, 0
.l2:
}

	mov	ax, [esi+4]
	convert_16_to_8
	mov	[edi], al
	mov	ax, [esi+2]
	convert_16_to_8
	mov	[edi+1], al
	mov	ax, [esi]
	convert_16_to_8
	mov	[edi+2], al
	;mov	ax, [esi+6]
	;convert_16_to_8
	;mov	[edi+3], al
	add	esi, 8
	add	edi, 4
	sub	ecx, 8
	jnz	.rgb_alpha2.loop
	sub	edx, 1
	jnz	.rgb_alpha2_16bit
	jmp	.convert_done
.grayscale2:
	push	edi edx
	mov	edi, [eax + Image.Palette]
	mov	ecx, [.bit_depth]
	cmp	cl, 16
	jnz	@f
	mov	cl, 8
@@:
	push	1
	pop	eax
	shl	eax, cl
	xchg	eax, ecx
	mov	edx, 0x010101
	cmp	al, 8
	jz	.graypal_common
	mov	edx, 0x111111
	cmp	al, 4
	jz	.graypal_common
	mov	edx, 0x555555
	cmp	al, 2
	jz	.graypal_common
	mov	edx, 0xFFFFFF
.graypal_common:
	xor	eax, eax
@@:
	stosd
	add	eax, edx
	loop	@b
	pop	edx edi
	cmp	[.bit_depth], 16
	jz	.grayscale2_16bit
.palette2:
	cmp	[.bit_depth], 1
	jz	.palette2_1bit
	cmp	[.bit_depth], 2
	jz	.palette2_2bit
	cmp	[.bit_depth], 4
	jz	.palette2_4bit
.palette2_8bit:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
	push	ecx
	shr	ecx, 2
	rep	movsd
	pop	ecx
	and	ecx, 3
	rep	movsb
	sub	edx, 1
	jnz	.palette2_8bit
	jmp	.convert_done
.palette2_4bit:
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, [.scanline_len]
	jc	.convert_done
	push	edx
	mov	ecx, [.width]
@@:
	mov	al, [esi]
	add	esi, 1
	mov	dl, al
	shr	al, 4
	and	dl, 0xF
	mov	[edi], al
	sub	ecx, 1
	jz	@f
	mov	[edi+1], dl
	add	edi, 2
	sub	ecx, 1
	jnz	@b
	sub	edi, 1
@@:
	pop	edx
	add	edi, 1
	sub	edx, 1
	jnz	.palette2_4bit
	jmp	.convert_done
.palette2_2bit:
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, [.scanline_len]
	jc	.convert_done
	push	edx
	mov	ecx, [.width]
@@:
	mov	al, [esi]
	add	esi, 1
	mov	dl, al
	shr	al, 6
	and	dl, not 11000000b
	mov	[edi], al
	add	edi, 1
	sub	ecx, 1
	jz	@f
	mov	al, dl
	shr	dl, 4
	and	al, not 00110000b
	mov	[edi], dl
	add	edi, 1
	sub	ecx, 1
	jz	@f
	mov	dl, al
	shr	al, 2
	and	dl, not 00001100b
	mov	[edi], al
	add	edi, 1
	sub	ecx, 1
	jz	@f
	mov	[edi], dl
	add	edi, 1
	sub	ecx, 1
	jnz	@b
@@:
	pop	edx
	sub	edx, 1
	jnz	.palette2_2bit
	jmp	.convert_done
.palette2_1bit:
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, [.scanline_len]
	jc	.convert_done
	push	edx
	mov	ecx, [.width]
@@:
	mov	al, [esi]
	add	esi, 1
repeat 3
	mov	dl, al
	shr	al, 9-%*2
	and	dl, not (1 shl (9-%*2))
	mov	[edi], al
	add	edi, 1
	sub	ecx, 1
	jz	@f
	mov	al, dl
	shr	dl, 8-%*2
	and	al, not (1 shl (8-%*2))
	mov	[edi], dl
	add	edi, 1
	sub	ecx, 1
	jz	@f
end repeat
	mov	dl, al
	shr	al, 1
	and	dl, not (1 shl 1)
	mov	[edi], al
	add	edi, 1
	sub	ecx, 1
	jz	@f
	mov	[edi], dl
	add	edi, 1
	sub	ecx, 1
	jnz	@b
@@:
	pop	edx
	sub	edx, 1
	jnz	.palette2_1bit
	jmp	.convert_done
.grayscale2_16bit:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
@@:
	convert_16_to_8
	add	esi, 2
	add	edi, 1
	sub	ecx, 2
	jnz	@b
	sub	edx, 1
	jnz	.grayscale2_16bit
	jmp	.convert_done
.rgb2:
	cmp	[.bit_depth], 16
	jz	.rgb2_16bit
.rgb2.next:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
@@:
	mov	al, [esi+2]
	mov	[edi], al
	mov	al, [esi+1]
	mov	[edi+1], al
	mov	al, [esi]
	mov	[edi+2], al
	add	esi, 3
	add	edi, 3
	sub	ecx, 3
	jnz	@b
	sub	edx, 1
	jnz	.rgb2.next
	jmp	.convert_done
.rgb2_16bit:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
.rgb2.loop:
	mov	ax, [esi+4]
	convert_16_to_8
	mov	[edi], al
	mov	ax, [esi+2]
	convert_16_to_8
	mov	[edi+1], al
	mov	ax, [esi]
	convert_16_to_8
	mov	[edi+2], al
	add	esi, 6
	add	edi, 3
	sub	ecx, 6
	jnz	.rgb2.loop
	sub	edx, 1
	jnz	.rgb2_16bit
	jmp	.convert_done
.grayscale_alpha2:
	cmp	[.bit_depth], 16
	jz	.grayscale_alpha2_16bit
.grayscale_alpha2.next:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
@@:
	mov	al, [esi]
	mov	[edi], al
	add	esi, 2
	add	edi, 1
	sub	ecx, 2
	jnz	@b
	sub	edx, 1
	jnz	.grayscale_alpha2.next
	jmp	.convert_done
.grayscale_alpha2_16bit:
	mov	ecx, [.scanline_len]
	sub	ebx, 1
	jc	.convert_done
	add	esi, 1
	sub	ebx, ecx
	jc	.convert_done
@@:
	convert_16_to_8
	add	esi, 4
	add	edi, 1
	sub	ecx, 4
	jnz	@b
	sub	edx, 1
	jnz	.grayscale_alpha2_16bit
.convert_done:
	pop	ecx
	mcall	68, 13
	mov	esi, [.cur_chunk_ptr]
	add	esi, [.cur_chunk_size]
	push	[.length]
	jmp	.next_chunk

.deflate_callback:
	mov	ebp, [esp+4]
	mov	ebx, [esp+8]
	xor	eax, eax
	mov	esi, [.cur_chunk_size]
	mov	[ebx], esi
	test	esi, esi
	jz	.deflate_callback.ret
	mov	eax, [.cur_chunk_ptr]
	mov	ecx, [.length]
	add	esi, eax
	mov	[.cur_chunk_ptr], esi
	and	[.cur_chunk_size], 0
@@:
	sub	ecx, 12
	jb	.deflate_callback.ret
	cmp	dword [esi+4+4], 'IDAT'
	jnz	.deflate_callback.ret
	mov	edx, [esi+4]
	bswap	edx
	sub	ecx, edx
	jb	.deflate_callback.ret
	add	esi, 4+8
	test	edx, edx
	jz	@b
	mov	[.cur_chunk_size], edx
	mov	[.cur_chunk_ptr], esi
	mov	[.length], ecx
.deflate_callback.ret:
	ret	8
;endp

img.encode.png:
	xor	eax, eax
	ret	8
