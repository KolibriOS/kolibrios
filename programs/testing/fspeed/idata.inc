; Language support for locales: ru_RU (CP866), en_US.

;-----------------------------------------------------------------------------
s_text:
if lang eq ru_RU
	db 'Выбор файла',0
else ; Default to en_US
	db 'Select file',0
end if
;-----------------------------------------------------------------------------
r_text:
if lang eq ru_RU
	db 'Пуск',0
else ; Default to en_US
	db 'Start',0
end if
;-----------------------------------------------------------------------------
result_table_text:
if lang eq ru_RU
	db 'Размер   Чтение (КБ/с)   Запись (КБ/с)',0
else ; Default to en_US
	db 'Size     Read (KB/s)     Write (KB/s)',0
end if
;-----------------------------------------------------------------------------
title	db 'File Speed  v0.3',0
;-----------------------------------------------------------------------------
include_debug_strings
;-----------------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, library_path, system_dir_Boxlib, import_box_lib

library02  l_libs system_dir_ProcLib+9, library_path, system_dir_ProcLib, ProcLib_import

end_l_libs:
;-----------------------------------------------------------------------------
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0

;-----------------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog_Version	dd aOpenDialog_Version

;ColorDialog_Init	dd aColorDialog_Init
;ColorDialog_Start	dd aColorDialog_Start
;ColorDialog_Version	dd aColorDialog_Version

        dd      0
        dd      0

aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0

;aColorDialog_Init	db 'ColorDialog_init',0
;aColorDialog_Start	db 'ColorDialog_start',0
;aColorDialog_Version	db 'Version_ColorDialog',0
;-----------------------------------------------------------------------------
include '../../develop/libraries/box_lib/import.inc'
;-----------------------------------------------------------------------------
PathShow_data:
.type			dd 0	;+0
.start_y		dw 5+4	;+4
.start_x		dw 5+5	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 400-30	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;-----------------------------------------------------------------------------
check1 check_box2 (100 shl 16)+12,(27 shl 16)+12,6,0xFFFFFF,0,0xffffff,\
		check_text1,ch_flag_middle

check_text1:
if lang eq ru_RU
	db 'Тестировать запись',0
else ; Default to en_US
	db 'Use testing of write',0
end if

check_box_warning_text:
if lang eq ru_RU
	db 'ВНИМАНИЕ! Проверка скорости записи уничтожает содержимое файла!',0
else ; Default to en_US
	db 'WARNING! Testing of write speed will destroy file contents!',0
end if
;-----------------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd process_info	;+4
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
dd	Filter.end - Filter.1
.1:
;db	'BIN',0
;db	'DAT',0
.end:
db	0

start_temp_file_name:
	db 'default.dtp',0

default_dtp:
	db '/sys/default.dtp',0
;-----------------------------------------------------------------------------
sector equ 512
;--------------------------------------
result_table:
	dd	a512b,	0,	0,	sector*1
	dd	a1K,	1,	1,	sector*2
	dd	a2K,	2,	2,	sector*4
	dd	a4K,	3,	3,	sector*8
	dd	a8K,	4,	4,	sector*16
	dd	a16K,	5,	5,	sector*32
	dd	a32K,	6,	6,	sector*64
	dd	a64K,	7,	7,	sector*128
	dd	a128K,	8,	8,	sector*256
	dd	a256K,	9,	9,	sector*512
	dd	a512K,	10,	10,	sector*1024
	dd	a1M,	11,	11,	sector*2*1024
	dd	a2M,	12,	12,	sector*4*1024
	dd	a4M,	13,	13,	sector*8*1024
	dd	a8M,	14,	14,	sector*16*1024
	dd	a16M,	15,	15,	sector*32*1024
	dd	a32M,	16,	16,	sector*64*1024
	dd	a64M,	17,	17,	sector*128*1024
;-----------------------------------------------------------------------------
a512b	db ' 512',0
a1K	db '  1K',0
a2K	db '  2K',0
a4K	db '  4K',0
a8K	db '  8K',0
a16K	db ' 16K',0
a32K	db ' 32K',0
a64K	db ' 64K',0
a128K	db '128K',0
a256K	db '256K',0
a512K	db '512K',0
a1M	db '  1M',0
a2M	db '  2M',0
a4M	db '  4M',0
a8M	db '  8M',0
a16M	db ' 16M',0
a32M	db ' 32M',0
a64M	db ' 64M',0
;-----------------------------------------------------------------------------
align 4
fileinfo:
.subfunction	dd 5
.offset		dd 0
.offset_1	dd 0
.size		dd 0
.return		dd file_info
		db 0
.name:		dd fname
;-----------------------------------------------------------------------------
align 4
fileread:
.subfunction	dd 0
.offset		dd 0
.offset_1	dd 0
.size		dd 0
.return		dd process_info
		db 0
.name:		dd fname
;-----------------------------------------------------------------------------
align 4
filewrite:
.subfunction	dd 3
.offset		dd 0
.offset_1	dd 0
.size		dd 0
.data		dd process_info
		db 0
.name:		dd fname
;-----------------------------------------------------------------------------
