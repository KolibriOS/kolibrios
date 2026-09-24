;---------------------------------------------
;-------------interraction MOUSE--------------
;---------------------------------------------
align 4
mouse:
	stdcall [kmainmenu_dispatch_cursorevent], [main_menu]

	or ecx,-1
	mcall SF_THREAD_INFO,procinfo
	
	movzx eax,byte [procinfo.wnd_state]
	test eax,100b
	jne	still
	
	call GetMouseClick
	bt [MouseBut],16
	jnc @f
	mov	[scroll_drag],0
@@:

	call GetMouseCoordinats
	mov	eax,[MouseX]
	mov	ebx,[MouseY]
	mov	ecx,[WorkScreen_SizeX]
	mov	edx,[WorkScreen_SizeY]
	add	ecx,ci_scrollh_coord_x_min
	add	edx,ci_scrollv_coord_y_min

	cmp [scroll_drag],1
	je sb_hor_mouse
	cmp [scroll_drag],2
	je sb_ver_mouse

	cmp	eax,ecx
	jae	mouse_scroll

	cmp	ebx,edx
	jae	mouse_scroll
	jmp	no_scrollers

mouse_scroll:
	mov	[exit_from_work_arrea],1 ;mouse situated after work arrea
;scrollers
	cmp [MouseBut],0
	je still

	;call GetMouseCoordinats
;interraction with horizontal scroller
	movzx	eax,[sb_horizont.x_pos]
	movzx	ebx,[sb_horizont.y_pos]
	mov	ecx,[MouseX]
	mov	edx,[MouseY]
	movzx	esi,[sb_horizont.x_size]
	movzx	edi,[sb_horizont.y_size]
	call	columnus
	test	eax,eax
	jz	no_horizontal

	bt [MouseBut],8
	jnc sb_hor_mouse
	mov	[scroll_drag],1
sb_hor_mouse:
	stdcall [scrollbar_h_mouse],sb_horizont
	mov	eax,[sb_horizont.position]
	mov	ebx,[Picture_SizeX]
	imul	eax,ebx
	mov	ebx,[sb_horizont.max_area]
	cdq
	idiv	ebx
	mov	[PosX],eax
	jmp	no_vertical

no_horizontal:
	movzx	eax,[sb_vertical.x_pos]
	movzx	ebx,[sb_vertical.y_pos]
	mov	ecx,[MouseX]
	mov	edx,[MouseY]
	movzx	esi,[sb_vertical.x_size]
	movzx	edi,[sb_vertical.y_size]
	call	columnus
	test	eax,eax
	jz	no_vertical

	bt [MouseBut],8
	jnc sb_ver_mouse
	mov	[scroll_drag],2
sb_ver_mouse:
	stdcall [scrollbar_v_mouse],sb_vertical
	mov	eax,[sb_vertical.position]
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	mov	ebx,[sb_vertical.max_area]
	cdq
	idiv	ebx
	mov	[PosY],eax

no_vertical:
	call	CalculatePositionScreen
	call	draw_scrollers
	call	MovePictureToWorkScreen
	jmp	still

no_scrollers:
	mov	eax,[MouseX]
	mov	ebx,[MouseY]
	mov	ecx,[Window_SizeX]
	mov	edx,[Window_SizeY]
	sub	ecx,ci_scroll_dim+ci_edit_wnd_border+5+ci_offs_skin_w
	sub	edx,ci_scroll_dim+ci_edit_wnd_border+5+ci_offs_skin_h

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
	mcall SF_MOUSE_GET, SSF_SET_CURSOR, 0 ;set standart cursor
	jmp	still

mouse_in_work_arrea:
	call	GetScreenCordinats
	call	PrintMousePos

	;set cursor for current instrument
	mov eax,CursorsID
	mov ebx,[Current_instrument]
	call set_cursor

	cmp [MouseBut],0
	je no_use_instruments

	cmp	[Activate_instrument],0
	jne	no_undo___
;------------begin copy for undo-------------
;copy bufer_0 -> bufer_2
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
	mcall SF_MOUSE_GET,SSF_BUTTON_EXT
	mov [MouseBut],eax
	ret