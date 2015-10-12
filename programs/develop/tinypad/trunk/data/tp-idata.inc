
vscrl_capt	dd	-1
hscrl_capt	dd	-1
body_capt	dd	-1
s_status	dd	0
lineHeight	dd	LINEH
charWidth	dd	6
scale		db	0
ins_mode	db	1

sz app_fasm    ,'/rd/1/develop/fasm',0
sz app_board   ,'/rd/1/develop/board',0
sz app_tinypad ,'/rd/1/tinypad',0
sz app_docpak  ,'/rd/1/docpack',0

sz sysfuncs_param,'f',0

include 'tp-tables.inc'
include 'tp-locale.inc'

macro editor_lines [str, flags]
{
  common
    local size
  forward
    virtual at 0
      db str
      size = $
    end virtual
    dd size
    dw flags
    db str
  common
    dd 0
}

;// options dialog data [
label optsdlg_editor at $-EDITOR.Bounds
  dd ?,?,?,?   ; Bounds         RECT
  dd @f        ; Lines          dd ?
  dd ?	       ; Lines.Size     dd ?
  dd 9	       ; Lines.Count    dd ?
  dd 21        ; Columns.Count  dd ?
  dd 0,4       ; Caret          POINT
  dd 100,4     ; SelStart       POINT
  dd 0,0       ; TopLeft        POINT
  dd 0,0       ; VScroll        SCROLLBAR
  dd 0,0       ; HScroll        SCROLLBAR
  dd 0	       ; Gutter.Width   dd ?
  db 0	       ; Gutter.Visible db ?
  db 1	       ; AsmMode        db ?
  db 0	       ; Modified       db ?

@@:
editor_lines \
  ' ',			   0, \
  ' org 100h',		   EDITOR_LINE_FLAG_MOFIFIED, \
  ' ',			   0, \
  ' mov ah,09h  ; write',  0, \
  ' mov dx,text',	   0, \
  ' int 21h',		   0, \
  ' int 20h',		   EDITOR_LINE_FLAG_MOFIFIED + EDITOR_LINE_FLAG_SAVED, \
  ' ',			   0, \
  ' text db "Hello!",24h', 0

optsdlg_editor_parts:	; left,top,right,bottom,type
  db 0, 12, 13, 29, 22
  db 0, 12, 33, 47, 42
  db 0, 12, 53, 29, 72
  db 0, 12, 83, 53, 92
  db 2, 12, 43, 77, 52
  db 3,  4, 43,148, 52
  db 4, 48, 33, 53, 42
  db 4,108, 83,113, 92
  db 5, 36, 13, 59, 22
  db 5, 54, 33, 71, 42
  db 5, 36, 53, 53, 72
  db 5,114, 83,131, 92
  db 6, 60, 83,107, 92
  db 7, 84, 33,125, 42
  db 8,  1, 13,  5, 22
  db 9,  1, 63,  5, 72
  db 1,  1,  1,148,105
  db -1
;// ]

sz symbols_ex,';?.%"',"'"
sz symbols,'#&*\:/<>|{}()[]=+-, '

; INI file section/key names

sz ini_sec_window,INI_SEC_PREFIX,'window',0
sz ini_window_top,'top',0
sz ini_window_left,'left',0
sz ini_window_width,'width',0
sz ini_window_height,'height',0

sz ini_sec_colors,INI_SEC_PREFIX,'colors',0
sz ini_colors_text,'text',0
sz ini_colors_back,'back',0
sz ini_colors_text_sel,'text_sel',0
sz ini_colors_back_sel,'back_sel',0
sz ini_colors_symbol,'symbol',0
sz ini_colors_number,'number',0
sz ini_colors_string,'string',0
sz ini_colors_comment,'comment',0
sz ini_colors_line_moded,'line_moded',0
sz ini_colors_line_saved,'line_saved',0

sz ini_sec_options,INI_SEC_PREFIX,'options',0
sz ini_options_tabs_pos,'tabs_pos',0
sz ini_options_secure_sel,'secure_sel',0
sz ini_options_auto_braces,'auto_braces',0
sz ini_options_auto_indent,'auto_indent',0
sz ini_options_optim_save,'optim_save',0
sz ini_options_line_nums,'line_nums',0
;-----------------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd p_info	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window_for_OD	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname_Info	;+36
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

Filter:
dd	Filter.end - Filter
.1:
db	'TXT',0
db	'ASM',0
db	'INC',0
db	'MAC',0
db	'DBG',0
db	'INI',0
db	'LOG',0
db	'DAT',0
db	'INF',0
db	'BAT',0
db	'SH',0

.end:
db	0

open_dialog	db 0
;-----------------------------------------------------------------------------

