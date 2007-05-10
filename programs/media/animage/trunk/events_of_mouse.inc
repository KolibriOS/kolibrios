;---------------------------------------------
;-------------interraction MOUSE--------------
;---------------------------------------------
mouse:

	call GetMouseCoordinats

	;panel
	mov edx,[Window_SizeX]
	sub edx,5

	cmp ebx,20
	jle no_panel

	cmp ebx,20+15
	jae no_panel

	cmp eax,5
	jle no_panel

	cmp eax,edx ;585
	jae no_panel

	call GetMouseCoordinats

	mov [counter],7
	mov edi,panel_text
	call panel_interraction

	jmp still
	no_panel:

	cmp [Panel_flag],0
	jz no_redraw_panel

	mov [counter],7
	mov edi,panel_text
	call panel_interraction

	no_redraw_panel:

	call GetMouseCoordinats
	mov eax,[MouseX]
	mov ebx,[MouseY]
	mov ecx,[WorkScreen_SizeX]
	mov edx,[WorkScreen_SizeY]
	add ecx,9
	add edx,89

	cmp eax,ecx
	jae mouse_scroll

	cmp ebx,edx
	jae mouse_scroll

	jmp no_scrollers

	mouse_scroll:

	mov [exit_from_work_arrea],1  ;mouse situated after work arrea

	;scrollers
	call GetMouseClick

	cmp eax,1
	jne no_click

	call GetMouseCoordinats
	;interraction with horizontal scroller
	mov eax,[Scroll1CoordinatX]
	mov ebx,[Scroll1CoordinatY]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[Scroll1MaxSizeX]
	mov edi,[Scroll1MaxSizeY]
	call columnus

	test eax,eax
	jz no_horizontal


	mov eax,9
	mov ebx,[Scroll1CoordinatY]
	mov ecx,[Scroll1MaxSizeX]
	mov edx,14+10
	mov esi,7
	inc ecx
	call draw_volume_rectangle

	mov eax,[MouseX]
	mov ebx,[Scroll1SizeX]
	shr ebx,1
	sub eax,ebx
	mov ecx,[MouseX]
	add ecx,ebx
	mov edx,8+1
	add edx,[Scroll1MaxSizeX]
	mov [Scroll1CoordinatX],eax

	sub eax,9
	jns no_min_scroll

	mov [Scroll1CoordinatX],9

	no_min_scroll:

	cmp ecx,edx
	jl no_max_scroll

	sub edx,ebx
	sub edx,ebx
	mov [Scroll1CoordinatX],edx

	no_max_scroll:
	;
	mov eax,[Scroll1CoordinatX]
	sub eax,9
	mov ebx,[Picture_SizeX]
	imul eax,ebx
	mov ebx,[Scroll1MaxSizeX]
	cdq
	idiv ebx
	mov [PosX],eax

	jmp no_vertical

	no_horizontal:

	mov eax,[Scroll2CoordinatX]
	mov ebx,[Scroll2CoordinatY]
	mov ecx,[MouseX]
	mov edx,[MouseY]
	mov esi,[Scroll2MaxSizeX]
	mov edi,[Scroll2MaxSizeY]
	call columnus

	test eax,eax
	jz no_vertical

	mov eax,[Scroll2CoordinatX]
	mov ebx,89
	mov ecx,14+10
	mov edx,[Scroll2MaxSizeY]
	mov esi,7
	inc edx
	call draw_volume_rectangle

	mov eax,[MouseY]
	mov ebx,[Scroll2SizeY]
	shr ebx,1
	sub eax,ebx
	mov ecx,[MouseY]
	add ecx,ebx
	mov edx,89
	add edx,[Scroll2MaxSizeY]
	mov [Scroll2CoordinatY],eax

	sub eax,89
	jns no_min_scroll2

	mov [Scroll2CoordinatY],89

	no_min_scroll2:

	cmp ecx,edx
	jl no_max_scroll2

	sub edx,ebx
	sub edx,ebx
	mov [Scroll2CoordinatY],edx

	no_max_scroll2:
	;
	mov eax,[Scroll2CoordinatY]
	sub eax,89
	mov ebx,[Picture_SizeY]
	imul eax,ebx
	mov ebx,[Scroll2MaxSizeY]
	cdq
	idiv ebx
	mov [PosY],eax

	no_vertical:

	call CalculatePositionScreen

	call draw_scrollers
	call MovePictureToWorkScreen

	no_click:

	jmp still
	no_scrollers:


	mov eax,[MouseX]
	mov ebx,[MouseY]
	mov ecx,[Window_SizeX]
	mov edx,[Window_SizeY]
	sub ecx,36
	sub edx,35

	cmp eax,9
	jle not_work_arrea

	cmp eax,ecx
	jae not_work_arrea

	cmp ebx,20+15+1+50
	jle not_work_arrea

	cmp ebx,edx
	jae not_work_arrea

	jmp mouse_in_work_arrea

	not_work_arrea:

	mov [exit_from_work_arrea],1

	jmp still

	mouse_in_work_arrea:

	call GetScreenCordinats
	call PrintMousePos
	call GetMouseClick



	test eax,eax
	jz no_use_instruments

	cmp [Activate_instrument],0

	jnz no_undo___
	;------------begin copy for undo-------------
	inc [number_undo]

	cmp [number_undo],1
	jne no_one__

	mov edi,[PointerToCopyPicture]

	no_one__:

	cmp [number_undo],2
	jne no_two__

	mov edi,[PointerToCopyPicture2]

	no_two__:

	cmp [number_undo],3
	jne no_three__

	;copy bufer number two to bufer number one
	mov esi,[PointerToCopyPicture2]
	mov edi,[PointerToCopyPicture]
	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	add ecx,4
	shr ecx,2
	inc ecx
	cld
	rep movsd
	;end copy
	dec [number_undo]
	mov edi,[PointerToCopyPicture2]

	no_three__:

	mov esi,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	add ecx,4
	shr ecx,2
	inc ecx
	cld
	rep movsd
	;--------------end copy for undo-------------
	no_undo___:

	call TakeButtonInstruments
	no_use_instruments:

	mov eax,[Current_instrument]
	and [Activate_instrument],0

	jmp still
;-----------------------------------------------
;---------get mouse cordinats-------------------
;-----------------------------------------------
GetMouseCoordinats:

	mov eax,37
	mov ebx,1
	mcall

	mov ebx,eax
	shr eax,16
	and ebx,0xffff

	mov [MouseX],eax
	mov [MouseY],ebx

	ret
;------------------------------------------------
;-------get mouse attributs----------------------
;------------------------------------------------
GetMouseClick:
	 mov eax,37
	 mov ebx,2
	 mcall

	 ret