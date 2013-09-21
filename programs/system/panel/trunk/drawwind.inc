;------------------------------------------------------------------------------
; ***************************************************
; ********* WINDOW DEFINITIONS AND DRAW *************
; ***************************************************
align 4
draw_window:
	pusha
	mov	[running_applications],-1
	mcall	12,1

	mcall	48,3,system_colours,10*4

	mov	eax, [system_colours+4*6]
;	sub	eax, 0x101010
;	mov	[wcolor], eax

	mov	edx,[system_colours+4*6]
	mov	eax,COLOR_CHANGE_MAGNITUDE
	call	subtract_color_change_magnitude
	mov	[wcolor], edx

	mcall	14	; get screen max x & max y

	cmp	[width],dword 0
	je	no_def_width

	and	eax,0xffff
	mov	ebx,[width]
	shl	ebx,16
	add	eax,ebx
;--------------------------------------
align 4
no_def_width:
	mov	ebx,eax
	mov	[screenxy],ebx
	shr	ebx,16
	mov	ecx,eax

	cmp	[place_attachment],1
	je	@f
	
	xor	ecx,ecx
	mov	cx,[height]
	dec	cx
	jmp	.attachment_selected
;--------------------------------------
align 4
@@:
	sub	ecx,[height]
	inc	ecx
	shl	ecx,16
	mov	cx,[height]
	dec	cx
;--------------------------------------
align 4
.attachment_selected:
	xor	eax,eax 		    ; DEFINE AND DRAW WINDOW
	mov 	edx, [wcolor]
	or	edx, 0x01000000 ; do not draw the window
	mov	esi, [wcolor]
	or	esi, 0x01000000 ; unmovable window
	mov 	edi, [wcolor]
	mov	[panel_x_pos], ebx
	mov	[panel_y_pos], ecx	 ; Пока что так.
	mcall

	movzx	eax,word [screenxy+2]
	mov	[max_x],eax
	
	call	fill_window
	call	minimize_left_button
	call	minimize_right_button
	call	draw_menu_and_clean_desktop
	mov	[ptime],0
	call	draw_tray
	call	draw_application_buttons

	mov	[redraw_window_flag],0
	mcall	12,2
	popa
	ret
;------------------------------------------------------------------------------
align 4
fill_window:	
	movzx	ebx,word [screenxy+2]
	xor	ecx,ecx
	mov	edx,[wcolor]
;	add	edx,0x161616
	mov	eax,COLOR_CHANGE_MAGNITUDE_0
	imul	eax,dword [soften_height]
	call	subtract_color_change_magnitude
	cmp	[soften_up],dword 0
	je	no_su
;--------------------------------------
align 4
@@:			; debug.inc has macros with the same name
;	sub	edx,0x040404
	mov	eax,COLOR_CHANGE_MAGNITUDE_0
	call	add_color_change_magnitude
; draw soften_up - the width of 5 pixels
	and	edx,0x00FFFFFF
	mcall	38

	add	ecx,1*65536+1
	cmp	cx,[soften_height]	;5
	jb	@r
;--------------------------------------
align 4
no_su:
	cmp	[soften_down],dword 0
	je	no_sd
; draw soften_down - the width of 5 pixels
	pusha
	mov	esi,[soften_height]
	mov	ecx,[height]
	dec	ecx
	shl	ecx,16
	add	ecx,[height]
	dec	ecx
	mov	edx,[wcolor]
	mov	eax,COLOR_CHANGE_MAGNITUDE_0
	imul	eax,dword [soften_height]
	call	subtract_color_change_magnitude
;--------------------------------------
align 4
@@:
	mov	eax,COLOR_CHANGE_MAGNITUDE_0
	call	add_color_change_magnitude
	and	edx,0x00FFFFFF
	mcall	38
	sub	ecx,1*65536+1
	dec	esi
	jnz	@r
	popa
;--------------------------------------
align 4
no_sd:
	movzx	ebx,word [screenxy+2]
	xor	ecx,ecx
	cmp	[soften_up],dword 0
	je	@f

	add	ecx,[soften_height]
	rol	ecx,16
	add	ecx,[soften_height]
;--------------------------------------
align 4
@@:
;	mov	esi,stripe
	mov	edx,[wcolor]
;--------------------------------------
align 4
newline3:
	and	edx,0x00FFFFFF
	mov	eax,[height]
	cmp	[soften_up],dword 0
	je	@f

	sub	eax,[soften_height]
;--------------------------------------
align 4
@@:
	cmp	[soften_down],dword 0
	je	@f

	sub	eax,[soften_height]
;--------------------------------------
align 4
@@:
	mov	cx,ax
	inc	ebx
	mcall	13
	ret
;------------------------------------------------------------------------------
align 4
minimize_left_button:
	cmp	[minimize_left],dword 0
	je	.exit

	mov	ecx,1 *65536
	add	ecx,[height]
	dec	ecx
	mov	edx,101
;	add	edx,[button_frames]
	or	edx,0x40000000
	mcall	8,<0,9>,,,[wcolor]	; ABS LEFT

	mov	ebx,2*65536	;+6
	mov	bx,[height]
	shr	bx,1
	sub	bx,3
;	mov	ecx,[wcolor]
;	add	ecx,0x303030
	mov	edx,[wcolor]
	mov	eax,COLOR_CHANGE_MAGNITUDE_2
	call	add_color_change_magnitude
	mov	ecx,edx
	mcall	4,,,hidetext,1	; HIDE TEXT
;--------------------------------------
align 4
.exit:
	ret
;------------------------------------------------------------------------------
align 4	
minimize_right_button:
	cmp	[minimize_right],dword 0
	je	.exit
	mov	eax,[max_x]
	sub	eax,77
	shl	eax,16
	mov	ebx,eax
	add	ebx,67

	mov	ecx,1 *65536
	add	ecx,[height]
	dec	ecx
	add	ebx,68*65536
	mov	bx,10 ;9
	mov	edx,102
;	add	edx,[button_frames]
	or	edx,0x40000000
	mcall	8,,,,[wcolor]	; ABS RIGHT

	mov	ebx,[max_x]
	sub	ebx,6
	shl	ebx,16
	mov	bx,[height]
	shr	bx,1
	sub	bx,3
;	mov	ecx,[wcolor]
;	add	ecx,0x303030
	mov	edx,[wcolor]
	mov	eax,COLOR_CHANGE_MAGNITUDE_2
	call	add_color_change_magnitude
	mov	ecx,edx
	mov	esi,1
	mcall	4,,,hidetext+1
;--------------------------------------
align 4
.exit:
	ret
;------------------------------------------------------------------------------
align 4
draw_menu_and_clean_desktop:
	pusha
; check draw for menu
	cmp	[menu_enable],dword  0
	je	no_menu
; calculate and draw menu
	mov	ebx, (0 shl 16) + MENU_SIZE-3
; check for left minimize button enabled
	cmp	[minimize_left],dword 0
	je	@f

	add	ebx, ML_SIZE shl 16
;--------------------------------------
align 4
@@:
	call	calculate_button_y_coordinate_and_size
	mov	edx, 0x40d1ff01
;	mov	edx, 0xd1ff01
;	add	edx, [button_frames]
	mcall	8,,,,[wcolor]	; MENU BUTTON
	
	mov	eax,ebx
	shr	eax,16
	mov	[menu_button_x.start],eax
	
	mov	eax,ebx
	and	eax,0xffff
	mov	[menu_button_x.size],eax
	
	mov	eax,ecx
	shr	eax,16
	mov	[menu_button_y.start],eax
	
	mov	eax,ecx
	and	eax,0xffff
	mov	[menu_button_y.size],eax

	mov	edx,[MenuButton_color]	;0x44aa44
	mov	esi,[wcolor]
	call	draw_appl_button	

	add	ebx, 8*65536
	mov	bx,[height]
	shr	bx,1
	sub	bx,3
	mov	ecx,[PanelText_color]
	or	ecx,0x10000000
	if lang eq et
	mcall	4,,,m_text,5
	else
	mcall	4,,,m_text,4
	end if
