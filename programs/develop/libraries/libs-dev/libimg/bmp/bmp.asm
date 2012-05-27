;;================================================================================================;;
;;//// bmp.asm //// (c) mike.dld, 2007-2008, (c) diamond, 2009 ///////////////////////////////////;;
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
;;   1. "Microsoft Windows Bitmap File Format Summary"                                            ;;
;;      from "Encyclopedia of Graphics File Formats" by O'Reilly                                  ;;
;;      http://www.fileformat.info/format/bmp/                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;


include 'bmp.inc'

;;================================================================================================;;
;;proc img.is.bmp _data, _length ;////////////////////////////////////////////////////////////////;;
img.is.bmp:
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in BMP format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
; test 1 (length of data): data must contain FileHeader and required fields from InfoHeader
	cmp	dword [esp+8], sizeof.bmp.FileHeader + 12
	jb	.nope
; test 2: signature
	mov	eax, [esp+4]
	cmp	word [eax], 'BM'
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
proc img.decode.bmp _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in BMP format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
  length_rest dd ?
  img dd ?
  bTopDown db ?
  bIsIco db ?
endl
img.decode.bmp.length_rest equ length_rest
	mov	[bIsIco], 0
.common: ; common place for BMP and ICO

	push	ebx esi edi

	mov	ebx, [_data]
	cmp	[bIsIco], 0
	jnz	@f
	add	ebx, sizeof.bmp.FileHeader
	sub	[_length], sizeof.bmp.FileHeader
   @@:

	mov	eax, [ebx + bmp.InfoHeader.Size]
; sanity check: file length must be greater than size of headers
	cmp	[_length], eax
	jbe	.error

	mov	[bTopDown], 0

	cmp	eax, 12		; 0x0C
	jz	.old1
	cmp	eax, 40		; 0x28
	jz	.normal
	cmp	eax, 56		; 0x38
	je	.normal
	cmp	eax, 0x6C
	je	.normal
	cmp	eax, 0x7C
	jnz	.error
; convert images with <= 8 bpp to 8bpp, other - to 32 bpp
.normal:
	m2m	eax, Image.bpp8
	cmp	byte [ebx + 14], 8	; bit count
	jbe	@f
	mov	al, Image.bpp32
@@:
	push	eax
	mov	eax, [ebx + 8]	;[ebx + bmp.InfoHeader.Height]
	test	eax, eax
	jns	@f
	inc	[bTopDown]
	neg	eax
@@:
	cmp	[bIsIco], 0	; for icons Height is two times larger than image height
	jz	@f
	shr	eax, 1
@@:
	pushd	eax
	pushd	[ebx + 4]	;[ebx + bmp.InfoHeader.Width]
	jmp	.create
.old1:
	m2m	eax, Image.bpp8
	cmp	byte [ebx + 10], 8	; bit count
	jbe	@f
	mov	al, Image.bpp32
@@:
	push	eax
	movsx	eax, word [ebx + 6]	;[ebx + bmp.InfoHeader.OldHeight]
	test	eax, eax
	jns	@f
	inc	[bTopDown]
	neg	eax
@@:
	cmp	[bIsIco], 0	; for icons Height is two times larger than image height
	jz	@f
	shr	eax, 1
@@:
	push	eax
	movzx	eax, word [ebx + 4]	;[ebx + bmp.InfoHeader.OldWidth]
	push	eax
.create:
	call	img.create

	or	eax, eax
	jz	.error
	mov	[img], eax
	mov	edx, eax

	invoke	mem.alloc, sizeof.bmp.Image
	or	eax, eax
	jz	.error.free
	mov	[edx + Image.Extended], eax
	push	eax
	mov	edi, eax
	mov	ecx, sizeof.bmp.Image/4
	xor	eax, eax
	rep	stosd
	pop	edi
	push	edi
	mov	esi, ebx
	mov	ecx, [ebx]	;[ebx + bmp.InfoHeader.Size]
	cmp	ecx, 12
	jz	.old2
	rep	movsb
	jmp	.decode
.old2:
	movsd	; Size
	movzx	eax, word [esi]	; OldWidth -> Width
	stosd
	movsx	eax, word [esi+2]	; OldHeight -> Height
	stosd
	lodsd	; skip OldWidth+OldHeight
	movsd	; Planes+BitCount
