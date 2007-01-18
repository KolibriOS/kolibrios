key:
	mov	ecx,1
	mcall	66,3
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
    @@: mov	[shi],ebx
	test	al,0x03
	jz	@f
	inc	cl
    @@: mcall	26,2,,key1

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
	or	eax,[shi]

	test	al,0x80
	jnz	still.skip_write
	mov	[chr],al

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
	test	[options],OPTS_SECURESEL
	jz	.lp1
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]
  .lp1: mov	[s_status],0
	call	dword[esi+4]
	jmp	still
    @@: add	esi,8
	cmp	byte[esi],0
	jne	.acc

	test	dword[shi],KM_CTRLALT
	jnz	still.skip_write

	mov	[s_status],0

	movzx	eax,[chr]
	movzx	eax,[eax+key0]
	or	al,al
	jz	still.skip_write
	movzx	eax,[eax+key1]
	push	eax

	test	[options],OPTS_SECURESEL
	jz	.lp2
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]
	jmp	.put
  .lp2: call	delete_selection

	test	[options],OPTS_AUTOBRACES
	jz	.put
	cmp	al,'['
	jne	@f
	mov	al,']'
	call	.lp3
	dec	[cur_editor.Caret.X] ;! [pos.x]
	jmp	.put
    @@: cmp	al,'('
	jne	@f
	mov	al,')'
	call	.lp3
	dec	[cur_editor.Caret.X] ;! [pos.x]
	jmp	.put
    @@: cmp	al,'{'
	jne	.put
	mov	al,'}'
	call	.lp3
	dec	[cur_editor.Caret.X] ;! [pos.x]

  .put: pop	eax
	push	still
	inc	[cur_editor.SelStart.X] ;! [sel.x]
  .lp3: push	[cur_editor.Caret.X] eax ;! [pos.x] eax
	inc	dword[esp+4]
	mov	eax,1
	jmp	key.tab.direct

;-----------------------------------------------------------------------------
func key.ctrl_a ;///// SELECT ALL DOCUMENT ///////////////////////////////////
;-----------------------------------------------------------------------------
	xor	eax,eax
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
	mov	ecx,[cur_editor.Lines.Count] ;! ecx,[lines]
	dec	ecx
	mov	[cur_editor.Caret.Y],ecx ;! [pos.y],ecx
	call	get_line_offset
	call	get_real_length
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	call	draw_file
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_o ;///// ENTER OPEN FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[bot_dlg_mode2],0

  .direct:
	mov	[s_status],s_enter_filename

	mov	[bot_mode],1
	mov	[bot_dlg_height],16*2+4*2-1
	mov	[bot_dlg_handler],osdlg_handler
	mov	[focused_tb],tb_opensave

	mov	ecx,[f_info.length]
	mov	[tb_opensave.length],cl
	jecxz	@f
	mov	esi,f_info.path
	mov	edi,tb_opensave.text
	cld
	rep	movsb

    @@: mov	al,[tb_opensave.length]
	mov	[tb_opensave.pos.x],al
	mov	[tb_opensave.sel.x],0
	mov	[tb_casesen],0;1
	call	drawwindow
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_s ;///// ENTER SAVE FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[cur_editor.FilePath],'/'
	jne	key.shift_ctrl_s
	cmp	[cur_editor.Modified],0 ;! [modified],0
	je	.exit
	;cmp     [f_info.length],0
	;je      key.shift_ctrl_s
	call	save_file
	call	drawwindow
  .exit:
	ret

    key.shift_ctrl_s:
	mov	[bot_dlg_mode2],1
	jmp	key.ctrl_o.direct
endf

;-----------------------------------------------------------------------------
func key.ctrl_n ;///// CREATE NEW FILE (TAB) /////////////////////////////////
;-----------------------------------------------------------------------------
	call	create_tab
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_f ;///// ENTER KEYWORD TO FIND /////////////////////////////////
;-----------------------------------------------------------------------------
	mov	[bot_dlg_mode2],0
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
endf

func key.ctrl_h
	mov	[bot_dlg_mode2],1
	mov	[bot_dlg_height],16*3+4*2+1

	mov	[s_status],s_enter_text_to_replace

	jmp	key.ctrl_f.direct
endf

func key.ctrl_g
	ret
macro comment {
	mov	[bot_dlg_mode2],0
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
}
endf

