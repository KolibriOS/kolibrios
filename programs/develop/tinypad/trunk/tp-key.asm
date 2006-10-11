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

	mov	esi,accel_table
  .acc: cmp	eax,[esi]
	jne	@f
	test	[options],OPTS_SECURESEL
	jz	.lp1
	m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]
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
	m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]
	jmp	.put
  .lp2: call	delete_selection

	test	[options],OPTS_AUTOBRACES
	jz	.put
	cmp	al,'['
	jne	@f
	mov	al,']'
	call	.lp3
	dec	[pos.x]
	jmp	.put
    @@: cmp	al,'('
	jne	@f
	mov	al,')'
	call	.lp3
	dec	[pos.x]
	jmp	.put
    @@: cmp	al,'{'
	jne	.put
	mov	al,'}'
	call	.lp3
	dec	[pos.x]

  .put: pop	eax
	push	still
	inc	[sel.x]
  .lp3: push	[pos.x] eax
	inc	dword[esp+4]
	mov	eax,1
	jmp	key.tab.direct

;-----------------------------------------------------------------------------
func key.ctrl_a ;///// SELECT ALL DOCUMENT ///////////////////////////////////
;-----------------------------------------------------------------------------
	xor	eax,eax
	mov	[sel.x],eax
	mov	[sel.y],eax
	mov	ecx,[lines]
	dec	ecx
	mov	[pos.y],ecx
	call	get_line_offset
	call	get_real_length
	mov	[pos.x],eax
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
	mov	[tb_casesen],1
	call	drawwindow
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_s ;///// ENTER SAVE FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[modified],0
	je	.exit
	cmp	[f_info.length],0
	je	key.shift_ctrl_s
	call	save_file
	call	drawwindow
  .exit:
	ret

    key.shift_ctrl_s:
	mov	[bot_dlg_mode2],1
	jmp	key.ctrl_o.direct
endf

