;---------------------------------------------------------------------
system_dir_1: db '/sys/lib/libimg.obj',0
system_dir_2 db '/sys/lib/proc_lib.obj',0
system_dir_3: db '/sys/lib/kmenu.obj',0
system_dir_4: db '/sys/lib/box_lib.obj',0
;---------------------------------------------------------------------
align 4
l_libs_start:
	lib1 l_libs system_dir_1+9, library_path, system_dir_1, import_libimg
	lib2 l_libs system_dir_2+9, library_path, system_dir_2, ProcLib_import
	lib3 l_libs system_dir_3+9, library_path, system_dir_3, import_libkmenu
	lib4 l_libs system_dir_4+9, library_path, system_dir_4, import_box_lib
end_l_libs:
;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type			dd 1	; Save
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd drawwin	;+28
.status			dd 0	;+32
.openfile_pach 		dd file_path	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_pach:
	db '/rd0/1/',0

Filter:
dd	Filter.end - Filter
.1:
db	'BMP',0
db	'GIF',0
db	'JPG',0
db	'PNG',0
.end:
db	0

align 4
ColorDialog_data:
.type			dd 0
.procinfo		dd procinfo ;+4
.com_area_name		dd cd_communication_area_name ;+8
.com_area		dd 0 ;+12
.start_path		dd colordialog_path ;+16
.draw_window		dd drawwin ;+20
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

;start_temp_file_name:	db 'pattern.mgb',0

path4	db '/sys/noname.bmp',0
;---------------------------------------------------------------------
include '../../develop/libraries/libs-dev/libimg/import.inc'

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

align 4
import_libkmenu:
	kmenu_init                     dd akmenu_init
	kmainmenu_draw                 dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new                   dd aksubmenu_new
	ksubmenu_delete                dd aksubmenu_delete
	ksubmenu_draw                  dd aksubmenu_draw
	ksubmenu_add                   dd aksubmenu_add
	kmenuitem_new                  dd akmenuitem_new
	kmenuitem_delete               dd akmenuitem_delete
	kmenuitem_draw                 dd akmenuitem_draw
dd 0,0
	akmenu_init                     db 'kmenu_init',0
	akmainmenu_draw                 db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new                   db 'ksubmenu_new',0
	aksubmenu_delete                db 'ksubmenu_delete',0
	aksubmenu_draw                  db 'ksubmenu_draw',0
	aksubmenu_add                   db 'ksubmenu_add',0
	akmenuitem_new                  db 'kmenuitem_new',0
	akmenuitem_delete               db 'kmenuitem_delete',0
	akmenuitem_draw                 db 'kmenuitem_draw',0

include '../../develop/libraries/box_lib/import.inc'
;---------------------------------------------------------------------