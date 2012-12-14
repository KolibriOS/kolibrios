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

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

max_open_file_size equ 64*1024 ;64 Kb

align 4
open_file_vox dd 0 ;указатель на область для открытия файлов
run_file_70 FileInfoBlock
f_name db 'vaz2106.vox',0

vox_offs_tree_table equ 4

;поворот объекта на 90 градусов
;x0y0 - x1y0
;x1y0 - x1y1
;x0y1 - x0y0
;x1y1 - x0y1
align 4
proc vox_obj_rot_z uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table
	mov ecx,2
	cld
	@@:
		mov eax,dword[ebx]
		mov byte[ebx+1],al
		mov byte[ebx+3],ah
		shr eax,16
		mov byte[ebx],al
		mov byte[ebx+2],ah
		add ebx,4
		loop @b
	ret
endp



align 4
start:
	load_library vectors_name, cur_dir_path, library_path, system_path, \
		err_message_found_lib, head_f_l, import_buf2d_lib, err_message_import, head_f_i
	cmp eax,-1
	jz button.exit

	mcall 40,0x27
	stdcall [buf2d_create], buf_0 ;создаем буфер
	stdcall [buf2d_create], buf_z

	stdcall mem.Alloc,max_open_file_size
	mov dword[open_file_vox],eax

	copy_path f_name,[32],file_name,0x0

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], max_open_file_size
	m2m [run_file_70.Buffer], [open_file_vox]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;загружаем воксельный объект

	stdcall [buf2d_vox_obj_draw_1g], buf_0, buf_z, [open_file_vox], 0,0, 7
	stdcall vox_obj_rot_z, [open_file_vox]
	stdcall [buf2d_vox_obj_draw_1g], buf_0, buf_z, [open_file_vox], 128,0, 7
	stdcall vox_obj_rot_z, [open_file_vox]
	stdcall [buf2d_vox_obj_draw_1g], buf_0, buf_z, [open_file_vox], 0,128, 7
	stdcall vox_obj_rot_z, [open_file_vox]
	stdcall [buf2d_vox_obj_draw_1g], buf_0, buf_z, [open_file_vox], 128,128, 7

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

	mov edx,0x33000000
	mcall 0,(50 shl 16)+330,(30 shl 16)+295,,,caption

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
	stdcall [buf2d_delete],buf_z
	stdcall mem.Free,[open_file_vox]
	mcall -1 ;выход из программы

caption db 'Test buf2d library, [Esc] - exit',0

;--------------------------------------------------
align 4
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete

	;воксельные функции:
	;buf2d_vox_brush_create dd sz_buf2d_vox_brush_create
	;buf2d_vox_brush_delete dd sz_buf2d_vox_brush_delete
	buf2d_vox_obj_draw_1g dd sz_buf2d_vox_obj_draw_1g
	;buf2d_vox_obj_get_img_w_3g dd sz_buf2d_vox_obj_get_img_w_3g
	;buf2d_vox_obj_get_img_h_3g dd sz_buf2d_vox_obj_get_img_h_3g
	;buf2d_vox_obj_draw_3g dd sz_buf2d_vox_obj_draw_3g
	;buf2d_vox_obj_draw_3g_scaled dd sz_buf2d_vox_obj_draw_3g_scaled
	;buf2d_vox_obj_draw_3g_shadows dd sz_buf2d_vox_obj_draw_3g_shadows
	;buf2d_vox_obj_draw_pl dd sz_buf2d_vox_obj_draw_pl
	;buf2d_vox_obj_draw_pl_scaled dd sz_buf2d_vox_obj_draw_pl_scaled

	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0

	;воксельные функции:
	;sz_buf2d_vox_brush_create db 'buf2d_vox_brush_create',0
	;sz_buf2d_vox_brush_delete db 'buf2d_vox_brush_delete',0
	sz_buf2d_vox_obj_draw_1g db 'buf2d_vox_obj_draw_1g',0
	;sz_buf2d_vox_obj_get_img_w_3g db 'buf2d_vox_obj_get_img_w_3g',0
	;sz_buf2d_vox_obj_get_img_h_3g db 'buf2d_vox_obj_get_img_h_3g',0
	;sz_buf2d_vox_obj_draw_3g db 'buf2d_vox_obj_draw_3g',0
	;sz_buf2d_vox_obj_draw_3g_scaled db 'buf2d_vox_obj_draw_3g_scaled',0
	;sz_buf2d_vox_obj_draw_3g_shadows db 'buf2d_vox_obj_draw_3g_shadows',0
	;sz_buf2d_vox_obj_draw_pl db 'buf2d_vox_obj_draw_pl',0
	;sz_buf2d_vox_obj_draw_pl_scaled db 'buf2d_vox_obj_draw_pl_scaled',0

align 4
buf_0:
	dd 0 ;указатель на буфер изображения
	dw 20 ;+4 left
	dw 5 ;+6 top
	dd 256 ;+8 w
	dd 256 ;+12 h
	dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_z:
	dd 0 ;указатель на буфер изображения
	dw 20 ;+4 left
	dw 5 ;+6 top
	dd 256 ;+8 w
	dd 256 ;+12 h
	dd 0 ;+16 color
	db 32 ;+20 bit in pixel

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
	file_name:
		rb 4096
cur_dir_path:
	rb 4096
library_path:
	rb 4096
mem:

