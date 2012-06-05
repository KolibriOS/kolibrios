;

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4]
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

vox_offs_tree_table equ 4
vox_offs_data equ 12

;description:
; функция рисующая одиночный воксел
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; buf_v - буфер с изображением вокселя (32 бита)
; v_color - цвет
align 4
proc draw_vox, buf_i:dword, buf_z:dword, buf_v:dword,\
coord_x:dword, coord_y:dword, coord_z:dword, v_color:dword
pushad
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[buf_v]
	mov ecx,buf2d_h
	mov esi,buf2d_w
	imul ecx,esi
	add esi,eax
	mov edx,buf2d_data
	cld
	;ecx - count pixels in voxel
	;edx - указатель на данные в воксельном буфере
	;edi - указатель на воксельный буфер
	;esi - width voxel buffer add coord x
	.cycle:
		cmp dword[edx],0
		je @f
			;проверяем буфер глубины
			push eax
			stdcall [buf2d_get_pixel], [buf_z],eax,ebx
			sub eax,[coord_z]
			cmp eax,[edx]
			jl .dr_pixel
				pop eax
				jmp @f
			.dr_pixel:
				;рисуем точку
				pop eax
				stdcall [buf2d_set_pixel], [buf_i],eax,ebx,[v_color]
				push ecx
				mov ecx,[coord_z]
				add ecx,[edx]
				stdcall [buf2d_set_pixel], [buf_z],eax,ebx,ecx
				pop ecx
		@@:
		add edx,4
		inc eax
		cmp eax,esi
		jl @f
			inc ebx
			sub eax,buf2d_w
		@@:
		loop .cycle
popad
	ret
endp

;description:
; функция рисующая воксельный объект
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; h_br - кисть с изображениями вокселей (32 бита)
; v_obj - воксельный объект
; k_scale - коэф. для масштабирования изображения
align 4
proc buf2d_vox_obj_draw_3g, buf_i:dword, buf_z:dword, h_br:dword, v_obj:dword,\
coord_x:dword, coord_y:dword, coord_z:dword, k_scale:dword
pushad
	mov edi,[v_obj]
	mov eax,[k_scale]
	mov ebx,[coord_x]

;---
	;тестовая рамка
	mov ecx,[h_br]

	movzx edx,byte[ecx]
	movzx esi,byte[ecx+1]
	cmp eax,1
	jl .end_c0
		mov ecx,eax
		shl edx,cl
		shl esi,cl
	.end_c0:
	stdcall [buf2d_rect_by_size], [buf_i], ebx,[coord_y],edx,esi, 0xd0d0d0
;---

	mov ecx,[coord_y]
	mov edx,eax
	add edi,vox_offs_data

	mov esi,[coord_z]
	stdcall vox_go_in_node, [buf_i], [buf_z], [h_br], [v_obj]

popad
	ret
endp

;description:
; функция рисующая тени
;input:
; buf_i - буфер в котором рисуется (24 бита)
; buf_z - буфер глубины (32 бита по числу пикселей должен совпадать с buf_i)
; h_br - кисть с изображениями вокселей (32 бита)
; k_scale - коэф. для масштабирования изображения
align 4
proc buf2d_vox_obj_draw_3g_shadows, buf_i:dword, buf_z:dword, h_br:dword, \
coord_x:dword, coord_y:dword, color:dword, k_scale:dword, prop:dword
locals
	correct_z dd 0 ;коректировка для буфера глубины
