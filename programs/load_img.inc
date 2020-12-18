;
; Макросы для декодирования изображений чрез функции библиотеки libimg.
;
; (1) макрос load_image_file:
;
; Проверяет размер полученного файла с изображением.
; Выделяет под него память и загружает туда файл.
; Определяет размер памяти необходимой для распаковки изображения.
; Перевыделяет память, и распаковывает изображение из форматов jpg, png, ...
;    в формат rgb который можно выводить на экран через сист. функцию.
; При необходимости можно также получить размеры открываемого изображения для
;    их дальнейшего использования в программе.
;
; В использующей программе должна быть подключена библиотека libimg,
;    объявлена переменная file_name размером около 4096 байт,
;    а также структура run_file_70 типа FileInfoBlock.
;
; (2) макрос include_image_file:
;
; Внедряет изображение в файл программы.
; Определяет размер памяти необходимой для распаковки изображения.
; Выделяет память, и распаковывает изображение из форматов jpg, png, ...
;    в формат rgb который можно выводить на экран через сист. функцию.
; При необходимости можно также получить размеры внедренного изображения для
;    их дальнейшего использования в программе.

; подключение некоторых необходимых файлов:
include 'dll.inc'
;include 'load_lib.mac'
include 'develop/libraries/libs-dev/libio/libio.inc'

align 4
open_b rb 560

txt_err_img_file:
if lang eq ru
	db 'Ошибка N'
.n: db '* при открытии файла '
else
	db 'Error N'
.n: db '*. Can',39,'t open file '
end if
.f: dd 0,0

; path - может быть переменной или строковым параметром
; buf - переменная куда будет записан указатель на изображение в формате rgb,
;    в случае если файл не откроется то будет записан 0
; img_w, img_h - переменные куда будут записаны размеры открываемого
;    изображения, не обязательные параметры
; bytes_p_p - сколько байт памяти брать на пиксель, не обязательный параметр
;    если не указан берется 3. Минимальное значение должно быть не меньше 3.
macro load_image_file path, buf, img_w, img_h, bytes_p_p
{
local .err_open
local .end_open

if path eqtype '' ;проверяем задан ли строкой параметр path
	local .path_str
	jmp @f
		.path_str db path ;формируем локальную переменную
		db 0
	@@:
	;32 - стандартный адрес по которому должен быть буфер с системным путем
	copy_path .path_str,[32],file_name,0
else
	copy_path path,[32],file_name,0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
end if
	mov dword[buf],0

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	mov dword[run_file_70.Buffer], open_b
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], file_name
	mcall SF_FILE,run_file_70
	or eax,eax
	jnz .err_open

		mov ecx,dword[open_b+32] ;+32 qword: размер файла в байтах
		stdcall mem.Alloc,ecx ;выделяем память для изображения
		mov [buf],eax
		mov [run_file_70.Function], SSF_READ_FILE
		mov [run_file_70.Position], 0
		mov [run_file_70.Flags], 0
		mov [run_file_70.Count], ecx
		mov [run_file_70.Buffer], eax
		mov byte[run_file_70+20], 0
		mov [run_file_70.FileName], file_name
		mcall SF_FILE,run_file_70 ;загружаем файл изображения
		test eax,eax
		jnz .err_open
		cmp ebx,0xffffffff
		je .end_open
			;определяем вид изображения и пишем его параметры
			stdcall [img_decode], [buf],ebx,0
			mov ebx,eax
			;определяем размер декодированного изображения
			mov ecx,[eax+4] ;+4 = image width
if img_w eq
else
			mov dword[img_w],ecx
end if
if img_h eq
			imul ecx,[eax+8] ;+8 = image height
else
			mov eax,[eax+8] ;+8 = image height
			mov dword[img_h],eax
			imul ecx,eax
end if
if bytes_p_p eq
			imul ecx,3 ;need for r,g,b
else
			imul ecx,bytes_p_p
end if
			stdcall mem.ReAlloc,[buf],ecx ;изменяем размер для буфера
			mov [buf],eax
			stdcall [img_to_rgb2], ebx,[buf] ;преобразуем изображение к формату rgb
			stdcall [img_destroy], ebx ;удаляем временный буфер с параметрами изображения
			jmp .end_open
	.err_open:
		add al,'0'
		mov byte[txt_err_img_file.n],al
if path eqtype ''
		mov eax,dword[.path_str]
		mov dword[txt_err_img_file.f],eax
		mov eax,dword[.path_str+4]
else
		mov eax,dword[path]
		mov dword[txt_err_img_file.f],eax
		mov eax,dword[path+4]
end if
		mov dword[txt_err_img_file.f+4],eax
		mov byte[txt_err_img_file.f+7],0
		notify_window_run txt_err_img_file
	.end_open:
}



; path - имя включаемого файла
; buf - переменная куда будет записан указатель на изображение в формате rgb
; img_w, img_h - переменные куда будут записаны размеры изображения,
;    не обязательные параметры
; bytes_p_p - сколько байт памяти брать на пиксель, не обязательный параметр
;    если не указан берется 3. Минимальное значение должно быть не меньше 3.
macro include_image_file path, buf, img_w, img_h, bytes_p_p
{
local .beg_file
local .end_file

jmp .end_file
align 4
.beg_file:
	file path
.end_file:

	;определяем вид изображения и пишем его параметры
	stdcall [img_decode], .beg_file,.end_file-.beg_file,0
	mov ebx,eax
	;определяем размер декодированного изображения
	mov ecx,[eax+4] ;+4 = image width
if img_w eq
else
	mov dword[img_w],ecx
end if
if img_h eq
	imul ecx,[eax+8] ;+8 = image height
else
	mov eax,[eax+8] ;+8 = image height
	mov dword[img_h],eax
	imul ecx,eax
end if
if bytes_p_p eq
	imul ecx,3 ;need for r,g,b
else
	imul ecx,bytes_p_p
end if
	stdcall mem.Alloc,ecx ;изменяем размер для буфера
	mov [buf],eax
	stdcall [img_to_rgb2], ebx,[buf] ;преобразуем изображение к формату rgb
	stdcall [img_destroy], ebx ;удаляем временный буфер с параметрами изображения
}