;;================================================================================================;;
;;//// libimg.asm //// (c) mike.dld, 2007-2008, (c) diamond, 2009 ////////////////////////////////;;
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


format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../struct.inc'
include '../../../../proc32.inc'
include '../../../../macros.inc'
purge section,mov,add,sub

include 'libimg.inc'

section '.flat' code readable align 16

include 'bmp/bmp.asm'
include 'gif/gif.asm'
include 'jpeg/jpeg.asm'
include 'png/png.asm'

mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

;;================================================================================================;;
proc lib_init ;///////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Library entry point (called after library load)                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = pointer to memory allocation routine                                                     ;;
;> ebx = pointer to memory freeing routine                                                        ;;
;> ecx = pointer to memory reallocation routine                                                   ;;
;> edx = pointer to library loading routine                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 1 (fail) / 0 (ok) (library initialization result)                                        ;;
;;================================================================================================;;
	mov	[mem.alloc], eax
	mov	[mem.free], ebx
	mov	[mem.realloc], ecx
	mov	[dll.load], edx

	call	img.initialize.jpeg

  .ok:	xor	eax,eax
	ret
endp

;;================================================================================================;;
proc img.is_img _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ebx
	mov	ebx, img._.formats_table
    @@: stdcall [ebx + FormatsTableEntry.Is], [_data], [_length]
	or	eax, eax
	jnz	@f
	add	ebx, sizeof.FormatsTableEntry
	cmp	dword[ebx], 0
	jnz	@b
	xor	eax, eax
    @@: pop	ebx
	ret
endp

;;================================================================================================;;
proc img.info _data, _length ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.from_file _filename ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                                     ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.to_file _img, _filename ;////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.from_rgb _rgb_data ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                                     ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.to_rgb2 _img, _out ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	esi edi
	mov	esi, [_img]
	stdcall	img._.validate, esi
	or	eax, eax
	jnz	.ret
	mov	edi, [_out]
	call	img._.do_rgb
.ret:
	pop	edi esi
	ret
endp

;;================================================================================================;;
proc img.to_rgb _img ;////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to rgb_data (array of [rgb] triplets)                                        ;;
;;================================================================================================;;
	push	esi edi
	mov	esi, [_img]
	stdcall img._.validate, esi
	or	eax, eax
	jnz	.error

	mov	esi, [_img]
	mov	ecx, [esi + Image.Width]
	imul	ecx, [esi + Image.Height]
	lea	eax, [ecx * 3 + 4 * 3]
	invoke	mem.alloc, eax
	or	eax, eax
	jz	.error

	mov	edi, eax
	push	eax
	mov	eax, [esi + Image.Width]
	stosd
	mov	eax, [esi + Image.Height]
	stosd
	call	img._.do_rgb
	pop	eax
	pop	edi esi
	ret

  .error:
	xor	eax, eax
	pop	edi esi
	ret
endp

;;================================================================================================;;
proc img._.do_rgb ;///////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [esi + Image.Width]
	imul	ecx, [esi + Image.Height]
	mov	eax, [esi + Image.Type]
	dec	eax
	jz	.bpp8
	dec	eax
	jz	.bpp24
; 32 BPP -> 24 BPP
	mov	esi, [esi + Image.Data]

    @@: dec	ecx
	js	@f
	movsd
	dec	edi
	jmp	@b

    @@:
	ret

.bpp24:
; 24 BPP -> 24 BPP
	lea	ecx, [ecx*3 + 3]
	mov	esi, [esi + Image.Data]
	shr	ecx, 2
	rep	movsd
	ret

.bpp8:
; 8 BPP -> 24 BPP
	push	ebx
	mov	ebx, [esi + Image.Palette]
	mov	esi, [esi + Image.Data]
@@:
	movzx	eax, byte [esi]
	add	esi, 1
	mov	eax, [ebx + eax*4]
	mov	[edi], eax
	add	edi, 3
	sub	ecx, 1
	jnz	@b
	pop	ebx
	ret
