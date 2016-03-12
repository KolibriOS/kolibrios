;---------------------------------------------------------------------
head_f_i:
head_f_l	db 'System error',0
;---------------------------------------------------------------------
system_dir_1: db '/sys/lib/libimg.obj',0
err_message_found_lib_1 db 39,'libimg.obj',39,' - Not found!',0
err_message_import_1 db 39,'libimg.obj',39,' - Wrong import!',0

system_dir_ProcLib db '/sys/lib/proc_lib.obj',0
err_message_found_lib2 db 39,'proc_lib.obj',39,' - Not found!',0
err_message_import2	db 39,'proc_lib.obj',39,' - Wrong import!',0

system_dir_3: db '/sys/lib/kmenu.obj',0
err_message_found_lib_3 db 39,'kmenu.obj',39,' - Not found!',0
err_message_import_3 db 39,'kmenu.obj',39,' - Wrong import!',0
;---------------------------------------------------------------------
align 4
l_libs_start:
	lib1 l_libs system_dir_1+9, cur_dir_path, library_path, system_dir_1,\
		err_message_found_lib_1, head_f_l, import_libimg, err_message_import_1, head_f_i
	lib2 l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
		err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i
	lib3 l_libs system_dir_3+9, cur_dir_path, library_path, system_dir_3,\
		err_message_found_lib_3, head_f_l, import_libkmenu, err_message_import_3, head_f_i
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

;start_temp_file_name:	db 'pattern.mgb',0

path4	db '/sys/noname.bmp',0
;---------------------------------------------------------------------
align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw
dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0

align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
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
;---------------------------------------------------------------------