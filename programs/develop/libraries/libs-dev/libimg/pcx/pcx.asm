;;================================================================================================;;
;;//// pcx.asm //// (c) dunkaist, 2010,2012 //////////////////////////////////////////////////////;;
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

include	'pcx.inc'

;;================================================================================================;;
proc img.is.pcx _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in pcx format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	push	ecx edi
	xor	eax, eax

	mov	edi, [_data]

	cmp	byte[edi + pcx_header.magic_number], 0x0A
	jne	.is_not_pcx
	cmp	byte[edi + pcx_header.version], 5
	jne	.is_not_pcx
	cmp	byte[edi + pcx_header.encoding], 1
	jne	.is_not_pcx
	cmp	byte[edi + pcx_header.reserved], 0
	jne	.is_not_pcx

	add	edi, pcx_header.filler
	xor	al, al
	mov	ecx, 58
	cld
	repe	scasb
	test	ecx, ecx
	jnz	.is_not_pcx

  .is_pcx:
	inc	eax

  .is_not_pcx:
	pop	edi ecx
	ret
endp


;;================================================================================================;;
proc img.decode.pcx _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in pcx format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
	nplanes			rd	1
	xsize			rd	1
	ysize			rd	1
	bpl			rd	1
	total_bpl		rd	1
	line_begin		rd	1
	retvalue		rd	1		; 0 (error) or pointer to image 
endl

	pusha

	mov	esi, [_data]
	movzx	eax, byte[esi + pcx_header.nplanes]
	mov	[nplanes], eax
	movzx	ebx, word[esi + pcx_header.bpl]
	mov	[bpl], ebx
	imul	eax, ebx
	mov	[total_bpl], eax

	movzx	eax, word[esi + pcx_header.xmax]
	sub	ax, word[esi + pcx_header.xmin]
	inc	eax
	mov	[xsize], eax

	movzx	ebx, word[esi + pcx_header.ymax]
	sub	bx, word[esi + pcx_header.ymin]
	inc	ebx
	mov	[ysize], ebx

	cmp	[esi + pcx_header.bpp], 1
	jz	.monochrome
	cmp	byte[esi + pcx_header.nplanes], 3
	jnz	.indexed


  .24bit:

	stdcall	img.create, eax, ebx, Image.bpp24
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	esi, [_data]
	add	esi, 128			; skip header
	mov	edi, [eax + Image.Data]
	add	edi, 2
	mov	[line_begin], edi

  .24bit.scanline:
	mov	ebx, [total_bpl]
  .24bit.color_line:
	mov	edx, [bpl]
  .24bit.next_byte:
	call	pcx._.get_byte
	sub	edx, ecx
    @@:
	mov	[edi], al
	add	edi, [nplanes]
	dec	ecx
	jnz	@b

	test	edx, edx
	jnz	.24bit.next_byte

  .24bit.end_color_line:
	test	ebx, ebx
	jz	.24bit.end_full_line
	dec	[line_begin]
	mov	edi, [line_begin]
	jmp	.24bit.color_line

  .24bit.end_full_line:
	dec	[ysize]
	jz	.quit
	add	edi, 2
	bt	[xsize], 0
	jnc	@f
	sub	edi, 3
    @@:
	mov	[line_begin], edi
	jmp	.24bit.scanline


  .indexed:

	stdcall	img.create, eax, ebx, Image.bpp8
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	ebx, eax
	mov	esi, [_data]
	add	esi, [_length]
	sub	esi, 768
	mov	edi, [eax + Image.Palette]
	mov	ecx, 256
	xor	eax, eax
    @@:
	lodsw
	xchg	al, ah
	shl	eax, 8
	lodsb
	stosd
	dec	ecx
	jnz	@b

	mov	esi, [_data]
	add	esi, 128
	mov	edi, [ebx + Image.Data]

  .indexed.line:
	mov	ebx, [total_bpl]
  .indexed.next_byte:
	call	pcx._.get_byte
    @@:
	stosb
	dec	ecx
	jnz	@b
	test	ebx, ebx
	jnz	.indexed.next_byte

	dec	[ysize]
	jnz	.indexed.line
	jmp	.quit


  .monochrome:

	stdcall	img.create, eax, ebx, Image.bpp1
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	edi, [eax + Image.Palette]
	mov	[edi], dword 0xff000000
	mov	[edi + 4], dword 0xffffffff

	mov	esi, [_data]
	add	esi, 128
	mov	edi, [eax + Image.Data]


  .monochrome.line:
	mov	ebx, [total_bpl]
  .monochrome.next_byte:
	call	pcx._.get_byte
    @@:
	stosb
	dec	ecx
	jnz	@b
	test	ebx, ebx
	jnz	.monochrome.next_byte
	dec	[ysize]
	jnz	.monochrome.line
;	jmp	.quit


  .quit:
	popa
	mov	eax, [retvalue]
	ret
endp


;;================================================================================================;;
proc img.encode.pcx _img, _common, _specific ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in pcx format                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> [_img]      = pointer to image                                                                 ;;
;> [_common]   = format independent options                                                       ;;
;> [_specific] = 0 / pointer to the structure of format specific options                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                                              ;;
;< ecx = error code / the size of encoded data                                                    ;;
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
proc	pcx._.get_byte

	xor	ecx, ecx
	lodsb
	cmp	al, 0xC0
	setb	cl
	jb	.done
	and	al, 0x3F
	mov	cl, al
	lodsb
  .done:
	sub	ebx, ecx
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
