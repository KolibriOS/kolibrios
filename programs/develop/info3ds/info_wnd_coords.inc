;
; в этом файле собраны функции нужные для создания и
; работы окна с координатами вершин
;

prop_wnd_width  equ 340 ;ширина окна со свойствами объекта
prop_wnd_height equ 460 ;высота окна со свойствами объекта
3d_wnd_l equ   5 ;отступ для tinygl буфера слева
3d_wnd_t equ  23 ;отступ для tinygl буфера сверху
3d_wnd_w equ 320
3d_wnd_h equ 240
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

if lang eq ru
txt_select_vert: db 'Выбрано: '
else
txt_select_vert: db 'Select: '
end if
.count:
dq 0,0

;даное окно (процесс) вызываеться функцией but_wnd_coords
align 4
prop_start:
pushad
	mcall SF_SET_EVENTS_MASK,0x27 ;маска ожидаемых событий
	inc byte[prop_wnd_run]
	mov dword[w_scr_t3.type],1
	mov edi,o3d
	stdcall obj_clear_param, edi
	call mnu_reset_settings

	stdcall [tl_node_get_data],tree1
	xor edx,edx
	mov dword[capt_p],txt_q
	cmp eax,0
	je .no_points
		mov ebx,[eax] ;получаем значение сдвига выбранного блока
		add ebx,[open_file_data]

		cmp word[ebx],CHUNK_OBJBLOCK
		jne .end_oblo
			mov dword[capt_p],txt_4000
			stdcall obj_init,edi
			jmp .no_points
		.end_oblo:
		cmp word[ebx],CHUNK_TRIMESH
		jne .end_trim
			mov dword[capt_p],txt_4100
			stdcall obj_init,edi
			jmp .no_points
		.end_trim:
		cmp word[ebx],CHUNK_VERTLIST
		jne .end_vlist
			mov dword[capt_p],txt_4110
			stdcall obj_init,edi
			jmp .no_points
		.end_vlist:
		cmp word[ebx],CHUNK_TRACKPIVOT
		jne @f
			inc edx
			add ebx,6
			mov dword[capt_p],txt_b013
			jmp .set_points
		@@:
		cmp word[ebx],0xb014
		jne @f
			mov edx,2
			add ebx,6
			mov dword[capt_p],txt_b014
			;jmp .set_points
		@@:
		.set_points:
		mov [edi+obj_3d.poi_data],ebx
		mov [edi+obj_3d.poi_count],edx
	.no_points:

	;настройка списка объектов
	mov ebx,[edi+obj_3d.poi_data]
	mov dword[tree3.info_max_count],2
	mov edx,[edi+obj_3d.poi_count]
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
		mov dword[w_scr_t3.redraw],0
		.new_point:
			mov dword[obj_point],ebx
			mov dword[obj_point.text],0
			cld
			mov ecx,3
			@@:
				fld dword[ebx]
				fstp qword[Data_Double]
				call DoubleFloat_to_String
				call String_crop_0
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
			stdcall dword[tl_node_add], tree3,(ID_ICON_POINT shl 16),obj_point 
			stdcall dword[tl_cur_next], tree3
			dec edx
			cmp edx,0
			jg .new_point
			mov dword[w_scr_t3.redraw],1
			stdcall dword[tl_cur_beg], tree3
	.no_object:
	stdcall obj_set_sizes, o3d
	stdcall obj_set_normals, o3d
	edit_boxes_set_sys_color edit1,editboxes_end,sc ;установка системных цветов
	stdcall [edit_box_set_text], edit1, txt_q
	stdcall [edit_box_set_text], edit2, txt_q
	stdcall [edit_box_set_text], edit3, txt_q
	mov eax,0xc0c0c0
	mov ebx,[color_ox]
	shr ebx,2
	and ebx,0x3f3f3f
	add eax,ebx
	mov [edit1.color],eax
	mov eax,0xc0c0c0
	mov ebx,[color_oy]
	shr ebx,2
	and ebx,0x3f3f3f
	add eax,ebx
	mov [edit2.color],eax
	mov eax,0xc0c0c0
	mov ebx,[color_oz]
	shr ebx,2
	and ebx,0x3f3f3f
	add eax,ebx
	mov [edit3.color],eax
popad
	call prop_red_win

;не отрывать эту функцию от предыдущей
align 4
prop_still:
	pushad

	mcall SF_WAIT_EVENT_TIMEOUT,10
	or eax,eax
	jnz @f
		call prop_timer_funct
		jmp .end
	@@:

	cmp al,1 ;изм. положение окна
	jne @f
		call prop_red_win
		jmp .end
	@@:
	cmp al,2
	jne @f
		call prop_key
		jmp .end
	@@:
	cmp al,3
	jz prop_button
	cmp al,6
	jne @f 
		call prop_mouse
	@@:
.end:
	popad
	jmp prop_still

align 4
prop_red_win:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	xor eax,eax
	mov edi,dword[capt_p] ;children window caption
	mov bx,word[procinfo.box.left]
	add bx,word[buf_0.l]
	add bx,5 ;ширина боковой рамки
	shl ebx,16
	mov bx,prop_wnd_width
	mov cx,word[procinfo.box.top]
	add cx,word[buf_0.t]
	shl ecx,16
	mov cx,prop_wnd_height
	mov edx,[sc.work]
	or  edx,0x33000000
	int 0x40

	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON, (5 shl 16)+20, (266 shl 16)+20, 0x40000003
	mcall , (30 shl 16)+20, (266 shl 16)+20, 0x40000004

	mov ebx,[image_data_toolbar]
	add ebx,3*IMAGE_TOOLBAR_ICON_SIZE
	mcall SF_PUT_IMAGE, , (21 shl 16)+21, (5 shl 16)+266 ;load

	sub ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(30 shl 16)+266 ;save
	int 0x40

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t3
	stdcall [tl_draw], tree3
	stdcall [edit_box_draw], edit1
	stdcall [edit_box_draw], edit2
	stdcall [edit_box_draw], edit3

	stdcall [kmainmenu_draw], [main_menu]
	stdcall draw_3d, o3d
	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
proc prop_key uses eax ebx
	mcall SF_GET_KEY

	test word [edit1.flags],10b ;ed_focus
	je @f
		stdcall [edit_box_key], edit1
		jmp .end
	@@:
	test word [edit2.flags],10b
	je @f
		stdcall [edit_box_key], edit2
		jmp .end
	@@:
	test word [edit3.flags],10b
	je @f
		stdcall [edit_box_key], edit3
		jmp .end
	@@:
	cmp dword[el_focus], tree3
	jne @f
		stdcall [tl_key], tree3
		jmp .end
	@@:

	cmp ah,178 ;Up
	jne @f
		fld dword[angle_x]
		fadd dword[delt_size]
		fstp dword[angle_x]
		stdcall draw_3d, o3d
		jmp .end
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		stdcall draw_3d, o3d
		jmp .end
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		stdcall draw_3d, o3d
		jmp .end
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		stdcall draw_3d, o3d
		;jmp .end
	@@:
	.end:
	ret
endp

align 4
prop_mouse:
	push eax ebx
	mcall SF_MOUSE_GET,SSF_BUTTON_EXT
	bt eax,0
	jnc .end_m
		;mouse l. but. move
		cmp dword[mouse_drag],1
		jne .end_m
		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION ;get mouse coords
		mov ebx,eax
		shr ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jg @f
			mov ebx,3d_wnd_l
		@@:
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jle @f
			mov ebx,3d_wnd_w
		@@:
		and eax,0xffff ;mouse.y
		cmp eax,3d_wnd_t
		jg @f
			mov eax,3d_wnd_t
		@@:
		sub eax,3d_wnd_t
		cmp eax,3d_wnd_h
		jle @f
			mov eax,3d_wnd_h
		@@:
		finit
		fild dword[mouse_y]
		mov [mouse_y],eax
		fisub dword[mouse_y]
		fdiv dword[angle_dxm] ;если курсор движется по оси y (вверх или вниз) то поворот делаем вокруг оси x
		fadd dword[angle_x]
		fstp dword[angle_x]

		fild dword[mouse_x]
		mov [mouse_x],ebx
		fisub dword[mouse_x]
		fdiv dword[angle_dym] ;если курсор движется по оси x (вверх или вниз) то поворот делаем вокруг оси y
		fadd dword[angle_y]
		fstp dword[angle_y]

		stdcall draw_3d, o3d
		jmp .end_d
	.end_m:
	bt eax,16
	jnc @f
		;mouse l. but. up
		mov dword[mouse_drag],0
		jmp .end_d
	@@:
	bt eax,8
	jnc .end_d
		;mouse l. but. press
		mcall SF_MOUSE_GET,SSF_WINDOW_POSITION ;get mouse coords
		mov ebx,eax
		shr ebx,16 ;mouse.x
		cmp ebx,3d_wnd_l
		jl .end_d
		sub ebx,3d_wnd_l
		cmp ebx,3d_wnd_w
		jg .end_d
		and eax,0xffff ;mouse.y
		cmp eax,3d_wnd_t
		jl .end_d
		sub eax,3d_wnd_t
		cmp eax,3d_wnd_h
		jg .end_d
		mov dword[mouse_drag],1
		mov dword[mouse_x],ebx
		mov dword[mouse_y],eax
	.end_d:

	stdcall [kmainmenu_dispatch_cursorevent], [main_menu]
	stdcall [tl_mouse], tree3
	stdcall [edit_box_mouse], edit1
	stdcall [edit_box_mouse], edit2
	stdcall [edit_box_mouse], edit3
	pop ebx eax
	ret

align 4
prop_button:
	mcall SF_GET_BUTTON

	cmp ah,3
	jne @f
		call get_point_coords
		stdcall draw_3d, o3d
		jmp prop_still.end
	@@:
	cmp ah,4
	jne @f
		push eax ebx ecx edi esi
		stdcall [tl_node_get_data],tree3
		cmp eax,0
		je .no_select_1
			mov ebx,[eax]
			finit

			mov esi,string1
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp dword[ebx] ;сохраняем как float

			mov esi,string2
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
			call String_to_DoubleFloat
			fld  qword[Data_Double] ;считываем как double
			fstp dword[ebx+4] ;сохраняем как float

			mov esi,string3
			mov edi,Data_String
			cld
			mov ecx,8
			rep movsd
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
				call String_crop_0
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
			mov byte[obj_point.text],'*' ;отмечаем что вершина изменялась
			stdcall dword[tl_node_set_data], tree3,obj_point
			stdcall dword[tl_draw], tree3
			mov byte[can_save],1 ;для показа кнопки 'Сохранить'
			mov dword[offs_last_timer],0 ;для обновления блока в таймере
			stdcall draw_3d, o3d
		.no_select_1:
		pop esi edi ecx ebx eax
		jmp prop_still.end
	@@:
	cmp ah,5
	jne @f
		call mnu_vertexes_on
		jmp prop_still.end
	@@:
	cmp ah,6
	jne @f
		call mnu_edges_on
		jmp prop_still.end
	@@:
	cmp ah,7
	jne @f
		call mnu_faces_on
		jmp prop_still.end
	@@:
	cmp ah,8
	jne @f
		call mnu_light_on_off
		jmp prop_still.end
	@@:
	cmp ah,9
	jne @f
		call mnu_smooth_on_off
		jmp prop_still.end
	@@:
	cmp ah,10
	jne @f
		call mnu_reset_settings
		jmp prop_still.end
	@@:
	cmp ah,11
	jne @f
		call mnu_vertexes_select
		jmp prop_still.end
	@@:
	cmp ah,12
	jne @f
		call mnu_vertexes_deselect
		jmp prop_still.end
	@@:
	cmp ah,13
	jne @f
		call mnu_reset_average_x
		jmp prop_still.end
	@@:
	cmp ah,14
	jne @f
		call mnu_reset_average_y
		jmp prop_still.end
	@@:
	cmp ah,15
	jne @f
		call mnu_reset_average_z
		jmp prop_still.end
	@@:

	cmp ah,1
	jne prop_still.end
.exit:
	mov dword[tree3.data_img],0 ;чистим указатели на иконки что-бы их не удалил деструктор
	mov dword[tree3.data_img_sys],0 ;т. к. они еще используються в других элементах
	stdcall dword[tl_data_clear], tree3
	mov edi,o3d
	stdcall mem.Free,[edi+obj_3d.normals_tri_data]
	stdcall mem.Free,[edi+obj_3d.normals_poi_data]
	mov dword[edi+obj_3d.normals_tri_data],0
	mov dword[edi+obj_3d.normals_poi_data],0
	mov byte[prop_wnd_run],0 ;обнуляем счетчик окон
	mcall SF_TERMINATE_PROCESS ;выход из программы

align 4
get_point_coords:
	;загрузка координат в editbox-ы
	push eax ebx
	stdcall [tl_node_get_data],tree3
	cmp eax,0
	je @f
		mov ebx,[eax]
		mov word[NumberSymbolsAD],8
		finit
		fld dword[ebx]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall [edit_box_set_text], edit1, Data_String
		fld dword[ebx+4]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall [edit_box_set_text], edit2, Data_String
		fld dword[ebx+8]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		stdcall [edit_box_set_text], edit3, Data_String
		stdcall [edit_box_draw], edit1
		stdcall [edit_box_draw], edit2
		stdcall [edit_box_draw], edit3
	@@:
	pop ebx eax
	ret

align 4
proc prop_timer_funct uses eax
	;просматриваем выделенную координату
	stdcall [tl_node_get_data],tree3
	cmp [cursor_last_draw],eax
	je @f
		mov [cursor_last_draw],eax
		stdcall draw_3d, o3d
	@@:
	ret
endp

align 4
proc mnu_vertexes_select uses eax ebx
	stdcall [tl_node_get_data],tree3
	cmp eax,0
	je .no_points
	stdcall found_block_data,tree3
	cmp eax,0
	je .no_points
		mov bx,word[eax]
		cmp bx,ID_ICON_POINT_SEL
		je @f
			mov word[eax],ID_ICON_POINT_SEL
			inc dword[obj_poi_sel_c]
			jmp .change
		@@:
			mov word[eax],ID_ICON_POINT
			dec dword[obj_poi_sel_c]
		.change:
			mov eax,[obj_poi_sel_c]
			stdcall convert_int_to_str, txt_select_vert.count,16
	.no_points:
	ret
endp

align 4
proc mnu_vertexes_deselect uses eax
	cmp dword[obj_poi_sel_c],0
	je .no_points
	mov dword[obj_poi_sel_c],0
	stdcall [tl_node_poi_get_info], tree3,0
	cmp eax,0
	je .no_points
	@@:
		mov word[eax],ID_ICON_POINT
		stdcall [tl_node_poi_get_next_info], tree3,eax
		cmp eax,0
		jne @b
	.no_points:
	ret
endp

align 4
proc mnu_reset_average_x uses eax ebx ecx edx edi
	cmp dword[obj_poi_sel_c],2
	jl .end_select
		finit
		fldz
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel0
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov eax,[eax]
				fadd dword[eax]
				dec ecx
				jz @f
			.next_sel0:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
		fidiv dword[obj_poi_sel_c]
		fstp dword[eax]
		mov edx,[eax]
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel1
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov edi,eax
				mov eax,[eax]
				mov [eax],edx
				stdcall update_tree, edi
				dec ecx
				jz @f
			.next_sel1:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
    .end_select:
	ret
endp

align 4
proc mnu_reset_average_y uses eax ebx ecx edx edi
	cmp dword[obj_poi_sel_c],2
	jl .end_select
		finit
		fldz
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel0
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov eax,[eax]
				fadd dword[eax+4]
				dec ecx
				jz @f
			.next_sel0:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
		fidiv dword[obj_poi_sel_c]
		fstp dword[eax+4]
		mov edx,[eax+4]
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel1
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov edi,eax
				mov eax,[eax]
				mov [eax+4],edx
				stdcall update_tree, edi
				dec ecx
				jz @f
			.next_sel1:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
    .end_select:
	ret
endp

align 4
proc mnu_reset_average_z uses eax ebx ecx edx edi
	cmp dword[obj_poi_sel_c],2
	jl .end_select
		finit
		fldz
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel0
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov eax,[eax]
				fadd dword[eax+8]
				dec ecx
				jz @f
			.next_sel0:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
		fidiv dword[obj_poi_sel_c]
		fstp dword[eax+8]
		mov edx,[eax+8]
		mov ecx,dword[obj_poi_sel_c]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel1
				stdcall [tl_node_poi_get_data], tree3,ebx
				mov edi,eax
				mov eax,[eax]
				mov [eax+8],edx
				stdcall update_tree, edi
				dec ecx
				jz @f
			.next_sel1:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
		@@:
    .end_select:
	ret
endp

align 4
proc update_tree h_mem:dword
pushad
	mov edi,[h_mem]
	mov ebx,[edi]
	finit

	;обновление измененного узла в списке tree3 и главного окна (по таймеру)
	mov word[NumberSymbolsAD],5
	mov dword[obj_point.text],0
	cld
	mov ecx,3
	.coord:
		fld dword[ebx]
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
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
	mov esi,obj_point.text
	mov byte[esi],'*' ;отмечаем что вершина изменялась
	mov byte[can_save],1 ;для показа кнопки 'Сохранить'
	add edi,4
	mov ecx,MAX_OBJECT_SIZE-4
	rep movsb
popad
	ret
endp

align 4
angle_dxm dd 1.7775 ;~ 3d_wnd_w/180 - прибавление углов поворота сцены при вращении мышей
angle_dym dd 1.3335 ;~ 3d_wnd_h/180
ratio dd 1.3333 ;~ 3d_wnd_w/3d_wnd_h
cursor_last_draw dd 0 ;положение курсора при последней прорисовке 3d окна
capt_p dd 0

;дерево с объектами в пользовательском файле
tree3 tree_list MAX_OBJECT_SIZE,3, tl_key_no_edit+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0x400040, 5,290,303,140, 16, 4,0, el_focus,\
	w_scr_t3,get_point_coords

edit1 edit_box 80,  76, 269, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 32, string1, mouse_dd, 0
edit2 edit_box 80, 160, 269, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 32, string2, mouse_dd, 0
edit3 edit_box 80, 244, 269, 0xd0ffd0, 0xff, 0x80ff, 0, 0x8000, 32, string3, mouse_dd, 0
editboxes_end:

string1 rb 34
string2 rb 34
string3 rb 34

align 4
w_scr_t3 scrollbar 16, 0,0,0, 15, 100,0,0, 0,0,0, 1