endl
pushad
	mov eax,[k_scale]
	add eax,[prop]
	mov dword[correct_z],8
	sub [correct_z],eax
	mov ebx,[coord_x]
	;correct_z = 8-k_scale-prop

	stdcall buf2d_vox_obj_get_img_w_3g, [h_br],[k_scale]
	mov edx,eax ;edx - ширина изображения
	stdcall buf2d_vox_obj_get_img_h_3g, [h_br],[k_scale]
	mov esi,eax

	mov edi,[coord_y]
	mov ecx,edx
	add edx,ebx ;ширина + отступ слева
	imul ecx,esi
	cld
	.cycle_0:
		stdcall [buf2d_get_pixel], [buf_z],ebx,edi
		cmp eax,0
		je @f
			stdcall vox_correct_z, [correct_z]
			push eax
			stdcall [buf2d_get_pixel], [buf_i],ebx,edi
			stdcall combine_colors_3,eax,[color] ;,eax
			stdcall [buf2d_set_pixel], [buf_i],ebx,edi,eax
		@@:
		inc ebx
		cmp ebx,edx
		jl @f
			mov ebx,[coord_x]
			inc edi
		@@:
		loop .cycle_0

popad
	ret
endp

;output:
; eax - scaled coord z
align 4
proc vox_correct_z uses ecx, correct_z:dword
	mov ecx,[correct_z]
	cmp ecx,0
	je .end_f
	jl .end_0
		shl eax,cl
		jmp .end_f
	.end_0:
		neg ecx
		inc ecx
		shr eax,cl
	.end_f:
	ret
endp

;output:
; eax - color
align 4
proc combine_colors_3 uses ebx ecx edx edi esi, col_0:dword, col_1:dword, alpha:dword

	mov ebx,[col_0]
	mov ecx,[col_1]
	movzx di,byte[alpha] ;pro
	mov si,0x00ff ;---get transparent---
	sub si,di ;256-pro

	;---blye---
	movzx ax,bl
	imul ax,si
	movzx dx,cl
	imul dx,di
	add ax,dx
	mov cl,ah
	;---green---
	movzx ax,bh
	imul ax,si
	movzx dx,ch
	imul dx,di
	add ax,dx
	mov ch,ah
	shr ebx,16
	ror ecx,16
	;---red---
	movzx ax,bl
	imul ax,si
	movzx dx,cl
	imul dx,di
	add ax,dx

	shl eax,8
	ror ecx,16
	mov ax,cx
	and eax,0xffffff

	ret
endp

;input:
; h_br - кисть с изображениями вокселей (32 бита)
; ebx - coord_x
; ecx - coord_y
; esi - coord_z
; edx - уровень текушего узла
; edi - указатель на данные воксельного объекта
align 4
proc vox_go_in_node, buf_i:dword, buf_z:dword, h_br:dword, v_obj:dword
;, coord_z:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees
		;рекурсивный перебор поддеревьев
		push eax ecx

		;прорисовка рамки если размер узла = 1
		cmp edx,0
		jne .end_2
			push eax
				stdcall vox_get_sub_brush,[h_br],0 ;определяем кисть для рисования
				cmp eax,0 ;если кисть не найдена
				je @f
					stdcall draw_vox, [buf_i], [buf_z], eax, ebx,ecx,esi, [edi]
				@@:
			pop eax
		.end_2:

		;вход внутрь узла
		dec edx
;---
		push ebx
			;mov eax,(7-4/2)
			mov ebx,[h_br]
			movzx eax,byte[ebx+1]
			movzx ebx,byte[ebx+2]
			shr ebx,1
			sub eax,ebx
		cmp edx,1
		jl .end_c1
		push ecx
			mov ecx,edx
			shl eax,cl
			shl ebx,cl ;???
		pop ecx
		.end_c1:
		add esi,ebx ;???
		pop ebx
		add ecx,eax ;коректировка высоты под воксель нижнего уровня
;---
		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push ebx ecx esi
				stdcall vox_corect_coords, [h_br], [v_obj]
				stdcall vox_go_in_node, [buf_i], [buf_z], [h_br], [v_obj]
				pop esi ecx ebx
			.c_next:
			shr ah,1
			dec al
			jnz .cycle

		;выход из узла
		inc edx
		pop ecx eax

		jmp .end_f
	.sub_trees:
		;рисуем узел
		push eax
			stdcall vox_get_sub_brush,[h_br],edx ;определяем кисть для рисования
			cmp eax,0 ;если кисть не найдена
			je @f
				stdcall draw_vox, [buf_i], [buf_z], eax, ebx,ecx,esi, [edi]
			@@:
		pop eax

		add edi,4
	.end_f:
	ret