.decode:

	pop	edi
	cmp	[bIsIco], 0
	jnz	@f
	mov	edi, [_length]
	add	edi, sizeof.bmp.FileHeader
	mov	esi, [ebx - sizeof.bmp.FileHeader + bmp.FileHeader.OffBits]
	jmp	.offset_calculated
@@:
	xor	esi, esi
	mov	cl, byte [edi + bmp.Image.info.BitCount]
	cmp	cl, 8
	ja	@f
	inc	esi
	add	cl, 2
	shl	esi, cl
@@:
	add	esi, [edi + bmp.Image.info.Size]
	mov	edi, [_length]
.offset_calculated:
	sub	edi, esi
	jbe	.error.free
	add	esi, [_data]

	mov	eax, [edx + Image.Extended]
	mov	eax, [eax + bmp.Image.info.Compression]
	cmp	eax, bmp.BI_RGB
	jne	@f
	stdcall ._.rgb
	jmp	.decoded
    @@: cmp	eax, bmp.BI_RLE8
	jne	@f
	cmp	word [ebx + 14], 8 ;bmp.InfoHeader.BitCount
	jnz	.error.free
	stdcall ._.rle
	jmp	.decoded
    @@: cmp	eax, bmp.BI_RLE4
	jne	@f
	cmp	word [ebx + 14], 4
	jnz	.error.free
	stdcall ._.rle
	jmp	.decoded
    @@: cmp	eax, bmp.BI_BITFIELDS
	jne	.error.free
	stdcall ._.bitfields
	jmp	.decoded
; BI_JPEG and BI_PNG constants are not valid values for BMP file,
; they are intended for WinAPI
;    @@: cmp	eax, bmp.BI_JPEG
;	jne	@f
;	stdcall ._.jpeg
;	jmp	.decoded
;    @@: cmp	eax, bmp.BI_PNG
;	jne	.error
;	stdcall ._.png

  .decoded:
	or	eax, eax
	jz	@f
  .error.free:
	stdcall img.destroy, [img]
	jmp	.error

    @@:
	cmp	[bTopDown], 0
	jnz	@f
	stdcall img.flip, [img], FLIP_VERTICAL
    @@:
	mov	eax, [img]
	mov	ecx, [length_rest]	; return length for ICO code
	cmp	[bIsIco], 0
	jz	@f
	mov	[esp + 4], esi	; return pointer to end-of-data for ICO code
    @@:
	pop	edi esi ebx
	ret

  .error:
	xor	eax, eax
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc img.encode.bmp _img, _common, _specific ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in BMP format                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> [_img]      = pointer to image                                                                 ;;
;> [_common]   = format independent options                                                       ;;
;> [_specific] = 0 / pointer to the structure of format specific options                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                                              ;;
;< ecx = error code / the size of encoded data                                                    ;;
;;================================================================================================;;
locals
	bytes_per_scanline	rd 1
	encoded_file		rd 1
	encoded_file_size	rd 1
	encoded_data_size	rd 1
