;;================================================================================================;;
;;//// tiff.asm //// (c) dunkaist, 2011-2012 /////////////////////////////////////////////////////;;
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

include 'tiff.inc'
;include '../../../../../system/board/trunk/debug.inc'

;;================================================================================================;;
proc img.is.tiff _data, _length ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in tiff format)                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	push	esi

	mov	esi, [_data]
	lodsw
	cmp	ax, word 'II'
	je	.little_endian
	cmp	ax, word 'MM'
	je	.big_endian
	jmp	.is_not_tiff

  .little_endian:
	lodsw
	cmp	ax, 0x002A
	je	.is_tiff
	jmp	.is_not_tiff

  .big_endian:
	lodsw
	cmp	ax, 0x2A00
	je	.is_tiff

  .is_not_tiff:
	pop	esi
	xor	eax, eax
	ret

  .is_tiff:
	pop	esi
	xor	eax, eax
	inc	eax
	ret
endp

;;================================================================================================;;
proc img.decode.tiff _data, _length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in tiff format                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
	_endianness		rd 1		; 0 stands for LE, otherwise BE
	retvalue		rd 1		; 0 (error) or pointer to image
endl

	push	ebx edx esi edi

	mov	esi, [_data]
	lodsw
	mov	[_endianness], 0
	cmp	ax, word 'II'
	seta	byte[_endianness]

	lodsw_
	lodsd_
    @@:
	stdcall	tiff._.parse_IFD, [_data], eax, [_endianness]
	mov	ebx, eax
	mov	[retvalue], eax
	lodsd_
	test	eax, eax
;	jnz	@b


  .quit:
	mov	eax, [retvalue]
	pop	edi esi edx ebx
	ret
endp


;;================================================================================================;;
proc img.encode.tiff _img, _p_length, _options ;//////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in tiff format                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to encoded data                                                     ;;
;< _p_length = encoded data length                                                                ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
proc tiff._.parse_IFD _data, _IFD, _endianness
locals
	extended		rd	1
	retvalue		rd	1
	decompress		rd	1
endl
	push	ebx edx edi
	mov	[retvalue], 0

	invoke	mem.alloc, sizeof.tiff_extra
	test	eax, eax
	jz	.quit
	mov	[extended], eax
	mov	ebx, eax
	mov	edi, eax
	mov	ecx, sizeof.tiff_extra/4
	xor	eax, eax
	rep	stosd

	mov	esi, [_IFD]
	add	esi, [_data]
	lodsw_
	movzx	ecx, ax
    @@:
	push	ecx
	stdcall	tiff._.parse_IFDE, [_data], [_endianness]
	pop	ecx
	dec	ecx
	jnz	@b

	call	tiff._.define_image_type

	stdcall	img.create, [ebx + tiff_extra.image_width], [ebx + tiff_extra.image_height], eax
	test	eax, eax
	jz	.quit
	mov	[retvalue], eax
	mov	edx, eax
	mov	[edx + Image.Extended], ebx

	cmp	[ebx+tiff_extra.compression], TIFF.COMPRESSION.UNCOMPRESSED
	jne	@f
	mov	[decompress], tiff._.decompress.uncompressed
	jmp	.decompressor_defined
    @@:
	cmp	[ebx + tiff_extra.compression], TIFF.COMPRESSION.PACKBITS
	jne	@f
	mov	[decompress], tiff._.decompress.packbits
	jmp	.decompressor_defined
    @@:
	mov	[decompress], tiff._.decompress.ccitt1d
	jmp	.decompressor_defined
	jmp	.quit
  .decompressor_defined:

	push	esi		; fixme!!

	mov	ecx, [edx + Image.Type]
	dec	ecx
	jz	.bpp8
	dec	ecx
	jz	.bpp24
	dec	ecx
	jz	.bpp32
	dec	ecx
	dec	ecx		; tiff doesn't handle 15bpp images
	jz	.bpp16
	dec	ecx
	jz	.bpp1
	dec	ecx
	jz	.bpp4
;error report!!

  .bpp1:
  .bpp1.palette:
	mov	edi, [edx+Image.Palette]
	cmp	[ebx + tiff_extra.photometric], TIFF.PHOTOMETRIC.BLACK_IS_ZERO
	jne	.bpp1.white_is_zero
  .bpp1.black_is_zero:
	mov	[edi], dword 0x00000000
	mov	[edi + 4], dword 0x00ffffff
	jmp	.common
  .bpp1.white_is_zero:
	mov	[edi], dword 0x00ffffff
	mov	[edi + 4], dword 0x00000000
	jmp	.common

  .bpp4:
	jmp	.common

  .bpp8:
	cmp	[ebx + tiff_extra.palette], 0
	je	.bpp8.grayscale

	mov	esi, [ebx + tiff_extra.palette]
	mov	ah, 2
  .bpp8.channel:
	mov	edi, eax
	and	edi, 0x0000ff00
	shr	edi, 8
	add	edi, [edx + Image.Palette]
	mov	ecx, 256
    @@:
	lodsb
	stosb
	lodsb
	add	edi, 3
	dec	ecx
	jnz	@b
	dec	ah
	jns	.bpp8.channel
	jmp	.common
  .bpp8.grayscale:
	mov	edi, [edx + Image.Palette]
	mov	eax, 0xff000000
    @@:
	stosd
	add	eax, 0x00010101
	jnc	@b
	jmp	.common

  .bpp16:
	jmp	.common

  .bpp24:
	jmp	.common

  .bpp32:
	jmp	.common


  .common:
	mov	edi, [edx+Image.Data]
	mov	esi, [ebx+tiff_extra.strip_offsets]
	mov	edx, [ebx+tiff_extra.strip_byte_counts]


	cmp	[ebx + tiff_extra.strip_offsets_length], TIFF.IFDE_TYPE_LENGTH.SHORT
	jne	.l_x
	cmp	[ebx + tiff_extra.strip_byte_counts_length], TIFF.IFDE_TYPE_LENGTH.SHORT
	jne	.s_l
	jmp	.s_s
  .l_x:	cmp	[ebx + tiff_extra.strip_byte_counts_length], TIFF.IFDE_TYPE_LENGTH.SHORT
	jne	.l_l
	jmp	.l_s

  .s_s:
	xor	eax, eax
	lodsw_
	push	esi
	mov	esi, eax
	add	esi, [_data]
	xor	ecx, ecx
	mov	cx, word[edx]
	test	[_endianness], 1
	jz	@f
	xchg	cl, ch
    @@:
	add	edx, 2
	stdcall	[decompress], [retvalue]
	pop	esi
	dec	[ebx + tiff_extra.offsets_number]
	jnz	.s_s
	jmp	.decoded

  .s_l:
	xor	eax, eax
	lodsw_
	push	esi
	mov	esi, eax
	add	esi, [_data]
	mov	ecx, [edx]
	test	[_endianness], 1
	jz	@f
	bswap	ecx
    @@:
	add	edx, 4
	stdcall	[decompress], [retvalue]
	pop	esi
	dec	[ebx + tiff_extra.offsets_number]
	jnz	.s_l
	jmp	.decoded

  .l_s:
	lodsd_
	push	esi
	mov	esi, eax
	add	esi, [_data]
	xor	ecx, ecx
	mov	cx, word[edx]
	test	[_endianness], 1
	jz	@f
	xchg	cl, ch
    @@:
	add	edx, 2
	stdcall	[decompress], [retvalue]
	pop	esi
	dec	[ebx + tiff_extra.offsets_number]
	jnz	.l_s
	jmp	.decoded

  .l_l:
	lodsd_
	push	esi
	mov	esi, eax
	add	esi, [_data]
	mov	ecx, [edx]
	test	[_endianness], 1
	jz	@f
	bswap	ecx
    @@:
	add	edx, 4
	stdcall	[decompress], [retvalue]
	pop	esi
	dec	[ebx + tiff_extra.offsets_number]
	jnz	.l_l
	jmp	.decoded


  .decoded:
	cmp	[ebx + tiff_extra.samples_per_pixel], 3
	jne	.pop_quit
	mov	eax, [retvalue]
	mov	esi, [eax + Image.Data]
	mov	edi, [eax + Image.Data]
	mov	ecx, [eax + Image.Width]
	imul	ecx, [eax + Image.Height]
    @@:
	lodsw
	movsb
	mov	byte[esi - 1], al
	add	edi, 2
	dec	ecx
	jnz	@b


  .pop_quit:
	pop	esi
  .quit:
	pop	edi edx ebx
	mov	eax, [retvalue]
	ret
