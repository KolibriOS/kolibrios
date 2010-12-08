;
; в этом файле собраны функции создающие програмный код
;
u_object object ;временный объект для задания смещений

macro insert_coma
{
	stdcall [ted_text_add], tedit0,txt_coma,1,ted_opt_ed_move_cursor
}
macro insert_nl
{
	stdcall [ted_text_add], tedit0,txt_nl,1,ted_opt_ed_move_cursor
}
macro insert_com
{
	stdcall [ted_text_add], tedit0,txt_com,1,ted_opt_ed_move_cursor
}
macro insert_sp
{
	stdcall [ted_text_add], tedit0,txt_sp,1,ted_opt_ed_move_cursor
}

;функция вставляет текстовую строку в текстовый редактор
align 4
proc insert_text, txt:dword
	push eax
	stdcall str_len,dword[txt]
	cmp eax,1
	jl @f
		stdcall [ted_text_add], tedit0,dword[txt],eax,ted_opt_ed_move_cursor
	@@:
	pop eax
	ret
endp

;вставляет указанное свойство из объекта в текстовый редактор
align 4
proc insert_text_prop, p_obj:dword,index:dword
	push eax edx
		mov eax,[p_obj]
		mov edx,[index]
		stdcall get_obj_text_col,eax,edx
		stdcall str_len,edx
		stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
	pop edx eax
	ret
endp

macro insert_tab
{
	stdcall [ted_text_add], tedit0,txt_tab,1,ted_opt_ed_move_cursor
}
macro insert_br
{
	stdcall [ted_text_add], tedit0,txt_br,3,ted_opt_ed_move_cursor
}
macro insert_a_met
{
	stdcall [ted_text_add], tedit0,txt_a_met,3,ted_opt_ed_move_cursor
}
macro insert_0
{
	stdcall [ted_text_add], tedit0,txt_0,1,ted_opt_ed_move_cursor
}
macro insert_plus
{
	stdcall [ted_text_add], tedit0,txt_plus,1,ted_opt_ed_move_cursor
}
macro insert_xor
{
	stdcall [ted_text_add], tedit0,txt_xor,4,ted_opt_ed_move_cursor
}
macro insert_cmp
{
	stdcall [ted_text_add], tedit0,txt_cmp,4,ted_opt_ed_move_cursor
}
macro insert_reg reg_name
{
	stdcall [ted_text_add], tedit0,reg_name,3,ted_opt_ed_move_cursor
}
macro insert_reg8 reg_name
{
	stdcall [ted_text_add], tedit0,reg_name,2,ted_opt_ed_move_cursor
}
macro insert_i65p
{
	stdcall [ted_text_add], tedit0,txt_i65p,7,ted_opt_ed_move_cursor
}


;
macro insert_but_beg
{
	stdcall [ted_text_add], tedit0,txt_but_1,txt_but_2-txt_but_1,ted_opt_ed_move_cursor
}
macro insert_but_end
{
	stdcall [ted_text_add], tedit0,txt_but_2,txt_but_3-txt_but_2,ted_opt_ed_move_cursor
}
;
macro insert_key_beg
{
	stdcall [ted_text_add], tedit0,txt_key_1,txt_key_2-txt_key_1,ted_opt_ed_move_cursor
}
macro insert_key_end
{
	stdcall [ted_text_add], tedit0,txt_key_2,txt_key_3-txt_key_2,ted_opt_ed_move_cursor
}

macro insert_d_poi
{
	stdcall [ted_text_add], tedit0,txt_d_poi,1,ted_opt_ed_move_cursor
}
macro insert_ret
{
	stdcall [ted_text_add], tedit0,txt_ret,4,ted_opt_ed_move_cursor
}


align 4
but_update:
	call draw_constructor
	cmp byte[show_mode],0 ;условие видимости окна конструктора
	jne @f
		stdcall [buf2d_draw], buf_fon
	@@:
	call code_gen
	cmp byte[show_mode],1 ;условие видимости текстового окна
	jne @f
		stdcall [ted_draw], tedit0
	@@:
	ret

;функция создающая код всего файла
align 4
code_gen:
	stdcall [ted_clear], tedit0,1
pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	xor bl,bl
	call code_gen_obj
	stdcall [ted_text_add], tedit0,\
		txt_nl,1,ted_opt_ed_move_cursor+ted_opt_ed_change_time ;добавляем символ новой строки с опцией ted_opt_ed_change_time для возможности сохранения файла
	stdcall [ted_init_scroll_bars], tedit0, 4 ;меняем размеры скроллингов
popad
	ret

;input:
; bl - уровень объекта
; esi - указатель на узел
align 4
code_gen_obj:
push ebx
	.beg_iterat:
		cmp esi,0
		je .find_no

		cmp byte[esi+2],bl ;lev
		jl .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop eax ;получаем данные узла
		cmp eax,0
		je .find_no

		cmp byte[eax],C_STR
		jne @f
			stdcall get_code_struct,eax ;обработка объекта
			stdcall dword[tl_node_poi_get_next_info], esi,tree2
			pop esi ;переходим к следущему узлу
			cmp esi,0
			je .end_st
			cmp byte[esi+2],bl ;lev
			jle .end_st
				push bx
					mov bl,byte[esi+2]
					call code_gen_obj ;рекурсивный вызов дочерних объектов
				pop bx
			.end_st:
			stdcall [ted_text_add], tedit0,txt_ends,6,ted_opt_ed_move_cursor
			jmp .beg_iterat
		@@:
		cmp byte[eax],C_TXT
		jne @f
			stdcall get_code_text,eax ;обработка объекта
		@@:
		cmp byte[eax],C_INC
		jne @f
			stdcall get_code_incl,eax ;обработка объекта
		@@:
		cmp byte[eax],C_COM
		jne @f
			stdcall get_code_comment,eax ;обработка объекта
		@@:
		cmp byte[eax],C_VAR
		jne @f
			stdcall get_code_variable,eax,ebx ;обработка объекта
		@@:
		cmp byte[eax],C_WND
		jne @f
			stdcall get_code_window,eax ;обработка объекта
		@@:
		cmp byte[eax],C_ACO
		jne @f
			stdcall get_code_auto,eax ;обработка объекта
		@@:
		cmp byte[eax],C_KEY ;клавиатура
		jne @f
			stdcall get_code_key,eax ;обработка объекта
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
pop ebx
	ret


align 4
proc get_code_text, p_obj:dword
	pushad
		insert_nl
		mov ebx,dword[p_obj]
		stdcall get_obj_text_col,ebx,0
		stdcall str_len,edx
		stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
		insert_nl
	popad
	ret
endp

;обработка объекта создающего автоматичексие коды
align 4
proc get_code_auto, p_obj:dword
	pushad
		insert_nl
		mov ebx,dword[p_obj]
		mov ecx,dword[ebx+u_object.modif-u_object]
		bt ecx,BIT_MOD_ACM
		jae @f
			;автоматические события от мыши
			call get_code_auto_m
		@@:
		bt ecx,BIT_MOD_ABU
		jae @f
			;создание функции button
			insert_but_beg
			call get_code_auto_but ;коды вызова кнопок
			insert_but_end
			call get_code_a_but_fun ;коды функций, вызываемых при нажатии кнопок
		@@:
		bt ecx,BIT_MOD_ACD
		jae @f
			call get_code_auto_data_el
			call get_code_auto_data_dop

			xor edx,edx
			mov dl,C_LIB
			stdcall find_obj_in_file,edx ;eax - указатель на структуру библиотеки
			cmp eax,0
			je @f
				stdcall insert_text, txt_head_i
				stdcall insert_text, txt_d_poi
				insert_nl
				stdcall insert_text, txt_head_l
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall insert_text, txt_sys_err
				stdcall insert_text, txt_str_end
				insert_nl

				call get_code_auto_data_libs_param
				stdcall insert_text, txt_ll_start
					call get_code_auto_data_libs_struc
				stdcall insert_text, txt_ll_end
				call get_code_auto_data_libs ;таблицы импорта библиотек
		@@:
		insert_nl
	popad
	ret
endp


