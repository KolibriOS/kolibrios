;---------------------------------------------------------------------
error_window:
	test	dword [status],32
	jnz	still
	
	or	dword [status],32
	mcall	9,procinfo,-1
	mov	eax,[ebx+46]
	test	eax,eax
	jnz	@f
	
	mov	eax,100
	mov	[error_window_y+2],ax
	mov	[error_window_x+2],ax
	jmp	.end
;---------------------------------------------------------------------
@@:
	shr	eax,1
	add	eax,[ebx+38]
	sub	eax,50
	mov	[error_window_y+2],ax
	mov	eax,[ebx+42]
	shr	eax,1
	add	eax,[ebx+34]
	sub	eax,150
	mov	[error_window_x+2],ax
.end:
	call	get_memory_for_thread_stack
	mov	[thread_stack_6],eax
	mcall	51,1,thread6	; thread_stack_6
	ret
;---------------------------------------------------------------------
thread6:
	mcall	9,procinfo_threads,-1
	mov	eax,[ebx+30]
	mov	[PID6],eax
	mcall	40,0x7
	mov	[error_pointer],string
;--------------------------------------
.red:
	call	.draw_error_window
;--------------------------------------
.still:
	mcall	10
	cmp	eax,1
	je	.red

	cmp	eax,2			; key in buffer ?
	je	.key

	cmp	eax,3
	je	.button

	jmp	.still
;---------------------------------------------------------------------
.key: 		; key
.button:
	xor	dword [status],32
	xor	eax,eax
	mov	[PID6],eax
	mcall	68,13,[thread_stack_6]
	mcall	-1
	jmp	.still
;---------------------------------------------------------------------
;	*********************************************
;	*******	WINDOW DEFINITIONS AND DRAW ********
;	*********************************************
.draw_error_window:
	mcall	12, 1
	mcall	0,[error_window_x],[error_window_y],0x03ff0000
	mcall	71,1, errortext
	mcall	4,<10,30>,0x80ffffff,[error_pointer]
	cmp	[return_code],0
	je	.continue

	cmp	[return_code],1
	jne	@f

	mcall	4,<10,50>,0x90ffffff,error_not_supported_file
	jmp	.end
;---------------------------------------------------------------------
@@:
	cmp	[return_code],2
	jne	@f
	
	mcall	4,<10,50>,0x90ffffff,error_not_enough_memory
	jmp	.end
;---------------------------------------------------------------------
@@:
	mcall	4,<10,50>,0x90ffffff,error_unknown
	jmp	.end
;---------------------------------------------------------------------
.continue:
	mcall	4,<10,50>,0x90ffffff,error_type
	mcall	47,0x800a0000,[error_fs],<140,50>,0x10ffffff
	mov	edx,[error_fs]
	cmp	edx,0
	jb	.end
	
	cmp	edx,11
	ja	.end
	
	shl	edx,2
	add	edx,error_fs_text_pointers
	mov	edx,[edx]
	mcall	4,<10,70>,0x90ffffff,
;	mcall	47,0x800a0000,[error_locate],<140,5>,0x10ffffff
.end:
;	mcall	47,0x80000,[PID6],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------