endp

proc tiff._.parse_IFDE _data, _endianness

	push	ebx edx edi

	lodsw_
	mov	edx, tiff.IFDE_tag_table.begin
	mov	ecx, (tiff.IFDE_tag_table.end-tiff.IFDE_tag_table.begin)/8
  .tag:
	cmp	ax, word[edx]
	jne	@f
	lodsw_
	jmp	dword[edx + 4]
    @@:
	add	edx, 8
	dec	ecx
	jnz	.tag
  .tag_default:				; unknown/unsupported/uninteresting/unimportant
	lodsw
	lodsd
	lodsd
	jmp	.quit			; just skip it

  .tag_100:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	lodsd
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.image_width], eax
	lodsw
	jmp	.quit
    @@:
	cmp	ax, TIFF.IFDE_TYPE.LONG
	jne	@f
	lodsd
	lodsd_
	mov	[ebx + tiff_extra.image_width], eax
	jmp	.quit
    @@:
	jmp	.quit

  .tag_101:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	lodsd
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.image_height], eax
	lodsw
	jmp	.quit
    @@:
	cmp	ax, TIFF.IFDE_TYPE.LONG
	jne	@f
	lodsd
	lodsd_
	mov	[ebx + tiff_extra.image_height], eax
	jmp	.quit
    @@:
	jmp	.quit

  .tag_102:
	lodsd_
	imul	eax, TIFF.IFDE_TYPE_LENGTH.SHORT
	cmp	eax, 4
	ja	@f
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.bits_per_sample], eax
	lodsw
	jmp	.quit
    @@:
	lodsd_
	add	eax, [_data]
	push	esi
	mov	esi, eax
	xor	eax, eax
	lodsw_
	pop	esi
	mov	[ebx + tiff_extra.bits_per_sample], eax
	jmp	.quit

  .tag_103:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	lodsd
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.compression], eax
	lodsw
	jmp	.quit
    @@:
	jmp	.quit

  .tag_106:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	lodsd
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.photometric], eax
	lodsw
	jmp	.quit
    @@:

	jmp	.quit

  .tag_111:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	mov	[ebx + tiff_extra.strip_offsets_length], TIFF.IFDE_TYPE_LENGTH.SHORT
	jmp	.tag_111.common
    @@:
	mov	[ebx + tiff_extra.strip_offsets_length], TIFF.IFDE_TYPE_LENGTH.LONG
  .tag_111.common:
	lodsd_
	mov	[ebx + tiff_extra.offsets_number], eax
	imul	eax, [ebx+tiff_extra.strip_offsets_length]
	cmp	eax, 4
	ja	@f
	mov	[ebx + tiff_extra.strip_offsets], esi
	lodsd
	jmp	.quit
    @@:
	lodsd_
	add	eax, [_data]
	mov	[ebx + tiff_extra.strip_offsets], eax
	jmp	.quit

  .tag_115:
	lodsd_
	imul	eax, TIFF.IFDE_TYPE_LENGTH.SHORT
	cmp	eax, 4
	ja	@f
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.samples_per_pixel], eax
	lodsw
	jmp	.quit
    @@:
	lodsd_
	add	eax, [_data]
	movzx	eax, word[eax]
	jmp	.quit

  .tag_116:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	lodsd
	xor	eax, eax
	lodsw_
	mov	[ebx + tiff_extra.rows_per_strip], eax
	lodsw
	jmp	.quit
    @@:
	lodsd
	lodsd_
	mov	[ebx + tiff_extra.rows_per_strip], eax
	jmp	.quit

  .tag_117:
	cmp	ax, TIFF.IFDE_TYPE.SHORT
	jne	@f
	mov	[ebx + tiff_extra.strip_byte_counts_length], TIFF.IFDE_TYPE_LENGTH.SHORT
	jmp	.tag_117.common
    @@:
	mov	[ebx + tiff_extra.strip_byte_counts_length], TIFF.IFDE_TYPE_LENGTH.LONG
  .tag_117.common:
	lodsd_
	imul	eax, [ebx + tiff_extra.strip_byte_counts_length]
	cmp	eax, 4
	ja	@f
	mov	[ebx + tiff_extra.strip_byte_counts], esi
	lodsd
	jmp	.quit
    @@:
	lodsd_
	add	eax, [_data]
	mov	[ebx + tiff_extra.strip_byte_counts], eax
	jmp	.quit

  .tag_140:
	lodsd
	lodsd_
	add	eax, [_data]
	mov	[ebx + tiff_extra.palette], eax
	jmp	.quit

  .quit:
	pop	edi edx ebx
	ret
endp


proc tiff._.define_image_type

	xor	eax, eax

	cmp	[ebx + tiff_extra.bits_per_sample], 1
	jg	.not_bilevel
	mov	eax, Image.bpp1
	jmp	.quit
  .not_bilevel:
	cmp	[ebx + tiff_extra.palette], 0
	je	.without_palette
	cmp	[ebx + tiff_extra.bits_per_sample], 4
	jne	@f
	mov	eax, Image.bpp4
	jmp	.quit
    @@:
	cmp	[ebx + tiff_extra.bits_per_sample], 8
	jne	@f
	mov	eax, Image.bpp8
	jmp	.quit
    @@: 
	jmp	.quit
  .without_palette:
	cmp	[ebx + tiff_extra.samples_per_pixel], 1
	jg	.not_grayscale
	cmp	[ebx + tiff_extra.bits_per_sample], 4
	jne	@f
	mov	eax, Image.bpp4
	jmp	.quit
    @@:
	cmp	[ebx + tiff_extra.bits_per_sample], 8
	jne	@f
	mov	eax, Image.bpp8
	jmp	.quit
  .not_grayscale:
	cmp	[ebx + tiff_extra.samples_per_pixel], 3
	jne	@f
	mov	eax, Image.bpp24
	jmp	.quit
    @@:
	jmp	.quit
  .quit:
	ret
endp


proc tiff._.decompress.uncompressed _image

	rep	movsb
	ret
endp


proc tiff._.decompress.packbits _image

	push	ebx ecx edx esi

	mov	edx, ecx

  .decode:
	lodsb
	dec	edx
	cmp	al, 0xff
	jbe	.different
	cmp	al, 0x80
	jne	.identical
	test	edx, edx
	jz	.quit
	jmp	.decode

  .identical:
	neg	al
	inc	al
	movzx	ecx, al
	dec	edx
	lodsb
	rep	stosb
	test	edx, edx
	jnz	.decode
	jmp	.quit

  .different:
	movzx	ecx, al
	inc	ecx
	sub	edx, ecx
	rep	movsb
	test	edx, edx
	jnz	.decode

  .quit:
	pop	esi edx ecx ebx
	ret
endp


proc	tiff._.decompress.ccitt1d _image
locals
	current_tree		rd	1
	old_tree		rd	1
	width			rd	1
	height			rd	1
	width_left		rd	1
	is_makeup		rd	1
endl
	push	ebx ecx edx esi
	mov	[is_makeup], 0

	mov	ebx, [_image]
	push	[ebx + Image.Height]
	pop	[height]
	push	[ebx + Image.Width]
	pop	[width]

	mov	edx, esi
  .next_scanline:
	push	[width]
	pop	[width_left]
	dec	[height]
	js	.error
	mov	[current_tree], tiff._.huffman_tree_white.begin
	mov	[old_tree], tiff._.huffman_tree_black.begin
	mov	ebx, 0
	mov	ecx, 8
  .next_run:
	mov	esi, [current_tree]
  .branch:
	lodsd
	btr	eax, 31
	jnc	.not_a_leaf
	cmp	eax, 63
	seta	byte[is_makeup]
	ja	@f
	push	[current_tree]
	push	[old_tree]
	pop	[current_tree]
	pop	[old_tree]
    @@:
	stdcall	tiff._.write_run, [width_left], [current_tree]
	mov	[width_left], eax
	test	byte[is_makeup], 0x01
	jnz	.next_run
	test	eax, eax
	jnz	.next_run
	jmp	.next_scanline
  .not_a_leaf:
	test	bh, bh
	jnz	@f
	mov	bl, byte[edx]
	inc	edx
	mov	bh, 8
    @@:
	test	al, 0x02
	jz	.not_a_corner
	dec	bh
	sal	bl, 1
	lahf
	and	ah, 0x03
	cmp	al, ah
	jne	.error
	mov	esi, [esi]
	jmp	.branch
  .not_a_corner:
	lodsd
	dec	bh
	sal	bl, 1
	jc	.branch
	mov	esi, eax
	jmp	.branch
  .error:
  .quit:
	pop	esi edx ecx ebx
	ret
endp


proc	tiff._.write_run _width_left, _current_tree

	push	ebx

	test	eax, eax
	jz	.done
	sub	[_width_left], eax
	js	.error
	cmp	esi, tiff._.huffman_tree_black.begin
	seta	bh

	cmp	ecx, eax
	ja	.one_byte
  .many_bytes:
	mov	bl, [edi]
    @@:
	shl	bl, 1
	or	bl, bh
	dec	eax
	dec	ecx
	jnz	@b
	mov	[edi], bl
	inc	edi
	mov	ecx, eax
	and	eax, 0x07
	shr	ecx, 3

	push	eax
	xor	eax, eax
	test	bh, bh
	jz	@f
	dec	al
    @@:
	rep	stosb
	pop	eax

	mov	ecx, 8
	test	eax, eax
	jz	.done

  .one_byte:
	mov	bl, [edi]
    @@:
	shl	bl, 1
	or	bl, bh
	dec	ecx
	dec	eax
	jnz	@b
	mov	byte[edi], bl

	cmp	[_width_left], 0
	jne	.done
	mov	bl, [edi]
	shl	bl, cl
	mov	byte[edi], bl
	inc	edi
  .done:
	mov	eax, [_width_left]
	jmp	.quit
  .error:
  .quit:
	pop	ebx
	ret
endp


proc	tiff._.get_word _endianness

	lodsw
	test	[_endianness], 1
	jnz	@f
	ret
    @@:
	xchg	al, ah
	ret
endp


proc	tiff._.get_dword _endianness

	lodsd
	test	[_endianness], 1
	jnz	@f
	ret
    @@:
	bswap	eax
	ret

	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
tiff.IFDE_tag_table.begin:
  .tag_100:		dd	0x0100,	tiff._.parse_IFDE.tag_100		; image width
  .tag_101:		dd	0x0101,	tiff._.parse_IFDE.tag_101		; image height (this is called 'length' in spec)
  .tag_102:		dd	0x0102,	tiff._.parse_IFDE.tag_102		; bits per sample
  .tag_103:		dd	0x0103,	tiff._.parse_IFDE.tag_103		; compression
  .tag_106:		dd	0x0106,	tiff._.parse_IFDE.tag_106		; photometric interpretation
  .tag_111:		dd	0x0111,	tiff._.parse_IFDE.tag_111		; strip offsets
  .tag_115:		dd	0x0115,	tiff._.parse_IFDE.tag_115		; samples per pixel
  .tag_116:		dd	0x0116,	tiff._.parse_IFDE.tag_116		; rows per strip
  .tag_117:		dd	0x0117,	tiff._.parse_IFDE.tag_117		; strip byte counts
  .tag_140:		dd	0x0140,	tiff._.parse_IFDE.tag_140		; color map
tiff.IFDE_tag_table.end:

include 'huffman.asm'		; huffman trees for ccitt1d compression method
