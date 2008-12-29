diff16 'tp-key.asm',0,$

key:
	mov	ecx,1
	mcall	66,3
	mov	[shi],eax
	xor	ebx,ebx
	test	al,0x03
	jz	@f
	or	ebx,KM_SHIFT
    @@: test	al,0x0C
	jz	@f
	or	ebx,KM_CTRL
    @@: test	al,0x30
	jz	@f
	or	ebx,KM_ALT
    @@: mov	edx,ebx
	test	al,0x03
	jz	@f
	inc	cl
    @@:
	mcall	2
	cmp	al,0
	jne	still.skip_write
	shr	eax,8
	cmp	al,224
	jne	@f
	mov	[ext],0x01
	jmp	still.skip_write
    @@: cmp	al,225
	jne	@f
	mov	[ext],0x02
	jmp	still.skip_write
    @@:
	mov	ah,[ext]
	mov	[ext],0

	mov	esi,numpad_table_off
	test	[shi], 0x00000080 ; NumLock is on?
	jz	.num
	mov	esi,numpad_table_on
  .num: cmp	eax,[esi]
	jne	@f
	mov	eax,[esi+4]
	mov	ebx,eax
	or	eax,edx
	shr	ebx,8
	or	ebx,0x0000FFFF
	and	eax,ebx
	mov	ecx,eax
	shr	ecx,16
	and	cl,1
	inc	cl
	jmp	.lp0
    @@: add	esi,8
	cmp	dword[esi],0
	jne	.num

	or	eax,edx

  .lp0: test	al,0x80
	jnz	still.skip_write

	push	eax
	mcall	26,2,,key1
	pop	eax

	mov	[chr],eax

	cmp	[bot_mode],0
	je	@f
	mov	ebx,eax
	mov	al,2
	call	[bot_dlg_handler]
	jmp	still
    @@:

	mov	esi,accel_table_main
  .acc: cmp	eax,[esi]
	jne	@f
	test	[secure_sel],1
	jz	.lp1
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
  .lp1: mov	[s_status],0
	call	dword[esi+4]
	call	editor_check_for_changes
	jmp	still
    @@: add	esi,8
	cmp	dword[esi],0
	jne	.acc

	test	[chr],KM_CTRLALT
	jnz	still.skip_write

	mov	[s_status],0

	movzx	eax,byte[chr]
	movzx	eax,[eax+key0]
	or	al,al
	jz	still.skip_write
	movzx	eax,[eax+key1]
	push	eax

	test	[secure_sel],1
	jz	.lp2
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
	jmp	.put
  .lp2: call	delete_selection

	test	[auto_braces],1
	jz	.put
	cmp	al,'['
	jne	@f
	mov	al,']'
	call	.lp3
	dec	[cur_editor.Caret.X]
	jmp	.put
    @@: cmp	al,'('
	jne	@f
	mov	al,')'
	call	.lp3
	dec	[cur_editor.Caret.X]
	jmp	.put
    @@: cmp	al,'{'
	jne	.put
	mov	al,'}'
	call	.lp3
	dec	[cur_editor.Caret.X]

  .put: pop	eax
	push	still editor_check_for_changes
	inc	[cur_editor.SelStart.X]
  .lp3: push	[cur_editor.Caret.X] eax
	inc	dword[esp+4]
	mov	eax,1
	jmp	key.tab.direct

;-----------------------------------------------------------------------------
proc key.ctrl_a ;///// SELECT ALL DOCUMENT ///////////////////////////////////
;-----------------------------------------------------------------------------
	xor	eax,eax
	mov	[cur_editor.SelStart.X],eax
	mov	[cur_editor.SelStart.Y],eax
	mov	ecx,[cur_editor.Lines.Count]
	dec	ecx
	mov	[cur_editor.Caret.Y],ecx
	call	get_line_offset
	call	get_real_length
	mov	[cur_editor.Caret.X],eax
	call	draw_editor
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_o ;///// ENTER OPEN FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[bot_mode2],0

  .direct:
	cmp	[bot_mode2], 2
	je	.ask
	mov	[s_status],s_enter_filename
	jmp	.ask1

   .ask:
	mov	[s_status],s_ask_save
  .ask1:
	mov	[bot_mode],1
	mov	[bot_dlg_height],16*2+4*2-1
	mov	[bot_dlg_handler],osdlg_handler
	mov	[focused_tb],tb_opensave

    @@: mov	al,[tb_opensave.length]
	mov	[tb_opensave.pos.x],al
	mov	[tb_opensave.sel.x],0
	mov	[tb_casesen],0;1
	call	drawwindow
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_s ;///// ENTER SAVE FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[cur_editor.FilePath], 0
	je	key.shift_ctrl_s
	cmp	[cur_editor.Modified],0
	je	.exit
	call	save_file
	call	drawwindow
  .exit:
	ret

    key.shift_ctrl_s:
	mov	[bot_mode2],1
	jmp	key.ctrl_o.direct
endp

;-----------------------------------------------------------------------------
proc key.ctrl_n ;///// CREATE NEW FILE (TAB) /////////////////////////////////
;-----------------------------------------------------------------------------
	call	create_tab
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_f ;///// ENTER KEYWORD TO FIND /////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[bot_mode2],0
	mov	[bot_dlg_height],16*2+4*2-1

	mov	[s_status],s_enter_text_to_find

  .direct:
	mov	[bot_mode],1
	mov	[bot_dlg_handler],finddlg_handler
	mov	[focused_tb],tb_find

	mov	ecx,[s_search.size]
	mov	[tb_find.length],cl
	jecxz	@f
	mov	esi,s_search
	mov	edi,tb_find.text
	cld
	rep	movsb

    @@: mov	al,[tb_find.length]
	mov	[tb_find.pos.x],al
	mov	[tb_find.sel.x],0
	mov	[tb_casesen],0
	call	drawwindow
	ret
endp

proc key.ctrl_h
	mov	[bot_mode2],1
	mov	[bot_dlg_height],16*3+4*2+1

	mov	[s_status],s_enter_text_to_replace

	jmp	key.ctrl_f.direct
endp

proc key.ctrl_g
	ret
@^
	mov	[bot_mode2],0
	mov	[bot_dlg_height],16*2+4*2-1

	mov	[bot_mode],1
	mov	[bot_dlg_handler],gotodlg_handler
	mov	[focused_tb],tb_gotorow

	mov	al,[tb_gotorow.length]
	mov	[tb_gotorow.pos.x],al
	mov	[tb_gotorow.sel.x],0
	mov	[tb_casesen],0
	call	drawwindow
	ret
^@
endp

;-----------------------------------------------------------------------------
proc key.ctrl_left ;///// GO TO PREVIOUS WORD ////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_left: ;///// GO TO PREVIOUS WORD, WITH SELECTION /////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Caret.Y]
	mov	edx,[cur_editor.Caret.X]
	cld
	mov	ecx,ebx
	call	get_line_offset
  .lp1: cmp	edx,[esi+EDITOR_LINE_DATA.Size]
	jle	@f
	mov	edx,[esi+EDITOR_LINE_DATA.Size]
    @@: dec	edx
	jl	.nx1
	add	esi,sizeof.EDITOR_LINE_DATA
	add	esi,edx
	mov	ecx,edx
    @@: push	ecx
	mov	edi,symbols_ex
	mov	ecx,symbols_ex.size+symbols.size
	mov	al,[esi]
	dec	esi
	repne	scasb
	pop	ecx
	jne	@f
	dec	edx
	dec	ecx
	jnz	@b
  .nx1: dec	ebx
	js	.exit.2
	mov	ecx,ebx
	call	get_line_offset
	mov	edx,[esi+EDITOR_LINE_DATA.Size]
	dec	edx
	jmp	.lp1
    @@:
	mov	ecx,ebx
	call	get_line_offset
  .lp2: cmp	edx,[esi+EDITOR_LINE_DATA.Size]
	jle	@f
	mov	edx,[esi+EDITOR_LINE_DATA.Size]
    @@: or	edx,edx
	jl	.nx2
	add	esi,sizeof.EDITOR_LINE_DATA
	add	esi,edx
    @@: mov	edi,symbols_ex
	mov	ecx,symbols_ex.size+symbols.size
	mov	al,[esi]
	dec	esi
	repne	scasb
	je	@f
	dec	edx
	jns	@b
	jmp	@f
  .nx2: dec	ebx
	js	.exit.2
	mov	ecx,ebx
	call	get_line_offset
	mov	edx,[esi+EDITOR_LINE_DATA.Size]
	dec	edx
	jmp	.lp2
    @@:
	inc	edx
	mov	[cur_editor.Caret.Y],ebx
	mov	[cur_editor.Caret.X],edx
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],ebx
	mov	[cur_editor.SelStart.X],edx
    @@: sub	ebx,[cur_editor.TopLeft.Y]
	jge	@f
	add	[cur_editor.TopLeft.Y],ebx
    @@: mov	eax,edx
	sub	eax,[cur_editor.TopLeft.X]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[cur_editor.TopLeft.X],eax
	jmp	.exit
    @@: cmp	edx,[cur_editor.TopLeft.X]
	jge	.exit
	mov	[cur_editor.TopLeft.X],edx
  .exit:
	call	editor_check_for_changes
  .exit.2:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_right ;///// GO TO NEXT WORD ///////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_right: ;///// GO TO NEXT WORD, WITH SELECTION ////////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Caret.Y]
	mov	edx,[cur_editor.Caret.X]
	cld
  .lp1: mov	ecx,ebx
	call	get_line_offset
	mov	ecx,[esi+EDITOR_LINE_DATA.Size]
	cmp	edx,ecx
	jge	.nx1
	add	esi,sizeof.EDITOR_LINE_DATA
	add	esi,edx
	sub	ecx,edx
    @@: push	ecx
	mov	edi,symbols_ex
	mov	ecx,symbols_ex.size+symbols.size
	lodsb
	repne	scasb
	pop	ecx
	je	@f
	inc	edx
	dec	ecx
	jnz	@b
  .nx1: inc	ebx
	cmp	ebx,[cur_editor.Lines.Count]
	jge	.exit.2
	xor	edx,edx
	jmp	.lp1
    @@:

  .lp2: mov	ecx,ebx
	call	get_line_offset
	mov	ecx,[esi+EDITOR_LINE_DATA.Size]
	cmp	edx,ecx
	jge	.nx2
	add	esi,sizeof.EDITOR_LINE_DATA
	add	esi,edx
	sub	ecx,edx
    @@: push	ecx
	mov	edi,symbols_ex
	mov	ecx,symbols_ex.size+symbols.size
	lodsb
	repne	scasb
	pop	ecx
	jne	@f
	inc	edx
	dec	ecx
	jnz	@b
  .nx2: inc	ebx
	cmp	ebx,[cur_editor.Lines.Count]
	jge	.exit.2
	xor	edx,edx
	jmp	.lp2
    @@:
	mov	[cur_editor.Caret.Y],ebx
	mov	[cur_editor.Caret.X],edx
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],ebx
	mov	[cur_editor.SelStart.X],edx
    @@: sub	ebx,[cur_editor.TopLeft.Y]
	cmp	ebx,[lines.scr]
	jl	@f
	sub	ebx,[lines.scr]
	inc	ebx
	add	[cur_editor.TopLeft.Y],ebx
    @@: mov	eax,edx
	sub	eax,[cur_editor.TopLeft.X]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[cur_editor.TopLeft.X],eax
	jmp	.exit
    @@: cmp	edx,[cur_editor.TopLeft.X]
	jge	.exit
	mov	[cur_editor.TopLeft.X],edx
  .exit:
	call	editor_check_for_changes
  .exit.2:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_x
	cmp	[sel.selected],0
	je	@f
	call	key.ctrl_c
	call	key.del
	mov	[cur_editor.Modified],1
    @@: ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_c
	mov	[copy_size],0
	cmp	[sel.selected],0
	je	.exit

	call	get_selection_size
	stdcall mem.ReAlloc,[copy_buf],eax
	mov	[copy_buf],eax

	cld
	mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.single_line
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	inc	ecx
	push	ecx
	mov	edi,[copy_buf]
	call	get_real_length
	sub	eax,[sel.begin.x]
	jge	@f
	xor	eax,eax
    @@: add	esi,[sel.begin.x]
	add	esi,sizeof.EDITOR_LINE_DATA
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	ecx,eax
	jecxz	@f
	rep	movsb
    @@: mov	ecx,[sel.end.y]
	call	get_line_offset
	pop	ecx
	cmp	ecx,[sel.end.y]
	je	@f
	mov	eax,esi
	call	get_line_offset
	sub	eax,esi
	mov	ecx,eax
	rep	movsb
	mov	ecx,[sel.end.y]
    @@: call	get_line_offset
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	add	esi,sizeof.EDITOR_LINE_DATA
	cmp	eax,[sel.end.x]
	jle	@f
	mov	eax,[sel.end.x]
    @@: mov	ebx,edi
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	ecx,eax
	jecxz	@f
	rep	movsb
	sub	eax,[sel.end.x]
	jz	@f
	neg	eax
	mov	ecx,eax
	add	[ebx],eax
	mov	al,' '
	rep	stosb
    @@: sub	edi,[copy_buf]
	mov	[copy_size],edi
	mov	eax,[sel.end.y]
	sub	eax,[sel.begin.y]
	inc	eax
	mov	[copy_count],eax
  .exit:
	ret

  .single_line:
	mov	eax,[sel.end.x]
	sub	eax,[sel.begin.x]
	mov	edi,[copy_buf]
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	mov	ebx,[sel.begin.x]
	mov	ecx,[sel.end.x]
	cmp	ebx,[esi+EDITOR_LINE_DATA.Size]
	jge	.add_spaces
	cmp	ecx,[esi+EDITOR_LINE_DATA.Size]
	jle	.lp1
	mov	ecx,[esi+EDITOR_LINE_DATA.Size]
  .lp1: sub	ecx,[sel.begin.x]
	sub	eax,ecx
	lea	esi,[esi+ebx+sizeof.EDITOR_LINE_DATA]
	rep	movsb

  .add_spaces:
	mov	ecx,eax
	mov	al,' '
	jecxz	@b
	rep	stosb
	jmp	@b
endp

;-----------------------------------------------------------------------------
proc key.ctrl_v
	cmp	[copy_size],0
	je	.exit

	call	delete_selection

	mov	eax,[copy_size]
	call	editor_realloc_lines

	mov	ebx,[cur_editor.Lines.Size]
	add	ebx,[copy_size]
	mov	[cur_editor.Lines.Size],ebx
	stdcall mem.ReAlloc,[cur_editor.Lines],ebx
	mov	[cur_editor.Lines],eax

	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	pushd	[esi+EDITOR_LINE_DATA.Size] esi
	mov	ecx,[cur_editor.Caret.X]
	call	line_add_spaces
	add	[esp],eax
	add	esi,eax
	mov	ecx,[copy_size]
	sub	ecx,sizeof.EDITOR_LINE_DATA
	mov	edi,[cur_editor.Lines]
	add	edi,[cur_editor.Lines.Size] ;*** add edi,[edi-4]
	dec	edi
	mov	eax,esi
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[eax+sizeof.EDITOR_LINE_DATA]
	add	ecx,[cur_editor.Caret.X]
	neg	ecx
	lea	ecx,[esi+ecx+1]
	std
	rep	movsb

	mov	ecx,[copy_count]
	dec	ecx
	jz	.single_line

	cld
	pop	edi
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	esi,[copy_buf]
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	add	esi,sizeof.EDITOR_LINE_DATA

	mov	ebx,[cur_editor.Caret.X]
	add	eax,ebx
	mov	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size],eax
	mov	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	sub	eax,ebx
	call	.check_columns
	add	edi,ebx
    @@: push	ecx
	mov	ecx,eax
	rep	movsb
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	add	esi,sizeof.EDITOR_LINE_DATA
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	mov	[edi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	add	edi,sizeof.EDITOR_LINE_DATA
	pop	ecx
	loop	@b

	pop	ecx
	sub	ecx,ebx
	add	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size],ecx
	call	.check_columns
	mov	ecx,eax
	rep	movsb

	mov	[cur_editor.Caret.X],eax
	mov	[cur_editor.SelStart.X],eax
	mov	eax,[copy_count]
	dec	eax
	add	[cur_editor.Caret.Y],eax
	add	[cur_editor.SelStart.Y],eax
	add	[cur_editor.Lines.Count],eax

	mov	[cur_editor.Modified],1
	jmp	.exit

  .single_line:
	cld
	pop	edi
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	esi,[copy_buf]
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	add	esi,sizeof.EDITOR_LINE_DATA
	add	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size],eax
	and	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Flags],not EDITOR_LINE_FLAG_SAVED
	or	[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	call	.check_columns
	add	edi,[cur_editor.Caret.X]
	add	esp,4
	mov	ecx,eax
	rep	movsb

	add	[cur_editor.Caret.X],eax
	add	[cur_editor.SelStart.X],eax

	mov	[cur_editor.Modified],1

  .exit:
	ret

  .check_columns:
	push	eax
	mov	eax,[edi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size]
	cmp	eax,[cur_editor.Columns.Count]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax
    @@: pop	eax
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_d ;///// INSERT SEPARATOR //////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,90+sizeof.EDITOR_LINE_DATA
	call	editor_realloc_lines

	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	mov	ebx,esi

	mov	ecx,[cur_editor.Lines.Count]
	call	get_line_offset
	lea	edi,[esi+90+sizeof.EDITOR_LINE_DATA]
	lea	ecx,[esi+sizeof.EDITOR_LINE_DATA]
	sub	ecx,ebx
	std
	rep	movsb

	lea	edi,[ebx+sizeof.EDITOR_LINE_DATA+1]
	mov	[ebx+EDITOR_LINE_DATA.Size],90
	mov	al,ASEPC
	mov	ecx,79
	cld
	rep	stosb
	mov	al,' '
	mov	ecx,10
	rep	stosb
	mov	byte[ebx+sizeof.EDITOR_LINE_DATA],';'

	inc	[cur_editor.Lines.Count]
	inc	[cur_editor.Caret.Y]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]

	mov	[cur_editor.Modified],1

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_y ;///// DELETE CURRENT LINE ///////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.Y]
	inc	eax
	cmp	eax,[cur_editor.Lines.Count]
	jge	.exit

	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	mov	edi,esi
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	lea	esi,[esi+eax+sizeof.EDITOR_LINE_DATA]
	push	eax

	dec	[cur_editor.Lines.Count]
	mov	ecx,[cur_editor.Lines]
	add	ecx,[cur_editor.Lines.Size] ;*** add ecx,[ecx-4]
	sub	ecx,esi
	shr	ecx,2
	cld
	rep	movsd

	pop	eax
	add	eax,sizeof.EDITOR_LINE_DATA
	neg	eax
	call	editor_realloc_lines

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]

	mov	[cur_editor.Modified],1

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.up ;///// GO TO PREVIOUS LINE ///////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_up: ;///// GO TO PREVIOUS LINE, WITH SELECTION ////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.Y]
	dec	eax
	jns	@f
	xor	eax,eax
    @@: mov	ecx,[cur_editor.TopLeft.Y]
	cmp	eax,ecx
	jae	@f
	dec	ecx
	jns	@f
	xor	ecx,ecx
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.down ;///// GO TO NEXT LINE /////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_down: ;///// GO TO NEXT LINE, WITH SELECTION //////////////////
;-----------------------------------------------------------------------------

	mov	eax,[cur_editor.Caret.Y]
	inc	eax
	cmp	eax,[cur_editor.Lines.Count]
	jb	@f
	dec	eax
    @@: mov	ecx,[cur_editor.TopLeft.Y]
	mov	edx,eax
	sub	edx,ecx
	cmp	edx,[lines.scr]
	jb	@f
	inc	ecx
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.left ;///// GO TO PREVIOUS CHAR /////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_left: ;///// GO TO PREVIOUS CHAR, WITH SELECTION //////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.X]
	dec	eax
	jns	@f
	inc	eax
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.X],eax
    @@: mov	[cur_editor.Caret.X],eax

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.right ;///// GO TO NEXT CHAR ////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_right: ;///// GO TO NEXT CHAR, WITH SELECTION /////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.X]
	inc	eax
	cmp	eax,[cur_editor.Columns.Count]
	jbe	@f
	dec	eax
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.X],eax
    @@: mov	[cur_editor.Caret.X],eax

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.pgup ;///// GO TO PREVIOUS PAGE /////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_pgup: ;///// GO TO PREVIOUS PAGE, WITH SELECTION //////////////
;-----------------------------------------------------------------------------
	mov	edx,[lines.scr]
	dec	edx
	mov	eax,[cur_editor.Caret.Y]
	mov	ecx,[cur_editor.TopLeft.Y]
	sub	eax,edx
	jns	@f
	xor	eax,eax
    @@: sub	ecx,edx
	jns	@f
	xor	ecx,ecx
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.pgdn ;///// GO TO NEXT PAGE /////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_pgdn: ;///// GO TO NEXT PAGE, WITH SELECTION //////////////////
;-----------------------------------------------------------------------------
	mov	edx,[lines.scr]
	dec	edx
	mov	eax,[cur_editor.Caret.Y]
	mov	ecx,[cur_editor.TopLeft.Y]
	add	eax,edx
	add	ecx,edx
	cmp	eax,[cur_editor.Lines.Count]
	jb	@f
	mov	eax,[cur_editor.Lines.Count]
	dec	eax
    @@: test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.home ;///// GO TO LINE START ////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_home: ;///// GO TO LINE START, WITH SELECTION /////////////////
;-----------------------------------------------------------------------------
	mov	[cur_editor.Caret.X],0
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.X],0
    @@:

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.end ;///// GO TO LINE END ///////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_end: ;///// GO TO LINE END, WITH SELECTION ////////////////////
;-----------------------------------------------------------------------------
	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	call	get_real_length
	mov	[cur_editor.Caret.X],eax
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.X],eax
    @@:

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_home ;///// GO TO PAGE START ///////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_home: ;///// GO TO PAGE START, WITH SELECTION ////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.TopLeft.Y]
	mov	ecx,eax
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_end ;///// GO TO PAGE END //////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_end: ;///// GO TO PAGE END, WITH SELECTION ///////////////
;-----------------------------------------------------------------------------
	mov	ecx,[cur_editor.TopLeft.Y]
	mov	eax,[lines.scr]
	cmp	eax,[cur_editor.Lines.Count]
	jle	@f
	mov	eax,[cur_editor.Lines.Count]
    @@: add	eax,ecx
	dec	eax
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@: mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.TopLeft.Y],ecx

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_pgup ;///// GO TO DOCUMENT START ///////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_pgup: ;///// GO TO DOCUMENT START, WITH SELECTION ////////
;-----------------------------------------------------------------------------
	xor	eax,eax
	mov	[cur_editor.TopLeft.Y],eax
	mov	[cur_editor.Caret.Y],eax
	test	[chr],KM_SHIFT
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax
    @@:

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_pgdn ;///// GO TO DOCUMENT END /////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_pgdn: ;///// GO TO DOCUMENT END, WITH SELECTION //////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Lines.Count]
	mov	[cur_editor.Caret.Y],eax
	sub	eax,[lines.scr]
	jns	@f
	xor	eax,eax
    @@: mov	[cur_editor.TopLeft.Y],eax
	dec	[cur_editor.Caret.Y]
	test	[chr],KM_SHIFT
	jnz	@f
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
    @@:

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.del ;///// DELETE NEXT CHAR OR SELECTION ////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	jnc	.exit

	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	and	[esi+EDITOR_LINE_DATA.Flags],not EDITOR_LINE_FLAG_SAVED
	or	[esi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	lea	ebx,[esi+sizeof.EDITOR_LINE_DATA]
	mov	ebp,esi

	call	get_real_length
	or	eax,eax
	je	.line_up

	mov	ecx,[cur_editor.Caret.X]
	cmp	ecx,eax
	jae	.line_up
	lea	edi,[ebx+ecx]
	neg	ecx
	mov	eax,[ebp+EDITOR_LINE_DATA.Size]
	add	ecx,eax;[ebp]
	repe	scasb
	je	.line_up

	mov	edi,ebx
	mov	ecx,[cur_editor.Caret.X]
	add	edi,ecx
	lea	esi,[edi+1]
	neg	ecx
	mov	eax,[ebp+EDITOR_LINE_DATA.Size]
	add	ecx,eax;[ebp]
	dec	ecx
	rep	movsb
	mov	byte[edi],' '

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
	mov	[cur_editor.Modified],1
	ret

  .line_up:
	mov	eax,[cur_editor.Lines.Count]
	dec	eax
	cmp	eax,[cur_editor.Caret.Y]
	je	.exit
	mov	edi,[temp_buf]
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	esi,ebx
	mov	ecx,[cur_editor.Caret.X]
	rep	movsb
	mov	ecx,[cur_editor.Caret.X]
	mov	eax,[temp_buf]
	mov	[eax+EDITOR_LINE_DATA.Size],ecx
	cmp	ecx,[ebp+EDITOR_LINE_DATA.Size]
	jbe	@f
	mov	eax,[ebp+EDITOR_LINE_DATA.Size]
	sub	ecx,eax
	sub	edi,ecx
	mov	al,' '
	rep	stosb
    @@: lea	esi,[ebx+sizeof.EDITOR_LINE_DATA]
	mov	eax,[ebp+EDITOR_LINE_DATA.Size]
	add	esi,eax
	mov	ecx,[esi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size]
	mov	eax,[temp_buf]
	add	[eax+EDITOR_LINE_DATA.Size],ecx
	or	[eax+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	rep	movsb

	mov	ecx,edi
	sub	ecx,[temp_buf]

	mov	esi,[temp_buf]
	call	get_real_length
	cmp	eax,[cur_editor.Columns.Count]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax
    @@:
	push	ecx
	mov	edi,[cur_editor.Lines]
	add	edi,[cur_editor.Lines.Size] ;*** add edi,[edi-4]
	dec	edi
	lea	esi,[edi+sizeof.EDITOR_LINE_DATA*2] ; !!! CHECK THIS !!!
	sub	esi,ecx
	mov	eax,[ebp+EDITOR_LINE_DATA.Size]
	add	esi,eax
	mov	eax,[ebp+eax+sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size]
	add	esi,eax
	lea	ecx,[esi-sizeof.EDITOR_LINE_DATA]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	jz	.lp1
	mov	edi,ebp
	add	edi,[esp]
	lea	esi,[ebp+sizeof.EDITOR_LINE_DATA*2] ; !!! CHECK THIS !!!
	mov	eax,[esi-sizeof.EDITOR_LINE_DATA*2+EDITOR_LINE_DATA.Size] ; !!! CHECK THIS !!!
	add	esi,eax
	mov	eax,[esi-sizeof.EDITOR_LINE_DATA+EDITOR_LINE_DATA.Size] ; !!! CHECK THIS !!!
	add	esi,eax
	mov	ecx,[cur_editor.Lines]
	add	ecx,[cur_editor.Lines.Size] ;*** add ecx,[ecx-4]
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp1: pop	ecx
	mov	esi,[temp_buf]
	mov	edi,ebp
	cld
	rep	movsb

  .ok.dec.lines:
	dec	[cur_editor.Lines.Count]
	mov	eax,[cur_editor.Lines.Count]
	cmp	[cur_editor.Caret.Y],eax
	jb	@f
	dec	eax
	mov	[cur_editor.Caret.Y],eax
    @@: m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]

	mov	ecx,[cur_editor.Lines.Count]
	call	get_line_offset
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	lea	esi,[esi+eax+sizeof.EDITOR_LINE_DATA]
	mov	eax,[cur_editor.Lines]
	add	eax,[cur_editor.Lines.Size] ;*** add eax,[eax-4]
	sub	esi,eax
	lea	eax,[esi+4096]
	call	editor_realloc_lines

	mov	[cur_editor.Modified],1

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ins ;///// TOGGLE INSERT/OVERWRITE MODE /////////////////////////////
;-----------------------------------------------------------------------------
	xor	[ins_mode],1
	mov	eax,[cur_editor.Caret.Y]
	mov	ebx,eax
	call	draw_editor_text.part
	call	draw_editor_caret
	ret
endp

;-----------------------------------------------------------------------------
proc key.bkspace ;///// DELETE PREVIOUS CHAR OR SELECTION ////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	jnc	key.del.exit

	mov	eax,[cur_editor.Caret.X]
	dec	eax
	js	.line_up

	dec	[cur_editor.Caret.X]
	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	and	[esi+EDITOR_LINE_DATA.Flags],not EDITOR_LINE_FLAG_SAVED
	or	[esi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED

	mov	ebx,eax
	call	get_real_length
	cmp	eax,[cur_editor.Caret.X]
	jae	@f
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	mov	[cur_editor.Modified],1
	ret

    @@: lea	edi,[esi+sizeof.EDITOR_LINE_DATA+ebx]
	mov	ecx,ebx
	neg	ecx
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	add	ecx,eax
	dec	ecx
	lea	esi,[edi+1]
	cld
	rep	movsb
	mov	byte[edi],' '

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
	mov	[cur_editor.Modified],1
	ret

  .line_up:
	cmp	[cur_editor.Caret.Y],0
	jne	@f
	ret
    @@: mov	ecx,[cur_editor.Caret.Y]
	dec	ecx
	call	get_line_offset
	and	[esi+EDITOR_LINE_DATA.Flags],not EDITOR_LINE_FLAG_SAVED
	or	[esi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED

	mov	ebp,esi
	lea	ebx,[esi+sizeof.EDITOR_LINE_DATA]
	mov	ecx,[ebp+EDITOR_LINE_DATA.Size]
    @@: cmp	byte[ebx+ecx-1],' '
	jne	@f
	dec	ecx
	jg	@b
    @@: mov	[cur_editor.Caret.X],ecx
	dec	[cur_editor.Caret.Y]
	cld
	jmp	key.del.line_up
endp

;-----------------------------------------------------------------------------
proc key.tab ;///// TABULATE /////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	mov	eax,[cur_editor.Caret.X]

	mov	ecx,eax
	add	eax,ATABW
	and	eax,not(ATABW-1)
	push	eax ' '
	sub	eax,ecx
  .direct:
	push	eax
	call	editor_realloc_lines
	pop	eax
	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset
	and	[esi+EDITOR_LINE_DATA.Flags],not EDITOR_LINE_FLAG_SAVED
	or	[esi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED

	xchg	eax,ecx

	call	get_real_length
	cmp	eax,[cur_editor.Caret.X]
	jae	@f
	mov	eax,[cur_editor.Caret.X]
    @@: mov	edx,[esi+EDITOR_LINE_DATA.Size]
	sub	edx,eax
	cmp	ecx,edx
	jl	@f
	push	eax
	mov	eax,10
	call	editor_realloc_lines
	add	esi,eax
	pop	eax
	pushad
	mov	ecx,[cur_editor.Lines]
	add	ecx,[cur_editor.Lines.Size] ;*** add ecx,[ecx-4]
	dec	ecx
	mov	edi,ecx
	add	ecx,-10+1
	mov	eax,[esi+EDITOR_LINE_DATA.Size]
	lea	eax,[esi+eax+sizeof.EDITOR_LINE_DATA]
	sub	ecx,eax
	lea	esi,[edi-10]
	std
	rep	movsb
	mov	ecx,10
	mov	al,' '
	rep	stosb
	popad
	add	[esi+EDITOR_LINE_DATA.Size],10
	jmp	@b
    @@: lea	ebx,[esi+sizeof.EDITOR_LINE_DATA]
	push	ecx
	mov	edi,[esi+EDITOR_LINE_DATA.Size]
	lea	edi,[ebx+edi-1]
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[esi+1]
	sub	ecx,ebx
	sub	ecx,[cur_editor.Caret.X]
	std
	rep	movsb
  .ok:	pop	ecx
	pop	eax
	rep	stosb
	cld
	pop	[cur_editor.Caret.X]
	lea	esi,[ebx-sizeof.EDITOR_LINE_DATA]
	call	get_real_length
	cmp	eax,[cur_editor.Caret.X]
	jae	@f
	mov	eax,[cur_editor.Caret.X]
    @@: cmp	eax,[cur_editor.Columns.Count]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax
    @@: m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
	mov	[cur_editor.Modified],1

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.return ;///// CARRIAGE RETURN ///////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection

	mov	eax,14
	call	editor_realloc_lines

	mov	ecx,[cur_editor.Caret.Y]
	call	get_line_offset

	mov	ebx,[cur_editor.Caret.X]
	cmp	ebx,[esi+EDITOR_LINE_DATA.Size]
	jb	@f
	mov	ebx,[esi+EDITOR_LINE_DATA.Size]
	dec	ebx
	jns	@f
	xor	ebx,ebx
    @@:
	cld

	mov	edi,[temp_buf]
	mov	ebp,esi
	lea	ecx,[ebx+1]
    @@: dec	ecx
	jz	@f
	cmp	byte[esi+ecx+sizeof.EDITOR_LINE_DATA-1],' '
	je	@b
    @@: lea	eax,[ecx+10]
	mov	[edi+EDITOR_LINE_DATA.Size],eax
	mov	[edi+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED
	add	edi,sizeof.EDITOR_LINE_DATA
	jecxz	@f
	push	esi
	add	esi,sizeof.EDITOR_LINE_DATA
	rep	movsb
	pop	esi
    @@: mov	al,' '
	mov	ecx,10
	rep	stosb

	mov	ecx,[esi+EDITOR_LINE_DATA.Size]
	sub	ecx,ebx
	add	esi,ebx
	add	esi,sizeof.EDITOR_LINE_DATA
	inc	ecx
    @@: dec	ecx
	jz	@f
	cmp	byte[esi+ecx-1],' '
	je	@b
    @@: jz	.lp1
    @@: cmp	byte[esi],' '
	jne	.lp1
	inc	esi
	loop	@b
  .lp1: test	[auto_indent],1
	jz	.lp2
	push	edi ecx
	mov	ecx,[ebp+EDITOR_LINE_DATA.Size]
	lea	edi,[ebp+sizeof.EDITOR_LINE_DATA]
	mov	al,' '
	repe	scasb
	mov	eax,ecx
	pop	ecx edi
	je	.lp2
	neg	eax
	mov	edx,[ebp+EDITOR_LINE_DATA.Size]
	add	eax,edx;[ebp]
	dec	eax
	jmp	@f
  .lp2: xor	eax,eax
    @@: mov	edx,edi
	add	edi,sizeof.EDITOR_LINE_DATA
	mov	[cur_editor.Caret.X],eax
	jecxz	@f
	push	ecx
	mov	ecx,eax
	mov	al,' '
	rep	stosb
	pop	ecx
    @@: jecxz	@f
	rep	movsb
    @@: mov	ecx,10
	mov	al,' '
	rep	stosb

	lea	eax,[edi-sizeof.EDITOR_LINE_DATA]
	sub	eax,edx
	mov	[edx+EDITOR_LINE_DATA.Size],eax
	mov	[edx+EDITOR_LINE_DATA.Flags],EDITOR_LINE_FLAG_MOFIFIED

	mov	ecx,edi
	sub	ecx,[temp_buf]

	push	ecx
	mov	edi,[cur_editor.Lines]
	add	edi,[cur_editor.Lines.Size] ;*** add edi,[edi-4]
	dec	edi
	lea	esi,[edi+sizeof.EDITOR_LINE_DATA]
	sub	esi,ecx
	mov	ecx,[ebp+EDITOR_LINE_DATA.Size]
	add	esi,ecx
	lea	ecx,[esi-sizeof.EDITOR_LINE_DATA]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	je	.lp3
	lea	esi,[ebp+sizeof.EDITOR_LINE_DATA]
	mov	eax,[esp]
	lea	edi,[esi+eax-sizeof.EDITOR_LINE_DATA]
	mov	ecx,[ebp+EDITOR_LINE_DATA.Size]
	add	esi,ecx
	mov	ecx,[cur_editor.Lines]
	add	ecx,[cur_editor.Lines.Size] ;*** add ecx,[ecx-4]
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp3: pop	ecx
	mov	esi,[temp_buf]
	mov	edi,ebp
	cld
	rep	movsb

	inc	[cur_editor.Caret.Y]
	inc	[cur_editor.SelStart.Y]
	inc	[cur_editor.Lines.Count]

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]

	mov	[cur_editor.Modified],1

  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_tab ;///// SWITCH TO NEXT TAB //////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[tab_bar.Items.Count],1
	je	.exit
	xor	eax,eax
	mov	ebp,[tab_bar.Items]
    @@: cmp	ebp,[tab_bar.Current.Ptr]
	je	@f
	inc	eax
	add	ebp,sizeof.TABITEM
	jmp	@b
    @@: add	ebp,sizeof.TABITEM
	inc	eax
	cmp	eax,[tab_bar.Items.Count]
	jb	@f
	mov	ebp,[tab_bar.Items]
    @@: call	set_cur_tab
	call	make_tab_visible
	call	align_editor_in_tab
	call	draw_editor
	call	draw_tabctl
	call	update_caption
  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.shift_ctrl_tab ;///// SWITCH TO PREVIOUS TAB ////////////////////////
;-----------------------------------------------------------------------------
	cmp	[tab_bar.Items.Count],1
	je	.exit
	xor	eax,eax
	mov	ebp,[tab_bar.Items]
    @@: cmp	ebp,[tab_bar.Current.Ptr]
	je	@f
	inc	eax
	add	ebp,sizeof.TABITEM
	jmp	@b
    @@: add	ebp,-sizeof.TABITEM
	dec	eax
	jge	@f
	imul	eax,[tab_bar.Items.Count],sizeof.TABITEM
	add	eax,[tab_bar.Items]
	lea	ebp,[eax-sizeof.TABITEM]
    @@: call	set_cur_tab
	call	make_tab_visible
	call	align_editor_in_tab
	call	draw_editor
	call	draw_tabctl
	call	update_caption
  .exit:
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_f4 ;///// CLOSE CURRENT TAB ////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[cur_editor.Modified],0
	je	.close
	mov	[bot_mode2],2
	jmp	key.ctrl_o.direct
 .close:
	mov	[do_not_draw],1
	push	[tab_bar.Current.Ptr]
	cmp	[tab_bar.Items.Count],1
	jne	@f
	;call    create_tab
	jmp	key.alt_x.close 	; close program
    @@: pop	ebp
	call	delete_tab
	dec	[do_not_draw]
	call	align_editor_in_tab
	call	draw_editor
	call	draw_tabctl
	call	draw_statusbar
	ret
endp

;-----------------------------------------------------------------------------
proc key.shift_f9 ;///// SET DEFAULT TAB /////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[tab_bar.Current.Ptr]
	cmp	eax,[tab_bar.Default.Ptr]
	jne	@f
	xor	eax,eax
    @@: mov	[tab_bar.Default.Ptr],eax
	mov	ebp,[tab_bar.Current.Ptr]
	call	make_tab_visible
	cmp	[tab_bar.Style],2
	jbe	@f
	call	align_editor_in_tab
	call	draw_editor
    @@: call	draw_tabctl
	ret
endp

;-----------------------------------------------------------------------------
proc key.f3 ;///// FIND NEXT MATCH ///////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	search
	jc	@f
    @@: ret
endp

;-----------------------------------------------------------------------------
proc key.f9 ;///// COMPILE AND RUN ///////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	bl,1
	call	start_fasm
	ret
endp

;-----------------------------------------------------------------------------
proc key.ctrl_f9 ;///// COMPILE //////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	bl,0
	call	start_fasm
	ret
endp

;-----------------------------------------------------------------------------
proc key.alt_x ;///// EXIT PROGRAM ///////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[main_closing],1
	mov	eax,[tab_bar.Items]
	mov	[exit_tab_item],eax
	mov	eax,[tab_bar.Items.Count]
	mov	[exit_tab_num],eax
  .direct:
	call	try_to_close_tabs
	or	eax,eax
	jz	@f
	mov	[bot_mode2],2
	jmp	key.ctrl_o.direct

    @@: stdcall save_settings

  .close:
	mov	[main_closed],1
	mcall	-1
endp

;-----------------------------------------------------------------------------
proc try_to_close_tabs ;///// FIND TABS TO BE SAVED BEFORE CLOSE /////////////
;-----------------------------------------------------------------------------
	push	ecx ebp
	call	flush_cur_tab
	mov	ebp,[exit_tab_item] ; [tab_bar.Items]
	add	ebp,-sizeof.TABITEM
    @@: dec	[exit_tab_num]
	js	.ok
	add	ebp,sizeof.TABITEM
	mov	al,[ebp+TABITEM.Editor.Modified]
	cmp	[ebp+TABITEM.Editor.Modified],0
	je	@b
	mov	[exit_tab_item],ebp
	call	set_cur_tab
	call	make_tab_visible
	xor	eax,eax
	inc	eax
    @@: pop	ebp ecx
	ret
  .ok:	xor	eax,eax
	jmp	@b
endp
