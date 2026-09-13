 ; Language support for locales: ru_RU (CP866), et_EE, en_US.

;-----------------------------------------------------------------------------
text:
        db ' Unused1                        '
        db ' Unused2                        '
if lang eq ru_RU
	db ' 3D темная сторона              '
	db ' 3D светлая сторона             '
	db ' Заголовок окна                 '
	db ' Фон окна                       '
	db ' Кнопка                         '
	db ' Текст на кнопке                '
	db ' Текст в окне                   '
	db ' Графика в окне                 '
	db 'x'
else if lang eq et_EE
	db ' 3D dark                        '
	db ' 3D light                       '
	db ' Akna pealkiri                  '
	db ' Akna tЎЎpiirkond               '
	db ' Akna tЎЎpiirkonna nupp         '
	db ' Akna tЎЎpiirkonna nuppu tekst  '
	db ' Akna tЎЎpiirkonna tekst        '
	db ' Akna tЎЎpiirkonna graafika     '
	db 'x'
else ; Default to en_US
	db ' 3D dark                        '
	db ' 3D light                       '
	db ' Window title                   '
	db ' Window background              '
	db ' Button                         '
	db ' Button text                    '
	db ' Window text                    '
	db ' Window graph                   '
	db 'x'
end if
;-----------------------------------------------------------------------------
if lang eq ru_RU
	sz t1,'ЗАГРУЗИТЬ  СОХРАНИТЬ                                       ПРИМЕНИТЬ'
else if lang eq et_EE
	sz t1,'  LAADI     SALVESTA                                        KINNITA'
else ; Default to en_US
	sz t1,'  LOAD       SAVE                                            APPLY'
end if
;-----------------------------------------------------------------------------
if lang eq ru_RU
	sz t2,'ЗАГРУЗИТЬ                3D   ПЛОСКИЕ                      ПРИМЕНИТЬ'
else if lang eq et_EE
	sz t2,'  LAADI                  3D    LAME                         KINNITA'
else ; Default to en_US
	sz t2,'  LOAD                   3D    FLAT                          APPLY'
end if
;-----------------------------------------------------------------------------
select_dtp_text:
if lang eq ru_RU
	db ' Выбор цветового оформления: ',0
else if lang eq et_EE
	db ' Valik varviskeeme: ',0
else ; Default to en_US
	db ' Choice of color schemes: ',0
end if
;-----------------------------------------------------------------------------
select_skin_text:
if lang eq ru_RU
	db ' Выбор скина: ',0
else if lang eq et_EE
	db ' Valik nahad: ',0
else ; Default to en_US
	db ' Choice of skins: ',0
end if
;-----------------------------------------------------------------------------
if lang eq ru_RU
	sz caption_text,'Заголовок'
else if lang eq et_EE
	sz caption_text,'Pealkiri'
else ; Default to en_US
	sz caption_text,'Caption'
end if
;-----------------------------------------------------------------------------
sz  close_text,'x'
;-----------------------------------------------------------------------------
if lang eq ru_RU
	sz window_text,'Текст в окне'
else if lang eq et_EE
	sz window_text,'Akna tekst'
else ; Default to en_US
	sz window_text,'Window text'
end if
;-----------------------------------------------------------------------------
if lang eq ru_RU
	sz button_text,'Текст на кнопке'
else if lang eq et_EE
	sz button_text,'Nupu tekst'
else ; Default to en_US
	sz button_text,'Button text'
end if
;-----------------------------------------------------------------------------
if lang eq ru_RU
  title db 'Настройка окон',0
else if lang eq et_EE
  title db 'Akna seaded',0
else if lang eq it_IT
  title db 'Impostazioni',0
else ; Default to en_US
  title db 'Windows settings',0
end if
;---------------------------------------------------------------------
LibIniImportTable:
library                                                 \
        libini, 'libini.obj'

import  libini, \
        ini_set_int  ,'ini_set_int'

aIni          db  '/sys/settings/system.ini',0
aSectionSkn   db  'style',0
aButtonStyle  db  'buttons_gradient',0
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, library_path, system_dir_Boxlib, \
 import_box_lib

library02  l_libs system_dir_ProcLib+9, library_path, system_dir_ProcLib, \
 ProcLib_import

end_l_libs:
;---------------------------------------------------------------------
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0

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
include '../../develop/libraries/box_lib/import.inc'
;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw frame_1.y+19	;+4
.start_x		dw frame_1.x+15	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw frame_1.w-28	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd dtp_name	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
PathShow_data_2:
.type			dd 0	;+0
.start_y		dw frame_2.y+19	;+4
.start_x		dw frame_2.x+15	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw frame_2.w-28	;+10
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
.openfile_pach 		dd dtp_name	;+36
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
	db '/sys/colrdial',0
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
.font_number		dd 1 ;+32
.font_size_y		dd 14 ;+36
.font_color		dd 0x0 ;+40
.font_backgr_color	dd 0xffffff ;+44
;---------------------------------------------------------------------
IncludeIGlobals
