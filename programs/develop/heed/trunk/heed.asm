;************************************************************************
; v.015 08.03.2010
; Support for OpenDialog - Open and Save
; Some optimization
;************************************************************************
; v.014 05.02.2010
; 
; PageUp, PageDown      - страница вверх/вниз
; Ctrl+UP, Ctrl+Down    - прокрутка страницы на строку вверх/вниз без смещения курсора
; Home,End              - в начало/конец строки
; Ctrl+Home, Ctrl+End   - к первому/последнему байту файла
; Left, Right           - курсор влево/вправо
; Ctrl+O                - открыть файл
; Ctrl+S                - сохранить
; Ctrl+F                - поиск (+Tab для OptionBox)
; Ctrl+G                - переход на смещение (+Tab для OptionBox)
; Ctrl+B                - выделить блок
; ESC                   - снять выделение
; Ctrl+C		- копировать блок
; Ctrl+V		- вставить в выделенную область
; Ctrl+X		- вырезать выделенную область (в буфер)
; n                     - инвертировать байт под курсором
; Ins                   - режим замены/вставки (по умолчанию)
;   Del                 - в режиме вставки - удалить байт под курсором
;   BackSpace           - в режиме вставки - удалить байт перед курсором
; ~                     - смена кодировки (cp866,cp1251)
; Shift+~               - смена кодировки (cp866 или cp1251,koi8-r)

; Память перераспределяется на увеличение i umen'shenie.
; Файл загружается целиком.

; Макросы load_lib.mac, editbox_ex и библиотеку box_lib.obj создали:
; <Lrz> - Alexey Teplov / Алексей Теплов
; Mario79, Mario - Marat Zakiyanov / Марат Закиянов
; Diamondz - Evgeny Grechnikov / Евгений Гречников и др.
;
; staper@inbox.ru
; babalbes@yandex.ru

use32
	org	0x0
	db	'MENUET01'
	dd	0x1
	dd	START	;program start
	dd	I_END	;program image	size
	dd	(D_END+0x600) and not 3	;required amount of memory
	dd	(D_END+0x600) and not 3	;stack
	dd	0x0	;buf_cmd_lin
	dd	cur_dir_path

_title	equ 'HeEd 0.15'

include	'lang.inc'
include '../../../macros.inc'
include	'../../libraries/box_lib/trunk/box_lib.mac'
include	'../../libraries/box_lib/load_lib.mac'
include	'../../libraries/box_lib/asm/trunk/opendial.mac'

@use_library
use_OpenDialog

times	16	dd	0

frgrd_color	equ	0xfefefe
bkgrd_color	equ	0x000000
kursred_color	equ	0x0039ff
kurstxt_color	equ	0x708090
text_color	equ	0xaaaaaa

panel_clr1	equ	0xe9e9e2
panel_clr2	equ	0x8b8b89
panel_clr3	equ	0x777777;eaeae3


palitra:
.1	dd	frgrd_color,bkgrd_color	;цвет невыделенного символа
.2	dd	frgrd_color,text_color	;левый,правый столбцы,часть нижней строки
.3	dd	kursred_color,frgrd_color	;курсора
.4	dd	kurstxt_color,bkgrd_color	;курсора в текстовой области
.5	dd	panel_clr1,not	text_color	;нижняя панель

FIRST_HEX equ 0*65536+24
scroll_width_size equ 15

struct	f70
	func_n	rd 1
	param1	rd 1
	param2	rd 1
	param3	rd 1
	param4	rd 1
	rezerv	rb 1
	name	rd 1
ends

START:
	mcall	68,11
;OpenDialog	initialisation
init_OpenDialog	OpenDialog_data

	load_library	boxlib_name,cur_dir_path,buf_cmd_lin,system_path,\
	err_message_found_lib,head_f_l,myimport,err_message_import,head_f_i

	mcall	40,0x27

	mcall	68,12,32*1024	;страничный буфер
	mov	[screen_table],eax
	mcall	68,12,4*1024
;	mov	[blocks_table],eax
	mov	[file_buffer],eax
;	mov	esi,eax
;	mcall	68,12,4*1024
;	mov	[esi],eax
;	mov	[blocks_counter],1

;	mcall	68,12,1024	;Procinfo area for function 9 in MenuBar
;	mov	[menu_data_1.procinfo],eax
;	mov	[menu_data_2.procinfo],eax
	mcall	68,12,1024
	mov	[copy_buf],eax


	;размер	текущего пути
	mov	esi,cur_dir_path
@@:
	cmp	byte [esi],0
	je	@f
	inc	esi
	jmp	@b
@@:
	sub	esi,cur_dir_path
	mov	[edit1.pos],esi
	mov	[edit1.size],esi

	;общесистемные клавиши для Shift+курсоры
;	mcall	66,4,75,1
;	mcall	66,,77
;	mcall	66,,72
;	mcall	66,,80

	call	ready_screen_buffer
;	jmp	open_file

redraw_all:
	call	draw_window_1
still:
	mcall	10

	cmp	eax,6
	je	mouse
	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jz	button
	jmp	still

red:	call	ready_screen_buffer
	call	main_area
	jmp	still

draw_window_1:
	call	start_draw
	call	draw_window
	call	show_file_size
	call	show_codepage
	call	show_insert
	call	ready_screen_buffer
	call	main_area
	ret

key:
	mcall	2
	dec	al
	jz	still
	dec	al
	jz	key.syst
	cmp	ah,2
	je	Ctrl_B	;выделить блок
	cmp	ah,3
	je	Ctrl_C	;copy
	cmp	ah,6
	je	Ctrl_F	;find
	cmp	ah,7
	je	Ctrl_G	;go to
	cmp	ah,8
	je	BackSpace
	cmp	ah,15
	je	open_dialog	;open_file ;Ctrl+O
	cmp	ah,19
	je	save_file	;Ctrl+S
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
@@:
	cmp	ah,82
	jne	@f
	call	Ctrl_UP
	jmp	red
@@:
	cmp	ah,84
	jne	@f
	call	Ctrl_HOME
	jmp	red
@@:
	cmp	ah,85
	je	Ctrl_END
	cmp	ah,96
	je	change_codepage	;тильда, cp866 - cp1251
	cmp	ah,97
	jb	still
	cmp	ah,102
	jbe	input_from_keyboard ;a-f
	cmp	ah,126
	jne	@f
	xor	ah,ah
	jmp	change_codepage	;Shift+~, koi8-r