endl
	mov	ebx, [_img]
	mov	eax, [ebx + Image.Type]
	cmp	eax, Image.bpp24
	je	.bpp24
	cmp	eax, Image.bpp32
	je	.bpp32
	mov	ecx, LIBIMG_ERROR_BIT_DEPTH
	jmp	.error

  .bpp24:
	mov	eax, [ebx + Image.Width]
	call	img._.get_scanline_len
	test	eax, 0x03
	jz	@f
	and	al, 0xfc
	add	eax, 4
    @@:
	mov	[bytes_per_scanline], eax
	imul	eax, [ebx + Image.Height]
	mov	[encoded_data_size], eax
	add	eax, 108 + 14
	mov	[encoded_file_size], eax
	stdcall	[mem.alloc], eax
	test	eax, eax
	jz	.error
	mov	[encoded_file], eax
	mov	edi, eax

	mov	word[edi], 'BM'
	add	edi, 2
	mov	eax, [encoded_file_size]
	stosd
	xor	eax, eax
	stosd
	mov	eax, 108 + 14
	stosd
	mov	eax, 108
	stosd
	mov	eax, [ebx + Image.Width]
	stosd
	mov	eax, [ebx + Image.Height]
	stosd
	mov	ax, 1	; Planes
	stosw
	mov	ax, 24	; BitCount
	stosw
	mov	eax, bmp.BI_RGB
	stosd
	mov	eax, [encoded_data_size]
	stosd
	mov	eax, 0x00000B13
	stosd
	stosd
	xor	eax, eax
	stosd
	stosd
	mov	eax, 'BGRs'
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	mov	eax, 2
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd

	mov	esi, [ebx + Image.Data]
	mov	ecx, [ebx + Image.Width]
	lea	ecx, [ecx*3]
	mov	eax, [ebx + Image.Height]
	mov	edx, [bytes_per_scanline]
	sub	edx, ecx
	mov	dh, cl
	and	dh, 3
	shr	ecx, 2
	push	ecx
	add	edi, [encoded_data_size]
	sub	edi, [bytes_per_scanline]
    @@:
	pop	ecx
	push	ecx
	rep	movsd
	mov	cl, dh
	rep	movsb
	mov	cl, dl
	add	edi, ecx
	sub	edi, [bytes_per_scanline]
	sub	edi, [bytes_per_scanline]
	dec	eax
	jnz	@b
	pop	ecx
	mov	eax, [encoded_file]
	mov	ecx, [encoded_file_size]
	jmp	.quit

  .bpp32:
	mov	eax, [ebx + Image.Width]
	call	img._.get_scanline_len
	mov	[bytes_per_scanline], eax
	imul	eax, [ebx + Image.Height]
	mov	[encoded_data_size], eax
	add	eax, 0x7C + 14
	mov	[encoded_file_size], eax
	stdcall	[mem.alloc], eax
	test	eax, eax
	jz	.error
	mov	[encoded_file], eax
	mov	edi, eax

	mov	word[edi], 'BM'
	add	edi, 2
	mov	eax, [encoded_file_size]
	stosd
	xor	eax, eax
	stosd
	mov	eax, 0x7C + 14
	stosd
	mov	eax, 0x7C
	stosd
	mov	eax, [ebx + Image.Width]
	stosd
	mov	eax, [ebx + Image.Height]
	stosd
	mov	ax, 1	; Planes
	stosw
	mov	ax, 32	; BitCount
	stosw
	mov	eax, 3	; WTF? bmp.BI_RGB
	stosd
	mov	eax, [encoded_data_size]
	stosd
	mov	eax, 0x00000B13
	stosd
	stosd
	xor	eax, eax
	stosd
	stosd
	mov	eax, 0xFF000000
	stosd
	shr	eax, 8
	stosd
	shr	eax, 8
	stosd
;	shr	eax, 8
	xor	eax, eax
	stosd
	mov	eax, 'BGRs'
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	stosd
	mov	eax, 2
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd

	mov	esi, [ebx + Image.Data]
	mov	ecx, [ebx + Image.Width]
	mov	eax, [ebx + Image.Height]
	add	edi, [encoded_data_size]
	sub	edi, [bytes_per_scanline]
	push	ecx
  .next_line:
	pop	ecx
	push	ecx
	push	eax
    @@:
	dec	ecx
	js	@f
	lodsd
	rol	eax, 8
	stosd
	jmp	@b
    @@:
	sub	edi, [bytes_per_scanline]
	sub	edi, [bytes_per_scanline]
	pop	eax
	dec	eax
	jnz	.next_line
	pop	ecx
	mov	eax, [encoded_file]
	mov	ecx, [encoded_file_size]
	jmp	.quit

  .error:
	xor	eax, eax
  .quit:
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

	movzx	eax, [ecx + bmp.Image.info.BitCount]
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
	mov	eax, [edx + Image.Width]
	lea	eax, [eax*3 + 3]
	and	eax, not 3
	mov	ecx, [edx + Image.Height]
	imul	eax, ecx
	sub	edi, eax
	jb	img.decode.bmp._.rgb.error
	mov	[img.decode.bmp.length_rest], edi
	mov	edi, [edx + Image.Data]

  .next_line:
	push	ecx edx
	mov	ecx, [edx + Image.Width]
	xor	edx, edx

  .next_line_pixel:
	movsd
	dec	esi
	inc	edx
	dec	ecx
	jnz	.next_line_pixel

	and	edx, 0x03
	add	esi, edx
	pop	edx ecx
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
	mov	eax, [edx + Image.Width]
	add	eax, 3
	call	img.decode.bmp._.rgb.prepare_palette
	jc	img.decode.bmp._.rgb.error

  .next_line:
	push	ecx
	mov	ecx, [edx + Image.Width]
	mov	eax, ecx
	neg	eax
	and	eax, 3
	rep	movsb
	add	esi, eax
	pop	ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.4bpp:
	mov	eax, [edx + Image.Width]
	add	eax, 7
	shr	eax, 1
	call	img.decode.bmp._.rgb.prepare_palette
	jc	img.decode.bmp._.rgb.error

  .next_line:
	push	ecx edx
	mov	ecx, [edx + Image.Width]

  .next_line_dword:
	push	ecx
	lodsd
	bswap	eax
	xchg	edx, eax
	mov	ecx, 32 / 4

  .next_pixel:
	rol	edx, 4
	mov	al, dl
	and	al, 0x0000000F
	stosb
	dec	dword[esp]
	jz	@f
	dec	ecx
	jnz	.next_pixel

    @@: pop	ecx
	or	ecx, ecx
	jnz	.next_line_dword

	pop	edx ecx
	dec	ecx
	jnz	.next_line

	jmp	img.decode.bmp._.rgb.exit

