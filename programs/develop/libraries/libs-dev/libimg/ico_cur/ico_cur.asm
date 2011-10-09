;;================================================================================================;;
;;//// ico.asm //// (c) mike.dld, 2007-2008, (c) diamond, 2009 ///////////////////////////////////;;
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
;;   1. "Icons in Win32"                                                                          ;;
;;      by John Hornick, Microsoft Corporation                                                    ;;
;;      http://msdn2.microsoft.com/en-us/library/ms997538.aspx                                    ;;
;;                                                                                                ;;
;;================================================================================================;;

include 'ico_cur.inc'

;;================================================================================================;;
;;proc img.is.ico _data, _length ;////////////////////////////////////////////////////////////////;;
img.is.ico:
	mov	edx, 0x00010000	; icon type = 1
	jmp	@f
img.is.cur:
	mov	edx, 0x00020000	; cursor type = 2
@@:
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in ICO format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
; test 1 (length of data): data must contain FileHeader
	mov	ecx, [esp+8]
	sub	ecx, sizeof.ico.FileHeader
	jb	.nope
; test 2: signature
	mov	eax, [esp+4]
	cmp	dword [eax], edx	; Reserved & Type
	jne	.nope
; test 3: count must be non-zero
	movzx	eax, [eax + ico.FileHeader.Count]
	test	eax, eax
	jz	.nope
; test 4 (length of data): data must containt Count dir entries
	shl	eax, 4
	sub	ecx, eax
	jae	.yep

  .nope:
	xor	eax, eax
	ret	8

  .yep:
	xor	eax, eax
	inc	eax
	ret	8
;endp

;;================================================================================================;;
proc img.decode.ico_cur _data, _length, _options ;////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in ICO/CUR format              ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;> _options = options for decoding (e.g. background color)                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
  count dd ?
  img dd ?
  main_img dd ?
endl

	push	ebx esi edi

; img.is.ico has been already called by img.decode
;	stdcall img.is.ico, [_data], [_length]
;	or	eax, eax
;	jz	.error

	mov	ebx, [_data]
	movzx	eax, [ebx + ico.FileHeader.Count]
	mov	[count], eax
	and	[img], 0
	and	[main_img], 0
.loop:
	mov	ecx, [ebx + sizeof.ico.FileHeader + ico.DirEntry.ByteSize]
	mov	edx, [ebx + sizeof.ico.FileHeader + ico.DirEntry.ImageOffset]
	mov	eax, [_length]
	sub	eax, edx
	jb	.skip
	cmp	eax, ecx
	jb	.skip
	cmp	ecx, 12	; length test, see img.is.bmp
	jb	.skip
	add	edx, [_data]
	push	[_options]
	push	ecx
	push	edx
	call	img.decode.ico._.decode_icon_data
	test	eax, eax
	jz	.skip
	push	0xFFFFFF	; set bgr to white if no given
	mov	edx, [_options]
	test	edx, edx
	jz	@f
	cmp	[edx + ImageDecodeOptions.UsedSize], ImageDecodeOptions.BackgroundColor + 4
	jb	@f
	add	esp, 4
	push	[edx + ImageDecodeOptions.BackgroundColor]
@@:
	call	img.decode.ico._.decode_icon_mask
	test	eax, eax
	jz	.skip
	mov	edx, [img]
	test	edx, edx
	jz	.first
	mov	[edx + Image.Next], eax
	mov	[eax + Image.Previous], edx
	jmp	@f
.first:
	mov	[main_img], eax
@@:
	mov	[img], eax
.skip:
	add	ebx, sizeof.ico.DirEntry
	dec	[count]
	jnz	.loop

	mov	eax, [main_img]
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
img.encode.cur:
proc img.encode.ico _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in ICO format                                                       ;;
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

img.decode.ico._.decode_icon_data:
; create stack frame and jump to common BMP+ICO code
	push	ebp
	mov	ebp, esp
	sub	esp, 12
	mov	byte [ebp - 3], 1	; bIsIco
	jmp	img.decode.bmp.common

img.decode.ico._.decode_icon_mask:
	mov	edx, [eax + Image.Width]
	add	edx, 31
	shr	edx, 3
	and	edx, not 3
	push	edx
	imul	edx, [eax + Image.Height]
	cmp	ecx, edx
	pop	edx
	jb	.error.free
	mov	edi, [eax + Image.Data]
	push	ebp ebx eax
	xor	ebp, ebp
	mov	ebx, [eax + Image.Extended]
	cmp	[ebx + bmp.Image.info.Height], 0
	js	@f
	push	edx
	imul	edx, [eax + Image.Height]
	add	esi, edx
	pop	edx
	lea	ebp, [edx+edx]
	sub	esi, edx
@@:
	mov	ebx, [eax + Image.Height]
	mov	ecx, [eax + Image.Width]
; for now, BMP code produces only 8 and 32 bpp images
	cmp	[eax + Image.Type], Image.bpp8
	jz	.bpp8
.bpp32:
	mov	edx, [esp+16]	; get background color
.bpp32.extloop:
	push	ecx
.bpp32.innloop:
	lodsd
	bswap	eax
	push	32
.bpp32.dwordloop:
	add	eax, eax
	jnc	@f
	mov	[edi], edx
@@:
	add	edi, 4
	dec	ecx
	jz	@f
	dec	dword [esp]
	jnz	.bpp32.dwordloop
@@:
	pop	eax
	test	ecx, ecx
	jnz	.bpp32.innloop
	sub	esi, ebp
	pop	ecx
	dec	ebx
	jnz	.bpp32.extloop
	pop	eax ebx ebp
	ret	4
.bpp8:
	push	edi
	mov	edi, [eax + Image.Palette]
	mov	eax, [esp+20]	; get background color
; if palette already has index for bgr color, use it
	push	ecx
	mov	ecx, 256
	repnz	scasd
	jnz	.bpp8.notfound
	not	cl	; cl = index
	pop	edx
	pop	edi
.bpp8.extloop:
	push	edx
.bpp8.innloop:
	lodsd
	bswap	eax
	push	32
.bpp8.dwordloop:
	add	eax, eax
	jnc	@f
	mov	[edi], cl
@@:
	inc	edi
	dec	edx
	jz	@f
	dec	dword [esp]
	jnz	.bpp8.dwordloop
@@:
	pop	eax
	test	edx, edx
	jnz	.bpp8.innloop
	sub	esi, ebp
	pop	edx
	dec	ebx
	jnz	.bpp8.extloop
	pop	eax ebx ebp
	ret	4
.bpp8.notfound:
; get maximum used color; if index < 255, then we can add one color to palette
	pop	ecx
	pop	edi
	pop	eax
	mov	edx, [eax + Image.Width]
	imul	edx, ebx
	mov	edi, [eax + Image.Data]
	xor	ecx, ecx
.bpp8.scanloop:
	cmp	[edi], cl
	jb	@f
	mov	cl, [edi]
@@:
	inc	edi
	dec	edx
	jnz	.bpp8.scanloop
	inc	cl
	jz	.bpp8.nospace
	mov	edx, [esp+8]
	mov	edi, [eax + Image.Palette]
	mov	[edi+ecx*4], edx	; set palette color
	mov	edi, [eax + Image.Data]
	mov	edx, [eax + Image.Width]
	push	eax
	jmp	.bpp8.extloop
.bpp8.nospace:
; convert to 24 bpp
	mov	edx, [eax + Image.Width]
	imul	edx, ebx
	lea	edx, [edx*3]
	push	eax
	invoke	mem.alloc, edx
	mov	edi, eax
	pop	eax
	test	edi, edi
	jz	.error.free2
	push	eax esi edi
	mov	esi, eax
	call	img._.do_rgb
	pop	edi esi eax
	xchg	edi, [eax + Image.Data]
	mov	byte [eax + Image.Type], Image.bpp24
	push	eax
	invoke	mem.free, edi
	pop	eax
	push	eax
	mov	ecx, [eax + Image.Width]
	mov	edi, [eax + Image.Data]
.bpp24:
	mov	edx, [esp+16]	; get background color
.bpp24.extloop:
	push	ecx
.bpp24.innloop:
	lodsd
	bswap	eax
	push	32
.bpp24.dwordloop:
	add	eax, eax
	jnc	@f
	mov	[edi], dx
	ror	edx, 16
	mov	[edi+2], dl
	ror	edx, 16
@@:
	add	edi, 3
	dec	ecx
	jz	@f
	dec	dword [esp]
	jnz	.bpp24.dwordloop
@@:
	pop	eax
	test	ecx, ecx
	jnz	.bpp24.innloop
	sub	esi, ebp
	pop	ecx
	dec	ebx
	jnz	.bpp24.extloop
	pop	eax ebx ebp
	ret	4
.error.free2:
	pop	ebx ebp
.error.free:
	stdcall	img._.delete, eax
	xor	eax, eax
	ret	4

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;
