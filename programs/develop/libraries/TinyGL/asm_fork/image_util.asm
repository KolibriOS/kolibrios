;include 'zgl.inc'

align 16
proc gl_resizeImage uses ecx edi esi, dest:dword, xsize_dest:dword, ysize_dest:dword,\
	src:dword, xsize_src:dword, ysize_src:dword

	;сжатие по ширине
	mov edi,[xsize_src]
	cmp edi,[xsize_dest]
	jle @f
		stdcall img_rgb24_wresize, [src],edi,[ysize_src],[xsize_dest]
		mov edi,[xsize_dest]
	@@:

	;сжатие по высоте
	mov ecx,[ysize_src]
	cmp ecx,[ysize_dest]
	jle @f
		stdcall img_rgb24_hresize, [src],edi,ecx,[ysize_dest]
		mov ecx,[ysize_dest]
	@@:

	;копирование сжатой текстуры
	imul ecx,edi
	mov edi,[dest]
	mov esi,[src]
	imul ecx,3
	rep movsb
	ret
endp

;description:
; сжатие изображения по ширине (размеры буфера не меняются)
;input:
; data_rgb - pointer to rgb data
; size_w - width img in pixels
; size_h - height img in pixels
; size_w_new - new width img in pixels
align 16
proc img_rgb24_wresize, data_rgb:dword, size_w:dword, size_h:dword, size_w_new:dword
locals
	pr dd 0
	pg dd 0
	pb dd 0
	img_n dd ? ;указатель на данные нового изображения
	lines dd ?
endl
pushad
;eax - delta for inp. img
;ebx - delta for outp. img
;esi - pointer to data_rgb
	mov esi,[data_rgb]
	mov [img_n],esi
	mov eax,[size_h]
	mov [lines],eax
align 4
	.cycyle_0:
	mov eax,[size_w_new]
	mov ecx,[size_w]
	mov ebx,ecx
align 4
	.cycyle_1:
		cmp eax,ebx
		jg .else_0
			;копируемый пиксель максимально влияет на результат
			;накапливаем rgb для интерполяции пикселей
			mov edx,[size_w_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			cmp eax,ebx
			je .d2_add
			jmp .if_0_end
		.else_0:
			;копируемый пиксель попадет на границу пикселей
			mov edx,ebx
			sub edx,eax
			add edx,[size_w_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			;сохраняем готовое rgb
			.d2_add:
			push eax
				mov edi,[img_n]
				mov eax,[pb]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
				mov eax,[pg]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
				mov eax,[pr]
				xor edx,edx
				div dword[size_w] ;eax /= [size_w]
				stosb
			pop eax
			add dword[img_n],3 ;next pixel
			;обновляем rgb для нового пикселя
			mov edx,eax
			sub edx,ebx
			movzx edi,byte[esi]
			imul edi,edx
			mov [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			mov [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			mov [pr],edi
			add ebx,[size_w]
		.if_0_end:
		add eax,[size_w_new]
		add esi,3 ;next pixel
		dec ecx
		jnz .cycyle_1
	dec dword[lines]
	jnz .cycyle_0
popad
	ret
endp

;description:
; сжатие изображения по высоте (размеры буфера не меняются)
;input:
; data_rgb - pointer to rgb data
; size_w - width img in pixels
; size_h - height img in pixels
; size_h_new - new height img in pixels
align 16
proc img_rgb24_hresize, data_rgb:dword, size_w:dword, size_h:dword, size_h_new:dword
locals
	pr dd 0
	pg dd 0
	pb dd 0
	img_n dd ? ;указатель на данные нового изображения
	cols dd ?
	lin_b dd ? ;размер линии изображения в байтах
	data_n dd ? ;указатель на данные для нового столбца пикселей
endl
pushad
;eax - delta for inp. img
;ebx - delta for outp. img
;esi - pointer to data_rgb
	mov esi,[data_rgb]
	mov [data_n],esi
	mov eax,[size_w]
	mov [cols],eax
	lea eax,[eax+eax*2]
	mov [lin_b],eax
align 4
	.cycyle_0:
	mov eax,[size_h_new]
	mov ecx,[size_h]
	mov ebx,ecx
	mov esi,[data_n]
	mov [img_n],esi
	add dword[data_n],3 ;переход на следующий столбец пикселей
align 4
	.cycyle_1:
		cmp eax,ebx
		jg .else_0
			;копируемый пиксель максимально влияет на результат
			;накапливаем rgb для интерполяции пикселей
			mov edx,[size_h_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			cmp eax,ebx
			je .d2_add
			jmp .if_0_end
		.else_0:
			;копируемый пиксель попадет на границу пикселей
			mov edx,ebx
			sub edx,eax
			add edx,[size_h_new]
			movzx edi,byte[esi]
			imul edi,edx
			add [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			add [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			add [pr],edi
			;сохраняем готовое rgb
			.d2_add:
			push eax
				mov edi,[img_n]
				mov eax,[pb]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
				mov eax,[pg]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
				mov eax,[pr]
				xor edx,edx
				div dword[size_h] ;eax /= [size_h]
				stosb
			pop eax
			mov edx,[lin_b]
			add dword[img_n],edx ;next pixel
			;обновляем rgb для нового пикселя
			mov edx,eax
			sub edx,ebx
			movzx edi,byte[esi]
			imul edi,edx
			mov [pb],edi
			movzx edi,byte[esi+1]
			imul edi,edx
			mov [pg],edi
			movzx edi,byte[esi+2]
			imul edi,edx
			mov [pr],edi
			add ebx,[size_h]
		.if_0_end:
		add eax,[size_h_new]
		add esi,[lin_b] ;next pixel
		dec ecx
		jnz .cycyle_1
	dec dword[cols]
	jnz .cycyle_0
popad
	ret
endp

align 4
proc gl_getPervPowerOfTwo uses ebx, n:dword
	mov ebx,[n]
	mov eax,(1 shl ZB_POINT_TEXEL_SIZE) ;max size
	cmp ebx,eax
	jge .set
	@@:
		shr eax,1
		cmp ebx,eax
		jl @b
	cmp eax,8 ;min size
	jge .set
		mov eax,8
	.set:
	ret
endp