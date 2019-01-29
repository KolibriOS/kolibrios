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

;;; Returns 0 on success. -1 on failure.
proc init_boxlib
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp

;; Wrapper to handle edit_box_key function for editboxes.
;; Call this baby from C (refer kolibri_editbox.h for details)
;public editbox_key_thunk as '_editbox_key@4'   ; renamed due to ambiguity
;public press_key as '_press_key'
;; replaced by siemargl as inline ASM in C wrapper
;editbox_key_thunk:
;	mov [oldebp], ebp	;Save ebp because GCC is crazy for it otherwise.
;	pop ebp			;Save return address in ebp. Stack top is param now.
;	mov eax, dword [press_key]
;	call [edit_box_key]	; The pointer we passed should be on the stack already.
;	push ebp		;push the return address back to stack
;	mov ebp, [oldebp]
;	ret
;oldebp dd ?
;press_key dd ?



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
	fb_draw_panel, 'FileBrowser_draw' , \
	fb_mouse, 'FileBrowser_mouse' , \
	fb_key, 'FileBrowser_key' , \
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
	path_show_prepare, 'PathShow_prepare' , \
	path_show_draw, 'PathShow_draw' , \
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
	ted_but_find, 'ted_but_find' , \
	ted_but_replace, 'ted_but_replace' , \
	ted_text_colored, 'ted_text_colored' , \
	ted_go_to_position, 'ted_go_to_position' , \
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

public dynamic_button_draw as '_dynamic_button_draw'
public dynamic_button_mouse as '_dynamic_button_mouse'

public path_show_prepare as '_path_show_prepare'
public path_show_draw as '_path_show_draw'

public fb_draw_panel as '_filebrowse_draw'
public fb_mouse as '_filebrowse_mouse'
public fb_key as '_filebrowse_key'

public ted_but_sumb_upper as '_ted_but_sumb_upper'
public ted_but_sumb_lover as '_ted_but_sumb_lover'
public ted_but_convert_by_table as '_ted_but_convert_by_table'
public ted_can_save as '_ted_can_save'
public ted_clear as '_ted_clear'
public ted_delete as '_ted_delete'
public ted_draw as '_ted_draw'
public ted_init as '_ted_init'
public ted_init_scroll_bars as '_ted_init_scroll_bars'
public ted_init_syntax_file as '_ted_init_syntax_file'
public ted_is_select as '_ted_is_select'
public ted_key as '_ted_key_asm'
public ted_mouse as '_ted_mouse'
public ted_open_file as '_ted_open_file_asm'
public ted_save_file as '_ted_save_file_asm'
public ted_text_add as '_ted_text_add'
public ted_but_select_word as '_ted_but_select_word'
public ted_but_cut as '_ted_but_cut'
public ted_but_copy as '_ted_but_copy'
public ted_but_paste as '_ted_but_paste'
public ted_but_undo as '_ted_but_undo'
public ted_but_redo as '_ted_but_redo'
public ted_but_reverse as '_ted_but_reverse'
public ted_but_find as '_ted_but_find'
public ted_but_replace as '_ted_but_replace'
public ted_text_colored as 'ted_text_colored_asm'
public ted_go_to_position as '_ted_go_to_position'

public tl_data_init as '_tl_data_init'
public tl_data_clear as '_tl_data_clear'
public tl_info_clear as '_tl_info_clear'
public tl_key as '_tl_key_asm'
public tl_mouse as '_tl_mouse'
public tl_draw as '_tl_draw'
public tl_info_undo as '_tl_info_undo'
public tl_info_redo as '_tl_info_redo'
public tl_node_add as '_tl_node_add'
public tl_node_set_data as '_tl_node_set_data'
public tl_node_get_data as '_tl_node_get_data'
public tl_node_delete as '_tl_node_delete'
public tl_cur_beg as '_tl_cur_beg'
public tl_cur_next as '_tl_cur_next'
public tl_cur_perv as '_tl_cur_perv'
public tl_node_close_open as '_tl_node_close_open'
public tl_node_lev_inc as '_tl_node_lev_inc'
public tl_node_lev_dec as '_tl_node_lev_dec'
public tl_node_move_up as '_tl_node_move_up'
public tl_node_move_down as '_tl_node_move_down'
public tl_node_poi_get_info as '_tl_node_poi_get_info'
public tl_node_poi_get_next_info as '_tl_node_poi_get_next_info'
public tl_node_poi_get_data as '_tl_node_poi_get_data'
public tl_save_mem as '_tl_save_mem_asm'
public tl_load_mem as '_tl_load_mem_asm'
public tl_get_mem_size as '_tl_get_mem_size_asm'
