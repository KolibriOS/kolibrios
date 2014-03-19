diff16 'tp-mouse.asm',0,$

proc check_mouse_in_edit_area
	mcall	37,1
	mov	ebx,eax
	and	ebx,0x0000FFFF
	shr	eax,16
	mov	ecx,[cur_editor.Bounds.Top]
	inc	ecx
	pushd	[cur_editor.Bounds.Left] ecx [cur_editor.Bounds.Right] ecx
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	sub	[__rc+0x8],SCRLW+6
	mov	ecx,[cur_editor.Gutter.Width]
	add	[__rc+0x0],ecx
	imul	ecx,[lines.scr],LINEH
	dec	ecx
	add	[__rc+0xC],ecx
	mov	ecx,__rc
	call	pt_in_rect
	ret
endp

proc get_mouse_event
	mcall	37,2
	and	al,3
	mov	bl,[ecx]
	cmp	[ecx],al
	mov	[ecx],al
	jne	@f
	mcall	37,7
	or	eax,eax
	jz	.mv
	add	[ecx+6],ax
	shr	eax,16
	add	[ecx+4],ax
	mov	eax,MEV_WHEEL
	ret
  .mv:	mov	eax,MEV_MOVE
	ret
    @@: mov	bh,al
	and	ebx,0x0101
	cmp	bl,bh
	je	.rb
	test	al,1
	jz	@f
	mov	eax,MEV_LDOWN
	ret
    @@: mov	eax,MEV_LUP
	ret
  .rb:	test	al,2
	jz	@f
	mov	eax,MEV_RDOWN
	ret
    @@: mov	eax,MEV_RUP
	ret
endp

mouse_ev dd mouse.l_down,mouse.l_up,mouse.r_down,mouse.r_up,mouse.wheel,mouse.move

mouse:

	mcall	9,p_info,-1
	
	mov	eax,[p_info+70] ;status of window
	test	eax,100b
	jne	still.skip_write	
	
	mov	ecx,mst
	call	get_mouse_event
	cmp	[bot_mode],0
	je	@f
	mov	ah,al
	mov	al,4
	call	[bot_dlg_handler]
	jmp	still.skip_write
    @@: cmp	al,MEV_MOVE
	jne	.no_move
	cmp	[popup_active],1
	je	@f
  .no_move:
	mov	[s_status],0

	push	eax
	mcall	9,p_info,-1
	cmp	ax,[p_info.window_stack_position]
	pop	eax
	jne	still.skip_write
    @@:
;!!!        cmp     [just_from_popup],0
;!!!        je      @f
;!!!        cmp     al,MEV_LUP
;!!!        jne     still.skip_write
    @@: mov	[mev],al
	jmp	[mouse_ev+eax*4-4]

  .wheel:
	movsx	eax,word[mst+4]
	lea	eax,[eax*3]
	add	[cur_editor.TopLeft.X],eax
	movsx	eax,word[mst+6]
	lea	eax,[eax*3]
	add	[cur_editor.TopLeft.Y],eax
	xor	eax,eax
	mov	[mst+4],eax
	call	check_bottom_right
	call	draw_editor
	jmp	still.skip_write

  .move:
	mcall	37,1
	movsx	ebx,ax
	sar	eax,16
	cmp	[body_capt],0
	jge	.check_body.2
	cmp	[vscrl_capt],0
	jge	.check_vscroll.2
	cmp	[hscrl_capt],0
	jge	.check_hscroll.2

	cmp	[do_not_draw],0
	jne	still.skip_write
	mov	eax,[mi_cur]
	call	get_active_menu_item
	cmp	eax,[mi_cur]
	je	still.skip_write
	push	[mi_cur]
	cmp	[popup_active],0
	je	@f
	mov	[mi_cur],eax
    @@: call	draw_main_menu
	pop	[mi_cur]
	cmp	[popup_active],0
	je	still.skip_write
	mov	ecx,[mi_cur]
	or	ecx,ecx
	js	still.skip_write
	mov	eax,[main_menu.popups+ecx*4-4]
	mov	edx,main_menu
	call	dword[main_menu.onshow+ecx*4-4]
	call	setup_main_menu_popup
	mcall	60,2,[h_popup],POPUP_STACK,4
	
	call draw_main_menu

	jmp	still.skip_write


  .r_down:
    @@: cmp	[popup_active],0
	je	@f
	mcall	5,1
	jmp	@b
    @@: cmp	[mouse_captured],0
	jne	still.skip_write
	call	check_mouse_in_edit_area
	jnc	still.skip_write
	mcall	37,0
	mov	[mm.Edit+POPUP.pos],eax
    @@: mcall	37,2
	cmp	eax,ebx
	jnz	@f
	mcall	5,1
	jmp	@b
    @@: and	[mst],0xFD
	call	onshow.edit
	mov	dword[POPUP_STACK],mm.Edit
	mcall	51,1,popup_thread_start,POPUP_STACK
	mov	[h_popup],eax
	jmp	still.skip_write

  .r_up:
	jmp	still.skip_write

  .l_down:
	call	check_mouse_in_edit_area
	jnc	.check_vscroll
	mov	[mouse_captured],1
	mov	[body_capt],1

	call	clear_selection

  .check_body.2:
	sub	eax,[cur_editor.Bounds.Left]
	sub	ebx,[cur_editor.Bounds.Top]
	sub	eax,[cur_editor.Gutter.Width]
	sub	eax,LCHGW
	sub	ebx,2
	push	eax
	mov	eax,ebx
	cdq
	mov	ecx,LINEH
	idiv	ecx
    @@: add	eax,[cur_editor.TopLeft.Y]
	mov	ebx,eax
	pop	eax
	cdq
	mov	ecx,6
	idiv	ecx
    @@: add	eax,[cur_editor.TopLeft.X]

	cmp	eax,[cur_editor.Columns.Count]
	jl	@f
	mov	eax,[cur_editor.Columns.Count]
    @@: cmp	ebx,[cur_editor.Lines.Count]
	jl	@f
	mov	ebx,[cur_editor.Lines.Count]
	dec	ebx
    @@:
	cmp	[cur_editor.Caret.X],eax
	jne	.change_cur_pos
	cmp	[cur_editor.Caret.Y],ebx
	jne	.change_cur_pos
	call	editor_check_for_changes
	jmp	still.skip_write

  .change_cur_pos:
	mov	[cur_editor.Caret.X],eax
	mov	[cur_editor.Caret.Y],ebx
	call	editor_check_for_changes
	jmp	still

  .check_vscroll:
	mov	ecx,[cur_editor.Bounds.Right]
	sub	ecx,SCRLW-1
	pushd	ecx [cur_editor.Bounds.Top] ecx [cur_editor.Bounds.Bottom]
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	add	[__rc+0x8],SCRLW-2
	add	[__rc+0x4],SCRLW-1
	sub	[__rc+0xC],SCRLW*2+1
	mov	ecx,__rc
	call	pt_in_rect
	jnc	.check_hscroll

  .check_vscroll.2:
	sub	ebx,[cur_editor.Bounds.Top]
	sub	ebx,SCRLW
	cmp	[vscrl_capt],0
	jge	.vcaptured
	mov	eax,[cur_editor.VScroll.Top]
	cmp	ebx,eax
	jb	.center_vcapture
	add	eax,[cur_editor.VScroll.Size]
	cmp	ebx,eax
	jae	.center_vcapture
	mov	eax,ebx
	sub	eax,[cur_editor.VScroll.Top]
	dec	eax
	mov	[vscrl_capt],eax
	dec	ebx
	jmp	.vcaptured
  .center_vcapture:
	mov	eax,[cur_editor.VScroll.Size]
	shr	eax,1
	mov	[vscrl_capt],eax
  .vcaptured:
	sub	ebx,[vscrl_capt]
	jns	@f
	xor	ebx,ebx
    @@: mov	[mouse_captured],1
	mov	eax,[cur_editor.Bounds.Bottom]
	sub	eax,[cur_editor.Bounds.Top]
	sub	eax,[cur_editor.VScroll.Size]
	sub	eax,SCRLW*3
	cmp	eax,ebx
	jge	@f
	mov	ebx,eax
    @@:
	mov	[cur_editor.VScroll.Top],ebx
	mov	eax,[cur_editor.Lines.Count]
	sub	eax,[lines.scr]
	imul	ebx
	mov	ebx,[cur_editor.Bounds.Bottom]
	sub	ebx,[cur_editor.Bounds.Top]
	sub	ebx,SCRLW*3
	sub	ebx,[cur_editor.VScroll.Size]
	idiv	ebx
	cmp	eax,[cur_editor.TopLeft.Y]
	je	still.skip_write
	mov	[cur_editor.TopLeft.Y],eax
	call	check_bottom_right
	call	draw_editor
	jmp	still.skip_write

  .check_hscroll:
	pushd	[cur_editor.Bounds.Left] [cur_editor.Bounds.Bottom] [cur_editor.Bounds.Right] [cur_editor.Bounds.Bottom]
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
	add	[__rc+0x8],-SCRLW*2-1
	add	[__rc+0x4],-SCRLW+1
	add	[__rc+0xC],-1
	add	[__rc+0x0],SCRLW+1
	mov	ecx,__rc
	call	pt_in_rect
	jnc	.check_main_menu

  .check_hscroll.2:
	mov	ebx,eax
	sub	ebx,SCRLW+1
	sub	ebx,[cur_editor.Bounds.Left]
	cmp	[hscrl_capt],0
	jge	.hcaptured
	mov	eax,[cur_editor.HScroll.Top]
	cmp	ebx,eax
	jl	.center_hcapture
	add	eax,[cur_editor.HScroll.Size]
	cmp	ebx,eax
	jge	.center_hcapture
	mov	eax,ebx
	sub	eax,[cur_editor.HScroll.Top]
	dec	eax
	mov	[hscrl_capt],eax
	dec	ebx
	jmp	.hcaptured
  .center_hcapture:
	mov	eax,[cur_editor.HScroll.Size]
	shr	eax,1
	mov	[hscrl_capt],eax
  .hcaptured:
	sub	ebx,[hscrl_capt]
	jns	@f
	xor	ebx,ebx
    @@: mov	[mouse_captured],1
	mov	eax,[cur_editor.Bounds.Right]
	sub	eax,[cur_editor.HScroll.Size]
	sub	eax,SCRLW*3+1
	cmp	eax,ebx
	jge	@f
	mov	ebx,eax
    @@:
	mov	[cur_editor.HScroll.Top],ebx
	mov	eax,[cur_editor.Columns.Count]
	sub	eax,[columns.scr]
	imul	ebx
	mov	ebx,[cur_editor.Bounds.Right]
	sub	ebx,SCRLW*3+1
	sub	ebx,[cur_editor.HScroll.Size]
	idiv	ebx
	cmp	eax,[cur_editor.TopLeft.X]
	je	still.skip_write
	mov	[cur_editor.TopLeft.X],eax
	call	check_bottom_right
	call	draw_editor
	jmp	still.skip_write

  .check_main_menu:
	cmp	[do_not_draw],0
	jne	.capture_off

    @@: mcall	37,2
	test	eax,0x01
	jz	@f
	mcall	5,1
	jmp	@b
    @@: and	[mst],0xFE

	cmp	[mi_cur],0
	jle	.capture_off
	mov	ecx,[mi_cur]
	mov	eax,[main_menu.popups+ecx*4-4]
	mov	edx,main_menu
	call	dword[main_menu.onshow+ecx*4-4]
	call	setup_main_menu_popup
	mcall	51,1,popup_thread_start,POPUP_STACK
	mov	[h_popup],eax

  .l_up:
  .capture_off:
	or	eax,-1
	mov	[vscrl_capt],eax
	mov	[hscrl_capt],eax
	mov	[body_capt],eax
	mov	[mouse_captured],0
;!!!        mov     [just_from_popup],0
	jmp	still.skip_write
