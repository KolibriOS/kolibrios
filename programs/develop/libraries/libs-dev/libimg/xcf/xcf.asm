;;================================================================================================;;
;;//// xcf.asm //// (c) dunkaist, 2011-2012 //////////////////////////////////////////////////////;;
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
;;                                                                                                ;;
;; References:                                                                                    ;;
;;   1. "SPECIFICATION OF THE XCF FILE FORMAT"                                                    ;;
;;      by Henning Makholm                                                                        ;;
;;      http://svn.gnome.org/viewvc/gimp/trunk/devel-docs/xcf.txt?view=markup                     ;;
;;   2. "Layer Modes"                                                                             ;;
;;      from docs.gimp.org                                                                        ;;
;;      http://docs.gimp.org/en/gimp-concepts-layer-modes.html                                    ;;
;;                                                                                                ;;
;;================================================================================================;;
include	'xcf.inc'
;include	'../../../../../system/board/trunk/debug.inc'

MAX_LAYERS		=	255

;;================================================================================================;;
proc img.is.xcf _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in xcf format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	push	edi
	xor	eax, eax

	mov	edi, [_data]

	cmp	dword[edi + xcf_header.magic_string], 'gimp'
	jne	.is_not_xcf
	cmp	dword[edi + xcf_header.magic_string + 4], ' xcf'
	jne	.is_not_xcf

	cmp	[edi + xcf_header.version], 'file'
	je	@f
	cmp	[edi + xcf_header.version], 'v001'
	je	@f
	cmp	[edi + xcf_header.version], 'v002'
	je	@f
	jmp	.is_not_xcf
    @@:

	cmp	byte[edi + xcf_header.reserved], 0
	jne	.is_not_xcf

  .is_xcf:
	inc	eax

  .is_not_xcf:
	pop	edi
	ret
endp


;;================================================================================================;;
proc img.decode.xcf _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in xcf format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
	layer_count	rd	1
	retvalue	rd	1
endl

	push	ebx esi edi

	mov	esi, [_data]
	add	esi, xcf_header.width

	lodsd
	bswap	eax
	mov	ebx, eax
	lodsd
	bswap	eax
	mov	edx, eax

	lodsd
	bswap	eax
	test	eax, eax
	jz	.process_rgb
	dec	eax
	jz	.process_grayscale
	dec	eax
	jz	.process_indexed
	jmp	.error


  .process_rgb:

	stdcall	img.create, ebx, edx, Image.bpp32
	mov	[retvalue], eax
	test	eax, eax
	jz	.error

	mov	ebx, eax

	mov	edx, XCF_BASETYPE_RGB

	jmp	.common_process

  .process_grayscale:

	stdcall	img.create, ebx, edx, Image.bpp8i
	mov	[retvalue], eax
	test	eax, eax
	jz	.error

	mov	ebx, eax

	mov	eax, [ebx + Image.Width]
	imul	[ebx + Image.Height]
	shl	eax, 1
	mov	[ebx + Image.Palette], eax
	add	eax, 256*4
	invoke	mem.realloc, [ebx + Image.Data], eax
	mov	[ebx + Image.Data], eax
	add	[ebx + Image.Palette], eax

	mov	edi, [ebx + Image.Palette]
	mov	eax, 0xff000000
    @@:
	stosd
	add	eax, 0x00010101
	jnc	@b

	mov	edx, XCF_BASETYPE_GRAY

	jmp	.common_process


  .process_indexed:

	stdcall	img.create, ebx, edx, Image.bpp8i
	mov	[retvalue], eax
	test	eax, eax
	jz	.error

	mov	ebx, eax

	mov	eax, [ebx + Image.Width]
	imul	[ebx + Image.Height]
	shl	eax, 1
	mov	[ebx + Image.Palette], eax
	add	eax, 256*4
	invoke	mem.realloc, [ebx + Image.Data], eax
	mov	[ebx + Image.Data], eax
	add	[ebx + Image.Palette], eax

	mov	edx, XCF_BASETYPE_INDEXED
;	jmp	.common_process

  .common_process:

	invoke	mem.alloc, sizeof.xcf_ext
	or	eax, eax
	jz	.error
	mov	[ebx + Image.Extended], eax
	mov	[eax + xcf_ext.opacity], 0xffffffff
	mov	[eax + xcf_ext.type], edx

	stdcall	xcf._.parse_properties, ebx

	mov	edi, esi
	xor	eax, eax
	mov	ecx, MAX_LAYERS
	mov	[layer_count], MAX_LAYERS-1
	repne	scasd
	sub	[layer_count], ecx
	mov	esi, edi
	xor	ecx, ecx

  .still:
	sub	esi, 8
	lodsd
	bswap	eax

	push	ecx
	stdcall	xcf._.decode_layer, eax, [_data]
	pop	ecx
	test	eax, eax
	jz	@f
	push	ecx
	stdcall	xcf._.merge_down, eax, [retvalue], ecx
	pop	ecx
	add	ecx, 1
    @@:
	dec	[layer_count]
	jnz	.still

	cmp	[ebx + Image.Type], Image.bpp8i
	jne	.quit
	stdcall	xcf._.pack_8a, ebx
	jmp	.quit

  .error:
	mov	[retvalue], 0
  .quit:
	pop	edi esi ebx
	mov	eax, [retvalue]
	ret
endp


;;================================================================================================;;
proc img.encode.xcf _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in xcf format                                                       ;;
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
proc	xcf._.parse_properties _img

	mov	ebx, [_img]
  .begin:
	lodsd
	bswap	eax

	mov	ecx, (xcf._.prop_table_end - xcf._.prop_table_begin)/8
	mov	edi, xcf._.prop_table_begin

  .still:
	cmp	eax, [edi]
	jne	@f
	jmp	dword[edi + 4]
    @@:
	add	edi, 8
	dec	ecx
	jnz	.still
	lodsd
	bswap	eax
	add	esi, eax
	jmp	.begin

  .00:			; PROP_END
	lodsd
	ret

  .01:			; PROP_COLORMAP
	lodsd
	mov	ecx, [ebx + Image.Extended]
	cmp	[ecx + xcf_ext.type], XCF_BASETYPE_INDEXED
	je	@f
	bswap	eax
	add	esi, eax
	jmp	xcf._.parse_properties.begin
    @@:
	lodsd
	bswap	eax
	mov	ecx, eax
	mov	edi, [ebx + Image.Palette]

    @@:
	lodsd
	sub	esi, 1
	bswap	eax
	shr	eax, 8
	or	eax, 0xff000000
	stosd
	dec	ecx
	jnz	@b
	jmp	xcf._.parse_properties.begin

  .06:			; PROP_OPACITY
	lodsd
	lodsd
	bswap	eax
	mov	ecx, [ebx + Image.Extended]
	mov	[ecx + xcf_ext.opacity], eax
	jmp	xcf._.parse_properties.begin

  .07:			; PROP_MODE
	lodsd
	lodsd
	bswap	eax
	mov	ecx, [ebx + Image.Extended]
	mov	[ecx + xcf_ext.layer_mode], eax
	jmp	xcf._.parse_properties.begin

  .08:			; PROP_VISIBLE
	lodsd
	lodsd
	bswap	eax
	mov	ecx, [ebx + Image.Extended]
	mov	[ecx + xcf_ext.visible], eax
	jmp	xcf._.parse_properties.begin

  .11:			; PROP_APPLY_MASK
	lodsd
	lodsd
	bswap	eax
	mov	ecx, [ebx + Image.Extended]
	mov	[ecx + xcf_ext.apply_mask], eax
	jmp	xcf._.parse_properties.begin

  .15:			; PROP_OFFSETS
	lodsd
	lodsd
	mov	ecx, [ebx + Image.Extended]
	bswap	eax
	mov	[ecx + xcf_ext.offset_x], eax
	lodsd
	bswap	eax
	mov	[ecx + xcf_ext.offset_y], eax
	jmp	xcf._.parse_properties.begin
endp


proc	xcf._.decode_channel _channel_begin, _data
locals
	channel_width		rd	1
	channel_height		rd	1
	planes_todo		rd	1
	total_bpl		rd	1
endl

	push	ebx esi edi
	mov	esi, [_channel_begin]
	add	esi, [_data]
	lodsd
	bswap	eax
	mov	[channel_width], eax
	mov	[total_bpl], eax
	lodsd
	bswap	eax
	mov	[channel_height], eax
	lodsd
	bswap	eax
	add	esi, eax

	stdcall	img.create, [channel_width], [channel_height], Image.bpp8i
	mov	ebx, eax
	test	ebx, ebx
	jz	.quit
	invoke	mem.alloc, sizeof.xcf_ext
	or	eax, eax
	jz	.error
	mov	[ebx + Image.Extended], eax

	stdcall	xcf._.parse_properties, ebx

	lodsd
	bswap	eax
	mov	esi, eax
	add	esi, [_data]
	lodsd
	lodsd
	lodsd
	bswap	eax
	mov	[planes_todo], eax
	lodsd
	bswap	eax
	mov	esi, eax
	add	esi, [_data]
	lodsd
	lodsd

	mov	edi, [ebx + Image.Data]
	mov	ecx, 0
    @@:
	lodsd
	test	eax, eax
	jz	.quit
	bswap	eax
	add	eax, [_data]
	stdcall	xcf._.decode_tile, eax, [channel_width], [channel_height], [total_bpl], [planes_todo], 1
	add	ecx, 1
	jmp	@b

  .error:
	stdcall	img.destroy, ebx
	mov	ebx, 0
  .quit:
	mov	eax, ebx
	pop	edi esi ebx
	ret
endp


proc	xcf._.decode_layer _layer_begin, _data
locals
	layer_width		rd	1
	layer_height		rd	1
	planes_todo		rd	1
	total_bpl		rd	1
	color_step		rd	1
