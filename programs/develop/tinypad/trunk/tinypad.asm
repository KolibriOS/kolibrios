;-----------------------------------------------------------------------------
; project name:      TINYPAD
; compiler:          flat assembler 1.67.21
; memory to compile: 3.0/9.0 MBytes (without/with size optimizations)
; version:           4.0.5
; last update:       2007-09-18 (Sep 18, 2007)
; minimal kernel:    revision #270 (svn://kolibrios.org/kernel/trunk)
;-----------------------------------------------------------------------------
; originally by:     Ville Michael Turjanmaa >> villemt@aton.co.jyu.fi
; maintained by:     Mike Semenyako          >> mike.dld@gmail.com
;                    Ivan Poddubny           >> ivan-yar@bk.ru
;-----------------------------------------------------------------------------
; TODO (4.1.0):
;   - add vertical selection, undo, goto position, overwrite mode, smart tabulation
;   - improve window drawing with small dimensions
;   - save/load settings to/from ini file, not executable
;   - path autocompletion for open/save dialogs
;   - other bug-fixes and speed/size optimizations
;-----------------------------------------------------------------------------
; See history.txt for complete changelog
;-----------------------------------------------------------------------------

include 'lang.inc'

include '../../../macros.inc' ; useful stuff
include '../../../struct.inc'
include '../../../proc32.inc'

include 'external/libio.inc'

include 'tinypad.inc'

;purge mov,add,sub            ;  SPEED

header '01',1,@CODE,TINYPAD_END,STATIC_MEM_END,MAIN_STACK,@PARAMS,self_path

APP_VERSION equ '4.0.5'

TRUE = 1
FALSE = 0

;include 'debug.inc'
;define __DEBUG__ 1
;define __DEBUG_LEVEL__ 1
;include 'debug-fdo.inc'

ASEPC = '-'  ; separator character (char)
ATOPH = 19   ; menu bar height (pixels)
SCRLW = 16   ; scrollbar widht/height (pixels)
ATABW = 8    ; tab key indent width (chars)
LINEH = 10   ; line height (pixels)
PATHL = 256  ; maximum path length (chars) !!! don't change !!!
AMINS = 8    ; minimal scroll thumb size (pixels)
LCHGW = 3    ; changed/saved marker width (pixels)

STATH = 16   ; status bar height (pixels)
TBARH = 18   ; tab bar height (pixels)

;-----------------------------------------------------------------------------
section @OPTIONS ;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

label color_tbl dword
  .text:       RGB(  0,  0,  0)
  .back:       RGB(255,255,255)
  .text.sel:   RGB(255,255,255)
  .back.sel:   RGB( 10, 36,106)
  .symbol:     RGB( 48, 48,240)
  .number:     RGB(  0,144,  0)
  .string:     RGB(176,  0,  0)
  .comment:    RGB(128,128,128)
  .line.moded: RGB(255,238, 98)
  .line.saved: RGB(108,226,108)

ins_mode db 1
tab_pos  db 2

options  db OPTS_AUTOINDENT+OPTS_OPTIMSAVE+OPTS_SMARTTAB

mainwnd_pos:
  .x dd 250
  .y dd 75
  .w dd 6*80+6+SCRLW+5	;- 220
  .h dd 402		;- 220

;-----------------------------------------------------------------------------
section @CODE ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

;       fninit

;       stdcall ini.get_int,finfo_ini,ini_sec_window,ini_window_left,50
;       mov     [mainwnd_pos.x],eax
;       stdcall ini.get_int,finfo_ini,ini_sec_window,ini_window_top,50
;       mov     [mainwnd_pos.y],eax
;       stdcall ini.get_int,finfo_ini,ini_sec_window,ini_window_right,350
;       sub     eax,[mainwnd_pos.x]
;       mov     [mainwnd_pos.w],eax
;       stdcall ini.get_int,finfo_ini,ini_sec_window,ini_window_bottom,450
;       sub     eax,[mainwnd_pos.y]
;       mov     [mainwnd_pos.h],eax

	cld
	mov	edi,@UDATA
	mov	ecx,@PARAMS-@UDATA
	mov	al,0
	rep	stosb

	mov	al,[tab_pos]
	mov	[tab_bar.Style],al

	mcall	68,11
	or	eax,eax
	jz	key.alt_x.close

	stdcall dll.Load,@IMPORT
	or	eax,eax
	jnz	key.alt_x.close

	stdcall mem.Alloc,65536
	mov	[temp_buf],eax

	inc	[do_not_draw]

	mov	dword[app_start],7

	mov	esi,s_example
	mov	edi,tb_opensave.text
	mov	ecx,s_example.size
	mov	[tb_opensave.length],cl
	rep	movsb

	mov	esi,s_still
	mov	edi,s_search
	mov	ecx,s_still.size
	mov	[s_search.size],ecx
	rep	movsb

	cmp	byte[@PARAMS],0
	jz	no_params

;// Willow's code to support DOCPAK [

	cmp	byte[@PARAMS],'*'
	jne	.noipc

;// diamond [ (convert size from decimal representation to dword)
;--     mov     edx,dword[@PARAMS+1]
	mov	esi,@PARAMS+1
	xor	edx,edx
	xor	eax,eax
    @@: lodsb
	test	al,al
	jz	@f
	lea	edx,[edx*4+edx]
	lea	edx,[edx*2+eax-'0']
	jmp	@b
    @@:
;// diamond ]

	add	edx,20

	stdcall mem.Alloc,edx
	mov	ebp,eax
	push	eax

	mov	dword[ebp+0],0
	mov	dword[ebp+4],8
	mcall	60,1,ebp
	mcall	40,1000000b

	mcall	23,200

	cmp	eax,7
	jne	key.alt_x.close
	mov	byte[ebp],1

	mov	ecx,[ebp+12]
	lea	esi,[ebp+16]
	call	create_tab
	call	load_from_memory

	pop	ebp
	stdcall mem.Free,ebp

	jmp	@f
  .noipc:

;// Willow's code to support DOCPAK ]

	mov	esi,@PARAMS
	mov	edi,tb_opensave.text
	mov	ecx,PATHL
	rep	movsb
	mov	edi,tb_opensave.text
	mov	ecx,PATHL
	xor	al,al
	repne	scasb
	jne	key.alt_x.close
	lea	eax,[edi-tb_opensave.text-1]
	mov	[tb_opensave.length],al
	call	load_file
	jnc	@f

  no_params:
	call	create_tab

    @@:
	mov	[s_status],0
	dec	[do_not_draw]
	mcall	66,1,1
	mcall	40,00100111b
red:
	call	drawwindow

;-----------------------------------------------------------------------------

still:
	call	draw_statusbar ; write current position & number of strings

  .skip_write:
	mcall	10	; wait here until event
	cmp	[main_closed],0
	jne	key.alt_x
	dec	eax	; redraw ?
	jz	red
	dec	eax	; key ?
	jz	key
	dec	eax	; button ?
	jz	button
	sub	eax,3	; mouse ?
	jz	mouse

	jmp	still.skip_write

;-----------------------------------------------------------------------------
proc get_event ctx ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mcall	10
	dec	eax	; redraw ?
	jz	.redraw
	dec	eax	; key ?
	jz	.key
	dec	eax	; button ?
	jz	.button
	sub	eax,2	; background ?
	jz	.background
	dec	eax	; mouse ?
	jz	.mouse
	dec	eax	; ipc ?
	jz	.ipc
	dec	eax	; network ?
	jz	.network
	dec	eax	; debug ?
	jz	.debug
	sub	eax,7	; irq ?
	js	.nothing
	cmp	eax,15
	jg	.nothing
	jmp	.irq

  .nothing:
	mov	eax,EV_IDLE
	ret

  .redraw:
	mov	eax,EV_REDRAW
	ret

  .key:
	mov	eax,EV_KEY
	ret

  .button:
	mov	eax,EV_BUTTON
	ret

  .background:
	mov	eax,EV_BACKGROUND
	ret

  .mouse:
	mov	eax,EV_MOUSE
	ret

  .ipc:
	mov	eax,EV_IPC
	ret

  .network:
	mov	eax,EV_NETWORK
	ret

  .debug:
	mov	eax,EV_DEBUG
	ret
endp

;-----------------------------------------------------------------------------
proc start_fasm ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; BL = run after compile
;-----------------------------------------------------------------------------
; FASM infile,outfile,/path/to/files[,run]
;-----------------------------------------------------------------------------
	cmp	[cur_editor.AsmMode],0
	jne	@f
	ret
    @@:
	mov	eax,[tab_bar.Default.Ptr]
	or	eax,eax
	jnz	@f
	mov	eax,[tab_bar.Current.Ptr]
    @@: cmp	byte[eax+TABITEM.Editor.FilePath],'/'
	je	@f
	ret
    @@:
	mov	edi,fasm_parameters
	push	eax

	cld

	lea	esi,[eax+TABITEM.Editor.FilePath]
	add	esi,[eax+TABITEM.Editor.FileName]
	push	esi esi
    @@: lodsb
	cmp	al,0
	je	@f
	stosb
	cmp	al,'.'
	jne	@b
	mov	ecx,esi
	jmp	@b
    @@:
	mov	al,','
	stosb

	pop	esi
	sub	ecx,esi
	dec	ecx
	jz	@f
	rep	movsb
    @@:
	mov	al,','
	stosb

	pop	ecx esi
	add	esi,TABITEM.Editor.FilePath
	sub	ecx,esi
	rep	movsb

	cmp	bl,0 ; run outfile ?
	je	@f
	mov	dword[edi],',run'
	add	edi,4
    @@:
	mov	al,0
	stosb

	mov	[app_start.filename],app_fasm
	mov	[app_start.params],fasm_parameters
start_ret:
	mcall	70,app_start
	ret
endp

;-----------------------------------------------------------------------------
proc open_debug_board ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[app_start.filename],app_board
	mov	[app_start.params],0
	jmp	start_ret
endp

;-----------------------------------------------------------------------------
proc open_sysfuncs_txt ;//////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[app_start.filename],app_docpak
	mov	[app_start.params],sysfuncs_param
	call	start_ret
	cmp	eax,0xfffffff0
	jb	@f
	mov	[app_start.filename],app_tinypad
	mov	[app_start.params],sysfuncs_filename
	call	start_ret
    @@: ret
endp

set_opt:

  .dialog:
	mov	[bot_mode],1
	mov	[bot_dlg_height],128
	mov	[bot_dlg_handler],optsdlg_handler
	mov	[focused_tb],tb_color
	mov	al,[tb_color.length]
	mov	[tb_color.pos.x],al
	mov	[tb_color.sel.x],0
	mov	[tb_casesen],1
	mov	[cur_part],0
	m2m	[cur_color],dword[color_tbl.text]
	mov	esi,color_tbl
	mov	edi,cur_colors
	mov	ecx,10
	cld
	rep	movsd
	call	drawwindow
	ret

  .line_numbers:
	mov	al,OPTS_LINENUMS
	jmp	.main
  .optimal_fill:
	mov	al,OPTS_OPTIMSAVE
	jmp	.main
  .auto_indents:
	mov	al,OPTS_AUTOINDENT
	jmp	.main
  .auto_braces:
	mov	al,OPTS_AUTOBRACES
	jmp	.main
  .secure_sel:
	mov	al,OPTS_SECURESEL

  .main:
	xor	[options],al
	ret

;-----------------------------------------------------------------------------

include 'data/tp-defines.inc'

include 'tp-draw.asm'
include 'tp-key.asm'
include 'tp-button.asm'
include 'tp-mouse.asm'
include 'tp-files.asm'
include 'tp-common.asm'
include 'tp-dialog.asm'
include 'tp-popup.asm'
include 'tp-tbox.asm'
include 'tp-tabctl.asm'
include 'tp-editor.asm'
include 'tp-recode.asm'

include 'external/dll.inc'

;-----------------------------------------------------------------------------
section @DATA ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

include 'data/tp-idata.inc'

;-----------------------------------------------------------------------------
section @IMPORT ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------
;align 16
;@IMPORT:

library \
	libini,'libini.obj',\
	libio,'libio.obj',\
	libgfx,'libgfx.obj'

import	libini, \
	ini.get_str,'ini.get_str',\
	ini.set_str,'ini.set_str',\
	ini.get_int,'ini.get_int',\
	ini.set_int,'ini.set_int'

import	libio, \
	file.find_first,'file.find_first',\
	file.find_next ,'file.find_next',\
	file.find_close,'file.find_close',\
	file.size      ,'file.size',\
	file.open      ,'file.open',\
	file.read      ,'file.read',\
	file.write     ,'file.write',\
	file.seek      ,'file.seek',\
	file.tell      ,'file.tell',\
	file.eof?      ,'file.eof?',\
	file.truncate  ,'file.truncate',\
	file.close     ,'file.close'

import	libgfx, \
	gfx.open	,'gfx.open',\
	gfx.close	,'gfx.close',\
	gfx.pen.color	,'gfx.pen.color',\
	gfx.brush.color ,'gfx.brush.color',\
	gfx.pixel	,'gfx.pixel',\
	gfx.move.to	,'gfx.move.to',\
	gfx.line.to	,'gfx.line.to',\
	gfx.line	,'gfx.line',\
	gfx.polyline	,'gfx.polyline',\
	gfx.polyline.to ,'gfx.polyline.to',\
	gfx.fillrect	,'gfx.fillrect',\
	gfx.fillrect.ex ,'gfx.fillrect.ex',\
	gfx.framerect	,'gfx.framerect',\
	gfx.framerect.ex,'gfx.framerect.ex',\
	gfx.rectangle	,'gfx.rectangle',\
	gfx.rectangle.ex,'gfx.rectangle.ex'

TINYPAD_END:	 ; end of file

;-----------------------------------------------------------------------------
section @UDATA ;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

include 'data/tp-udata.inc'

;-----------------------------------------------------------------------------
section @PARAMS ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

fasm_parameters:

p_info	process_information
p_info2 process_information
sc	system_colors

rb 1024*4
MAIN_STACK:
rb 1024*4
POPUP_STACK:

STATIC_MEM_END:

diff10 'Main memory size',0,$
