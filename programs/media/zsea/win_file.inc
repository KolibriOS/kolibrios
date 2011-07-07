;---------------------------------------------------------------------
kfile:
	test	dword [status],1
	jnz	still
	or	dword [status],1

	call	load_directory.copy_dir_path
	cmp	[OpenDialog_path],dword 'NoKe'
	je	@f
	
	mov	[OpenDialog_data.start_path],dword OpenDialog_path
	jmp	.start_OpenDialog
;---------------------------------------------------------------------
@@:
	mov	ebx,open_dialog_name
	mov	esi,path
	mov	edi,library_path
	call	copy_file_path
	
.start_OpenDialog:
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	call	clear_control_key_flag
	cmp	[OpenDialog_data.status],2
	je	thread1_start
	
	cmp	[OpenDialog_data.status],1
	jne	@f
	
	xor	dword [status],1
	jmp	kopen_1
;---------------------------------------------------------------------
@@:
	xor	dword [status],1
	jmp	still
;---------------------------------------------------------------------	
thread1_start:
	call	get_memory_for_thread_stack
	mov	[thread_stack],eax
	mcall	51,1,thread1	; thread_stack
	jmp	still
;---------------------------------------------------------------------
thread1:			; start of thread1
	mcall	9,procinfo_threads, -1
	mov	eax,[ebx+30]
	mov	[PID1],eax
	mcall	40,0x27
	or	ecx,-1		; get information about me
	call	getappinfo
	mov	esi,string
;-------------------------------------
	cld
@@:
	lodsb
	test	al,al
	jne	@r
;-------------------------------------	
	sub	esi,string
	mov	eax,esi
	dec	eax
	mov	edi, edit1
	mov	[edi+48], eax  ;ed_size
	mov	[edi+52], eax  ;ed_pos
;-------------------------------------
.red:
	call	.draw_window
;-------------------------------------
.still:
	mcall	10		; wait here for event

	cmp	eax,1			; redraw request ?
	je	.red

	cmp	eax,2			; key in buffer ?
	je	.key

	cmp	eax,3			; button in buffer ?
	je	.button

	push	dword name_editboxes
	call	[edit_box_mouse]
	jmp	.still
;---------------------------------------------------------------------
.key: 			; key
	mcall
	cmp	ah,13
	je	.close_with_open_file

	cmp	ah,27
	je	.close
	
	push	dword name_editboxes
	call	[edit_box_key]
	jmp	.still
;---------------------------------------------------------------------
.close_with_open_file:
	mov	[open_file_flag],byte 1
	jmp	.close
;---------------------------------------------------------------------
.button:			; button
	mcall	17 		; get id
	cmp	ah,1			; button id=1 ?
	jne	.still
.close:
	xor	dword [status],1
	xor	eax,eax
	mov	[PID1],eax
	mcall	68,13,[thread_stack]
	mcall	-1 		; close this program
	jmp	.still
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
.draw_window:
	mcall	12,1
	xor	eax,eax		   ; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,300>,<100,80>,0x13eeeeee,,labelt1
	push    dword name_editboxes
	call    [edit_box_draw]
;	mcall	47,0x80000,[PID1],<200, 5>,0xffffff	
	mcall	12,2
	ret
;---------------------------------------------------------------------