@@:
	cmp	ah,110
	je	invert_byte ;n
	cmp	ah,176
	jne	@f
	call	LEFT
	jmp	red
@@:
	cmp	ah,177
	jne	@f
	call	DOWN
	jmp	red
@@:
	cmp	ah,178
	je	UP
	cmp	ah,179
	je	RIGHT
	cmp	ah,180
	jne	@f
	call	HOME
	jmp	red
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
.syst:
	cmp	ah,72
	cmp	ah,75
	cmp	ah,77
	cmp	ah,80
	jmp	still

button:
	mcall	17
	dec	ah
	jnz	still
	jmp	close_prog

align	4
mouse:
	mcall	37,7
	test	eax,eax
	jz	.menu_bar_1;.mouse
	bt	eax,15
	jc	@f	;обработка колёсика мыши
	mov	ecx,eax
	shl	ecx,2
.1:
	call	Ctrl_DOWN
	loop	.1
	jmp	red
@@:
	xor	ecx,ecx
	sub	cx,ax
	shl	cx,2
.2:
	call	Ctrl_UP
	loop	.2
	jmp	red

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
.menu_bar_2:
	push	dword menu_data_2
	call	[menu_bar_mouse]
	cmp	[menu_data_2.click],dword 1
	jne	.menu_bar_3
	cmp	[menu_data_2.cursor_out],dword 0
	jne	.analyse_out_menu_2
	jmp	.menu_bar_1
.menu_bar_3:
	push	dword menu_data_3
	call	[menu_bar_mouse]
	cmp	[menu_data_3.click],dword 1
	jne	.scroll_bar
	cmp	[menu_data_3.cursor_out],dword 0
	jne	.analyse_out_menu_3
	jmp	.menu_bar_1

.set_mouse_flag:
	xor	eax,eax
	inc	eax
	mov	[menu_data_1.get_mouse_flag],eax
	mov	[menu_data_2.get_mouse_flag],eax
	mov	[menu_data_3.get_mouse_flag],eax
	ret

.analyse_out_menu_1:
	cmp	[menu_data_1.cursor_out],dword 1
	je	open_dialog
	cmp	[menu_data_1.cursor_out],dword 2
	je	open_dialog_save	;save_file
	cmp	[menu_data_1.cursor_out],dword 3
	je	close_prog
	jmp	still

.analyse_out_menu_2:
	cmp	[menu_data_2.cursor_out],dword 1
	jne	@f
	add	[bytes_per_line],4
	jmp	redraw_all
@@:
	cmp	[menu_data_2.cursor_out],dword 2
	jne	@f
	add	[bytes_per_line],8
	jmp	redraw_all
@@:
	cmp	[menu_data_2.cursor_out],dword 3
	jne	@f
	cmp	[bytes_per_line],4
	je	still
	sub	[bytes_per_line],4
	jmp	redraw_all
@@:
	cmp	[menu_data_2.cursor_out],dword 4
	jne	still
	cmp	[bytes_per_line],8
	jbe	still
	sub	[bytes_per_line],8
	jmp	redraw_all

.analyse_out_menu_3:	;analyse result of Menu 2
	cmp	[menu_data_3.cursor_out],dword 1
	jne	still
	call	create_help_window
	jmp	still

.scroll_bar:
;	mcall	37,2
;	test	eax,eax
;	jnz	@f
;	btr	[flags],5
;	btr	[flags],7
;	jmp	still

.mouse:
.vertical:
	mcall	37,2
	test	eax,eax
	jnz	@f
	btr	[flags],5
	btr	[flags],7
	jmp	still
@@:
	bt	[flags],5
	jc	@f
	bt	[flags],7
	jc	.horizontal_0

	mcall	37,1
	shr	eax,16
	cmp	ax,[scroll_bar_data_vertical.start_x]
	jb	.horizontal
	sub	ax,[scroll_bar_data_vertical.start_x]
	cmp	ax,[scroll_bar_data_vertical.size_x]
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
@@:
	mov	[begin_offset],eax
	bts	[flags],5

	mov	eax,scroll_bar_data_vertical.redraw
	xor	ebx,ebx
;	cmp	[eax],ebx
;	je	@f
	mov	[eax],ebx
	jmp	red
;@@:
;	cmp	[scroll_bar_data_vertical.delta2],0
;	jne	still
.horizontal:
	mov	eax,[scroll_bar_data_horizontal.max_area]
	cmp	eax,[scroll_bar_data_horizontal.cur_area]
	jbe	.other

	mcall	37,1
	cmp	ax,[scroll_bar_data_horizontal.start_y]
	jb	still
	sub	ax,[scroll_bar_data_horizontal.start_y]
	cmp	ax,[scroll_bar_data_horizontal.size_y]
	jge	still

	; mouse event for Horizontal ScrollBar
.horizontal_0:
;	mcall	37,2
;	test	eax,eax
;	jnz	@f
;	btr	[flags],5
;	btr	[flags],7
;	jmp	still
;@@:;	bt	[flags],7
;	jc	@f

;	mcall	37,1
;	shr	eax,16
;	cmp	ax,[scroll_bar_data_vertical.start_x]
;	jb	.horizontal
;	sub	ax,[scroll_bar_data_vertical.start_x]
;	cmp	ax,[scroll_bar_data_vertical.size_x]
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
.other:
;	cmp	[scroll_bar_data_vertical.delta2],0
;	jne	still
;	cmp	[scroll_bar_data_horizontal.delta2],0
;	jne	still
	jmp	still



;------------------------------------------------

;------------------------------------------------


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
	mov	word [esi+16],0x203a	;двоеточие
	mov	eax,[bytes_per_line]
	add	[.offset],eax
	mov	[.string_size],2+4*4+4+2
	add	esi,4*4+4+2
	pop	edi
	mov	[.temp],edi
	push	[.cursor]
	pop	[.cursor_temp]

	;hex	значения
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