;;------------------------------------------------------------------------------------------------;;

img.decode.bmp._.rgb.1bpp:
	mov	eax, [edx + Image.Width]
	add	eax, 31
	shr	eax, 3
	call	img.decode.bmp._.rgb.prepare_palette
	jc	img.decode.bmp._.rgb.error

  .next_line:
	push	ecx edx
	mov	ecx, [edx + Image.Width]

  .next_line_dword:
	push	ecx
	lodsd
	bswap	eax
	xchg	edx, eax
	mov	ecx, 32 / 1

  .next_pixel:
	rol	edx, 1
	mov	al, dl
	and	al, 0x00000001
	stosb
	dec	dword[esp]
	jz	@f
	dec	ecx
	jnz	.next_pixel

    @@: pop	ecx
	or	ecx, ecx
	jnz	.next_line_dword

	pop	edx ecx
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

img.decode.bmp._.rgb.prepare_palette:
	and	eax, not 3
	mov	ecx, [edx + Image.Height]
	imul	eax, ecx
	sub	edi, eax
	jb	.ret
	mov	[img.decode.bmp.length_rest], edi
	push	esi
	sub	esi, ebx
	jc	.ret.pop
	sub	esi, [ebx + bmp.InfoHeader.Size]
	jc	.ret.pop
	mov	eax, esi
	mov	edi, [edx + Image.Palette]
	push	ecx
	mov	ecx, 256
	mov	esi, [ebx + bmp.InfoHeader.Size]
	cmp	esi, 12
	jz	.old
	shr	eax, 2
	add	esi, ebx
	cmp	ecx, eax
	jb	@f
	mov	ecx, eax
@@:
	rep	movsd
	jmp	.common
.old:
	add	esi, ebx
@@:
	movsd
	dec	esi
	sub	eax, 3
	jbe	@f
	sub	ecx, 1
	jnz	@b
@@:
.common:
	pop	ecx
	mov	edi, [edx + Image.Data]
	clc
.ret.pop:
	pop	esi
.ret:
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
  height	dd ?
