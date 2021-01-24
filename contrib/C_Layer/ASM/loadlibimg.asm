
format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/dll.inc'
	
public init_libimg as '_kolibri_libimg_init'
;;; Returns 0 on success. -1 on failure.

proc init_libimg
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	

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
	
public libimg_init as  '_libimg_init'
public img_to_rgb as  '_img_to_rgb'
public img_to_rgb2 as  '_img_to_rgb2'
public img_decode as  '_img_decode'
public img_encode as  '_img_encode'
public img_create as  '_img_create'
public img_destroy as  '_img_destroy'
public img_destroy_layer as  '_img_destroy_layer'
public img_count as  '_img_count'
public img_flip as  '_img_flip'
public img_flip_layer as  '_img_flip_layer'
public img_rotate as  '_img_rotate'
public img_rotate_layer as  '_img_rotate_layer'
public img_draw as  '_img_draw'
public img_blend as '_img_blend'
public img_convert as '_img_convert'
public img_resize_data as '_img_resize_data'
public img_scale as '_img_scale'