.fl	db 0
.fl_buf	dd 0
.to_null:
	dec	[.fl]
	mov	[.fl_buf],edi
	jmp	.pre_next_byte0

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
	mov	word [esi+16],0x203a	;двоеточие
	mov	eax,[bytes_per_line]
	add	[.offset],eax
	mov	[.string_size],2+4*4+4+2
	add	esi,4*4+4+2
	push	[.cursor]
	pop	[.cursor_temp]

	;hex значения
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
	cmp	[.fl],0	;проверка флага
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

;корректно обрабатываем финальную строку файла, длина которой вариативна
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

.string_size	dd 16
.number_strings	dw 0
.length_to_end	dd 0
.temp		dd 0
.offset		dd 0
.cursor		dd 0
.cursor_temp	dd 0
.shblock_beg	dd 0
.shblock_end	dd 0


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
	xor	edx,edx	;ползунок
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

	mcall	37,2	;кпопка	мыши нажата - нет смысла перерисовывать ScrollBar
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
	mov	esi,0x000001	;цвет и число бит на пиксель
	mov	edx,FIRST_HEX	;координаты первого hex
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
	cmp	bl,128	;проверка на принадлежность символа к расширенной таблице
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
	mcall	65
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
	jz	.next_string

.string_size	dd 0
.number_strings	dw 0
.len_str_scr	dd 0

@@:
	pushad
	mov	ecx,edx
	shl	ecx,16
	mov	ebx,edx
	shr	ebx,16
	cmp	bx,[scroll_bar_data_vertical.start_x]
	jge	.ls1
	mov	ax,[scroll_bar_data_vertical.start_x]
	sub	ax,bx
	shl	ebx,16
	mov	bx,ax
	mov	cx,16
	mcall	13,,,frgrd_color
.ls1:
	popad
	jmp	@f
.loop_str:
	bt	[flags],6
	jc	@b
@@:
	dec	[.string_size]
	jz	.next_string
	add	edi,2
	add	edx,8*65536
	jmp	@b

@@:;очистка фоновым цветом незакрашенных областей
	pushad
	ror	edx,16
	mov	dx,16
	mov	ecx,edx
	mov	edx,frgrd_color
	movzx	ebx,[scroll_bar_data_vertical.start_x]
	sub	ecx,2*65536
	mov	cx,2
	mcall	13
	popad

	pushad
	mov	ecx,edx
	shl	ecx,16
	mov	ebx,edx
	shr	ebx,16
	cmp	bx,[scroll_bar_data_vertical.start_x]
	jge	.10
	mov	ax,[scroll_bar_data_vertical.start_x]
	sub	ax,bx
	shl	ebx,16
	mov	bx,ax
	mov	cx,16
	mcall	13,,,frgrd_color
.10:
	popad
	jmp	@f
.next_string:
	bt	[flags],6
	jc	@b
@@:
	push	[len_str_scr]
	pop	[.len_str_scr]
	mov	eax,[ready_screen_buffer.string_size]
	shr	eax,1
	mov	[.string_size],eax	;коррекция смещения входных данных
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


align	4
show_current_offset:
	pushad
	push	edx	;вывод текущего смещения в файле
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
	;двоичное значение байта
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

	;десятичное
	push	edx
	mov	edx,[file_buffer]
	mov	ebx,[current_offset]
	add	edx,ebx
	xor	eax,eax
	inc	ebx
	cmp	ebx,[file_size]	;0 если за границей файла
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
	mcall	65
	pop	eax
	sub	edx,8*65536
	dec	dword [esp]
	jnz	@b
	add	esp,4
;	mov	edx,[low_area]	;вывод esp
;	add	edx,298*65536
;	mov	ebx,esp
;	mov	ecx,8
;	call	hex_output
	pop	edx
	popad
	ret


align	4
hex_output:	;вывод hex строки из 8 символов
	pushad
	mov	edi,(hex8_string)	;адрес буфера
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
	mcall	65
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
@@:
	call	show_file_size
	mov	ebx,[current_offset]
	add	ebx,[file_buffer]
	mov	byte [ebx],0
.2:
	mov	ecx,[current_offset]
	add	ecx,[file_buffer]
	;см.	первую	версию	heed.asm
	mov	dl,[ecx]	;оригинальный байт
	mov	ebx,[cursor]
	and	bl,1	;нечет - редактируем старший полубайт
	jnz	.hi_half_byte ;чёт - старший
	and	dl,0xf0	;обнуляем мл. п-байт оригинального байта
	jmp	.patch_byte
.hi_half_byte:	;одновременно сдвигаем нужное значение в ст п-т и обнуляем младший
	shl	ax,4
	and	dl,0x0f	;обнуляем старший полубайт у оригинального байта
.patch_byte:
	or	ah,dl
	mov	[ecx],ah
	jmp	RIGHT

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
@@:
	mov	[prev_f_size_bl],eax
	xor	edx,edx
	mul	ecx
	add	ecx,eax
.1:
	mcall	68,20,,[file_buffer]
.ret:
	popad
	ret
;---------------------------------------

align	4
show_file_size:
	mov	ebx,[file_size]
	mov	edx,[low_area];
	mov	esi,1
	mov	ecx,8
	call	hex_output
	ret


align	4
draw_window:
	mcall	0,100*65536+653,100*65536+360,((0x73 shl 24) + frgrd_color),,title
	mcall	9,threath_buf,-1
	cmp	byte [threath_buf+70],3	;окно свёрнуто в заголовок?
	jnae	@f
.@d:
	call	end_draw
	add	esp,4
	jmp	still
@@:
	cmp	dword [threath_buf+66],(24*4)	;проверка минимальной высоты
	jae	@f
	mov	esi,dword [threath_buf+46]
	sub	esi,dword [threath_buf+66]
	add	esi,24*4
	mcall	67,-1,-1,-1,
	jmp	.@d
@@:
	cmp	dword [threath_buf+62],(26*6)	;проверка минимальной ширины
	jae	@f
	mov	edx,dword [threath_buf+42]
	sub	edx,dword [threath_buf+62]
	add	edx,26*6
	mcall	67,-1,-1,,-1
	jmp	.@d
@@:
	mov	eax,[file_size]
	mov	ebx,[bytes_per_line]
	xor	edx,edx
	div	ebx
	mov	[scroll_bar_data_vertical.size_x],0
	cmp	eax,[number_strings]
	jl	@f
	mov	[scroll_bar_data_vertical.size_x],scroll_width_size
@@:
	mov	eax,dword [threath_buf+62]	;ширина клиентской области
	sub	ax,[scroll_bar_data_vertical.size_x]
	mov	[scroll_bar_data_vertical.start_x],ax
	mov	eax,dword [threath_buf+66]	;высота клиентской области
	sub	eax,24+24-11
	mov	[scroll_bar_data_vertical.size_y],ax
	mov	ebx,eax
	push	eax
	add	ebx,20
	mov	[scroll_bar_data_vertical.max_area],ebx
	mov	ebx,[scroll_bar_data_vertical.btn_high]
	shl	ebx,1
	add	ebx,20
	mov	[scroll_bar_data_vertical.cur_area],ebx
	pop	eax
	sub	eax,3
	mov	ebx,18
	xor	edx,edx
	div	bx
	mov	[number_strings],eax	;кол-во hex строк в окне
	mov	ebx,[bytes_per_line]
	mul	ebx
	mov	edi,[screen_table]	;кол-во байтов для вывода
	mov	dword [edi],eax

	push	eax

	mov	ebx,dword [threath_buf+62]
	inc	ebx
	mov	ecx,(FIRST_HEX-18)
	ror	ecx,16
	mov	cx,18
	ror	ecx,16
	mcall	13,,,frgrd_color	;полоса сверху

	mcall	,,18,panel_clr1	;верхняя панель

	dec	ebx
	mcall	38,,<18,18>,panel_clr2
	mov	ecx,dword [threath_buf+66]
	sub	cx,18
	push	cx
	shl	ecx,16
	pop	cx
	mcall	,,,panel_clr3	;нижняя панель
	add	ecx,1*65536
	mov	cx,18
;	inc	ebx
	mcall	13,,,panel_clr1


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
	movzx	ebx,	word [scroll_bar_data_vertical.start_x]
	mcall	13,,,frgrd_color

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
	mov	[scroll_bar_data_horizontal.start_y],ax

;cur_area/(size_x-30)=len_str_scr/string_size

	mov	eax,dword [threath_buf+62]
	sub	ax,[scroll_bar_data_vertical.size_x]
	mov	[scroll_bar_data_horizontal.size_x],ax
	sub	eax,[scroll_bar_data_horizontal.btn_high]
	sub	eax,[scroll_bar_data_horizontal.btn_high]
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
@@:
	bts	[flags],6
	ret

align	4
start_draw:
	mcall	12,1
	ret

end_draw:
	mcall	12,2
	ret

close_prog:
	mcall	-1
;-------------------------------------------------------------------------------
change_codepage:	;меняем вторую половину таблицы
	test	ah,ah
	jnz	@f
	btc	[flags],4
	jc	.1
	push	[codepage_offset]
	pop	[codepage_offset_previous]
	mov	[codepage_offset],2*128
	jmp	.end
.1:
	push	[codepage_offset_previous]
	pop	[codepage_offset]
	jmp	.end
@@:
	cmp	[codepage_offset],0
	jne	@f
	add	[codepage_offset],128
	jmp	.end
@@:
	mov	[codepage_offset],0
.end:
	call	show_codepage
	jmp	red

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
	mcall	65
	add	edx,8*65536
	pop	edi
	inc	edi
	dec	dword [esp]
	jnz	@b
	add	esp,4
	ret

show_insert:	;отображение режима вставки/замены
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
	mcall	65
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
	mcall	51,1,.thread,(.threat_stack+16*4)
	popad
	ret
.thread:
	call	.window
.still:
	mcall	10
	dec	al
	jz	.red
	dec	al
	jz	.key
	dec	al
	jz	.button
	jmp	.still
	mcall	-1
.button:
	mcall	17,1
	cmp	ah,1
	jne	@f
	mcall	-1
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
@@:
	cmp	ah,3
	jne	.still
	cmp	[cur_help_string],0
	je	.still
	dec	[cur_help_string]
	jmp	.red

.key:
	mcall	2
	jmp	.still

.red:
	call	.window
	jmp	.still

.window:
	pushad
	mcall	12,1
	mcall	0,50*65536+320,0x70*65536+240,0x13000000,,help_but_text
	mcall	8,<130,20>,<6,12>,2,0xaaaaaa
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
	mov	eax,4
@@:
	add	ebx,0x10
	mcall
	add	edx,51
	dec	edi
	jnz	@b
	mcall	12,2
	popad
	ret

.threat_stack:	times	16	dd	0
;-------------------------------------------------

open_file:
	mov	dword [ed_box_data],1
	mov	dword [ed_box_data+4],edit1
	call	draw_ed_box	;рисуем editbox
	;размер файла?
.0:
	mov	[func_70.func_n],5
	mov	[func_70.param1],0
	mov	[func_70.param2],0
	mov	[func_70.param3],0
	mov	[func_70.param4],bufferfinfo
	mov	[func_70.rezerv],0
	mov	[func_70.name],file_name
	mcall	70,func_70

	test	al,al	;файл найден?
	jz	@f
	mcall	4,400*65536+31,0x80CC0000,error_open_file_string
	jmp	open_file
@@:
;	mov	edx,[blocks_counter]
;	mov	edi,[blocks_table]
;	@@:	mov	ecx,[edi]	;высвобождаем:
;	mcall	68,13	;блоки файла
;	add	edi,8
;	dec	edx
;	jnz	@b
;	mcall	68,13,[blocks_table]	;таблицу

	mov	eax,	dword [bufferfinfo+32]	;копируем размер файла
	mov	[file_size],eax

;	mov	ebx,65536	;64КБ блок
;	xor	edx,edx
;	div	ebx
;	push	dx	;длина последнего блока
;	test	dx,dx
;	jz	@f
;	inc	eax
;	@@:	test	eax,eax
;	jnz	@f
;	inc	eax
;	@@:	mov	[blocks_counter],eax
;	sal	eax,3;*8	;размер таблицы с индексами блоков
;;	add	eax,32	;решаем	проблему с 32МБ файлами

;	mov	ecx,eax	;выделяем память:
;	mcall	68,12	;под таблицу
;	mov	[blocks_table],eax
;	mov	edi,eax
;	mov	ecx,[blocks_counter]
;	@@:	mov	dword [edi+4],65536
;	add	edi,8
;	loop	@b
;	xor	edx,edx
;	pop	dx	;длина последнего блока
;	mov	dword [edi-4],edx

