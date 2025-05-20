;---------------------------------------------------------
;------print mouse position on panel----------------------
;---------------------------------------------------------
PrintMousePos:
	pushad
	mov	eax,[Window_SizeX]
	sub	eax,75+ci_offs_skin_w
	mov	ebx,ci_panel_y_pos+6
	mov	ecx,52
	mov	edx,18
	mov	esi,4
	call draw_volume_rectangle ; border for mouse x pos

	mov	eax,[Window_SizeX]
	sub	eax,75+ci_offs_skin_w
	mov	ebx,ci_panel_y_pos+6+18+4
	mov	ecx,52
	mov	edx,18
	mov	esi,4
	call draw_volume_rectangle ; border for mouse y pos

	mov	eax,[Window_SizeX]
	sub	eax,70+ci_offs_skin_w
	mov	ebx,ci_panel_y_pos+6+6
	and	ecx,0
	mov	edx,mouse_pos_x
	mov	esi,2
	call print_text

	mov	eax,[Window_SizeX]
	sub	eax,70+ci_offs_skin_w
	mov	ebx,ci_panel_y_pos+6+6+18+4
	and	ecx,0
	mov	edx,mouse_pos_y
	mov	esi,2
	call print_text

	mov	ecx,[ScreenX]
	add	ecx,[PosX]
	mov	edx,[Window_SizeX]
	sub	edx,55+ci_offs_skin_w
	shl	edx,16
	add	edx,ci_panel_y_pos+6+6
	and	esi,0
	mcall SF_DRAW_NUMBER,<4,0>

	mov	ecx,[ScreenY]
	add	ecx,[PosY]
	mov	edx,[Window_SizeX]
	sub	edx,55+ci_offs_skin_w
	shl	edx,16
	add	edx,ci_panel_y_pos+6+18+4+6
	mcall

	popad
	ret

;----------------------------------------------------------
;------------draw lines of scoll---------------------------
;----------------------------------------------------------
draw_scrollers:
	mov	edi,[CounterX]
	mov	eax,[Scroll1CoordinatX]
	mov	ebx,[Window_SizeY]
	sub	ebx,(ci_scroll_dim-3)+5+ci_offs_skin_h
	mov	ecx,[Window_SizeX]
	sub	ecx,ci_scroll_dim+5+4+ci_scrollh_coord_x_min+ci_offs_skin_w
	mov	edx,ci_scroll_dim-6
	mov	esi,1
	mov	[Scroll1CoordinatY],ebx
	mov	[Scroll1MaxSizeY],edx
	mov	[Scroll1MaxSizeX],ecx
	imul	ecx,edi
	push	eax	ebx	edx
	mov	eax,ecx
	mov	ebx,[Picture_SizeX]
	cdq
	idiv	ebx
	mov	ecx,eax
	pop	edx	ebx	eax
	mov	[Scroll1SizeX],ecx
	mov	edi,[Scroll1MaxSizeX]
	sub	edi,ecx
	mov	[Scroll1FreeX],edi
	call draw_volume_rectangle ; horizontal scrollbar polzunok

	mov	eax,[Window_SizeX]
	sub	eax,(ci_scroll_dim-3)+5+ci_offs_skin_w
	mov	ebx,[Scroll2CoordinatY]
	mov	ecx,ci_scroll_dim-6
	mov	edx,[Window_SizeY]
	sub	edx,ci_scroll_dim+5+4+ci_scrollv_coord_y_min+ci_offs_skin_h
	mov	esi,1
	mov	[Scroll2CoordinatX],eax
	mov	[Scroll2MaxSizeX],ecx
	mov	[Scroll2MaxSizeY],edx

	imul	edx,[CounterY]
	push	eax	ebx
	mov	eax,edx
	mov	ebx,[Picture_SizeY]
	cdq
	idiv	ebx
	mov	edx,eax
	pop	ebx	eax
	mov	[Scroll2SizeY],edx
	mov	edi,[Scroll2MaxSizeY]
	sub	edi,edx
	mov	[Scroll2FreeY],edi
	call draw_volume_rectangle ; vertical scrollbar polzunok

	ret

button_menu_size_x	dd 0
button_menu_size_y	dd 0