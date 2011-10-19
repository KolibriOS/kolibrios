;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                 ;;
;;  flat assembler source                          ;;
;;  Copyright (c) 1999-2006, Tomasz Grysztar       ;;
;;  All rights reserved.                           ;;
;;                                                 ;;
;;  KolibriOS port by KolibriOS Team               ;;
;;  Menuet port by VT                              ;;
;;                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

NORMAL_MODE    = 8
CONSOLE_MODE   = 32

MAGIC1	       = 6*(text.line_size-1)+14
MAX_PATH       = 100

APP_MEMORY     = 0x00800000

;; Menuet header

appname equ "flat assembler "
;---------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'  ; 8 byte id
	dd 0x01	 ; header version
	dd START	 ; program start
	dd program_end ; program image size
	dd stacktop	 ; required amount of memory
	dd stacktop	 ; stack
	dd params,cur_dir_path  ; parameters,icon
;---------------------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
purge add,sub	 ; macros.inc does incorrect substitution
include 'fasm.inc'

include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
  @use_library

center fix true
;---------------------------------------------------------------------
START:	    ; Start of execution
	mov	edi, fileinfos
	mov	ecx, (fileinfos_end-fileinfos)/4
	or	eax, -1
	rep	stosd
	push	68
	pop	eax
	push	11
	pop	ebx
	mcall

	cmp	[params],0
	jz	start_1
	
	mov	ecx,10
	mov	eax,'    '
	mov	edi,infile
	push	ecx
	cld
	rep	stosd
	mov	ecx,[esp]
	mov	edi,outfile
	rep	stosd
	pop	ecx
	mov	edi,path
	rep	stosd
	
	mov	 esi,params
;	DEBUGF	"params: %s\n",esi
	mov	edi,infile
	call	mov_param_str
;	mov	edi,infile
;	DEBUGF	" input: %s\n",edi
	mov	edi,outfile
	call	mov_param_str
;	mov	edi,outfile
;	DEBUGF	"output: %s\n",edi
	mov	edi,path
	call	mov_param_str
;	mov	edi,path
;	DEBUGF	"  path: %s\n",edi
	dec	esi
	cmp	[esi], dword ',run'
	jne	@f
	mov	[_run_outfile],1
@@:
	mov	[_mode],CONSOLE_MODE
	jmp	start
;---------------------------------------------------------------------
start_1:
;sys_
load_libraries l_libs_start,load_lib_end

	cmp	eax,-1
	jne	@f
	mcall	-1 ;exit if not open box_lib.obj
@@:
	mcall	40,0x27 ;маска системных событий
;---------------------------------------------------------------------
init_checkboxes2 ch1_dbg,ch1_dbg+ch_struc_size
get_sys_colors 1,0
edit_boxes_set_sys_color edit1,editboxes_end,sc
check_boxes_set_sys_color2 ch1_dbg,ch1_dbg+ch_struc_size,sc
;---------------------------------------------------------------------
; OpenDialog initialisation
	push dword OpenDialog_data
	call dword [OpenDialog_Init]
;---------------------------------------------------------------------
red:	; Redraw
	call	draw_window

still:	
	mcall	10	; Wait here for event
	cmp	al,6
	je	call_mouse
	dec	eax 
	je	red	     ; Redraw request
	dec	eax 
	jne	button	     ; Button in buffer
key:		     ; Key
	mcall	2	; Read it and ignore
	
	push	dword edit1
	call	[edit_box_key]
	push	dword edit2
	call	[edit_box_key]
	push	dword edit3
	call	[edit_box_key]
	jmp	still
;---------------------------------------------------------------------
call_mouse:
	call	mouse
	jmp	still
;---------------------------------------------------------------------
button:    ; Button in Window
	mcall	17
	cmp	ah,1
	jne	noclose
	or	eax,-1
	mcall
;---------------------------------------------------------------------
noclose:    
	cmp	ah,5 ;press button for OpenDialog
	jne	@f
	call	fun_opn_dlg
@@:
	cmp	ah,2	      ; Start compiling
	je	start
	cmp	ah,3	      ; Start compiled file
	jnz	norunout
	
	mov	edx,outfile
	call	make_fullpaths
	mcall	70,file_info_start
;	xor	ecx,ecx
	jmp	still
;---------------------------------------------------------------------
norunout:
	cmp	ah,4
	jnz	norundebug
	
	mov	edx,outfile
	call	make_fullpaths
	mcall	70,file_info_debug
	jmp	still
;---------------------------------------------------------------------
norundebug:
	jmp	still
;---------------------------------------------------------------------
mouse:
	push	dword edit1
	call	[edit_box_mouse]
	push	dword edit2
	call	[edit_box_mouse]
	push	dword edit3
	call	[edit_box_mouse]
	push	dword ch1_dbg
	call	[check_box_mouse]
	ret
