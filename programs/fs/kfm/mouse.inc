align 4
mouse:
    mcall SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
    cmp  [active_process],eax
    jne  still
    cld
    stdcall [kmainmenu_dispatch_cursorevent], [main_menu]

	mcall	SF_MOUSE_GET, SSF_SCROLL_DATA
	mov	[mouse_scroll_data],eax
	mcall	SF_MOUSE_GET, SSF_WINDOW_POSITION
	mov	[mouse_position],eax

	cmp	[sb_left.delta2],0
	jnz	.scrollbar_l
	cmp	[sb_right.delta2],0
	jnz	.scrollbar_r

	mov	[fb_left.select_flag],0
	stdcall	[FileBrowser_mouse], fb_left
	cmp [fb_left.select_flag],0
	jz @f
	mov	eax,fb_left.mouse_keys_delta
	cmp	[eax],dword 3
	jne	.scrollbar_l
	mov	[eax],dword 0
	jmp  key_28.1
@@:
	mov	[fb_right.select_flag],0
	stdcall	[FileBrowser_mouse], fb_right
	cmp [fb_right.select_flag],0
	jz @f
	mov	eax,fb_right.mouse_keys_delta
	cmp	[eax],dword 3
	jne	.scrollbar_r
	mov	[eax],dword 0
	jmp  key_28.1
@@:

	stdcall	[scrollbar_v_mouse], sb_left
	cmp	[sb_left.delta2],0
	jz @f
	cmp [fb_left.select_panel_counter],1
	je @f
	mov [fb_left.select_panel_counter],1
	mov [fb_right.select_panel_counter],0
	call on_fb_focus_change
@@:
	cmp	[sb_left.redraw],0
	jnz	.check_scroll_event

	stdcall	[scrollbar_v_mouse], sb_right
	cmp	[sb_right.delta2],0
	jz @f
	cmp [fb_right.select_panel_counter],1
	je @f
	mov [fb_left.select_panel_counter],0
	mov [fb_right.select_panel_counter],1
	call on_fb_focus_change
@@:
	cmp	[sb_right.redraw],0
	jnz	.check_scroll_event
;---------------------------------------------------
align 4
.scrollbar_l:
	mov	eax,[sb_left.max_area]
	cmp	eax,[sb_left.cur_area]
	jbe	.check_scroll_event

	stdcall	[scrollbar_v_mouse], sb_left
	cmp	[sb_left.redraw],0
	je	.check_scroll_event
	mov	[sb_left.redraw],0

	mov	[fb_left.all_redraw],2
	mov	eax,[sb_left.position]
	mov ebx,fb_left
	call on_sb_change_fb_update
	mov	[fb_left.start_draw_line],eax
	
	stdcall	[FileBrowser_draw], fb_left
	xor	eax,eax
	mov	[fb_left.all_redraw],eax
	jmp	still
;---------------------------------------------------
align 4
.scrollbar_r:
	mov	eax,[sb_right.max_area]
	cmp	eax,[sb_right.cur_area]
	jbe	.check_scroll_event

	stdcall	[scrollbar_v_mouse], sb_right
	cmp	[sb_right.redraw],0
	je	.check_scroll_event
	mov	[sb_right.redraw],0

	mov	[fb_right.all_redraw],2
	mov	eax,[sb_right.position]
	mov ebx,fb_right
	call on_sb_change_fb_update
	mov	[fb_right.start_draw_line],eax
	
	stdcall	[FileBrowser_draw], fb_right
	xor	eax,eax
	mov	[fb_right.all_redraw],eax
	jmp	still
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