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
	mov	eax,option_group1
	mov	ecx,[eax]
	push	dword option_boxes_bcgr
	call	[option_box_mouse]
	cmp	ecx,[eax]
	je	.option_boxes_SCC
	
	mov	ebx,bgrmode
	cmp	[eax],dword op1
	jne	@f
	mov	[ebx],dword 1
	jmp	.option_group1_color_set
;---------------------------------------------------------------------
@@:
	cmp	[eax],dword op2
	jne	@f
	mov	[ebx],dword 2
	jmp	.option_group1_color_set
;---------------------------------------------------------------------
@@:
	cmp	[eax],dword op3
	jne	@f
	mov	[ebx],dword 3
	jmp	.option_group1_color_set
;---------------------------------------------------------------------
@@:
	cmp	[eax],dword op4
	jne	.option_boxes_SCC
	mov	[ebx],dword 4
.option_group1_color_set:
	mov	[bcgr_group_pointer],0
	call	.option_boxes_redraw
	jmp	.still
;---------------------------------------------------------------------
.option_boxes_SCC:
	mov	eax,[option_group3]
	push	dword option_boxes_SCC
	call	[option_box_mouse]
	cmp	eax,[option_group3]
	je	.still
	
	call	.convert_1
	mov	[bcgr_group_pointer],1
	call	.option_boxes_redraw
	jmp	.still
;---------------------------------------------------------------------
.tab:
	mov	al,[bcgr_group_pointer]
	inc	al
	and	al,1
	mov	[bcgr_group_pointer],al
	call	.option_boxes_redraw
	jmp	.still
;---------------------------------------------------------------------
.key:
	mcall
	cmp	ah,27
	je	.close
	
	cmp	ah,13
	je	.kok
	
	cmp	ah,9
	je	.tab
	
	cmp	[bcgr_group_pointer],1
	je	.key_option_group3
;---------------------------------------------------------------------
.key_option_group1:
	mov	ebx,bgrmode
	
	cmp	ah,178 ;up
	jne	.nofup
	
	cmp	[ebx],dword 1
	je	.fdn
.fup:
	dec	dword [ebx]
	jmp	.flagcont
;--------------------------------------
.nofup:
	cmp	ah,177 ;down
	jne	.still
	
	cmp	[ebx],dword 4
	je	.fup
.fdn:
	inc	dword [ebx]
.flagcont:
	mov	eax,option_group1
	cmp	[ebx],dword 1
	jne	@f
	
	mov	[eax],dword op1
	call	.draw_opt_boxes_bcgr
	jmp	.still
;---------------------------------------
@@:
	cmp	[ebx],dword 2
	jne	@f
	
	mov	[eax],dword op2
	call	.draw_opt_boxes_bcgr
	jmp	.still
;----------------------------------------
@@:
	cmp	[ebx],dword 3
	jne	@f
	
	mov	[eax],dword op3
	call	.draw_opt_boxes_bcgr
	jmp	.still
;-----------------------------------------
@@:
	mov	[eax],dword op4
	call	.draw_opt_boxes_bcgr
	jmp	.still
;---------------------------------------------------------------------
.key_option_group3:
	push	eax
	call	.convert_1
	pop	eax

	cmp	ah,178	;up
	jne	@f

	dec	word [ebx+2]
	jmp	.flagcont_1
;------------------------------------------
@@:
	cmp	ah,177	;down
	jne	@f

	inc	word [ebx+2]
	jmp	.flagcont_1	
;------------------------------------------	
@@:
	cmp	ah,176	;left
	jne	@f

	dec	word [ebx]
	jmp	.flagcont_1
;------------------------------------------
@@:
	cmp	ah,179	;right
	jne	.still

	inc	word [ebx]
;------------------------------------------
.flagcont_1:	
	call	.convert_2
	call	.convert_1	;for additional check
	call	.draw_opt_boxes_SCC
	jmp	.still
;---------------------------------------------------------------------
.convert_1:
	mov	eax,option_group3
	mov	ebx,Stretch_Crop_Corner
	cmp	[eax],dword op_UL
	jne	@f
	mov	[ebx],dword 0 shl 16+0
	jmp	.option_group3_color_set
;----------------------------------------
@@:
	cmp	[eax],dword op_UC
	jne	@f
	mov	[ebx],dword 0 shl 16+1
	jmp	.option_group3_color_set
;-----------------------------------------
@@:
	cmp	[eax],dword op_UR
	jne	@f
	mov	[ebx],dword 0 shl 16+2
	jmp	.option_group3_color_set
;-----------------------------------------
@@:
	cmp	[eax],dword op_CL
	jne	@f
	mov	[ebx],dword 1 shl 16+0
	jmp	.option_group3_color_set
;------------------------------------------
@@:
	cmp	[eax],dword op_CC
	jne	@f
	mov	[ebx],dword 1 shl 16+1
	jmp	.option_group3_color_set
;-------------------------------------------
@@:
	cmp	[eax],dword op_CR
	jne	@f
	mov	[ebx],dword 1 shl 16+2
	jmp	.option_group3_color_set
;--------------------------------------------
@@:
	cmp	[eax],dword op_DL
	jne	@f
	mov	[ebx],dword 2 shl 16+0
	jmp	.option_group3_color_set
;--------------------------------------------
@@:
	cmp	[eax],dword op_DC
	jne	@f
	mov	[ebx],dword 2 shl 16+1
	jmp	.option_group3_color_set
;--------------------------------------------
@@:
	cmp	[eax],dword op_DR
	jne	.option_group3_color_set
	mov	[ebx],dword 2 shl 16+2
.option_group3_color_set:
	ret
;---------------------------------------------------------------------	
.convert_2:
	mov	eax,Stretch_Crop_Corner
	mov	ebx,option_group3
	cmp	[eax],dword 0 shl 16+0
	jne	@f
	mov	[ebx],dword op_UL
	jmp	.Stretch_Crop_Corner_set
;----------------------------------------
@@:
	cmp	[eax],dword 0 shl 16+1
	jne	@f
	mov	[ebx],dword op_UC
	jmp	.Stretch_Crop_Corner_set
;-----------------------------------------
@@:
	cmp	[eax],dword 0 shl 16+2
	jne	@f
	mov	[ebx],dword op_UR
	jmp	.Stretch_Crop_Corner_set
;-----------------------------------------
@@:
	cmp	[eax],dword 1 shl 16+0
	jne	@f
	mov	[ebx],dword op_CL
	jmp	.Stretch_Crop_Corner_set
;------------------------------------------
@@:
	cmp	[eax],dword 1 shl 16+1
	jne	@f
	mov	[ebx],dword op_CC
	jmp	.Stretch_Crop_Corner_set
;-------------------------------------------
@@:
	cmp	[eax],dword 1 shl 16+2
	jne	@f
	mov	[ebx],dword op_CR
	jmp	.Stretch_Crop_Corner_set
;--------------------------------------------
@@:
	cmp	[eax],dword 2 shl 16+0
	jne	@f
	mov	[ebx],dword op_DL
	jmp	.Stretch_Crop_Corner_set
;--------------------------------------------
@@:
	cmp	[eax],dword 2 shl 16+1
	jne	@f
	mov	[ebx],dword op_DC
	jmp	.Stretch_Crop_Corner_set
;--------------------------------------------
@@:
	cmp	[eax],dword 2 shl 16+2
	jne	.Stretch_Crop_Corner_set
	mov	[ebx],dword op_DR
.Stretch_Crop_Corner_set:
	ret	
;---------------------------------------------------------------------
.button:		; button
	mcall	17	; get id
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
	mcall	,<100,300>,<100,150>,0x13909090,,labelt4
	mcall	8,<70,40>,<110,20>,4,0xdddddd
	mcall	4,<46,28>,0xffffff,bgrdtext,bgrdtext.size
	add	ebx,40*65536+88
	xor	ecx,ecx
	mcall	,,,ok_btn,ok_btn.size
	call	.option_boxes_redraw
;	mcall	47,0x80000,[PID4],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------
.option_boxes_redraw:
	call	.draw_opt_boxes_bcgr
	call	.draw_opt_boxes_SCC
	ret
;---------------------------------------------------------------------
.draw_opt_boxes_bcgr:
	mov	edx,0x13909090
	cmp	[bcgr_group_pointer],0
	jne	@f
	
	mov	edx,0xffff80
@@:
	mcall	13,<59,14>,<39,59>,
	push	dword option_boxes_bcgr
	call	[option_box_draw]
	ret
;---------------------------------------------------------------------
.draw_opt_boxes_SCC:
	mov	edx,0x13909090
	cmp	[bcgr_group_pointer],1
	jne	@f
	
	mov	edx,0xffff80
@@:
	mcall	13,<198,70>,<38,70>,
	push	dword option_boxes_SCC
	call	[option_box_draw]
	ret