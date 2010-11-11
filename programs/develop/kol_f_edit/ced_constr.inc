;
; в этом файле собраны функции рисующие окно конструктора
;

macro move_to_skin coord_x,coord_y
{
	add coord_x,SKIN_W1
	add coord_y,SKIN_H
}

;перерисовка окна конструктора
align 4
draw_constructor:
	stdcall [buf2d_clear], buf_fon,0xffffff

pushad
	cmp [err_ini1],1
	jne @f
		stdcall [buf2d_draw_text], buf_fon, buf_font,txtErrIni1,10,10,\
			0xff0000 ;рисуем строку с текстом
		stdcall [buf2d_draw_text], buf_fon, buf_font,fp_obj_opt,10,20,\
			0xff0000 ;рисуем строку с текстом
		jmp .end_draw
	@@:

	xor edx,edx
	mov dl,C_WI
	stdcall find_obj_in_file,edx ;eax - указатель на структуру окна
	cmp eax,0
	je .end_draw
	mov dword[obj_m_win],eax

	;4-width
	stdcall get_obj_text_col,[obj_m_win],4 ;edx = offset
	stdcall conv_str_to_int,edx
	mov ecx,eax ;ecx=width

	;5-height
	stdcall get_obj_text_col,[obj_m_win],5 ;edx = offset
	stdcall conv_str_to_int,edx
	;eax=height

	stdcall [buf2d_rect_by_size], buf_fon, 0,0,ecx,eax, dword[sc.frame] ;рисуем внешнюю рамку окна
	sub eax,2
	sub ecx,2
	stdcall [buf2d_filled_rect_by_size], buf_fon, 1,1,ecx,eax, dword[sc.grab] ;рисуем фон рамки окна

	sub eax,6
	sub ecx,6
	stdcall [buf2d_rect_by_size], buf_fon, 4,4,ecx,eax, dword[sc.frame] ;рисуем внутреннюю рамку окна
	sub eax,2
	sub ecx,2
	stdcall [buf2d_filled_rect_by_size], buf_fon, 5,5,ecx,eax, dword[sc.work] ;рисуем фон окна

;---;---;---
;jmp .end_skin
;---;---;---

	stdcall get_obj_text_col,[obj_m_win],1
	stdcall conv_str_to_int,edx ;eax = window style
	cmp eax,3
	jl .end_skin
		stdcall [buf2d_bit_blt], buf_fon, 0,0, buf_skin1 ;левый скин
		mov edi,buf_skin1
		mov eax,buf2d_w ;eax - позиция для рисования среднего скина
		mov edi,buf_skin2
		mov ebx,buf2d_w ;ebx - ширина левого скина
		sub ecx,ebx ;отнимаем от ширины окна ширину среднего скина
		@@:
			stdcall [buf2d_bit_blt], buf_fon, eax,0, buf_skin2 ;прорисовка среднего скина
			add eax,ebx
			cmp eax,ecx ;сравниваем позицию среднего скина с шириной окна
			jl @b

		;4-width
		stdcall get_obj_text_col,[obj_m_win],4 ;edx = offset
		stdcall conv_str_to_int,edx
		mov edi,buf_skin3
		sub eax,buf2d_w ;отнимаем ширину правого скина
		stdcall [buf2d_bit_blt], buf_fon, eax,0, buf_skin3 ;прорисовка правого скина
	.end_skin:

	xor edx,edx ;рисуем графические элементы
	mov dl,C_BU
	stdcall draw_arr_objects, edx,draw_obj_but
	mov dl,C_ED
	stdcall draw_arr_objects, edx,draw_obj_edit
	mov dl,C_CB
	stdcall draw_arr_objects, edx,draw_obj_check
	mov dl,C_DT
	stdcall draw_arr_objects, edx,draw_obj_text
	mov dl,C_RE
	stdcall draw_arr_objects, edx,draw_obj_rect_f13

	.end_draw:
popad
	ret

;функция рисует массивы определенных элементов
align 4
proc draw_arr_objects, obj_t:dword,p_fun_draw:dword
push eax ebx
	stdcall find_obj_in_file,[obj_t] ;eax - указатель на структуру объекта
	cmp eax,0
	je .end_draw_o
		mov ebx,[p_fun_draw]
		stdcall ebx,eax
		@@:
			stdcall find_obj_in_file_next,[obj_t] ;eax - указатель на структуру объекта
			cmp eax,0
			je .end_draw_o
				stdcall ebx,eax
		jmp @b
	.end_draw_o:
pop ebx eax
	ret
endp

