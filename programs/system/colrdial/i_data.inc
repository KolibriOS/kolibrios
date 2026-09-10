;---------------------------------------------------------------------
title:
	db 'ColorDialog',0
	
history_text:
	db ' History: ',0

communication_area	dd param
;---------------------------------------------------------------------
align 4
window_x:
.x_size		dw 510
.x_start	dw 10
window_y:
.y_size		dw 300
.y_start	dw 10
;---------------------------------------------------------------------
;plugins_directory	db 'plugins/',0
plugins_directory	db 0

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0

align	4
l_libs_start:
library01	l_libs	system_dir_Boxlib+9,file_name,system_dir_Boxlib,\
import_box_lib,plugins_directory

end_l_libs:
;---------------------------------------------------------------------
include '../../develop/libraries/box_lib/import.inc'
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
.font_backgr_color	dd 0xEEEeee ;+44
;---------------------------------------------------------------------
mouse_dd dd edit1
edit1 edit_box ed_size_x, c_start_x+c_size_x+5, c_start_y, 0xffffff, 0xa0a0a0, 0xff, 0, 0x10000000, 6, string1, mouse_dd, 0
sz_0x db '______',0,0
string1 rb 8
;---------------------------------------------------------------------
OK_Cancel db 'OK              Cancel',0