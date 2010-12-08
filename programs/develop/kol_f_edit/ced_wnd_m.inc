
;поднимаем объект вверх по списку
align 4
but_obj_move_up:
	stdcall [tl_node_move_up], tree2
	ret

;опускаем объект вниз по списку
align 4
but_obj_move_down:
	stdcall [tl_node_move_down], tree2
	ret

;копируем объект
align 4
but_obj_copy:
push ecx esi edi
	stdcall [tl_node_get_data], tree2
	pop esi
	mov edi,[tree2.data_info] ;данные 0-го узла, используем для копирования
	xor ecx,ecx
	mov cx,word[tree2.info_size]
	cld
	rep movsb
pop edi esi ecx
	ret

;вставляем объект
align 4
but_obj_paste:
push eax ebx ecx edi
		mov eax,[tree2.data_info] ;данные 0-го узла, используем для копирования
		mov bl,byte[eax]
		call find_obj_in_opt
		xor ecx,ecx
		mov cx,word[edi+obj_opt.img-obj_opt] ;cx - индекс главной иконки добавляемого объекта
		shl ecx,16
		stdcall [tl_node_add], eax, ecx, tree2 ;добавляем объект
		stdcall [tl_draw], tree2
pop edi ecx ebx eax
	ret

;обновление объектов в tree2, по значениям текстовых полей
;функция обратная к данной on_file_object_select
align 4
but_element_change:
	;foc_obj - указатель на последний выбранный узел в tree2
	cmp dword[foc_obj],0
	je .err_obj
		pushad
		;установка текстовых свойств
		xor ebx,ebx
		mov edi,edit2
		@@:
			call get_obj_win_param
			inc ebx
			add edi,ed_struc_size
			cmp edi,prop_wnd_edits_end
		jl @b

		;установка ссылки на объект типа
		mov edx,dword[foc_obj]
		mov dword[edx+u_object.typid-u_object],0 ;на случай если ничего не выбрано
		stdcall [tl_node_get_data],tree3
		pop eax
		cmp eax,0
		je @f
			mov eax,dword[eax] ;берем порядковый номер (в памяти) выбранного объекта
			mov dword[edx+u_object.typid-u_object],eax
		@@:

		;установка логических свойств
		mov edi,ch1.flags
		mov eax,1
		xor ebx,ebx
		@@:
			bt dword[edi],1 ;почему оно тут не работает я не знаю ... ???
			jnc .opt_no_sel
				or ebx,eax ;накопление отмеченных опций
			.opt_no_sel:
			shl eax,1
			add edi,ch_struc_size
			cmp edi,check_boxes_end
		jl @b
		mov dword[edx+u_object.modif-u_object],ebx
		popad
	.err_obj:
	call draw_constructor ;обновляем окно конструктора
	;call draw_window
	ret

;input:
; ebx - property column
; edi - pointer to property editbox
align 4
get_obj_win_param:
	pushad
		mov esi,ed_text
		stdcall get_obj_text_col, [foc_obj], ebx ;edx - смещение для текста
		stdcall get_obj_text_len_col, [foc_obj], ebx ;eax - длинна поля
		cld
		mov edi,edx
		mov ecx,eax
		rep movsb
	popad
	ret

;input:
;  obj - pointer to object
;  ind - column index
;output:
;  edx + offset
;  edx = 0 if not text
align 4
proc get_obj_text_col, obj:dword, ind:dword
	mov edx,dword[obj]
	inc edx ;goto text
push ebx
	mov ebx,dword[ind]
	cmp bx,0
	je .end_fun

	shl ebx,8 ;в bh попадает номер искомой колонки
	mov bl,byte[edx-1] ;в bl попадает код объекта

	push ecx edi
	call find_obj_in_opt ;ищем структуру с опциями объекта

	add edi,obj_opt.Col-obj_opt
	dec bh
	and ebx,0xff00
	shr ebx,7 ;сдвигаем на 7, т.к. нужно значение bh умноженное на 2
	add edi,ebx ;+ бывшее bh*2
	xor ecx,ecx
	mov cx,word[edi] ;ecx - сдиг для искомого поля
	add edx,ecx

	cmp cx,0
	jg @f
		mov edx,0 ;not text
	@@:
	cmp cx,MAX_LEN_OBJ_TXT
	jl @f
		mov edx,0 ;not text
	@@:

	pop edi ecx
	.end_fun:
pop ebx
	ret
endp

;input:
;  obj - указатель на объект
;  ind - номер поля внутри объекта
;output:
;  eax - длинна текстового поля внутри объекта
align 4
proc get_obj_text_len_col, obj:dword, ind:dword
	xor eax,eax
push ebx
	mov ebx,dword[ind]
	shl ebx,8 ;в bh попадает номер искомой колонки
	mov eax,dword[obj]
	mov bl,byte[eax] ;в bl попадает код объекта
	xor eax,eax

	push edi
	call find_obj_in_opt ;ищем структуру с опциями объекта

	add edi,obj_opt.Col-obj_opt
	and ebx,0xff00
	shr ebx,7 ;сдвигаем на 7, т.к. нужно значение bh умноженное на 2
	add edi,ebx ;+ бывшее bh*2

	mov ax,word[edi] ;eax - сдиг для следущего поля
	cmp ax,0
	jg @f
		mov eax,MAX_LEN_OBJ_TXT ;последние поля для удобства кодируются 0, а не MAX_LEN_OBJ_TXT, исправляем ситуацию
	@@:
	cmp dword[ind],0
	je .end_fun ;для первого поля сдиг = 0, потому в структуре он не хранится, т.к. известен заранее
	sub ax,word[edi-2] ;отнимаем сдиг для искомого поля, получаем длинну поля

	cmp word[edi-2],0
	jg @f
		mov eax,0 ;поле пустое, не используется
	@@:
	cmp ax,MAX_LEN_OBJ_TXT
	jl @f
		mov eax,0 ;not text
	@@:

	pop edi
	.end_fun:
pop ebx
	ret
endp

align 4
proc draw_obj_icon, o_type:dword,n_col:dword, coord_y:dword
	pushad
	xor ebx,ebx
	mov bl,byte[o_type]
	call find_obj_in_opt

	add edi,obj_opt.img-obj_opt ;get index (+0)
	mov ebx,[n_col]
	shl ebx,1
	add edi,ebx ;move index (+bh)
	xor ecx,ecx
	mov cx,[edi]

	cmp cx,0
	jge @f
		jmp .end_fun ;error icon index
	@@:

	mov ebx,bmp_icon
	imul ecx,0x300
	add ebx,ecx

	mov eax,7
	mov ecx,0x100010 ;ecx - size:16*16
	mov edx,3 ;отступ от левого края рамки до иконки
	shl edx,16
	add edx,[coord_y]
	int 0x40
	.end_fun:
	popad
	ret
endp

;вывод текста с подписью
align 4
proc draw_obj_caption, o_type:dword,n_col:dword, coord_y:dword
	pushad
	stdcall get_obj_text_len_col, [foc_obj], [n_col] ;eax - длинна поля
	cmp eax,1
	jl .no_draw

	xor ebx,ebx
	mov bl,byte[o_type]
	call find_obj_in_opt

	mov edx,edi
	add edx,obj_opt.caption-obj_opt

	mov eax,[n_col]
	mov ebx,150
	shl ebx,16
	add ebx,dword[coord_y]
	add ebx,3
	mov ecx,0 ;text color

	mov esi,edx
	.beg_found:
		cmp byte[esi],0
		je .end_found
		cmp byte[esi],'*'
		je @f
		inc esi
		jmp .beg_found
	@@:
	cmp eax,0
	je .end_found
		dec eax
		inc esi
		mov edx,esi ;переключаем начало строки на след. подпись
		jmp .beg_found
	.end_found:
	cmp esi,edx
	jle .no_draw
		sub esi,edx ;esi = text len
		mov eax,4
		int 0x40
	.no_draw:
	popad
	ret
endp

align 4
proc draw_icon, ind:dword,coord_x:dword,coord_y:dword
	pushad
	mov ecx,[ind]
	cmp ecx,0
	jl .end_fun
		mov ebx,bmp_icon
		imul ecx,0x300
		add ebx,ecx

		mov eax,7
		mov ecx,0x100010 ;ecx - size:16*16
		mov edx,[coord_x]
		shl edx,16
		add edx,[coord_y]
		int 0x40
	.end_fun:
	popad
	ret
endp

;input
;  bl = cod object
;output:
;  edi = pointer to option struct
align 4
find_obj_in_opt:
	push ecx
	xor ecx,ecx ;find 'bl' in options
	mov edi,obj_opt
	@@:
		cmp bl,[edi] ;find
		je @f
		inc ecx
		cmp ecx,MAX_OBJ_TYPES
		je @f ;not find
		add edi,sizeof.ObjOpt
		jmp @b
	@@:
	pop ecx
	ret


obj_find_last dd 0 ;указатель на последний удачно найденный объект,
	;нужен для ускорения последущего поиска

;выход:
;eax - указатель на структуру найденного объекта
align 4
proc find_obj_in_file, o_type:dword
push ebx esi
	mov ebx,[o_type]
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi

	@@:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop eax ;получаем данные узла
		cmp eax,0
		je .find_no

		cmp byte[eax],bl ;смотрим описан ли искомый объект в узле
		je .find_yes

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp @b

	.find_yes:
		mov dword[obj_find_last],esi
		jmp @f
	.find_no:
		mov dword[obj_find_last],0
		xor eax,eax
	@@:
pop esi ebx
	ret
endp

;выход:
;eax - указатель на структуру найденного объекта
align 4
proc find_obj_in_file_next, o_type:dword
push ebx esi
	cmp dword[obj_find_last],0
	je .find_no

	mov ebx,[o_type]

	stdcall dword[tl_node_poi_get_next_info], dword[obj_find_last],tree2
	pop esi ;переходим к следущему узлу

	@@:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop eax ;получаем данные узла
		cmp eax,0
		je .find_no

		cmp byte[eax],bl ;смотрим описан ли искомый объект в узле
		je .find_yes

		stdcall dword[tl_node_poi_get_next_info], esi, tree2
		pop esi ;переходим к следущему узлу

		jmp @b

	.find_yes:
		mov dword[obj_find_last],esi
		jmp @f
	.find_no:
		mov dword[obj_find_last],0
		xor eax,eax
	@@:
pop esi ebx
	ret
endp

align 4
but_show_constructor:
	mov byte[show_mode],0
	stdcall [buf2d_draw], buf_fon
	ret

align 4
but_show_code:
	mov byte[show_mode],1
	stdcall [ted_draw], tedit0
	ret

;input:
; buf - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc conv_str_to_int, buf:dword
	xor eax,eax
	push ebx esi
	xor ebx,ebx
	mov esi,[buf]
	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;считывание 10-тичных цифр
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;считывание 16-ричных цифр
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;отсеиваем символы >'9' и <'A'
		.us1: ;составное условие
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;отсеиваем символы >'F' и <'a'
			sub bl,32 ;переводим символы в верхний регистр, для упрощения их последущей обработки
		.us2: ;составное условие
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:

	pop esi ebx
	ret
endp
