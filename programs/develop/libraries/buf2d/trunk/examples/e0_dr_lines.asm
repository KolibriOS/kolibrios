use32
	org 0x0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 0x1
	dd start
	dd i_end ;размер приложения
	dd mem,stacktop
	dd 0,cur_dir_path

include '../../../../../macros.inc'
include '../../../../../proc32.inc'
include '../../../../../develop/libraries/box_lib/load_lib.mac'
include '../../../../../dll.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc, 0 ;dll.Load

align 4
start:
	load_library vectors_name, cur_dir_path, library_path, system_path, \
		err_message_found_lib, head_f_l, import_buf2d_lib, err_message_import, head_f_i
	cmp eax,-1
	jz button.exit

	mcall 40,0x27
	stdcall [buf2d_create], buf_0 ;создаем буфер
	stdcall [buf2d_line], buf_0, 110, 20, 125, 90, 0xffff00 ;рисуем линию
	stdcall [buf2d_line], buf_0, 60, 120, 110, 20, 0xd000 ;рисуем линию
	stdcall [buf2d_curve_bezier], buf_0, (10 shl 16)+20,(110 shl 16)+10,(50 shl 16)+90, dword 0xff
	stdcall [buf2d_circle], buf_0, 125, 90, 30, 0xffffff ;рисуем окружность
	stdcall [buf2d_circle], buf_0, 25, 70, 15, 0xff0000 ;рисуем окружность

align 4
red_win:
	call draw_window

align 4
still:
	mcall 10
	cmp al,1 ;изменилось положение окна
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	jmp still

align 4
draw_window:
	pushad
	mcall 12,1

	;mov edx,0x32000000
	mov edx,0x33000000
	mcall 0,(50 shl 16)+330,(30 shl 16)+275,,,caption

	stdcall [buf2d_draw], buf_0

	mcall 12,2
	popad
	ret

align 4
key:
	mcall 2

	cmp ah,27 ;Esc
	je button.exit

	jmp still

align 4
button:
	mcall 17 ;получить код нажатой кнопки
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0 ;удаляем буфер
	mcall -1 ;выход из программы

caption db 'Test buf2d library, [Esc] - exit',0

;--------------------------------------------------
align 4
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_line dd sz_buf2d_line
	buf2d_circle dd sz_buf2d_circle
	buf2d_img_hdiv2 dd sz_buf2d_img_hdiv2
	buf2d_img_wdiv2 dd sz_buf2d_img_wdiv2
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_conv_24_to_32 dd sz_buf2d_conv_24_to_32
	buf2d_bit_blt dd sz_buf2d_bit_blt
	buf2d_bit_blt_transp dd sz_buf2d_bit_blt_transp
	buf2d_bit_blt_alpha dd sz_buf2d_bit_blt_alpha
	buf2d_curve_bezier dd sz_buf2d_curve_bezier
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_circle db 'buf2d_circle',0 ;рисование окружности
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_curve_bezier db 'buf2d_curve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0

align 4
buf_0:
	dd 0 ;указатель на буфер изображения
	dw 20 ;+4 left
	dw 10 ;+6 top
	dd 160 ;+8 w
	dd 128 ;+12 h
	dd 0x80 ;+16 color
	db 24 ;+20 bit in pixel

;--------------------------------------------------
system_path db '/sys/lib/'
vectors_name db 'buf2d.obj',0
err_message_found_lib db 'Sorry I cannot load library buf2d.obj',0
head_f_i:
head_f_l db 'System error',0
err_message_import db 'Error on load import library buf2d.obj',0
;--------------------------------------------------

i_end: ;конец кода
	rb 1024
stacktop:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:

