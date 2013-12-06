;
; в этом файле собраны функции нужные для создания и
; работы окна с координатами
;

prop_wnd_width equ 350 ;ширина окна со свойствами объекта
SIZE_ONE_FLOAT equ 14
MAX_OBJECT_SIZE equ (4+SIZE_ONE_FLOAT*3+1)

prop_wnd_run db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно

txt_q db '?',0
txt_space:
	db SIZE_ONE_FLOAT dup(' ')
.end:
	db 0

obj_point: ;добавляемый объект
	dd 0   ;указатель
.text:
	rb MAX_OBJECT_SIZE



;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
prop_start:
pushad
	mcall 40,0x27 ;маска ожидаемых событий
	inc byte[prop_wnd_run]

	stdcall [tl_node_get_data],tree1
	pop ebx
	xor edx,edx
	mov dword[capt_p],txt_q
	cmp ebx,0
	je .no_points
		mov ebx,dword[ebx] ;получаем значение сдвига выбранного блока
		add ebx,dword[open_file_lif]

		cmp word[ebx],CHUNK_VERTLIST
		jne @f
			movzx edx,word[ebx+6] ;колличество точек
			add ebx,8
			mov dword[capt_p],txt_4110
			jmp .no_points
		@@:
		cmp word[ebx],0xb013
		jne @f
			inc edx
			add ebx,6
			mov dword[capt_p],txt_b013
			jmp .no_points
		@@:
		cmp word[ebx],0xb014
		jne @f
			mov edx,2
			add ebx,6
			mov dword[capt_p],txt_b014
			;jmp .no_points
		@@:
	.no_points:

	;настройка списка объектов
	mov dword[tree3.info_max_count],2
	add dword[tree3.info_max_count],edx ;ставим число узлов, равное колличеству точек (+2 на начало и конец списка)
	stdcall dword[tl_data_init], tree3
	mov eax,dword[icon_tl_sys]
	mov dword[tree3.data_img_sys],eax
	mov eax,dword[icon_toolbar]
	mov dword[tree3.data_img],eax

	cmp edx,0
	je .no_object
		mov word[NumberSymbolsAD],5
		finit
		.new_point:
			mov dword[obj_point],ebx
			mov dword[obj_point.text],0
			cld
			mov ecx,3
			@@:
				fld dword[ebx]
				fstp qword[Data_Double]
				call DoubleFloat_to_String
				add ebx,4
				stdcall str_len, Data_String
				mov esi,txt_space
				add esi,eax
				cmp esi,txt_space.end
				jl .normal_size
					mov esi,txt_space.end-1 ;минимум 1 пробел нужно добавить, что-бы цифры не слипались
				.normal_size:
				stdcall str_cat, obj_point.text,esi
				stdcall str_cat, obj_point.text,Data_String
				loop @b
			stdcall dword[tl_node_add], obj_point, (ID_ICON_DATA shl 16), tree3
			stdcall dword[tl_cur_next], tree3
			dec edx
			cmp edx,0
			jg .new_point
			stdcall dword[tl_cur_beg], tree3
	.no_object:
	edit_boxes_set_sys_color edit1,editboxes_end,sc ;установка системных цветов
	stdcall [edit_box_set_text], edit1, txt_q
	stdcall [edit_box_set_text], edit2, txt_q
	stdcall [edit_box_set_text], edit3, txt_q
popad

align 4
prop_red_win:
pushad
	mcall 12,1

	xor eax,eax
	mov edi,dword[capt_p] ;children window caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	shl ebx,16
	mov bx,prop_wnd_width
	mov cx,word[procinfo.box.top]
	add cx,word[buf_0.t]
	shl ecx,16
	mov cx,250
	mov edx,0xffffd0
	or  edx,0x33000000
	int 0x40

	mov eax,8
	mov ebx,(5 shl 16)+20
	mov ecx,(195 shl 16)+20
	mov edx,3
	mov esi,[sc.work_button]
	int 0x40

	mov ebx,(30 shl 16)+20
	mov ecx,(195 shl 16)+20
	mov edx,4
	int 0x40

	mov eax,7
	mov ebx,[image_data_toolbar]
	add ebx,4*IMAGE_TOOLBAR_ICON_SIZE
	mov ecx,(16 shl 16)+16
	mov edx,(7 shl 16)+197 ;load
	int 0x40

	sub ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(32 shl 16)+197 ;save
	int 0x40

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t3
	stdcall [tl_draw], tree3
	stdcall [edit_box_draw], edit1
	stdcall [edit_box_draw], edit2
	stdcall [edit_box_draw], edit3
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

align 4
prop_key:
push eax
	mcall 2
	stdcall [edit_box_key], edit1
	stdcall [edit_box_key], edit2
	stdcall [edit_box_key], edit3
	stdcall [tl_key], tree3
pop eax
	ret

align 4
prop_mouse:
	;push ecx edi
	stdcall [tl_mouse], tree3
	stdcall [edit_box_mouse], edit1
	stdcall [edit_box_mouse], edit2
	stdcall [edit_box_mouse], edit3
	;pop edi ecx
	ret

align 4
prop_button:
	mcall 17 ;получить код нажатой кнопки

	cmp ah,3
	jne @f
		;загрузка координат в editbox-ы
		push ebx
		stdcall [tl_node_get_data],tree3
		pop ebx
		cmp ebx,0
		je .no_select_0
			mov ebx,[ebx]
			mov word[NumberSymbolsAD],8
			finit
			fld dword[ebx]
			fstp qword[Data_Double]
			call DoubleFloat_to_String
			stdcall [edit_box_set_text], edit1, Data_String
			fld dword[ebx+4]
			fstp qword[Data_Double]
			call DoubleFloat_to_String
			stdcall [edit_box_set_text], edit2, Data_String
			fld dword[ebx+8]
			fstp qword[Data_Double]
			call DoubleFloat_to_String
			stdcall [edit_box_set_text], edit3, Data_String
			stdcall [edit_box_draw], edit1
			stdcall [edit_box_draw], edit2
			stdcall [edit_box_draw], edit3
		.no_select_0:
		pop ebx
	@@:
	cmp ah,4
	jne @f
		push ebx ecx esi edi
		stdcall [tl_node_get_data],tree3
		pop ebx
		cmp ebx,0
		je .no_select_1
			mov ebx,[ebx]
			finit

			mov esi,string1
			mov edi,Data_String
			cld
			mov ecx,32
			rep movsb
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp dword[ebx] ;сохраняем как float

			mov esi,string2
			mov edi,Data_String
			cld
			mov ecx,32
			rep movsb
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp dword[ebx+4] ;сохраняем как float

			mov esi,string3
			mov edi,Data_String
			cld
			mov ecx,32
			rep movsb
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp dword[ebx+8] ;сохраняем как float

			;обновление измененного узла в списке tree3 и главного окна (по таймеру)
			mov word[NumberSymbolsAD],5
			mov dword[obj_point],ebx
			mov dword[obj_point.text],0
			cld
			mov ecx,3
			.coord:
				fld dword[ebx]
				fstp qword[Data_Double]
				call DoubleFloat_to_String
				add ebx,4
				stdcall str_len, Data_String
				mov esi,txt_space
				add esi,eax
				cmp esi,txt_space.end
				jl .normal_size
					mov esi,txt_space.end-1 ;минимум 1 пробел нужно добавить, что-бы цифры не слипались
				.normal_size:
				stdcall str_cat, obj_point.text,esi
				stdcall str_cat, obj_point.text,Data_String
				loop .coord
			stdcall dword[tl_node_set_data], obj_point, tree3
			stdcall dword[tl_draw], tree3
			mov byte[can_save],1 ;для показа кнопки 'Сохранить'
			mov dword[offs_last_timer],0 ;для обновления блока в таймере
		.no_select_1:
		pop edi esi ecx ebx
	@@:

	cmp ah,1
	jne prop_still
.exit:
	mov dword[tree3.data_img],0 ;чистим указатели на иконки что-бы их не удалил деструктор
	mov dword[tree3.data_img_sys],0 ;т. к. они еще используються в других элементах
	stdcall dword[tl_data_clear], tree3
	mov byte[prop_wnd_run],0 ;обнуляем счетчик окон
	mcall -1 ;выход из программы

capt_p dd 0

;дерево с объектами в пользовательском файле
tree3 tree_list MAX_OBJECT_SIZE,3,\
	tl_key_no_edit+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0xd000ff, 5,30,300,160, 16, 4,0, el_focus,\
	w_scr_t3,0

edit1 edit_box 80,  70, 200, 0xffffff, 0xff, 0x80ff, 0, 0x8000, 34, string1, mouse_dd, 0
edit2 edit_box 80, 154, 200, 0xffffff, 0xff, 0x80ff, 0, 0x8000, 34, string2, mouse_dd, 0
edit3 edit_box 80, 238, 200, 0xffffff, 0xff, 0x80ff, 0, 0x8000, 34, string3, mouse_dd, 0
editboxes_end:

string1 rb 34
string2 rb 34
string3 rb 34

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

