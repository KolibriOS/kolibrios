use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, 0x1000, 0x1000, 0, 0

;-----------------------------------------------------------------------------

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../dll.inc'

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

	stdcall	[img.encode], [image_to_rgb2], (LIBIMG_FORMAT_PNM), 0
;	stdcall	[img.encode], [image_initial], (LIBIMG_FORMAT_PNM), 0
	test	eax, eax
	jz	exit
	mov	[encoded_file], eax
	mov	[encoded_file_size], ecx

	invoke	file.open, output_file, O_WRITE OR O_CREATE
	or	eax, eax
	jz	exit
	mov	[fh], eax

	invoke	file.write, [fh], [encoded_file], [encoded_file_size]
	cmp	eax, [encoded_file_size]
	jne	exit

	invoke	file.close, [fh]
	inc	eax
	jz	exit

	stdcall	mem.Free, [encoded_file]
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

window_title	db 'libimg to_rgb2 & encode demo',0

;input_file	db '/hd0/1/in_1bpp.wbmp',0
;input_file	db '/hd0/1/in_8bpp.tiff',0
input_file	db '/hd0/1/in_32bpp.png',0

;output_file	db '/hd0/1/out_1bpp.pnm',0
;output_file	db '/hd0/1/out_8bpp.pnm',0
output_file	db '/hd0/1/out_24bpp.pnm',0
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
	file.write , 'file_write' , \
	file.close , 'file_close'

import	libimg			   , \
	libimg.init , 'lib_init'   , \
	img.draw    , 'img_draw'   , \
	img.decode  , 'img_decode' , \
	img.encode  , 'img_encode' , \
	img.create  , 'img_create' , \
	img.destroy , 'img_destroy', \
	img.to_rgb2 , 'img_to_rgb2', \
	img.formats_table, 'img_formats_table'

;-----------------------------------------------------------------------------

I_END:

img_data	  dd ?
img_data_len	  dd ?
fh		  dd ?

image_initial	  dd ?
image_to_rgb2	  dd ?

encoded_file	  dd ?
encoded_file_size dd ?
