;;================================================================================================;;
;;//// pnm.asm //// (c) dunkaist, 2012 ///////////////////////////////////////////////////////////;;
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

include	'pnm.inc'

;;================================================================================================;;
proc img.is.pnm _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in pnm format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	xor	eax, eax

	mov	ecx, [_data]
	mov	cx, word[ecx]
	xchg	cl, ch
	cmp	cx, '1P'
	jb	.is_not_pnm
	cmp	cx, '6P'
	ja	.is_not_pnm

  .is_pnm:
	inc	eax
  .is_not_pnm:
	ret

endp

;;================================================================================================;;
proc img.decode.pnm _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in pnm format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
	width			rd	1
	height			rd	1
	pnm_type		rd	1
	data_type		rd	1	; raw or ascii
	maxval			rd	1
	retvalue		rd	1
endl

	pusha

	mov	esi, [_data]
	lodsw
	cmp	ax, 'P1'
	jne	@f
	mov	[pnm_type], PNM_PBM
	mov	[data_type], PNM_ASCII
	jmp	.parse_header
    @@:
	cmp	ax, 'P2'
	jne	@f
	mov	[pnm_type], PNM_PGM
	mov	[data_type], PNM_ASCII
	jmp	.parse_header
    @@:
	cmp	ax, 'P3'
	jne	@f
	mov	[pnm_type], PNM_PPM
	mov	[data_type], PNM_ASCII
	jmp	.parse_header
    @@:
	cmp	ax, 'P4'
	jne	@f
	mov	[pnm_type], PNM_PBM
	mov	[data_type], PNM_RAW
	jmp	.parse_header
    @@:
	cmp	ax, 'P5'
	jne	@f
	mov	[pnm_type], PNM_PGM
	mov	[data_type], PNM_RAW
	jmp	.parse_header
    @@:
	cmp	ax, 'P6'
	jne	@f
	mov	[pnm_type], PNM_PPM
	mov	[data_type], PNM_RAW
	jmp	.parse_header
    @@:

  .parse_header:
	xor	eax, eax
	mov	[width], eax
	mov	[height], eax
	mov	[maxval], eax

  .next_char:
	lodsb
	cmp	al, '#'
	jb	.next_char
	ja	.read_number
  .comment:
	mov	edi, esi
	mov	al, 0x0A
	mov	ecx, edi
	sub	ecx, [_data]
	neg	ecx
	add	ecx, [_length]
	repne	scasb
	mov	esi, edi
	jmp	.next_char

  .read_number:
	sub	eax, 0x30
	mov	ebx, eax
    @@:
	lodsb
	cmp	al, '0'
	jb	.number_done
	sub	eax, 0x30
	imul	ebx, 10
	add	ebx, eax
	jmp	@b

  .number_done:
	cmp	[width], 0
	jne	@f
	mov	[width], ebx
	jmp	.next_char
    @@:
	cmp	[height], 0
	jne	@f
	mov	[height], ebx
	cmp	[pnm_type], PNM_PBM
	je	.header_parsed
	jmp	.next_char
    @@:
	mov	[maxval], ebx

  .header_parsed:

        cmp     [data_type], PNM_RAW
        jne     @f
        mov     ecx, [width]
        imul    ecx, [height]
        lea     eax, [ecx*3]
        mov     edx, [_data]
        add     edx, [_length]
        sub     edx, esi
        cmp     eax, edx
        ja      .error
    @@:

	mov	eax, [pnm_type]
	cmp	eax, PNM_PBM
	je	.pbm
	cmp	eax, PNM_PGM
	je	.pgm
	cmp	eax, PNM_PPM
	je	.ppm
	jmp	.quit


include	'pbm.asm'
include	'pgm.asm'
include	'ppm.asm'

  .error:
        popa
        xor     eax, eax
        ret

  .quit:
	popa
	mov	eax, [retvalue]
	ret

endp



;;================================================================================================;;
proc img.encode.pnm _img, _common, _specific ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in pnm format                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> [_img]      = pointer to image                                                                 ;;
;> [_common]   = format independent options                                                       ;;
;> [_specific] = 0 / pointer to the structure of format specific options                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                                              ;;
;< ecx = error code / the size of encoded data                                                    ;;
;;================================================================================================;;
locals
 	encoded_file		rd 1
 	encoded_file_size	rd 1
 	encoded_data_size	rd 1
endl
	push	ebx

	mov	ebx, [_img]
	mov	eax, [ebx + Image.Type]
	cmp	eax, Image.bpp1
	je	.pbm
	cmp	eax, Image.bpp8g
	je	.pgm
	cmp	eax, Image.bpp24
	je	.ppm
	mov	ecx, LIBIMG_ERROR_BIT_DEPTH
	jmp	.error
  .pbm:
	mov	ecx, [ebx + Image.Width]
	add	ecx, 7
	shr	ecx, 3
	imul	ecx, [ebx + Image.Height]
	mov	[encoded_data_size], ecx
	add	ecx, (2 + 1) + (2 + pnm._.creator_comment.size + 1) + (5 + 1 + 5 + 1) + (3 + 1)
	mov	[encoded_file_size], ecx
	stdcall	[mem.alloc], ecx
	mov	[encoded_file], eax
	test	eax, eax
	jnz	@f
	mov	ecx, LIBIMG_ERROR_OUT_OF_MEMORY
	jmp	.error
    @@:
	mov	edi, eax
	mov	ax, 'P4'
	call	pnm._.write_header
	mov	esi, [ebx + Image.Data]
	mov	ecx, [encoded_data_size]
	rep	movsb
	mov	eax, [encoded_file]
	mov	ecx, [encoded_file_size]
	jmp	.quit
	
  .pgm:
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
	mov	[encoded_data_size], ecx
	add	ecx, (2 + 1) + (2 + pnm._.creator_comment.size + 1) + (5 + 1 + 5 + 1) + (3 + 1)
	mov	[encoded_file_size], ecx
	stdcall	[mem.alloc], ecx
	mov	[encoded_file], eax
	test	eax, eax
	jnz	@f
	mov	ecx, LIBIMG_ERROR_OUT_OF_MEMORY
	jmp	.error
    @@:
	mov	edi, eax
	mov	ax, 'P5'
	call	pnm._.write_header
	mov	dword[edi], '255 '
	add	edi, 3
	mov	byte[edi], 0x0A
	add	edi, 1
	mov	esi, [ebx + Image.Data]
	mov	ecx, [encoded_data_size]
	rep	movsb
	mov	eax, [encoded_file]
	mov	ecx, [encoded_file_size]
	jmp	.quit

  .ppm:
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
	lea	ecx, [ecx*3]
	mov	[encoded_data_size], ecx
	add	ecx, (2 + 1) + (2 + pnm._.creator_comment.size + 1) + (5 + 1 + 5 + 1) + (3 + 1)
	mov	[encoded_file_size], ecx
	stdcall	[mem.alloc], ecx
	mov	[encoded_file], eax
	test	eax, eax
	jnz	@f
	mov	ecx, LIBIMG_ERROR_OUT_OF_MEMORY
	jmp	.error
    @@:
	mov	edi, eax
	mov	ax, 'P6'
	call	pnm._.write_header
	mov	dword[edi], '255 '
	add	edi, 3
	mov	byte[edi], 0x0A
	add	edi, 1
	mov	esi, [ebx + Image.Data]
	mov	ecx, [ebx + Image.Width]
	imul	ecx, [ebx + Image.Height]
    @@:
	lodsb
	mov	byte[edi+2], al
	lodsb
	mov	byte[edi+1], al
	movsb
	add	edi, 2
	dec	ecx
	jnz	@b
	mov	eax, [encoded_file]
	mov	ecx, [encoded_file_size]
	jmp	.quit

  .error:
	xor	eax, eax
  .quit:
	pop	ebx
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
proc	pnm._.get_number
	sub	eax, '0'
	mov	ebx, eax
    @@:
	lodsb
	cmp	al, '0'
	jb	.quit
	sub	eax, '0'
	lea	eax, [ebx*8 + eax]
	lea	ebx, [ebx*2 + eax]
;	imul	ebx, 10
;	add	ebx, eax
	jmp	@b
  .quit:
	ret
endp


proc	pnm._.write_header
	stosw
	mov	byte[edi], 0x0A
	add	edi, 1
	mov	word[edi], '# '
	add	edi, 2
	mov	esi, pnm._.creator_comment
	mov	ecx, pnm._.creator_comment.size
	rep	movsb
	mov	byte[edi], 0x0A
	add	edi, 1

	push	edi
	mov	al, ' '
	mov	ecx, (5 + 1 + 5)
	rep	stosb
	pop	edi
	push	edi
	add	edi, 4
	mov	eax, [ebx + Image.Width]
	mov	ecx, 10
  .write_width:
	xor	edx, edx
	div	cx
	add	dl, '0'
	mov	byte[edi], dl
	dec	edi
	test	ax, ax
	jnz	.write_width
	mov	eax, [ebx + Image.Height]
	pop	edi
	push	edi
	add	edi, 10
  .write_height:
	xor	edx, edx
	div	cx
	add	dl, '0'
	mov	byte[edi], dl
	dec	edi
	test	ax, ax
	jnz	.write_height
	pop	edi
	add	edi, 11
	mov	byte[edi], 0x0A
	add	edi, 1
	ret
endp
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;

sz pnm._.creator_comment ,'CREATOR: KolibriOS / libimg'

