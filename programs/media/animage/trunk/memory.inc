
mem_screen equ 1200*1000*3 ;размер памяти для области экрана
mem_panel_b equ 50*(20*20*3) ;размер памяти для панели инструментов
mem_flood_f equ 500000 ;размер буфера для заливки, а также для палитры

;----------------------------------------------------------
;--------change size of memory which use program-----------
;----------------------------------------------------------
GetMemory:
	pusha
	mov	ecx,[Picture_SizeX]
	mov	edx,[Picture_SizeY]
	imul	ecx,edx
	lea	ecx,[ecx+ecx*2]
	lea	ecx,[ecx+ecx*4] ;(Picture_SizeX*Picture_SizeY*3)*5
	add	ecx,mem_screen+mem_panel_b+mem_flood_f
	mcall SF_SYS_MISC,SSF_MEM_REALLOC,,[ScreenPointer]
	test eax,eax
	jnz	have_new_memory
	mov	esi,sound_havent_memory
	call	sound
	popa
	add	esp,4
	jmp	still

have_new_memory:
	mov	[ScreenPointer],eax
	call	prepare_data_pointers
	popa
	ret
;---------------------------------------------------------
prepare_data_pointers:
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	lea	eax,[eax+eax*2]
	mov	ebx,[ScreenPointer]
	add	ebx,mem_screen+mem_panel_b+mem_flood_f

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
	add	eax,mem_screen+mem_panel_b
	mov	[PointerToPalette],eax
	mov	[ReserveArray],eax

	call scrollbar_move_00 ;обнуление скроллинга
	ret
;---------------------------------------------------------