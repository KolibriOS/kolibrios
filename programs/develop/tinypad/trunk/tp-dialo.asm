;-----------------------------------------------------------------------------
func define_3d_button ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; ebx = <x,width>
; ecx = <y,height>
; esp+4*3 = id
; esp+4*2 = text
; esp+4*1 = text length
;-----------------------------------------------------------------------------
	pushad
	mov	edx,[esp+4*8+4+4*2]
	or	edx,0x40000000
	mcall	8
	mov	eax,[esp+4*8+4*1]
	add	eax,2
	imul	eax,6
	pushad
	shr	ebx,16
	shr	ecx,16
	push	ebx ecx eax 15
	call	draw_3d_panel
	popad
	shr	ecx,16
	mov	bx,cx
	add	ebx,7*65536+4
	mcall	4,,[sc.work_text],[esp+4*8+4*2],[esp+4*8+4*1]
	popad
	ret	4*3
endf

finddlg_handler:
	cmp	al,1
	je	.draw
	cmp	al,2
	je	.key
	cmp	al,3
	je	botdlg.button
	ret

  .draw:
	mov	ebx,[bot_ofs]
	add	ebx,(1+3)*65536+6
	mcall	4,,[sc.work_text],s_2find,s_2find.size+1
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(3+17)*65536+15

	push	ecx
	cmp	[bot_dlg_mode2],0
	je	@f
	add	ebx,18
	mcall	4,,[sc.work_text],s_2replace,s_2replace.size+1
	mov	ecx,[esp]
	add	ecx,18*65536
    @@:

	mov	ebx,[p_info.client_box.width]
	shl	ebx,16

	push	20003
	cmp	[bot_dlg_mode2],0
	jne	.draw.lp1
	add	ebx,-(2+6*(s_2find.size+2))*65536+6*(s_2find.size+2)
	push	s_2find s_2find.size
	jmp	@f

  .draw.lp1:
	add	ebx,-(2+6*(s_2replace.size+2))*65536+6*(s_2replace.size+2)
	push	s_2replace s_2replace.size

    @@: call	define_3d_button
	sub	ebx,(6*(s_2cancel.size+2)+3)*65536
	mov	bx,6*(s_2cancel.size+2)
	push	20001 s_2cancel s_2cancel.size
	call	define_3d_button

	mov	ecx,[esp]

	mov	ebp,tb_find
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2find.size+2)+1
	add	eax,6*(s_2find.size+2)*65536
	mov	dword[tbox.x],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.y],ecx
	call	textbox.draw

	pop	ecx

	cmp	[bot_dlg_mode2],0
	je	@f
	mov	ebp,tb_replace
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2replace.size+2)+1
	add	eax,6*(s_2replace.size+2)*65536
	mov	dword[tbox.x],eax
	inc	ecx
	mov	dword[tbox.y],ecx
	call	textbox.draw
    @@:

	ret

  .key:
	cmp	ebx,KEY_ESCAPE
	je	btn.bot.cancel
	cmp	ebx,KEY_RETURN
	je	btn.bot.find
	cmp	ebx,KEY_NUMRETURN
	je	btn.bot.find
	cmp	ebx,KEY_TAB
	je	..tab
	call	textbox.key
	ret

  ..tab:
	cmp	[bot_dlg_mode2],0
	je	@f
	mov	eax,tb_replace
	cmp	eax,[focused_tb]
	jne	.key.lp1
	mov	eax,tb_find
  .key.lp1:
	mov	[focused_tb],eax
	call	.draw
    @@: ret

osdlg_handler:
	cmp	al,1
	je	.draw
	cmp	al,2
	je	.key
	cmp	al,3
	je	botdlg.button
	ret

  .draw:
	mov	ebx,[bot_ofs]
	add	ebx,(1+3)*65536+6
	mcall	4,,[sc.work_text],s_2filename,s_2filename.size
	mov	ebx,[p_info.client_box.width]
	shl	ebx,16
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(2+18)*65536+15

	push	20002
	cmp	[bot_dlg_mode2],0
	jne	.draw.lp1
	add	ebx,-(2+6*(s_2open.size+2))*65536+6*(s_2open.size+2)
	push	s_2open s_2open.size
	jmp	@f
  .draw.lp1:
	add	ebx,-(2+6*(s_2save.size+2))*65536+6*(s_2save.size+2)
	push	s_2save s_2save.size

    @@: call	define_3d_button
	sub	ebx,(6*(s_2cancel.size+2)+3)*65536
	mov	bx,6*(s_2cancel.size+2)
	push	20001 s_2cancel s_2cancel.size
	call	define_3d_button

	mov	ebp,tb_opensave
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2filename.size+1)+1
	add	eax,6*(s_2filename.size+1)*65536
	mov	dword[tbox.x],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.y],ecx
	call	textbox.draw

	ret

  .key:
	cmp	ebx,KEY_ESCAPE
	je	btn.bot.cancel
	cmp	ebx,KEY_RETURN
	je	btn.bot.opensave
	cmp	ebx,KEY_NUMRETURN
	je	btn.bot.opensave
	call	textbox.key
	ret

gotodlg_handler:
	cmp	al,1
	je	.draw
	cmp	al,2
	je	.key
	cmp	al,3
	je	botdlg.button
	ret

  .draw:
	mov	ebx,[bot_ofs]
	add	ebx,(1+3)*65536+6
	mcall	4,,[sc.work_text],s_2filename,s_2filename.size
	mov	ebx,[p_info.box.width]
	shl	ebx,16
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(2+18)*65536+15

	push	20002
	cmp	[bot_dlg_mode2],0
	jne	.draw.lp1
	add	ebx,-(2+6*(s_2open.size+2))*65536+6*(s_2open.size+2)
	push	s_2open s_2open.size
	jmp	@f
  .draw.lp1:
	add	ebx,-(2+6*(s_2save.size+2))*65536+6*(s_2save.size+2)
	push	s_2save s_2save.size

    @@: call	define_3d_button
	sub	ebx,(6*(s_2cancel.size+2)+3)*65536
	mov	bx,6*(s_2cancel.size+2)
	push	20001 s_2cancel s_2cancel.size
	call	define_3d_button

	mov	ebp,tb_opensave
	mov	eax,[p_info.box.width]
	sub	eax,6*(s_2filename.size+3)
	add	eax,6*(s_2filename.size+2)*65536
	mov	dword[tbox.x],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.y],ecx
	call	textbox.draw

	ret

  .key:
	cmp	ebx,KEY_ESCAPE
	je	btn.bot.cancel
	cmp	ebx,KEY_RETURN
	je	btn.bot.opensave
	cmp	ebx,KEY_NUMRETURN
	je	btn.bot.opensave
	call	textbox.key
	ret

botdlg.button:
	mov	esi,accel_table2_botdlg
  .acc: cmp	ebx,[esi]
	jne	@f
	call	dword[esi+4]
	ret
    @@: add	esi,8
	cmp	byte[esi],0
	jne	.acc
	ret

  btn.bot.cancel:
	xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	drawwindow
	ret

  btn.bot.opensave:
	cmp	[bot_dlg_mode2],0
	je	.lp1
	call	save_file
	jnc	@f
	jmp	.lp2
  .lp1: call	btn.load_file
	jnc	@f
  .lp2:
	ret
    @@: call	update_caption
	xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	drawwindow
	ret

  btn.bot.find:
	movzx	ecx,[tb_find.length]
	mov	[s_search.size],ecx
	mov	esi,tb_find.text
	mov	edi,s_search
	cld
	rep	movsb

	cmp	[bot_dlg_mode2],0
	je	@f
	call	search
	jnc	.found
	call	check_inv_all
	ret

  .found:
;---------------------------------------
	push	[copy_size] [copy_count]

	mov	esi,AREA_CBUF
	mov	edi,AREA_CBUF-304
	mov	ecx,300/4
	rep	movsd

	movzx	eax,[tb_replace.length]
	mov	esi,tb_replace.text
	mov	edi,AREA_CBUF
	stosd
	mov	ecx,eax
	jecxz	.lp1
	rep	movsb
  .lp1: add	eax,4
	mov	[copy_size],eax
	mov	[copy_count],1

	push	[sel.x]
	call	init_sel_vars
	call	key.ctrl_v
	pop	[sel.x]

	mov	esi,AREA_CBUF-304
	mov	edi,AREA_CBUF
	mov	ecx,300/4
	rep	movsd

	pop	[copy_count] [copy_size]
;---------------------------------------

	call	check_inv_all
	ret
    @@: xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	btn.search
	call	drawwindow
	ret
