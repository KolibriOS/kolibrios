; �ணࠬ�� ��� �८�ࠧ������ �ᥫ �� ��ப�
;   � �ଠ� float, double, � ⠪�� �� 10 ��� 16 �筮��
;   �� float.
; ������� �� �᭮�� hex2dec2bin � �ਬ�஢ �� 䠩�� list2_05.inc
;   (���� �㫠��� �������� ������쥢�� 24.05.2002),
;   ����� ChE ��।���� � 16 �� 32 ��� �� ��ᥬ���� fasm.
; �ணࠬ�� �������� ���� ⠪�� �����:
; 1) �᫮ � ��ப���� ���� ��ॢ��� � 4 ���� (float) � ��設�� ���
; 2) �᫮ � ��ப���� ���� ��ॢ��� � 8 ���� (double) � ��設�� ���
; 3) �᫮ � ��設��� ���� (float) ��ॢ��� � ��ப��� ��� (5 ������ ��᫥ ����⮩)

use32
    org 0
    db	'MENUET01'
    dd	1,start,i_end,e_end,e_end,0,sys_path

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../KOSfuncs.inc'
include '../../../load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/info3ds/info_fun_float.inc'
include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.

@use_library

align 4
start:
	load_libraries l_libs_start,l_libs_end
	;�஢�ઠ �� ᪮�쪮 㤠筮 ���㧨���� ��� ������⥪�
	mov	ebp,lib_0
	cmp	dword[ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mcall SF_SET_EVENTS_MASK,0xC0000027
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS, sys_colors, sizeof.system_colors
	edit_boxes_set_sys_color edit1,editboxes_end,sys_colors
	option_boxes_set_sys_color sys_colors,Option_boxes1

align 4
red:
    call draw_window

align 4
still:
    mcall SF_WAIT_EVENT

    cmp  eax,1		; ����ᮢ��� ���� ?
    je	 red		; �᫨ �� - �� ���� red
    cmp  eax,2		; ����� ������ ?
    je	 key		; �᫨ �� - �� key
    cmp  eax,3		; ����� ������ ?
    je	 button 	; �᫨ �� - �� button
    cmp  eax,6
    je	 mouse

    jmp  still		; �᫨ ��㣮� ᮡ�⨥ - � ��砫� 横��

align 4
key: ; ����� ������ �� ���������
	mcall SF_GET_KEY
	;cmp ah,13
	stdcall [edit_box_key], edit1
	jmp  still ; �������� � ��砫� 横��

align 4
button:
	mcall SF_GET_BUTTON
	cmp   ah, 1	; �᫨ �� ����� ������ � ����஬ 1,
	jne   @f
		mcall SF_TERMINATE_PROCESS
	@@:
	cmp ah, 5
	jne @f
		cmp dword[option_group1],opt3
		jne .opt_3_end
			stdcall conv_str_to_int,[edit1.text]
			mov dword[Data_Double],eax
			finit
			fld dword[Data_Double]
			fstp qword[Data_Double]

			; Data_Double - �८�ࠧ㥬�� �᫮
			mov word[NumberSymbolsAD],8 ; ������⢮ ������ �᫠ ��᫥ ����⮩ (1-17)
			call DoubleFloat_to_String
			call String_crop_0
			mov dword[Data_Double],eax ;����⠭�������� ���祭�� � �ଠ� float
			jmp .opt_all_end
		.opt_3_end:

		mov esi,string1
		mov edi,Data_String
		cld
		mov ecx,32
		rep movsb

		call String_to_DoubleFloat
		cmp dword[option_group1],opt1
		jne .opt_all_end ;�᫨ ��࠭ float, � �८�ࠧ㥬 �� ࠭�� ����祭���� double
			finit
			fld  qword[Data_Double] ;�⠥� �� double
			fstp dword[Data_Double] ;� �����頥� �� float
		.opt_all_end:
		jmp red
	@@:
	jmp still

align 4
mouse:
	stdcall [edit_box_mouse], edit1
	stdcall [option_box_mouse], Option_boxes1
	jmp still

;------------------------------------------------
align 4
draw_window:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS, sys_colors, sizeof.system_colors

	mcall SF_REDRAW,SSF_BEGIN_DRAW
	mov edx, 0x14000000
	or  edx, [sys_colors.work]
	mcall SF_CREATE_WINDOW, (200 shl 16)+300, (200 shl 16)+175, ,,title

	mcall SF_DEFINE_BUTTON, ((300-53) shl 16)+38, (145 shl 16)+15, 5, [sys_colors.work_button] ; ������ Ok

	mov ecx, 0x80000000
	or  ecx, [sys_colors.work_text]
	mcall SF_DRAW_TEXT, (15 shl 16) +30,, binstr,
	mcall  , (15 shl 16) +58,, decstr,
	mcall  , ((240-56*3) shl 16) +58,, Data_String,
	mcall  , (15 shl 16) +72,, hexstr,
	mcall  , (15 shl 16)+150,, numstr,

	mov ecx, 0x80000000
	or  ecx, [sys_colors.work_button_text]
	mcall  , ((300-42) shl 16)+149,	, Okstr,3

	cmp dword[option_group1],opt1
	je @f ;�᫨ ��࠭ float, � ���訥 4 ���� (�� double) �� ���⠥�
	cmp dword[option_group1],opt3
	je @f ;�᫨ ��࠭ float, � ���訥 4 ���� (�� double) �� ���⠥�
		mov ecx, dword[Data_Double+4]
		mcall  SF_DRAW_NUMBER, (8 shl 16)+256,,(185 shl 16)+72,[sys_colors.work_text]    ; 16-���

		mov ecx, dword[Data_Double+4]
		mcall	 ,(8 shl 16)+512,,(240 shl 16)+30,	  ; 2-���
		ror ecx, 8
		mcall	 ,,,((240-56) shl 16)+30,
		ror ecx, 8
		mcall	 ,,,((240-56*2) shl 16)+30,
		ror ecx, 8
		mcall	 ,,,((240-56*3) shl 16)+30,
		ror ecx, 8
	@@:

	mov ecx,dword[Data_Double]
	mcall  SF_DRAW_NUMBER, (8 shl 16)+256,,(240 shl 16)+72,[sys_colors.work_text]	 ; 16-���

	mov ecx,dword[Data_Double]
	mcall	 , (8 shl 16)+512,,(240 shl 16)+44,	  ; 2-���
	ror ecx, 8
	mcall	 ,,,((240-56) shl 16)+44,
	ror ecx, 8
	mcall	 ,,,((240-56*2) shl 16)+44,
	ror ecx, 8
	mcall	 ,,,((240-56*3) shl 16)+44,
	ror ecx, 8

	mcall SF_DRAW_LINE, (15 shl 16)+300-15, (137 shl 16)+137, [sys_colors.work_graph]
	stdcall [edit_box_draw], edit1
	stdcall [option_box_draw], Option_boxes1
	mcall SF_REDRAW,SSF_END_DRAW

ret


string1 db 32 dup (0)
string1_end:


;input:
; buf - 㪠��⥫� �� ��ப�, �᫮ ������ ���� � 10 ��� 16 �筮� ����
;output:
; eax - �᫮
align 4
proc conv_str_to_int uses ebx ecx esi, buf:dword
	xor eax,eax
	xor ebx,ebx
	mov esi,[buf]
	;��।������ ����⥫��� �ᥫ
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;���뢠��� 10-���� ���
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;���뢠��� 16-���� ���
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;��ᥨ���� ᨬ���� >'9' � <'A'
		.us1: ;��⠢��� �᫮���
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;��ᥨ���� ᨬ���� >'F' � <'a'
			sub bl,32 ;��ॢ���� ᨬ���� � ���孨� ॣ����, ��� ��饭�� �� ��᫥��饩 ��ࠡ�⪨
		.us2: ;��⠢��� �᫮���
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:
	cmp ecx,0 ;�᫨ �᫮ ����⥫쭮�
	jne @f
		sub ecx,eax
		mov eax,ecx
	@@:
	ret
endp

;-------------------------------------------------
title db 'string to double 03.01.21',0
hexstr db 'hex:',0
decstr db 'dec:',0
binstr db 'bin:',0

if lang eq ru_RU
	numstr db '��᫮:',0
	Okstr db '����',0
else ; Default to en_US
	numstr db 'Number:',0
	Okstr db 'Ok',0
end if

mouse_dd dd 0
edit1 edit_box 182, 59, 146, 0xffffff, 0xff, 0x80ff, 0, 0x8000, (string1_end-string1), string1, mouse_dd, 0
editboxes_end:

;option_boxes
opt1 option_box option_group1, 15,  90, 8, 12, 0xffffff, 0x80ff, 0, op_text.1, 17
opt2 option_box option_group1, 15, 106, 8, 12, 0xffffff, 0x80ff, 0, op_text.2, 18
opt3 option_box option_group1, 15, 122, 8, 12, 0xffffff, 0x80ff, 0, op_text.3, 21

op_text: ;⥪�� ��� ࠤ�� ������
  .1 db 'str(dec) -> float'
  .2 db 'str(dec) -> double'
  .3 db 'float(dec,hex) -> str'
;㪠��⥫� ��� option_box
option_group1 dd opt1
Option_boxes1 dd opt1, opt2, opt3, 0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'box_lib.obj',0


l_libs_start:
	lib_0 l_libs lib_name_0, library_path, system_dir_0,import_box_lib
l_libs_end:

align 4
import_box_lib:
	;dd sz_init1
	edit_box_draw dd sz_edit_box_draw
	edit_box_key dd sz_edit_box_key
	edit_box_mouse dd sz_edit_box_mouse
	;edit_box_set_text dd sz_edit_box_set_text
	option_box_draw dd aOption_box_draw
	option_box_mouse dd aOption_box_mouse
	;version_op dd aVersion_op
dd 0,0
	;sz_init1 db 'lib_init',0
	sz_edit_box_draw db 'edit_box_draw',0
	sz_edit_box_key db 'edit_box_key',0
	sz_edit_box_mouse db 'edit_box_mouse',0
	;sz_edit_box_set_text db 'edit_box_set_text',0
	aOption_box_draw db 'option_box_draw',0
	aOption_box_mouse db 'option_box_mouse',0
	;aVersion_op db 'version_op',0

i_end:
	sys_colors system_colors
align 16
	sys_path rb 4096
	library_path rb 4096
	rb 0x400 ;stack
e_end: ; ��⪠ ���� �ணࠬ��
