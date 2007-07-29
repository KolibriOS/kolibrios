;-----------------------------------------------------------------------------
; project name:      TINYPAD
; compiler:          flat assembler 1.67.18
; memory to compile: 3.0/9.0 MBytes (without/with size optimizations)
; version:           4.0.4
; last update:       2007-02-08 (Feb 08, 2007)
; minimal kernel:    revision #270 (svn://kolibrios.org/kernel)
;-----------------------------------------------------------------------------
; originally by:     Ville Michael Turjanmaa >> villemt@aton.co.jyu.fi
; maintained by:     Mike Semenyako          >> mike.dld@gmail.com
;                    Ivan Poddubny           >> ivan-yar@bk.ru
;-----------------------------------------------------------------------------
; TODO (4.1.0):
;   - optimize drawing (reduce flickering)
;   - add vertical selection, undo, goto position, overwrite mode
;   - improve window drawing with small dimensions
;   - save settings to ini file, not to executable
;   - add prompt to save file before closing/opening
;   - other bug-fixes and speed/size optimizations
;
; HISTORY:
; 4.0.5 (mike.dld and Rus)
;   bug-fixes:
;     - a new tab is created when opening a file from folder of tinypad
;     - correct call save-dialog for saving new files or files from folder of tinypad
;   new features:
;     - added prompt to save file before closing
;     - added close tab button
; 4.0.4 (mike.dld)
;   bug-fixes:
;     - statusbar contained hint after dialog operation cancelled
;     - small drawing fix for gutter and line saved/modified markers
;       (incorrect calculations)
;     - incorrect lines marking on Ctrl+V
;     - put filename of current tab into open/save dialog (noticed by Maxxxx32)
;   changes:
;     - editor and other modifications to ease parts placement changing,
;       including changes in look
;     - modified/saved colors now match those in MSVS
;     - function 70 for *all* file operations (including diamond's fixes)
;     - use memory manager instead of statically allocated region
;     - case-insensitive filenames input, to be able to open/save files with
;       non-latin chars in name (russian etc.)
;     - reduced flickering (changes checker)
;     - overall code cleanup
;   new features:
;     - recode tables between CP866, CP1251 and KOI8-R (suggested by Victor)
;     - tabbed interface, ability to open several files in one app instance
;       (thanks IRC guys for ideas and testing)
;     - make any tab default to compile it disregarding currently active tab
;     - configuration dialog (colors, tabs positioning)
; 4.0.3 (mike.dld)
;   bug-fixes:
;     - 1-char selection if pressing <BS> out of real line length
;     - fault in `writepos`, added call to function 9
;     - main menu items weren't highlighted if popup opened and cursor
;       isn't in main menu item's area
;     - statusbar and textboxes drawing fixes (wrong colors)
;     - perform no redraw while pressing Shift, Ctrl, Alt keys
;     - data length from DOCPAK in string representation (fixed by diamond)
;     - compare file extension case-insensitively (fixed by diamond)
;   changes:
;     - function 70 instead of 58 for files loading/saving
;     - clientarea-relative drawing (less code)
;     - every line's dword is now splitted into 2 words;
;       low word - line block length, so max line length is 65535 now
;       high word - various flags. for now, only 2 of 16 bits are used:
;         if bit #0 is set, line was modified since file open
;         if bit #1 is set, line was saved after last modification
;       high word could also be further used for code collapsing and different
;         line marking features (breakpoints, errors, bookmarks, etc.)
;   new features:
;     - line markers for modified and saved lines
;     - status messages for various actions
; 4.0.2 (mike.dld)
;   bug-fixes:
;     - program terminates if started with parameters (fine for DOCPAK)
; 4.0.1 (mike.dld)
;   bug-fixes:
;     - unable to run program after exiting from main menu (Alt+X)
;   new features:
;     - integration with DOCPAK
; 4.0 (mike.dld)
;   bug-fixes:
;     - main menu popups' "on show" was called only for first shown popup
;     - clear selection on left/right arrow keys
;   new features:
;     - "replace" dialog (w/o "skip", "replace all")
;     - line numbers display
;     - options (except "appearance" and "smart tabulation")
;     - options saving (colors, window position, "Options" popup triggers)
; 4.0 beta 2 (mike.dld)
;   bug-fixes:
;     - unable to start if /rd/1/example.asm is missing (from Halyavin)
;     - clicking on menu items draws main window in popup (from Rohan)
;     - passed parameters aren't taken into account (from Mario79)
;     - background isn't erased if text lines < screen lines after
;       selection deletion (from Rohan)
; 4.0 beta 1 (mike.dld)
;   menu bar and popup menu;
;     removed buttons from the top and input fields from the bottom since
;     now they're accesible through main/popup menu;
;   improved keyboard handling (using 66th function);
;     support for almost all FASMW keyboard shourtcuts;
;   added text selection ability, standard selection operations
;     (copy,cut,paste);
;   new integrated dialogs (open, save, find)
;   fix to collapse SPACEs into TABs only for *.asm and *.inc files
; 3.78a (mike.dld)
;   fixed termination while typing in x positions higher than (line_length+10);
;   improved drawing on small heights
;     don't draw window while its height = 0 (Kolibri "minimize" support)
; 3.78 (mike.dld)
;   now lines may be of ANY length;
;     optimized memory usage (less memory for internal file representation)
;       after loading file, it's internal size equals to its real size
;       plus 14 bytes for each line (4 bytes for line length
;         and 10 spaced to the end - to reduce data relocations count);
;     completely rewritten keyboard handling;
;     added horizontal scrollbar;
;   all line feed formats are supported: WIN(CRLF),*NIX(LF),MAC(CR);
;   etc.
; 3.77 (mike.dld)
;   changed save_string to collapse SPACEs into TABs;
;   rewrote drawfile from scratch (speed++)
;     through some drawing improvements  needed
;     (some checkups to reduce flickering);
;   writepos (size--);
;   fixed drawing window while height < 100px, and for non-asm files;
;   several small fixes; speed/size optimizations
; 3.76 (mike.dld)
;   changed loadfile/loadhdfile to expand TABs into SPACEs;
;   changed TAB,ENTER,DELETE,BSPACE keys behaviour (rewritten from scratch);
;   vertical scrollbar;
;   extra window resizing capabilities (added a couple of constants);
;   completely new text cursor management & moving text cursor with mouse;
;   improved search function, moving cursor to beginning of text found;
;   adjustable max line width (change LINE_WIDTH & recompile) // (obsolet)
; 3.75a
;   fixed converting char to upper case in read_string
; 3.75
;   rewrote save_file from scratch; bugfix in loadfile;
; 3.74
;   optimisation
; 3.73
;   completly new load_file function
; 3.72
;   speed++
; 3.71
;   error beep
; 3.6,3.7:
;   many bugs fixed
;   simple toolbar
;   compile, run applications from TINYPAD, all fasm output is in debug board
;   TAB button
;   auto-indent
;   Ctrl+L - insert comment string
;-----------------------------------------------------------------------------

include 'lang.inc'
include '..\..\..\macros.inc' ; useful stuff
;include 'proc32.inc'
include 'tinypad.inc'
;purge mov,add,sub            ;  SPEED

header '01',1,@CODE,TINYPAD_END,STATIC_MEM_END,MAIN_STACK,@PARAMS,self_path

APP_VERSION equ '4.0.4'

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

	mov	eax,65536
	call	mem.Alloc
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

	mov	eax,edx
	call	mem.Alloc
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
	mov	eax,ebp
	call	mem.Free

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
func start_fasm ;/////////////////////////////////////////////////////////////
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
endf

;-----------------------------------------------------------------------------
func open_debug_board ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[app_start.filename],app_board
	mov	[app_start.params],0
	jmp	start_ret
endf

;-----------------------------------------------------------------------------
func open_sysfuncs_txt ;//////////////////////////////////////////////////////
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
endf

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

include 'tp-defines.inc'

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

;include 'lib-ini.asm'

;-----------------------------------------------------------------------------
section @DATA ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

vscrl_capt dd -1
hscrl_capt dd -1
body_capt  dd -1

key0 db \
  0x00,0x00,0x02,0x03,0x04,0x05,0x06,0x07,\
  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x00,0x00,\
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,\
  0x18,0x19,0x1A,0x1B,0x00,0x00,0x1E,0x1F,\
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,\
  0x28,0x29,0x00,0x2B,0x2C,0x2D,0x2E,0x2F,\
  0x30,0x31,0x32,0x33,0x34,0x35,0x00,0x00,\
  0x00,0x39,0x00,0x00,0x00,0x00,0x00,0x00
  times 12*16 db 0x00

accel_table_main dd		    \
  0x0000000E,key.bkspace	   ,\ ; BackSpace
  0x0000000F,key.tab		   ,\ ; Tab
  0x0000001C,key.return 	   ,\ ; Return
  0x0000003D,key.f3		   ,\ ; F3
  0x00000043,key.f9		   ,\ ; F9
  0x00000147,key.home		   ,\ ; Home
  0x00000148,key.up		   ,\ ; Up
  0x00000149,key.pgup		   ,\ ; PageUp
  0x0000014B,key.left		   ,\ ; Left
  0x0000014D,key.right		   ,\ ; Right
  0x0000014F,key.end		   ,\ ; End
  0x00000150,key.down		   ,\ ; Down
  0x00000151,key.pgdn		   ,\ ; PageDown
  0x00000152,key.ins		   ,\ ; Insert
  0x00000153,key.del		   ,\ ; Delete
  0x00010043,key.shift_f9	   ,\ ; Shift+F9
  0x00010147,key.shift_home	   ,\ ; Shift+Home
  0x00010148,key.shift_up	   ,\ ; Shift+Up
  0x00010149,key.shift_pgup	   ,\ ; Shift+PageUp
  0x0001014B,key.shift_left	   ,\ ; Shift+Left
  0x0001014D,key.shift_right	   ,\ ; Shift+Right
  0x0001014F,key.shift_end	   ,\ ; Shift+End
  0x00010150,key.shift_down	   ,\ ; Shift+Down
  0x00010151,key.shift_pgdn	   ,\ ; Shift+PageDown
  0x00010153,key.del		   ,\ ; Shift+Delete
  0x0002000F,key.ctrl_tab	   ,\ ; Ctrl+Tab
  0x00020015,key.ctrl_y 	   ,\ ; Ctrl+Y
  0x00020018,key.ctrl_o 	   ,\ ; Ctrl+O
  0x0002001E,key.ctrl_a 	   ,\ ; Ctrl+A
  0x0002001F,key.ctrl_s 	   ,\ ; Ctrl+S
  0x00020020,key.ctrl_d 	   ,\ ; Ctrl+D
  0x00020021,key.ctrl_f 	   ,\ ; Ctrl+F
  0x00020022,key.ctrl_g 	   ,\ ; Ctrl+G
  0x00020023,key.ctrl_h 	   ,\ ; Ctrl+H
\;0x00020026,key.ctrl_l            ,\ ; Ctrl+L
  0x0002002D,key.ctrl_x 	   ,\ ; Ctrl+X
  0x0002002E,key.ctrl_c 	   ,\ ; Ctrl+C
  0x0002002F,key.ctrl_v 	   ,\ ; Ctrl+V
  0x00020031,key.ctrl_n 	   ,\ ; Ctrl+N
  0x0002003E,key.ctrl_f4	   ,\ ; Ctrl+F4
  0x00020043,key.ctrl_f9	   ,\ ; Ctrl+F9
  0x00020147,key.ctrl_home	   ,\ ; Ctrl+Home
\;0x00020148,key.ctrl_up           ,\ ; Ctrl+Up
  0x00020149,key.ctrl_pgup	   ,\ ; Ctrl+PageUp
  0x0002014B,key.ctrl_left	   ,\ ; Ctrl+Left
  0x0002014D,key.ctrl_right	   ,\ ; Ctrl+Right
  0x0002014F,key.ctrl_end	   ,\ ; Ctrl+End
\;0x00020150,key.ctrl_down         ,\ ; Ctrl+Down
  0x00020151,key.ctrl_pgdn	   ,\ ; Ctrl+PageDown
  0x00020153,key.del		   ,\ ; Ctrl+Del
  0x0003000F,key.shift_ctrl_tab    ,\ ; Shift+Ctrl+Tab
  0x0003001F,key.shift_ctrl_s	   ,\ ; Shift+Ctrl+S
  0x00030147,key.shift_ctrl_home   ,\ ; Shift+Ctrl+Home
\;0x00030148,key.shift_ctrl_up     ,\ ; Shift+Ctrl+Up
  0x00030149,key.shift_ctrl_pgup   ,\ ; Shift+Ctrl+PageUp
  0x0003014B,key.shift_ctrl_left   ,\ ; Shift+Ctrl+Left
  0x0003014D,key.shift_ctrl_right  ,\ ; Shift+Ctrl+Right
  0x0003014F,key.shift_ctrl_end    ,\ ; Shift+Ctrl+End
\;0x00030150,key.shift_ctrl_down   ,\ ; Shift+Ctrl+Down
  0x00030151,key.shift_ctrl_pgdn   ,\ ; Shift+Ctrl+PageDown
  0x0004002D,key.alt_x		   ,\ ; Alt+X
  0

accel_table_textbox dd		    \
\;0x00000001,key.tb.escape         ,\ ; Escape
  0x0000000E,key.tb.bkspace	   ,\ ; BackSpace
\;0x0000000F,key.tb.tab            ,\ ; Tab
\;0x0000001C,key.tb.return         ,\ ; Return
  0x00000147,key.tb.home	   ,\ ; Home
  0x0000014B,key.tb.left	   ,\ ; Left
  0x0000014D,key.tb.right	   ,\ ; Right
  0x0000014F,key.tb.end 	   ,\ ; End
  0x00000153,key.tb.del 	   ,\ ; Delete
  0x00010147,key.tb.shift_home	   ,\ ; Shift+Home
  0x0001014B,key.tb.shift_left	   ,\ ; Shift+Left
  0x0001014D,key.tb.shift_right    ,\ ; Shift+Right
  0x0001014F,key.tb.shift_end	   ,\ ; Shift+End
  0x00010153,key.tb.del 	   ,\ ; Shift+Del
  0

accel_table2 dd 	  \
  1    ,key.alt_x	 ,\
  'VSL',btn.vscroll_up	 ,\
  'VSG',btn.vscroll_down ,\
  'HSL',btn.hscroll_up	 ,\
  'HSG',btn.hscroll_down ,\
  'TBL',btn.tabctl_right ,\
  'TBG',btn.tabctl_left  ,\
  177  , key.ctrl_f4	 ,\ ; close current tab button
  0

accel_table2_botdlg dd	   \
  1    ,key.alt_x	  ,\
  20001,btn.bot.cancel	  ,\
  20002,btn.bot.opensave  ,\
  20003,btn.bot.find	  ,\
  20004,btn.bot.appearance,\
  20007, btn.bot.no	  ,\
  21001,btn.bot.tabpos	  ,\
  0

add_table:
  times $1A db -$20
  times $25 db -$00
  times $10 db -$20
  times $30 db -$00
  times $10 db -$50
  times $04 db -$00,-$01
  times $08 db -$00

s_status dd 0

@^
fasm_start:
	dd	7
	dd	0
	dd	fasm_parameters
	dd	0
	dd	0
	db	'/sys/DEVELOP/FASM',0
board_start:
	dd	7
	dd	0
	dd	0
	dd	0
	dd	0
	db	'/sys/DEVELOP/BOARD',0
tinypad_start:
	dd	7
	dd	0
	dd	?
	dd	0
	dd	0
	db	'/sys/TINYPAD',0
docpak_start:
	dd	7
	dd	0
	dd	sysfuncs_param
	dd	0
	dd	0
	db	'/sys/DOCPAK',0
^@

sz app_fasm    ,'/sys/DEVELOP/FASM',0
sz app_board   ,'/sys/DEVELOP/BOARD',0
sz app_tinypad ,'/sys/TINYPAD',0
sz app_docpak  ,'/sys/DOCPAK',0

sz sysfuncs_param,'g',0

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

TINYPAD_END:	 ; end of file

self_path rb PATHL

;-----------------------------------------------------------------------------
section @UDATA ;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

app_start:
	   dd ?,?
 .params   dd ?
	   dd ?,?
	   db ?
 .filename dd ?

f_info.length dd ?
f_info.path:
    times PATHL+1 db ?
f_info70 rd 7

file_info FILEINFO

checker_ed    EDITOR
checker_ed_ll dd ?

tab_bar      TABCTL
virtual at tab_bar.Current
  cur_tab      TABITEM
end virtual
virtual at tab_bar.Current.Editor
  cur_editor   EDITOR
end virtual

lines.scr     dd ?    ; number of lines on the screen
columns.scr   dd ?    ; number of columns on the screen
top_ofs       dd ?    ; height occupied by top buttons
bot_ofs       dd ?    ; height occupied by bottom buttons
	      dw ?
left_ofs      dd ?    ;
__rc	      dd ?,?,?,?
copy_count    dd ?    ; number of lines for copying (Ctrl+E)
copy_size     dd ?    ; size of data to copy
s_title.size  dd ?    ; caption length

cur_line_len  dd ?
h_popup       dd ?
bot_dlg_handler dd ?

sel.begin.x   dd ?
sel.begin.y   dd ?
sel.end.x     dd ?
sel.end.y     dd ?
sel.selected  db ?

in_sel	      db ?

do_not_draw   db ?    ; draw top and bottom buttons?
main_closed   db ?    ; main window closed?
tb_casesen    db ?    ; focused textbox is case-sensitive?

draw_blines   db ?

align 4
s_fname.size  dd ?
s_fname       rb PATHL+1
align 4
s_search.size dd ?
s_search      rb PATHL+1

s_title       rb PATHL+11  ; window caption

chr db ?
ext db ?
shi dd ?

align 4
cl_3d_normal dd ?
cl_3d_pushed dd ?
cl_3d_outset dd ?
cl_3d_inset  dd ?
cl_3d_grayed dd ?

tb_color     TBOX
tb_opensave  TBOX
tb_find      TBOX
tb_replace   TBOX
tb_gotorow   TBOX
tb_gotocol   TBOX

focused_tb   dd ?

key1 rb 256

mst  db ?
mst2 db ?
mev  db ?
mouse_captured	db ?
just_from_popup db ?

bot_mode db ?

align 4

bot_dlg_height dd ?
bot_dlg_mode2  db ?

temp_buf dd ?
copy_buf dd ?

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
