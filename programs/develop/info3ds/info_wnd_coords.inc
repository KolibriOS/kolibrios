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

bit_vertexes equ 0
bit_faces equ 1
bit_faces_fill equ 2
bit_light equ 3

ini_name db 'info3ds.ini',0
ini_sec_w3d db 'Window 3d',0
key_dv  db 'draw_vertices',0
key_df  db 'draw_faces',0
key_dff db 'draw_faces_fill',0
key_dl  db 'draw_light',0
key_ox  db 'col_ox',0
key_oy  db 'col_oy',0
key_oz  db 'col_oz',0
key_bk  db 'col_bkgnd',0
key_vert db 'col_vertices',0
key_face db 'col_faces',0

prop_wnd_run db 0 ;переменная следящая за тем что-бы не запускать больше 1-го окна со свойствами одновременно

txt_q db '?',0
txt_space:
	db SIZE_ONE_FLOAT dup(' ')
.end:
	db 0

rad_c dd 150 ;для вычисления радиуса

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
	mov dword[w_scr_t3.type],1
	call prop_wnd_clear_param

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
			call found_block_data
			cmp eax,0
			je .no_points
			mov esi,eax
			movzx ecx,byte[esi+2] ;cl - уровень выбранного узла
			.cycle_0:
				stdcall [tl_node_poi_get_next_info], tree1,esi
				cmp eax,0
				je .no_points
				cmp byte[eax+2],cl
				jle .no_points
				mov esi,eax
				stdcall [tl_node_poi_get_data], tree1,esi
				mov ebx,[eax]
				add ebx,[open_file_data]
				cmp word[ebx],CHUNK_VERTLIST
				je .ini_vlist
				jmp .cycle_0
			jmp .no_points
		.end_oblo:
		cmp word[ebx],CHUNK_TRIMESH
		jne .end_trim
			mov dword[capt_p],txt_4100
			call found_block_data
			cmp eax,0
			je .no_points
			mov esi,eax
			movzx ecx,byte[esi+2] ;cl - уровень выбранного узла
			.cycle_1:
				stdcall [tl_node_poi_get_next_info], tree1,esi
				cmp eax,0
				je .no_points
				cmp byte[eax+2],cl
				jle .no_points
				mov esi,eax
				stdcall [tl_node_poi_get_data], tree1,esi
				mov ebx,[eax]
				add ebx,[open_file_data]
				cmp word[ebx],CHUNK_VERTLIST
				je .ini_vlist
				jmp .cycle_1
			jmp .no_points
		.end_trim:
		cmp word[ebx],CHUNK_VERTLIST
		jne .end_vlist
			mov dword[capt_p],txt_4110
			.ini_vlist:
			movzx edx,word[ebx+6] ;колличество точек
			add ebx,8
			;поиск данных для треугольков
			call found_block_data
			cmp eax,0
			je .set_points
			mov esi,eax
			;если нашли выбранный узел, то далее пытаемся найти узел с треугольниками
			movzx ecx,byte[esi+2] ;cl - уровень выбранного узла
			.cycle_2:
				stdcall [tl_node_poi_get_next_info], tree1,esi
				cmp eax,0
				je .set_points
				mov esi,eax
				stdcall [tl_node_poi_get_data], tree1,esi
				cmp byte[esi+2],cl
				jl .set_points
				je .found
				jmp .cycle_2
			.found: ;если нашли узел тогоже уровня что и выбранный узел
				cmp eax,0
				je .set_points
				mov eax,[eax]
				add eax,[open_file_data]
				cmp word[eax],CHUNK_FACELIST
				jne .cycle_2
				movzx ecx,word[eax+6]
				mov [obj_tri_count],ecx
				add eax,8 ;2+4+2 (chunk+size+count)
				mov [obj_tri_data],eax
			jmp .set_points
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
		mov [obj_poi_data],ebx
		mov [obj_poi_count],edx
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
			stdcall dword[tl_node_add], tree3,(ID_ICON_DATA shl 16),obj_point 
			stdcall dword[tl_cur_next], tree3
			dec edx
			cmp edx,0
			jg .new_point
			stdcall dword[tl_cur_beg], tree3
	.no_object:
	call obj_set_sizes
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

	mcall 23,10
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
	mcall 12,1

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
	mcall 8, (5 shl 16)+20, (266 shl 16)+20, 3
	mcall , (30 shl 16)+20, (266 shl 16)+20, 4

	mov ebx,[image_data_toolbar]
	add ebx,3*IMAGE_TOOLBAR_ICON_SIZE
	mcall 7, , (16 shl 16)+16, (7 shl 16)+268 ;load

	sub ebx,IMAGE_TOOLBAR_ICON_SIZE
	mov edx,(32 shl 16)+268 ;save
	int 0x40

	mov dword[w_scr_t3.all_redraw],1
	stdcall [scrollbar_ver_draw],dword w_scr_t3
	stdcall [tl_draw], tree3
	stdcall [edit_box_draw], edit1
	stdcall [edit_box_draw], edit2
	stdcall [edit_box_draw], edit3

	stdcall [kmainmenu_draw], [main_menu]
	call draw_3d
	mcall 12,2
