;
; структуры и функции для рисования 3d объектов
;

include '../../develop/libraries/TinyGL/asm_fork/zbuffer.inc'

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
	mat1_data dd ? ;данные 1-го материала в объекте
ends

struct material
	name dd ?
	col_ambient dd ? ;цвет материала
	col_diffuse dd ?
	col_specular dd ?
ends

bit_vertexes equ 0
bit_faces equ 1 ;грани рисуются контурами
bit_faces_fill equ 2 ;грани с заливкой
bit_light equ 3
bit_faces_mat equ 4 ;грани с цветом материала
bit_smooth equ 5 ;грани со сглаживанием

ini_name db 'info3ds.ini',0
ini_sec_w3d db 'Window 3d',0
key_dv  db 'draw_vertices',0
key_df  db 'draw_faces',0
key_dff db 'draw_faces_fill',0
key_dfm db 'draw_faces_material',0
key_dl  db 'draw_light',0
key_ds  db 'draw_smooth',0
key_ox  db 'col_ox',0
key_oy  db 'col_oy',0
key_oz  db 'col_oz',0
key_bk  db 'col_bkgnd',0
key_vert db 'col_vertices',0
key_face db 'col_faces',0
key_select db 'col_select',0

txt_err_save_img_file:
if lang eq ru
	db '"Не могу сохранить файл." -W',0
else
	db '"Can',39,'t save file." -W',0
end if

align 4
rad_c dd 150 ;для вычисления радиуса
png_data dd ?
png_size dd ?

align 4
proc hex_in_str, buf:dword,val:dword,zif:dword
pushad
	mov edi,dword[buf]
	mov ecx,dword[zif]
	add edi,ecx
	dec edi
	mov ebx,dword[val]

	.cycle:
		mov al,bl
		and al,0xf
		cmp al,10
		jl @f
			add al,'a'-'0'-10
		@@:
		add al,'0'
		mov byte[edi],al
		dec edi
		shr ebx,4
	loop .cycle
popad
	ret
endp

;установка всех основных переменных по умолчанию
align 4
proc obj_clear_param uses edi, o_data:dword
	mov edi,[o_data]
	;обнуление указателей на данные объектов
	mov dword[edi+obj_3d.poi_data],0
	mov dword[edi+obj_3d.poi_count],0
	mov dword[edi+obj_3d.tri_data],0
	mov dword[edi+obj_3d.tri_count],0
	mov dword[edi+obj_3d.normals_tri_count],0
	mov dword[edi+obj_3d.normals_poi_count],0
	mov dword[edi+obj_3d.mat1_data],0
	cmp dword[edi+obj_3d.normals_tri_data],0
	je @f
		stdcall mem.Free,[edi+obj_3d.normals_tri_data]
		mov dword[edi+obj_3d.normals_tri_data],0
	@@:
	cmp dword[edi+obj_3d.normals_poi_data],0
	je @f
		stdcall mem.Free,[edi+obj_3d.normals_poi_data]
		mov dword[edi+obj_3d.normals_poi_data],0
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
	cmp dword[edi+obj_3d.poi_count],0
	je .end
		finit
		mov eax,[edi+obj_3d.poi_data]
		mov ebx,dword[eax]
		mov [edi+obj_3d.x_min],ebx
		mov [edi+obj_3d.x_max],ebx
		mov ebx,dword[eax+4]
		mov [edi+obj_3d.y_min],ebx
		mov [edi+obj_3d.y_max],ebx
		mov ebx,dword[eax+8]
		mov [edi+obj_3d.z_min],ebx
		mov [edi+obj_3d.z_max],ebx

	mov ebx,[edi+obj_3d.poi_data]
	mov esi,[edi+obj_3d.poi_count]