align 4
proc get_code_auto_m
	pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_std_mouse_ed
			;insert_sp
			stdcall get_obj_text_col,ebx,0
			stdcall str_len,edx
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			insert_nl
		@@:
		cmp byte[ebx],C_CHB
		jne @f
			stdcall insert_text, txt_std_mouse_cb
			;insert_sp
			stdcall get_obj_text_col,ebx,0
			stdcall str_len,edx
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;создание данных (структур объектов)
align 4
proc get_code_auto_data_el
	pushad
	insert_nl ;отступ для форматирования

	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_EDB
		jne @f
			;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt
			stdcall insert_text_prop, ebx,0
			stdcall insert_text, txt_ebox

			mov ecx,1
			.prop:
				insert_sp
				stdcall insert_text_prop, ebx,ecx
				insert_coma
				inc ecx
				cmp ecx,10
			jl .prop

			insert_br
			stdcall insert_text, txt_txt

			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx

			stdcall insert_text, txt_mouse_dd

			stdcall insert_text, txt_0 ;тут будут свойства editbox а пока просто ставим 0
			xchg eax,edx ;сохраняем значение edx
			stdcall get_obj_text_col,ebx,10
			cmp byte[edx],0 ;проверяем пустая ли строка при запуске editbox
			je .null_string
				xchg eax,edx ;востанавливаем значение edx
				stdcall insert_text, txt_coma

				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text,txt_met_end
				stdcall insert_text,txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text,txt_minus
				stdcall insert_text, txt_1
				stdcall insert_text, txt_coma

				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text,txt_met_end
				stdcall insert_text,txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text,txt_minus
				stdcall insert_text, txt_1
			.null_string:
			insert_nl
		@@:
		cmp byte[ebx],C_DTT
		jne @f
			stdcall insert_text, txt_txt
			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			stdcall insert_text, txt_db
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;данные для библиотек
align 4
proc get_code_auto_data_libs_param
	pushad
	insert_nl ;отступ для форматирования

	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...

			stdcall insert_text, txt_sys_dir
			;object code
			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			;stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,1
			;stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text, txt_lib_name
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,0
			stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text,txt_err_m_f_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,4
			stdcall insert_text, txt_str_end
			insert_nl

			stdcall insert_text,txt_err_m_i_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_db
			stdcall insert_text, txt_str_beg
			stdcall insert_text_prop, ebx,5
			stdcall insert_text, txt_str_end
			insert_nl

			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;данные для библиотек
align 4
proc get_code_auto_data_libs_struc
	pushad
	;insert_nl ;отступ для форматирования

	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...

			stdcall insert_text, txt_lib
			;object code
			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			insert_sp
			stdcall insert_text, txt_ll
			stdcall insert_text, txt_lib_name
			stdcall insert_text,edx
			stdcall insert_text,txt_coma

			stdcall insert_text, txt_lib_path

			stdcall insert_text, txt_sys_dir
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_br
			stdcall insert_text,txt_tab

			stdcall insert_text,txt_err_m_f_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_head_l
			stdcall insert_text,txt_coma

			stdcall insert_text_prop, ebx,2
			stdcall insert_text,txt_coma

			stdcall insert_text,txt_err_m_i_lib
			stdcall insert_text,edx
			stdcall insert_text,txt_coma
			stdcall insert_text,txt_head_i

			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;данные для библиотек
