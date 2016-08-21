format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../programs/system/run/trunk/txtbut.inc'
include '../../../programs/dll.inc'
	
public init_boxlib as '_kolibri_boxlib_init'
public editbox_key_thunk as '_editbox_key@4'   ; renamed due to ambiguity
public press_key as '_press_key'
;;; Returns 0 on success. -1 on failure.

proc init_boxlib
	mcall 68,11
	stdcall dll.Load, @IMPORT
	ret
endp	
	
;; Wrapper to handle edit_box_key function for editboxes.
;; Call this baby from C (refer kolibri_editbox.h for details)
editbox_key_thunk:
	mov [oldebp], ebp	;Save ebp because GCC is crazy for it otherwise.
	pop ebp			;Save return address in ebp. Stack top is param now.
	mov eax, dword [press_key]
	call [edit_box_key]	; The pointer we passed should be on the stack already.
	push ebp		;push the return address back to stack
	mov ebp, [oldebp]
	ret
	
oldebp dd ?
press_key dd ?

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
	scrollbar_v_draw, 'scrollbar_v_draw' , \
	scrollbar_v_mouse, 'scrollbar_v_mouse' , \
	scrollbar_h_draw, 'scrollbar_h_draw' , \
	scrollbar_h_mouse, 'scrollbar_h_mouse' , \
	dynamic_button_draw, 'dbutton_draw' , \
	dynamic_button_mouse, 'dbutton_mouse' , \
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
public edit_box_set_text as '_edit_box_set_text' 

public check_box_draw2 as '_check_box_draw2'
public check_box_mouse2 as '_check_box_mouse2'
public init_checkbox2 as '_init_checkbox2'

public progressbar_draw as '_progressbar_draw'
public progressbar_progress as '_progressbar_progress'

public frame_draw as '_frame_draw'

public scrollbar_v_draw as '_scrollbar_v_draw'
public scrollbar_v_mouse as '_scrollbar_v_mouse'
public scrollbar_h_draw as '_scrollbar_h_draw'
public scrollbar_h_mouse as '_scrollbar_h_mouse'

public option_box_draw as '_option_box_draw'
public option_box_mouse as '_option_box_mouse'

public menu_bar_draw as '_menu_bar_draw' 
public menu_bar_mouse as '_menu_bar_mouse' 
public menu_bar_activate as '_menu_bar_activate' 