;	mov	edx,[blocks_counter]
;	mov	edi,[blocks_table]
;@@:	mcall	68,12,[edi+4]	;под блок
;	mov	[edi],eax
;	add	edi,8
;	dec	edx
;	jnz	@b

	mcall	68,13,[file_buffer]
	test	eax,eax
	jnz	@f
	;здесь ошибка на не освобождение блока
@@:
	mcall	68,12,[file_size]
	mov	[file_buffer],eax

;;имеем таблицу: [ DWORD указатель на первый элемент блока : DWORD длина блока ]

;	mov	ecx,[blocks_counter]	;открываем файл
;	mov	edi,[blocks_table]
	mov	[func_70.func_n],0
	mov	[func_70.param1],0
	mov	[func_70.param2],0
	mov	[func_70.rezerv],0
	mov	[func_70.name],file_name
;@@:
	push	dword [file_size];dword [edi+4]
	pop	dword [func_70.param3]
	push	dword [file_buffer];dword [edi]
	pop	dword [func_70.param4]
	mcall	70,func_70
;	add	edi,8
;	add	dword [func_70.param1],65536
;	loop	@b

	test	eax,eax
	jz	@f
	;ошибка чтения
@@:
	call	Ctrl_HOME

	jmp	redraw_all
;-------------------------------------------------------------------------------
open_dialog_save:
	call	get_filter_data
	mov	[OpenDialog_data.type],1	; Save
	start_OpenDialog	OpenDialog_data
	cmp	[OpenDialog_data.status],2	; OpenDialog does not start
;	je	.sysxtree	; some kind of alternative, instead OpenDialog
	je	save_file
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
@@:
	mov	byte [edi],0
	sub	esi,path
	mov	[edit1.size],esi
	mov	[edit1.pos],esi
	jmp	save_file.1
;-------------------------------------------------------------------------------
save_file:	;сохраняем файл
	mov	dword [ed_box_data],1
	mov	dword [ed_box_data+4],edit1
	call	draw_ed_box
.1:
	mov	[func_70.func_n],2
	mov	[func_70.param1],0
	mov	[func_70.param2],0
	push	[file_size]
	pop	[func_70.param3]
	push	[file_buffer]
	pop	[func_70.param4]
	mov	[func_70.rezerv],0
	mov	[func_70.name],file_name
	mcall	70,func_70
	cmp	al,0	;сохранён удачно?
	je	redraw_all
	mcall	4,400*65536+31,0x80CC0000,error_save_file_string
	jmp	save_file
;-------------------------------------------------------------------------------
draw_ed_box:	;рисование edit box'а
.1:
	push	eax	ebx	ecx	edx
	mcall	13,180*65536+220,25*65536+70,0xaaaaaa
	bt	[flags],9
	jnc	@f
	mcall	4,246*65536+35,0x80ffffff,sel_text
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
	mcall	10
	cmp	al,6
	je	.mouse
	cmp	al,3
	je	.button
	cmp	al,2
	je	.keys
	cmp	al,1
	jne	.2
	call	draw_window
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

.keys:
	mcall	2
	cmp	ah,13
	je	.4
	cmp	ah,27
	je	.3

	bt	[flags],2	;проверка на применимость символов 0-9,a-b
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
@@:
	cmp	edx,op2
	jne	@f
	mov	edx,op3
	jmp	.eb1_2
@@:
	mov	edx,op1
.eb1_2:
	mov	[option_group1],edx
	pop	edx
	jmp	.1
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
@@:
	mov	edx,op11
.eb2_1:
	mov	[option_group2],edx
	pop	edx
	jmp	.1
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
@@:
	pop	edx
	dec	[edit5.shift]
	dec	[edit5.shift+4]
	jmp	.2
.eb3_1:
	push	edx
	mov	edx,[edit4.flags]
	and	edx,2
	jz	@f
	pop	edx
	mov	[edit5.flags],2
	mov	[edit4.flags],0
	jmp	.eb3_2
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

.eb:
	push	dword [ed_box_data+4];[esp]
	call	[edit_box_key]
	jmp	.2

.button:
	mcall	17
	cmp	ah,1
	jne	.2
	jmp	close_prog
.3:
	btr	[flags],2
	btr	[flags],3
	add	esp,4
	jmp	redraw_all
.4:
	mcall	13,180*65536+220,25*65536+70,frgrd_color
	ret


;-------------------------------------------------
;-------------------------------------------------
;-------------------------------------------------

strtohex:
;enter: edi - pointer to string,ebx - pointer to size of string; exit: eax in hex
	mov	esi,hex8_string
@@:
	mov	ah,[edi+ecx-1]	;обработка введённых символов
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

	cmp	eax,[file_size]	;выбор check_box'а
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

invert_byte:
	mov	ebx,[current_offset]
	cmp	ebx,[file_size]
	jae	still
	add	ebx,[file_buffer]
	not	byte [ebx]
	jmp	red


Insert:	;переключение	режима	вставки/замены
	btc	[flags],1	;not [insert_mod]
	call	show_insert
	jmp	red


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
@@:
	dec	[file_size]
	call	show_file_size
	jmp	red


Ctrl_UP:
	cmp	[begin_offset],0
	je	@f
	mov	eax,[bytes_per_line]
	sub	[begin_offset],eax
@@:
	ret


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


DOWN:	;на	строку	вниз
	mov	eax,[current_offset]
	add	eax,[bytes_per_line]
	bt	[flags],1
	jnc	@f
	dec	eax
@@:
	cmp	eax,[file_size]
	jge	still	;если мы на последней строке файла, то стоп
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	edx,[cursor]
	dec	dx
	shr	dx,1
	add	edx,[bytes_per_line]
	cmp	eax,edx	;на последней строке?
	jbe	@f
	mov	eax,[bytes_per_line]
	shl	ax,1
	add	[cursor],eax
	ret
@@:
	mov	eax,[bytes_per_line]
	add	[begin_offset],eax
	ret


LEFT:
	cmp	[cursor],1
	jbe	@f
	dec	[cursor]
	jmp	.end
@@:
	cmp	[begin_offset],0	;курсор	на первой строке со смещением 0?
	jne	@f	;иначе смещаем с прокруткой вверх вверх и в конец строки
;	inc	[cursor]
	jmp	.end;still	;тогда стоп
@@:
	mov	eax,[bytes_per_line]
	sub	[begin_offset],eax
	shl	ax,1
	dec	eax
	add	[cursor],eax
.end:
	ret


RIGHT:
	mov	ecx,[begin_offset]	;вычисляем смещение курсора
	mov	edx,[cursor]	;для проверки существования
	shr	edx,1	;следующего символа
	add	ecx,edx
	bt	[flags],1
	jnc	@f
	dec	ecx	;сравниваем с размером файла
@@:
	cmp	ecx,[file_size]	;положением курсора - не далее 1 байта от конца файла
	jge	red
	cmp	[file_size],0
	je	still
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	ecx,[begin_offset]
	cmp	eax,edx	;сравнение на нижнюю строку
	jbe	@f
	inc	[cursor]	;курсор вправо
	jmp	red
@@:
	mov	ecx,[bytes_per_line]	;смещаемся на строчку вниз
	add	[begin_offset],ecx	;с прокруткой
	shl	cx,1
	dec	cx
	sub	[cursor],ecx
	jmp	red


PGDN:
	mov	edi,[screen_table]
	mov	eax,[edi]
	shl	eax,1
	add	eax,[begin_offset]
	cmp	eax,[file_size]	;есть ли возможность сместиться на страницу?
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


PGUP:
	mov	eax,[screen_table]
	mov	eax,[eax]
	mov	edx,[begin_offset]
	cmp	eax,edx
	jbe	@f
	call	Ctrl_HOME
	jmp	red
@@:
	sub	[begin_offset],eax
	jmp	red


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


Ctrl_HOME:
	mov	[begin_offset],0
	mov	[cursor],1
	ret


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


ESC:
	btr	[flags],8
	jmp	red

copy_to_buf:
	bt	[flags],8
	jnc	.1
	mov	eax,[shblock_end]
	sub	eax,[shblock_beg]
	inc	eax
	mov	ecx,eax
	mov	[copy_len],eax
	mcall	68,20,,[copy_buf]
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
.1:
	or	eax,-1
	ret

Ctrl_C:
	call	copy_to_buf
	jmp	still

shblock_sz	dd	0

Ctrl_V:
	bt	[flags],10
	jnc	still
	bt	[flags],8
	jnc	.past_kurs
;вставляем блок в выделенную область
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
.del:
	dec	ebx
	jz	red
	mov	ecx,[file_size]
	add	ecx,[file_buffer]
	sub	ecx,edi
	mov	esi,edi
	add	esi,ebx
	cld
@@:	movsb
	loop	@b
	sub	[file_size],ebx
	call	raspred_mem
	mov	eax,[shblock_beg]
	add	eax,[copy_len]
	dec	eax
	mov	[shblock_end],eax
	jmp	red

;если блок не выделен, то вставляем блок перед курсором 
.past_kurs:
;	bt	[flags],1
;	jnc	still
;	mov	esi,[file_buffer]
;	add	esi,[current_offset]
;	jmp	red
	jmp	still

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
	call	get_filter_data
	mov	[OpenDialog_data.type],0	; Open
	start_OpenDialog	OpenDialog_data
	cmp	[OpenDialog_data.status],2	; OpenDialog does not start
	je	.sysxtree	; some kind of alternative, instead OpenDialog
	cmp	[OpenDialog_data.status],1
	jne	still
	mov	esi,fname_buf
	jmp	.load
.sysxtree:
	call	opendialog
	jc	still
	mov	esi,path
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
	jmp	open_file.0
;---------------------------------------------------------------------
get_filter_data:
	mov	edi,[OpenDialog_data.com_area]
	test	edi,edi
	jnz	@f
	add	esp,4
	jmp	still
@@:
	add	edi,4096+4
	mov	esi,Filter
	mov	ecx,[esi]
	inc	ecx
	cld
	rep	movsb
	mov	edi,[OpenDialog_data.com_area]
	mov	[edi+4096],dword 1
	ret
;---------------------------------------------------------------------
;##################################
opendialog:
;
; STEP 1 Run SYSXTREE with parametrs MYPID 4 bytes in dec,
; 1 byte space, 1 byte type of dialog (O - Open ,S - Save)
;
	mov	edi,path
	xor	eax,eax
	mov	ecx,(1024+16)/4
	rep	stosd

	mov	[dlg_pid_get],0

; Get my PID in dec format 4 bytes
	mov	eax,9
	mov	ebx,procinfo
	or	ecx,-1
	mcall

; convert eax bin to param dec
	mov	eax,dword [procinfo+30]	;offset of myPID
	mov	edi,param+4-1	;offset to 4 bytes
	mov	ecx,4
	mov	ebx,10
.new_d:
	xor	edx,edx
	div	ebx
	add	dl,'0'
	mov	[edi],dl
	dec	edi
	loop	.new_d

; wirite 1 byte space to param
	mov	[param+4],byte 32	;Space for next parametr
; and 1 byte type of dialog to param
	mov	[param+5],byte 'O'	;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
	mov	[path],dword 0
	mov	[path+4],dword 8

; define IPC memory
	mov	eax,60
	mov	ebx,1		; define IPC
	mov	ecx,path	; offset of area
	mov	edx,1024+16	; size
	mcall

; change wanted events list 7-bit IPC event
	mov	eax,40
	mov	ebx,01000111b
;	cmp	[image],0
;	jnz	@f
;	mov	bl,01000110b
;@@:
	mcall

;
; STEP 3 run SYSTEM XTREE with parameters
;
	mov	eax,70
	mov	ebx,run_fileinfo
	mcall
	bt	eax,31
	jnc	@f
	mcall	40,0x27
	add	esp,4
	jmp	open_file
@@:
	mov	[get_loops],0
.getmesloop:
	mov	eax,23
	mov	ebx,50	;0.5 sec
	mcall
	dec	eax
	jz	.mred
	dec	eax
	jz	.mkey
	dec	eax
	jz	.mbutton
	cmp	al,	7-3
	jz	.mgetmes
; Get number of procces
	mov	ebx,procinfo
	mov	ecx,-1
	mov	eax,9
	mcall
	mov	ebp,eax

.loox:
	mov	eax,9
	mov	ebx,procinfo
	mov	ecx,ebp
	mcall
	mov	eax,[DLGPID]
	cmp	[procinfo+30],eax	;IF Dialog find
	je	.dlg_is_work	;jmp to dlg_is_work
	dec	ebp
	jnz	.loox

	jmp	.erroff

.dlg_is_work:
	cmp	[procinfo+50],word 9	;If slot state 9 - dialog is terminated
	je	.erroff	;TESTODP2 terminated too

	cmp	[dlg_pid_get],dword 1
	je	.getmesloop
	inc	[get_loops]
	cmp	[get_loops],4	;2 sec if DLG_PID not get TESTOP2 terminated
	jae	.erroff
	jmp	.getmesloop

.mred:
;	cmp	[image],	0
;	jz	.getmesloop
;	call	redraw_all
	call	draw_window_1
	jmp	.getmesloop
.mkey:
	mov	eax,2
	mcall	;	read (eax=2)
	jmp	.getmesloop
.mbutton:
	mov	eax,17	; get id
	mcall
	cmp	ah,1	; button id=1 ?
	jne	.getmesloop
	mov	eax,-1	; close this program
	mcall
.mgetmes:

; If dlg_pid_get then second message get jmp to still
	cmp	[dlg_pid_get],dword 1
	je	.ready

; First message is number of PID SYSXTREE dialog

; convert PID dec to PID bin
	movzx	eax,byte [path+16]
	sub	eax,48
	imul	eax,10
	movzx	ebx,byte [path+16+1]
	add	eax,ebx
	sub	eax,48
	imul	eax,10
	movzx	ebx,byte [path+16+2]
	add	eax,ebx
	sub	eax,48
	imul	eax,10
	movzx	ebx,byte [path+16+3]
	add	eax,ebx
	sub	eax,48
	mov	[DLGPID],eax

; Claear and prepare IPC area for next message
	mov	[path],dword 0
	mov	[path+4],dword 8
	mov	[path+8],dword 0
	mov	[path+12],dword 0
	mov	[path+16],dword 0

; Set dlg_pid_get for get next message
	mov	[dlg_pid_get],dword 1
;	cmp	[image],0
;	jz	.getmesloop
;	call	redraw_all
	call	draw_window_1
	jmp	.getmesloop

.ready:
;
; The second message get
; Second message is 100 bytes path to SAVE/OPEN file
; shl path string on 16 bytes
;
	mov	esi,path+16
	mov	edi,path
	mov	ecx,1024/4
	rep	movsd
	mov	[edi],byte 0

.openoff:
	mcall	40,0x27
	clc
	ret

.erroff:
	mcall	40,0x27
	stc
	ret
;##################################

;	DATA	AREA


;---------------------------------------------------------
;----------------------- DATA AREA------------------------
;---------------------------------------------------------
align	4
myimport:
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

	dd 0
	dd 0

aEdit_box_draw		db 'edit_box',0
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
scroll_bar_data_vertical:
.x:
.size_x		dw scroll_width_size;+0
.start_x	dw 565	;+2
.y:
.size_y		dw 284	;+4
.start_y	dw 19	;+6
.btn_high	dd scroll_width_size	;+8
.type		dd 1	;+12
.max_area	dd 300+20	;+16
.cur_area	dd 50	;+20
.position	dd 0	;+24
.bckg_col	dd 0xAAAAAA	;+28
.frnt_col	dd 0xCCCCCC	;+32
.line_col	dd 0	;+36
.redraw		dd 0	;+40
.delta		dw 0	;+44
.delta2		dw 0	;+46
.run_x:
.r_size_x	dw 0	;+48
.r_start_x	dw 0	;+50
.run_y:
.r_size_y	dw 0	;+52
.r_start_y	dw 0	;+54
.m_pos		dd 0	;+56
.m_pos_2	dd 0	;+60
.m_keys		dd 0	;+64
.run_size	dd 0	;+68
.position2	dd 0	;+72
.work_size	dd 0	;+76
.all_redraw	dd 0	;+80
.ar_offset	dd 10	;+84
;---------------------------------------------------------------------
align	4
scroll_bar_data_horizontal:
.x:
.size_x		dw 300	;0	;+0
.start_x	dw 0	;0	;+2
.y:
.size_y		dw scroll_width_size	;0	;+4
.start_y	dw 300	;0	;+6
.btn_high	dd scroll_width_size	;+8
.type		dd 1	;+12
.max_area	dd 300	;+16
.cur_area	dd 30	;+20
.position	dd 1	;+24
.bckg_col	dd 0xAAAAAA	;+28
.frnt_col	dd 0xCCCCCC	;+32
.line_col	dd 0	;+36
.redraw		dd 0	;+40
.delta		dw 0	;+44
.delta2		dw 0	;+46
.run_x:
.r_size_x	dw 0	;+48
.r_start_x	dw 0	;+50
.run_y:
.r_size_y	dw 0	;+52
.r_start_y	dw 0	;+54
.m_pos		dd 0	;+56
.m_pos_2	dd 0	;+60
.m_keys		dd 0	;+64
.run_size	dd 0	;+68
.position2	dd 0	;+72
.work_size	dd 0	;+76
.all_redraw	dd 0	;+80
.ar_offset	dd 10	;+84
;---------------------------------------------------------------------
align	4
menu_data_1:
.type:		dd 0	;+0
.x:
.size_x		dw 40	;+4
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
.size_x1	dw 40	;+32
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
	db 'File',0
.1:
	db 'Open',0
	db 'Save',0
	db 'Exit',0
.end:
	db 0
;---------------------------------------------------------------------
align	4
menu_data_2:
.type:		dd 0	;+0
.x:
.size_x		dw 40	;+4
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
.size_x1	dw 50	;+32
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
	db 'View',0
.1:
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
.size_x		dw 40	;+4
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
.size_x1	dw 40	;+32
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
	db 'Help',0
.1:
	db 'Help',0
.end:
	db 0
;---------------------------------------------------------------------
edit1	edit_box 200,190,27,0xffffff,0x6a9480,0,0xAABBCC,0,134,cur_dir_path,ed_focus,ed_focus,6,6	;файл	открыть\сохранить
edit2	edit_box 55,260,29,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,go_to_string,ed_focus,ed_focus,0,0	;перехожд	на	смещение
edit3	edit_box 55,260,29,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,find_string,ed_focus,ed_focus,0,0	;поиск
edit4	edit_box 55,220,49,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,sel1_string,ed_focus,ed_focus,0,0	;выделить	блок	нач.смещ.
edit5	edit_box 55,300,49,0xeeeeee,0x6a9480,0,0xAABBCC,4,8,sel2_string,ed_focus,0,0,0	;выделить	блок	кон.смещ.

