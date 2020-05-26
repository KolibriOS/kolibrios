;
; файл для работы с выделенными точками,
; по которым делается преобразование изображения
;[0-3] - рамка (исх. и.)
;[4] - средняя точка (исх. и.)
;[5-6] - рамка (вых. и.)
;[7] - средняя точка (вых. и.)
;

struct point2d
	x dd ? ;координата относит изображения
	y dd ?
	tx dd ? ;координата для текстуры
	ty dd ?
	captx rb 6
	capty rb 6
ends

;настройка точек при загрузке программы
;(точки не активны)
align 4
points_init:
	xor eax,eax
	dec eax
	mov ecx,8
	mov edi,sel_pt
	@@:
		mov [edi+point2d.x],eax
		mov [edi+point2d.y],eax
		add edi,sizeof.point2d
		loop @b
	ret

;настройка точек при открытии изображения
;(точки ставятся по углам изображения)
align 4
proc points_init_2 uses eax ecx edx edi, ot_x:dword, ot_y:dword
	mov eax,[ot_y]
	mov edi,sel_pt
	mov edx,[ot_x]
	mov [edi+point2d.x],edx ;1
	mov [edi+point2d.y],eax ;1
	add edi,sizeof.point2d
	mov ecx,[buf_i0.w]
	sub ecx,edx
	mov [edi+point2d.x],ecx ;2
	mov [edi+point2d.y],eax ;2
	add edi,sizeof.point2d
	mov [edi+point2d.x],ecx ;3
	mov ecx,[buf_i0.h]
	sub ecx,eax
	mov [edi+point2d.y],ecx ;3
	add edi,sizeof.point2d
	mov [edi+point2d.x],edx ;4
	mov [edi+point2d.y],ecx ;4
	call points_update_prop
	ret
endp

align 4
proc points_update_prop uses eax ebx ecx edi esi
	;вычисление координат средней точки
	mov edi,sel_pt
	xor eax,eax
	xor ebx,ebx
	mov ecx,4
	@@:
		add eax,[edi+point2d.x]
		add ebx,[edi+point2d.y]
		add edi,sizeof.point2d
		loop @b
	shr eax,2
	mov [edi+point2d.x],eax
	shr ebx,2
	mov [edi+point2d.y],ebx
	add edi,sizeof.point2d
	
	cmp byte[trans_a],0 ;проверяем алгоритм растягивания текстуры
	je @f
		;на весь буфер
		xor eax,eax
		mov ebx,edi
		mov edi,buf_i0
		mov [ebx+point2d.x],eax
		mov [ebx+point2d.y],eax
		add ebx,sizeof.point2d
		mov eax,buf2d_w
		mov [ebx+point2d.x],eax
		mov ecx,buf2d_h
		mov [ebx+point2d.y],ecx
		add ebx,sizeof.point2d
		shr eax,1
		mov [ebx+point2d.x],eax
		shr ecx,1
		mov [ebx+point2d.y],ecx
		jmp .end0
	@@:
		;на часть буфера
		mov eax,[edi+point2d.x-5*sizeof.point2d]
		add eax,[edi+point2d.x-2*sizeof.point2d]
		shr eax,1
		mov [edi+point2d.x],eax
		mov eax,[edi+point2d.x-4*sizeof.point2d]
		add eax,[edi+point2d.x-3*sizeof.point2d]
		shr eax,1
		mov [edi+point2d.x+sizeof.point2d],eax

		mov eax,[edi+point2d.y-5*sizeof.point2d]
		add eax,[edi+point2d.y-4*sizeof.point2d]
		shr eax,1
		mov [edi+point2d.y],eax
		mov eax,[edi+point2d.y-3*sizeof.point2d]
		add eax,[edi+point2d.y-2*sizeof.point2d]
		shr eax,1
		mov [edi+point2d.y+sizeof.point2d],eax
		
		lea esi,[edi-sizeof.point2d]
		add edi,2*sizeof.point2d
		movsd ;copy coord x
		movsd ;copy coord y
	.end0:
	
	mov edi,buf_i0
	mov ebx,sel_pt
	mov ecx,8
	finit
	.cycle0:
		fild dword[ebx+point2d.x]
		fidiv buf2d_w
		fst dword[ebx+point2d.tx]
		
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		m2m dword[ebx+point2d.captx],dword[Data_String]
		mov al,byte[Data_String+4]
		mov byte[ebx+point2d.captx+4],al
		
		fild dword[ebx+point2d.y]
		fidiv buf2d_h
		fst dword[ebx+point2d.ty]
		
		fstp qword[Data_Double]
		call DoubleFloat_to_String
		call String_crop_0
		m2m dword[ebx+point2d.capty],dword[Data_String]
		mov al,byte[Data_String+4]
		mov byte[ebx+point2d.capty+4],al

		add ebx,sizeof.point2d
		loop .cycle0

	call draw_transf_texture
	ret
endp

;рисование выбранных точек
align 4
proc points_draw
locals
	PervX dd 0 ;pervios point coord X
	PervY dd 0 ;pervios point coord Y
	GrafX dd ? ;active point coord X
	GrafY dd ? ;active point coord Y
endl
	mov edi,buf_ogl
	cmp buf2d_data,0
	je .endf

	xor ecx,ecx
	mov edi,sel_pt
	mov dword[txt_buf],'0'

	cmp byte[view_b],1
	je .cycle0end
	;берем координаты последней точки
	mov eax,[edi+3*sizeof.point2d+point2d.x]
	add eax,[nav_x]
	mov [PervX],eax
	mov ebx,[edi+3*sizeof.point2d+point2d.y]
	add ebx,[nav_y]
	mov [PervY],ebx
align 4
	.cycle0:
		cmp dword[edi+point2d.x],-1
		je .end0
			mov eax,[edi+point2d.x]
			add eax,[nav_x]
			mov [GrafX],eax
			sub eax,3
			mov ebx,[edi+point2d.y]
			add ebx,[nav_y]
			mov [GrafY],ebx
			sub ebx,3
			cmp ecx,[sel_act]
			je @f
				stdcall [buf2d_filled_rect_by_size], buf_0, eax,ebx, 5,5, 0xffff00
			@@:
			dec eax
			dec ebx
			stdcall [buf2d_rect_by_size], buf_0, eax,ebx, 7,7, 0x808000
			call point_draw_info
			;рисование линии
			stdcall [buf2d_line], buf_0, [PervX],[PervY], [GrafX],[GrafY], 0x808000
			mov eax,[GrafX]
			mov [PervX],eax
			mov eax,[GrafY]
			mov [PervY],eax
		.end0:
		add edi,sizeof.point2d
		inc ecx
		cmp ecx,4
		jl .cycle0
		jmp .cycle1end
	.cycle0end:
	add edi,5*sizeof.point2d
align 4
	.cycle1:
		cmp dword[edi+point2d.x],-1
		je .end1
			mov eax,[GrafX]
			mov [PervX],eax
			mov eax,[GrafY]
			mov [PervY],eax
			mov eax,[edi+point2d.x]
			add eax,[nav_x]
			mov [GrafX],eax
			sub eax,3
			mov ebx,[edi+point2d.y]
			add ebx,[nav_y]
			mov [GrafY],ebx
			sub ebx,3
			stdcall [buf2d_filled_rect_by_size], buf_0, eax,ebx, 5,5, 0xffff00
			dec eax
			dec ebx
			stdcall [buf2d_rect_by_size], buf_0, eax,ebx, 7,7, 0x808000
			call point_draw_info
		.end1:
		add edi,sizeof.point2d
		inc ecx
		cmp ecx,2
		jl .cycle1
			mov eax,[GrafX]
			sub eax,[PervX]
			mov [GrafX],eax
			mov eax,[GrafY]
			sub eax,[PervY]
			mov [GrafY],eax
		stdcall [buf2d_rect_by_size], buf_0, [PervX],[PervY], [GrafX],[GrafY], 0x808000
	.cycle1end:
	;средняя точка
	mov eax,[edi+point2d.x]
	add eax,[nav_x]
	sub eax,3
	mov ebx,[edi+point2d.y]
	add ebx,[nav_y]
	sub ebx,3
	stdcall [buf2d_filled_rect_by_size], buf_0, eax,ebx, 5,5, 0xff0000
	dec eax
	dec ebx
	stdcall [buf2d_rect_by_size], buf_0, eax,ebx, 7,7, 0x808000
	add ebx,9
	inc byte[txt_buf]
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0
	.endf:
	ret
endp

;input:
; eax - coord x
; ebx - coord y
; edi - указатель на структуру point2d
align 4
point_draw_info:
	;вывод текста
	add ebx,9
	inc byte[txt_buf]
	inc eax
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0xffffff
	sub eax,2
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0xffffff
	inc eax
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0xffffff
	;dec ebx
	sub ebx,2
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0xffffff
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,txt_buf,eax,ebx,0
	add ebx,9
	lea edx,[edi+point2d.captx]
	inc eax
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	sub eax,2
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	inc eax
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	;dec ebx
	sub ebx,2
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0
	add ebx,9
	lea edx,[edi+point2d.capty]
	inc eax
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	sub eax,2
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	inc eax
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	;dec ebx
	sub ebx,2
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0xffffff
	inc ebx
	stdcall [buf2d_draw_text], buf_0, buf_font,edx,eax,ebx,0
	ret

