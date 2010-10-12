; Copyright (C) KolibriOS team 2004-2010. All rights reserved.
; Refer to the GNU General Public license (the "GPL") for full details.
;
;*****************************************************************************
; Box_Lib - library of graphical components
;
; Authors:
; Alexey Teplov aka <Lrz>
; Marat Zakiyanov aka Mario79, aka Mario
; Evtikhov Maxim aka Maxxxx32
; Eugene Grechnikov aka Diamond
; hidnplayr
; Igor Afanasiev aka IgorA
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16
include '../../../../macros.inc'
include '../../../../proc32.inc'
include 'bl_sys.mac'
include 'box_lib.mac' ;macro which should make life easier :)

;-----------------------------------------------------------------------------
mem.alloc   dd ? ;функция для выделения памяти
mem.free    dd ? ;функция для освобождения памяти
mem.realloc dd ? ;функция для перераспределения памяти
dll.load    dd ?

;----------------------------------------------------
;EditBox
;----------------------------------------------------
align 16
;use_editbox_draw	;macro reveals the function of the display.
align 16
;use_editbox_key 	;macro reveals processing function of the keypad.
align 16
;use_editbox_mouse	;macro reveals processing function of the mouse.

;----------------------------------------------------
;CheckBox
;----------------------------------------------------
align 16
_init_checkbox		;macro for init checkbox
align 16
use_checkbox_draw	;macro reveals the function of the display.
align 16
use_checkbox_mouse	;macro reveals processing function of the mouse.


;--------------------------------------------------
;radiobutton Group
;--------------------------------------------------
align 16
;use_optionbox_driver	;macro that control the operating modes
align 16
;use_optionbox_draw	;macro reveals the function of the display.
align 16
;use_optionbox_mouse	;macro reveals processing function of the mouse.

;--------------------------------------------------
;scrollbar Group
;--------------------------------------------------
align 16
;use_scroll_bar
align 16
;use_scroll_bar_vertical
align 16
;use_scroll_bar_horizontal

;--------------------------------------------------
;dinamic button Group
;--------------------------------------------------
align 16
;use_dinamic_button

;--------------------------------------------------
;menubar Group
;--------------------------------------------------
align 16
;use_menu_bar

;--------------------------------------------------
;filebrowser Group
;--------------------------------------------------
align 16
;use_file_browser

;--------------------------------------------------
;tree list
;--------------------------------------------------
align 16
;use_tree_list

;--------------------------------------------------
;PathShow Group
;--------------------------------------------------
align 16
;use_path_show

;--------------------------------------------------
;text editor
;--------------------------------------------------
align 16
;use_text_edit

;input:
; eax = указатель на функцию выделения памяти
; ebx = ... освобождения памяти
; ecx = ... перераспределения памяти
; edx = ... загрузки библиотеки (пока не используется)
align 16
lib_init:
	mov	[mem.alloc], eax
	mov	[mem.free], ebx
	mov	[mem.realloc], ecx
	mov	[dll.load], edx
ret


align 16
EXPORTS:


dd	sz_init,			lib_init
dd	sz_version,			0x00000001

;dd	sz_edit_box,			edit_box
;dd	sz_edit_box_key,		edit_box_key
;dd	sz_edit_box_mouse,		edit_box_mouse
;dd	sz_edit_box_set_text,		edit_box_set_text
;dd	szVersion_ed,			0x00000001

dd	sz_init_checkbox,		init_checkbox
dd	sz_check_box_draw,		check_box_draw
dd	sz_check_box_mouse,		check_box_mouse
dd	szVersion_ch,			0x00000002

;dd	sz_option_box_draw,		option_box_draw
;dd	sz_option_box_mouse,		option_box_mouse
;dd	szVersion_op,			0x00000001

;dd	sz_Scrollbar_ver_draw,		scroll_bar_vertical.draw
;dd	sz_Scrollbar_ver_mouse, 	scroll_bar_vertical.mouse
;dd	sz_Scrollbar_hor_draw,		scroll_bar_horizontal.draw
;dd	sz_Scrollbar_hor_mouse, 	scroll_bar_horizontal.mouse
;dd	szVersion_scrollbar,		0x00010001

;dd	sz_Dbutton_draw,		dinamic_button.draw
;dd	sz_Dbutton_mouse,		dinamic_button.mouse
;dd	szVersion_dbutton,		0x00010001

;dd	sz_Menu_bar_draw,		menu_bar.draw
;dd	sz_Menu_bar_mouse,		menu_bar.mouse
;dd	sz_Menu_bar_activate,		menu_bar.activate
;dd	szVersion_menu_bar,		0x00010002

