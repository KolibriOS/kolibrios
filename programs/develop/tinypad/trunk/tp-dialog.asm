diff16 'tp-dialog.asm',0,$

;-----------------------------------------------------------------------------
proc define_3d_button ;///////////////////////////////////////////////////////
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
endp
;-----------------------------------------------------------------------------
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
	cmp	[bot_mode2],0
	je	@f
	add	ebx,18
	mcall	4,,[sc.work_text],s_2replace,s_2replace.size+1
	mov	ecx,[esp]
	add	ecx,18*65536
    @@:

	mov	ebx,[p_info.client_box.width]
	shl	ebx,16

	push	20003
	cmp	[bot_mode2],0
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

	cmp	[bot_mode2],0
	je	@f
	mov	ebp,tb_replace
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2replace.size+2)+1
	add	eax,6*(s_2replace.size+2)*65536
	mov	dword[tbox.width],eax
	inc	ecx
	mov	dword[tbox.height],ecx
	call	textbox.draw
    @@:

	pop	ecx

	mov	ebp,tb_find
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2find.size+2)+1
	add	eax,6*(s_2find.size+2)*65536
	mov	dword[tbox.width],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.height],ecx
	call	textbox.draw

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
	cmp	[bot_mode2],0
	je	@f
	mov	eax,tb_replace
	cmp	eax,[focused_tb]
	jne	.key.lp1
	mov	eax,tb_find
  .key.lp1:
	mov	[focused_tb],eax
	call	.draw
    @@: ret
;-----------------------------------------------------------------------------
osdlg_handler:
	cmp	[bot_mode2], 2
	je	@f
	mov	[open_dialog],1
	ret
@@:
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
	cmp	[bot_mode2],0
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

	cmp	[bot_mode2], 2	    ; exit-save dialog
	jne	@f

	sub	ebx,(6*(s_2save_no.size+2)+3)*65536
	mov	bx,6*(s_2save_no.size+2)
	push	20007 s_2save_no s_2save_no.size
	call	define_3d_button

    @@: mov	ebp,tb_opensave
	mov	eax,[p_info.client_box.width]
	sub	eax,6*(s_2filename.size+1)+1
	add	eax,6*(s_2filename.size+1)*65536
	mov	dword[tbox.width],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.height],ecx
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
;-----------------------------------------------------------------------------
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
	cmp	[bot_mode2],0
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
	mov	dword[tbox.width],eax
	add	ecx,-18*65536+1
	mov	dword[tbox.height],ecx
	call	textbox.draw

	sub	ebx,(6*(s_2cancel.size+2)+3)*65536
	mov	bx,6*(s_2cancel.size+2)
	push	20010 s_2cancel s_2cancel.size
	call	define_3d_button

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
;-----------------------------------------------------------------------------
cur_part   dd ?
cur_color  dd ?
cur_colors rd 10
;-----------------------------------------------------------------------------
optsdlg_handler:
	cmp	al,1
	je	.draw
	cmp	al,2
	je	.key
	cmp	al,3
	je	botdlg.button
	cmp	al,4
	je	.mouse
	ret

  .xchg_colors:
	mov	ecx,10
	mov	esi,color_tbl
	mov	edi,cur_colors
    @@: lodsd
	xchg	eax,[edi]
	mov	[esi-4],eax
	add	edi,4
	loop	@b
	ret

  .draw:
	call	.draw_editor

	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	push	ecx
	add	ecx,128
	mcall	38,<2+3+165,2+3+165>,,[cl_3d_inset]
	mov	bx,word[p_info.client_box.width]
	pop	ecx
	add	ecx,35*65536+35
	mcall
	add	ecx,0x00240024
	mcall

	shr	ecx,16
	mov	bx,cx
	add	ebx,38*65536-21
	mcall	4,,[sc.work_text],s_tabpos,s_tabpos.size

	call	.draw_tabpos

	mov	ebx,[bot_ofs]
	add	ebx,(2+6+165+35)*65536+5+16
	mcall	4,,[sc.work_text],s_color,s_color.size

	mov	dword[tb_color.width],(2+6+6*s_color.size+165+35)*65536+(6*6+8)
	mov	eax,ebx
	add	ax,-3
	shl	eax,16
	mov	ax,14
	mov	dword[tb_color.height],eax

	call	.draw_color

	mov	ebx,[p_info.client_box.width]
	shl	ebx,16
	mov	ecx,[p_info.client_box.height]
	shl	ecx,16
	add	ecx,(-34)*65536+15

	add	ebx,-(2+6*(s_2save.size+2))*65536+6*(s_2save.size+2)
	push	20004 s_2save s_2save.size
	call	define_3d_button
	sub	ebx,(6*(s_2cancel.size+2)+3)*65536
	mov	bx,6*(s_2cancel.size+2)
	push	20001 s_2cancel s_2cancel.size
	call	define_3d_button

	ret

  .get_color:
	movzx	ecx,[tb_color.length]
	mov	esi,tb_color.text
	xor	eax,eax
	xor	ebx,ebx
	jecxz	.lp2
  .lp1: lodsb
	cmp	al,'9'
	mov	dl,-'0'
	jbe	@f
	mov	dl,-'A'+10
    @@: add	al,dl
	shl	ebx,4
	add	bl,al
	loop	.lp1
  .lp2: mov	eax,[cur_part]
	mov	[cur_colors+eax*4],ebx
	mov	[cur_color],ebx
	jmp	.draw_color.2

  .draw_editor:
	;push    dword[options] [tab_bar.Current.Ptr]
	push	[tab_bar.Current.Ptr]
	;mov     [options],0
	mov	ebp,optsdlg_editor
	call	set_cur_tab

	call	.xchg_colors
	mov	[cur_editor.Bounds.Left],2
	mov	[cur_editor.Bounds.Right],2+165
	mov	eax,[p_info.client_box.height]
	add	eax,-125-STATH
	mov	[cur_editor.Bounds.Top],eax
	add	eax,122
	mov	[cur_editor.Bounds.Bottom],eax
	call	draw_editor
	call	.xchg_colors

	;pop     ebp eax
	pop	ebp
	;mov     [options],al
	call	set_cur_tab
	ret

  .draw_tabpos:
	mov	ecx,[bot_ofs-2]
	xor	cx,cx
	mov	ebx,(2+3+165+3)*65536+31
	add	ecx,38*65536+31
	mov	edx,[cl_3d_inset]
	mov	esi,[sc.work]
	call	draw_fillrect
	dec	ebx
	dec	ecx
	mcall	8,,,0x40000000+21001
	mov	esi,[cl_3d_normal]
	mov	al,[tabs_pos]

TPOSH = 6
TPOSW = 10

	dec	al
	jnz	@f
	;// top
	inc	ebx
	mov	cx,TPOSH
	call	draw_fillrect
	add	ecx,2*65536-2
	jmp	.lp3
    @@: dec	al
	jnz	@f
	;// bottom
	inc	ebx
	mov	cx,TPOSH
	add	ecx,(31-TPOSH)*65536
	call	draw_fillrect
	add	ecx,-2
	jmp	.lp3
    @@: dec	al
	jnz	@f
	;// left
	inc	ecx
	mov	bx,TPOSW
	call	draw_fillrect
	add	ebx,2*65536-2
	jmp	.lp4
    @@: dec	al
	jnz	@f
	;// right
	inc	ecx
	mov	bx,TPOSW
	add	ebx,(31-TPOSW)*65536
	call	draw_fillrect
	add	ebx,-2
	jmp	.lp4

  .lp3:
	add	ebx,(2+TPOSW-2+1)*65536-31+TPOSW-2
	call	draw_fillrect
	add	ebx,-(TPOSW-2+1)*65536
	mov	esi,[sc.work]
	call	draw_fillrect
	ret
  .lp4:
	add	ecx,7*65536-31+4
	call	draw_fillrect
	add	ecx,-5*65536
	mov	esi,[sc.work]
	call	draw_fillrect
	ret

  .draw_color:
	mov	ecx,[cur_part]
	mov	edx,s_appearance+1
    @@: dec	ecx
	js	@f
	movzx	eax,byte[edx-1]
	lea	edx,[edx+eax+1]
	jmp	@b
    @@:
	movzx	esi,byte[edx-1]
	mov	ebx,[bot_ofs]
	add	ebx,(2+6+165+35)*65536+8
	push	ebx ecx edx
	mov	ecx,ebx
	shl	ecx,16
	mov	bx,s_appearance.maxl*6
	mov	cx,10
	mcall	13,,,[cl_3d_normal]
	pop	edx ecx ebx
	mcall	4,,[sc.work_text]

	mov	eax,[cur_color]
	mov	edi,tb_color.text
	mov	ebx,6
	mov	ecx,16
	call	uint2strz
	mov	[tb_color.length],6
	mov	[tb_color.sel.x],0
	mov	[tb_color.pos.x],6

	mov	ebp,tb_color
	call	textbox.draw

  .draw_color.2:
	mov	ecx,[p_info.client_box.height]
	sub	ecx,[bot_dlg_height]
	add	ecx,-STATH
	shl	ecx,16
	add	ecx,3*65536+31
	mov	ebx,(2+6+165)*65536+31
	mov	edx,[cl_3d_inset]
	mov	esi,[cur_color]
	call	draw_fillrect

	ret

  .key:
	cmp	ebx,KEY_ESCAPE
	je	btn.bot.cancel
	cmp	ebx,KEY_RETURN
	je	btn.bot.appearance
	cmp	ebx,KEY_NUMRETURN
	je	btn.bot.appearance

	cmp	ebx,KEY_BACKSPACE
	je	.key.tb.2
	cmp	ebx,KEY_TAB
	je	.key.tb.2
	cmp	bx,0x00FF
	ja	.key.tb.2
	;cmp     ebx,KEY_LSHIFT
	;je      .key.tb.2
	;cmp     ebx,KEY_RSHIFT
	;je      .key.tb.2
	;cmp     ebx,0x00000147
	;jb      .key.exit
	;cmp     ebx,0x00000153
	;jbe     .key.tb.2

	test	[chr],KM_CTRLALT
	jnz	.key.exit
	movzx	eax,byte[chr]
	movzx	eax,[eax+key0]
	or	al,al
	jz	.key.exit
	movzx	eax,[eax+key1]

	cmp	al,'0'
	jb	@f
	cmp	al,'9'
	jbe	.key.tb
    @@: cmp	al,'A'
	jb	@f
	cmp	al,'F'
	jbe	.key.tb
    @@: cmp	al,'a'
	jb	@f
	cmp	al,'f'
	jbe	.key.tb
    @@: ret
  .key.tb:
	cmp	[tb_color.length],6
	jb	@f
	mov	al,[tb_color.sel.x]
	cmp	al,[tb_color.pos.x]
	jne	@f
	ret
  .key.tb.2:
    @@: call	textbox.key
	call	.get_color
	call	.draw_editor
  .key.exit:
	ret

  .mouse:
	cmp	ah,MEV_LDOWN
	jne	.mouse.exit
	mcall	37,1
	movsx	ebx,ax
	sar	eax,16
	cmp	eax,[optsdlg_editor+EDITOR.Bounds.Right]
	jg	.mouse.exit
	cmp	ebx,[optsdlg_editor+EDITOR.Bounds.Bottom]
	jg	.mouse.exit
	sub	eax,[optsdlg_editor+EDITOR.Bounds.Left]
	js	.mouse.exit
	sub	ebx,[optsdlg_editor+EDITOR.Bounds.Top]
	js	.mouse.exit
	mov	esi,optsdlg_editor_parts-5
    @@: add	esi,5
	cmp	byte[esi+0],-1
	je	.mouse.exit
	cmp	al,byte[esi+1]
	jb	@b
	cmp	bl,byte[esi+2]
	jb	@b
	cmp	al,byte[esi+3]
	ja	@b
	cmp	bl,byte[esi+4]
	ja	@b
	movzx	ebp,byte[esi+0]
	mov	[cur_part],ebp
	m2m	[cur_color],[cur_colors+ebp*4]
	;mcall   13,<200,30>,<250,30>,[color_tbl+ebp*4]
	;call    .draw_color
	call	.draw

  .mouse.exit:
	ret
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
  btn.bot.cancel:
	xor	eax,eax
	mov	[bot_mode],al
	mov	[main_closing],al
	mov	[bot_dlg_height],eax
	mov	[s_status],eax
	call	drawwindow
	ret
;-----------------------------------------------------------------------------
  btn.bot.opensave:
	cmp	[bot_mode2],0
	je	.lp1
	call	save_file
	jnc	@f
	jmp	.lp2
  .lp1: call	load_file
	jnc	@f
  .lp2:
	ret
    @@: ;call    update_caption
	xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	drawwindow
	ret
;-----------------------------------------------------------------------------
  btn.bot.no:
	xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	mov	[s_status],eax
	call	drawwindow
	cmp	[main_closing],0
	je	@f
	add	[exit_tab_item],sizeof.TABITEM
	jmp	key.alt_x.direct
	ret
    @@: call	key.ctrl_f4.close
	ret
;-----------------------------------------------------------------------------
  btn.bot.find:
	movzx	ecx,[tb_find.length]
	mov	[s_search.size],ecx
	mov	esi,tb_find.text
	mov	edi,s_search
	cld
	rep	movsb

	cmp	[bot_mode2],0
	je	@f
	call	search
	jnc	.found
	call	editor_check_for_changes
	ret

  .found:
	push	[copy_size] [copy_count] [copy_buf]

	movzx	eax,[tb_replace.length]
	add	eax,10
	stdcall mem.Alloc,eax
	mov	[copy_buf],eax

	movzx	eax,[tb_replace.length]
	mov	esi,tb_replace.text
	mov	edi,[copy_buf]
;	stosd
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	ecx,eax
	jecxz	.lp1
	rep	movsb
  .lp1: add	eax,sizeof.EDITOR_LINE_DATA
	mov	[copy_size],eax
	mov	[copy_count],1

	push	[cur_editor.SelStart.X]
	mov	ebp,cur_editor
	call	init_sel_vars
	call	key.ctrl_v
	pop	[cur_editor.SelStart.X]

	stdcall mem.Free,[copy_buf]

	pop	[copy_buf] [copy_count] [copy_size]

	call	editor_check_for_changes
	ret
    @@: xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	key.f3
	call	drawwindow
	ret
;-----------------------------------------------------------------------------
  btn.bot.appearance:
    @@: xor	eax,eax
	mov	[bot_mode],al
	mov	[bot_dlg_height],eax
	call	optsdlg_handler.xchg_colors
	mov	al,[tabs_pos]
	mov	[tab_bar.Style],al

	stdcall save_settings
	call	drawwindow
	ret
;-----------------------------------------------------------------------------
tabpos_round db 4,3,1,2
;-----------------------------------------------------------------------------
  btn.bot.tabpos:
	movzx	eax,[tabs_pos]
	mov	al,[tabpos_round+eax-1]
	mov	[tabs_pos],al
    @@: call	optsdlg_handler.draw_tabpos
	ret
;-----------------------------------------------------------------------------