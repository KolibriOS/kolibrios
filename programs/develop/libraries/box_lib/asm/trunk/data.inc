;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
header_1 db 'Box_lib Control Demo by Mario79',0

ok_btn db 'Ok',0
        
labelt3 db 'About program',0
;---------------------------------------------------------------------
icons_file_name: db 'reload_16x16_8b.png',0
system_dir1      db '/sys/lib/'
plugin_BMP_name db 'cnv_bmp.obj',0

system_dir      db '/sys/lib/'

;plugins_directory               db 'plugins/',0
plugins_directory               db 0

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_CnvPNG	db '/sys/lib/cnv_png.obj',0
system_dir_UNPACK	db '/sys/lib/archiver.obj',0
; Если есть желание разъединить, то нужно использовать следующию конструкцию
;system_path      db '/sys/lib/box_lib.obj',0
;... любая последовательность других команд и определений.
;library_name     db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ihead_f_i:
ihead_f_l        db 'System error',0

er_message_found_lib   db 'Sorry I cannot load library box_lib.obj',0
er_message_import      db 'Error on load import library box_lib.obj',0

er_message_found_lib2	db 'cnv_png.obj - Not found!',0
er_message_import2	db 'cnv_png.obj - Wrong import!',0

err_message_found_lib3	db 'archiver.obj - Not found!',0
err_message_import3	db 'archiver.obj - Wrong import!',0

align 4
l_libs_start:
library01  l_libs system_dir_Boxlib+9, path, file_name, system_dir_Boxlib, \
er_message_found_lib, ihead_f_l, Box_lib_import, er_message_import, ihead_f_i, plugins_directory

library02  l_libs system_dir_CnvPNG+9, path, file_name, system_dir_CnvPNG,\
er_message_found_lib2, ihead_f_l, cnv_png_import, er_message_import2, ihead_f_i, plugins_directory

library03  l_libs system_dir_UNPACK+9, path, file_name, system_dir_UNPACK, \
err_message_found_lib3, ihead_f_l, UNPACK_import, err_message_import3, ihead_f_i, plugins_directory

end_l_libs:
;---------------------------------------------------------------------
align 4
cnv_png_import:
.Start			dd aCP_Start
.Version		dd aCP_Version
.Check			dd aCP_Check
.Assoc			dd aCP_Assoc
        dd      0
        dd      0
aCP_Start		db 'START',0
aCP_Version		db 'version',0
aCP_Check		db 'Check_Header',0
aCP_Assoc		db 'Associations',0
;---------------------------------------------------------------------
align 4
UNPACK_import:
;unpack_Version				dd aUnpack_Version
;unpack_PluginLoad			dd aUnpack_PluginLoad 
;unpack_OpenFilePlugin		dd aUnpack_OpenFilePlugin
;unpack_ClosePlugin			dd aUnpack_ClosePlugin
;unpack_ReadFolder			dd aUnpack_ReadFolder 
;unpack_SetFolder			dd aUnpack_SetFolder
;unpack_GetFiles				dd aUnpack_GetFiles
;unpack_GetOpenPluginInfo	dd aUnpack_GetOpenPluginInfo
;unpack_Getattr				dd aUnpack_Getattr
;unpack_Open					dd aUnpack_Open
;unpack_Read					dd aUnpack_Read
;unpack_Setpos				dd aUnpack_Setpos
;unpack_Close				dd aUnpack_Close
unpack_DeflateUnpack		dd aUnpack_DeflateUnpack
unpack_DeflateUnpack2		dd aUnpack_DeflateUnpack2
        dd      0
        dd      0

;aUnpack_Version        db      'version',0
;aUnpack_PluginLoad     db      'plugin_load',0
;aUnpack_OpenFilePlugin db      'OpenFilePlugin',0
;aUnpack_ClosePlugin    db      'ClosePlugin',0
;aUnpack_ReadFolder     db      'ReadFolder',0
;aUnpack_SetFolder      db      'SetFolder',0
;aUnpack_GetFiles       db      'GetFiles',0
;aUnpack_GetOpenPluginInfo db   'GetOpenPluginInfo',0
;aUnpack_Getattr        db      'getattr',0
;aUnpack_Open           db      'open',0
;aUnpack_Read           db      'read',0
;aUnpack_Setpos         db      'setpos',0
;aUnpack_Close          db      'close',0
aUnpack_DeflateUnpack  db      'deflate_unpack',0
aUnpack_DeflateUnpack2 db      'deflate_unpack2',0

;---------------------------------------------------------------------

fileinfo:
     dd 5
     dd 0
     dd 0
     dd 0
     dd file_info
string:
    db 0
        dd      file_name
        
plugin       dd 0
;---------------------------------------------------------------------
; not change this section!!!
; start section
;---------------------------------------------------------------------
image_file     dd 0
raw_pointer    dd 0
return_code    dd 0
img_size       dd 0
deflate_unpack dd 0
;---------------------------------------------------------------------
; end section
;---------------------------------------------------------------------

