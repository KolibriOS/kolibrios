;---------------------------------------------------------------------
;start_error_window_thread:
;	mcall	9,procinfo,-1
;	mov	eax,[ebx+46]
;	shr	eax,1
;	add	eax,[ebx+38]
;	sub	eax,40
;	mov	[error_window_y+2],ax
;	mov	eax,[ebx+42]
;	shr	eax,1
;	add	eax,[ebx+34]
;	sub	eax,125
;	mov	[error_window_x+2],ax
;	mcall	51,1,thread_start,thread_stack
;        DEBUGF  1, "start_error_window_thread\n"
;	ret
;---------------------------------------------------------------------
thread_start:
	mov	eax,[N_error]
;        DEBUGF  1, "thread_start N_error: %d\n",eax
;	cmp	al,1
;	jne	@f
;	mov	[N_error],load_ini_error_type
;	mov	[error_path],file_name
;	jmp	.error_type
;--------------------------------------
;@@:
	cmp	al,2
	jne	@f
	mov	[N_error],load_icons_error_type
	mov	[error_path],file_name
;        DEBUGF  1, "error_path: %s\n",file_name
	jmp	.error_type
;--------------------------------------
@@:
	cmp	al,3
	jne	@f
	mov	[N_error],memory_free_error_type
	xor	eax,eax
	mov	[error_path],eax
	mov	[error_type],eax
	jmp	.red
;--------------------------------------
@@:
	cmp	al,4
	jne	@f
	mov	[N_error],memory_get_error_type
	xor	eax,eax
	mov	[error_path],eax
	mov	[error_type],eax
	jmp	.red
;--------------------------------------
@@:
;	cmp	al,5
;	jne	@f
;	mov	[N_error],load_directory_error_type
;	mov	[error_path],dir_path
;	jmp	.error_type
;--------------------------------------
;@@:
	cmp	al,6
	jne	.button
	mov	[N_error],convert_icons_error_type
	mov	[error_path],file_name
	xor	eax,eax
	mov	[error_type],eax
	jmp	.red
;--------------------------------------
.error_type:
;        DEBUGF  1, ".error_type:\n"
	mov	eax,[error_type]
	shl	eax,2
	add	eax,error_fs_text_pointers
	mov	eax,[eax]
	mov	[error_type],eax
;--------------------------------------
.red:
;        DEBUGF  1, "draw_error_window\n"
	call	draw_error_window
;--------------------------------------
.still:
	mcall	10
	cmp	eax,1
	je	.red
	cmp	eax,2
	je	.key
	cmp	eax,3
	je	.button
	jmp	.still
;--------------------------------------
.key:
	mcall	2
	jmp	.still
;--------------------------------------
.button:
	mcall	-1
;---------------------------------------------------------------------
draw_error_window:
	mcall	12,1
	mcall	0,[error_window_x],[error_window_y],0x03ff0000
	mcall	4,<10,30>,0x90ffffff,[N_error]
	mov	eax,[error_path]
	test	eax,eax
	jz	@f
	mcall	4,<10,50>,,[error_path]
;--------------------------------------
@@:
	mov	eax,[error_type]
	test	eax,eax
	jz	@f
	mcall	4,<10,70>,,[error_type]
;--------------------------------------
@@:
	mcall	12,2
	ret
;---------------------------------------------------------------------
