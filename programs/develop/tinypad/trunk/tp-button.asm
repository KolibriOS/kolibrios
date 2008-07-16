diff16 'tp-button.asm',0,$

button:
	mcall	17
	cmp	al,0
	jne	still
	shr	eax,8

	cmp	[bot_mode],0
	je	@f
	mov	ebx,eax
	mov	al,3
	call	[bot_dlg_handler]
	jmp	still

    @@: mov	esi,accel_table2
  .acc: cmp	eax,[esi]
	jne	@f
	call	dword[esi+4]
	jmp	still.skip_write
    @@: add	esi,8
	cmp	byte[esi],0
	jne	.acc

	cmp	eax,[tab_bar.Buttons.First]
	jb	@f
	cmp	eax,[tab_bar.Buttons.Last]
	ja	@f

	;// TAB CONTROL BUTTONS

	add	eax,-1000
	imul	ebp,eax,sizeof.TABITEM
	add	ebp,[tab_bar.Items]
	cmp	ebp,[tab_bar.Current.Ptr]
	je	@f
	call	set_cur_tab
	call	align_editor_in_tab
	call	draw_editor
	call	draw_statusbar
	call	draw_tabctl
	call	update_caption

    @@:
	jmp	still.skip_write

  btn.vscroll_up:
	dec	[cur_editor.TopLeft.Y]
	jns	@f
	inc	[cur_editor.TopLeft.Y]
	ret
    @@: call	editor_check_for_changes.direct
	ret

  btn.vscroll_down:
	inc	[cur_editor.TopLeft.Y]
	mov	eax,[cur_editor.Lines.Count]
	sub	eax,[lines.scr]
	cmp	eax,[cur_editor.TopLeft.Y]
	jge	@f
	dec	[cur_editor.TopLeft.Y]
	ret
    @@: call	editor_check_for_changes.direct
	ret

  btn.hscroll_up:
	dec	[cur_editor.TopLeft.X]
	jns	@f
	inc	[cur_editor.TopLeft.X]
	ret
    @@: call	editor_check_for_changes.direct
	ret

  btn.hscroll_down:
	inc	[cur_editor.TopLeft.X]
	mov	eax,[cur_editor.Columns.Count]
	sub	eax,[columns.scr]
	cmp	eax,[cur_editor.TopLeft.X]
	jge	@f
	dec	[cur_editor.TopLeft.X]
	ret
    @@: call	editor_check_for_changes.direct
	ret

  btn.tabctl_right:
	call	get_hidden_tabitems_number
	or	eax,eax
	jz	@f
	inc	[tab_bar.Items.Left]
	call	draw_tabctl
    @@: ret
  btn.tabctl_left:
	dec	[tab_bar.Items.Left]
	jns	@f
	inc	[tab_bar.Items.Left]
    @@: call	draw_tabctl
	ret

  btn.debug_board:
	call	open_debug_board
	ret
  btn.sysfuncs_txt:
	call	open_sysfuncs_txt
	ret

proc search
	cld
	mov	ecx,[cur_editor.Caret.Y]
	mov	edx,ecx
	call	get_line_offset
	cmp	word[esi],0
	je	.exit
	call	get_real_length
	add	esi,4
	or	eax,eax
	jz	.end_line.2
	mov	ecx,eax
	sub	ecx,[cur_editor.Caret.X]
	push	esi
	add	esi,[cur_editor.Caret.X]
	jmp	@f

  .next_line:
	push	esi
    @@: sub	ecx,[s_search.size]
	inc	ecx

  .next_char:
	dec	ecx
	js	.end_line
	xor	edi,edi

  .next_ok:
	movzx	eax,byte[edi+esi]
	movzx	ebx,byte[edi+s_search]

	cmp	al,$61
	jb	@f
	add	al,[eax+add_table-$61]
    @@: cmp	bl,$61
	jb	@f
	add	bl,[ebx+add_table-$61]
    @@:
	cmp	al,bl
	je	@f

	inc	esi
	jmp	.next_char
    @@:
	inc	edi
	cmp	edi,[s_search.size]
	jne	.next_ok

  .found:
	add	esp,4
	mov	[cur_editor.Caret.Y],edx
	mov	[cur_editor.SelStart.Y],edx
	mov	ecx,edx
	lea	eax,[esi-4]
	call	get_line_offset
	sub	eax,esi
	mov	[cur_editor.SelStart.X],eax
	add	eax,[s_search.size]
	mov	[cur_editor.Caret.X],eax
	mov	[s_status],0
	clc
	ret

  .end_line:
	pop	esi
  .end_line.2:
	movzx	eax,word[esi-4]
	add	esi,eax
	inc	edx
	call	get_real_length
	mov	ecx,eax
	lodsd
	or	eax,eax
	jnz	.next_line
  .exit:
	mov	[s_status],s_text_not_found
	stc
	ret
endp
