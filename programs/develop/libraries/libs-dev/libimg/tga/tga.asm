;;================================================================================================;;
;;//// tga.asm //// (c) Nable, 2007-2008, (c) dunkaist, 2012 /////////////////////////////////////;;
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
;;   1. Hiview 1.2 by Mohammad A. REZAEI                                                          ;;
;;   2. Truevision TGA FILE FORMAT SPECIFICATION Version 2.0                                      ;;
;;      Technical Manual Version 2.2 January, 1991                                                ;;
;;                                                                                                ;;
;;================================================================================================;;

include 'tga.inc'

;;================================================================================================;;
proc img.is.tga _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in Targa format)                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	push	ebx
	cmp	[_length], 18
	jbe	.nope
	mov	ebx, [_data]
	mov	eax, dword[ebx + tga_header.colormap_type]
	cmp	al, 1
	ja	.nope
	cmp	ah, 11
	ja	.nope
	cmp	ah, 9
	jae	.cont1
	cmp	ah, 3
	ja	.nope
  .cont1:
	mov	eax, dword[ebx + tga_header.image_spec.depth]
	test	eax, 111b	; bpp must be 8, 15, 16, 24 or 32
	jnz	.maybe15
	shr	al, 3
	cmp	al, 4
	ja	.nope
	jmp	.cont2
  .maybe15:
	cmp	al, 15
	jne	.nope
  .cont2: 			; continue testing
	movzx	eax, byte[ebx + tga_header.colormap_spec.entry_size]	; palette bpp
	cmp	eax, 0
	je	.yep
	cmp	eax, 16
	je	.yep
	cmp	eax, 24
	je	.yep
	cmp	eax, 32
	je	.yep
  .nope:
	xor	eax, eax
	pop	ebx
	ret

  .yep:
	xor	eax, eax
	inc	eax
	pop	ebx
	ret
endp

;;================================================================================================;;
proc img.decode.tga _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in Targa format                ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
	width		dd ?
	height		dd ?
	bytes_per_pixel	dd ?
	retvalue	dd ?
endl
	push	ebx esi edi
	mov	ebx, [_data]
	movzx	esi, byte[ebx]
	lea	esi, [esi + ebx + sizeof.tga_header]	; skip comment and header
	mov	edx, dword[ebx + tga_header.image_spec.width]
	movzx	ecx, dx			; ecx = width
	shr	edx, 16			; edx = height
	mov	[width], ecx
	mov	[height], edx
	movzx	eax, byte[ebx + tga_header.image_spec.depth]
	add	eax, 7
	shr	eax, 3
	mov	[bytes_per_pixel], eax
	movzx	eax, byte[ebx + tga_header.image_spec.depth]

	cmp	eax, 8
	jne	@f
	mov	eax, Image.bpp8i
	jmp	.type_defined
    @@:
	cmp	eax, 15
	jne	@f
	mov	eax, Image.bpp15
	jmp	.type_defined
    @@:
	cmp	eax, 16
	jne	@f
	mov	eax, Image.bpp15	; 16bpp tga images are really 15bpp ARGB
	jmp	.type_defined
    @@:
	cmp	eax, 24
	jne	@f
	mov	eax, Image.bpp24
	jmp	.type_defined
    @@:
	cmp	eax, 32
	jne	@f
	mov	eax, Image.bpp32
	jmp	.type_defined
    @@:
  .type_defined:
	stdcall	img.create, ecx, edx, eax
	mov	[retvalue], eax
	test	eax, eax		; failed to allocate?
	jz	.done			; then exit
	mov	ebx, eax
	cmp	dword[ebx + Image.Type], Image.bpp8i
	jne	.palette_parsed
	mov	edi, [ebx + Image.Palette]
	mov	ecx, [_data]
	cmp	byte[ecx + tga_header.image_type], 3	; we also have grayscale subtype
	jz	.write_grayscale_palette		; that don't hold palette in file
	cmp	byte[ecx + tga_header.image_type], 11
	jz	.write_grayscale_palette
	movzx	eax, byte[ecx + tga_header.colormap_spec.entry_size]	; size of colormap entries in bits
	movzx	ecx, word[ecx + tga_header.colormap_spec.colormap_length]	; number of colormap entries
	cmp	eax, 24
	je	.24bpp_palette
	cmp	eax, 16
	je	.16bpp_palette
	rep	movsd			; else they are 32 bpp
	jmp	.palette_parsed
  .write_grayscale_palette:
	mov	ecx, 0x100
	xor	eax, eax
    @@:
	stosd
	add	eax, 0x010101
	loop	@b
	jmp	.palette_parsed
  .16bpp_palette:			; FIXME: code copypasted from img.do_rgb, should use img.convert
	push	ebx edx ebp
    @@:
	movzx	eax, word[esi]
	mov	ebx, eax
	add	esi, 2
	and	eax, (0x1F) or (0x1F shl 10)
	and	ebx, 0x1F shl 5
	lea	edx, [eax + eax]
	shr	al, 2
	mov	ebp, ebx
	shr	ebx, 2
	shr	ah, 4
	shl	dl, 2
	shr	ebp, 7
	add	eax, edx
	add	ebx, ebp
	mov	[edi], al
	mov	[edi + 1], bl
	mov	[edi + 2], ah
	add	edi, 4
	loop	@b
	pop	ebp edx ebx
	jmp	.palette_parsed

  .24bpp_palette:
    @@:
	lodsd
	dec	esi
	and	eax, 0xffffff
	stosd
	loop	@b
  .palette_parsed:
	mov	edi, [ebx + Image.Data]
	mov	ebx, [width]
	imul	ebx, [height]
	mov	edx, [bytes_per_pixel]
	mov	eax, [_data]
	test	byte[eax + tga_header.image_type], 0x08
	jz	.uncompressed
  .next_rle_packet:
	xor	eax, eax
	lodsb
	btr	ax, 7			; Run-length packet?
	jnc	.raw_packet
	add	eax, 1
	sub	ebx, eax
    @@:
	mov	ecx, edx
	rep	movsb
	sub	esi, edx
	sub	eax, 1
	jnz	@b
	add	esi, edx
	test	ebx, ebx
	jnz	.next_rle_packet
	jmp	.done
  .raw_packet:
	mov	ecx, eax
	add	ecx, 1
	sub	ebx, ecx
	imul	ecx, edx
	rep	movsb
	test	ebx, ebx
	jnz	.next_rle_packet
  .uncompressed:
	imul	edx, ebx
	mov	ecx, edx
	rep	movsb
  .done:
	xor	ebx, ebx
	mov	esi, [_data]
	test	byte[esi + tga_header.image_spec.descriptor], TGA_START_TOP
	jnz	@f
	or	ebx, FLIP_VERTICAL
    @@:
	test	byte[esi + tga_header.image_spec.descriptor], TGA_START_RIGHT
	jz	@f
	or	ebx, FLIP_HORIZONTAL
    @@:
	test	ebx, ebx
	jz	@f
	stdcall	img.flip, [retvalue], ebx
    @@:
	pop	edi esi ebx
	mov	eax, [retvalue]
	ret
endp

;;================================================================================================;;
proc img.encode.tga _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in Targa format                                                     ;;
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
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
