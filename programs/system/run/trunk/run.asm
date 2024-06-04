window_h=85
window_w=430
;--- ��㣨� ������ ---
include '../../../KOSfuncs.inc'
include '../../../load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'txtbut.inc'
include '../../../macros.inc'
include 'run.mac'
include 'lang.inc' ; Language support for locales: ru_RU (CP866), it_IT, en_US.
use32
    org 0
    db 'MENUET01'
    dd 1
    dd start
    dd i_end
    dd mem
    dd mem
    dd par
    dd cur_dir_path


;        meos_app_start
;        use_edit_box
	@use_library
	use_txt_button
;        code
;load system lib
align 4
start:
sys_load_library  library_name, library_path, system_path, myimport
	cmp	eax,-1
	jz	close

	cmp	[par],byte 0
	jne	read_par
	mcall	SF_SET_EVENTS_MASK, EVM_MOUSE+EVM_BUTTON+EVM_KEY+EVM_REDRAW+EVM_MOUSE_FILTER
red:
	mcall	SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,40
	edit_boxes_set_sys_color input_fn,input_fn_end,sc
	set_sys_colors_txt_button run_but,sc
	push	dword [sc.work_graph]
	pop	[input_fn.focus_border_color]
	call	draw_window
still:				;�᭮���� ��ࠡ��稪
	mcall	SF_WAIT_EVENT
	dec  eax
	jz   red
	dec  eax
	jz   key
	dec  eax
	jz   button
;        mouse_edit_box input_fn
	push	dword input_fn
	call	[edit_box_mouse]

	jmp still    ;�᫨ ��祣� �� ����᫥����� � ᭮�� � 横�
key:
	mcall	SF_GET_KEY
	cmp	ah,13
	je	run
;        key_edit_box input_fn
	push	dword input_fn
	call	[edit_box_key]

	jmp	still
button:
	mcall	SF_GET_BUTTON
	dec	ah
	jz	close
	dec	ah
	jz	run
	jmp	still

read_par:
	mov	esi,par
	mov	edi,fn
	mov	ecx,256
	rep	movsb
run:
	xor	eax,eax
	mov	edi,file_info.name
	mov	ecx,512
	rep	stosb
	mov	edi,run_par
	mov	ecx,256
	rep	stosb

	mov	esi,fn
	mov	edi,file_info.name
	cmp	[esi],byte '"'
	je	copy_fn_with_spaces
copy_fn:
	cmp	[esi],byte ' '
	je	.stop
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	copy_fn
.stop:

	jmp	copy_par

copy_fn_with_spaces:
	inc	esi
@@:
	cmp	[esi],byte '"'
	je	.stop
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	@b
.stop:

copy_par:
@@:
	inc	esi
	cmp	[esi],byte ' '
	je	@b
	mov	edi,run_par
@@:
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	@b
.stop:
	mcall	SF_FILE,file_info

	cmp	eax,0
	jl	error
	mov	[status],run_ok
	call	draw_status
	jmp	still
close:
	mcall SF_TERMINATE_PROCESS

error:
	neg	eax
	cmp_err 3,bad_file_sys
	cmp_err 5,file_not_find
	cmp_err 9,bad_fat_table
	cmp_err 10,acces_denyied
	cmp_err 11,device_error
	cmp_err 30,out_of_memory
	cmp_err 31,file_not_executable
	cmp_err 32,many_processes

	call	draw_status
	jmp	still

draw_window:
	mcall	SF_STYLE_SETTINGS,SSF_GET_SCREEN_AREA
	mov	si,bx

	mcall	SF_REDRAW, SSF_BEGIN_DRAW
	mcall	SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov	dx,ax
	mcall	SF_GET_SCREEN_SIZE
	xor	ecx,ecx
	sub	cx,window_h+40
	sub	cx,dx
	add	cx,si
	shl	ecx,16
	mov	cx,dx
	add	cx,window_h
	shr	eax,16
	mov	bx,ax
	sub	bx,window_w
	shl	ebx,15
	mov	bx,window_w
	mov	edx,[sc.work]
	or	edx,0x33000000
	xor	esi,esi
	mov	edi,grab_text
	mcall	SF_CREATE_WINDOW

	mcall	SF_THREAD_INFO,procinfo,-1

	mov	eax,[procinfo.box.width]
	sub	eax,20
	mov	[input_fn.width],eax
	mov	[run_but.width],ax

	; ; draw line
	; xor	bx,bx
	; shl	ebx,16
	; mov	bx,ax
	; add	bx,10
	; mov	cx,58
	; push	cx
	; shl	ecx,16
	; pop	cx
	; mov	edx,[sc.work_graph]
	; mcall	SF_DRAW_LINE

	; draw_edit_box input_fn
	push	dword input_fn
	call	[edit_box_draw]

	draw_txt_button run_but

	call	draw_status_text

	mcall	SF_REDRAW, SSF_END_DRAW