endp

;;================================================================================================;;
proc img.decode _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                                     ;;
;;================================================================================================;;
	push	ebx
	mov	ebx, img._.formats_table
    @@: stdcall [ebx + FormatsTableEntry.Is], [_data], [_length]
	or	eax, eax
	jnz	@f
	add	ebx, sizeof.FormatsTableEntry
	cmp	dword[ebx], 0
	jnz	@b
	jmp	.error
    @@: stdcall [ebx + FormatsTableEntry.Decode], [_data], [_length]

  .error:
	ret
endp

;;================================================================================================;;
proc img.encode _img, _p_length ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                                              ;;
;< [_p_length] = data length                                                                      ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.create _width, _height, _type ;//////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                                     ;;
;;================================================================================================;;
	push	ecx

	stdcall img._.new
	or	eax, eax
	jz	.error

	mov	ecx, [_type]
	mov	[eax + Image.Type], ecx

	push	eax

	stdcall img._.resize_data, eax, [_width], [_height]
	or	eax, eax
	jz	.error.2

	pop	eax
	jmp	.ret

  .error.2:
;       pop     eax
	stdcall img._.delete; eax
	xor	eax, eax

  .error:
  .ret:
	pop	ecx
	ret
endp

;;================================================================================================;;
proc img.destroy _img ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	;TODO: link Next and Previous
	stdcall img._.delete, [_img]
	ret
endp

;;================================================================================================;;
proc img.count _img ;/////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Get number of images in the list (e.g. in animated GIF file)                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (fail) / >0 (ok)                                                                      ;;
;;================================================================================================;;
	push	ecx edx
	mov	edx, [_img]
	stdcall img._.validate, edx
	or	eax, eax
	jnz	.error

    @@: mov	eax, [edx + Image.Previous]
	or	eax, eax
	jz	@f
	mov	edx, eax
	jmp	@b

    @@: xor	ecx, ecx
    @@: inc	ecx
	mov	eax, [edx + Image.Next]
	or	eax, eax
	jz	.exit
	mov	edx, eax
	jmp	@b

  .exit:
	mov	eax, ecx
	pop	edx ecx
	ret

  .error:
	or	eax, -1
	pop	edx ecx
	ret
endp

;;//// image processing //////////////////////////////////////////////////////////////////////////;;

;;================================================================================================;;
proc img.lock_bits _img, _start_line, _end_line ;/////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to bits                                                                      ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.unlock_bits _img, _lock ;////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.flip _img, _flip_kind ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Flip image                                                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;> _flip_kind = one of FLIP_* constants                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
locals
  scanline_len dd ?
endl

	push	ebx esi edi
	mov	ebx, [_img]
	stdcall img._.validate, ebx
	or	eax, eax
	jnz	.error

	mov	ecx, [ebx + Image.Height]
	mov	eax, [ebx + Image.Width]
	call	img._.get_scanline_len
	mov	[scanline_len], eax

	test	[_flip_kind], FLIP_VERTICAL
	jz	.dont_flip_vert

	imul	eax, ecx
	sub	eax, [scanline_len]
	shr	ecx, 1
	mov	esi, [ebx + Image.Data]
	lea	edi, [esi + eax]
	
  .next_line_vert:
	push	ecx

	mov	ecx, [scanline_len]
	push	ecx
	shr	ecx, 2
    @@: mov	eax, [esi]
	xchg	eax, [edi]
	mov	[esi], eax
	add	esi, 4
	add	edi, 4
	sub	ecx, 1
	jnz	@b
	pop	ecx
	and	ecx, 3
	jz	.cont_line_vert
    @@:
	mov	al, [esi]
	xchg	al, [edi]
	mov	[esi], al
	add	esi, 1
	add	edi, 1
	dec	ecx
	jnz	@b
    .cont_line_vert:

	pop	ecx
	mov	eax, [scanline_len]
	shl	eax, 1
	sub	edi, eax
	dec	ecx
	jnz	.next_line_vert

  .dont_flip_vert:

	test	[_flip_kind], FLIP_HORIZONTAL
	jz	.exit

	mov	ecx, [ebx + Image.Height]
	mov	eax, [ebx + Image.Type]
	mov	esi, [ebx + Image.Data]
	mov	edi, [scanline_len]
	add	edi, esi

	dec	eax
	jz	.bpp8.2
	dec	eax
	jz	.bpp24.2

	sub	edi, 4

  .next_line_horz:
	push	ecx esi edi

	mov	ecx, [scanline_len]
	shr	ecx, 3
    @@: mov	eax, [esi]
	xchg	eax, [edi]
	mov	[esi], eax
	add	esi, 4
	add	edi, -4
	sub	ecx, 1
	jnz	@b

	pop	edi esi ecx
	add	esi, [scanline_len]
	add	edi, [scanline_len]
	dec	ecx
	jnz	.next_line_horz
	jmp	.exit

