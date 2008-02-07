;;================================================================================================;;
;;//// bmp.asm //// (c) mike.dld, 2007-2008 //////////////////////////////////////////////////////;;
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
;;                                                                                                ;;
;; References:                                                                                    ;;
;;   1. "Microsoft Windows Bitmap File Format Summary"                                            ;;
;;      from "Encyclopedia of Graphics File Formats" by O'Reilly                                  ;;
;;      http://www.fileformat.info/format/bmp/                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;


include 'bmp.inc'

;;================================================================================================;;
proc img.is.bmp _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in BMP format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	cmp	[_length], 2
	jb	.nope
	mov	eax, [_data]
	cmp	word[eax], 'BM'
	je	.yep

  .nope:
	xor	eax, eax
	ret

  .yep:
	xor	eax,eax
	inc	eax
	ret
endp

;;================================================================================================;;
proc img.decode.bmp _data, _length ;//////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in BMP format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
  img dd ?
endl

	push	ebx

	stdcall img.is.bmp, [_data], [_length]
	or	eax, eax
	jz	.error

	mov	ebx, [_data]
;       cmp     [ebx + bmp.Header.info.Compression], bmp.BI_RGB
;       je      @f
;       mov     eax, [ebx + bmp.Header.file.Size]
;       cmp     eax, [_length]
;       jne     .error

    @@: stdcall img.create, [ebx + bmp.Header.info.Width], [ebx + bmp.Header.info.Height]
	or	eax, eax
	jz	.error
	mov	[img], eax
	mov	edx, eax

	invoke	mem.alloc, sizeof.bmp.Image
	or	eax, eax
	jz	.error
	mov	[edx + Image.Extended], eax
	mov	esi, ebx
	add	esi, sizeof.bmp.FileHeader
	mov	edi, eax
	mov	ecx, sizeof.bmp.InfoHeader
	rep	movsb

	mov	eax, [ebx + bmp.Header.info.Compression]
	cmp	eax, bmp.BI_RGB
	jne	@f
	stdcall ._.rgb
	jmp	.decoded
    @@: cmp	eax, bmp.BI_RLE8
	jne	@f
	stdcall ._.rle
	jmp	.decoded
    @@: cmp	eax, bmp.BI_RLE4
	jne	@f
	stdcall ._.rle
	jmp	.decoded
    @@: cmp	eax, bmp.BI_BITFIELDS
	jne	@f
	stdcall ._.bitfields
	jmp	.decoded
    @@: cmp	eax, bmp.BI_JPEG
	jne	@f
	stdcall ._.jpeg
	jmp	.decoded
    @@: cmp	eax, bmp.BI_PNG
	jne	.error
	stdcall ._.png

  .decoded:
	or	eax, eax
	jz	@f
	stdcall img.destroy, [img]
	jmp	.error
	
    @@: stdcall img.flip, [img], FLIP_VERTICAL
	mov	eax, [img]
	ret

  .error:
	xor	eax, eax
	pop	ebx
	ret
endp

;;================================================================================================;;
proc img.encode.bmp _img, _p_length ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in BMP format                                                       ;;
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


