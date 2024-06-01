;--------------------------------------------------------------------
; v.016 30.07.2011
; Start with open file path
; Show working file path
; Some optimization
;
; Marat Zakiyanov aka Mario79, aka Mario
;--------------------------------------------------------------------
; v.015 08.03.2010
; Support for OpenDialog - Open and Save
; Some optimization
;
; Marat Zakiyanov aka Mario79, aka Mario
;--------------------------------------------------------------------
; v.014 05.02.2010
;
; PageUp, PageDown      - ��࠭�� �����/����
; Ctrl+UP, Ctrl+Down    - �ப��⪠ ��࠭��� �� ��ப� �����/���� ��� ᬥ饭�� �����
; Home,End              - � ��砫�/����� ��ப�
; Ctrl+Home, Ctrl+End   - � ��ࢮ��/��᫥����� ����� 䠩��
; Left, Right           - ����� �����/��ࠢ�
; Ctrl+O                - ������ 䠩�
; Ctrl+S                - ��࠭���
; Ctrl+F                - ���� (+Tab ��� OptionBox)
; Ctrl+G                - ���室 �� ᬥ饭�� (+Tab ��� OptionBox)
; Ctrl+B                - �뤥���� ����
; ESC                   - ���� �뤥�����
; Ctrl+C		- ����஢��� ����
; Ctrl+V		- ��⠢��� � �뤥������ �������
; Ctrl+X		- ��१��� �뤥������ ������� (� ����)
; n                     - ������஢��� ���� ��� ����஬
; Ins                   - ०�� ������/��⠢�� (�� 㬮�砭��)
;   Del                 - � ०��� ��⠢�� - 㤠���� ���� ��� ����஬
;   BackSpace           - � ०��� ��⠢�� - 㤠���� ���� ��। ����஬
; ~                     - ᬥ�� ����஢�� (cp866,cp1251)
; Shift+~               - ᬥ�� ����஢�� (cp866 ��� cp1251,koi8-r)

; ������ �����।������ �� 㢥��祭�� i umen'shenie.
; ���� ����㦠���� 楫����.

; ������ load_lib.mac � ������⥪� box_lib.obj ᮧ����:
; <Lrz> - Alexey Teplov / ����ᥩ ������
; Mario79, Mario - Marat Zakiyanov / ���� ����ﭮ�
; Diamondz - Evgeny Grechnikov / ������� ��筨��� � ��.
;
; staper@inbox.ru
; babalbes@yandex.ru
;--------------------------------------------------------------------
use32
	org	0
	db	'MENUET01'
	dd	1
	dd	START	;program start
	dd	I_END	;program image	size
	dd	(D_END+0x600) and not 3	;required amount of memory
	dd	(D_END+0x600) and not 3	;stack
	dd	fname_buf
	dd	cur_dir_path
;--------------------------------------------------------------------
_title	equ 'HeEd 0.16', 0

include	'lang.inc'      ; Language support for locales: ru_RU (CP866), en_US.
include '../../../KOSfuncs.inc'
include '../../../config.inc'		; for nightbuild
include '../../../macros.inc'
include	'../../libraries/box_lib/trunk/box_lib.mac'
include	'../../../load_lib.mac'

@use_library
;--------------------------------------------------------------------
times	16	dd	0

frgrd_color	equ	0xfefefe
bkgrd_color	equ	0x000000
kursred_color	equ	0x0039ff
kurstxt_color	equ	0x708090
text_color	equ	0xaaaaaa

panel_clr1	equ	0xe9e9e2
panel_clr2	equ	0x8b8b89
panel_clr3	equ	0x777777;eaeae3
;--------------------------------------------------------------------
palitra:
.1	dd	frgrd_color,bkgrd_color	;梥� ���뤥������� ᨬ����
.2	dd	frgrd_color,text_color	;����,�ࠢ� �⮫���,���� ������ ��ப�
.3	dd	kursred_color,frgrd_color	;�����
.4	dd	kurstxt_color,bkgrd_color	;����� � ⥪�⮢�� ������
.5	dd	panel_clr1,not	text_color	;������ ������
;--------------------------------------------------------------------
FIRST_HEX equ 0*65536+24
scroll_width_size equ 15
;--------------------------------------------------------------------
struct	f70
	Function	rd 1
	Position	rd 1
	Flags	rd 1
	Count	rd 1
	Buffer	rd 1
	rezerv	rb 1
	FileName	rd 1
ends
;--------------------------------------------------------------------
START:
	mcall	SF_SYS_MISC,SSF_HEAP_INIT

load_libraries l_libs_start,end_l_libs
;--------------------------------------------------------------------
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]
;--------------------------------------------------------------------
	mov	esi,fname_buf
	cmp	[esi],byte 0
	je	.start_temp_file_name

	cld
@@:
	lodsb
	test	al,al
	jne	@b

	std
@@:
	lodsb
	cmp	al,'/'
	jne	@b
	add	esi,2
	jmp	.selected_start_file_name
;--------------------------------------------------------------------
.start_temp_file_name:
	mov	esi,start_temp_file_name
.selected_start_file_name:
	mov	edi,filename_area
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b

	mcall	SF_SET_EVENTS_MASK,0x27

	mcall	SF_SYS_MISC,SSF_MEM_ALLOC,32*1024	;��࠭��� ����
	mov	[screen_table],eax
	mcall	SF_SYS_MISC,SSF_MEM_ALLOC,4*1024

	mov	[file_buffer],eax

	mcall	SF_SYS_MISC,SSF_MEM_ALLOC,1024
	mov	[copy_buf],eax


	;ࠧ���	⥪�饣� ���
	mov	esi,cur_dir_path
@@:
	cmp	byte [esi],0
	je	@f
	inc	esi
	jmp	@b
;-------------------------------------
@@:
	sub	esi,cur_dir_path
	mov	[edit1.pos],esi
	mov	[edit1.size],esi

	;�����⥬�� ������ ��� Shift+������
;	mcall	SF_KEYBOARD,SSF_SET_SYS_HOTKEY,75,1
;	mcall	SF_KEYBOARD,,77
;	mcall	SF_KEYBOARD,,72
;	mcall	SF_KEYBOARD,,80

	call	ready_screen_buffer
;	jmp	open_file
	mcall	SF_THREAD_INFO,procinfo,-1
	mov	ecx,[ebx+30]	; PID
	mcall	SF_SYSTEM,SSF_GET_THREAD_SLOT
	mov	[active_process],eax	; WINDOW SLOT
;--------------------------------------------------------------------
; open the file if program has a file path, when it was launched
	cmp	[fname_buf],byte 0
	jne	open_dialog.start
;--------------------------------------------------------------------
redraw_all:
	call	control_minimal_window_size
	call	draw_window_1
still:
	mcall	SF_WAIT_EVENT

	cmp	eax,6
	je	mouse
	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jz	button
	jmp	still
;---------------------------------------------------------------------
control_minimal_window_size:
	pusha
	mcall	SF_THREAD_INFO,procinfo,-1
	mov	eax,[ebx+70]
	test	eax,10b
	jnz	.end
	test	eax,100b
	jnz	.end
	test	eax,1b
	jnz	.end
	mov	esi,-1
	mov	eax,procinfo
	mov	eax,[eax+46]
	cmp	eax,299
	jae	@f
	mov	esi,299
	mcall	SF_CHANGE_WINDOW,-1,ebx,ebx
@@:
	mov	edx,-1
	mov	eax,procinfo
	mov	eax,[eax+42]
	cmp	eax,399
	jae	@f
	mov	edx,399
	mcall	SF_CHANGE_WINDOW,-1,ebx,,ebx
@@:
.end:
	popa
	ret
;---------------------------------------------------------------------
red:
	call	ready_screen_buffer
	call	main_area
	jmp	still
;---------------------------------------------------------------------
draw_window_1:
	call	start_draw
	call	draw_window

	mov     eax,[threath_buf+70]
	test	eax,10b
	jnz     .end
	test	eax,100b
	jnz     .end

	call	show_file_size
	call	show_codepage
	call	show_insert
	call	ready_screen_buffer
	call	main_area
.end:
	ret
;---------------------------------------------------------------------
key:
	mcall	SF_GET_KEY
	dec	al
	jz	still
	dec	al
	jz	key.syst
	cmp	ah,2
	je	Ctrl_B	;�뤥���� ����
	cmp	ah,3
	je	Ctrl_C	;copy
	cmp	ah,6
	je	Ctrl_F	;find
	cmp	ah,7
	je	Ctrl_G	;go to
	cmp	ah,8
	je	BackSpace
	cmp	ah,15
	je	open_dialog ;open_file ;Ctrl+O
	cmp	ah,19
	je	open_dialog_save ;save_file ;Ctrl+S
	cmp	ah,22
	je	Ctrl_V	;paste
	cmp	ah,24
	je	Ctrl_X	;cut
	cmp	ah,27
	je	ESC
	cmp	ah,48
	jb	still
	cmp	ah,57
	jbe	input_from_keyboard	;0-9
	cmp	ah,65
	jb	still
	cmp	ah,70
	jbe	input_from_keyboard	;A-F
	cmp	ah,81
	jne	@f
	call	Ctrl_DOWN
	jmp	red
;--------------------------------------
@@:
	cmp	ah,82
	jne	@f
	call	Ctrl_UP
	jmp	red
;--------------------------------------
@@:
	cmp	ah,84
	jne	@f
	call	Ctrl_HOME
	jmp	red
;--------------------------------------
@@:
	cmp	ah,85
	je	Ctrl_END
	cmp	ah,96
	je	change_codepage	;⨫줠, cp866 - cp1251
	cmp	ah,97
	jb	still
	cmp	ah,102
	jbe	input_from_keyboard ;a-f
	cmp	ah,126
	jne	@f
	xor	ah,ah
	jmp	change_codepage	;Shift+~, koi8-r
;--------------------------------------
@@:
	cmp	ah,110
	je	invert_byte ;n
	cmp	ah,176
	jne	@f
	call	LEFT
	jmp	red
;--------------------------------------
@@:
	cmp	ah,177
	jne	@f
	call	DOWN
	jmp	red
;--------------------------------------
@@:
	cmp	ah,178
	je	UP
	cmp	ah,179
	je	RIGHT
	cmp	ah,180
	jne	@f
	call	HOME
	jmp	red
;--------------------------------------
@@:
	cmp	ah,181
	je	END_
	cmp	ah,182
	je	DEL
	cmp	ah,183
	je	PGDN
	cmp	ah,184
	je	PGUP
	cmp	ah,185
	je	Insert
	jmp	still
;--------------------------------------
.syst:
	cmp	ah,72
	cmp	ah,75
	cmp	ah,77
	cmp	ah,80
	jmp	still
;--------------------------------------------------------------------
button:
	mcall	SF_GET_BUTTON
	dec	ah
	jnz	still

	jmp	close_prog
;--------------------------------------------------------------------
align	4
mouse:
	mcall	SF_SYSTEM,SSF_GET_ACTIVE_WINDOW
	cmp	[active_process],eax
	jne	still

	mcall	SF_MOUSE_GET,SSF_SCROLL_DATA
	test	eax,eax
	jz	.menu_bar_1;.mouse
	bt	eax,15
	jc	@f	;��ࠡ�⪠ ����ᨪ� ���
	mov	ecx,eax
	shl	ecx,2
.1:
	call	Ctrl_DOWN
	loop	.1
	jmp	red
;--------------------------------------
@@:
	xor	ecx,ecx
	sub	cx,ax
	shl	cx,2
.2:
	call	Ctrl_UP
	loop	.2
	jmp	red
;--------------------------------------------------------------------
.menu_bar_1:
	call	.set_mouse_flag
@@:
	push	dword menu_data_1	;mouse event for Menu 1
	call	[menu_bar_mouse]
	cmp	[menu_data_1.click],dword 1
	jne	.menu_bar_2
	cmp	[menu_data_1.cursor_out],dword 0
	jne	.analyse_out_menu_1
	jmp	.menu_bar_1
;--------------------------------------------------------------------
.menu_bar_2:
	push	dword menu_data_2
	call	[menu_bar_mouse]
	cmp	[menu_data_2.click],dword 1
	jne	.menu_bar_3
	cmp	[menu_data_2.cursor_out],dword 0
	jne	.analyse_out_menu_2
	jmp	.menu_bar_1
;--------------------------------------------------------------------
.menu_bar_3:
	push	dword menu_data_3
	call	[menu_bar_mouse]
	cmp	[menu_data_3.click],dword 1
	jne	.scroll_bar
	cmp	[menu_data_3.cursor_out],dword 0
	jne	.analyse_out_menu_3
	jmp	.menu_bar_1
;--------------------------------------------------------------------
.set_mouse_flag:
	xor	eax,eax
	inc	eax
	mov	[menu_data_1.get_mouse_flag],eax
	mov	[menu_data_2.get_mouse_flag],eax
	mov	[menu_data_3.get_mouse_flag],eax
	ret
;--------------------------------------------------------------------
.analyse_out_menu_1:
	cmp	[menu_data_1.cursor_out],dword 1
	je	open_dialog
	cmp	[menu_data_1.cursor_out],dword 2
	je	open_dialog_save	;save_file
	cmp	[menu_data_1.cursor_out],dword 3
	je	close_prog
	jmp	still
;--------------------------------------------------------------------
.analyse_out_menu_2:
	cmp	[menu_data_2.cursor_out],dword 1
	jne	@f
	add	[bytes_per_line],4
	jmp	redraw_all
;--------------------------------------
@@:
	cmp	[menu_data_2.cursor_out],dword 2
	jne	@f
	add	[bytes_per_line],8
	jmp	redraw_all
;--------------------------------------
@@:
	cmp	[menu_data_2.cursor_out],dword 3
	jne	@f
	cmp	[bytes_per_line],4
	je	still
	sub	[bytes_per_line],4
	jmp	redraw_all
;--------------------------------------
@@:
	cmp	[menu_data_2.cursor_out],dword 4
	jne	still
	cmp	[bytes_per_line],8
	jbe	still
	sub	[bytes_per_line],8
	jmp	redraw_all
;--------------------------------------------------------------------
.analyse_out_menu_3:	;analyse result of Menu 2
	cmp	[menu_data_3.cursor_out],dword 1
	jne	still
	call	create_help_window
	jmp	still
;--------------------------------------------------------------------
.scroll_bar:
;	mcall	SF_MOUSE_GET,SSF_BUTTON
;	test	eax,eax
;	jnz	@f
;	btr	[flags],5
;	btr	[flags],7
;	jmp	still

.mouse:
.vertical:
	mcall	SF_MOUSE_GET,SSF_BUTTON
	test	eax,eax
	jnz	@f
	btr	[flags],5
	btr	[flags],7
	jmp	still
;--------------------------------------
@@:
	bt	[flags],5
	jc	@f
	bt	[flags],7
	jc	.horizontal_0

	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
	shr	eax,16
	cmp	ax,[scroll_bar_data_vertical.x_pos]
	jb	.horizontal
	sub	ax,[scroll_bar_data_vertical.x_pos]
	cmp	ax,[scroll_bar_data_vertical.x_size]
	jge	still


@@:
;	cmp	[scroll_bar_data_vertictal.delta2],0
	;	jne	.horizontal

;	mov	eax,[scroll_bar_data_vertical.max_area]
;	cmp	eax,[scroll_bar_data_vertical.cur_area]
;	jbe	.horizontal

	;	mouse event for Vertical ScrollBar
	mov	edi,[screen_table]
	mov	edi,[edi]
	cmp	edi,[file_size]
	jae	still
	push	dword scroll_bar_data_vertical	;draw for Vertical ScrollBar
	call	[scrollbar_ver_mouse]

	xor	edx,edx
	mov	ebx,[scroll_bar_data_vertical.max_area]
	sub	ebx,[scroll_bar_data_vertical.cur_area]
	mov	eax,[file_size]
	mov	esi,[bytes_per_line]
	mov	ecx,[scroll_bar_data_vertical.position]
	div	esi
	xor	edx,edx
	mul	ecx
	div	ebx
	mul	esi
	cmp	eax,[file_size]
	jng	@f
	sub	eax,esi;[bytes_per_line]
@@:
	mov	ecx,[cursor]
	inc	ecx
	shr	ecx,1
	add	ecx,eax
@@:
	cmp	ecx,[file_size]
	jle	@f
	sub	ecx,esi;[bytes_per_line]
	sub	eax,esi;[bytes_per_line]
	jmp	@b
;--------------------------------------
@@:
	mov	[begin_offset],eax
	bts	[flags],5

	mov	eax,scroll_bar_data_vertical.redraw
	xor	ebx,ebx
;	cmp	[eax],ebx
;	je	@f
	mov	[eax],ebx
	jmp	red
;--------------------------------------------------------------------
;@@:
;	cmp	[scroll_bar_data_vertical.delta2],0
;	jne	still
.horizontal:
	mov	eax,[scroll_bar_data_horizontal.max_area]
	cmp	eax,[scroll_bar_data_horizontal.cur_area]
	jbe	.other

	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
	cmp	ax,[scroll_bar_data_horizontal.y_pos]
	jb	still
	sub	ax,[scroll_bar_data_horizontal.y_pos]
	cmp	ax,[scroll_bar_data_horizontal.y_size]
	jge	still

	; mouse event for Horizontal ScrollBar
.horizontal_0:
;	mcall	SF_MOUSE_GET,SSF_BUTTON
;	test	eax,eax
;	jnz	@f
;	btr	[flags],5
;	btr	[flags],7
;	jmp	still
;@@:;	bt	[flags],7
;	jc	@f

;	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
;	shr	eax,16
;	cmp	ax,[scroll_bar_data_vertical.x_pos]
;	jb	.horizontal
;	sub	ax,[scroll_bar_data_vertical.x_pos]
;	cmp	ax,[scroll_bar_data_vertical.x_size]
;	jge	still
;@@:
	; mouse event for Vertical ScrollBar

	mov	eax,[ready_screen_buffer.string_size]
	shr	eax,1
	cmp	eax,[len_str_scr]
	jle	still
	push	dword scroll_bar_data_horizontal	;draw for Horizontal ScrollBar
	call	[scrollbar_hor_mouse]

	xor	edx,edx
	mov	eax,[scroll_bar_data_horizontal.position]
	mov	ebx,[ready_screen_buffer.string_size]
	shr	ebx,1
	mul	ebx
	mov	ebx,[scroll_bar_data_horizontal.max_area]
	sub	ebx,[scroll_bar_data_horizontal.cur_area]
	div	ebx
	mov	[beg_str_scr],eax
	add	eax,[len_str_scr]
	mov	ebx,[ready_screen_buffer.string_size]
	shr	ebx,1
	cmp	eax,ebx
	jle	@f
	mov	eax,[ready_screen_buffer.string_size]
	shr	eax,1
	sub	eax,[len_str_scr]
	mov	[beg_str_scr],eax
@@:
	mov	eax,scroll_bar_data_horizontal.redraw
	xor	ebx,ebx
	cmp	[eax],ebx
	je	.other
	mov	[eax],ebx
;	call	draw_window
	call	main_area
	bts	[flags],7
	jmp	still
;--------------------------------------------------------------------
.other:
;	cmp	[scroll_bar_data_vertical.delta2],0
;	jne	still
;	cmp	[scroll_bar_data_horizontal.delta2],0
;	jne	still
	jmp	still
;--------------------------------------------------------------------
align	4
ready_screen_buffer:
	mov	[.fl],0
	mov	esi,[screen_table]
	add	esi,4
	mov	ecx,[number_strings]
	mov	[.number_strings],cx
	push	[cursor]
	pop	[.cursor]
	push	[file_size]
	pop	[.length_to_end]
	mov	edi,[begin_offset]

	mov	[.shblock_end],0
	mov	[.shblock_beg],0
	bt	[flags],8
	jnc	@f
	mov	eax,[shblock_beg]
	add	eax,[file_buffer]
	mov	[.shblock_beg],eax
	mov	eax,[shblock_end]
	add	eax,[file_buffer]
	mov	[.shblock_end],eax
@@:
	mov	[.offset],edi
	sub	[.length_to_end],edi
	jz	.pre_next_string
	add	edi,[file_buffer]

.next_string:
	mov	word [esi],0x2020
	add	esi,2
	mov	eax,0x20302030
	mov	dword [esi],eax
	mov	dword [esi+4],eax
	mov	dword [esi+8],eax
	mov	dword [esi+12],eax
	push	edi
	mov	ecx,8
	mov	edi,[.offset]
@@:
	mov	eax,edi
	and	eax,0xF
	cmp	al,10
	sbb	al,69h
	das
	mov	[esi+ecx*2-2],al
	shr	edi,4
	loop	@b
	mov	word [esi+16],0x203a	;�����稥
	mov	eax,[bytes_per_line]
	add	[.offset],eax
	mov	[.string_size],2+4*4+4+2
	add	esi,4*4+4+2
	pop	edi
	mov	[.temp],edi
	push	[.cursor]
	pop	[.cursor_temp]

	;hex	���祭��
	mov	dword [esi-4],0x20202020

	mov	edx,[bytes_per_line]
.next_group:
	mov	ecx,[group_bytes]

.next_byte:
	mov	al,[edi]
	ror	ax,4
	cmp	al,10
	sbb	al,69h
	das
	mov	[esi],al
	mov	byte [esi+1],0x20
	cmp	edi,[.shblock_end]
	ja	@f
	cmp	edi,[.shblock_beg]
	jl	@f
	mov	byte [esi+1],(0x20+3*4)
@@:
	dec	[.cursor]
	jnz	@f
	mov	byte [esi+1],(0x20+2*4)
@@:
	shr	ax,16-4
	cmp	al,10
	sbb	al,69h
	das
	mov	[esi+2],al
	mov	byte [esi+3],0x20
	cmp	edi,[.shblock_end]
	ja	@f
	cmp	edi,[.shblock_beg]
	jl	@f
	mov	byte [esi+3],(0x20+3*4)
@@:
	dec	[.cursor]
	jnz	@f
	mov	byte [esi+3],(0x20+2*4)
@@:
	mov	word [esi+4],0x2020
	add	[.string_size],6
	add	esi,6
	inc	edi
	dec	[.length_to_end]
	jz	.to_null
	dec	ecx
	jz	@f
	dec	edx
	jnz	.next_byte
	jz	.ascii
@@:
	mov	word [esi],0x2020
	add	esi,2
	add	[.string_size],2
	dec	edx
	jnz	.next_group;byte
	sub	esi,2
	sub	[.string_size],2
	jz	.ascii
.ascii:
	push	edi
	mov	edi,[.temp]
	mov	word [esi],0x2020
	mov	ebx,[.cursor_temp]
	add	esi,2
	add	[.string_size],2
	mov	ecx,[bytes_per_line]
@@:
	mov	al,[edi]
	mov	[esi],al
	mov	byte [esi+1],0x20
	dec	ebx
	jnz	.s1
	mov	byte [esi+1],(0x20+3*4)
.s1:
	dec	ebx
	jnz	.s2
	mov	byte [esi+1],(0x20+3*4)
.s2:
	inc	edi
	add	esi,2
	add	[.string_size],2
	loop	@b
	pop	edi
	mov	edx,[bytes_per_line]
	dec	[.number_strings]
	jnz	.next_string
	ret
;---------------------------------------------------------------------
.fl	db 0
.fl_buf	dd 0
.to_null:
	dec	[.fl]
	mov	[.fl_buf],edi
	jmp	.pre_next_byte0
;--------------------------------------------------------------------
.pre_next_string:
	mov	word [esi],0x2020
	add	esi,2
	mov	eax,0x20302030
	mov	dword [esi],eax
	mov	dword [esi+4],eax
	mov	dword [esi+8],eax
	mov	dword [esi+12],eax
	mov	ecx,8
	mov	edi,[.offset]
@@:
	mov	eax,edi
	and	eax,0xF
	cmp	al,10
	sbb	al,69h
	das
	mov	[esi+ecx*2-2],al
	shr	edi,4
	loop	@b
	mov	word [esi+16],0x203a	;�����稥
	mov	eax,[bytes_per_line]
	add	[.offset],eax
	mov	[.string_size],2+4*4+4+2
	add	esi,4*4+4+2
	push	[.cursor]
	pop	[.cursor_temp]

	;hex ���祭��
	mov	dword [esi-4],0x20202020

	mov	edx,[bytes_per_line]
.pre_next_group:
	mov	ecx,[group_bytes]

.pre_next_byte:
	mov	dword [esi],0x20202020
	dec	[.cursor]
	jnz	@f
	mov	byte [esi+1],(0x20+2*4)
@@:
	dec	[.cursor]
	jnz	@f
	mov	byte [esi+3],(0x20+2*4)
@@:
	mov	word [esi+4],0x2020
	add	[.string_size],6
	add	esi,6
.pre_next_byte0:
	dec	ecx
	jz	@f
	dec	edx
	jnz	.pre_next_byte
	jz	.pre_ascii
@@:
	mov	word [esi],0x2020
	add	esi,2
	add	[.string_size],2
	dec	edx
	jnz	.pre_next_group;byte
	sub	esi,2
	sub	[.string_size],2
	jz	.pre_ascii

.pre_ascii:
	mov	word [esi],0x2020
	add	esi,2
	add	[.string_size],2
	mov	ecx,[bytes_per_line]
	cmp	[.fl],0	;�஢�ઠ 䫠��
	jne	.last_ascii
	mov	ebx,[.cursor_temp]
@@:
	mov	word [esi],0x2020
	dec	ebx
	jnz	.ps1
	mov	byte [esi+1],(0x20+3*4)
.ps1:
	dec	ebx
	jnz	.ps2
	mov	byte [esi+1],(0x20+3*4)
.ps2:
	add	esi,2
	add	[.string_size],2
.1:
	loop	@b
	mov	edx,[bytes_per_line]
	dec	[.number_strings]
	jnz	.pre_next_string
	ret
;---------------------------------------------------------------------
;���४⭮ ��ࠡ��뢠�� 䨭����� ��ப� 䠩��, ����� ���ன ��ਠ⨢��
.last_ascii:
	mov	ebx,[.fl_buf]
	sub	ebx,[.temp]
	sub	ecx,ebx
	mov	edi,[.temp]
	mov	[.fl],0
@@:
	mov	al,[edi]
	mov	[esi],al
	mov	byte [esi+1],0x20
	dec	[.cursor_temp]
	jnz	.la1
	mov	byte [esi+1],(0x20+3*4)
.la1:
	dec	[.cursor_temp]
	jnz	.la2
	mov	byte [esi+1],(0x20+3*4)
.la2:
	inc	edi
	add	esi,2
	add	[.string_size],2
	dec	ebx
	jnz	@b
	inc	ecx
	jmp	.1
;--------------------------------------------------------------------
.string_size	dd 16
.number_strings	dw 0
.length_to_end	dd 0
.temp		dd 0
.offset		dd 0
.cursor		dd 0
.cursor_temp	dd 0
.shblock_beg	dd 0
.shblock_end	dd 0
;--------------------------------------------------------------------
align	4
main_area:
	mov	ecx,[number_strings]
	mov	[.number_strings],cx
	push	[len_str_scr]
	pop	[.len_str_scr]
	mov	eax,[ready_screen_buffer.string_size]
	shr	eax,1
	mov	[.string_size],eax
	mov	edi,[screen_table]
	mov	edi,[edi]
	cmp	[file_size],edi
	jbe	.4
	xor	edx,edx	;����㭮�
	mov	ebx,[scroll_bar_data_vertical.max_area]
	sub	ebx,[scroll_bar_data_vertical.cur_area]
	mov	ecx,[file_size]
	mov	eax,[current_offset]
	test	eax,eax
	jnz	.3
	inc	eax
.3:
	mul	ebx
	test	ecx,ecx
	jnz	.5
	inc	ecx
.5:
	div	ecx
	mov	[scroll_bar_data_vertical.position],eax

	mcall	SF_MOUSE_GET,SSF_BUTTON	;������	��� ����� - ��� ��᫠ ����ᮢ뢠�� ScrollBar
	test	eax,eax
	jnz	.4
	push	dword scroll_bar_data_vertical
	call	[scrollbar_ver_mouse]

	push	dword scroll_bar_data_vertical	;draw for Vertical ScrollBar
	call	[scrollbar_ver_draw]

;	push	dword scroll_bar_data_vertical	;draw for Vertical ScrollBar
;	call	[scrollbar_ver_draw]

;	xor	eax,eax
;	inc	eax
;	mov	[scroll_bar_data_vertical.all_redraw],eax
;	push	dword scroll_bar_data_vertical	;draw for Vertical ScrollBar
;	call	[scrollbar_ver_draw]
;	xor	eax,eax	;reset	all_redraw	flag
;	mov	[scroll_bar_data_vertical.all_redraw],eax

.4:
	mov	esi,0x000001	;梥� � �᫮ ��� �� ���ᥫ�
	mov	edx,FIRST_HEX	;���न���� ��ࢮ�� hex
	call	show_current_offset
	mov	edi,[screen_table]
	add	edi,4
	mov	ecx,[beg_str_scr]
	sub	[.string_size],ecx
	shl	ecx,1
	add	edi,ecx
.out:
	push	edi
	movzx	ebx,byte [edi]
	cmp	bl,128	;�஢�ઠ �� �ਭ���������� ᨬ���� � ���७��� ⠡���
	jb	.7
	add	ebx,[codepage_offset]
.7:
	movzx	edi,byte [edi+1]
	lea	edi,[(edi*2-0x40)]
	add	edi,palitra
	shl	bx,4
	add	ebx,font_buffer
	mov	ecx,8*65536+16
	mov	ebp,0
	mcall	SF_PUT_IMAGE_EXT
	pop	edi

	add	edi,2
	add	edx,8*65536

	dec	[.len_str_scr]
	jz	.loop_str
	dec	[.string_size]
	jz	.next_string

	push	edi
	movzx	ebx,byte [edi]
	cmp	bl,128
	jb	.8
	add	ebx,[codepage_offset]
.8:
	movzx	edi,byte [edi+1]
	lea	edi,[(edi*2-0x40)]
	add	edi,palitra
	shl	bx,4
	add	ebx,font_buffer
	mcall
	pop	edi

	add	edi,2
	add	edx,8*65536

	dec	[.len_str_scr]
	jz	.loop_str
	dec	[.string_size]
	jnz	.out
	jz	.next_string	; WTF?
;--------------------------------------------------------------------
.string_size	dd 0
.number_strings	dw 0
.len_str_scr	dd 0
;--------------------------------------------------------------------
@@:
	pushad
	mov	ecx,edx
	shl	ecx,16
	mov	ebx,edx
	shr	ebx,16
	cmp	bx,[scroll_bar_data_vertical.x_pos]
	jge	.ls1
	mov	ax,[scroll_bar_data_vertical.x_pos]
	sub	ax,bx
	shl	ebx,16
	mov	bx,ax
	mov	cx,16
	mcall	SF_DRAW_RECT,,,frgrd_color
.ls1:
	popad
	jmp	@f
;--------------------------------------------------------------------
.loop_str:
	bt	[flags],6
	jc	@b
@@:
	dec	[.string_size]
	jz	.next_string
	add	edi,2
	add	edx,8*65536
	jmp	@b
;--------------------------------------
@@:;���⪠ 䮭��� 梥⮬ ������襭��� �����⥩
	pushad
	ror	edx,16
	mov	dx,16
	mov	ecx,edx
	mov	edx,frgrd_color
	movzx	ebx,[scroll_bar_data_vertical.x_pos]

	mov	ax,[scroll_bar_data_vertical.x_size]
	test	ax,ax
	jnz	.no_inc_ebx
	inc	ebx
.no_inc_ebx:
	sub	ecx,2*65536
	mov	cx,2
	mcall	SF_DRAW_RECT
	popad

	pushad
	mov	ecx,edx
	shl	ecx,16
	mov	ebx,edx
	shr	ebx,16
	cmp	bx,[scroll_bar_data_vertical.x_pos]
	jge	.10
	mov	ax,[scroll_bar_data_vertical.x_pos]
	sub	ax,bx
	shl	ebx,16
	mov	bx,ax
	mov	cx,16

	mov	ax,[scroll_bar_data_vertical.x_size]
	test	ax,ax
	jnz	.no_inc_ebx_2
	inc	ebx
.no_inc_ebx_2:
	mcall	SF_DRAW_RECT,,,frgrd_color
.10:
	popad
	jmp	@f
;--------------------------------------------------------------------
.next_string:
	bt	[flags],6
	jc	@b
@@:
	push	[len_str_scr]
	pop	[.len_str_scr]
	mov	eax,[ready_screen_buffer.string_size]
	shr	eax,1
	mov	[.string_size],eax	;���४�� ᬥ饭�� �室��� ������
	mov	ecx,[beg_str_scr]
	sub	[.string_size],ecx
	sub	eax,ecx
	shl	ecx,1
	add	edi,ecx

	shl	eax,19
	sub	edx,eax
	add	edx,18
	dec	[.number_strings]
	jnz	.out
	btr	[flags],6
	jmp	end_draw
;--------------------------------------------------------------------
align	4
show_current_offset:
	pushad
	push	edx	;�뢮� ⥪�饣� ᬥ饭�� � 䠩��
	mov	edi,palitra.5
	mov	eax,[begin_offset]
	mov	ebx,[cursor]
	dec	ebx
	shr	bx,1
	add	ebx,eax
	mov	[current_offset],ebx
	mov	edx,[low_area]
	lea	eax,[8*8+8+8]
	shl	eax,16
	add	edx,eax
	mov	ecx,8
	call	hex_output
	lea	eax,[8*8+14]
	shl	eax,16
	add	edx,eax
	push	edx
	;����筮� ���祭�� ����
	mov	edx,[file_buffer]
	add	edx,ebx;[current_offset]
	xor	eax,eax
	cmp	ebx,[file_size]
	jae	@f
	mov	al,[edx]
@@:
	mov	bx,2
	mov	ebp,8
	xor	ecx,ecx
	xor	edx,edx
@@:
	div	bx
	or	cl,dl
	ror	ecx,4
	dec	ebp
	jnz	@b
	mov	ebx,ecx
	pop	edx
	mov	ecx,8
	call	hex_output

	;�����筮�
	push	edx
	mov	edx,[file_buffer]
	mov	ebx,[current_offset]
	add	edx,ebx
	xor	eax,eax
	inc	ebx
	cmp	ebx,[file_size]	;0 �᫨ �� �࠭�楩 䠩��
	jb	@f
	mov	edx,D_END
@@:
	mov	al,[edx]
	xor	ebx,ebx
;	mov	ebp,3
	mov	cl,10
@@:
	div	cl
	mov	bl,ah
	xor	ah,ah
	shl	ebx,8
	test	al,al
;	dec	ebp
	jnz	@b
	shr	ebx,8
	cmp	byte [edx],100
	jb	.1
	mov	ebp,3
	jmp	@f
;--------------------------------------
.1:
	mov	ebp,1
	cmp	byte [edx],10
	jb	@f
	mov	ebp,2
@@:
	mov	al,bl
	shr	ebx,8
	cmp	al,10
	sbb	al,69h
	das
	shl	eax,8
;	test	bx,bx
	dec	ebp
	jnz	@b

	mov	ecx,8*65536+16
	pop	edx
	add	edx,(8*8+30)*65536;268*65536
	mov	edi,palitra.5
	mov	ebp,0
	push	dword 3
@@:
	shr	eax,8
	xor	ebx,ebx
	mov	bl,al
	shl	ebx,4
	add	ebx,font_buffer
	push	eax
	mcall	SF_PUT_IMAGE_EXT
	pop	eax
	sub	edx,8*65536
	dec	dword [esp]
	jnz	@b
	add	esp,4
;	mov	edx,[low_area]	;�뢮� esp
;	add	edx,298*65536
;	mov	ebx,esp
;	mov	ecx,8
;	call	hex_output
	pop	edx
	popad
	ret
;---------------------------------------------------------------------
align	4
hex_output:	;�뢮� hex ��ப� �� 8 ᨬ�����
	pushad
	mov	edi,(hex8_string)	;���� ����
	mov	dword [edi],0x30303030
	mov	dword [edi+4],0x30303030
	push	ecx
.1:
	mov	eax,ebx
	and	eax,0xF
	cmp	al,10
	sbb	al,69h
	das
	mov	[edi+ecx-1],al
	shr	ebx,4
	loop	.1
	mov	ecx,8*65536+16
.2:
	push	edi
	xor	ebx,ebx
	mov	al,[edi]
	shl	eax,4
	add	eax,font_buffer
	xchg	eax,ebx
	mov	edi,palitra.5
	mov	ebp,0
	mcall	SF_PUT_IMAGE_EXT
	add	edx,8*65536
	pop	edi
	inc	edi
	dec	dword [esp]
	jnz	.2
	add	esp,4
	popad
	ret
;------------------------------------------------------
align	4
input_from_keyboard:
	xor	al,al
	sub	ah,48
	cmp	ah,9
	jle	.1
	sub	ah,7
	cmp	ah,15
	jle	.1
	sub	ah,32
.1:
	bt	[flags],1
	jnc	.2
	mov	ebx,[cursor]
	and	bl,1
	jz	.2
	inc	[file_size]
	call	raspred_mem
	mov	edi,[current_offset]
	add	edi,[file_buffer]
	mov	esi,[file_buffer]
	add	esi,[file_size]
	dec	esi
@@:
	cmp	edi,esi
	ja	@f
	mov	bl,[esi]
	mov	[esi+1],bl
	dec	esi
	jmp	@b
;--------------------------------------
@@:
	call	show_file_size
	mov	ebx,[current_offset]
	add	ebx,[file_buffer]
	mov	byte [ebx],0
.2:
	mov	ecx,[current_offset]
	add	ecx,[file_buffer]
	;�.	�����	�����	heed.asm
	mov	dl,[ecx]	;�ਣ������ ����
	mov	ebx,[cursor]
	and	bl,1	;���� - ।����㥬 ���訩 ���㡠��
	jnz	.hi_half_byte ;��� - ���訩
	and	dl,0xf0	;����塞 ��. �-���� �ਣ����쭮�� ����
	jmp	.patch_byte
;--------------------------------------
.hi_half_byte:	;�����६���� ᤢ����� �㦭�� ���祭�� � �� �-� � ����塞 ����訩
	shl	ax,4
	and	dl,0x0f	;����塞 ���訩 ���㡠�� � �ਣ����쭮�� ����
.patch_byte:
	or	ah,dl
	mov	[ecx],ah
	jmp	RIGHT
;--------------------------------------------------------------------
raspred_mem:
	pushad
	xor	edx,edx
	mov	ecx,4096
	mov	eax,[file_size]
	inc	eax
	div	ecx
	cmp	eax,[prev_f_size_bl]
	ja	@f
	je	.ret
	cmp	[file_size],4096
	jbe	.ret
	mov	[prev_f_size_bl],eax
	xor	edx,edx
	mul	ecx
	mov	ecx,eax
	jmp	.1
;--------------------------------------
@@:
	mov	[prev_f_size_bl],eax
	xor	edx,edx
	mul	ecx
	add	ecx,eax
.1:
	mcall	SF_SYS_MISC,SSF_MEM_REALLOC,,[file_buffer]
.ret:
	popad
	ret
;--------------------------------------------------------------------
align	4
show_file_size:
	mov	ebx,[file_size]
	mov	edx,[low_area];
	mov	esi,1
	mov	ecx,8
	call	hex_output
	ret
;---------------------------------------------------------------------
align	4
create_title:
	mov	edi,title_buf
	mov	esi,title
	cld
@@:
	lodsb
	stosb
	test	al,al
	jne	@b
	mov	[edi-1],byte ' '
	mov	esi,fname_buf
@@:
	lodsb
	stosb
	test	al,al
	jne	@b
	ret
;---------------------------------------------------------------------
align	4
draw_window:
	call	create_title
	xor	esi,esi
	mcall	SF_CREATE_WINDOW,100*65536+653,100*65536+360,((0x73 shl 24) + frgrd_color),,title_buf	;title
	mcall	SF_THREAD_INFO,threath_buf,-1
;	cmp	byte [threath_buf+70],3	;���� ����� � ���������?
;	jnae	@f
	mov	eax,[threath_buf+70]
	test	eax,10b
	jnz	.@d
	test	eax,100b
	jz	@f
.@d:
	call	end_draw
	ret
;--------------------------------------
@@:
	cmp	dword [threath_buf+66],(24*4)	;�஢�ઠ �������쭮� �����
	jae	@f
	mov	esi,dword [threath_buf+46]
	sub	esi,dword [threath_buf+66]
	add	esi,24*4
	mcall	SF_CHANGE_WINDOW,-1,-1,-1,
	jmp	.@d
;--------------------------------------
@@:
	cmp	dword [threath_buf+62],(26*6)	;�஢�ઠ �������쭮� �ਭ�
	jae	@f
	mov	edx,dword [threath_buf+42]
	sub	edx,dword [threath_buf+62]
	add	edx,26*6
	mcall	SF_CHANGE_WINDOW,-1,-1,,-1
	jmp	.@d
;--------------------------------------
@@:
	mov	eax,[file_size]
	mov	ebx,[bytes_per_line]
	xor	edx,edx
	div	ebx
	mov	[scroll_bar_data_vertical.x_size],0
	cmp	eax,[number_strings]
	jl	@f
	mov	[scroll_bar_data_vertical.x_size],scroll_width_size
@@:
	mov	eax,dword [threath_buf+62]	;�ਭ� ������᪮� ������
	sub	ax,[scroll_bar_data_vertical.x_size]
	mov	[scroll_bar_data_vertical.x_pos],ax
	mov	eax,dword [threath_buf+66]	;���� ������᪮� ������
	sub	eax,24+24-11
	mov	[scroll_bar_data_vertical.y_size],ax
	mov	ebx,eax
	push	eax
	add	ebx,20
	mov	[scroll_bar_data_vertical.max_area],ebx
	mov	ebx,[scroll_bar_data_vertical.btn_height]
	shl	ebx,1
	add	ebx,20
	mov	[scroll_bar_data_vertical.cur_area],ebx
	pop	eax
	sub	eax,3
	mov	ebx,18
	xor	edx,edx
	div	bx
	mov	[number_strings],eax	;���-�� hex ��ப � ����
	mov	ebx,[bytes_per_line]
	mul	ebx
	mov	edi,[screen_table]	;���-�� ���⮢ ��� �뢮��
	mov	dword [edi],eax

	push	eax

	mov	ebx,dword [threath_buf+62]
	inc	ebx
	mov	ecx,(FIRST_HEX-18)
	ror	ecx,16
	mov	cx,18
	ror	ecx,16
	mcall	SF_DRAW_RECT,,,frgrd_color	;����� ᢥ���

	mcall	,,18,panel_clr1	;������ ������

	dec	ebx
	mcall	SF_DRAW_LINE,,<18,18>,panel_clr2
	mov	ecx,dword [threath_buf+66]
	sub	cx,18
	push	cx
	shl	ecx,16
	pop	cx
	mcall	,,,panel_clr3	;������ ������
	inc	ebx
	add	ecx,1*65536
	mov	cx,18
	mcall	SF_DRAW_RECT,,,panel_clr1


	mov	eax,dword [threath_buf+62]
	sub	eax,scroll_width_size
	shr	eax,3	;div 8
	mov	[len_str_scr],eax


	mov	eax,[len_str_scr]
	shl	eax,1
	cmp	eax,[ready_screen_buffer.string_size]
	jae	@f

	mov	edi,[screen_table]
	mov	eax,[bytes_per_line]
	sub	dword [edi],eax
	dec	[number_strings]

@@:
	shr	ecx,16
	mov	edx,ecx
	mov	ecx,(FIRST_HEX)
	shr	ecx,16
	mov	eax,[number_strings]
	lea	ebx,[eax*8];*18
	lea	ebx,[ebx*2]
	lea	eax,[eax*2]
	add	eax,ebx
	add	cx,ax
	add	cx,21
	sub	dx,cx
	shl	ecx,16
	add	cx,dx
	sub	ecx,1*65536
	movzx	ebx,	word [scroll_bar_data_vertical.x_pos]
	inc	ebx
	mcall	SF_DRAW_RECT,,,frgrd_color

	pop	eax

	cmp	eax,[file_size]
	jge	@f
;	push	dword scroll_bar_data_vertical
;	call	[scrollbar_ver_mouse]
	xor	eax,eax
	inc	eax
	mov	[scroll_bar_data_vertical.all_redraw],eax
	push	dword scroll_bar_data_vertical	;draw for Vertical ScrollBar
	call	[scrollbar_ver_draw]
	xor	eax,eax	;reset all_redraw flag
	mov	[scroll_bar_data_vertical.all_redraw],eax
@@:
	push	dword menu_data_1	;draw for Menu 1
	call	[menu_bar_draw]
	push	dword menu_data_2	;draw for Menu 2
	call	[menu_bar_draw]
	push	dword menu_data_3	;draw for Menu 3
	call	[menu_bar_draw]

;;;

	mov	eax,dword [threath_buf+66]
	add	eax,8*65536-15
	mov	[low_area],eax

	mov	ebx,[beg_str_scr]
	mov	[beg_str_scr],0
	mov	eax,[len_str_scr]
	shl	eax,1
	cmp	eax,[ready_screen_buffer.string_size]
	jae	@f

	mov	[beg_str_scr],ebx
	movzx	eax,word [threath_buf+66]
	sub	eax,34
	mov	[scroll_bar_data_horizontal.y_pos],ax

;cur_area/(x_size-30)=len_str_scr/string_size

	mov	eax,dword [threath_buf+62]
	sub	ax,[scroll_bar_data_vertical.x_size]
	mov	[scroll_bar_data_horizontal.x_size],ax
	sub	eax,[scroll_bar_data_horizontal.btn_height]
	sub	eax,[scroll_bar_data_horizontal.btn_height]
	mov	[scroll_bar_data_horizontal.max_area],eax
	xor	edx,edx
	mov	ebx,[len_str_scr]
	mul	ebx
	mov	ebx,[ready_screen_buffer.string_size]
	shr	ebx,1
	div	ebx
	mov	[scroll_bar_data_horizontal.cur_area],eax

	push	dword scroll_bar_data_horizontal	;draw for Horizontal ScrollBar
	call	[scrollbar_hor_mouse]
	xor	eax,eax
	inc	eax
	mov	[scroll_bar_data_horizontal.all_redraw],eax
	push	dword scroll_bar_data_horizontal	;draw for Vertical ScrollBar
	call	[scrollbar_hor_draw]
	xor	eax,eax	;reset all_redraw flag
	mov	[scroll_bar_data_horizontal.all_redraw],eax
@@:
	mov	eax,[low_area]
	mov	edi,[screen_table]
	mov	esi,[bytes_per_line]
	mov	ecx,esi
	shl	ecx,1
	mov	eax,[edi]
	mov	ebx,[cursor]
	inc	ebx
	shr	ebx,1
@@:
	cmp	eax,ebx
	jge	@f
	add	[begin_offset],esi
	sub	[cursor],ecx
	sub	ebx,esi
	jmp	@b
;--------------------------------------
@@:
	bts	[flags],6
	ret
;--------------------------------------------------------------------
align	4
start_draw:
	mcall	SF_REDRAW,SSF_BEGIN_DRAW
	ret
;--------------------------------------------------------------------
end_draw:
	mcall	SF_REDRAW,SSF_END_DRAW
	ret
;--------------------------------------------------------------------
close_prog:
	mcall	SF_TERMINATE_PROCESS
;--------------------------------------------------------------------
change_codepage:	;���塞 ����� �������� ⠡����
	test	ah,ah
	jnz	@f
	btc	[flags],4
	jc	.1
	push	[codepage_offset]
	pop	[codepage_offset_previous]
	mov	[codepage_offset],2*128
	jmp	.end
;--------------------------------------
.1:
	push	[codepage_offset_previous]
	pop	[codepage_offset]
	jmp	.end
;--------------------------------------
@@:
	cmp	[codepage_offset],0
	jne	@f
	add	[codepage_offset],128
	jmp	.end
;--------------------------------------
@@:
	mov	[codepage_offset],0
.end:
	call	show_codepage
	jmp	red
;--------------------------------------------------------------------
show_codepage:
	mov	ebp,6
	mov	edx,dword [threath_buf+62]
	sub	edx,73
	shl	edx,16
	add	edx,[low_area]
	mov	edi,string_cp866
	cmp	[codepage_offset],0
	je	@f
	add	edi,6
	cmp	[codepage_offset],128
	je	@f
	add	edi,6
@@:
	mov	ecx,8*65536+16
	mov	esi,1
	push	ebp
	mov	ebp,0
@@:
	xor	ebx,ebx
	push	edi
	mov	bl,[edi]
	shl	bx,4
	add	ebx,font_buffer
	mov	edi,palitra.5
	mcall	SF_PUT_IMAGE_EXT
	add	edx,8*65536
	pop	edi
	inc	edi
	dec	dword [esp]
	jnz	@b
	add	esp,4
	ret
;--------------------------------------------------------------------
show_insert:	;�⮡ࠦ���� ०��� ��⠢��/������
	mov	ebp,3
	mov	edx,dword [threath_buf+62]
	sub	edx,120
	shl	edx,16	; mov edx,428*65536+335
	add	edx,[low_area]
	mov	edi,string_ins
	push	ebp
	mov	ecx,8*65536+16
	mov	esi,1
	mov	ebp,0
.1:
	xor	ebx,ebx
	push	edi
	bt	[flags],1
	jnc	.2
	mov	bl,[edi]
	shl	bx,4
.2:
	add	ebx,font_buffer
	mov	edi,palitra.5
	mcall	SF_PUT_IMAGE_EXT
	add	edx,8*65536
	pop	edi
	inc	edi
	dec	dword [esp]
	jnz	.1
	add	esp,4
	ret
;-------------------------------------------------------------------------------
	;help window
create_help_window:
	pushad
        cmp	[help_is_open_already], 1
        jne	@f
  	mov     ecx, [help_window_pid]
        mcall   SF_SYSTEM, SSF_GET_THREAD_SLOT
        xchg    eax, ecx
        mcall   SF_SYSTEM, SSF_FOCUS_WINDOW
	popad
        ret
;---------------------------------------------------------------------
@@:
	mcall	SF_CREATE_THREAD,1,.thread,(.threat_stack+16*4)
        mov     [help_is_open_already], 1
        mov     [help_window_pid], eax
	popad
	ret
;--------------------------------------------------------------------
.thread:
	call	.window
;--------------------------------------------------------------------
.still:
	mcall	SF_WAIT_EVENT
	dec	al
	jz	.red
	dec	al
	jz	.key
	dec	al
	jz	.button
	jmp	.still
;--------------------------------------------------------------------
        and	[help_is_open_already], 0
	mcall	SF_TERMINATE_PROCESS
.button:
	mcall	SF_GET_BUTTON
	cmp	ah,1
	jne	@f
        and	[help_is_open_already], 0
	mcall	SF_TERMINATE_PROCESS
@@:
	cmp	ah,2
	jne	@f
	mov	edi,(help_end-help_text)/51
	movzx	eax,[cur_help_string]
	sub	edi,13
	sub	edi,eax
	jz	.still
	inc	[cur_help_string]
	jmp	.red
;--------------------------------------
@@:
	cmp	ah,3
	jne	.still
	cmp	[cur_help_string],0
	je	.still
	dec	[cur_help_string]
	jmp	.red
;--------------------------------------------------------------------
.key:
	mcall	SF_GET_KEY
	jmp	.still
;--------------------------------------------------------------------
.red:
	call	.window
	jmp	.still
;--------------------------------------------------------------------
.window:
	pushad
	mcall	SF_REDRAW,SSF_BEGIN_DRAW
	mcall	SF_CREATE_WINDOW,50*65536+320,0x70*65536+240,0x13000000,,help_but_text
	mcall	SF_DEFINE_BUTTON,<130,20>,<6,12>,2,0xaaaaaa
	mcall	,<150,20>,,3,
	mov	ebx,8*65536+15
	mov	ecx,0x00DDDDDD
	xor	edx,edx
	movzx	eax,byte [cur_help_string]
	mov	edi,(help_end-help_text)/51
	sub	edi,eax
	mov	esi,51
	mul	si
	mov	edx,help_text
	add	edx,eax
	mov	eax,SF_DRAW_TEXT
@@:
	add	ebx,0x10
	mcall
	add	edx,51
	dec	edi
	jnz	@b
	mcall	SF_REDRAW,SSF_END_DRAW
	popad
	ret
;--------------------------------------------------------------------
.threat_stack:	times	16	dd	0
;--------------------------------------------------------------------
open_file:
	mov	[func_70.Function],SSF_GET_INFO
	mov	[func_70.Position],0
	mov	[func_70.Flags],0
	mov	[func_70.Count],0
	mov	[func_70.Buffer],bufferfinfo
	mov	[func_70.rezerv],0
	mov	[func_70.FileName],file_name
	mcall	SF_FILE,func_70

	test	al,al	;䠩� ������?
	jz	@f
	mcall	SF_DRAW_TEXT,400*65536+31,0x80CC0000,error_open_file_string
	jmp	open_file
;--------------------------------------------------------------------
@@:
	mov	eax,	dword [bufferfinfo+32]	;�����㥬 ࠧ��� 䠩��
	mov	[file_size],eax

	mcall	SF_SYS_MISC,SSF_MEM_FREE,[file_buffer]
	test	eax,eax
	jnz	@f
	;����� �訡�� �� �� �᢮�������� �����
@@:
	mov ecx,[file_size]
	or ecx,ecx
	jnz @f
	inc ecx ;�᫨ ࠧ��� 䠩�� 0 ����
@@:
	mcall	SF_SYS_MISC,SSF_MEM_ALLOC
	mov	[file_buffer],eax

;;����� ⠡����: [ DWORD 㪠��⥫� �� ���� ����� ����� : DWORD ����� ����� ]

	mov	[func_70.Function],SSF_READ_FILE
	mov	[func_70.Position],0
	mov	[func_70.Flags],0
	mov	[func_70.rezerv],0
	mov	[func_70.FileName],file_name
	push	dword [file_size];dword [edi+4]
	pop	dword [func_70.Count]
	push	dword [file_buffer];dword [edi]
	pop	dword [func_70.Buffer]
	mcall	SF_FILE,func_70

	test	eax,eax
	jz	@f
	;�訡�� �⥭��
@@:
	call	Ctrl_HOME

	jmp	redraw_all
;-------------------------------------------------------------------------------
open_dialog_save:
	mov	[OpenDialog_data.type],1	; Save

	push    dword OpenDialog_data
	call    [OpenDialog_Start]

;	cmp	[OpenDialog_data.status],2	; OpenDialog does not start
	cmp	[OpenDialog_data.status],1
	jne	still
	mov	esi,fname_buf
	mov	edi,file_name
	cld
@@:
	cmp	byte [esi],0
	je	@f
	movsb
	jmp	@b
;--------------------------------------
@@:
	mov	byte [edi],0
	sub	esi,path
	mov	[edit1.size],esi
	mov	[edit1.pos],esi
;	jmp	save_file
;-------------------------------------------------------------------------------
save_file:	;��࠭塞 䠩�
	mov	[func_70.Function],SSF_CREATE_FILE
	mov	[func_70.Position],0
	mov	[func_70.Flags],0
	push	[file_size]
	pop	[func_70.Count]
	push	[file_buffer]
	pop	[func_70.Buffer]
	mov	[func_70.rezerv],0
	mov	[func_70.FileName],file_name
	mcall	SF_FILE,func_70
	cmp	al,0	;��࠭� 㤠筮?
	je	redraw_all
	mcall	SF_DRAW_TEXT,400*65536+31,0x80CC0000,error_save_file_string
	jmp	save_file
;-------------------------------------------------------------------------------
draw_ed_box:	;�ᮢ���� edit box'�
.1:
	push	eax	ebx	ecx	edx
	mcall	SF_DRAW_RECT,180*65536+220,25*65536+70,0xaaaaaa
	bt	[flags],9
	jnc	@f
	mcall	SF_DRAW_TEXT,246*65536+35,0x80ffffff,sel_text
@@:
	bt	[flags],2
	jnc	@f
	push	dword Option_boxs
	call	[option_box_draw]
@@:
	bt	[flags],3
	jnc	@f
	push	dword Option_boxs2
	call	[option_box_draw]
@@:
	mov	eax,ed_box_data
	mov	ecx,[eax]
@@:
	add	eax,4
	push	dword [eax]
	call	[edit_box_draw]
	loop	@b
	pop	edx	ecx	ebx	eax
.2:
	mcall	SF_WAIT_EVENT
	cmp	al,6
	je	.mouse
	cmp	al,3
	je	.button
	cmp	al,2
	je	.keys
	cmp	al,1
	jne	.2
	call	draw_window

	mov     eax,[threath_buf+70]
	test    eax,10b
	jnz     .2
	test    eax,100b
	jnz     .2

	call	main_area
	bt	[flags],2
	jnc	@f
	push	dword Option_boxs
	call	[option_box_draw]
@@:
	bt	[flags],3
	jnc	@f
	push	dword Option_boxs2
	call	[option_box_draw]
@@:
	jmp	.1
;--------------------------------------------------------------------
.mouse:
	bt	[flags],2
	jnc	@f
	push	dword Option_boxs
	call	[option_box_mouse]
@@:
	bt	[flags],3
	jnc	@f
	push	dword Option_boxs2
	call	[option_box_mouse]
@@:
	jmp	.2
;--------------------------------------------------------------------
.keys:
	mcall	SF_GET_KEY
	cmp	ah,13
	je	.4
	cmp	ah,27
	je	.3

	bt	[flags],2	;�஢�ઠ �� �ਬ�������� ᨬ����� 0-9,a-b
	jnc	.eb2
.eb1:
	cmp	ah,9
	jne	.eb1_1
	push	edx
	mov	edx,[option_group1]
	cmp	edx,op1
	jne	@f
	mov	edx,op2
	jmp	.eb1_2
;--------------------------------------
@@:
	cmp	edx,op2
	jne	@f
	mov	edx,op3
	jmp	.eb1_2
;--------------------------------------
@@:
	mov	edx,op1
.eb1_2:
	mov	[option_group1],edx
	pop	edx
	jmp	.1
;--------------------------------------
.eb1_1:
	cmp	ah,48
	jb	.eb1_3
	cmp	ah,57
	jbe	.eb
	cmp	ah,102
	jg	.eb1_3
	cmp	ah,97
	jge	.eb
.eb1_3:
	cmp	ah,182
	je	.eb
	cmp	ah,8
	je	.eb
	cmp	ah,176
	je	.eb
	cmp	ah,179
	je	.eb
	dec	[edit2.shift]
	dec	[edit2.shift+4]

	push	dword [ed_box_data+4];	[esp]
	call	[edit_box_draw]
	jmp	.2
;--------------------------------------
.eb2:
	bt	[flags],3
	jnc	.eb3
	cmp	ah,9
	jne	.eb2_2
	push	edx
	mov	edx,[option_group2]
	cmp	edx,op11
	jne	@f
	mov	edx,op12
	jmp	.eb2_1
;--------------------------------------
@@:
	mov	edx,op11
.eb2_1:
	mov	[option_group2],edx
	pop	edx
	jmp	.1
;--------------------------------------
.eb2_2:
	cmp	ah,182
	je	.eb
	cmp	ah,8
	je	.eb
	cmp	ah,176
	je	.eb
	cmp	ah,179
	je	.eb
	mov	edx,[option_group2]
	cmp	edx,op11
	jne	.eb
	cmp	ah,48
	jb	.eb2_3
	cmp	ah,57
	jbe	.eb
	cmp	ah,102
	jg	.eb2_3
	cmp	ah,97
	jge	.eb
.eb2_3:
	dec	[edit3.shift]
	dec	[edit3.shift+4]
	push	dword [ed_box_data+4];[esp]
	call	[edit_box_draw]
	jmp	.2
;--------------------------------------
.eb3:
	bt	[flags],9
	jnc	.eb
	cmp	ah,9
	je	.eb3_1
	cmp	ah,182
	je	.eb3_2
	cmp	ah,8
	je	.eb3_2
	cmp	ah,176
	je	.eb3_2
	cmp	ah,179
	je	.eb3_2
	cmp	ah,48
	jb	.eb3_3
	cmp	ah,57
	jbe	.eb3_2
	cmp	ah,102
	jg	.eb3_3
	cmp	ah,97
	jge	.eb3_2
.eb3_3:
	push	edx
	mov	edx,[edit4.flags]
	and	edx,2
	jz	@f
	pop	edx
	dec	[edit4.shift]
	dec	[edit4.shift+4]
	jmp	.2
;--------------------------------------
@@:
	pop	edx
	dec	[edit5.shift]
	dec	[edit5.shift+4]
	jmp	.2
;--------------------------------------
.eb3_1:
	push	edx
	mov	edx,[edit4.flags]
	and	edx,2
	jz	@f
	pop	edx
	mov	[edit5.flags],2
	mov	[edit4.flags],0
	jmp	.eb3_2
;--------------------------------------
@@:
	pop	edx
	mov	[edit4.flags],2
	mov	[edit5.flags],0
.eb3_2:
	push	dword [ed_box_data+4]
	call	[edit_box_key]
	push	dword [ed_box_data+8]
	call	[edit_box_key]
	jmp	.1
;--------------------------------------
.eb:
	push	dword [ed_box_data+4];[esp]
	call	[edit_box_key]
	jmp	.2
;--------------------------------------------------------------------
.button:
	mcall	SF_GET_BUTTON
	cmp	ah,1
	jne	.2
	jmp	close_prog
.3:
	btr	[flags],2
	btr	[flags],3
	add	esp,4
	jmp	redraw_all
.4:
	mcall	SF_DRAW_RECT,180*65536+220,25*65536+70,frgrd_color
	ret
;--------------------------------------------------------------------
strtohex:
;enter: edi - pointer to string,ebx - pointer to size of string; exit: eax in hex
	mov	esi,hex8_string
@@:
	mov	ah,[edi+ecx-1]	;��ࠡ�⪠ ������� ᨬ�����
	sub	ah,48
	cmp	ah,9
	jbe	.1
	sub	ah,7
	cmp	ah,15
	jbe	.1
	sub	ah,32
.1:
	mov	[esi+ecx-1],ah
	dec	ecx
	jnz	@b
	mov	ecx,[ebx]
	xor	eax,eax
.2:
	shl	eax,4
	or	al,[esi]
	inc	esi
	dec	ecx
	jnz	.2
	ret

Ctrl_G:
	bts	[flags],2
	mov	dword [ed_box_data],1
	mov	dword [ed_box_data+4],edit2
	call	draw_ed_box
	btr	[flags],2
	mov	ecx,[edit2.size]
	test	ecx,ecx
	jz	.end
	cmp	ecx,8
	jg	Ctrl_G

	mov	edi,go_to_string
	mov	ebx,edit2.size
	call	strtohex

	cmp	eax,[file_size]	;�롮� check_box'�
	jg	Ctrl_G
	mov	edx,[option_group1]
	cmp	edx,op1	;abs
	je	.abs
	cmp	edx,op2
	jne	.back
	add	eax,[current_offset]	;forward
	cmp	eax,[file_size]
	jg	Ctrl_G
	mov	edi,[screen_table]
	mov	edi,[edi]
	xor	edx,edx
@@:
	add	edx,edi
	cmp	eax,edx
	jg	@b
	sub	edx,edi
	mov	[begin_offset],edx
	sub	eax,edx
	shl	eax,1
	inc	eax
	mov	[cursor],eax
	jmp	.end
;--------------------------------------------------------------------
.back:
	cmp	eax,[current_offset]	;back
	jg	Ctrl_G
	mov	edi,[screen_table]
	mov	edi,[edi]
	mov	ebx,[current_offset]
	sub	ebx,eax
	xor	edx,edx
@@:
	add	edx,edi
	cmp	edx,ebx
	jb	@b
	sub	edx,edi
	mov	[begin_offset],edx
	sub	ebx,edx
	mov	edx,ebx
	shl	edx,1
	inc	edx
	mov	[cursor],edx
	jmp	.end
;--------------------------------------------------------------------
.abs:
	mov	esi,[screen_table]
	mov	esi,[esi]
	xor	ebx,ebx
.3:
	add	ebx,esi
	cmp	eax,ebx
	jg	.3
	sub	ebx,esi
	cmp	ebx,[file_size]
	jg	Ctrl_G
	mov	[begin_offset],ebx
	sub	eax,ebx
	shl	eax,1
	inc	eax
	mov	[cursor],eax
.end:
	jmp	red
;--------------------------------------------------------------------
Ctrl_B:
	bts	[flags],9
	mov	dword [ed_box_data],2
	mov	dword [ed_box_data+4],edit4
	mov	dword [ed_box_data+8],edit5
	call	draw_ed_box
	btr	[flags],9

	mov	ecx,[edit4.size]
	test	ecx,ecx
	jz	.end
	cmp	ecx,8
	jg	Ctrl_B

	mov	edi,sel1_string
	mov	ebx,edit4.size
	call	strtohex

	cmp	eax,[file_size]
	jge	Ctrl_B
	push	eax	;from
	mov	ecx,[edit5.size]
	test	ecx,ecx
	jz	.end
	cmp	ecx,8
	jg	Ctrl_B

	mov	edi,sel2_string
	mov	ebx,edit5.size
	call	strtohex

	cmp	eax,[file_size]
	jb	@f
	pop	eax
	jmp	Ctrl_B

@@:
	pop	[shblock_beg]
	cmp	eax,[shblock_beg]
	jae	@f
	xchg	eax,[shblock_beg]
@@:
	mov	[shblock_end],eax
	bts	[flags],8
.end:
	jmp	red
;--------------------------------------------------------------------
Ctrl_F:
	bts	[flags],3
	mov	dword [ed_box_data],1
	mov	dword [ed_box_data+4],edit3
	call	draw_ed_box
	btr	[flags],3
	mov	ecx,[edit3.size]
	test	ecx,ecx
	jz	.end
	cmp	ecx,8
	jg	Ctrl_F
	mov	edi,find_string
	mov	edx,[option_group2]
	cmp	edx,op11
	jne	.find
	mov	eax,find_string
	push	dword [eax]
	push	dword [eax+4]
	bts	[flags],0
	mov	ebx,edit3.size
	call	strtohex
	mov	ecx,[edit3.size]
	bt	cx,0
	jnc	.3
	inc	ecx
	shl	eax,4
.3:
	shr	ecx,1
.4:
	mov	[edi+ecx-1],al
	shr	eax,8
	loop	.4
.find:
	mov	esi,[current_offset]
	mov	ebx,[file_size]
	mov	eax,ebx
	add	eax,[file_buffer]
	add	esi,[file_buffer]
.5:
	mov	ecx,[edit3.size]
	cmp	edx,op11
	jne	.7
	bt	cx,0
	jnc	.6
	inc	ecx
.6:
	shr	ecx,1
.7:
	cld
@@:
	cmp	esi,eax
	jg	.end
	cmpsb
	je	.8
	mov	edi,find_string
	jmp	.5
;--------------------------------------
.8:
	loop	@b
	sub	esi,[file_buffer]
	mov	ecx,[edit3.size]
	cmp	edx,op11
	jne	.10
	bt	cx,0
	jnc	.9
	inc	ecx
.9:
	shr	ecx,1
.10:
	sub	esi,ecx
	xor	edx,edx
	mov	edi,[screen_table]
	mov	edi,[edi]
@@:
	add	edx,edi
	cmp	edx,esi
	jb	@b
	sub	edx,edi
	mov	[begin_offset],edx
	sub	esi,edx
	shl	esi,1
	inc	esi
	mov	[cursor],esi
.end:
	bt	[flags],0
	jnc	@f
	mov	eax,find_string
	pop	dword [eax+4]
	pop	dword [eax]
	btr	[flags],0
@@:
	jmp	red
;--------------------------------------------------------------------
invert_byte:
	mov	ebx,[current_offset]
	cmp	ebx,[file_size]
	jae	still
	add	ebx,[file_buffer]
	not	byte [ebx]
	jmp	red
;--------------------------------------------------------------------
Insert:	;��४��祭��	०���	��⠢��/������
	btc	[flags],1	;not [insert_mod]
	call	show_insert
	jmp	red
;--------------------------------------------------------------------
DEL:
	bt	[flags],1
	jnc	still
	mov	edi,[current_offset]
	mov	esi,[file_buffer]
	mov	edx,[file_size]
	test	edx,edx
	jz	still
	dec	edx
	cmp	edi,edx
	jbe	@f
	call	LEFT
	call	LEFT
	jmp	red
@@:
	jb	@f
	call	LEFT
	call	LEFT
@@:
	cmp	edi,edx
	je	@f
	mov	al,[edi+esi+1]
	mov	[edi+esi],al
	inc	edi
	jmp	@b
@@:
	dec	[file_size]
	call	show_file_size
	jmp	red
;--------------------------------------------------------------------
BackSpace:
	bt	[flags],1	;cmp [insert_mod],0
	jnc	still	;je still
	mov	edi,[current_offset]
	mov	esi,[file_buffer]
	mov	edx,[file_size]
	test	edx,edx
	jz	still
	test	edi,edi
	jz	still
	call	LEFT
	call	LEFT
	cmp	[cursor],2
	jne	@f
	cmp	edx,1
	jne	@f
	dec	[cursor]
@@:
	cmp	edi,edx
	jge	@f
	mov	al,[edi+esi]
	mov	[edi+esi-1],al
	inc	edi
	jmp	@b
;--------------------------------------
@@:
	dec	[file_size]
	call	show_file_size
	jmp	red
;--------------------------------------------------------------------
Ctrl_UP:
	cmp	[begin_offset],0
	je	@f
	mov	eax,[bytes_per_line]
	sub	[begin_offset],eax
@@:
	ret
;--------------------------------------------------------------------
Ctrl_DOWN:
	mov	eax,[cursor]
	dec	eax
	shr	eax,1
	add	eax,[begin_offset]
	mov	ebx,[bytes_per_line]
	add	eax,ebx
	cmp	eax,[file_size]
	jge	@f
	add	[begin_offset],ebx
@@:
	ret
;--------------------------------------------------------------------
UP:
	mov	eax,[current_offset]
	cmp	eax,[bytes_per_line]
	jb	still
	mov	eax,[cursor]
	dec	ax
	shr	ax,1
	cmp	eax,[bytes_per_line]
	jge	@f
	mov	eax,[bytes_per_line]
	sub	[begin_offset],eax
	jmp	red
@@:
	mov	eax,[bytes_per_line]
	shl	ax,1
	sub	[cursor],eax
	jmp	red
;--------------------------------------------------------------------
DOWN:	;��	��ப�	����
	mov	eax,[current_offset]
	add	eax,[bytes_per_line]
	bt	[flags],1
	jnc	@f
	dec	eax
@@:
	cmp	eax,[file_size]
	jge	still	;�᫨ �� �� ��᫥���� ��ப� 䠩��, � �⮯
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	edx,[cursor]
	dec	dx
	shr	dx,1
	add	edx,[bytes_per_line]
	cmp	eax,edx	;�� ��᫥���� ��ப�?
	jbe	@f
	mov	eax,[bytes_per_line]
	shl	ax,1
	add	[cursor],eax
	ret
@@:
	mov	eax,[bytes_per_line]
	add	[begin_offset],eax
	ret
;--------------------------------------------------------------------
LEFT:
	cmp	[cursor],1
	jbe	@f
	dec	[cursor]
	jmp	.end
;--------------------------------------
@@:
	cmp	[begin_offset],0	;�����	�� ��ࢮ� ��ப� � ᬥ饭��� 0?
	jne	@f	;���� ᬥ頥� � �ப��⪮� ����� ����� � � ����� ��ப�
;	inc	[cursor]
	jmp	.end;still	;⮣�� �⮯
;--------------------------------------
@@:
	mov	eax,[bytes_per_line]
	sub	[begin_offset],eax
	shl	ax,1
	dec	eax
	add	[cursor],eax
.end:
	ret
;--------------------------------------------------------------------
RIGHT:
	mov	ecx,[begin_offset]	;����塞 ᬥ饭�� �����
	mov	edx,[cursor]	;��� �஢�ન ����⢮�����
	shr	edx,1	;᫥���饣� ᨬ����
	add	ecx,edx
	bt	[flags],1
	jnc	@f
	dec	ecx	;�ࠢ������ � ࠧ��஬ 䠩��
@@:
	cmp	ecx,[file_size]	;���������� ����� - �� ����� 1 ���� �� ���� 䠩��
	jge	red
	cmp	[file_size],0
	je	still
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	ecx,[begin_offset]
	cmp	eax,edx	;�ࠢ����� �� ������ ��ப�
	jbe	@f
	inc	[cursor]	;����� ��ࠢ�
	jmp	red
;--------------------------------------
@@:
	mov	ecx,[bytes_per_line]	;ᬥ頥��� �� ����� ����
	add	[begin_offset],ecx	;� �ப��⪮�
	shl	cx,1
	dec	cx
	sub	[cursor],ecx
	jmp	red
;--------------------------------------------------------------------
PGDN:
	mov	edi,[screen_table]
	mov	eax,[edi]
	shl	eax,1
	add	eax,[begin_offset]
	cmp	eax,[file_size]	;���� �� ����������� ᬥ������ �� ��࠭���?
	jg	Ctrl_END
	mov	eax,[edi]
	add	[begin_offset],eax
;	mov	ebx,[cursor]
;	dec	ebx
;	xor	ecx,ecx
;	bt	ebx,0
;	jnc	@f
;	inc	ecx
;	@@:	shr	ebx,1
;	add	ebx,eax
;	@@:	cmp	ebx,[file_size]
;	jbe	@f
;	sub	ebx,[bytes_per_line]
;	jmp	@b
;	@@:	sub	ebx,eax
;	shl	ebx,1
;	inc	ebx
;	add	ebx,ecx
;	mov	[cursor],ebx
	jmp	red
;--------------------------------------------------------------------
PGUP:
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	edx,[begin_offset]
	cmp	eax,edx
	jbe	@f
	call	Ctrl_HOME
	jmp	red
;--------------------------------------
@@:
	sub	[begin_offset],eax
	jmp	red
;--------------------------------------------------------------------
HOME:
	mov	eax,[cursor]
	dec	ax
	shr	ax,1
	mov	ecx,[bytes_per_line]
	xor	edx,edx
	div	ecx
	shl	dx,1
	sub	[cursor],edx
	bt	[cursor],0
	jc	@f
	dec	[cursor]
@@:
	ret
;--------------------------------------------------------------------
END_:
	mov	eax,[cursor]
	dec	ax
	shr	ax,1
	mov	ecx,[bytes_per_line]
	xor	edx,edx
	div	ecx
	mov	eax,[current_offset]
	sub	eax,edx
	add	eax,[bytes_per_line]
	mov	edx,[file_size]
	cmp	eax,edx
	jbe	@f
	sub	edx,eax
	add	eax,edx
@@:
	sub	eax,[begin_offset]
	shl	eax,1
	test	eax,eax
	jz	red
	dec	eax
	mov	[cursor],eax
	jmp	red
;--------------------------------------------------------------------
Ctrl_HOME:
	mov	[begin_offset],0
	mov	[cursor],1
	ret
;--------------------------------------------------------------------
Ctrl_END:
	mov	eax,[file_size]
	mov	ecx,[screen_table]
	mov	ecx,[ecx]
	xor	edx,edx
	div	ecx
	test	dx,dx
	jnz	@f
	test	eax,eax
	jz	@f
	mov	edx,ecx
	dec	eax
@@:
	push	dx
	xor	dx,dx
	mul	ecx
	pop	dx
	shl	edx,1
	cmp	edx,1
	jg	@f
	mov	edx,2
@@:
	dec	edx
	mov	[begin_offset],eax
	mov	[cursor],edx
	jmp	red
;--------------------------------------------------------------------
ESC:
	btr	[flags],8
	jmp	red
;--------------------------------------------------------------------
copy_to_buf:
	bt	[flags],8
	jnc	.1
	mov	eax,[shblock_end]
	sub	eax,[shblock_beg]
	inc	eax
	mov	ecx,eax
	mov	[copy_len],eax
	mcall	SF_SYS_MISC,SSF_MEM_REALLOC,,[copy_buf]
	mov	esi,[shblock_beg]
	mov	edi,[copy_buf]
	add	esi,[file_buffer]
	mov	ecx,[copy_len]
	cld
@@:
	movsb
	loop	@b
	bts	[flags],10
	xor	eax,eax
	ret
;---------------------------------------------------------------------
.1:
	or	eax,-1
	ret
;--------------------------------------------------------------------
Ctrl_C:
	call	copy_to_buf
	jmp	still
;--------------------------------------------------------------------
shblock_sz	dd	0
;--------------------------------------------------------------------
Ctrl_V:
	bt	[flags],10
	jnc	still
	bt	[flags],8
	jnc	.past_kurs
;��⠢�塞 ���� � �뤥������ �������
	mov	ebx,[shblock_end]
	sub	ebx,[shblock_beg]
	inc	ebx
	mov	[shblock_sz],ebx
	mov	esi,[copy_buf]
	mov	edi,[file_buffer]
	add	edi,[shblock_beg]
	mov	eax,[copy_len]
.1:
	cld
@@:
	movsb
	dec	eax
	jz	.del
	dec	ebx
	jnz	@b
.add:
	push	esi	edi	eax
	push	[file_size]
	add	[file_size],eax
	call	raspred_mem
	pop	ecx
	mov	edi,[esp+4]
	add	ecx,[file_buffer]
	sub	ecx,edi
	inc	ecx
	mov	edi,[file_size]
	add	edi,[file_buffer]
	mov	esi,edi
	sub	esi,eax
	std
@@:
	movsb
	loop	@b
	pop	eax edi esi
	cld
@@:
	movsb
	dec	eax
	jnz	@b
	mov	eax,[shblock_beg]
	add	eax,[copy_len]
	dec	eax
	mov	[shblock_end],eax
	jmp	red
;--------------------------------------
.del:
	dec	ebx
	jz	red
	mov	ecx,[file_size]
	add	ecx,[file_buffer]
	sub	ecx,edi
	mov	esi,edi
	add	esi,ebx
	cld
@@:
	movsb
	loop	@b
	sub	[file_size],ebx
	call	raspred_mem
	mov	eax,[shblock_beg]
	add	eax,[copy_len]
	dec	eax
	mov	[shblock_end],eax
	jmp	red
;--------------------------------------------------------------------
;�᫨ ���� �� �뤥���, � ��⠢�塞 ���� ��। ����஬
.past_kurs:
;	bt	[flags],1
;	jnc	still
;	mov	esi,[file_buffer]
;	add	esi,[current_offset]
;	jmp	red
	jmp	still
;--------------------------------------------------------------------
Ctrl_X:
	bt	[flags],1
	jnc	still
	call	copy_to_buf
	test	eax,eax
	jnz	still

	mov	ecx,[file_size]
	sub	ecx,[copy_len]
	sub	ecx,[shblock_beg]
	inc	ecx
	mov	ebx,[file_size]
	mov	edi,[shblock_beg]
	mov	esi,[shblock_end]
	inc	esi
	add	esi,[file_buffer]
	add	edi,[file_buffer]
	add	ebx,[file_buffer]
	cld
@@:
	cmp	esi,ebx
	je	.1
	movsb
	loop	@b
.1:
	mov	eax,[file_size]
	sub	eax,[copy_len]
	mov	[file_size],eax
	btr	[flags],8

	mov	eax,[shblock_beg]
	mov	esi,[screen_table]
	mov	esi,[esi]
	xor	ebx,ebx
.3:
	add	ebx,esi
	cmp	eax,ebx
	jg	.3
	sub	ebx,esi
;	cmp	ebx,[file_size]
;	jg	Ctrl_G
	mov	[begin_offset],ebx
	sub	eax,ebx
	shl	eax,1
	inc	eax
	mov	[cursor],eax
	jmp	red
;---------------------------------------------------------------------
open_dialog:
	mov	[OpenDialog_data.type],0	; Open

	push    dword OpenDialog_data
	call    [OpenDialog_Start]

;	cmp	[OpenDialog_data.status],2	; OpenDialog does not start
	cmp	[OpenDialog_data.status],1
	jne	still
.start:
	mov	esi,fname_buf
.load:
	mov	edi,file_name
	cld
@@:
	cmp	byte [esi],0
	je	@f
	movsb
	jmp	@b
@@:
	mov	byte [edi],0
	sub	esi,path
	mov	[edit1.size],esi
	mov	[edit1.pos],esi
	jmp	open_file
;---------------------------------------------------------------------
;----------------------- DATA AREA------------------------
;--------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0,0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align	4
Box_lib_import:
edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
version_ed		dd aVersion_ed

option_box_draw		dd aOption_box_draw
option_box_mouse	dd aOption_box_mouse
version_op		dd aVersion_op

scrollbar_ver_draw	dd aScrollbar_ver_draw
scrollbar_ver_mouse	dd aScrollbar_ver_mouse
scrollbar_hor_draw	dd aScrollbar_hor_draw
scrollbar_hor_mouse	dd aScrollbar_hor_mouse
version_scrollbar	dd aVersion_scrollbar

menu_bar_draw		dd aMenu_bar_draw
menu_bar_mouse		dd aMenu_bar_mouse
version_menu_bar	dd aVersion_menu_bar

	dd 0,0

aEdit_box_draw		db 'edit_box_draw',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
aVersion_ed		db 'version_ed',0

aOption_box_draw	db 'option_box_draw',0
aOption_box_mouse	db 'option_box_mouse',0
aVersion_op		db 'version_op',0

aScrollbar_ver_draw	db 'scrollbar_v_draw',0
aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
aScrollbar_hor_draw	db 'scrollbar_h_draw',0
aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
aVersion_scrollbar	db 'version_scrollbar',0

aMenu_bar_draw		db 'menu_bar_draw',0
aMenu_bar_mouse		db 'menu_bar_mouse',0
aVersion_menu_bar	db 'version_menu_bar',0
;---------------------------------------------------------------------
align	4
scroll_bar_data_vertical scrollbar scroll_width_size, 565, 284, 19, scroll_width_size, 300+20, 50, 0, 0xAAAAAA, 0xCCCCCC, 0, 10
;---------------------------------------------------------------------
align	4
scroll_bar_data_horizontal scrollbar 300, 0, scroll_width_size, 300, scroll_width_size, 300, 30, 1, 0xAAAAAA, 0xCCCCCC, 0, 10
;---------------------------------------------------------------------
align	4
menu_data_1:
.type:		dd 0	;+0
.x:
.size_x	dw 40	;+4
.start_x	dw 2	;+6
.y:
.size_y		dw 15	;+8
.start_y	dw 2	;+10
.text_pointer:	dd menu_text_area	;0	;+12
.pos_pointer:	dd menu_text_area.1	;0	;+16
.text_end	dd menu_text_area.end	;0	;+20
.mouse_pos	dd 0	;+24
.mouse_keys	dd 0	;+28
.x1:
if lang eq ru_RU
 .size_x1	dw 4*2+9*6	;+32
else ; Default to en_US
 .size_x1	dw 40	;+32
end if
.start_x1	dw 2	;+34
.y1:
.size_y1	dw 100	;+36
.start_y1	dw 18	;+38
.bckg_col	dd 0xeeeeee	;+40
.frnt_col	dd 0xff	;+44
.menu_col	dd 0xffffff	;+48
.select		dd 0	;+52
.out_select	dd 0	;+56
.buf_adress	dd 0	;+60
.procinfo	dd 0	;+64
.click		dd 0	;+68
.cursor		dd 0	;+72
.cursor_old	dd 0	;+76
.interval	dd 16	;+80
.cursor_max	dd 0	;+84
.extended_key	dd 0	;+88
.menu_sel_col	dd 0x00cc00	;+92
.bckg_text_col	dd 0	;+96
.frnt_text_col	dd 0xffffff	;+100
.mouse_keys_old	dd 0	;+104
.font_height	dd 8	;+108
.cursor_out	dd 0	;+112
.get_mouse_flag	dd 0	;+116

menu_text_area:
if lang eq ru_RU
  	 db '����',0
 .1:
	 db '������',0
	 db '���࠭���',0
	 db '��室',0
else ; Default to en_US
  	 db 'File',0
 .1:
	 db 'Open',0
	 db 'Save',0
	 db 'Exit',0
end if
.end:
	 db 0

;---------------------------------------------------------------------
align	4
menu_data_2:
.type:		dd 0	;+0
.x:
.size_x	dw 40	;+4
.start_x	dw 43	;+6
.y:
.size_y		dw 15	;+8
.start_y	dw 2	;+10
.text_pointer:	dd menu_text_area_2	;0	;+12
.pos_pointer:	dd menu_text_area_2.1	;0	;+16
.text_end	dd menu_text_area_2.end	;0	;+20
.mouse_pos	dd 0	;+24
.mouse_keys	dd 0	;+28
.x1:
.size_x1	dw 4*2+5*6	;+32
.start_x1	dw 43	;+34
.y1:
.size_y1	dw 100	;+36
.start_y1	dw 18	;+38
.bckg_col	dd 0xeeeeee	;+40
.frnt_col	dd 0xff	;+44
.menu_col	dd 0xffffff	;+48
.select		dd 0	;+52
.out_select	dd 0	;+56
.buf_adress	dd 0	;+60
.procinfo	dd 0	;+64
.click		dd 0	;+68
.cursor		dd 0	;+72
.cursor_old	dd 0	;+76
.interval	dd 16	;+80
.cursor_max	dd 0	;+84
.extended_key	dd 0	;+88
.menu_sel_col	dd 0x00cc00	;+92
.bckg_text_col	dd 0	;	+96
.frnt_text_col	dd 0xffffff	;+100
.mouse_keys_old	dd 0	;+104
.font_height	dd 8	;+108
.cursor_out	dd 0	;+112
.get_mouse_flag	dd 0	;+116

menu_text_area_2:
if lang eq ru_RU
	 db '���',0
 .1:
else ; Default to en_US
	 db 'View',0
 .1:
end if
	 db 'Add 4',0
	 db 'Add 8',0
	 db 'Sub 4',0
	 db 'Sub 8',0
.end:
	 db 0

;---------------------------------------------------------------------
align	4
menu_data_3:
.type:		dd 0	;+0
.x:
if lang eq ru_RU
 .size_x	dw 4*2+7*6	;+32
else ; Default to en_US
 .size_x	dw 40	;+4
end if
.start_x	dw 84	;+6
.y:
.size_y		dw 15	;+8
.start_y	dw 2	;+10
.text_pointer:	dd menu_text_area_3	;0	;+12
.pos_pointer:	dd menu_text_area_3.1	;0	;+16
.text_end	dd menu_text_area_3.end	;0	;+20
.mouse_pos	dd 0	;+24
.mouse_keys	dd 0	;+28
.x1:
if lang eq ru_RU
 .size_x1	dw 4*2+7*6	;+32
else ; Default to en_US
 .size_x1	dw 40	;+32
end if
.start_x1	dw 84	;+34
.y1:
.size_y1	dw 100	;+36
.start_y1	dw 18	;+38
.bckg_col	dd 0xeeeeee	;+40
.frnt_col	dd 0xff	;+44
.menu_col	dd 0xffffff	;+48
.select		dd 0	;+52
.out_select	dd 0	;+56
.buf_adress	dd 0	;+60
.procinfo	dd 0	;+64
.click		dd 0	;+68
.cursor		dd 0	;+72
.cursor_old	dd 0	;+76
.interval	dd 16	;+80
.cursor_max	dd 0	;+84
.extended_key	dd 0	;+88
.menu_sel_col	dd 0x00cc00	;+92
.bckg_text_col	dd 0	;	+96
.frnt_text_col	dd 0xffffff	;+100
.mouse_keys_old	dd 0	;+104
.font_height	dd 8	;+108
.cursor_out	dd 0	;+112
.get_mouse_flag	dd 0	;+116

menu_text_area_3:
if lang eq ru_RU
	db '��ࠢ��',0
 .1:
	db '��ࠢ��',0
else ; Default to en_US
	db 'Help',0
 .1:
	db 'Help',0
end if
.end:
	db 0
;---------------------------------------------------------------------
edit1	edit_box 200,190,27,0xffffff,0x6a9480,0,0xAABBCC,0,134,cur_dir_path,ed_focus,ed_focus,6,6	;䠩�	������\��࠭���
edit2	edit_box 55,270,29,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,go_to_string,ed_focus,ed_focus,0,0	;���宦�	��	ᬥ饭��
edit3	edit_box 55,270,29,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,find_string,ed_focus,ed_focus,0,0	;����
edit4	edit_box 55,220,49,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,sel1_string,ed_focus,ed_focus,0,0	;�뤥����	����	���.ᬥ�.
edit5	edit_box 55,300,49,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,sel2_string,ed_focus,0,0,0	;�뤥����	����	���.ᬥ�.

op1	option_box option_group1,210,50,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1,1
op2	option_box option_group1,310,50,6,12,0xFFFFFF,0,0,op_text.2,op_text.e2-op_text.2
op3	option_box option_group1,310,65,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3
op11	option_box option_group2,210,50,6,12,0xffffff,0,0,op_text2.11,op_text2.e11-op_text2.11
op12	option_box option_group2,310,50,6,12,0xffffff,0,0,op_text2.21,op_text2.e21-op_text2.21

option_group1	dd op1	;㪠��⥫�, ��� �⮡ࠦ����� �� 㬮�砭��, ����� �뢮�����
option_group2	dd op11	;�ਫ������
Option_boxs	dd op1,op2,op3,0
Option_boxs2	dd op11,op12,0

op_text:	; ���஢�����騩 ⥪�� ��� 祪 ���ᮢ
if lang eq ru_RU
 .1	db '��᮫�⭮�'
 .e1:
 .2	db '���।'
 .e2:
 .3	db '�����'
 .e3:
else ; Default to en_US
 .1	db 'Absolutely'
 .e1:
 .2	db 'Forward'
 .e2:
 .3	db 'Back'
 .e3:
end if

op_text2:
.11	db 'Hex'
.e11:
.21	db 'ASCII'
.e21:
;--------------------------------------------------------------------
system_dir_Boxlib			db '/sys/lib/box_lib.obj',0
system_dir_ProcLib			db '/sys/lib/proc_lib.obj',0

sel_text	db "From to",0

help_but_text	= menu_text_area_3 ;db	'Help',0

head_f_i:
if lang eq ru_RU
 error_open_file_string	db "���� �� ������!",0
 error_save_file_string	db "���� �� ��࠭��!",0
else ; Default to en_US
 error_open_file_string	db "Isn't found!",0
 error_save_file_string	db "Isn't saved!",0
end if
string_cp866	db ' cp866'
string_cp1251	db 'cp1251'
string_koi8r	db 'koi8-r'
string_ins	db 'Ins'


align	4
number_strings	dd 16	;������⢮ ��ப �� ����
;bytes_per_line	dd 16	;���-�� �⮫�殢

group_bytes	dd 8
bytes_per_line	dd 16



;blocks_counter	dd 1
;blocks_table	dd 0
cursor	dd 1

flags	dw 001000010b
;���	0: � edit_box - �����⨥/(��)	��� �������� ᨬ�����
;1:	0/1 - ०�� ������/��⠢��
;2:	� edit_box - ��ࠡ�⪠ Ctrl_G
;3:	� edit_box - ��ࠡ�⪠ Ctrl_F
;4:	� change_codepage - �᫨ ������, � ����⠭����� �।����� ����஢��
;5:	vertical_scroll_bar move
;6:	������	����ᮢ�� ����
;7:	horizontal_scroll_bar move
;8:	1 - �뤥��� ����
;9:	� edit_box - ��ࠡ�⪠ Ctrl_B
;10:	� ����� "����" ᪮��஢���� ����
;--------------------------------------------------------------------
help_text:
if lang eq ru_RU
 db 'Ctrl+O              - ������ 䠩�                 '
 db 'Ctrl+S              - ��࠭���                    '
 db 'PageUp, PageDown    - ��࠭�� �����/����          '
 db 'Ctrl+UP, Ctrl+Down  - �ப��⪠ ��࠭��� �� ���-  '
 db '                 �� �����/���� ��� ᬥ饭�� �����'
 db 'Home,End            - � ��砫�/����� ��ப�        '
 db 'Ctrl+Home, Ctrl+End - � ��砫�/����� 䠩��         '
 db 'Left, Right, UP, DN - ����� �����/��ࠢ�/���/����'
 db 'n                   - ������஢��� ����           '
 db 'Ins                 - ०�� ������/��⠢��         '
 db '  Del               - 㤠���� ���� ��� ����஬    '
 db '  BackSpace         - 㤠���� ���� ��। ����஬  '
 db '~                   - ᬥ�� ����஢�� cp866,cp1251 '
 db 'Shift+~             - cp866/cp1251,koi8r           '
 db 'Ctrl+F              - ����                        '
 db 'Ctrl+G              - ���室 �� ᬥ饭��          '
 db 'Ctrl+B              - �뤥���� ����                '
 db 'ESC                 - ���� �뤥�����              '
 db 'Ctrl+C              - ����஢��� ����              '
 db 'Ctrl+V              - ��⠢��� � �뤥������ �������'
 db 'Ctrl+X              - ��१��� � ����             '
else ; Default to en_US
 db 'Ctrl+O              - open file                    '
 db 'Ctrl+S              - save file                    '
 db 'PageUp, PageDown    - page up/down                 '
 db 'Ctrl+UP, Ctrl+Down  - scroll page by one string    '
 db '                    up/down without cursor movement'
 db 'Home,End            - at the start/end of string   '
 db 'Ctrl+Home, Ctrl+End - at the start/end of file     '
 db 'Left, Right, Up, Dn - move cursor to the lft/rght..'
 db 'n                   - invert byte                  '
 db 'Ins                 - replace/past mode            '
 db '  Del               - delete byte under cursor     '
 db '  BackSpace         - delete byte before cursor    '
 db '~                   - change codepages cp866,cp1251'
 db 'Shift+~             - cp866/cp1251,koi8r           '
 db 'Ctrl+F              - find                         '
 db 'Ctrl+G              - go to offset                 '
 db 'Ctrl+B              - select area                  '
 db 'ESC                 - deselect area                '
 db 'Ctrl+C              - copy area                    '
 db 'Ctrl+V              - past into area from buf      '
 db 'Ctrl+X              - cut area into buffer         '
end if
help_end:
;--------------------------------------------------------------------
;align	4096
font_buffer	file 'cp866-8x16'	;ASCII+cp866	(+�,�)
cp1251		file 'cp1251-8x16'
koi8_r		file 'koi8-r-8x16'

title	db	_title
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, buf_cmd_lin, system_dir_Boxlib, Box_lib_import

library02  l_libs system_dir_ProcLib+9, buf_cmd_lin, system_dir_ProcLib, ProcLib_import

end_l_libs:
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window_1	;+28
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
dd	Filter.end - Filter.1
.1:
;db	'BIN',0
;db	'DAT',0
.end:
db	0

start_temp_file_name:	db 'temp.bin',0

;---------------------------------------------------------------------
I_END:
;--------------------------------------------------------------------
file_name:
cur_dir_path	rb 4096
buf_cmd_lin	rb 0
procinfo:	;opendialog
threath_buf:
	rb 0x400
path:
		rb 1024+16	;opendialog

screen_table	rd 1
begin_offset	rd 1
file_buffer	rd 1

copy_buf	rd 1
copy_len	rd 1

current_offset	rd 1
;	rd	1 ;��� ���訩 dword
codepage_offset	rd 1
codepage_offset_previous	rd 1
low_area	rd 1	;���न���� ������ ��ப�
right_table_xy	rd 1

prev_f_size_bl	rd 1

len_str_scr	rd 1
beg_str_scr	rd 1

shblock_beg	rd 1	;ᬥ饭�� ��砫� �����
shblock_end	rd 1	;ᬥ饭�� ���� �����

;mouse_flag	rd 1
file_size	rd 1
;	rd	1 ;��� ���訩 dword

active_process	rd 1

ed_box_data:	rd 8	;���� dword -���-�� ����⮢ ��� ��ࠡ�⪨, ����� 㪠��⥫� �� editX;

bufferfinfo	rb 40
hex8_string	rb 9	;���� ��� hex_output
go_to_string	rb 9
find_string	rb 17
sel1_string	rb 9
sel2_string	rb 9
cur_help_string	rb 1	;����� ��ப�, � ���ன �뢮����� ⥪�� � help - ����

help_is_open_already	rb 1  ;�᫨ ���� �ࠢ�� �����, � ����� 1
help_window_pid 	rd 1

func_70	f70
;---------------------------------------------------------------------
title_buf:
	rb 4096
;---------------------------------------------------------------------
fname_buf:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
D_END:
