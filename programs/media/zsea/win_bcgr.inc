;---------------------------------------------------------------------
k_background:
	test	dword [status],8
	jnz	still
	or	dword [status],8

	call	get_memory_for_thread_stack
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
;---------------------------------------------------------------------
.mouse:
	push	dword option_boxes_bcgr
	call	[option_box_mouse]
	
	cmp	[option_group1],op1
	jne	@f
	mov	[bgrmode],dword 1
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[option_group1],op2
	jne	@f
	mov	[bgrmode],dword 2
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[option_group1],op3
	jne	@f
	mov	[bgrmode],dword 3
	jmp	.still
;---------------------------------------------------------------------
@@:
;	cmp	[option_group1],op4
;	jne	.still
	mov	[bgrmode],dword 4
	jmp	.still
;---------------------------------------------------------------------
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
;---------------------------------------------------------------------
.nofup:
	cmp	ah,177 ;down
	jne	.still
	
	cmp	dword [bgrmode],4
	je	.fup
.fdn:
	inc	dword [bgrmode]
.flagcont:
	cmp	[bgrmode],dword 1
	jne	@f
	
	mov	[option_group1],op1
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[bgrmode],dword 2
	jne	@f
	
	mov	[option_group1],op2
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[bgrmode],dword 3
	jne	@f
	
	mov	[option_group1],op3
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	mov	[option_group1],op4
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
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
;---------------------------------------------------------------------
;	*********************************************
;	*******	WINDOW DEFINITIONS AND DRAW ********
;	*********************************************
.draw_window:
	mcall	12,1
	xor	eax,eax
	xor	esi,esi
	mcall	,<100,200>,<100,200>,0x13909090,,labelt4
	mcall	8,<70,40>,<110,20>,4,0xdddddd
	mcall	4,<46,28>,0xffffff,bgrdtext,bgrdtext.size
	add	ebx,40*65536+88
	xor	ecx,ecx
	mcall	,,,ok_btn,ok_btn.size
	call	.draw_opt_boxes
;	mcall	47,0x80000,[PID4],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------
.draw_opt_boxes:
	push	dword option_boxes_bcgr
	call	[option_box_draw]
	ret