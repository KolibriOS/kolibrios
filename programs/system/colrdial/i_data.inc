;---------------------------------------------------------------------
title:
	db 'ColorDialog',0
;---------------------------------------------------------------------
align 4
window_x:
.x_size		dw 420
.x_start	dw 10
window_y:
.y_size		dw 320
.y_start	dw 10
;---------------------------------------------------------------------
;plugins_directory	db 'plugins/',0
plugins_directory	db 0

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0

ihead_f_i:
ihead_f_l	db 'System	error',0

er_message_found_lib	db 'box_lib.obj - Not found!',0
er_message_import	db 'box_lib.obj - Wrong import!',0

align	4
l_libs_start:
library01	l_libs	system_dir_Boxlib+9,path,file_name,system_dir_Boxlib,\
er_message_found_lib,ihead_f_l,Box_lib_import,er_message_import,ihead_f_i,plugins_directory

end_l_libs:
;---------------------------------------------------------------------
;---------------------------------------------------------------------
align	4
Box_lib_import:	
;init_lib	dd a_init
;version_lib	dd a_version


;edit_box_draw		dd aEdit_box_draw
;edit_box_key		dd aEdit_box_key
;edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

;check_box_draw	dd aCheck_box_draw
;check_box_mouse	dd aCheck_box_mouse
;version_ch		dd aVersion_ch

;option_box_draw	dd aOption_box_draw
;option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

scrollbar_ver_draw	dd aScrollbar_ver_draw
scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;dinamic_button_draw	dd aDbutton_draw
;dinamic_button_mouse	dd aDbutton_mouse
;version_dbutton	dd aVersion_dbutton

;menu_bar_draw		dd aMenu_bar_draw
;menu_bar_mouse		dd aMenu_bar_mouse
;menu_bar_activate	dd aMenu_bar_activate
;version_menu_bar	dd aVersion_menu_bar

;FileBrowser_draw	dd aFileBrowser_draw
;FileBrowser_mouse	dd aFileBrowser_mouse
;FileBrowser_key		dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

;PathShow_prepare	dd sz_PathShow_prepare
;PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show

Frame_draw		dd sz_Frame_draw
;Version_frame		dd szVersion_frame

	dd 0
	dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

;aEdit_box_draw		db 'edit_box',0
;aEdit_box_key		db 'edit_box_key',0
;aEdit_box_mouse		db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

;aOption_box_draw	db 'option_box_draw',0
;aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

aScrollbar_ver_draw	db 'scrollbar_v_draw',0
aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aDbutton_draw		db 'dbutton_draw',0
;aDbutton_mouse		db 'dbutton_mouse',0
;aVersion_dbutton	db 'version_dbutton',0

;aMenu_bar_draw		db 'menu_bar_draw',0
;aMenu_bar_mouse		db 'menu_bar_mouse',0
;aMenu_bar_activate	db 'menu_bar_activate',0
;aVersion_menu_bar	db 'version_menu_bar',0

;aFileBrowser_draw	db 'FileBrowser_draw',0
;aFileBrowser_mouse	db 'FileBrowser_mouse',0
;aFileBrowser_key	db 'FileBrowser_key',0
;aVersion_FileBrowser	db 'version_FileBrowser',0

;sz_PathShow_prepare	db 'PathShow_prepare',0
;sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0

sz_Frame_draw		db 'frame_draw',0
;szVersion_frame	db 'version_frame',0
;---------------------------------------------------------------------
align 4
scroll_bar_data_red:
.x:
.size_x     dw 15 ;+0
.start_x    dw t_start_x + p_size_y + 20
.y:
.size_y     dw p_size_y + p_start_y - (c_start_y + c_size_y + 10)
.start_y    dw c_start_y + c_size_y + 10
.btn_high   dd 15 ;+8
.type       dd 2  ;+12
.max_area   dd 255  ;+16
.cur_area   dd 0  ;+20
.position   dd 128  ;+24
.bckg_col   dd 0xFFDBDB ;+28
.frnt_col   dd 0xFF7777   ;0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta      dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos      dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 1 ;+80
.ar_offset  dd 10 ;+84
;---------------------------------------------------------------------
align 4
scroll_bar_data_green:
.x:
.size_x     dw 15 ;+0
.start_x    dw t_start_x + p_size_y + 45
.y:
.size_y     dw p_size_y + p_start_y - (c_start_y + c_size_y + 10)
.start_y    dw c_start_y + c_size_y + 10
.btn_high   dd 15 ;+8
.type       dd 2  ;+12
.max_area   dd 255  ;+16
.cur_area   dd 0  ;+20
.position   dd 128  ;+24
.bckg_col   dd 0xDBFFDB ;+28
.frnt_col   dd 0x77FF77   ;0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta      dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos      dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 1 ;+80
.ar_offset  dd 10 ;+84
;---------------------------------------------------------------------
align 4
scroll_bar_data_blue:
.x:
.size_x     dw 15 ;+0
.start_x    dw t_start_x + p_size_y + 70
.y:
.size_y     dw p_size_y + p_start_y - (c_start_y + c_size_y + 10)
.start_y    dw c_start_y + c_size_y + 10
.btn_high   dd 15 ;+8
.type       dd 2  ;+12
.max_area   dd 255  ;+16
.cur_area   dd 0  ;+20
.position   dd 128  ;+24
.bckg_col   dd 0xDBDBFF ;+28
.frnt_col   dd 0x7777FF   ;0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta      dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos      dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 1 ;+80
.ar_offset  dd 10 ;+84
;---------------------------------------------------------------------
frame_data: 
.type			dd 0 ;+0
.x:
.x_size			dw 0 ;+4
.x_start		dw 0 ;+6
.y:
.y_size			dw 0 ;+8
.y_start		dw 0 ;+10
.ext_fr_col		dd 0x0 ;+12
.int_fr_col		dd 0xffffff ;+16
.draw_text_flag		dd 1 ;+20
.text_pointer		dd 0 ;+24
.text_position		dd 0 ;+28
.font_number		dd 0 ;+32
.font_size_y		dd 9 ;+36
.font_color		dd 0x0 ;+40
.font_backgr_color	dd 0xffffff ;+44
;---------------------------------------------------------------------