;--------------------------------------
align 4
no_menu:
; check draw for clean desktop button
	cmp	[clean_desktop_enable],dword 0
	je	.exit
; calculate and draw clean desktop button
	mov	ebx, (0 shl 16) + CLD_SIZE-5
; check for left minimize button enabled	
	cmp	[minimize_left],dword 0
	je	@f

	add	ebx, ML_SIZE shl 16
;--------------------------------------
align 4
@@:
; check for menu button enabled
	cmp	[menu_enable],dword 0
	je	@f
	
	add	ebx, MENU_SIZE shl 16
;--------------------------------------
align 4
@@:
; Inserted code for drawing buttons 103, 104, 105 (Clean, Restore, Exchange windows)
	mov	esi, dword [system_colours+24]    ; drawing buttons
	and	esi, 0x00ffffff
	mov	edx, 0x40000000 + 103
	call	calculate_button_y_coordinate_and_size
	mcall	8	;,,<3,13>
	
	mov	edx,[CleanDesktopButton_color]	;time_bgr_color
	mov	esi,[wcolor]
	call	draw_appl_button
	
;	mov	ecx, dword [system_colours+28]
;	and	ecx, 0x00ffffff
	mov	ecx,[PanelText_color]
	add	ebx,5 shl 16
;	mov	bx,6
	mov	bx,[height]
	shr	bx,1
	sub	bx,2
	mcall	4,,,page_clean_but,1
	sub	bx,2
	mcall	,,,page_clean_but+1
;--------------------------------------
align 4
.exit:
	popa
	ret
;------------------------------------------------------------------------------
align 4
draw_application_buttons:
	pusha

	cmp	[run_appl],dword 0	; do not draw application buttons
	je	.exit

	call	calculate_offset_X

;	mcall	14
	mov	eax,[screenxy]
	shr	eax,16

	sub	eax,[offset_X]
; check for left minimize button enabled
	cmp	[minimize_right],dword 0
	je	@f

	sub	eax, MR_SIZE
;--------------------------------------
align 4
@@:
	cmp	[clock_enable],dword 0
	je	@f
	
	sub	eax,CLOCK_SIZE
;--------------------------------------
align 4	
@@:
	cmp	[cpu_usage_enable],dword 0
	je	@f
	
	sub	eax,CPU_USAGE_SIZE
;--------------------------------------
align 4	
@@:
	cmp	[chlang_enable],dword 0
	je	@f
	
	sub	eax,CHLANG_SIZE
;--------------------------------------
align 4	
@@:
	cmp	[page_list_enable],dword 0
	je	@f
	
	sub	eax,PAGE_LIST_SIZE
;--------------------------------------
align 4	
@@:
	mov	ebx, TAB_SIZE
	xor	edx,edx
	div	ebx
	mov	[max_applications], eax
	xor	edi,edi
;--------------------------------------
align 4
.nb:
	mov	ebx,edi
	imul	ebx,TAB_SIZE
	add	ebx,[offset_X]
	shl	ebx,16
	mov	bx, TAB_SIZE-1
	mov	edx,edi
	add	edx,52
	or	edx,0x60000000
	mov	ecx, 1*65536
	add	ecx, [height]
	sub	ecx,3
	mcall	8,,,,[wcolor]

	inc	edi
	cmp	edi,[max_applications]
	jb	.nb
;--------------------------------------
align 4
.exit:
	popa
	ret
;------------------------------------------------------------------------------
align 4
calculate_offset_X:
	push	eax
	xor	eax,eax
; check for left minimize button enabled
	cmp	[minimize_left],dword 0
	je	@f

	add	eax, ML_SIZE
;--------------------------------------
align 4
@@:	
; check for menu button enabled
	cmp	[menu_enable],dword 0
	je	@f
	
	add	eax, MENU_SIZE
;--------------------------------------
align 4
@@:
; check for clean desktop button enabled
	cmp	[clean_desktop_enable],dword 0
	je	@f

	add	eax, CLD_SIZE
;--------------------------------------
align 4
@@:
	mov	[offset_X],eax
	pop	eax
	ret
;------------------------------------------------------------------------------
