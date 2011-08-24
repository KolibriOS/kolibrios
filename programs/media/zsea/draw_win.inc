;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
	or	[bWasDraw],1
	mcall	12,1

	xor	eax,eax		; function 0 : define and draw window
	mov	ebx,[wnd_width]
	mov	ecx,[wnd_height]
	xor	esi,esi
	mcall	,,,0x73ffffff,,Authors_text.1

	call	get_window_param
	mov	eax,[procinfo+70]	;status of window
	test	eax,100b
	jne	.end
	
	call	draw_other

	push	dword menu_data_1
	call	[menu_bar_draw]
	push	dword menu_data_2
	call	[menu_bar_draw]
	push	dword menu_data_3
	call	[menu_bar_draw]
	push	dword menu_data_4
	call	[menu_bar_draw]
	push	dword menu_data_5
	call	[menu_bar_draw]
	push	dword menu_data_6
	call	[menu_bar_draw]

	push	dword dinamic_button_data_1
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_2
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_3
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_4
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_5
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_6
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_7
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_8
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_9
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_10
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_11
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_12
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_13
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_14
	call	[dinamic_button_draw]
	push	dword dinamic_button_data_15
	call	[dinamic_button_draw]
	
	mcall	13,<225,2>,<3,22>,0
	mcall	,<280,2>
	mcall	,<410,2>
	mcall	,<490,2>
	mcall	,<545,2>
	mcall	,<625,2>
	mcall	,<720,2>
	mcall	4,<630,3>,0x80000000, file_mount
	mcall	13,<677,40>,<13,11>,0xe5e5e5
	mov	ecx,[load_directory_pointer]
	mov	ecx,[ecx+4]
	mcall	47,0x80060000, ,<679, 15>,0
	mcall	8,[dinamic_button_data_5.x],[dinamic_button_data_5.y],0x40000002
.end:
	mcall	12,2
	ret
;---------------------------------------------------------------------
set_draw_scrollbar_all:
	xor	eax,eax
	inc	eax
	mov	[scroll_bar_data_vertical.all_redraw],eax
	mov	[scroll_bar_data_horizontal.all_redraw],eax
	ret
;---------------------------------------------------------------------
set_nodraw_scrollbar_all:
	xor	eax,eax
	mov	[scroll_bar_data_vertical.all_redraw],eax
	mov	[scroll_bar_data_horizontal.all_redraw],eax
	ret
;---------------------------------------------------------------------
draw_other:
	call	drawimage
@@:
	xor	ebx,ebx
	shl	ebx,16
	add	ebx,[window_width]
	xor	ecx,ecx
	shl	ecx,16
	add	ecx,28
	mov	edx,0xffffff
	cmp	[no_draw_window],0
	jne	@f
	mcall 13   ; fill top area
@@:
	mov	eax,[soi]
	test	eax,eax
	jnz	@f
	mov	ecx,[window_high]
	sub	ecx,28
	add	ecx,28 shl 16
	mcall	13
@@:
	call	draw_number_of_file
	call	draw_zoom_factors
	cmp	[scaling_mode],dword 0
	jne	@f
	ret
@@:
	call	set_draw_scrollbar_all
	call	draw_scrollbars
	call	set_nodraw_scrollbar_all
	ret
;---------------------------------------------------------------------	
draw_number_of_file:
	mcall	13,<630,42>,<13,11>,0xe5e5e5
	xor	ecx,ecx
	mov	eax,[load_directory_pointer]
	cmp	[eax+4],dword 0
	je 	@f
	mov	ecx,[load_file_number]
	inc	ecx
@@:
	mcall	47,0x80060000, ,<632, 15>,0
	ret
;---------------------------------------------------------------------
draw_zoom_factors:
	mcall	13,<725,30>,<13,11>,0xe5e5e5
	mov	ecx,[zoom_factors]
	mov	ecx,[ecx]
	mov	[scaling_mode],ecx
	test	ecx,ecx
	jnz	@f
	mcall	4,<727,14>,0x80000000,zoom_auto_text
	ret
@@:
	mcall	47,0x80060000,,<727, 15>,0
	ret
;---------------------------------------------------------------------