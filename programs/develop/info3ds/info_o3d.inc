;
; структуры и функции для рисования 3d объектов
;

offs_zbuf_pbuf equ 24 ;const. from 'zbuffer.inc'

struct obj_3d
	poi_data dd ? ;указатель на координаты вершин
	poi_count dd ? ;колличество вершин
	tri_data dd ? ;указатель на данные треугольников
	tri_count dd ? ;колличество треугольников
	normals_poi_data dd ? ;данные нормалей к точкам
	normals_poi_count dd ? ;колличество нормалей к точкам
	normals_tri_data dd ? ;данные нормалей к граням
	normals_tri_count dd ? ;колличество нормалей к граням
	x_max dd ?
	x_min dd ?
	x_centr dd ?
	x_scale dd ?
	y_max dd ?
	y_min dd ?
	y_centr dd ?
	y_scale dd ?
	z_max dd ?
	z_min dd ?
	z_centr dd ?
	z_scale dd ?
ends

offs_obj_poi_data equ 0 ;указатель на координаты вершин
offs_obj_poi_count equ 4 ;колличество вершин
offs_obj_tri_data equ 8 ;указатель на данные треугольников
offs_obj_tri_count equ 12 ;колличество треугольников
offs_obj_normals_poi_data equ 16
offs_obj_normals_poi_count equ 20
offs_obj_normals_tri_data equ 24
offs_obj_normals_tri_count equ 28
offs_obj_x_max equ 32
offs_obj_x_min equ 36
offs_obj_x_centr equ 40
offs_obj_x_scale equ 44
offs_obj_y_max equ 48
offs_obj_y_min equ 52
offs_obj_y_centr equ 56
offs_obj_y_scale equ 60
offs_obj_z_max equ 64
offs_obj_z_min equ 68
offs_obj_z_centr equ 72
offs_obj_z_scale equ 76


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
key_select db 'col_select',0


rad_c dd 150 ;для вычисления радиуса


;установка всех основных переменных по умолчанию
align 4
proc obj_clear_param uses edi, o_data:dword
	mov edi,[o_data]
	;обнуление указателей на данные объектов
	mov dword[edi+offs_obj_poi_data],0
	mov dword[edi+offs_obj_poi_count],0
	mov dword[edi+offs_obj_tri_data],0
	mov dword[edi+offs_obj_tri_count],0
	mov dword[edi+offs_obj_normals_tri_count],0
	mov dword[edi+offs_obj_normals_poi_count],0
	cmp dword[edi+offs_obj_normals_tri_data],0
	je @f
		stdcall mem.Free,[edi+offs_obj_normals_tri_data]
		mov dword[edi+offs_obj_normals_tri_data],0
	@@:
	cmp dword[edi+offs_obj_normals_poi_data],0
	je @f
		stdcall mem.Free,[edi+offs_obj_normals_poi_data]
		mov dword[edi+offs_obj_normals_poi_data],0
	@@:
	ret
endp

;поиск данных
;input:
;eax - текущий узел
;output:
;eax - структура текущего узла (или 0 при неудаче)
align 4
proc found_block_data uses ebx ecx edx, tlist:dword
	mov edx,[tlist]
	mov ecx,eax
	stdcall [tl_node_poi_get_info], edx,0
	@@:
		mov ebx,eax
		stdcall [tl_node_poi_get_data], edx,ebx
		cmp eax,ecx
		je @f ;если попали на выбранный узел
		stdcall [tl_node_poi_get_next_info], edx,ebx
		cmp eax,0
		jne @b
		jmp .end
	@@:
		mov eax,ebx
	.end:
	ret
endp

;установка размеров объекта
align 4
proc obj_set_sizes uses eax ebx esi edi, o_data:dword
	mov edi,[o_data]
	cmp dword[edi+offs_obj_poi_count],0
	je .end
		finit
		mov eax,[edi+offs_obj_poi_data]
		mov ebx,dword[eax]
		mov [edi+offs_obj_x_min],ebx
		mov [edi+offs_obj_x_max],ebx
		mov ebx,dword[eax+4]
		mov [edi+offs_obj_y_min],ebx
		mov [edi+offs_obj_y_max],ebx
		mov ebx,dword[eax+8]
		mov [edi+offs_obj_z_min],ebx
		mov [edi+offs_obj_z_max],ebx

	mov ebx,[edi+offs_obj_poi_data]
	mov esi,[edi+offs_obj_poi_count]
align 4
	.cycle_0: ;работа с координатой x
		fld dword[ebx]
		fld dword[edi+offs_obj_x_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx]
			mov [edi+offs_obj_x_min],eax
			ffree st0
			fincstp
			jmp .next_x
		@@:
		fld dword[edi+offs_obj_x_max]
		fcompp
		fstsw ax
		sahf
		ja .next_x
			mov eax,[ebx]
			mov [edi+offs_obj_x_max],eax
		.next_x:

		add ebx,12
		dec esi
		jnz .cycle_0

		fld1
		fld dword[edi+offs_obj_x_max]
		fsub dword[edi+offs_obj_x_min]
		fst dword[edi+offs_obj_x_centr]
		fdivp ;1/size.x
		fstp dword[edi+offs_obj_x_scale]

		fld dword[edi+offs_obj_x_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.x = size.x/2
		fadd dword[edi+offs_obj_x_min]
		fchs
		fstp dword[edi+offs_obj_x_centr]

	mov ebx,[edi+offs_obj_poi_data]
	mov esi,[edi+offs_obj_poi_count]
align 4
	.cycle_1: ;работа с координатой y
		fld dword[ebx+4]
		fld dword[edi+offs_obj_y_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+4]
			mov [edi+offs_obj_y_min],eax
			ffree st0
			fincstp
			jmp .next_y
		@@:
		fld dword[edi+offs_obj_y_max]
		fcompp
		fstsw ax
		sahf
		ja .next_y
			mov eax,[ebx+4]
			mov [edi+offs_obj_y_max],eax
		.next_y:

		add ebx,12
		dec esi
		jnz .cycle_1

		fld1
		fld dword[edi+offs_obj_y_max]
		fsub dword[edi+offs_obj_y_min]
		fst dword[edi+offs_obj_y_centr]
		fdivp ;1/size.y
		fstp dword[edi+offs_obj_y_scale]

		fld dword[edi+offs_obj_y_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.y = size.y/2
		fadd dword[edi+offs_obj_y_min]
		fchs
		fstp dword[edi+offs_obj_y_centr]

	mov ebx,[edi+offs_obj_poi_data]
	mov esi,[edi+offs_obj_poi_count]
align 4
	.cycle_2: ;работа с координатой z
		fld dword[ebx+8]
		fld dword[edi+offs_obj_z_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+8]
			mov [edi+offs_obj_z_min],eax
			ffree st0
			fincstp
			jmp .next_z
		@@:
		fld dword[edi+offs_obj_z_max]
		fcompp
		fstsw ax
		sahf
		ja .next_z
			mov eax,[ebx+8]
			mov [edi+offs_obj_z_max],eax
		.next_z:

		add ebx,12
		dec esi
		jnz .cycle_2

		fld1
		fld dword[edi+offs_obj_z_max]
		fsub dword[edi+offs_obj_z_min]
		fst dword[edi+offs_obj_z_centr]
		fdivp ;1/size.z
		fstp dword[edi+offs_obj_z_scale]

		fld dword[edi+offs_obj_z_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.z = size.z/2
		fadd dword[edi+offs_obj_z_min]
		fchs
		fstp dword[edi+offs_obj_z_centr]

		;находим радиус для сферы
		fld dword[edi+offs_obj_x_max]
		fsub dword[edi+offs_obj_x_min]
		fld dword[edi+offs_obj_y_max]
		fsub dword[edi+offs_obj_y_min]
		faddp
		fild dword[rad_c]
		fdivp ;radius=(size.x+size.y)/rad_c
		fstp dword[sph_radius]

	;делаем одинаковый масштаб по осям x и y, что-бы не нарушались пропорции изображения
	fld dword[edi+offs_obj_y_scale]
	fdiv dword[ratio] ;приведение масштаба по оси y учитывая что по высоте экран меньше
	fcomp dword[edi+offs_obj_x_scale]
	fstsw ax
	sahf
	jb @f
		;сжатие масштаба по y
		fld dword[edi+offs_obj_x_scale]
		fmul dword[ratio]
		fstp dword[edi+offs_obj_y_scale] ;масштаб по оси y = масштаб по оси x
		jmp .end
	@@:
		;сжатие масштаба по x >...<
		fld	dword[edi+offs_obj_x_scale]
		fdiv dword[ratio]
		fstp dword[edi+offs_obj_x_scale]
	.end:
	ret
endp

;установка нормалей для граней
align 4
proc obj_set_normals, o_data:dword
locals
	tbl1 rd 9 ;ebp-72
	tbl2 rd 9 ;ebp-36
endl
pushad
	mov ecx,[o_data]
	cmp dword[ecx+offs_obj_poi_count],0
	je .end
		mov eax,[ecx+offs_obj_tri_count]
		mov [ecx+offs_obj_normals_tri_count],eax ;на каждый треугольник 1 нормаль
		cmp eax,0
		je .end
		finit
		imul eax,12
		stdcall mem.Alloc, eax
		mov [ecx+offs_obj_normals_tri_data],eax

		mov edx,[ecx+offs_obj_tri_data]
		mov ebx,[ecx+offs_obj_tri_count]
		mov ecx,[ecx+offs_obj_poi_data]
align 4
		@@:
			mov edi,ebp
			sub edi,36 ;tbl2
			movzx esi,word[edx] ;1-я вершина
			imul esi,12
			add esi,ecx
			movsd
			movsd
			movsd
			movzx esi,word[edx+2] ;2-я вершина
			imul esi,12
			add esi,ecx
			movsd
			movsd
			movsd
			movzx esi,word[edx+4] ;3-я вершина
			imul esi,12
			add esi,ecx
			movsd
			movsd
			movsd

			mov esi,ebp
			sub esi,72 ;tbl1
			sub edi,36 ;tbl2
			;v1 <-> v2
			fld dword[edi+12]
			fsub dword[edi]
			fstp dword[esi] ;(x1-x2)
			fld dword[edi+16]
			fsub dword[edi+4]
			fstp dword[esi+4] ;(y1-y2)
			fld dword[edi+20]
			fsub dword[edi+8]
			fstp dword[esi+8] ;(z1-z2)
			;v2 <-> v3
			fld dword[edi+24]
			fsub dword[edi+12]
			fstp dword[esi+12] ;(x2-x3)
			fld dword[edi+28]
			fsub dword[edi+16]
			fstp dword[esi+16] ;(y2-y3)
			fld dword[edi+32]
			fsub dword[edi+20]
			fstp dword[esi+20] ;(z2-z3)
			;v3 <-> v1
			fld dword[edi]
			fsub dword[edi+24]
			fstp dword[esi+24] ;(x3-x1)
			fld dword[edi+4]
			fsub dword[edi+28]
			fstp dword[esi+28] ;(y3-y1)
			fld dword[edi+8]
			fsub dword[edi+32]
			fstp dword[esi+32] ;(z3-z1)

			;a1,a2,a3
			fld dword[esi+4]
			fmul dword[esi+8]
			fstp dword[edi] ;a1=(y1-y2)*(z1-z2)
			fld dword[esi+16]
			fmul dword[esi+20]
			fstp dword[edi+4] ;a2=(y2-y3)*(z2-z3)
			fld dword[esi+28]
			fmul dword[esi+32]
			fstp dword[edi+8] ;a3=(y3-y1)*(z3-z1)
			;b1,b2,b3
			fld dword[esi]
			fmul dword[esi+8]
			fstp dword[edi+12] ;b1=(x1-x2)*(z1-z2)
			fld dword[esi+12]
			fmul dword[esi+20]
			fstp dword[edi+16] ;b2=(x2-x3)*(z2-z3)
			fld dword[esi+24]
			fmul dword[esi+32]
			fstp dword[edi+20] ;b3=(x3-x1)*(z3-z1)
			;c1,с2,с3
			fld dword[esi]
			fmul dword[esi+4]
			fstp dword[edi+24] ;c1=(x1-x2)*(y1-y2)
			fld dword[esi+12]
			fmul dword[esi+16]
			fstp dword[edi+28] ;c2=(x2-x3)*(y2-y3)
			fld dword[esi+24]
			fmul dword[esi+28]
			fstp dword[edi+32] ;c3=(x3-x1)*(y3-y1)
			
			fld dword[edi]
			fadd dword[edi+4]
			fadd dword[edi+8]
			fstp dword[eax] ;set normal.a
			fld dword[edi+12]
			fadd dword[edi+16]
			fadd dword[edi+20]
			fstp dword[eax+4] ;set normal.b
			fld dword[edi+24]
			fadd dword[edi+28]
			fadd dword[edi+32]
			fstp dword[eax+8] ;set normal.c
			
			add edx,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
			add eax,12
			dec ebx
			jnz @b
	.end:
popad
	ret
endp

;настройка структуры 3d объекта
align 4
proc obj_init, o_data:dword
	pushad
	mov edi,[o_data]
	stdcall obj_clear_param, edi
	stdcall [tl_node_get_data],tree1
	xor edx,edx
	cmp eax,0
	je .no_points
		mov ebx,[eax] ;получаем значение сдвига выбранного блока
		add ebx,[open_file_data]
		xor ecx,ecx
		cmp word[ebx],CHUNK_OBJBLOCK
		jne @f
			mov ecx,ebx
			add ecx,dword[ecx+2] ;граница блока
			push eax edi
				mov edi,ebx
				add edi,6
				cld
				xor al,al
				repne scasb ;пропуск имени объекта
				mov ebx,edi
			pop edi eax
		@@:
		cmp word[ebx],CHUNK_TRIMESH
		jne @f
			mov ecx,ebx
			add ecx,dword[ecx+2] ;граница блока
			add ebx,6
		@@:
		cmp word[ebx],CHUNK_VERTLIST
		jne .no_points
			mov esi,ebx
			movzx edx,word[ebx+6] ;колличество точек
			add ebx,8
			mov [edi+offs_obj_poi_data],ebx
			mov [edi+offs_obj_poi_count],edx

			.cycle_0: ;цикл для поиска треугольников
				add esi,dword[esi+2]
				cmp word[esi],CHUNK_FACELIST
				je @f
					cmp ecx,esi
					jg .cycle_0 ;пока не достигнута грница старшего блока
					jmp .end_points ;если не нашли треугольники
			@@:
			;если нашли треугольники
			movzx edx,word[esi+6]
			mov [edi+offs_obj_tri_count],edx
			add esi,8 ;2+4+2 (chunk+size+count)
			mov [edi+offs_obj_tri_data],esi
		.end_points:

		stdcall obj_set_sizes, edi
		stdcall obj_set_normals, edi
if version_edit eq 0
		stdcall draw_3d, edi
end if
	.no_points:
	popad
	ret
endp

;рисование точек в 3d
align 4
proc draw_3d uses eax ebx ecx edi, o_data:dword
	mov edi,[o_data]
if 0 ;debug
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT
	mov eax,[edi+offs_obj_poi_data]
	stdcall convert_int_to_str, txt_select_vert.count,16
	stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_select_vert,5,3,[color_select]
	stdcall [kosglSwapBuffers]
	jmp .end_f
end if
	cmp dword[edi+offs_obj_poi_count],2
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
	stdcall [glScalef], [edi+offs_obj_x_scale],[edi+offs_obj_y_scale],[edi+offs_obj_z_scale]
	stdcall [glScalef], 1.0,1.0,0.7 ;correct for z
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0
	stdcall [glTranslatef], [edi+offs_obj_x_centr],[edi+offs_obj_y_centr],[edi+offs_obj_z_centr]

	;рисование точек
	bt dword[draw_mode],bit_vertexes
	jnc .end_points
		stdcall [glColor3ub],[color_vert+2],[color_vert+1],[color_vert]
		stdcall [glBegin],GL_POINTS
		mov eax,[edi+offs_obj_poi_data]
		mov ebx,[edi+offs_obj_poi_count]
		@@:
			stdcall [glVertex3f], [eax],[eax+4],[eax+8]
			add eax,12
			dec ebx
			jnz @b
		stdcall [glEnd]
	.end_points:

if version_edit eq 1
	;рисование активной точки
	stdcall [tl_node_get_data],tree3
	cmp eax,0
	je @f
		;выделение активной точки
		mov eax,[eax]

		stdcall [glBegin],GL_LINES
		stdcall [glColor3ub], [color_ox+2],[color_ox+1],[color_ox]
		stdcall [glVertex3f], [edi+offs_obj_x_min],[eax+4],[eax+8]
		stdcall [glVertex3f], [edi+offs_obj_x_max],[eax+4],[eax+8]
		stdcall [glColor3ub], [color_oy+2],[color_oy+1],[color_oy]
		stdcall [glVertex3f], [eax],[edi+offs_obj_y_min],[eax+8]
		stdcall [glVertex3f], [eax],[edi+offs_obj_y_max],[eax+8]
		stdcall [glColor3ub], [color_oz+2],[color_oz+1],[color_oz]
		stdcall [glVertex3f], [eax],[eax+4],[edi+offs_obj_z_min]
		stdcall [glVertex3f], [eax],[eax+4],[edi+offs_obj_z_max]
		stdcall [glEnd]

		stdcall [glColor3ub],[color_select+2],[color_select+1],[color_select]
		stdcall [glPushMatrix]
			stdcall [glTranslatef], [eax],[eax+4],[eax+8]
			stdcall [gluSphere], [qObj], [sph_radius], 8,8
		stdcall [glPopMatrix]
	@@:

	;рисование выделенных точек
	cmp dword[obj_poi_sel_c],0
	je .end_select
		mov ecx,dword[obj_poi_sel_c]
		stdcall [glColor3ub],[color_select+2],[color_select+1],[color_select]
		stdcall dword[tl_node_poi_get_info], tree3,0
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel
			stdcall [tl_node_poi_get_data], tree3,ebx
			mov eax,[eax]
			stdcall [glPushMatrix]
				stdcall [glTranslatef], [eax],[eax+4],[eax+8]
				stdcall [gluSphere], [qObj], [sph_radius], 4,4
			stdcall [glPopMatrix]
			dec ecx
			jz .end_select
			.next_sel:
			stdcall dword[tl_node_poi_get_next_info], tree3,ebx
			jmp @b
    .end_select:
end if

	;рисование граней
	bt dword[draw_mode],bit_faces
	jnc .end_triangles
	cmp dword[edi+offs_obj_tri_count],0
	je .end_triangles
	cmp dword[edi+offs_obj_poi_count],0
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
		mov eax,[edi+offs_obj_tri_data]
		mov ecx,[edi+offs_obj_tri_count]
		mov edx,[edi+offs_obj_normals_tri_data]
		@@:
			;
			bt dword[draw_mode],bit_light
			jnc .norm
				;[obj_normals_tri_count]
				stdcall [glNormal3fv], edx
				add edx,12
			.norm:
			;
			movzx ebx,word[eax] ;1-я вершина
			imul ebx,12
			add ebx,[edi+offs_obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+2] ;2-я вершина
			imul ebx,12
			add ebx,[edi+offs_obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+4] ;3-я вершина
			imul ebx,12
			add ebx,[edi+offs_obj_poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			add eax,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
			dec ecx
			jnz @b
		stdcall [glEnd]
	.end_triangles:

	stdcall [glPopMatrix]
if version_edit eq 1
	cmp dword[obj_poi_sel_c],0
	je @f
		stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_select_vert,5,3,[color_select]
	@@:
end if
	stdcall [kosglSwapBuffers]
	.end_f:
	ret
endp

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

;включаем режим рисования вершин
align 4
mnu_vertexes_on:
	or dword[draw_mode], 1 shl bit_vertexes
	and dword[draw_mode], not (1 shl bit_faces) ;выключаем режим рисования граней (что-бы не мешали)
	call update_obj
	ret

;включаем режим рисования каркасных граней
align 4
mnu_edges_on:
	or dword[draw_mode], 1 shl bit_faces
	and dword[draw_mode], not ((1 shl bit_vertexes)+(1 shl bit_faces_fill))
	call update_obj
	ret

;включаем режим рисования сплошных граней
align 4
mnu_faces_on:
	or dword[draw_mode], (1 shl bit_faces)+(1 shl bit_faces_fill)
	and dword[draw_mode], not (1 shl bit_vertexes)
	call update_obj
	ret

;включаем или отключаем освещение
align 4
mnu_light_on_off:
	xor dword[draw_mode], 1 shl bit_light
	call update_obj
	ret

align 4
mnu_reset_settings:
	mov dword[angle_x],0.0
	mov dword[angle_y],0.0
	mov dword[angle_z],0.0
	m2m dword[draw_mode],dword[def_dr_mode]
	call update_obj
	ret

if version_edit eq 1

align 4
update_obj:
	stdcall draw_3d, o3d
	ret

else

align 4
update_obj:
push eax
	stdcall [tl_node_get_data],tree1
	cmp eax,0
	je @f
		add eax,14 ;14=list_offs_obj3d
		stdcall draw_3d, eax
	@@:
pop eax
	ret

end if

align 4
draw_mode dd 0 ;режим рисования объекта
def_dr_mode dd 0 ;режим рисования по умолчанию
delt_size dd 3.0 ;изменение углов при поворотах с кливиатуры
mouse_drag dd 0 ;режим поворота сцены от перемещении курсора мыши
mouse_x dd 0
mouse_y dd 0
sph_radius dd 0 ;радиус сферы для выделения активной точки