popad
	ret

;установка всех основных переменных по умолчанию
align 4
prop_wnd_clear_param:
	;установка параметров отображения
	call mnu_reset_settings

	;обнуление указателей на данные объектов
	mov dword[obj_poi_data],0
	mov dword[obj_poi_count],0
	mov dword[obj_tri_data],0
	mov dword[obj_tri_count],0
	ret

;поиск данных
;input:
;eax - текущий узел
;output:
;eax - структура текущего узла (или 0 при неудаче)
align 4
found_block_data:
push ebx ecx
	mov ecx,eax
	stdcall [tl_node_poi_get_info], tree1,0
	@@:
		mov ebx,eax
		stdcall [tl_node_poi_get_data], tree1,ebx
		cmp eax,ecx
		je @f ;если попали на выбранный узел
		stdcall [tl_node_poi_get_next_info], tree1,ebx
		cmp eax,0
		jne @b
		jmp .end
	@@:
		mov eax,ebx
	.end:
pop ecx ebx
	ret

;установка размеров объекта
align 4
proc obj_set_sizes uses eax ebx esi
	cmp dword[obj_poi_count],0
	je .end
		finit
		mov eax,[obj_poi_data]
		mov ebx,dword[eax]
		mov [obj_x_min],ebx
		mov [obj_x_max],ebx
		mov ebx,dword[eax+4]
		mov [obj_y_min],ebx
		mov [obj_y_max],ebx
		mov ebx,dword[eax+8]
		mov [obj_z_min],ebx
		mov [obj_z_max],ebx

	mov ebx,[obj_poi_data]
	mov esi,[obj_poi_count]
