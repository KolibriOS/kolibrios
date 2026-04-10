
draw_tray:	; draw cpu usage, time, date etc.
	pusha
	call	draw_tray_buttons
	call	draw_time_skin
	call	draw_time
	mov	[draw_flag_certainly],1
	call	draw_flag	; language
	call	draw_list_button
	popa
	ret
;---------------------------------------------------------------
draw_tray_buttons:
	mov	[offset_x_tray],0
	cmp	[minimize_right],dword 0
	je	@f
	add	[offset_x_tray],MR_SIZE
@@:
	mov	ecx,[height]
	mov	esi,[wcolor]
	mov	eax,8
	cmp	[clock_enable],dword 0
	je	@f
	add	[offset_x_tray],CLOCK_SIZE
	mov	ebx,[max_x]
	sub	ebx,[offset_x_tray]
	shl	ebx,16
	add	ebx,CLOCK_SIZE+1
	mcall	,,,0x60000000+2		; time/date button
	sub	ebx,3
	mov	[pos_x_clock],ebx
@@:
	cmp	[cpu_usage_enable],dword 0
	je	@f
	add	[offset_x_tray],CPU_USAGE_SIZE
	mov	ebx,[max_x]
	sub	ebx,[offset_x_tray]
	shl	ebx,16
	add	ebx,CPU_USAGE_SIZE+1
	mcall	,,,0x60000000+18	;button 18 - sysmeter_name (GMON)
	sub	ebx,3
	mov	[pos_x_cpu_usage],ebx
@@:
	cmp	[chlang_enable],dword 0
	je	@f
	add	[offset_x_tray],CHLANG_SIZE
	mov	ebx,[max_x]
	sub	ebx,[offset_x_tray]
	shl	ebx,16
	add	ebx,CHLANG_SIZE+1
	mcall	,,,0x60000000+16	;button 16 - chang language
	sub	ebx,3
	mov	[pos_x_cpu_chlang],ebx
@@:
	cmp	[page_list_enable],dword 0
	je	@f
	add	[offset_x_tray],PAGE_LIST_SIZE
	mov	ebx,[max_x]
	sub	ebx,[offset_x_tray]
	shl	ebx,16
	add	ebx,10
	mov	[pos_x_page_list],ebx
	mcall	,,,0x60000000+22	;button 22 - page list decrease

	add	ebx,24 shl 16
	mcall	,,,0x60000000+21	;button 21 - page list decrease
@@:
	ret
;---------------------------------------------------------------
draw_time_skin:
	cmp	[clock_enable],dword 0
	je	@b
	mov	ebx,[pos_x_clock]
	call	calculate_button_y_coordinate_and_size

	mov	edx,[Clock_color]	;time_bgr_color
	mov	esi,[wcolor]
	jmp	draw_appl_button
;---------------------------------------------------------------
draw_time:
	cmp	[clock_enable],dword 0
	je	@b
	mcall	3
	cmp	eax,[ptime]
	jz	@b
	mov	[ptime],eax

; draw hours and minutes
	movzx	ebx,al
	shr	eax,8
	movzx	ecx,al
	shr	eax,8
	movzx	edx,al
	push	ebx
	push	ecx

	mov	ebx,[pos_x_clock]
	call	calculate_button_y_coordinate_and_size
	movzx	eax,cx
	shr	eax,1
	sub	eax,4
	shr	ecx,16
	add	eax,ecx
	mov	bx,ax
	add	ebx,18 shl 16
	mov	ecx,[PanelText_color]
	or	ecx,0x40000000
	mov	edx,[esp]	; __:_X
	and	edx,15
	add	ebx,10*65536
	add	edx,text
	mcall	4,,,,1,[Clock_color]	;time_bgr_color

	pop	edx		; __:X_
	shr	edx,4
	and	edx,15
	sub	ebx,6*65536
	add	edx,text
	mcall

	mov	edx,[esp]	; _X:__
	and	edx,15
	sub	ebx,11*65536
	add	edx,text
	mcall

	pop	edx		; X_:__
	shr	edx,4
	and	edx,15
	sub	ebx,6*65536
	add	edx,text
	mcall

; draw seconds
	mov	ebx,[pos_x_clock]
	call	calculate_button_y_coordinate_and_size
	movzx	eax,cx
	shr	eax,1
	sub	eax,4
	shr	ecx,16
	add	eax,ecx
	mov	bx,ax
	add	ebx,17 shl 16
	mcall	3

	mov	ecx,eax
	shr	ecx,16
	and	ecx,1
	mov	edx,[PanelText_color]
	sub	edx,[Clock_color]	;time_bgr_color;[wcolor]
	imul	ecx,edx
	add	ecx,[Clock_color]	;time_bgr_color;[wcolor]
	mcall	4,,,sec,1

	jmp	draw_cpu_usage
;---------------------------------------------------------------
draw_list_button:
	cmp	[page_list_enable],dword 0
	je	@f

; draw page list button  "skin"
	mov	ebx,[pos_x_page_list]
	mov	bx,10
	call	calculate_button_y_coordinate_and_size

	xor	edx,edx
	mov	edx,[PageList_color]	;0xffffff
	mov	esi,[wcolor]
	call	draw_appl_button

	add	ebx,24 shl 16 ; 18 shl 16
	mov	edx,[PageList_color]	;0xffffff
	mov	esi,[wcolor]
	call	draw_appl_button

; draw page list button  text <>
	mov	ebx,[pos_x_page_list]
	add	ebx,2 shl 16
	call	calculate_button_y_coordinate_and_size
	movzx	eax,cx
	shr	eax,1
	sub	eax,4
	shr	ecx,16
	add	eax,ecx
	mov	bx,ax
	xor	ecx,ecx
	mcall	4,,,page_a1,5

	add	ebx,1 shl 16
	mcall

; draw page list number
	mov	edx,ebx
	add	edx,9 shl 16
	mcall	47,0x20000,[page_list],,[system_colours.work_button_text]
@@:
	ret
;---------------------------------------------------------------
draw_cpu_usage:
	cmp	[cpu_usage_enable],dword 0
	je	@b

	pushad

	mov	eax,[height]
	sub	eax,[button_top_offset]
	sub	eax,[button_bottom_offset]
	sub	eax,2
	mov	[ysi],eax	;12
	mcall	18,5	; TSC / SEC

	shr	eax,20
	push	eax
	mcall	18,4	; IDLE / SEC

	shr	eax,20
	xor	edx,edx
	imul	eax,[ysi]
	cdq
	pop	ebx
	inc	ebx
	div	ebx
	cmp	eax,[ysi]
	jng	@f
	mov	eax,[ysi]
@@:
	push	eax
	mov	ebx,[pos_x_cpu_usage]
	mov	ecx,[button_top_offset]
	shl	ecx,16
	add	ecx,[ysi]
	add	ecx,2
	mov	edx,[CpuUsageBckgr_color]	;0xdd2222
	mov	esi,[wcolor]
	call	draw_appl_button
	pop	eax

	mov	ecx,[button_top_offset]
	inc	ecx
	shl	ecx,16
	add	ecx,eax
	add	ebx,1 shl 16
	sub	ebx,2
	mcall	13,,,[CpuUsage_color]	;0x44aa44

	popad
@@:
	ret
;---------------------------------------------------------------
; get and draw keyboard layout
draw_flag:
	cmp	[chlang_enable],dword 0
	je	@b
	cmp	[draw_flag_certainly],0
	je	@b

	pusha

	mcall	26,2,9
	mov	edx,eax
	mov	[type_lang],al

	pushad
	mov	ebx,[pos_x_cpu_chlang]
	call	calculate_button_y_coordinate_and_size
	mov	edx,[ChangeLang_color]	;time_bgr_color	;0xff ;[wcolor]
	mov	esi,[wcolor]
	call	draw_appl_button
	popad

	mov	ebx,[pos_x_cpu_chlang]
	mov	ax,bx
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	sub	ebx,6 shl 16
	call	calculate_button_y_coordinate_and_size
	movzx	eax,cx
	shr	eax,1
	sub	eax,4
	shr	ecx,16
	add	eax,ecx
	mov	bx,ax
	mov	ecx,[PanelText_color]

if caps_lock_check
	push	ebx
	mov	[flag_text],flagTextSm
	mcall	66,3
	test	eax, 0x40
	jz	@f
	mov	[flag_text],flagTextBg
@@:
	pop	ebx
end if

	dec	edx
	shl	edx,1
	add	edx,[flag_text]
	mcall	4,,,,2
	popa
	mov	[draw_flag_certainly],0
	ret