align 4
proc get_code_auto_data_libs
	pushad
	insert_nl ;отступ для форматирования

	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_LIB
		jne @f
			;0,1,2,3,4,5 - l_nam,s_dir,i_met,...
			stdcall insert_text, txt_align_4
			stdcall insert_text_prop, ebx,2
			stdcall insert_text, txt_d_poi
			insert_nl

			call get_code_lib_table
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;input:
; esi
align 4
get_code_lib_table:
pushad
	mov bl,byte[esi+2]
	mov edi,esi
	stdcall dword[tl_node_poi_get_data], esi,tree2
	pop edx ;получение данных родительского объекта
	cmp edx,0
	je @f
	@@:
		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу
		cmp esi,0
		je @f
		cmp byte[esi+2],bl ;lev
		jle @f
			stdcall dword[tl_node_poi_get_data], esi,tree2
			pop ecx ;получаем данные узла
			cmp ecx,0
			je @f
			cmp byte[ecx],C_IFN
			jne @b
				;вставка указателя на функцию
				insert_tab
				stdcall insert_text_prop, ecx,0
				stdcall insert_text, txt_dd
				stdcall insert_text_prop, edx,3 ;приставка к функции
				stdcall insert_text_prop, ecx,0
				insert_nl
			jmp @b
	@@:
	stdcall insert_text, txt_dd_0_0
	mov esi,edi
	@@:
		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу
		cmp esi,0
		je @f
		cmp byte[esi+2],bl ;lev
		jle @f
			stdcall dword[tl_node_poi_get_data], esi,tree2
			pop ecx ;получаем данные узла
			cmp ecx,0
			je @f
			cmp byte[ecx],C_IFN
			jne @b
				;вставка имени импортированной функции
				insert_tab
				stdcall insert_text_prop, edx,3 ;приставка к функции
				stdcall insert_text_prop, ecx,0
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall insert_text_prop, ecx,1
				stdcall insert_text, txt_str_end
				insert_nl
			jmp @b
	@@:
popad
	ret

;
align 4
proc get_code_auto_data_dop
	pushad
	insert_nl ;отступ для форматирования

	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_EDB
		jne @f
			;1,2,3,4,...,6,...8,...,10 - w,x,y,col_fon,...,col_frame,...,col_txt,...,txt
			stdcall insert_text, txt_txt
			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx

			stdcall get_obj_text_col,ebx,10
			cmp byte[edx],0
			je .no_text
				stdcall insert_text, txt_d_poi
				stdcall insert_text, txt_db
				stdcall insert_text, txt_str_beg
				stdcall str_len,edx
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
				stdcall insert_text, txt_str_end
				insert_nl

				insert_tab
				stdcall insert_text, txt_met_end
				stdcall insert_text, txt_rb
				stdcall insert_text, txt_txt
				mov edx,fp_icon
				stdcall get_obj_id_str, ebx,edx
				stdcall insert_text,edx

				stdcall insert_text, txt_minus
				stdcall insert_text, txt_txt
				stdcall insert_text,edx
				stdcall insert_text, txt_met_end

				insert_plus
				stdcall insert_text_prop,ebx,9
				insert_plus
				stdcall insert_text, txt_3
				
				insert_nl

				jmp @f

			.no_text: ;первоначально текст не задан
				stdcall insert_text, txt_times

				stdcall insert_text_prop,ebx,9
				insert_plus
				stdcall insert_text, txt_2
				stdcall insert_text, txt_db
				insert_0
				insert_nl
		@@:
		;cmp byte[ebx],C_CHB
		;jne @f
		;@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;связываем кнопки с функциями
align 4
proc get_code_auto_but
	pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_BUT
		jne @f
			insert_tab
			insert_cmp
			insert_reg8 txt_ah
			insert_coma
			stdcall get_obj_text_col,ebx,0
			stdcall str_len,edx
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			insert_nl
			insert_tab
			stdcall insert_text, txt_jne_f
			insert_nl
			insert_tab
			insert_tab
			stdcall insert_text, txt_call
			stdcall get_obj_text_col,ebx,6
			stdcall str_len,edx
			cmp eax,1
			jl .c_fun
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
			.c_fun:
			insert_nl
			insert_tab
			insert_a_met ;@@:
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;берем коды для функций нажатий на кнопки
align 4
proc get_code_a_but_fun
	pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_BUT
		jne @f
			stdcall get_obj_text_col,ebx,6
			stdcall str_len,edx
			cmp eax,1
			jl .c_fun
				insert_nl
				stdcall insert_text, txt_align_4
				stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
				insert_d_poi
				insert_nl
				insert_tab
				insert_ret
			.c_fun:
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

align 4
proc get_code_key, p_obj:dword
	pushad
		insert_nl
		mov ebx,dword[p_obj]

		insert_key_beg
		call get_code_funct_key ;коды вызова кнопок
		insert_key_end

		insert_nl
	popad
	ret
endp

align 4
proc get_code_funct_key
	pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_std_key_ed
			stdcall insert_text_prop,ebx,0
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp

;объекты подключения файлов 'include ...'
align 4
proc get_code_incl, p_obj:dword
	pushad
		stdcall insert_text, txt_incl
		mov ebx,dword[p_obj]
		stdcall insert_text_prop,ebx,0

		stdcall get_obj_text_col,ebx,1
		stdcall str_len,edx
		cmp eax,1
		jl @f
			insert_sp
			insert_com
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
		@@:

		insert_nl
	popad
	ret
endp

align 4
proc get_code_comment, p_obj:dword
	insert_com
	stdcall insert_text_prop, dword[p_obj],0
	insert_nl
	ret
endp

;создаем код задания окна
align 4
proc get_code_window, p_obj:dword
	pushad
		mov ebx,dword[p_obj]
		;2,3,4,5,6 - x,y,w,h,col_bkg

		stdcall insert_text, txt_draw_wb

		insert_tab
		insert_xor
		insert_reg txt_eax
		insert_coma
		insert_reg txt_eax
		insert_nl

		;mov ebx,...
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_ebx
		insert_coma
		stdcall insert_text_prop, ebx,2
		insert_i65p
		stdcall insert_text_prop, ebx,4
		insert_nl

		;mov ecx,...
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_ecx
		insert_coma
		stdcall insert_text_prop, ebx,3
		insert_i65p
		stdcall insert_text_prop, ebx,5
		insert_nl

		;mov edx,...
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_edx
		insert_coma
		stdcall insert_text_prop, ebx,6
		insert_nl

		;or edx,0x33000000
		insert_tab
		stdcall insert_text, txt_or
		insert_reg txt_edx
		insert_coma
		stdcall insert_text, txt_0x33000000
		insert_nl

		;mov edi,...
		insert_tab
		stdcall insert_text, txt_mov
		insert_reg txt_edi
		insert_coma
		stdcall insert_text_prop, ebx,0
		insert_nl

		insert_tab
		stdcall insert_text, txt_i_40
		insert_nl

		call get_code_draw_obj

		stdcall insert_text, txt_draw_we ;код завершающий функцию рисования
	popad
	ret
endp


;формируем вызов функций для рисования объектов
align 4
proc get_code_draw_obj
	pushad
	stdcall dword[tl_node_poi_get_info], 0,tree2
	pop esi
	.beg_iterat:
		cmp esi,0
		je .find_no

		stdcall dword[tl_node_poi_get_data], esi,tree2
		pop ebx ;получаем данные узла
		cmp ebx,0
		je .find_no

		cmp byte[ebx],C_EDB
		jne @f
			stdcall insert_text, txt_editb_draw
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:
		cmp byte[ebx],C_CHB
		jne @f
			stdcall insert_text, txt_cbox_draw
			stdcall insert_text_prop, ebx,0
			insert_nl
		@@:
		cmp byte[ebx],C_BUT
		jne @f
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_8
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_text_prop, ebx,1
			insert_i65p
			stdcall insert_text_prop, ebx,3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_text_prop, ebx,2
			insert_i65p
			stdcall insert_text_prop, ebx,4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text_prop, ebx,0
			insert_nl

			insert_tab
			stdcall insert_text, txt_or
			insert_reg txt_edx
			insert_coma
			stdcall insert_text, txt_but_create
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_esi
			insert_coma
			stdcall insert_text_prop, ebx,5 ;button color
			insert_nl

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		@@:
		cmp byte[ebx],C_REC
		jne @f
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_1
			stdcall insert_text, txt_3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_text_prop, ebx,1
			insert_i65p
			stdcall insert_text_prop, ebx,3
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_text_prop, ebx,2
			insert_i65p
			stdcall insert_text_prop, ebx,4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text_prop, ebx,0 ;rect color
			insert_nl

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		@@:
		cmp byte[ebx],C_DTT ;рисование текста через сист. ф. 4
		jne @f
			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_eax
			insert_coma
			stdcall insert_text, txt_4
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ebx
			insert_coma
			stdcall insert_text_prop, ebx,1
			insert_i65p
			stdcall insert_text_prop, ebx,2
			insert_nl

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_ecx
			insert_coma
			stdcall insert_text_prop, ebx,3
			insert_nl

;or ecx,0x80000000

			insert_tab
			stdcall insert_text, txt_mov
			insert_reg txt_edx
			insert_coma
			stdcall insert_text, txt_txt
			mov edx,fp_icon
			stdcall get_obj_id_str, ebx,edx
			stdcall insert_text,edx
			insert_nl

;mov esi,...

			insert_tab
			stdcall insert_text, txt_i_40
			insert_nl
		@@:

		stdcall dword[tl_node_poi_get_next_info], esi,tree2
		pop esi ;переходим к следущему узлу

		jmp .beg_iterat
	.find_no:
	popad
	ret
endp



align 4
proc get_code_struct, p_obj:dword
	pushad
		mov ebx,dword[p_obj]
		;0 - name

		stdcall [ted_text_add], tedit0,txt_struct,7,ted_opt_ed_move_cursor
		stdcall insert_text_prop, ebx,0
		insert_nl

	popad
	ret
endp

;создать код объявления переменной
align 4
proc get_code_variable, p_obj:dword,o_lev:dword
	pushad
		mov ebx,dword[p_obj]
		;0,1,2 - name,value,comment

		mov ecx,dword[o_lev]
		cmp cl,0
		je .no_tab ;избегаем зацикливания (0-1 = 2^32-1)
		and cx,0xff
		cld
		@@:
			insert_tab ;формирование отступа перед переменной
		loop @b
		.no_tab:

		stdcall insert_text_prop, ebx,0 ;имя переменной
		insert_sp

		mov ecx,dword[ebx+u_object.typid-u_object] ;ecx - относительный индекс родительского объекта (в памяти)
		xor eax,eax
		mov ax,[tree2.info_size]
		imul ecx,eax ;умножаем индекс на размер структуры
		add ecx,[tree2.data_info] ;ecx - указатель данные родительского узла

		;проверку на правильность ecx ???

		stdcall insert_text_prop, ecx,0 ;тип переменной
		insert_sp
		stdcall insert_text_prop, ebx,1 ;значение

		stdcall get_obj_text_col,ebx,2 ;комментарий
		stdcall str_len,edx
		cmp eax,1
		jl @f
			insert_sp
			insert_com
			stdcall [ted_text_add], tedit0,edx,eax,ted_opt_ed_move_cursor
		@@:
		insert_nl

	popad
	ret
endp



;--- текстовые константы ---
txt_str_beg db 39,0
txt_str_end db 39,',0',0
txt_coma db ',',0
txt_i_40 db 'int 0x40',13,0
txt_0 db '0',0
txt_1 db '1',0
txt_2 db '2',0
txt_3 db '3',0
txt_4 db '4',0
txt_8 db '8',0
txt_plus db '+',0
txt_minus db '-',0
;txt_dol db '$',0
txt_nl db 13,0
txt_com db ';',0
txt_sp db ' ',0
txt_tab db 9,0
txt_br db '\',13,9,0
txt_txt db 'txt_',0
txt_db db ' db ',0
txt_dd db ' dd ',0
txt_rb db ' rb ',0
txt_dd_0_0 db 9,'dd 0,0',13,0
txt_mouse_dd db ', mouse_dd, ',0
txt_times db ':',13,9,'times ',0
txt_draw_wb:
	db 13,'align 4',13,'draw_window:',13,'pushad',13,9,'mcall 12,1',13,0
txt_draw_we:
	db 9,'mcall 12,2',13,'popad',13,9,'ret',13,0
txt_sys_dir db 'system_dir_',0
txt_lib_name db 'lib_name_',0
txt_lib_path db ' sys_path, library_path, ',0
txt_sys_err db '‘ЁбвҐ¬­ п ®иЁЎЄ ',0
txt_ll_start db 'l_libs_start:',13,0
txt_lib db 9,'lib_',0
txt_ll db 'l_libs ',0
txt_err_m_f_lib db 'err_msg_found_lib_',0
txt_head_l db 'head_f_l',0
txt_err_m_i_lib db 'err_msg_import_',0
txt_head_i db 'head_f_i',0
txt_ll_end db 'l_libs_end:',13,0
txt_but_create db '0x40000000',0 ;tmp
txt_0x33000000 db '0x33000000',0 ;tmp
txt_struct db 'struct ',0
txt_met_end db '.end',0
txt_ends db 'ends',13,13,0
txt_mov db 'mov ',0
txt_xor db 'xor ',0
txt_or db 'or  ',0
txt_cmp db 'cmp ',0
txt_call db 'call ',0
txt_jne_f db 'jne @f',0
txt_a_met db '@@:',0
txt_ah db 'ah',0
txt_eax db 'eax',0
txt_ebx db 'ebx',0
txt_ecx db 'ecx',0
txt_edx db 'edx',0
txt_edi db 'edi',0
txt_esi db 'esi',0
txt_align_4 db 'align 4',13,0
txt_d_poi db ':',0
txt_ret db 'ret',13,0
txt_i65p db '*65536+',0
txt_std_mouse_ed:
	db 9,'stdcall [edit_box_mouse], ',0
txt_std_mouse_cb:
	db 9,'stdcall [check_box_mouse], ',0
txt_std_key_ed:
	db 9,'stdcall [edit_box_key], ',0
txt_incl:
	db 'include ',0
txt_ebox db ' edit_box',0
txt_editb_draw:
	db 9,'stdcall [edit_box_draw], ',0
txt_cbox_draw:
	db 9,'stdcall [check_box_draw], ',0

txt_but_1:
db 'align 4',13,'button:',13,9,'mcall 17',13
txt_but_2:
db 9,'cmp ah,1',13,9,'jne still',13,'.exit:',13,9,'mcall -1',13
txt_but_3:

txt_key_1:
db 'align 4',13,'key:',13,9,'mcall 2',13
txt_key_2:
db 9,'jmp still',13
txt_key_3:

;получаем порядковый номер объекта относительно памяти элемента tree2
;output:
; txt_buf - строка с номером объекта
align 4
proc get_obj_id_str, p_obj:dword, txt_buf:dword
	push eax ecx edx
	mov eax,dword[p_obj]
	sub eax,[tree2.data_info]
	xor edx,edx
	xor ecx,ecx
	mov cx,word[tree2.info_size]
	div ecx

	mov edx,dword[txt_buf]
	mov byte[edx],0
	stdcall str_cat_value,edx,eax
	pop edx ecx eax
	ret
endp

;получаем порядковый номер объекта относительно памяти элемента tree2
;output:
; eax - номер объекта
align 4
proc get_obj_id, p_obj:dword
	push ecx edx
	mov eax,dword[p_obj]
	sub eax,[tree2.data_info]
	xor edx,edx
	xor ecx,ecx
	mov cx,word[tree2.info_size]
	div ecx
	pop edx ecx
	ret
endp

;--- текстовые функции ---

align 4
proc mem_clear, mem:dword, len:dword
	push eax ecx edi
	cld
	xor al,al
	mov edi, dword[mem]
	mov ecx, dword[len]
	repne stosb
	pop edi ecx eax
	ret
endp

align 4
proc str_cat, str1:dword, str2:dword
	push eax ecx edi esi
	mov esi,dword[str2]
	stdcall str_len,esi
	mov ecx,eax
	inc ecx
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	pop esi edi ecx eax
	ret
endp

align 4
proc str_cat_value, str1:dword, value:dword
	pushad
	mov edi,[str1]
	stdcall str_len,edi
	add edi,eax
	mov eax,[value]
	call tl_convert_to_str
	popad
	ret
endp

;output:
; eax = strlen
align 4
proc str_len, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
	ret
endp

;input:
; eax = value
; edi = string buffer
align 4
tl_convert_to_str:
	pushad
	stdcall mem_clear,edi,12
	call .str
	popad
	ret

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
		;преревод числа в ASCII строку входные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx ;очистить edx
		div ecx     ;разделить - остаток в edx
		push edx    ;положить в стек
		;dec edi    ;смещение необходимое для записи с конца строки
		call .str   ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	or al,0x30  ;данная команда короче  чем две выше 
	stosb	    ;записать элемент из регистра al в ячеку памяти es:edi
	ret