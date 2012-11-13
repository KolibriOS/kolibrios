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

	stdcall	mem.Free, [img_data]
	test	eax, eax
	jz	exit
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
	mcall	0, <200, 150>, <200, 150>, 0x73FFFFFF, 0x00000000, window_title
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
	cmp	[image_initial], 0
	je	@f
	invoke	img.destroy, [image_initial]
    @@:
	cmp	[image_scaled], 0
	je	@f
	invoke	img.destroy, [image_scaled]
    @@:
	mcall	-1


proc	draw_image

	cmp	[image_scaled], 0
	je	@f
	invoke	img.destroy, [image_scaled]
    @@:

	mcall	9, proc_info, -1

	mov	ecx, [proc_info.client_box.height]
	inc	ecx
	mov	edx, [proc_info.client_box.width]
	inc	edx

	mov	ebx, [image_initial]
;	invoke	img.scale, ebx, 1, 2, 5, 5, 0, LIBIMG_SCALE_TYPE_STRETCH, LIBIMG_SCALE_ALG_BILINEAR, edx, ecx
;	invoke	img.scale, ebx, 1, 2, 5, 5, 0, LIBIMG_SCALE_TYPE_STRETCH, LIBIMG_SCALE_ALG_INTEGER, 3, 3
	invoke	img.scale, ebx, 0, 0, [ebx + Image.Width], [ebx + Image.Height], 0, LIBIMG_SCALE_TYPE_STRETCH, LIBIMG_SCALE_ALG_BILINEAR, edx, ecx

; proc img.scale _src, _crop_x, _crop_y, _crop_width, _crop_height, _dst, _scale_type, _scale_alg, _param1, _param2
; see libimg.inc for available scale types and algorithms
; LIBIMG_SCALE_ALG_BILINEAR: _param1, _param2 -- width and height of rectangle to fit _src image to
; LIBIMG_SCALE_ALG_INTEGER:  _param1 -- scale factor (i.e. 3 means scaling 7x7 to 21x21); _param2 ignored
; LIBIMG_SCALE_TYPE_*: just try and see, they are common STRETCH, FIT_BY_WIDTH etc.
; returns pointer to a scaled image

;	invoke	img.scale, ebx, 0, 0, [ebx + Image.Width], [ebx + Image.Height], 0, LIBIMG_SCALE_TYPE_STRETCH, LIBIMG_SCALE_ALG_INTEGER, 3, 3
	test	eax, eax
	jz	exit
	mov	[image_scaled], eax

	invoke	img.draw, eax, 0, 0, [eax + Image.Width], [eax + Image.Height], 0, 0

	ret
endp

;-----------------------------------------------------------------------------

window_title	db 'img.scale example',0

input_file	db '/hd0/1/gray_5x7.tiff',0
;input_file	db '/hd0/1/grayscale_123x123.tiff',0
;input_file	db '/hd0/1/grayscale_357x357.tiff',0
;input_file	db '/hd0/1/grayscale_620x620.tiff',0
;input_file	db '/hd0/1/rgb_220.jpg',0
;input_file	db '/hd0/1/rgba_217.tiff',0
;input_file	db '/hd0/1/rgb_7x9.tiff',0
;input_file	db '/hd0/1/rgba_7x9.tiff',0
;input_file	db '/hd0/1/gray_7x9.tiff',0
;input_file	db '/hd0/1/rgb_70x90.png',0
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
	img.scale	, 'img_scale'	, \
	img.formats_table,'img_formats_table'

;-----------------------------------------------------------------------------

I_END:

fh		dd ?
img_data_len	dd ?
img_data	dd ?

image_initial	dd ?
image_scaled	dd ?

proc_info	process_information

rd 0x1000	; stack
E_END:
