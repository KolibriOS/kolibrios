;
; в этом файле собраны функции нужные для создания и
; работы окна с координатами
;

prop_edits_top equ 25
prop_edits_height equ 18
prop_wnd_width equ 350 ;ширина окна со свойствами объекта
SIZE_ONE_FLOAT equ 14
MAX_OBJECT_SIZE equ (4+SIZE_ONE_FLOAT*3+1)

prop_wnd_run db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно

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
	cmp ebx,0
	je @f
		mov ebx,dword[ebx] ;получаем значение сдвига выбранного блока
		add ebx,dword[open_file_lif]
		mov dx,word[ebx+6] ;колличество точек
	@@:

	;настройка списка объектов
	mov dword[tree3.info_max_count],2
	add dword[tree3.info_max_count],edx ;ставим число узлов, равное колличеству точек (+2 на начало и конец списка)
	stdcall dword[tl_data_init], tree3
	mov eax,dword[icon_tl_sys]
	mov dword[tree3.data_img_sys],eax
	mov eax,dword[icon_toolbar]
	mov dword[tree3.data_img],eax

	cmp ebx,0
	je .no_object
		cmp word[ebx],CHUNK_VERTLIST
		jne .no_object ;если не тот объект
		add ebx,8
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
popad

align 4
prop_red_win:
pushad
	mcall 12,1

	xor eax,eax
	mov edi,txt_4110 ;caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	shl ebx,16
	mov bx,prop_wnd_width
	mov cx,word[procinfo.box.top]
	shl ecx,16
	mov cx,250
	mov edx,0xffffd0
	or  edx,0x33000000
	int 0x40

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t3
	stdcall [tl_draw], tree3
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
	stdcall [tl_key], tree3
pop eax
	ret

align 4
prop_mouse:
	;push ecx edi
	stdcall [tl_mouse], tree3
	;pop edi ecx
	ret

align 4
prop_button:
	mcall 17 ;получить код нажатой кнопки

	cmp ah,10
	jne @f
		;;;call but_element_change
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
tree3 tree_list MAX_OBJECT_SIZE,3,\
	tl_key_no_edit+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0xd000ff, 5,30,300,150, 16, 4,0, el_focus,\
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

