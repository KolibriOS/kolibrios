;---------------------------------------------------------------------
k_sort_directory:
	test	dword [status],16
	jnz	still
	or	dword [status],16

	call	get_memory_for_thread_stack
	mov	[thread_stack_5],eax
	mcall	51,1,thread5	; thread_stack_5
	jmp	still
;---------------------------------------------------------------------
thread5:				; start of bgrd thread
	mcall	9,procinfo_threads,-1
	mov	eax,[ebx+30]
	mov	[PID5],eax
	mcall	40,0x27
	mov	eax,[sort_type]
	cmp	[sort_type],dword 0
	jne	@f
	
	mov	[option_group2],sort0
	jmp	.thread5
;---------------------------------------------------------------------
@@:
	cmp	[sort_type],dword 2
	jne	@f
	
	mov	[option_group2],sort2
	jmp	.thread5
;---------------------------------------------------------------------
@@:
	cmp	[sort_type],dword 4
	jne	@f
	
	mov	[option_group2],sort4
	jmp	.thread5
;---------------------------------------------------------------------
@@:
	mov	[option_group2],sort6
;-----------------------------------
.thread5:
	mov	[sort_type_old],eax
	mov	eax,[option_group2]
	mov	[option_group2_old],eax
;-----------------------------------
.red:
	call	.draw_window
;-----------------------------------
.still:
	mcall	10	; wait here for event

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
	push	dword option_boxes_sort
	call	[option_box_mouse]

	cmp	[option_group2],sort0
	jne	@f
	
	mov	[sort_type],dword 0
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[option_group2],sort2
	jne	@f
	
	mov	 [sort_type],dword 2
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[option_group2],sort4
	jne	@f
	
	mov	[sort_type],dword 4
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[option_group2],sort6
	jne	@f
	
	mov	[sort_type],dword 6
	jmp	.still
;---------------------------------------------------------------------
.key:
	mcall
	cmp	ah,27
	je	.close

	cmp	ah,13
	je	.kok

	cmp	ah,177 ;down
	je	.fdn

	cmp	ah,178 ;up
	jne	.still

	cmp	[sort_type],0
	je	.still
	
	sub	[sort_type],2
	jmp	.flagcont
;---------------------------------------------------------------------
.fdn:
	cmp	[sort_type],6
	je	.still
	
	add	[sort_type],2
	
.flagcont:
	cmp	[sort_type],dword 0
	jne	@f
	
	mov	[option_group2],sort0
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[sort_type],dword 2
	jne	@f
	
	mov	[option_group2],sort2
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[sort_type],dword 4
	jne	@f
	
	mov	[option_group2],sort4
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
@@:
	cmp	[sort_type],dword 6
	jne	.still	;@f
	
	mov	[option_group2],sort6
	call	.draw_opt_boxes
	jmp	.still
;---------------------------------------------------------------------
.button:			 ; button
	mcall	17 		; get id
	cmp	ah,1			; button id=1 ?
	jne	.noclose
;-------------------------------
.close:
	mov	eax,[sort_type_old]
	mov	[sort_type],eax
	mov	eax,[option_group2_old]
	mov	[option_group2],eax
.close_1:
	xor	dword [status],16
	xor	eax,eax
	mov	[PID5],eax
	mcall	68,13,[thread_stack_5]
	mcall	-1 		; close this program
;---------------------------------------------------------------------
.noclose:
.kok:
	mov	eax,[load_directory_pointer]
	cmp	[eax+4],dword 0
	je	.close_1
	
	call	load_directory.copy_dir_path
	call	load_directory.sort
	mov	[sort_directory_flag],byte 1
	jmp	.close_1
;---------------------------------------------------------------------
;	 *********************************************
;	 ******** WINDOW DEFINITIONS AND DRAW ********
;	 *********************************************
.draw_window:
	mcall	12,1
	xor	eax,eax			 ; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,300>,<100,110>,0x13909090,,labelt5
	mcall	8,<120,40>,<50,20>,4,0xdddddd
	mcall	4,<46,28>,0xffffff,sorttext,sorttext.size
	xor	ecx,ecx
	mcall	,<135,57>,,ok_btn,ok_btn.size
	call	.draw_opt_boxes
;	mcall	47,0x80000,[PID5],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------
.draw_opt_boxes:
	push	dword option_boxes_sort
	call	[option_box_draw]
	ret
;---------------------------------------------------------------------
