use32
	org 0x0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 0x1
	dd start
	dd i_end ;размер приложения
	dd mem
	dd stacktop
	dd 0
	dd sys_path

include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../develop/libraries/box_lib/load_lib.mac'
include '../../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../../dll.inc'

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'Просмотр информации Exif 15.07.14',0 ;подпись окна

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

run_file_70 FileInfoBlock
image_data dd 0 ;указатель на временную память. для нужен преобразования изображения

fn_toolbar db 'toolbar.png',0
IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
IMAGE_TOOLBAR_SIZE equ IMAGE_TOOLBAR_ICON_SIZE*5
image_data_toolbar dd 0

IMAGE_FILE1_SIZE equ 128*144*3+54 ;размер файла с изображением

memory_file_size dd 512*1024 ;размер памяти для открытия файлов (в начале 512 Kb, но может увеличиватся при необходимости)

macro load_image_file path,buf,size { ;макрос для загрузки изображений
	;path - может быть переменной или строковым параметром
	if path eqtype '' ;проверяем задан ли строкой параметр path
		jmp @f
			local .path_str
			.path_str db path ;формируем локальную переменную
			db 0
		@@:
		;32 - стандартный адрес по которому должен быть буфер с системным путем
		copy_path .path_str,[32],file_name,0x0
	else
		copy_path path,[32],file_name,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
	end if

	stdcall mem.Alloc, dword size ;выделяем память для изображения
	mov [buf],eax

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je @f
		;определяем вид изображения и переводим его во временный буфер image_data
		stdcall dword[img_decode], dword[buf],ebx,0
		mov dword[image_data],eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], dword[image_data],dword[buf]
		;удаляем временный буфер image_data
		stdcall dword[img_destroy], dword[image_data]
	@@:
}



align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mcall 48,3,sc,sizeof.system_colors
	mcall 40,0xC0000027
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	stdcall [buf2d_create], buf_0 ;создание буфера

	load_image_file 'font8x9.bmp', image_data_toolbar,IMAGE_FILE1_SIZE
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_1

	load_image_file fn_toolbar, image_data_toolbar,IMAGE_TOOLBAR_SIZE

	stdcall mem.Alloc,[memory_file_size]
	mov dword[open_file],eax
	
	call but_new_file

align 4
red_win:
	call draw_window

align 4
still:
	mcall 10

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6 ;мышь
	jne @f
		jmp mouse
	@@:
	jmp still

align 4
draw_window:
pushad
	mcall 12,1

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	xor eax,eax
	mov ebx,(20 shl 16)+480
	mov ecx,(20 shl 16)+410
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x10000000+0x20000000
	mov edi,caption
	int 0x40

	; *** создание кнопок на панель ***
	mov eax,8
	mov ebx,(5 shl 16)+20
	mov ecx,(5 shl 16)+20
	mov edx,3
	mov esi,[sc.work_button]
	int 0x40

	mov ebx,(30 shl 16)+20
	mov edx,4
	int 0x40

	mov ebx,(55 shl 16)+20
	mov edx,5
	int 0x40

	mov ebx,(85 shl 16)+20
	mov edx,6
	int 0x40

	mov ebx,(110 shl 16)+20
	mov edx,7
	int 0x40

	; *** рисование иконок на кнопках ***
	mov eax,7
	mov ebx,[image_data_toolbar]
	mov ecx,(16 shl 16)+16
	mov edx,(7 shl 16)+7 ;icon new
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40

	; *** рисование буфера ***
	stdcall [buf2d_draw], buf_0

	mcall 12,2
popad
	ret

align 4
key:
	mcall 2
	jmp still

align 4
mouse:

	jmp still

align 4
button:
	mcall 17
	cmp ah,3
	jne @f
		call but_new_file
	@@:
	cmp ah,4
	jne @f
		call but_open_file
	@@:
	cmp ah,5
	jne @f
		call but_save_file
	@@:
	cmp ah,6
	jne @f
		call but_1
	@@:
	cmp ah,7
	jne @f
		call but_2
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall [buf2d_delete],buf_1 ;удаляем буфер
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file]
	mcall -1


align 4
but_new_file:
	mov dword[open_file_size],0
	call draw_file
	ret

align 4
open_file dd 0 ;указатель на память для открытия файлов
open_file_size dd 0 ;размер открытого файла (должен быть не больше memory_file_size)

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 5
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	m2m [run_file_70.Buffer], [open_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp eax,0
	jne .end_open_file

	mov eax,[open_file]
	mov ebx,dword[eax+32] ;dword[eax+32] - размер открываемого файла
	mov dword[open_file_size],ebx ;ebx - размер открываемого файла
	;memory_file_size - размер выделенной памяти для файла
	cmp dword[memory_file_size],ebx
	jge @f
		;увеличиваем память если не хватило
		mov dword[memory_file_size],ebx
		stdcall mem.ReAlloc, dword[open_file],ebx
	@@:

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	m2m dword[run_file_70.Count], dword[open_file_size]
	m2m dword[run_file_70.Buffer],dword[open_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open_file

	mov [open_file_size],ebx
	;add ebx,dword[open_file]
	;mov byte[ebx],0 ;на случай если ранее был открыт файл большего размера чистим конец буфера с файлом
	stdcall [exif_get_app1], [open_file],h_app1
	mcall 71,1,openfile_path

	call draw_file
	.end_open_file:
	popad
	ret

align 4
draw_file:
pushad
	stdcall [buf2d_clear], buf_0, [buf_0.color]
	cmp dword[open_file_size],0
	je .open_file

	mov eax,1
	mov ebx,2
	.cycle_0:
		stdcall [exif_get_app1_tag], [h_app1],eax,txt_buf,80
		stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,4,ebx,0xb0
		inc eax
		add ebx,10
		cmp byte[txt_buf],0
		jne .cycle_0

	jmp @f
	.open_file:
		stdcall [buf2d_draw_text], buf_0, buf_1,txt_openfile,3,3,0xb000
	@@:
	stdcall [buf2d_draw], buf_0
popad
	ret

align 4
but_save_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_save_file
	;код при удачном открытии диалога

	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 2
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov ebx, dword[open_file]
	mov [run_file_70.Buffer], ebx
	mov ebx,[open_file_size]
	mov dword[run_file_70.Count], ebx ;размер файла
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;сохраняем файл изображения
	;cmp ebx,0xffffffff
	;je .end_save_file
	; ... сообщение о неудачном сохранении ...

	.end_save_file:
	popad
	ret

align 4
but_1:
	stdcall [exif_get_app1_tag], [h_app1],1,txt_buf,80
	notify_window_run txt_buf	
	ret

align 4
but_2:
	stdcall [exif_get_app1_tag], [h_app1],2,txt_buf,80
	notify_window_run txt_buf
	ret

;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type			dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать дтректорию
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/rd/1',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/rd/1/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'JPG',0
db 'JPEG',0
.end:
db 0



head_f_i:
head_f_l db 'Системная ошибка',0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
err_message_found_lib_0 db 'Не найдена библиотека ',39,'proc_lib.obj',39,0
err_message_import_0 db 'Ошибка при импорте библиотеки ',39,'proc_lib.obj',39,0

system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
err_message_found_lib_1 db 'Не найдена библиотека ',39,'libimg.obj',39,0
err_message_import_1 db 'Ошибка при импорте библиотеки ',39,'libimg.obj',39,0

system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
err_msg_found_lib_2 db 'Не найдена библиотека ',39,'buf2d.obj',39,0
err_msg_import_2 db 'Ошибка при импорте библиотеки ',39,'buf2d',39,0

system_dir_3 db '/sys/lib/'
lib_name_3 db 'exif.obj',0
err_msg_found_lib_3 db 'Не найдена библиотека ',39,'exif.obj',39,0
err_msg_import_3 db 'Ошибка при импорте библиотеки ',39,'exif',39,0

l_libs_start:
	lib_0 l_libs lib_name_0, sys_path, file_name, system_dir_0,\
		err_message_found_lib_0, head_f_l, proclib_import,err_message_import_0, head_f_i
	lib_1 l_libs lib_name_1, sys_path, file_name, system_dir_1,\
		err_message_found_lib_1, head_f_l, import_libimg, err_message_import_1, head_f_i
	lib_2 l_libs lib_name_2, sys_path, library_path, system_dir_2,\
		err_msg_found_lib_2,head_f_l,import_buf2d,err_msg_import_2,head_f_i
	lib_3 l_libs lib_name_3, sys_path, library_path, system_dir_3,\
		err_msg_found_lib_3,head_f_l,import_exif,err_msg_import_3,head_f_i
l_libs_end:

align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw

	dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0

align 4
proclib_import: ;описание экспортируемых функций
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0

align 4
import_exif: ;описание экспортируемых функций
	exif_get_app1 dd sz_des_encryption
	exif_get_app1_tag dd sz_des_decryption
dd 0,0
	sz_des_encryption db 'exif_get_app1',0
	sz_des_decryption db 'exif_get_app1_tag',0
	;exif_get_image_160_120

sc system_colors 

align 16
procinfo process_information 

align 4
buf_0: dd 0 ;указатель на буфер изображения
	dw 5 ;+4 left
	dw 31 ;+6 top
.w: dd 456 ;+8 w
.h: dd 350 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_1:
	dd 0 ;указатель на буфер изображения
	dw 25 ;+4 left
	dw 25 ;+6 top
	dd 128 ;+8 w
	dd 144 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

h_app1 dd 0
txt_openfile db 'Откройте файл изображения в формате *.jpg.',0
txt_buf rb 80

i_end:
	rb 2048
stacktop:
	sys_path rb 1024
	file_name:
		rb 1024 ;4096 
	library_path rb 1024
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
