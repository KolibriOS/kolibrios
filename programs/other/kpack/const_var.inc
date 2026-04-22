;---------------------------------------------------------------------
aQuestion	db '?',0
caption_str	db 'KPack - Kolibri Packer',0
buttons1names	db ' InFile:'
		db 'OutFile:'
		db '   Path:'
aCompress	db ' Compress',0
aDecompress	db 'Decompress',0
definoutname	db 0
defpath		db '/sys',0
kernel_name db 'kernel.mnt',0
;curedit		dd 0

info_str	db 'KPack - Kolibri Packer, version 0.22',10
		db 'Uses LZMA v4.32 compression library',10,10
info_len	= $ - info_str
usage_str	db 'Written by diamond in 2006-2009 specially for KolibriOS',10
		db 'Disassembled code, using Box_Lib, Proc_Lib and OpenDialog',10
		db 'by Mario79, 2010',10
		db 'LZMA is copyright (c) 1999-2005 by Igor Pavlov',10
		db 10
		db 'Command-line usage:',10
		db '   path_to_infile with zero',10
		db '   use <-ker> key to pack kernel',10
		db 10
		db 'Window usage:',10
		db '   Select file with PATH-button or set all data in the',10
		db '   text fields manually, then press an appropriate button',10
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
        dd      0,0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align	4
Box_lib_import:
edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse

init_checkbox		dd aInit_checkbox
check_box_draw		dd aCheck_box_draw
check_box_mouse		dd aCheck_box_mouse

	dd 0,0

aEdit_box_draw		db 'edit_box_draw',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0

aInit_checkbox		db 'init_checkbox2',0
aCheck_box_draw		db 'check_box_draw2',0
aCheck_box_mouse	db 'check_box_mouse2',0

;---------------------------------------------------------------------
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
;---------------------------------------------------------------------
align 4
l_libs_start:

library01  l_libs system_dir_Boxlib+9, library_path, system_dir_Boxlib, Box_lib_import

library02  l_libs system_dir_ProcLib+9, library_path, system_dir_ProcLib, ProcLib_import

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
	db '/sys',0

path4	db '/sys/temp',0

Filter:
dd	Filter.end - Filter.1
.1:
;db	'BIN',0
;db	'DAT',0
.end:
db	0

WIN_W = 490
WIN_H = 350
LINE_H = 25
RIGHT_BTN_W = 103
EDIT_X = 76
LOG_Y = LINE_H*3+5
LOG_H = WIN_H-LOG_Y-9

start_temp_file_name:	db 'temp.bin',0

;---------------------------------------------------------------------
align 4
edit1 edit_box WIN_W-EDIT_X-RIGHT_BTN_W-12, EDIT_X, 3,          0xffffff,\ 
        0xA4C4E4, 0x80ff, 0, 0x10000000, 256, inname,  mouse_dd, 0, 11,11
edit2 edit_box WIN_W-EDIT_X-RIGHT_BTN_W-12, EDIT_X, LINE_H+3,   0xffffff,\
        0xA4C4E4, 0x80ff, 0, 0x10000000, 256, outname, mouse_dd, 0, 7,7
edit3 edit_box WIN_W-EDIT_X-RIGHT_BTN_W-12, EDIT_X, LINE_H*2+3, 0xffffff,\
        0xA4C4E4, 0x80ff, 0, 0x10000000, 256, path,    mouse_dd, 0, 6,6
editboxes_end:

mouse_dd dd 0
;---------------------------------------------------------------------

check1 check_box2 ((WIN_W - RIGHT_BTN_W - 5) shl 16)+15, ((LINE_H*2+6) shl 16)+15, 6, 0xffffff,\
        0x80ff, 0x10000000, check_text,ch_flag_top
check1_end:

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