; CHANGE_LANG_LAYOUT { ; Ctrl + F8
; 3 times english -> russian
; 2 times russian -> english
;        call    layout
;        jmp     still
; CHANGE_LANG_LAYOUT }

;-----------------------------------------------------------------------------
func key.ctrl_left ;///// GO TO PREVIOUS WORD ////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_left: ;///// GO TO PREVIOUS WORD, WITH SELECTION /////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Caret.Y] ;! ebx,[pos.y]
	mov	edx,[cur_editor.Caret.X] ;! edx,[pos.x]
	cld
	mov	ecx,ebx
	call	get_line_offset
  .lp1: cmp	dx,[esi]
	jle	@f
	movzx	edx,word[esi]
    @@: dec	edx
	jl	.nx1
	add	esi,4
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
	movzx	edx,word[esi]
	dec	edx
	jmp	.lp1
    @@:
	mov	ecx,ebx
	call	get_line_offset
  .lp2: cmp	dx,[esi]
	jle	@f
	movzx	edx,word[esi]
    @@: or	edx,edx
	jl	.nx2
	add	esi,4
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
	movzx	edx,word[esi]
	dec	edx
	jmp	.lp2
    @@:
	inc	edx
	mov	[cur_editor.Caret.Y],ebx ;! [pos.y],ebx
	mov	[cur_editor.Caret.X],edx ;! [pos.x],edx
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],ebx ;! [sel.y],ebx
	mov	[cur_editor.SelStart.X],edx ;! [sel.x],edx
    @@: sub	ebx,[cur_editor.TopLeft.Y] ;! ebx,[top_line]
	jge	@f
	add	[cur_editor.TopLeft.Y],ebx ;! [top_line],ebx
    @@: mov	eax,edx
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[cur_editor.TopLeft.X],eax ;! [left_col],eax
	jmp	.exit
    @@: cmp	edx,[cur_editor.TopLeft.X] ;! edx,[left_col]
	jge	.exit
	mov	[cur_editor.TopLeft.X],edx ;! [left_col],edx
  .exit:
	call	draw_file
  .exit.2:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_right ;///// GO TO NEXT WORD ///////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_right: ;///// GO TO NEXT WORD, WITH SELECTION ////////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Caret.Y] ;! ebx,[pos.y]
	mov	edx,[cur_editor.Caret.X] ;! edx,[pos.x]
	cld
  .lp1: mov	ecx,ebx
	call	get_line_offset
	movzx	ecx,word[esi]
	cmp	edx,ecx
	jge	.nx1
	add	esi,4
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
	cmp	ebx,[cur_editor.Lines.Count] ;! ebx,[lines]
	jge	.exit.2
	xor	edx,edx
	jmp	.lp1
    @@:

  .lp2: mov	ecx,ebx
	call	get_line_offset
	movzx	ecx,word[esi]
	cmp	edx,ecx
	jge	.nx2
	add	esi,4
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
	cmp	ebx,[cur_editor.Lines.Count] ;! ebx,[lines]
	jge	.exit.2
	xor	edx,edx
	jmp	.lp2
    @@:
	mov	[cur_editor.Caret.Y],ebx ;! [pos.y],ebx
	mov	[cur_editor.Caret.X],edx ;! [pos.x],edx
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],ebx ;! [sel.y],ebx
	mov	[cur_editor.SelStart.X],edx ;! [sel.x],edx
    @@: sub	ebx,[cur_editor.TopLeft.Y] ;! ebx,[top_line]
	cmp	ebx,[lines.scr]
	jl	@f
	sub	ebx,[lines.scr]
	inc	ebx
	add	[cur_editor.TopLeft.Y],ebx ;! [top_line],ebx
    @@: mov	eax,edx
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[cur_editor.TopLeft.X],eax ;! [left_col],eax
	jmp	.exit
    @@: cmp	edx,[cur_editor.TopLeft.X] ;! edx,[left_col]
	jge	.exit
	mov	[cur_editor.TopLeft.X],edx ;! [left_col],edx
  .exit:
	call	draw_file
  .exit.2:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_x
	cmp	[sel.selected],0
	je	@f
	call	key.ctrl_c
	call	key.del
	mov	[cur_editor.Modified],1 ;! [modified],1
    @@: ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_c
	mov	[copy_size],0
	cmp	[sel.selected],0
	je	.exit

	call	get_selection_size
	mov	ebx,eax
	mov	eax,[copy_buf]
	call	mem.ReAlloc
	mov	[copy_buf],eax

	cld
	mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.single_line
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	inc	ecx
	push	ecx
	mov	edi,[copy_buf] ;! AREA_CBUF
	call	get_real_length
	sub	eax,[sel.begin.x]
	jge	@f
	xor	eax,eax
    @@: add	esi,[sel.begin.x]
	add	esi,4
	stosd
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
	movzx	eax,word[esi]
	add	esi,4