align 4
	.cycle_0: ;работа с координатой x
		fld dword[ebx]
		fld dword[edi+obj_3d.x_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx]
			mov [edi+obj_3d.x_min],eax
			ffree st0
			fincstp
			jmp .next_x
		@@:
		fld dword[edi+obj_3d.x_max]
		fcompp
		fstsw ax
		sahf
		ja .next_x
			mov eax,[ebx]
			mov [edi+obj_3d.x_max],eax
		.next_x:

		add ebx,12
		dec esi
		jnz .cycle_0

		fld1
		fld dword[edi+obj_3d.x_max]
		fsub dword[edi+obj_3d.x_min]
		fst dword[edi+obj_3d.x_centr]
		fdivp ;1/size.x
		fstp dword[edi+obj_3d.x_scale]

		fld dword[edi+obj_3d.x_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.x = size.x/2
		fadd dword[edi+obj_3d.x_min]
		fchs
		fstp dword[edi+obj_3d.x_centr]

	mov ebx,[edi+obj_3d.poi_data]
	mov esi,[edi+obj_3d.poi_count]
align 4
	.cycle_1: ;работа с координатой y
		fld dword[ebx+4]
		fld dword[edi+obj_3d.y_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+4]
			mov [edi+obj_3d.y_min],eax
			ffree st0
			fincstp
			jmp .next_y
		@@:
		fld dword[edi+obj_3d.y_max]
		fcompp
		fstsw ax
		sahf
		ja .next_y
			mov eax,[ebx+4]
			mov [edi+obj_3d.y_max],eax
		.next_y:

		add ebx,12
		dec esi
		jnz .cycle_1

		fld1
		fld dword[edi+obj_3d.y_max]
		fsub dword[edi+obj_3d.y_min]
		fst dword[edi+obj_3d.y_centr]
		fdivp ;1/size.y
		fstp dword[edi+obj_3d.y_scale]

		fld dword[edi+obj_3d.y_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.y = size.y/2
		fadd dword[edi+obj_3d.y_min]
		fchs
		fstp dword[edi+obj_3d.y_centr]

	mov ebx,[edi+obj_3d.poi_data]
	mov esi,[edi+obj_3d.poi_count]
align 4
	.cycle_2: ;работа с координатой z
		fld dword[ebx+8]
		fld dword[edi+obj_3d.z_min]
		fcomp
		fstsw ax
		sahf
		jb @f
			mov eax,[ebx+8]
			mov [edi+obj_3d.z_min],eax
			ffree st0
			fincstp
			jmp .next_z
		@@:
		fld dword[edi+obj_3d.z_max]
		fcompp
		fstsw ax
		sahf
		ja .next_z
			mov eax,[ebx+8]
			mov [edi+obj_3d.z_max],eax
		.next_z:

		add ebx,12
		dec esi
		jnz .cycle_2

		fld1
		fld dword[edi+obj_3d.z_max]
		fsub dword[edi+obj_3d.z_min]
		fst dword[edi+obj_3d.z_centr]
		fdivp ;1/size.z
		fstp dword[edi+obj_3d.z_scale]

		fld dword[edi+obj_3d.z_centr]
		fld1
		fld1
		faddp
		fdivp ;centr.z = size.z/2
		fadd dword[edi+obj_3d.z_min]
		fchs
		fstp dword[edi+obj_3d.z_centr]

		;находим радиус для сферы
		fld dword[edi+obj_3d.x_max]
		fsub dword[edi+obj_3d.x_min]
		fld dword[edi+obj_3d.y_max]
		fsub dword[edi+obj_3d.y_min]
		faddp
		fild dword[rad_c]
		fdivp ;radius=(size.x+size.y)/rad_c
		fstp dword[sph_radius]

	;делаем одинаковый масштаб по осям x и y, что-бы не нарушались пропорции изображения
	fld dword[edi+obj_3d.y_scale]
	fdiv dword[ratio] ;приведение масштаба по оси y учитывая что по высоте экран меньше
	fcomp dword[edi+obj_3d.x_scale]
	fstsw ax
	sahf
	jb @f
		;сжатие масштаба по y
		fld dword[edi+obj_3d.x_scale]
		fmul dword[ratio]
		fstp dword[edi+obj_3d.y_scale] ;масштаб по оси y = масштаб по оси x
		jmp .end
	@@:
		;сжатие масштаба по x >...<
		fld	dword[edi+obj_3d.x_scale]
		fdiv dword[ratio]
		fstp dword[edi+obj_3d.x_scale]
	.end:
	ret
endp

;description:
; вычисление нормалей для граней по 3-м вершинам
; а также нормалей для сглаживания к вершинам
align 4
proc obj_set_normals, o_data:dword
locals
	tbl1 rd 9 ;ebp-72
	tbl2 rd 9 ;ebp-36
endl
pushad
	mov ecx,[o_data]
	cmp dword[ecx+obj_3d.poi_count],0
	je .end
		mov eax,[ecx+obj_3d.tri_count]
		mov [ecx+obj_3d.normals_tri_count],eax ;на каждый треугольник 1 нормаль
		cmp eax,0
		je .end
		finit
		imul eax,12
		stdcall mem.Alloc,eax
		mov [ecx+obj_3d.normals_tri_data],eax

		mov edx,[ecx+obj_3d.tri_data]
		mov ebx,[ecx+obj_3d.tri_count]
		mov ecx,[ecx+obj_3d.poi_data]
align 4
		@@:
			lea edi,[ebp-36] ;tbl2
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

			lea esi,[ebp-72] ;tbl1
			sub edi,36 ;tbl2
			;v1 <-> v2
			fld dword[edi+12]
			fsub dword[edi]
			fstp dword[esi] ;(x2-x1)
			fld dword[edi+16]
			fsub dword[edi+4]
			fstp dword[esi+4] ;(y2-y1)
			fld dword[edi+20]
			fsub dword[edi+8]
			fstp dword[esi+8] ;(z2-z1)
			;v2 <-> v3
			fld dword[edi+12]
			fsub dword[edi+24]
			fstp dword[esi+12] ;(x2-x3)
			fld dword[edi+16]
			fsub dword[edi+28]
			fstp dword[esi+16] ;(y2-y3)
			fld dword[edi+20]
			fsub dword[edi+32]
			fstp dword[esi+20] ;(z2-z3)
			;v3 <-> v1
			fld dword[edi+24]
			fsub dword[edi]
			fstp dword[esi+24] ;(x3-x1)
			fld dword[edi+28]
			fsub dword[edi+4]
			fstp dword[esi+28] ;(y3-y1)
			fld dword[edi+32]
			fsub dword[edi+8]
			fstp dword[esi+32] ;(z3-z1)

			;A = (y2-y1) * (z3-z1) - (z2-z1) * (y3-y1)
			fld dword[esi+4]
			fmul dword[esi+32]
			fld dword[esi+8]
			fmul dword[esi+28]
			fsubp
			fstp dword[eax] ;set normal.a

			;B = (z2-z1) * (x3-x1) - (x2-x1) * (z3-z1)
			fld dword[esi+8]
			fmul dword[esi+24]
			fld dword[esi]
			fmul dword[esi+32]
			fsubp
			fstp dword[eax+4] ;set normal.b

			;C = (x2-x1) * (y3-y1) - (y2-y1) * (x3-x1)
			fld dword[esi]
			fmul dword[esi+28]
			fld dword[esi+4]
			fmul dword[esi+24]
			fsubp
			fstp dword[eax+8] ;set normal.c

			stdcall V3_Norm,eax

			add edx,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
			add eax,12
			dec ebx
			jnz @b

		;нормали к вершинам
		mov ebx,[o_data]
		mov ecx,[ebx+obj_3d.poi_count]
		mov [ebx+obj_3d.normals_poi_count],ecx
		cmp ecx,0
		je .end
		imul ecx,12
		stdcall mem.Alloc,ecx
		mov [ebx+obj_3d.normals_poi_data],eax

		mov edi,eax
		mov eax,0.0
		shr ecx,2
		rep stosd ;очистка 0-ми float

		mov edx,[ebx+obj_3d.tri_count]
		mov edi,[ebx+obj_3d.tri_data]
		mov eax,[ebx+obj_3d.normals_tri_data]
		mov ecx,[ebx+obj_3d.poi_data]
		mov ebx,[ebx+obj_3d.normals_poi_data]
align 4
		.cycle_0: ;по граням
			movzx esi,word[edi] ;1-я вершина
			imul esi,12
			add esi,ecx
			push esi
			movzx esi,word[edi+2] ;2-я вершина
			imul esi,12
			add esi,ecx
			push esi
			movzx esi,word[edi+4] ;3-я вершина
			imul esi,12
			add esi,ecx
			push esi

			movzx esi,word[edi] ;1-я вершина
			imul esi,12
			add esi,ebx
			push esi
			movzx esi,word[edi+2] ;2-я вершина
			imul esi,12
			add esi,ebx
			push esi
			movzx esi,word[edi+4] ;3-я вершина
			imul esi,12
			add esi,ebx
			push esi

			stdcall poi_normal_add,eax

			add edi,8
			add eax,12
			dec edx
			jnz .cycle_0
		.cycle_0_end:
	.end:
popad
	ret
endp

;input:
; norm_tr - указатель на нормаль к грани
; n1, n2, n3 - указатели на нормали вершин
; c1, c2, c3 - указатели на координаты вершин
align 4
proc poi_normal_add uses eax ebx,\
	norm_tr:dword, n1:dword, n2:dword, n3:dword, c1:dword, c2:dword, c3:dword

	mov ebx,[norm_tr]

	mov eax,[n1]
	fld dword[eax]
	fadd dword[ebx]
	fstp dword[eax]
	fld dword[eax+4]
	fadd dword[ebx+4]
	fstp dword[eax+4]
	fld dword[eax+8]
	fadd dword[ebx+8]
	fstp dword[eax+8]

	mov eax,[n2]
	fld dword[eax]
	fadd dword[ebx]
	fstp dword[eax]
	fld dword[eax+4]
	fadd dword[ebx+4]
	fstp dword[eax+4]
	fld dword[eax+8]
	fadd dword[ebx+8]
	fstp dword[eax+8]

	mov eax,[n3]
	fld dword[eax]
	fadd dword[ebx]
	fstp dword[eax]
	fld dword[eax+4]
	fadd dword[ebx+4]
	fstp dword[eax+4]
	fld dword[eax+8]
	fadd dword[ebx+8]
	fstp dword[eax+8]

	ret
endp

;description:
; normalize vector
align 4
proc V3_Norm uses eax ebx, a:dword
	mov ebx,[a]
	fld dword[ebx]
	fmul st0,st0
	fld dword[ebx+4]
	fmul st0,st0
	faddp
	fld dword[ebx+8]
	fmul st0,st0
	faddp
	fsqrt ;st0 = sqrt(a.X^2 +a.Y^2 +a.Z^2)
	ftst
	fstsw ax
	sahf
	je .r1 ;if (sqrt(...)==0) return 1
		fld dword[ebx] ;offs_X = 0
		fdiv st0,st1
		fstp dword[ebx] ;a.X/=sqrt(...)
		fld dword[ebx+4]
		fdiv st0,st1
		fstp dword[ebx+4] ;a.Y/=sqrt(...)
		fld dword[ebx+8]
		fdiv st0,st1
		fstp dword[ebx+8] ;a.Z/=sqrt(...)
	.r1:
	ffree st0
	fincstp
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
			mov [edi+obj_3d.poi_data],ebx
			mov [edi+obj_3d.poi_count],edx

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
			mov [edi+obj_3d.tri_count],edx
			add esi,8 ;2+4+2 (chunk+size+count)
			mov [edi+obj_3d.tri_data],esi

			;поиск материалов граней
			shl edx,3
			add esi,edx
			cmp esi,ecx
			jge .end_points ;если нет блока с материалом
			cmp word[esi],CHUNK_FACEMAT
			jne .end_points
				mov [edi+obj_3d.mat1_data],esi
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

;настройка материала
;input:
; m_data - указатель на заполняемую структуру с материалом
; f_data - указательна на блок CHUNK_MATERIAL из файла 3ds
align 4
proc mat_init, m_data:dword, f_data:dword
	pushad
	mov edi,[m_data]

	;... clear edi ...
	mov dword[edi+material.name],0
	mov dword[edi+material.col_ambient],0
	mov dword[edi+material.col_diffuse],0
	mov dword[edi+material.col_specular],0

	xor edx,edx
	mov ebx,[f_data]
	xor ecx,ecx
	cmp word[ebx],CHUNK_MATERIAL
	jne .no_material
		mov ecx,ebx
		add ecx,dword[ecx+2] ;граница блока
		add ebx,6
	cmp word[ebx],CHUNK_MATNAME
	jne .no_material
		;*** (0) ***
		mov esi,ebx
		add ebx,6
		mov [edi+material.name],ebx
		sub ebx,6
		;*** (1) ***
		.cycle_0: ;цикл для поиска цвета материала
			add esi,dword[esi+2]
			cmp word[esi],0xa010
			je @f
				cmp ecx,esi
				jg .cycle_0 ;пока не достигнута грница старшего блока
				jmp .end_material ;если не нашли цвет материала
		@@:
		;если нашли цвет материала
		add esi,6
		cmp word[esi],0x0011
		jne .end_material
			add esi,6
			mov dl,byte[esi]
			mov byte[edi+material.col_ambient+2],dl
			mov dx,word[esi+1]
			ror dx,8
			mov word[edi+material.col_ambient],dx
		;*** (2) ***
		mov esi,ebx
		.cycle_1: ;цикл для поиска цвета материала
			add esi,dword[esi+2]
			cmp word[esi],0xa020
			je @f
				cmp ecx,esi
				jg .cycle_1 ;пока не достигнута грница старшего блока
				jmp .end_material ;если не нашли цвет материала
		@@:
		;если нашли цвет материала
		add esi,6
		cmp word[esi],0x0011
		jne .end_material
			add esi,6
			mov dl,byte[esi]
			mov byte[edi+material.col_diffuse+2],dl
			mov dx,word[esi+1]
			ror dx,8
			mov word[edi+material.col_diffuse],dx
		;*** (3) ***
		mov esi,ebx
		.cycle_2: ;цикл для поиска цвета материала
			add esi,dword[esi+2]
			cmp word[esi],0xa030
			je @f
				cmp ecx,esi
				jg .cycle_2 ;пока не достигнута грница старшего блока
				jmp .end_material ;если не нашли цвет материала
		@@:
		;если нашли цвет материала
		add esi,6
		cmp word[esi],0x0011
		jne .end_material
			add esi,6
			mov dl,byte[esi]
			mov byte[edi+material.col_specular+2],dl
			mov dx,word[esi+1]
			ror dx,8
			mov word[edi+material.col_specular],dx

		.end_material:
	.no_material:
	popad
	ret
endp

align 4
proc buf_draw_beg uses eax, buf:dword
	mov eax,[buf]
	stdcall [buf2d_clear], eax, [eax+16] ;чистим буфер
	stdcall [buf2d_draw_text], eax, buf_1,txt_open_3ds,5,5,0xff
	stdcall [buf2d_draw_text], eax, buf_1,openfile_path,5,15,0xff
	cmp dword[level_stack],FILE_ERROR_CHUNK_SIZE ;возможна ошибка файла
	jne @f
		stdcall [buf2d_draw_text], eax, buf_1,txt_3ds_err_sizes,5,25,0xff0000
	@@:
	ret
endp

;рисование материала
align 4
proc draw_material uses edi, m_data:dword
	mov edi,[m_data]
	stdcall buf_draw_beg, buf_ogl
	stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_afff,5,35,0xff

	stdcall [buf2d_draw_text], buf_ogl,buf_1,[edi+material.name],15,45,0 ;имя материала
	stdcall draw_color, [edi+material.col_ambient], txt_a010,2,55 ;окружающий цвет
	stdcall draw_color, [edi+material.col_diffuse], txt_a020,2,75 ;дифузный цвет
	stdcall draw_color, [edi+material.col_specular],txt_a030,2,95 ;зеркальный цвет

	stdcall [buf2d_draw], buf_ogl
	ret
endp

align 4
proc draw_color uses eax ebx, color:dword, text:dword, c_x:dword, c_y:dword
	mov eax,[c_x]
	mov ebx,[c_y]
	; прямоугольник
	inc ebx
	stdcall [buf2d_filled_rect_by_size], buf_ogl,eax,ebx,30,17,[color]
	dec ebx
	stdcall [buf2d_rect_by_size], buf_ogl,eax,ebx,30,19, 0
	; подписи
	add eax,33
	stdcall [buf2d_draw_text], buf_ogl, buf_1,[text],eax,ebx,0
	add ebx,10
	stdcall hex_in_str, buffer,[color],6
	mov byte[buffer+6],0
	stdcall [buf2d_draw_text], buf_ogl, buf_1,buffer,eax,ebx,0
	ret
endp

;рисование точек в 3d
align 4
proc draw_3d uses eax ebx ecx edi, o_data:dword
	mov edi,[o_data]
	cmp dword[edi+obj_3d.poi_count],2
	jl .end_f
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
	call [glPushMatrix]
	bt dword[draw_mode],bit_light
	jnc @f
		call SetLight
		jmp .end_l
	@@:
		stdcall [glDisable],GL_LIGHTING
	.end_l:
	stdcall [glTranslatef], 0.0,0.0,0.5 ;correct for z
	stdcall [glScalef], [edi+obj_3d.x_scale],[edi+obj_3d.y_scale],[edi+obj_3d.z_scale]
	stdcall [glScalef], 1.0,1.0,0.7 ;correct for z
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0
	stdcall [glTranslatef], [edi+obj_3d.x_centr],[edi+obj_3d.y_centr],[edi+obj_3d.z_centr]

if version_edit eq 0
	;рисование граней с использованием цветов материалов
	bt dword[draw_mode],bit_faces_mat
	jnc @f
		mov eax,[edi+obj_3d.mat1_data]
		or eax,eax
		jnz .b_found
			stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_mat_null,5,3,[color_select]
			jmp .mat_null ;если нет материала, рисуем все грани одним цветом
		.b_found:
			mov ebx,eax
			stdcall get_mat_color, eax
			stdcall draw_3d_faces_color, edi,ebx,eax
			stdcall found_mat_faces, ebx
			or eax,eax
			jnz .b_found
		jmp .end_points
		
	@@:
end if

	;рисование граней без цветов материалов
	bt dword[draw_mode],bit_faces
	jnc .end_triangles
	.mat_null:
	cmp dword[edi+obj_3d.tri_count],0
	je .end_triangles
	cmp dword[edi+obj_3d.poi_count],0
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
		
		bt dword[draw_mode],bit_smooth
		jnc .triangles_flat
		bt dword[draw_mode],bit_light
		jnc .triangles_flat
			;triangles smooth
			stdcall [glBegin],GL_TRIANGLES
			mov eax,[edi+obj_3d.tri_data]
			mov ecx,[edi+obj_3d.tri_count]
			mov edx,[edi+obj_3d.normals_tri_data]
align 4
			@@:
				movzx ebx,word[eax] ;1-я вершина
				imul ebx,12
					add ebx,[edi+obj_3d.normals_poi_data]
					stdcall [glNormal3fv], ebx
					sub ebx,[edi+obj_3d.normals_poi_data]
				add ebx,[edi+obj_3d.poi_data]
				stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
				movzx ebx,word[eax+2] ;2-я вершина
				imul ebx,12
					add ebx,[edi+obj_3d.normals_poi_data]
					stdcall [glNormal3fv], ebx
					sub ebx,[edi+obj_3d.normals_poi_data]
				add ebx,[edi+obj_3d.poi_data]
				stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
				movzx ebx,word[eax+4] ;3-я вершина
				imul ebx,12
					add ebx,[edi+obj_3d.normals_poi_data]
					stdcall [glNormal3fv], ebx
					sub ebx,[edi+obj_3d.normals_poi_data]
				add ebx,[edi+obj_3d.poi_data]
				stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
				add eax,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
				dec ecx
				jnz @b
			stdcall [glEnd]
			jmp .end_points

		.triangles_flat:
		stdcall [glBegin],GL_TRIANGLES
		mov eax,[edi+obj_3d.tri_data]
		mov ecx,[edi+obj_3d.tri_count]
		mov edx,[edi+obj_3d.normals_tri_data]
align 4
		@@:
			;
			bt dword[draw_mode],bit_light
			jnc .norm1
				stdcall [glNormal3fv], edx
				add edx,12
			.norm1:
			movzx ebx,word[eax] ;1-я вершина
			imul ebx,12
			add ebx,[edi+obj_3d.poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+2] ;2-я вершина
			imul ebx,12
			add ebx,[edi+obj_3d.poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			movzx ebx,word[eax+4] ;3-я вершина
			imul ebx,12
			add ebx,[edi+obj_3d.poi_data]
			stdcall [glVertex3f], [ebx],[ebx+4],[ebx+8]
			add eax,8 ;размер треугольника = (3 вершины (по 2б) + свойства (2б))
			dec ecx
			jnz @b
		stdcall [glEnd]
		jmp .end_points
	.end_triangles:

	;рисование точек
	bt dword[draw_mode],bit_vertexes
	jnc .end_points
		stdcall [glColor3ub],[color_vert+2],[color_vert+1],[color_vert]
		stdcall [glBegin],GL_POINTS
		mov eax,[edi+obj_3d.poi_data]
		mov ebx,[edi+obj_3d.poi_count]
		mov edx,[edi+obj_3d.normals_poi_data]
align 4
		@@:
			bt dword[draw_mode],bit_light
			jnc .norm0
			or edx,edx
			jz .norm0 ;если нормалей нет
				stdcall [glNormal3fv], edx
				add edx,12
			.norm0:
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
		stdcall [glVertex3f], [edi+obj_3d.x_min],[eax+4],[eax+8]
		stdcall [glVertex3f], [edi+obj_3d.x_max],[eax+4],[eax+8]
		stdcall [glColor3ub], [color_oy+2],[color_oy+1],[color_oy]
		stdcall [glVertex3f], [eax],[edi+obj_3d.y_min],[eax+8]
		stdcall [glVertex3f], [eax],[edi+obj_3d.y_max],[eax+8]
		stdcall [glColor3ub], [color_oz+2],[color_oz+1],[color_oz]
		stdcall [glVertex3f], [eax],[eax+4],[edi+obj_3d.z_min]
		stdcall [glVertex3f], [eax],[eax+4],[edi+obj_3d.z_max]
		stdcall [glEnd]

		stdcall [glColor3ub],[color_select+2],[color_select+1],[color_select]
		call [glPushMatrix]
			stdcall [glTranslatef], [eax],[eax+4],[eax+8]
			stdcall [gluSphere], [qObj], [sph_radius], 8,8
		call [glPopMatrix]
	@@:

	;рисование выделенных точек
	cmp dword[obj_poi_sel_c],0
	je .end_select
		mov ecx,dword[obj_poi_sel_c]
		stdcall [glColor3ub],[color_select+2],[color_select+1],[color_select]
		stdcall [tl_node_poi_get_info], tree3,0
align 4
		@@:
			cmp eax,0
			je .end_select
			mov ebx,eax
			cmp word[ebx],ID_ICON_POINT_SEL
			jne .next_sel
			stdcall [tl_node_poi_get_data], tree3,ebx
			mov eax,[eax]
			call [glPushMatrix]
				stdcall [glTranslatef], [eax],[eax+4],[eax+8]
				stdcall [gluSphere], [qObj], [sph_radius], 4,4
			call [glPopMatrix]
			dec ecx
			jz .end_select
			.next_sel:
			stdcall [tl_node_poi_get_next_info], tree3,ebx
			jmp @b
    .end_select:
end if

	call [glPopMatrix]
if version_edit eq 1
	cmp dword[obj_poi_sel_c],0
	je @f
		stdcall [buf2d_draw_text], buf_ogl, buf_1,txt_select_vert,5,3,[color_select]
	@@:
end if
	call [kosglSwapBuffers]
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
	and dword[draw_mode], not ((1 shl bit_faces)+(1 shl bit_faces_mat)) ;выключаем режимы рисования, которые могут мешать
	call update_obj
	ret

;включаем режим рисования каркасных граней
align 4
mnu_edges_on:
	or dword[draw_mode], 1 shl bit_faces
	and dword[draw_mode], not ((1 shl bit_vertexes)+(1 shl bit_faces_fill)+(1 shl bit_faces_mat))
	call update_obj
	ret

;включаем режим рисования сплошных граней
align 4
mnu_faces_on:
	or dword[draw_mode], (1 shl bit_faces)+(1 shl bit_faces_fill)
	and dword[draw_mode], not ((1 shl bit_vertexes)+(1 shl bit_faces_mat))
	call update_obj
	ret

;включаем или отключаем освещение
align 4
mnu_light_on_off:
	xor dword[draw_mode], 1 shl bit_light
	call update_obj
	ret

align 4
mnu_smooth_on_off:
	xor dword[draw_mode], 1 shl bit_smooth
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

align 4
mnu_make_scrshot:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.3 ;.png
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],1
	jne .end_save_file
		;код при удачном открытии диалога
		mov dword[png_data],0

		;create image struct
		stdcall [img_create], [buf_ogl.w], [buf_ogl.h], Image.bpp24
		mov ebx,eax
		test eax,eax
		jz @f
			;copy foto to image buffer
			mov edi,[eax+Image.Data]
			mov esi,[buf_ogl]
			mov ecx,[buf_ogl.w]
			mov edx,[buf_ogl.h]
			imul ecx,edx
			imul ecx,3
			shr ecx,2 ;OpenGL buffer align to 4
			rep movsd

			;encode image
			stdcall [img_encode], eax, LIBIMG_FORMAT_PNG, 0
			test eax,eax
			jz @f
				mov [png_data],eax
				mov [png_size],ecx
		@@:
		stdcall [img_destroy],ebx

        ; заполняем структуру для сохранения файла
        mov ebx,run_file_70
        mov dword[ebx],SSF_CREATE_FILE
        mov eax,[png_size]
        mov [ebx+12],eax ;file size
        mov eax,[png_data]
        mov [ebx+16],eax
        mov dword[ebx+FileInfoBlock.FileName], openfile_path

        mcall SF_FILE,run_file_70
        test eax,eax
        jnz .save_error
			;notify_window_run openfile_path
			jmp @f
		.save_error:
			;сообщение о неудачном сохранении
			notify_window_run txt_err_save_img_file
		@@:
		mcall SF_SYS_MISC, SSF_MEM_FREE, [png_data]
	.end_save_file:
	popad
	ret

if version_edit eq 1

align 4
update_obj:
	stdcall draw_3d, o3d
	ret

else

;включаем режим рисования сплошных граней
align 4
mnu_faces_mat:
	or dword[draw_mode], (1 shl bit_faces_mat)
	call update_obj
	ret

align 4
update_obj:
push eax ebx
	stdcall [tl_node_get_data],tree1
	cmp eax,0
	je @f
		mov ebx,dword[eax] ;получаем значение сдвига выбранного блока относительно начала файла
		add ebx,dword[open_file_data] ;получаем значение сдвига в памяти
		cmp word[ebx],CHUNK_OBJBLOCK
		jne @f
			add eax,list_offs_obj3d
			stdcall draw_3d,eax
	@@:
pop ebx eax
	ret

;input:
; p_mat - указатель на данные предыдущего материала
align 4
proc found_mat_faces uses ebx, p_mat:dword
	mov eax,[p_mat]
	or eax,eax
	jz @f
	add eax,[eax+2]
	cmp word[eax],CHUNK_FACEMAT
	je @f
		xor eax,eax
	@@:
	ret
endp

align 4
proc get_mat_color uses ebx ecx edx edi esi, p_mat:dword
	stdcall [tl_node_poi_get_info], tree1,0
	mov ecx,eax
	.cycle_0:
		cmp ecx,0
		je .cycle_0_end
			stdcall [tl_node_poi_get_data], tree1,ecx
			mov ebx,[eax] ;получаем значение сдвига выбранного блока
			add ebx,[open_file_data]
			cmp word[ebx],CHUNK_MATERIAL
			jne .end_add_p3
			add eax,list_offs_obj3d
			mov esi,[eax+material.name]
			cmp esi,0
			jne @f
				;init material
				stdcall mat_init,eax,ebx
				mov esi,[eax+material.name]
			@@:
			mov edi,[p_mat]
			mov eax,5
			add edi,eax
align 4
			@@:
				cmp al,0
				je .found
				lodsb
				inc edi
				cmp byte[edi],al
				je @b
		.end_add_p3:
		stdcall [tl_node_poi_get_next_info], tree1,ecx
		mov ecx,eax ;переходим к следущему узлу
		jmp .cycle_0
	.cycle_0_end:

	.no_found:
		xor eax,eax
		jmp .end_f
	.found:
		stdcall [tl_node_poi_get_data], tree1,ecx
		add eax,list_offs_obj3d
		mov eax,[eax+material.col_diffuse]
	.end_f:
	ret
endp

align 4
proc draw_3d_faces_color o_data:dword, p_mat:dword, color:dword
pushad
	stdcall [glPolygonMode], GL_FRONT_AND_BACK,GL_FILL
	stdcall [glColor3ub],[color+2],[color+1],[color]
	stdcall [glBegin],GL_TRIANGLES
	mov edi,[o_data]
	mov esi,[p_mat]
	add esi,6
	@@:
		lodsb
		cmp al,0
		jne @b
	movzx ecx,word[esi] ;ecx - число граней с данным материалом
	mov edx,[edi+obj_3d.poi_data]

	bt dword[draw_mode],bit_smooth
	jnc @f
	bt dword[draw_mode],bit_light
	jnc @f
align 4
	.cycle_0:
		add esi,2
		;word[esi] - номер треугольника
		movzx ebx,word[esi]

		shl ebx,3
		add ebx,[edi+obj_3d.tri_data]
	
		movzx eax,word[ebx] ;1-я вершина
		imul eax,12 ;float(x,y,z)
			add eax,[edi+obj_3d.normals_poi_data]
			stdcall [glNormal3fv], eax
			sub eax,[edi+obj_3d.normals_poi_data]
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]

		movzx eax,word[ebx+2] ;2-я вершина
		imul eax,12 ;float(x,y,z)
			add eax,[edi+obj_3d.normals_poi_data]
			stdcall [glNormal3fv], eax
			sub eax,[edi+obj_3d.normals_poi_data]
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]

		movzx eax,word[ebx+4] ;3-я вершина
		imul eax,12 ;float(x,y,z)
			add eax,[edi+obj_3d.normals_poi_data]
			stdcall [glNormal3fv], eax
			sub eax,[edi+obj_3d.normals_poi_data]
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]
		
		dec ecx
		jnz .cycle_0
	stdcall [glEnd]
	jmp .end_f

align 4
	@@:
		add esi,2
		;word[esi] - номер треугольника
		movzx ebx,word[esi]
		;нормали для освещения
		bt dword[draw_mode],bit_light
		jnc .norm
			mov eax,ebx
			imul eax,12
			add eax,[edi+obj_3d.normals_tri_data]
			stdcall [glNormal3fv], eax
		.norm:

		shl ebx,3
		add ebx,[edi+obj_3d.tri_data]
	
		movzx eax,word[ebx] ;1-я вершина
		imul eax,12 ;float(x,y,z)
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]

		movzx eax,word[ebx+2] ;2-я вершина
		imul eax,12 ;float(x,y,z)
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]

		movzx eax,word[ebx+4] ;3-я вершина
		imul eax,12 ;float(x,y,z)
		add eax,edx
		stdcall [glVertex3f], [eax],[eax+4],[eax+8]
		
		dec ecx
		jnz @b
	stdcall [glEnd]

.end_f:
popad
	ret
endp

end if

align 4
draw_mode dd 0 ;режим рисования объекта
def_dr_mode dd 0 ;режим рисования по умолчанию
delt_size dd 3.0 ;изменение углов при поворотах с кливиатуры
mouse_drag dd 0 ;режим поворота сцены от перемещении курсора мыши
mouse_x dd 0
mouse_y dd 0
sph_radius dd 0 ;радиус сферы для выделения активной точки

