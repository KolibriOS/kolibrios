;-----------------------------------------------------------------------------
func check_cur_vis_inv ;//////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	eax ebx
	xor	bl,bl
  .chk_y:
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	or	eax,eax
	jge	@f
	mov	[cur_editor.Caret.Y],0 ;! [pos.y],0
	jmp	.chk_dy
    @@: cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jl	.chk_dy
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	dec	eax
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
  .chk_dy:
	mov	eax,[cur_editor.TopLeft.Y] ;! eax,[top_line]
	cmp	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	jle	@f
	m2m	[cur_editor.TopLeft.Y],[cur_editor.Caret.Y]
;!      push    [pos.y]
;!      pop     [top_line]
	inc	bl
    @@: add	eax,[lines.scr]
	cmp	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	jg	.chk_x
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	sub	eax,[lines.scr]
	inc	eax
	mov	[cur_editor.TopLeft.Y],eax ;! [top_line],eax
	inc	bl
  .chk_x:
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	or	eax,eax
	jge	@f
	mov	[cur_editor.Caret.X],0 ;! [pos.x],0
	jmp	.chk_dx
    @@: cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jl	.chk_dx
	mov	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
  .chk_dx:
	mov	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	cmp	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	jle	@f
	m2m	[cur_editor.TopLeft.X],[cur_editor.Caret.X]
;!      push    [pos.x]
;!      pop     [left_col]
	inc	bl
    @@: add	eax,[columns.scr]
	cmp	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	jg	@f
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	sub	eax,[columns.scr]
	inc	eax
	mov	[cur_editor.TopLeft.X],eax ;! [left_col],eax
	inc	bl
    @@: cmp	[mev],MEV_LDOWN
	jne	.exit
	push	[cur_editor.Caret.X] [cur_editor.Caret.Y] ;! [pos.x] [pos.y]
	pop	[cur_editor.SelStart.Y] [cur_editor.SelStart.X] ;! [sel.y] [sel.x]
  .exit:
	or	bl,bl
	clc
	jz	@f
	call	draw_file
	stc
    @@: pop	ebx eax
	ret
endf

;-----------------------------------------------------------------------------
func clear_selection ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	eax ebx
	mov	eax,[cur_editor.SelStart.Y] ;! eax,[sel.y]
	mov	ebx,[cur_editor.Caret.Y] ;! ebx,[pos.y]
	cmp	eax,ebx
	jle	@f
	xchg	eax,ebx
    @@: push	[cur_editor.Caret.X] [cur_editor.Caret.Y] ;! [pos.x] [pos.y]
	pop	[cur_editor.SelStart.Y] [cur_editor.SelStart.X] ;! [sel.y] [sel.x]
	call	draw_file.ex
	pop	ebx eax
	ret
endf

;-----------------------------------------------------------------------------
func pt_in_rect ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	eax,[ecx+0x0]
	jl	@f
	cmp	ebx,[ecx+0x4]
	jl	@f
	cmp	eax,[ecx+0x8]
	jg	@f
	cmp	ebx,[ecx+0xC]
	jg	@f
	stc
	ret
    @@: clc
	ret
endf

;-----------------------------------------------------------------------------
func check_bottom_right ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	eax
	mov	eax,[cur_editor.TopLeft.Y] ;! eax,[top_line]
	add	eax,[lines.scr]
	cmp	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	jbe	.lp1
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	sub	eax,[lines.scr]
	jns	@f
	xor	eax,eax
    @@: mov	[cur_editor.TopLeft.Y],eax ;! [top_line],eax
  .lp1: mov	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	add	eax,[columns.scr]
	cmp	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	jbe	.exit
	mov	eax,[cur_editor.Columns.Count] ;! eax,[columns]
	sub	eax,[columns.scr]
	jns	@f
	xor	eax,eax
    @@: mov	[cur_editor.TopLeft.X],eax ;! [left_col],eax
  .exit:
	pop	eax
	ret
endf

;-----------------------------------------------------------------------------
func check_inv_str ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
@^
	mov	eax,[pos.y]
	mov	ecx,[top_line]
  .skip_init:
	call	check_cur_vis
	mov	[pos.y],eax
	mov	[top_line],ecx
  .skip_check:
;       call    invalidate_string
	call	drawfile
	ret
^@
endf

;-----------------------------------------------------------------------------
func check_inv_all ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Caret.Y] ;! eax,[pos.y]
	mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
  .skip_init:
	call	check_cur_vis
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	mov	[cur_editor.TopLeft.Y],ecx ;! [top_line],ecx
  .skip_check:
;       call    clear_screen
	call	draw_file
	ret
endf

;-----------------------------------------------------------------------------
func check_cur_vis ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	eax,ecx
	jb	.low
	mov	edx,ecx
	add	edx,[lines.scr]
	cmp	edx,[cur_editor.Lines.Count] ;! edx,[lines]
	jbe	@f
	mov	edx,[cur_editor.Lines.Count] ;! edx,[lines]
    @@: cmp	eax,edx
	jb	@f
	lea	ecx,[eax+1]
	sub	ecx,[lines.scr]
	jns	@f
	xor	ecx,ecx
	jmp	@f
  .low: mov	ecx,eax
    @@: mov	edx,ecx
	add	edx,[lines.scr]
	cmp	edx,[cur_editor.Lines.Count] ;! edx,[lines]
	jbe	@f
	mov	ecx,[cur_editor.Lines.Count] ;! ecx,[lines]
	sub	ecx,[lines.scr]
	jns	@f
	xor	ecx,ecx
    @@:;mov     [top_line],ecx

	pushad
	mov	eax,[cur_editor.Caret.X] ;! eax,[pos.x]
	mov	ebx,[cur_editor.TopLeft.X] ;! ebx,[left_col]
	mov	ecx,ebx
	add	ecx,[columns.scr]
	cmp	eax,ebx
	jb	.lp1
	cmp	eax,ecx
	jb	.exit
	lea	ebx,[eax]
	sub	ebx,[columns.scr]
	jmp	@f
  .lp1: mov	ebx,eax
    @@: mov	[cur_editor.TopLeft.X],ebx ;! [left_col],ebx

  .exit:
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	popad

	ret
endf

;-----------------------------------------------------------------------------
func get_real_length ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	movzx	eax,word[esi]
    @@: cmp	byte[esi+eax+4-1],' '
	jne	@f
	dec	eax
	jnz	@b
    @@: ret
endf

;-----------------------------------------------------------------------------
func get_line_offset ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  ECX = line number
; Output:
;  ESI = line data offset
;-----------------------------------------------------------------------------
	push	eax ecx
	mov	esi,[cur_editor.Lines] ;! AREA_EDIT
    @@: dec	ecx
	js	.exit
	movzx	eax,word[esi]
	lea	esi,[esi+eax+4]
	jmp	@b
  .exit:
	pop	ecx eax
	ret
endf

;-----------------------------------------------------------------------------
func init_sel_vars ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad
	mov	[sel.selected],1
	mov	eax,[cur_editor.SelStart.X] ;! eax,[sel.x]
	mov	ebx,[cur_editor.SelStart.Y] ;! ebx,[sel.y]
	mov	ecx,[cur_editor.Caret.X] ;! ecx,[pos.x]
	mov	edx,[cur_editor.Caret.Y] ;! edx,[pos.y]
	cmp	ebx,edx
	jl	.lp2
	jne	@f
	cmp	eax,ecx
	jl	.lp2
	jne	.lp1
	dec	[sel.selected]
	jmp	.lp2
    @@: xchg	ebx,edx
  .lp1: xchg	eax,ecx
  .lp2: mov	[sel.begin.x],eax
	mov	[sel.begin.y],ebx
	mov	[sel.end.x],ecx
	mov	[sel.end.y],edx
	popad
	ret
endf

;-----------------------------------------------------------------------------
func get_scroll_vars ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  EAX = maximum data size      (units)
;  EBX = visible data size      (units)
;  ECX = current data position  (units)
;  EDX = scrolling area size    (pixels)
; Output:
;  EAX = srcoller offset        (pixels)
;  EBX = scroller size          (pixels)
;-----------------------------------------------------------------------------
	push	eax ebx edx
