use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1,start,i_end,mem,stacktop,0,sys_path

include '../../../../../macros.inc'
include '../../../../../proc32.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../load_lib.mac'
include '../../../../../dll.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

IMAGE_FILE1_SIZE equ 200*186*3 ;размер файла с изображением 200 x 100

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4]
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения
vox_offs_tree_table equ 4

macro load_image_file path,buf,size { ;макрос для загрузки изображений
	;path - может быть переменной или строковым параметром
	if path eqtype '' ;проверяем задан ли строкой параметр path
		jmp @f
			local .path_str
			.path_str db path ;формируем локальную переменную
			db 0
		@@:
		;32 - стандартный адрес по которому должен быть буфер с системным путем
		copy_path .path_str,[32],file_name,0
	else
		copy_path path,[32],file_name,0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
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
	load_libraries l_libs_start,load_lib_end

	;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mov	ebp,lib1
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:

	mcall SF_SET_EVENTS_MASK,0x27
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors ;получаем системные цвета

	load_image_file 'img1.png',image_data_rgb, IMAGE_FILE1_SIZE
	stdcall [buf2d_create_f_img], buf_0,[image_data_rgb] ;создаем буфер
	stdcall [buf2d_create_f_img], buf_1,[image_data_rgb] ;создаем буфер
	stdcall [buf2d_create_f_img], buf_2,[image_data_rgb] ;создаем буфер
	stdcall mem.Free,[image_data_rgb] ;освобождаем память

	stdcall [buf2d_filter_dither], buf_1,2
	stdcall [buf2d_filter_dither], buf_2,3

align 4
red_win:
	call draw_window

align 4
still: ;главный цикл
	mcall SF_WAIT_EVENT

	cmp al,1 ;изменилось положение окна
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button

	jmp still

align 4
key:
	push eax ebx
	mcall SF_GET_KEY
;...
	pop ebx eax
	jmp still


align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mov edx,[sc.work]
	or  edx,0x33000000
	mcall SF_CREATE_WINDOW,(20 shl 16)+670,(20 shl 16)+520,,,caption ;создание окна

	stdcall [buf2d_draw], buf_0
	stdcall [buf2d_draw], buf_1
	stdcall [buf2d_draw], buf_2
	
	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

system_dir0 db '/sys/lib/'
name_buf2d db 'buf2d.obj',0

system_dir1 db '/sys/lib/'
name_libimg db 'libimg.obj',0

;library structures
l_libs_start:
	lib0 l_libs name_buf2d,  file_name, system_dir0, import_buf2d_lib
	lib1 l_libs name_libimg, file_name, system_dir1, import_libimg
load_lib_end:

align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0 ;удаляем буфер
	stdcall [buf2d_delete],buf_1 ;удаляем буфер
	stdcall [buf2d_delete],buf_2 ;удаляем буфер
	mcall SF_TERMINATE_PROCESS

image_data dd 0 ;память для преобразования картинки функциями libimg
image_data_gray dd 0 ;память с преобразованным изображением в формате 8-bit
image_data_rgb dd 0 ;память с преобразованным изображением в формате rgb
image_data_foto dd 0

run_file_70 FileInfoBlock
caption db 'Draw images 28.11.20',0 ;подпись окна
sc system_colors  ;системные цвета

align 4
buf_0:
	dd 0 ;указатель на буфер изображения
	dw 5 ;+4 left
	dw 5 ;+6 top
	dd 200 ;+8 w
	dd 186 ;+12 h
	dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_1:
	dd 0 ;указатель на буфер изображения
	dw 5 ;+4 left
	dw 200 ;+6 top
	dd 200 ;+8 w
	dd 186 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_2:
	dd 0 ;указатель на буфер изображения
	dw 210 ;+4 left
	dw 200 ;+6 top
	dd 200 ;+8 w
	dd 186 ;+12 h
	dd 0 ;+16 color
	db 24 ;+20 bit in pixel

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
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_filter_dither dd sz_buf2d_filter_dither
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_filter_dither db 'buf2d_filter_dither',0

align 16
i_end:
	sys_path rb 4096
	file_name:
		rb 4096
	plugin_path:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
		rb 1024
stacktop:
mem:
