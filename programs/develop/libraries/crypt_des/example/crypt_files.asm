use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1, start, i_end, mem, stacktop, 0, sys_path

include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../KOSfuncs.inc'
include '../../../../load_lib.mac'
include '../../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../../dll.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'Шифрование алгоритмом DES 21.05.25',0 ;подпись окна

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

max_open_file_size equ 64*1024 ;64 Kb

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

	mov eax,SF_FILE
	mov [run_file_70.Function], SSF_READ_FILE
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
		stdcall [img_decode], [buf],ebx,0
		mov [image_data],eax
		;преобразуем изображение к формату rgb
		stdcall [img_to_rgb2], [image_data],[buf]
		;удаляем временный буфер image_data
		stdcall [img_destroy], [image_data]
	@@:
}



align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0xC0000027
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	stdcall [buf2d_create], buf_0 ;создание буфера

	load_image_file 'font8x9.bmp', image_data_toolbar,IMAGE_FILE1_SIZE
	stdcall [buf2d_create_f_img], buf_1,[image_data_toolbar] ;создаем буфер
	stdcall mem.Free,[image_data_toolbar] ;освобождаем память
	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_convert_text_matrix], buf_1

	load_image_file fn_toolbar, image_data_toolbar,IMAGE_TOOLBAR_SIZE

	stdcall mem.Alloc,max_open_file_size
	mov dword[open_file],eax

	call but_new_file

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_WAIT_EVENT

	cmp al,EV_REDRAW
	jz red_win
	cmp al,EV_KEY
	jz key
	cmp al,EV_BUTTON
	jz button
	cmp al,EV_MOUSE
	jne @f
		jmp mouse
	@@:
	jmp still

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x10000000+0x20000000
	mov edi,caption
	mcall SF_CREATE_WINDOW, (20 shl 16)+480, (20 shl 16)+410

	; *** создание кнопок на панель ***
	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON, (5 shl 16)+20, (5 shl 16)+20, 3
	mcall ,(30 shl 16)+20,,4
	mcall ,(55 shl 16)+20,,5
	mcall ,(85 shl 16)+20,,6
	mcall ,(110 shl 16)+20,,7

	; *** рисование иконок на кнопках ***
	mcall SF_PUT_IMAGE, [image_data_toolbar], (16 shl 16)+16, (7 shl 16)+7 ;icon new

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

	stdcall [edit_box_draw], edit1

	; *** рисование буфера ***
	stdcall [buf2d_draw], buf_0

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
key:
	mcall SF_GET_KEY
	stdcall [edit_box_key], dword edit1
	jmp still

align 4
mouse:
	stdcall [edit_box_mouse], edit1
	jmp still

align 4
button:
	mcall SF_GET_BUTTON
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
	mcall SF_TERMINATE_PROCESS


align 4
but_new_file:
	mov dword[open_file_size],0
	call draw_file
	ret

align 4
open_file dd 0 ;указатель на память для открытия файлов
open_file_size dd 0 ;размер открытого файла (должен быть не больше max_open_file_size)

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога

	mov eax,SF_FILE
	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], max_open_file_size
	m2m [run_file_70.Buffer], [open_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je .end_open_file

	mov [open_file_size],ebx
	add ebx,[open_file]
	mov byte[ebx],0 ;на случай если ранее был открыт файл большего размера чистим конец буфера с файлом
	mcall SF_SET_CAPTION,1,openfile_path

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
	mov eax,[open_file]
	mov ebx,3
	mov edx,[open_file_size]
	.cycle_0:
		mov edi,txt_buf
		mov esi,eax
		mov ecx,56
		;cld
		rep movsb
		mov byte[edi],0
		mov edi,txt_buf
		mov ecx,56
		.cycle_1:
			cmp byte[edi],0
			je @f
			cmp byte[edi],13
			je @f
			jmp .ok
			@@:
				mov byte[edi],' ' ;непечатные символы заменяем на пробел
			.ok:
			inc edi
			loop .cycle_1
		stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,4,ebx,0xb0
		sub edx,56
		cmp edx,1
		jl @f
		add eax,56
		add ebx,10
		cmp ebx,[buf_0.h]
		jl .cycle_0
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

	mov eax,SF_FILE
	mov [run_file_70.Function], SSF_CREATE_FILE
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
push eax
	mov eax,[open_file_size]
	shr eax,3
	stdcall [des_encryption], txt_key,mem_key,[open_file],eax
pop eax
	call draw_file
	ret

align 4
but_2:
push eax
	mov eax,[open_file_size]
	shr eax,3
	stdcall [des_decryption], txt_key,mem_key,[open_file],eax
pop eax
	call draw_file
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

default_dir db '/sys',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'TXT',0
db 'ASM',0
.end:
db 0


system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'crypt_des.obj',0
system_dir_4 db '/sys/lib/'
lib_name_4 db 'box_lib.obj',0

l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, library_path, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, library_path, system_dir_3, import_des
	lib_4 l_libs lib_name_4, library_path, system_dir_4, import_box_lib
l_libs_end:

include '../../libs-dev/libimg/import.inc'

align 4
import_proclib: ;описание экспортируемых функций
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

include '../../buf2d/import.inc'

align 4
import_des: ;описание экспортируемых функций
	des_encryption dd sz_des_encryption
	des_decryption dd sz_des_decryption
dd 0,0
	sz_des_encryption db 'des_encryption',0
	sz_des_decryption db 'des_decryption',0

include '../../box_lib/import.inc'

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

edit1 edit_box 58, 140,8, 0xffffff, 0xff, 0x80ff, 0, 0x8000, 8, txt_key, mouse_dd, ed_focus+ed_always_focus,8,8

txt_openfile db 'Откройте файл для шифрования или дешифрования.',0
txt_key db 'des_0123',0

align 16
i_end:
	txt_buf rb 80
	mem_key rb 120
	mouse_dd rd 1
	procinfo process_information
	sc system_colors 
	rb 2048
stacktop:
	sys_path rb 1024
	file_name rb 1024 ;4096 
	library_path rb 1024
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