.bpp8.2:
	dec	edi
  .next_line_horz8:
	push	ecx esi edi

	mov	ecx, [scanline_len]
	shr	ecx, 1
    @@: mov	al, [esi]
	mov	dl, [edi]
	mov	[edi], al
	mov	[esi], dl
	add	esi, 1
	sub	edi, 1
	sub	ecx, 1
	jnz	@b

	pop	edi esi ecx
	add	esi, [scanline_len]
	add	edi, [scanline_len]
	dec	ecx
	jnz	.next_line_horz8
	jmp	.exit

.bpp24.2:
	sub	edi, 3
  .next_line_horz32:
	push	ecx esi edi

	mov	ecx, [ebx + Image.Width]
	shr	ecx, 1
    @@:
	mov	al, [esi]
	mov	dl, [edi]
	mov	[edi], al
	mov	[esi], dl
	mov	al, [esi+1]
	mov	dl, [edi+1]
	mov	[edi+1], al
	mov	[esi+1], dl
	mov	al, [esi+2]
	mov	dl, [edi+2]
	mov	[edi+2], al
	mov	[esi+2], dl
	add	esi, 3
	sub	edi, 3
	sub	ecx, 1
	jnz	@b

	pop	edi esi ecx
	add	esi, [scanline_len]
	add	edi, [scanline_len]
	dec	ecx
	jnz	.next_line_horz32

  .exit:
	xor	eax, eax
	inc	eax
	pop	edi esi ebx
	ret

  .error:
	xor	eax, eax
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc img.rotate _img, _rotate_kind ;//////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Rotate image                                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;> _rotate_kind = one of ROTATE_* constants                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
locals
  scanline_len_old    dd ?
  scanline_len_new    dd ?
  scanline_pixels_new dd ?
  line_buffer	      dd ?
  pixels_ptr	      dd ?
endl

	mov	[line_buffer], 0

	push	ebx esi edi
	mov	ebx, [_img]
	stdcall img._.validate, ebx
	or	eax, eax
	jnz	.error

	cmp	[_rotate_kind], ROTATE_90_CCW
	je	.rotate_ccw_low
	cmp	[_rotate_kind], ROTATE_90_CW
	je	.rotate_cw_low
	cmp	[_rotate_kind], ROTATE_180
	je	.flip
	jmp	.exit

  .rotate_ccw_low:
	mov	eax, [ebx + Image.Height]
	mov	[scanline_pixels_new], eax
	call	img._.get_scanline_len
	mov	[scanline_len_new], eax

	invoke	mem.alloc, eax
	or	eax, eax
	jz	.error
	mov	[line_buffer], eax

	mov	eax, [ebx + Image.Width]
	mov	ecx, eax
	call	img._.get_scanline_len
	mov	[scanline_len_old], eax

	mov	eax, [scanline_len_new]
	imul	eax, ecx
	add	eax, [ebx + Image.Data]
	mov	[pixels_ptr], eax

	cmp	[ebx + Image.Type], Image.bpp8
	jz	.rotate_ccw8
	cmp	[ebx + Image.Type], Image.bpp24
	jz	.rotate_ccw24

  .next_column_ccw_low:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -4

	mov	ecx, [scanline_pixels_new]
	mov	esi, [ebx + Image.Data]
	mov	edi, [line_buffer]
    @@: mov	eax, [esi]
	stosd
	add	esi, edx
	dec	ecx
	jnz	@b

	mov	eax, [scanline_pixels_new]
	mov	edi, [ebx + Image.Data]
	lea	esi, [edi + 4]
	mov	edx, [scanline_len_old]
	shr	edx, 2
    @@: mov	ecx, edx
	rep	movsd
	add	esi, 4
	dec	eax
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, [scanline_pixels_new]
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	rep	movsd

	pop	ecx
	jmp	.next_column_ccw_low

.rotate_ccw8:
  .next_column_ccw_low8:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -1

	mov	ecx, [scanline_pixels_new]
	mov	esi, [ebx + Image.Data]
	mov	edi, [line_buffer]
    @@: mov	al, [esi]
	mov	[edi], al
	add	esi, edx
	add	edi, 1
	sub	ecx, 1
	jnz	@b

	mov	eax, [scanline_pixels_new]
	mov	edi, [ebx + Image.Data]
	lea	esi, [edi + 1]
	mov	edx, [scanline_len_old]
    @@: mov	ecx, edx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	add	esi, 1
	sub	eax, 1
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, [scanline_pixels_new]
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	mov	edx, ecx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb

	pop	ecx
	jmp	.next_column_ccw_low8

.rotate_ccw24:
  .next_column_ccw_low24:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -3

	mov	ecx, [scanline_pixels_new]
	mov	esi, [ebx + Image.Data]
	mov	edi, [line_buffer]
    @@: mov	al, [esi]
	mov	[edi], al
	mov	al, [esi+1]
	mov	[edi+1], al
	mov	al, [esi+2]
	mov	[edi+2], al
	add	esi, edx
	add	edi, 3
	sub	ecx, 1
	jnz	@b

	mov	eax, [scanline_pixels_new]
	mov	edi, [ebx + Image.Data]
	lea	esi, [edi + 3]
	mov	edx, [scanline_len_old]
    @@: mov	ecx, edx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	add	esi, 3
	sub	eax, 1
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, eax
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	shr	ecx, 2
	rep	movsd
	mov	ecx, eax
	and	ecx, 3
	rep	movsb

	pop	ecx
	jmp	.next_column_ccw_low24

  .rotate_cw_low:
	mov	eax, [ebx + Image.Height]
	mov	[scanline_pixels_new], eax
	call	img._.get_scanline_len
	mov	[scanline_len_new], eax

	invoke	mem.alloc, eax
	or	eax, eax
	jz	.error
	mov	[line_buffer], eax

	mov	eax, [ebx + Image.Width]
	mov	ecx, eax
	call	img._.get_scanline_len
	mov	[scanline_len_old], eax

	mov	eax, [scanline_len_new]
	imul	eax, ecx
	add	eax, [ebx + Image.Data]
	mov	[pixels_ptr], eax

	cmp	[ebx + Image.Type], Image.bpp8
	jz	.rotate_cw8
	cmp	[ebx + Image.Type], Image.bpp24
	jz	.rotate_cw24

  .next_column_cw_low:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -4

	mov	ecx, [scanline_pixels_new]
	mov	esi, [pixels_ptr]
	add	esi, -4
	mov	edi, [line_buffer]
    @@: mov	eax, [esi]
	stosd
	sub	esi, edx
	dec	ecx
	jnz	@b

	mov	eax, [scanline_pixels_new]
	dec	eax
	mov	edi, [ebx + Image.Data]
	add	edi, [scanline_len_old]
	lea	esi, [edi + 4]
	mov	edx, [scanline_len_old]
	shr	edx, 2
    @@: mov	ecx, edx
	rep	movsd
	add	esi, 4
	dec	eax
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, [scanline_pixels_new]
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	rep	movsd

	pop	ecx
	jmp	.next_column_cw_low

.rotate_cw8:
  .next_column_cw_low8:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -1

	mov	ecx, [scanline_pixels_new]
	mov	esi, [pixels_ptr]
	add	esi, -1
	mov	edi, [line_buffer]
    @@: mov	al, [esi]
	mov	[edi], al
	sub	esi, edx
	add	edi, 1
	sub	ecx, 1
	jnz	@b

	mov	eax, [scanline_pixels_new]
	dec	eax
	mov	edi, [ebx + Image.Data]
	add	edi, [scanline_len_old]
	lea	esi, [edi + 1]
	mov	edx, [scanline_len_old]
    @@: mov	ecx, edx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	add	esi, 1
	sub	eax, 1
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, eax
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	shr	ecx, 2
	rep	movsd
	mov	ecx, eax
	and	ecx, 3
	rep	movsb

	pop	ecx
	jmp	.next_column_cw_low8

.rotate_cw24:
  .next_column_cw_low24:
	dec	ecx
	js	.exchange_dims
	push	ecx

	mov	edx, [scanline_len_old]
	add	[scanline_len_old], -3

	mov	ecx, [scanline_pixels_new]
	mov	esi, [pixels_ptr]
	add	esi, -3
	mov	edi, [line_buffer]
    @@: mov	al, [esi]
	mov	[edi], al
	mov	al, [esi+1]
	mov	[edi+1], al
	mov	al, [esi+2]
	mov	[edi+2], al
	sub	esi, edx
	add	edi, 3
	sub	ecx, 1
	jnz	@b

	mov	eax, [scanline_pixels_new]
	dec	eax
	mov	edi, [ebx + Image.Data]
	add	edi, [scanline_len_old]
	lea	esi, [edi + 3]
	mov	edx, [scanline_len_old]
    @@: mov	ecx, edx
	shr	ecx, 2
	rep	movsd
	mov	ecx, edx
	and	ecx, 3
	rep	movsb
	add	esi, 3
	sub	eax, 1
	jnz	@b

	mov	eax, [scanline_len_new]
	sub	[pixels_ptr], eax
	mov	ecx, eax
	mov	esi, [line_buffer]
	mov	edi, [pixels_ptr]
	shr	ecx, 2
	rep	movsd
	mov	ecx, eax
	and	ecx, 3
	rep	movsb

	pop	ecx
	jmp	.next_column_cw_low24

  .flip:
	jmp	.exit

  .exchange_dims:
	push	[ebx + Image.Width] [ebx + Image.Height]
	pop	[ebx + Image.Width] [ebx + Image.Height]

  .exit:
	invoke	mem.free, [line_buffer]
	xor	eax, eax
	inc	eax
	pop	edi esi ebx
	ret

  .error:
	invoke	mem.free, [line_buffer]
	xor	eax, eax
	pop	edi esi ebx
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;;================================================================================================;;
proc img._.validate, _img ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img._.new ;//////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to image                                                                     ;;
;;================================================================================================;;
	invoke	mem.alloc, sizeof.Image
	test	eax, eax
	jz	@f
	push	ecx
	xor	ecx, ecx
	mov	[eax + Image.Data], ecx
	mov	[eax + Image.Type], ecx
	mov	[eax + Image.Extended], ecx
	mov	[eax + Image.Previous], ecx
	mov	[eax + Image.Next], ecx
	pop	ecx
@@:
	ret
endp

;;================================================================================================;;
proc img._.delete _img ;//////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	push	edx
	mov	edx, [_img]
	cmp	[edx + Image.Data], 0
	je	@f
	invoke	mem.free, [edx + Image.Data]
    @@: cmp	[edx + Image.Extended], 0
	je	@f
	invoke	mem.free, [edx + Image.Extended]
    @@: invoke	mem.free, edx
	pop	edx
	ret
