;------------------------------------------------------------------------------
confirmation_action:
	mov	[work_confirmation],0
	mov	[work_confirmation_yes],0
	jmp	.red
;--------------------------------------
.red_1:
	call	draw_window
;--------------------------------------
.red:
	call	draw_confirmation_button
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
	je	.no_del

	cmp	ah,160
	je	.del

	cmp	ah,1
	jne	.still

	mcall	SF_TERMINATE_PROCESS
;--------------------------------------
.del:
	mov	[work_confirmation_yes],1
;--------------------------------------
.no_del:
	call	draw_window
;--------------------------------------
.ret:
	ret
;------------------------------------------------------------------------------
draw_confirmation_button:
	mov	ecx,[window_high]
	mov	ebx,[window_width]
	shr	ecx,1
	sub ecx,[skin_high]
	shr	ebx,1
	sub	ebx,50
	shl	ecx,16
	shl	ebx,16
	mov	cx,15
	mov	bx,40
	mcall	SF_DEFINE_BUTTON, ,,0x40000000+160,0xffffff
	add	ebx,50 shl 16
	inc	edx
	mcall
	ret
;------------------------------------------------------------------------------
draw_confirmation_menu:
	mov	ecx,[window_high]
	mov	ebx,[window_width]
	shr	ecx,1
	shr	ebx,1
	sub	ecx,22
	sub ecx,[skin_high]
	sub	ebx,65
	shl	ecx,16
	shl	ebx,16
	mov	cx,44
	mov	bx,120
	mcall	SF_DRAW_RECT, ,,0x6060ff
	push	ebx ecx
	add	ebx,15 shl 16
	add	ecx,22 shl 16
	mov	bx,40
	mov	cx,15
	mov	edx,0
	push	ebx ecx
	cmp	[work_confirmation],0
	jne	@f

	add	ebx,50 shl 16
;--------------------------------------
@@:
	mcall
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,38
	mov	cx,13
	mcall	,,,0x6060ff
	pop	ecx ebx
	mov	edx,0xff0000
	cmp	[work_confirmation],0
	je	@f

	add	ebx,50 shl 16
	mov	edx,0xaa00
;--------------------------------------
@@:
	mcall
	pop	ecx	ebx
	shr	ecx,16
	mov	bx,cx
	add	ebx,7 shl 16+2
	mcall	SF_DRAW_TEXT, ,0x90ffffff,[confirmation_type]
	add	ebx,15 shl 16+20
	mcall	,,,type_yes
	add	ebx,56 shl 16
	mcall	,,,type_no
	ret
;------------------------------------------------------------------------------
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