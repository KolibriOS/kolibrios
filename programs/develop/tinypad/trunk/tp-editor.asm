
;-----------------------------------------------------------------------------
func draw_editor ;///// DRAW EDITOR //////////////////////////////////////////
;-----------------------------------------------------------------------------

	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,word[cur_editor.Bounds.Right]
	sub	bx,word[cur_editor.Bounds.Left]
	inc	ebx
	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,word[cur_editor.Bounds.Bottom]
	sub	cx,word[cur_editor.Bounds.Top]
	inc	ecx
	mov	edx,[cl_3d_inset]
	call	draw_framerect
@^
	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,word[cur_editor.Bounds.Right]
	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,word[cur_editor.Bounds.Top]
	mcall	38,,,[cl_3d_inset]
	mov	ecx,[cur_editor.Bounds.Bottom-2]
	mov	cx,word[cur_editor.Bounds.Bottom]
	mcall
	mov	bx,word[cur_editor.Bounds.Left]
	mov	cx,word[cur_editor.Bounds.Top]
	mcall
	mov	ebx,[cur_editor.Bounds.Right-2]
	mov	bx,word[cur_editor.Bounds.Right]
	mov	cx,word[cur_editor.Bounds.Top]
	mcall
^@
	mov	[cur_editor.Gutter.Visible],0
	test	[options],OPTS_LINENUMS
	jnz	@f
	xor	eax,eax ;! mov eax,2+LCHGW
	jmp	.lp1
    @@: inc	[cur_editor.Gutter.Visible]
	mov	edi,p_info+100
	mov	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	mov	ecx,10
	call	uint2str
	lea	eax,[edi-p_info-100]
	cmp	eax,3
	jae	@f
	mov	eax,3
    @@: imul	eax,6
	add	eax,8
  .lp1: mov	[cur_editor.Gutter.Width],eax ;! [left_ofs],eax
	mov	[left_ofs],eax

	call	draw_editor_gutter
	call	draw_editor_vscroll
	call	draw_editor_hscroll
	call	draw_editor_text
	call	draw_editor_caret

	ret
endf

;-----------------------------------------------------------------------------
func draw_editor_gutter ;///// DRAW EDITOR GUTTER (LEFT PANEL) ///////////////
;-----------------------------------------------------------------------------
	cmp	[cur_editor.Gutter.Visible],0
	je	.exit

	add	esp,-4*8*2

	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,word[cur_editor.Gutter.Width]
	add	ebx,0x00010000
	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,word[cur_editor.Bounds.Bottom]
	sub	cx,word[cur_editor.Bounds.Top]
	add	cx,-SCRLW
	add	ecx,0x00010000
	dec	cx
	mcall	13,,,[cl_3d_normal]

	add	bx,word[cur_editor.Bounds.Left]
	push	bx
	shl	ebx,16
	pop	bx
	add	ecx,[cur_editor.Bounds.Top]
	mcall	38,,,[cl_3d_inset]

	add	ebx,-2*65536
	mov	bx,word[cur_editor.Bounds.Top]
	add	bx,3
	mov	edi,[sc.work_text]
	mov	ecx,[cur_editor.TopLeft.Y]
	inc	ecx
	mov	edx,p_info+100
    @@: pushad
	push	eax edx edi
	mov	eax,ecx
	mov	ecx,10
	mov	edi,edx
	call	uint2str
	mov	esi,edi
	pop	edi edx eax
	sub	esi,edx
	imul	eax,esi,6*65536
	sub	ebx,eax
	mcall	4,,edi
	popad
	add	ebx,LINEH
	inc	ecx
	cmp	ecx,[cur_editor.Lines.Count]
	jg	@f
	mov	esi,ecx
	sub	esi,[cur_editor.TopLeft.Y]
	cmp	esi,[lines.scr]
	jbe	@b
    @@: add	esp,4*8*2

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func draw_editor_vscroll ;///// DRAW EDITOR VERTICAL SCROLL BAR //////////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Bounds.Right]
	shl	ebx,16
	add	ebx,(-SCRLW)*65536+SCRLW
	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,SCRLW
	mcall	8,,,'VSL' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
	mov	eax,8

	pushad
	push	0x18
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

	mov	ecx,[cur_editor.Bounds.Bottom]
	shl	ecx,16
	add	ecx,(-SCRLW*2)*65536+SCRLW
	mcall	,,,'VSG' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
	mov	eax,8

	pushad
	push	0x19
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

	push	ebx
	mov	eax,[cur_editor.Lines.Count]
	mov	ebx,[lines.scr]
	mov	ecx,[cur_editor.TopLeft.Y]
	mov	edx,[cur_editor.Bounds.Bottom]
	sub	edx,[cur_editor.Bounds.Top]
	add	edx,-SCRLW*3+1
	call	get_scroll_vars
	mov	[cur_editor.VScroll.Top],eax
	mov	[cur_editor.VScroll.Size],ebx
	pop	ebx

	mov	ecx,eax
	add	ecx,[cur_editor.Bounds.Top]
	add	ecx,SCRLW+1

	pushad
	sar	ebx,16
	push	ebx ecx SCRLW [cur_editor.VScroll.Size]
	dec	dword[esp]
	call	draw_3d_panel
	popad
	mov	eax,13
	add	ebx,1*65536-1

	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,word[cur_editor.VScroll.Top]
	add	ecx,(SCRLW+1)*65536
	mov	edx,[sc.work]
	or	cx,cx
	jle	@f
	mcall	13
    @@:
	mov	ecx,[cur_editor.Bounds.Top]
	add	ecx,[cur_editor.VScroll.Top]
	add	ecx,[cur_editor.VScroll.Size]
	add	ecx,SCRLW+1
	mov	di,cx
	shl	ecx,16
	mov	cx,word[cur_editor.Bounds.Bottom]
	sub	cx,di
	sub	cx,SCRLW*2;+1
	jle	@f
	mcall
    @@:
	rol	ebx,16
	dec	bx
	push	bx
	rol	ebx,16
	pop	bx
	mov	ecx,[cur_editor.Bounds.Top-2]
	mov	cx,word[cur_editor.Bounds.Bottom]
	add	ecx,(SCRLW)*65536-SCRLW*2-1
	mcall	38,,,[cl_3d_inset]

	ret
endf

;-----------------------------------------------------------------------------
func draw_editor_hscroll ;///// DRAW EDITOR HORIZONTAL SCROLL BAR ////////////
;-----------------------------------------------------------------------------
	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,SCRLW
	mov	ecx,[cur_editor.Bounds.Bottom]
	shl	ecx,16
	add	ecx,(-SCRLW)*65536+SCRLW
	mcall	8,,,'HSL' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad

	pushad
	push	0x1B
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

	mov	ebx,[cur_editor.Bounds.Right]
	shl	ebx,16
	add	ebx,(-SCRLW*2)*65536+SCRLW
	mcall	8,,,'HSG' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad

	pushad
	push	0x1A
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

	push	ecx
	mov	eax,[cur_editor.Columns.Count]
	mov	ebx,[columns.scr]
	mov	ecx,[cur_editor.TopLeft.X]
	mov	edx,[cur_editor.Bounds.Right]
	add	edx,-(SCRLW*3)
	call	get_scroll_vars
	mov	[cur_editor.HScroll.Top],eax
	mov	[cur_editor.HScroll.Size],ebx
	pop	ecx

	mov	ebx,eax
	add	ebx,[cur_editor.Bounds.Left]
	add	ebx,SCRLW+1
	shl	ebx,16
	mov	bx,word[cur_editor.HScroll.Size]

	pushad
	sar	ecx,16
	rol	ebx,16
	movsx	eax,bx
	sar	ebx,16
	dec	ebx
	push	eax ecx ebx SCRLW
	call	draw_3d_panel
	popad
	add	ecx,1*65536-1

	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,word[cur_editor.Bounds.Left]
	mov	bx,word[cur_editor.HScroll.Top]
	add	ebx,(1+SCRLW)*65536
	mcall	13,,,[sc.work]
	mov	ebx,[cur_editor.Bounds.Left]
	add	ebx,1+SCRLW
	add	ebx,[cur_editor.HScroll.Top]
	add	ebx,[cur_editor.HScroll.Size]
	mov	di,bx
	shl	ebx,16
	mov	bx,word[cur_editor.Bounds.Right]
	sub	bx,di
	sub	bx,SCRLW*2
	jle	@f
	mcall
    @@:
	mov	ebx,[cur_editor.Bounds.Left-2]
	mov	bx,word[cur_editor.Bounds.Right]
	add	ebx,(SCRLW)*65536-SCRLW*2-1
	rol	ecx,16
	dec	cx
	push	cx
	rol	ecx,16
	pop	cx
	mcall	38,,,[cl_3d_inset]

	ret
