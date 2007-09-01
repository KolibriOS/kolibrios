
vscrl_capt dd -1
hscrl_capt dd -1
body_capt  dd -1

s_status dd 0

sz app_fasm    ,'/RD/1/DEVELOP/FASM',0
sz app_board   ,'/RD/1/BOARD',0
sz app_tinypad ,'/RD/1/TINYPAD',0
sz app_docpak  ,'/RD/1/DOCPAK',0

sz sysfuncs_param,'g',0

include 'tp-tables.inc'
include 'tp-locale.inc'

;// options dialog data [
label optsdlg_editor at $-EDITOR.Bounds
; rb PATHL     ; FilePath       db PATHL dup(?)
; dd 0         ; FileName       dd ?
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
  dd 0x00000000+1
  db ' '
  dd 0x00010000+9
  db ' org 100h'
  dd 0x00000000+1
  db ' '
  dd 0x00000000+20
  db ' mov ah,09h  ; write'
  dd 0x00000000+12
  db ' mov dx,text'
  dd 0x00000000+8
  db ' int 21h'
  dd 0x00030000+8
  db ' int 20h'
  dd 0x00000000+1
  db ' '
  dd 0x00000000+21
  db ' text db "Hello!",24h'
  dd 0

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
sz symbols   ,'#&*\:/<>|{}()[]=+-, '

sz ini_sec_window   ,'Window',0
sz ini_window_top   ,'Top',0
sz ini_window_left  ,'Left',0
sz ini_window_right ,'Right',0
sz ini_window_bottom,'Bottom',0
