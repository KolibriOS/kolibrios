;
; Функции для преобразования файла *.stl в *.3ds
;
; Структура создаваемого файла *.3ds:
; CHUNK_MAIN (40+n+v+f)
; + CHUNK_OBJMESH (34+n+v+f)
;   + CHUNK_OBJBLOCK (28+n+v+f)
;     + CHUNK_TRIMESH (22+v+f)
;       + CHUNK_VERTLIST (8+v)
;       + CHUNK_FACELIST (8+f)
;
; в скобках указаны размеры блоков:
;   n - память для имени объекта
;   v - память для вершин
;   f - память для граней

; Формат бинарного *.stl:
; char[80] - заголовок
; uint32 - число граней
; для каждой грани:
; float[3] - вектор нормали
; float[9] - вершины 1,2,3
; uint16 - атрибуты


if lang eq ru
txt_err_stl_open:
	db '"STL',13,10
	db 'Файл *.stl очень большой." -tW',0
txt_err_stl_null_v:
	db '"STL',13,10
	db 'Вершины не найдены." -tE',0
else
txt_err_stl_open:
	db '"STL',13,10
	db 'File *.stl is very large." -tW',0
txt_err_stl_null_v:
	db '"STL',13,10
	db 'Vertexes not found." -tE',0
end if

;output:
; eax - указатель на сформированый файл 3ds (в случае неудачи 0)
; ecx - размер сформированого файла 3ds
align 4
proc convert_stl_3ds uses ebx edx edi esi, f_data:dword, f_size:dword
locals
	c_mem dd ? ;память для преобразования (convert memory)
	c_size dd ? ;размер памяти для преобразований (convert memory size)
	vert_c dd ? ;число вершин (vertex count)
	face_c dd ? ;число граней (faces count)
endl
	xor eax,eax
	mov esi,[f_data]
	cmp dword[esi],'soli'
	jne .bin_stl
	cmp word[esi+4],'d '
	jne .bin_stl

	jmp @f
	.bin_stl:
		;проверяем файл на бинарный формат
		mov ecx,[esi+80]
		imul ecx,50
		add ecx,84 ;заголовок
		cmp ecx,[f_size]
		jne .no_stl
		call convert_binary_stl
		jmp .no_stl
	@@:
	
	;в начале найдено 'solid '
	stdcall txt_next_line, 80

	mov eax,[f_data]
	sub eax,esi
	add eax,[f_size]
	stdcall get_stl_vertex_count, esi,eax
	or eax,eax
	jnz @f
		notify_window_run txt_err_stl_null_v
		jmp .no_stl
	@@:
	cmp eax,0xffff
	jle @f
		notify_window_run txt_err_stl_open
		mov eax,0xffff
	@@:

	mov [vert_c],eax
	mov ecx,3
	xor edx,edx
	div ecx
	mov [face_c],eax
	shl eax,3
	mov ecx,[vert_c]
	imul ecx,12
	lea ecx,[ecx+eax+40]
	lea edx,[esi-6]
	sub edx,[f_data]
	cmp edx,2
	jge @f
		mov edx,2 ;минимальный размер для имени объекта
	@@:
	add ecx,edx ;for object name
	mov [c_size],ecx
	stdcall mem.Alloc,ecx
	mov [c_mem],eax
	mov ebx,eax

	mov word[ebx],CHUNK_MAIN
	mov dword[ebx+2],ecx
	add ebx,6

	;3d3d
	mov word[ebx],CHUNK_OBJMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4000
	mov word[ebx],CHUNK_OBJBLOCK
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	push ecx esi
		mov ecx,edx
		mov edi,ebx
		mov esi,[f_data]
		add esi,6 ;пропускаем 'solid '
		rep movsb ;копируем имя объекта
		mov byte[edi-1],0
		add ebx,edx
	pop esi ecx

	;4100
	mov word[ebx],CHUNK_TRIMESH 
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4110
	mov word[ebx],CHUNK_VERTLIST 
	mov dword[ebx+2],8 ;+ число вершин * 12
	add ebx,6

	mov edx,ebx
	mov word[edx],0 ;кол. вершин
	add ebx,2
	finit
	.cycle0:
		call txt_ignore_space
		cmp dword[esi],'face'
		jne .end_v
		stdcall txt_next_line, 80

		call txt_ignore_space
		cmp dword[esi],'oute'
		jne .end_v
		stdcall txt_next_line, 80

		mov ecx,3 ;3 точки на 1 грань
		@@:
			stdcall stl_vertex_init, ebx
			or eax,eax
			jz .end_v
			add ebx,12
			inc word[edx]
			loop @b

		mov eax,[vert_c]
		cmp word[edx],ax
		jge .end_v
		call txt_ignore_space
		cmp dword[esi],'endl'
		jne .end_v
		stdcall txt_next_line, 80

		call txt_ignore_space
		cmp dword[esi],'endf'
		jne .end_v
		stdcall txt_next_line, 80
		jmp .cycle0
	.end_v:

	movzx eax,word[edx]
	imul eax,12
	add [edx-4],eax ;исправляем размер блока 4110

	;4120
	mov word[ebx],CHUNK_FACELIST
	mov ecx,[face_c]
	mov edx,ecx
	shl edx,3 ;кол. граней * 8
	add edx,8
	mov [ebx+2],edx
	add ebx,6
	mov [ebx],cx ;кол. граней
	add ebx,2
	xor eax,eax
	@@:
		mov [ebx],ax
		inc eax
		mov [ebx+2],ax
		inc eax
		mov [ebx+4],ax
		inc eax
		mov word[ebx+6],0 ;атрибуты
		add ebx,8
		loop @b

	mov eax,[c_mem]
	mov ecx,[c_size]
	.no_stl:
	ret
