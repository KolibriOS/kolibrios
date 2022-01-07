
; flat assembler source
; Copyright (c) 1999-2021, Tomasz Grysztar.
; All rights reserved.

; KolibriOS port by KolibriOS Team

format binary as ''
appname equ 'flat assembler '
;-------------------------------------------------
; HEADER
;-------------------------------------------------
	db 'MENUET01'  ; 8 byte id
	dd 0x01  ; header version
	dd START	 ; program start
	dd program_end ; program image size
	dd stacktop	 ; required amount of memory
	dd stacktop	 ; stack
	dd params	 ; parameters
	dd cur_dir_path  ; icon
	if defined import.data
		dd import.data
	else
		dd 0
	end if
;-------------------------------------------------
; INCLUDES
;-------------------------------------------------
include 'kolibria.inc'
include 'fasm.inc'
;-------------------------------------------------
; CODE
;-------------------------------------------------
use32
include 'kolibri/osloader/osloader.inc'
;-------------------------------------------------
parse_params:
	
	cmp [params],'/'
	jnz @f
	ret
@@:
	cmp	[params],0
	jnz	.chunk.console
	ret
      .chunk.console:
	mov	[_mode],CONSOLE_MODE
	mov	dword [esp], CUI_START ; force retaddr to CUI_START

	mov	eax, '    '
	mov	esi, 10
	cld
	mov	ecx, esi
	mov	edi, infile
	rep stosd
	mov	ecx, esi
	mov	edi, outfile
	rep stosd
	mov	ecx, esi
	mov	edi, path
	rep stosd

	or	ecx, -1
	mov	edi, params
	mov	al, ' '
	repe scasb
	mov	esi, edi
	dec	esi

	mov	edi, dbgWord
    @@: lodsb
	scasb
	jne	.NoOutDebugInfo
	cmp	byte[edi], 0
	jnz	@b

	cmp	byte[esi],' '
	jne	.NoOutDebugInfo

	mov	edi,esi
	mov	al,' '
	repe	scasb
	mov	esi,edi
	dec	esi

	mov	edi,params
    @@: lodsb
	stosb
	test	al,al
	jnz	@b

	or	dword[ch1_dbg.flags],CB_FLAG_EN


      .NoOutDebugInfo:
	mov	[_mode],CONSOLE_MODE
	regcall mov_param_str,,,,,params,infile
	regcall mov_param_str,,,,,esi,outfile
	regcall mov_param_str,,,,,esi,path
	mov	eax, [esi-1]
	cmp	al,','
	jne	.locret
	cmp	eax, ',run'
	jne	.check_again
	mov	[_run_outfile],1
	jmp	.locret
      .check_again:
	cmp	eax, ',dbg'
	jne	.locret
	mov	[_run_outfile],2
      .locret:
	ret
;-------------------------------------------------
START:	    ; Start of execution
	mov	edi, file_IO_slots
	mov	ecx, (file_IO_end-file_IO_slots)/4
	or	eax, -1
	rep	stosd
	mcall	SF_SYS_MISC,SSF_HEAP_INIT

	call	parse_params
	mcall	SF_SYS_MISC,SSF_MEM_ALLOC,$1000
	mov	[file_io_notify.flags],eax

