use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, 0x1000, 0x1000, 0, 0

;-----------------------------------------------------------------------------

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../dll.inc'

include '../../libio/libio.inc'
include '../../libimg/libimg.inc'

;-----------------------------------------------------------------------------

START:
	mcall	68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	exit

	invoke	file.open, input_file, O_READ
	or	eax, eax
	jz	exit
	mov	[fh], eax

	invoke	file.size, input_file
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
	
	invoke	img.decode, [img_data], [img_data_len], 0
	or	eax, eax
	jz	exit
	mov	[image_initial], eax

	stdcall	mem.Free, [img_data]
	test	eax, eax
	jz	exit

;-----------------------------------------------------------------------------

	mov	eax, [image_initial]
	stdcall	[img.create], [eax + Image.Width], [eax + Image.Height], Image.bpp24
	test	eax, eax
	jz	exit
	mov	[image_to_rgb2] ,eax
	stdcall [img.to_rgb2], [image_initial], [eax + Image.Data]

	mov	eax, [image_to_rgb2]
	mov	esi, [eax + Image.Data]
	mov	edi, esi
	mov	ecx, [eax + Image.Width]
	imul	ecx, [eax + Image.Height]
	lea	ecx, [ecx*3]
    @@:
	lodsb
	not	al
	stosb
	dec	ecx
	jnz	@b
;-----------------------------------------------------------------------------

redraw:
	call	draw_window

still:
	mcall	10
	cmp	eax, 1
	je	redraw
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
	shr	eax, 8

	cmp	eax, 1
	jne	still

  exit:
	cmp	[image_initial], 0
	je	@f
	stdcall	[img.destroy], [image_initial]
    @@:
	cmp	[image_to_rgb2], 0
	je	@f
	stdcall	[img.destroy], [image_to_rgb2]
    @@:
	mcall	-1


draw_window:
	mcall	12, 1
	mcall	0, <100, 388>, <100, 350>, 0x33FFFFFF, , window_title

	mov	eax, [image_initial]
	stdcall	[img.draw], eax, 0, 0, [eax + Image.Width], [eax + Image.Height], 0, 0

	mov	eax, [image_to_rgb2]
	stdcall	[img.draw], eax, 0, [eax + Image.Height], [eax + Image.Width], [eax + Image.Height], 0, 0

	mcall	12, 2
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

window_title	db 'img.to_rgb2 test app',0

input_file	db '/hd0/1/in_32bpp.png',0
;-----------------------------------------------------------------------------

align 16
@IMPORT:

library 			\
	libio  , 'libio.obj'  , \
	libimg , 'libimg.obj'

import	libio			  , \
	libio.init , 'lib_init'   , \
	file.size  , 'file_size'  , \
	file.open  , 'file_open'  , \
	file.read  , 'file_read'  , \
	file.close , 'file_close'

import	libimg			   , \
	libimg.init , 'lib_init'   , \
	img.draw    , 'img_draw'   , \
	img.decode  , 'img_decode' , \
	img.create  , 'img_create' , \
	img.destroy , 'img_destroy', \
	img.to_rgb2 , 'img_to_rgb2'

;-----------------------------------------------------------------------------

I_END:

img_data	dd ?
img_data_len	dd ?
fh		dd ?

image_initial	dd ?
image_to_rgb2	dd ?