;---------------------------------------------------------------------
draw_window:
	pusha
	mcall	12,1 ; Start of draw
;get_sys_colors 1,0

	mov	edx,[sc.work]
	or	edx,0x33000000
	xor	eax,eax
	xor	esi,esi
	mcall	,<100,280>,<90,260>,,,title	       ; Draw Window Label Text

	mcall	9,PROCESSINFO,-1

	mov	eax,[PROCESSINFO+70] ;status of window
	test	eax,100b
	jne	.end

	cmp	dword[pinfo.box.width],230 ; яЁютхЁ хь °шЁшэє юъэр
	jge	@f
	mov	dword[pinfo.box.width],230 ; хёыш юъэю юўхэ№ єчъюх, єтхышўштрхь °шЁшэє фы  шчсхцрэш  уы■ъют
@@:
	mpack	ecx,1,1
	mov	ebx,[pinfo.box.width]
	sub	ebx,10
	mov	eax,8
	mov	edx,0x4000000B
	mpack	ebx,[pinfo.box.width],MAGIC1
	msub	ebx,MAGIC1+10+1,0
	mpack	ecx,0, (14*3+16)/3-1
	madd	ecx,1,0
	mcall	,,,0x00000002,[sc.work_button]
	madd	ecx, (14*3+16)/3+1,0
	mcall	,,,0x00000003
	madd	ecx, (14*3+16)/3+1,0
	mcall	,,,4
;button for OpenDialog [..]
	mov	ebx, 5*65536+47
	mov	ecx, 33*65536+14
	mcall	,,,5

	mpack	ebx,6,0    ; Draw Window Text
	add	ebx,1+ 14/2-3
	mcall	4,,[sc.work_text],text,text.line_size	;InFile

	add	ebx, 16 ;14
	add	edx,text.line_size
	mcall	;OutFile

	mov	ecx,[sc.work_button_text]
	add	ebx, 16 ;14
	add	edx,text.line_size
	mcall	;Path

	mov	ebx,[pinfo.box.width]
	sub	ebx,MAGIC1+10+1-9
	shl	ebx,16
	add	ebx,1+( (14*3+16)/3-1)/2-3
	mcall	,,[sc.work_button_text],s_compile,7
	add	ebx,(14*3+16)/3+1
	mcall	,,,s_run
	add	ebx,(14*3+16)/3+1
	mcall	,,,s_debug
	
	mpack	ebx,MAGIC1+6,0
	add	ebx,1+ 14/2-3+ 14*0
	mov	esi,[pinfo.box.width]
	sub	esi,MAGIC1*2+5*2+6+3
	mov	eax,esi
	mov	cl,6
	div	cl
	cmp	al,MAX_PATH
	jbe	@f
	mov	al,MAX_PATH
@@:
	movzx	esi,al

	call	draw_messages
	
	mov	eax,dword [pinfo.box.width]
	sub	eax,127
	mov	dword[edit1.width],eax ; єёЄрэртыштрхь °шЁшэє ЄхъёЄют√ї яюыхщ
	mov	dword[edit2.width],eax
	mov	dword[edit3.width],eax
	
	push	dword edit1
	call	[edit_box_draw]
	push	dword edit2
	call	[edit_box_draw]
	push	dword edit3
	call	[edit_box_draw]
	push	dword ch1_dbg
	call	[check_box_draw]
.end:	
	mcall	12,2 ; End of Draw
	popa
	ret
;---------------------------------------------------------------------
bottom_right dd ?

align 4
fun_opn_dlg: ;функция для вызова OpenFile диалога
	pushad
	copy_path open_dialog_name,communication_area_default_path,library_path,0
	mov	[OpenDialog_data.type],0

	xor	al,al
	mov	edi,dword [edit3.text]
	mov	ecx,dword [edit3.max]
	cld
	repne	scasb
	cmp	byte[edi-2],'/'
	jne	@f
	mov	byte[edi-2],0 ;если в конце пути есть слеш, то путь укорачиваем на 1 символ
@@:
	push	dword OpenDialog_data
	call	dword [OpenDialog_Start]
	cmp	[OpenDialog_data.status],2
	je	@f

	xor	al,al
	mov	edi,dword [edit3.text]
	mov	ebx,edi ;copy text pointer
	mov	ecx,dword [edit3.max]
	cld
	repne	scasb
	cmp	byte[edi-2],'/'
	jne	.no_slash
	
	dec	edi ;если в конце пути есть слеш, то путь укорачиваем на 1 символ
