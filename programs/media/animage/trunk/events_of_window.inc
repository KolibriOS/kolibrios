;----------------------------------------------------------
;-------------draw window of program-----------------------
;----------------------------------------------------------
drawwin:

	mov eax,9
	mov ebx,IPC_table
	or ecx,-1
	mcall

	mov eax,[IPC_table+34]
	mov ebx,[IPC_table+38]
	mov ecx,[IPC_table+42]
	mov edx,[IPC_table+46]
	mov [Window_CordinatX],eax
	mov [Window_CordinatY],ebx
	mov [Window_SizeX],ecx
	mov [Window_SizeY],edx

	cmp [Window_SizeX],585
	jae no_minimum_size_x

	mov [Window_SizeX],585
	mov ecx,[Window_SizeX]

	no_minimum_size_x:

	cmp [Window_SizeY],400
	jae no_minimum_size_y

	mov [Window_SizeY],400
	mov edx,[Window_SizeY]

	no_minimum_size_y:

	mov [MaxWorkScreen_SizeX],ecx
	mov [MaxWorkScreen_SizeY],edx
	sub [MaxWorkScreen_SizeX],20+10+5+10
	sub [MaxWorkScreen_SizeY],20+10+15+1+49+20+10

	cmp [MaxWorkScreen_SizeX],0
	jns no_znak1

	mov [MaxWorkScreen_SizeX],ecx

	no_znak1:

	cmp [MaxWorkScreen_SizeY],0
	jns no_znak2

	mov [MaxWorkScreen_SizeY],edx

	no_znak2:

	mov eax,12
	mov ebx,1
	mcall

	and eax,0
	mov ebx,[Window_CordinatX]
	mov ecx,[Window_CordinatY]
	shl ebx,16
	shl ecx,16
	add ebx,[Window_SizeX]
	add ecx,[Window_SizeY]
	mov edx,0x03aabbcc
	mov esi,0x805080d0
	mov edi,0x005080d0
	mcall

	call draw_panel
	call PrintMousePos

	mov [counter],7
	mov edi,panel_text
	call print_panel_text

	;>>>>>>Leency
	;mov eax,10
	;mov ebx,5
	;mov ecx,0xffffff
	;mov edx,name_of_program
	;mov esi,34
	;call print_text

	mov eax,71
	mov ebx,1
	mov ecx,name_of_program
	int 40h
	;Leency<<<<<<<

	mov eax,12
	mov ebx,2
	mcall

	mov ecx,[k]

	and [PosX],0
	and [PosY],0
	call MovePictureToWorkScreen
	call draw_scrollers
	call draw_icons

	ret