align 4
	.cycle_0: ;работа с координатой x
		fld dword[ebx]
		fld dword[obj_x_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx]
			mov [obj_x_min],eax
			ffree st0
			fincstp
			jmp .next_x
		@@:
		fld dword[obj_x_max]
		fcompp
		fstsw ax
		sahf
		ja .next_x
			mov eax,[ebx]
			mov [obj_x_max],eax
		.next_x:

		add ebx,12
		dec esi
		jnz .cycle_0

		fld1
		fld dword[obj_x_max]
		fsub dword[obj_x_min]
		fst dword[obj_x_centr]
		fdivp ;1/size.x
		fstp dword[obj_x_scale]

		fld dword[obj_x_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.x = size.x/2
		fadd dword[obj_x_min]
		fchs
		fstp dword[obj_x_centr]

	mov ebx,[obj_poi_data]
	mov esi,[obj_poi_count]
align 4
	.cycle_1: ;работа с координатой y
		fld dword[ebx+4]
		fld dword[obj_y_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+4]
			mov [obj_y_min],eax
			ffree st0
			fincstp
			jmp .next_y
		@@:
		fld dword[obj_y_max]
		fcompp
		fstsw ax
		sahf
		ja .next_y
			mov eax,[ebx+4]
			mov [obj_y_max],eax
		.next_y:

		add ebx,12
		dec esi
		jnz .cycle_1

		fld1
		fld dword[obj_y_max]
		fsub dword[obj_y_min]
		fst dword[obj_y_centr]
		fdivp ;1/size.y
		fstp dword[obj_y_scale]

		fld dword[obj_y_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.y = size.y/2
		fadd dword[obj_y_min]
		fchs
		fstp dword[obj_y_centr]

	mov ebx,[obj_poi_data]
	mov esi,[obj_poi_count]
align 4
	.cycle_2: ;работа с координатой z
		fld dword[ebx+8]
		fld dword[obj_z_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+8]
			mov [obj_z_min],eax
			ffree st0
			fincstp
			jmp .next_z
		@@:
		fld dword[obj_z_max]
		fcompp
		fstsw ax
		sahf
		ja .next_z
			mov eax,[ebx+8]
			mov [obj_z_max],eax
		.next_z:

		add ebx,12
		dec esi
		jnz .cycle_2

		fld1
		fld dword[obj_z_max]
		fsub dword[obj_z_min]
		fst dword[obj_z_centr]
		fdivp ;1/size.z
		fstp dword[obj_z_scale]

		fld dword[obj_z_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.z = size.z/2
		fadd dword[obj_z_min]
		fchs
		fstp dword[obj_z_centr]

		;находим радиус для сферы
		fld dword[obj_x_max]
		fsub dword[obj_x_min]
		fld dword[obj_y_max]
		fsub dword[obj_y_min]
		faddp
		fild dword[rad_c]
		fdivp ;radius=(size.x+size.y)/rad_c
		fstp dword[sph_radius]
	.end:
	ret
endp

;рисование точек в 3d
align 4
draw_3d:
	cmp dword[obj_poi_count],2
	jl .end_f
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
	stdcall [glPushMatrix]
	bt dword[draw_mode],bit_light
	jnc @f
		call SetLight
		jmp .end_l
	@@:
		stdcall [glDisable],GL_LIGHTING
	.end_l:
	stdcall [glTranslatef], 0.0,0.0,0.5 ;correct for z
	stdcall [glScalef], [obj_x_scale],[obj_y_scale],[obj_z_scale]
	stdcall [glScalef], 1.0,1.0,0.7 ;correct for z
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0
	stdcall [glTranslatef], [obj_x_centr],[obj_y_centr],[obj_z_centr]

	;рисование точек
	bt dword[draw_mode],bit_vertexes
	jnc .end_points
		stdcall [glColor3ub],[color_vert+2],[color_vert+1],[color_vert]
		stdcall [glBegin],GL_POINTS
		mov eax,[obj_poi_data]
		mov ebx,[obj_poi_count]
		@@:
			stdcall [glVertex3f], [eax],[eax+4],[eax+8]
			add eax,12
			dec ebx
			jnz @b
		stdcall [glEnd]
	.end_points:

	;рисование выделенной точки
	stdcall [tl_node_get_data],tree3
	cmp eax,0
	je @f
		;выделение активной точки
		mov eax,[eax]

		stdcall [glBegin],GL_LINES
		stdcall [glColor3ub],[color_ox+2],[color_ox+1],[color_ox]
		stdcall [glVertex3f], [obj_x_min],[eax+4],[eax+8]
		stdcall [glVertex3f], [obj_x_max],[eax+4],[eax+8]
		stdcall [glColor3ub],[color_oy+2],[color_oy+1],[color_oy]
		stdcall [glVertex3f], [eax],[obj_y_min],[eax+8]
		stdcall [glVertex3f], [eax],[obj_y_max],[eax+8]
		stdcall [glColor3ub],[color_oz+2],[color_oz+1],[color_oz]
		stdcall [glVertex3f], [eax],[eax+4],[obj_z_min]
		stdcall [glVertex3f], [eax],[eax+4],[obj_z_max]
		stdcall [glEnd]

		stdcall [glColor3f], 1.0, 1.0, 0.0
		stdcall [glPushMatrix]
			stdcall [glTranslatef], [eax],[eax+4],[eax+8]
			stdcall [gluSphere], [qObj], [sph_radius], 8,8
		stdcall [glPopMatrix]
	@@:

	;рисование граней
	bt dword[draw_mode],bit_faces
	jnc .end_triangles
	cmp dword[obj_tri_count],0
	je .end_triangles
	cmp dword[obj_poi_count],0
	je .end_triangles
		bt dword[draw_mode],bit_faces_fill
		jc @f
			push GL_LINE
			jmp .end_0
		@@:
			push GL_FILL
		.end_0:
		stdcall [glPolygonMode], GL_FRONT_AND_BACK ;mode - уже в стеке
		stdcall [glColor3ub],[color_face+2],[color_face+1],[color_face]
		stdcall [glBegin],GL_TRIANGLES
		mov eax,[obj_tri_data]
		mov ecx,[obj_tri_count]
		@@:
			movzx ebx,word[eax] ;1-я вершина
			imul ebx,12
			add ebx,[obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+2] ;2-я вершина
			imul ebx,12
			add ebx,[obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+4] ;3-я вершина
			imul ebx,12
			add ebx,[obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			add eax,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
			dec ecx
			jnz @b
		stdcall [glEnd]
	.end_triangles:

	stdcall [glPopMatrix]
	stdcall [kosglSwapBuffers]
	.end_f:
	ret

align 4
SetLight:
	stdcall [glLightfv], GL_LIGHT0, GL_POSITION, light_position
	stdcall [glLightfv], GL_LIGHT0, GL_SPOT_DIRECTION, light_dir

	stdcall [glLightfv], GL_LIGHT0, GL_DIFFUSE, white_light
	stdcall [glLightfv], GL_LIGHT0, GL_SPECULAR, white_light

	stdcall [glEnable], GL_COLOR_MATERIAL
	stdcall [glColorMaterial], GL_FRONT, GL_AMBIENT_AND_DIFFUSE
	stdcall [glMaterialfv], GL_FRONT, GL_SPECULAR, mat_specular
	stdcall [glMaterialf], GL_FRONT, GL_SHININESS, mat_shininess
	stdcall [glLightModelfv], GL_LIGHT_MODEL_AMBIENT, lmodel_ambient

	stdcall [glEnable],GL_LIGHTING
	stdcall [glEnable],GL_LIGHT0
	ret

align 4
proc prop_key uses eax ebx
	mcall 2

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
		call draw_3d
		jmp .end
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		call draw_3d
		jmp .end
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		jmp .end
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		;jmp .end
	@@:
	.end:
	ret
endp

align 4
prop_mouse:
	push eax ebx
	mcall 37,3
	bt eax,0
	jnc .end_m
		;mouse l. but. move
		cmp dword[mouse_drag],1
		jne .end_m
		mcall 37,1 ;get mouse coords
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

		call draw_3d
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
		mcall 37,1 ;get mouse coords
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
	mcall 17 ;получить код нажатой кнопки

	cmp ah,3
	jne @f
		call get_point_coords
		call draw_3d
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
			stdcall dword[tl_node_set_data], tree3,obj_point
			stdcall dword[tl_draw], tree3
			mov byte[can_save],1 ;для показа кнопки 'Сохранить'
			mov dword[offs_last_timer],0 ;для обновления блока в таймере
			call draw_3d
		.no_select_1:
		pop esi edi ecx ebx eax
		jmp prop_still.end
	@@:
	cmp ah,5
	jne @f
		call mnu_vertexes_on_off
		jmp prop_still.end
	@@:
	cmp ah,6
	jne @f
		call mnu_faces_on_off
		jmp prop_still.end
	@@:
	cmp ah,7
	jne @f
		call mnu_faces_fill_on_off
		jmp prop_still.end
	@@:
	cmp ah,8
	jne @f
		call mnu_light_on_off
		jmp prop_still.end
	@@:
	cmp ah,9
	jne @f
		call mnu_reset_settings
		jmp prop_still.end
	@@:

	cmp ah,1
	jne prop_still.end
.exit:
	mov dword[tree3.data_img],0 ;чистим указатели на иконки что-бы их не удалил деструктор
	mov dword[tree3.data_img_sys],0 ;т. к. они еще используються в других элементах
	stdcall dword[tl_data_clear], tree3
	mov byte[prop_wnd_run],0 ;обнуляем счетчик окон
	mcall -1 ;выход из программы

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
		call draw_3d
	@@:
	ret
endp

;изменяем режим рисования вершин
align 4
mnu_vertexes_on_off:
	xor dword[draw_mode], 1 shl bit_vertexes
	ret

;изменяем режим рисования граней
align 4
mnu_faces_on_off:
	xor dword[draw_mode], 1 shl bit_faces
	ret

;изменяем каркасные или сплошные грани
align 4
mnu_faces_fill_on_off:
	xor dword[draw_mode], 1 shl bit_faces_fill
	or  dword[draw_mode], 1 shl bit_faces ;включаем режим рисования граней (если вдруг был отключен)
	ret

;включаем или отключаем освещение
align 4
mnu_light_on_off:
	xor dword[draw_mode], 1 shl bit_light
	ret

align 4
mnu_reset_settings:
	mov dword[angle_x],0.0
	mov dword[angle_y],0.0
	mov dword[angle_z],0.0
	m2m dword[draw_mode],dword[def_dr_mode]
	ret

align 4
def_dr_mode dd 0 ;режим рисования по умолчанию
delt_size dd 3.0 ;изменение углов при поворотах с кливиатуры
mouse_drag dd 0 ;режим поворота сцены от перемещении курсора мыши
mouse_x dd 0
mouse_y dd 0
angle_dxm dd 1.7775 ;~ 3d_wnd_w/180 - прибавление углов поворота сцены при вращении мышей
angle_dym dd 1.3335 ;~ 3d_wnd_h/180
;angle_dzm dd ?
draw_mode dd 0 ;режим рисования объекта
obj_poi_data dd 0 ;указатель на координаты вершин
obj_poi_count dd 0 ;колличество вершин
obj_tri_data dd 0 ;указатель на данные треугольников
obj_tri_count dd 0 ;колличество треугольников
cursor_last_draw dd 0 ;положение курсора при последней прорисовке 3d окна
sph_radius dd 0 ;радиус сферы для выделения активной точки
obj_x_max dd 0
obj_x_min dd 0
obj_x_centr dd 0
obj_x_scale dd 1.0
obj_y_max dd 0
obj_y_min dd 0
obj_y_centr dd 0
obj_y_scale dd 1.0
obj_z_max dd 0
obj_z_min dd 0
obj_z_centr dd 0
obj_z_scale dd 1.0

capt_p dd 0

;дерево с объектами в пользовательском файле
tree3 tree_list MAX_OBJECT_SIZE,3, tl_key_no_edit+tl_list_box_mode,\
	16,16, 0xffffff,0xb0d0ff,0x400040, 5,290,303,140, 16, 4,0, el_focus,\
	w_scr_t3,get_point_coords

edit1 edit_box 80,  76, 269, 0xffd0d0, 0xff, 0x80ff, 0, 0x8000, 34, string1, mouse_dd, 0
edit2 edit_box 80, 160, 269, 0xd0d0ff, 0xff, 0x80ff, 0, 0x8000, 34, string2, mouse_dd, 0
edit3 edit_box 80, 244, 269, 0xd0ffd0, 0xff, 0x80ff, 0, 0x8000, 34, string3, mouse_dd, 0
editboxes_end:

string1 rb 34
string2 rb 34
string3 rb 34

align 4
w_scr_t3 scrollbar 16, 0,0,0, 15, 100,0,0, 0,0,0, 1