endp

;input:
; esi - указатель на начало файла
; ecx - размер файла
align 4
proc convert_binary_stl
locals
	c_mem dd ? ;память для преобразования (convert memory)
	c_size dd ? ;размер памяти для преобразований (convert memory size)
	vert_c dd ? ;число вершин (vertex count)
	face_c dd ? ;число граней (faces count)
endl
	mov eax,[esi+80]
	or eax,eax
	jnz @f
		notify_window_run txt_err_stl_null_v
		jmp .no_stl
	@@:
	cmp eax,0xffff/3
	jle @f
		notify_window_run txt_err_stl_open
		mov eax,0xffff/3
	@@:

	mov [face_c],eax
	mov ecx,eax
	imul eax,3
	mov [vert_c],eax
	shl ecx,3
	imul eax,12
	lea ecx,[ecx+eax+40+4] ;+4 размер для имени объекта
	mov [c_size],ecx
	stdcall mem.Alloc,ecx
	mov [c_mem],eax
	mov ebx,eax

	mov word[ebx],CHUNK_MAIN
	mov dword[ebx+2],ecx
	add ebx,6

	;3d3d
	mov word[ebx],CHUNK_OBJMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4000
	mov word[ebx],CHUNK_OBJBLOCK
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6
	mov dword[ebx],'Stl' ;имя объекта
	add ebx,4

	;4100
	mov word[ebx],CHUNK_TRIMESH 
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4110
	mov word[ebx],CHUNK_VERTLIST 
	mov dword[ebx+2],8 ;+ число вершин * 12
	add ebx,6

	mov edx,ebx
	mov ecx,[vert_c]
	mov word[edx],cx ;кол. вершин
	add ebx,2
	
	add esi,80+4+12 ;пропуск заголовка, числа граней, 1-го вектора нормалей
	mov edi,ebx
	mov eax,[face_c]
	@@: ;цикл по граням
		mov ecx,9
		rep movsd ;копируем координаты 3-х вершин
		add esi,14 ;пропуск вектора нормалей и атрибутов
		dec eax
		jnz @b
	mov ebx,edi

	movzx eax,word[edx]
	imul eax,12
	add [edx-4],eax ;исправляем размер блока 4110

	;4120
	mov word[ebx],CHUNK_FACELIST
	mov ecx,[face_c]
	mov edx,ecx
	shl edx,3 ;кол. граней * 8
	add edx,8
	mov [ebx+2],edx
	add ebx,6
	mov [ebx],cx ;кол. граней
	add ebx,2
	xor eax,eax
	@@:
		mov [ebx],ax
		inc eax
		mov [ebx+2],ax
		inc eax
		mov [ebx+4],ax
		inc eax
		mov word[ebx+6],0 ;атрибуты
		add ebx,8
		loop @b

	mov eax,[c_mem]
	mov ecx,[c_size]
	.no_stl:
	ret
endp

;output:
; eax - vertex count
align 4
proc get_stl_vertex_count uses ebx ecx edi, f_data:dword, f_size:dword
	mov al,'v'
	xor ebx,ebx
	mov ecx,[f_size]
	mov edi,[f_data]
	.cycle0:
	repne scasb
	cmp dword[edi],'erte'
	jne @f
		inc ebx
		add edi,4
		sub ecx,4
		cmp ecx,4
		jg .cycle0
	@@:
	mov eax,ebx
	ret
endp

;input:
; esi - input description text
; ppoi - pointer to vertex struct
;output:
; eax - 0 (if error init) or 1
; esi - output description text
align 4
proc stl_vertex_init uses ebx ecx edi, ppoi:dword
	call txt_ignore_space
	cmp dword[esi],'vert'
	jne .err_init
	cmp word[esi+4],'ex'
	jne .err_init
		add esi,6
		mov ebx,[ppoi]

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx] ;coord X

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx+4] ;coord X

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx+8] ;coord Z

		stdcall txt_next_line, 80
		jmp @f
	.err_init:
		xor eax,eax
		jmp .end_f
	@@:
		xor eax,eax
		inc eax
	.end_f:
	ret
endp

;input:
; esi - указатель на начало строки с пробелами
;output:
; al - destroy
; ecx - destroy
; esi - указатель на первый непробельный символ
align 4
txt_ignore_space:
	mov	ecx,64 ;защита от зацикливания
	@@:
		lodsb
		cmp	al,' '
		jne	@f
		loop @b
	@@:
	dec esi
	ret

;input:
; esi - указатель на строку с числом (пробелы в начале строки игнорируються)
;output:
; al - destroy
; ecx - destroy
; edi - destroy
; esi - указатель на конец копированного числа
; Data_String - строка с числом из esi
align 4
txt_copy_data:
	call txt_ignore_space
	mov ecx,32
	mov edi,esi
	@@:
		lodsb
		or al,al
		jz @f
		cmp al,' '
		je @f
		cmp al,13
		je @f
		loop @b
	@@:
	mov esi,edi
	sub ecx,32
	neg ecx
	mov edi,Data_String
	rep movsb
	mov byte[edi],0
	ret

;input:
; esi - text pointer
align 4
proc txt_next_line uses eax ecx edi, mlen:dword
	mov al,13
	mov ecx,[mlen]
	mov edi,esi
	repne scasb
	cmp byte[edi],10
	jne @f
		inc edi
	@@:
	mov esi,edi
	ret
endp