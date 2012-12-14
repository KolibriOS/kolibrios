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

vox_offs_tree_table equ 4
vox_offs_data equ 12

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

;input:
; ebx = coord x
; ecx = coord y
; edx = pixel color
; edi = pointer to buffer struct
align 4
draw_pixel:
	;cmp buf2d_bits,24
	;jne @f
	bt ebx,31
	jc @f
	bt ecx,31
	jc @f
	cmp ebx,buf2d_w
	jge @f
	cmp ecx,buf2d_h
	jge @f
	push esi
		mov esi,buf2d_w ;size x
		imul esi,ecx ;size_x*y
		add esi,ebx	 ;size_x*y+x
		cmp buf2d_bits,8
		je .beg8
		cmp buf2d_bits,32
		je .beg32
			lea esi,[esi+esi*2] ;(size_x*y+x)*3
			add esi,buf2d_data  ;ptr+(size_x*y+x)*3
			mov word[esi],dx ;copy pixel color
			ror edx,16
			mov byte[esi+2],dl
			ror edx,16
			jmp .end_draw
		.beg8: ;рисование точки в 8 битном буфере
			add esi,buf2d_data  ;ptr+(size_x*y+x)
			mov byte[esi],dl
			jmp .end_draw
		.beg32: ;рисование точки в 32 битном буфере
			shl esi,2
			add esi,buf2d_data  ;ptr+(size_x*y+x)
			mov dword[esi],edx
		.end_draw:
	pop esi
	@@:
	ret

;input:
; ebx = coord x
; ecx = coord y
; edi = pointer to buffer struct
;output:
; eax = цвет точки
; в случае ошибки eax = 0xffffffff
align 4
get_pixel_8:
	mov eax,0xffffffff

	bt ebx,31
	jc @f
	bt ecx,31
	jc @f
	cmp ebx,buf2d_w
	jge @f
	cmp ecx,buf2d_h
	jge @f
	push esi
		mov esi,buf2d_w ;size x
		imul esi,ecx ;size_x*y
		add esi,ebx	 ;size_x*y+x
		add esi,buf2d_data  ;ptr+(size_x*y+x)

		movzx eax,byte[esi] ;copy pixel color
	pop esi
	@@:
	ret

;input:
; ebx = coord x
; ecx = coord y
; edi = pointer to buffer struct
;output:
; eax = цвет точки
; в случае ошибки eax = 0xffffffff
align 4
get_pixel_24:
	mov eax,0xffffffff

	bt ebx,31
	jc @f
	bt ecx,31
	jc @f
	cmp ebx,buf2d_w
	jge @f
	cmp ecx,buf2d_h
	jge @f
	push esi
		mov esi,buf2d_w ;size x
		imul esi,ecx ;size_x*y
		add esi,ebx	 ;size_x*y+x
		lea esi,[esi+esi*2] ;(size_x*y+x)*3
		add esi,buf2d_data  ;ptr+(size_x*y+x)*3

		xor eax,eax
		mov ax,word[esi] ;copy pixel color
		ror eax,16
		mov al,byte[esi+2]
		ror eax,16
	pop esi
	@@:
	ret

;input:
; ebx = coord x
; ecx = coord y
; edi = pointer to buffer struct
;output:
; eax = цвет точки
; в случае ошибки eax = 0xffffffff
align 4
get_pixel_32:
	mov eax,0xffffffff

	bt ebx,31
	jc @f
	bt ecx,31
	jc @f
	cmp ebx,buf2d_w
	jge @f
	cmp ecx,buf2d_h
	jge @f
	push esi
		mov esi,buf2d_w ;size x
		imul esi,ecx ;size_x*y
		add esi,ebx	 ;size_x*y+x
		shl esi,2
		add esi,buf2d_data  ;ptr+(size_x*y+x)*4

		mov eax,dword[esi] ;copy pixel color
	pop esi
	@@:
	ret

;input:
; ebx = coord x
; ecx = coord y
; edx = pixel color + transparent
; edi = pointer to buffer struct
; t_prop, m_prop - коэфициенты необходимые для вычисления степени прозрачности
align 4
transp_32 dd 0 ;цвет рисуемой точки + прозрачность
align 4
proc draw_pixel_transp, t_prop:dword, m_prop:dword
	;cmp buf2d_bits,24
	;jne @f
	bt ebx,31
	jc @f
	bt ecx,31
	jc @f
	cmp ebx,buf2d_w
	jge @f
	cmp ecx,buf2d_h
	jge @f
	push eax ebx edx edi esi
		mov esi,buf2d_w ;size x
		imul esi,ecx ;size_x*y
		add esi,ebx	 ;size_x*y+x
		lea esi,[esi+esi*2] ;(size_x*y+x)*3
		add esi,buf2d_data  ;ptr+(size_x*y+x)*3

		mov edi,esi ;указатель на цвет фона
		mov dword[transp_32],edx ;цвет рисуемой точки

		xor edx,edx
		mov eax,[t_prop]
		shl eax,8 ;*=256
		mov ebx,[m_prop]
		div ebx ;вычисляем коэф. прозрачности (должен быть от 0 до 255)
		bt ax,8
		jnc .over_255
			;если коеф. прозрачности >=256 то уменьшаем его до 255
			mov al,0xff
		.over_255:

		mov byte[transp_32+3],al ;прозрачность рисуемой точки
		mov esi,dword transp_32 ;указатель на цвет рисуемой точки

		call combine_colors_0
	pop esi edi edx ebx eax
	@@:
	ret
endp

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

	cmp buf2d_bits,24
	jne @f
		cld
		mov esi,[rgb_data]
		mov edi,eax ;eax=buf2d_data
		rep movsb ;копируем биты изображения в буфер
		jmp .end_create
	@@:
		stdcall buf_clear,edi,buf2d_color ;очистка буфера фоновым цветом
	.end_create:
	popad
	ret
endp

align 4
proc buf_clear, buf_struc:dword, color:dword ;очистка буфера заданым цветом
	pushad
	mov edi,dword[buf_struc]

	mov ecx,buf2d_w
	mov ebx,buf2d_h
	imul ecx,ebx

	cld

	cmp buf2d_bits,8
	jne .end_clear_8
		mov edi,buf2d_data
		mov al,byte[color]
		rep stosb
		jmp .end_clear_32
	.end_clear_8:

	cmp buf2d_bits,24
	jne .end_clear_24
		mov edi,buf2d_data
		mov eax,dword[color]
		mov ebx,eax
		shr ebx,16
		@@:
			stosw
			mov byte[edi],bl
			inc edi
			loop @b
		jmp .end_clear_32
	.end_clear_24:

	cmp buf2d_bits,32
	jne .end_clear_32
		mov edi,buf2d_data
		mov eax,dword[color]
		rep stosd
		;jmp .end_clear_32
	.end_clear_32:
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
proc buf_draw_buf, buf_struc:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .error
		mov eax,7
		mov ebx,buf2d_data

		mov ecx,buf2d_w
		ror ecx,16
		mov edx,buf2d_h
		mov cx,dx

		mov edx,buf2d_size_lt
		ror edx,16
		int 0x40
		jmp .end_draw_24
	.error:
		stdcall print_err,sz_buf2d_draw,txt_err_n24b
	.end_draw_24:
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

align 4
proc buf_resize, buf_struc:dword, new_w:dword, new_h:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .24bit
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
proc buf_line_brs, buf_struc:dword, coord_x0:dword, coord_y0:dword, coord_x1:dword, coord_y1:dword, color:dword
locals
	loc_1 dd ?
	loc_2 dd ?
	napravl db ?
endl
	pushad
		mov eax,dword[coord_x1]
		sub eax,dword[coord_x0]
		bt eax,31
		jae @f
			neg eax
			inc eax
		@@:
		mov ebx,dword[coord_y1]
		sub ebx,dword[coord_y0]
		jnz @f
			;если задана горизонтальная линия y0=y1
			stdcall buf_line_h, [buf_struc], [coord_x0], [coord_y0], [coord_x1], [color]
			jmp .coord_end
		@@:
		bt ebx,31
		jae @f
			neg ebx
			inc ebx
		@@:
		mov edx,dword[color]

		mov [napravl],byte 0 ;bool steep=false
		cmp eax,ebx
		jle @f
			mov [napravl],byte 1 ;bool steep=true
			swap dword[coord_x0],dword[coord_y0] ;swap(x0, y0);
			swap dword[coord_x1],dword[coord_y1] ;swap(x1, y1);
		@@:
		mov eax,dword[coord_y0] ;x0
		cmp eax,dword[coord_y1] ;if(x0>x1)
		jle @f
			swap dword[coord_y0],dword[coord_y1] ;swap(x0, x1);
			swap dword[coord_x0],dword[coord_x1] ;swap(y0, y1);
		@@:

; int deltax esi
; int deltay edi
; int error  ebp-6
; int ystep  ebp-8

		mov eax,dword[coord_y0]
		mov esi,dword[coord_y1]
		sub esi,eax ;deltax = y1-y0
		mov ebx,esi
		shr ebx,1
		mov [loc_1],ebx ;error = deltax/2

		mov eax,dword[coord_x0]
		mov edi,dword[coord_x1]
		mov [loc_2],dword -1 ;ystep = -1
		cmp eax,edi ;if (x0<x1) ystep = 1;
		jge @f
			mov [loc_2],dword 1 ;ystep = 1
		@@:
		sub edi,eax ;x1-x0

		bts edi,31
		jae @f
			neg edi
			inc edi
		@@:
		and edi,0x7fffffff ;deltay = abs(x1-x0)

		mov eax,edi
		mov edi,[buf_struc]
		cmp buf2d_bits,8
		je @f
		cmp buf2d_bits,24
		je @f
			jmp .coord_end
		@@:

		cmp [napravl],0
		jne .coord_yx
			mov ebx,dword[coord_x0]
			mov ecx,dword[coord_y0]

			@@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
				cmp ecx,dword[coord_y1]
				jg @f ;jge ???
				call draw_pixel

				sub dword[loc_1],eax ;error -= deltay
				cmp dword[loc_1],0 ;if(error<0)
				jge .if0
					add ebx,[loc_2] ;y += ystep
					add [loc_1],esi ;error += deltax
				.if0:
				inc ecx
				jmp @b
			@@:
			jmp .coord_end
		.coord_yx:
			mov ebx,dword[coord_y0]
			mov ecx,dword[coord_x0]

			@@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
				cmp ebx,dword[coord_y1]
				jg @f ;jge ???
				call draw_pixel

				sub dword[loc_1],eax ;error -= deltay
				cmp dword[loc_1],0 ;if(error<0)
				jge .if1
					add ecx,[loc_2] ;y += ystep
					add [loc_1],esi ;error += deltax
				.if1:
				inc ebx
				jmp @b
			@@:
	.coord_end:
	popad
	ret
endp

;рисование сглаженной линии
align 4
proc buf_line_brs_sm, buf_struc:dword, coord_x0:dword, coord_y0:dword, coord_x1:dword, coord_y1:dword, color:dword
locals
	loc_1 dd ?
	loc_2 dd ?
	napravl db ?
endl
	pushad
		mov eax,dword[coord_x1]
		sub eax,dword[coord_x0]
		bt eax,31
		jae @f
			neg eax
			inc eax
		@@:
		mov ebx,dword[coord_y1]
		sub ebx,dword[coord_y0]
		jnz @f
			;если задана горизонтальная линия y0=y1
			stdcall buf_line_h, [buf_struc], [coord_x0], [coord_y0], [coord_x1], [color]
			jmp .coord_end
		@@:
		bt ebx,31
		jae @f
			neg ebx
			inc ebx
		@@:
		mov edx,dword[color]

		mov [napravl],byte 0 ;bool steep=false
		cmp eax,ebx
		jle @f
			mov [napravl],byte 1 ;bool steep=true
			swap dword[coord_x0],dword[coord_y0] ;swap(x0, y0);
			swap dword[coord_x1],dword[coord_y1] ;swap(x1, y1);
		@@:
		mov eax,dword[coord_y0] ;x0
		cmp eax,dword[coord_y1] ;if(x0>x1)
		jle @f
			swap dword[coord_y0],dword[coord_y1] ;swap(x0, x1);
			swap dword[coord_x0],dword[coord_x1] ;swap(y0, y1);
		@@:

; int deltax esi
; int deltay edi
; int error  ebp-6
; int ystep  ebp-8

		mov eax,dword[coord_y0]
		mov esi,dword[coord_y1]
		sub esi,eax ;deltax = y1-y0
		mov ebx,esi
		shr ebx,1
		mov [loc_1],ebx ;error = deltax/2

		mov eax,dword[coord_x0]
		mov edi,dword[coord_x1]
		mov [loc_2],dword -1 ;ystep = -1
		cmp eax,edi ;if (x0<x1) ystep = 1;
		jge @f
			mov [loc_2],dword 1 ;ystep = 1
		@@:
		sub edi,eax ;x1-x0

		bts edi,31
		jae @f
			neg edi
			inc edi
		@@:
		and edi,0x7fffffff ;deltay = abs(x1-x0)

		mov eax,edi
		mov edi,[buf_struc]
		cmp buf2d_bits,24
		jne .coord_end

		cmp [napravl],0
		jne .coord_yx
			mov ebx,dword[coord_x0]
			mov ecx,dword[coord_y0]

			@@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
				cmp ecx,dword[coord_y1]
				jg @f ;jge ???
				push eax
					mov eax,esi
					sub eax,[loc_1]
					stdcall draw_pixel_transp, eax,esi
				pop eax
				add ebx,[loc_2]
				stdcall draw_pixel_transp, [loc_1],esi
				sub ebx,[loc_2]

				sub dword[loc_1],eax ;error -= deltay
				cmp dword[loc_1],0 ;if(error<0)
				jge .if0
					add ebx,[loc_2] ;y += ystep
					add [loc_1],esi ;error += deltax
				.if0:
				inc ecx
				jmp @b
			@@:
			jmp .coord_end
		.coord_yx:
			mov ebx,dword[coord_y0]
			mov ecx,dword[coord_x0]

			@@: ;for (x=x0 ; x<x1; x++) ;------------------------------------
				cmp ebx,dword[coord_y1]
				jg @f ;jge ???
				push eax
					mov eax,esi
					sub eax,[loc_1]
					stdcall draw_pixel_transp, eax,esi
				pop eax
				add ecx,[loc_2]
				stdcall draw_pixel_transp, [loc_1],esi
				sub ecx,[loc_2]

				sub dword[loc_1],eax ;error -= deltay
				cmp dword[loc_1],0 ;if(error<0)
				jge .if1
					add ecx,[loc_2] ;y += ystep
					add [loc_1],esi ;error += deltax
				.if1:
				inc ebx
				jmp @b
			@@:
	.coord_end:
	popad
	ret
endp

;рисование горизонтальной линии, потому нет параметра coord_y1
align 4
proc buf_line_h, buf_struc:dword, coord_x0:dword, coord_y0:dword, coord_x1:dword, color:dword
	pushad
	pushfd
		mov edi,[buf_struc]
		cmp buf2d_bits,8
		je @f
		cmp buf2d_bits,24
		je @f
			jmp .end24
		@@: ;определение координат линии относительно буфера

		mov ecx,dword[coord_y0]
		bt ecx,31
		jc .end24 ;если координата y0 отрицательная
		cmp ecx,buf2d_h
		jge .end24 ;если координата y0 больше высоты буфера

		mov ebx,dword[coord_x0]
		mov esi,dword[coord_x1]
		cmp ebx,esi
		jle @f
			xchg ebx,esi ;если x0 > x1 то меняем местами x0 и x1
		@@:
		bt ebx,31
		jae @f
			;если координата x0 отрицательная
			xor ebx,ebx
		@@:
		cmp esi,buf2d_w
		jl @f
			;если координата x0 больше ширины буфера
			mov esi,buf2d_w
			dec esi
		@@:
		cmp ebx,esi
		jg .end24 ;если x0 > x1 может возникнуть когда обе координаты x0, x1 находились за одним из пределов буфера

		cmp buf2d_bits,24
		je .beg24
			;рисование в 8 битном буфере
			;в edx вычисляем начало 1-й точки линии в буфере изображения
			mov edx,buf2d_w ;size x
			imul edx,ecx ;size_x*y
			add edx,ebx	 ;size_x*y+x
			add edx,buf2d_data ;ptr+(size_x*y+x)
			mov edi,edx ;теперь можем портить указатель на буфер

			mov ecx,esi
			sub ecx,ebx ;в ecx колличество точек линии выводимых в буфер
			inc ecx ;что-бы последняя точка линии также отображалась
			mov eax,dword[color] ;будем использовать только значение в al
			cld
			rep stosb ;цикл по оси x от x0 до x1 (включая x1)
			jmp .end24

		.beg24: ;рисование в 24 битном буфере
		;в eax вычисляем начало 1-й точки линии в буфере изображения
		mov eax,buf2d_w ;size x
		imul eax,ecx ;size_x*y
		add eax,ebx	 ;size_x*y+x
		lea eax,[eax+eax*2] ;(size_x*y+x)*3
		add eax,buf2d_data  ;ptr+(size_x*y+x)*3

		mov ecx,esi
		sub ecx,ebx ;в ecx колличество точек линии выводимых в буфер
		inc ecx ;что-бы последняя точка линии также отображалась
		mov edx,dword[color]
		mov ebx,edx ;координата x0 в ebx уже не нужна
		ror edx,16 ;поворачиваем регистр что бы 3-й байт попал в dl
		cld
		@@: ;цикл по оси x от x0 до x1 (включая x1)
			mov word[eax],bx ;copy pixel color
			mov byte[eax+2],dl
			add eax,3
			loop @b
		.end24:
	popfd
	popad
	ret
endp

align 4
proc buf_rect_by_size, buf_struc:dword, coord_x:dword,coord_y:dword,w:dword,h:dword, color:dword
pushad
	mov edi,[buf_struc]
	cmp buf2d_bits,8
	je @f
	cmp buf2d_bits,24
	je @f
		jmp .coord_end
	@@:

		mov eax,[coord_x]
		mov ebx,[coord_y]
		mov ecx,[w]
		;cmp ecx,1
		;jl .coord_end
		cmp ecx,0
		je .coord_end
		jg @f
			add eax,ecx
			inc eax
			neg ecx
		@@:
		add ecx,eax
		dec ecx
		mov edx,[h]
		;cmp edx,1
		;jl .coord_end
		cmp edx,0
		je .coord_end
		jg @f
			add ebx,edx
			inc ebx
			neg edx
		@@:

		add edx,ebx
		dec edx
		mov esi,dword[color]
		stdcall buf_line_h, edi, eax, ebx, ecx, esi ;линия -
		stdcall buf_line_brs, edi, eax, ebx, eax, edx, esi ;линия |
		stdcall buf_line_h, edi, eax, edx, ecx, esi ;линия -
		stdcall buf_line_brs, edi, ecx, ebx, ecx, edx, esi ;линия |
	.coord_end:
popad
	ret
endp

align 4
proc buf_filled_rect_by_size, buf_struc:dword, coord_x:dword,coord_y:dword,w:dword,h:dword, color:dword
pushad
	mov edi,[buf_struc]
	cmp buf2d_bits,8
	je @f
	cmp buf2d_bits,24
	je @f
		jmp .coord_end
	@@:
		mov eax,[coord_x]
		mov ebx,[coord_y]
		mov edx,[w]
		cmp edx,0
		je .coord_end ;если высота 0 пикселей
		jg @f ;если высота положительная
			add eax,edx
			inc eax
			neg edx ;ширину делаем положительной
			;inc edx ;почему тут не добавляем 1-цу я не знаю, но с ней работает не правильно
		@@:
		add edx,eax
		dec edx
		mov ecx,[h]
		cmp ecx,0
		je .coord_end ;если высота 0 пикселей
		jg @f ;если высота положительная
			add ebx,ecx ;сдвигаем верхнюю координату прямоугольника
			inc ebx
			neg ecx ;высоту делаем положительной
			;inc ecx ;почему тут не добавляем 1-цу я не знаю, но с ней работает не правильно
		@@:
		mov esi,dword[color]
		cld
		@@:
			stdcall buf_line_h, edi, eax, ebx, edx, esi ;линия -
			inc ebx
			loop @b
	.coord_end:
popad
	ret
endp

align 4
proc buf_circle, buf_struc:dword, coord_x:dword, coord_y:dword, r:dword, color:dword
locals
	po_x dd ?
	po_y dd ?
endl
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,8
	je @f
	cmp buf2d_bits,24
	je @f
		jmp .error
	@@:
		mov edx,dword[color]

		finit
		fild dword[coord_x]
		fild dword[coord_y]
		fild dword[r]
		fldz ;px=0
		fld st1 ;py=r

		fldpi
		fmul st0,st3
		fistp dword[po_x]
		mov esi,dword[po_x] ;esi=pi*r
		shl esi,1 ;esi=2*pi*r

		;st0 = py
		;st1 = px
		;st2 = r
		;st3 = y
		;st4 = x

		@@:
			;Point(px + x, y - py)
			fld st1 ;st0=px
			fadd st0,st5 ;st0=px+x
			fistp dword[po_x]
			mov ebx,dword[po_x]
			fld st3 ;st0=y
			fsub st0,st1 ;st0=y-py
			fistp dword[po_y]
			mov ecx,dword[po_y]
			call draw_pixel
			;px += py/r
			fld st0 ;st0=py
			fdiv st0,st3 ;st0=py/r
			faddp st2,st0 ;st3+=st0
			;py -= px/r
			fld st1 ;st0=px
			fdiv st0,st3 ;st0=px/r
			fsubp st1,st0 ;st2-=st0

			dec esi
			cmp esi,0
			jge @b
		jmp .exit_fun
	.error:
		stdcall print_err,sz_buf2d_circle,txt_err_n8_24b
	.exit_fun:

	popad
	ret
endp

;функция для заливки области выбранным цветом
align 4
proc buf_flood_fill, buf_struc:dword, coord_x:dword, coord_y:dword, mode:dword, color_f:dword, color_b:dword
	pushad
		mov edi,[buf_struc]
		cmp buf2d_bits,24
		jne .end24

			mov ebx,dword[coord_x]
			mov ecx,dword[coord_y]
			mov edx,dword[color_f]
			mov esi,dword[color_b]

			cmp dword[mode],1 ;в зависимости от 'mode' определяем каким алгоритмом будем пользоваться
			je @f
				call buf_flood_fill_recurs_0 ;заливаем до пикселей цвета esi
				jmp .end24
			@@:
				call buf_flood_fill_recurs_1 ;заливаем пиксели имеющие цвет esi

		.end24:
	popad
	ret
endp

;input:
; ebx = coord_x
; ecx = coord_y
; edx = цвет заливки
; esi = цвет границы, до которой будет ити заливка
; edi = buf_struc
;output:
; eax = портится
align 4
buf_flood_fill_recurs_0:
	call get_pixel_24
	cmp eax,0xffffffff ;if error coords
	je .end_fun
	cmp eax,edx ;если цвет пикселя совпал с цветом заливки, значит заливка в этой области уже была сделана
	je .end_fun

		call draw_pixel

		dec ebx
		call get_pixel_24
		cmp eax,esi
		je @f
			call buf_flood_fill_recurs_0
		@@:
		inc ebx


		inc ebx
		call get_pixel_24
		cmp eax,esi
		je @f
			call buf_flood_fill_recurs_0
		@@:
		dec ebx

		dec ecx
		call get_pixel_24
		cmp eax,esi
		je @f
			call buf_flood_fill_recurs_0
		@@:
		inc ecx

		inc ecx
		call get_pixel_24
		cmp eax,esi
		je @f
			call buf_flood_fill_recurs_0
		@@:
		dec ecx

	.end_fun:
	ret

;input:
; ebx = coord_x
; ecx = coord_y
; edx = цвет заливки
; esi = цвет пикселей, по которым будет ити заливка
; edi = buf_struc
;output:
; eax = портится
align 4
buf_flood_fill_recurs_1:
	call get_pixel_24
	cmp eax,0xffffffff ;if error coords
	je .end_fun
	cmp eax,edx ;если цвет пикселя совпал с цветом заливки, значит заливка в этой области уже была сделана
	je .end_fun
	cmp eax,esi ;если цвет пикселя не совпал с заливаемым цветом заливки, то прекращаем заливку
	jne .end_fun

		call draw_pixel

		dec ebx
		call get_pixel_24
		cmp eax,esi
		jne @f
			call buf_flood_fill_recurs_1
		@@:
		inc ebx


		inc ebx
		call get_pixel_24
		cmp eax,esi
		jne @f
			call buf_flood_fill_recurs_1
		@@:
		dec ebx

		dec ecx
		call get_pixel_24
		cmp eax,esi
		jne @f
			call buf_flood_fill_recurs_1
		@@:
		inc ecx

		inc ecx
		call get_pixel_24
		cmp eax,esi
		jne @f
			call buf_flood_fill_recurs_1
		@@:
		dec ecx

	.end_fun:
	ret

;функция для рисования точки
align 4
proc buf_set_pixel uses ebx ecx edx edi, buf_struc:dword, coord_x:dword, coord_y:dword, color:dword
	mov edi,dword[buf_struc]
	mov ebx,dword[coord_x]
	mov ecx,dword[coord_y]
	mov edx,dword[color]
	call draw_pixel
	ret
endp

;output:
; eax = цвет точки
; в случае ошибки eax = 0xffffffff
align 4
proc buf_get_pixel uses ebx ecx edi, buf_struc:dword, coord_x:dword, coord_y:dword
	mov edi,dword[buf_struc]
	mov ebx,[coord_x]
	mov ecx,[coord_y]

	cmp buf2d_bits,8
	jne @f
		call get_pixel_8
		jmp .end_fun
	@@:
	cmp buf2d_bits,24
	jne @f
		call get_pixel_24
		jmp .end_fun
	@@:
	cmp buf2d_bits,32
	jne @f
		call get_pixel_32
		;jmp .end_fun
	@@:
	.end_fun:
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
        jle .end_24
        mov ecx,[line_pix]
        @@:
            lodsw
            mov dx,word[edi]
            mov word[esi-2],dx
            mov [edi],ax
            lodsb
            mov ah,byte[edi+2]
            mov byte[esi-1],ah
            mov [edi+2],al
            add edi,3
            loop @b
        sub edi,[line_2byte]
        dec ebx
        jmp .flip_24
    .end_24:
    popad
    ret
endp

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
; сжатие изображения по высоте (высота буфера не меняется)
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
	inc edi
	add edi,255
	neg esi
	inc esi
	add esi,255

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
	inc ecx
	add ecx,255

	shl ecx,24
	add ebx,ecx
	mov edx,ebx
pop ebx eax

	jmp .end_f
	.c0_c1: ;если прозрачности обоих цветов совпадают
		mov edx,dword[eax]
		shr edx,1
		and edx,011111110111111101111111b
		mov esi,dword[ebx]
		shr esi,1
		and esi,011111110111111101111111b
		add edx,esi
		ror edi,8 ;перемещаем значение прозрачности в старший байт edi
		or edx,edi
		jmp .end_f
	.c0z: ;если цвет в eax прозрачный
		mov edx,dword[ebx]
		movzx edi,byte[ebx+3]
		jmp @f
	.c1z: ;если цвет в ebx прозрачный
		mov edx,dword[eax]
	@@:
		add edi,255 ;делаем цвет на половину прозрачным
		shr edi,1
		cmp edi,255
		jl @f
			mov edi,255 ;максимальная прозрачность не более 255
		@@:
		shl edi,24
		and edx,0xffffff ;снимаем старую прозрачность
		add edx,edi
	.end_f:
	ret
endp

;преобразование буфера из 24-битного в 8-битный
; spectr - определяет какой спектр брать при преобразовании 0-синий, 1-зеленый, 2-красный
align 4
proc buf_conv_24_to_8, buf_struc:dword, spectr:dword
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .error
		mov eax,buf2d_w
		mov ecx,buf2d_h
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
	.error:
		stdcall print_err,sz_buf2d_conv_24_to_8,txt_err_n24b
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
		right_bytes dd ?
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
			;inc ecx
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

	mov [right_bytes],0
	mov ecx,[coord_x]
	cmp ecx,ebx
	jl @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		lea ecx,[ecx+ecx*2] ;ecx - число байт в 1-й строке картинки, которые вылазят за правую сторону
		mov [right_bytes],ecx
	@@:

	lea eax,[eax+eax*2] ;колличество байт в 1-й строке копируемой картинки
	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки

	cld
	cmp [right_bytes],0
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
		add esi,[right_bytes] ;добавляем байты, которые вылазят за правую границу
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
			;inc ecx
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

	mov [right_bytes],0
	mov ecx,[coord_x]
	cmp ecx,ebx
	jl @f
		sub ecx,ebx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
		shl ecx,2 ;ecx - число байт в 1-й строке картинки, которые вылазят за правую сторону
		mov [right_bytes],ecx
	@@:

	;eax - колличество пикселей в 1-й строке копируемой картинки
	lea ebx,[ebx+ebx*2] ;колличество байт в 1-й строке буфера минус число байт в 1-й строке копируемой картинки

	cld
	cmp [right_bytes],0
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
		add esi,[right_bytes] ;добавляем байты, которые вылазят за правую границу
		dec edx
		cmp edx,0
		jg .copy_3

	.copy_end:
	popad
	ret
endp

;input:
; esi = pointer to color1 + transparent
; edi = pointer to background color2
;output:
; [edi] = combine color
align 4
combine_colors_0:
	push ax bx cx dx
	mov bx,0x00ff ;---get transparent---
	movzx cx,byte[esi+3] ;pro
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

	pop dx cx bx ax
	ret

;функция копирует изображение из буфера buf_source (32b) в buf_destination (24b)
; указываются координаты вставки буфера buf_source относительно buf_destination
; при копировании учитывается прозрачность
align 4
proc buf_bit_blt_transp, buf_destination:dword, coord_x:dword, coord_y:dword, buf_source:dword
	locals
		lost_bytes dd ?
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
			;inc ecx
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
		;inc ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		shl ecx,2
		mov [lost_bytes],ecx
		add esi,ecx
		shr ecx,2
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
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

;	mov [right_bytes],0
;	mov ecx,[coord_x]
;	cmp ecx,ebx
;	jl @f
;		sub ecx,ebx
;		sub eax,ecx ;укорачиваем копируемую строку
;		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
;		shl ecx,2 ;ecx - число байт в 1-й строке картинки, которые вылазят за правую сторону
;		mov [right_bytes],ecx
;	@@:

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
; ebx - color1
; esi = pointer to transparent
; edi = pointer to background color2
;output:
; [edi] = combine color
align 4
combine_colors_2:
	push ax ebx cx dx si
	mov cl,byte[esi] ;pro
	xor ch,ch
	mov si,0x00ff ;---get transparent---
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

	pop si dx cx ebx ax
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
			;inc ecx
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
		;inc ecx
		cmp eax,ecx ;eax - ширина копируемой картинки
		jle .copy_end ;если копируемое изображение находится полностью за левой границей буфера (coord_x<0 и |coord_x|>buf_source.w)
		mov [lost_bytes],ecx
		sub eax,ecx ;укорачиваем копируемую строку
		add ebx,ecx ;удлинняем строку для сдвига главной картинки буфера
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

;преобразование 8-битного буфера размером 16*16 в размер 1*256 символов
align 4
proc buf_convert_text_matrix, buf_struc:dword
	locals
		tmp_mem dd ?
		c1 dw ?
		c2 dd ?
		c3 dw ?
	endl
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,8
	jne .error
		mov ecx,buf2d_h
		mov ebx,ecx
		shr ebx,4 ;предполагаем что в буфере 16 строк с символами, потому делим на 2^4
		mov edx,buf2d_w
		imul ecx,edx ;ecx = size  8 b
		invoke mem.alloc,ecx ;выделяем временную память
		mov [tmp_mem],eax ;eax - new memory

		shr edx,4 ;предполагаем что в буфере 16 колонок с символами, потому делим на 2^4
		mov eax,ebx
		imul ebx,edx ;вычисляем кооличество пикселей на 1 символ
		;eax = bhe - высота буквы
		;ebx = bwi*bhe - колличество пикселей в 1-й букве
		;edx = bwi - ширина буквы
		;ecx,esi,edi - используются в цикле .c_0
		shr buf2d_w,4
		shl buf2d_h,4 ;преобразовываем размеры буфера

		cld
		mov esi,buf2d_data
		mov edi,[tmp_mem]
		mov word[c3],16
		.c_3:
			mov dword[c2],eax
			.c_2:
				mov word[c1],16
				.c_1:
					mov ecx,edx ;.c_0:
					rep movsb
					add edi,ebx
					sub edi,edx ;edi+=(bwi*bhe-bwi)
					dec word[c1]
					cmp word[c1],0
					jg .c_1
				add edi,edx
				shl ebx,4
				sub edi,ebx ;edi-=(16*bwi*bhe-bwi)
				shr ebx,4
				dec dword[c2]
				cmp dword[c2],0
				jg .c_2
			sub edi,ebx
			shl ebx,4
			add edi,ebx ;edi+=(15*bwi*bhe)
			shr ebx,4
			dec word[c3]
			cmp word[c3],0
			jg .c_3

		mov edi,dword[buf_struc] ;копирование новой матрицы в основной буфер
		mov edi,buf2d_data
		mov esi,[tmp_mem]
		mov ecx,ebx
		shl ecx,8
		rep movsb
		invoke mem.free,[tmp_mem] ;чистим временную память
		jmp .end_conv
	.error:
		stdcall print_err,sz_buf2d_convert_text_matrix,txt_err_n8b
	.end_conv:
	popad
	ret
endp

align 4
buf_s_matr buf_2d_header ? ;локальная матрица символа

align 4
proc buf_draw_text, buf_struc:dword, buf_t_matr:dword, text:dword, coord_x:dword, coord_y:dword, color:dword
	locals
		buf_t_matr_offs dd ?
	endl
	pushad
	mov edi,dword[buf_struc]
	cmp buf2d_bits,24
	jne .error2
	mov edi,dword[buf_t_matr]
	cmp buf2d_bits,8
	jne .error1
		mov edx,buf2d_data
		mov [buf_t_matr_offs],edx
		mov ecx,BUF_STRUCT_SIZE ;копируем структуру текстовой матрицы
		mov esi,edi
		lea edi,[buf_s_matr]
		cld
		rep movsb
		lea edi,[buf_s_matr]
		shr buf2d_h,8 ;делим высоту символьного буфера на 256, для нахождения высоты 1-го символа
		mov ebx,buf2d_h ;берем высоту символа
		mov ecx,buf2d_w ;берем ширину символа

		mov eax,[coord_x]
		mov esi,[text]
		cmp byte[esi],0
		je .end_draw ;если пустая строка
		@@:
			xor edx,edx
			mov dl,byte[esi] ;берем код символа
			imul edx,ebx ;умножаем его на высоту символа
			imul edx,ecx ;умножаем на ширину символа
			add edx,[buf_t_matr_offs] ;прибавляем смещение 0-го символа, т. е. получается смещение выводимого символа
			mov buf2d_data,edx ;в локальный буфер символа, ставим указатель на нужный символ из буфера buf_t_matr
			stdcall buf_bit_blt_alpha, [buf_struc], eax,[coord_y], edi,[color]
			add eax,ecx
			.new_s:
				inc esi
				cmp byte[esi],13
				jne .no_13
					mov eax,[coord_x]
					add [coord_y],ebx
					jmp .new_s
				.no_13:
			cmp byte[esi],0
			jne @b
		jmp .end_draw
	.error1:
		stdcall print_err,sz_buf2d_draw_text,txt_err_n8b
		jmp .end_draw
	.error2:
		stdcall print_err,sz_buf2d_draw_text,txt_err_n24b
	.end_draw:
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

;input:
; ebp+8  = p0
; ebp+12 = p1
align 4
line_len4i:
	push ebp
	mov ebp,esp
		finit
		fild word [ebp+8]
		fisub word [ebp+12]
		fmul st0,st0 ;st0=x^2
		fild word [ebp+10]
		fisub word [ebp+14]
		fmul st0,st0 ;st0=y^2
		fadd st0,st1
		fsqrt
		fstp dword [ebp+12]
	pop ebp
	ret 4 ;8

align 4
proc buf_curve_bezier, buffer:dword, coord_p0:dword,coord_p1:dword,coord_p2:dword, color:dword
	locals
		delt_t dd ?
		opr_param dd ?
		v_poi_0 dd ?
	endl
	pushad

;float t, xt,yt;
;for(t=.0;t<1.;t+=.005){
;  xt=pow(1.-t,2)*x0+2*t*(1.-t)*x1+pow(t,2)*x2;
;  yt=pow(1.-t,2)*y0+2*t*(1.-t)*y1+pow(t,2)*y2;
;  dc.SetPixel(xt,yt,255L);
;}

	mov edx,[color] ;set curve color
	mov edi,[buffer]
	xor ebx,ebx
	xor ecx,ecx

	finit

	; calculate delta t
	stdcall line_len4i, dword[coord_p1],dword[coord_p0]
	fadd dword[esp]
	add esp,4 ;pop ...

	stdcall line_len4i, dword[coord_p2],dword[coord_p1]
	fadd dword[esp]
	add esp,4 ;pop ...

	fadd st0,st0 ; len*=2
	ftst
	fstsw ax

	fld1
	sahf
	jle @f ;избегаем деления на 0
		fdiv st0,st1
	@@:
	fstp dword[delt_t]

	finit

	;fild word[coord_p2+2] ;y2
	fild word[coord_p1+2] ;y1
	fild word[coord_p0+2] ;y0
	fild word[coord_p2] ;x2
	fild word[coord_p1] ;x1
	fild word[coord_p0] ;x0
	fld dword[delt_t]
	fldz ;t=.0

	@@:
		fld1
		fsub st0,st1 ;1.-t
		fmul st0,st0 ;pow(1.-t,2)
		fmul st0,st3 ;...*x0
		fstp dword[opr_param]

		fld1
		fsub st0,st1 ;1.-t
		fmul st0,st1 ;(1.-t)*t
		fadd st0,st0
		fmul st0,st4 ;...*x1
		mov esi,dword[opr_param]
		fstp dword[opr_param]

		fldz
		fadd st0,st1 ;0+t
		fmul st0,st0 ;t*t
		fmul st0,st5 ;...*x2

		fadd dword[opr_param]
		mov dword[opr_param],esi
		fadd dword[opr_param]
		fistp word[v_poi_0] ;x

		fld1
		fsub st0,st1 ;1.-t
		fmul st0,st0 ;pow(1.-t,2)
		fmul st0,st6 ;...*y0
		fstp dword[opr_param]

		fld1
		fsub st0,st1 ;1.-t
		fmul st0,st1 ;(1.-t)*t
		fadd st0,st0
		fmul st0,st7 ;...*y1
		mov esi,dword[opr_param]
		fstp dword[opr_param]

		fldz
		fadd st0,st1 ;0+t
		fmul st0,st0 ;t*t
		fimul word[coord_p2+2] ;...*y2

		fadd dword[opr_param]
		mov dword[opr_param],esi
		fadd dword[opr_param]
		fistp word[v_poi_0+2] ;y

		mov eax,1
		mov bx,word[v_poi_0+2]
		mov cx,word[v_poi_0]
		call draw_pixel

		fadd st0,st1 ;t+dt

		fld1
		fcomp
		fstsw ax
		sahf
	jae @b

	popad
	ret
endp

;фильтер
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
		lea   edx,[edx*3]
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
		call dither_2
		jmp .dither_end
	.error:
		stdcall print_err,sz_buf2d_filter_dither,txt_err_n24b
	.dither_end:
	popad
	ret
endp

align 16
dither_0: ; Sierra Filter Lite algoritm
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
dither_1: ; Floyd-Steinberg algoritm
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
dither_2: ; Burkers algoritm
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



;*** функции для работы с воксельной графикой ***



;создание воксельных кистей
align 4
proc vox_brush_create uses eax ebx ecx edi, h_br:dword, buf_z:dword
	mov edi,[h_br]
	movzx ecx,byte[edi+3]
	add edi,4

	; *** создание единичной кисти ***
	mov eax,[buf_z]
	mov buf2d_data,eax
	movzx eax,byte[edi-4] ;ширина единичной кисти
	mov buf2d_w,eax ;ширина буфера
	movzx eax,byte[edi-4+1] ;высота единичной кисти
	mov buf2d_h,eax ;высота буфера
	mov buf2d_size_lt,0 ;отступ слева и справа для буфера
	mov buf2d_color,0 ;цвет фона буфера
	mov buf2d_bits,32 ;количество бит в 1-й точке изображения

	; *** создание следующих кистей ***
	cmp ecx,1
	jl .end_creat
	movzx ebx,byte[edi-4+2] ;высота основания единичной кисти
	shr ebx,1
	cld
	@@:
		mov eax,edi
		add edi,BUF_STRUCT_SIZE
		stdcall vox_create_next_brush, eax, edi, ebx
		shl ebx,1
		loop @b
	.end_creat:
	ret
endp

;удаление воксельных кистей
align 4
proc vox_brush_delete uses ecx edi, h_br:dword
	mov edi,[h_br]
	movzx ecx,byte[edi+3]
	add edi,4

	; *** удаление кистей ***
	cmp ecx,1
	jl .end_delete
	cld
	@@:
		add edi,BUF_STRUCT_SIZE
		stdcall buf_delete, edi
		loop @b
	.end_delete:
	ret
endp

;функция для создания вокселя следующего порядка
; buf_v1 - буфер с исходным вокселем
; buf_v2 - буфер с увеличеным вокселем
; h - высота основания исходного вокселя : 2
align 4
proc vox_create_next_brush uses eax ebx ecx edx edi, buf_v1:dword, buf_v2:dword, h:dword
	mov edi,[buf_v1]
	mov ebx,buf2d_h
	mov ecx,buf2d_w
	mov edi,[buf_v2]
	mov buf2d_h,ebx
	shl buf2d_h,1
	mov buf2d_w,ecx
	shl buf2d_w,1
	mov buf2d_color,0
	mov buf2d_bits,32

	stdcall buf_create, [buf_v2] ;создание буфера глубины
	shr ecx,1
	mov edx,[h]
	shl edx,1
	sub ebx,edx
	;ecx - ширина исходного вокселя : 2
	;ebx - высота исходного вокселя (без основания)
	;edx - высота основания исходного вокселя
	mov eax,[h]
	cmp eax,0
	je @f
		stdcall vox_add, [buf_v2], [buf_v1], ecx,0,0
		stdcall vox_add, [buf_v2], [buf_v1], ecx,ebx,0

		stdcall vox_add, [buf_v2], [buf_v1], 0,eax,eax
		push eax ;stdcall ...
		add eax,ebx
		stdcall vox_add, [buf_v2], [buf_v1], 0,eax ;,...
		sub eax,ebx
		shl ecx,1

		;ecx - ширина исходного вокселя
		stdcall vox_add, [buf_v2], [buf_v1], ecx,eax,eax
		push eax ;stdcall ...,[h]
		add eax,ebx
		stdcall vox_add, [buf_v2], [buf_v1], ecx,eax;,[h]
		;sub eax,ebx
		shr ecx,1

		;ecx - ширина исходного вокселя : 2
		stdcall vox_add, [buf_v2], [buf_v1], ecx,edx,edx
		add ebx,edx
		stdcall vox_add, [buf_v2], [buf_v1], ecx,ebx,edx

		jmp .end_0
	@@:
		;если h = 0, тогда получаем кисть на 2 грани
		;в таком случае для получения глубины берем ширину / 2
		mov eax,ecx
		;2 левых вокселя
		stdcall vox_add, [buf_v2], [buf_v1], 0,0,eax
		stdcall vox_add, [buf_v2], [buf_v1], 0,ebx,eax
		shl eax,1
		;2 центральных передних вокселя (задние центральные не выводим)
		stdcall vox_add, [buf_v2], [buf_v1], ecx,0,eax
		stdcall vox_add, [buf_v2], [buf_v1], ecx,ebx,eax
		shr eax,1
		shl ecx,1
		;2 правых вокселя
		stdcall vox_add, [buf_v2], [buf_v1], ecx,0,eax
		stdcall vox_add, [buf_v2], [buf_v1], ecx,ebx,eax

	.end_0:


	ret
endp

;
align 4
proc vox_add, buf_v1:dword, buf_v2:dword, coord_x:dword, coord_y:dword, coord_z:dword
pushad
	mov ebx,[coord_x]
	mov eax,[coord_y]
	mov edi,[buf_v2]
	mov ecx,buf2d_h
	mov esi,buf2d_w
	imul ecx,esi
	add esi,ebx
	mov edx,buf2d_data
	cld
	;ecx - count pixels in voxel
	;edx - указатель на данные в воксельном буфере
	;edi - указатель на воксельный буфер
	;esi - width voxel buffer add coord x
	.cycle:
		cmp dword[edx],0
		je @f
			;проверяем буфер глубины
			push eax ecx edi esi
			mov ecx,eax
			mov edi,[buf_v1]
			call get_pixel_32 ;stdcall buf_get_pixel, [buf_v1],ebx,ecx
			mov esi,[edx]
			add esi,[coord_z]
			cmp eax,esi
			jge .end_draw
			stdcall buf_set_pixel, [buf_v1],ebx,ecx,esi ;esi = new coord z
			.end_draw:
			pop esi edi ecx eax
		@@:
		add edx,4
		inc ebx
		cmp ebx,esi
		jl @f
			inc eax
			sub ebx,buf2d_w
		@@:
		loop .cycle
popad
	ret
endp

;description:
; возврашает ширину воксельного изображения с 3-мя гранями
; принимает указатель на кисть и масштаб
align 4
proc buf_vox_obj_get_img_w_3g uses ecx, h_br:dword,k_scale:dword
	mov ecx,[h_br]

	movzx eax,byte[ecx]
	cmp dword[k_scale],1
	jl .end_c0
		mov ecx,[k_scale]
		shl eax,cl
	.end_c0:
	ret
endp

;description:
; возврашает высоту воксельного изображения с 3-мя гранями
; принимает указатель на кисть и масштаб
align 4
proc buf_vox_obj_get_img_h_3g uses ecx, h_br:dword,k_scale:dword
	mov ecx,[h_br]

	movzx eax,byte[ecx+1]
	cmp dword[k_scale],1
	jl .end_c0
		mov ecx,[k_scale]
		shl eax,cl
	.end_c0:
	ret
endp

;description:
; функция рисующая воксельный объект (видна 1 грань)
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
align 4
proc buf_vox_obj_draw_1g, buf_i:dword, buf_z:dword, v_obj:dword, coord_x:dword,\
coord_y:dword, k_scale:dword
	cmp [k_scale],0
	jl .end_f
pushad
	mov edi,[buf_i]
	cmp buf2d_bits,24
	jne .error1
	mov edi,[buf_z]
	cmp buf2d_bits,32
	jne .error2

	mov ecx,[k_scale]
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor esi,esi
	stdcall draw_sub_vox_obj_1g, [buf_i],[buf_z],[v_obj]

	jmp .end_0
	.error1:
		stdcall print_err,sz_buf2d_vox_obj_draw_1g,txt_err_n24b
		jmp .end_0
	.error2:
		stdcall print_err,sz_buf2d_vox_obj_draw_1g,txt_err_n32b
	.end_0:
popad
	.end_f:
	ret
endp

;input:
; ebx - coord_x
; edx - coord_y
; esi - coord_z
; ecx - уровень текушего узла
; edi - указатель на данные воксельного объекта
align 4
proc draw_sub_vox_obj_1g, buf_i:dword, buf_z:dword, v_obj:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;прорисовка рамки если размер узла = 1
		cmp ecx,0
		jne @f
			;проверка глубины esi
			stdcall buf_get_pixel, [buf_z], ebx,edx, esi
			cmp eax,esi
			jge @f
				push ecx
				mov ecx,dword[edi]
				and ecx,0xffffff
				stdcall buf_set_pixel, [buf_i], ebx,edx, ecx
				stdcall buf_set_pixel, [buf_z], ebx,edx, esi
				pop ecx
		@@:

		;рекурсивный перебор поддеревьев
		push edx
		;вход внутрь узла
		dec ecx

		mov eax,1
		cmp ecx,1
		jl @f
			shl eax,cl
		@@:

		add edx,eax ;коректировка высоты под воксель нижнего уровня

		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push eax ebx edx esi
				stdcall vox_corect_coords_pl, [v_obj],1
				stdcall draw_sub_vox_obj_1g, [buf_i],[buf_z],[v_obj]
				pop esi edx ebx eax
			.c_next:
			shr ah,1
			dec al
			jnz .cycle
		;выход из узла
		inc ecx
		pop edx
		jmp .end_f
	.sub_trees:
		cmp ecx,0
		jl .end_0 ;не рисуем очень маленькие воксели

			;рисуем узел
			mov eax,[edi]
			and eax,0xffffff
			
			cmp ecx,1
			jl @f
				;квадрат больше текущего масштаба
				stdcall vox_draw_square_1g, [buf_i],[buf_z],eax
				jmp .end_0
			@@:
				;квадрат текущего масштаба
				push ecx
				mov ecx,eax
				stdcall buf_get_pixel, [buf_z], ebx,edx
				cmp eax,esi
				jge .end_1
				stdcall buf_set_pixel, [buf_i], ebx,edx,ecx
				stdcall buf_set_pixel, [buf_z], ebx,edx,esi
				.end_1:
				pop ecx
		.end_0:
		add edi,4
	.end_f:
	ret
endp

;output:
; eax - разрушается
align 4
proc vox_draw_square_1g uses ecx edx edi, buf_i:dword, buf_z:dword, color:dword
locals
	img_size dd ?
	coord_y dd ?
endl
	mov edi,[buf_z]
	xor eax,eax
	inc eax
	shl eax,cl
	mov [img_size],eax
	mov [coord_y],eax
	.cycle_0:
	push ebx
	mov ecx,[img_size]
	cld
	.cycle_1:
		push ecx
		mov ecx,edx
		call get_pixel_32
		pop ecx
		cmp eax,esi
		jge @f
			stdcall buf_set_pixel, [buf_i], ebx,edx, [color]
			stdcall buf_set_pixel, edi, ebx,edx, esi
		@@:
		inc ebx
	loop .cycle_1
	pop ebx
	inc edx
	dec dword[coord_y]
	jnz .cycle_0
	ret
endp

;description:
; функция рисующая воксельный объект (видно 3 грани)
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; h_br - кисть с изображениями вокселей (32 бита)
; v_obj - воксельный объект
; k_scale - коэф. для масштабирования изображения
align 4
proc buf_vox_obj_draw_3g, buf_i:dword, buf_z:dword, h_br:dword, v_obj:dword,\
coord_x:dword, coord_y:dword, coord_z:dword, k_scale:dword
pushad
	mov edi,[v_obj]
	mov ecx,[k_scale]
	mov ebx,[coord_x]
	mov edx,[coord_y]
	add edi,vox_offs_data
	mov esi,[coord_z]
	stdcall vox_go_in_node, [buf_i], [buf_z], [h_br], [v_obj]
popad
	ret
endp

;description:
; функция рисующая часть воксельного объекта
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; h_br - кисть с изображениями вокселей (32 бита)
; v_obj - воксельный объект
; k_scale - коэф. для масштабирования изображения
align 4
proc buf_vox_obj_draw_3g_scaled, buf_i:dword, buf_z:dword, h_br:dword, v_obj:dword,\
coord_x:dword, coord_y:dword, coord_z:dword, k_scale:dword,\
s_c_x:dword, s_c_y:dword, s_c_z:dword, s_k_scale:dword,b_color:dword
pushad
locals
	p_node dd 0 ;родительский узел
endl
	mov edi,[v_obj]
	add edi,vox_offs_data

	mov ecx,[k_scale]
	mov ebx,[coord_x]

	;тестовая рамка
	mov eax,[h_br]

	movzx edx,byte[eax]
	movzx esi,byte[eax+1]
	cmp ecx,1
	jl .end_c0
		shl edx,cl
		shl esi,cl
	.end_c0:
	;stdcall buf_rect_by_size, [buf_i], ebx,[coord_y],edx,esi, [b_color]

	;вертикальная полоса
	add ebx,edx
	shr edx,cl
	stdcall buf_rect_by_size, [buf_i], ebx,[coord_y],edx,esi, [b_color]
	mov ecx,[s_k_scale]
	shr esi,cl
	xor eax,eax
	inc eax
	shl eax,cl
	dec eax
	sub eax,[s_c_z] ;значения по оси z возрастают с низу вверх
	imul eax,esi
	add eax,[coord_y]
	stdcall buf_filled_rect_by_size, [buf_i], ebx,eax,edx,esi, [b_color]
	mov ebx,[coord_y]
	shl esi,cl
	add ebx,esi
	stdcall buf_vox_obj_get_img_w_3g, [h_br],[k_scale]
	shr eax,1
	mov esi,[h_br]
	movzx esi,byte[esi+1]
	;ползунок
	stdcall draw_polz_hor, [buf_i], [coord_x],ebx,eax,esi, [s_c_x], [s_k_scale], [b_color]
	mov edx,[coord_x]
	add edx,eax
	;ползунок
	stdcall draw_polz_hor, [buf_i], edx,ebx,eax,esi, [s_c_y], [s_k_scale], [b_color]
;---

	mov esi,[s_k_scale]
	cmp esi,1
	jl .end_2

	; *** (1) ***
	.found:
	stdcall vox_obj_get_node_position, [v_obj],[s_c_x],[s_c_y],[s_c_z],esi
	movzx bx,byte[edi+3]
	mov [p_node],edi
	add edi,4
	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
			xor eax,eax
			stdcall vox_obj_rec0 ;в eax вычисляется число дочерних узлов, в данной ветви
		.end_0:
		shr bx,1
		loop @b
	.end_1:
	bt bx,0
	jnc .end_2 ;если поддерева не существует
	dec esi
	cmp esi,0
	jg .found

	; *** (2) ***
	;рисование части объекта
	mov ecx,[k_scale]
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov esi,[coord_z]
	stdcall vox_go_in_node, [buf_i], [buf_z], [h_br], [v_obj]
	.end_2:

popad
	ret
endp

;input:
; h_br - кисть с изображениями вокселей (32 бита)
; ebx - coord_x
; edx - coord_y
; esi - coord_z
; ecx - уровень текушего узла
; edi - указатель на данные воксельного объекта
align 4
proc vox_go_in_node, buf_i:dword, buf_z:dword, h_br:dword, v_obj:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees
		;рекурсивный перебор поддеревьев
		push eax edx

		;прорисовка рамки если размер узла = 1
		cmp ecx,0
		jne .end_2
			push eax
				stdcall vox_get_sub_brush,[h_br],0 ;определяем кисть для рисования
				cmp eax,0 ;если кисть не найдена
				je @f
					stdcall draw_vox, [buf_i], [buf_z], eax, ebx,edx,esi, [edi]
				@@:
			pop eax
		.end_2:

		;вход внутрь узла
		dec ecx
;---
		push ebx
			;mov eax,(h-h_osn/2)
			mov ebx,[h_br]
			movzx eax,byte[ebx+1]
			cmp byte[ebx+2],0
			je @f
				;если кисть с 3-мя гранями
				movzx ebx,byte[ebx+2]
				shr ebx,1
				sub eax,ebx
				jmp .end_0
			@@:
				;если кисть с 2-мя гранями
				movzx ebx,byte[ebx]
				shr ebx,1
			.end_0:
		cmp ecx,1
		jl @f
			shl eax,cl
			shl ebx,cl
		@@:
		add esi,ebx
		pop ebx
		add edx,eax ;коректировка высоты под воксель нижнего уровня
;---
		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push ebx edx esi
				stdcall vox_corect_coords, [h_br], [v_obj]
				stdcall vox_go_in_node, [buf_i], [buf_z], [h_br], [v_obj]
				pop esi edx ebx
			.c_next:
			shr ah,1
			dec al
			jnz .cycle

		;выход из узла
		inc ecx
		pop edx eax

		jmp .end_f
	.sub_trees:
		;рисуем узел
		push eax
			stdcall vox_get_sub_brush,[h_br],ecx ;определяем кисть для рисования
			cmp eax,0 ;если кисть не найдена
			je @f
				stdcall draw_vox, [buf_i], [buf_z], eax, ebx,edx,esi, [edi]
			@@:
		pop eax

		add edi,4
	.end_f:
	ret
endp

;description:
; функция рисующая одиночный воксел
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; buf_v - буфер с изображением вокселя (32 бита)
; v_color - цвет
align 4
proc draw_vox, buf_i:dword, buf_z:dword, buf_v:dword,\
coord_x:dword, coord_y:dword, coord_z:dword, v_color:dword
pushad
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[buf_v]
	mov ecx,buf2d_h
	mov esi,buf2d_w
	imul ecx,esi
	add esi,eax
	mov edx,buf2d_data
	cld
	;ecx - count pixels in voxel
	;edx - указатель на данные в воксельном буфере
	;edi - указатель на воксельный буфер
	;esi - width voxel buffer add coord x
	.cycle:
		cmp dword[edx],0
		je @f
			;проверяем буфер глубины
			push eax
			stdcall buf_get_pixel, [buf_z],eax,ebx
			sub eax,[coord_z]
			cmp eax,[edx]
			jl .dr_pixel
				pop eax
				jmp @f
			.dr_pixel:
				;рисуем точку
				pop eax
				stdcall buf_set_pixel, [buf_i],eax,ebx,[v_color]
				push ecx
				mov ecx,[coord_z]
				add ecx,[edx]
				stdcall buf_set_pixel, [buf_z],eax,ebx,ecx
				pop ecx
		@@:
		add edx,4
		inc eax
		cmp eax,esi
		jl @f
			inc ebx
			sub eax,buf2d_w
		@@:
		loop .cycle
popad
	ret
endp

;description:
;функция для коректировки координат
;направления осей координат в вокселе:
;*z
;|
;+
;  * y
; /
;+
; \
;  * x
;input:
;  al - номер узла в дереве (от 1 до 8)
; ebx - координата x
; edx - координата y
; esi - координата z
; ecx - уровень текушего узла
;output:
; ebx - новая координата x
; edx - новая координата y
; esi - новая координата z
align 4
proc vox_corect_coords, h_br:dword, v_obj:dword
locals
	osn_w_2 dd ? ;ширина основания единичного вокселя : 2
	vox_h dd ? ;высота единичного вокселя
endl
	cmp ecx,0
	jl .end_f ;для ускорения отрисовки

	push eax edi
	and eax,15 ;выделяем номер узла в дереве
	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	add edi,8
	sub edi,eax

	push ebx ecx
		mov ebx,[h_br]

		movzx ecx,byte[ebx]
		shr ecx,1
		mov dword[osn_w_2],ecx

		movzx ecx,byte[ebx+2]
		movzx ebx,byte[ebx+1]
		sub ebx,ecx
		mov dword[vox_h],ebx
		shr ecx,1
		mov eax,ecx ;eax - высота основания единичного вокселя : 2
	pop ecx ebx

	cmp ecx,1
	jl @f ;во избежание зацикливания
		shl eax,cl
		shl dword[osn_w_2],cl
		shl dword[vox_h],cl
	@@:

;	add esi,eax ;меняем глубину для буфера z (компенсация для координаты y)
	bt word[edi],0 ;test voxel coord x
	jnc @f
		add ebx,[osn_w_2]
		cmp eax,0
		jne .end_0
			add esi,[osn_w_2] ;меняем глубину для буфера z
			jmp @f
		.end_0:
		add edx,eax
		add esi,eax ;меняем глубину для буфера z
	@@:
	bt word[edi],1 ;test voxel coord y
	jnc @f
		add ebx,[osn_w_2]
		cmp eax,0
		jne .end_1
			sub esi,[osn_w_2] ;меняем глубину для буфера z
			jmp @f
		.end_1:
		sub edx,eax
		sub esi,eax ;меняем глубину для буфера z
	@@:
	bt word[edi],2 ;test voxel coord z
	jnc @f
		sub edx,[vox_h]
	@@:
	pop edi eax
	.end_f:
	ret
endp

;извлекаем из h_br указатель на буфер с изображением вокселя, указанного порядка n
align 4
proc vox_get_sub_brush uses ebx ecx, h_br:dword, n:dword
	xor eax,eax
	mov ebx,[n]
	cmp ebx,0
	jl @f
	mov ecx,[h_br]
	cmp bl,byte[ecx+3]
	jg @f
		add ecx,4
		imul ebx,BUF_STRUCT_SIZE
		mov eax,ebx
		add eax,ecx
	@@:
	ret
endp

;description:
; функция рисующая срез воксельного обьекта
;input:
; v_size - размер квадрата с вокселем
; k_scale - степень детализации изображения
; n_plane - номер плоскости сечния (в пределах от 0 до 2^k_scale - 1)
; b_color - цвет границы
align 4
proc buf_vox_obj_draw_pl, buf_i:dword, v_obj:dword, coord_x:dword,\
coord_y:dword, v_size:dword, k_scale:dword, n_plane:dword, b_color:dword
	cmp [k_scale],0
	jl .end_f
pushad
	mov eax,[v_size]
	mov ecx,[k_scale]
	mov ebx,eax
	cmp ecx,1
	jl @f
		shl ebx,cl
	@@:
	;ebx - полный размер изображения
	stdcall buf_rect_by_size, [buf_i], [coord_x],[coord_y],ebx,ebx, [b_color] ;рамка на рисунок
	mov edx,ebx
	add ebx,[coord_y]
	stdcall draw_polz_hor, [buf_i], [coord_x],ebx,edx,eax, [n_plane], [k_scale], [b_color] ;ползунок, показывающий номер сечения

	;рисование точек для сетки
	push ecx
	mov edi,1
	cmp ecx,1
	jl @f
		shl edi,cl
	@@:
	dec edi
	cmp edi,1
	jl .end_0
	mov ecx,edi
	imul ecx,edi
	mov ebx,[coord_x]
	mov edx,[coord_y]
	add edx,eax
	xor esi,esi
	cld
	@@:
		add ebx,eax
		inc esi
		stdcall buf_set_pixel, [buf_i], ebx,edx, [b_color]
		cmp esi,edi
		jl .end_1
			;переход точек на новую строку
			xor esi,esi
			mov ebx,[coord_x]
			add edx,eax
		.end_1:
		loop @b
	.end_0:
	pop ecx

	;eax - размер одного квадрата
	;edi - указатель на рисуемые данные из объекта
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor esi,esi
	push eax
	mov eax,1
	shl eax,cl
	dec eax
	sub eax,[n_plane]
	stdcall draw_sub_vox_obj_pl, [buf_i],[v_obj],eax
popad
	.end_f:
	ret
endp

;description:
; функция рисующая срез части воксельного обьекта
;input:
; s_c_x, s_c_y, s_c_z, s_k_scale - параметры определяющие часть воксельного объекта, которая будет рисоваться
align 4
proc buf_vox_obj_draw_pl_scaled, buf_i:dword, v_obj:dword, coord_x:dword,\
coord_y:dword, v_size:dword, k_scale:dword, n_plane:dword, b_color:dword,\
s_c_x:dword, s_c_y:dword, s_c_z:dword, s_k_scale:dword
	cmp [k_scale],0
	jl .end_f
pushad
locals
	p_node dd 0 ;родительский узел
endl
	mov eax,[v_size]
	mov ecx,[k_scale]
	mov ebx,eax
	cmp ecx,1
	jl @f
		shl ebx,cl
	@@:
	;ebx - полный размер изображения
	stdcall buf_rect_by_size, [buf_i], [coord_x],[coord_y],ebx,ebx, [b_color] ;рамка на рисунок
	mov edx,ebx
	add ebx,[coord_y]
	stdcall draw_polz_hor, [buf_i], [coord_x],ebx,edx,eax, [n_plane], [k_scale], [b_color] ;ползунок, показывающий номер сечения

	;рисование точек для сетки
	push ecx
	mov edi,1
	cmp ecx,1
	jl @f
		shl edi,cl
	@@:
	dec edi
	cmp edi,1
	jl .end_3
	mov ecx,edi
	imul ecx,edi
	mov ebx,[coord_x]
	mov edx,[coord_y]
	add edx,eax
	xor esi,esi
	cld
	@@:
		add ebx,eax
		inc esi
		stdcall buf_set_pixel, [buf_i], ebx,edx, [b_color]
		cmp esi,edi
		jl .end_4
			;переход точек на новую строку
			xor esi,esi
			mov ebx,[coord_x]
			add edx,eax
		.end_4:
		loop @b
	.end_3:
	pop ecx

	mov esi,[s_k_scale]
	cmp esi,1
	jl .end_2
	mov edi,[v_obj]
	add edi,vox_offs_data

	; *** (1) ***
	.found:
	stdcall vox_obj_get_node_position, [v_obj],[s_c_x],[s_c_y],[s_c_z],esi
	movzx bx,byte[edi+3]
	mov [p_node],edi
	add edi,4
	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
			xor eax,eax
			stdcall vox_obj_rec0 ;в eax вычисляется число дочерних узлов, в данной ветви
		.end_0:
		shr bx,1
		loop @b
	.end_1:
	bt bx,0
	jnc .end_2 ;если поддерева не существует
	dec esi
	cmp esi,0
	jg .found

	mov eax,[v_size]
	;eax - размер одного квадрата
	;edi - указатель на рисуемые данные из объекта
	mov ecx,[k_scale]
	mov ebx,[coord_x]
	mov edx,[coord_y]
	xor esi,esi
	push eax
	mov eax,1
	shl eax,cl
	dec eax
	sub eax,[n_plane]
	stdcall draw_sub_vox_obj_pl, [buf_i],[v_obj], eax

	.end_2:
popad
	.end_f:
	ret
endp

;description:
; определение позиции узла в дереве (от 0 до 7)
align 4
proc vox_obj_get_node_position uses ebx ecx edi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov ecx,[k_scale]
	dec ecx
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[coord_z]
	cmp ecx,1
	jl .end_0
		shr eax,cl
		shr ebx,cl
		shr edi,cl
	.end_0:
	and eax,1
	bt ebx,0
	jnc @f
		bts eax,1
	@@:
	bt edi,0
	jnc @f
		bts eax,2
	@@:

	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	@@:
		cmp al,byte[edi]
		je @f
		inc edi
		jmp @b
	@@:
	sub edi,[v_obj]
	sub edi,vox_offs_tree_table
	mov eax,edi
	
	ret
endp

;input:
; edi - указатель на данные воксельного объекта
;output:
; eax - eax + число узлов в данных вокс. объекта
; edi - указатель на смещенные данные вокс. объекта
align 4
proc vox_obj_rec0
	inc eax
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;рекурсивный перебор поддеревьев
		push ebx ecx
		mov bh,byte[edi+3]
		add edi,4
		mov bl,8
		.cycle:
			bt bx,8 ;тестируем только bh
			jnc .c_next
				stdcall vox_obj_rec0
			.c_next:
			shr bh,1
			dec bl
			jnz .cycle
		pop ecx ebx

		jmp .end_f
	.sub_trees:
		add edi,4
	.end_f:
	ret
endp

;description:
; функция рисующая горизонтальную полосу с ползунком
align 4
proc draw_polz_hor uses eax ebx ecx, buf:dword, coord_x:dword, coord_y:dword,\
size_x:dword, size_y:dword, pos:dword, k_scale:dword, color:dword
	mov ebx,[size_x]
	stdcall buf_rect_by_size, [buf], [coord_x],[coord_y],ebx,[size_y], [color]
	mov ecx,[k_scale]
	shr ebx,cl
	mov eax,[pos]
	imul eax,ebx
	add eax,[coord_x]
	stdcall buf_filled_rect_by_size, [buf], eax,[coord_y],ebx,[size_y], [color]
	ret
endp

;input:
; ebx - coord_x
; edx - coord_y
; esi - coord_z
; ecx - уровень текушего узла
; edi - указатель на данные воксельного объекта
align 4
proc draw_sub_vox_obj_pl, buf_i:dword, v_obj:dword, clip_z:dword,\
v_size:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;прорисовка рамки если размер узла = 1
		cmp ecx,0
		jne @f
			;проверка глубины esi
			;clip_z=n_plane
			stdcall vox_is_clip, [clip_z];,[v_size]
			cmp eax,0
			je @f
				push ecx
				mov ecx,dword[edi]
				and ecx,0xffffff
				stdcall buf_rect_by_size, [buf_i], ebx,edx, [v_size],[v_size],ecx
				pop ecx
		@@:

		;рекурсивный перебор поддеревьев
		push edx
		;вход внутрь узла
		dec ecx

		mov eax,[v_size]
		cmp ecx,1
		jl @f
			shl eax,cl
		@@:

		add edx,eax ;коректировка высоты под воксель нижнего уровня

		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push eax ebx edx esi
				stdcall vox_corect_coords_pl, [v_obj],[v_size]
				stdcall draw_sub_vox_obj_pl, [buf_i],[v_obj],[clip_z],[v_size]
				pop esi edx ebx eax
			.c_next:
			shr ah,1
			dec al
			jnz .cycle
		;выход из узла
		inc ecx
		pop edx
		jmp .end_f
	.sub_trees:
		cmp ecx,0
		jl .end_0 ;не рисуем очень маленькие воксели

			;проверка глубины esi
			;clip_z=n_plane
			stdcall vox_is_clip, [clip_z]
			cmp eax,0
			je .end_0

			;рисуем узел
			mov eax,[edi]
			and eax,0xffffff
			push eax ;цвет узла

			mov eax,[v_size]
			cmp ecx,1
			jl @f
				;квадрат больше текущего масштаба
				shl eax,cl ;размер узла
				stdcall buf_filled_rect_by_size, [buf_i], ebx,edx, eax,eax
				push ebx edx esi
				mov esi,eax
				inc ebx
				inc edx
				sub esi,2
				mov eax,[buf_i]
				push dword 128
				push dword[eax+16] ;+16 - b_color
				stdcall combine_colors_3,[edi]
				stdcall buf_rect_by_size, [buf_i], ebx,edx, esi,esi,eax
				pop esi edx ebx
				jmp .end_0
			@@:
				;квадрат текущего масштаба
				stdcall buf_filled_rect_by_size, [buf_i], ebx,edx, eax,eax
		.end_0:
		add edi,4
	.end_f:
	ret
endp

;description:
; вспомогательная функция для проверки глубины esi
;input:
; ecx - уровень текушего узла
; esi - coord z
; clip_z - n_plane
;output:
; eax - 0 if no draw, 1 if draw
align 4
proc vox_is_clip uses ebx edi, clip_z:dword
	xor eax,eax
	mov ebx,[clip_z]
	mov edi,1
	cmp ecx,1
	jl @f
		shl edi,cl
	@@:
	;edi = 2^ecx
	add edi,esi
	cmp edi,ebx ;if (esi+2^ecx <= n_plane) no draw
	jle @f
	inc ebx
	cmp esi,ebx ;if (esi >= (n_plane+1)) no draw
	jge @f
		inc eax
	@@:
	ret
endp

;функция для коректировки координат
;направления осей координат в вокселе:
;*z
;|
;+-* x
;input:
;  al - номер узла в дереве (от 1 до 8)
; ebx - координата x
; edx - координата y
; esi - координата z
; ecx - уровень текушего узла
;output:
; ebx - новая координата x
; edx - новая координата y
; esi - новая координата z
align 4
proc vox_corect_coords_pl, v_obj:dword, v_size:dword
	cmp ecx,0
	jl .end_f ;для ускорения отрисовки

	push eax edi
	and eax,15 ;выделяем номер узла в дереве
	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	add edi,8
	sub edi,eax

	mov eax,[v_size]
	cmp ecx,1
	jl @f
		shl eax,cl
	@@:

	bt word[edi],0 ;test voxel coord x
	jnc @f
		add ebx,eax
	@@:
	bt word[edi],2 ;test voxel coord z
	jnc @f
		sub edx,eax
	@@:
	bt word[edi],1 ;test voxel coord y
	jc @f
		mov eax,1
		cmp ecx,1
		jl .end_0
			shl eax,cl
		.end_0:
		add esi,eax ;меняем глубину для буфера z
	@@:
	pop edi eax
	.end_f:
	ret
endp

;description:
; функция рисующая тени
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; h_br - кисть с изображениями вокселей (32 бита)
; k_scale - коэф. для масштабирования изображения
align 4
proc buf_vox_obj_draw_3g_shadows, buf_i:dword, buf_z:dword, h_br:dword, \
coord_x:dword, coord_y:dword, color:dword, k_scale:dword, prop:dword
locals
	correct_z dd 0 ;коректировка для буфера глубины
endl
pushad
	mov eax,[k_scale]
	add eax,[prop]
	mov dword[correct_z],8
	sub [correct_z],eax
	mov ebx,[coord_x]
	;correct_z = 8-k_scale-prop

	stdcall buf_vox_obj_get_img_w_3g, [h_br],[k_scale]
	mov edx,eax ;edx - ширина изображения
	stdcall buf_vox_obj_get_img_h_3g, [h_br],[k_scale]
	mov esi,eax

	mov edi,[coord_y]
	mov ecx,edx
	add edx,ebx ;ширина + отступ слева
	imul ecx,esi
	cld
	.cycle_0:
		stdcall buf_get_pixel, [buf_z],ebx,edi
		cmp eax,0
		je @f
			stdcall vox_correct_z, [correct_z]
			push eax
			stdcall buf_get_pixel, [buf_i],ebx,edi
			stdcall combine_colors_3,eax,[color] ;,eax
			stdcall buf_set_pixel, [buf_i],ebx,edi,eax
		@@:
		inc ebx
		cmp ebx,edx
		jl @f
			mov ebx,[coord_x]
			inc edi
		@@:
		loop .cycle_0

popad
	ret
endp

;output:
; eax - scaled coord z
align 4
proc vox_correct_z uses ecx, correct_z:dword
	mov ecx,[correct_z]
	cmp ecx,0
	je .end_f
	jl .end_0
		shl eax,cl
		jmp .end_f
	.end_0:
		neg ecx
		inc ecx
		shr eax,cl
	.end_f:
	ret
endp

;output:
; eax - color
align 4
proc combine_colors_3 uses ebx ecx edx edi esi, col_0:dword, col_1:dword, alpha:dword

	mov ebx,[col_0]
	mov ecx,[col_1]
	movzx di,byte[alpha] ;pro
	mov si,0x00ff ;---get transparent---
	sub si,di ;256-pro

	;---blye---
	movzx ax,bl
	imul ax,si
	movzx dx,cl
	imul dx,di
	add ax,dx
	mov cl,ah
	;---green---
	movzx ax,bh
	imul ax,si
	movzx dx,ch
	imul dx,di
	add ax,dx
	mov ch,ah
	shr ebx,16
	ror ecx,16
	;---red---
	movzx ax,bl
	imul ax,si
	movzx dx,cl
	imul dx,di
	add ax,dx

	shl eax,8
	ror ecx,16
	mov ax,cx
	and eax,0xffffff

	ret
endp

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