endl

	push	ebx esi edi
	mov	esi, [_layer_begin]
	add	esi, [_data]
	lodsd
	bswap	eax
	mov	[layer_width], eax
	mov	[total_bpl], eax
	shl	[total_bpl], 1
	lodsd
	bswap	eax
	mov	[layer_height], eax
	lodsd
	bswap	eax
	mov	edx, Image.bpp8a
	mov	[color_step], 1
	cmp	eax, 2
	jge	@f
	mov	[color_step], 3
	mov	edx, Image.bpp32
	shl	[total_bpl], 1
    @@:
	stdcall	img.create, [layer_width], [layer_height], edx
	mov	ebx, eax
	test	ebx, ebx
	jz	.quit
	invoke	mem.alloc, sizeof.xcf_ext
	or	eax, eax
	jz	.error
	mov	[ebx + Image.Extended], eax

	lodsd
	bswap	eax
	add	esi, eax
	stdcall	xcf._.parse_properties, ebx
	mov	edx, [ebx + Image.Extended]
	or	[edx + xcf_ext.visible], 0
	jz	.unvisible

	lodsd
	bswap	eax
	push	esi
	mov	esi, eax
	add	esi, [_data]
	lodsd
	lodsd
	lodsd
	bswap	eax
	mov	[planes_todo], eax
;	mov	ecx, [ebx + Image.Extended]
;	mov	[ecx + xcf_ext.planes], eax
	lodsd
	bswap	eax
	mov	esi, eax
	add	esi, [_data]
	lodsd
	lodsd

	mov	edi, [ebx + Image.Data]
	mov	ecx, 0
    @@:
	lodsd
	test	eax, eax
	jz	@f
	bswap	eax
	add	eax, [_data]
	stdcall	xcf._.decode_tile, eax, [layer_width], [layer_height], [total_bpl], [planes_todo], 0
	add	ecx, 1
	jmp	@b
    @@:

	stdcall	xcf._.apply_opacity, ebx, [color_step]

	pop	esi
	lodsd
	bswap	eax
	test	eax, eax
	jz	.quit

	stdcall	xcf._.decode_channel, eax, [_data]
	test	eax, eax
	jz	.error

	mov	edx, [ebx + Image.Extended]
	cmp	[edx + xcf_ext.apply_mask], 0
	je	.quit

	stdcall	xcf._.apply_alpha_mask, ebx, eax, [color_step]
	jmp	.quit

  .unvisible:
  .error:
	stdcall	img.destroy, ebx
	mov	ebx, 0
  .quit:
	mov	eax, ebx
	pop	edi esi ebx
	ret
endp


proc	xcf._.decode_tile _tile_data, _width, _height, _total_bpl, _bytes_pp, _is_channel
locals
	tile_x			rd	1
	tile_y			rd	1
	tile_width		rd	1
	tile_height		rd	1
	planes_todo		rd	1
	color_step		rd	1
endl

	push	ebx ecx edx esi edi
	pushd	[_bytes_pp]
	popd	[planes_todo]

	cmp	[_is_channel], 1
	je	@f
	test	[_bytes_pp], 0x01
	jz	@f
	add	[_bytes_pp], 1
    @@:
	mov	ebx, [_bytes_pp]
	sub	ebx, 1
	mov	[color_step], ebx

	mov	esi, [_tile_data]
	mov	eax, ecx
	mov	ebx, [_width]
	dec	ebx
	shr	ebx, 6
	inc	ebx
	mov	edx, 0
	div	bx
	mov	[tile_x], edx
	mov	[tile_y], eax

	mov	[tile_width], 64
	mov	ebx, [_width]
	test	ebx, 0x0000003F
	jz	@f
	dec	ebx
	shr	ebx, 6
	cmp	ebx, [tile_x]
	jne	@f
	mov	ebx, [_width]
	and	ebx, 0x0000003F
	mov	[tile_width], ebx
    @@:

	mov	[tile_height], 64
	mov	ebx, [_height]
	test	ebx, 0x0000003F
	jz	@f
	dec	ebx
	shr	ebx, 6
	cmp	ebx, [tile_y]
	jne	@f
	mov	ebx, [_height]
	and	ebx, 0x0000003F
	mov	[tile_height], ebx
    @@:


	mov	eax, [_total_bpl]
	shl	eax, 6
	mul	[tile_y]
	add	edi, eax

	mov	eax, [tile_x]
	shl	eax, 6
	imul	eax, [_bytes_pp]
	add	edi, eax

	cmp	[_is_channel], 1
	jne	@f
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	jmp	.quit
    @@:
	mov	eax, [planes_todo]
	dec	eax
	jz	.p1
	dec	eax
	jz	.p2
	dec	eax
	jz	.p3
	jmp	.p4
  .p1:
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	add	edi, 1
	stdcall	xcf._.fill_color, [tile_width], [tile_height], [_total_bpl], [_bytes_pp], [color_step]
	jmp	.quit
  .p2:
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	add	edi, 1
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	jmp	.quit
  .p3:
	add	edi, 2
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	sub	edi, 1
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	sub	edi, 1
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	add	edi, 3
	stdcall	xcf._.fill_color, [tile_width], [tile_height], [_total_bpl], [_bytes_pp], [color_step]
	jmp	.quit
  .p4:
	add	edi, 2
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	sub	edi, 1
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	sub	edi, 1
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
	add	edi, 3
	stdcall	xcf._.decode_color, [tile_width], [tile_height], [color_step], [_total_bpl], [_bytes_pp]
;	jmp	.quit

  .quit:
	pop	edi esi edx ecx ebx
	ret
endp


proc	xcf._.fill_color _tile_width, _tile_height, _total_bpl, _bytes_pp, _color_step
	push	ebx
	mov	edx, [_color_step]
	mov	ebx, [_total_bpl]
	mov	eax, [_bytes_pp]
	mul	byte[_tile_width]
	sub	ebx, eax

	mov	ch, byte[_tile_height]
	mov	al, 0xff
  .line:
	mov	cl, byte[_tile_width]
    @@:
	stosb
	add	edi, edx
	dec	cl
	jnz	@b
	add	edi, ebx
	dec	ch
	jnz	.line
	pop	ebx
	ret
endp


proc	xcf._.decode_color _tile_width, _tile_height, _color_step, _total_bpl, _bytes_pp
locals
	level_width		rd	1
	level_height		rd	1
	line_step		rd	1	; [_total_bpl] - [_tile_width]*[_bytes_pp]
endl

	push	edi

	mov	ebx, [_total_bpl]
	movzx	eax, byte[_bytes_pp]
	mul	byte[_tile_width]
	sub	ebx, eax
	mov	[line_step], ebx
	mov	ebx, [_tile_height]
	mov	edx, [_tile_width]

  .decode:
	lodsb
	cmp	al, 127
	je	.long_identical
	jb	.short_identical
	test	al, 0x7f
	jz	.long_different
	jmp	.short_different

  .short_identical:
	movzx	ecx, al
	add	ecx, 1
	lodsb
	jmp	.step1
  .long_identical:
	mov	ecx, 0
	lodsw
	mov	cx, ax
	xchg	cl, ch
	lodsb
  .step1:
	cmp	cx, dx
	je	.step2
	jl	.step3
	xchg	cx, dx
	sub	dx, cx
	sub	bx, 1
    @@:
	stosb
	add	edi, [_color_step]
	loop	@b
	mov	cx, dx
	mov	edx, [_tile_width]
	add	edi, [line_step]
	jmp	.step1

  .step2:
    @@:
	stosb
	add	edi, [_color_step]
	loop	@b
	mov	edx, [_tile_width]
	add	edi, [line_step]
	dec	bx
	jz	.quit
	jmp	.decode
  .step3:
	sub	dx, cx
    @@:
	stosb
	add	edi, [_color_step]
	loop	@b
	jmp	.decode


  .short_different:
	movzx	ecx, al
	neg	cx
	add	cx, 256
	jmp	.step4
  .long_different:
	mov	ecx, 0
	lodsb
	mov	ch, al
	lodsb
	mov	cl, al
  .step4:
	cmp	cx, dx
	je	.step5
	jl	.step6
	xchg	cx, dx
	sub	dx, cx
	sub	bx, 1
    @@:
	movsb
	add	edi, [_color_step]
	loop	@b
	mov	cx, dx
	mov	edx, [_tile_width]
	add	edi, [line_step]
	jmp	.step4

  .step5:
    @@:
	movsb
	add	edi, [_color_step]
	loop	@b
	mov	edx, [_tile_width]
	add	edi, [line_step]
	dec	bx
	jz	.quit
	jmp	.decode

  .step6:
	sub	dx, cx
    @@:
	movsb
	add	edi, [_color_step]
	loop	@b
	jmp	.decode

  .quit:
	pop	edi
	ret
endp


proc	xcf._.merge_down _img, _bottom, _layer_number
locals
	copy_width		rd	1
	copy_height		rd	1
	img_x1			rd	1
	img_y1			rd	1
	bottom_x1		rd	1
	bottom_y1		rd	1
	img_total_bpl		rd	1
	bottom_total_bpl	rd	1
	img_length		rd	1
	bottom_length		rd	1
endl
	push	ebx esi edi

	mov	ebx, [_bottom]
	mov	edx, [_img]

	mov	[img_x1], 0
	push	[edx + Image.Width]
	pop	[img_length]

	mov	[bottom_x1], 0
	mov	ecx, [ebx + Image.Width]
	mov	[bottom_length], ecx        

	mov	eax, [edx + Image.Extended]
	movsx	eax, word[eax + xcf_ext.offset_x]
	cmp	eax, 0
	jg	.greater_x
	jl	.lesser_x
	mov	[copy_width], ecx
	jmp	.done_x
  .greater_x:
	add	[bottom_x1], eax
	sub	[bottom_length], eax
	jns	.label_x
	mov	[copy_width], 0
	jmp	.done_x
  .lesser_x:
	sub	[img_x1], eax
	add	[img_length], eax
	jns	.label_x
	mov	[copy_width], 0
	jmp	.done_x
  .label_x:
	mov	ecx, [img_length]
	cmp	ecx, [bottom_length]
	jng	@f
	mov	ecx, [bottom_length]
    @@:
	mov	[copy_width], ecx
  .done_x:


	mov	[img_y1], 0
	push	[edx + Image.Height]
	pop	[img_length]

	mov	[bottom_y1], 0
	mov	ecx, [ebx + Image.Height]
	mov	[bottom_length], ecx        

	mov	eax, [edx + Image.Extended]
	movsx	eax, word[eax + xcf_ext.offset_y]
	cmp	eax, 0
	jg	.greater_y
	jl	.lesser_y
	mov	[copy_height], ecx
	jmp	.done_y
  .greater_y:
	add	[bottom_y1], eax
	sub	[bottom_length], eax
	jns	.label_y
	mov	[copy_height], 0
	jmp	.done_y
  .lesser_y:
	sub	[img_y1], eax
	add	[img_length], eax
	jns	.label_y
	mov	[copy_height], 0
	jmp	.done_y
  .label_y:
	mov	ecx, [img_length]
	cmp	ecx, [bottom_length]
	jng	@f
	mov	ecx, [bottom_length]
    @@:
	mov	[copy_height], ecx
  .done_y:

	mov	esi, [edx + Image.Data]
	mov	edi, [ebx + Image.Data]

	mov	eax, [edx + Image.Width]
	imul	eax, [img_y1]
	add	eax, [img_x1]
	shl	eax, 1
	cmp	[edx + Image.Width], Image.bpp8a
	je	@f
	shl	eax, 1
    @@:
	add	esi, eax

	mov	eax, [ebx + Image.Width]
	imul	eax, [bottom_y1]
	add	eax, [bottom_x1]
	shl	eax, 1
	cmp	[ebx + Image.Width], Image.bpp8i
	je	@f
	shl	eax, 1
    @@:
	add	edi, eax


	mov	eax, [edx + Image.Width]
	sub	eax, [copy_width]
	shl	eax, 1
	cmp	[edx + Image.Width], Image.bpp8a
	je	@f
	shl	eax, 1
    @@:
	mov	[img_total_bpl], eax

	mov	eax, [ebx + Image.Width]
	sub	eax, [copy_width]
	shl	eax, 1
	cmp	[ebx + Image.Width], Image.bpp8i
	je	@f
	shl	eax, 1
    @@:
	mov	[bottom_total_bpl], eax

	cmp	[_layer_number], 0
	jne	.not_first
	mov	ecx, [copy_width]
	imul	ecx, [copy_height]
	cmp	[ebx + Image.Type], Image.bpp8i
	je	.bpp8a
  .bpp32:
	rep	movsd
	jmp	.done
  .bpp8a:
	rep	movsw
	jmp	.done
  .not_first:

	push	edi
	mov	ecx, [edx + Image.Extended]
	mov	eax, [ecx + xcf_ext.layer_mode]

	mov	ecx, [ebx + Image.Extended]
	mov	ecx, [ecx + xcf_ext.type]

	cmp	ecx, XCF_BASETYPE_RGB
	jne	@f
	mov	edx, 4
	jmp	.type_defined
    @@:
	cmp	ecx, XCF_BASETYPE_GRAY
	jne	@f
	mov	edx, 8
	jmp	.type_defined
    @@:
	mov	edx, 12
  .type_defined:
	mov	ecx, (xcf._.composite_table.end - xcf._.composite_table.begin) / 8
	mov	edi, xcf._.composite_table.begin

  .still:
	cmp	eax, [edi]
	jne	@f
	add	edi, edx
	mov	edx, [edi]
	jmp	.composite_found
    @@: 
	add	edi, 16
	dec	ecx
	jnz	.still

  .composite_found:
	pop	edi

	mov	ecx, [ebx + Image.Extended]
	cmp	[ecx + xcf_ext.type], XCF_BASETYPE_INDEXED
	jne	@f
	stdcall	edx, [copy_width], [copy_height], [bottom_total_bpl], [img_total_bpl]
	jmp	.done
    @@:
	cmp	eax, 1
	ja	@f
	stdcall	edx, [copy_width], [copy_height], [bottom_total_bpl], [img_total_bpl]
	jmp	.done
    @@:


	cmp	[ebx + Image.Type], Image.bpp8i
	jne	@f
	stdcall	xcf._.merge_8a, [copy_width], [copy_height], [img_total_bpl], [bottom_total_bpl]
	jmp	.done
    @@:
	stdcall	xcf._.merge_32, [copy_width], [copy_height], [img_total_bpl], [bottom_total_bpl]
;	jmp	.done
  .done:
	stdcall	img.destroy, [_img]
	pop	edi esi ebx
	ret