;-----------------------------------------------------------------------------
func key.ctrl_n ;///// ENTER SAVE FILENAME ///////////////////////////////////
;-----------------------------------------------------------------------------
	call	new_file
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
	mov	ebx,[pos.y]
	mov	edx,[pos.x]
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
	mov	[pos.y],ebx
	mov	[pos.x],edx
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],ebx
	mov	[sel.x],edx
    @@: sub	ebx,[top_line]
	jge	@f
	add	[top_line],ebx
    @@: mov	eax,edx
	sub	eax,[left_col]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[left_col],eax
	jmp	.exit
    @@: cmp	edx,[left_col]
	jge	.exit
	mov	[left_col],edx
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
	mov	ebx,[pos.y]
	mov	edx,[pos.x]
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
	cmp	ebx,[lines]
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
	cmp	ebx,[lines]
	jge	.exit.2
	xor	edx,edx
	jmp	.lp2
    @@:
	mov	[pos.y],ebx
	mov	[pos.x],edx
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],ebx
	mov	[sel.x],edx
    @@: sub	ebx,[top_line]
	cmp	ebx,[lines.scr]
	jl	@f
	sub	ebx,[lines.scr]
	inc	ebx
	add	[top_line],ebx
    @@: mov	eax,edx
	sub	eax,[left_col]
	cmp	eax,[columns.scr]
	jl	@f
	sub	eax,[columns.scr]
	inc	eax
	add	[left_col],eax
	jmp	.exit
    @@: cmp	edx,[left_col]
	jge	.exit
	mov	[left_col],edx
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
	mov	[modified],1
    @@: ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_c
	mov	[copy_size],0
	cmp	[sel.selected],0
	je	.exit
	cld
	mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.single_line
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	inc	ecx
	push	ecx
	mov	edi,AREA_CBUF
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
    @@: sub	edi,AREA_CBUF
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
	mov	edi,AREA_CBUF
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

	mov	ecx,[pos.y]
	call	get_line_offset
	pushd	[esi] esi
	mov	ecx,[pos.x]
	call	line_add_spaces
	mov	ecx,[copy_size]
	sub	ecx,4
	mov	edi,AREA_TEMP2
	mov	eax,esi
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[eax+4]
	add	ecx,[pos.x]
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
	mov	esi,AREA_CBUF
	lodsd

	mov	ebx,[pos.x]
	add	eax,ebx
	mov	[edi-4],ax
	sub	eax,ebx
	call	.check_columns
	add	edi,ebx
    @@: push	ecx
	mov	ecx,eax
	rep	movsb
	lodsd
	and	eax,0x0000FFFF
	stosd
	or	dword[edi-4],0x00010000
	pop	ecx
	loop	@b

	pop	ecx
	sub	ecx,ebx
	add	[edi-4],cx
	call	.check_columns
	mov	ecx,eax
	rep	movsb

	mov	[pos.x],eax
	mov	[sel.x],eax
	mov	eax,[copy_count]
	dec	eax
	add	[pos.y],eax
	add	[sel.y],eax
	add	[lines],eax

	call	check_inv_all
	mov	[modified],1
	jmp	.exit

  .single_line:
	cld
	pop	edi
	add	edi,4
	mov	esi,AREA_CBUF
	lodsd
	add	[edi-4],ax
	and	dword[edi-4],not 0x00020000
	or	dword[edi-4],0x00010000
	call	.check_columns
	add	edi,[pos.x]
	add	esp,4
	mov	ecx,eax
	rep	movsb

	add	[pos.x],eax
	add	[sel.x],eax

	call	check_inv_all
	mov	[modified],1

  .exit:
	ret

  .check_columns:
	push	eax
	movzx	eax,word[edi-4]
	cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
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
	mov	ecx,[pos.y]
	call	get_line_offset
	mov	ebx,esi

	mov	ecx,[lines]
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

	inc	[lines]
	inc	[pos.y]
	m2m	[sel.y],[pos.y]

	call	check_inv_all
	mov	[modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.ctrl_y ;///// DELETE CURRENT LINE ///////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[pos.y]
	inc	eax
	cmp	eax,[lines]
	jge	.exit

	mov	ecx,[pos.y]
	call	get_line_offset
	mov	edi,esi
	lodsd
	add	esi,eax

	dec	[lines]
	mov	ecx,AREA_TEMP2
	sub	ecx,esi
	shr	ecx,2		       ;// fixed (was 4)
	cld
	rep	movsd
	call	check_inv_all
	mov	[modified],1

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
	mov	eax,[pos.y]
	dec	eax
	jns	@f
	xor	eax,eax
    @@: mov	ecx,[top_line]
	cmp	eax,ecx
	jae	@f
	dec	ecx
	jns	@f
	xor	ecx,ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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

	mov	eax,[pos.y]
	inc	eax
	cmp	eax,[lines]
	jb	@f
	dec	eax
    @@: mov	ecx,[top_line]
	mov	edx,eax
	sub	edx,ecx
	cmp	edx,[lines.scr]
	jb	@f
	inc	ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	eax,[pos.x]
	dec	eax
	jns	@f
	inc	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.x],eax
    @@: mov	[pos.x],eax
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
	mov	eax,[pos.x]
	inc	eax
	cmp	eax,[columns]
	jbe	@f
	dec	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.x],eax
    @@: mov	[pos.x],eax
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
	mov	eax,[pos.y]
	mov	ecx,[top_line]
	sub	eax,edx
	jns	@f
	xor	eax,eax
    @@: sub	ecx,edx
	jns	@f
	xor	ecx,ecx
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	eax,[pos.y]
	mov	ecx,[top_line]
	add	eax,edx
	add	ecx,edx
	cmp	eax,[lines]
	jb	@f
	mov	eax,[lines]
	dec	eax
    @@: test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	[pos.x],0
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.x],0
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
	mov	ecx,[pos.y]
	call	get_line_offset
	call	get_real_length
	mov	[pos.x],eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.x],eax
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
	mov	eax,[top_line]
	mov	ecx,eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	ecx,[top_line]
	mov	eax,[lines.scr]
	cmp	eax,[lines]
	jle	@f
	mov	eax,[lines]
    @@: add	eax,ecx
	dec	eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	[top_line],eax
	mov	[pos.y],eax
	test	byte[shi+2],0x01
	jnz	@f
	mov	[sel.y],eax
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
	mov	eax,[lines]    ; eax = lines in the file
	mov	[pos.y],eax
	sub	eax,[lines.scr]   ; eax -= lines on the screen
	jns	@f
	xor	eax,eax
    @@: mov	[top_line],eax
	dec	[pos.y]
	test	byte[shi+2],0x01
	jnz	@f
	push	[pos.y]
	pop	[sel.y]
    @@: call	check_inv_all.skip_check

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.del ;///// DELETE NEXT CHAR OR SELECTION ////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	jnc	.exit.2

	mov	ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000
	lea	ebx,[esi+4]
	mov	ebp,esi

	call	get_real_length
	or	eax,eax
	je	.line_up

	mov	ecx,[pos.x]
	cmp	ecx,eax
	jae	.line_up
	lea	edi,[ebx+ecx]
	neg	ecx
	movzx	eax,word[ebp]
	add	ecx,eax;[ebp]
	repe	scasb
	je	.line_up

	mov	edi,ebx
	mov	ecx,[pos.x]
	add	edi,ecx
	lea	esi,[edi+1]
	neg	ecx
	movzx	eax,word[ebp]
	add	ecx,eax;[ebp]
	dec	ecx
	rep	movsb
	mov	byte[edi],' '

	m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]
	call	check_inv_all
	mov	[modified],1
	ret

  .line_up:
	mov	eax,[lines]
	dec	eax
	cmp	eax,[pos.y]
	je	.exit
	mov	edi,AREA_TEMP+4
	mov	esi,ebx
	mov	ecx,[pos.x]
	rep	movsb
	mov	ecx,[pos.x]
	mov	[AREA_TEMP],ecx
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
	add	[AREA_TEMP],ecx
	or	dword[AREA_TEMP],0x00010000
	rep	movsb

	lea	ecx,[edi-AREA_TEMP]

	mov	esi,AREA_TEMP
	call	get_real_length
	cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@:
	push	ecx
	mov	edi,AREA_TEMP2
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
	mov	ecx,AREA_TEMP2
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp1: pop	ecx
	mov	esi,AREA_TEMP
	mov	edi,ebp
	cld
	rep	movsb

  .ok.dec.lines:
	dec	[lines]
	mov	eax,[lines]
	cmp	[pos.y],eax
	jb	@f
	dec	eax
	mov	[pos.y],eax
    @@: m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]

	mov	[modified],1
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

	mov	eax,[pos.x]
	dec	eax
	js	.line_up

	dec	[pos.x]
	mov	ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000

	mov	ebx,eax
	call	get_real_length
	cmp	eax,[pos.x]
	jae	@f
	m2m	[sel.x],[pos.x]
	call	check_inv_all
	mov	[modified],1
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

	m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]
	call	check_inv_str
	mov	[modified],1
	ret

  .line_up:
	cmp	[pos.y],0
	jne	@f
	ret
    @@: mov	ecx,[pos.y]
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
    @@: mov	[pos.x],ecx
	dec	[pos.y]
	cld
	jmp	key.del.line_up
endf

;-----------------------------------------------------------------------------
func key.tab ;///// TABULATE /////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection
	mov	eax,[pos.x]

	mov	ecx,eax
	add	eax,ATABW
	and	eax,not(ATABW-1)
	push	eax ' '
	sub	eax,ecx
  .direct:
	mov	ecx,[pos.y]
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000

	xchg	eax,ecx

	call	get_real_length
	cmp	eax,[pos.x]
	jae	@f
	mov	eax,[pos.x]
    @@: movzx	edx,word[esi]
	sub	edx,eax
	cmp	ecx,edx
	jl	@f
	pushad; esi ecx eax
	mov	ecx,AREA_TEMP2-10+1
;       lea     eax,[esi+4]
;       add     eax,[esi]
	movzx	eax,word[esi]
	lea	eax,word[esi+eax+4]
	sub	ecx,eax
	mov	edi,AREA_TEMP2
	mov	esi,AREA_TEMP2-10
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
	sub	ecx,[pos.x]
	std
	rep	movsb
  .ok:	pop	ecx		;*******
	pop	eax
	rep	stosb
	cld
	pop	[pos.x]
	lea	esi,[ebx-4]
	call	get_real_length
	cmp	eax,[pos.x]
	jae	@f
	mov	eax,[pos.x]
    @@: cmp	eax,[columns]
	jbe	@f
	mov	[columns],eax
    @@: m2m	[sel.x],[pos.x]
	m2m	[sel.y],[pos.y]
	call	check_inv_all
	mov	[modified],1

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func key.return ;///// CARRIAGE RETURN ///////////////////////////////////////
;-----------------------------------------------------------------------------
	call	delete_selection

	mov	ecx,[pos.y]
	call	get_line_offset

	mov	ebx,[pos.x]
	cmp	bx,[esi]
	jb	@f
	movzx	ebx,word[esi]
	dec	ebx
	jns	@f
	xor	ebx,ebx
    @@:
	cld

	mov	edi,AREA_TEMP
	mov	ebp,esi
	mov	ecx,ebx
	inc	ecx
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
	mov	[pos.x],eax
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

	lea	ecx,[edi-AREA_TEMP]

	push	ecx
	mov	edi,AREA_TEMP2
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
	mov	ecx,AREA_TEMP2
	sub	ecx,esi
	cld
    @@: rep	movsb
  .lp3: pop	ecx
	mov	esi,AREA_TEMP
	mov	edi,ebp
	cld
	rep	movsb

	inc	[pos.y]
	inc	[sel.y]
	inc	[lines]

	m2m	[sel.x],[pos.x]

	call	check_inv_all
	mov	[modified],1

  .exit:
	ret
endf