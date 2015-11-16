;
; в этом файле собраны функции нужные для создания и
; работы окна со свойствами объекта
;

prop_edits_top equ 25
prop_edits_height equ 18
prop_wnd_width equ 520 ;ширина окна со свойствами объекта

prop_wnd_run db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно

;даное окно (процесс) вызываеться функцией on_file_object_select
align 4
prop_start:
pushad
	mcall 40,0x27 ;маска ожидаемых событий

	call init_obj_count_txt_props ;получаем число используемых полей

	; настройка текстовых полей
	xor ebx,ebx
	mov edi,edit2
	@@:
		stdcall set_obj_win_param, ebx,edi
		inc ebx
		add edi,ed_struc_size
		cmp edi,prop_wnd_edits_end
		jl @b

	; настройка списка объектов, определяющих типы переменных
	stdcall dword[tl_data_init], tree3
	cmp byte[err_ini0],1
	je @f
		mov dword[tree3.data_img],bmp_icon
	@@:
	mov eax,dword[icon_tl_sys]
	mov dword[tree3.data_img_sys],eax

	mov edx,[foc_obj]
	cmp dword[edx],C_VAR
	jne .sel_end

	stdcall [tl_node_poi_get_info], tree2,0
	@@:
		cmp eax,0
		je @f
		mov edx,eax
		stdcall [tl_node_poi_get_data], tree2,edx
		mov esi,eax ;получаем данные узла
;------ добавление объекта в список -------------------------------------------
		mov ebx,[foc_obj]
		cmp ebx,esi
		je .add_end ;избегаем циклической ссылки
		mov ebx,dword[esi]
		cmp ebx,C_TDF
		je .add_beg
		cmp ebx,C_STR
		je .add_beg
			jmp .add_end
		.add_beg:
			stdcall get_obj_id,esi
			mov edi,u_object
			mov dword[edi],eax
			add edi,4
			add esi,u_object.txt-u_object ;переходим на имя объекта
			mov ecx,MAX_LEN_OBJ_TXT
			cld
			rep movsb

			call find_obj_in_opt ;edi = pointer to ObjOpt struct
			mov cx,word[edi+obj_opt.img-obj_opt]
			cmp cx,0
			jge .zero
				xor cx,cx ;что-бы не глючило с отрицательным индексом
			.zero:
			shl ecx,16 ;в ecx индекс иконки

			stdcall dword[tl_node_add], tree3,ecx,u_object ;структура u_object в данном случае не соблюдается, она нужна просто как память для вставки
			stdcall dword[tl_cur_next], tree3 ;переносим курсор вниз, что-бы не поменялся порядок
		.add_end:
;-------
		stdcall [tl_node_poi_get_next_info], tree2,edx
		mov edx,eax ;переходим к следущему узлу
		jmp @b
	@@:

	;ставим курсор на нужную позицию
	mov edx,[foc_obj] ;берем указатель на объект
	cmp edx,0
	je .sel_end
		mov edx,dword[edx+u_object.typid-u_object]
		stdcall [tl_cur_beg],tree3
		@@:
			stdcall [tl_node_get_data],tree3
			cmp eax,0
			je .sel_end
			cmp dword[eax],edx
			je .sel_end ;если нашли указатель
			stdcall [tl_cur_next],tree3
		jmp @b
	.sel_end:

	;чистка подписей
	mov dword[ch1.text],ch_text
	mov dword[ch2.text],ch_text
	mov dword[ch3.text],ch_text
	mov dword[ch4.text],ch_text

	;установка новых подписей
	mov esi,[foc_obj] ;берем указатель на объект
	mov ebx,dword[esi]
	call find_obj_in_opt
	mov edi,dword[edi+obj_opt.bit_prop-obj_opt]
	cmp edi,0
	je .no_bit_capts ;если 1-е битовое свойство пустое, то в цикл не заходим
	mov edx,ch1.text
	xor al,al ;0 - символ конца строки
	add edi,obj_opt
	@@:
		mov [edx],edi ;записываем текст в check_box
		xor ecx,ecx
		dec ecx ;ecx - число просматриваемых байт
		cld
		repne scasb ;ищем конец строки (al=0)
		cmp byte[edi],0
		je .no_bit_capts ;если следующей строки нет выход из цикла

		add edx,ch_struc_size
		cmp edx,check_boxes_end
		jl @b
	.no_bit_capts:

	;установка флажков
	mov esi,[foc_obj] ;берем указатель на объект
	mov edx,ch1.flags
	xor eax,eax
	@@:
		bt dword[esi+u_object.modif-u_object],eax
		jc .flag_set
			btr dword[edx],1 ;снимаем галочку
			jmp .end_set
		.flag_set:
			bts dword[edx],1 ;ставим галочку (ch_flag_en = 10b = 2^1)
		.end_set:

		inc eax
		add edx,ch_struc_size
		cmp edx,check_boxes_end
	jl @b

	init_checkboxes2 check_boxes,check_boxes_end
	check_boxes_set_sys_color2 check_boxes,check_boxes_end,sc
popad

align 4
prop_red_win:
pushad
	mcall 12,1

	xor eax,eax
	mov edi,hed
	mov edx,[foc_obj] ;берем указатель на объект
	cmp edx,0
	je @f
		mov ebx,dword[edx]
		call find_obj_in_opt
		add edi,obj_opt.info-obj_opt ;edi - подпись (название объекта)
	@@:
	mov ebx,30*65536+prop_wnd_width
	mov ecx,50*65536+250
	mov edx,0xffffd0
	or  edx,0x33000000
	int 0x40

	mov edx,[foc_obj] ;берем указатель на объект
	cmp edx,0
	je .exit_fun

		mov eax,8 ;кнопка для сохранения изменений
		mov ebx,prop_wnd_width-18-20
		shl ebx,16
		add ebx,18
		mov ecx,3
		shl ecx,16
		add ecx,18
		mov edx,10
		mov esi,0xd040
		int 0x40

		ror ebx,16 ;вычисляем координаты для иконки
		ror ecx,16
		and ebx,0xffff
		and ecx,0xffff
		inc ebx
		inc ecx
		stdcall draw_icon, 17,ebx,ecx ;17 - save

		mov edi,edit2
		mov ecx,[obj_count_txt_props]
		cld
		.cycle:
			stdcall [edit_box_draw], edi
			add edi,ed_struc_size
		loop .cycle

		cmp [err_ini0],1
		je .exit_fun
		cmp [err_ini1],1
		je .exit_fun

		mov edx,[foc_obj] ;берем указатель на объект
		xor ebx,ebx ;номер поля
		.draw_column:
			mov eax,ebx
			imul eax,prop_edits_height
			add eax,prop_edits_top
			stdcall draw_obj_icon,[edx],ebx,eax
			stdcall draw_obj_caption,[edx],ebx,eax

			inc ebx
			cmp ebx,MAX_OPT_FIELDS
			jl .draw_column

		cmp dword[edx],C_VAR
		jne .no_tree
			mov dword[w_scr_t3.all_redraw],1
			stdcall [tl_draw], tree3
		.no_tree:

		checkboxes_draw2 check_boxes,check_boxes_end

	.exit_fun:
	mcall 12,2
popad

;не отрывать эту функцию от предыдущей
align 4
prop_still:
	pushad
	mcall 10

	cmp al,1 ;изм. положение окна
	jz prop_red_win
	cmp al,2
	jne @f
		call prop_key
	@@:
	cmp al,3
	jz prop_button
	cmp al,6
	jne @f 
		call prop_mouse
	@@:
	popad
	jmp prop_still

;в цикле пробегаем по всем полям со свойствами,
;что-бы посчитать число используемых полей
align 4
init_obj_count_txt_props:
pushad
	mov edx,[foc_obj] ;берем указатель на объект
	xor ebx,ebx
	mov edi,edit2
	.cycle:
		stdcall get_obj_text_len_col, edx,ebx ;eax - длинна поля
		cmp eax,1
		jl @f ;выход из цикла если поле не используеться
		inc ebx

		add edi,ed_struc_size
		cmp edi,prop_wnd_edits_end
		jl .cycle
	@@:
	mov [obj_count_txt_props],ebx
popad
	ret

