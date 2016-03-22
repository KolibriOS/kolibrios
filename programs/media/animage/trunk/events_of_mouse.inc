;---------------------------------------------
;-------------interraction MOUSE--------------
;---------------------------------------------
mouse:
	stdcall [kmainmenu_dispatch_cursorevent], [main_menu]

	or ecx,-1
	mcall SF_THREAD_INFO,procinfo
	
	mov	eax,[procinfo+70] ;staus of window
	test eax,100b
	jne	still
	
	call GetMouseCoordinats
;panel
	mov	edx,[Window_SizeX]
	sub	edx,5

	;cmp ebx,0
	;jle no_panel

	cmp ebx,ci_panel_y_pos
	jae no_panel

	cmp	eax,5
	jle	no_panel

	cmp	eax,edx
	jae	no_panel

	call GetMouseCoordinats
	jmp	panel_interraction

no_panel:
	cmp	[Panel_flag],0
	jz	no_redraw_panel

	jmp	panel_interraction

no_redraw_panel:
	call GetMouseCoordinats
	mov	eax,[MouseX]
	mov	ebx,[MouseY]
	mov	ecx,[WorkScreen_SizeX]
	mov	edx,[WorkScreen_SizeY]
	add	ecx,ci_scrollh_coord_x_min
	add	edx,ci_scrollv_coord_y_min

	cmp	eax,ecx
	jae	mouse_scroll

	cmp	ebx,edx
	jae	mouse_scroll
	jmp	no_scrollers

mouse_scroll:
	mov	[exit_from_work_arrea],1 ;mouse situated after work arrea
;scrollers
	call	GetMouseClick
	cmp	eax,1
	jne	no_click

	call	GetMouseCoordinats
;interraction with horizontal scroller
	mov	eax,[Scroll1CoordinatX]
	mov	ebx,[Scroll1CoordinatY]
	mov	ecx,[MouseX]
	mov	edx,[MouseY]
	mov	esi,[Scroll1MaxSizeX]
	mov	edi,[Scroll1MaxSizeY]
	call	columnus
	test	eax,eax
	jz	no_horizontal

	mov	eax,ci_scrollh_coord_x_min
	mov	ebx,[Scroll1CoordinatY]
	mov	ecx,[Scroll1MaxSizeX]
	inc ecx
	mov	edx,ci_scroll_dim-6
	mov	esi,7
	call draw_volume_rectangle ; horizontal scrollbar clear

	mov	eax,[MouseX]
	mov	ebx,[Scroll1SizeX]
	shr	ebx,1
	sub	eax,ebx
	mov	ecx,[MouseX]
	add	ecx,ebx
	mov	edx,ci_scrollh_coord_x_min
	add	edx,[Scroll1MaxSizeX]
	mov	[Scroll1CoordinatX],eax
	sub	eax,ci_scrollh_coord_x_min
	jns	no_min_scroll

	mov	[Scroll1CoordinatX],ci_scrollh_coord_x_min

no_min_scroll:
	cmp	ecx,edx
	jl	no_max_scroll

	sub	edx,ebx
	sub	edx,ebx
	mov	[Scroll1CoordinatX],edx

no_max_scroll:
	mov	eax,[Scroll1CoordinatX]
	sub	eax,ci_scrollh_coord_x_min
	mov	ebx,[Picture_SizeX]
	imul	eax,ebx
	mov	ebx,[Scroll1MaxSizeX]
	cdq
	idiv	ebx
	mov	[PosX],eax
	jmp	no_vertical