endf

;-----------------------------------------------------------------------------
func draw_editor_text ;///// DRAW EDITOR TEXT ////////////////////////////////
;-----------------------------------------------------------------------------
	mov	eax,[cur_editor.Bounds.Bottom]
	sub	eax,[cur_editor.Bounds.Top]
	cmp	eax,LINEH
	jge	@f
	ret
    @@:
	call	init_sel_vars
	call	check_bottom_right

	pushad

	mov	eax,[cur_editor.Bounds.Left]
	add	eax,[cur_editor.Gutter.Width]
	add	eax,LCHGW+3
	mov	[left_ofs],eax
	mov	eax,[cur_editor.Bounds.Top]
	add	eax,3
	mov	[top_ofs],eax

	mov	ebx,[top_ofs]
	add	ebx,[left_ofs-2]

	mov	ecx,[cur_editor.TopLeft.Y] ;! ecx,[top_line]
	push	ecx
	call	get_line_offset

  .start:
	add	esp,4
	mov	ecx,[lines.scr]
	or	ecx,ecx
	jle	.exit
	add	esp,-4

	imul	ebp,[cur_editor.TopLeft.X],6*65536 ;! ebp,[left_col],6*65536
	mov	eax,[lines.scr]
	sub	eax,[cur_editor.Lines.Count] ;! eax,[lines]
	mov	[draw_blines],eax

  .next_line:

	push	ecx ebx

	mov	ecx,ebx
	shl	ecx,16
	mov	cl,LINEH
	mov	ebx,[cur_editor.Bounds.Right]
	;sub     ebx,[cur_editor.Bounds.Left]
	add	ebx,-SCRLW
	add	ebx,[left_ofs-2]
	sub	ebx,[left_ofs]
	add	ebx,-2*65536+2

  ; selection (text background)
	mov	[in_sel],0
	mov	edx,[color_tbl+4*5]
	mov	eax,[esp+4*2]
	cmp	eax,[sel.begin.y]
	jl	.lp6
	je	.lp1
	cmp	eax,[sel.end.y]
	jg	.lp6
	je	.lp3
	jmp	.lp6.2
  .lp1: mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.lp5
  .lp2: mov	eax,[sel.begin.x]
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	jle	.lp6.2
	cmp	eax,[columns.scr]
	jge	.lp6
	imul	eax,6
	pushad
	sub	bx,ax
	rol	ebx,16
	mov	bx,ax
	add	ebx,[left_ofs]
	add	ebx,-2
	rol	ebx,16
	mov	edx,[color_tbl+4*7]
	mcall	13
	popad
	mov	bx,ax
	mov	[in_sel],2
	jmp	.lp6
  .lp3: mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.lp5
  .lp4: mov	eax,[sel.end.x]
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	jle	.lp6
	cmp	eax,[columns.scr]
	jg	.lp6.2
	imul	eax,6
	pushad
	sub	bx,ax
	rol	ebx,16
	add	eax,[left_ofs];OLEFT-1
	add	eax,-2
	mov	bx,ax
	rol	ebx,16
	mcall	13
	popad
	inc	eax
	mov	edx,[color_tbl+4*7]
	mov	bx,ax
	mov	[in_sel],3
	jmp	.lp6
  .lp5: mov	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	cmp	eax,[sel.begin.x]
	jge	.lp4
	add	eax,[columns.scr]
	cmp	eax,[sel.end.x]
	jl	.lp2
	mov	eax,[sel.begin.x]
	cmp	eax,[sel.end.x]
	je	.lp6
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	imul	eax,6
	pushad
	mov	ebx,[sel.end.x]
	sub	ebx,[sel.begin.x]
	imul	ebx,6
	sal	ebx,16
	dec	eax
	add	eax,[left_ofs]
	mov	bx,ax
	rol	ebx,16
	mov	edx,[color_tbl+4*7]
	mcall	13
	movzx	eax,bx
	sar	ebx,16
	add	eax,ebx
	mov	ebx,eax
	sal	ebx,16
	sub	ax,[esp+4*4]
	neg	ax
	add	ax,word[left_ofs]
	add	ax,-2
	mov	bx,ax
	mov	edx,[color_tbl+4*5]
	mcall	13
	popad
	mov	bx,ax
	mov	[in_sel],4
	jmp	.lp6

  .lp6.2:
	mov	edx,[color_tbl+4*7]
	inc	[in_sel]
  .lp6:
	mcall	13

	lodsd

	pushad
	mov	edx,[color_tbl+4*5]
	test	eax,0x00010000
	jz	@f
	mov	edx,[color_tbl+4*8]
	test	eax,0x00020000
	jz	@f
	mov	edx,[color_tbl+4*9]
    @@: mov	ebx,[left_ofs]

	add	ebx,-LCHGW-2;-4
	shl	ebx,16
	mov	bx,LCHGW
	mcall	13
	popad

	xor	ecx,ecx
	and	eax,0x0000FFFF
	mov	[cur_line_len],eax

	or	eax,eax
	ja	.next_block
	add	esp,4*2
	jmp	.exit ; .draw_cursor

  .next_block:

	push	esi ecx
	call	get_next_part
	pop	ebx

	push	ecx
	mov	ecx,eax

	push	esi ebx
	mov	eax,ebx
	sub	ebx,[cur_editor.TopLeft.X] ;! ebx,[left_col]
	cmp	ebx,[columns.scr]
	jge	.skip_t
	add	ebx,esi
	jle	.skip_t
	mov	ebx,[esp+8+4*2] ;// 4*2=esi+ebx
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	jge	.qqq
	sub	edx,eax
	add	esi,eax
