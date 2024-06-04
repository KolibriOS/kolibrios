;
;   Screenshooter for Kolibri
;
;---------------------------------------------------------------------
; ���� �ணࠬ�� ���客 ���ᨬ (Maxxxx32)
;
; 26.11.16 - IgorA ᭨��� ��࠭������ � �ଠ� *.png
; 02.11.10 - �ᯮ������ checkbox ���ᨨ 2
;
; version:	1.2
; last update:  08/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select path for save with OpenDialog,
;               bag fix for threads stacks
; 01.06.09 - <Lrz> ���������� �������� �� ��⥬��� ������⥪�
; 24.07.08 - <Lrz> �������� editbox
; 01.02.07 - ������� editbox
; 31.01.07 - ��� ⥯��� ������ �⭮�⥫쭮 ������᪮� ������


format binary as ""

title equ 'Screenshooter v1.21' ; ��������� ����
include '../../load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../config.inc'		;for nightbuild
include '../../proc32.inc'
include '../../macros.inc'
include '../../KOSfuncs.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include 'txtbut.inc'
include 'label.inc'
include 'textwork.inc'
include 'scrshoot.mac'

use32
    org 0
    db 'MENUET01'
    dd 1, start, IM_END, i_end, stacktop, cmdstr, cur_dir_path

include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.
include '../../dll.inc'

align 4
	@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
	use_txt_button				;  |
	use_label				;  |-- GUI ���������� � ������� ��楤���
	use_text_work				; /

include 'scrwin.inc'  ; ��⠢�塞 ��� ���� �य�ᬮ��
include 'scrsavef.inc'; ��⠢�塞 ��楤��� ��࠭���� 䠩��
include 'gp.inc'
include 'srectwin.inc'

macro get_sys_colors col_buf
{
	mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, col_buf, 40
}

;--- ��砫� �ணࠬ�� ---
align 4
start:

load_libraries l_libs_start,end_l_libs

;if return code =-1 then exit, else nornary work
;        cmp     eax,-1
	inc	eax
	test	eax,eax
	jz	close
;;;;;;;;;;;;;;;; init memory 68/11
	mcall SF_SYS_MISC, SSF_HEAP_INIT
	test	eax,eax
	jz	close

;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,start_temp_file_name
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b


	mov	edi,fname_buf
	mov	esi,ed_buffer.1
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b

;OpenDialog	initialisation
	stdcall    [OpenDialog_Init], OpenDialog_data

; prepare for PathShow
	stdcall	[PathShow_prepare], PathShow_data_1
;---------------------------------------------------------------------
	;mov     al,[gs:1280*4*1024]
		    ; ��⠭�������� ipc �����
	xor	ebx,ebx
	inc	ebx
	mcall SF_IPC,, app_ipc, 32

	; ��⠭�������� ��������� ������� �������
	mov	cl,55	 ; 55 - PrintScrn
	xor	edx,edx
	mcall SF_KEYBOARD, SSF_SET_SYS_HOTKEY

	mov	ebx,app
	call	get_slot_n
	mov	[slot_n],ecx

	; ��⠭�������� ���� ᮡ�⨩
	set_events_mask (evm_redraw+evm_key+evm_button+evm_mouse+evm_ipc)
;���樠������ checkboxes
	init_checkboxes2	check_boxes,check_boxes_end

; � ��룠�� �� ������ ����ᮢ��
red:
	get_procinfo app   ; ����砥� ���ଠ�� � �����
	get_sys_colors sc  ; ����砥� ��⥬�� 梥�

	; ��⠭�������� ��⥬�� 梥� � GUI ��������⮢
	txt_but_set_sys_color buttons,buttons_end,sc		 ; \
	labels_set_sys_color labels,labels_end,sc		 ; |
	check_boxes_set_sys_color2 check_boxes,check_boxes_end,sc; |
	edit_boxes_set_sys_color editboxes,editboxes_end,sc	 ; /

	get_screen_prop scr ; ����砥� ���ଠ�� �� �࠭�
;------------------------------------------------------------------------------
; Boot with "DIRECT" parameter - get screen and save
	mov	eax,cmdstr
	cmp	[eax],byte 0
	je	no_boot

	cmp	[eax],dword 'DIRE'
	jne	no_boot

	cmp	[eax+4],word 'CT'
	jne	no_boot

	and	[ch2.flags],dword 0
	or	[ch5.flags],dword 1
	mov	[OpenDialog_data.draw_window],dword draw_window_direct

	call	shoot
	jmp	close
;------------------------------------------------------------------------------
draw_window_direct:
	mcall SF_REDRAW, SSF_BEGIN_DRAW
	mcall SF_GET_SCREEN_SIZE
; eax = [xsize]*65536 + [ysize]
	mov	ebx,eax
	shr	ebx,17
	sub	ebx,100
	shl	ebx,16
	mov	bx,200
	mov	ecx,eax
	and	ecx,0xffff
	shr	ecx,1
	sub	ecx,50
	shl	ecx,16
	mov	cx,100
	xor	esi,esi
	mcall SF_CREATE_WINDOW,,,0x34ffffff,,grab_text

	mcall SF_DRAW_TEXT, <10,30>,0x90000000,saving

	mcall SF_REDRAW, SSF_END_DRAW
	ret
;------------------------------------------------------------------------------
no_boot:
	call	draw_window ; ����ᮢ뢠�� ����
still:
	wait_event red,key,button,mouse,ipc,still ; ���� ᮡ�⨩

key:
	get_key
	cmp	al,2
	jne	@f
	cmp	ah,55
	jne	still
	mov	[PrintScreen],1
	call	shoot
	mov	[PrintScreen],0
	jmp	still
@@:
;	stdcall	[edit_box_key], edit1
	stdcall	[edit_box_key], edit2
	stdcall	[edit_box_key], edit3

	jmp	still
button:
	get_pressed_button
	cmp	ah,1
	je	p_close
	txt_but_ev buttons,buttons_end
	jmp	still
mouse:
	get_active_window
	cmp	eax,[slot_n]
	jne	still
;----------------------------------
;	stdcall	[edit_box_mouse], edit1
	stdcall	[edit_box_mouse], edit2
	stdcall	[edit_box_mouse], edit3
;----------------------------------
	checkboxes_mouse2	check_boxes,check_boxes_end
;-----------------------------------
	jmp	still
ipc:
	cmp	word [app_ipc+8],2
	jne	@f
	min_window
	jmp	.clear_ipc
@@:
	cmp	word [app_ipc+8],3
	jne	@f
	mov	ecx,[slot_n]
	activ_window
	jmp	.clear_ipc
@@:
	call	draw_number
	call	dr_st

.clear_ipc:
	cld
	xor	eax,eax
	mov	ecx,32/4
	mov	edi,app_ipc
	rep	stosd
	jmp	still

p_close:
	btr	dword [flags],1

	bt	dword [flags],3
	jnc	@f
	mcall SF_SYSTEM, SSF_TERMINATE_THREAD_ID, [set_rect_window_pid]
@@:

close:
	app_close

;---------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall SF_DRAW_RECT, <4,302>, <35,15>, 0xffffff
; draw for PathShow
	push	dword PathShow_data_1
	call	[PathShow_draw]
	popa
	ret
;--------------------------------------------------------------------
;--- �ம楤�� ����ᮢ�� �������� ���� ---------------------------
;--------------------------------------------------------------------
draw_window:
start_draw_window	 ; ��砫� ����ᮢ��
	; ��।��塞 ����
	get_skin_height
	mov	ecx,100*65536+220
	add	cx,ax
	mov	edx,[sc.work]
	or	edx,0x34000000;0x33000000
	xor	esi,esi
	;xor     edi,edi
	mov	edi,grab_text
	mcall SF_CREATE_WINDOW, 100*65536+320

	; �뢮��� �᫠ � ����
	movsx	ecx,word [scr.width] ; �ਭ� �࠭�
	mcall SF_DRAW_NUMBER, <4,0>, , <188,[label1.top]>, [sc.work_text]
	movsx	ecx,word [scr.height]	; ���� �࠭�
	add edx, 37 shl 16
	mcall
	add edx, 36 shl 16
	mov	ebx,2 shl 16
	movsx	ecx,word [scr.bitspp]	; ��� �� ���ᥫ�
	mcall

	call	draw_number	 ; ���ᮢ뢠�� ����� ᭨���
	call	dr_st

	draw_labels labels,labels_end		 ; ��⪨
;------ show check editbox -----------
;	stdcall	[edit_box_draw], edit1
	stdcall	[edit_box_draw], edit2
	stdcall	[edit_box_draw], edit3
;------ end check all editbox -------

	call	draw_PathShow

	draw_txt_buttons buttons,buttons_end	 ; ������
;------ check all checkbox ---------

	checkboxes_draw2	check_boxes,check_boxes_end

;------ end check all checkbox ------

stop_draw_window      ; ����� ����ᮢ��
ret

shoot:
	bt	dword [ch4.flags],1   ; ����祭� �� ����প� ?
	jnc	@f
	draw_status delay_now
	mov	edi,ed_buffer.2
	call	zstr_to_int
	mov	ebx,eax
	delay
@@:
	call	get_mem_for_shoot; ����砥� ������ ��� ᭨���


	bts	dword [flags],0       ; �㫥��� 䫠� - ᭨��� ���� � �����

	; ������ ᭨���
	draw_status shooting
	mov	esi,scr
	mov	edi,[scr_buf.ptr]
	call	copy_screen_to_image

	bt	dword [use_rect.flags],1
	jc	.use_rect
	push	dword [scr]
	jmp	@f
.use_rect:
	push	dword [rect.height]
@@:
	pop	dword [scr_buf.size]

	cmp	[autoshoot_flag],1
	jne	.2

	bt	dword [ch5.flags],1  ; ����祭� �� �����࠭���� ?
	jnc	@f
	call	save_file
@@:
	draw_status	shooted_ok
.2:
	bt	dword [flags],1
	jnc	@f
	ret
@@:
	bt	dword [ch2.flags],1  ; �������� ���� �।��ᬮ�� ?
	jnc	@f
	cmp	[PrintScreen],0
	jne	@f
	call	show_scr_window
	ret
@@:
	bt	dword [ch5.flags],1  ; ����祭� �� �����࠭���� ?
	jnc	@f

	call	[OpenDialog_data.draw_window]
; invoke OpenDialog
	stdcall    [OpenDialog_Start], OpenDialog_data
	cmp	[OpenDialog_data.status],1
	je	.1
	ret
.1:
; prepare for PathShow
	stdcall	[PathShow_prepare], PathShow_data_1
	call	[OpenDialog_data.draw_window]
	call	save_file
@@:
	ret

;--- ������� ������ ��� ᭨��� ---
get_mem_for_shoot:
;clean memory
	mcall SF_SYS_MISC, SSF_MEM_FREE, [scr_buf.ptr]

	bt	dword [use_rect.flags],1 ; �⪠�� ������� �࠭�
	jc	.use_area
	movzx	ecx, word [scr.width]
	movzx	ebx, word [scr.height]
	jmp	@f
.use_area:
	call	read_rect
	movzx	ecx, word [rect.width]
	movzx	ebx, word [rect.height]
@@:
	imul	ecx,ebx
	lea	ecx,[ecx*3]
;        add     ecx,i_end
	mcall SF_SYS_MISC, SSF_MEM_ALLOC
	mov	[scr_buf.ptr],eax
	add	eax,ecx
	mov	[scr_buf.end_ptr],ecx
ret

;--- ᮤ��� ���� �।��ᬮ�� ---
show_scr_window:
pusha
	bt	dword [flags],0
	jnc	@f
	xor	ebx,ebx
	inc	ebx
	mcall SF_CREATE_THREAD,, scr_window, i_end_tread-512
@@:
popa
ret

;--- ������ � ���ᮢ��� ⥪�騩 ����� ---
apply_number:
	mov	edi,ed_buffer.3
	call	zstr_to_int
	mov	[cur_number],eax
	call	draw_number
ret

;--- ��楤�� ���ᮢ�� ⥪�饣� ����� ---
draw_number:
	mov	[sign_n],4
	mov	cx,[label9.top]
	shl	ecx,16
	mov	cx,10
	mov	edx,[sc.work]
	mcall SF_DRAW_RECT, 150*65536+96
	movsx	bx,byte [sign_n]
	shl	ebx,16
	mov	edx,150 shl 16
	mov	dx,[label9.top]
	mov	esi,[sc.work_text]
	mcall SF_DRAW_NUMBER,, [cur_number]
ret

;--- ��楤��, ����᪠��� ��⮪ ����ꥬ�� ---
start_autoshoot:
	bts	dword [flags],1
	jc	@f
	xor	ebx,ebx
	inc	ebx
	mcall SF_CREATE_THREAD,, autoshoot, i_end_tread
@@:
ret

;--- ��⠭�������� ��� ��⮪ ---
stop_autoshoot:
	btr	dword [flags],1
ret