;функция рисует объект кнопки, в окне конструктора
align 4
proc draw_obj_but, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3,4,5 - x,y,w,h,color

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov edi,eax
		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		mov ecx,eax
		stdcall get_obj_text_col,ebx,5
		stdcall conv_str_to_int,edx
		stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],edi,ecx,eax
	popad
	ret
endp

;функция рисует объект текстового поля, в окне конструктора
align 4
proc draw_obj_edit, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	col_txt dd ? ;цвет тектса
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt

		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov ecx,eax
		stdcall get_obj_text_col,ebx,6
		stdcall conv_str_to_int,edx
		stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			ecx,15, eax

		inc dword[coor_x]
		inc dword[coor_y]
		sub ecx,2
		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			ecx,13, eax

		stdcall get_obj_text_col,ebx,8
		stdcall conv_str_to_int,edx
		mov [col_txt],eax
		add [coor_x],2-1 ;смещаем координату, что-бы
		add [coor_y],4-1 ;текст был в поле выровнен
		stdcall get_obj_text_col,ebx,10
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,dword[coor_x],dword[coor_y],\
			dword[col_txt] ;рисуем строку с текстом

	popad
	ret
endp

;функция рисует объект флажка, в окне конструктора
align 4
proc draw_obj_check, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	ot_txt dd ? ;отступ для текста
	size_wh dd ? ;размер квадратика
	col_txt dd ? ;цвет тектса
endl
	pushad
		mov ebx,dword[p_obj]
		;1,2,3, 4,5,6, 7,8 - x,y,txt, col_fon,col_frame,col_txt, w_txt,w_h

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]
		stdcall get_obj_text_col,ebx,7
		stdcall conv_str_to_int,edx
		mov [ot_txt],eax
		stdcall get_obj_text_col,ebx,8
		stdcall conv_str_to_int,edx
		dec eax ;размер меньше на 1 пиксель
		mov [size_wh],eax
		stdcall get_obj_text_col,ebx,5
		stdcall conv_str_to_int,edx
		stdcall [buf2d_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			dword[size_wh],dword[size_wh], eax

		inc dword[coor_x]
		inc dword[coor_y]
		sub dword[size_wh],2
		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		stdcall [buf2d_filled_rect_by_size], buf_fon, dword[coor_x],dword[coor_y],\
			dword[size_wh],dword[size_wh], eax

		stdcall get_obj_text_col,ebx,6
		stdcall conv_str_to_int,edx
		mov [col_txt],eax
		mov eax,[ot_txt]
		add eax,[size_wh]
		add eax,2-1 ;sub dword[size_wh],2 ;inc dword[coor_x]
		add [coor_x],eax ;вычисляем координаты для текстовой подписи
		add [coor_y],5-1 ;inc dword[coor_y]
		stdcall get_obj_text_col,ebx,3
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,dword[coor_x],dword[coor_y],\
			dword[col_txt] ;рисуем строку с текстом
	popad
	ret
endp

;функция рисует объект текстовой подписи, в окне конструктора
align 4
proc draw_obj_text, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	col_txt dd ? ;цвет тектса
endl
	pushad
		mov ebx,dword[p_obj]
		;0,1,2,3,... - txt,x,y,col_txt,...

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]

		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov [col_txt],eax

		stdcall get_obj_text_col,ebx,0
		stdcall [buf2d_draw_text], buf_fon, buf_font,edx,dword[coor_x],dword[coor_y],\
			dword[col_txt] ;рисуем строку с текстом
	popad
	ret
endp

;функция рисует объект прямоугольника (сисит. ф. 13)
align 4
proc draw_obj_rect_f13, p_obj:dword
locals
	coor_x dd ?
	coor_y dd ?
	color dd ? ;цвет тектса
endl
	pushad
		mov ebx,dword[p_obj]
		;0,1,2,3,4 - color,x,y,w,h

		stdcall get_obj_text_col,ebx,1
		stdcall conv_str_to_int,edx
		mov [coor_x],eax
		stdcall get_obj_text_col,ebx,2
		stdcall conv_str_to_int,edx
		mov [coor_y],eax
		move_to_skin [coor_x],[coor_y]

		stdcall get_obj_text_col,ebx,0
		stdcall conv_str_to_int,edx
		mov [color],eax

		stdcall get_obj_text_col,ebx,3
		stdcall conv_str_to_int,edx
		mov ecx,eax

		stdcall get_obj_text_col,ebx,4
		stdcall conv_str_to_int,edx
		mov esi,eax

		stdcall [buf2d_filled_rect_by_size], buf_fon,\
			dword[coor_x],dword[coor_y],ecx,esi,dword[color]
	popad
	ret
endp
