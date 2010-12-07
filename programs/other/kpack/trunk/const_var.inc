;---------------------------------------------------------------------
aQuestion	db '?'
caption_str	db 'KPack',0
buttons1names	db ' InFile:'
		db 'OutFile:'
		db '   Path:'
aCompress	db 'COMPRESS',0
aDecompress	db 'DECOMPRESS',0
definoutname	db 0
defpath		db '/RD/1/'
;curedit		dd 0

info_str	db 'KPack - Kolibri Packer, version 0.20',10
		db 'Uses LZMA v4.32 compression library',10,10
info_len	= $ - info_str
usage_str	db 'Written by diamond in 2006, 2007, 2009 specially for KolibriOS',10
		db 'Disassembled code, using Box_Lib, Proc_Lib and OpenDialog',10
		db '    by Mario79, 2010',10
		db 'LZMA  is copyright (c) 1999-2005 by Igor Pavlov',10
		db 10
		db 'Command-line usage:',10
		db ' path_to_infile with zero',10
		db 10
		db 'Window usage:',10
		db 'Select file with PATH-button and press needed button',10
		db ' or enter input and output file name,',10 
		db ' enter path and press needed button',10
usage_len	= $ - usage_str
already_str	db 'Already packed kernel.mnt file',10
already_len	= $ - already_str
notkernel_str	db 'This is not kernel.mnt file',10
notkernel_len	= $ - notkernel_str
errload_str	db 'Cannot load input file',10
errload_len	= $ - errload_str
outfileerr_str	db 'Cannot save output file',10
outfileerr_len	= $ - outfileerr_str
nomem_str	db 'No memory',10
nomem_len	= $ - nomem_str
too_big_str	db 'failed, output is greater than input.',10
too_big_len	= $ - too_big_str
compressing_str	db 'Compressing ... '
compressing_len = $ - compressing_str
lzma_memsmall_str db	'Warning: not enough memory for default LZMA settings,',10
		db '         will use less dictionary size',10
lzma_memsmall_len = $ - lzma_memsmall_str
notpacked_str	db 'Input file is not packed with KPack!',10
notpacked_len	= $ - notpacked_str
unpacked_ok	db 'Unpacked successful',10
unpacked_len	= $ - unpacked_ok

done_str	db 'OK! Compression ratio: '
ratio		dw	'00'
		db '%',10
done_len	= $ - done_str
;---------------------------------------------------------------------
align 4
LiteralNextStates:
db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:
db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:
db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:
db 9,9,9,9,9,9,9,11,11,11,11,11
;---------------------------------------------------------------------
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
;---------------------------------------------------------------------
align	4
Box_lib_import:
edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

check_box_draw		dd aCheck_box_draw
check_box_mouse		dd aCheck_box_mouse
;version_ch		dd aVersion_ch

;option_box_draw	dd aOption_box_draw
;option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

;scrollbar_ver_draw	dd aScrollbar_ver_draw
;scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;menu_bar_draw		dd aMenu_bar_draw
;menu_bar_mouse		dd aMenu_bar_mouse
;version_menu_bar	dd aVersion_menu_bar

	dd 0
	dd 0

aEdit_box_draw		db 'edit_box',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

aCheck_box_draw		db 'check_box_draw',0
aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

;aOption_box_draw	db 'option_box_draw',0
;aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

;aScrollbar_ver_draw	db 'scrollbar_v_draw',0
;aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aMenu_bar_draw		db 'menu_bar_draw',0
;aMenu_bar_mouse		db 'menu_bar_mouse',0
;aVersion_menu_bar	db 'version_menu_bar',0
;---------------------------------------------------------------------
head_f_i:
head_f_l	db 'System error',0

err_message_found_lib1	db 'box_lib.obj - Not found!',0
err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import1	db 'box_lib.obj - Wrong import!',0
err_message_import2	db 'proc_lib.obj - Wrong import!',0

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
;---------------------------------------------------------------------
align 4
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

load_lib_end:
;---------------------------------------------------------------------
align 4
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
.openfile_pach 		dd fname_buf	;+36
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
	db '/rd/1',0

path4	db '/rd/1/temp',0

Filter:
dd	Filter.end - Filter.1
.1:
;db	'BIN',0
;db	'DAT',0
.end:
db	0

start_temp_file_name:	db 'temp.bin',0

;---------------------------------------------------------------------
align 4
edit1 edit_box 160, 60, 1, 0xffffff, 0xff, 0x80ff, 0, 0x8000, 256, inname, mouse_dd, 0, 11,11
edit2 edit_box 160, 60, 17, 0xffffff, 0xff, 0x80ff, 0, 0x8000,256, outname, mouse_dd, 0, 7,7
edit3 edit_box 160, 60, 33, 0xffffff, 0xff, 0x80ff, 0, 0x8000,256, path, mouse_dd, 0, 6,6
editboxes_end:

mouse_dd dd 0
;---------------------------------------------------------------------

check1 check_box 353,40,4,10,0x80AABBCC,0,0,check_text,6

check_text db 'Kernel',0
;*********************************************************************
; Kerpack data
;*********************************************************************
align 4
fn70_read:
		dd	0
		dd	0
		dd	0
insize3		dd	200*1024
infile3		dd	infile
		db	0
		dd	fullname

align 4
fn70_write:
		dd	2
		dd	0
		dd	0
outsize3	dd	?
outfile3	dd	outfile
		db	0
		dd	fullname
;*********************************************************************