use32
org 0x0
	db 'MENUET01'
	dd 0x01, START, I_END, E_END, E_END, 0, 0

;-----------------------------------------------------------------------------

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../dll.inc'
;include '../../../../../debug.inc'

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

;pushfd
;pushad
;mov ebx, [image_initial]
;debug_print_dec dword[ebx + Image.Type]
;newline
;popad
;popfd
	stdcall	mem.Free, [img_data]
	test	eax, eax
	jz	exit

	invoke	img.convert, [image_initial], 0, Image.bpp8g, 0, 0
	test	eax, eax
	jz	exit
	mov	[image_converted], eax

	invoke	img.destroy, [image_initial]

	mov	ebx, [image_converted]
	mov	eax, [ebx + Image.Width]
	add	eax, 200*0x10000 + 5*2 - 1	; window x position + 10 pixels width for skin borders
	mov	[window_width], eax

	mcall	48, 4	; get skin height
	mov	ebx, [image_converted]
	add	eax, [ebx + Image.Height]
	add	eax, 100*0x10000 + 5 - 1	; window y position + 5 pixels height for skin bottom border
	mov	[window_height], eax
;-----------------------------------------------------------------------------

still:
	mcall	10
	cmp	eax, 1
	je	.draw_window
	cmp	eax, 2
	je	.key
	cmp	eax, 3
	je	.button
	jmp	still


  .draw_window:
	mcall	12, 1
	mcall	0, [window_width], [window_height], 0x74FFFFFF, 0x00000000, window_title
	call	draw_image
	mcall	12, 2
	jmp	still

  .key:
	mcall	2
	jmp	still

  .button:
	mcall	17
	shr	eax, 8
	cmp	eax, 1
	jne	still

exit:
	invoke	img.destroy, [image_converted]
	mcall	-1


proc	draw_image

	mov	ebx, [image_converted]
	invoke	img.draw, ebx, 0, 0, [ebx + Image.Width], [ebx + Image.Height], 0, 0

	ret
endp

;-----------------------------------------------------------------------------

window_title	db 'img.convert example',0
input_file:
	db '/hd0/1/kolibri_logo.jpg',0
;	db '/hd0/1/tga/CTC16.TGA',0
;	db '/hd0/1/indexed_packbits_le_test_00.tiff',0
;	db '/hd0/1/graya_123x123.tiff',0
;	db '/hd0/1/bilevel_00.wbmp',0
;	db '/hd0/1/rgb_af.jpg',0
;	db '/hd0/1/gray_5x7.tiff',0
;	db '/hd0/1/rgb_lzw_le_2x2.tiff',0
;	db '/hd0/1/grayscale_123x123.tiff',0
;	db '/hd0/1/grayscale_357x357.tiff',0
;	db '/hd0/1/grayscale_620x620.tiff',0
;	db '/hd0/1/rgb_220.jpg',0
;	db '/hd0/1/rgba_217.tiff',0
;	db '/hd0/1/rgb_7x9.tiff',0
;	db '/hd0/1/rgba_7x9.tiff',0
;	db '/hd0/1/gray_7x9.tiff',0
;	db '/hd0/1/rgb_70x90.png',0
;-----------------------------------------------------------------------------

align 4
@IMPORT:

library				  \
	libio	, 'libio.obj'	, \
	libimg	, 'libimg.obj'

import	libio				, \
	libio.init	, 'lib_init'	, \
	file.size	, 'file_size'	, \
	file.open	, 'file_open'	, \
	file.read	, 'file_read'	, \
	file.close	, 'file_close'

import	libimg				, \
	libimg.init	, 'lib_init'	, \
	img.decode	, 'img_decode'	, \
	img.destroy	, 'img_destroy'	, \
	img.draw	, 'img_draw'	, \
	img.convert	, 'img_convert'	, \
	img.types_table , 'img_types_table'

;-----------------------------------------------------------------------------

I_END:

fh		dd ?
img_data_len	dd ?
img_data	dd ?

image_initial	dd ?
image_converted	dd ?

window_width	dd ?
window_height	dd ?

rd 0x1000	; stack
E_END:
