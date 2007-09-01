
;POP_WIDTH   = (popup_text.max_title+popup_text.max_accel+6)*6
POP_IHEIGHT = 16
POP_SHEIGHT = 3
;POP_HEIGHT  = popup_text.cnt_item*POP_IHEIGHT+popup_text.cnt_sep*4+4

popup_thread_start:
	mov	[popup_active],1
	mov	[pi_cur],0
	mov	ebp,[esp]
	mcall	14
	movzx	ebx,ax
	shr	eax,16
	movzx	ecx,[ebp+POPUP.x]
	add	cx,[ebp+POPUP.width]
	cmp	ecx,eax
	jle	@f
	mov	cx,[ebp+POPUP.width]
	sub	[ebp+POPUP.x],cx
    @@: movzx	ecx,[ebp+POPUP.y]
	add	cx,[ebp+POPUP.height]
	cmp	ecx,ebx
	jle	@f
	mov	cx,[ebp+POPUP.height]
	sub	[ebp+POPUP.y],cx
    @@: mcall	40,01100111b		; ipc mouse button key redraw
	cmp	[mi_cur],0
	jl	.2
	sub	esp,32-16
	push	0 0 8 0
	mcall	60,1,esp,32
  .2:	call	draw_popup_wnd

  still_popup:
	cmp	[main_closed],1
	je	close_popup
	mcall	10
	cmp	eax,1
	je	popup_thread_start.2
	cmp	eax,2
	je	key_popup
	cmp	eax,3
	je	button_popup
	cmp	eax,6
	je	mouse_popup
	cmp	eax,7
	jne	still_popup

	mov	ebp,[POPUP_STACK]
	mov	dword[POPUP_STACK-32+4],8
	movzx	ebx,[ebp+POPUP.x]
	movzx	ecx,[ebp+POPUP.y]
	movzx	edx,[ebp+POPUP.width]
	movzx	esi,[ebp+POPUP.height]
	mcall	67
	jmp	still_popup

  mouse_popup:
	mov	ecx,mst2
	call	get_mouse_event
	cmp	al,MEV_LDOWN
	je	check_popup_click
	cmp	al,MEV_MOVE
	je	check_popup_move

	mcall	9,p_info2,-1
	cmp	ax,[p_info2.window_stack_position]
	jne	close_popup

	jmp	still_popup

  check_popup_click:
	mov	eax,[pi_cur]
	or	al,al
	js	close_popup
	jz	still_popup
	mov	ebx,[ebp+POPUP.actions]
	mov	[just_from_popup],1
	call	dword[ebx+eax*4-4]
	inc	[just_from_popup]
	jmp	close_popup

  check_popup_move:
	mov	eax,[pi_cur]
	call	get_active_popup_item
	cmp	eax,[pi_cur]
	je	still_popup
	call	draw_popup_wnd
	jmp	still_popup

  key_popup:
	mcall	;2
	cmp	ah,27
	jne	still_popup

  button_popup:
	mcall	17

  close_popup:
	mcall	18,3,[p_info.PID]
	mov	[popup_active],0
	mov	[mi_cur],0
	mcall	-1