;dd	sz_FileBrowser_draw,		fb_draw_panel
;dd	sz_FileBrowser_mouse,		fb_mouse
;dd	sz_FileBrowser_key,		fb_key
;dd	szVersion_FileBrowser,		0x00010001

;dd	sz_tl_data_init,		tl_data_init
;dd	sz_tl_data_clear,		tl_data_clear
;dd	sz_tl_info_clear,		tl_info_clear
;dd	sz_tl_key,			tl_key
;dd	sz_tl_mouse,			tl_mouse
;dd	sz_tl_draw,			tl_draw
;dd	sz_tl_info_undo,		tl_info_undo
;dd	sz_tl_info_redo,		tl_info_redo
;dd	sz_tl_node_add, 		tl_node_add
;dd	sz_tl_node_set_data,		tl_node_set_data
;dd	sz_tl_node_get_data,		tl_node_get_data
;dd	sz_tl_node_delete,		tl_node_delete
;dd	sz_tl_cur_beg,			tl_cur_beg
;dd	sz_tl_cur_next, 		tl_cur_next
;dd	sz_tl_cur_perv, 		tl_cur_perv
;dd	sz_tl_node_close_open,		tl_node_close_open
;dd	sz_tl_node_lev_inc,		tl_node_lev_inc
;dd	sz_tl_node_lev_dec,		tl_node_lev_dec
;dd	sz_tl_node_move_up,		tl_node_move_up
;dd	sz_tl_node_move_down,		tl_node_move_down
;dd	sz_tl_node_poi_get_info,	tl_node_poi_get_info
;dd	sz_tl_node_poi_get_next_info,	tl_node_poi_get_next_info
;dd	sz_tl_node_poi_get_data,	tl_node_poi_get_data
;dd	sz_tl_save_mem, 		tl_save_mem
;dd	sz_tl_load_mem, 		tl_load_mem
;dd	sz_tl_get_mem_size,		tl_get_mem_size
;dd	sz_tl_version_tree_list,	0x00000001

;dd	sz_PathShow_prepare,		path_show.prepare
;dd	sz_PathShow_draw,		path_show.draw
;dd	szVersion_path_show,		0x00010001

;dd	sz_ted_but_save_file,		ted_but_save_file
;dd	sz_ted_but_sumb_upper,		ted_but_sumb_upper
;dd	sz_ted_but_sumb_lover,		ted_but_sumb_lover
;dd	sz_ted_can_save,		ted_can_save
;dd	sz_ted_clear,			ted_clear
;dd	sz_ted_delete,			ted_delete
;dd	sz_ted_draw,			ted_draw
;dd	sz_ted_init,			ted_init
;dd	sz_ted_init_scroll_bars,	ted_init_scroll_bars
;dd	sz_ted_init_syntax_file,	ted_init_syntax_file
;dd	sz_ted_is_select,		ted_is_select
;dd	sz_ted_key,			ted_key
;dd	sz_ted_mouse,			ted_mouse
;dd	sz_ted_open_file,		ted_open_file
;dd	sz_ted_text_add,		ted_text_add
;dd	sz_ted_but_select_word, 	ted_but_select_word
;dd	sz_ted_but_cut, 		ted_but_cut
;dd	sz_ted_but_copy,		ted_but_copy
;dd	sz_ted_but_paste,		ted_but_paste
;dd	sz_ted_but_undo,		ted_but_undo
;dd	sz_ted_but_redo,		ted_but_redo
;dd	sz_ted_but_reverse,		ted_but_reverse
;dd	sz_ted_but_find_next,		ted_but_find_next
;dd	sz_ted_text_colored,		ted_text_colored
;dd	sz_ted_version, 		0x00000002

dd	0,0


sz_init 			db 'lib_init',0
sz_version			db 'version',0

sz_edit_box			db 'edit_box',0
sz_edit_box_key 		db 'edit_box_key',0
sz_edit_box_mouse		db 'edit_box_mouse',0
sz_edit_box_set_text		db 'edit_box_set_text',0
szVersion_ed			db 'version_ed',0

sz_init_checkbox		db 'init_checkbox',0
sz_check_box_draw		db 'check_box_draw',0
sz_check_box_mouse		db 'check_box_mouse',0
szVersion_ch			db 'version_ch',0

sz_option_box_draw		db 'option_box_draw',0
sz_option_box_mouse		db 'option_box_mouse',0
szVersion_op			db 'version_op',0

