;-----------------------------------------------------------------------------
; project name:      TINYPAD
; compiler:          flat assembler 1.67.21
; memory to compile: 3.0/9.0 MBytes (without/with size optimizations)
; version:           SVN (4.0.8)
; last update:       2014-01-22 (Jan 22, 2014)
; minimal kernel:    revision #4199 (svn://kolibrios.org/kernel/trunk)
;-----------------------------------------------------------------------------
; originally by:     Ville Michael Turjanmaa >> villemt@aton.co.jyu.fi
; maintained by:     Mike Semenyako          >> mike.dld@gmail.com
;                    Ivan Poddubny           >> ivan-yar@bk.ru
;                    Marat Zakiyanov aka Mario79
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

include '../../../config.inc'		;for nightbuild
include '../../../macros.inc'		; useful stuff
include '../../../struct.inc'
include '../../../proc32.inc'

include 'external/libio.inc'

include 'tinypad.inc'

;purge mov,add,sub            ;  SPEED

header '01',1,@CODE,TINYPAD_END,STATIC_MEM_END,MAIN_STACK,@PARAMS,ini_path

APP_VERSION equ 'SVN (4.0.8)'

TRUE = 1
FALSE = 0

;define __DEBUG__ 1
;define __DEBUG_LEVEL__ 1
;include '../../../debug-fdo.inc'

; compiled-in options

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

INI_SEC_PREFIX equ ''

;-----------------------------------------------------------------------------
section @CODE ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------
	cld
	mov	edi,@UDATA
	mov	ecx,@PARAMS-@UDATA
	mov	al,0
	rep	stosb

	mcall	68,11
	or	eax,eax
	jz	key.alt_x.close

	stdcall dll.Load,@IMPORT
	or	eax,eax
	jnz	key.alt_x.close

	mov	edi,ini_path
	xor	al,al
	mov	ecx,PATHL
	repne	scasb
	mov	dword[edi-1],'.ini'
	mov	byte[edi+3],0

	stdcall load_settings

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

	mov	al,[tabs_pos]
	mov	[tab_bar.Style],al

;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,s_example+5
	call	copy_str_1

	mov	esi,tb_opensave.text
	mov	edi,fname_Info
	call	copy_str_1
	xor	eax,eax
	mov	[edi],al
;---------------------------------------------------------------------
;OpenDialog     initialisation
	push	dword OpenDialog_data
	call	[OpenDialog_Init]
;---------------------------------------------------------------------


	mcall	66,1,1
	mcall	40,00100111b
red:
	call	drawwindow

;-----------------------------------------------------------------------------

still:
	call	draw_statusbar ; write current position & number of strings

  .skip_write:
	cmp	[open_dialog],1
	je	.open_dialog
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
;---------------------------------------------------------------------
.open_dialog:
	pusha

	call	btn.bot.cancel

	mov	esi,tb_opensave.text
	mov	edi,[OpenDialog_data.openfile_pach]
	movzx	ecx,[tb_opensave.length]
	mov	edx,[OpenDialog_data.filename_area]
	mov	ebx,[OpenDialog_data.opendir_pach]
	call	copy_str_2
	movzx	eax,byte [bot_mode2]
	mov	[OpenDialog_data.type],eax
	popa
; invoke OpenDialog
	push	dword OpenDialog_data
	call	[OpenDialog_Start]

	cmp	[OpenDialog_data.status],1
	jne	.3

	pusha
	mov	edi,tb_opensave.text
	mov	esi,[OpenDialog_data.openfile_pach]
	call	copy_str_1
	sub	edi,tb_opensave.text
	dec	edi
	mov	eax,edi
	mov	[tb_opensave.length],al
	popa

	cmp	[bot_mode2],0
	je	.2
	call	save_file
	jmp	.3
.2:
	call	load_file
.3:
	mov	[open_dialog],0
	jmp	red
;-----------------------------------------------------------------------------
draw_window_for_OD:
	call	drawwindow
	call	draw_statusbar
	ret
;-----------------------------------------------------------------------------
copy_str_2:
	cld
	push	esi ecx
	rep	movsb	; edi  openfile_pach
	xor	eax,eax
	mov	[edi],al
	pop	ecx esi
	mov	edi,ebx
	rep	movsb	; edi opendir_pach
	mov	[edi],al
	mov	esi,edi
	std
@@:
	lodsb
	cmp	al,byte '/'
	jne	@b
	inc	esi
	xor	eax,eax
	mov	[esi],al
	inc	esi
	mov	edi,edx ; edi filename_area
	call	copy_str_1
	ret
;-----------------------------------------------------------------------------
copy_str_1:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
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
proc load_settings ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad

	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_tabs_pos,2
	mov	[tabs_pos],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_secure_sel,0
	mov	[secure_sel],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_auto_braces,0
	mov	[auto_braces],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_auto_indent,1
	mov	[auto_indent],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_smart_tab,1
	mov	[smart_tab],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_optim_save,1
	mov	[optim_save],al
	invoke	ini.get_int,ini_path,ini_sec_options,ini_options_line_nums,0
	mov	[line_nums],al

	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_text,0x00000000
	mov	[color_tbl.text],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_back,0x00ffffff
	mov	[color_tbl.back],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_text_sel,0x00ffffff
	mov	[color_tbl.text.sel],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_back_sel,0x000a246a
	mov	[color_tbl.back.sel],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_symbol,0x003030f0
	mov	[color_tbl.symbol],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_number,0x00009000
	mov	[color_tbl.number],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_string,0x00b00000
	mov	[color_tbl.string],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_comment,0x00808080
	mov	[color_tbl.comment],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_line_moded,0x00ffee62
	mov	[color_tbl.line.moded],eax
	invoke	ini.get_color,ini_path,ini_sec_colors,ini_colors_line_saved,0x006ce26c
	mov	[color_tbl.line.saved],eax

	invoke	ini.get_int,ini_path,ini_sec_window,ini_window_left,250
	mov	[mainwnd_pos.x],eax
	invoke	ini.get_int,ini_path,ini_sec_window,ini_window_top,75
	mov	[mainwnd_pos.y],eax
	invoke	ini.get_int,ini_path,ini_sec_window,ini_window_width,6*80+6+SCRLW+5
	mov	[mainwnd_pos.w],eax
	invoke	ini.get_int,ini_path,ini_sec_window,ini_window_height,402
	mov	[mainwnd_pos.h],eax

	popad
	ret
endp

;-----------------------------------------------------------------------------
proc save_settings ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad

	movzx	eax,[tabs_pos]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_tabs_pos,eax
	movzx	eax,[secure_sel]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_secure_sel,eax
	movzx	eax,[auto_braces]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_auto_braces,eax
	movzx	eax,[auto_indent]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_auto_indent,eax
	movzx	eax,[smart_tab]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_smart_tab,eax
	movzx	eax,[optim_save]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_optim_save,eax
	movzx	eax,[line_nums]
	invoke	ini.set_int,ini_path,ini_sec_options,ini_options_line_nums,eax

	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_text,[color_tbl.text]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_back,[color_tbl.back]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_text_sel,[color_tbl.text.sel]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_back_sel,[color_tbl.back.sel]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_symbol,[color_tbl.symbol]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_number,[color_tbl.number]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_string,[color_tbl.string]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_comment,[color_tbl.comment]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_line_moded,[color_tbl.line.moded]
	invoke	ini.set_color,ini_path,ini_sec_colors,ini_colors_line_saved,[color_tbl.line.saved]

	invoke	ini.set_int,ini_path,ini_sec_window,ini_window_left,[mainwnd_pos.x]
	invoke	ini.set_int,ini_path,ini_sec_window,ini_window_top,[mainwnd_pos.y]
	invoke	ini.set_int,ini_path,ini_sec_window,ini_window_width,[mainwnd_pos.w]
	invoke	ini.set_int,ini_path,ini_sec_window,ini_window_height,[mainwnd_pos.h]

	popad
	ret
endp

;-----------------------------------------------------------------------------
proc start_fasm ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; BL = 0 - compile
; BL = 1 - run after compile
; BL = 2 - run under mtdbg after compile
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

	cmp	bl,0 ; compile outfile ?
	je	@f
	mov	dword[edi],',run'
	cmp	bl,1 ; run outfile ?
	je	do_run
	mov	dword[edi],',dbg'
do_run:
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
	xor	[line_nums],1
	ret
  .optimal_fill:
	xor	[optim_save],1
	ret
  .auto_indents:
	xor	[auto_indent],1
	ret
  .auto_braces:
	xor	[auto_braces],1
	ret
  .secure_sel:
	xor	[secure_sel],1
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
include 'tp-clipboard.asm'

include '../../../dll.inc'

;-----------------------------------------------------------------------------
section @DATA ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

;include_debug_strings

include 'data/tp-idata.inc'

;-----------------------------------------------------------------------------
section @IMPORT ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

library \
	libini,'libini.obj',\
	libio,'libio.obj',\
	libgfx,'libgfx.obj',\
	proc_lib,'proc_lib.obj'

import	libini, \
	ini.get_str  ,'ini_get_str',\
	ini.set_str  ,'ini_set_str',\
	ini.get_int  ,'ini_get_int',\
	ini.set_int  ,'ini_set_int',\
	ini.get_color,'ini_get_color',\
	ini.set_color,'ini_set_color'

import	libio, \
	file.find_first,'file_find_first',\
	file.find_next ,'file_find_next',\
	file.find_close,'file_find_close',\
	file.size      ,'file_size',\
	file.open      ,'file_open',\
	file.read      ,'file_read',\
	file.write     ,'file_write',\
	file.seek      ,'file_seek',\
	file.tell      ,'file_tell',\
	file.eof?      ,'file_iseof',\
	file.truncate  ,'file_truncate',\
	file.close     ,'file_close'

import	libgfx, \
	gfx.open	,'gfx_open',\
	gfx.close	,'gfx_close',\
	gfx.pen.color	,'gfx_pen_color',\
	gfx.brush.color ,'gfx_brush_color',\
	gfx.pixel	,'gfx_pixel',\
	gfx.move.to	,'gfx_move_to',\
	gfx.line.to	,'gfx_line_to',\
	gfx.line	,'gfx_line',\
	gfx.polyline	,'gfx_polyline',\
	gfx.polyline.to ,'gfx_polyline_to',\
	gfx.fillrect	,'gfx_fillrect',\
	gfx.fillrect.ex ,'gfx_fillrect_ex',\
	gfx.framerect	,'gfx_framerect',\
	gfx.framerect.ex,'gfx_framerect_ex',\
	gfx.rectangle	,'gfx_rectangle',\
	gfx.rectangle.ex,'gfx_rectangle_ex'

import	proc_lib, \
	OpenDialog_Init  ,'OpenDialog_init',\
	OpenDialog_Start  ,'OpenDialog_start'

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

ini_path rb PATHL

;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
fname_Info:
	rb 4096 	   ; filename
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
rb 1024*4
MAIN_STACK:
rb 1024*4
POPUP_STACK:

STATIC_MEM_END:

diff10 'Main memory size',0,$