;       mov     eax,OLEFT*65536
	xor	eax,eax
	jmp	.qqq2
  .qqq:
;       inc     eax
	imul	eax,6*65536
  .qqq2:
	and	ebx,0x0000FFFF
	add	eax,[left_ofs-2];OLEFT*65536
	add	ebx,eax

	mov	eax,[esp]   ; ebx
	add	eax,[esp+4] ; esi
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	sub	eax,[columns.scr]
	jle	.qweqwe
	sub	esi,eax
  .qweqwe:

	mov	al,[in_sel]
	cmp	al,0
	je	.draw_t
	dec	al
	jz	.ya4
  .nt1: dec	al
	jnz	.nt2
	mov	eax,[esp]
	cmp	eax,[sel.begin.x]
	jge	.ya4
	add	eax,[esp+4]
	cmp	eax,[sel.begin.x]
	jl	.draw_t
;---[ selection crosses block from the right ]-(-
  .ya1: mov	eax,esi
	mov	esi,[sel.begin.x]
	sub	esi,[esp]
	pushad
	mov	ecx,[cur_editor.TopLeft.X] ;! ecx,[left_col]
	sub	ecx,[esp+4*8]
	jle	@f
	sub	esi,ecx
	sub	[esp+4],ecx
    @@: sub	eax,esi
	add	edx,esi
	imul	esi,6
	rol	ebx,16
	add	bx,si
	rol	ebx,16
	mov	esi,eax
	mov	ecx,[color_tbl+4*6]
	mcall	4
	popad
	jmp	.draw_t
;----------------------------------------------)-
  .nt2: dec	al
	jnz	.nt3
	mov	eax,[esp]
	cmp	eax,[sel.end.x]
	jge	.draw_t
	add	eax,[esp+4]
	cmp	eax,[sel.end.x]
	jl	.ya4
;---[ selection crosses block from the left ]--(-
  .ya2: mov	eax,[sel.end.x]
	sub	eax,[esp]
	push	ebx
	mov	ebx,[esp+4]
	sub	ebx,[cur_editor.TopLeft.X] ;! ebx,[left_col]
	jge	.ya2.1
	add	eax,ebx
  .ya2.1:
	pop	ebx
	pushad
	mov	esi,eax
	mov	ecx,[color_tbl+4*6]
	mcall	4
	popad
	sub	esi,eax
	add	edx,eax
	imul	eax,6*65536
	add	ebx,eax
	jmp	.draw_t
;----------------------------------------------)-
  .nt3: mov	eax,[esp]
	cmp	eax,[sel.end.x]
	jge	.draw_t
	cmp	eax,[sel.begin.x]
	jge	@f
	add	eax,[esp+4]
	cmp	eax,[sel.begin.x]
	jl	.draw_t
	cmp	eax,[sel.end.x]
	jl	.ya1
;---[ selection inside block ]-----------------(-
	mov	eax,esi
	mov	esi,[sel.begin.x]
	sub	esi,[esp]
	push	eax
	mov	eax,[esp+4]
	sub	eax,[cur_editor.TopLeft.X] ;! eax,[left_col]
	jge	.nt3.1
	add	esi,eax
  .nt3.1:
	pop	eax
	sub	eax,esi
	pushad
	add	edx,esi
	imul	esi,6*65536
	add	ebx,esi
	mov	esi,[sel.end.x]
	sub	esi,[sel.begin.x]
	mov	ecx,[color_tbl+4*6]
	sub	eax,esi
	push	eax
	mcall	4
	add	edx,esi
	imul	esi,6*65536
	add	ebx,esi
	pop	esi
	mov	ecx,[esp+4*6]
	mcall	4
	popad
	jmp	.draw_t
;----------------------------------------------)-
    @@: add	eax,esi
	dec	eax
	cmp	eax,[sel.end.x]
	jge	.ya2
;---[ block inside selection ]-----------------(-
  .ya4: mov	ecx,[color_tbl+4*6]
;----------------------------------------------)-

  .draw_t:
	mcall	4;[esp+8]
  .skip_t:
	pop	eax eax ; ebx esi
	imul	eax,6
	add	[esp+4*2+2],ax
	pop	ecx esi
	cmp	ecx,[cur_line_len];LINE_WIDTH
	jl	.next_block

	pop	ebx ecx
	and	ebx,0x0000FFFF
	add	ebx,[left_ofs-2]
	add	ebx,LINEH
	add	esi,[cur_line_len];LINE_WIDTH
	inc	dword[esp]
	dec	ecx
	jg	.next_line

  .exit:

	cmp	[draw_blines],0
	jl	@f
	mov	ecx,[esp-8]
	shl	ecx,16
	mov	cx,word[cur_editor.Bounds.Bottom]
	sub	cx,[esp-8]
	add	cx,-SCRLW
	mov	eax,[cur_editor.Bounds.Left]
	add	eax,[cur_editor.Gutter.Width]
	inc	eax
	mov	ebx,eax
	shl	ebx,16
	mov	bx,word[cur_editor.Bounds.Right]
	sub	bx,ax
	add	ebx,-SCRLW
	mcall	13,,,[color_tbl+4*5]
    @@:

	popad
	add	esp,4
	ret
endf

;-----------------------------------------------------------------------------
func draw_editor_caret ;///// DRAW EDITOR TEXT CARET /////////////////////////
;-----------------------------------------------------------------------------
	cmp	[bot_mode],0
	jne	@f
	mov	ebx,[cur_editor.Caret.X]
	sub	ebx,[cur_editor.TopLeft.X]
	js	@f
	cmp	ebx,[columns.scr]
	ja	@f
	imul	ebx,6
	add	ebx,[left_ofs]
	dec	ebx
	push	bx
	shl	ebx,16
	pop	bx
	mov	eax,[cur_editor.Caret.Y]
	sub	eax,[cur_editor.TopLeft.Y]
	js	@f
	cmp	eax,[lines.scr]
	jge	@f
	imul	eax,LINEH
	add	eax,[top_ofs]
	mov	esi,eax
	shl	esi,16
	add	eax,LINEH-2
	mov	si,ax
	mov	ecx,2
	cmp	[ins_mode],0
	jne	.lp8
	add	cl,4
  .lp8: push	ecx
	mcall	38,,esi,0x01000000
	add	ebx,0x00010001
	pop	ecx
	loop	.lp8
    @@:
	ret
endf