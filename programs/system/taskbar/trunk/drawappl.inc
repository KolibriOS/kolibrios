;------------------------------------------------------------------------------
align 4
draw_running_applications:
	pusha

	cmp	[run_appl],dword 0
	je	.exit

	call	calculate_applications

	cmp	edi,[running_applications]
	jne	.noret
	popa
	ret
;--------------------------------------
align 4
.noret:
	call	draw_window
	mov	[running_applications],edi
	call	redraw_window_tabs
;--------------------------------------
align 4
.exit:
	popa
	ret
;------------------------------------------------------------------------------
align 4
need_window_tab:
; in: ebx->process info
; out: ZF set <=> do not draw
	cmp	byte [ebx+10], '@'
	jz	.nodraw
; do not draw undefined (zero-sized) windows
	cmp	dword [ebx+42], 0
	jnz	@f
	cmp	dword [ebx+46], 0
	jz	.nodraw
;--------------------------------------
align 4
@@:
; do not draw OpenDialog windows
	push	edi
	mov	edi, dword [ebx+10]
	or	edi, 0x20202020
	cmp	edi, 'open'
	jne	@f

	mov	edi, dword [ebx+14]
	or	edi, 0x20202020
	cmp	dword [ebx+14], 'dial'
	jne	@f

	pop	edi
	jmp	.nodraw
;--------------------------------------
align 4
@@:
	pop	edi
	cmp	dword [ebx+10], 'ICON'
	jnz	@f

	cmp	[ebx+42], dword 51
	jnz	@f

	cmp	[ebx+46], dword 51
	jz	.nodraw
;--------------------------------------
align 4
@@:
	cmp	[ebx+10], dword '    '
;--------------------------------------
align 4
.nodraw:
	ret
;------------------------------------------------------------------------------
align 4
calculate_applications:
	mov	eax,[max_applications]
	mul	[page_list]
	test	eax,eax
	je	@f

	inc	eax
;--------------------------------------
align 4
@@:
	mov	[draw_start_position],eax

	mov	edi,app_list
	mov	ecx,20 ; show max 20 application ???!!!
	mov	eax,-1
	cld
	rep	stosd

	mov	edi,0
	mov	ecx,2
;--------------------------------------
align 4
cnewpr:
	mcall	9,procinfo_window_tabs

	call	need_window_tab
	jz	cnorpl

	sub	[draw_start_position], 1
	jg	cnorpl

	mov	[app_list+edi*4],ecx
	inc	edi
;--------------------------------------
align 4
cnorpl:
	inc	ecx
	cmp	eax,ecx
	jge	cnewpr

	mov	[app_tab_count], edi
	ret
;------------------------------------------------------------------------------
align 4
redraw_window_tabs:
	xor	edi, edi
	mov	[contrast], 0

	push	ebp
	mcall	18,7
	mov	ebp,eax
	
	call	calculate_offset_X
;--------------------------------------
align 4
.loop:
	mov	ecx,[app_list+edi*4]
	cmp	ecx,-1
	jz	.done

	mov	[active_slot_number],ecx
	mcall	9,procinfo_window_tabs

	
	imul	ebx, edi, TAB_SIZE
	add	ebx,[offset_X]
	add	ebx,2
	shl	ebx,16
	add	ebx,TAB_SIZE-2	;54

	push	edi
	mov	edx,[system_colours.work_button]
	xor	edi,edi

	cmp	ebp,[active_slot_number]	;ecx	; ecx=active slot number
	jnz	@f

	mov	edx,[wcolor]
	inc	edi
;--------------------------------------
align 4
@@:
	mov	esi,[current_alt_tab_app]
	cmp	esi,-1
	jz	@f

	cmp	ecx,[alt_tab_list+esi*8]
	jnz	@f

	mov	edx,[AltTab_color]	;0xFF8000	; current select for ALT+Tab
;--------------------------------------
align 4
@@:
	call	calculate_button_y_coordinate_and_size

	mov	esi,[wcolor]
	call	draw_appl_button
	pop	edi
;------------------------------------------------------------------------------
	add	ebx,4 shl 16
	mov	bx,cx	;[height]
	shr	bx,1
	sub	bx,4
	shr	ecx,16
	add	bx,cx
	
	mov	edx, procinfo_window_tabs+10
	mcall	4,,[system_colours.work_button_text],,11
;--------------------------------------
align 4
.nodraw:
	inc	edi
	cmp	edi, [max_applications]
	jb	.loop

	mov	ecx,[app_list+edi*4]
	cmp	ecx,-1
	jz	.done

	xor	eax,eax
	cmp	[page_list],eax
	jne	@f
	
	inc	eax
	cmp	[page_list_enable],eax
	je	@f
	
	mov	[page_list_enable],eax
	mov	[redraw_window_flag],eax
;--------------------------------------
align 4
@@:
	pop	ebp
	ret
;--------------------------------------
align 4
.done:
	xor	eax,eax
	cmp	[page_list],eax
	jne	@f

	cmp	[page_list_enable],eax
	je	@f

	mov	[page_list_enable],eax
	inc	eax
	mov	[redraw_window_flag],eax
;--------------------------------------
align 4
@@:
	pop	ebp
	ret
;------------------------------------------------------------------------------