.no_slash:
	mov	byte[edi-1],'/' ;ставим в конце пути слеш
	mov	byte[edi],0 ;отрезаем имя найденного файла
	sub	edi,ebx ;edi = strlen(edit3.text)
	mov	[edit3.size],edi
	mov	[edit3.pos],edi

	push	dword [OpenDialog_data.filename_area]
	push	dword edit1
	call	dword [edit_box_set_text]

	push	dword [OpenDialog_data.filename_area]
	push	dword edit2
	call	dword [edit_box_set_text]

	mov	esi,[edit2.text]
	xor	eax,eax
	cld
.cycle:
	lodsb
	test	eax,eax
	jnz	.cycle

	sub	esi,5
	cmp	esi,[edit2.text]
	jle	.short_fn
	
	mov	byte[esi],0
	sub	dword [edit2.size],4
	sub	dword [edit2.pos],4

.short_fn:
	push	dword edit1
	call	dword [edit_box_draw]
	push	dword edit2
	call	dword [edit_box_draw]
	push	dword edit3
	call	dword [edit_box_draw]
@@:
	popad
	ret
;---------------------------------------------------------------------
draw_messages:
	mpack	ebx,7-2,[pinfo.box.width]
	sub	ebx,5*2+7*2-1-2*2
	mpack	ecx,0,[pinfo.box.height]
	madd	ecx, 14*3+16+1+7+1,-( 14*3+16+1+7*2+25)
	mov	word[bottom_right+2],bx
	mov	word[bottom_right],cx
	msub	[bottom_right],7,11
	add	[bottom_right],7 shl 16 + 53
	mcall	13,,,[sc.work]	; clear work area
_cy = 0
_sy = 2
_cx = 4
_sx = 6
	push	ebx ecx
	mpack	ebx,4,5
	add	bx,[esp+_cx]
	mov	ecx,[esp+_sy-2]
	mov	cx,[esp+_sy]
	msub	ecx,1,1
	mcall	38,,,[sc.work_graph]
	mov	si,[esp+_cy]
	add	cx,si
	shl	esi,16
	add	ecx,esi
	madd	ecx,1,1
	mcall
	mpack	ebx,4,4
	mov	esi,[esp+_sy-2]
	mov	si,cx
	mov	ecx,esi
	mcall
	mov	si,[esp+_cx]
	add	bx,si
	shl	esi,16
	add	ebx,esi
	madd	ebx,1,1
	mcall
	pop	ecx ebx
	ret
;---------------------------------------------------------------------
; DATA
;---------------------------------------------------------------------
if lang eq ru
text:
  db ' ВхФайл:'
.line_size = $-text
  db 'ВыхФайл:'
  db '   Путь:'
  ;db 'x'

  s_compile db 'Компил.'
  s_run     db ' Пуск  '
  s_debug   db 'Отладка'
  s_dbgdescr db 'Создавать отладочную информацию',0


  err_message_found_lib0 db 'Не найдена библиотека box_lib.obj',0  ;строка, которая будет в сформированном окне, если библиотека не будет найдена
  err_message_import0 db 'Ошибка при импорте библиотеки box_lib.obj',0
  err_message_found_lib1 db 'Не найдена библиотека proc_lib.obj',0
  err_message_import1 db 'Ошибка при импорте библиотеки proc_lib.obj',0
  head_f_i:
  head_f_l db 'Системная ошибка',0 ;заголовок окна, при возникновении ошибки
else
text:
  db ' InFile:'
.line_size = $-text
  db 'OutFile:'
  db '   Path:'
  ;db 'x'

  s_compile db 'COMPILE'
  s_run     db '  RUN  '
  s_debug   db ' DEBUG '
  s_dbgdescr db 'Generate debug information',0


  err_message_found_lib0 db 'Sorry I cannot found library box_lib.obj',0
  err_message_import0 db 'Error on load import library box_lib.obj',0
  err_message_found_lib1 db 'Sorry I cannot found library proc_lib.obj',0
  err_message_import1 db 'Error on load import library proc_lib.obj',0

  head_f_i:
  head_f_l db 'System error',0 ;заголовок окна, при возникновении ошибки
end if

  system_dir0 db '/sys/lib/'
  lib0_name db 'box_lib.obj',0

  system_dir1 db '/sys/lib/'
  lib1_name db 'proc_lib.obj',0
;---------------------------------------------------------------------
align 4
import_box_lib:
edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
edit_box_set_text	dd aEdit_box_set_text
;version_ed		dd aVersion_ed

init_checkbox 		dd aInit_checkbox
check_box_draw		dd aCheck_box_draw
check_box_mouse		dd aCheck_box_mouse
;version_ch		dd aVersion_ch

			dd 0,0

aEdit_box_draw		db 'edit_box',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
aEdit_box_set_text	db 'edit_box_set_text',0
;aVersion_ed		db 'version_ed',0

aInit_checkbox		db 'init_checkbox2',0
aCheck_box_draw		db 'check_box_draw2',0
aCheck_box_mouse	db 'check_box_mouse2',0
;aVersion_ch		db 'version_ch2',0
;---------------------------------------------------------------------
align 4
import_proc_lib:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
			dd 0,0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;---------------------------------------------------------------------
;library structures
l_libs_start:
  lib0 l_libs lib0_name, cur_dir_path, library_path, system_dir0, err_message_found_lib0, head_f_l, import_box_lib, err_message_import0, head_f_i
  lib1 l_libs lib1_name, cur_dir_path, library_path, system_dir1, err_message_found_lib1, head_f_l, import_proc_lib,err_message_import1, head_f_i
load_lib_end:

edit1 edit_box 153, 56, 1, 0xffffff, 0xff, 0x80ff, 0, 0x8000, (outfile-infile-1), infile, mouse_dd, 0, 11,11
edit2 edit_box 153, 56, 17, 0xffffff, 0xff, 0x80ff, 0, 0x8000,(path-outfile-1), outfile, mouse_dd, 0, 7,7
edit3 edit_box 153, 56, 33, 0xffffff, 0xff, 0x80ff, 0, 0x8000,(path_end-path-1), path, mouse_dd, 0, 6,6
editboxes_end:
ch1_dbg check_box2 (5 shl 16)+12, (49 shl 16)+12, 6, 0xffffff, 0x80ff, 0, s_dbgdescr,ch_flag_bottom
;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type			dd 0
.procinfo		dd pinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd path ;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd library_path ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd path ;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/rd/1',0 ;директория по умолчанию

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/rd/1/File managers/',0

Filter:
dd Filter.end - Filter
.1:
db 'ASM',0
.end:
db 0
;---------------------------------------------------------------------
mouse_dd dd 0 ;эєцэю фы  Shift-р т editbox
;---------------------------------------------------------------------
infile	  db 'example.asm'
  times MAX_PATH-$+infile  db 0
outfile db 'example'
  times MAX_PATH-$+outfile db 0
path	db '/rd/1//' ;OpenDialog при запуске убирает последний слеш, но диалог может использоваться не всегда, потому слеша 2
  times MAX_PATH-$+path    db 0
path_end:
lf db 13,10,0
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
start:
	cmp	[_mode],NORMAL_MODE
	jne	@f
	call	draw_messages
	mov	[textxy],7 shl 16 + 70
@@:
	mov	esi,_logo
	call	display_string

;---------------------------------------------------------------------
;   Fasm native code
;---------------------------------------------------------------------
	mov	[input_file],infile
	mov	[output_file],outfile
	
	call	init_memory
	
	call	make_timestamp
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
	call	make_timestamp
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
	mcall	70,file_info_start
@@:
	jmp	exit_program
;---------------------------------------------------------------------
include 'system.inc'
include 'version.inc'
include 'errors.inc'
include 'symbdump.inc'
include 'preproce.inc'
include 'parser.inc'
include 'exprpars.inc'
include 'assemble.inc'
include 'exprcalc.inc'
include 'formats.inc'
include 'x86_64.inc'
include 'avx.inc'
include 'tables.inc'
include 'messages.inc'
;---------------------------------------------------------------------
title db appname,VERSION_STRING,0

_logo db 'flat assembler  version ',VERSION_STRING,13,10,0

_passes_suffix db ' passes, ',0
_seconds_suffix db ' seconds, ',0
_bytes_suffix db ' bytes.',13,10,0

_include db 'INCLUDE',0

_counter db 4,'0000'

_mode	       dd NORMAL_MODE
_run_outfile  dd 0
;bGenerateDebugInfo db 0

sub_table:
times $41 db $00
times $1A db $20
times $25 db $00
times $10 db $20
times $30 db $00
times $10 db $50
times $04 db $00,$01
times $08 db $00

;include_debug_strings
program_end:
;  params db 0 ; 'TINYPAD.ASM,TINYPAD,/HD/1/TPAD4/',
params	rb 4096
cur_dir_path rb 4096
library_path rb 4096
filename_area rb 256

align 4

include 'variable.inc'

program_base dd ?
buffer_address dd ?
memory_setting dd ?
start_time dd ?
memblock	dd	?

predefinitions rb 1000h

dbgfilename	rb	MAX_PATH+4

sc    system_colors
max_handles = 8
fileinfos rb (4+20+MAX_PATH)*max_handles
fileinfos_end:
pinfo process_information

align 1000h
rb 1000h
stacktop:
