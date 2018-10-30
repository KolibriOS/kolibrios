; элемент TreeList для библиотеки box_lib.obj
; на код применена GPL2 лицензия
; последняя модификация 12.09.2017 IgorA


struct TreeNode
	type dw ? ;+ 0 тип элемента, или индекс иконки для узла
	level db ? ;+ 2 уровень элемента
	close db ? ;+ 3 флаг закрытия, или открытия (имеет смысл для родительского узла)
	perv dd ? ;+ 4 индекс предыдущего элемента
	next dd ? ;+ 8 индекс последующего элемента
	t_create dd ? ;+12 врем. создания
	t_delete dd ? ;+16 врем. удаления
ends


;выделние памяти для структур списка и основной информации (конструктор)
align 16
proc tl_data_init uses eax ecx edi, tlist:dword
	mov edi,[tlist]

	xor ecx,ecx
	mov cx,tl_info_size
	imul ecx,tl_info_max_count
	invoke mem.alloc,ecx
	mov tl_data_info,eax ;копируем указатель на полученую память в структуру
	mov tl_data_img,0  ;обнуляем указатель 'data_img'
	mov tl_data_img_sys,0 ;обнуляем указатель 'data_img_sys'

	mov ecx,sizeof.TreeNode
	imul ecx,tl_info_max_count
	invoke mem.alloc,ecx
	mov tl_data_nodes,eax ;копируем указатель на полученую память в структуру

	stdcall tl_info_clear, edi

	;настройки дочернего скроллинга
	cmp tl_p_scroll,0
	je @f
		mov eax,tl_p_scroll
		;*** цветовые настройки ***
		mov ecx,tl_col_bkg
		mov dword[eax+sb_offs_bckg_col],ecx
		mov ecx,tl_col_zag
		mov dword[eax+sb_offs_frnt_col],ecx
		mov ecx,tl_col_txt
		mov dword[eax+sb_offs_line_col],ecx
		;*** настройки размеров ***
		mov ecx,tl_box_left
		add ecx,tl_box_width
		mov word[eax+2],cx
		mov ecx,tl_box_height
		mov word[eax+4],cx
		mov ecx,tl_box_top
		mov word[eax+6],cx
	@@:
	ret
endp

;очистка памяти элемента (деструктор)
align 4
proc tl_data_clear uses eax edi, tlist:dword
	mov edi,[tlist]
	cmp tl_data_img,0
	je @f
		invoke mem.free,tl_data_img ;чистка системных иконок
	@@:
	cmp tl_data_img_sys,0
	je @f
		invoke mem.free,tl_data_img_sys ;чистка системных иконок
	@@:
	invoke mem.free,tl_data_info
	invoke mem.free,tl_data_nodes
	ret
endp

;очистка списка (информации)
align 4
proc tl_info_clear uses eax ecx edi, tlist:dword
	mov edi,[tlist]
	mov tl_ch_tim,0
	mov tl_tim_undo,0
	mov tl_cur_pos,0
	mov ecx,sizeof.TreeNode
	imul ecx,tl_info_max_count
	mov eax,tl_data_nodes
	@@:
		mov byte[eax],0 ;чистим узлы 0-ми
		inc eax
		loop @b
	mov eax,tl_data_nodes ;указатель на 0-й узел
	mov dword[eax+TreeNode.next],1 ;указатель next в 0-м узле приравниваем к 1

	cmp tl_p_scroll,0 ;обработка скроллинга
	je @f
		mov eax,tl_p_scroll
		mov dword[eax+sb_offs_position],0
		call tb_scrol_resize
	@@:
	ret
endp

;реакция на клавиатуру
align 4
proc tl_key uses ebx ecx edi, tlist:dword
	mov edi,[tlist]

	mov ebx,tl_el_focus
	cmp [ebx],edi
	jne .no_focus ;элемент не в фокусе

	push eax
	mcall SF_KEYBOARD,SSF_GET_INPUT_MODE ;получить режим ввода с клавиатуры

	lea ecx,[tl_key_scan]
	cmp eax,1 ;1 = сканкоды
	je @f
		lea ecx,[tl_key_ascii]
	@@:
	pop eax

	xor bx,bx
	cmp ah,byte[ecx] ;Enter
	jne @f
	cmp tl_on_press,0
	je @f
		call tl_on_press
	@@:
	cmp ah,byte[ecx+1] ;Space
	jne @f
		stdcall tl_node_close_open, edi
	@@:
	cmp ah,byte[ecx+2] ;Up
	jne @f
		stdcall tl_cur_perv, edi
	@@:
	cmp ah,byte[ecx+3] ;Down
	jne @f
		stdcall tl_cur_next, edi
	@@:
	cmp ah,byte[ecx+7] ;Page Up
	jne @f
		stdcall tl_cur_page_up, edi
	@@:
	cmp ah,byte[ecx+8] ;Page Down
	jne @f
		stdcall tl_cur_page_down, edi
	@@:

	bt tl_style,0 ;tl_key_no_edit
	jc .no_edit
		cmp ah,byte[ecx+4] ;Left
		jne @f
			stdcall tl_node_lev_dec, edi
			mov bx,1
		@@:
		cmp ah,byte[ecx+5] ;Right
		jne @f
			stdcall tl_node_lev_inc, edi
			mov bx,1
		@@:
		cmp ah,byte[ecx+6] ;Delete
		jne @f
			stdcall tl_node_delete, edi
			mov bx,1
		@@:
	.no_edit:

	cmp bx,1
	jne .no_focus
		stdcall tl_draw, edi
	.no_focus:
	ret
endp

;реакция на мышь
align 4
proc tl_mouse, tlist:dword
	pushad
	mov edi,[tlist]

	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION ;координаты мыши относительно окна

	mov ebx,tl_box_left
	shl ebx,16
	cmp eax,ebx ;левая граница окна
	jl .no_in_wnd ;.test_scroll не применяем
		shr ebx,16
		add ebx,tl_box_width
		shl ebx,16
		cmp eax,ebx ;правая граница окна
		jg .test_scroll

		mov ebx,tl_box_top
		add ebx,tl_box_height
		cmp ax,bx ;нижняя граница окна
		jg .test_scroll

		sub ebx,tl_box_height
		add bx,tl_capt_cy
		cmp ax,bx ;верхняя граница окна + высота подписи
		jl .test_scroll

push eax ebx
	mcall SF_MOUSE_GET,SSF_SCROLL_DATA
	mov edx,eax
	xor	ecx,ecx
	test eax,eax
	jz .mouse_next
	test ax,0x8000
	jnz .decr
	shr eax,16
	test ax,0x8000
	jnz .decr_1

	mov cx,dx ;dx = mouse scroll data vertical
	test ecx,ecx
	jnz @f
	shr edx,16
	mov cx,dx ;dx = mouse scroll data horizontal
	test ecx,ecx
	jz .mouse_next
@@:
	stdcall tl_cur_next, edi
	loop @r
	jmp .mouse_next
;------------------------------------------------
.decr: ;вертикальная прокрутка отрицательная
	mov bx,ax ;ax = mouse scroll data vertical
	jmp @f
.decr_1: ;горизонтальная прокрутка отрицательная
	mov bx,ax ;ax = mouse scroll data horizontal
@@:
	mov ecx,0xffff
	sub ecx,ebx
	inc ecx
@@:
	stdcall tl_cur_perv, edi
	loop @r
;------------------------------------------------
.mouse_next:

		mcall SF_MOUSE_GET,SSF_BUTTON
		bt eax,0 ;left mouse button press
pop ebx eax
		jae .no_draw

		mov esi,tl_el_focus
		mov [esi],edi ;set focus

		; if '+' or '-' press
		mov esi,eax
		shr esi,16
		sub esi,tl_box_left ;esi = mouse x coord in element window

		and eax,0xffff
		sub eax,ebx
		xor edx,edx
		movzx ecx,tl_img_cy
		div ecx
		cmp tl_p_scroll,0 ;учитываем скроллинг
		je @f
			mov edx,tl_p_scroll
			add eax,[edx+sb_offs_position] ;добавляем скроллинг на верху
		@@:

		mov ecx,eax
		call tl_get_node_count ;eax = node count
		bt tl_style,3 ;tl_cursor_pos_limited
		jnc @f
		or eax,eax
		jz @f
			dec eax ;если курсор стает на существующие узлы
		@@:
		cmp eax,ecx
		jl @f
			mov eax,ecx ;если курсор не вышел за пределы узлов, восстанавливаем старое значение eax
		@@:

		cmp eax,tl_cur_pos ;если новое значение курсора совпало с предыдущим
		je @f ;то не стираем курсор
		push esi
			mov esi,tl_box_top
			add esi,tl_box_height ;esi = coord bottom border
			call tl_draw_null_cursor ;стираем курсор
		pop esi
		@@:

		mov tl_cur_pos,eax

		; if '+' or '-' press
		call tl_get_cur_node_index ;eax = node index
		cmp eax,2
		jl .no_open_close ;курсор стоит на пустом месте, без узлов
		imul eax,sizeof.TreeNode
		add eax,tl_data_nodes
		xor bx,bx
		mov bl,byte[eax+TreeNode.level] ;сохраняем уровень текущего узла
		inc bx ;+ поле для курсора

		cmp si,tl_img_cx
		jl .no_open_close ;мышей попали на левое поле для курсора, где точно нет '+' и '-'
			mov eax,esi
			xor edx,edx
			xor ecx,ecx
			mov cx,tl_img_cx
			div ecx

			cmp ax,bx
			jne .no_open_close

			stdcall tl_node_close_open, edi
		.no_open_close:

		mov esi,tl_box_top
		add esi,tl_box_height ;esi = coord bottom border
		call tl_draw_cursor ;перерисовка курсора
		call tl_draw_caption_cur_pos
		jmp .no_draw
;--- mouse event for children scrollbar ----------------------------------------
.test_scroll:
	mov edx,tl_p_scroll
	or edx,edx
	jz .no_in_wnd ;пользователь не создал дочернего скроллинга
		shr ebx,16
		add bx,word[edx] ;+0 .size_x
		shl ebx,16
		cmp eax,ebx ;правая граница окна
		jg .no_in_wnd

		mov eax,[edx+sb_offs_max_area]
		cmp eax,[edx+sb_offs_cur_area]
		jbe .no_in_wnd ;все узлы попадают в окно скроллинга
			stdcall scroll_bar_vertical.mouse, edx ;scrollbar_ver_mouse

			cmp dword[edx+sb_offs_redraw],0
			je @f
				mov dword[edx+sb_offs_redraw],0
				stdcall tl_draw, edi ;произошли изменения скроллинга
			@@:
			cmp dword[edx+sb_offs_delta2],0
			jne .no_draw ;попали на скроллинг - не снимаем фокус с TreeList
;-------------------------------------------------------------------------------
	.no_in_wnd: ;не попали в окно - потеря фокуса (при условии что фокус был на данном эелементе)
	mcall SF_MOUSE_GET,SSF_BUTTON
	or eax,eax ;ничего не нажали eax=0
	jz .no_draw
		mov ebx,tl_el_focus
		cmp [ebx],edi
		jne .no_draw ;элемент не в фокусе
			mov dword[ebx],0 ;reset focus
			mov esi,tl_box_top
			add esi,tl_box_height ;esi = coord bottom border
			call tl_draw_cursor ;рисуем курсор с потеряным фокусом
	.no_draw:

	popad
	ret
endp

;отмена действия
align 4
proc tl_info_undo uses eax edi, tlist:dword
	mov edi,[tlist]
	mov eax,tl_tim_undo
	cmp tl_ch_tim,eax
	jbe @f
		inc tl_tim_undo
		call tb_scrol_resize ;обработка скроллинга
	@@:
	ret
endp

;повтор действия
align 4
proc tl_info_redo uses edi, tlist:dword
	mov edi,[tlist]
	cmp tl_tim_undo,1
	jl @f
		dec tl_tim_undo
		call tb_scrol_resize ;обработка скроллинга
	@@:
	ret
endp

