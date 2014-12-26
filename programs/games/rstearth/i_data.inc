;---------------------------------------------------------------------
; initialized data
;---------------------------------------------------------------------
plugins_directory	db 0

;system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_CnvPNG	db '/sys/lib/cnv_png.obj',0
system_dir_UNPACK	db '/sys/lib/archiver.obj',0

ihead_f_i:
ihead_f_l	db 'System error',0

;er_message_found_lib	db 'box_lib.obj - Not found!',0
;er_message_import	db 'box_lib.obj - Wrong import!',0

er_message_found_lib2	db 'cnv_png.obj - Not found!',0
er_message_import2	db 'cnv_png.obj - Wrong import!',0

err_message_found_lib3	db 'archiver.obj - Not found!',0
err_message_import3	db 'archiver.obj - Wrong import!',0

align	4
l_libs_start:
;library01	l_libs	system_dir_Boxlib+9,path,file_name,system_dir_Boxlib,\
;er_message_found_lib,ihead_f_l,Box_lib_import,er_message_import,ihead_f_i,plugins_directory

library02	l_libs	system_dir_CnvPNG+9,path,file_name,system_dir_CnvPNG,\
er_message_found_lib2,ihead_f_l,cnv_png_import,er_message_import2,ihead_f_i,plugins_directory

library04	l_libs	system_dir_UNPACK+9,path,file_name,system_dir_UNPACK,\
err_message_found_lib3,ihead_f_l,UNPACK_import,err_message_import3,ihead_f_i,plugins_directory
end_l_libs:
;---------------------------------------------------------------------
align	4
UNPACK_import:
;unpack_Version			dd aUnpack_Version
;unpack_PluginLoad		dd aUnpack_PluginLoad	
;unpack_OpenFilePlugin		dd aUnpack_OpenFilePlugin
;unpack_ClosePlugin		dd aUnpack_ClosePlugin
;unpack_ReadFolder		dd aUnpack_ReadFolder	
;unpack_SetFolder		dd aUnpack_SetFolder
;unpack_GetFiles		dd aUnpack_GetFiles
;unpack_GetOpenPluginInfo	dd aUnpack_GetOpenPluginInfo
;unpack_Getattr			dd aUnpack_Getattr
;unpack_Open			dd aUnpack_Open
;unpack_Read			dd aUnpack_Read
;unpack_Setpos			dd aUnpack_Setpos
;unpack_Close			dd aUnpack_Close
;unpack_DeflateUnpack		dd aUnpack_DeflateUnpack
unpack_DeflateUnpack2		dd aUnpack_DeflateUnpack2
	dd 0
	dd 0

;aUnpack_Version		db 'version',0
;aUnpack_PluginLoad		db 'plugin_load',0
;aUnpack_OpenFilePlugin		db 'OpenFilePlugin',0
;aUnpack_ClosePlugin		db 'ClosePlugin',0
;aUnpack_ReadFolder		db 'ReadFolder',0
;aUnpack_SetFolder		db 'SetFolder',0
;aUnpack_GetFiles		db 'GetFiles',0
;aUnpack_GetOpenPluginInfo	db 'GetOpenPluginInfo',0
;aUnpack_Getattr		db 'getattr',0
;aUnpack_Open			db 'open',0
;aUnpack_Read			db 'read',0
;aUnpack_Setpos			db 'setpos',0
;aUnpack_Close			db 'close',0
;aUnpack_DeflateUnpack		db 'deflate_unpack',0
aUnpack_DeflateUnpack2		db 'deflate_unpack2',0

;---------------------------------------------------------------------
align	4
cnv_png_import:
.Start		dd aCP_Start
;.Version	dd aCP_Version
;.Check		dd aCP_Check
;.Assoc		dd aCP_Assoc
	dd 0
	dd 0
aCP_Start	db 'START',0
;aCP_Version	db 'version',0
;aCP_Check	db 'Check_Header',0
;aCP_Assoc	db 'Associations',0
;---------------------------------------------------------------------
;align	4
;Box_lib_import:	
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
;FileBrowser_key		dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

;PathShow_prepare	dd sz_PathShow_prepare
;PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
;	dd 0
;	dd 0

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

;sz_PathShow_prepare	db 'PathShow_prepare',0
;sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------

;---------------------------------------------------------------------
; not	change	this	section!!!
; start	section
;---------------------------------------------------------------------
align	4
image_file	dd 0
raw_pointer	dd 0
return_code	dd 0
img_size	dd 0
deflate_unpack	dd 0
raw_pointer_2	dd 0	;+20
;---------------------------------------------------------------------
; end	section
;---------------------------------------------------------------------
align	4
fileinfo:
.subfunction	dd 5
.Offset		dd 0
.Offset_1	dd 0
.size		dd 0
.return		dd file_info
		db 0
.name:		dd file_name
;---------------------------------------------------------------------
error_window_x:	dd 100 shl 16+250
error_window_y:	dd 100 shl 16+120
;---------------------------------------------------------------------
load_ini_error_type:
	db 'Error loading INI file',0

load_icons_error_type:
	db 'Error loading of icons file',0

memory_free_error_type:
	db 'Error of free memory',0

memory_get_error_type:
	db 'Memory allocation error',0

load_directory_error_type:
	db 'Error loading directory',0

convert_icons_error_type:
	db 'Unsupported or corrupt data for icons file',0
;---------------------------------------------------------------------
error_help_text:
	db 'For continue press <Esc> key or <Cancel>',0
;---------------------------------------------------------------------
align	4
error_fs_text_pointers:
	dd error_fs_text_0
	dd error_fs_text_1
	dd error_fs_text_2
	dd error_fs_text_3
	dd error_fs_text_4
	dd error_fs_text_5
	dd error_fs_text_6
	dd error_fs_text_7
	dd error_fs_text_8
	dd error_fs_text_9
	dd error_fs_text_10
	dd error_fs_text_11

error_fs_text_0:	db '0 - Success full',0
error_fs_text_1:	db '1 - Base and/or partition of a hard disk is not defined',0
error_fs_text_2:	db '2 - Function is not supported for the given file system',0
error_fs_text_3:	db '3 - Unknown file system',0
error_fs_text_4:	db '4 - Reserved, is never returned in the current implementation',0
error_fs_text_5:	db '5 - File not found',0
error_fs_text_6:	db '6 - End of file, EOF',0
error_fs_text_7:	db '7 - Pointer lies outside of application memory',0
error_fs_text_8:	db '8 - Disk is full',0
error_fs_text_9:	db '9 - FAT table is destroyed',0
error_fs_text_10:	db '10 - Access denied',0
error_fs_text_11:	db '11 - Device error',0
;---------------------------------------------------------------------
font_icons_file_name		db 'font_russo_1bpp.png',0
base_icons_file_name		db 'base_8bpp.png',0
red_brick_icons_file_name	db 'red_brick_8bpp.png',0
white_brick_icons_file_name	db 'white_brick_8bpp.png',0
npc_miku_icons_file_name	db 'miku_8bpp.png',0
npc_death_icons_file_name	db 'death_8bpp.png',0
npc_skeleton_icons_file_name	db 'skeleton_8bpp.png',0
npc_ifrit_icons_file_name	db 'ifrit_8bpp.png',0
npc_barret_icons_file_name	db 'barret_8bpp.png',0
background_music_file_name	db 'walking_with_poseidon.wav',0
stone_kick_sound_file_name	db 'flaunch.wav',0
;---------------------------------------------------------------------
protagonist_route	dd 2
protagonist_position:
.x	dd 4
.y	dd 4
;---------------------------------------------------------------------
main_menu_text_pointers:
	dd menu_game_label
	dd menu_settings_label
	dd menu_credits_label
	dd menu_help_label
	dd menu_exit_label
	dd 0
;---------------------------------------------------------------------
menu_credits_text_pointers:
	dd menu_credits_text.1
	dd menu_credits_text.2
	dd menu_credits_text.3
	dd menu_credits_text.4
	dd menu_credits_text.5
	dd menu_credits_text.6
	dd 0
;---------------------------------------------------------------------
menu_help_text_pointers:
	dd menu_help_text.1
	dd menu_help_text.2
	dd menu_help_text.3
	dd menu_help_text.4
	dd 0
;---------------------------------------------------------------------
settings_menu_text_pointers:
	dd menu_settings_sound_label
	dd menu_settings_music_label
	dd menu_setting_exit
	dd 0
;---------------------------------------------------------------------
