;
;*** функции рисования в буфере ***
;

align 16
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
txt_err_size_w16 db 'image width < 16 pixels',13,10,0

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
align 16
proc draw_pixel_transp, t_prop:dword, m_prop:dword
locals
	transp_32 dd 0 ;цвет рисуемой точки + прозрачность
endl
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
		mov [transp_32],edx ;цвет рисуемой точки

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

		mov esi,ebp
		sub esi,4 ;указатель на transp_32
		mov byte[esi+3],al ;прозрачность рисуемой точки

		call combine_colors_0
	pop esi edi edx ebx eax
	@@:
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
	jne .error0
		mov ecx,buf2d_h
		mov ebx,ecx
		shr ebx,4 ;предполагаем что в буфере 16 строк с символами, потому делим на 2^4
		or ebx,ebx
		jz .error1
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
	.error0:
		stdcall print_err,sz_buf2d_convert_text_matrix,txt_err_n8b
		jmp .end_conv
	.error1:
		stdcall print_err,sz_buf2d_convert_text_matrix,txt_err_size_w16
	.end_conv:
	popad
	ret
endp

align 4
proc buf_draw_text, buf_struc:dword, buf_t_matr:dword, text:dword, coord_x:dword, coord_y:dword, color:dword
	locals
		buf_t_matr_offs dd ?
		buf_s_matr buf_2d_header ? ;ebp-BUF_STRUCT_SIZE локальная матрица символа
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
		mov edi,ebp
		sub edi,BUF_STRUCT_SIZE ;&buf_s_matr
		cld
		rep movsb
		sub edi,BUF_STRUCT_SIZE ;&buf_s_matr
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

;input:
; ebp+8  = p0
; ebp+12 = p1
align 4
line_len4i:
	push ebp
	mov ebp,esp
		fild word [ebp+8]
		fisub word [ebp+12]
		fmul st0,st0 ;st0=x^2
		fild word [ebp+10]
		fisub word [ebp+14]
		fmul st0,st0 ;st0=y^2
		faddp
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
	fldz

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

	ffree st0 ;1.0
	fincstp

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

		fld st0 ;st0=t
		fmul st0,st0 ;t^2
		fmul st0,st5 ;(t^2)*x2

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

		fld st0 ;st0=t
		fmul st0,st0 ;t^2
		fimul word[coord_p2+2] ;(t^2)*y2

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