;--- 横� ��⮪� ����ꥬ�� ---
autoshoot:
	mov	[autoshoot_flag],1
	mov	ecx,[slot_n]
	activ_window
.next:
	bt	dword [flags],1
	jnc	close
	mov	esi,2
	mcall SF_IPC, SSF_SEND_MESSAGE, [app.pid], messages.draw_number
	call	shoot
	jmp	autoshoot.next
.close:
	mov	[autoshoot_flag],0
	jmp	close
;--- ��楤�� ���ᮢ�� ��ப� ���ﭨ� ---
; (������ ��뢠���� ��⮪�� �������� ����)
dr_st:
	mcall SF_DRAW_LINE, 0*65536+310, 198*65536+198, [sc.work_graph]

	mov	bx,310
	mov	ecx,199*65536+15
	mov	edx,[sc.work]
	mcall SF_DRAW_RECT ; ����᪠

	mov	edi,status
	call	draw_label
ret

;--- ��楤�� ����祭�� ����� ᫮� ��⮪� ---
; �室 ebx - ����� 1024 ����
; ��室 ecx - ����� ᫮�
get_slot_n:
	xor	ecx,ecx
	dec	ecx
	mcall SF_THREAD_INFO

	mov	edx,[ebx+30]
	xor	ecx,ecx
@@:
	inc	ecx
	mcall SF_THREAD_INFO
	cmp	[ebx+30],edx
	je	@f
	jmp	@b
@@:
ret

;--- ��楤��, ����᪠��� ��⮪, �����騩 1 ᭨��� ---
one_shoot:
	mov	ecx,one_shoot_thread
	mov	edx,shoot_esp
	jmp	@f
;--- ��楤�, ����᪠��� ��⮪, ��࠭��騩 ᭨��� ---
save_shoot:
; invoke OpenDialog
	stdcall    [OpenDialog_Start], OpenDialog_data
	cmp	[OpenDialog_data.status],1
	je	.1
	ret
.1:
; prepare for PathShow
	stdcall	[PathShow_prepare], PathShow_data_1

	call	draw_PathShow

	mov	ecx,save_shoot_thread
	mov	edx,shoot_esp
@@:
	bts	dword [flags],2
	jc	.running
	bt	dword [flags],1
	jc	.running

	xor	ebx,ebx
	inc	ebx
	mcall SF_CREATE_THREAD
.running:
ret

;--- ��⮪, �����騩 1 ᭨��� ---
one_shoot_thread:
	mov	ecx,[slot_n]
	activ_window
	bt	dword [ch1.flags],1   ; ��������஢��� ���� ?
	jnc	 @f
	mov	esi,2
	mcall SF_IPC, SSF_SEND_MESSAGE, [app.pid], messages.min_window
@@:
	call	shoot
	btr	dword [flags],2
	jmp	close

;--- ��楤��, ��ࠢ����� �������� ���� ᮮ�饭�� � ����ᮢ��
; ��ப� ���ﭨ� ---
send_draw_status:
	mov	esi,2
	mcall SF_IPC, SSF_SEND_MESSAGE, [app.pid], messages.draw_status
ret

;--- ��⮪, ��࠭� 䠩� ---
save_shoot_thread:
	mov	ecx,[slot_n]
	activ_window
	call	save_file
	btr	dword [flags],2
	jmp	close

;--- ��楤��, ����᪠��� ��⮪ ���� ��⠭���� ������ �ꥬ�� ---
show_set_rect_window:
	bts	dword [flags],3
	jc	@f
	xor	ebx,ebx
	inc	ebx
	mcall SF_CREATE_THREAD,, set_rect_window, set_rect_window_esp

	mov	[set_rect_window_pid],eax
ret

@@:
	mcall SF_SYSTEM, SSF_TERMINATE_THREAD_ID, [set_rect_window_pid]
	btr	dword [flags],3
ret

;--- ����祭�� ���ଠ樨 �� ��⨢��� ���� ---
get_active_window_info:
	mcall SF_SYSTEM, SSF_GET_ACTIVE_WINDOW

	mov	ecx,eax
	mcall SF_THREAD_INFO, active_app
ret

;====================================================================
;=== ����� �ணࠬ�� ===============================================
;====================================================================
messages:
.draw_number dw 0
.draw_status dw 1
.min_window  dw 2
.act_window  dw 3

grab_text:
	db	title,0

labels:
label1 label 5,8,0,text.1   ; screen size and color depth
label3 label 5,25,0,text.3   ; ������ ��� 䠩��
label9 label 5,52,0,text.9   ; ����� ⥪�饣� ᨬ��
status label 5,201,0,no_shoot
labels_end:

;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, library_path, system_dir_Boxlib, \
	Box_lib_import

library02  l_libs system_dir_ProcLib+9, library_path, system_dir_ProcLib, \
	ProcLib_import

library03  l_libs system_dir_LibImg+9, library_path, system_dir_LibImg, \
	import_libimg

end_l_libs:
;---------------------------------------------------------------------
editboxes:
;edit1 edit_box 300,5,35,cl_white,0,0,0,0,300,ed_buffer.1,mouse_dd,ed_focus,10,10    ; ���� � 䠩��
edit2 edit_box 35,170,134,cl_white,0,0,0,0,9,ed_buffer.2,mouse_dd,ed_figure_only,3,3 ; ����প�
edit3 edit_box 35,165,164,cl_white,0,0,0,0,9,ed_buffer.3,mouse_dd,ed_figure_only     ; ��⮭㬥���
editboxes_end:
;---------------------------------------------------------------------
buttons:
but1 txt_button 150,5,15,65,2,0,0,but_text.1,one_shoot		   ; ᤥ���� ᭨���
but2 txt_button 34,274,15,34,3,0,0,but_text.2,save_shoot	   ; "..." - ����� ���� ��࠭����
but3 txt_button 145,160,15,65,3,0,0,but_text.3,show_scr_window    ; �������� ᭨���
but4 txt_button 80,205,15,163,5,0,0,but_text.4,apply_number	   ; �ਬ����� �����
but5 txt_button 150,5,15,85,6,0,0,but_text.5,start_autoshoot	   ; ����� �������
but6 txt_button 145,160,15,85,7,0,0,but_text.6,stop_autoshoot	   ; ��⠭����� �������
but7 txt_button 40,70,10,180,8,0,0,but_text.7,show_set_rect_window ; ������ �������
buttons_end:
;---------------------------------------------------------------------
check_boxes:
ch1 check_box2 (5 shl 16+11),(105 shl 16 +11),5,cl_white,0,0x80000000,ch_text.1,ch_flag_en+ch_flag_middle	; ᢥ���� ����
ch2 check_box2 (5 shl 16+11),(120 shl 16 +11),5,cl_white,0,0x80000000,ch_text.2,ch_flag_en+ch_flag_middle	; show screenshot
ch4 check_box2 (5 shl 16+11),(135 shl 16 +11),5,cl_white,0,0x80000000,ch_text.4,ch_flag_en+ch_flag_middle	; ����প�
ch5 check_box2 (5 shl 16+11),(150 shl 16 +11),5,cl_white,0,0x80000000,ch_text.5,ch_flag_en+ch_flag_middle
ch6 check_box2 (5 shl 16+11),(165 shl 16 +11),5,cl_white,0,0x80000000,ch_text.6,ch_flag_en+ch_flag_middle
use_rect check_box2 (5 shl 16+11),(180 shl 16 +11),5,cl_white,0,0x80000000,ch_text.7,ch_flag_middle		; ��. �������
; ��⮭㬥���
check_boxes_end:
;---------------------------------------------------------------------

if lang eq ru_RU

text:
.1 db '������ �࠭� � ��㡨�� 梥�:      �     �   bit',0
.3 db '���� ��� ��࠭���� ᭨���:',0
.9 db '����� ⥪�饣� ᭨���:',0

but_text:
.1 db '������� ᭨��� �࠭�',0
.2 db '...',0
.3 db '�������� ᭨��� ᥩ��',0
.4 db '�ਬ�����',0
.5 db '����� ����ꥬ��',0
.6 db '��⠭����� ����ꥬ��',0
.7 db '������',0

ch_text:
.1 db '������� ����',0
.2 db '�������� ᭨���',0
.4 db '����প� � �����ᥪ㭤��:',0
.5 db '�����࠭����',0
.6 db '��⮭㬥���, ��稭�� �',0
.7 db '�������',0

no_shoot db '������ �� ᤥ���',0
shooting db '��⮣��஢����...',0
shooted_ok db '������ ᤥ���',0
saving db '���࠭����...',0
saved_ok db '������ ��࠭��',0
delay_now db '����প�...',0
bad_file_name db '��� 䠩�� ������� ����୮',0
disk_filled db '��� ��������',0
bad_fat_table db '������ FAT ࠧ��襭�',0
ac_den db '����� ����饭',0
device_er db '�訡�� ���ன�⢠',0
not_shooted db '�訡��: ���砫� ᤥ���� ᭨���',0
no_file_name db '�訡��: ᫥��� ����� ��� 䠩��',0
invalid_rect db '�������⨬� ࠧ���� ������',0
keyforexit db '�� ��� ᭨���. ��室 - �� ������.',0


else ; Default to en_US

text:
.1 db 'Screen size and color depth:        �     �   bit',0
.3 db 'Screenshot save path:',0
.9 db 'Current photo number:',0

but_text:
.1 db 'Make screen photo',0
.2 db 'Save screen photo',0
.3 db 'Show photo now',0
.4 db 'Apply',0
.5 db 'Start autoshooting',0
.6 db 'Stop autoshooting',0
.7 db 'Set',0

ch_text:
.1 db 'Minimize window',0
.2 db 'Show photo',0
.4 db 'Delay in milliseconds:',0
.5 db 'Autosave',0
.6 db 'Start numeration from',0
.7 db 'Area',0

no_shoot db 'There is no photo',0
shooting db 'Photographing...',0
shooted_ok db 'Photo created',0
saving db 'Saving...',0
saved_ok db 'Photo saved',0
delay_now db 'Delay...',0
bad_file_name db 'File name is wrong',0
disk_filled db 'Disk is full',0
bad_fat_table db 'FAT table destroyed',0
ac_den db 'Access denied',0
device_er db 'Device error',0
not_shooted db 'Error: you need to make a photo first',0
no_file_name db 'Please enter file name.',0
invalid_rect db 'Wrong area size',0
keyforexit db 'This is your screenshot. Press any key.',0

end if




;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw 38	;+4
.start_x		dw 6	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 300	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname_buf	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 1	; Save
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname_buf	;+36
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
db	'PNG',0
.end:
db	0

start_temp_file_name:	db '1.png',0

;---------------------------------------------------------------------

PrintScreen	db  0
autoshoot_flag	db  0

app_ipc ipc_buffer 32
align 4

mouse_flag: dd 0x0
;---------------------------------------------------------------------
align 4

ed_buffer:
.1: db '/sys/1.png',0
;rb 287
.2:
	db '100',0
	rb 6
.3:
	rb 10
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
structure_of_potock:
rb 100
;---------------------------------------------------------------------
align 4

cur_number	  dd	  ?

sign_n	      db      ?

slot_n	      dd ?

flags	     dd ?

scr_buf:
.ptr	    dd	    ?
.end_ptr	dd	?
.size:
.height        dw      ?
.width	      dw      ?

fs_struc:
.funk_n        dd      ?
	dd	?
	dd	?
.bytes_to_write  dd	 ?
.data_ptr	 dd    ?,?
.fn_ptr        dd      ?

sf_buf:
.bmp_header   dd      ?
.bmp_area     dd      ?
.end	      dd      ?

set_rect_window_pid dd ?
set_rect_window_slot dd ?
;---------------------------------------------------------------------
align 4
rect_input_buffer:
.left rb 8
.top  rb 8
.width  rb 8
.height rb 8

cmdstr rb 257
;---------------------------------------------------------------------
align 4

file_name:
	rb	1058

scr screen_prop
rect:
.left dw ?
.top dw ?
.height dw ?
.width dw ?

sc sys_color_table
app procinfo	    ; ���ଠ�� � ������� ����
active_app procinfo ; ���ଠ�� �� ��⨢��� ����
set_rect_window_procinfo procinfo  ; ���ଠ�� �� ���� ������
;---------------------------------------------------------------------
	rb 512		   ; �⥪ ��⮪� �⪠���
shoot_esp:
;---------------------------------------------------------------------
	rb 512	   ; �⥪ ���� ������
set_rect_window_esp:
;---------------------------------------------------------------------
;        app_end    ; ����� �ணࠬ��
mouse_dd	rd 1
;---------------------------------------------------------------------
align 4
cur_dir_path   rb 4096
library_path   rb 4096
temp_dir_pach  rb 4096
text_work_area rb 1024
fname_buf      rb 4096
procinfo       rb 1024
filename_area  rb  256
;---------------------------------------------------------------------
	rb 1024
i_end_tread:
;---------------------------------------------------------------------
	rb 1024
stacktop:
;---------------------------------------------------------------------
i_end:
