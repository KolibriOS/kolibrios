;;================================================================================================;;
;;//// wbmp.asm //// (c) dunkaist, 2011-2012 /////////////////////////////////////////////////////;;
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
;;   1. WAP WAE Specification                                                                     ;;
;;      http://www.wapforum.org/what/technical/SPEC-WAESpec-19990524.pdf                          ;;
;;   2. "Converting a BMP picture to WBMP format"                                                 ;;
;;      by Sajjitha Gunawardana                                                                   ;;
;;      http://www.codeproject.com/KB/graphics/bmp_to_wbmp_converter.aspx                         ;;
;;                                                                                                ;;
;;================================================================================================;;

;include 'wbmp.inc'			; wbmp is too simple, so we do not need any *.inc files

;;================================================================================================;;
proc img.is.wbmp _data, _length ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in wbmp format)                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	push	esi

	mov	esi, [_data]
	lodsw
	test	ax, ax			; two first bytes of any wbmp file are zeros, check this signature
	jnz	.is_not_wbmp

					; but two byte signature is not enough to be sure it is wbmp file
					; let's calculate the whole image size and compare it with [_length]

					; calculate width first. you can read how this number is stored in wbmp format specification (see "References" above)
	xor	eax, eax
    @@:
	shl	eax, 7
	lodsb
	sal	al, 1
	jc	@b
	shr	eax, 1
	add	eax, 7
	shr	eax, 3
	mov	ecx, eax

					; calculate height
	xor	eax, eax
    @@:
	shl	eax, 7
	lodsb
	sal	al, 1
	jc	@b
	shr	eax, 1

	imul	eax, ecx		; image_size = width*height
					; raw file consists of a header and data. our image_size is the size of data only
	sub	esi, [_data]		; get header size
	add	eax, esi
	cmp	eax, [_length]
	je	.is_wbmp


  .is_not_wbmp:				; return 0
	pop	esi
	xor	eax, eax
	ret

  .is_wbmp:				; return 1
	pop	esi
	xor	eax, eax
	inc	eax
	ret
endp

;;================================================================================================;;
proc img.decode.wbmp _data, _length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in wbmp format                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
	push	ebx edx esi edi

	mov	esi, [_data]
	lodsw

	xor	eax, eax
    @@:
	shl	eax, 7
	lodsb
	sal	al, 1
	jc	@b
	shr	eax, 1	
	mov	ebx, eax

	xor	eax, eax
    @@:
	shl	eax, 7
	lodsb
	sal	al, 1
	jc	@b
	shr	eax, 1
	mov	edx, eax

	push	ebx ecx edx
	stdcall	img.create, ebx, edx, Image.bpp1
	pop	edx ecx ebx
	test	eax, eax
	jz	.quit

	mov	edi, [eax + Image.Palette]
	mov	[edi], dword 0xff000000
	mov	[edi + 4], dword 0xffffffff

	add	ebx, 7
	shr	ebx, 3
	imul	ebx, edx

	mov	ecx, ebx
	mov	edi, [eax + Image.Data]
	rep	movsb

  .quit:
	pop	edi esi edx ebx
	ret
endp



;;================================================================================================;;
proc img.encode.wbmp _img, _p_length, _options ;//////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in wbmp format                                                      ;;
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