endp

;извлекаем из h_br указатель на буфер с изображением вокселя, указанного порядка n
align 4
proc vox_get_sub_brush uses ebx ecx, h_br:dword, n:dword
	xor eax,eax
	mov ebx,[n]
	cmp ebx,0
	jl @f
	mov ecx,[h_br]
	cmp bl,byte[ecx+3]
	jg @f
		add ecx,4
		imul ebx,BUF_STRUCT_SIZE
		mov eax,ebx
		add eax,ecx
	@@:
	ret
endp

;функция для коректировки координат
;направления осей координат в вокселе:
;*z
;|
;+
;  * y
; /
;+
; \
;  * x
;input:
;  al - номер узла в дереве (от 1 до 8)
; ebx - координата x
; ecx - координата y
; esi - координата z
; edx - уровень текушего узла
;output:
; ebx - новая координата x
; ecx - новая координата y
; esi - новая координата z
align 4
proc vox_corect_coords, h_br:dword, v_obj:dword
locals
	osn_w_2 dd ? ;ширина основания единичного вокселя : 2
	vox_h dd ? ;высота единичного вокселя
endl
	cmp edx,0
	jl .end_f ;для ускорения отрисовки

	push eax edi
	and eax,15 ;выделяем номер узла в дереве
	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	add edi,8
	sub edi,eax

	push ebx ecx
		mov ebx,[h_br]

		movzx ecx,byte[ebx]
		shr ecx,1
		mov dword[osn_w_2],ecx

		movzx ecx,byte[ebx+2]
		movzx ebx,byte[ebx+1]
		sub ebx,ecx
		mov dword[vox_h],ebx
		shr ecx,1
		mov eax,ecx ;eax - высота основания единичного вокселя : 2
	pop ecx ebx

	cmp edx,1
	jl .no_scale ;во избежание зацикливания
	push ecx
		mov ecx,edx
		shl eax,cl
		shl dword[osn_w_2],cl
		shl dword[vox_h],cl
	pop ecx
	.no_scale:

;	add esi,eax ;меняем глубину для буфера z (компенсация для координаты y)
	bt word[edi],0 ;test voxel coord x
	jnc @f
		add ebx,[osn_w_2]
		add ecx,eax
		add esi,eax ;меняем глубину для буфера z
	@@:
	bt word[edi],1 ;test voxel coord y
	jnc @f
		add ebx,[osn_w_2]
		sub ecx,eax
		sub esi,eax ;меняем глубину для буфера z
	@@:
	bt word[edi],2 ;test voxel coord z
	jnc @f
		sub ecx,[vox_h]
	@@:
	pop edi eax
	.end_f:
	ret
endp

;
;x0y0 - x1y0
;x1y0 - x1y1
;x0y1 - x0y0
;x1y1 - x0y1
align 4
proc vox_obj_rot_z uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table
	mov ecx,2
	cld
	@@:
		mov eax,dword[ebx]
		mov byte[ebx+1],al
		mov byte[ebx+3],ah
		shr eax,16
		mov byte[ebx],al
		mov byte[ebx+2],ah
		add ebx,4
		loop @b
	ret
endp

;
;y0z0 - y0z1
;y0z1 - y1z1
;y1z1 - y1z0
;y1z0 - y0z0
align 4
proc vox_obj_rot_x uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table
	mov eax,dword[ebx]
	mov ecx,dword[ebx+4]

	mov byte[ebx+4],al
	mov byte[ebx+5],ah
	shr eax,16
	mov byte[ebx+0],al
	mov byte[ebx+1],ah

	mov byte[ebx+6],cl
	mov byte[ebx+7],ch
	shr ecx,16
	mov byte[ebx+2],cl
	mov byte[ebx+3],ch

	ret
endp

;рисование среза изображения
; v_size - размер квадрата с вокселем
; k_scale - степень детализации изображения
; n_plane - номер плоскости сечния (в пределах от 0 до 2^k_scale - 1)
; b_color - цвет границы
align 4
proc buf2d_vox_obj_draw_pl, buf_i:dword, v_obj:dword, coord_x:dword,\
coord_y:dword, v_size:dword, k_scale:dword, n_plane:dword, b_color:dword
	cmp [k_scale],0
	jl .end_f
pushad
	mov eax,[v_size]
	mov ecx,[k_scale]
	mov ebx,eax
	cmp ecx,1
	jl @f
		shl ebx,cl
	@@:
	;ebx - полный размер изображения
	stdcall [buf2d_rect_by_size], [buf_i], [coord_x],[coord_y],ebx,ebx, [b_color] ;рамка на рисунок
	add ebx,[coord_y]
	mov edx,[n_plane]
	imul edx,eax
	add edx,[coord_x]
	stdcall [buf2d_filled_rect_by_size], [buf_i], edx,ebx, eax,eax, [b_color] ;квадратик, показывающий номер сечения

	;рисование точек для сетки
	push ecx
	mov edi,1
	cmp ecx,1
	jl @f
		shl edi,cl
	@@:
	dec edi
	cmp edi,1
	jl .end_0
	mov ecx,edi
	imul ecx,edi
	mov ebx,[coord_x]
	mov edx,[coord_y]
	add edx,eax
	xor esi,esi
	cld
	@@:
		add ebx,eax
		inc esi
		stdcall [buf2d_set_pixel], [buf_i], ebx,edx, [b_color]
		cmp esi,edi
		jl .end_1
			;переход точек на новую строку
			xor esi,esi
			mov ebx,[coord_x]
			add edx,eax
		.end_1:
		loop @b
	.end_0:
	pop ecx

	;eax - размер одного квадрата
	;edi - указатель рисуемые данные из объекта
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor esi,esi
	push eax
	imul eax,[n_plane]
	stdcall draw_sub_vox_obj_pl, [buf_i],[v_obj], eax
popad
	.end_f:
	ret
endp

;input:
; ebx - coord_x
; edx - coord_y
; esi - coord_z
; ecx - уровень текушего узла
; edi - указатель на данные воксельного объекта
align 4
proc draw_sub_vox_obj_pl, buf_i:dword, v_obj:dword, clip_z:dword,\
v_size:dword
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;прорисовка рамки если размер узла = 1
		cmp ecx,0
		jne @f
		;проверка глубины esi
		;clip_z=n_plane*v_size
		stdcall vox_is_clip, [clip_z],[v_size]
		cmp eax,0
		je @f
			push ecx
			mov ecx,dword[edi]
			and ecx,0xffffff
			stdcall [buf2d_rect_by_size], [buf_i], ebx,edx, [v_size],[v_size],ecx
			pop ecx
		@@:

		;рекурсивный перебор поддеревьев
		push edx

		;вход внутрь узла
		dec ecx
;---
		mov eax,[v_size]
		cmp ecx,1
		jl @f
			shl eax,cl
		@@:
		add edx,eax ;коректировка высоты под воксель нижнего уровня
;---
		mov ah,byte[edi+3]
		add edi,4
		mov al,8
		.cycle:
			bt ax,8 ;тестируем только ah
			jnc .c_next
				push eax ebx edx esi
				stdcall vox_corect_coords_pl, [v_obj],[v_size]
				stdcall draw_sub_vox_obj_pl, [buf_i],[v_obj],[clip_z],[v_size]
				pop esi edx ebx eax
			.c_next:
			shr ah,1
			dec al
			jnz .cycle

		;выход из узла
		inc ecx
		pop edx

		jmp .end_f
	.sub_trees:
		cmp ecx,0
		jl .end_0 ;не рисуем очень маленькие воксели

		;проверка глубины esi
		;clip_z=n_plane*v_size
		stdcall vox_is_clip, [clip_z],[v_size]
		cmp eax,0
		je .end_0

		;рисуем узел
		mov eax,[edi]
		and eax,0xffffff
		push eax ;цвет узла

		mov eax,[v_size]
		cmp ecx,1
		jl @f
			shl eax,cl ;размер узла
		@@:

		stdcall [buf2d_filled_rect_by_size], [buf_i], ebx,edx, eax,eax
		.end_0:
		add edi,4
	.end_f:
	ret