ret

draw_status:
	mov	ebx,[procinfo.box.width]
	sub	bx,10
	mov	ecx,(60)*65536+15
	mov	edx,[sc.work]
	mcall	SF_DRAW_RECT
draw_status_text:
	mov	edx,[status]
	xor	esi,esi
@@:
	cmp	[edx+esi],byte 0
	je	@f
	inc	esi
	jmp	@b
@@:
	mov	ecx,[sc.work_text]
	or  ecx,0x90000000
	mcall	SF_DRAW_TEXT,5*65536+(60)
ret

run_but txt_button 0,5,20,33,2,0,0x90000000,run_but_text,
input_fn edit_box 0,5,5,0xffffff,0x6a9480,0,0xaaaaaa,0x90000000,511,fn,mouse_dd,ed_focus+ed_always_focus
;mouse_flag: dd 0x0
input_fn_end:
if lang eq ru_RU
	hello db '������ ����� ���� � 䠩�� � ������ Enter',0
	bad_file_sys db '�������⭠� 䠩����� ��⥬�',0 ; 3
	file_not_find db '���� �� ������',0		 ; 5
	bad_fat_table db '������ FAT ࠧ��襭�',0	 ; 9
	acces_denyied db '����� ����饭',0		 ; 10
	device_error db '�訡�� ���ன�⢠',0		 ; 11
	out_of_memory db '�������筮 �����',0	 ; 30
	file_not_executable db '���� �� ���� �ᯮ��塞�',0 ; 31
	many_processes db '���誮� ����� ����ᮢ',0	 ; 32
	run_ok db '�ணࠬ�� �ᯥ譮 ����饭�',0
	grab_text db '����� �ணࠬ��',0
	run_but_text db '���������',0
else if lang eq it_IT
	hello db 'Inserisci percorso completo al file e premi <Enter>',0
	bad_file_sys db 'Filesysrem sconosciuto',0 	       ; 3
	file_not_find db 'File non trovato',0		       ; 5
	bad_fat_table db 'Tabella FAT corrotta',0	       ; 9
	acces_denyied db 'Accesso negato',0		       ; 10
	device_error db 'Device error',0		       ; 11
	out_of_memory db 'Out of memory',0		       ; 30
	file_not_executable db 'File non eseguibile',0      ; 31
	many_processes db 'Troppo processi',0	       ; 32
	run_ok db 'Il programma eseguito correttamente',0
	grab_text db 'RUN',0
	run_but_text db 'Esegui',0
else ; Default to en_US
	hello db 'Enter full path to file and press <Enter>',0
	bad_file_sys db 'Unknown file system',0 	       ; 3
	file_not_find db 'File not found',0		       ; 5
	bad_fat_table db 'FAT table corrupted',0	       ; 9
	acces_denyied db 'Access denied',0		       ; 10
	device_error db 'Device error',0		       ; 11
	out_of_memory db 'Out of memory',0		       ; 30
	file_not_executable db 'File is not executable',0      ; 31
	many_processes db 'Too many processes',0	       ; 32
	run_ok db 'The program was started successfully',0
	grab_text db 'RUN',0
	run_but_text db 'RUN',0
end if
status dd hello

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA �����
;�ᥣ�� ᮡ��� ��᫥����⥫쭮��� � �����.
system_path	 db '/sys/lib/'
library_name	 db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


myimport:

edit_box_draw	dd	aEdit_box_draw
edit_box_key	dd	aEdit_box_key
edit_box_mouse	dd	aEdit_box_mouse
version_ed	dd	aVersion_ed

		dd	0,0

aEdit_box_draw	db 'edit_box_draw',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0




file_info:
.mode dd SSF_START_APP
.flags dd 0
.par dd run_par
dd 0,0
.name rb 512

flags rw 1

sc system_colors

procinfo process_information

run_par rb 256
par rb 256
fn rb 512
mouse_dd	rd 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cur_dir_path	rb 4096
library_path	rb 4096
i_end:
rb 1024
mem:
;meos_app_end
;udata
