;---------------------------------------------------------------------
kinfo:
	test	dword [status],2
	jnz	still
	or	dword [status],2

	call	get_memory_for_thread_stack
	mov	[thread_stack_2],eax
	mcall	51,1,thread2	;, thread_stack_2
	jmp	still
;---------------------------------------------------------------------
thread2:			    ; start of info thread
	mcall	9,procinfo_threads,-1
	mov	eax,[ebx+30]
	mov	[PID2],eax
	mcall	40,0x7
	call	load_directory.copy_dir_path
;---------------------------------------------------------------------
.red:
	call	.draw_window
;---------------------------------------------------------------------
.still:
	mcall	10	; wait here for event
	cmp	eax,1	; redraw request ?
	je	.red

	cmp	eax,2	; key in buffer ?
	je	.close

	cmp	eax,3	; button in buffer ?
	je	.button

	jmp	.still
;---------------------------------------------------------------------
.button:			   ; button
	mcall	17	; get id
	cmp	ah,1	; button id=1 ?
	jne	.still  ;.noclose
.close:
	xor	dword [status],2
	xor	eax,eax
	mov	[PID2],eax
	mcall	68,13,[thread_stack_2]
	mcall	-1	; close this program
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
.draw_window:
	mcall	12,2
	xor	eax,eax		; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,330>,<100,120>,0x13eeeeee,,labelt2
	mcall	4,<120,30>,0x80000000,file_name
	mov	edx,fitext
	mov	esi,14
	mov	ebx,10*65536+30
	mov	ecx,0x10000000
@@:
	mcall
	add	ebx,10
	add	edx,esi
	cmp	ebx,10*65536+80
	jbe	@r
	
	mov	esi,ecx
	mcall	47,0x800a0000,[img_size],<120,40>
	
	add	edx,10
	mcall	,,[img_width_2]
	
	add	edx,10
	mcall	,,[img_high_2]
	
	add	edx,10
	mcall	,,[img_resolution_2]
	
	add	edx,10
	mcall	,,[Total_Repeat_N]

;	mcall 47,0x80000,[PID2],<200, 5>,0xffffff
;	mcall 47,0x800a0000,[error_locate],<140,5>,0x10ffffff
	mcall	12,2
	ret
;--------------------------------------------------------------------