endp

;;================================================================================================;;
proc img._.resize_data _img, _width, _height ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ebx esi
	mov	ebx, [_img]
	mov	eax, [_height]
; our memory is limited, [_width]*[_height] must not overflow
; image with width or height greater than 65535 is most likely bogus
	cmp	word [_width+2], 0
	jnz	.error
	cmp	word [_height+2], 0
	jnz	.error
	imul	eax, [_width]
	test	eax, eax
	jz	.error
; do not allow images which require too many memory
	cmp	eax, 4000000h
	jae	.error
	cmp	[ebx + Image.Type], Image.bpp8
	jz	.bpp8
	cmp	[ebx + Image.Type], Image.bpp24
	jz	.bpp24
.bpp32:
	shl	eax, 2
	jmp	@f
.bpp24:
	lea	eax, [eax*3]
	jmp	@f
.bpp8:
	add	eax, 256*4	; for palette
@@:
	mov	esi, eax
	invoke	mem.realloc, [ebx + Image.Data], eax
	or	eax, eax
	jz	.error

	mov	[ebx + Image.Data], eax
	push	[_width]
	pop	[ebx + Image.Width]
	push	[_height]
	pop	[ebx + Image.Height]
	cmp	[ebx + Image.Type], Image.bpp8
	jnz	.ret
	lea	esi, [eax + esi - 256*4]
	mov	[ebx + Image.Palette], esi
	jmp	.ret

  .error:
	xor	eax, eax
  .ret:
	pop	esi ebx
	ret
endp

;;================================================================================================;;
img._.get_scanline_len: ;/////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	cmp	[ebx + Image.Type], Image.bpp8
	jz	.bpp8.1
	cmp	[ebx + Image.Type], Image.bpp24
	jz	.bpp24.1
	shl	eax, 2
	jmp	@f
.bpp24.1:
	lea	eax, [eax*3]
.bpp8.1:
@@:
	ret


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


img._.formats_table:
  .bmp dd img.is.bmp, img.decode.bmp, img.encode.bmp
; .ico dd img.is.ico, img.decode.ico, img.encode.ico
; .cur dd img.is.cur, img.decode.cur, img.encode.cur
  .gif dd img.is.gif, img.decode.gif, img.encode.gif
  .png dd img.is.png, img.decode.png, img.encode.png
  .jpg dd img.is.jpg, img.decode.jpg, img.encode.jpg
       dd 0


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Exported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 4
@EXPORT:

export					      \
	lib_init	, 'lib_init'	    , \
	0x00010002	, 'version'	    , \
	img.is_img	, 'img.is_img'	    , \
	img.info	, 'img.info'	    , \
	img.from_file	, 'img.from_file'   , \
	img.to_file	, 'img.to_file'     , \
	img.from_rgb	, 'img.from_rgb'    , \
	img.to_rgb	, 'img.to_rgb'	    , \
	img.to_rgb2     , 'img.to_rgb2'     , \
	img.decode	, 'img.decode'	    , \
	img.encode	, 'img.encode'	    , \
	img.create	, 'img.create'	    , \
	img.destroy	, 'img.destroy'     , \
	img.count	, 'img.count'	    , \
	img.lock_bits	, 'img.lock_bits'   , \
	img.unlock_bits , 'img.unlock_bits' , \
	img.flip	, 'img.flip'	    , \
	img.rotate	, 'img.rotate'

; import from deflate unpacker
; is initialized only when PNG loading is requested
align 4
@IMPORT:

library kfar_arc, '../File Managers/kfar_arc.obj'
import	kfar_arc, \
	deflate_unpack2, 'deflate_unpack2'

; mutex for unpacker loading
deflate_loader_mutex	dd	0

section '.data' data readable writable align 16
; uninitialized data - global constant tables

; data for YCbCr -> RGB translation
color_table_1		rd	256
color_table_2		rd	256
color_table_3		rd	256
color_table_4		rd	256