;       lodsd
	cmp	eax,[sel.end.x]
	jle	@f
	mov	eax,[sel.end.x]
    @@: mov	ebx,edi
	stosd
;       add     esi,eax
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
    @@: sub	edi,[copy_buf] ;! AREA_CBUF
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
	mov	edi,[copy_buf] ;! AREA_CBUF
	stosd
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	mov	ebx,[sel.begin.x]
	mov	ecx,[sel.end.x]
	cmp	ebx,[esi]
	jge	.add_spaces
	cmp	ecx,[esi]
	jle	.lp1
	mov	ecx,[esi]
  .lp1: sub	ecx,[sel.begin.x]
	sub	eax,ecx
	lea	esi,[esi+ebx+4]
	rep	movsb

  .add_spaces:
	mov	ecx,eax
	mov	al,' '
	jecxz	@b
	rep	stosb
	jmp	@b
endf

;-----------------------------------------------------------------------------
func key.ctrl_v
	cmp	[copy_size],0
	je	.exit

	call	delete_selection

	mov	eax,[copy_size]
	call	editor_realloc_lines

	mov	eax,[cur_editor.Lines]
	mov	ebx,[cur_editor.Lines.Size]
	add	ebx,[copy_size]
	mov	[cur_editor.Lines.Size],ebx
	call	mem.ReAlloc
	mov	[cur_editor.Lines],eax

	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	pushd	[esi] esi
	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	call	line_add_spaces
	add	[esp],eax		 ;!!!
	add	esi,eax 		 ;!!!
	mov	ecx,[copy_size]
	sub	ecx,4
	mov	edi,[cur_editor.Lines] ;! AREA_TEMP2
	add	edi,[edi-4]
	dec	edi
	mov	eax,esi
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[eax+4]
	add	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	neg	ecx
	lea	ecx,[esi+ecx+1]
	std
	rep	movsb

	mov	ecx,[copy_count]
	dec	ecx
	jz	.single_line

	cld
	pop	edi
	add	edi,4
	mov	esi,[copy_buf] ;! AREA_CBUF
	lodsd

	mov	ebx,[cur_editor.Caret.X] ;! ebx,[pos.x]
	add	eax,ebx
	mov	[edi-4],ax
	mov	byte[edi-4+2],0x0001
	sub	eax,ebx
	call	.check_columns
	add	edi,ebx
    @@: push	ecx
	mov	ecx,eax
	rep	movsb
	lodsd
	and	eax,0x0000FFFF
	stosd
	mov	byte[edi-4+2],0x0001
	pop	ecx
	loop	@b

	pop	ecx
	sub	ecx,ebx
	add	[edi-4],cx
	call	.check_columns
	mov	ecx,eax
	rep	movsb

	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
	mov	eax,[copy_count]
	dec	eax
	add	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	add	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
	add	[cur_editor.Lines.Count],eax ;! [lines],eax

	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1
	jmp	.exit

  .single_line:
	cld
	pop	edi
	add	edi,4
	mov	esi,[copy_buf] ;! AREA_CBUF
	lodsd
	add	[edi-4],ax
	and	dword[edi-4],not 0x00020000
	or	dword[edi-4],0x00010000
	call	.check_columns
	add	edi,[cur_editor.Caret.X] ;! edi,[pos.x]
	add	esp,4
	mov	ecx,eax
	rep	movsb

	add	[cur_editor.Caret.X],eax ;! [pos.x],eax
	add	[cur_editor.SelStart.X],eax ;! [sel.x],eax

	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1

  .exit:
	ret

  .check_columns:
	push	eax
	movzx	eax,word[edi-4]
	cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax ;! [columns],eax
    @@: pop	eax
	ret
;        push    eax ebx esi
;        add     esi,eax
;        xor     ebx,ebx
;        neg     eax
;        jz      .lp1
;    @@: cmp     byte[esi+ebx-1],' '
;        jne     .lp1
;        dec     ebx
;        cmp     ebx,eax
;        jg      @b
;  .lp1: add     ebx,[edi-4]
;        cmp     [columns],ebx
;        jae     @f
;        mov     [columns],ebx
;    @@: pop     esi ebx eax
;        ret

endf

;-----------------------------------------------------------------------------
func key.ctrl_d ;///// INSERT SEPARATOR //////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,94
	call	editor_realloc_lines

	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	mov	ebx,esi

	mov	ecx,[cur_editor.Lines.Count] ;! ecx,[lines]
	call	get_line_offset
	lea	edi,[esi+90+4]
	lea	ecx,[esi+4]
	sub	ecx,ebx
	std
	rep	movsb

	lea	edi,[ebx+5]
	mov	word[ebx],90
	mov	al,ASEPC
	mov	ecx,79
	cld
	rep	stosb
	mov	al,' '
	mov	ecx,10
	rep	stosb
	mov	byte[ebx+4],';'

	inc	[cur_editor.Lines.Count] ;! [lines]
	inc	[cur_editor.Caret.Y] ;! [pos.y]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]

	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_y ;///// DELETE CURRENT LINE ///////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	inc	eax
	cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jge	.exit

	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	mov	edi,esi
	lodsd
	and	eax,0x0000FFFF
	add	esi,eax
	push	eax

	dec	[cur_editor.Lines.Count] ;! [lines]
	;mov     ecx,[temp_buf] ;! AREA_TEMP2
	mov	ecx,[cur_editor.Lines]
	add	ecx,[ecx-4]
	sub	ecx,esi
	shr	ecx,2		       ;// fixed (was 4)
	cld
	rep	movsd

	pop	eax
	add	eax,4
	neg	eax
	call	editor_realloc_lines

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]

	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.up ;///// GO TO PREVIOUS LINE ///////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_up: ;///// GO TO PREVIOUS LINE, WITH SELECTION ////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	dec	eax
	jns	@f
	xor	eax,eax
    @@: mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	cmp	eax,ecx
	jae	@f
	dec	ecx
	jns	@f
	xor	ecx,ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.down ;///// GO TO NEXT LINE /////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_down: ;///// GO TO NEXT LINE, WITH SELECTION //////////////////
;-----------------------------------------------------------------------------

	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	inc	eax
	cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jb	@f
	dec	eax
    @@: mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	mov	edx,eax
	sub	edx,ecx
	cmp	edx,[lines.scr]
	jb	@f
	inc	ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.left ;///// GO TO PREVIOUS CHAR /////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_left: ;///// GO TO PREVIOUS CHAR, WITH SELECTION //////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	dec	eax
	jns	@f
	inc	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
    @@: mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	call	 check_inv_all

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.right ;///// GO TO NEXT CHAR ////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_right: ;///// GO TO NEXT CHAR, WITH SELECTION /////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	inc	eax
	cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jbe	@f
	dec	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
    @@: mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	call	check_inv_all

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.pgup ;///// GO TO PREVIOUS PAGE /////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_pgup: ;///// GO TO PREVIOUS PAGE, WITH SELECTION //////////////
;-----------------------------------------------------------------------------
	mov	edx,[lines.scr]
	dec	edx
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	sub	eax,edx
	jns	@f
	xor	eax,eax
    @@: sub	ecx,edx
	jns	@f
	xor	ecx,ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.pgdn ;///// GO TO NEXT PAGE /////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_pgdn: ;///// GO TO NEXT PAGE, WITH SELECTION //////////////////
;-----------------------------------------------------------------------------
	mov	edx,[lines.scr]
	dec	edx
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	add	eax,edx
	add	ecx,edx
	cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jb	@f
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	dec	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.home ;///// GO TO LINE START ////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_home: ;///// GO TO LINE START, WITH SELECTION /////////////////
;-----------------------------------------------------------------------------
	mov	[cur_editor.Caret.X],0 ;! [pos.x],0
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.X],0 ;! [sel.x],0
    @@: call	check_inv_all

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.end ;///// GO TO LINE END ///////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_end: ;///// GO TO LINE END, WITH SELECTION ////////////////////
;-----------------------------------------------------------------------------
	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	call	get_real_length
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
    @@: call	check_inv_all

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_home ;///// GO TO PAGE START ///////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_home: ;///// GO TO PAGE START, WITH SELECTION ////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.TopLeft.Y] ;! eax,[top_line]
	mov	ecx,eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_end ;///// GO TO PAGE END //////////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_end: ;///// GO TO PAGE END, WITH SELECTION ///////////////
;-----------------------------------------------------------------------------
	mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	mov	eax,[lines.scr]
	cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jle	@f
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
    @@: add	eax,ecx
	dec	eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_init

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_pgup ;///// GO TO DOCUMENT START ///////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_pgup: ;///// GO TO DOCUMENT START, WITH SELECTION ////////
;-----------------------------------------------------------------------------
	xor	eax,eax
	mov	[cur_editor.TopLeft.Y],eax ;! [top_line],eax
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax
    @@: call	check_inv_all.skip_check

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_pgdn ;///// GO TO DOCUMENT END /////////////////////////////////
;-----------------------------------------------------------------------------
	call	clear_selection

;-----------------------------------------------------------------------------
     key.shift_ctrl_pgdn: ;///// GO TO DOCUMENT END, WITH SELECTION //////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]    ; eax = lines in the file
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	sub	eax,[lines.scr]   ; eax -= lines on the screen
	jns	@f
	xor	eax,eax
    @@: mov	[cur_editor.TopLeft.Y],eax ;! [top_line],eax
	dec	[cur_editor.Caret.Y] ;! [pos.y]
	test	byte[shi+2],0x01
	jnz	@f
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y]
;!      push    [pos.y]
;!      pop     [sel.y]
    @@: call	check_inv_all.skip_check

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.del ;///// DELETE NEXT CHAR OR SELECTION ////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	jnc	.exit.2

	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000
	lea	ebx,[esi+4]
	mov	ebp,esi

	call	get_real_length
	or	eax,eax
	je	.line_up

	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	cmp	ecx,eax
	jae	.line_up
	lea	edi,[ebx+ecx]
	neg	ecx
	movzx	eax,word[ebp]
	add	ecx,eax;[ebp]
	repe	scasb
	je	.line_up

	mov	edi,ebx
	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	add	edi,ecx
	lea	esi,[edi+1]
	neg	ecx
	movzx	eax,word[ebp]
	add	ecx,eax;[ebp]
	dec	ecx
	rep	movsb
	mov	byte[edi],' '

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]
	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1
	ret

  .line_up:
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	dec	eax
	cmp	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	je	.exit
	mov	edi,[temp_buf] ;! AREA_TEMP+4
	add	edi,4
	mov	esi,ebx
	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	rep	movsb
	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	mov	eax,[temp_buf]
	mov	[eax],ecx ;! [AREA_TEMP],ecx
	cmp	cx,[ebp]
	jbe	@f
	movzx	eax,word[ebp]
	sub	ecx,eax;[ebp]
	sub	edi,ecx
	mov	al,' '
	rep	stosb
    @@: lea	esi,[ebx+4]
	movzx	eax,word[ebp]
	add	esi,eax;[ebp]
	movzx	ecx,word[esi-4]
	mov	eax,[temp_buf]
	add	[eax],ecx ;! [AREA_TEMP],ecx
	or	dword[eax],0x00010000 ;! dword[AREA_TEMP],0x00010000
	rep	movsb

	mov	ecx,edi ;! [edi-AREA_TEMP]
	sub	ecx,[temp_buf]

	mov	esi,[temp_buf] ;! AREA_TEMP
	call	get_real_length
	cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax ;! [columns],eax
    @@:
	push	ecx
	mov	edi,[cur_editor.Lines] ;! AREA_TEMP2
	add	edi,[edi-4]
	dec	edi
	lea	esi,[edi+8]
	sub	esi,ecx
	movzx	eax,word[ebp]
	add	esi,eax;[ebp]
;       lea     eax,[ebp+4]
;       add     eax,[ebp]
	movzx	eax,word[ebp]
	movzx	eax,word[ebp+eax+4]
	add	esi,eax;[eax]
	lea	ecx,[esi-4]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	jz	.lp1
	mov	edi,ebp
	add	edi,[esp]
	lea	esi,[ebp+8]
	movzx	eax,word[esi-8]
	add	esi,eax;[esi-8]
	movzx	eax,word[esi-4]
	add	esi,eax;[esi-4]
	mov	ecx,[cur_editor.Lines] ;! AREA_TEMP2
	add	ecx,[ecx-4]
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp1: pop	ecx
	mov	esi,[temp_buf] ;! AREA_TEMP
	mov	edi,ebp
	cld
	rep	movsb

  .ok.dec.lines:
	dec	[cur_editor.Lines.Count] ;! [lines]
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	cmp	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	jb	@f
	dec	eax
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
    @@: m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]

	mov	ecx,[cur_editor.Lines.Count]
	call	get_line_offset
	movzx	eax,word[esi]
	lea	esi,[esi+eax+4]
	mov	eax,[cur_editor.Lines]
	add	eax,[eax-4]
	sub	esi,eax
	lea	eax,[esi+4096]
	call	editor_realloc_lines

	mov	[cur_editor.Modified],1 ;! [modified],1
  .exit.2:
	call	check_inv_all

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
; INSERT {
  key.ins:
;// ... toggle insert/overwrite mode here ...
	xor	[ins_mode],1
	call	check_inv_str
	ret
; INSERT }

;-----------------------------------------------------------------------------
func key.bkspace ;///// DELETE PREVIOUS CHAR OR SELECTION ////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	jnc	key.del.exit.2

	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	dec	eax
	js	.line_up

	dec	[cur_editor.Caret.X] ;! [pos.x]
	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000

	mov	ebx,eax
	call	get_real_length
	cmp	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	jae	@f
	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1
	ret

    @@: lea	edi,[esi+4+ebx]
	mov	ecx,ebx
	neg	ecx
	movzx	eax,word[esi]
	add	ecx,eax;[esi]
	dec	ecx
	lea	esi,[edi+1]
	cld
	rep	movsb
	mov	byte[edi],' '

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]
	call	check_inv_str
	mov	[cur_editor.Modified],1 ;! [modified],1
	ret

  .line_up:
	cmp	[cur_editor.Caret.Y],0 ;! [pos.y],0
	jne	@f
	ret
    @@: mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	dec	ecx
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000

	mov	ebp,esi
	lea	ebx,[esi+4]
	movzx	ecx,word[ebp]
    @@: cmp	byte[ebx+ecx-1],' '
	jne	@f
	dec	ecx
	jg	@b
    @@: mov	[cur_editor.Caret.X],ecx ;! [pos.x],ecx
	dec	[cur_editor.Caret.Y] ;! [pos.y]
	cld
	jmp	key.del.line_up
endf

;-----------------------------------------------------------------------------
func key.tab ;///// TABULATE /////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]

	mov	ecx,eax
	add	eax,ATABW
	and	eax,not(ATABW-1)
	push	eax ' '
	sub	eax,ecx
  .direct:
	push	eax
	call	editor_realloc_lines
	pop	eax
	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000

	xchg	eax,ecx

	call	get_real_length
	cmp	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	jae	@f
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
    @@: movzx	edx,word[esi]
	sub	edx,eax
	cmp	ecx,edx
	jl	@f
	push	eax
	mov	eax,10
	call	editor_realloc_lines
	add	esi,eax
	pop	eax
	pushad; esi ecx eax
	mov	ecx,[cur_editor.Lines] ;! AREA_TEMP2-10+1
	add	ecx,[ecx-4]
	dec	ecx
	mov	edi,ecx ;! AREA_TEMP2
	add	ecx,-10+1
;       lea     eax,[esi+4]
;       add     eax,[esi]
	movzx	eax,word[esi]
	lea	eax,[esi+eax+4]
	sub	ecx,eax
	lea	esi,[edi-10] ;! AREA_TEMP2-10
	std
	rep	movsb
	mov	ecx,10
	mov	al,' '
	rep	stosb
	popad;  eax ecx esi
	add	word[esi],10
	jmp	@b
    @@: lea	ebx,[esi+4]
	push	ecx
