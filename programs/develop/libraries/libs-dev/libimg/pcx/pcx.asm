;;================================================================================================;;
;;//// pcx.asm //// (c) dunkaist, 2010,2012-2013 /////////////////////////////////////////////////;;
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

	push	edi
	xor	eax, eax

	mov	edi, [_data]

        mov     ecx, [edi]
        shl     ecx, 8
        cmp     ecx, 0x01050a00
	jne	.is_not_pcx
	cmp	byte[edi + pcx_header.reserved], 0
	jne	.is_not_pcx

	add	edi, pcx_header.filler
	mov	ecx, 58/2
	repe	scasw
	jne	.is_not_pcx

  .is_pcx:
	inc	eax
  .is_not_pcx:
	pop	edi
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
	num_planes		rd	1
	width			rd	1
	height			rd	1
	bp_plane        	rd	1
	bp_scanline		rd	1
	line_begin		rd	1
        cur_scanline            rd      1
	retvalue		rd	1		; 0 (error) or pointer to image 
endl

	pusha

	mov	esi, [_data]
	movzx	eax, byte[esi + pcx_header.nplanes]
	mov	[num_planes], eax
	movzx	ebx, word[esi + pcx_header.bpl]
	mov	[bp_plane], ebx
	imul	eax, ebx
	mov	[bp_scanline], eax

	movzx	eax, word[esi + pcx_header.xmax]
	sub	ax, word[esi + pcx_header.xmin]
	inc	eax
	mov	[width], eax

	movzx	ebx, word[esi + pcx_header.ymax]
	sub	bx, word[esi + pcx_header.ymin]
	inc	ebx
	mov	[height], ebx

	cmp	[esi + pcx_header.bpp], 1
	jz	.monochrome
	cmp	byte[esi + pcx_header.nplanes], 3
	jnz	.indexed


  .24bit:
	stdcall	img.create, [bp_plane], 1, Image.bpp24
	mov	[cur_scanline], eax
	test	eax, eax
	jz	.quit

	stdcall	img.create, [width], [height], Image.bpp24
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	esi, [_data]
	add	esi, sizeof.pcx_header
	mov	edx, [eax + Image.Data]

  .24bit.scanline:
        mov     edi, [cur_scanline]
	mov	ebx, [bp_scanline]
    @@:
	call	pcx._.get_byte
        rep     stosb
        test    ebx, ebx
        jnz     @b
	stdcall pcx._.scanline_unpack, [width], [bp_plane], [cur_scanline], [num_planes]
	dec	[height]
	jnz	.24bit.scanline
        jmp     .quit


  .indexed:
	stdcall	img.create, [width], [height], Image.bpp8i
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	ebx, eax
	mov	esi, [_data]
	add	esi, [_length]
	sub	esi, 256*3                      ; rgb triplets
	mov	edi, [eax + Image.Palette]
	mov	ecx, 256
	mov     eax, 0x0000ff00
    @@:
        mov     al, [esi + 0]
        mov     [edi + 2], ax
        mov     al, [esi + 1]
        mov     [edi + 1], al
        mov     al, [esi + 2]
        mov     [edi + 0], al
        add     esi, 3
        add     edi, 4
	dec	ecx
	jnz	@b

	mov	esi, [_data]
	add	esi, sizeof.pcx_header
	mov	edi, [ebx + Image.Data]

  .indexed.scanline:
	mov	ebx, [bp_scanline]
    @@:
	call	pcx._.get_byte
	rep     stosb
	test	ebx, ebx
	jnz	@b
	dec	[height]
	jnz	.indexed.scanline
	jmp	.quit


  .monochrome:
	stdcall	img.create, [width], [height], Image.bpp1
	mov	[retvalue], eax
	test	eax, eax
	jz	.quit

	mov	edi, [eax + Image.Palette]
	mov	[edi], dword 0xff000000
	mov	[edi + 4], dword 0xffffffff

	mov	esi, [_data]
	add	esi, sizeof.pcx_header
	mov	edi, [eax + Image.Data]

  .monochrome.scanline:
	mov	ebx, [bp_scanline]
    @@:
	call	pcx._.get_byte
	rep     stosb
	test	ebx, ebx
	jnz	@b
	dec	[height]
	jnz	.monochrome.scanline
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

        mov     ecx, 1
        xor     eax, eax
	lodsb
	cmp	eax, 0xc0
	jb	@f
	and	eax, 0x3f
	mov	ecx, eax
	lodsb
    @@:
	sub	ebx, ecx
	ret
endp


proc pcx._.scanline_unpack _width, _bp_plane, _scanline, _num_planes
        push    esi

        mov     esi, [_scanline]
        mov     ebx, [_num_planes]
        dec     ebx

  .plane:
        mov     ecx, [_width]
        mov     edi, edx
        add     edi, ebx
    @@:
        mov     al, [esi]
        mov     [edi], al
        add     esi, 1
        add     edi, [_num_planes]
        dec     ecx
        jnz     @b
        add     esi, [_bp_plane]
        sub     esi, [_width]
        dec     ebx
        jns     .plane

        mov     edx, edi
        pop     esi
        ret
endp
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
