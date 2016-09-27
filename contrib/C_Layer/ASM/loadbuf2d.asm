
format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/dll.inc'
	
public init_buf2d as '_kolibri_buf2d_init'
;;; Returns 0 on success. -1 on failure.

proc init_buf2d
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	

@IMPORT:
library lib_buf2d, 	'buf2d.obj'

import lib_buf2d, \
	buf2d_create, 'buf2d_create' , \
	buf2d_clear, 'buf2d_clear' , \
	buf2d_draw, 'buf2d_draw' , \
	buf2d_delete, 'buf2d_delete', \
	buf2d_rotate, 'buf2d_rotate', \
	buf2d_resize, 'buf2d_resize', \
	buf2d_line, 'buf2d_line', \
	buf2d_line_sm, 'buf2d_line_sm', \
	buf2d_rect_by_size, 'buf2d_rect_by_size', \
	buf2d_filled_rect_by_size, 'buf2d_filled_rect_by_size', \
	buf2d_circle, 'buf2d_circle', \
	buf2d_img_hdiv2, 'buf2d_img_hdiv2', \
	buf2d_img_wdiv2, 'buf2d_img_wdiv2', \
	buf2d_conv_24_to_8, 'buf2d_conv_24_to_8', \
	buf2d_conv_24_to_32, 'buf2d_conv_24_to_32', \
	buf2d_bit_blt, 'buf2d_bit_blt', \
	buf2d_bit_blt_transp, 'buf2d_bit_blt_transp', \
	buf2d_bit_blt_alpha, 'buf2d_bit_blt_alpha', \
	buf2d_curve_bezier, 'buf2d_curve_bezier', \
	buf2d_convert_text_matrix, 'buf2d_convert_text_matrix', \
	buf2d_draw_text, 'buf2d_draw_text', \
	buf2d_crop_color, 'buf2d_crop_color', \
	buf2d_offset_h, 'buf2d_offset_h', \
	buf2d_flood_fill, 'buf2d_flood_fill', \
	buf2d_set_pixel, 'buf2d_set_pixel', \
	buf2d_get_pixel, 'buf2d_get_pixel', \
	buf2d_flip_h, 'buf2d_flip_h', \
	buf2d_flip_v, 'buf2d_flip_v', \
	buf2d_filter_dither, 'buf2d_filter_dither'
	
public buf2d_create as '_buf2d_create_asm' 
public buf2d_clear as '_buf2d_clear' 
public buf2d_draw as '_buf2d_draw' 
public buf2d_delete as '_buf2d_delete'
public buf2d_rotate as '_buf2d_rotate'
public buf2d_resize as '_buf2d_resize'
public buf2d_line as '_buf2d_line'
public buf2d_line_sm as '_buf2d_line_sm'
public buf2d_rect_by_size as '_buf2d_rect_by_size'
public buf2d_filled_rect_by_size as '_buf2d_filled_rect_by_size'
public buf2d_circle as '_buf2d_circle'
public buf2d_img_hdiv2 as '_buf2d_img_hdiv2'
public buf2d_img_wdiv2 as '_buf2d_img_wdiv2'
public buf2d_conv_24_to_8 as '_buf2d_conv_24_to_8'
public buf2d_conv_24_to_32 as '_buf2d_conv_24_to_32'
public buf2d_bit_blt as '_buf2d_bit_blt'
public buf2d_bit_blt_transp as '_buf2d_bit_blt_transp'
public buf2d_bit_blt_alpha as '_buf2d_bit_blt_alpha'
public buf2d_curve_bezier as '_buf2d_curve_bezier_asm'
public buf2d_convert_text_matrix as '_buf2d_convert_text_matrix'
public buf2d_draw_text as '_buf2d_draw_text'
public buf2d_crop_color as '_buf2d_crop_color'
public buf2d_offset_h as '_buf2d_offset_h'
public buf2d_flood_fill as '_buf2d_flood_fill'
public buf2d_set_pixel as '_buf2d_set_pixel'
public buf2d_get_pixel as '_buf2d_get_pixel'
public buf2d_flip_h as '_buf2d_flip_h'
public buf2d_flip_v as '_buf2d_flip_v'
public buf2d_filter_dither as '_buf2d_filter_dither'