sz_Scrollbar_ver_draw		db 'scrollbar_v_draw',0
sz_Scrollbar_ver_mouse		db 'scrollbar_v_mouse',0
sz_Scrollbar_hor_draw		db 'scrollbar_h_draw',0
sz_Scrollbar_hor_mouse		db 'scrollbar_h_mouse',0
szVersion_scrollbar		db 'version_scrollbar',0

sz_Dbutton_draw 		db 'dbutton_draw',0
sz_Dbutton_mouse		db 'dbutton_mouse',0
szVersion_dbutton		db 'version_dbutton',0

sz_Menu_bar_draw		db 'menu_bar_draw',0
sz_Menu_bar_mouse		db 'menu_bar_mouse',0
sz_Menu_bar_activate		db 'menu_bar_activate',0
szVersion_menu_bar		db 'version_menu_bar',0

sz_FileBrowser_draw		db 'FileBrowser_draw',0
sz_FileBrowser_mouse		db 'FileBrowser_mouse',0
sz_FileBrowser_key		db 'FileBrowser_key',0
szVersion_FileBrowser		db 'version_FileBrowser',0

sz_tl_data_init 		db 'tl_data_init',0
sz_tl_data_clear		db 'tl_data_clear',0
sz_tl_info_clear		db 'tl_info_clear',0
sz_tl_key			db 'tl_key',0
sz_tl_mouse			db 'tl_mouse',0
sz_tl_draw			db 'tl_draw',0
sz_tl_info_undo 		db 'tl_info_undo',0
sz_tl_info_redo 		db 'tl_info_redo',0
sz_tl_node_add			db 'tl_node_add',0
sz_tl_node_set_data		db 'tl_node_set_data',0
sz_tl_node_get_data		db 'tl_node_get_data',0
sz_tl_node_delete		db 'tl_node_delete',0
sz_tl_cur_beg			db 'tl_cur_beg',0
sz_tl_cur_next			db 'tl_cur_next',0
sz_tl_cur_perv			db 'tl_cur_perv',0
sz_tl_node_close_open		db 'tl_node_close_open',0
sz_tl_node_lev_inc		db 'tl_node_lev_inc',0
sz_tl_node_lev_dec		db 'tl_node_lev_dec',0
sz_tl_node_move_up		db 'tl_node_move_up',0
sz_tl_node_move_down		db 'tl_node_move_down',0
sz_tl_node_poi_get_info 	db 'tl_node_poi_get_info',0
sz_tl_node_poi_get_next_info	db 'tl_node_poi_get_next_info',0
sz_tl_node_poi_get_data 	db 'tl_node_poi_get_data',0
sz_tl_save_mem			db 'tl_save_mem',0
sz_tl_load_mem			db 'tl_load_mem',0
sz_tl_get_mem_size		db 'tl_get_mem_size',0
sz_tl_version_tree_list 	db 'version_tree_list',0

sz_PathShow_prepare		db 'PathShow_prepare',0
sz_PathShow_draw		db 'PathShow_draw',0
szVersion_path_show		db 'version_PathShow',0

sz_ted_but_save_file		db 'ted_but_save_file',0
sz_ted_but_sumb_upper		db 'ted_but_sumb_upper',0
sz_ted_but_sumb_lover		db 'ted_but_sumb_lover',0
sz_ted_can_save 		db 'ted_can_save',0
sz_ted_clear			db 'ted_clear',0
sz_ted_delete			db 'ted_delete',0
sz_ted_draw			db 'ted_draw',0
sz_ted_init			db 'ted_init',0
sz_ted_init_scroll_bars 	db 'ted_init_scroll_bars',0
sz_ted_init_syntax_file 	db 'ted_init_syntax_file',0
sz_ted_is_select		db 'ted_is_select',0
sz_ted_key			db 'ted_key',0
sz_ted_mouse			db 'ted_mouse',0
sz_ted_open_file		db 'ted_open_file',0
sz_ted_text_add 		db 'ted_text_add',0
sz_ted_but_select_word		db 'ted_but_select_word',0
sz_ted_but_cut			db 'ted_but_cut',0
sz_ted_but_copy 		db 'ted_but_copy',0
sz_ted_but_paste		db 'ted_but_paste',0
sz_ted_but_undo 		db 'ted_but_undo',0
sz_ted_but_redo 		db 'ted_but_redo',0
sz_ted_but_reverse		db 'ted_but_reverse',0
sz_ted_but_find_next		db 'ted_but_find_next',0
sz_ted_text_colored		db 'ted_text_colored',0
sz_ted_version			db 'version_text_edit',0