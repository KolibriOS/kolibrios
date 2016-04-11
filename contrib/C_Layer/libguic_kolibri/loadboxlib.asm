format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include 'proc32.inc'
include 'macros.inc'
purge section,mov,add,sub
	
include 'box_lib.mac'
include 'txtbut.inc'
include 'dll.inc'
	
public init_boxlib as '_init_boxlib_asm'
public editbox_key as '_editbox_key@4'
	
;;; Returns 0 on success. -1 on failure.

proc init_boxlib
	
	mcall 68,11
	
	stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     error
	
	mov eax, 0
	ret
	
error:	
	mov eax, -1
	ret
endp	
	
;; Wrapper to handle edit_box_key function for editboxes.
;; Call this baby from C (refer kolibri_editbox.h for details)
editbox_key:
	mov [oldebp], ebp	;Save ebp because GCC is crazy for it otherwise.
	pop ebp			;Save return address in ebp. Stack top is param now.
	mcall 2
	call [edit_box_key]	; The pointer we passed should be on the stack already.
	push ebp		;push the return address back to stack
	mov ebp, [oldebp]
	ret
	
oldebp dd ?

@IMPORT:
library lib_boxlib, 	'box_lib.obj'

import lib_boxlib, \
	edit_box_draw, 'edit_box' , \
	edit_box_key, 'edit_box_key' , \
	edit_box_mouse, 'edit_box_mouse', \
	edit_box_set_text, 'edit_box_set_text' , \
	init_checkbox2,  'init_checkbox2' , \
	check_box_draw2, 'check_box_draw2' , \
	check_box_mouse2, 'check_box_mouse2' , \
	option_box_draw,  'option_box_draw' , \
	option_box_mouse, 'option_box_mouse' , \
	scroll_bar_vertical_draw, 'scrollbar_ver_draw' , \
	scroll_bar_vertical_mouse, 'scrollbar_ver_mouse' , \
	scroll_bar_horizontal_draw, 'scrollbar_hor_draw' , \
	scroll_bar_horizontal_mouse, 'scrollbar_hor_mouse' , \
	dinamic_button_draw, 'dbutton_draw' , \
	dinamic_button_mouse, 'dbutton_mouse' , \
	menu_bar_draw, 'menu_bar_draw' , \
	menu_bar_mouse, 'menu_bar_mouse' , \
	menu_bar_activate, 'menu_bar_activate' , \
	fb_draw_panel, 'filebrowser_draw' , \
	fb_mouse, 'filebrowser_mouse' , \
	fb_key, 'filebrowser_key' , \
	tl_data_init, 'tl_data_init' , \
	tl_data_clear, 'tl_data_clear' , \
	tl_info_clear, 'tl_info_clear' , \
	tl_key, 'tl_key' , \
	tl_mouse, 'tl_mouse' , \
	tl_draw, 'tl_draw' , \
	tl_info_undo, 'tl_info_undo' , \
	tl_info_redo, 'tl_info_redo' , \
	tl_node_add, 'tl_node_add' , \
	tl_node_set_data, 'tl_node_set_data' , \
	tl_node_get_data, 'tl_node_get_data' , \
	tl_node_delete, 'tl_node_delete' , \
	tl_cur_beg, 'tl_cur_beg' , \
	tl_cur_next, 'tl_cur_next' , \
	tl_cur_perv, 'tl_cur_perv' , \
	tl_node_close_open, 'tl_node_close_open' , \
	tl_node_lev_inc, 'tl_node_lev_inc' , \
	tl_node_lev_dec, 'tl_node_lev_dec' , \
	tl_node_move_up, 'tl_node_move_up' , \
	tl_node_move_down, 'tl_node_move_down' , \
	tl_node_poi_get_info, 'tl_node_poi_get_info' , \
	tl_node_poi_get_next_info, 'tl_node_poi_get_next_info' , \
	tl_node_poi_get_data, 'tl_node_poi_get_data' , \
	tl_save_mem, 'tl_save_mem' , \
	tl_load_mem, 'tl_load_mem' , \
	tl_get_mem_size, 'tl_get_mem_size' , \
	path_show_prepare, 'pathshow_prepare' , \
	path_show_draw, 'pathshow_draw' , \
	ted_but_sumb_upper, 'ted_but_sumb_upper' , \
	ted_but_sumb_lover, 'ted_but_sumb_lover' , \
	ted_but_convert_by_table, 'ted_but_convert_by_table' , \
	ted_can_save, 'ted_can_save' , \
	ted_clear, 'ted_clear' , \
	ted_delete, 'ted_delete' , \
	ted_draw, 'ted_draw' , \
	ted_init, 'ted_init' , \
	ted_init_scroll_bars, 'ted_init_scroll_bars' , \
	ted_init_syntax_file, 'ted_init_syntax_file' , \
	ted_is_select, 'ted_is_select' , \
	ted_key, 'ted_key' , \
	ted_mouse, 'ted_mouse' , \
	ted_open_file, 'ted_open_file' , \
	ted_save_file, 'ted_save_file' , \
	ted_text_add, 'ted_text_add' , \
	ted_but_select_word, 'ted_but_select_word' , \
	ted_but_cut, 'ted_but_cut' , \
	ted_but_copy, 'ted_but_copy' , \
	ted_but_paste, 'ted_but_paste' , \
	ted_but_undo, 'ted_but_undo' , \
	ted_but_redo, 'ted_but_redo' , \
	ted_but_reverse, 'ted_but_reverse' , \
	ted_but_find_next, 'ted_but_find_next' , \
	ted_text_colored, 'ted_text_colored' , \
	frame_draw, 'frame_draw' , \
	progressbar_draw,'progressbar_draw' , \
	progressbar_progress, 'progressbar_progress'
	
public edit_box_draw as '_edit_box_draw'
public edit_box_key as '_edit_box_key'
public edit_box_mouse as '_edit_box_mouse'

public check_box_draw2 as '_check_box_draw2'
public check_box_mouse2 as '_check_box_mouse2'
