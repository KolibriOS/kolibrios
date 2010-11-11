align 4
draw_obj_info:
	mov eax,13 ;rect
	mov edx,WND_CAPT_COLOR
	mov ebx,[wndObjI.left]
	shl ebx,16
	add ebx,[wndObjI.width]
	mov ecx,[wndObjI.top]
	add ecx,[recMain.top]
	shl ecx,16
	add ecx,[wndObjI.height]
	sub ecx,[recMain.top]
	mov edx,0xffffff ;0x80ffffff
	int 0x40 ;прямоугольник для окна

	mov ecx,[wndObjI.top] ;draw caption
	shl ecx,16
	add ecx,[recMain.top]
	mov edx,WND_CAPT_COLOR
	int 0x40

	mov edx,[foc_obj] ;init icon pointers
	cmp edx,0
	je .exit_fun
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    pushad
    ;draw caption wndObjI
    mov eax,4 ;подпись - название объекта

    mov bl,[edx]
    call find_obj_in_opt
    mov edx,edi

    mov ebx,[wndObjI.left]
    add ebx,[recMain.left]
    add ebx,4
    shl ebx,16
    add ebx,[wndObjI.top]
    add ebx,4
    mov ecx,0xc0d000ff
    mov edi,WND_CAPT_COLOR
    add edx,obj_opt.info-obj_opt
    int 0x40

	mov eax,8 ;кнопка для сохранения изменений
	mov ebx,125*65536+20
	mov ecx,161*65536+10
	mov edx,10
	mov esi,0xd040
	int 0x40
    popad
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	stdcall [edit_box_draw], dword edit2
	stdcall [edit_box_draw], dword edit3
	stdcall [edit_box_draw], dword edit4
	stdcall [edit_box_draw], dword edit5
	stdcall [edit_box_draw], dword edit6
	stdcall [edit_box_draw], dword edit7
	stdcall [edit_box_draw], dword edit8
	stdcall [edit_box_draw], dword edit9

	xor ebx,ebx ;номер поля
	.draw_column:
		mov eax,ebx
		imul eax,[recMain.height]
		add eax,[recMain.top]
		add eax,[wndObjI.top]
		stdcall draw_icon_obj,[edx],ebx,eax

		inc ebx
		cmp ebx,MAX_OPT_FIELDS
	jl .draw_column
	.exit_fun:
	ret

;обновление объектов в tree2, по значениям текстовых полей
;функция обратная к данной on_file_object_select
align 4
but_element_change:
	;foc_obj - указатель на последний выбранный узел в tree2
	cmp dword[foc_obj],0
	je @f
		pushad
		stdcall get_obj_win_param, 0,edit2
		stdcall get_obj_win_param, 1,edit3
		stdcall get_obj_win_param, 2,edit4
		stdcall get_obj_win_param, 3,edit5
		stdcall get_obj_win_param, 4,edit6
		stdcall get_obj_win_param, 5,edit7
		stdcall get_obj_win_param, 6,edit8
		stdcall get_obj_win_param, 7,edit9
		popad
	@@:
	call draw_constructor ;обновляем окно конструктора
	call draw_window
	ret

align 4
proc get_obj_win_param, col:dword, edit:dword
;pushad
		stdcall get_obj_text_col, [foc_obj], [col] ;edx - смещение для текста
		stdcall get_obj_text_len_col, [foc_obj], [col] ;eax - длинна поля
		cld
		mov edi,[edit]
		mov esi,ed_text
		mov edi,edx
		mov ecx,eax
		rep movsb
	ret
;popad
endp

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
proc draw_icon_obj, o_type:dword,n_col:dword, coord_y:dword
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
	mov edx,[wndObjI.left]
	add edx,5
	shl edx,16
	add edx,[coord_y]
	int 0x40
	.end_fun:
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