unresolved_proc_termination: call    LoadLibraries

	mcall	SF_SET_EVENTS_MASK,EVM_MOUSE_FILTER or EVM_MOUSE or EVM_BUTTON or EVM_KEY or EVM_REDRAW;0x80000027 ; filter events: 80000000h - don`t receive mouse events if wnd nonactive , 7 - allowed events: redraw,keypresses, button presses
	invoke	init_checkbox,ch1_dbg
	invoke	OpenDialog_Init,OpenDialog_data

	call	prepare_esp_and_redraw
still:
	sub	esp,4
	mcall	SF_WAIT_EVENT	   ; Wait here for event
	movzx	ecx,al
	jmp	[event_handlers+4*ecx]
event_handlers	dd 0,do_redraw,key,button,0,0,mouse
;-------------------------------------------------
key:		     ; Key
	mcall	SF_GET_KEY	 ; Read it and ignore
	invoke	edit_box_key, edit1
	invoke	edit_box_key, edit2
	invoke	edit_box_key, edit3
	ret
;-------------------------------------------------
button:    ; Button in Window
	mcall	SF_GET_BUTTON
	movzx	ecx,ah
	jmp	[button_handlers+4*ecx]
button_handlers dd 0,btn_close,CUI_START,btn_runout,btn_rundbg,fun_opn_dlg
;-------------------------------------------------
btn_close:
	mcall	SF_TERMINATE_PROCESS
;-------------------------------------------------
btn_runout:
	mov	edx,outfile
	call	make_fullpaths
	mcall	SF_FILE,file_io_start
	ret
;-------------------------------------------------
btn_rundbg:
	mov	edx,outfile
	call	make_fullpaths
	mcall	SF_FILE,file_io_debug
	ret
;-------------------------------------------------
mouse:
	invoke	edit_box_mouse, edit1
	invoke	edit_box_mouse, edit2
	invoke	edit_box_mouse, edit3
	invoke	check_box_mouse,ch1_dbg
	ret
;-------------------------------------------------
Edit_Update_Colors:
	mov	[edi+EDIT_BOX.focus_border_color], ebx
	mov	[edi+EDIT_BOX.blur_border_color], eax
	ret
;-------------------------------------------------
CheckBox_Update_Colors:
	or eax, 0x10000000
	mov	[edi+CHECK_BOX2.text_color], eax
	mov	[edi+CHECK_BOX2.border_color], ebx
	ret
;-------------------------------------------------
accept_systemcolors:
	mcall	SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	or	[sc.work], $3000000
	mov	esi, sc
	mov	edi, sc_prev
	mov	ecx, sizeof.system_colors/4
	repe cmpsd
	jne	.chunk.update_colors
	ret
      .chunk.update_colors:
	inc	ecx    ; move back
	sub	edi, 4 ; on first element
	sub	esi, 4 ; that not match
	rep movsd      ; copy only difference part
	mov	eax, [sc.work]
	mov	ebx, [sc.work_graph]
	shr	eax, 1
	shr	ebx, 1
	and	eax, $7F7F7F
	and	ebx, $7F7F7F
	add	eax, ebx
	regcall Edit_Update_Colors,eax, ebx,,,,edit1
	regcall Edit_Update_Colors,eax, ebx,,,,edit2
	regcall Edit_Update_Colors,eax, ebx,,,,edit3
	mov	eax, [sc.work_text]
	or	eax, $80000000
	mov	ebx, [sc.work_graph]
	regcall CheckBox_Update_Colors,eax, ebx,,,,ch1_dbg
	ret
;-------------------------------------------------
draw_window:
	cmp	dword[PROCESS_INFO.client_box.width],WIN_MIN_W
	jge	@f
	mcall	SF_CHANGE_WINDOW,-1,-1,WIN_MIN_W+20,-1
	ret
@@:
	cmp	dword[PROCESS_INFO.client_box.height],WIN_MIN_H
	jge	@f
	mcall	SF_CHANGE_WINDOW,-1,-1,-1,WIN_MIN_H+50
	ret
@@:
	mpack	ebx,[PROCESS_INFO.client_box.width],RIGHT_BTN_W
	msub	ebx,RIGHT_BTN_W+1,0
	mcall	SF_DEFINE_BUTTON,ebx,<LINE_H*0+3,LINE_H-4>,ID_COMPILE_BTN,[sc.work_button]
	mcallb	SF_DEFINE_BUTTON,ebx,<LINE_H*1+3,LINE_H-4>,ID_EXECUTE_BTN
	mcallb	SF_DEFINE_BUTTON,ebx,<LINE_H*2+3,LINE_H-4>,ID_EXECDBG_BTN

	mcallb	SF_DEFINE_BUTTON,<5,62>,<LINE_H*2+3,LINE_H-5>,ID_OPENDLG_BTN ;button for OpenDialog [..]

	mov	ecx, [sc.work_text]
	or	ecx, $10000000
	mcall	SF_DRAW_TEXT,<6,LINE_H*0+6>,,text+text.line_size*0,text.line_size	;InFile
	mcallb	SF_DRAW_TEXT,<6,LINE_H*1+6>,,text+text.line_size*1,esi	     ;OutFile
	mov	ecx, [sc.work_button_text]
	or	ecx, $10000000
	mcallb	SF_DRAW_TEXT,<0,LINE_H*2+6>,,text+text.line_size*2,esi	 ;Path

	mov	ebx,[PROCESS_INFO.client_box.width]
	sub	ebx,RIGHT_BTN_W-11
	shl	ebx,16
	add	ebx,LINE_H/2-6
	mov	ecx, [sc.work_button_text]
	or	ecx, $10000000
	mcallb	SF_DRAW_TEXT,ebx,ecx,s_compile,7
	add	ebx,LINE_H
	mcallb	SF_DRAW_TEXT,ebx,ecx,s_run
	add	ebx,LINE_H
	mcallb	SF_DRAW_TEXT,ebx,ecx,s_debug
	
	;MAGIC1 = 6*(text.line_size-1)+14 ;MAGIC???? MAGIC??????????? GO FYSLF.
	;mpack  ebx,MAGIC1+6,1+ 14/2-3+ 14*0
	;mov    esi,[PROCESS_INFO.client_box.width]
	;sub    esi,MAGIC1*2+6+3
	;mov    eax,esi
	;mov    cl,6
	;div    cl
	;cmp    al,MAX_PATH
	;jbe    @f
	;mov    al,MAX_PATH
;@@:
	movzx	esi,al

	call	draw_messages
	
	mov	eax,dword [PROCESS_INFO.client_box.width]
	sub	eax,[edit1.left]
	sub	eax,RIGHT_BTN_W+6
	mov	dword[edit1.width],eax
	mov	dword[edit2.width],eax
	mov	dword[edit3.width],eax
	invoke	edit_box_draw, edit1
	invoke	edit_box_draw, edit2
	invoke	edit_box_draw, edit3
	invoke	check_box_draw, ch1_dbg
	ret
;-------------------------------------------------
prepare_esp_and_redraw:
	mov	[processing_esp],esp
do_redraw:
	pusha
	mcall	SF_REDRAW,SSF_BEGIN_DRAW ; Start of draw
	
	call	accept_systemcolors
	mov	edx,[sc.work]
	or	edx,CW_CAPTION or CW_CLIENTRECTCOORDS or CW_SKINED;0x33000000
	mcall	SF_CREATE_WINDOW,<150,DEFAULT_WIN_W>,<150,DEFAULT_WIN_H>,edx,,title
	mcall	SF_THREAD_INFO,PROCESS_INFO,-1

	mov	eax,dword[PROCESS_INFO.wnd_state] ;status of window
	test	eax,100b
	jnz	.skip_draw_window
	call	draw_window
.skip_draw_window:
	mcall	SF_REDRAW,SSF_END_DRAW ; End of Draw
	popa
	ret
;---------------------------------------------------------------------
bottom_right dd ?

align 4
fun_opn_dlg:
	pushad
	mov	edx, open_dialog_name
	mov	edi, communication_area_default_path
	mov	esi, library_path
	call	@copy_path_wo_pdname
	mov	[OpenDialog_data.type], 0

	xor	al, al
	mov	edi, dword [edit3.text]
	mov	ecx, dword [edit3.max]
	cld
	repne	scasb
	cmp	byte[edi-2], '/'
	jne	@f
	mov	byte[edi-2], 0 ;if last symbol is slash cut it off
@@:
	invoke	OpenDialog_Start, OpenDialog_data
	cmp	[OpenDialog_data.status], 2
	je	@f

	xor	al, al
	mov	edi, dword [edit3.text]
	mov	ebx, edi ;copy text pointer
	mov	ecx, dword [edit3.max]
	cld
	repne	scasb
	cmp	byte[edi-2], '/'
	jne	.no_slash
	
	dec	edi ;if last symbol is slash cut it off
.no_slash:
	mov	byte[edi-1], '/' ;add slash as last symbol
	mov	byte[edi], 0 ;cut off file name
	sub	edi, ebx ;edi = strlen(edit3.text)
	mov	[edit3.size], edi
	mov	[edit3.pos], edi
	invoke	edit_box_set_text, edit1, [OpenDialog_data.filename_area]
	invoke	edit_box_set_text, edit2, [OpenDialog_data.filename_area]

	mov	esi, [edit2.text]
	xor	eax, eax
	cld
.cycle:
	lodsb
	test	eax, eax
	jnz	.cycle

	sub	esi, 5
	cmp	esi, [edit2.text]
	jle	.short_fn
	
	mov	byte[esi], 0
	sub	dword [edit2.size], 4
	sub	dword [edit2.pos], 4

.short_fn:
	invoke	edit_box_draw, edit1
	invoke	edit_box_draw, edit2
	invoke	edit_box_draw, edit3
@@:
	popad
	ret
;---------------------------------------------------------------------
draw_messages:
	mpack	ebx, 5,[PROCESS_INFO.client_box.width]
	sub	ebx, 9
	mpack	ecx, 0,[PROCESS_INFO.client_box.height]
	madd	ecx, LINE_H*4,-( LINE_H*4+5)
	mov	word[bottom_right+2], bx
	mov	word[bottom_right], cx
	msub	[bottom_right], 7,11
	add	[bottom_right], 7 shl 16 + 53
	mcall	SF_DRAW_RECT,,,0xFeFefe  ; clear work area

	; draw top shadow
	push ecx
	mov cx,1
	mov edx,0xDADEDA
	mcall
	
	; draw left shadow
	pop  ecx
	push ebx
	mov bx,1
	mcall
	pop  ebx	

_cy = 0
_sy = 2
_cx = 4
_sx = 6
	push	ebx ecx
	mpack	ebx, 4,5
	add	bx, [esp+_cx]
	mov	ecx, [esp+_sy-2]
	mov	cx, [esp+_sy]
	msub	ecx, 1,1
	mcall	SF_DRAW_LINE,,,[sc.work_graph]
	mov	si, [esp+_cy]
	add	cx, si
	shl	esi, 16
	add	ecx, esi
	madd	ecx, 1,1
	mcallb	SF_DRAW_LINE
	mpack	ebx, 4,4
	mov	esi, [esp+_sy-2]
	mov	si, cx
	mov	ecx, esi
	mcallb	SF_DRAW_LINE
	mov	si,[esp+_cx]
	add	bx,si
	shl	esi,16
	add	ebx,esi
	madd	ebx,1,1
	mcallb	SF_DRAW_LINE
	pop	ecx ebx
	ret
;---------------------------------------------------------------------
mov_param_str:
	cld
@@:
	lodsb
	cmp	al,','
	je	@f
	stosb
	test	al,al
	jnz	@b
@@:
	xor	al,al
	stosb
	ret
;---------------------------------------------------------------------
CUI_START:
	cmp	[_mode],NORMAL_MODE
	jne	@f
	call	draw_messages
	mov	[textxy],8 shl 16 + LINE_H*4+4
@@:
	mov	esi,_logo
	call	display_string

;---------------------------------------------------------------------
;   Fasm native code
;---------------------------------------------------------------------
	mov	[input_file],infile
	mov	[output_file],outfile
	
	call	init_memory
	
	call	get_tickcount
	mov	[start_time],eax
	
	call	preprocessor
	call	parser
	call	assembler
	bt	dword[ch1_dbg.flags],1 ;cmp [bGenerateDebugInfo], 0
	jae	@f			  ;jz @f
	call	symbol_dump
@@:
	call	formatter
	
	call	display_user_messages
	movzx	eax,[current_pass]
	inc	eax
	call	display_number
	mov	esi,_passes_suffix
	call	display_string
	call	get_tickcount
	sub	eax,[start_time]
	xor	edx,edx
	mov	ebx,100
	div	ebx
	or	eax,eax
	jz	display_bytes_count
	xor	edx,edx
	mov	ebx,10
	div	ebx
	push	edx
	call	display_number
	mov	dl,'.'
	call	display_character
	pop	eax
	call	display_number
	mov	esi,_seconds_suffix
	call	display_string
display_bytes_count:
	mov	eax,[written_size]
	call	display_number
	mov	esi,_bytes_suffix
	call	display_string
	xor	al,al
	
	cmp	[_run_outfile],0
	je	@f
	mov	edx,outfile
	call	make_fullpaths
	xor	ecx,ecx

	cmp	[_run_outfile],2 ; param is ',dbg'
	jne	run
	mcall	SF_FILE,file_io_debug
	jmp	@f
run:
	mcall	SF_FILE,file_io_start
@@:
	jmp	exit_program
	
;---------------------------------------------------------------------
include 'system.inc'
include 'core/version.inc'
include 'core/errors.inc'
include 'core/symbdump.inc'
include 'core/preproce.inc'
include 'core/parser.inc'
include 'core/exprpars.inc'
include 'core/assemble.inc'
include 'core/exprcalc.inc'
include 'core/formats.inc'
include 'core/x86_64.inc'
include 'core/avx.inc'
include 'core/tables.inc'
include 'core/messages.inc'
;---------------------------------------------------------------------
; IMPORT
;---------------------------------------------------------------------
  library box_lib,'box_lib.obj',\
	  proc_lib,'proc_lib.obj'

  import box_lib,edit_box_draw,'edit_box_draw',\
		 edit_box_key,'edit_box_key',\
		 edit_box_mouse,'edit_box_mouse',\
		 edit_box_set_text,'edit_box_set_text',\
		 version_ed,'version_ed',\
		 init_checkbox,'init_checkbox2',\
		 check_box_draw,'check_box_draw2',\
		 check_box_mouse,'check_box_mouse2',\
		 version_ch,'version_ch'

  import proc_lib,OpenDialog_Init,'OpenDialog_init',\
		  OpenDialog_Start,'OpenDialog_start'
;---------------------------------------------------------------------
; INITIALIZED DATA
;---------------------------------------------------------------------
include 'traslations.inc'

edit1 EDIT_BOX 153, 72, 3,	    0xffffff, 0xA4C4E4, 0x80ff, 0, 0x10000000,(outfile-infile-1), infile, mouse_dd, 0, 11,11
edit2 EDIT_BOX 153, 72, LINE_H+3,   0xffffff, 0xA4C4E4, 0x80ff, 0, 0x10000000,(path-outfile-1), outfile, mouse_dd, 0, 7,7
edit3 EDIT_BOX 153, 72, LINE_H*2+3, 0xffffff, 0xA4C4E4, 0x80ff, 0, 0x10000000,(path_end-path-1), path, mouse_dd, 0, 5,5
editboxes_end:
ch1_dbg CHECK_BOX2 (5 shl 16)+15, ((LINE_H*3+3) shl 16)+15, 6, 0xffffff, 0x80ff, 0x10000000, s_dbgdescr,CB_FLAG_TOP
;---------------------------------------------------------------------
align 4
OpenDialog_data OPEN_DLG 0,PROCESS_INFO,communication_area_name,0,path,default_dir,library_path,do_redraw,0,path,filename_area,Filter,420,10,320,10

default_dir	db '/sys',0

communication_area_name db 'FFFFFFFF_open_dialog',0
open_dialog_name	db 'opendial',0
communication_area_default_path db '/sys/File managers/',0

Filter:
	dd Filter.end - Filter
      .1:
	db 'ASM',0
      .end:
	db 0


;---------------------------------------------------------------------
mouse_dd dd 0 ;needed for Shift in editbox
;---------------------------------------------------------------------
infile	path 'EXAMPLE.ASM',0
outfile path 'EXAMPLE',0
path	path '/sys//',0
path_end:
crlf	db $D,$A,0
title	db appname,VERSION_STRING,0

_logo	db 'flat assembler  version ',VERSION_STRING,$D,$A,0

_passes_suffix	db ' passes, ',0
_seconds_suffix db ' seconds, ',0
_bytes_suffix	db ' bytes.',$D,$A,0

_include	db 'INCLUDE',0

_counter	db 4,'0000'

_mode		dd NORMAL_MODE
_run_outfile	dd 0

dbgWord 	db '-d',0

character   db ?,0

textxy	 dd 5:$A0
dc	 db 0
filesize dd 0


error_prefix		db 'error: ',0
error_suffix		db '.',0
line_data_start 	db ':'
line_number_start	db ' [',0

import_loader.state dd 0
file_io_notify		FILEIO SSF_START_APP
file_io_notify.path	db 0
file_io_notify.lppath	dd notify_path
notify_path		db '/sys/@notify',0

file_io_start		FILEIO SSF_START_APP
file_io_start.path	path

file_io_debug		FILEIO SSF_START_APP,0,file_io_start.path
file_io_debug.path	db '/SYS/DEVELOP/MTDBG',0
_ramdisk		db '/rd/1/'
filepos 		dd 0

sub_table:
times $41 db $00
times $1A db $20
times $25 db $00
times $10 db $20
times $30 db $00
times $10 db $50
times $04 db $00,$01
times $08 db $00
program_end:
;---------------------------------------------------------------------
; UNINITIALIZED DATA
;---------------------------------------------------------------------
params		rb $1000
cur_dir_path	rb $1000
library_path	rb $1000
filename_area	rb $100

align 4

include 'core/variable.inc'

align 4

program_base	dd ?
buffer_address	dd ?
memory_setting	dd ?
start_time	dd ?
memblock	dd ?

fileinfo	FILEINFO

predefinitions	rb $1000

fullpath_open	path
fullpath_write	path
dbgfilename	rb MAX_PATH+4

sc		system_colors
sc_prev 	system_colors
max_handles = 8
file_IO_slots	    rb (4+sizeof.FILEIO+MAX_PATH)*max_handles;(4+20+MAX_PATH)*max_handles
file_IO_end:
PROCESS_INFO	process_information

bytes_count dd ?
displayed_count dd ?
last_displayed	rb 2
processing_esp dd ?

align 1000h
;---------------------------------------------------------------------
; STACK
;---------------------------------------------------------------------
		rb $1000
stacktop:
