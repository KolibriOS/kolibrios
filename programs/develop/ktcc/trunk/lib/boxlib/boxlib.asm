; writed by maxcodehack
; adaptation of clayer for ktcc
format elf
use32                                   ; Tell compiler to use 32 bit instructions
; ELF section
section '.text' executable


include '../../../../../proc32.inc'
include '../../../../../macros.inc'
purge section,mov,add,sub
        
include '../../../../../dll.inc'


public init_boxlib as 'kolibri_boxlib_init'

proc init_boxlib
local retval dd ?
        mov [retval], eax
        pusha
        mcall 68, 11
        test eax, eax
        jnz @f
                mov [retval], -1
                jmp exit_init_boxlib
@@:     
        stdcall dll.Load, @IMPORT
        test eax, eax
        jz      exit_init_boxlib
                mov [retval], -1
exit_init_boxlib:       
        popa
        mov eax, [retval]
        ret
endp    

;; Wrapper to handle edit_box_key function for editboxes.
;; Call this baby from C (refer kolibri_editbox.h for details)
public editbox_key_thunk as 'edit_box_key'   ; renamed due to ambiguity
;; replaced by siemargl as inline ASM in C wrapper
editbox_key_thunk:
	mov eax, [esp+8]
	mov [oldebp], ebp	;Save ebp because GCC is crazy for it otherwise.
	pop ebp			;Save return address in ebp. Stack top is param now.
	;mov eax, dword [press_key]
	call [edit_box_key]	; The pointer we passed should be on the stack already.
	push ebp		;push the return address back to stack
	mov ebp, [oldebp]
	ret
oldebp dd ?
       
section '.data' writeable

@IMPORT:
library lib_boxlib,     'box_lib.obj'

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


public edit_box_draw as 'edit_box_draw'
;public edit_box_key as 'edit_box_key'

public edit_box_mouse as 'edit_box_mouse'
public edit_box_set_text as 'edit_box_set_text'

public check_box_draw2 as 'check_box_draw2'
public check_box_mouse2 as 'check_box_mouse2'
public init_checkbox2 as 'init_checkbox2'

public progressbar_draw as 'progressbar_draw'
public progressbar_progress as 'progressbar_progress'

public frame_draw as 'frame_draw'

public scrollbar_v_draw as 'scrollbar_v_draw'
public scrollbar_v_mouse as 'scrollbar_v_mouse'
public scrollbar_h_draw as 'scrollbar_h_draw'
public scrollbar_h_mouse as 'scrollbar_h_mouse'

public option_box_draw as 'option_box_draw'
public option_box_mouse as 'option_box_mouse'

public menu_bar_draw as 'menu_bar_draw'
public menu_bar_mouse as 'menu_bar_mouse'
public menu_bar_activate as 'menu_bar_activate'

public dynamic_button_draw as 'dynamic_button_draw'
public dynamic_button_mouse as 'dynamic_button_mouse'

public path_show_prepare as 'path_show_prepare'
public path_show_draw as 'path_show_draw'

public fb_draw_panel as 'filebrowse_draw'
public fb_mouse as 'filebrowse_mouse'
public fb_key as 'filebrowse_key'

public ted_but_sumb_upper as 'ted_but_sumb_upper'
public ted_but_sumb_lover as 'ted_but_sumb_lover'
public ted_but_convert_by_table as 'ted_but_convert_by_table'
public ted_can_save as 'ted_can_save'
public ted_clear as 'ted_clear'
public ted_delete as 'ted_delete'
public ted_draw as 'ted_draw'
public ted_init as 'ted_init'
public ted_init_scroll_bars as 'ted_init_scroll_bars'
public ted_init_syntax_file as 'ted_init_syntax_file'
public ted_is_select as 'ted_is_select'
public ted_key as 'ted_key_asm'
public ted_mouse as 'ted_mouse'
public ted_open_file as 'ted_open_file_asm'
public ted_save_file as 'ted_save_file_asm'
public ted_text_add as '_ted_text_add'
public ted_but_select_word as 'ted_but_select_word'
public ted_but_cut as 'ted_but_cut'
public ted_but_copy as 'ted_but_copy'
public ted_but_paste as 'ted_but_paste'
public ted_but_undo as 'ted_but_undo'
public ted_but_redo as 'ted_but_redo'
public ted_but_reverse as 'ted_but_reverse'
public ted_but_find as 'ted_but_find'
public ted_but_replace as 'ted_but_replace'
public ted_text_colored as 'ted_text_colored_asm'
public ted_go_to_position as 'ted_go_to_position'

public tl_data_init as 'tl_data_init'
public tl_data_clear as 'tl_data_clear'
public tl_info_clear as 'tl_info_clear'
public tl_key as 'tl_key_asm'
public tl_mouse as 'tl_mouse'
public tl_draw as 'tl_draw'
public tl_info_undo as 'tl_info_undo'
public tl_info_redo as 'tl_info_redo'
public tl_node_add as 'tl_node_add'
public tl_node_set_data as 'tl_node_set_data'
public tl_node_get_data as 'tl_node_get_data'
public tl_node_delete as 'tl_node_delete'
public tl_cur_beg as 'tl_cur_beg'
public tl_cur_next as 'tl_cur_next'
public tl_cur_perv as 'tl_cur_perv'
public tl_node_close_open as 'tl_node_close_open'
public tl_node_lev_inc as 'tl_node_lev_inc'
public tl_node_lev_dec as 'tl_node_lev_dec'
public tl_node_move_up as 'tl_node_move_up'
public tl_node_move_down as 'tl_node_move_down'
public tl_node_poi_get_info as 'tl_node_poi_get_info'
public tl_node_poi_get_next_info as 'tl_node_poi_get_next_info'
public tl_node_poi_get_data as 'tl_node_poi_get_data'
public tl_save_mem as 'tl_save_mem_asm'
public tl_load_mem as 'tl_load_mem_asm'
public tl_get_mem_size as 'tl_get_mem_size_asm'
