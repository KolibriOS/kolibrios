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
align 4
draw_scrollers:
	mov	edi,[CounterX]
	mov	ebx,[Window_SizeY]
	sub	ebx,ci_scroll_dim+5+ci_offs_skin_h
	mov	ecx,[Window_SizeX]
	sub	ecx,ci_scroll_dim+2+ci_scrollh_coord_x_min+ci_offs_skin_w
	mov	[sb_horizont.y_pos],bx
	mov	[sb_horizont.max_area],ecx
	mov	[sb_horizont.x_size],cx
	imul	ecx,edi
	push	eax	ebx	edx
	mov	eax,ecx
	mov	ebx,[Picture_SizeX]
	cdq
	idiv	ebx
	mov	ecx,eax
	pop	edx	ebx	eax
	mov	[sb_horizont.cur_area],ecx

	mov	eax,[Window_SizeX]
	sub	eax,ci_scroll_dim+5+ci_offs_skin_w
	mov	edx,[Window_SizeY]
	sub	edx,ci_scroll_dim+2+ci_scrollv_coord_y_min+ci_offs_skin_h
	mov [sb_vertical.x_pos],ax
	mov [sb_vertical.max_area],edx
	mov [sb_vertical.y_size],dx

	imul	edx,[CounterY]
	push	eax	ebx
	mov	eax,edx
	mov	ebx,[Picture_SizeY]
	cdq
	idiv	ebx
	mov	edx,eax
	pop	ebx	eax
	mov	[sb_vertical.cur_area],edx

	mov [sb_vertical.all_redraw],1
	stdcall [scrollbar_v_draw],sb_vertical
	mov [sb_horizont.all_redraw],1
	stdcall [scrollbar_h_draw],sb_horizont
	ret

button_menu_size_x	dd 0
button_menu_size_y	dd 0