;---------------------------------------------------------------------
k_option:
	test	dword [status],64
	jnz	still
	or	dword [status],64

	call	get_memory_for_thread_stack
	mov	[thread_stack_7],eax
	mcall	51,1,thread7	; thread_stack_7
	jmp	still
;---------------------------------------------------------------------
get_memory_for_thread_stack:
	mcall	68,12,4096
	mov	edx,eax
	add	edx,4096
	ret
;---------------------------------------------------------------------
thread7:				; start of bgrd thread
	mcall	9, procinfo_threads, -1
	mov	eax,[ebx+30]
	mov	[PID7],eax
	mcall	40,0x27
	mov	eax,[slide_show_delay]
	mov	[slide_show_delay_old],eax
	mov	eax,[arrows_offset]
	mov	[arrows_offset_old],eax
	mov	eax,[check1+32]
	mov	[show_file_name_old],eax
	mov	eax,[check2+32]
	mov	[filtering_flag_old],eax
	xor	eax,eax
	mov	[option_pointer],eax
	call	.set_pointers
;-----------------------------------------
.red:
	call	.draw_window
;-----------------------------------------
.still:
	mcall	10 		; wait here for event

	cmp	eax,1			; redraw request ?
	je	.red
	
	cmp	eax,2			; key in buffer ?
	je	.key
	
	cmp	eax,3			; button in buffer ?
	je	.button
	
	cmp	eax,6	; mouse in buffer ?
	je	.mouse
	
	jmp	.still
;---------------------------------------------------------------------
.mouse:
	mov	ebx,[check1+32]
	and	ebx,10b
	push	dword check1
	call	[check_box_mouse]
	mov	eax,[check1+32]
	and	eax,10b
	cmp	ebx,eax
	je	@f	;.still
	
	mov	[option_pointer],2
	call	.set_pointers
	call	.draw_all_options_values
	jmp	.still
;---------------------------------------------------------------------
@@:
	mov	ebx,[check2+32]
	and	ebx,10b
	push	dword check2
	call	[check_box_mouse]
	mov	eax,[check2+32]
	and	eax,10b
	cmp	ebx,eax
	je	.still
	
	mov	[option_pointer],3
	call	.set_pointers
	call	.draw_all_options_values
	jmp	.still
;---------------------------------------------------------------------
.key:
	mcall
	cmp	ah,27
	je	.close
	
	cmp	ah,13
	je	.kok
	
	cmp	ah,32
	je	.select_space
	
	cmp	ah,56 ;plus
	je	.select_plus	;.plus_slide_show_delay
	
	cmp	ah,43 ;plus
	je	.select_plus	;.plus_slide_show_delay
	
	cmp	ah,61 ;plus
	je	.select_plus ;.plus_slide_show_delay
	
	cmp	ah,54 ;minus
	je	.select_minus ;.minus_slide_show_delay
	
	cmp	ah,45 ;minus
	je	.select_minus	;.minus_slide_show_delay
	
	cmp	ah,95 ;minus
	je	.select_minus ;.minus_slide_show_delay
	
	cmp	ah,178 ;up arrow
	je	.up_arrow
	
	cmp	ah,177 ;down arrow
	je	.down_arrow
	
	jmp	.still
;---------------------------------------------------------------------
.button:			 ; button
	mov	eax,17 		; get id
	mcall
	
	cmp	ah,5
	je	.minus_arrows_offset
	
	cmp	ah,4
	je	.plus_arrows_offset
	
	cmp	ah,3
	je	.minus_slide_show_delay
	
	cmp	ah,2
	je	.plus_slide_show_delay

	cmp	ah,1			; button id=1 ?
	jne	.noclose
;-----------------------------------
.close:
	mov	eax,[slide_show_delay_old]
	mov	[slide_show_delay],eax
	mov	eax,[arrows_offset_old]
	mov	[arrows_offset],eax
	mov	eax,[show_file_name_old]
	mov	[check1+32],eax
	mov	eax,[filtering_flag_old]
	mov	[check2+32],eax
	jmp	.close_all
;---------------------------------------------------------------------
.close_1:
	mov	eax,[check2+32]
	test	eax,10b
	jz	@f
	
	mov	[filtering_flag],1
	jmp	.redraw_flag
;---------------------------------------------------------------------
@@:
	mov	[filtering_flag],0
.redraw_flag:
	mov	[redraw_flag],byte 1
.close_all:
	xor	dword [status],64
	xor	eax,eax
	mov	[PID7],eax
	mcall	68,13,[thread_stack_7]
	mcall	-1 		; close this program
;---------------------------------------------------------------------
.noclose:
.kok:
	jmp	.close_1
;---------------------------------------------------------------------
.select_plus:
	jmp	[plus_pointer]
;---------------------------------------------------------------------
.select_minus:
	jmp	[minus_pointer]
;---------------------------------------------------------------------
.select_space:
	jmp	[space_pointer]
;---------------------------------------------------------------------
.plus_slide_show_delay:
	cmp	[option_pointer],0
	je	@f
	
	mov	[option_pointer],0
	call	.draw_all_options_values
@@:
	cmp	[slide_show_delay],99990
	je	.still
	
	add	[slide_show_delay],10
	call	.draw_slide_show_delay
	jmp	.still
;---------------------------------------------------------------------
.minus_slide_show_delay:
	cmp	[option_pointer],0
	je	@f
	
	mov	[option_pointer],0
	call	.draw_all_options_values
@@:
	cmp	[slide_show_delay],0
	je	.still
	
	sub	[slide_show_delay],10
	call	.draw_slide_show_delay
	jmp	.still
