;------------------------------------------------------------------------------
align 4


; DATA
stripe:
	dd -0x010101
	dd -0x010101
	dd -0x020202
	dd -0x010101
	dd -0x000000

	dd 0x000000
	dd 0x010101
	dd 0x020202
	dd 0x010101
	dd 0x010101

m_icon:
	dd 0x0
	dd 0x808080
	dd 0x000000
	dd 0x000000
	dd 0xffffff

lsz m_text,\
  ru, "Œ…ž",\
  en, "MENU",\
  et, "MENÜÜ",\
  sp, "MENU"

mi_text		db 'WAVETETRBGRDGPL '

flag_text	db 'EnFiGeRuFrEtUa'

type_lang	db 0

;sound_flag	db 0
;button_frames	dd 0x0

;checks		dd -1
hidetext	db 0x11,0x10,0x1e

turn_text	db  '><'
gpl		db  'GPL'

chlang	db 'LANG',0
;syslang db 'SLAN',0

contrast	db 0

running_applications	dd 0x100
max_applications	dd 11

current_alt_tab_app	dd -1

page_list		dd 0
draw_start_position	dd 0

;b_size_y:	dd 0x0
ysi		dd 0
small_draw	dd 0x0

ptime		dd 0x0
max_x		dd 0x0
text		db '0123456789'

page_a1		db '<   >'
page_clean_but	db 25,24,18

;bte	dd 0x0	;ccddee

wcolor	dd 0x506070

sec	db ': '
;pros	db '                                                  '
;	db '                                                  '

screenxy    dd	0x0
stcount     dd	0x0

setup_exec:
		dd 7
		dd 0
.cmdline	dd 0
		dd 0
		dd 0
		db '/sys/'
file_sys	db 'SETUP',0

exec_fileinfo:
	dd 7
	dd 0
	dd 0
	dd 0
	dd 0
	db 0
.name	dd 0

bootparam_printscreen	db 'DIRECT',0

Path_def_val db 'NONE',0

this_slot	dd 0	      ; Slot of this process
max_slot	dd 255

;--------------------------------------
menu_button_x:
.start	dd 1234	;0xffff
.size	dd 0x999
;--------------------------------------
menu_button_y:
.start	dd 0xaa
.size	dd 0xc
;--------------------------------------
;------------------------------------------------------------------------------
; INI variables
width:			dd 0	;305
height:			dd 30	;18
soften_height:		dd 5
button_top_offset:	dd 3
button_bottom_offset:	dd 3
;------------------------------------------------------------------------------
; INI flags
;buttons 	dd 1	;  0 no frames  ; 1 frames
soften_up:	dd 1	;  0 no         ; 1 yes
soften_down:	dd 0	;  0 no         ; 1 yes
minimize_left:	dd 1
minimize_right:	dd 1
;icons_position	dd 95	; not used!
menu_enable:	dd 1
;tray_enable:	dd 0	; draw cpu usage, time, date
;graph_text	dd 1	; not used!
;soften_middle	dd 1	; not used!	;  0 no         ; 1 yes
run_appl:		dd 1	;  0 defaults   ; 1 activate
clean_desktop_enable:	dd 1
clock_enable:		dd 1
cpu_usage_enable:	dd 1
chlang_enable:		dd 1
place_attachment	dd 1
button_style		dd 1
;------------------------------------------------------------------------------
page_list_enable:	dd 0
redraw_window_flag	dd 0
;------------------------------------------------------------------------------
; INI colors
MenuButton_color		dd 0x44aa44	; Green
CleanDesktopButton_color	dd 0x66cc	; Blue
Clock_color			dd 0x66cc	; Blue
CpuUsage_color			dd 0x44aa44	; Green
CpuUsageBckgr_color		dd 0xdd2222	; Red
ChangeLang_color		dd 0x66cc	; Blue
PageList_color			dd 0xffffff	; White
PanelText_color			dd 0xffffff	; White
AltTab_color			dd 0xff8000	; Orange
;------------------------------------------------------------------------------
aVariables		db 'Variables',0
aPanelHeight		db 'PanelHeight',0
aPanelWidth		db 'PanelWidth',0
aSoftenHeight		db 'SoftenHeight',0
aButtonTopOffset	db 'ButtonTopOffset',0
aButtonBottomOffset	db 'ButtonBottOffset',0
;------------------------------------------------------------------------------
aFlags			db 'Flags',0
aSoftenUp		db 'SoftenUp',0
aSoftenDown		db 'SoftenDown',0
aMinimizeLeftButton	db 'MinLeftButton',0
aMinimizeRightButton	db 'MinRightButton',0
aMenuButton		db 'MenuButton',0
aRunApplButtons		db 'RunApplButtons',0
aCleanDesktopButton	db 'ClnDesktButton',0
aClock			db 'Clock',0
aCpuUsage		db 'CpuUsage',0
aChangeLang		db 'ChangeLang',0
aPageList		db 'PageList',0
aAttachment		db 'Attachment',0
aButtonsStyle		db 'ButtonsStyle',0
;------------------------------------------------------------------------------
aColors			db 'Colors',0
aCpuUsageBckgr		db 'CpuUsageBckgr',0
aPanelText		db 'Text',0
aAltTab			db 'AltTab',0
;------------------------------------------------------------------------------
aApplicationsPaths	db 'ApplicationsPaths',0
aEnd			db 'End',0
aMenu			db 'Menu',0
aRun			db 'Run',0
aPrnScr			db 'PrnScr',0
aMouseEmul		db 'MouseEmul',0
;------------------------------------------------------------------------------
plugins_directory               db 0

ini_file_name                   db 'panel.ini',0

system_dir_Libini			db '/sys/lib/libini.obj',0

err_message_found_lib2		db 'libini.obj - Not found!',0
err_message_import2			db 'libini.obj - Wrong import!',0

head_f_i:
head_f_l        db 'Errors of PLugins:',0
;------------------------------------------------------------------------------
l_libs_start:

library02  l_libs system_dir_Libini+9, path, library_path, system_dir_Libini, \
err_message_found_lib2, head_f_l, Libini_import, err_message_import2, head_f_i, plugins_directory

end_l_libs:
;------------------------------------------------------------------------------
align 4
Libini_import:
lib_init		dd aLib_init
version_lib_ini		dd aVersion_lib_ini
ini_enum_sections	dd aIni_enum_sections
ini_enum_keys		dd aIni_enum_keys
ini_get_str		dd aIni_get_str
ini_get_int		dd aIni_get_int
ini_get_color		dd aIni_get_color
ini_set_str		dd aIni_set_str
ini_set_int		dd aIni_set_int
ini_set_color		dd aIni_set_color
        dd      0
        dd      0

aLib_init		db 'lib_init',0
aVersion_lib_ini	db 'version',0
aIni_enum_sections	db 'ini_enum_sections',0
aIni_enum_keys		db 'ini_enum_keys',0
aIni_get_str		db 'ini_get_str',0
aIni_get_int		db 'ini_get_int',0
aIni_get_color		db 'ini_get_color',0
aIni_set_str		db 'ini_set_str',0
aIni_set_int		db 'ini_set_int',0
aIni_set_color		db 'ini_set_color',0
;------------------------------------------------------------------------------