;       lea     edi,[ebx-1]
;       add     edi,[esi]
	movzx	edi,word[esi]
	lea	edi,[ebx+edi-1]
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[esi+1]
	sub	ecx,ebx
	sub	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	std
	rep	movsb
  .ok:	pop	ecx		;*******
	pop	eax
	rep	stosb
	cld
	pop	[cur_editor.Caret.X] ;! [pos.x]
	lea	esi,[ebx-4]
	call	get_real_length
	cmp	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	jae	@f
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
    @@: cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jbe	@f
	mov	[cur_editor.Columns.Count],eax ;! [columns],eax
    @@: m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]
	m2m	[cur_editor.SelStart.Y],[cur_editor.Caret.Y] ;! [sel.y],[pos.y]
	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.return ;///// CARRIAGE RETURN ///////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection

	mov	eax,14
	call	editor_realloc_lines

	mov	ecx,[cur_editor.Caret.Y] ;! ecx,[pos.y]
	call	get_line_offset

	mov	ebx,[cur_editor.Caret.X] ;! ebx,[pos.x]
	cmp	bx,[esi]
	jb	@f
	movzx	ebx,word[esi]
	dec	ebx
	jns	@f
	xor	ebx,ebx
    @@:
	cld

	mov	edi,[temp_buf] ;! AREA_TEMP
	mov	ebp,esi
	lea	ecx,[ebx+1]
    @@: dec	ecx
	jz	@f
	cmp	byte[esi+ecx+4-1],' '
	je	@b
    @@: lea	eax,[ecx+10]
	or	eax,0x00010000
	stosd
	jecxz	@f
	push	esi
	add	esi,4
	rep	movsb
	pop	esi
    @@: mov	al,' '
	mov	ecx,10
	rep	stosb

	movzx	ecx,word[esi]
	sub	ecx,ebx;[pos.x]
	add	esi,ebx;[pos.x]
	add	esi,4
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
  .lp1: test	[options],OPTS_AUTOINDENT
	jz	.lp2
	push	edi ecx
	movzx	ecx,word[ebp]
	lea	edi,[ebp+4]
	mov	al,' '
	repe	scasb
	mov	eax,ecx
	pop	ecx edi
	je	.lp2
	neg	eax
	movzx	edx,word[ebp]
	add	eax,edx;[ebp]
	dec	eax
	jmp	@f
  .lp2: xor	eax,eax
    @@: mov	edx,edi
	add	edi,4
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
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

	lea	eax,[edi-4]
	sub	eax,edx
	or	eax,0x00010000
	mov	[edx],eax

	mov	ecx,edi ;! [edi-AREA_TEMP]
	sub	ecx,[temp_buf]

	push	ecx
	mov	edi,[cur_editor.Lines] ;! AREA_TEMP2
	add	edi,[edi-4]
	dec	edi
	lea	esi,[edi+4]
	sub	esi,ecx
	movzx	ecx,word[ebp]
	add	esi,ecx;[ebp]
	lea	ecx,[esi-4]
	sub	ecx,ebp
	std
	cmp	esi,edi
	jb	@f
	je	.lp3
	lea	esi,[ebp+4]
	mov	eax,[esp]
	lea	edi,[esi+eax-4]
	movzx	ecx,word[ebp]
	add	esi,ecx;[ebp]
	mov	ecx,[cur_editor.Lines] ;! AREA_TEMP2
	add	ecx,[ecx-4]
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp3: pop	ecx
	mov	esi,[temp_buf] ;! AREA_TEMP
	mov	edi,ebp
	cld
	rep	movsb

	inc	[cur_editor.Caret.Y] ;! [pos.y]
	inc	[cur_editor.SelStart.Y] ;! [sel.y]
	inc	[cur_editor.Lines.Count] ;! [lines]

	m2m	[cur_editor.SelStart.X],[cur_editor.Caret.X] ;! [sel.x],[pos.x]

	call	check_inv_all
	mov	[cur_editor.Modified],1 ;! [modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_tab ;///// SWITCH TO NEXT TAB //////////////////////////////////
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
  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.shift_ctrl_tab ;///// SWITCH TO PREVIOUS TAB ////////////////////////
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
  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_f4 ;///// CLOSE CURRENT TAB ////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	ebp,[tab_bar.Current.Ptr]
	call	delete_tab
	cmp	[tab_bar.Items.Count],0
	jne	@f
	call	create_tab
    @@: ret
endf

;-----------------------------------------------------------------------------
func key.shift_f9 ;///// SET DEFAULT TAB /////////////////////////////////////
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
endf
