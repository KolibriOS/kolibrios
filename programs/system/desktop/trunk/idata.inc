;-----------------------------------------------------------------------------
text:
if lang eq ru
	db ' €ŒŠ€ Š€                     '
	db ' ‹‘€ ‡€ƒ‹‚Š€               '
	db ' ŠŠ€ € ‹‘… ‡€ƒ‹‚Š€     '
	db ' ’…Š‘’ € ŠŠ… € ‡€ƒ‹‚Š…   '
	db ' ’…Š‘’ ‡€ƒ‹‚Š€                '
	db ' €—€Ÿ ‹€‘’œ                '
	db ' ŠŠ€ ‚ €—…‰ ‹€‘’ˆ       '
	db ' ’…Š‘’ € ŠŠ…                '
	db ' ’…Š‘’ ‚ €—…‰ ‹€‘’ˆ        '
	db ' ƒ€”ˆŠ€ ‚ €—…‰ ‹€‘’ˆ      '
	db 'x'
else if lang eq et
	db ' AKNA RAAM                      '
	db ' AKNA HAARAMISE RIBA            '
	db ' AKNA HAARAMISE NUPP            '
	db ' AKNA HAARAMISE NUPU TEKST      '
	db ' AKNA HAARAMISE PEALKIRI        '
	db ' AKNA TÖÖPIIRKOND               '
	db ' AKNA TÖÖPIIRKONNA NUPP         '
	db ' AKNA TÖÖPIIRKONNA NUPPU TEKST  '
	db ' AKNA TÖÖPIIRKONNA TEKST        '
	db ' AKNA TÖÖPIIRKONNA GRAAFIKA     '
	db 'x'
else
	db ' WINDOW FRAME                   '
	db ' WINDOW GRAB BAR                '
	db ' WINDOW GRAB BUTTON             '
	db ' WINDOW GRAB BUTTON TEXT        '
	db ' WINDOW GRAB TITLE              '
	db ' WINDOW WORK AREA               '
	db ' WINDOW WORK AREA BUTTON        '
	db ' WINDOW WORK AREA BUTTON TEXT   '
	db ' WINDOW WORK AREA TEXT          '
	db ' WINDOW WORK AREA GRAPH         '
	db 'x'
end if
;-----------------------------------------------------------------------------
if lang eq ru
	sz t1,'‡€ƒ“‡ˆ’œ  ‘•€ˆ’œ                                   ˆŒ…ˆ’œ'
else if lang eq et
	sz t1,'  LAADI     SALVESTA                                    KINNITA'
else
	sz t1,'  LOAD        SAVE                                       APPLY'
end if
;-----------------------------------------------------------------------------
if lang eq ru
	sz t2,'‡€ƒ“‡ˆ’œ                3D   ‹‘Šˆ…                  ˆŒ…ˆ’œ'
else if lang eq et
	sz t2,'  LAADI                  3D    LAME                     KINNITA'
else
	sz t2,'  LOAD                   3D    FLAT                      APPLY'
end if
;-----------------------------------------------------------------------------
select_dtp_text:
if lang eq ru
	db ' ‚› –‚…’‚ƒ ”Œ‹…ˆŸ: ',0
else if lang eq et
	db ' VALIK VARVISKEEME: ',0
else
	db ' CHOICE OF COLOR SCHEMES: ',0
end if
;-----------------------------------------------------------------------------
select_skin_text:
if lang eq ru
	db ' ‚› ˜Š“Šˆ: ',0
else if lang eq et
	db ' VALIK NAHAD: ',0
else
	db ' CHOICE OF SKINS: ',0
end if
;-----------------------------------------------------------------------------
if lang eq ru
	sz caption_text,'‡ £®«®¢®ª'
else if lang eq et
	sz caption_text,'Pealkiri'
else
	sz caption_text,'Caption'
end if
;-----------------------------------------------------------------------------
sz  close_text,'x'
;-----------------------------------------------------------------------------
if lang eq ru
	sz window_text,'’¥ªáâ ¢ ®ª­¥'
else if lang eq et
	sz window_text,'Akna tekst'
else
	sz window_text,'Window text'
end if
;-----------------------------------------------------------------------------
if lang eq ru
	sz button_text,'’¥ªáâ ­  ª­®¯ª¥'
else if lang eq et
	sz button_text,'Nupu tekst'
else
	sz button_text,'Button text'
end if
;-----------------------------------------------------------------------------
if lang eq ru
  title db '€‘’‰Š€ Š',0
else if lang eq et
  title db 'AKNA SEADED - VALI VÄRV JA VAJUTA OBJEKTILE',0
else if lang eq it
  title db 'Impostazioni - definire il colore e cliccare lo oggetto',0
else
  title db 'WINDOWS SETTINGS - DEFINE COLOR AND CLICK ON TARGET',0
end if
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0

head_f_i:
head_f_l	db 'System error',0

err_message_found_lib1	db 'box_lib.obj - Not found!',0
err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import1	db 'box_lib.obj - Wrong import!',0
err_message_import2	db 'proc_lib.obj - Wrong import!',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start

ColorDialog_Init	dd aColorDialog_Init
ColorDialog_Start	dd aColorDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0

aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0

aColorDialog_Init	db 'ColorDialog_init',0
aColorDialog_Start	db 'ColorDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
Box_lib_import:
;init_lib		dd a_init
;version_lib		dd a_version


;edit_box_draw		dd aEdit_box_draw
;edit_box_key		dd aEdit_box_key
;edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

;check_box_draw		dd aCheck_box_draw
;check_box_mouse	dd aCheck_box_mouse
;version_ch		dd aVersion_ch

;option_box_draw	dd aOption_box_draw
;option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

;scrollbar_ver_draw	dd aScrollbar_ver_draw
;scrollbar_ver_mouse	dd aScrollbar_ver_mouse
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
;FileBrowser_key	dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show

Frame_draw		dd sz_Frame_draw
;Version_frame		dd szVersion_frame

			dd 0
			dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

;aEdit_box_draw		db 'edit_box',0
;aEdit_box_key		db 'edit_box_key',0
;aEdit_box_mouse	db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

;aOption_box_draw	db 'option_box_draw',0
;aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

;aScrollbar_ver_draw	db 'scrollbar_v_draw',0
;aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
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

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0

sz_Frame_draw		db 'frame_draw',0
;szVersion_frame	db 'version_frame',0
;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw frame_1.y+10	;+4
.start_x		dw frame_1.x+7	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw frame_1.width-20	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
PathShow_data_2:
.type			dd 0	;+0
.start_y		dw frame_2.y+10	;+4
.start_x		dw frame_2.x+7	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw frame_2.width-20	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd skin_info	;+28
.work_area_pointer	dd text_work_area2	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

OpenDialog_data2:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name2	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach2	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd skin_info	;+36
.filename_area		dd filename_area2	;+40
.filter_area		dd Filter2
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name2:
	db 'FFFFFFFF_open_dialog',0

communication_area_name:
	db 'FFFFFFFF_open_dialog2',0
open_dialog_path:
if __nightbuild eq yes
	db '/sys/MANAGERS/opendial',0
else
	db '/sys/File Managers/opendial',0
end if
communication_area_default_pach:
	db '/sys',0

Filter:
	dd Filter.end - Filter
.1:
	db 'DTP',0
.end:
	db 0

Filter2:
	dd Filter.end - Filter
.1:
	db 'SKN',0
.end:
	db 0

start_temp_file_name:
	db 'default.dtp',0

default_skin:
	db '/sys/default.skn',0
default_dtp:
	db '/sys/default.dtp',0
;---------------------------------------------------------------------
ColorDialog_data:
.type			dd 0
.procinfo		dd procinfo ;+4
.com_area_name		dd cd_communication_area_name ;+8
.com_area		dd 0 ;+12
.start_path		dd colordialog_path ;+16
.draw_window		dd draw_window ;+20
.status			dd 0 ;+24
.x:
.x_size			dw 510 ;+28 ; Window X size
.x_start		dw 10 ;+30 ; Window X position
.y:
.y_size			dw 310 ;+32 ; Window y size
.y_start		dw 10 ;+34 ; Window Y position
.color_type		dd 0 ;+36 ; 0- RGB, 1 or other - reserved
.color			dd 0 ;+40 ; Selected color

cd_communication_area_name:
	db 'FFFFFFFF_color_dialog',0
colordialog_path:
	db '/rd/1/colrdial',0
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
IncludeIGlobals
