DLG_YN_W equ 160 ;must be a multiple of 2
;------------------------------------------------------------------------------
;description:
;  run dialog [yes] [no]
align 4
confirmation_action:
	mov	[work_confirmation],0
	mov	[work_confirmation_yes],0
	cmp	[confirmation_type],sz_Exit
	je	.red
	call get_files_information
	;call get_information_marked_files
	;or eax,eax
	;jnz .error
	jmp	.red
;.error:
	;ret
;--------------------------------------
.red_1:
	call	control_minimal_window_size
	call	draw_window
.red:
	call	draw_confirmation_menu
;--------------------------------------
.still:
	mcall	SF_WAIT_EVENT
	cmp	eax,EV_REDRAW
	je	.red_1

	cmp	eax,EV_KEY
	je	key_menu_confirmation

	cmp	eax,EV_BUTTON
	jne	.still

	mcall	SF_GET_BUTTON
	cmp	ah,161
	je	.no
	cmp	ah,160
	jne	@f
.yes:
	mov	[work_confirmation_yes],1
.no:
	call	draw_window ;to clear the dialog box
	ret
@@:
	cmp	ah,1
	jne	.still
	ret
;------------------------------------------------------------------------------
align 4
draw_confirmation_menu:
    stdcall draw_dialogbox_rectangle,DLG_YN_W,65
	mov	[ps_dialog.area_size_x],DLG_YN_W-10
if USE_STATIC_LIBS eq 0
	stdcall	[PathShow_prepare],ps_dialog
else
	stdcall	path_show.prepare,ps_dialog
end if
	pushad
	add	ebx,(DLG_YN_W-90) shl 15
	add	ecx,(65-25) shl 16
	mov	cx,15
	mov	bx,40
	mcall	SF_DEFINE_BUTTON, ,,0x40000000+160,[sc.work_button]
	add	ebx,50 shl 16
	inc	edx
	mcall
	popad
	push	ebx ecx
	add	ebx,(DLG_YN_W-90) shl 15
	add	ecx,(65-25) shl 16
	mov	bx,40
	mov	cx,15
	mov	edx,[sc.work_graph]
	push	ebx ecx
	cmp	[work_confirmation],0
	jne	@f
	add	ebx,50 shl 16 ;x_pos+=50
@@:
	mcall ;inactive button border
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,38 ;x_size
	mov	cx,13
	mcall	,,,[sc.work] ;fill in inactive buttons
	pop	ecx ebx
	mov	edx,[sc.work_button] ;color [yes] in focus
	cmp	[work_confirmation],0
	je	@f
	add	ebx,50 shl 16 ;x_pos+=50
	mov	edx,[sc.work_button] ;color [no] in focus
@@:
	mcall ;rect for button
	pop	ecx	ebx
	shr	ecx,16
	mov	bx,cx
	push ebx
	add	ebx,7 shl 16+2
	mov ecx,[sc.work_text]
	or ecx,0x90000000
	mcall	SF_DRAW_TEXT,,,[confirmation_type]
	cmp	[confirmation_type],sz_Exit
	je	@f ;do not display the file name if the exit dialog box is used
	add	ebx,22
	mov dword[ps_dialog.start_y],ebx
if USE_STATIC_LIBS eq 0
	stdcall	[PathShow_draw],ps_dialog
else
	stdcall	path_show.draw,ps_dialog
end if
@@:
	pop ebx
	add	ebx,(20+DLG_YN_W-90) shl 15+40
	mov ecx,[sc.work_button_text]
	or ecx,0x90000000
	mcall	,,,sz_Yes
	add	ebx,54 shl 16
	mcall	,,,sz_No
	ret
;------------------------------------------------------------------------------
align 4
key_menu_confirmation:
	mcall	SF_GET_KEY
	cmp	[extended_key],1
	je	.extended_key

	test	al,al
	jnz	.end_1

	cmp	ah,0xE0
	jne	@f
;--------------------------------------
.end_1:
	jmp	confirmation_action.still
;--------------------------------------
@@:
	cmp	ah,75
	je	confirmation_key_75.1

	cmp	ah,77
	je	confirmation_key_75.1

	cmp	ah,28
	je	confirmation_key_28.1
;--------------------------------------
.end:
	cmp	ah,1
	jne	confirmation_action.still

	ret
;------------------------------------------------------------------------------
.extended_key:
	mov	[extended_key],0
;------------------------------------------------------------------------------
confirmation_key_75:
	cmp	ah,75	; arrow left
	je	.1

	cmp	ah,77	; arrow right
	jne	confirmation_key_28
;--------------------------------------
.1:
	dec	[work_confirmation]
	and	[work_confirmation],1
	jmp	confirmation_action.red
;------------------------------------------------------------------------------
confirmation_key_28:
	cmp	ah,28	; Enter
	jne	confirmation_action.still
;--------------------------------------
.1:
	cmp	[work_confirmation],0
	jne	@f

	mov	[work_confirmation_yes],1
;--------------------------------------
@@:
	ret
;------------------------------------------------------------------------------