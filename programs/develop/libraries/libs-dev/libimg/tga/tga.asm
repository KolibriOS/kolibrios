;;================================================================================================;;
;;//// tga.asm //// (c) Nable, 2007-2008 /////////////////////////////////////////////////////////;;
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
	push ebx
	cmp	[_length], 18
	jbe	.nope
	mov	eax, [_data]
	mov	ebx,[eax+1] ;bl=cmatype,bh=subtype
	cmp	bl,1		;cmatype is in [0..1]
	ja	.nope
	cmp	bh,11		;subtype is in [1..3] (non-rle) or in [9..11] (rle)
	ja	.nope
	cmp	bh,9
	jae	.cont1
	cmp	bh,3
	ja	.nope
.cont1: 			;continue testing
	mov	ebx,[eax+16] ;bl=bpp, bh=flags //image descriptor
	test ebx,111b	;bpp must be 8, 15, 16, 24 or 32
	jnz	.maybe15
	shr	bl,3
	cmp	bl,4
	ja	.nope
	jmp	.cont2
.maybe15:
	cmp	bl,15
	jne	.nope
.cont2: 			;continue testing
	test bh,tga.flags.interlace_type ;deinterlacing is not supported yet
	jnz	.nope
	cmp	byte[eax+7],24	;test palette bpp - only 24 and 32 are supported
	je	.yep
	cmp	byte[eax+7],32	;test palette bpp - only 24 and 32 are supported
	je	.yep
.nope:
	xor	eax, eax
	pop ebx
	ret

.yep:
	xor	eax, eax
	inc	eax
	pop ebx
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
  IMGwidth		dd ?
  IMGheight		dd ?
  IMGbpp		dd ?
  DupPixelCount dd ?
  TgaBlockCount dd ?
endl
	pushad
	cld						;paranoia
	and	[DupPixelCount],0	;prepare variables
	and	[TgaBlockCount],0	;prepare variables
	mov	eax,[_data]
	movzx esi,byte[eax]
	lea	esi,[esi+eax+18]	;skip comment and header
	mov	ebx,[eax+12]
	movzx ecx,bx		       ;ecx=width
	shr	ebx,16				;ebx=height
	mov	[IMGwidth],ecx
	mov	[IMGheight],ebx
	movzx edx,byte[eax+16]
	cmp	edx,16
	jnz	@f
	dec	edx					;16bpp tga images are really 15bpp ARGB
@@:
	sub	edx, 16 - Image.bpp16	; 15 -> Image.bpp15, 16 -> Image.bpp16
	mov	[IMGbpp],edx
	stdcall img.create,ecx,ebx,edx
	mov	[esp+28],eax		;save return value
	test eax,eax			;failed to allocate?
	jz	.locret 			;then exit
	cmp	edx,8
	jne	.palette_parsed
	mov edi,[eax+Image.Palette]
	mov	ecx,[_data]
	cmp	byte[ecx+2],3		;we also have grayscale subtype
	jz	.write_grayscale_palette ;that don't hold palette in file
	cmp	byte[ecx+2],11
	jz	.write_grayscale_palette
	mov dh,[ecx+7]			;size of colormap entries in bits
	movzx ecx,word[ecx+5]	;number of colormap entries
	cmp	dh,24
	jz	.skip_24bpp_palette	;test if colormap entries are 24bpp
	rep	movsd				;else they are 32 bpp
	jmp	.palette_parsed
.write_grayscale_palette:
	push eax
	mov	ecx,0x100
	xor	eax,eax
@@:
	stosd
	add	eax,0x010101
	loop @b
	pop	eax
	jmp	.palette_parsed
.skip_24bpp_palette:
	push eax
@@:
	lodsd
	dec esi
	and	eax,0xFFFFFF
;	bswap eax
;	shr	eax,8
	stosd
	loop @b
	pop	eax
.palette_parsed:
	mov	edi,[eax+Image.Data]
	imul ebx,[IMGwidth]		;ebx=width*height

	mov	edx,[IMGbpp]
	add	edx,7
	shr	edx,3				;edx=bytes per pixel
	mov	dh,dl				;dh=dl=bytes per pixel

	mov	eax,[_data]
	cmp	byte[eax+2],9
	jb	.not_an_rle
.tga_read_rle_pixel:
    cmp  [DupPixelCount],0	;Duplicate previously read pixel?
    jg	 .duplicate_previously_read_pixel
    dec  [TgaBlockCount]	;Decrement pixels remaining in block
    jns  .read_non_rle_pixel
    xor  eax,eax
    lodsb
    test al,al				;Start of duplicate-pixel block?
    jns  .2
    and  al,0x7f
    mov  [DupPixelCount],eax ;Number of duplications after this one
    and  [TgaBlockCount],0	;Then read new block header
    jmp  .read_non_rle_pixel
.2:
    mov  dword[TgaBlockCount],eax
.read_non_rle_pixel:
    xor  eax,eax
    mov  dl,dh
@@:
    shl  eax,8
    lodsb
    dec  dl
    jnz  @b
    cmp  dh,3
    jne  .put_pixel
    bswap eax
    shr  eax,8
	jmp	.put_pixel
.duplicate_previously_read_pixel:
    dec  [DupPixelCount]
.put_pixel:
	mov	dl,dh
	push eax
@@:
    stosb
    shr eax,8
    dec dl
    jnz @b
	pop	eax
	dec	ebx
	jnz	.tga_read_rle_pixel
	jmp	.locret
.not_an_rle:
	movzx edx,dl			;dh contains bpp too (for decoding needs)
	imul edx,ebx
	mov	ecx,edx
	rep	movsb				;just copy the image
.locret:	
	popad
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

;