;;================================================================================================;;
proc img.decode.bmp._.rgb ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [edx + Image.Extended]
	mov	[ecx + bmp.Image.info.AlphaMask], 0
	mov	edi, [edx + Image.Data]

	movzx	eax, [ebx + bmp.Header.info.BitCount]
	cmp	eax, 32
	je	.32bpp
	cmp	eax, 24
	je	.24bpp
	cmp	eax, 16
	je	.16bpp
	cmp	eax, 8
	je	.8bpp
	cmp	eax, 4
	je	.4bpp
	cmp	eax, 1
	je	.1bpp
	jmp	.error

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.32bpp:
	mov	[ecx + bmp.Image.info.RedMask],   00000000111111110000000000000000b ; 8-0-0
	mov	[ecx + bmp.Image.info.GreenMask], 00000000000000001111111100000000b ; 0-8-0
	mov	[ecx + bmp.Image.info.BlueMask],  00000000000000000000000011111111b ; 0-0-8
	stdcall img.decode.bmp._.bitfields
	ret

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.24bpp:
	mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]
	mov	ecx, [ebx + bmp.Header.info.Height]

  .next_line:
	push	ecx
	mov	ecx, [ebx + bmp.Header.info.Width]
	xor	edx, edx

  .next_line_pixel:
	movsd
	dec	esi
	inc	edx
	dec	ecx
	jnz	.next_line_pixel

	and	edx, 0x03
	add	esi, edx
	pop	ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.16bpp:
	mov	[ecx + bmp.Image.info.RedMask],   00000000000000000111110000000000b ; 5-0-0
	mov	[ecx + bmp.Image.info.GreenMask], 00000000000000000000001111100000b ; 0-5-0
	mov	[ecx + bmp.Image.info.BlueMask],  00000000000000000000000000011111b ; 0-0-5
	stdcall img.decode.bmp._.bitfields
	ret

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.8bpp:
	mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]
	mov	ecx, [ebx + bmp.Header.info.Height]

  .next_line:
	push	ecx
	mov	ecx, [ebx + bmp.Header.info.Width]

  .next_line_dword:
	lodsb
	and	eax, 0x000000FF
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	dec	ecx
	jnz	.next_line_dword

	pop	ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.4bpp:
	mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]
	mov	ecx, [ebx + bmp.Header.info.Height]

  .next_line:
	push	ecx
	mov	ecx, [ebx + bmp.Header.info.Width]

  .next_line_dword:
	push	ecx
	lodsd
	bswap	eax
	xchg	edx, eax
	mov	ecx, 32 / 4

  .next_pixel:
	rol	edx, 4
	mov	al, dl
	and	eax, 0x0000000F
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	dec	dword[esp]
	jz	@f
	dec	ecx
	jnz	.next_pixel

    @@: pop	ecx
	or	ecx, ecx
	jnz	.next_line_dword

	pop	ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.1bpp:
	mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]
	mov	ecx, [ebx + bmp.Header.info.Height]

  .next_line:
	push	ecx
	mov	ecx, [ebx + bmp.Header.info.Width]

  .next_line_dword:
	push	ecx
	lodsd
	bswap	eax
	xchg	edx, eax
	mov	ecx, 32 / 1

  .next_pixel:
	rol	edx, 1
	mov	al, dl
	and	eax, 0x00000001
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	dec	dword[esp]
	jz	@f
	dec	ecx
	jnz	.next_pixel

    @@: pop	ecx
	or	ecx, ecx
	jnz	.next_line_dword

	pop	ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

  img.decode.bmp._.rgb.exit:
	xor	eax, eax
	ret

  img.decode.bmp._.rgb.error:
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc img.decode.bmp._.rle ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
locals
  scanline_len	dd ?
  marker_x	dd ?
  marker_y	dd ?
  abs_mode_addr dd ?
  enc_mode_addr dd ?
endl

	mov	edi, [edx + Image.Data]

	mov	[abs_mode_addr], .absolute_mode.rle8
	mov	[enc_mode_addr], .encoded_mode.rle8
	cmp	[ebx + bmp.Header.info.Compression], bmp.BI_RLE4
	jne	@f
	mov	[abs_mode_addr], .absolute_mode.rle4
	mov	[enc_mode_addr], .encoded_mode.rle4

    @@: mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]
	mov	eax, [edx + Image.Width]
	shl	eax, 2
	mov	[scanline_len], eax
	xor	eax, eax
	mov	[marker_x], eax
	mov	[marker_y], eax

  .next_run:
	xor	eax, eax
	lodsb
	or	al, al
	jz	.escape_mode
	jmp	[enc_mode_addr]

  .escape_mode:
	lodsb
	cmp	al, 0
	je	.end_of_scanline
	cmp	al, 1
	je	.exit
	cmp	al, 2
	je	.offset_marker
	jmp	[abs_mode_addr]

  .end_of_scanline: ; 0
	mov	eax, [marker_x]
	shl	eax, 2
	neg	eax
	add	eax, [scanline_len]
	add	edi, eax
	mov	[marker_x], 0
	inc	[marker_y]
	jmp	.next_run

  .offset_marker: ; 2: dx, dy
	lodsb
	mov	edx, [marker_x]
	add	edx, eax
	cmp	edx, [ebx + bmp.Header.info.Width]
	jae	.exit
	mov	[marker_x], edx
	shl	eax, 2
	add	edi, eax
	lodsb
	and	eax, 0x0FF
	mov	edx, [marker_y]
	add	edx, eax
	cmp	edx, [ebx + bmp.Header.info.Height]
	jae	.exit
	mov	[marker_y], edx
	imul	eax, [scanline_len]
	add	edi, eax
	jmp	.next_run

  .encoded_mode.rle8: ; N: b1 * N
	mov	edx, eax
	lodsb
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
    @@: dec	edx
	js	.fix_marker
	stosd
	inc	[marker_x]
	jmp	@b

  .absolute_mode.rle8: ; N: b1 .. bN
	mov	edx, eax
	push	eax
    @@: dec	edx
	js	@f
	lodsb
	and	eax, 0x0FF
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	inc	[marker_x]
	jmp	@b
    @@: pop	eax
	test	eax, 1
	jz	.fix_marker
	inc	esi
	jmp	.fix_marker

  .encoded_mode.rle4: ; N: b1 * N
	mov	edx, eax
	lodsb
	mov	ecx, eax
	shr	ecx, 4
	mov	ecx, [ebx + ecx * 4 + bmp.Header.info.Palette]
	and	eax, 0x00F
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
    @@: dec	edx
	js	.fix_marker
	test	edx, 1
	jz	.odd
	mov	[edi], ecx
	add	edi, 4
	inc	[marker_x]
	jmp	@b
    .odd:
	stosd
	inc	[marker_x]
	jmp	@b

  .absolute_mode.rle4: ; N: b1 .. bN
	mov	edx, eax
	push	eax
    @@: dec	edx
	js	@f
	lodsb
	and	eax, 0x0FF
	mov	ecx, eax
	shr	eax, 4
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	inc	[marker_x]
	dec	edx
	js	@f
	mov	eax, ecx
	and	eax, 0x00F
	mov	eax, [ebx + eax * 4 + bmp.Header.info.Palette]
	stosd
	inc	[marker_x]
	jmp	@b
    @@: pop	eax
	and	eax, 0x03
	jz	.fix_marker
	cmp	eax, 3
	je	.fix_marker
	inc	esi
	jmp	.fix_marker

  .fix_marker:
	mov	eax, [marker_x]
    @@: sub	eax, [ebx + bmp.Header.info.Width]
	jle	.next_run
	mov	[marker_x], eax
	inc	[marker_y]
	jmp	@b

  .exit:
	xor	eax, eax
	ret

  .error:
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc img.decode.bmp._.bitfields ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
locals
  shift   bmp.RgbByteQuad
  unshift bmp.RgbByteQuad
  mask	  bmp.RgbQuad
  delta   dd ?