func draw_popup_wnd
	mcall	12,1

	mov	ebx,dword[ebp+POPUP.x-2]
	mov	bx,[ebp+POPUP.width]
	mov	ecx,dword[ebp+POPUP.y-2]
	mov	cx,[ebp+POPUP.height]
	mcall	0,,,0x01000000,0x01000000

	movzx	ebx,bx
	movzx	ecx,cx
	pushd	0 0 ebx ecx
	call	draw_3d_panel

	mov	[pi_sel],0
	mov	eax,4
	mpack	ebx,3*6,3
	mov	ecx,[sc.work_text]
	mov	edx,[ebp+POPUP.data]
    @@: inc	[pi_sel]
	inc	edx
	movzx	esi,byte[edx-1]
	cmp	byte[edx],'-'
	jne	.lp1
	pushad
	mov	ecx,ebx
	shl	ecx,16
	mov	cx,bx
	movzx	ebx,[ebp+POPUP.width]
	add	ebx,0x00010000-1
	add	ecx,0x00010001
	mcall	38,,,[cl_3d_inset]
	add	ecx,0x00010001
	mcall	,,,[cl_3d_outset]
	popad
	add	ebx,4
	jmp	.lp2
  .lp1: mov	edi,[pi_sel]
	cmp	edi,[pi_cur]
	jne	.lp3
	test	byte[ebp+edi-1],0x01
	jz	.lp3
	pushad
	movzx	ecx,bx
	shl	ecx,16
	mov	cl,POP_IHEIGHT-1
	movzx	ebx,[ebp+POPUP.width]
	add	ebx,0x00010000-1
	mcall	13,,,[cl_3d_pushed]
	rol	ecx,16
	mov	ax,cx
	rol	ecx,16
	mov	cx,ax
	mcall	38,,,[cl_3d_inset]
	add	ecx,(POP_IHEIGHT-1)*65536+POP_IHEIGHT-1
	mcall	,,,[cl_3d_outset]
	popad
  .lp3: add	ebx,(POP_IHEIGHT-7)/2

	pushad
	test	byte[ebp+edi-1],0x02
	jz	.lp8
	movzx	ecx,bx
	shr	ebx,16
	add	ebx,-11
	add	ecx,2
	mov	edx,[sc.work_text]
	call	draw_check
  .lp8: popad

	mov	ecx,[sc.work_text]
	test	byte[ebp+edi-1],0x01
	jnz	.lp5
	add	ebx,0x00010001
	mov	ecx,[cl_3d_outset]
	mcall
	sub	ebx,0x00010001
	mov	ecx,[cl_3d_inset]
  .lp5: mcall
	push	ebx
	add	edx,esi
	inc	edx
	movzx	esi,byte[edx-1]
	add	ebx,[ebp+POPUP.acc_ofs]
	cmp	edi,[pi_cur]
	je	.lp4
	mov	ecx,[cl_3d_inset]
  .lp4: test	byte[ebp+edi-1],0x01
	jnz	.lp6
	add	ebx,0x00010001
	mov	ecx,[cl_3d_outset]
	mcall
	sub	ebx,0x00010001
	mov	ecx,[cl_3d_inset]
  .lp6: mcall
	pop	ebx
	add	ebx,POP_IHEIGHT-(POP_IHEIGHT-7)/2
  .lp2: add	edx,esi
	cmp	byte[edx],0
	jne	@b
  .exit:
	mcall	12,2
	ret
endf

func setup_main_menu_popup
	mov	ebx,[p_info.box.left]
	add	ebx,[p_info.client_box.left]
    @@: dec	ecx
	jz	@f
	add	edx,8+1
	movzx	esi,byte[edx-1]
	add	edx,esi
	jmp	@b
    @@: movzx	ecx,word[edx+2]
	add	ebx,ecx

	mov	[eax+POPUP.x],bx
	mov	ebx,[p_info.box.top]
	add	ebx,[p_info.client_box.top]
	add	ebx,ATOPH-1
	mov	[eax+POPUP.y],bx
	mov	[POPUP_STACK],eax
	ret
endf

onshow:

  .file:
	or	byte[mm.File+3],0x01
	cmp	[f_info.length],0
	jne	@f
	and	byte[mm.File+3],0xFE
    @@: ret

  .edit:
	or	byte[mm.Edit+2],0x01
	cmp	[copy_size],0
	jne	@f
	and	byte[mm.Edit+2],0xFE
    @@: or	dword[mm.Edit+0],0x01000101
	cmp	[sel.selected],0
	jne	@f
	and	dword[mm.Edit+0],0xFEFFFEFE
    @@: ret

  .search:
	mov	byte[mm.Search+0],0
	ret
  .run:
	ret
  .recode:
	ret
  .options:
	;mov     word[mm.Options+0],0
	mov	byte[mm.Options+5],0
	or	byte[mm.Options+2],0x02
	test	[options],OPTS_SECURESEL
	jnz	@f
	and	byte[mm.Options+2],0xFD
    @@: or	byte[mm.Options+3],0x02
	test	[options],OPTS_AUTOBRACES
	jnz	@f
	and	byte[mm.Options+3],0xFD
    @@: or	byte[mm.Options+4],0x02
	test	[options],OPTS_AUTOINDENT
	jnz	@f
	and	byte[mm.Options+4],0xFD
    @@: or	byte[mm.Options+6],0x02
	test	[options],OPTS_OPTIMSAVE
	jnz	@f
	and	byte[mm.Options+6],0xFD
    @@: or	byte[mm.Options+8],0x02
	test	[options],OPTS_LINENUMS
	jnz	@f
	and	byte[mm.Options+8],0xFD
    @@: ret

pi_sel	 dd ?
pi_cur	 dd ?
p_pos	 dd ?
popup_active db 0