;---------------------------------------------------------------------
.plus_arrows_offset:
	cmp	[option_pointer],1
	je	@f
	
	mov	[option_pointer],1
	call	.draw_all_options_values
@@:
	cmp	[arrows_offset],100
	je	.still
	
	inc	[arrows_offset]
	call	.draw_arrows_offset
	call	calculate_arrows_offset
	jmp	.still
;---------------------------------------------------------------------	
.minus_arrows_offset:
	cmp	[option_pointer],1
	je	@f
	
	mov	[option_pointer],1
	call	.draw_all_options_values
@@:
	cmp	[arrows_offset],1
	je	.still
	
	dec	[arrows_offset]
	call	.draw_arrows_offset
	call	calculate_arrows_offset
	jmp	.still
;---------------------------------------------------------------------
.space_check1:
	test	[check1+32],dword 10b
	jnz	@f
	
	or	[check1+32],dword 10b
	jmp	.space_check1_1
;---------------------------------------------------------------------
@@:
	xor	[check1+32],dword 10b
.space_check1_1:
	call	.draw_all_options_values
	jmp	.still
;---------------------------------------------------------------------
.space_check2:
	test	[check2+32],dword 10b
	jnz	@f
	
	or	[check2+32],dword 10b
	jmp	.space_check2_1
;---------------------------------------------------------------------
@@:
	xor	[check2+32],dword 10b
.space_check2_1:
	call	.draw_all_options_values
	jmp	.still
;---------------------------------------------------------------------
.down_arrow:
	cmp	[option_pointer],3
	je	.still
	
	inc	[option_pointer]
.draw_all_points:
	call	.set_pointers
	call	.draw_all_options_values
	jmp	.still
;---------------------------------------------------------------------	
.up_arrow:
	cmp	[option_pointer],0
	je	.still
	
	dec	[option_pointer]
	jmp	.draw_all_points
;---------------------------------------------------------------------	
.set_pointers:
	cmp	[option_pointer],0
	jne	@f
	
	mov	[plus_pointer],thread7.plus_slide_show_delay
	mov	[minus_pointer],thread7.minus_slide_show_delay
	mov	[space_pointer],thread7.still
	ret
@@:
	cmp	[option_pointer],1
	jne	@f
	
	mov	[plus_pointer],thread7.plus_arrows_offset
	mov	[minus_pointer],thread7.minus_arrows_offset
	mov	[space_pointer],thread7.still
	ret
@@:
	cmp	[option_pointer],2
	jne	@f
	
	mov	[plus_pointer],thread7.still
	mov	[minus_pointer],thread7.still
	mov	[space_pointer],thread7.space_check1
	ret
@@:
	mov	[plus_pointer],thread7.still
	mov	[minus_pointer],thread7.still
	mov	[space_pointer],thread7.space_check2
	ret
;---------------------------------------------------------------------
.draw_all_options_values:
	call	.draw_slide_show_delay
	call	.draw_arrows_offset
	call	.draw_check1
	call	.draw_check2
	ret
;---------------------------------------------------------------------
;	*********************************************
;	*******	WINDOW DEFINITIONS AND DRAW ********
;	*********************************************
.draw_window:
	mcall	12,1
	xor	eax,eax			; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,300>,<100,120>,0x13909090,,optiontext
	mcall	8,<15,10>,<50,11>,2,0xdddddd
	inc	edx
	mcall	,<77,10>
;---------------------------------
	inc	edx
	mcall	,<15,10>,<90,11>
	inc	edx
	mcall	,<77,10>
;---------------------------------
	inc	edx
	mcall	,<250,40>,<50,20>
;---------------------------------
	mcall	4,<10,35>,0x80ffffff,slide_show_delay_title
	mcall	,<17,53>,0x90000000,slide_show_delay_text
	mcall	,<10,75>,0x80ffffff,arrows_offset_title
	mcall	,<17,93>,0x90000000,slide_show_delay_text
	mcall	,<265,57>, ,ok_btn
	call	.draw_all_options_values
;	mcall	47,0x80000,[PID5],<200, 5>,0xffffff
	mcall	12,2
	ret
;---------------------------------------------------------------------
.draw_slide_show_delay:
	mov	edx,0xffffff
	cmp	[option_pointer],0
	jne	@f
	
	mov	edx,0xffff80
@@:
	mcall	13,<29,45>,<50,12>,
	mcall	47,0x800a0000,[slide_show_delay],<32,52>,0x10000000;	0x40000000,0xffffff
	ret
;---------------------------------------------------------------------
.draw_arrows_offset:
	mov	edx,0xffffff
	cmp	[option_pointer],1
	jne	@f
	
	mov	edx,0xffff80
@@:
	mcall	13,<29,45>,<90,12>,
	mcall	47,0x800a0000,[arrows_offset],<32,92>,0x10000000;	0x40000000,0xffffff
	ret
;---------------------------------------------------------------------
.draw_check1:
	mov	[check1+12],dword 0xffffff
	cmp	[option_pointer],2
	jne	@f
	
	mov	[check1+12],dword 0xffff80
@@:
	push	dword check1
	call	[check_box_draw]
	ret
;---------------------------------------------------------------------
.draw_check2:
	mov	[check2+12],dword 0xffffff
	cmp	[option_pointer],3
	jne	@f
	
	mov	[check2+12],dword 0xffff80
@@:
	push	dword check2
	call	[check_box_draw]
	ret
;---------------------------------------------------------------------	