endl

	mov	[abs_mode_addr], .absolute_mode.rle8
	mov	[enc_mode_addr], .encoded_mode.rle8
	cmp	[ebx + bmp.InfoHeader.Compression], bmp.BI_RLE4
	jne	@f
	mov	[abs_mode_addr], .absolute_mode.rle4
	mov	[enc_mode_addr], .encoded_mode.rle4
    @@:

	push	edi
	xor	eax, eax	; do not check file size in .prepare_palette
	push	ebp
	mov	ebp, [ebp]	; set parent stack frame
	call	img.decode.bmp._.rgb.prepare_palette
	pop	ebp
	pop	ecx	; ecx = rest bytes in file
	jc	.error

	mov	eax, [edx + Image.Width]
	mov	[scanline_len], eax
	mov	eax, [edx + Image.Height]
	mov	[height], eax
	xor	eax, eax
	mov	[marker_x], eax
	mov	[marker_y], eax
	mov	edi, [edx + Image.Data]

  .next_run:
	sub	ecx, 1
	jc	.eof
	xor	eax, eax
	lodsb
	or	al, al
	jz	.escape_mode
	jmp	[enc_mode_addr]

  .escape_mode:
	sub	ecx, 1
	jc	.eof
	lodsb
	cmp	al, 0
	je	.end_of_scanline
	cmp	al, 1
	je	.exit
	cmp	al, 2
	je	.offset_marker
	jmp	[abs_mode_addr]

  .end_of_scanline: ; 0
	sub	edi, [marker_x]
	add	edi, [scanline_len]
	mov	[marker_x], 0
	mov	eax, [marker_y]
	inc	eax
	mov	[marker_y], eax
	cmp	eax, [height]
	jb	.next_run
	jmp	.exit

  .offset_marker: ; 2: dx, dy
	sub	ecx, 2
	jc	.eof
	lodsb
	mov	edx, [marker_x]
	add	edx, eax
	cmp	edx, [scanline_len]
	jae	.exit
	mov	[marker_x], edx
	add	edi, eax
	lodsb
	mov	edx, [marker_y]
	add	edx, eax
	cmp	edx, [height]
	jae	.exit
	mov	[marker_y], edx
	imul	eax, [scanline_len]
	add	edi, eax
	jmp	.next_run

  .encoded_mode.rle8: ; N: b1 * N
	call	.fix_marker
	sub	ecx, 1
	jc	.eof
	lodsb
	push	ecx
	mov	ecx, edx
	rep	stosb
	pop	ecx
	jmp	.check_eoi

  .absolute_mode.rle8: ; N: b1 .. bN
	call	.fix_marker
	cmp	ecx, edx
	jae	@f
	mov	edx, ecx
    @@:
	push	ecx
	mov	ecx, edx
	rep	movsb
	pop	ecx
	sub	ecx, edx
	jz	.eof
	test	edx, 1
	jz	.check_eoi
	sub	ecx, 1
	jc	.eof
	inc	esi
  .check_eoi:
	mov	eax, [marker_y]
	cmp	eax, [height]
	jb	.next_run
	jmp	.exit

  .encoded_mode.rle4: ; N: b1 * N
	call	.fix_marker
	sub	ecx, 1
	jc	.eof
	movzx	eax, byte [esi]
	inc	esi
	push	ecx
	mov	ecx, eax
	and	eax, 0xF
	shr	ecx, 4
    @@:
	dec	edx
	js	@f
	mov	[edi], cl
	dec	edx
	js	@f
	mov	[edi+1], al
	add	edi, 2
	jmp	@b
    @@:
	pop	ecx
	jmp	.check_eoi

  .absolute_mode.rle4: ; N: b1 .. bN
	call	.fix_marker
	lea	eax, [edx+1]
	shr	eax, 1
	cmp	ecx, eax
	jbe	@f
	lea	edx, [ecx*2]
    @@:
	push	ecx edx
    @@: dec	edx
	js	@f
	lodsb
	mov	cl, al
	shr	al, 4
	and	cl, 0xF
	stosb
	dec	edx
	js	@f
	mov	[edi], cl
	inc	edi
	jmp	@b
    @@: pop	eax ecx
	and	eax, 0x03
	jp	.check_eoi
	sub	ecx, 1
	jc	.eof
	inc	esi
	jmp	.check_eoi

  .fix_marker:
	mov	edx, eax
	add	eax, [marker_x]
	mov	[marker_x], eax
    @@:
	sub	eax, [scanline_len]
	jle	@f
	mov	[marker_x], eax
	push	eax
	mov	eax, [marker_y]
	inc	eax
	mov	[marker_y], eax
	cmp	eax, [height]
	pop	eax
	jb	@b
	sub	edx, eax
    @@:
        retn

  .exit:
  .eof:
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

	mov	[delta], 4
	mov	eax, [edx + Image.Extended]
	cmp	[eax + bmp.Image.info.BitCount], 32
	je	@f
	cmp	[eax + bmp.Image.info.BitCount], 16
	jne	.error
	mov	[delta], 2
    @@:
	mov	ecx, [edx + Image.Width]
	imul	ecx, [edx + Image.Height]
	imul	ecx, [delta]
	sub	edi, ecx
	jb	.error
	mov	ecx, [ebp]	; use parent stack frame
	mov	[ecx + img.decode.bmp.length_rest - ebp], edi	; !

	push	esi
	mov	esi, eax

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
	pop	esi

;;------------------------------------------------------------------------------------------------;;

	mov	ecx, [edx + Image.Height]

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
	pop	edi
	ret

  .error:
	or	eax, -1
	pop	edi
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

if 0
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
end if

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;