no_horizontal:
	mov	eax,[Scroll2CoordinatX]
	mov	ebx,[Scroll2CoordinatY]
	mov	ecx,[MouseX]
	mov	edx,[MouseY]
	mov	esi,[Scroll2MaxSizeX]
	mov	edi,[Scroll2MaxSizeY]
	call	columnus
	test	eax,eax
	jz	no_vertical

	mov	eax,[Scroll2CoordinatX]
	mov	ebx,ci_scrollv_coord_y_min
	mov	ecx,ci_scroll_dim-6
	mov	edx,[Scroll2MaxSizeY]
	inc edx
	mov	esi,7
	call draw_volume_rectangle ; vertical scrollbar clear

	mov	eax,[MouseY]
	mov	ebx,[Scroll2SizeY]
	shr	ebx,1
	sub	eax,ebx
	mov	ecx,[MouseY]
	add	ecx,ebx
	mov	edx,ci_scrollv_coord_y_min
	add	edx,[Scroll2MaxSizeY]
	mov	[Scroll2CoordinatY],eax

	sub	eax,ci_scrollv_coord_y_min
	jns	no_min_scroll2

	mov	[Scroll2CoordinatY],ci_scrollv_coord_y_min

no_min_scroll2:
	cmp	ecx,edx
	jl	no_max_scroll2

	sub	edx,ebx
	sub	edx,ebx
	mov	[Scroll2CoordinatY],edx

no_max_scroll2:
	mov	eax,[Scroll2CoordinatY]
	sub	eax,ci_scrollv_coord_y_min
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	mov	ebx,[Scroll2MaxSizeY]
	cdq
	idiv	ebx
	mov	[PosY],eax

no_vertical:
	call	CalculatePositionScreen
	call	draw_scrollers
	call	MovePictureToWorkScreen

no_click:
	jmp	still

no_scrollers:
	mov	eax,[MouseX]
	mov	ebx,[MouseY]
	mov	ecx,[Window_SizeX]
	mov	edx,[Window_SizeY]
	sub	ecx,36
	sub	edx,35

	cmp	eax,ci_scrollh_coord_x_min
	jle	not_work_arrea

	cmp	eax,ecx
	jae	not_work_arrea

	cmp	ebx,ci_scrollv_coord_y_min
	jle	not_work_arrea

	cmp	ebx,edx
	jae	not_work_arrea
	jmp	mouse_in_work_arrea

not_work_arrea:
	mov	[exit_from_work_arrea],1
	jmp	still

mouse_in_work_arrea:
	call	GetScreenCordinats
	call	PrintMousePos
	call	GetMouseClick
	test	eax,eax
	jz	no_use_instruments

	cmp	[Activate_instrument],0
	jnz	no_undo___
;------------begin copy for undo-------------
;copy bufer number 2 to bufer number 0
	mov edi,[PointerToCopyPicture2]
	mov esi,[PointerToPicture]
	mov ecx,[Picture_SizeX]
	imul ecx,[Picture_SizeY]
	lea ecx,[ecx+ecx*2]
	mov ebx,ecx
	shr ecx,2
	cld
	rep movsd
	mov ecx,ebx
	and ecx,3
	rep movsb
	;call MovePictureToWorkScreen
;rotate bufers +1
	and	[number_undo],0 ;erase all undo
	mov eax,[PointerToPicture]
	mov ebx,[PointerToCopyPicture]
	mov ecx,[PointerToCopyPicture2]
	mov [PointerToPicture],ecx
	mov [PointerToCopyPicture],eax
	mov [PointerToCopyPicture2],ebx
;--------------end copy for undo-------------
no_undo___:
	call TakeButtonInstruments
	jmp still

no_use_instruments:
	mov	eax,[Current_instrument]
	and	[Activate_instrument],0
	jmp	still
;-----------------------------------------------
;---------get mouse cordinats-------------------
;-----------------------------------------------
GetMouseCoordinats:
	mcall SF_MOUSE_GET,SSF_WINDOW_POSITION
	mov	ebx,eax
	shr	eax,16
	and	ebx,0xffff
	mov	[MouseX],eax
	mov	[MouseY],ebx
	ret
;------------------------------------------------
;-------get mouse attributs----------------------
;------------------------------------------------
GetMouseClick:
	mcall SF_MOUSE_GET,SSF_BUTTON
	ret