;вывод текстовых подписей для полей со свойствами
align 4
proc draw_obj_caption, o_type:dword,n_col:dword, coord_y:dword
	pushad
	stdcall get_obj_text_len_col, [foc_obj], [n_col] ;eax - длинна поля
	cmp eax,1
	jl .no_draw

	xor ebx,ebx
	mov ebx,dword[o_type]
	call find_obj_in_opt

	mov edx,edi
	add edx,obj_opt.caption-obj_opt

	mov eax,[n_col]
	mov ebx,143 ;отступ слева
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
proc set_obj_win_param, col:dword, edit:dword
	pushad
		stdcall get_obj_text_col, [foc_obj], [col] ;edx - text
		stdcall get_obj_text_len_col, [foc_obj], [col] ;eax - длинна поля
		mov edi,[edit]
		cmp eax,1
		jl @f
			dec eax
			mov ed_max,eax ;ed_max = edi+.max
			mov ed_focus_border_color, dword 0xff
			mov ed_blur_border_color, dword 0x808080
			stdcall [edit_box_set_text], edi,edx ;обновляем editbox
			jmp .end_f
		@@:
			mov ed_max,dword MAX_LEN_OBJ_TXT
			;stdcall [edit_box_set_text], edi,txt_null
		.end_f:
	popad
	ret
endp

align 4
prop_key:
push eax ecx edi
	mcall 2
	mov edi,edit2
	mov ecx,[obj_count_txt_props]
	cld
	.cycle:
		stdcall [edit_box_key], edi
		add edi,ed_struc_size
	loop .cycle

	mov edi,dword[foc_obj]
	cmp dword[edi],C_VAR
	jne @f
		stdcall [tl_key], tree3
	@@:
pop edi ecx eax
	ret

align 4
prop_mouse:
	push ecx edi
	mov edi,edit2
	mov ecx,[obj_count_txt_props]
	cld
	.cycle:
		stdcall [edit_box_mouse], edi
		add edi,ed_struc_size
	loop .cycle

	mov edi,dword[foc_obj]
	cmp dword[edi],C_VAR
	jne @f
		stdcall [tl_mouse], tree3
	@@:

	checkboxes_mouse2 check_boxes,check_boxes_end
	pop edi ecx
	ret

align 4
prop_button:
	mcall 17 ;получить код нажатой кнопки

	cmp ah,10
	jne @f
		call but_element_change
		mov ah,1 ;для закрытия окна
	@@:

	cmp ah,1
	jne prop_still
.exit:
	mov dword[tree3.data_img],0 ;чистим указатели на иконки что-бы их не удалил деструктор
	mov dword[tree3.data_img_sys],0 ;т. к. они еще используються в других элементах
	stdcall dword[tl_data_clear], tree3
	mov byte[prop_wnd_run],0 ;обнуляем счетчик окон
	mcall -1 ;выход из программы

;дерево с объектами в пользовательском файле
tree3 tree_list sizeof.object,(MAX_CED_OBJECTS shr 2)+2, tl_key_no_edit+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0xd000ff, 320,30,165,125, 0,4,MAX_LEN_OBJ_TXT, el_focus,\
	w_scr_t3,0

align 4
w_scr_t3:
.size_x     dw 16 ;+0
rb 2+2+2
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
rb 4+4
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
rb 4+2+2
.run_x:
rb 2+2+2+2+4+4+4+4+4+4
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84

check_boxes:
ch1 check_box2 (320 shl 16+11),(160 shl 16 +11),5,0xffffff,0,0x80000000,ch_text,ch_flag_en+ch_flag_middle
ch2 check_box2 (320 shl 16+11),(175 shl 16 +11),5,0xffffff,0,0x80000000,ch_text,ch_flag_en+ch_flag_middle
ch3 check_box2 (320 shl 16+11),(190 shl 16 +11),5,0xffffff,0,0x80000000,ch_text,ch_flag_en+ch_flag_middle
ch4 check_box2 (320 shl 16+11),(205 shl 16 +11),5,0xffffff,0,0x80000000,ch_text,ch_flag_en+ch_flag_middle
check_boxes_end:

ch_text db 0
