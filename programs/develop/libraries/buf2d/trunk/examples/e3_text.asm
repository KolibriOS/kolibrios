use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../../../../KOSfuncs.inc'
include '../../../../../macros.inc'
include '../../../../../proc32.inc'
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

IMAGE_FILE1_SIZE equ 128*144*3 ;размер файла с изображением 128 x 144

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
		mcall -1 ;exit not correct
	@@:
	mov	ebp,lib1
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:

	mcall 40,0x27
	mcall 48,3,sc,sizeof.system_colors ;получаем системные цвета

	stdcall [buf2d_create], buf_0 ;создаем буфер

	load_image_file 'font8x9.bmp', image_data_foto,IMAGE_FILE1_SIZE
	stdcall [buf2d_create_f_img], buf_1,[image_data_foto] ;создаем буфер
	stdcall mem.Free,[image_data_foto] ;освобождаем память

	stdcall [buf2d_conv_24_to_8], buf_1,1 ;делаем буфер прозрачности 8 бит
	stdcall [buf2d_bit_blt_alpha], buf_0, 70,10, buf_1,0x20d020 ;рисуем исходную текстовую матрицу 16*16
	stdcall [buf2d_convert_text_matrix], buf_1
	stdcall [buf2d_bit_blt_alpha], buf_0, 10,10, buf_1,0xff8080 ;рисуем преобразованную текстовую матрицу 1*256
	stdcall [buf2d_draw_text], buf_0, buf_1,some_text,70,170,0x4040ff ;рисуем строку с текстом

align 4
red_win:
	call draw_window

align 4
still: ;главный цикл
	mcall 10

	cmp al,0x1 ;изменилось положение окна
	jz red_win
	cmp al,0x2
	jz key
	cmp al,0x3
	jz button

	jmp still

align 4
key:
	push eax ebx
	mcall 2
;...
	pop ebx eax
	jmp still


align 4
draw_window:
	pushad
	mcall 12,1

	mov edx,[sc.work]
	or  edx,0x33000000
	mcall 0,(20 shl 16)+670,(20 shl 16)+520,,,caption ;создание окна

	stdcall [buf2d_draw], buf_0
	
	mcall 12,2
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
	mcall 17 ;получить код нажатой кнопки
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0 ;удаляем буфер
	stdcall [buf2d_delete],buf_1 ;удаляем буфер
	mcall -1 ;выход из программы

image_data dd 0 ;память для преобразования картинки функциями libimg
image_data_gray dd 0 ;память с преобразованным изображением в формате 8-bit
image_data_rgb dd 0 ;память с преобразованным изображением в формате rgb
image_data_foto dd 0

run_file_70 FileInfoBlock
caption db 'Draw text 13.09.11',0 ;подпись окна
sc system_colors  ;системные цвета

align 4
buf_0:
	dd 0 ;указатель на буфер изображения
	dw 5 ;+4 left
	dw 5 ;+6 top
	dd 640 ;+8 w
	dd 480 ;+12 h
	dd 0xffffff ;+16 color
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

some_text db 'Пример вывода теста в буфер, возможен вывод:',13,13
db ' * в несколько строк',13
db ' * можно задавать цвет текста',13,13
db 'Сегодня 13 сентября 2011.',0

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
	buf2d_resize dd sz_buf2d_resize
	buf2d_line dd sz_buf2d_line
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
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
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_flood_fill dd sz_buf2d_flood_fill
	buf2d_set_pixel dd sz_buf2d_set_pixel

	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
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
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_flood_fill db 'buf2d_flood_fill',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

i_end:
	rb 1024
stacktop:
	cur_dir_path rb 4096
	file_name:
		rb 4096
	plugin_path:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
mem:
