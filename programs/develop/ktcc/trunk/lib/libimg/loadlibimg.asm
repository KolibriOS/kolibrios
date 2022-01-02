format elf
use32                                   ; Tell compiler to use 32 bit instructions

; ELF section
section '.text' executable


include '../../../../../proc32.inc'
include '../../../../../macros.inc'
purge section,mov,add,sub
	
include '../../../../../dll.inc'

	
public init_libimg as 'kolibri_libimg_init'
;;; Returns 0 on success. -1 on failure.

proc init_libimg
local retval dd ?
	mov [retval], eax
	pusha
	mcall 68, 11
	test eax, eax
	jnz @f
		mov [retval], -1
		jmp exit_init_libimg
@@:	
	stdcall dll.Load, @IMPORT
	test eax, eax
	jz	exit_init_libimg
		mov [retval], -1
exit_init_libimg:	
	popa
	mov eax, [retval]
	ret
endp	

; ELF section
section '.data' writeable

@IMPORT:
library lib_libimg, 	'libimg.obj'

import lib_libimg, \
	libimg_init, 'lib_init' , \
	img_is_img, 'img_is_img' , \
	img_info, 'img_info' , \
	img_from_file, 'img_from_file', \
	img_to_file, 'img_to_file', \
	img_from_rgb, 'img_from_rgb', \
	img_to_rgb, 'img_to_rgb', \
	img_to_rgb2, 'img_to_rgb2', \
	img_decode, 'img_decode', \
	img_encode, 'img_encode', \
	img_create, 'img_create', \
	img_destroy, 'img_destroy', \
	img_destroy_layer, 'img_destroy_layer', \
	img_count, 'img_count', \
	img_lock_bits, 'img_lock_bits', \
	img_unlock_bits, 'img_unlock_bits', \
	img_flip, 'img_flip', \
	img_flip_layer, 'img_flip_layer', \
	img_rotate, 'img_rotate', \
	img_rotate_layer, 'img_rotate_layer', \
	img_draw, 'img_draw', \ 
	img_blend, 'img_blend', \
	img_convert, 'img_convert', \
    img_resize_data, 'img_resize_data', \
    img_scale, 'img_scale'
	
public libimg_init as  'libimg_init'
public img_to_rgb as  'img_to_rgb'
public img_to_rgb2 as  'img_to_rgb2'
public img_decode as  'img_decode'
public img_encode as  'img_encode'
public img_create as  'img_create'
public img_destroy as  'img_destroy'
public img_destroy_layer as  'img_destroy_layer'
public img_count as  'img_count'
public img_flip as  'img_flip'
public img_flip_layer as  'img_flip_layer'
public img_rotate as  'img_rotate'
public img_rotate_layer as  'img_rotate_layer'
public img_draw as  'img_draw'
public img_blend as 'img_blend'
public img_convert as 'img_convert'
public img_resize_data as 'img_resize_data'
public img_scale as 'img_scale'