;       sub     eax,ebx
	mov	esi,eax
	mov	eax,edx
	imul	ebx
	idiv	esi
	cmp	eax,[esp]
	jge	.null
	cmp	eax,AMINS
	jge	@f
	neg	eax
	add	eax,AMINS
	sub	[esp],eax
	mov	eax,AMINS
    @@: mov	[esp+4],eax	; scroller size
	mov	eax,[esp]
	imul	ecx
	idiv	esi
	or	eax,eax
	jns	@f
	xor	eax,eax
   @@:	mov	[esp+8],eax    ; scroller offset
	add	eax,[esp+4]
	cmp	eax,[esp]
	jle	@f
;        mov     eax,[esp]
;        sub     eax,[esp+4]
;        js      @f
;        mov     [esp+8],eax
    @@:
	pop	edx ebx eax
	ret
  .null:
	mov	dword[esp+4],0
	mov	dword[esp+8],0
	jmp	@b
endf

;-----------------------------------------------------------------------------
func uint2strz ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	dec	ebx
	jz	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	uint2strz
	pop	eax
    @@: cmp	al,10
	sbb	al,$69
	das
	stosb
	ret
endf

;-----------------------------------------------------------------------------
func uint2str ;///////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	eax,ecx
	jb	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	uint2str
	pop	eax
    @@: cmp	al,10
	sbb	al,$69
	das
	stosb
	ret
endf

;-----------------------------------------------------------------------------
func strlen ;/////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx
	mov	ebx,eax
	xor	eax,eax
    @@: cmp	byte[ebx+eax],0
	je	@f
	inc	eax
	jmp	@b
    @@: pop	ebx
	ret
endf

;-----------------------------------------------------------------------------
func rgb_to_gray ;////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	0 eax
	and	dword[esp],0x000000FF
	fild	dword[esp]
	fmul	[float_gray_b]
	shr	eax,8
	mov	[esp],eax
	and	dword[esp],0x000000FF
	fild	dword[esp]
	fmul	[float_gray_g]
	faddp
	shr	eax,8
	and	eax,0x000000FF
	mov	[esp],eax
	fild	dword[esp]
	fmul	[float_gray_r]
	faddp
	frndint
	fist	dword[esp]
	fist	dword[esp+1]
	fistp	dword[esp+2]
	pop	eax
	add	esp,4
	ret
endf

;float_gray_r dd 0.30f
;float_gray_g dd 0.59f
;float_gray_b dd 0.11f

;-----------------------------------------------------------------------------
func get_active_menu_item ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad
	mov	[mi_cur],0
	mcall	37,1
	movsx	ebx,ax
	sar	eax,16
	mov	ecx,__rc
	pushd	2 0 (main_menu.width+7) (ATOPH-2)
	popd	[__rc+0xC] [__rc+0x8] [__rc+0x4] [__rc+0x0]
;       add     [__rc+0xC],ATOPH-2
	call	pt_in_rect
	jnc	.outside_menu
	m2m	dword[ecx+0x8],dword[ecx+0x0]
	mov	edx,main_menu
    @@: inc	[mi_cur]
	movzx	esi,word[edx+0]
	add	[ecx+0x8],esi
	call	pt_in_rect
	jc	.exit
	m2m	dword[ecx+0x0],dword[ecx+0x8]
	add	edx,8+1
	movzx	esi,byte[edx-1]
	add	edx,esi
	cmp	byte[edx+8],0
	jne	@b
	mov	[mi_cur],0
  .exit:
	popad
	ret
  .outside_menu:
	or	[mi_cur],-1
    @@: popad
	ret
endf

;-----------------------------------------------------------------------------
func get_active_popup_item ;//////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad
	mov	[pi_cur],0
	mcall	37,1
	movsx	ebx,ax
	sar	eax,16
	mov	ecx,__rc
	mov	dword[ecx+0x0],0
	mov	dword[ecx+0x4],0
	movzx	edx,[ebp+POPUP.width]
	mov	dword[ecx+0x8],edx;POP_WIDTH
	movzx	edx,[ebp+POPUP.height]
	mov	dword[ecx+0xC],edx;POP_HEIGHT
	call	pt_in_rect
	jnc	.outside_window
	inc	dword[ecx+0x0]
	mov	dword[ecx+0x4],3
	dec	dword[ecx+0x8]
	mov	dword[ecx+0xC],3+POP_IHEIGHT-1
	mov	edx,[ebp+POPUP.data];popup_text.data
    @@: inc	[pi_cur]
	inc	edx
	movzx	esi,byte[edx-1]
	cmp	byte[edx],'-'
	jne	.lp1
	pushd	[ecx+0xC]
	sub	dword[ecx+0xC],POP_IHEIGHT-4
	call	pt_in_rect
	popd	[ecx+0xC]
	jc	.separator
	add	dword[ecx+0x4],4
	add	dword[ecx+0xC],4
	jmp	.lp3
  .lp1: call	pt_in_rect
	jnc	.lp2
	mov	eax,[pi_cur]
	test	byte[ebp+eax-1],1;byte[popup_text+eax-1],1
	jnz	.exit
	jmp	.separator
  .lp2: add	dword[ecx+0x4],POP_IHEIGHT
	add	dword[ecx+0xC],POP_IHEIGHT
	add	edx,esi
	inc	edx
	movzx	esi,byte[edx-1]
  .lp3: add	edx,esi
	cmp	byte[edx],0
	jne	@b
  .separator:
	mov	[pi_cur],0
  .exit:
	popad
	ret
  .outside_window:
	or	[pi_cur],-1
	jmp	.exit
endf

;-----------------------------------------------------------------------------
func line_add_spaces ;////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  ESI = line offset
;  ECX = needed line length
; Output:
;  EAX = delta
;-----------------------------------------------------------------------------
	xor	eax,eax
	pushad
	movzx	edx,word[esi]
	cmp	ecx,edx
	jbe	.exit
	sub	ecx,edx
	lea	eax,[ecx+4]
	call	editor_realloc_lines
	mov	[esp+4*7],eax
	add	esi,eax
	push	ecx
	mov	edi,[cur_editor.Lines] ;! AREA_TEMP2
	add	edi,[edi-4]
	dec	edi
	mov	eax,esi
	mov	esi,edi
	sub	esi,ecx
	lea	ecx,[eax+4]
	add	ecx,edx;[eax]
	push	ecx
	neg	ecx
	lea	ecx,[esi+ecx+1]
	std
	rep	movsb
	pop	edi ecx
	add	[eax],cx
	mov	al,' '
	cld
	rep	stosb
  .exit:
	popad
	ret
endf

;-----------------------------------------------------------------------------
func delete_selection ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
;       call    init_sel_vars

	cmp	[sel.selected],0
	je	.exit.2

	pushad
	mov	ecx,[sel.begin.y]
	cmp	ecx,[sel.end.y]
	je	.single_line
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000
	mov	ecx,[sel.begin.x]
	call	line_add_spaces
	add	esi,eax
	lea	edi,[esi+4]
	mov	ecx,[sel.end.y]
	call	get_line_offset
	call	get_real_length
	cmp	eax,[sel.end.x]
	jbe	@f
	mov	eax,[sel.end.x]
    @@: movzx	ecx,word[esi]
	sub	ecx,eax
	mov	ebx,[sel.begin.x]
	add	ebx,ecx
	mov	[edi-4],bx
	add	edi,[sel.begin.x]
	lea	esi,[esi+eax+4]
	mov	ecx,[cur_editor.Lines] ;! AREA_TEMP2
	add	ecx,[ecx-4]
	sub	ecx,esi
	cld
	rep	movsb
	mov	eax,[sel.end.y]
	sub	eax,[sel.begin.y]
	sub	[cur_editor.Lines.Count],eax ;! [lines],eax
	jmp	.exit

  .single_line:
	call	get_line_offset
	and	dword[esi],not 0x00020000
	or	dword[esi],0x00010000
	call	get_real_length
	cmp	eax,[sel.begin.x]
	jbe	.exit
	mov	ecx,[sel.end.x]
	cmp	ecx,eax
	jbe	@f
	mov	ecx,eax
    @@: sub	ecx,[sel.begin.x]
	sub	[esi],cx
	lea	edi,[esi+4]
	add	edi,[sel.begin.x]
	lea	esi,[edi+ecx]
	mov	ecx,[cur_editor.Lines] ;! AREA_TEMP2
	add	ecx,[ecx-4]
	sub	ecx,esi
	cld
	rep	movsb

  .exit:
	mov	eax,[sel.begin.x]
	mov	[cur_editor.Caret.X],eax ;! [pos.x],eax
	mov	[cur_editor.SelStart.X],eax ;! [sel.x],eax
	mov	eax,[sel.begin.y]
	mov	[cur_editor.Caret.Y],eax ;! [pos.y],eax
	mov	[cur_editor.SelStart.Y],eax ;! [sel.y],eax

	mov	ecx,[cur_editor.Lines.Count]
	call	get_line_offset
	movzx	eax,word[esi]
	lea	esi,[esi+eax+4]
	mov	eax,[cur_editor.Lines]
	add	eax,[eax-4]
	sub	esi,eax
	lea	eax,[esi+4096]
	call	editor_realloc_lines

	popad
	mov	[cur_editor.Modified],1 ;! [modified],1
	clc
	ret

  .exit.2:
	stc
	ret
endf

;-----------------------------------------------------------------------------
func get_selection_size ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ecx esi
	mov	ecx,[sel.end.y]
	inc	ecx
	call	get_line_offset
	mov	eax,esi
	mov	ecx,[sel.begin.y]
	call	get_line_offset
	sub	eax,esi
	pop	esi ecx
	ret
endf

;-----------------------------------------------------------------------------
func get_lines_in_file ;//////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  ESI = data pointer
;  ECX = data length
; Output:
;  EAX = lines number
;  EBX = extra length after tabs expansion
;-----------------------------------------------------------------------------
	push	ecx edx esi 0
	or	ebx,-1
	xor	edx,edx
  .lp0: inc	ebx
  .lp1: dec	ecx
	jle	.lp2
	lodsb
	cmp	al,0
	je	.lp2
	cmp	al,9
	je	.TB
	cmp	al,10
	je	.LF
	cmp	al,13
	je	.CR
	inc	edx
	jmp	.lp1
  .lp2: lea	eax,[ebx+1]
	pop	ebx esi edx ecx
	ret

   .CR: cmp	byte[esi],10
	jne	.LF
	lodsb
   .LF: xor	edx,edx
	jmp	.lp0
   .TB: and	edx,00000111b
	add	dword[esp],ATABW
	sub	[esp],edx
	xor	edx,edx
	jmp	.lp1
endf

;-----------------------------------------------------------------------------
func mem.Alloc ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx
	lea	ecx,[eax+4+4095]
	and	ecx,not 4095
	mcall	68,12
	add	ecx,-4
	mov	[eax],ecx
	add	eax,4
	pop	ecx ebx
	ret
endf

;-----------------------------------------------------------------------------
func mem.ReAlloc ;////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx esi edi eax
	or	eax,eax
	jz	@f
	lea	ecx,[ebx+4+4095]
	and	ecx,not 4095
	add	ecx,-4
	cmp	ecx,[eax-4]
	je	.exit
    @@: mov	eax,ebx
	call	mem.Alloc
	xchg	eax,[esp]
	or	eax,eax
	jz	.exit
	mov	esi,eax
	xchg	eax,[esp]
	mov	edi,eax
	mov	ecx,[esi-4]
	cmp	ecx,[edi-4]
	jbe	@f
	mov	ecx,[edi-4]
    @@: add	ecx,3
	shr	ecx,2
	cld
	rep	movsd
	xchg	eax,[esp]
	call	mem.Free
  .exit:
	pop	eax edi esi ecx ebx
	ret
endf

;-----------------------------------------------------------------------------
func mem.Free ;///////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx
	lea	ecx,[eax-4]
	mcall	68,13
	pop	ecx ebx
	ret
endf
