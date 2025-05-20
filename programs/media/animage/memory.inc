
mem_screen equ 1200*1000*3 ;размер памяти для области экрана
mem_panel_but equ 50*(20*20*3) ;память для панели инструментов
mem_panel_zoom equ 6*(18*21*3) ;память для панели с кнопками масштабов
mem_panel_all equ mem_panel_but+mem_panel_zoom ;размер памяти для
	;всех панелей инструментов
mem_flood_f equ 500000 ;размер буфера для заливки, а также для палитры
mem_offs_shape equ 4807*4 ;смещение для разделения массива ReserveArray

; Формулы примерного вычисления памяти для редактирования:
;
; mem_flood_f примерно равно 4*((1200^2+1000^2)^0.5)*3*(1+5*5)
;   Т.е. 4 диагонали экрана 1200 на 1000 умноженных на 26.
;   На каждый пиксель рисуемой фигуры приходится также область
;   памяти для запоминания фонового изображения, с учетом того
;   что максимальный размер кисти 5*5 получаем 26 = 1 + 5*5.
;
; Память ReserveArray делится на 2 области: 1-я для точек
;   рисуемой фигуры, 2-я для запоминания участков фона 5*5
;   прикрепленных к каждой точке рисуемой фигуры.
;   Разделение этой памяти происходит в пропорции 1 к 26,
;   что примерно равно: mem_flood_f / (1+5*5) или 4807*4.

;----------------------------------------------------------
;--------change size of memory which use program-----------
;----------------------------------------------------------
GetMemory:
	pusha
	mov	ecx,[Picture_SizeX]
	mov	edx,[Picture_SizeY]
	imul ecx,edx
	lea	ecx,[ecx+ecx*2]
	lea	ecx,[ecx+ecx*4] ;(Picture_SizeX*Picture_SizeY*3)*5
	add	ecx,mem_screen+mem_panel_all+mem_flood_f
	mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[ScreenPointer]
	test eax,eax
	jnz	.have_new_memory
	;mov esi,sound_havent_memory
	;call sound
	notify_window_run txt_not_memory
	popa
	add	esp,4
	jmp	still

.have_new_memory:
	mov	[ScreenPointer],eax
	call prepare_data_pointers
	popa
	ret
;---------------------------------------------------------
prepare_data_pointers:
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	lea	eax,[eax+eax*2]
	mov	ebx,[ScreenPointer]
	add	ebx,mem_screen+mem_panel_all+mem_flood_f

	mov	[PointerToPicture],ebx
	mov	[PointerToCopyPicture],ebx
	mov	[PointerToCopyPicture2],ebx
	mov	[PointerToEditBufer],ebx
	mov	[PointerToSpriteBufer],ebx

	add	[PointerToCopyPicture],eax

	add	[PointerToCopyPicture2],eax
	add	[PointerToCopyPicture2],eax

	add	[PointerToEditBufer],eax
	add	[PointerToEditBufer],eax
	add	[PointerToEditBufer],eax

	shl eax,2
	add	[PointerToSpriteBufer],eax

	mov	eax,[ScreenPointer]
	add	eax,mem_screen+mem_panel_all
	mov	[PointerToPalette],eax
	mov	[ReserveArray],eax

	call scrollbar_move_00 ;обнуление скроллинга
	ret
;---------------------------------------------------------