endp

;проверка глубины esi
;input:
; ecx - уровень текушего узла
; esi - coord z
; clip_z - n_plane*v_size
;output:
; eax - 0 if no draw, 1 if draw
align 4
proc vox_is_clip uses ebx edi, clip_z:dword, v_size:dword
	xor eax,eax
	mov ebx,[clip_z]
	mov edi,[v_size]
	cmp ecx,1
	jl @f
		shl edi,cl
	@@:
	;edi = 2^ecx
	add edi,esi
	cmp edi,ebx ;if (esi+2^ecx*v_size <= (n_plane*v_size)) no draw
	jle @f
	add ebx,[v_size]
	cmp esi,ebx ;if (esi >= (n_plane+1)*v_size) no draw
	jge @f
		inc eax
	@@:
	ret
endp

;функция для коректировки координат
;направления осей координат в вокселе:
;*z
;|
;+-* x
;input:
;  al - номер узла в дереве (от 1 до 8)
; ebx - координата x
; edx - координата y
; esi - координата z
; ecx - уровень текушего узла
;output:
; ebx - новая координата x
; edx - новая координата y
; esi - новая координата z
align 4
proc vox_corect_coords_pl, v_obj:dword, v_size:dword
	cmp ecx,0
	jl .end_f ;для ускорения отрисовки

	push eax edi
	and eax,15 ;выделяем номер узла в дереве
	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	add edi,8
	sub edi,eax

	mov eax,[v_size] ;eax - высота основания единичного вокселя
	cmp ecx,1
	jl @f ;во избежание зацикливания
		shl eax,cl
	@@:

	bt word[edi],0 ;test voxel coord x
	jnc @f
		add ebx,eax
	@@:
	bt word[edi],1 ;test voxel coord y
	jnc @f
		add esi,eax ;меняем глубину для буфера z
	@@:
	bt word[edi],2 ;test voxel coord z
	jnc @f
		sub edx,eax
	@@:
	pop edi eax
	.end_f:
	ret
endp

;
;output:
; eax - число узлов в объекте v_obj
align 4
proc vox_obj_get_nodes uses edi, v_obj:dword
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor eax,eax
	stdcall vox_obj_rec0
	ret
endp

;input:
; edi - указатель на данные воксельного объекта
;output:
; eax - eax + число узлов в данных вокс. объекта
; edi - указатель на смещенные данные вокс. объекта
align 4
proc vox_obj_rec0
	inc eax
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;рекурсивный перебор поддеревьев
		push ebx ecx
		mov bh,byte[edi+3]
		add edi,4
		mov bl,8
		.cycle:
			bt bx,8 ;тестируем только bh
			jnc .c_next
				stdcall vox_obj_rec0
			.c_next:
			shr bh,1
			dec bl
			jnz .cycle
		pop ecx ebx

		jmp .end_f
	.sub_trees:
		add edi,4
	.end_f:
	ret
endp

;
;output:
; eax - размер в байтах занимаемый объектом v_obj
align 4
proc buf2d_vox_obj_get_size, v_obj:dword
	stdcall vox_obj_get_nodes,[v_obj]
	shl eax,2
	add eax,vox_offs_data
	ret
endp

if 0
;output:
; eax - указатель на данные вокселя
align 4
proc buf2d_vox_obj_is_node uses ebx ecx edx edi esi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov edi,[v_obj]
	add edi,vox_offs_data
	mov esi,[k_scale]
	xor eax,eax
	xor edx,edx
	stdcall vox_obj_rec1, [v_obj],[coord_x],[coord_y],[coord_z], 0,0,0
	ret
endp

;input:
; eax - ...
; edx - ...
; edi - указатель на данные воксельного объекта
; esi - уровень искомого вокселя
align 4
proc vox_obj_rec1, v_obj:dword, coord_x:dword,coord_y:dword,coord_z, tc_x:dword,tc_y:dword,tc_z
	cmp edx,esi 
	jne @f
		mov ebx,[tc_x]
		cmp [coord_x],ebx
		jne @f
		mov ebx,[tc_y]
		cmp [coord_y],ebx
		jne @f
		mov ebx,[tc_z]
		cmp [coord_z],ebx
		jne @f
		mov eax,edi
	@@:

	cmp eax,0
	jne .end_f ;если указатель уже найден

	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees
		inc edx
		shl dword[tc_x],1
		shl dword[tc_y],1
		shl dword[tc_z],1
		;рекурсивный перебор поддеревьев
		
		mov bh,byte[edi+3]
		add edi,4
		;mov bl,8
		xor bl,bl
		.cycle:
			bt bx,8 ;тестируем только bh
			jnc .c_next
push dword[tc_x]
push dword[tc_y]
push dword[tc_z]
				push edx
				movzx edx,bl
				add edx,[v_obj]
				add edx,vox_offs_tree_table
				bt word[edx],0 ;coord x
				jnc .end_x
					inc dword[tc_x]
				.end_x:
				bt word[edx],1 ;coord y
				jnc .end_y
					inc dword[tc_y]
				.end_y:
				bt word[edx],2 ;coord z
				jnc .end_z
					inc dword[tc_z]
				.end_z:
				pop edx
				push ebx ecx
				stdcall vox_obj_rec1, [v_obj],[coord_x],[coord_y],[coord_z], [tc_x],[tc_y],[tc_z]
				pop ecx ebx
pop dword[tc_z]
pop dword[tc_y]
pop dword[tc_x]
			.c_next:
			shr bh,1
			inc bl
			;jnz .cycle
			cmp bl,8
			jl .cycle
		
		dec edx
		jmp .end_f
	.sub_trees:
		add edi,4
	.end_f:
	ret
endp
end if

; создание вокселя в 3 этапа:
; 1) ищем место в структуре дерева, куда надо вставить (если ветвь существует, 2-й этап пропускаем)
; 2) вставляем новую ветвь с вокселем (3-й этап уже не делаем)
; 3) меняем цвет вокселя
align 4
proc buf2d_vox_obj_create_node, v_obj:dword,coord_x:dword,coord_y:dword,\
coord_z:dword,color:dword,k_scale:dword
pushad
locals
	p_node dd 0 ;родительский узел
endl
;stdcall print_err, txt_f1, txt_b

	mov edi,[v_obj]
	add edi,vox_offs_data
	mov esi,[k_scale]
	cmp esi,1
	jl .change
	; *** (1) ***
	.found:
	stdcall vox_obj_get_node_position, [v_obj],[coord_x],[coord_y],[coord_z],esi
	movzx bx,byte[edi+3]
	mov [p_node],edi
	add edi,4
;stdcall print_err, txt_chi, txt_e
	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ; цикл  для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
;stdcall print_err, txt_st, txt_e
			xor eax,eax
			stdcall vox_obj_rec0 ;в eax вычисляется число дочерних узлов, в данной ветви
		.end_0:
		shr bx,1
		loop @b
	.end_1:
	bt bx,0
	jnc .creat ;если поддерева не существует, переходим к созданию
	dec esi
	cmp esi,0
	jg .found
	jmp .change

	; *** (2) ***
	.creat:
	mov edx,[color] ;меняем цвет
	and edx,0xffffff ;для профилактики
	mov ecx,esi
	stdcall vox_obj_add_nodes_mem, [v_obj],edi,ecx ;расширяем место занимаемое объектом, для добавления новых узлов
	mov ebx,[p_node]
	cld
	@@:
		mov dword[edi],edx
		stdcall vox_obj_get_node_bit_mask, [v_obj],[coord_x],[coord_y],[coord_z],esi
		or byte[ebx+3],al

		mov ebx,edi
		add edi,4
		dec esi
		loop @b
	jmp .end_2

	; *** (3) ***
	.change:
	mov eax,[color] ;меняем цвет
	mov word[edi],ax
	shr eax,16
	mov byte[edi+2],al

	.end_2:
popad
	ret
endp

; удаление вокселя в 3 этапа:
; 1) ищем место в структуре дерева, где есть удаляемый узел (если ветвь не существует, выходим из функции)
; 2) вставляем новую ветвь с вокселем
align 4
proc buf2d_vox_obj_delete_node, v_obj:dword,coord_x:dword,coord_y:dword,\
coord_z:dword,k_scale:dword
pushad
locals
	p_node dd 0 ;родительский узел
endl
	mov edi,[v_obj]
	add edi,vox_offs_data
	mov esi,[k_scale]
	cmp esi,1
	jl .end_2
	; *** (1) ***
	.found:
	stdcall vox_obj_get_node_position, [v_obj],[coord_x],[coord_y],[coord_z],esi
	movzx bx,byte[edi+3]
	mov [p_node],edi
	add edi,4
	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ; цикл  для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
			xor eax,eax
			stdcall vox_obj_rec0 ;в eax вычисляется число дочерних узлов, в данной ветви
		.end_0:
		shr bx,1
		loop @b
	.end_1:
	bt bx,0
	jnc .end_2 ;если поддерева не существует
	dec esi
	cmp esi,0
	jg .found

	; *** (2) ***
	;delete
	stdcall vox_obj_del_nodes_mem, [v_obj],edi ;сужаем место занимаемое объектом, при удалении узла
	mov ebx,[p_node]

	stdcall vox_obj_get_node_bit_mask, [v_obj],[coord_x],[coord_y],[coord_z],esi
	xor byte[ebx+3],al

	.end_2:
popad
	ret
endp

; сдвигает узлы для добавления новых узлов
;input:
; p_insert - позиция для вставки
; count - колличество вставляемых узлов
align 4
proc vox_obj_add_nodes_mem uses eax ecx edi esi, v_obj:dword,p_insert:dword,count:dword
	stdcall buf2d_vox_obj_get_size,[v_obj]
	mov esi,[v_obj]
	add esi,eax ;esi - указатель на конец файла
	mov edi,[count]
	shl edi,2
	add edi,esi ;edi - указатель на будущий конец файла
	mov ecx,esi
	sub ecx,[p_insert]
	shr ecx,2 ;ecx - число циклов для копирования
	sub esi,4 ;esi - указатель на последний узел
	sub edi,4 ;edi - указатель на будущий последний узел
	std
	rep movsd ;сдвигаем память
	ret
endp

;input:
; p_insert - позиция для ...
align 4
proc vox_obj_del_nodes_mem uses eax ecx edi esi, v_obj:dword,p_insert:dword
	stdcall buf2d_vox_obj_get_size,[v_obj]
	mov ecx,eax
	mov edi,[p_insert]
	add ecx,[v_obj]
	sub ecx,edi
	xor eax,eax
	stdcall vox_obj_rec0
	shr ecx,2
	sub ecx,eax ;ecx - число циклов для копирования
	shl eax,2
	add eax,[p_insert]
	mov esi,eax
	mov edi,[p_insert]
	cld
	rep movsd ;сдвигаем память
	ret
endp

; определение позиции узла в дереве (от 0 до 7)
align 4
proc vox_obj_get_node_position uses ebx ecx edi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov ecx,[k_scale]
	dec ecx
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[coord_z]
	cmp ecx,1
	jl .end_0
		shr eax,cl
		shr ebx,cl
		shr edi,cl
	.end_0:
	and eax,1
	bt ebx,0
	jnc @f
		bts eax,1
	@@:
	bt edi,0
	jnc @f
		bts eax,2
	@@:

	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	@@:
		cmp al,byte[edi]
		je @f
		inc edi
		jmp @b
	@@:
	sub edi,[v_obj]
	sub edi,vox_offs_tree_table
	mov eax,edi
	
	ret
endp