;рисование преобразованной текстуры
align 4
draw_transf_texture:
	cmp dword[buf_ogl],0
	je .endf
	stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;очистим буфер цвета и глубины
	stdcall [glBegin],GL_TRIANGLES

	stdcall [glTexCoord2f], dword[sel_pt+4*sizeof.point2d+point2d.tx], dword[sel_pt+4*sizeof.point2d+point2d.ty] ;5
	stdcall [glVertex3f], dword[sel_pt+7*sizeof.point2d+point2d.tx], dword[sel_pt+7*sizeof.point2d+point2d.ty], 0.1 ;8
	stdcall [glTexCoord2f], dword[sel_pt+point2d.tx], dword[sel_pt+point2d.ty] ;1
	stdcall [glVertex3f], dword[sel_pt+5*sizeof.point2d+point2d.tx], dword[sel_pt+5*sizeof.point2d+point2d.ty], 0.1 ;6
	stdcall [glTexCoord2f], dword[sel_pt+sizeof.point2d+point2d.tx], dword[sel_pt+sizeof.point2d+point2d.ty] ;2
	stdcall [glVertex3f], dword[sel_pt+6*sizeof.point2d+point2d.tx], dword[sel_pt+5*sizeof.point2d+point2d.ty], 0.1 ;7|6

	stdcall [glTexCoord2f], dword[sel_pt+4*sizeof.point2d+point2d.tx], dword[sel_pt+4*sizeof.point2d+point2d.ty] ;5
	stdcall [glVertex3f], dword[sel_pt+7*sizeof.point2d+point2d.tx], dword[sel_pt+7*sizeof.point2d+point2d.ty], 0.1 ;8
	stdcall [glTexCoord2f], dword[sel_pt+sizeof.point2d+point2d.tx], dword[sel_pt+sizeof.point2d+point2d.ty] ;2
	stdcall [glVertex3f], dword[sel_pt+6*sizeof.point2d+point2d.tx], dword[sel_pt+5*sizeof.point2d+point2d.ty], 0.1 ;7|6
	stdcall [glTexCoord2f], dword[sel_pt+2*sizeof.point2d+point2d.tx], dword[sel_pt+2*sizeof.point2d+point2d.ty] ;3
	stdcall [glVertex3f], dword[sel_pt+6*sizeof.point2d+point2d.tx], dword[sel_pt+6*sizeof.point2d+point2d.ty], 0.1 ;7
	
	stdcall [glTexCoord2f], dword[sel_pt+4*sizeof.point2d+point2d.tx], dword[sel_pt+4*sizeof.point2d+point2d.ty] ;5
	stdcall [glVertex3f], dword[sel_pt+7*sizeof.point2d+point2d.tx], dword[sel_pt+7*sizeof.point2d+point2d.ty], 0.1 ;8
	stdcall [glTexCoord2f], dword[sel_pt+2*sizeof.point2d+point2d.tx], dword[sel_pt+2*sizeof.point2d+point2d.ty] ;3
	stdcall [glVertex3f], dword[sel_pt+6*sizeof.point2d+point2d.tx], dword[sel_pt+6*sizeof.point2d+point2d.ty], 0.1 ;7
	stdcall [glTexCoord2f], dword[sel_pt+3*sizeof.point2d+point2d.tx], dword[sel_pt+3*sizeof.point2d+point2d.ty] ;4
	stdcall [glVertex3f], dword[sel_pt+5*sizeof.point2d+point2d.tx], dword[sel_pt+6*sizeof.point2d+point2d.ty], 0.1 ;6|7
	
	stdcall [glTexCoord2f], dword[sel_pt+4*sizeof.point2d+point2d.tx], dword[sel_pt+4*sizeof.point2d+point2d.ty] ;5
	stdcall [glVertex3f], dword[sel_pt+7*sizeof.point2d+point2d.tx], dword[sel_pt+7*sizeof.point2d+point2d.ty], 0.1 ;8
	stdcall [glTexCoord2f], dword[sel_pt+3*sizeof.point2d+point2d.tx], dword[sel_pt+3*sizeof.point2d+point2d.ty] ;4
	stdcall [glVertex3f], dword[sel_pt+5*sizeof.point2d+point2d.tx], dword[sel_pt+6*sizeof.point2d+point2d.ty], 0.1 ;6|7
	stdcall [glTexCoord2f], dword[sel_pt+point2d.tx], dword[sel_pt+point2d.ty] ;1
	stdcall [glVertex3f], dword[sel_pt+5*sizeof.point2d+point2d.tx], dword[sel_pt+5*sizeof.point2d+point2d.ty], 0.1 ;6

	stdcall [glEnd]
	.endf:
	ret
