set_rect_window:
	mov	ebx,set_rect_window_procinfo
	call	get_slot_n

	mov	[set_rect_window_slot],ecx

	set_events_mask (evm_redraw+evm_key+evm_button+evm_mouse)
.red:
	edit_boxes_set_sys_color rect_input,rect_input_end,sc
	labels_set_sys_color rect_input_labels,rect_input_labels_end,sc
	check_boxes_set_sys_color riw_check_boxes,riw_check_boxes_end,sc
	call	.draw_window
.still:
	wait_event .red,.key,.button,.mouse

.key:
	get_key

	key_edit_boxes rect_input,rect_input_end

	call	read_rect
	call	draw_rect_on_screen

	jmp	.still

.button:
	get_pressed_button

	cmp	ah,1
	jne	@f
	btr	dword [flags],3
	jmp	close
@@:

	jmp	.still

.mouse:
	get_active_window
	cmp	eax,[set_rect_window_slot]
	jne	.still

	mouse_edit_boxes rect_input,rect_input_end
	mouse_check_boxes riw_check_boxes,riw_check_boxes_end

	jmp	.still

.draw_window:
	start_draw_window

	xor	eax,eax
	mov	ebx,100*65536+250
	mov	ecx,100*65536+130
	mov	edx,[sc.work]
	add	edx,0x33000000
	mov	edi,riw_grab_text
	xor	esi,esi
	int	0x40

	draw_labels rect_input_labels,rect_input_labels_end

	draw_edit_boxes rect_input,rect_input_end

	draw_check_boxes riw_check_boxes,riw_check_boxes_end

	call	read_rect
	call	draw_rect_on_screen

	stop_draw_window
ret

;--------------------------------------------------------------------
;--- Считывание облати ----------------------------------------------
;--------------------------------------------------------------------
read_rect:
	bt	dword [use_rect_active_window.flags],1
	jc	read_rect_from_active_window

	mov	edi,rect_input_buffer.left
	call	zstr_to_int
	cmp	ax,[scr.width]
	jb	@f
	mov	ax,[scr.width]
@@:
	mov	[rect.left],ax

	mov	edi,rect_input_buffer.top
	call	zstr_to_int
	cmp	ax,[scr.height]
	jb	@f
	mov	ax,[scr.height]
@@:
	mov	[rect.top],ax

	mov	edi,rect_input_buffer.width
	call	zstr_to_int
	mov	bx,[scr.width]
	sub	bx,[rect.left]
	cmp	ax,bx
	jb	@f
	mov	ax,bx
@@:
	mov	[rect.width],ax

	mov	edi,rect_input_buffer.height
	call	zstr_to_int
	mov	bx,[scr.height]
	sub	bx,[rect.top]
	cmp	ax,bx
	jb	@f
	mov	ax,bx
@@:
	mov	[rect.height],ax
ret

read_rect_from_active_window:
	call	get_active_window_info

	mov	eax,[active_app.left]
	mov	[rect.left],ax
	mov	eax,[active_app.top]
	mov	[rect.top],ax
	mov	eax,[active_app.width]
	inc	eax
	mov	[rect.width],ax
	mov	eax,[active_app.height]
	inc	eax
	mov	[rect.height],ax
ret

draw_rect_on_screen:

	mov	eax,13
	mov	ebx,150*65536+80
	mov	ecx,5*65536+60
	xor	edx,edx
	int	0x40

	;movzx   eax,word [rect.left]
	;mul     word [scr.width]
	xor	edx,edx
	movzx	eax,word [scr.width]
	mov	ebx,80
	div	ebx
	mov	ebx,eax

	xor	edx,edx
	movzx	eax,word [rect.height]
	div	ebx
	push	ax

	xor	edx,edx
	movzx	eax,word [rect.width]
	div	ebx
	push	ax

	xor	edx,edx
	movzx	eax,word [rect.top]
	div	ebx
	push	ax

	xor	edx,edx
	movzx	eax,word [rect.left]
	div	ebx
	push	ax

	pop	bx
	add	bx,150
	shl    ebx,16

	pop	cx
	add	cx,5
	shl	ecx,16

	pop	bx
	pop	cx

	mov	edx,0xffffff
	mov	eax,13
	int	0x40
ret

rect_input:
.left	edit_box 35,95,5,cl_white,0,0,0,5,rect_input_buffer.left,\
ed_figure_only+ed_focus
.top	edit_box 35,95,25,cl_white,0,0,0,5,rect_input_buffer.top,\
ed_figure_only
.width	edit_box 35,95,45,cl_white,0,0,0,5,rect_input_buffer.width,\
ed_figure_only
.height edit_box 35,95,65,cl_white,0,0,0,5,rect_input_buffer.height,\
ed_figure_only
rect_input_end:

rect_input_labels:
.left label 10,10,0,rect_input_labels_text.left
.top label 10,30,0,rect_input_labels_text.top
.width label 10,50,0,rect_input_labels_text.width
.height label 10,70,0,rect_input_labels_text.height
rect_input_labels_end:

rect_input_labels_text:
.left db 'Отступ слева:',0
.top  db 'Отступ сверху:',0
.width	db 'Ширина:',0
.height db 'Высота:',0

riw_check_boxes:
use_rect_active_window check_box 10,85,cl_white,0,0,\
riw_check_boxes_text.1,\
riw_check_boxes_text.2-riw_check_boxes_text.1
riw_check_boxes_end:

riw_check_boxes_text:
.1 db 'Использовать область активного окна'
.2 db ''

riw_grab_text db 'Координаты и размеры области:',0