;output:
; eax - 1,2,4,8,16, ... ,128
align 4
proc vox_obj_get_node_bit_mask uses ebx ecx edi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov ecx,[k_scale]
	dec ecx
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[coord_z]
	cmp ecx,1
	jl .end_0
		shr eax,cl
		shr ebx,cl
		shr edi,cl
	.end_0:
	and eax,1
	bt ebx,0
	jnc @f
		bts eax,1
	@@:
	bt edi,0
	jnc @f
		bts eax,2
	@@:

	mov ecx,[v_obj]
	add ecx,vox_offs_tree_table
	@@:
		cmp al,byte[ecx]
		je @f
		inc ecx
		jmp @b
	@@:
	mov eax,1 ;устанавливаем первоначальное значение бита
	sub ecx,[v_obj]
	sub ecx,vox_offs_tree_table
	jz @f
		shl eax,cl ;сдвигаем бит
	@@:
	
	ret
endp

align 4
proc buf2d_vox_obj_get_img_w_3g uses ecx, h_br:dword,k_scale:dword
	mov ecx,[h_br]

	movzx eax,byte[ecx]
	cmp dword[k_scale],1
	jl .end_c0
		mov ecx,[k_scale]
		shl eax,cl
	.end_c0:
	ret
endp

align 4
proc buf2d_vox_obj_get_img_h_3g uses ecx, h_br:dword,k_scale:dword
	mov ecx,[h_br]

	movzx eax,byte[ecx+1]
	cmp dword[k_scale],1
	jl .end_c0
		mov ecx,[k_scale]
		shl eax,cl
	.end_c0:
	ret
endp

;функция для рисования палитры
align 4
proc buf2d_draw_palete, buf:dword, coord_x:dword, coord_y:dword,\
cell_w:dword, cell_h:dword, colors_w:dword, colors:dword
pushad
	mov ebx,[coord_x]
	mov edx,[coord_y]
	mov ecx,[colors]
	xor edi,edi
	mov esi,[cell_h]
	cld
	.cycle_0:
		stdcall get_palete_color, ecx,3
		stdcall [buf2d_filled_rect_by_size], [buf], ebx,edx, [cell_w],esi,eax
		inc edi
		cmp edi,[colors_w]
		jl @f
			mov ebx,[coord_x]
			sub ebx,[cell_w]
			dec ebx
			add edx,esi ;высота ячейки
			inc edx ;отступ между ячейками
			xor edi,edi
		@@:
		add ebx,[cell_w]
		inc ebx
		loop .cycle_0
popad
	ret
endp

;input:
; если r = 1 то функция возвращает 8 цветов, 2 - 64, 3 - 512
;output:
; eax - color
align 4
proc get_palete_color uses ebx ecx edx edi, ind:dword, r:dword
	mov ecx,[r]
	xor edi,edi
	inc edi

	mov ebx,[ind]
	;b = b xor (b shr 1)
	;преобразуем индекс в кодировку Грея для получения более плавных цветовых переходов
	mov eax,ebx
	shr eax,1
	xor ebx,eax

	mov edx,ebx
	shr edx,cl
	mov eax,edx
	shr eax,cl

	mov ch,8
	sub ch,cl
	shr ecx,8
	shl ebx,cl
	shl edx,cl
	shl eax,cl

	shl edi,cl
	dec edi ;edi - 1...1b

	or ebx,edi
	or edx,edi
	or eax,edi

	shl eax,8
	mov al,dl
	shl eax,8
	mov al,bl
	and eax,0xffffff
	ret
endp


if 0
txt_f1 db 13,10,'f1',0
txt_chi db ' chi',0
txt_st db '  st',0

txt_b db 'beg'
txt_e db 13,10,0

align 4
proc print_err, fun:dword, mes:dword ;выводим сообщение об шибке на доску отладки
	pushad
	mov eax,63
	mov ebx,1

	mov esi,[fun]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	mov cl,':'
	int 0x40
	mov cl,' '
	int 0x40
	mov esi,[mes]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	popad
	ret
endp
end if
