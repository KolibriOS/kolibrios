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

;output:
; eax - (event shl 30) + position
align 4
proc get_mouse_ev_scale uses ebx ecx edx, h_br:dword, coord_x:dword, coord_y:dword, scale_obj:dword, scale_img:dword
	mov ecx,[scale_obj]
	stdcall [buf2d_vox_obj_get_img_h_3g], [h_br],ecx
	cmp eax,[coord_y]
	jle .test_x
	stdcall [buf2d_vox_obj_get_img_w_3g], [h_br],ecx
	cmp eax,[coord_x]
	jle .test_z

	.no_change:
	xor eax,eax
	jmp .end_f

	.test_x:
	mov ebx,[h_br]
	movzx ebx,byte[ebx+1]
	add eax,ebx
	cmp eax,[coord_y]
	jl .no_change
	stdcall [buf2d_vox_obj_get_img_w_3g], [h_br],ecx
	cmp eax,[coord_x]
	jl .no_change
	mov ebx,eax
	mov ecx,[scale_img]
	inc ecx
	shr ebx,cl
	xor edx,edx
	mov eax,[coord_x]
	div ebx
	dec ecx
	xor ebx,ebx
	inc ebx
	shl ebx,cl
	cmp eax,ebx
	jge @f
		or eax,1 shl 30
		jmp .end_f
	@@:
	sub eax,ebx
	or eax,2 shl 30
	jmp .end_f

	.test_z:
	mov ebx,[h_br]
	movzx ebx,byte[ebx]
	add eax,ebx
	cmp eax,[coord_x]
	jl .no_change
	mov ebx,[h_br]
	movzx ebx,byte[ebx+1]
	mov ecx,[scale_obj]
	shl ebx,cl
	mov ecx,[scale_img]
	shr ebx,cl
	mov eax,[coord_y]
	xor edx,edx
	div ebx
	;инверсия координаты y
	xor edx,edx
	inc edx
	shl edx,cl
	dec edx
	sub edx,eax
	mov eax,edx
	or eax,3 shl 30

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

;???
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

; создание вокселя в 3 этапа:
; 1) ищем место в структуре дерева, куда надо вставить (если ветвь существует, 2-й этап пропускаем)
; 2) вставляем новую ветвь с вокселем (3-й этап уже не делаем)
; 3) меняем цвет вокселя
align 4
proc buf2d_vox_obj_create_node, v_obj:dword,coord_x:dword,coord_y:dword,\
coord_z:dword,k_scale:dword,color:dword
pushad
locals
	p_node dd 0 ;родительский узел
endl

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

	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
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

; удаление вокселя в 2 этапа:
; 1) ищем место в структуре дерева, где есть удаляемый узел (если ветвь не существует, выходим из функции)
; 2) удаляем ветвь с вокселем
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
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
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

; взять цвет вокселя в 2 этапа:
; 1) ищем место в структуре дерева, где есть узел (если ветвь не существует, выходим из функции)
; 2) берем цвет вокселя
;output:
; eax - цвет вокселя
align 4
proc buf2d_vox_obj_node_get_color, v_obj:dword,coord_x:dword,coord_y:dword,\
coord_z:dword,k_scale:dword
push ebx ecx edi esi
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
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
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
	;get
		mov eax,dword[edi]
		and eax,0xffffff
		jmp .end_3
	.end_2:
		mov eax,1 shl 31
	.end_3:
pop esi edi ecx ebx
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

;???
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