;удаление отмененных действий
;внутренняя функция, не для экспорта
align 4
tl_info_set_undo:
	cmp tl_tim_undo,1
	jl .no_work

	push eax ebx ecx edx
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode
	call tl_move_next ;long i=node[0].next;

	mov eax,tl_tim_undo
	sub tl_ch_tim,eax ;ch_tim-=tim_undo;

	cmp edx,ecx
	jle @f
		;if(node[i].tc>ch_tim){ // если создание символа было отменено
		mov eax,tl_ch_tim
		cmp [edx+TreeNode.t_create],eax
		jle .no_u1
			mov dword[edx+TreeNode.t_create],0
			mov dword[edx+TreeNode.t_delete],0

			mov ebx, [edx+TreeNode.perv]
			imul ebx,sizeof.TreeNode
			add ebx, tl_data_nodes ;.next
			push dword[edx+TreeNode.next] ;node[node[i].perv].next=node[i].next;
			pop dword[ebx+TreeNode.next]

			mov ebx, [edx+TreeNode.next]
			imul ebx,sizeof.TreeNode
			add ebx, tl_data_nodes ;.perv
			push dword[edx+TreeNode.perv] ;node[node[i].next].perv=node[i].perv;
			pop dword[ebx+TreeNode.perv]
		.no_u1:

		;else if(node[i].td>ch_tim) node[i].td=0; // если удаление символа было отменено
		cmp [edx+TreeNode.t_delete],eax
		jle .no_u2
			mov dword[edx+TreeNode.t_delete],0
		.no_u2:
		call tl_move_next
		jmp @b
	@@:
	mov tl_tim_undo,0
	pop edx ecx ebx eax
	.no_work:
	ret

;вывод списка на экран
align 4
proc tl_draw, tlist:dword
	pushad
	;draw dir_list main rect
	mov edi,[tlist]
	mov ebx,tl_box_left
	shl ebx,16
	add ebx,tl_box_width
	mov ecx,tl_box_top
	shl ecx,16
	mov cx,tl_capt_cy
	mov edx,tl_col_zag
	mcall SF_DRAW_RECT ;draw window caption

	add ecx,tl_box_top
	shl ecx,16
	add ecx,tl_box_height
	sub cx,tl_capt_cy
	mcall ,,,tl_col_bkg ;draw window client rect

	cmp tl_capt_cy,9 ;9 - minimum caption height
	jl @f
	mov ebx,edi ;calculate cursor position
	mov eax,tl_cur_pos
	inc eax
	lea edi,[txt_capt_cur.v]
	stdcall tl_convert_to_str, 5

	mov edi,ebx
	mov eax,tl_tim_undo
	lea edi,[txt_capt_otm.v]
	stdcall tl_convert_to_str, 5
	mov edi,ebx ;restore edi

	mov eax,SF_DRAW_TEXT ;captions
	mov ebx,tl_box_left
	shl ebx,16
	add ebx,5*65536+3
	add ebx,tl_box_top
	mov ecx,tl_col_txt
	or  ecx,0x80000000
	lea edx,[txt_capt_cur]
	int 0x40

	mov ebx,tl_box_left
	shl ebx,16
	add ebx,100*65536+3
	add ebx,tl_box_top
	lea edx,[txt_capt_otm]
	int 0x40
	@@:

	;cycle to nodes
	xor eax,eax
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode

	;*** пропуск узлов, которые промотаны скроллингом ***
	cmp tl_p_scroll,0 ;если есть указатель на скроллинг
	je .end_c1
		mov esi,tl_p_scroll
		cmp dword[esi+sb_offs_position],0 ;если скроллинг на верху, выходим
		je .end_c1
		@@:
			call tl_iterat_next
			cmp edx,ecx
			jle .end_draw
			inc eax
			cmp eax,dword[esi+sb_offs_position]
			jl @b
	.end_c1:

	xor eax,eax
	mov esi,tl_box_top
	add esi,tl_box_height ;esi = coord bottom border
	@@:
		call tl_iterat_next
		cmp edx,ecx
		jle @f
		call tl_draw_node
		inc eax
		jmp @b
	@@:

	call tl_draw_cursor

	mov edi,tl_p_scroll ;рисуем дочерний скроллинг
	cmp edi,0    ;для того что-бы его не пришлось рисовать в пользовательской программе
	je .end_draw ;если нет скроллинга выходим
		stdcall scroll_bar_vertical.draw, edi
	.end_draw:
	popad
	ret
endp

;переход на следущий видимый узел (пропуская закрытые)
;input:
; ecx = pointer to 1 node struct
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer to next node struct
align 4
tl_iterat_next:
	push bx
	mov bl,0x7f
	cmp byte[edx+TreeNode.close],1
	jne @f
		mov bl,byte[edx+TreeNode.level]
	@@:

	cmp tl_tim_undo,0
	je .else

	push eax
	.beg0:
		call tl_move_next
		cmp edx,ecx
		jle @f
		call tl_node_not_vis ;пропуск удаленных и отмененных
		cmp al,1
		je .beg0
		cmp bl,byte[edx+TreeNode.level] ;пропуск закрытых
		jl .beg0
		@@:
	pop eax
	pop bx
	ret

	.else:
		call tl_move_next
		cmp edx,ecx
		jle .endif
		cmp dword[edx+TreeNode.t_delete],0 ;пропуск удаленных
		jne .else
		cmp bl,byte[edx+TreeNode.level] ;пропуск закрытых
		jl .else
	.endif:
	pop bx
	ret

;переход на следущий видимый узел (и на закрытые тоже)
;input:
; ecx = pointer to 1 node struct
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer to next visible node struct
align 4
tl_iterat_next_all:
	cmp tl_tim_undo,0
	je .else

	push eax
	@@:
		call tl_move_next
		cmp edx,ecx
		jle @f
		call tl_node_not_vis
		cmp al,1
		je @b
	@@:
	pop eax
	ret
	.else:
		call tl_move_next
		cmp edx,ecx
		jle .endif
		cmp dword[edx+TreeNode.t_delete],0 ;td -> time delete
		jne .else
	.endif:
	ret

;переход на предыдущий видимый узел (пропуская закрытые)
;input:
; ecx = pointer to 1 node struct
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
align 4
proc tl_iterat_perv uses eax
	cmp tl_tim_undo,0
	je .beg1

	.beg0:
		call tl_move_perv
		cmp edx,ecx
		jle @f
		call tl_node_not_vis ;пропуск удаленных и отмененных
		cmp al,1
		je .beg0

	.beg1:
		call tl_move_perv
		cmp edx,ecx
		jle @f
		cmp dword[edx+TreeNode.t_delete],0 ;td = 'time delete' -> пропуск удаленных
		jne .beg1

	@@:
	call tl_move_max_clo_par
	ret
endp

;находит родительский закрытый узел максимального уровня
;input:
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer closed parent node with maximum level
align 4
proc tl_move_max_clo_par uses eax ebx
	mov eax,edx
	xor ebx,ebx
	.beg:
		call tl_move_par
		cmp byte[edx+TreeNode.close],1 ;родительский узел закрыт ?
		jne @f
			mov eax,edx
		@@:
		cmp ebx,edx
		je .end_f
			mov ebx,edx
			jmp .beg
	.end_f:
	mov edx,eax
	ret
endp

;input:
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer to next node struct
align 4
tl_move_next:
	mov edx,[edx+TreeNode.next]
	imul edx,sizeof.TreeNode
	add edx,tl_data_nodes
	ret

;input:
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer to perv node struct
align 4
tl_move_perv:
	mov edx,[edx+TreeNode.perv]
	imul edx,sizeof.TreeNode
	add edx,tl_data_nodes
	ret

;передвигаемся на родительский узел, если такого нет, то оставляем старое значение указателя
;input:
; ecx = pointer to 1 node struct
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; edx = pointer to parent node struct
align 4
tl_move_par:
	cmp byte[edx+TreeNode.level],0
	je .end_f ;узел 0-го уровня не может быть дочерним
	push eax ebx esi
	mov esi,edx ;copy node pointer (edx)
	mov bl,byte[edx+TreeNode.level]
	@@:
		call tl_move_perv
		cmp edx,ecx
		jle @f ;все выше стоящие узлы не родительские
		call tl_node_not_vis ;пропуск удаленных и отмененных
		cmp al,1
		je @b
		cmp byte[edx+TreeNode.level],bl
		jl .end_0 ;удачно нашли родительский узел
		jmp @b
	@@:
		mov esi,ebx ;restore node pointer
	.end_0:
	pop esi ebx eax
	.end_f:
	ret

;проверяет видимый ли указанный узел с учетом: добавлений, удалений, отмен действий
;input:
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
;output:
; al = 1 if sumbol not visible
; (node[i].td+tim_Undo<=ch_tim && node[i].td) || (node[i].tc>ch_tim-tim_Undo)
align 4
tl_node_not_vis:
	cmp dword[edx+TreeNode.t_delete],0
	je @f
	mov eax,[edx+TreeNode.t_delete] ;eax=node[i].td
	add eax,tl_tim_undo
	cmp eax,tl_ch_tim
	jg @f
		mov al,1
		ret
	@@:

	mov eax,tl_ch_tim
	sub eax,tl_tim_undo
	cmp [edx+TreeNode.t_create],eax ;time create
	jle @f
		mov al,1
		ret
	@@:
	xor al,al
	ret

;рисуем курсор на экране
;input:
; edi = pointer to TreeInfo struct
; esi = coord bottom border
align 4
proc tl_draw_cursor uses eax ebx ecx edx esi
	call tl_get_display_cur_pos ;eax = cursor pos in screen
	cmp eax,0
	jl .end_f ;курсор находится выше окна, в области прокрученной скроллингом

	cmp tl_data_img_sys,0 ;смотрим есть ли указатель на картинку системных иконок
	jne @f
		mov ebx,tl_box_left
		shl ebx,16
		mov bx,tl_img_cx
		movzx ecx,tl_img_cy
		imul ecx,eax
		add ecx,tl_box_top
		add cx,tl_capt_cy

		;crop image if on the border
		cmp esi,ecx ;если курсор внизу и его вообще не видно
		jl .end_f

		sub esi,ecx
		shl ecx,16
		mov cx,tl_img_cy
		cmp si,tl_img_cy
		jge .crop0
			mov cx,si ;если курсор виден частично (попал на нижнюю границу)
		.crop0:

		mov edx,tl_col_txt
		mcall SF_DRAW_RECT ;рисуем простой прямоугольник, т.к. нет системных иконок
		jmp .end_f
	@@:
	mov ebx,tl_data_img_sys
	imul ax,tl_img_cy
	mov edx,tl_box_left
	shl edx,16
	mov dx,ax
	add edx,tl_box_top
	add dx,tl_capt_cy

	mov ecx,tl_el_focus ;проверяем в фокусе элемент или нет
	cmp dword[ecx],edi
	je .focus
		;если не в фокусе сдвигаем координаты на иконку не активного курсора
		movzx ecx,tl_img_cx
		movzx eax,tl_img_cy
		imul eax,ecx
		imul eax,4*3 ;4=icon index 3=rgb
		add ebx,eax
	.focus:

	mov cx,tl_img_cx
	shl ecx,16
	mov cx,tl_img_cy

	;crop image if on the border
	cmp si,dx ;если курсор внизу и его вообще не видно
	jl .end_f

		sub si,dx
		cmp si,tl_img_cy
		jge .crop1
			mov cx,si ;если курсор виден частично (попал на нижнюю границу)
		.crop1:

		mcall SF_PUT_IMAGE ;рисуем иконку курсора
	.end_f:
	ret
endp

;стираем курсор на экране
;input:
; edi = pointer to TreeInfo struct
; esi = coord bottom border
align 4
proc tl_draw_null_cursor uses eax ebx ecx edx esi
	call tl_get_display_cur_pos ;eax = cursor pos in screen
	cmp eax,0
	jl .end_f ;курсор находится выше окна, в области прокрученной скроллингом
		mov ebx,tl_box_left
		shl ebx,16
		mov bx,tl_img_cx
		movzx ecx,tl_img_cy
		imul ecx,eax
		add ecx,tl_box_top
		add cx,tl_capt_cy

		;crop image if on the border
		cmp esi,ecx ;если курсор внизу и его вообще не видно
		jl .end_f

		sub esi,ecx
		shl ecx,16
		mov cx,tl_img_cy
		cmp si,tl_img_cy
		jge @f
			mov cx,si ;если курсор виден частично (попал на нижнюю границу)
		@@:
		mcall SF_DRAW_RECT,,,tl_col_bkg ;рисуем простой прямоугольник с фоновым цветом
	.end_f:
	ret
endp

;берет позицию курсора, относительно экрана
;input:
; edi = pointer to TreeInfo struct
;output:
; eax = index
align 4
tl_get_display_cur_pos:
	mov eax,tl_cur_pos
	cmp tl_p_scroll,0
	je @f
		push ebx
		mov ebx,tl_p_scroll
		mov ebx,dword[ebx+sb_offs_position]
		sub eax,ebx ;отнимаем позицию скроллинга
		pop ebx
	@@:
	ret

;рисует узел с: картинкой, подписью, иконкой открытия/закрытия и линиями к родит. узлу
;input:
; eax = node position (0, ..., max_nodes-scroll_pos) не до tl_box_height/tl_img_cy
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
; esi = coord of bottom border
align 4
proc tl_draw_node uses eax ebx ecx edx esi
	mov ebx,1 ;1 - место под курсор
	bt tl_style,2 ;tl_list_box_mode
	jc @f
		inc ebx ;+1 - место под знак +,-
		add bl,byte[edx+TreeNode.level] ;добавляем уровень элемента для его учета в левом отступе иконки
	@@:
	imul bx,tl_img_cx
	add ebx,tl_box_left

	shl ebx,16
	mov bx,tl_img_cx
	movzx ecx,tl_img_cy
	imul ecx,eax
	add cx,tl_capt_cy
	jc .end_draw ;когда много узлов, то может быть переполнение координаты cx
	add ecx,tl_box_top

	;crop image if on the border
	cmp esi,ecx ;если узел внизу и его вообще не видно
	jl .end_draw

	sub esi,ecx
	shl ecx,16
	mov cx,tl_img_cy
	cmp si,tl_img_cy
	jge @f
		mov cx,si ;если узел виден частично (попал на нижнюю границу)
		jmp .crop ;пропускаем рисование надписи, которая скорее всего тоже вылезет за нижнюю границу
	@@:
		call tl_draw_node_caption
	.crop:
	mov esi,ecx ;save ecx

	push edx
	cmp tl_data_img,0
	jne .draw_img_n
		mcall SF_DRAW_RECT,,,tl_col_txt ;draw node rect
		jmp @f
	.draw_img_n:
	push ebx esi
		movzx esi,word[edx+TreeNode.type] ;get icon type
		mov edx,ebx
		ror ecx,16
		mov dx,cx
		mov cx,bx
		ror ecx,16
		mov ebx,3 ;rgb = 3 bytes
		imul bx,tl_img_cx
		imul bx,tl_img_cy
		imul ebx,esi ;esi = icon index
		add ebx,tl_data_img

		mcall SF_PUT_IMAGE ;draw node icon '-'
	pop esi ebx
	@@:
	pop edx

	mov al,byte[edx+TreeNode.level] ;draw minus '-'
	mov ecx,tl_data_nodes
	add ecx,sizeof.TreeNode

	mov ah,10 ;get icon index '+' or '-' ?
	cmp byte[edx+TreeNode.close],1
	jne .close
		dec ah
	.close:

	call tl_draw_node_icon_opn_clo ;рисование иконки открытого или закрытого узла
	bt tl_style,1
	jae .end_draw
		call tl_draw_node_icon_par_lin ;рисование линии к родительскому элементу
		call tl_draw_node_icon_par_lin_up ;рисование вертикальной линии к родительскому элементу
	.end_draw:
	ret
endp

;рисует иконки открытого или закрытого узла (обычно + или -)
;input:
; al = уровень элемента
; ecx = pointer to 1 node struct
; edx = pointer to node struct
;...
align 4
proc tl_draw_node_icon_opn_clo uses eax ebx ecx edx esi
	inc al
	call tl_iterat_next_all ;get next visible item
	cmp edx,ecx
	jle @f
		mov ecx,esi ;load ecx
		cmp al,byte[edx+TreeNode.level]
		jne @f
		ror ebx,16
		sub bx,tl_img_cx
		ror ebx,16
		cmp tl_data_img_sys,0
		jne .draw_img_s
			mcall SF_DRAW_RECT,,,tl_col_txt ;draw minus rect, if not system icons
			jmp @f
		.draw_img_s:
		mov ecx,esi ;load ecx
		mov edx,ebx
		ror ecx,16
		mov dx,cx
		mov cx,bx
		ror ecx,16
		mov ebx,3 ;rgb = 3 bytes
		imul bx,tl_img_cx
		imul bx,tl_img_cy
		shr eax,8
		and eax,0xff
		imul ebx,eax ;eax = icon index
		add ebx,tl_data_img_sys
		mcall SF_PUT_IMAGE ;draw minus icon '-'
	@@:
	ret
endp

;рисование линии к родительскому элементу
;input:
; al = уровень элемента
; ecx = pointer to 1 node struct
; edx = pointer to node struct
;...
align 4
tl_draw_node_icon_par_lin:
	cmp byte[edx+TreeNode.close],1
	je .close
	or al,al
	jz .close
	push eax ebx ecx edx esi
		call tl_iterat_next_all ;get next visible item
		cmp edx,ecx
		jle .line3 ;if end of list
			cmp al,byte[edx+TreeNode.level]
			jne .line3 ;jg ???
			mov eax,3 ;line in middle element
			jmp .line2
		.line3:
			mov eax,6 ;line in end element
		.line2:

		mov ecx,esi ;load ecx
		ror ebx,16
		sub bx,tl_img_cx
		ror ebx,16
		cmp tl_data_img_sys,0
		jne .draw_img_s
			mcall SF_DRAW_RECT,,,tl_col_txt ;draw minus rect, if not system icons
			jmp @f
		.draw_img_s:
		mov edx,ebx
		ror ecx,16
		mov dx,cx
		mov cx,bx
		ror ecx,16
		mov ebx,3 ;rgb = 3 bytes
		imul bx,tl_img_cx
		imul bx,tl_img_cy

		imul ebx,eax ;eax = icon index
		add ebx,tl_data_img_sys
		mcall SF_PUT_IMAGE ;draw line icon
		@@:
	pop esi edx ecx ebx eax
	.close:
	ret


;input:
; al = уровень элемента
; ebx = (node.left shl 16) + tl_img_cx
; ecx = pointer to 1 node struct
; edx = pointer to node struct
; edi = pointer to 'TreeList' struct
align 4
proc tl_draw_node_icon_par_lin_up uses eax ebx ecx edx esi
	cmp tl_data_img_sys,0 ;if not image
	je @f
	or al,al
	jz @f
		xor esi,esi ;в si будем насчитывать кол-во иконок, нужных для прорисовки линии
;--- цикл для вычисления колличества вертикальных линий ---
		.cycle0:
			call tl_iterat_perv ;get perv visible item
			cmp edx,ecx
			jle .cycle1 ;if begin of list

			cmp byte[edx+TreeNode.level],al
			jle .cycle0end ;уровень верхнего элемента не требует прорисовки
			inc si
			jmp .cycle0
		.cycle0end:
		or si,si ;si = кол-во иконок линии которые нужно нарисовать сверху
		jz @f
		shl esi,16

		pop ecx ;esi->ecx
		push ecx ;save esi

		ror ebx,16
		sub bx,tl_img_cx
		ror ebx,16

		mov edx,ebx
		ror ecx,16
		mov dx,cx
		mov cx,bx
		ror ecx,16
		mov cx,tl_img_cy ;restore size y (if crop)
		mov ebx,3 ;rgb = 3 bytes
		imul bx,tl_img_cx
		imul bx,tl_img_cy
		add ebx,tl_data_img_sys

		add esi,tl_box_top
		add si,tl_capt_cy ;si = верхняя граница окна
		mov eax,SF_PUT_IMAGE
;--- цикл для рисования вертикальной линии ---
		.cycle1:
		sub dx,tl_img_cy ;поднимаем координату y вверх
		cmp dx,si
		jl @f
			cmp esi,0x10000
			jl @f
			int 0x40 ;draw line icon
			sub esi,0x10000 ;уменьшаем счетчик иконок
		jmp .cycle1
	@@:
	ret
endp

;input:
; edi = pointer to TreeInfo struct
;output:
; eax = rows
align 4
tl_get_rows_count:
	push ecx edx
		mov eax,tl_box_height
		sub ax,tl_capt_cy
		movzx ecx,tl_img_cy
		xor edx,edx
		div ecx
	pop edx ecx
	ret

;input:
; eax = node position
; ebx = [координата по оси x]*65536 + [img_cx]
; ecx = [координата по оси y]*65536 + [img_cy]
; edx = pointer to node struct
; edi = pointer to TreeInfo struct
align 4
proc tl_draw_node_caption uses ebx ecx edx esi
	xor esi,esi
	mov si,tl_info_size
	cmp si,tl_info_capt_offs
	jle @f ;if caption size <= 0
		push eax
			call tl_get_node_index ;eax = node index
			imul esi,eax
		pop eax
		add si,tl_info_capt_offs
		add esi,tl_data_info
		mov edx,esi

		shr ebx,16
		add bx,tl_img_cx ;сдвигаем надпись по горизонтали --->
		add bx,3 ;отступ
		;bx = coord.x
		call tl_strlen ;eax = strlen
		call tl_get_draw_text_len
		mov cx,bx
		ror ecx,16
		mov ebx,ecx
		add bx,tl_img_cy ;выравнивиние по нижней границе иконки
		sub bx,9 ;отнимаем высоту текста
		mov ecx,tl_col_txt
		and ecx,0xffffff
		mcall SF_DRAW_TEXT
	@@:
	ret
endp

;input:
; eax = strlen
; ebx = text coord x
;output:
; esi = text len
align 4
proc tl_get_draw_text_len uses eax ecx edx
	mov esi,eax ;берем длинну строки
	mov eax,tl_box_left
	add eax,tl_box_width
	cmp eax,ebx
	jle .text_null ;если подпись полностью вся за экраном
		sub eax,ebx
		xor edx,edx
		mov ecx,6 ;ширина системного шрифта
		div ecx ;смотрим сколько символов может поместиться на экране
		cmp esi,eax
		jl @f
			mov esi,eax ;если длинна текста меньше, чем все место под строку
			jmp @f
	.text_null:
	xor esi,esi
	@@:
	ret
endp

;input:
; esi = pointer to string
;output:
; eax = strlen
align 4
tl_strlen:
	mov eax,esi
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,esi
	ret

;добавить узел
;input:
; tlist - указатель на структуру листа
; n_opt - опции добавления
; n_info - указатель на добавляемые данные
align 4
proc tl_node_add uses eax ebx ecx edx edi, tlist:dword, n_opt:dword, n_info:dword
	mov edi,[tlist]

	call tl_info_set_undo

	mov ebx,sizeof.TreeNode
	imul ebx,tl_info_max_count
	add ebx,tl_data_nodes
;--
	call tl_get_cur_node_index ;eax=po_t
	imul eax,sizeof.TreeNode
	add eax,tl_data_nodes
	mov edx,eax
	call tl_move_perv
	call tl_get_node_index ;eax = index of pointer [edx]
;--
	mov edx,sizeof.TreeNode
	shl edx,1
	add edx,tl_data_nodes
	@@: ;for(i=2;i<nodeMax;i++)
		cmp dword[edx+TreeNode.t_create],0
		jne .u0
		cmp dword[edx+TreeNode.t_delete],0
		jne .u0

		inc tl_ch_tim
		mov ecx,dword[n_opt]
		ror ecx,16 ;cx = type
		mov word[edx+TreeNode.type],cx
		rol ecx,8 ;cl = close|open
		mov byte[edx+TreeNode.close],cl ;node[i].clo
		mov byte[edx+TreeNode.level],0 ;node[i].lev=0
		bt tl_style,2 ;tl_list_box_mode
		jc .l_box_m
			mov cl,byte[n_opt]
			mov byte[edx+TreeNode.level],cl ;node[i].lev
		.l_box_m:
		push tl_ch_tim       ;node[i].tc=ch_tim;
		pop dword[edx+TreeNode.t_create]
		mov [edx+TreeNode.perv],eax ;node[i].perv=po_t;
		;*** copy node data ***
		push esi
		xor ecx,ecx
		mov cx,tl_info_size
		mov esi,ecx

		push eax
			call tl_get_node_index ;eax = node index
			imul esi,eax
		pop eax
		add esi,tl_data_info
		mov edi,dword[n_info] ;pointer to node data
		xchg edi,esi
		rep movsb

		mov esi,edi
		mov edi,[tlist] ;restore edi
		mov cx,tl_info_capt_offs
		cmp cx,tl_info_size
		jge .no_text_data
		cmp tl_info_capt_len,0 ;проверяем есть ли ограничение на длинну строки
		je .no_len_ogran
			add cx,tl_info_capt_len
			and ecx,0xffff
			add esi,ecx
			mov cx,tl_info_size
			sub esi,ecx
		.no_len_ogran:
		dec esi
		mov byte[esi],0
		.no_text_data:
		pop esi ;restore esi

		mov ecx,eax
		imul ecx,sizeof.TreeNode
		add ecx,tl_data_nodes ; *** ecx = node[po_t] ***
		add ecx,TreeNode.next ; *** ecx = node[po_t].next ***
		push dword[ecx] ;node[i].next=node[po_t].next;
		pop dword[edx+TreeNode.next]

		call tl_get_node_index ;*** eax = i ***
		cmp eax,tl_info_max_count
		jge .u0
			mov [ecx],eax ;node[po_t].next=i; // ссылки перенаправляем
			mov ecx,[edx+TreeNode.next] ; *** ecx = node[i].next ***
			imul ecx,sizeof.TreeNode
			add ecx,tl_data_nodes ; *** ecx = node[node[i].next] ***
			mov [ecx+TreeNode.perv],eax ;node[node[i].next].perv=i;

			call tb_scrol_resize ;обработка скроллинга
			jmp @f
		.u0:
		add edx,sizeof.TreeNode
		cmp edx,ebx ;enf of node memory ?
		jle @b
	@@:
	ret
endp

;input:
; edi = pointer to TreeInfo struct
align 4
proc tb_scrol_resize uses eax ecx edx
	cmp tl_p_scroll,0 ;обработка скроллинга
	je @f
		call tl_get_node_count ;eax = node count
		mov ecx,eax
		call tl_get_rows_count
		cmp ecx,eax
		jg .ye_sb
			xor ecx,ecx
		.ye_sb:
		mov edx,tl_p_scroll
		mov dword[edx+sb_offs_cur_area],eax
		mov dword[edx+sb_offs_max_area],ecx
		stdcall scroll_bar_vertical.draw,edx
	@@:
	ret
endp

;input:
;n_info - pointer to node info
align 4
proc tl_node_set_data uses eax ecx edx edi esi, tlist:dword, n_info:dword
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		xor ecx,ecx
		mov cx,tl_info_size
		imul eax,ecx
		add eax,tl_data_info
		mov edi,eax
		mov esi,dword[n_info] ;pointer to node data
		rep movsb

		mov esi,edi
		mov edi,[tlist] ;restore edi
		mov cx,tl_info_capt_offs
		cmp cx,tl_info_size
		jge .no_text_data
			mov ax,tl_info_capt_len ;проверяем есть ли ограничение на длинну текста
			or ax,ax
			jz .no_limit
				add cx,ax ;cx = tl_info_capt_offs + tl_info_capt_len
				and ecx,0xffff
				xor eax,eax
				mov ax,tl_info_size
				cmp eax,ecx
				jl .no_limit ;пользователь задал слишком большую длинну текста
				add esi,ecx
				sub esi,eax
			.no_limit:
			dec esi
			mov byte[esi],0 ;обнуляем последний символ подписи, что-бы не глючило если пользователь задал неправильную структуру
		.no_text_data:
	@@:
	ret
endp

;взять указатель на данные узла под курсором
;input:
; tlist - pointer to 'TreeList' struct
;output:
; eax - pointer to node data
align 4
proc tl_node_get_data uses ecx edi, tlist:dword
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		movzx ecx,tl_info_size
		imul eax,ecx
		add eax,tl_data_info
		jmp .end_f ;return node data pointer
	@@:
	xor eax,eax
	.end_f:
  ret
endp

;взять указатель на структуру узла в указанной позиции
;input:
; tlist - pointer to 'TreeList' struct
; node_ind - node index
;output:
; eax - pointer to node info
align 4
proc tl_node_poi_get_info uses ebx ecx edx edi, tlist:dword, node_ind:dword
	mov edi,[tlist]
	mov ebx,dword[node_ind]

	;cycle to nodes
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode
	@@:
		call tl_iterat_next_all
		cmp edx,ecx
		jle @f
		dec ebx
		cmp ebx,0
		jg @b
		jmp .find
	@@:
		xor edx,edx
	.find:
	mov eax,edx
	ret
endp

;взять указатель на следущую структуру узла
;input:
; tlist - pointer to 'TreeList' struct
; node_p - node param struct
;output:
; eax - pointer to next node struct
align 4
proc tl_node_poi_get_next_info uses ecx edx edi, tlist:dword, node_p:dword
	mov edi,[tlist]
	mov edx,dword[node_p]

	mov ecx,tl_data_nodes
	add ecx,sizeof.TreeNode

	call tl_iterat_next_all
	cmp edx,ecx
	jg @f
		xor edx,edx
	@@:
	mov eax,edx
	ret
endp

;взять указатель на данные узла
;input:
; tlist - pointer to 'TreeList' struct
; node_p - node param struct
;output:
; eax - pointer
align 4
proc tl_node_poi_get_data uses ecx edx edi, tlist:dword, node_p:dword
	mov edi,[tlist]
	mov edx,dword[node_p]

	call tl_get_node_index ;eax = node index
	cmp eax,2
	jl @f
		xor ecx,ecx
		mov cx,tl_info_size
		imul eax,ecx
		add eax,tl_data_info
		jmp .end_f ;return node data pointer
	@@:
		xor eax,eax ;возвращаем 0 в случае не удачного поиска
	.end_f:
	ret
endp

;берет позицию под курсором
;input:
; edi = pointer 'tl' struct
;output:
; eax = index of current node
align 4
proc tl_get_cur_node_index uses ecx edx
	;cycle to nodes
	xor eax,eax
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode
	@@:
		call tl_iterat_next
		cmp edx,ecx
		jle @f
		cmp eax,tl_cur_pos
		je @f
		inc eax
		jmp @b
	@@:
	mov eax,edx
	sub eax,tl_data_nodes
	xor edx,edx
	mov ecx,sizeof.TreeNode
	div ecx
	ret
endp

;берет позицию указанного символа
;input:
; edx = pointer node memory
; edi = pointer 'tl' struct
;output:
; eax = struct index of current node
align 4
tl_get_node_index:
	push ecx edx
	mov eax,edx
	sub eax,tl_data_nodes
	xor edx,edx
	mov ecx,sizeof.TreeNode
	div ecx
	pop edx ecx
	ret

;удалить узел
align 4
proc tl_node_delete uses eax edx edi, tlist:dword
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		imul eax,sizeof.TreeNode
		add eax,tl_data_nodes
		mov edx,eax
		inc tl_ch_tim
		mov eax,tl_ch_tim
		mov dword[edx+TreeNode.t_delete],eax
		call tb_scrol_resize ;обработка скроллинга
	@@:
	ret
endp

;поставить курсор на первый узел
align 4
proc tl_cur_beg uses edi, tlist:dword
	mov edi,[tlist]
	mov tl_cur_pos,0
	cmp tl_p_scroll,0
	je @f
		mov edi,tl_p_scroll
		mov dword[edi+sb_offs_position],0
		stdcall scroll_bar_vertical.draw, edi
	@@:
	ret
endp

;перенести курсор на 1 позицию ниже
align 4
proc tl_cur_next uses eax ebx edi esi, tlist:dword
	mov edi,[tlist]
	call tl_get_node_count ;eax = node count
	bt tl_style,3 ;tl_cursor_pos_limited
	jnc @f
	or eax,eax
	jz @f
		dec eax ;если курсор стает на существующие узлы
	@@:
	cmp tl_cur_pos,eax
	jge .no_redraw
		mov esi,tl_box_top
		add esi,tl_box_height ;esi = coord bottom border
		call tl_draw_null_cursor ;стираем курсор
		inc tl_cur_pos

		cmp tl_p_scroll,0 ;if not scrol struct
		je @f
		call tl_get_rows_count ;eax = rows count
		mov ebx,tl_p_scroll
		add eax,dword[ebx+sb_offs_position]
		cmp tl_cur_pos,eax
		jl @f
			inc dword[ebx+sb_offs_position]
			stdcall scroll_bar_vertical.draw,ebx
			stdcall tl_draw,dword[tlist] ;полная перерисовка окна
			jmp .no_redraw
		@@:
		mov edi,[tlist] ;restore edi
		call tl_draw_cursor ;перерисовка курсора
		call tl_draw_caption_cur_pos
	.no_redraw:
	ret
endp

;берет число всех видимых узлов (не считая закрытых дочерних)
;input:
; edi = pointer 'tl' struct
;output:
; eax = struct index of current node
align 4
tl_get_node_count:
push ecx edx
	;cycle to nodes
	xor eax,eax
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode
	@@:
		call tl_iterat_next
		cmp edx,ecx
		jle @f
		inc eax
		jmp @b
	@@:
pop edx ecx
	ret

;берет число всех видимых узлов (считая закрытые дочерние)
;input:
; edi = pointer 'tl' struct
;output:
; eax = struct index of current node
align 4
tl_get_node_count_all:
push ecx edx
	;cycle to nodes
	xor eax,eax
	mov edx,tl_data_nodes
	mov ecx,edx
	add ecx,sizeof.TreeNode
	@@:
		call tl_iterat_next_all
		cmp edx,ecx
		jle @f
		inc eax
		jmp @b
	@@:
pop edx ecx
	ret

;перенести курсор на 1 позицию выше
align 4
proc tl_cur_perv uses eax edi esi, tlist:dword
	mov edi,[tlist]
	cmp tl_cur_pos,0
	je .no_redraw
		mov esi,tl_box_top
		add esi,tl_box_height ;esi = coord bottom border
		call tl_draw_null_cursor ;стираем курсор
		dec tl_cur_pos ;двигаем курсор вверх

		cmp tl_p_scroll,0 ;если есть указатель на скроллинг
		je @f
		mov eax,tl_p_scroll
		cmp dword[eax+sb_offs_position],0 ;если скроллинг на верху, выходим
		je @f
		mov edi,tl_cur_pos
		cmp edi,dword[eax+sb_offs_position] ;если курсор ушел выше скроллинга, тогда опускаем скроллинг
		jge @f
			dec dword[eax+sb_offs_position]
			stdcall scroll_bar_vertical.draw, eax
			stdcall tl_draw, dword[tlist] ;полная перерисовка окна
			jmp .no_redraw
		@@:
			mov edi,[tlist] ;restore edi
			call tl_draw_cursor ;перерисовка курсора
			call tl_draw_caption_cur_pos
	.no_redraw:
	ret
endp

;перенести курсор на 1 страницу выше
align 4
proc tl_cur_page_up uses eax edi esi, tlist:dword
	mov edi,[tlist]

	cmp tl_p_scroll,0 ;если есть указатель на скроллинг
	je .no_redraw
		mov esi,tl_p_scroll
		call tl_get_rows_count ;eax = rows count

		cmp tl_cur_pos,0
		jne @f
		cmp dword[esi+sb_offs_position],0 ;если скроллинг на верху, выходим
		jne @f
			jmp .no_redraw
		@@:
		cmp tl_cur_pos,eax ;проверяем позицию курсора и кол-во сток на странице
		jl @f ;если меньше, то приравниваем к 0, что-бы не отнять больше чем надо
			sub tl_cur_pos,eax
			jmp .cursor
		@@:
			mov tl_cur_pos,0
		.cursor:
		cmp dword[esi+sb_offs_position],eax
		jl @f
			sub dword[esi+sb_offs_position],eax
			jmp .scroll
		@@:
		mov dword[esi+sb_offs_position],0
		.scroll:
		;перерисовки окна и скроллинга
		stdcall tl_draw, edi ;draw window
		stdcall scroll_bar_vertical.draw, esi
	.no_redraw:
	ret
endp

;перенести курсор на 1 страницу ниже
align 4
proc tl_cur_page_down uses eax ebx ecx edi esi, tlist:dword
;eax - кол-во строк на странице
;ebx - макс. позиция курсора
;ecx - макс. позиция скроллинга
	mov edi,[tlist]
	cmp tl_p_scroll,0 ;если есть указатель на скроллинг
	je .no_redraw
		mov esi,tl_p_scroll
		call tl_get_node_count ;eax = node count
		bt tl_style,3 ;tl_cursor_pos_limited
		jnc @f
		or eax,eax
		jz @f
			dec eax ;если курсор стает на существующие узлы
		@@:
		mov ebx,eax
		call tl_get_rows_count ;eax = rows count

		mov ecx,ebx
		inc ecx ;если нижний узел виден на половину
		cmp ecx,eax ;if (ecx>eax) { ecx=ecx-eax } else { ecx=0 }
		jl @f
			sub ecx,eax ;уменьшаем максимальную позицию скроллинга, так что-бы были видны последние узлы
			jmp .control
		@@:
			xor ecx,ecx ;ecx=0 - все узлы влазят в экран, скроллинг не нужен
		.control:

		cmp tl_cur_pos,ebx ;курсор внизу ?
		jl @f
		cmp dword[esi+sb_offs_position],ecx ;скроллинг внизу ?
		jl @f
			jmp .no_redraw
		@@:
		add tl_cur_pos,eax ;перемещаем курсор
		cmp tl_cur_pos,ebx
		jl @f
			mov tl_cur_pos,ebx
		@@:
		add dword[esi+sb_offs_position],eax ;перемещаем скроллинг
		cmp dword[esi+sb_offs_position],ecx
		jl @f
			mov dword[esi+sb_offs_position],ecx
		@@:

		;перерисовки окна и скроллинга
		stdcall tl_draw, edi ;draw window
		stdcall scroll_bar_vertical.draw, esi
	.no_redraw:
	ret
endp

;открыть/закрыть узел (работает с узлами которые имеют дочерние узлы)
align 4
proc tl_node_close_open uses eax edx edi, tlist:dword
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax = позиция узла на котором стоит курсор
	cmp eax,2 ;курсор стоит на узле ?
	jl @f
		imul eax,sizeof.TreeNode
		add eax,tl_data_nodes
		;eax = указатель на структуру узла выбранного курсором
		push eax
			stdcall tl_node_poi_get_next_info, edi,eax
			mov edx,eax ;edx = указатель на структуру узла который идет после узла eax
		pop eax
		or edx,edx ;есть ли узлы ниже выбранного нами ?
		jz @f
			mov dl,byte[edx+TreeNode.level] ;берем уровень нижнего узла
			cmp byte[eax+TreeNode.level],dl
			jge @f ;если нижние узлы меньшего уровня, значит они не дочерние, конец функции
				xor byte[eax+TreeNode.close],1 ;*** открытие/закрытие узла ***
				call tb_scrol_resize ;обработка скроллинга

				stdcall tl_draw, edi ;обновление окна
	@@:
	ret
endp

;увеличить уровень
align 4
proc tl_node_lev_inc uses eax ecx edx edi, tlist:dword
	mov edi,[tlist]
	bt tl_style,2 ;tl_list_box_mode
	jc @f
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		mov ecx,tl_data_nodes
		imul eax,sizeof.TreeNode
		add eax,ecx ;eax = pointer to node struct
		add ecx,sizeof.TreeNode ;ecx = pointer to 1 node struct

		mov edx,eax
		call tl_iterat_perv ;проверяем есть ли верхний узел
		cmp edx,ecx
		jle @f ;если верхнего узла нет то текущий узел не двигаем
		mov cl,byte[edx+TreeNode.level] ;берем уровень родительского узла
		inc cl ;добавляем 1 и получаем максимальное значение
		cmp byte[eax+TreeNode.level],cl
		jge @f
			inc byte[eax+TreeNode.level] ;увеличиваем значение узла
	@@:
	ret
endp

;уменьшить уровень
align 4
proc tl_node_lev_dec uses eax edi, tlist:dword
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		imul eax,sizeof.TreeNode
		add eax,tl_data_nodes
		cmp byte[eax+2],0
		je @f
		dec byte[eax+2]
	@@:
	ret
endp

;перемещаем узел вверх
align 4
proc tl_node_move_up tlist:dword
pushad
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		mov ebx,eax ;copy index of node struct
		mov edx,tl_data_nodes
		mov ecx,edx
		add ecx,sizeof.TreeNode
		imul eax,sizeof.TreeNode
		add eax,edx ;eax = pointer to 2 node struct
		mov edx,eax ;edx = pointer to 2 node struct
		mov esi,eax ;esi = pointer to 2 node struct
		call tl_iterat_perv ;edx = pointer to 1 node struct
		call tl_get_node_index ;eax = index of 1 node struct
		cmp edx,ecx
		jle @f
			cmp dword[edx+TreeNode.next],ebx
			jne .po8
				call tl_node_move_po6 ;узлы идут подряд меняем 6 ссылок
				jmp .cur_mov
			.po8:
				call tl_node_move_po8 ;узлы идут не подряд меняем 8 ссылок
			.cur_mov:
				push dword edi
				call tl_cur_perv
				push dword edi
				call tl_draw
	@@:
popad
	ret
endp

;перемещаем узел вниз
align 4
proc tl_node_move_down tlist:dword
pushad
	mov edi,[tlist]
	call tl_get_cur_node_index ;eax=po_t
	cmp eax,2
	jl @f
		mov ebx,eax ;copy index of node struct
		mov edx,tl_data_nodes
		mov ecx,edx
		add ecx,sizeof.TreeNode
		imul eax,sizeof.TreeNode
		add eax,edx ;eax = pointer to 1 node struct
		mov edx,eax ;edx = pointer to 1 node struct
		mov esi,eax ;esi = pointer to 1 node struct
		call tl_iterat_next ;edx = pointer to 2 node struct
		call tl_get_node_index ;eax = index of 2 node struct
		cmp edx,ecx
		jle @f
			cmp dword[esi+TreeNode.next],eax
			jne .po8
				xchg eax,ebx ;меняе порядок следования заменяемых узлов
				xchg edx,esi
				call tl_node_move_po6 ;узлы идут подряд меняем 6 ссылок
				jmp .cur_mov
			.po8: ;а тут порядок следования узлов не меняем
				call tl_node_move_po8 ;узлы идут не подряд меняем 8 ссылок
			.cur_mov:
				stdcall tl_cur_next, edi
				stdcall tl_draw, edi
	@@:
	ret
popad
endp

align 4
tl_node_move_po6:
	mov ecx,edx ;save node pointer
	call tl_move_perv
	mov dword[edx+TreeNode.next],ebx

	mov edx,esi
	call tl_move_next
	mov dword[edx+TreeNode.perv],eax
	mov edx,ecx ;restore node pointer

	mov ecx,dword[edx+TreeNode.perv]
	mov dword[esi+TreeNode.perv],ecx
	mov ecx,dword[esi+TreeNode.next]
	mov dword[edx+TreeNode.next],ecx

	mov dword[edx+TreeNode.perv],ebx
	mov dword[esi+TreeNode.next],eax
	ret

;input
;eax = index 1 node struct
;ebx = index 2 node struct
;edx = pointer 1 node struct
;esi = pointer 2 node struct
;edi = pointer to 'TreeList' struct
;output:
;eax = ?
;ebx = ?
;ecx = ?
align 4
tl_node_move_po8:
	mov ecx,edx ;save node pointer
	call tl_move_perv
	mov dword[edx+TreeNode.next],ebx
	mov edx,ecx
	call tl_move_next
	mov dword[edx+TreeNode.perv],ebx
	mov edx,esi
	call tl_move_perv
	mov dword[edx+TreeNode.next],eax
	mov edx,esi
	call tl_move_next
	mov dword[edx+TreeNode.perv],eax
	mov edx,ecx ;restore node pointer

	mov eax,dword[edx+TreeNode.perv]
	mov ebx,dword[esi+TreeNode.perv]
	xchg eax,ebx
	mov dword[edx+TreeNode.perv],eax
	mov dword[esi+TreeNode.perv],ebx

	mov eax,dword[edx+TreeNode.next]
	mov ebx,dword[esi+TreeNode.next]
	xchg eax,ebx
	mov dword[edx+TreeNode.next],eax
	mov dword[esi+TreeNode.next],ebx
	ret

;input:
; edi = pointer to 'TreeList' struct
align 4
tl_draw_caption_cur_pos:
	cmp tl_capt_cy,9 ;9 - minimum caption height
	jl @f
	pushad
		mov ebx,edi ;calculate cursor position
		mov eax,tl_cur_pos
		inc eax
		lea edi,[txt_capt_cur.v]
		stdcall tl_convert_to_str, 5
		mov edi,ebx

		mov ebx,tl_box_left
		shl ebx,16
		add ebx,5*65536+3
		add ebx,tl_box_top
		mov ecx,tl_col_txt
		or  ecx,0xc0000000 ;0x40000000 закрашивать фон цветом edi
		lea edx,[txt_capt_cur]
		mov edi,tl_col_zag
		mcall SF_DRAW_TEXT ;captions
	popad
	@@:
	ret

;input:
; tlist - pointer to 'TreeList' struct
; opt - options: 0 - first element, 1 - add next element
; h_mem - pointer to memory
; mem_size - memory size
;output:
; eax - error code
align 4
proc tl_save_mem uses ebx ecx edx edi esi, tlist:dword, opt:dword, h_mem:dword, mem_size:dword
	mov esi,[h_mem]
	mov edi,[tlist]

	cmp dword[opt],0 ;add mode
	je @f
		stdcall tl_get_mem_size, edi,esi ;eax = размер ранее сохранённых данных
		add esi,eax
	@@:

	xor ebx,ebx
	mov bx,tl_info_size

	call tl_get_node_count_all ;eax = all node count

	mov ecx,eax  ;вычисляем сколько памяти должно быть заполнено
	imul ecx,ebx ;умножаем на размер структуры узла
	add ecx,tl_save_load_heder_size+1 ;element header +1 end element sumbol
	add ecx,esi  ;добавляем указатель на начало памяти (с учетом ранее записанных структур)
	sub ecx,dword[h_mem] ;отнимаем указатель на начало памяти (без ранее записанных структур)
	cmp ecx,dword[mem_size] ;ecx = element memory size
	jg .err_mem_size

		;save tree params (in header)
		mov dword[esi],'tree'
		mov word[esi+4],bx
		mov dword[esi+6],eax ;element count

		mov eax,tl_style
		mov dword[esi+10],eax

		mov eax,tl_cur_pos
		mov dword[esi+14],eax

		mov ax,tl_info_capt_offs
		mov word[esi+18],ax

		mov ax,tl_info_capt_len
		mov word[esi+20],ax

		;copy scroll position
		mov edx,tl_p_scroll
		mov eax,dword[edx+sb_offs_position]
		mov dword[esi+22],eax

		add esi,tl_save_load_heder_size ;add header size

		;cycle to nodes
		mov edx,tl_data_nodes
		mov ecx,edx
		add ecx,sizeof.TreeNode
		@@:
			call tl_iterat_next_all
			cmp edx,ecx
			jle @f
		;save node params
		call tl_get_node_index ;eax = index of pointer [edx]
		mov dword[esi],eax

		add esi,4
		mov eax,dword[edx+TreeNode.type] ;eax = (type; level; close)
		mov dword[esi],eax
		add esi,4

			stdcall tl_node_poi_get_data, edi,edx ;eax - pointer node data

		;call tl_node_copy_data
		push ecx edi
		mov edi,eax
		mov ecx,ebx
		xchg esi,edi
		rep movsb
		mov esi,edi
		pop edi ecx
		;add esi,ebx
		jmp @b
	@@:
	mov byte[esi],0 ;end of 'treelist'
		xor eax,eax ;return error code
	jmp @f
	.err_mem_size:
		mov eax,tl_err_save_memory_size
	@@:
	ret
endp

;input:
; tlist - pointer to 'TreeList' struct
; opt   - options: element index + (2*(add mode)+(init mode)) shl 16
; h_mem - pointer to memory
; mem_size - memory size
;   размер памяти, пока не используется (назначался для контроля)
;   для его использования нужно доработать функцию
;output:
; eax - error code
;memory header format:
;  +0 - (4) 'tree'
;  +4 - (2) info size
;  +6 - (4) count nodes
; +10 - (4) tlist style
; +14 - (4) cursor pos
; +18 - (2) info capt offs
; +20 - (2) info capt len
; +22 - (4) scroll pos
;memory data format:
; +26 - (info size + 8) * count nodes
align 4
proc tl_load_mem uses ebx ecx edx edi esi, tlist:dword, opt:dword, h_mem:dword, mem_size:dword 
locals
	er_code dd ?
endl
	mov esi,[h_mem]
	mov edi,[tlist]

	mov dword[er_code],0 ;return error code

	mov ecx,[opt]
	or cx,cx ;load in array mode
	jz @f
		;stdcall tl_get_mem_size, esi,edi ;берем размер ранее сохранённых данных
		;add esi,eax
		and ecx,0xffff
		cld
		.beg_cycle:
			cmp dword[esi],'tree'
			jne .no_tree
			movzx ebx,word[esi+4]
			add bx,8
			imul ebx,dword[esi+6]
			add ebx,tl_save_load_heder_size
			add esi,ebx
			loop .beg_cycle
	@@:

	cmp dword[esi],'tree'
	jne .no_tree
		bt dword[opt],17 ;load in add mode
		jc @f
			stdcall tl_info_clear, edi
		@@:

		xor ebx,ebx
		mov bx,word[esi+4] ;info_size
		cmp bx,tl_info_size
		je @f
			or dword[er_code],tl_err_load_info_size
		@@:
		mov ecx,[esi+6] ;count nodes
		cmp ecx,1
		jl .end_f
		mov edx,esi ;save header pointer
		add esi,tl_save_load_heder_size

		cld
		@@: ;load node params
			mov eax,dword[esi+4]
			ror eax,16 ;eax - options (type; level; close)
			add esi,8
			stdcall tl_node_add, edi,eax,esi
			stdcall tl_cur_next, edi
;...
			add esi,ebx
			loop @b

		bt dword[opt],17 ;load in add mode
		jc .no_tree
			mov eax,dword[edx+14] ;set cursor pos
			mov tl_cur_pos,eax
			mov ebx,tl_p_scroll
			or ebx,ebx
			jz .end_f
				mov eax,dword[edx+22] ;set scroll pos
				mov dword[ebx+sb_offs_position],eax
				stdcall scroll_bar_vertical.draw, ebx
				jmp .end_f
	.no_tree:
	mov dword[er_code],tl_err_load_caption
	.end_f:
	mov eax,dword[er_code]
	ret
endp

;берет размер памяти занятой функцией tl_save_mem при сохранении элементов
;output:
; eax - error code
align 4
proc tl_get_mem_size uses ebx edi, tlist:dword, h_mem:dword 
	mov edi,[tlist]
	mov eax,[h_mem]
	@@:
		cmp dword[eax],'tree'
		jne @f
		xor ebx,ebx
		mov bx,word[eax+4]
		add bx,8 ;размер дополнительной информации об узле (индекс записи; индекс иконки, уровень, ...)
		imul ebx,dword[eax+6]
		add ebx,tl_save_load_heder_size
		add eax,ebx
		jmp @b
	@@:
	sub eax,dword[h_mem] ;отнимаем указатель на начало памяти
		;и получаем размер блока памяти
	ret
endp


;ascii scan key
;  13    28 Enter
;  32    57 Space
; 178    72 Up
; 177    80 Down
; 176    75 Left
; 179    77 Right
; 182    83 Delete
; 184    73 Pg Up
; 183    81 Pg Dn

tl_key_ascii db 13,32,178,177,176,179,182,184,183
tl_key_scan  db 28,57, 72, 80, 75, 77, 83, 73, 81

txt_capt_cur: db 'Строка '
.v: db '     ',0
txt_capt_otm: db 'Отмены '
.v: db '     ',0

;description:
; преревод числа в ASCII строку
;input:
; eax = value
; edi = string buffer
align 4
proc tl_convert_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,10 ;задается система счисления
	cmp eax,ecx  ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx ;очистить edx
		div ecx     ;разделить - остаток в edx
		push edx    ;положить в стек
		call .str   ;вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30  ;добавляем символ '0'
		stosb       ;записать al в ячеку памяти [edi]
		mov byte[edi],0
	@@:
	ret