endp


proc	xcf._.pack_8a _img
	mov	ebx, [_img]
	mov	esi, [ebx + Image.Data]
	mov	edi, esi
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
    @@:
	lodsw
	stosb
	dec	ecx
	jnz	@b
	ret
endp


proc	xcf._.apply_opacity _img, _color_step

	push	ebx

	mov	edx, [ebx + Image.Extended]
	mov	edx, [edx + xcf_ext.opacity]
	cmp	dl, 0xff
	je	.quit

	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
	mov	esi, [ebx + Image.Data]
	mov	ebx, [_color_step]
	add	esi, ebx
	mov	edi, esi
    @@:
	lodsb
	mul	dl
	shr	ax, 8
	stosb
	add	esi, ebx
	add	edi, ebx
	dec	ecx
	jnz	@b

  .quit:
	pop	ebx
	ret
endp


proc	xcf._.apply_alpha_mask _img, _mask, _color_step

	push	ebx

	mov	ebx, [_img]
	mov	esi, [_mask]
	mov	esi, [esi + Image.Data]
	mov	edi, [ebx + Image.Data]
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
	mov	ebx, [_color_step]
	add	edi, ebx
    @@:
	lodsb
	mul	byte[edi]
	shr	ax, 8
	stosb
	add	edi, ebx
	dec	ecx
	jnz	@b

	stdcall	img.destroy, [_mask]
	pop	ebx
	ret
endp


;;================================================================================================;;
proc	xcf._.rgb2hsv ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? convert color from RGB to HSV space                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = color (0xAARRGGBB)                                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = color (0xAAHHSSVV)                                                                       ;;
;;================================================================================================;;
locals
	vsha		rd	1
	max		rd	1
	min		rd	1
	med		rd	1
endl
	push	ebx ecx edx

	mov	[vsha], eax
	movzx	eax, byte[vsha]		; eax = al = blue
	movzx	ecx, byte[vsha+1]	; ecx = cl = green
	movzx	edx, byte[vsha+2]	; edx = dl = red

	cmp	al, cl
	jne	@f
	cmp	al, dl
	jne	@f
	ror	eax, 8
	mov	ax, 0
	rol	eax, 8
	jmp	.quit

    @@:
	cmp	dl, cl
	ja	@f
	cmp	dl, al
	ja	@f
	mov	byte[min], dl
	jmp	.min_found
    @@:
	cmp	cl, al
	ja	@f
	cmp	cl, dl
	ja	@f
	mov	byte[min], cl
	jmp	.min_found
    @@:
	mov	byte[min], al
;	jmp	.min_found
  .min_found:

	cmp	dl, cl
	jb	@f
	cmp	dl, al
	jb	@f
	mov	byte[max], dl
	sub	cx, ax
	mov	dx, cx
	mov	cx, 0
	jmp	.max_found
    @@:
	cmp	cl, al
	jb	@f
	cmp	cl, dl
	jb	@f
	mov	byte[max], cl
	sub	ax, dx
	mov	dx, ax
	mov	cx, 85
	jmp	.max_found
    @@:
	mov	byte[max], al
	sub	dx, cx
	mov	cx, 171
;	jmp	.max_found
  .max_found:

	mov	al, byte[max]
	sub	al, byte[min]
	mov	byte[med], al


	imul	dx, 43
	movsx	eax, dx
	ror	eax, 16
	mov	dx, ax
	rol	eax, 16
	mov	byte[med + 1], 0
	idiv	word[med]
	add	al, cl
	mov	byte[vsha + 2], al

	mov	al, byte[max]
	mov	byte[vsha], al

	mov	byte[vsha + 1], 0
	test	al, al
	jz	@f
	mov	byte[vsha + 1], 0xff
	cmp	al, byte[med]
	je	@f
	mov	al, byte[med]
	shl	ax, 8
	div	byte[max]
	mov	byte[vsha + 1], al
    @@:
	mov	eax, [vsha]

  .quit:
	pop	edx ecx ebx
	ret
endp


;;================================================================================================;;
proc	xcf._.hsv2rgb ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? convert color from HSV to RGB space                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = color (0xAAHHSSVV)                                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = color (0xAARRGGBB)                                                                       ;;
;;================================================================================================;;
locals
	vsha	rd	1
	f	rb	1
	c	rb	1
	x	rb	1