;---------------------------------------------------------------------
align 4
Box_lib_import:   
;init_lib            dd      a_init
;version_lib         dd      a_version


;edit_box_draw   dd      aEdit_box_draw
;edit_box_key    dd      aEdit_box_key
;edit_box_mouse  dd      aEdit_box_mouse
;version_ed      dd      aVersion_ed

;check_box_draw  dd      aCheck_box_draw
;check_box_mouse dd      aCheck_box_mouse
;version_ch      dd      aVersion_ch

;option_box_draw  dd      aOption_box_draw
;option_box_mouse dd      aOption_box_mouse
;version_op       dd      aVersion_op

scrollbar_ver_draw              dd aScrollbar_ver_draw
scrollbar_ver_mouse             dd aScrollbar_ver_mouse
scrollbar_hor_draw              dd aScrollbar_hor_draw
scrollbar_hor_mouse             dd aScrollbar_hor_mouse
;version_scrollbar               dd aVersion_scrollbar

dinamic_button_draw             dd      aDbutton_draw
dinamic_button_mouse            dd      aDbutton_mouse
;version_dbutton                 dd      aVersion_dbutton

menu_bar_draw                   dd      aMenu_bar_draw
menu_bar_mouse                  dd      aMenu_bar_mouse
menu_bar_activate               dd      aMenu_bar_activate
;version_menu_bar                dd      aVersion_menu_bar

;FileBrowser_draw                dd      aFileBrowser_draw
;FileBrowser_mouse               dd      aFileBrowser_mouse
;FileBrowser_key                 dd      aFileBrowser_key
;Version_FileBrowser             dd      aVersion_FileBrowser
                dd      0
                dd      0

;a_init          db 'lib_init',0
;a_version       db 'version',0

;aEdit_box_draw  db 'edit_box',0
;aEdit_box_key   db 'edit_box_key',0
;aEdit_box_mouse db 'edit_box_mouse',0
;aVersion_ed     db 'version_ed',0

;aCheck_box_draw  db 'check_box_draw',0
;aCheck_box_mouse db 'check_box_mouse',0
;aVersion_ch      db 'version_ch',0

;aOption_box_draw  db 'option_box_draw',0
;aOption_box_mouse db 'option_box_mouse',0
;aVersion_op       db 'version_op',0

aScrollbar_ver_draw             db 'scrollbar_v_draw',0
aScrollbar_ver_mouse            db 'scrollbar_v_mouse',0
aScrollbar_hor_draw             db 'scrollbar_h_draw',0
aScrollbar_hor_mouse            db 'scrollbar_h_mouse',0
;aVersion_scrollbar              db 'version_scrollbar',0

aDbutton_draw                   db 'dbutton_draw',0
aDbutton_mouse                  db 'dbutton_mouse',0
;aVersion_dbutton                db 'version_dbutton',0

aMenu_bar_draw                  db 'menu_bar_draw',0
aMenu_bar_mouse                 db 'menu_bar_mouse',0
aMenu_bar_activate              db 'menu_bar_activate',0
;aVersion_menu_bar               db 'version_menu_bar',0

;aFileBrowser_draw             db 'FileBrowser_draw',0
;aFileBrowser_mouse            db 'FileBrowser_mouse',0
;aFileBrowser_key              db 'FileBrowser_key',0
;aVersion_FileBrowser          db 'version_FileBrowser',0
;---------------------------------------------------------------------

;---------------------------------------------------------------------   
align 4
scroll_bar_data_vertical:
.x:
.size_x     dw 15 ;+0
.start_x    dw 370 ;+2
.y:
.size_y     dw 300 ;+4
.start_y    dw 50 ;+6
.btn_high   dd 15 ;+8
.type       dd 1  ;+12
.max_area   dd 300  ;+16
.cur_area   dd 30  ;+20
.position   dd 140  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
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
.all_redraw dd 0 ;+80
.ar_offset      dd 10 ;+84
;---------------------------------------------------------------------
align 4
scroll_bar_data_horizontal:
.x:
.size_x     dw 300  ;0 ;+0
.start_x    dw 30   ;0 ;+2
.y:
.size_y     dw 15   ;0 ;+4
.start_y    dw 370    ;0 ;+6
.btn_high   dd 15 ;+8
.type       dd 1  ;+12
.max_area   dd 300  ;+16
.cur_area   dd 30  ;+20
.position   dd 140  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
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
.all_redraw dd 0 ;+80
.ar_offset      dd 10 ;+84
;---------------------------------------------------------------------
align 4
menu_data_1:
.type:                  dd 0   ;+0
.x:
.size_x                 dw 40  ;+4
.start_x                dw 20   ;+6
.y:
.size_y                 dw 15   ;+8
.start_y                dw 25  ;+10
.text_pointer:  dd menu_text_area  ;0 ;+12
.pos_pointer:   dd menu_text_area.1 ;0 ;+16
.text_end               dd menu_text_area.end ;0 ;+20
.mouse_pos              dd 0  ;+24
.mouse_keys             dd 0  ;+28
.x1:
.size_x1                dw 80  ;+32
.start_x1               dw 20   ;+34
.y1:
.size_y1                dw 100   ;+36
.start_y1               dw 40  ;+38
.bckg_col       dd 0xeeeeee ;+40
.frnt_col       dd 0xff ;+44
.menu_col       dd 0xffffff ;+48
.select         dd 0 ;+52
.out_select     dd 0 ;+56
.buf_adress             dd 0 ;+60
.procinfo               dd procinfo ;+64
.click                  dd 0 ;+68
.cursor                 dd 0 ;+72
.cursor_old             dd 0 ;+76
.interval               dd 16 ;+80
.cursor_max             dd 0 ;+84
.extended_key   dd 0 ;+88
.menu_sel_col   dd 0x00cc00 ;+92
.bckg_text_col  dd 0 ; +96
.frnt_text_col  dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height    dd 8 ;+108
.cursor_out             dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;-------------------------
menu_text_area:
db 'File',0
.1:
db 'OpenDialog',0
db 'Test1',0
db 'Exit',0
.end:
db 0
;---------------------------------------------------------------------
align 4
menu_data_2:
.type:                  dd 0   ;+0
.x:
.size_x                 dw 40  ;+4
.start_x                dw 60   ;+6
.y:
.size_y                 dw 15   ;+8
.start_y                dw 25  ;+10
.text_pointer:  dd menu_text_area_2  ;0 ;+12
.pos_pointer:   dd menu_text_area_2.1 ;0 ;+16
.text_end               dd menu_text_area_2.end ;0 ;+20
.mouse_pos              dd 0  ;+24
.mouse_keys             dd 0  ;+28
.x1:
.size_x1                dw 50  ;+32
.start_x1               dw 60   ;+34
.y1:
.size_y1                dw 100   ;+36
.start_y1               dw 40  ;+38
.bckg_col       dd 0xeeeeee ;+40
.frnt_col       dd 0xff ;+44
.menu_col       dd 0xffffff ;+48
.select         dd 0 ;+52
.out_select     dd 0 ;+56
.buf_adress             dd 0 ;+60
.procinfo               dd procinfo ;+64
.click                  dd 0 ;+68
.cursor                 dd 0 ;+72
.cursor_old             dd 0 ;+76
.interval               dd 16 ;+80
.cursor_max             dd 0 ;+84
.extended_key   dd 0 ;+88
.menu_sel_col   dd 0x00cc00 ;+92
.bckg_text_col  dd 0 ; +96
.frnt_text_col  dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height    dd 8 ;+108
.cursor_out             dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;------------------------
menu_text_area_2:
db 'Help',0
.1:
db 'Test1',0
db 'About',0
.end:
db 0
;---------------------------------------------------------------------
align 4
dinamic_button_data_1:
.type                   dd 0 ;+0
.x:
.size_x                 dw 20 ;+4
.start_x                dw 110 ;+6
.y:
.size_y                 dw 20 ;+8
.start_y                dw 25 ;+10
.mouse_pos              dd 0 ;+12
.mouse_keys             dd 0 ;+16
.mouse_keys_old dd 0 ;+20
.active_raw             dd 0 ;+24
.passive_raw    dd 0 ;+28
.click_raw              dd 0 ;+32
.resolution_raw dd 0 ;+36
.palette_raw    dd 0 ;+40
.offset_raw             dd 0 ;+44
.select                 dd 0 ;+48
.click                  dd 0 ;+52
;---------------------------------------------------------------------
align 4
dinamic_button_data_2:
.type                   dd 0 ;+0
.x:
.size_x                 dw 20 ;+4
.start_x                dw 140 ;+6
.y:
.size_y                 dw 20 ;+8
.start_y                dw 25 ;+10
.mouse_pos              dd 0 ;+12
.mouse_keys             dd 0 ;+16
.mouse_keys_old dd 0 ;+20
.active_raw             dd 0 ;+24
.passive_raw    dd 0 ;+28
.click_raw              dd 0 ;+32
.resolution_raw dd 0 ;+36
.palette_raw    dd 0 ;+40
.offset_raw             dd 0 ;+44
.select                 dd 0 ;+48
.click                  dd 0 ;+52
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo ;+4
.com_area_name		dd communication_area_name ;+8
.com_area		dd 0 ;+12
.opendir_pach		dd plugin_pach ;+16
.dir_default_pach	dd communication_area_default_pach ;+20
.start_path		dd library_path ;+24
.draw_window		dd draw_window ;+28
.status			dd 0 ;+32
.openfile_pach		dd openfile_pach ;+36

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_pach:
	db '/rd/1',0

Filter:
dd  Filter.end - Filter
db 'JPEG',0
db 'JPG',0
db 'JPE',0
db 'PNG',0
db 'GIF',0
db 'BMP',0
db 'KEX',0
.end:
db 0
;---------------------------------------------------------------------