op1	option_box option_group1,210,50,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1,1
op2	option_box option_group1,310,50,6,12,0xFFFFFF,0,0,op_text.2,op_text.e2-op_text.2
op3	option_box option_group1,210,65,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3
op11	option_box option_group2,210,50,6,12,0xffffff,0,0,op_text2.11,op_text2.e11-op_text2.11
op12	option_box option_group2,310,50,6,12,0xffffff,0,0,op_text2.21,op_text2.e21-op_text2.21

option_group1	dd op1	;указатели, они отображаются по умолчанию, когда выводится
option_group2	dd op11	;приложение
Option_boxs	dd op1,op2,op3,0
Option_boxs2	dd op11,op12,0

op_text:	; Сопровождающий текст для чек боксов
.1	db 'Absolutely'
.e1:
.2	db 'Forward'
.e2:
.3	db 'Back'
.e3:

op_text2:
.11	db 'Hex'
.e11:
.21	db 'ASCII'
.e21:


system_path	db '/sys/lib/'
boxlib_name	db 'box_lib.obj',0

head_f_i:
head_f_l	db 'error',0
err_message_found_lib	db 'box_lib.obj was not found',0
err_message_import	db 'box_lib.obj was not imported',0

sel_text	db "From to",0

help_but_text	= menu_text_area_3 ;db	'Help',0
error_open_file_string	db "Isn't found!",0
error_save_file_string	db "Isn't saved!",0
string_cp866	db ' cp866'
string_cp1251	db 'cp1251'
string_koi8r	db 'koi8-r'
string_ins	db 'Ins'


align	4
number_strings	dd 16	;количество строк на листе
;bytes_per_line	dd 16	;кол-во столбцов

group_bytes	dd 8
bytes_per_line	dd 16



;blocks_counter	dd 1
;blocks_table	dd 0
cursor	dd 1

flags	dw 001000010b
;бит	0: в edit_box - восприятие/(не)	всех вводимых символов
;1:	0/1 - режим замены/вставки
;2:	в edit_box - обработка Ctrl_G
;3:	в edit_box - обработка Ctrl_F
;4:	в change_codepage - если поднят, то восстановить предыдущую кодировку
;5:	vertical_scroll_bar move
;6:	полная	перерисовка окна
;7:	horizontal_scroll_bar move
;8:	1 - выделен блок
;9:	в edit_box - обработка Ctrl_B
;10:	в памяти "висит" скопированный блок

help_text:
if lang eq ru
 db 'Ctrl+O              - открыть файл                 '
 db 'Ctrl+S              - сохранить                    '
 db 'PageUp, PageDown    - страница вверх/вниз          '
 db 'Ctrl+UP, Ctrl+Down  - прокрутка страницы на стро-  '
 db '                 ку вверх/вниз без смещения курсора'
 db 'Home,End            - в начало/конец строки        '
 db 'Ctrl+Home, Ctrl+End - в начало/конец файла         '
 db 'Left, Right, UP, DN - курсор влево/вправо/выше/ниже'
 db 'n                   - инвертировать байт           '
 db 'Ins                 - режим замены/вставки         '
 db '  Del               - удалить байт под курсором    '
 db '  BackSpace         - удалить байт перед курсором  '
 db '~                   - смена кодировок cp866,cp1251 '
 db 'Shift+~             - cp866/cp1251,koi8r           '
 db 'Ctrl+F              - поиск                        '
 db 'Ctrl+G              - переход на смещение          '
 db 'Ctrl+B              - выделить блок                '
 db 'ESC                 - снять выделение              '
 db 'Ctrl+C              - копировать блок              '
 db 'Ctrl+V              - вставить в выделенную область'
 db 'Ctrl+X              - вырезать в буфер             '
else
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




;align	4096
font_buffer	file 'cp866-8x16'	;ASCII+cp866	(+Ё,ё)
cp1251		file 'cp1251-8x16'
koi8_r		file 'koi8-r-8x16'


;##########################	open_dial
get_loops	dd 0
dlg_pid_get	dd 0
DLGPID	dd 0
param:
	dd 0	; My dec PID
	dd 0,0	; Type of dialog
run_fileinfo:
	dd 7
	dd 0
	dd param
	dd 0
	dd 0
;run_filepath
	db '/sys/SYSXTREE',0
readdir_fileinfo:
	dd 1
	dd 0
	dd 0
readblocks	dd	0
directory_ptr	dd	0

;##########################	


title	db	_title
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

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File Managers/opendial',0
communication_area_default_pach:
	db '/rd/1',0

Filter:
dd	Filter.end - Filter
db	'BIN',0
db	'DAT',0
.end:
db	0
;---------------------------------------------------------------------

I_END:
file_name:
cur_dir_path	rb 4096
buf_cmd_lin	rb 0
procinfo:	;opendialog
threath_buf	rb 0x400
path:
		rb 1024+16	;opendialog

screen_table	rd 1
begin_offset	rd 1
file_buffer	rd 1

copy_buf	rd 1
copy_len	rd 1

current_offset	rd 1
;	rd	1 ;под старший dword
codepage_offset	rd 1
codepage_offset_previous	rd 1
low_area	rd 1	;координаты нижней строки
right_table_xy	rd 1

prev_f_size_bl	rd 1

len_str_scr	rd 1
beg_str_scr	rd 1

shblock_beg	rd 1	;смещение начала блока
shblock_end	rd 1	;смещение конца блока

;mouse_flag	rd 1
file_size	rd 1
;	rd	1 ;под старший dword

ed_box_data:	rd 8	;первый dword -кол-во элементов для обработки, далее указатели на editX;

bufferfinfo	rb 40
hex8_string	rb 9	;буфер для hex_output
go_to_string	rb 9
find_string	rb 17
sel1_string	rb 9
sel2_string	rb 9
cur_help_string	rb 1	;номер строки, с которой выводится текст в help - окне

func_70	f70
;---------------------------------------------------------------------
fname_buf:
	rb 4096
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
D_END: