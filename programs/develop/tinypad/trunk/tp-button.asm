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

	jmp	still.skip_write

  btn.scroll_up:
	dec	[cur_tab.Editor.TopLeft.Y] ;! [top_line]
	jns	@f
	inc	[cur_tab.Editor.TopLeft.Y] ;! [top_line]
	ret
    @@: call	check_inv_all.skip_check
	ret

  btn.scroll_down:
	inc	[cur_tab.Editor.TopLeft.Y] ;! [top_line]
	mov	eax,[cur_tab.Editor.Lines] ;! eax,[lines]
	sub	eax,[lines.scr]
	cmp	eax,[cur_tab.Editor.TopLeft.Y] ;! eax,[top_line]
	jge	@f
	dec	[cur_tab.Editor.TopLeft.Y] ;! [top_line]
	ret
    @@: call	check_inv_all.skip_check
	ret

  btn.scroll_left:
	dec	[cur_tab.Editor.TopLeft.X] ;! [left_col]
	jns	@f
	inc	[cur_tab.Editor.TopLeft.X] ;! [left_col]
	ret;jmp     still.skip_write
    @@: call	check_inv_all.skip_check
	ret

  btn.scroll_right:
	inc	[cur_tab.Editor.TopLeft.X] ;! [left_col]
	mov	eax,[cur_tab.Editor.Columns] ;! eax,[columns]
	sub	eax,[columns.scr]
	cmp	eax,[cur_tab.Editor.TopLeft.X] ;! eax,[left_col]
	jge	@f
	dec	[cur_tab.Editor.TopLeft.X] ;! [left_col]
	ret
    @@: call	check_inv_all.skip_check
	ret

  btn.search:
  key.f3:
	call	search
	jc	@f
	call	check_inv_all
    @@: ret


func search
	cld
	mov	ecx,[cur_tab.Editor.Caret.Y] ;! ecx,[pos.y]
	mov	edx,ecx
	call	get_line_offset
	cmp	word[esi],0
	je	.exit
	call	get_real_length
	add	esi,4
	or	eax,eax
	jz	.end_line.2
	mov	ecx,eax
	sub	ecx,[cur_tab.Editor.Caret.X] ;! ecx,[pos.x]
	push	esi
	add	esi,[cur_tab.Editor.Caret.X] ;! esi,[pos.x]
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
	mov	[cur_tab.Editor.Caret.Y],edx ;! [pos.y],edx
	mov	[cur_tab.Editor.SelStart.Y],edx ;! [sel.y],edx
	mov	ecx,edx
	lea	eax,[esi-4]
	call	get_line_offset
	sub	eax,esi
	mov	[cur_tab.Editor.SelStart.X],eax ;! [sel.x],eax
	add	eax,[s_search.size]
	mov	[cur_tab.Editor.Caret.X],eax ;! [pos.x],eax
	mov	[s_status],0
	clc
	ret

  .end_line:
	pop	esi
  .end_line.2:
	movzx	eax,word[esi-4]
	add	esi,eax;[esi-4]
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
endf

  btn.compile:
  key.ctrl_f9:
	mov	bl,0
	call	start_fasm
	ret
  btn.compile_run:
  key.f9:
	mov	bl,1
	call	start_fasm
	ret
  btn.debug_board:
	call	open_debug_board
	ret
  btn.sysfuncs_txt:
	call	open_sysfuncs_txt
	ret

  btn.load_file:
  key.ctrl_l:
	call	load_file
	jnc	@f
	ret
    @@:

	xor	eax,eax
	mov	[cur_tab.Editor.TopLeft.Y],eax ;! [top_line],eax
	mov	[cur_tab.Editor.TopLeft.X],eax ;! [left_col],eax
	mov	[cur_tab.Editor.Caret.X],eax ;! [pos.x],eax
	mov	[cur_tab.Editor.Caret.Y],eax ;! [pos.y],eax
	mov	[cur_tab.Editor.SelStart.X],eax ;! [sel.x],eax
	mov	[cur_tab.Editor.SelStart.Y],eax ;! [sel.y],eax

	mov	[cur_tab.Editor.Modified],al ;! [modified],al

; enable color syntax for ASM and INC files:
	mov	[cur_tab.Editor.AsmMode],al ;! [asm_mode],al

	mov	eax,[f_info.length]
	add	eax,f_info.path
	mov	byte[eax],0
	mov	ecx, dword [eax-3]
	or	ecx, 0x202020
	cmp	ecx, 'asm'
	jne	@f
	inc	[cur_tab.Editor.AsmMode] ;! [asm_mode]
	jmp	.nocol
    @@: cmp	ecx, 'inc'
	jne	.nocol
	inc	[cur_tab.Editor.AsmMode] ;! [asm_mode]
    .nocol:

  update_caption:
	lea	esi,[cur_tab.Editor.FilePath] ;! mov     esi,f_info.path
	mov	edi,s_title

    @@: lodsb
	cmp	al,0
	je	@f
	stosb
	jmp	@b
    @@:
	;cld
	;mov     ecx,[f_info.length]
	;jecxz   @f
	;rep     movsb

	mov	dword[edi],' - '
	add	edi,3
    @@: mov	esi,htext
	mov	ecx,htext.size
	cld
	rep	movsb

	mov	al,0
	stosb

	mcall	71,1,s_title

	clc
	ret

  btn.close_main_window:
  key.alt_x:
	mov	esi,self_path
	mov	byte[esi+PATHL-1],0
	mov	edi,f_info.path
	cld
    @@: lodsb
	stosb
	or	al,al
	jnz	@b

	mov	[f_info70+0],2
	mov	[f_info70+4],0
	mov	[f_info70+8],0
	mov	[f_info70+12],TINYPAD_END
	mov	[f_info70+16],0
	mov	byte[f_info70+20],0
	mov	[f_info70+21],f_info.path
	mcall	70,f_info70

  .close:
	mov	[main_closed],1
	mcall	-1
