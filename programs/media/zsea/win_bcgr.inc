;---------------------------------------------------------------------
k_background:
	test	dword [status],8
	jnz	still
	or	dword [status],8

	call	get_memory_for_tread_stack
	mov	[thread_stack_4],eax
	mcall	51,1,thread4	;, thread_stack_4
	jmp	still
;---------------------------------------------------------------------
thread4:	; start of bgrd thread
	mcall	9, procinfo_threads, -1
	mov	eax,[ebx+30]
	mov	[PID4],eax
	mcall	40, 0x27
.red:
	call	.draw_window

.still:

	mov	eax,10	; wait here for event
	mcall

	cmp	eax,1	; redraw request ?
	je	.red
	cmp	eax,2	; key in buffer ?
	je	.key
	cmp	eax,3	; button in buffer ?
	je	.button
	cmp	eax,6	; mouse in buffer ?
	je	.mouse
	jmp	.still

.mouse:
	push	dword option_boxes_bcgr
	call	[option_box_mouse]
	
	cmp	[option_group1],op1
	jne	@f
	mov	[bgrmode],dword 1
	jmp	.still
@@:
	mov	[bgrmode],dword 2
	jmp	.still

.key:
	mcall
	cmp	ah,27
	je	.close
	cmp	ah,13
	je	.kok
	cmp	ah,178 ;up
	jne	.nofup
	cmp	dword [bgrmode],1
	je	.fdn
.fup:
	dec	dword [bgrmode]
	jmp	.flagcont
.nofup:
	cmp	ah,177 ;down
	jne	.still
	cmp	dword [bgrmode],2
	je	.fup
.fdn:
	inc	dword [bgrmode]
.flagcont:
	cmp	[bgrmode],dword 1
	jne	@f
	mov	[option_group1],op1
	call	.draw_opt_boxes
	jmp	.still
@@:
	mov	[option_group1],op2
	call	.draw_opt_boxes
	jmp	.still


.button:		; button
	mov	eax,17	; get id
	mcall

	cmp	ah,1	; button id=1 ?
	jne	.noclose
.close:
	xor	dword [status],8
	xor	eax,eax
	mov	[PID4],eax

	mcall	68,13,[thread_stack_4]
	mov	eax,-1	; close this program
	mcall
.noclose:
.kok:
	call	background
	jmp	.close

;	*********************************************
;	*******	WINDOW DEFINITIONS AND DRAW ********
;	*********************************************


.draw_window:
	mcall	12,1
					; DRAW WINDOW
	xor	eax,eax			; function 0 : define and draw window
	mov	ebx,100*65536+200	 ; [x start] *65536 + [x size]
	mov	ecx,100*65536+100	 ; [y start] *65536 + [y size]
	mov	edx,0x03909090		; color of work area RRGGBB,8->color gl
	mcall

	mov	eax,8
	mov	ebx,70*65536+40
	mov	ecx,70*65536+20
	mov	edx,4
	mov	esi,0xdddddd
	mcall

					; WINDOW LABEL
	mcall	71,1, labelt4

	mov	eax,4			; function 4 : write text to window
	mov	ebx,46*65536+28		; [x start] *65536 + [y start]
	mov	ecx,0xffffff
	mov	edx, bgrdtext
	mov	esi, bgrdtext.size
	mcall
	add	ebx,40*65536+48
	mov	edx, ok_btn
	mov	esi, ok_btn.size ;2
	mov	ecx,0
	mcall

	call	.draw_opt_boxes
;	mcall 47,0x80000,[PID4],<200, 5>,0xffffff
	mcall	12,2

	ret
;---------------------------------------------------------------------
.draw_opt_boxes:
	push	dword option_boxes_bcgr
	call	[option_box_draw]
	ret