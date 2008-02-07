use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, 0x1000, 0x1000, @PARAMS, 0x0

;-----------------------------------------------------------------------------

FALSE = 0
TRUE  = 1

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../dll.inc'

include '../../libio/libio.inc'

;-----------------------------------------------------------------------------

START:
	mcall	68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	exit

	invoke	file.open, @PARAMS, O_READ
	or	eax, eax
	jz	exit
	mov	[fh], eax
	invoke	file.size, @PARAMS
	mov	[img_data_len], ebx
	stdcall mem.Alloc, ebx
	or	eax, eax
	jz	exit
	mov	[img_data], eax
	invoke	file.read, [fh], eax, [img_data_len]
	cmp	eax, -1
	je	exit
	cmp	eax, [img_data_len]
	jne	exit
	invoke	file.close, [fh]
	inc	eax
	jz	exit
	
	invoke	img.is_img, [img_data], [img_data_len]
	or	eax, eax
	jz	exit
	invoke	img.decode, [img_data], [img_data_len]
	or	eax, eax
	jz	exit
	invoke	img.to_rgb, eax
	or	eax, eax
	jz	exit
	mov	[rgb_img], eax

;-----------------------------------------------------------------------------

red:
	call	draw_window

still:
	mcall	10
	cmp	eax, 1
	je	red
	cmp	eax, 2
	je	key
	cmp	eax, 3
	je	button
	jmp	still

  key:
	mcall	2
	jmp	still

  button:
	mcall	17
	cmp	ah, 1
	jne	still

  exit:
	mcall	-1

draw_window:
	invoke	gfx.open, TRUE
	mov	[ctx], eax

	mov	edi, [rgb_img]
	mov	ebx, 200 * 65536
	mov	bx, [edi + 0]
	add	bx, 9
	mov	ecx, 200 * 65536
	mov	cx, [edi + 4]
	add	cx, 5 + 21
	mcall	0, , , 0x33FF0000, , s_header

	mov	ebx, [rgb_img]
	mov	ecx, [ebx + 0]
	shl	ecx, 16
	mov	cx, [ebx + 4]
	add	ebx, 4 * 2
	mcall	7, , , <0, 0>

	invoke	gfx.close, [ctx]
	ret

;-----------------------------------------------------------------------------
proc mem.Alloc, size ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx
	mov	ecx, [size]
	add	ecx, 4
	mcall	68, 12
	add	ecx, -4
	mov	[eax], ecx
	add	eax, 4
	pop	ecx ebx
	ret
endp

;-----------------------------------------------------------------------------
proc mem.ReAlloc, mptr, size ;////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx edx
	mov	ecx, [size]
	or	ecx, ecx
	jz	@f
	add	ecx, 4
    @@: mov	edx, [mptr]
	or	edx, edx
	jz	@f
	add	edx, -4
    @@: mcall	68, 20
	or	eax, eax
	jz	@f
	add	ecx, -4
	mov	[eax], ecx
	add	eax, 4
    @@: pop	edx ecx ebx
	ret
endp

;-----------------------------------------------------------------------------
proc mem.Free, mptr ;/////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx
	mov	ecx, [mptr]
	or	ecx, ecx
	jz	@f
	add	ecx, -4
    @@: mcall	68, 13
	pop	ecx ebx
	ret
endp

;-----------------------------------------------------------------------------

s_header db 'Image Viewer (test app)', 0

;-----------------------------------------------------------------------------

align 16
@IMPORT:

library 			\
	libio  , 'libio.obj'  , \
	libgfx , 'libgfx.obj' , \
	libimg , 'libimg.obj'

import	libio			  , \
	file.size  , 'file.size'  , \
	file.open  , 'file.open'  , \
	file.read  , 'file.read'  , \
	file.close , 'file.close'

import	libgfx			, \
	gfx.open  , 'gfx.open'	, \
	gfx.close , 'gfx.close'

import	libimg			  , \
	img.is_img , 'img.is_img' , \
	img.to_rgb , 'img.to_rgb' , \
	img.decode , 'img.decode'

;-----------------------------------------------------------------------------

I_END:

img_data     dd ?
img_data_len dd ?
fh	     dd ?
rgb_img      dd ?

ctx dd ?

@PARAMS rb 512
