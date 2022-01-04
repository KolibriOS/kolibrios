format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../../macros.inc'
include '../../../../proc32.inc'

;-----------------------------------------------------------------------------
mem.alloc   dd ? ;функция для выделения памяти
mem.free    dd ? ;функция для освобождения памяти
mem.realloc dd ? ;функция для перераспределения памяти
dll.load    dd ?

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4]
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

struct buf_2d_header
	img_data dd ?
	left dw ? ;+4 left
	top dw ? ;+6 top
	size_x dd ? ;+8 w
	size_y dd ? ;+12 h
	color dd ? ;+16 color
	bit_pp db ? ;+21 bit in pixel
ends

macro swap v1, v2 {
  push v1
  push v2
  pop v1
  pop v2
}

;флаги, для функции обрезания буфера
BUF2D_OPT_CROP_TOP equ 1 ;обрезка сверху
BUF2D_OPT_CROP_LEFT equ 2 ;обрезка слева
BUF2D_OPT_CROP_BOTTOM equ 4 ;обрезка снизу
BUF2D_OPT_CROP_RIGHT equ 8 ;обрезка справа
BUF2D_BIT_OPT_CROP_TOP equ 0
BUF2D_BIT_OPT_CROP_LEFT equ 1
BUF2D_BIT_OPT_CROP_BOTTOM equ 2
BUF2D_BIT_OPT_CROP_RIGHT equ 3

;input:
; eax = указатель на функцию выделения памяти
; ebx = ... освобождения памяти
; ecx = ... перераспределения памяти
; edx = ... загрузки библиотеки (пока не используется)
align 16
lib_init:
	mov dword[mem.alloc], eax
	mov dword[mem.free], ebx
	mov dword[mem.realloc], ecx
	mov dword[dll.load], edx
	ret

include 'fun_draw.inc' ;функции рисования в буфере

;создание буфера
align 4
proc buf_create, buf_struc:dword
	pushad
	mov edi,dword[buf_struc]
	mov ecx,buf2d_w
	mov ebx,buf2d_h
	imul ecx,ebx
	cmp buf2d_bits,24
	jne @f
		lea ecx,[ecx+ecx*2] ; 24 bit = 3
		;;;inc ecx ;запасной байт в конце буфера, что-бы не глючили некоторые функции на изображениях кратных 4К
	@@:
	cmp buf2d_bits,32
	jne @f
		shl ecx,2 ; 32 bit = 4
	@@:
	invoke mem.alloc,ecx
	mov buf2d_data,eax

	stdcall buf_clear,edi,buf2d_color ;очистка буфера фоновым цветом
	popad
	ret
endp

;создание буфера на основе изображения rgb
align 4
proc buf_create_f_img, buf_struc:dword, rgb_data:dword
	pushad
	mov edi,dword[buf_struc]
	mov ecx,buf2d_w
	cmp ecx,1
	jl .error
	mov ebx,buf2d_h
	cmp ebx,1
	jl .error
	imul ecx,ebx
	cmp buf2d_bits,24
	jne @f
		lea ecx,[ecx+ecx*2] ; 24 bit = 3
	@@:
	cmp buf2d_bits,32
	jne @f
		shl ecx,2 ; 32 bit = 4
	@@:
	invoke mem.alloc,ecx
	mov buf2d_data,eax

	cmp buf2d_bits,24
	jne @f
		cld
		mov esi,[rgb_data]
		or esi,esi
		jz @f
		mov edi,eax ;eax=buf2d_data
		rep movsb ;копируем биты изображения в буфер
		jmp .end_create
	@@:
		stdcall buf_clear,edi,buf2d_color ;очистка буфера фоновым цветом
		jmp .end_create
	.error:
		stdcall print_err,sz_buf2d_create_f_img,txt_err_size_0
	.end_create:
	popad
	ret
endp

;функция для обрезания буферов 8 и 24 битных, по заданому цвету.
;параметр opt задается комбинацией констант:
; BUF2D_OPT_CROP_TOP - обрезка сверху
; BUF2D_OPT_CROP_LEFT - обрезка слева
; BUF2D_OPT_CROP_BOTTOM - обрезка снизу
; BUF2D_OPT_CROP_RIGHT - обрезка справа
align 4
proc buf_crop_color, buf_struc:dword, color:dword, opt:dword
locals
	crop_r dd ?
endl
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .24end_f

	bt dword[opt],BUF2D_BIT_OPT_CROP_BOTTOM
	jae .24no_crop_bottom
		mov eax,dword[color]
		mov edx,eax ;ax = colors - r,g
		shr edx,16 ;dl = color - b
		mov ecx,buf2d_h
		cmp ecx,1
		jle .24no_crop_bottom ;проверяем на случай если высота буфера 1 пиксель
		mov ebx,buf2d_w
		imul ecx,ebx
		lea esi,[ecx+ecx*2] ;esi=3*ecx
		add esi,buf2d_data
		cld
		@@:
			sub esi,3
			cmp word[esi],ax
			jne @f
			cmp byte[esi+2],dl
			jne @f
			loop @b
		@@:
		lea ebx,[ebx+ebx*2]
		xor edx,edx
		mov eax,buf2d_h
		imul eax,ebx
		add eax,buf2d_data ;eax - указатель на конец буфера изображения
		@@:
			add esi,ebx
			cmp esi,eax
			jge @f
			inc edx ;вычисляем число полных строк для обрезания
			loop @b
		@@:
		cmp edx,0
		je .24no_crop_bottom
			cmp edx,buf2d_h
			jge .24no_crop_bottom ;что-бы не получить пустой буфер
			sub buf2d_h,edx ;уменьшаем высоту буфера
			mov ecx,buf2d_h
			imul ecx,ebx ;ecx = новый размер изображения
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.24no_crop_bottom:

	bt dword[opt],BUF2D_BIT_OPT_CROP_TOP
	jae .24no_crop_top
		mov eax,dword[color]
		mov edx,eax ;ax = colors - r,g
		shr edx,16 ;dl = color - b
		mov esi,buf2d_data
		mov ecx,buf2d_h
		cmp ecx,1
		jle .24no_crop_top ;проверяем на случай если высота буфера 1 пиксель
		dec ecx ;при обрезании должна остаться минимум 1-на строка пикселей
		mov ebx,buf2d_w
		imul ecx,ebx
		cld
		@@:
			cmp word[esi],ax
			jne @f
			cmp byte[esi+2],dl
			jne @f
			add esi,3
			loop @b
		@@:
		lea ebx,[ebx+ebx*2]
		xor edx,edx
		@@:
			sub esi,ebx
			cmp esi,buf2d_data
			jl @f
			inc edx ;вычисляем число полных строк для обрезания
			loop @b
		@@:
		cmp edx,0
		je .24no_crop_top
			xor eax,eax
			sub eax,edx
			mov ebx,buf2d_h
			sub ebx,edx
			stdcall buf_offset_h, edi, eax, edx, ebx ;сдвигаем изображение в буфере вверх (eax<0)
			sub buf2d_h,edx ;уменьшаем высоту буфера
			mov ecx,buf2d_h
			add buf2d_t,dx ;сдвигаем отступ вниз, на число обрезанных строк
			mov ebx,buf2d_w
			imul ecx,ebx
			lea ecx,[ecx+ecx*2]
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.24no_crop_top:

	bt dword[opt],BUF2D_BIT_OPT_CROP_RIGHT
	jae .24no_crop_right
		mov eax,dword[color]
		mov edx,eax ;ax = colors - r,g
		shr edx,16 ;dl = color - b
		mov ebx,buf2d_w
		cmp ebx,1
		jle .24no_crop_right ;на случай если ширина буфера 1 пиксель
		lea ebx,[ebx+ebx*2]
		mov esi,ebx
		imul esi,buf2d_h
		add esi,buf2d_data ;esi - указатель на конец буфера изображения
		mov dword[crop_r],0
		cld
		.24found_beg_right:
		sub esi,3 ;двигаемся на 1-ну колонку влево
		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			cmp word[esi],ax
			jne .24found_right
			cmp byte[esi+2],dl
			jne .24found_right
			sub esi,ebx ;прыгаем на верхнюю строку
			loop @b
		inc dword[crop_r]

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp dword[crop_r],ecx
		jge .24found_right

		sub esi,3 ;двигаемся на 1-ну колонку влево
		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			add esi,ebx ;прыгаем на нижнюю строку
			cmp word[esi],ax
			jne .24found_right
			cmp byte[esi+2],dl
			jne .24found_right
			loop @b
		inc dword[crop_r]

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp dword[crop_r],ecx
		jl .24found_beg_right

		.24found_right:
		cmp dword[crop_r],0
		je .24no_crop_right
			mov ecx,buf2d_w
			sub ecx,dword[crop_r]
			stdcall img_rgb_crop_r, buf2d_data, buf2d_w, ecx, buf2d_h ;обрезаем буфер, по новому размеру
			mov buf2d_w,ecx ;ставим новую ширину для буфера
			mov ebx,buf2d_h
			imul ecx,ebx
			lea ecx,[ecx+ecx*2]
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.24no_crop_right:

	bt dword[opt],BUF2D_BIT_OPT_CROP_LEFT
	jae .24no_crop_left
		mov eax,dword[color]
		mov edx,eax ;ax = colors - r,g
		shr edx,16 ;dl = color - b
		mov ebx,buf2d_w
		cmp ebx,1
		jle .24no_crop_left ;на случай если ширина буфера 1 пиксель
		lea ebx,[ebx+ebx*2]
		mov esi,buf2d_data ;esi - указатель на начоло буфера изображения
		mov dword[crop_r],0
		cld
		.24found_beg_left:

		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			cmp word[esi],ax
			jne .24found_left
			cmp byte[esi+2],dl
			jne .24found_left
			add esi,ebx ;прыгаем на нижнюю строку
			loop @b
		inc dword[crop_r]
		add esi,3 ;двигаемся на 1-ну колонку вправо

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp dword[crop_r],ecx
		jge .24found_left

		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			sub esi,ebx ;прыгаем на верхнюю строку
			cmp word[esi],ax
			jne .24found_left
			cmp byte[esi+2],dl
			jne .24found_left
			loop @b
		inc dword[crop_r]
		add esi,3 ;двигаемся на 1-ну колонку вправо

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp dword[crop_r],ecx
		jl .24found_beg_left

		.24found_left:
		cmp dword[crop_r],0
		je .24no_crop_left
			mov ecx,buf2d_w
			sub ecx,dword[crop_r]
			stdcall img_rgb_crop_l, buf2d_data, buf2d_w, ecx, buf2d_h ;обрезаем буфер, по новому размеру
			mov buf2d_w,ecx ;ставим новую ширину для буфера
			mov ebx,buf2d_h
			imul ecx,ebx
			lea ecx,[ecx+ecx*2]
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
			mov eax,dword[crop_r]
			add buf2d_l,ax
	.24no_crop_left:

	.24end_f:


	cmp buf2d_bits,8
	jne .8end_f

	bt dword[opt],BUF2D_BIT_OPT_CROP_BOTTOM
	jae .8no_crop_bottom
		mov eax,dword[color]
		mov esi,buf2d_data
		mov ecx,buf2d_h
		cmp ecx,1
		jle .8no_crop_bottom ;проверяем на случай если высота буфера 1 пиксель
		mov ebx,buf2d_w
		imul ecx,ebx
		mov esi,ecx
		add esi,buf2d_data
		cld
		@@:
			dec esi
			cmp byte[esi],al
			jne @f
			loop @b
		@@:
		xor edx,edx
		mov eax,buf2d_h
		imul eax,ebx
		add eax,buf2d_data ;eax - указатель на конец буфера изображения
		@@:
			add esi,ebx
			cmp esi,eax
			jge @f
			inc edx
			loop @b
		@@:
		cmp edx,0
		je .8no_crop_bottom
			cmp edx,buf2d_h
			jge .8no_crop_bottom ;что-бы не получить пустой буфер
			sub buf2d_h,edx ;уменьшаем высоту буфера
			mov ecx,buf2d_h
			imul ecx,ebx ;ecx = новый размер изображения
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.8no_crop_bottom:

	bt dword[opt],BUF2D_BIT_OPT_CROP_TOP
	jae .8no_crop_top
		mov eax,dword[color]
		mov esi,buf2d_data
		mov ecx,buf2d_h
		cmp ecx,1
		jle .8no_crop_top ;проверяем на случай если высота буфера 1 пиксель
		dec ecx ;при обрезании должна остаться минимум 1-на строка пикселей
		mov ebx,buf2d_w
		imul ecx,ebx
		cld
		@@:
			cmp byte[esi],al
			jne @f
			inc esi
			loop @b
		@@:
		xor edx,edx
		@@:
			sub esi,ebx
			cmp esi,buf2d_data
			jl @f
			inc edx
			loop @b
		@@:
		cmp edx,0
		je .8no_crop_top
			xor eax,eax
			sub eax,edx
			mov ebx,buf2d_h
			sub ebx,edx
			stdcall buf_offset_h, edi, eax, edx, ebx
			mov ecx,buf2d_h
			sub ecx,edx
			mov buf2d_h,ecx ;уменьшаем высоту буфера
			add buf2d_t,dx ;сдвигаем отступ вниз, на число обрезанных строк
			mov ebx,buf2d_w
			imul ecx,ebx
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.8no_crop_top:

	bt dword[opt],BUF2D_BIT_OPT_CROP_RIGHT
	jae .8no_crop_right
		mov eax,dword[color]
		mov ebx,buf2d_w
		cmp ebx,1
		jle .8no_crop_right ;на случай если ширина буфера 1 пиксель
		mov esi,ebx
		imul esi,buf2d_h
		add esi,buf2d_data ;esi - указатель на конец буфера изображения
		xor edx,edx
		cld

		.8found_beg:
		dec esi ;двигаемся на 1-ну колонку влево
		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			cmp byte[esi],al
			jne .8found
			sub esi,ebx ;прыгаем на верхнюю строку
			loop @b
		inc edx
		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp edx,ecx
		jge .8found

		dec esi ;двигаемся на 1-ну колонку влево
		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			add esi,ebx ;прыгаем на нижнюю строку
			cmp byte[esi],al
			jne .8found
			loop @b
		inc edx

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp edx,ecx
		jl .8found_beg

		.8found:
		cmp edx,0
		je .8no_crop_right
			mov ecx,buf2d_w
			sub ecx,edx
			stdcall img_gray_crop_r, buf2d_data, buf2d_w, ecx, buf2d_h ;обрезаем буфер, по новому размеру
			mov buf2d_w,ecx ;ставим новую ширину для буфера
			mov ebx,buf2d_h
			imul ecx,ebx
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
	.8no_crop_right:

	bt dword[opt],BUF2D_BIT_OPT_CROP_LEFT
	jae .8no_crop_left
		mov eax,dword[color]
		mov ebx,buf2d_w
		cmp ebx,1
		jle .8no_crop_left ;на случай если ширина буфера 1 пиксель
		mov esi,buf2d_data ;esi - указатель на начоло буфера изображения
		mov edx,0
		cld
		.8found_beg_left:

		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			cmp word[esi],ax
			jne .8found_left
			add esi,ebx ;прыгаем на нижнюю строку
			loop @b
		inc edx
		inc esi ;двигаемся на 1-ну колонку вправо

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp edx,ecx
		jge .8found_left

		mov ecx,buf2d_h ;восстановление ecx для нового цикла
		@@:
			sub esi,ebx ;прыгаем на верхнюю строку
			cmp word[esi],ax
			jne .8found_left
			loop @b
		inc edx
		inc esi ;двигаемся на 1-ну колонку вправо

		mov ecx,buf2d_w
		dec ecx ;1 колонка на запас
		cmp edx,ecx
		jl .8found_beg_left

		.8found_left:
		cmp edx,0
		je .8no_crop_left
			mov ecx,buf2d_w
			sub ecx,edx
			stdcall img_gray_crop_l, buf2d_data, buf2d_w, ecx, buf2d_h ;обрезаем буфер, по новому размеру
			mov buf2d_w,ecx ;ставим новую ширину для буфера
			mov ebx,buf2d_h
			imul ecx,ebx
			invoke mem.realloc,buf2d_data,ecx
			mov buf2d_data,eax ;на случай если изменился указатель на данные
			mov eax,edx
			add buf2d_l,ax
	.8no_crop_left:

	.8end_f:

	popad
	ret
endp

;обрезаем цветное изображение с правой стороны
;input:
;data_rgb - pointer to rgb data
;size_w_old - width img in pixels
;size_w_new - new width img in pixels
;size_h - height img in pixels
align 4
proc img_rgb_crop_r, data_rgb:dword, size_w_old:dword, size_w_new:dword, size_h:dword
	pushad
	mov eax, dword[size_w_old]
	lea eax, dword[eax+eax*2] ;eax = width(old) * 3(rgb)
	mov ebx, dword[size_w_new]
	lea ebx, dword[ebx+ebx*2] ;ebx = width(new) * 3(rgb)
	mov edx, dword[size_h]
	mov edi, dword[data_rgb] ;edi - получает данные
	mov esi, edi
	add edi, ebx
	add esi, eax
	cld
	@@:
		dec edx ;уменьшаем счетчик оставшихся строк на 1
		cmp edx,0
		jle @f
		mov ecx, ebx
		rep movsb ;перенос (копирование) строки пикселей
		add esi,eax ;переход на новую строчку изображения
		sub esi,ebx
		jmp @b
	@@:
	popad
	ret
endp

;обрезаем серое изображение с правой стороны
;input:
;data_gray - pointer to gray data
;size_w_old - width img in pixels
;size_w_new - new width img in pixels
;size_h - height img in pixels
align 4
proc img_gray_crop_r, data_gray:dword, size_w_old:dword, size_w_new:dword, size_h:dword
	pushad
	mov eax, dword[size_w_old]
	mov ebx, dword[size_w_new]
	mov edx, dword[size_h]
	mov edi, dword[data_gray] ;edi - получает данные
	mov esi, edi
	add edi, ebx
	add esi, eax
	cld
	@@:
		dec edx ;уменьшаем счетчик оставшихся строк на 1
		cmp edx,0
		jle @f
		mov ecx, ebx
		rep movsb ;перенос (копирование) строки пикселей
		add esi,eax ;переход на новую строчку изображения
		sub esi,ebx
		jmp @b
	@@:
	popad
	ret
endp

;обрезаем цветное изображение с левой стороны
;input:
;data_rgb - pointer to rgb data
;size_w_old - width img in pixels
;size_w_new - new width img in pixels
;size_h - height img in pixels
align 4
proc img_rgb_crop_l, data_rgb:dword, size_w_old:dword, size_w_new:dword, size_h:dword
	pushad
	mov edi,dword[data_rgb]
	mov esi,edi
	mov eax,dword[size_w_old]
	mov ebx,dword[size_w_new]
	cmp eax,ebx
	jle .end_f ;старый размер изображения не может быть меньше нового (при условии обрезания картинки)
		lea eax,[eax+eax*2]
		lea ebx,[ebx+ebx*2]
		sub eax,ebx
		mov edx,dword[size_h] ;высота изображения
		cld
		@@:
			add esi,eax
			mov ecx,ebx
			rep movsb
			dec edx
			cmp edx,0
			jg @b
	.end_f:
	popad
	ret
endp

;обрезаем серое изображение с левой стороны
;input:
;data_gray - pointer to gray data
;size_w_old - width img in pixels
;size_w_new - new width img in pixels
;size_h - height img in pixels
align 4
proc img_gray_crop_l, data_gray:dword, size_w_old:dword, size_w_new:dword, size_h:dword
	pushad
	mov edi,dword[data_gray]
	mov esi,edi
	mov eax,dword[size_w_old]
	mov ebx,dword[size_w_new]
	cmp eax,ebx
	jle .end_f ;старый размер изображения не может быть меньше нового (при условии обрезания картинки)
		sub eax,ebx
		mov edx,dword[size_h] ;высота изображения
		cld
		@@:
			add esi,eax
			mov ecx,ebx
			rep movsb
			dec edx
			cmp edx,0
			jg @b
	.end_f:
	popad
	ret
endp

;hoffs - колличество пикселей на котрые поднимается/опускается изображение
;img_t - высота, с которой начинается двигающаяся часть изображения
align 4
proc buf_offset_h, buf_struc:dword, hoffs:dword, img_t:dword, img_h:dword ;сдвигает изображение по высоте
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .end_move_24

	mov eax,[hoffs]
	cmp eax,0
	je .end_move_24
		mov ebx,buf2d_w
		mov edx,dword[img_t]
			mov ecx,dword[img_h] ;ecx - высота сдвигаемых данных
			cmp ecx,buf2d_h
			jge .end_f ;ошибочное условие, высота изображения меньше чем высота сдвигаемого изображения
			imul ecx,ebx ;ecx - колличество пикселей в сдвигаемых данных
			lea ecx,[ecx+ecx*2]
		imul ebx,edx
		lea ebx,[ebx+ebx*2]
		mov esi,buf2d_data
		add esi,ebx

		add edx,eax ;edx = img_t+hoffs (hoffs<0)
		mov ebx,buf2d_w
		imul ebx,edx
		lea ebx,[ebx+ebx*2]
		mov edi,buf2d_data ;позиция, куда будет двигаться изображение
		add edi,ebx

		cmp eax,0
		jg .move_down_24
			;двигаем изображение вверх
			cld
			rep movsb
			jmp .end_f
		.move_down_24:
			;двигаем изображение вниз
			add esi,ecx
			dec esi
			add edi,ecx
			dec edi
			std
			rep movsb
			jmp .end_f
	.end_move_24:

;stdcall print_err,sz_buf2d_offset_h,txt_err_n24b

	cmp buf2d_bits,8
	jne .end_move_8

	mov eax,[hoffs]
	cmp eax,0
	je .end_move_8
		;двигаем изображение вверх
		mov ebx,buf2d_w
		mov edx,dword[img_t]
			mov ecx,dword[img_h] ;ecx - высота сдвигаемых данных
			cmp ecx,buf2d_h
			jge .end_f ;ошибочное условие, высота изображения меньше чем высота сдвигаемого изображения
			imul ecx,ebx ;ecx - колличество пикселей в сдвигаемых данных
		imul ebx,edx
		mov esi,buf2d_data
		add esi,ebx

		add edx,eax ;edx = img_t+hoffs (hoffs<0)
		mov ebx,buf2d_w
		imul ebx,edx
		mov edi,buf2d_data ;позиция, куда будет двигаться изображение
		add edi,ebx

		cmp eax,0
		jg .move_down_8
			cld
			rep movsb
			jmp .end_f
		.move_down_8:
			;двигаем изображение вниз
			add esi,ecx
			dec esi
			add edi,ecx
			dec edi
			std
			rep movsb
			jmp .end_f
	.end_move_8:

	.end_f:
	popad
	ret
endp

align 4
proc buf_delete, buf_struc:dword
	push eax edi
	mov edi,dword[buf_struc]
	invoke mem.free,buf2d_data
	pop edi eax
	ret
endp

;input:
; new_w - новая ширина (если 0 то не меняется)
; new_h - новая высота (если 0 то не меняется)
; options - параметры изменения буфера (1 - изменять размер буфера,
;    2 - изменять изображение в буфере, 3 - изменять буфер и изображение)
align 4
proc buf_resize, buf_struc:dword, new_w:dword, new_h:dword, options:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,8
	jne .8bit
		bt dword[options],1 ;сжатие изобр.
		jnc @f
			;...
		@@:
		bt dword[options],0 ;измен. буфер
		jnc .end_f
			;...
		jmp .end_f
	.8bit:
	cmp buf2d_bits,24
	jne .24bit
		bt dword[options],1 ;сжатие изобр.
		jnc .24_end_r
			mov eax,dword[new_w]
			cmp eax,1
			jl @f
			cmp eax,buf2d_w
			jge @f
				;сжатие по ширине
				stdcall img_rgb24_wresize, buf2d_data,buf2d_w,buf2d_h,eax
				jmp .24_r_h
			@@:
			mov eax,buf2d_w
			.24_r_h: ;eax - ширина буфера или ширина сжатого изображения
			mov ebx,dword[new_h]
			cmp ebx,1
			jl @f
			cmp ebx,buf2d_h
			jge @f
				;сжатие по высоте
				stdcall img_rgb24_hresize, buf2d_data,eax,buf2d_h,ebx
			@@:
		.24_end_r:
		bt dword[options],0 ;измен. буфер
		jnc .end_f
		mov eax,dword[new_w]
		cmp eax,1
		jl @f
			mov buf2d_w,eax
		@@:
		mov ecx,buf2d_w
		mov eax,dword[new_h]
		cmp eax,1
		jl @f
			mov buf2d_h,eax
		@@:
		mov ebx,buf2d_h
		imul ecx,ebx
		lea ecx,[ecx+ecx*2] ; 24 bit = 3
		invoke mem.realloc,buf2d_data,ecx ;изменяем память занимаемую буфером
		mov buf2d_data,eax ;на случай если изменился указатель на данные
	.24bit:
	.end_f:
	popad
	ret
endp

align 4
rot_table: ;таблица для указания на подфункции для поворотов
	dd buf_rotate.8b90,buf_rotate.24b90,buf_rotate.32b90,\
	buf_rotate.8b180,buf_rotate.24b180,buf_rotate.32b180

;поворот изображения на 90 или 180 градусов
align 4
proc buf_rotate, buf_struc:dword, angle:dword
locals
	n_data dd ?
	dec_h dd ? ;число байт, для уменьшения координаты y
endl
	pushad
	mov edi,[buf_struc]
	mov ebx,buf2d_w
	mov ecx,buf2d_h

	lea eax,[rot_table]
	cmp dword[angle],90 ;проверка угла поворота
	je .beg_0
	cmp dword[angle],180
	jne @f
		add eax,12
		jmp .beg_0
	@@:
	jmp .end_f
	.beg_0: ;проверка битности буфера
	cmp buf2d_bits,8
	jne @f
		jmp dword[eax]
	@@:
	cmp buf2d_bits,24
	jne @f
		add eax,4
		jmp dword[eax]
	@@:
	cmp buf2d_bits,32
	jne @f
		add eax,8
		jmp dword[eax]
	@@:
	jmp .end_f

	.8b90: ;поворот 8 битного буфера на 90 градусов
		mov edx,ecx ;edx - buf_h
		imul ecx,ebx
		invoke mem.alloc,ecx ;выделяем временную память
		cmp eax,0
		je .end_f
		mov [n_data],eax
		mov [dec_h],ecx
		inc dword[dec_h]

		;copy buf --> mem
		mov edi,[buf_struc]
		mov esi,buf2d_data
		mov edi,eax ;[n_data]
		dec edx ;коректируем edx на 1 байт, для компенсации сдвига в movsb
		add edi,edx
		xor eax,eax
		cld
		.cycle_0:
			movsb
			add edi,edx
			inc eax
			cmp eax,ebx
			jl @f
				xor eax,eax
				sub edi,[dec_h]
			@@:
			loop .cycle_0

		;change buf_w <---> buf_h
		mov esi,[n_data]
		mov edi,[buf_struc]
		mov edi,buf2d_data
		mov ecx,ebx
		inc edx ;исправляем скоректированный edx
		imul ecx,edx
		;copy buf <-- mem
		;cld
		rep movsb
		invoke mem.free,[n_data]
		jmp .change_w_h
	.24b90: ;поворот 24 битного буфера на 90 градусов
		mov esi,ecx
		imul esi,ebx
		lea ecx,[ecx+ecx*2]
		mov edx,ecx ;edx - buf_h * 3
		imul ecx,ebx
		invoke mem.alloc,ecx ;выделяем временную память
		cmp eax,0
		je .end_f
		mov [n_data],eax
		mov [dec_h],ecx
		add dword[dec_h],3

		;copy buf --> mem
		
		mov edi,[buf_struc]
		mov ecx,esi
		mov esi,buf2d_data
		mov edi,eax ;[n_data]
		sub edx,3 ;коректируем edx на 3 байта, для компенсации сдвига
		add edi,edx
		xor eax,eax
		cld
		.cycle_1:
			movsw
			movsb
			add edi,edx
			inc eax
			cmp eax,ebx
			jl @f
				xor eax,eax
				sub edi,[dec_h]
			@@:
			loop .cycle_1

		;copy buf <-- mem
		mov esi,[n_data]
		mov edi,[buf_struc]
		mov edi,buf2d_data
		mov ecx,ebx
		add edx,3 ;исправляем скоректированный edx
		imul ecx,edx
		;cld
		rep movsb
		invoke mem.free,[n_data]
		jmp .change_w_h
	.32b90: ;поворот 32 битного буфера на 90 градусов
		shl ecx,2
		mov edx,ecx ;edx - buf_h * 4
		imul ecx,ebx
		invoke mem.alloc,ecx ;выделяем временную память
		cmp eax,0
		je .end_f
		mov [n_data],eax
		mov [dec_h],ecx
		add dword[dec_h],4

		;copy buf --> mem
		mov edi,[buf_struc]
		shr ecx,2
		mov esi,buf2d_data
		mov edi,eax ;[n_data]
		sub edx,4 ;коректируем edx на 4 байта, для компенсации сдвига в movsd
		add edi,edx
		xor eax,eax
		cld
		.cycle_2:
			movsd
			add edi,edx
			inc eax
			cmp eax,ebx
			jl @f
				xor eax,eax
				sub edi,[dec_h]
			@@:
			loop .cycle_2

		;copy buf <-- mem
		mov esi,[n_data]
		mov edi,[buf_struc]
		mov edi,buf2d_data
		mov ecx,ebx
		add edx,4 ;исправляем скоректированный edx
		imul ecx,edx
		shr ecx,2
		;cld
		rep movsd
		invoke mem.free,[n_data]
		;jmp .change_w_h
	.change_w_h: ;change buf_w <---> buf_h
		mov edi,[buf_struc]
		mov eax,buf2d_w
		mov ebx,buf2d_h
		mov buf2d_h,eax
		mov buf2d_w,ebx
		jmp .end_f
	.8b180: ;поворот 8 битного буфера на 180 градусов
		mov edi,buf2d_data
		mov esi,edi
		imul ecx,ebx
		add esi,ecx
		dec esi
		shr ecx,1 ;ecx - число пикселей буфера : 2
		std
		@@:
			lodsb
			mov ah,byte[edi]
			mov byte[esi+1],ah
			mov byte[edi],al
			inc edi
			loop @b
			jmp .end_f
	.24b180: ;поворот 24 битного буфера на 180 градусов
		mov esi,buf2d_data
		mov edi,esi
		imul ecx,ebx
		mov eax,ecx
		lea ecx,[ecx+ecx*2]
		add edi,ecx
		sub edi,3
		shr eax,1
		mov ecx,eax ;ecx - число пикселей буфера : 2
		cld
		@@:
			lodsw
			mov edx,eax
			lodsb
			mov bx,word[edi]
			mov word[esi-3],bx
			mov bl,byte[edi+2]
			mov byte[esi-1],bl
			mov byte[edi+2],al
			mov word[edi],dx
			sub edi,3
			loop @b
			jmp .end_f
	.32b180: ;поворот 32 битного буфера на 180 градусов
		mov edi,buf2d_data
		mov esi,edi
		imul ecx,ebx
		shl ecx,2
		add esi,ecx
		sub esi,4
		shr ecx,3 ;ecx - число пикселей буфера : 2
		std
		@@:
			lodsd
			mov ebx,dword[edi]
			mov dword[esi+4],ebx
			mov dword[edi],eax
			add edi,4
			loop @b
		;jmp .end_f

	.end_f:
	popad
	ret
endp

align 4
proc buf_flip_h, buf_struc:dword
pushad
	mov edi,[buf_struc]
	cmp buf2d_bits,24
	jne .end_24
		mov esi,buf2d_data
		mov eax,buf2d_w
		mov ecx,eax
		shr ecx,1
		dec eax
		lea eax,[eax+eax*2]
		mov ebx,buf2d_h
		mov edi,esi		
		add esi,eax
		add eax,3
		cld
		.cycle_24:
		push ecx edi esi
align 4
		@@:
			;swap word[edi] <-> word[esi]
			mov dx,[edi]
			movsw
			mov [esi-2],dx
			;swap byte[edi] <-> byte[esi]
			mov dl,[edi]
			movsb
			mov [esi-1],dl
			sub esi,6
		loop @b
		pop esi edi ecx
		add edi,eax
		add esi,eax
		dec ebx
		or ebx,ebx
		jnz .cycle_24
		jmp .end_32
	.end_24:
	cmp buf2d_bits,32
	jne .end_32
		mov esi,buf2d_data
		mov eax,buf2d_w
		dec eax
		shl eax,2
		mov ebx,buf2d_h
		mov edi,esi
		add esi,eax
		add eax,4
		cld
		.cycle_32:
		mov ecx,eax
		shr ecx,3
		push edi esi
align 4
		@@:
			;swap dword[edi] <-> dword[esi]
			mov edx,[edi]
			movsd
			mov [esi-4],edx
			sub esi,8
		loop @b
		pop esi edi
		add edi,eax
		add esi,eax
		dec ebx
		or ebx,ebx
		jnz .cycle_32
	.end_32:
popad
	ret
endp

;отразить по вертикали (верх и низ меняются местами)
align 4
proc buf_flip_v, buf_struc:dword
locals
	line_pix dd ? ;кол. пикселей в линии буфера
	line_2byte dd ? ;кол. байт в линии буфера * 2
endl
	pushad
	mov edi,[buf_struc]
	cmp buf2d_bits,24
	jne .end_24
		mov edx,buf2d_w
		mov [line_pix],edx
		mov ebx,buf2d_h
		lea edx,[edx+edx*2]
		mov esi,edx
		imul esi,ebx
		sub esi,edx
		add esi,buf2d_data ;указатель на нижнюю линию
		shr ebx,1 ;кол. линейных циклов
		shl edx,1
		mov [line_2byte],edx
		mov edi,buf2d_data
		xchg edi,esi
		cld
		.flip_24:
		cmp ebx,0
		jle .end_32 ;здесь выход из функции (потому .end_24 не подходит)
		mov ecx,[line_pix]
align 4
		@@:
			lodsw
			mov dx,word[edi]
			mov word[esi-2],dx
			stosw
			lodsb
			mov ah,byte[edi]
			mov byte[esi-1],ah
			stosb
			loop @b
		sub edi,[line_2byte]
		dec ebx
		jmp .flip_24
	.end_24:
	cmp buf2d_bits,32
	jne .end_32
		mov edx,buf2d_w
		mov [line_pix],edx
		mov ebx,buf2d_h
		shl edx,2
		mov esi,edx
		imul esi,ebx
		sub esi,edx
		add esi,buf2d_data ;указатель на нижнюю линию
		shr ebx,1 ;кол. линейных циклов
		shl edx,1
		mov [line_2byte],edx
		mov edi,buf2d_data
		xchg edi,esi
		cld
		.flip_32:
		cmp ebx,0
		jle .end_32
		mov ecx,[line_pix]
align 4
		@@:
			lodsd
			mov edx,dword[edi]
			mov dword[esi-4],edx
			stosd
			loop @b
		sub edi,[line_2byte]
		dec ebx
		jmp .flip_32
	.end_32:
	popad
	ret
endp

;description:
; сжатие изображения по ширине в 2 раза (размеры буфера не меняются)
align 4
proc buf_img_wdiv2, buf_struc:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,8
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		stdcall img_8b_wdiv2, buf2d_data,ecx
	@@:
	cmp buf2d_bits,24
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		stdcall img_rgb24_wdiv2, buf2d_data,ecx
	@@:
	cmp buf2d_bits,32
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		stdcall img_rgba32_wdiv2, buf2d_data,ecx
	@@:
	popad
	ret
endp

;input:
;data_8b - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
align 4
proc img_8b_wdiv2 data_8b:dword, size:dword
	mov eax,dword[data_8b]
	mov ecx,dword[size] ;ecx = size
	cld
	@@: ;затемнение цвета пикселей
		shr byte[eax],1
		inc eax
		loop @b

	mov eax,dword[data_8b]
	mov ecx,dword[size] ;ecx = size
	shr ecx,1
	@@: ;сложение цветов пикселей
		mov bl,byte[eax+1] ;копируем цвет соседнего пикселя
		add byte[eax],bl
		add eax,2
		loop @b

	mov eax,dword[data_8b]
	inc eax
	mov ebx,eax
	inc ebx
	mov ecx,dword[size] ;ecx = size
	shr ecx,1
	dec ecx ;лишний пиксель
	@@: ;поджатие пикселей
		mov dl,byte[ebx]
		mov byte[eax],dl

		inc eax
		add ebx,2
		loop @b
	ret
endp

;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
align 4
proc img_rgb24_wdiv2 data_rgb:dword, size:dword
  mov eax,dword[data_rgb]
  mov ecx,dword[size] ;ecx = size
  lea ecx,[ecx+ecx*2]
  cld
  @@: ;затемнение цвета пикселей
		shr byte[eax],1
		inc eax
		loop @b

  mov eax,dword[data_rgb]
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  @@: ;сложение цветов пикселей
		mov bx,word[eax+3] ;копируем цвет соседнего пикселя
		add word[eax],bx
		mov bl,byte[eax+5] ;копируем цвет соседнего пикселя
		add byte[eax+2],bl
		add eax,6 ;=2*3
		loop @b

  mov eax,dword[data_rgb]
  add eax,3
  mov ebx,eax
  add ebx,3
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  dec ecx ;лишний пиксель
  @@: ;поджатие пикселей
		mov edx,dword[ebx]
		mov word[eax],dx
		shr edx,16
		mov byte[eax+2],dl

		add eax,3
		add ebx,6
		loop @b
  ret
endp

;input:
;data_rgba - pointer to rgba data
;size - count img pixels (size img data / 4(rgba) )
align 4
proc img_rgba32_wdiv2 data_rgba:dword, size:dword
	mov eax,dword[data_rgba]

	mov eax,dword[data_rgba]
	mov ebx,eax
	add ebx,4
	mov ecx,dword[size] ;ecx = size
	shr ecx,1
	@@: ;смешивание цветов пикселей
		call combine_colors_1
		mov [eax],edx
		add eax,8 ;=2*4
		add ebx,8
		loop @b

	mov eax,dword[data_rgba]
	add eax,4
	mov ebx,eax
	add ebx,4
	mov ecx,dword[size] ;ecx = size
	shr ecx,1
	dec ecx ;лишний пиксель
	@@: ;поджатие пикселей
		mov edx,dword[ebx]
		mov dword[eax],edx

		add eax,4
		add ebx,8
		loop @b
	ret
endp

;description:
; сжатие изображения по высоте в 2 раза (высота буфера не меняется)
align 4
proc buf_img_hdiv2, buf_struc:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,8
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		stdcall img_8b_hdiv2, buf2d_data,ecx,eax
		jmp .end_f ;edi портится в функции, потому использование buf2d_bits опасно
	@@:
	cmp buf2d_bits,24
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		stdcall img_rgb24_hdiv2, buf2d_data,ecx,eax
		jmp .end_f
	@@:
	cmp buf2d_bits,32
	jne @f
		mov eax,buf2d_w
		mov ecx,buf2d_h
		imul ecx,eax
		shl eax,2
		stdcall img_rgba32_hdiv2, buf2d_data,ecx,eax
		;jmp .end_f
	@@:
	.end_f:
	popad
	ret
endp

;input:
;data_8b - pointer to 8 bit data
;size - count img pixels (size img data)
;size_w - width img in pixels
align 4
proc img_8b_hdiv2, data_8b:dword, size:dword, size_w:dword

	mov eax,dword[data_8b] ;eax =
	mov ecx,dword[size]
	cld
	@@: ;затемнение цвета пикселей
		shr byte[eax],1
		inc eax
		loop @b

	mov eax,dword[data_8b] ;eax =
	mov esi,dword[size_w]
	mov ebx,esi
	add ebx,eax
	mov ecx,dword[size]  ;ecx = size
	shr ecx,1
	xor edi,edi
	@@: ;сложение цветов пикселей
		mov dl,byte[ebx] ;копируем цвет нижнего пикселя
		add byte[eax],dl

		inc eax
		inc ebx
		inc edi
		cmp edi,dword[size_w]
		jl .old_line
			add eax,esi
			add ebx,esi
			xor edi,edi
		.old_line:
		loop @b


	mov eax,dword[data_8b] ;eax =
	add eax,esi ;esi = width*3(rgb)
	mov ebx,eax
	add ebx,esi
	mov ecx,dword[size] ;ecx = size
	shr ecx,1
	sub ecx,dword[size_w] ;лишняя строка пикселей
	xor edi,edi
	@@: ;поджатие пикселей
		mov dl,byte[ebx] ;копируем цвет нижнего пикселя
		mov byte[eax],dl

		inc eax
		inc ebx
		inc edi
		cmp edi,dword[size_w]
		jl .old_line_2
			add ebx,esi
			xor edi,edi
		.old_line_2:
		loop @b
	ret
endp

;input:
;data_rgb - pointer to rgb data
;size - count img pixels (size img data / 3(rgb) )
;size_w - width img in pixels
align 4
proc img_rgb24_hdiv2, data_rgb:dword, size:dword, size_w:dword

  mov eax,dword[data_rgb] ;eax =
  mov ecx,dword[size]	  ;ecx = size
  lea ecx,[ecx+ecx*2]
  cld
  @@: ;затемнение цвета пикселей
    shr byte[eax],1
    inc eax
    loop @b

  mov eax,dword[data_rgb] ;eax =
  mov esi,dword[size_w]
  lea esi,[esi+esi*2] ;esi = width*3(rgb)
  mov ebx,esi
  add ebx,eax
  mov ecx,dword[size]  ;ecx = size
  shr ecx,1
  xor edi,edi
  @@: ;сложение цветов пикселей
    mov dx,word[ebx] ;копируем цвет нижнего пикселя
    add word[eax],dx
    mov dl,byte[ebx+2] ;копируем цвет нижнего пикселя
    add byte[eax+2],dl

    add eax,3
    add ebx,3
    inc edi
    cmp edi,dword[size_w]
    jl .old_line
      add eax,esi
      add ebx,esi
      xor edi,edi
    .old_line:
    loop @b

  mov eax,dword[data_rgb] ;eax =
  add eax,esi ;esi = width*3(rgb)
  mov ebx,eax
  add ebx,esi
  mov ecx,dword[size] ;ecx = size
  shr ecx,1
  sub ecx,dword[size_w] ;лишняя строка пикселей
  xor edi,edi
  @@: ;поджатие пикселей
    mov edx,dword[ebx] ;копируем цвет нижнего пикселя
    mov word[eax],dx
    shr edx,16
    mov byte[eax+2],dl

    add eax,3
    add ebx,3
    inc edi
    cmp edi,dword[size_w]
    jl .old_line_2
      add ebx,esi
      xor edi,edi
    .old_line_2:
    loop @b
  ret
endp

;input:
;data_rgba - pointer to rgba data
;size - count img pixels (size img data / 4(rgba) )
;size_w_b - width img in bytes
align 4
proc img_rgba32_hdiv2, data_rgba:dword, size:dword, size_w_b:dword

	mov eax,dword[data_rgba] ;eax =
	mov ebx,dword[size_w_b]
	add ebx,eax
	mov ecx,dword[size]  ;ecx = size
	shr ecx,1
	xor edi,edi
	@@: ;смешивание цветов пикселей
		call combine_colors_1
		mov dword[eax],edx

		add eax,4
		add ebx,4
		add edi,4
		cmp edi,dword[size_w_b]
		jl .old_line
			add eax,dword[size_w_b]
			add ebx,dword[size_w_b]
			xor edi,edi
		.old_line:
		loop @b

	mov eax,dword[data_rgba] ;eax =
	mov ebx,dword[size_w_b]
	add eax,ebx
	add ebx,eax
	mov ecx,dword[size] ;ecx = size
	shl ecx,1
	sub ecx,dword[size_w_b] ;лишняя строка пикселей
	shr ecx,2
	xor edi,edi
	@@: ;поджатие пикселей
		mov edx,dword[ebx] ;копируем цвет нижнего пикселя
		mov dword[eax],edx

		add eax,4
		add ebx,4
		add edi,4
		cmp edi,dword[size_w_b]
		jl .old_line_2
			add ebx,dword[size_w_b]
			xor edi,edi
		.old_line_2:
		loop @b
	ret
endp

;input:
; eax - указатель на 32-битный цвет
; ebx - указатель на 32-битный цвет
;output:
; edx - 32-битный цвет смешанный с учетом прозрачности
;destroy:
; esi
align 4
proc combine_colors_1 uses ecx edi
locals
	c_blye dd ?
	c_green dd ?
	c_red dd ?
endl		
	movzx edi,byte[eax+3]
	cmp edi,255
	je .c0z
	movzx esi,byte[ebx+3]
	cmp esi,255
	je .c1z
	cmp edi,esi
	je .c0_c1

	;переворачиваем значения прозрачностей
	neg edi
	add edi,256
	neg esi
	add esi,256

	movzx ecx,byte[eax]
	imul ecx,edi
	mov [c_blye],ecx
	movzx ecx,byte[ebx]
	imul ecx,esi
	add [c_blye],ecx

	movzx ecx,byte[eax+1]
	imul ecx,edi
	mov [c_green],ecx
	movzx ecx,byte[ebx+1]
	imul ecx,esi
	add [c_green],ecx

	movzx ecx,byte[eax+2]
	imul ecx,edi
	mov [c_red],ecx
	movzx ecx,byte[ebx+2]
	imul ecx,esi
	add [c_red],ecx

push eax ebx
	xor ebx,ebx
	mov eax,[c_red]
	xor edx,edx
	mov ecx,edi
	add ecx,esi
	div ecx
	mov bl,al
	shl ebx,16
	mov eax,[c_green]
	xor edx,edx
	div ecx
	mov bh,al
	mov eax,[c_blye]
	xor edx,edx
	div ecx
	mov bl,al

	shr ecx,1
	;переворачиваем значения прозрачности
	neg ecx
	add ecx,256

	shl ecx,24
	add ebx,ecx
	mov edx,ebx
pop ebx eax

	jmp .end_f
	.c0_c1: ;если прозрачности обоих цветов совпадают
		mov edx,[eax]
		shr edx,1
		and edx,011111110111111101111111b
		mov esi,[ebx]
		shr esi,1
		and esi,011111110111111101111111b
		add edx,esi
		ror edi,8 ;перемещаем значение прозрачности в старший байт edi
		or edx,edi
		jmp .end_f
	.c0z: ;если цвет в eax прозрачный
		mov edx,[ebx]
		movzx edi,byte[ebx+3]
		jmp @f
	.c1z: ;если цвет в ebx прозрачный
		mov edx,[eax]
	@@:
		add edi,255 ;делаем цвет на половину прозрачным
		shr edi,1
		cmp edi,255
		jle @f
			mov edi,255 ;максимальная прозрачность не более 255
		@@:
		shl edi,24
		and edx,0xffffff ;снимаем старую прозрачность
		add edx,edi
	.end_f:
	ret
endp

;description:
; сжатие изображения по ширине (размеры буфера не меняются)
;input:
; data_rgb - pointer to rgb data
; size_w - width img in pixels
; size_h - height img in pixels
; size_w_new - new width img in pixels
align 16
proc img_rgb24_wresize, data_rgb:dword, size_w:dword, size_h:dword, size_w_new:dword
locals
	pr dd 0
	pg dd 0
	pb dd 0
	img_n dd ? ;указатель на данные нового изображения
	lines dd ?
endl
pushad
;eax - delta for inp. img
;ebx - delta for outp. img
;esi - pointer to data_rgb
	mov esi,[data_rgb]
	mov [img_n],esi
	mov eax,[size_h]
	mov [lines],eax
align 4
	.cycyle_0:
	mov eax,[size_w_new]
	mov ecx,[size_w]
	mov ebx,ecx
align 4
	.cycyle_1:
		cmp eax,ebx
		jg .else_0
			;копируемый пиксель максимально влияет на результат
			;накапливаем rgb для интерполяции пикселей
			mov edx,[size_w_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			cmp eax,ebx
			je .d2_add
			jmp .if_0_end
		.else_0:
			;копируемый пиксель попадет на границу пикселей
			mov edx,ebx
			sub edx,eax
			add edx,[size_w_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			;сохраняем готовое rgb
			.d2_add:
			push eax
				mov edi,[img_n]
				mov eax,[pb]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
				mov eax,[pg]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
				mov eax,[pr]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
			pop eax
			add dword[img_n],3 ;next pixel
			;обновляем rgb для нового пикселя
			mov edx,eax
			sub edx,ebx
			movzx edi,byte[esi]
			imul edi,edx
			mov [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			mov [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			mov [pr],edi
			add ebx,[size_w]
		.if_0_end:
		add eax,[size_w_new]
		add esi,3 ;next pixel
		dec ecx
		jnz .cycyle_1
	dec dword[lines]
	jnz .cycyle_0
popad
	ret
endp

;description:
; сжатие изображения по высоте (размеры буфера не меняются)
;input:
; data_rgb - pointer to rgb data
; size_w - width img in pixels
; size_h - height img in pixels
; size_h_new - new height img in pixels
align 16
proc img_rgb24_hresize, data_rgb:dword, size_w:dword, size_h:dword, size_h_new:dword
locals
	pr dd 0
	pg dd 0
	pb dd 0
	img_n dd ? ;указатель на данные нового изображения
	cols dd ?
	lin_b dd ? ;размер линии изображения в байтах
	data_n dd ? ;указатель на данные для нового столбца пикселей
endl
pushad
;eax - delta for inp. img
;ebx - delta for outp. img
;esi - pointer to data_rgb
	mov esi,[data_rgb]
	mov [data_n],esi
	mov eax,[size_w]
	mov [cols],eax
	lea eax,[eax+eax*2]
	mov [lin_b],eax
align 4
	.cycyle_0:
	mov eax,[size_h_new]
	mov ecx,[size_h]
	mov ebx,ecx
	mov esi,[data_n]
	mov [img_n],esi
	add dword[data_n],3 ;переход на следующий столбец пикселей
align 4
	.cycyle_1:
		cmp eax,ebx
		jg .else_0
			;копируемый пиксель максимально влияет на результат
			;накапливаем rgb для интерполяции пикселей
			mov edx,[size_h_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			cmp eax,ebx
			je .d2_add
			jmp .if_0_end
		.else_0:
			;копируемый пиксель попадет на границу пикселей
			mov edx,ebx
			sub edx,eax
			add edx,[size_h_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			;сохраняем готовое rgb
			.d2_add:
			push eax
				mov edi,[img_n]
				mov eax,[pb]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
				mov eax,[pg]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
				mov eax,[pr]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
			pop eax
			mov edx,[lin_b]
			add dword[img_n],edx ;next pixel
			;обновляем rgb для нового пикселя
			mov edx,eax
			sub edx,ebx
			movzx edi,byte[esi]
			imul edi,edx
			mov [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			mov [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			mov [pr],edi
			add ebx,[size_h]
		.if_0_end:
		add eax,[size_h_new]
		add esi,[lin_b] ;next pixel
		dec ecx
		jnz .cycyle_1
	dec dword[cols]
	jnz .cycyle_0
popad
	ret
endp

;преобразование буфера из 24-битного в 8-битный
; spectr - определяет какой спектр брать при преобразовании 0-синий, 1-зеленый, 2-красный
align 4
proc buf_conv_24_to_8, buf_struc:dword, spectr:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .error0
		mov eax,buf2d_w
		cmp eax,1
		jl .error1
		mov ecx,buf2d_h
		cmp ecx,1
		jl .error1
		imul ecx,eax
		mov esi,ecx
		;ebx - память из которой копируется
		;edx - память куда копируется
		mov edx,buf2d_data
		mov ebx,edx
		cmp [spectr],3
		jge @f
			add ebx,[spectr]
		@@:
			mov al,byte[ebx]
			mov byte[edx],al
			add ebx,3
			inc edx
			loop @b
		mov buf2d_bits,8
		invoke mem.realloc,buf2d_data,esi ;уменьшаем память занимаемую буфером
		jmp .end_conv
	.error0:
		stdcall print_err,sz_buf2d_conv_24_to_8,txt_err_n24b
		jmp .end_conv
	.error1:
		stdcall print_err,sz_buf2d_conv_24_to_8,txt_err_size_0
	.end_conv:
	popad
	ret
endp

;преобразование буфера из 24-битного в 32-битный
align 4
proc buf_conv_24_to_32, buf_struc:dword, buf_str8:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .error1
		mov ecx,buf2d_w
		mov ebx,buf2d_h
		imul ebx,ecx
		mov ecx,ebx ;ecx = size  8 b
		shl ebx,2   ;ebx = size 32 b
		invoke mem.realloc,buf2d_data,ebx ;увеличиваем память занимаемую буфером
		mov buf2d_data,eax ;на случай если изменился указатель на данные
		mov buf2d_bits,32
		mov edx,ebx ;edx = size 32 b
		sub ebx,ecx ;ebx = size 24 b
		mov eax,ecx
		;eax - размер  8 битных данных
		;ebx - размер 24 битных данных
		;edx - размер 32 битных данных
		add ebx,buf2d_data
		add edx,buf2d_data
		mov edi,dword[buf_str8]
		cmp buf2d_bits,8
		jne .error2
		add eax,buf2d_data
		mov edi,edx
		;eax - указатель на конец  8 битных данных
		;ebx - указатель на конец 24 битных данных
		;edi - указатель на конец 32 битных данных
		@@:
			sub edi,4 ;отнимаем в начале цикла,
			sub ebx,3 ; потому, что указатели стоят
			dec eax   ; за пределами буферов
			mov edx,dword[ebx]
			mov dword[edi],edx
			mov dl,byte[eax]
			mov byte[edi+3],dl
			loop @b

		jmp .end_conv
	.error1:
		stdcall print_err,sz_buf2d_conv_24_to_32,txt_err_n24b
		jmp .end_conv
	.error2:
		stdcall print_err,sz_buf2d_conv_24_to_32,txt_err_n8b
	.end_conv:
	popad
	ret
endp

;функция копирует изображение из буфера buf_source (24b|32b) в buf_destination (24b)
; указываются координаты вставки буфера buf_source относительно buf_destination
; прозрачность при копировании не учитывается
align 4
proc buf_bit_blt, buf_destination:dword, coord_x:dword, coord_y:dword, buf_source:dword
	locals
		lost_bytes dd ? ;число потерянных байтов в строке копируемого изображеня (тех что не влазят в буфер)
	endl
	pushad

	mov edi,[buf_source]
	cmp buf2d_bits,24
	je .sou24
	cmp buf2d_bits,32
	je .sou32
		jmp .copy_end ;формат буфера не поодерживается

	.sou24: ;в источнике 24 битная картинка
	mov eax,buf2d_w
	mov edx,buf2d_h ;высота копируемой картинки
	mov esi,buf2d_data ;данные копируемой картинки

	mov edi,[buf_destination]
	cmp buf2d_bits,24
	jne .copy_end ;формат буфера не поодерживается
	mov ebx,[coord_x] ;в ebx временно ставим отступ изображения (для проверки)
	cmp ebx,buf2d_w   ;проверяем влазит ли изображение по ширине
	jge .copy_end	  ;если изображение полностью вылазит за правую сторону
		mov ebx,buf2d_h ;ebx - высота основного буфера
		mov ecx,[coord_y]
		cmp ecx,0
		jge @f
			;если координата coord_y<0 (1-я настройка)
			add edx,ecx ;уменьшаем высоту копируемой картинки
			cmp edx,0
			jle .copy_end ;если копируемое изображение находится полностью над верхней границей буфера (coord_y<0 и |coord_y|>buf_source.h)
			neg ecx
			imul ecx,eax
			lea ecx,[ecx+ecx*2] ;по 3 байта на пиксель
			add esi,ecx ;сдвигаем указатель с копируемыми данными, с учетом пропушеной части
			xor ecx,ecx ;обнуляем координату coord_y
		@@:
		cmp ecx,ebx
		jge .copy_end ;если координата 'y' больше высоты буфера
		add ecx,edx ;ecx - нижняя координата копируемой картинки
		cmp ecx,ebx
		jle @f
			sub ecx,ebx
			sub edx,ecx ;уменьшаем высоту копируемой картинки, в случе когда она вылазит за нижнюю границу
		@@:
		mov ebx,buf2d_w
		mov ecx,[coord_y] ;ecx используем для временных целей
		cmp ecx,0
		jg .end_otr_c_y_24
			;если координата coord_y<=0 (2-я настройка)
			mov ecx,[coord_x]
			jmp @f
		.end_otr_c_y_24:
		imul ecx,ebx
		add ecx,[coord_x]
		@@:
		lea ecx,[ecx+ecx*2]
		add ecx,buf2d_data
		sub ebx,eax
		mov edi,ecx ;edi указатель на данные буфера, куда будет производится копирование

	mov dword[lost_bytes],0
	mov ecx,[coord_x]
	cmp ecx,0
	jge @f
		neg ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		lea ecx,[ecx+ecx*2]
		mov [lost_bytes],ecx
		add esi,ecx
		add edi,ecx ;edi указатель на данные буфера, куда будет производится копирование
		xor ecx,ecx
	@@:
	cmp ecx,ebx
	jle @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		lea ecx,[ecx+ecx*2] ;ecx - число пикселей в 1-й строке картинки, которые вылазят за правую сторону
		add [lost_bytes],ecx
	@@:

	lea eax,[eax+eax*2] ;колличество байт в 1-й строке копируемой картинки
	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки

	cld
	cmp [lost_bytes],0
	jg .copy_1
	.copy_0: ;простое копирование
		mov ecx,eax
		rep movsb
		add edi,ebx
		dec edx
		cmp edx,0
		jg .copy_0
	jmp .copy_end
	.copy_1: ;не простое копирование (картинка вылазит за правую сторону)
		mov ecx,eax
		rep movsb
		add edi,ebx
		add esi,[lost_bytes] ;добавляем байты, которые вылазят за правую границу
		dec edx
		cmp edx,0
		jg .copy_1
	jmp .copy_end

	.sou32: ;в источнике 32 битная картинка
	mov eax,buf2d_w
	mov edx,buf2d_h ;высота копируемой картинки
	mov esi,buf2d_data ;данные копируемой картинки

	mov edi,[buf_destination]
	cmp buf2d_bits,24
	jne .copy_end ;формат буфера не поодерживается
	mov ebx,[coord_x] ;в ebx временно ставим отступ изображения (для проверки)
	cmp ebx,buf2d_w   ;проверяем влазит ли изображение по ширине
	jge .copy_end	  ;если изображение полностью вылазит за правую сторону
		mov ebx,buf2d_h ;ebx - высота основного буфера
		mov ecx,[coord_y]
		cmp ecx,0
		jge @f
			;если координата coord_y<0 (1-я настройка)
			add edx,ecx ;уменьшаем высоту копируемой картинки
			cmp edx,0
			jle .copy_end ;если копируемое изображение находится полностью над верхней границей буфера (coord_y<0 и |coord_y|>buf_source.h)
			neg ecx
			imul ecx,eax
			shl ecx,2 ;по 4 байта на пиксель
			add esi,ecx ;сдвигаем указатель с копируемыми данными, с учетом пропушеной части
			xor ecx,ecx ;обнуляем координату coord_y
		@@:
		cmp ecx,ebx
		jge .copy_end ;если координата 'y' больше высоты буфера
		add ecx,edx ;ecx - нижняя координата копируемой картинки
		cmp ecx,ebx
		jle @f
			sub ecx,ebx
			sub edx,ecx ;уменьшаем высоту копируемой картинки, в случе когда она вылазит за нижнюю границу
		@@:
		mov ebx,buf2d_w
		;mov ecx,ebx ;ecx используем для временных целей
		;imul ecx,[coord_y]
		;add ecx,[coord_x]
		mov ecx,[coord_y] ;ecx используем для временных целей
		cmp ecx,0
		jg .end_otr_c_y_32
			;если координата coord_y<=0 (2-я настройка)
			mov ecx,[coord_x]
			jmp @f
		.end_otr_c_y_32:
		imul ecx,ebx
		add ecx,[coord_x]
		@@:
		lea ecx,[ecx+ecx*2]
		add ecx,buf2d_data
		sub ebx,eax
		mov edi,ecx ;edi указатель на данные буфера, куда будет производится копирование

	mov dword[lost_bytes],0
	mov ecx,[coord_x]
	cmp ecx,0
	jge @f
		neg ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		shl ecx,2
		mov [lost_bytes],ecx
		add esi,ecx
		add edi,ecx ;edi указатель на данные буфера, куда будет производится копирование
		xor ecx,ecx
	@@:
	cmp ecx,ebx
	jle @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		shl ecx,2 ;ecx - число пикселей в 1-й строке картинки, которые вылазят за правую сторону
		add [lost_bytes],ecx
	@@:

	;eax - колличество пикселей в 1-й строке копируемой картинки
	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки

	cld
	cmp [lost_bytes],0
	jg .copy_3
	.copy_2: ;простое копирование
		mov ecx,eax
		@@:
			movsw
			movsb
			inc esi
			loop @b
		add edi,ebx
		dec edx
		cmp edx,0
		jg .copy_2
	jmp .copy_end
	.copy_3: ;не простое копирование (картинка вылазит за правую сторону)
		mov ecx,eax
		@@:
			movsw
			movsb
			inc esi
			loop @b
		add edi,ebx
		add esi,[lost_bytes] ;добавляем байты, которые вылазят за правую границу
		dec edx
		cmp edx,0
		jg .copy_3

	.copy_end:
	popad
	ret
endp

;input:
; esi = pointer to color1 + transparent (32b)
; edi = pointer to background color2 (24b)
;output:
; [edi] = combine color (24b)
align 4
combine_colors_0:
	push ax cx
	movzx cx,byte[esi+3] ;pro
	cmp cx,255
	je .end_f
	or cx,cx
	jnz @f
		mov ax,[esi]
		mov [edi],ax
		mov al,[esi+2]
		mov [edi+2],al
		jmp .end_f
align 4
	@@:
	inc cx
	push bx dx
	mov bx,0x0100 ;---get transparent---
	sub bx,cx ;256-pro
	;---blye---
	movzx ax,byte[esi]
	imul ax,bx
	movzx dx,byte[edi]
	imul dx,cx
	add ax,dx
	mov byte[edi],ah
	;---green---
	movzx ax,byte[esi+1]
	imul ax,bx
	movzx dx,byte[edi+1]
	imul dx,cx
	add ax,dx
	mov byte[edi+1],ah
	;---red---
	movzx ax,byte[esi+2]
	imul ax,bx
	movzx dx,byte[edi+2]
	imul dx,cx
	add ax,dx
	mov byte[edi+2],ah
	pop dx bx
.end_f:
	pop cx ax
	ret

;функция копирует изображение из буфера buf_source (32b) в buf_destination (24b)
; указываются координаты вставки буфера buf_source относительно buf_destination
; при копировании учитывается прозрачность
align 4
proc buf_bit_blt_transp, buf_destination:dword, coord_x:dword, coord_y:dword, buf_source:dword
	locals
		lost_bytes dd ? ;число потерянных байтов в строке копируемого изображеня (тех что не влазят в буфер)
	endl
	pushad

	mov edi,[buf_source]
	cmp buf2d_bits,32
	jne .copy_end ;формат буфера не поодерживается
	mov eax,buf2d_w
	mov edx,buf2d_h ;высота копируемой картинки
	mov esi,buf2d_data ;данные копируемой картинки

	mov edi,[buf_destination]
	cmp buf2d_bits,24
	jne .copy_end ;формат буфера не поодерживается
		mov ebx,buf2d_h ;ebx - высота основного буфера
		mov ecx,[coord_y]
		cmp ecx,0
		jge @f
			;если координата coord_y<0 (1-я настройка)
			add edx,ecx ;уменьшаем высоту копируемой картинки
			cmp edx,0
			jle .copy_end ;если копируемое изображение находится полностью над верхней границей буфера (coord_y<0 и |coord_y|>buf_source.h)
			neg ecx
			imul ecx,eax
			shl ecx,2 ;по 4 байта на пиксель
			add esi,ecx ;сдвигаем указатель с копируемыми данными, с учетом пропушеной части
			xor ecx,ecx ;обнуляем координату coord_y
		@@:
		cmp ecx,ebx
		jge .copy_end ;если координата 'y' больше высоты буфера
		add ecx,edx ;ecx - нижняя координата копируемой картинки
		cmp ecx,ebx
		jle @f
			sub ecx,ebx
			sub edx,ecx ;уменьшаем высоту копируемой картинки, в случе когда она вылазит за нижнюю границу
		@@:
		mov ebx,buf2d_w
		mov ecx,ebx ;ecx используем для временных целей
		cmp [coord_y],0
		jg .end_otr_c_y
			;если координата coord_y<=0 (2-я настройка)
			mov ecx,[coord_x]
			jmp @f
		.end_otr_c_y:
		imul ecx,[coord_y]
		add ecx,[coord_x]
		@@:
		lea ecx,[ecx+ecx*2]
		add ecx,buf2d_data
		sub ebx,eax
		mov edi,ecx ;edi указатель на данные буфера, куда будет производится копирование

	mov dword[lost_bytes],0
	mov ecx,[coord_x]
	cmp ecx,0
	jge @f
		neg ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		shl ecx,2
		mov [lost_bytes],ecx
		add esi,ecx
		shr ecx,2
		lea ecx,[ecx+ecx*2]
		add edi,ecx ;edi указатель на данные буфера, куда будет производится копирование
		xor ecx,ecx
	@@:
	cmp ecx,ebx
	jle @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		shl ecx,2 ;ecx - число пикселей в 1-й строке картинки, которые вылазят за правую сторону
		add [lost_bytes],ecx
	@@:

	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки

	cld
	cmp [lost_bytes],0
	jg .copy_1
	.copy_0: ;простое копирование
		mov ecx,eax
		@@:
			call combine_colors_0
			add edi,3
			add esi,4
			loop @b
		add edi,ebx
		dec edx
		cmp edx,0
		jg .copy_0
	jmp .copy_end
	.copy_1: ;не простое копирование (картинка вылазит за правую сторону)
		mov ecx,eax
		@@:
			call combine_colors_0
			add edi,3
			add esi,4
			loop @b
		add edi,ebx
		add esi,[lost_bytes] ;добавляем байты, которые вылазят за правую границу
		dec edx
		cmp edx,0
		jg .copy_1

	.copy_end:
	popad
	ret
endp

;input:
; ebx - color1 (24b)
; esi = pointer to transparent (8b)
; edi = pointer to background color2 (24b)
;output:
; [edi] = combine color (24b)
align 4
combine_colors_2:
	push ebx cx
	movzx cx,byte[esi] ;pro
	cmp cx,255
	je .end_f
	or cx,cx
	jnz @f
		mov [edi],bx
		shr ebx,16
		mov [edi+2],bl
		jmp .end_f
align 4
	@@:
	inc cx
	push ax dx si
	mov si,0x0100 ;---get transparent---
	sub si,cx ;256-pro

		;---blye---
		movzx ax,bl
		shr ebx,8
		imul ax,si
		movzx dx,byte[edi]
		imul dx,cx
		add ax,dx
		mov byte[edi],ah
		;---green---
		movzx ax,bl
		shr ebx,8
		imul ax,si
		movzx dx,byte[edi+1]
		imul dx,cx
		add ax,dx
		mov byte[edi+1],ah
		;---red---
		movzx ax,bl
		imul ax,si
		movzx dx,byte[edi+2]
		imul dx,cx
		add ax,dx
		mov byte[edi+2],ah
	pop si dx ax
.end_f:
	pop cx ebx
	ret

;функция копирует изображение из буфера buf_source (8b) в buf_destination (24b)
; указываются координаты вставки буфера buf_source относительно buf_destination
align 4
proc buf_bit_blt_alpha, buf_destination:dword, coord_x:dword, coord_y:dword, buf_source:dword, color:dword
	locals
		lost_bytes dd ? ;число потерянных байтов в строке копируемого изображеня (тех что не влазят в буфер)
		dest_w_bytes dd ? ;колличество байт в буфере приемнике по ширине - ширина вставляемой картинки
	endl
	pushad

	mov edi,[buf_source]
	cmp buf2d_bits,8
	jne .error1 ;формат буфера не поодерживается
	mov eax,buf2d_w ;ширина копируемой картинки
	mov edx,buf2d_h ;высота копируемой картинки
	mov esi,buf2d_data ;данные копируемой картинки

	mov edi,[buf_destination]
	cmp buf2d_bits,24
	jne .error2 ;формат буфера не поодерживается
	mov ebx,[coord_x] ;в ebx временно ставим отступ изображения (для проверки)
	cmp ebx,buf2d_w   ;проверяем влазит ли изображение по ширине
	jge .copy_end	  ;если изображение полностью вылазит за правую сторону
		mov ebx,buf2d_h ;ebx - высота основного буфера
		mov ecx,[coord_y]
		cmp ecx,0
		jge @f
			;если координата coord_y<0 (1-я настройка)
			add edx,ecx ;уменьшаем высоту копируемой картинки
			cmp edx,0
			jle .copy_end ;если копируемое изображение находится полностью над верхней границей буфера (coord_y<0 и |coord_y|>buf_source.h)
			neg ecx
			imul ecx,eax
			add esi,ecx ;сдвигаем указатель с копируемыми данными, с учетом пропушеной части
			xor ecx,ecx ;обнуляем координату coord_y
		@@:
		cmp ecx,ebx
		jge .copy_end ;если координата 'y' больше высоты буфера
		add ecx,edx ;ecx - нижняя координата копируемой картинки
		cmp ecx,ebx
		jle @f
			sub ecx,ebx
			sub edx,ecx ;уменьшаем высоту копируемой картинки, в случе когда она вылазит за нижнюю границу
		@@:
		mov ebx,buf2d_w
		mov ecx,[coord_y] ;ecx используем для временных целей
		cmp ecx,0
		jg .end_otr_c_y
			;если координата coord_y<=0 (2-я настройка)
			mov ecx,[coord_x]
			jmp @f
		.end_otr_c_y:
		imul ecx,ebx
		add ecx,[coord_x]
		@@:
		lea ecx,[ecx+ecx*2]
		add ecx,buf2d_data ;buf2d_data данные основного буфера
		sub ebx,eax ;ebx - ширина основного буфера минус ширина рисуемого буфера
		mov edi,ecx ;edi указатель на данные буфера, куда будет производится копирование

	mov dword[lost_bytes],0
	mov ecx,[coord_x]
	cmp ecx,0
	jge @f
		neg ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		mov [lost_bytes],ecx
		add esi,ecx
		lea ecx,[ecx+ecx*2]
		add edi,ecx ;edi указатель на данные буфера, куда будет производится копирование
		xor ecx,ecx
	@@:
	cmp ecx,ebx
	jle @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		;ecx - число пикселей в 1-й строке картинки, которые вылазят за правую сторону
		add [lost_bytes],ecx
	@@:

	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки
	mov [dest_w_bytes],ebx
	mov ebx,[color]

	cld
	cmp dword[lost_bytes],0
	jg .copy_1
	.copy_0: ;простое копирование
		mov ecx,eax
		@@:
			call combine_colors_2
			add edi,3
			inc esi
			loop @b
		add edi,[dest_w_bytes]
		dec edx
		cmp edx,0
		jg .copy_0
	jmp .copy_end
	.copy_1: ;не простое копирование (картинка вылазит за левую и/или правую сторону)
		mov ecx,eax
		@@:
			call combine_colors_2
			add edi,3
			inc esi
			loop @b
		add edi,[dest_w_bytes]
		add esi,[lost_bytes] ;добавляем байты, которые вылазят за правую границу
		dec edx
		cmp edx,0
		jg .copy_1

	jmp .copy_end
	.error1:
		stdcall print_err,sz_buf2d_bit_blt_alpha,txt_err_n8b
		jmp .copy_end
	.error2:
		stdcall print_err,sz_buf2d_bit_blt_alpha,txt_err_n24b
	.copy_end:
	popad
	ret
endp

align 4
proc print_err, fun:dword, mes:dword ;выводим сообщение об шибке на доску отладки
	pushad
	mov eax,63
	mov ebx,1

	mov esi,[fun]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	mov cl,':'
	int 0x40
	mov cl,' '
	int 0x40
	mov esi,[mes]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	popad
	ret
endp

;фильтр
align 4
proc buf_filter_dither, buffer:dword, algor:dword
	pushad
	mov edi,[buffer]
	cmp buf2d_bits,24
	jne .error
		mov edx,buf2d_w
		mov esi,buf2d_h
		mov edi,buf2d_data
;edi - pointer to 24bit bitmap
;edx - x size
;esi - y size
		lea   edx,[edx+edx*2]
		imul  esi,edx

		;определяем какой алгоритм использовать
		cmp dword[algor],0
		jne @f
			call dither_0
			jmp .dither_end
		@@:
		cmp dword[algor],1
		jne @f
			call dither_1
			jmp .dither_end
		@@:
		cmp dword[algor],2
		jne @f
			call dither_2
			jmp .dither_end
		@@:
		cmp dword[algor],3
		jne @f
			call dither_3
			jmp .dither_end
		@@:
		call dither_4
		jmp .dither_end
	.error:
		stdcall print_err,sz_buf2d_filter_dither,txt_err_n24b
	.dither_end:
	popad
	ret
endp

align 16
dither_0: ; Sierra Filter Lite algorithm
newp_0:   ; Dithering cycle
	xor   ebx,ebx ; At first threshold
	movzx ecx,byte[edi]
	cmp   cl,255
	je    newp_0.next
	test  cl,cl
	jz    newp_0.next
	jns   @f
	dec   ebx
	sub   ecx,255
@@:
	mov   [edi],bl               ; putpixel

	sar   ecx,1                  ; error/2
	;adc   ecx,0                  ; round to integer

	movzx eax,byte[edi+3]        ; pixel (x+1;y)
	add   eax,ecx                ; add error/2 to (x+1;y)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok
@@:
	cmp   eax,255
	jle   .ok
	or    al,255
.ok:
	mov   [edi+3],al             ; putpixel

	sar   ecx,1                  ; error/4
	adc   ecx,0                  ; round to integer

	movzx eax,byte[edi+edx-3]    ; pixel (x-1;y+1)
	add   eax,ecx                ; add error/4 to (x-1;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok1
@@:
	cmp   eax,255
	jle   .ok1
	or    al,255
.ok1:
	mov   [edi+edx-3],al         ; putpixel

	movzx eax,byte[edi+edx]      ; pixel (x;y+1)
	add   eax,ecx                ; add error/4 to (x;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok2
@@:
	cmp   eax,255
	jle   .ok2
	or    al,255
.ok2:
	mov   [edi+edx],al           ; putpixel

.next:
	inc   edi
	dec   esi
	jnz   newp_0
	ret

align 16
dither_1: ; Floyd-Steinberg algorithm
newp_1:   ; Dithering cycle
	xor   ebx,ebx ; At first threshold
	movzx ecx,byte[edi]
	cmp   cl,255
	je    newp_1.next
	test  cl,cl
	jz    newp_1.next
	jns   @f
	dec   ebx
	sub   ecx,255
@@:
	mov   [edi],bl               ; putpixel

	sar   ecx,4                  ; error/16
	adc   ecx,0                  ; round to integer
	mov   ebx,ecx

	movzx eax,byte[edi+edx+3]    ; pixel (x+1;y+1)
	add   eax,ecx                ; add error/16 to (x+1;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok
@@:
	cmp   eax,255
	jle   .ok
	or    al,255
.ok:
	mov   [edi+edx+3],al         ;putpixel

	imul  ecx,3
	movzx eax,byte[edi+edx-3]    ; pixel (x-1;y+1)
	add   eax,ecx                ; add 3*error/16 to (x-1;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok1
@@:
	cmp   eax,255
	jle   .ok1
	or    al,255
.ok1:
	mov   [edi+edx-3],al         ;putpixel

	mov   ecx,ebx
	imul  ecx,5
	movzx eax,byte[edi+edx]      ; pixel (x;y+1)
	add   eax,ecx                ; add 5*error/16 to (x;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok2
@@:
	cmp   eax,255
	jle   .ok2
	or    al,255
.ok2:
	mov   [edi+edx],al           ;putpixel

	mov   ecx,ebx
	imul  ecx,7
	movzx eax,byte[edi+3]        ; pixel (x+1;y)
	add   eax,ecx                ; add 7*error/16 to (x+1;y)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok3
@@:
	cmp   eax,255
	jle   .ok3
	or    al,255
.ok3:
	mov   [edi+3],al             ;putpixel

.next:
	inc  edi
	dec  esi
	jnz  newp_1
	ret

align 16
dither_2: ; Burkes algorithm
newp_2:   ; Dithering cycle
	xor   ebx,ebx ; At first threshold
	movsx ecx,byte[edi]
	cmp   cl,255
	je    newp_2.next
	test  cl,cl
	jz    newp_2.next
	jns   @f
	dec   ebx
@@:
	mov   [edi],bl               ; putpixel

	sar   ecx,2                  ; error/4
	adc   ecx,0                  ; round to integer

	movzx eax,byte[edi+3]        ; pixel (x+1;y)
	add   eax,ecx                ; add error/4 to (x+1;y)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok
@@:
	cmp   eax,255
	jle   .ok
	or    al,255
.ok:
	mov   [edi+3],al             ; putpixel

	movzx eax,byte[edi+edx]      ; pixel (x;y+1)
	add   eax,ecx                ; add error/4 to (x;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok1
@@:
	cmp   eax,255
	jle   .ok1
	or    al,255
.ok1:
	mov   [edi+edx],al           ; putpixel

	sar   ecx,1                  ; error/8
	adc   ecx,0                  ; round to integer

	movzx eax,byte[edi+6]        ; pixel (x+2;y)
	add   eax,ecx                ; add error/8 to (x+2;y)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok2
@@:
	cmp   eax,255
	jle   .ok2
	or    al,255
.ok2:
	mov   [edi+6],al             ; putpixel

	movzx eax,byte[edi+edx-3]    ; pixel (x-1;y+1)
	add   eax,ecx                ; add error/8 to (x-1;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok3
@@:
	cmp   eax,255
	jle   .ok3
	or    al,255
.ok3:
	mov   [edi+edx-3],al         ; putpixel

	movzx eax,byte[edi+edx+3]    ; pixel (x+1;y+1)
	add   eax,ecx                ; add error/8 to (x+1;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok4
@@:
	cmp   eax,255
	jle   .ok4
	or    al,255
.ok4:
	mov   [edi+edx+3],al         ; putpixel

	sar   ecx,1                  ; error/16
	;adc   ecx,0                  ; round to integer

	movzx eax,byte[edi+edx-6]    ; pixel (x-2;y+1)
	add   eax,ecx                ; add error/16 to (x-2;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok5
@@:
	cmp   eax,255
	jle   .ok5
	or    al,255
.ok5:
	mov   [edi+edx-6],al         ; putpixel

	movzx eax,byte[edi+edx+6]    ; pixel (x+2;y+1)
	add   eax,ecx                ; add error/16 to (x+2;y+1)
	jge   @f                     ; check_overflow
	xor   eax,eax
	jmp   .ok6
@@:
	cmp   eax,255
	jle   .ok6
	or    al,255
.ok6:
	mov   [edi+edx+6],al         ; putpixel

.next:
	inc   edi
	dec   esi
	jnz   newp_2
	ret


align 16
dither_3:                        ; Heavyiron_mod algorithm
 newp_3:                         ; Dithering cycle
    xor   ebx,ebx                ; At first threshold
    movzx ecx,byte[edi]
    cmp   cl,255
    je   .next
    test  cl,cl
    jz    .next
    jns   @f
    dec   ebx
    sub   ecx,255
  @@:
    mov   [edi],bl               ; putpixel

    sar   ecx,2                  ; error/4

    movzx eax,byte[edi+3]        ; pixel (x+1;y)
    add   eax,ecx                ; add error/4 to (x+1;y)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok
  @@:
    cmp   eax,255
    jle   .ok
    or    al,255
  .ok:
    mov   [edi+3],al             ; putpixel

    movzx eax,byte[edi+edx-3]    ; pixel (x-1;y+1)
    add   eax,ecx                ; add error/4 to (x-1;y+1)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok1
  @@:
    cmp   eax,255
    jle   .ok1
    or    al,255
  .ok1:
    mov   [edi+edx-3],al         ; putpixel

    movzx eax,byte[edi+edx]      ; pixel (x;y+1)
    add   eax,ecx                ; add error/4 to (x;y+1)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok2
  @@:
    cmp   eax,255
    jle   .ok2
    or    al,255
  .ok2:
    mov   [edi+edx],al           ; putpixel

  .next:
    inc   edi
    dec   esi
    jnz   newp_3
    ret

align 16
dither_4:                        ; Atkinson algorithm
 newp_4:                         ; Dithering cycle

    xor   ebx,ebx                ; At first threshold
    movsx ecx,byte[edi]
    cmp   cl,255
    je   .next
    test  cl,cl
    jz    .next
    jns   @f
    dec   ebx
  @@:
    mov   [edi],bl               ; putpixel

    sar   ecx,3                  ; error/8

    movzx eax,byte[edi+3]        ; pixel (x+1;y)
    add   eax,ecx                ; add error/8 to (x+1;y)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok
  @@:
    cmp   eax,255
    jle   .ok
    or    al,255
  .ok:
    mov   [edi+3],al             ; putpixel

    movzx eax,byte[edi+edx]      ; pixel (x;y+1)
    add   eax,ecx                ; add error/8 to (x;y+1)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok1
  @@:
    cmp   eax,255
    jle   .ok1
    or    al,255
  .ok1:
    mov   [edi+edx],al           ; putpixel

    movzx eax,byte[edi+6]        ; pixel (x+2;y)
    add   eax,ecx                ; add error/8 to (x+2;y)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok2
  @@:
    cmp   eax,255
    jle   .ok2
    or    al,255
  .ok2:
    mov   [edi+6],al             ; putpixel

    movzx eax,byte[edi+edx-3]    ; pixel (x-1;y+1)
    add   eax,ecx                ; add error/8 to (x-1;y+1)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok3
  @@:
    cmp   eax,255
    jle   .ok3
    or    al,255
  .ok3:
    mov   [edi+edx-3],al         ; putpixel

    movzx eax,byte[edi+edx+3]    ; pixel (x+1;y+1)
    add   eax,ecx                ; add error/8 to (x+1;y+1)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok4
  @@:
    cmp   eax,255
    jle   .ok4
    or    al,255
  .ok4:
    mov   [edi+edx+3],al         ; putpixel


    movzx eax,byte[edi+edx+edx]    ; pixel (x;y+2)
    add   eax,ecx                ; add error/8 to (x;y+2)
    jge   @f                     ; check_overflow
    xor   eax,eax
    jmp   .ok5
  @@:
    cmp   eax,255
    jle   .ok5
    or    al,255
  .ok5:
    mov   [edi+edx+edx],al         ; putpixel

  .next:
    inc   edi
    dec   esi
    jnz   newp_4
    ret



include 'fun_voxel.inc' ;функции для работы с воксельной графикой

txt_err_size_0 db 'image size < 1 pixel',13,10,0
txt_err_n8b db 'need buffer 8 bit',13,10,0
txt_err_n24b db 'need buffer 24 bit',13,10,0
txt_err_n32b db 'need buffer 32 bit',13,10,0
txt_err_n8_24b db 'need buffer 8 or 24 bit',13,10,0

align 16
EXPORTS:
	dd sz_lib_init, lib_init
	dd sz_buf2d_create, buf_create
	dd sz_buf2d_create_f_img, buf_create_f_img
	dd sz_buf2d_clear, buf_clear
	dd sz_buf2d_draw, buf_draw_buf
	dd sz_buf2d_delete, buf_delete
	dd sz_buf2d_resize, buf_resize
	dd sz_buf2d_rotate, buf_rotate
	dd sz_buf2d_line, buf_line_brs
	dd sz_buf2d_line_sm, buf_line_brs_sm
	dd sz_buf2d_rect_by_size, buf_rect_by_size
	dd sz_buf2d_filled_rect_by_size, buf_filled_rect_by_size
	dd sz_buf2d_circle, buf_circle
	dd sz_buf2d_img_hdiv2, buf_img_hdiv2
	dd sz_buf2d_img_wdiv2, buf_img_wdiv2
	dd sz_buf2d_conv_24_to_8, buf_conv_24_to_8
	dd sz_buf2d_conv_24_to_32, buf_conv_24_to_32
	dd sz_buf2d_bit_blt, buf_bit_blt
	dd sz_buf2d_bit_blt_transp, buf_bit_blt_transp
	dd sz_buf2d_bit_blt_alpha, buf_bit_blt_alpha
	dd sz_buf2d_curve_bezier, buf_curve_bezier
	dd sz_buf2d_convert_text_matrix, buf_convert_text_matrix
	dd sz_buf2d_draw_text, buf_draw_text
	dd sz_buf2d_crop_color, buf_crop_color
	dd sz_buf2d_offset_h, buf_offset_h
	dd sz_buf2d_flood_fill, buf_flood_fill
	dd sz_buf2d_set_pixel, buf_set_pixel
	dd sz_buf2d_get_pixel, buf_get_pixel
	dd sz_buf2d_flip_h, buf_flip_h
	dd sz_buf2d_flip_v, buf_flip_v
	dd sz_buf2d_filter_dither, buf_filter_dither
	dd sz_buf2d_vox_brush_create, vox_brush_create
	dd sz_buf2d_vox_brush_delete, vox_brush_delete
	dd sz_buf2d_vox_obj_get_img_w_3g, buf_vox_obj_get_img_w_3g
	dd sz_buf2d_vox_obj_get_img_h_3g, buf_vox_obj_get_img_h_3g
	dd sz_buf2d_vox_obj_draw_1g, buf_vox_obj_draw_1g
	dd sz_buf2d_vox_obj_draw_3g, buf_vox_obj_draw_3g
	dd sz_buf2d_vox_obj_draw_3g_scaled, buf_vox_obj_draw_3g_scaled
	dd sz_buf2d_vox_obj_draw_pl, buf_vox_obj_draw_pl
	dd sz_buf2d_vox_obj_draw_pl_scaled, buf_vox_obj_draw_pl_scaled
	dd sz_buf2d_vox_obj_draw_3g_shadows, buf_vox_obj_draw_3g_shadows
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0 ;очистка буфера указанным цветом
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_rotate db 'buf2d_rotate',0
	sz_buf2d_line db 'buf2d_line',0 ;рисование линии
	sz_buf2d_line_sm db 'buf2d_line_sm',0 ;рисование сглаженной линии
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0 ;рисование рамки прямоугольника, 2-я координата задана по размеру
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0 ;рисование залитого прямоугольника, 2-я координата задана по размеру
	sz_buf2d_circle db 'buf2d_circle',0 ;рисование окружности
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0 ;сжатие изображения по высоте в 2 раза (размер буфера не меняется)
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0 ;сжатие изображения по ширине в 2 раза (размер буфера не меняется)
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
	sz_buf2d_get_pixel db 'buf2d_get_pixel',0
	sz_buf2d_flip_h db 'buf2d_flip_h',0
	sz_buf2d_flip_v db 'buf2d_flip_v',0
	sz_buf2d_filter_dither db 'buf2d_filter_dither',0
	sz_buf2d_vox_brush_create db 'buf2d_vox_brush_create',0
	sz_buf2d_vox_brush_delete db 'buf2d_vox_brush_delete',0
	sz_buf2d_vox_obj_get_img_w_3g db 'buf2d_vox_obj_get_img_w_3g',0
	sz_buf2d_vox_obj_get_img_h_3g db 'buf2d_vox_obj_get_img_h_3g',0
	sz_buf2d_vox_obj_draw_1g db 'buf2d_vox_obj_draw_1g',0
	sz_buf2d_vox_obj_draw_3g db 'buf2d_vox_obj_draw_3g',0
	sz_buf2d_vox_obj_draw_3g_scaled db 'buf2d_vox_obj_draw_3g_scaled',0
	sz_buf2d_vox_obj_draw_pl db 'buf2d_vox_obj_draw_pl',0
	sz_buf2d_vox_obj_draw_pl_scaled db 'buf2d_vox_obj_draw_pl_scaled',0
	sz_buf2d_vox_obj_draw_3g_shadows db 'buf2d_vox_obj_draw_3g_shadows',0
