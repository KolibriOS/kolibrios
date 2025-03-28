; pcx.inc
; Copyright (c) 2002 Thomas Mathys
; killer@vantage.ch
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


;**********************************************************
; pcx header
;**********************************************************

PCXHEADER_SIZE	equ	128
struct PCXHEADER
	id		rb	1	; id, should be 10
	version	rb	1	; pcx version
	encoding	rb	1	; 1 = rle
	bpp		rb	1	; bits per pixel
	xmin		rw	1	; image dimensions
	ymin		rw	1
	xmax		rw	1
	ymax		rw	1
	hdpi		rw	1	; horizontal resolution in dpi
	vdpi		rw	1	; verttical resolution in dpi
	colormap	rb	48	; 16 color palette
	reserved1	rb	1
	nplanes	rb	1	; # of color planes
	bytesperline	rw	1	; # of bytes per scanline. always even
	palinfo	rw	1	; 1 = color/bw, 2 = grayscale
	hscreensize	rw	1	; horizontal screen size
	vscreensize	rw	1	; vertical screen size
	reserved2	rb	54
ends



;**********************************************************
; lousy pcx decoder. reads only 8 bit rle packed color images (pcx version 5)
; oh, and yes, currently it can't read images with an odd width.
;
; the routine does not only decode the image data, it converts
; it also to a menuetos putimage compatible format (24 bit, rrggbb)
;
; input		:	esi	address of the pcx file in memory
;			edi	pointer to the memory where the depacked
;				image data should go. (width*height*3 bytes)
;			ebx	file size in bytes
;
; output	:	decoded image in [edi]
;
; destroys	:	nothing
;
;**********************************************************

loadPCX:
	pushad
	pushfd

	; calculate number of pixels to decode.
	; this is actually wrong, because if the image has an odd
	; width, there will be an unused pixel at the end of each
	; scanline.
	movzx eax,word [esi+PCXHEADER.xmax]	; eax = image width
	sub ax,[esi+PCXHEADER.xmin]
	inc eax	
	movzx ecx,word [esi+PCXHEADER.ymax]	; ecx = image height
	sub cx,[esi+PCXHEADER.ymin]
	inc ecx
	mul ecx
	mov ebp,eax				; ebp = # of pixels to decode

	; move ebx to beginning of palette and
	; esi to beginning of packed image data
	sub ebx,768
	add ebx,esi
	add esi,PCXHEADER_SIZE
	
	; decode image data
	xor ecx,ecx				; clear bits 8..31 !
	xor eax,eax				;	"
.decode:
	lodsb					; read byte from input stream
	cmp al,192				; encoded/unencoded byte ?
	jae .encoded
	lea edx,[eax*2+eax]			; read color values from
	mov al,[ebx+edx+2]			; palette and store them
	stosb					; in the destination image
	mov al,[ebx+edx+1]
	stosb
	mov al,[ebx+edx+0]
	stosb
	dec ebp					; one less to go...
	jmp .continue	
.encoded:
	and al,00111111b			; calc # of times to repeat
	mov cl,al
	lodsb					; read data byte
	lea edx,[eax*2+eax]			; read color values from palette
	mov al,[ebx+edx+2]
	mov ah,[ebx+edx+1]
	mov dl,[ebx+edx+0]
.bla:						; write color values
	stosb
	mov [edi],ah
	inc edi
	mov [edi],dl
	inc edi
	dec ebp					; one less to go...
	loop .bla
	xor ah,ah				; reset ah to 0 !
.continue:
	or ebp,ebp				; all pixels decoded ?
	jnz .decode

	popfd
	popad
	ret