endl

	push	ebx ecx edx


	mov	[vsha], eax
	mov	bl, byte[vsha + 1]
	mul	bl
	mov	byte[c], ah

	movzx	eax, byte[vsha + 2]
	cmp	eax, 43
	ja	@f
	lea	eax, [eax*3]
	shl	eax, 1
	mov	ebx, eax
	shr	ebx, 7
	sub	eax, ebx
	shr	ebx, 1
	sub	eax, ebx
	jmp	.ok

    @@:
	cmp	eax, 86
	ja	@f
	sub	eax, 44
	lea	eax, [eax*3]
	shl	eax, 1
	neg	al
	add	al, 0xff
	jmp	.ok

    @@:
	cmp	eax, 129
	ja	@f
	sub	eax, 87
	lea	eax, [eax*3]
	shl	eax, 1
	jmp	.ok

    @@:
	cmp	eax, 171
	ja	@f
	sub	eax, 130
	lea	eax, [eax*3]
	shl	eax, 1
	neg	al
	add	al, 0xff
	jmp	.ok

    @@:
	cmp	eax, 214
	ja	@f
	sub	eax, 172
	lea	eax, [eax*3]
	shl	eax, 1
	jmp	.ok
    @@:
	sub	eax, 215
	lea	eax, [eax*3]
	shl	eax, 1
	neg	al
	add	al, 0xff
;	jmp	.ok
  .ok:

	neg	al
	add	al, 0xff
	neg	al
	add	al, 0xff
;	shr	ax, 8
	mul	byte[c]
	mov	byte[x], ah



	mov	al, byte[vsha+2]
	cmp	al, 43
	jae	@f
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, byte[c]
	shl	eax, 8
	mov	ah, byte[x]
	mov	al, 0
	jmp	.done

    @@:
	cmp	al, 86
	jae	@f
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, byte[x]
	shl	eax, 8
	mov	ah, byte[c]
	mov	al, 0
	jmp	.done

    @@:
	cmp	al, 129
	jae	@f
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, 0
	shl	eax, 8
	mov	ah, byte[c]
	mov	al, byte[x]
	jmp	.done

    @@:
	cmp	al, 171
	jae	@f
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, 0
	shl	eax, 8
	mov	ah, byte[x]
	mov	al, byte[c]
	jmp	.done

    @@:
	cmp	al, 214
	jae	@f
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, byte[x]
	shl	eax, 8
	mov	ah, 0
	mov	al, byte[c]
	jmp	.done

    @@:
	mov	eax, [vsha]
	shr	eax, 8
	mov	ah, byte[c]
	shl	eax, 8
	mov	ah, 0
	mov	al, byte[x]
;	jmp	.done

  .done:
	mov	bl, byte[vsha]
	sub	bl, byte[c]
	ror	eax, 8
	add	ah, bl
	rol	eax, 8
	add	ah, bl
	add	al, bl

  .quit:
	pop	edx ecx ebx
	ret
endp


;;================================================================================================;;
proc	xcf._.rgb2hsl ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? convert color from RGB to HSL space                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = color (0xAARRGGBB)                                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = color (0xAAHHSSLL)                                                                       ;;
;;================================================================================================;;
; http://www.asmcommunity.net/board/index.php?topic=7425
; iblis: "I don't know what X-Filez is, but yes you may use it however you wish.  That's why I made this post, to share."
; so pixel_rgb2hsl procedure is based on code by Greg Hoyer (iblis). thanks!
;--------------------------------------------------------------;
; By Greg Hoyer aka "Iblis"                                    ;
;                                                              ;
; RGB2HSL converts a COLORREF  oriented dword filled with 8bit ;
; Red/Green/Blue  values  (00ggbbrr) to  a similarly  oriented ;
; dword filled with Hue/Saturation/Luminance values (00llsshh) ;
; This procedure  returns the  full range,  from 0-255.   This ;
; offers slightly more  precision over Windows' "color picker" ;
; common dialog, which displays HSL values ranging from 0-240. ;
;                                                              ;
; It is important to note  that true HSL  values are  normally ;
; represented as  floating point  fractions from  0.0 to  1.0. ;
; As such, this  algorithm cannot  be used to  do the precise, ;
; consistent conversions  that may  be required  by heavy-duty ;
; graphics  applications.  To  get the  decimal  fraction  for ;
; the  returned values,  convert  the Hue,  Saturation, and/or ;
; Luminance values to floating  point, and then divide by 255. ;
;--------------------------------------------------------------;
locals
	bgra	rd	1
endl
	push	ebx esi edi

	mov	[bgra], eax

	movzx	esi, byte[bgra + 0]
	movzx	edi, byte[bgra + 1]
        movzx   ebx, byte[bgra + 2]
	mov	cl, -1
	cmp	esi, edi
	ja	.cmp1
	xchg	esi, edi
	neg	cl
	shl	cl, 1
  .cmp1:
	cmp	edi, ebx
	jb	.cmp2
	xchg	edi, ebx
	neg	cl
  .cmp2:
	cmp	esi, ebx
	ja	.cmp3
	xchg	esi, ebx
	not	cl
  .cmp3:
	neg	ebx
	add	ebx, esi
	mov	eax, edi
	add	edi, esi
	jz	.done
	sub	esi, eax
	jz	.done
	mov	eax, esi
	shl	eax, 8
	sub	eax, esi
	push	edi
	cmp	edi, 0xff
	jbe	.csat
	neg	edi
	add	edi, 510
  .csat:
	xor	edx, edx
	div	edi
	pop      edi
	shr	edi, 1
	shl	eax, 8
	or	edi, eax
	add	cl, 3
	jnc	.noneg
	neg	ebx
  .noneg:
	shl	cl, 2
	mov	eax, 0x13135db9
	shr	eax, cl
	and	eax, 7
	mul	esi
	add	eax, ebx
	mov	ebx, eax
	shl	eax, 8
	sub	eax, ebx
	mov	ebx, esi
	shl	esi, 1
	lea	ebx, [ebx*4 + esi]
	xor	edx, edx
	div	ebx
	shl	eax, 16
	or	eax, edi
  .done:
	bswap	eax
	shr	eax, 8

	mov	bl, byte[bgra + 3]
	bswap	eax
	mov	al, bl
	ror	eax, 8


	pop	edi esi ebx
	ret
endp


;;================================================================================================;;
proc	xcf._.hsl2rgb ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? convert color from HSL to RGB space                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = color (0xAAHHSSLL)                                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = color (0xAARRGGBB)                                                                       ;;
;;================================================================================================;;
; http://www.asmcommunity.net/board/index.php?topic=7425
; iblis: "I don't know what X-Filez is, but yes you may use it however you wish.  That's why I made this post, to share."
; so pixel_hsl2rgb procedure is based on code by Greg Hoyer (iblis). thanks!
;--------------------------------------------------------------;
; By Greg Hoyer aka "Iblis"                                    ;
;                                                              ;
; HSL2RGB  does  the  opposite  of  RGB2HSL.   It  converts  a ;
; Hue/Saturation/Luminance  (00llsshh)  dword  back  into  its ;
; corresponding  RGB  COLORREF  (00bbggrr).  This  function is ;
; intented  to be  used exclusively  with RGB2HSL  (see above) ;
;                                                              ;
; If  you're  using  this for  your own  custom  color-chooser ;
; dialog, remember that the  values are in the range of 0-255. ;
; If you MUST emulate the Windows'  color-chooser, convert HSL ;
; values this way before you display them:                     ;
;                                                              ;
; display_value = ( x * 240 ) / 255                            ;
;                                                              ;
; ...where x represents any one of the HSL values.             ;
;--------------------------------------------------------------;
locals
	lsha	rd	1
endl

	push	ebx esi edi

	mov	[lsha], eax

	movzx	ebx, byte[lsha + 0]
	lea	esi, [ebx*2]
	movzx	edi, byte[lsha + 1]
	xor	eax, eax
	mov	cl, 1
	cmp	bl, 0x7f
	ja	.lcase
	dec	al
	xor	ecx, ecx
  .lcase:
	add	eax, edi
	mul	ebx
	or	ecx, ecx
	jz	.scase
	neg	eax
	mov	ecx, ebx
	add	ecx, edi
	mov	edx, ecx
	shl	ecx, 8
	sub	ecx, edx
	add	eax, ecx
  .scase:
	xor	edx, edx
	xor	ecx, ecx
	dec	cl
	mov	edi, ecx
	div	ecx
	jz	.done
	mov	ecx, eax
	sub	esi, eax
	movzx	eax, byte[lsha + 2]
	mov	ebx, eax
	shl	eax, 1
	lea	eax, [ebx*4 + eax]
	xor	edx, edx
	div	edi
	mov	ebx, eax
	mov	eax, ecx
	sub	eax, esi
	mul	edx
	push	ebx
	mov	ebx, ecx
	shl	ebx, 8
	sub	ebx, ecx
	sub	ebx, eax
	xchg	eax, ebx
	xor	edx, edx
	div	edi
	shl	eax, 24
	or	ecx, eax
	mov	eax, esi
	shl	eax, 8
	sub	eax, esi
	shl	esi, 16
	or	ecx, esi
	add	eax, ebx
	xor	edx, edx
	div	edi
	mov	ch, al
	mov	eax, ecx
	pop	ecx
	cmp	cl, 6
	jz	.done
	or	ecx, ecx
	jz	.done
	bswap	eax
	rol	eax, 8
	xchg	ah, al
	dec	ecx
	jz	.done
	ror	eax, 8
	xchg	ah, al
	dec	ecx
	jz	.done
	rol	eax, 8
	xchg	ah, al
	dec	ecx
	jz	.done
	bswap	eax
	rol	eax, 8
	xchg	ah, al
	dec	ecx
	jz	.done
	ror	eax, 8
	xchg	ah, al
  .done:
	and	eax, 0x00ffffff

	mov	bl, byte[lsha + 3]
	bswap	eax
	mov	al, bl
	ror	eax, 8

	pop	edi esi ebx


	ret
endp

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
xcf._.prop_table_begin:
		dd	00, xcf._.parse_properties.00
		dd	01, xcf._.parse_properties.01
		dd	06, xcf._.parse_properties.06
		dd	07, xcf._.parse_properties.07
		dd	08, xcf._.parse_properties.08
		dd	11, xcf._.parse_properties.11
		dd	15, xcf._.parse_properties.15
xcf._.prop_table_end:
