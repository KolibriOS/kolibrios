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

  .quit:
	popa
	mov	eax, [retvalue]
	ret

endp



;;================================================================================================;;
proc img.encode.pnm _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in pnm format                                                       ;;
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

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;