endl

	push	esi edi
	mov	esi, [edx + Image.Extended]

	mov	ecx, [esi + bmp.Image.info.RedMask]
	call	.calc_shift
	mov	[shift.Red], al
	mov	[mask.Red], ecx
	call	.calc_unshift
	mov	[unshift.Red], al
	mov	ecx, [esi + bmp.Image.info.GreenMask]
	call	.calc_shift
	mov	[shift.Green], al
	mov	[unshift.Green], al
	mov	[mask.Green], ecx
	call	.calc_unshift
	mov	[unshift.Green], al
	mov	ecx, [esi + bmp.Image.info.BlueMask]
	call	.calc_shift
	mov	[shift.Blue], al
	mov	[unshift.Blue], al
	mov	[mask.Blue], ecx
	call	.calc_unshift
	mov	[unshift.Blue], al
	mov	ecx, [esi + bmp.Image.info.AlphaMask]
	call	.calc_shift
	mov	[shift.Alpha], al
	mov	[unshift.Alpha], al
	mov	[mask.Alpha], ecx
	call	.calc_unshift
	mov	[unshift.Alpha], al

	mov	edi, [edx + Image.Data]
	mov	esi, ebx
	add	esi, [ebx + bmp.Header.file.OffBits]

	mov	[delta], 4
	movzx	eax, [ebx + bmp.Header.info.BitCount]
	cmp	eax, 32
	je	@f
	cmp	eax, 16
	jne	.error
	mov	[delta], 2

;;------------------------------------------------------------------------------------------------;;

    @@: mov	ecx, [edx + Image.Height]

  .next_line:
	push	ecx
	mov	ecx, [edx + Image.Width]
	
  .next_pixel:
	push	ecx

	mov	eax, [esi]
	mov	cl, [shift.Blue]
	shr	eax, cl
	and	eax, [mask.Blue]
	mov	cl, [unshift.Blue]
	shl	eax, cl
	stosb

	mov	eax, [esi]
	mov	cl, [shift.Green]
	shr	eax, cl
	and	eax, [mask.Green]
	mov	cl, [unshift.Green]
	shl	eax, cl
	stosb

	mov	eax, [esi]
	mov	cl, [shift.Red]
	shr	eax, cl
	and	eax, [mask.Red]
	mov	cl, [unshift.Red]
	shl	eax, cl
	stosb

	mov	eax, [esi]
	mov	cl, [shift.Alpha]
	shr	eax, cl
	and	eax, [mask.Alpha]
	mov	cl, [unshift.Alpha]
	shl	eax, cl
	stosb

	add	esi, [delta]

	pop	ecx
	dec	ecx
	jnz	.next_pixel

	pop	ecx
	dec	ecx
	jnz	.next_line

;;------------------------------------------------------------------------------------------------;;

  .exit:
	xor	eax, eax
	pop	edi esi
	ret

  .error:
	or	eax, -1
	pop	edi esi
	ret
	
.calc_shift:
	xor	eax, eax
	or	ecx, ecx
	jnz	@f
	retn
    @@: test	ecx, 1
	jnz	@f
   .zz: shr	ecx, 1
	inc	eax
	jmp	@b
    @@: test	ecx, 0100000000b
	jnz	.zz
	retn
.calc_unshift:
	xor	eax, eax
	or	ecx, ecx
	jnz	@f
	retn
    @@: test	ecx, 1
	jz	@f
	shr	ecx, 1
	inc	eax
	jmp	@b
    @@: sub	eax, 8
	neg	eax
	retn
endp

;;================================================================================================;;
proc img.decode.bmp._.jpeg ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc img.decode.bmp._.png ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;
