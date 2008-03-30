;;================================================================================================;;
;;//// libimg.asm //// (c) mike.dld, 2007-2008 ///////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; General Public License as published by the Free Software Foundation, either version 3 of the   ;;
;; License, or (at your option) any later version.                                                ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; General Public License for more details.                                                       ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU General Public License along with Libs-Dev. If not, ;;
;; see <http://www.gnu.org/licenses/>.                                                            ;;
;;                                                                                                ;;
;;================================================================================================;;


format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../struct.inc'
include '../../../../proc32.inc'
include '../../../../macros.inc'
purge section,mov;add,sub

include 'libimg.inc'

section '.flat' code readable align 16

include 'bmp/bmp.asm'
include 'gif/gif.asm'

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
proc img.to_rgb _img ;////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to rgb_data (array of [rgb] triplets)                                        ;;
;;================================================================================================;;
	push	esi edi
	stdcall img._.validate, [_img]
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
	mov	esi, [esi + Image.Data]

    @@: dec	ecx
	js	@f
	movsd
	dec	edi
	jmp	@b

    @@: pop	eax
	pop	edi esi
	ret

  .error:
	xor	eax, eax
	pop	edi esi
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
	jnz	@f
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
proc img.create _width, _height ;/////////////////////////////////////////////////////////////////;;
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

	push	eax

	stdcall img._.resize_data, eax, [_width], [_height]
	or	eax, eax
	jz	.error.2

	pop	eax
	ret

  .error.2:
;       pop     eax
	stdcall img._.delete; eax
	xor	eax, eax

  .error:
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

	push	esi edi
	stdcall img._.validate, [_img]
	or	eax, eax
	jnz	.error

	mov	esi, [_img]
	mov	ecx, [esi + Image.Height]
	mov	eax, [esi + Image.Width]
	shl	eax, 2
	mov	[scanline_len], eax

	push	esi

	test	[_flip_kind], FLIP_VERTICAL
	jz	.dont_flip_vert

	imul	eax, ecx
	sub	eax, [scanline_len]
	shr	ecx, 1
	mov	esi, [esi + Image.Data]
	lea	edi, [esi + eax]
	
  .next_line_vert:
	push	ecx

	mov	ecx, [scanline_len]
	shr	ecx, 2
    @@: lodsd
	xchg	eax, [edi]
	mov	[esi - 4], eax
	add	edi, 4
	dec	ecx
	jnz	@b

	pop	ecx
	mov	eax, [scanline_len]
	shl	eax, 1
	sub	edi, eax
	dec	ecx
	jnz	.next_line_vert

  .dont_flip_vert:

	pop	esi

	test	[_flip_kind], FLIP_HORIZONTAL
	jz	.exit

	mov	ecx, [esi + Image.Height]
	mov	esi, [esi + Image.Data]
	lea	edi, [esi - 4]
	add	edi, [scanline_len]

  .next_line_horz:
	push	ecx esi edi

	mov	ecx, [scanline_len]
	shr	ecx, 3
    @@: mov	eax, [esi]
	xchg	eax, [edi]
	mov	[esi], eax
	add	esi, 4
	add	edi, -4
	dec	ecx
	jnz	@b

	pop	edi esi ecx
	add	esi, [scanline_len]
	add	edi, [scanline_len]
	dec	ecx
	jnz	.next_line_horz

  .exit:
	xor	eax, eax
	inc	eax
	pop	edi esi
	ret

  .error:
	xor	eax, eax
	pop	edi esi
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
	stdcall img._.validate, [_img]
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
	mov	ebx, [_img]
	mov	eax, [ebx + Image.Height]
	mov	[scanline_pixels_new], eax
	shl	eax, 2
	mov	[scanline_len_new], eax

	invoke	mem.alloc, eax
	or	eax, eax
	jz	.error
	mov	[line_buffer], eax

	mov	ecx, [ebx + Image.Width]
	lea	eax, [ecx * 4]
	mov	[scanline_len_old], eax

	mov	eax, [scanline_len_new]
	imul	eax, ecx
	add	eax, [ebx + Image.Data]
	mov	[pixels_ptr], eax

  .next_column_ccw_low:
	dec	ecx
	jz	.exchange_dims
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

  .rotate_cw_low:
	mov	ebx, [_img]
	mov	eax, [ebx + Image.Height]
	mov	[scanline_pixels_new], eax
	shl	eax, 2
	mov	[scanline_len_new], eax

	invoke	mem.alloc, eax
	or	eax, eax
	jz	.error
	mov	[line_buffer], eax

	mov	ecx, [ebx + Image.Width]
	lea	eax, [ecx * 4]
	mov	[scanline_len_old], eax

	mov	eax, [scanline_len_new]
	imul	eax, ecx
	add	eax, [ebx + Image.Data]
	mov	[pixels_ptr], eax

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
	push	ebx
	mov	ebx, [_img]
	mov	eax, [_height]
	imul	eax, [_width]
	shl	eax, 2
	invoke	mem.realloc, [ebx + Image.Data], eax
	or	eax, eax
	jz	.error

	mov	[ebx + Image.Data], eax
	push	[_width]
	pop	[ebx + Image.Width]
	push	[_height]
	pop	[ebx + Image.Height]

  .error:
	pop	ebx
	ret
endp


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
; .png dd img.is.png, img.decode.png, img.encode.png
; .jpg dd img.is.jpg, img.decode.jpg, img.encode.jpg
       dd 0


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Exported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@EXPORT:

export					      \
	lib_init	, 'lib_init'	    , \
	0x00010001	, 'version'	    , \
	img.is_img	, 'img.is_img'	    , \
	img.info	, 'img.info'	    , \
	img.from_file	, 'img.from_file'   , \
	img.to_file	, 'img.to_file'     , \
	img.from_rgb	, 'img.from_rgb'    , \
	img.to_rgb	, 'img.to_rgb'	    , \
	img.decode	, 'img.decode'	    , \
	img.encode	, 'img.encode'	    , \
	img.create	, 'img.create'	    , \
	img.destroy	, 'img.destroy'     , \
	img.count	, 'img.count'	    , \
	img.lock_bits	, 'img.lock_bits'   , \
	img.unlock_bits , 'img.unlock_bits' , \
	img.flip	, 'img.flip'	    , \
	img.rotate	, 'img.rotate'
