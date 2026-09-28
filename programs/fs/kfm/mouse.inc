align 4
mouse:
    mcall SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
    cmp  [active_process],eax
    jne  still
    cmp  [library03.status_lib],0 ; test load library
    jnz  .m_end
    cld
    stdcall [kmainmenu_dispatch_cursorevent], [main_menu]

	mov  eax,[main_menu]
	cmp  dword[eax+16],-1
	je   @f
	mov  [m_lost],1
	jmp  still
@@:
	cmp  [m_lost],0
	je   .m_end
	dec  [m_lost]
	jmp  still
.m_end:

	mcall	SF_MOUSE_GET, SSF_SCROLL_DATA
	mov	[mouse_scroll_data],eax
	mcall	SF_MOUSE_GET, SSF_WINDOW_POSITION
	mov	[mouse_position],eax

	cmp	[sb_left.delta2],0
	jnz	.scrollbar_l
	cmp	[sb_right.delta2],0
	jnz	.scrollbar_r

	mov	eax,[mouse_position]
	shr	eax,16	; eax mouse x

	cmp	ax,[sb_right.x_pos]
	jge .scrollbar_r
	cmp	ax,[fb_right.start_x]
	jge @f
	cmp	ax,[sb_left.x_pos]
	jge .scrollbar_l

	mov	[fb_left.select_flag],0
	stdcall	[FileBrowser_mouse], fb_left
	cmp [fb_left.select_flag],0
	jz .check_scroll_event
	mov	eax,fb_left.mouse_keys_delta
	cmp	[eax],dword 3
	jne	.scrollbar_l
	mov	[eax],dword 0
	jmp  key_28.1
@@:
	mov	[fb_right.select_flag],0
	stdcall	[FileBrowser_mouse], fb_right
	cmp [fb_right.select_flag],0
	jz .check_scroll_event
	mov	eax,fb_right.mouse_keys_delta
	cmp	[eax],dword 3
	jne	.scrollbar_r
	mov	[eax],dword 0
	jmp  key_28.1

align 4
.scrollbar_l:
	stdcall	[scrollbar_v_mouse], sb_left
	mov	eax,[sb_left.position]
	mov ebx,fb_left
	call on_sb_change_fb_update

	cmp	[sb_left.delta2],0
	jz .check_scroll_event
	cmp [fb_left.select_panel_counter],1
	je .check_scroll_event
	mov [fb_left.select_panel_counter],1
	mov [fb_right.select_panel_counter],0
	call on_fb_focus_change
	jmp	.check_scroll_event

align 4
.scrollbar_r:
	stdcall	[scrollbar_v_mouse], sb_right
	mov	eax,[sb_right.position]
	mov ebx,fb_right
	call on_sb_change_fb_update

	cmp	[sb_right.delta2],0
	jz .check_scroll_event
	cmp [fb_right.select_panel_counter],1
	je .check_scroll_event
	mov [fb_left.select_panel_counter],0
	mov [fb_right.select_panel_counter],1
	call on_fb_focus_change
	;jmp	.check_scroll_event

;---------------------------------------------------
align 4
.check_scroll_event:
	mov	eax,[mouse_position]
	movzx	ebx,ax	; ebx mouse y
	shr	eax,16	; eax mouse x

	mov	cx,[fb_right.start_x]
	mov	dx,[fb_left.start_y]

	cmp	bx,dx
	jb	still	; min y

	add	dx,[fb_left.size_y]
	cmp	bx,dx
	ja	still	; max y

	xor esi,esi
	cmp	ax,cx
	jb	@f	; coord x
	inc esi
@@:
	cmp [fb_right.select_panel_counter],esi
	je .no_click
	mcall SF_MOUSE_GET, SSF_BUTTON_EXT
	bt eax,8
	jc @f
	bt eax,9
	jnc .no_click
@@:
	mov [fb_right.select_panel_counter],esi
	xor esi,1
	mov [fb_left.select_panel_counter],esi
	xor esi,1
	call on_fb_focus_change
.no_click:

	mov	eax,[mouse_scroll_data]
	test	eax,eax
	jz	still

	test	ax,0x8000
	jnz	.decr
	shr	eax,16
	test	ax,0x8000
	jnz	.decr_1

	movzx	ecx,[mouse_scroll_data.vertical]
	test	ecx,ecx
	jnz	@f
	mov	cx,[mouse_scroll_data.horizontal]
	test	ecx,ecx
	jz	still
@@:
	push	ebx ecx
	mov	ebx,1
	call fb_on_key_1
	pop	ecx ebx
	dec	ecx
	jnz	@b
	jmp	still
;----------------------------------------
.decr:
	movzx	ebx,[mouse_scroll_data.vertical]
	jmp	@f
.decr_1:
	movzx	ebx,[mouse_scroll_data.horizontal]
@@:
	mov	ecx,0xffff
	sub	ecx,ebx
	inc	ecx
@@:
	push	ebx ecx
	mov	ebx,2
	call fb_on_key_1
	pop	ecx ebx
	dec	ecx
	jnz	@b
	jmp	still
;---------------------------------------------------------------------
;in:
;  eax - scrool bar position
;  ebx - pointer to file browser
align 4
on_sb_change_fb_update:
	cmp	[ebx+fb_left.start_draw_line-fb_left],eax
	je .end
	push ecx edx
	mov ecx,eax
	sub ecx,[ebx+fb_left.start_draw_line-fb_left]
	movzx edx,word[ebx+fb_left.line_size_y-fb_left]
	imul ecx,edx
	mov dx,[ebx+fb_left.start_draw_cursor_line-fb_left]
	sub dx,cx
	cmp dx,0
	jge @f
	xor dx,dx
@@:
	mov [ebx+fb_left.start_draw_cursor_line-fb_left],dx
	mov	[ebx+fb_left.all_redraw-fb_left],1
	mov	[ebx+fb_left.start_draw_line-fb_left],eax
	stdcall	[FileBrowser_draw], ebx
	xor eax,eax
	mov	[ebx+fb_left.all_redraw-fb_left],eax
	pop edx ecx
.end:
	ret
;---------------------------------------------------------------------
align 4
on_fb_focus_change:
	pushad
	call draw_left_panel_patch
	call draw_right_panel_patch
	popad
	mov [fb_left.all_redraw], 1
	mov [fb_right.all_redraw], 1
	stdcall	[FileBrowser_draw], fb_left
	stdcall	[FileBrowser_draw], fb_right
	mov	[fb_left.all_redraw],0
	mov	[fb_right.all_redraw],0
	ret
;---------------------------------------------------------------------