;-----------------------------------------------------------------------------
func drawwindow ;///// DRAW WINDOW ///////////////////////////////////////////
;-----------------------------------------------------------------------------

	cmp	[just_from_popup],1
	jne	@f
	ret
    @@:

	mcall	48,3,sc,sizeof.system_colors
	call	calc_3d_colors

	test	[options],OPTS_LINENUMS
	jnz	@f
	mov	eax,2+LCHGW
	jmp	.lp1
    @@: mov	edi,p_info+100
	mov	eax,[lines]
	mov	ecx,10
	call	uint2str
	lea	eax,[edi-p_info-100]
	cmp	eax,3
	jae	@f
	mov	eax,3
    @@: imul	eax,6
	add	eax,2+4+LCHGW
  .lp1: mov	[left_ofs],eax

	mcall	12,1

	push	[color_tbl+4*5]
	pop	[sc.work]

	mov	edx,[sc.work]
	add	edx,0x33000000
	mov	ebx,[mainwnd_pos.x-2]
	mov	bx,word[mainwnd_pos.w]
	mov	ecx,[mainwnd_pos.y-2]
	mov	cx,word[mainwnd_pos.h]
	mcall	0,,,,,s_title

	mcall	9,p_info,-1
	mov	esi,p_info.box.left
	mov	edi,mainwnd_pos
	mov	ecx,4
	cld
	rep	movsd

;	mcall	9,p_info,-1
	cmp	[p_info.client_box.height],LINEH
	jl	.exit.2

	mov	[top_ofs],ATOPH;+1

	mov	eax,[p_info.client_box.width]
	sub	eax,SCRLW+1
	sub	eax,[left_ofs]
	cdq
	mov	ebx,6
	div	ebx
	mov	[columns.scr],eax

	mov	eax,[p_info.client_box.height] ; calculate buttons position
	add	eax,-STATH;*3-2-2
	sub	eax,[bot_dlg_height]
	mov	[bot_ofs],eax

	call	draw_bottom_dialog

	mov	[do_not_draw],1 ; do_not_draw = true

	mov	ebx,eax
	sub	ebx,[top_ofs]
	sub	ebx,SCRLW*3+AMINS+5
	js	.no_draw

	dec	[do_not_draw]	 ; do_not_draw = false
	sub	eax,SCRLW+3
	sub	eax,[top_ofs]
	cdq
	mov	ebx,LINEH
	div	ebx
	mov	[lines.scr],eax

	mov	ebx,[p_info.client_box.width]
	mov	ecx,[top_ofs-2]
	mov	cx,word[top_ofs]
	sub	ecx,1*65536+1
	mcall	38,,,[cl_3d_inset];[sc.work_text]
	mov	ecx,[p_info.client_box.height]
	sub	ecx,STATH+1
	push	cx
	shl	ecx,16
	pop	cx
	mcall

	inc	[top_ofs]

	call	draw_main_menu

	jmp	.exit

  .no_draw:
	mov	[top_ofs],2
	mov	eax,[p_info.client_box.height]
	inc	eax
	mov	[bot_ofs],eax
	sub	eax,2
	push	eax
	add	eax,-2-SCRLW
	cdq
	mov	ebx,LINEH
	idiv	ebx
	mov	[lines.scr],eax
	pop	eax

	mov	ebx,[p_info.client_box.height]
	sub	ebx,SCRLW*3+AMINS+2
	jns	@f

	inc	[do_not_draw]

	dec	eax
	cdq
	mov	ebx,LINEH
	idiv	ebx
	mov	[lines.scr],eax

	mov	eax,[p_info.client_box.width]
	cdq
	mov	ebx,6
	idiv	ebx
	mov	[columns.scr],eax
    @@:

  .exit:
	call	draw_file
  .exit.2:
	mcall	12,2
	ret
endf

;-----------------------------------------------------------------------------
func draw_bottom_dialog ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[bot_dlg_height],0
	je	.exit
	pushad
	mov	ebx,[p_info.client_box.width]
	mov	ecx,[bot_ofs]
	dec	ecx
	push	cx
	shl	ecx,16
	pop	cx
	mcall	38,,,[cl_3d_inset]
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_dlg_height]
	dec	ecx
	mov	ebx,[p_info.client_box.width]
	inc	ebx
	mcall	13,,,[cl_3d_normal]
	mov	al,1
	call	[bot_dlg_handler]
	popad

  .exit:
	ret
endf

mi_sel	 dd ?
mi_cur	 dd -1

;-----------------------------------------------------------------------------
func draw_main_menu ;/////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	ebx,[p_info.client_box.width]
	inc	ebx
	mcall	13,,ATOPH-1,[cl_3d_normal]

	mov	edx,main_menu
	mov	ebx,9*65536+ATOPH/2-4
	mov	ecx,[sc.work_text]
	mov	[mi_sel],0
	mov	edi,[mi_cur]
    @@: inc	[mi_sel]
	cmp	[mi_sel],main_menu.cnt_item
	ja	.exit
	cmp	edi,[mi_sel]
	jne	.lp1
	pushad
	push	edx
	mcall	13,[edx+0],[edx+4],[cl_3d_pushed]
	mov	edx,[esp]
	mov	bx,[edx+2]
	mcall	38,,,[cl_3d_inset]
	pop	edx
	movzx	eax,word[edx]
	add	ebx,eax
	shl	eax,16
	add	ebx,eax
	mcall	38,,,[cl_3d_inset]
	popad
  .lp1: add	edx,8+1
	movzx	esi,byte[edx-1]
	mcall	4
	add	edx,esi
	add	esi,2
	imul	esi,6*65536
	add	ebx,esi
	jmp	@b

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func draw_file.ex ;///////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  EAX = start line
;  EBX = end line
;-----------------------------------------------------------------------------
	cmp	[p_info.client_box.height],LINEH
	jge	@f
	ret
    @@:
	call	init_sel_vars
	call	check_bottom_right

	pushad

	cmp	[lines.scr],0
	jle	draw_file.exit

	cmp	eax,ebx
	jle	@f
	xchg	eax,ebx
    @@: cmp	eax,[top_line]
	jge	@f
	mov	eax,[top_line]
    @@: mov	ecx,[top_line]
	add	ecx,[lines.scr]
	cmp	ebx,ecx
	jl	@f
	dec	ecx
	mov	ebx,ecx
    @@: cmp	eax,ebx
	jg	draw_file.exit

	mov	ecx,eax
	push	eax
	call	get_line_offset

  .start:
	mov	ecx,ebx
	sub	ecx,eax
	inc	ecx

	mov	ebx,[top_ofs]
	add	ebx,[left_ofs-2]
	sub	eax,[top_line]
	imul	eax,LINEH
	add	ebx,eax

	imul	ebp,[left_col],6*65536
	or	[draw_blines],-1

	jmp	draw_file.next_line
endf

;-----------------------------------------------------------------------------
func draw_file ;//////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[p_info.client_box.height],LINEH
	jge	@f
	ret
    @@:
	call	init_sel_vars
	call	check_bottom_right

	pushad

	mov	ebx,[top_ofs]
	add	ebx,[left_ofs-2]

	mov	ecx,[top_line]
	push	ecx
	call	get_line_offset

  .start:
	add	esp,4
	mov	ecx,[lines.scr]
	or	ecx,ecx
	jle	.exit
	add	esp,-4

	imul	ebp,[left_col],6*65536
	mov	eax,[lines.scr]
	sub	eax,[lines]
	mov	[draw_blines],eax

  .next_line:

	push	ecx ebx

	mov	ecx,ebx
	shl	ecx,16
	mov	cl,LINEH
	mov	ebx,[p_info.client_box.width]
	add	ebx,-SCRLW
	add	ebx,[left_ofs-2]
	sub	ebx,[left_ofs]
	add	ebx,-1*65536+1

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
	sub	eax,[left_col]
	jle	.lp6.2
	cmp	eax,[columns.scr]
	jge	.lp6
	imul	eax,6
	pushad
	sub	bx,ax
	rol	ebx,16
	mov	bx,ax
	add	ebx,[left_ofs];OLEFT-1
	dec	ebx
	rol	ebx,16
	mov	edx,[color_tbl+4*7]
	mcall	13
	popad
;       inc     eax
	mov	bx,ax
	mov	[in_sel],2
	jmp	.lp6
  .lp3: mov	eax,[sel.begin.y]
	cmp	eax,[sel.end.y]
	je	.lp5
  .lp4: mov	eax,[sel.end.x]
	sub	eax,[left_col]
	jle	.lp6
	cmp	eax,[columns.scr]
	jg	.lp6.2
	imul	eax,6
	pushad
	sub	bx,ax
	rol	ebx,16
	add	eax,[left_ofs];OLEFT-1
	dec	eax
	mov	bx,ax
	rol	ebx,16
;       inc     ebx
	mcall	13
	popad
	mov	edx,[color_tbl+4*7]
	mov	bx,ax
	mov	[in_sel],3
	jmp	.lp6
  .lp5: mov	eax,[left_col]
	cmp	eax,[sel.begin.x]
	jge	.lp4
	add	eax,[columns.scr]
	cmp	eax,[sel.end.x]
	jl	.lp2
	mov	eax,[sel.begin.x]
	cmp	eax,[sel.end.x]
	je	.lp6
	sub	eax,[left_col]
	imul	eax,6
	pushad
	mov	ebx,[sel.end.x]
	sub	ebx,[sel.begin.x]
;       inc     ebx
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
	add	ebx,-4
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
	jmp	.draw_cursor

  .next_block:

	push	esi ecx
	call	get_next_part
	pop	ebx

	push	ecx
	mov	ecx,eax

	push	esi ebx
	mov	eax,ebx
	sub	ebx,[left_col]
	cmp	ebx,[columns.scr]
	jge	.skip_t
	add	ebx,esi
	jle	.skip_t
	mov	ebx,[esp+8+4*2] ;// 4*2=esi+ebx
	sub	eax,[left_col]
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
	sub	eax,[left_col]
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
	mov	ecx,[left_col]
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
	sub	ebx,[left_col]
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
	sub	eax,[left_col]
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

;------------------------------------------------
  .draw_cursor:

;------------------------------------------------
	mov	ebx,[left_ofs]
	cmp	ebx,2+LCHGW
	je	.no_gutter
	add	esp,-4*8*2
	sub	ebx,3+LCHGW
	mov	ecx,[top_ofs]
	dec	ecx
	shl	ecx,16
	add	ecx,[bot_ofs]
	sub	ecx,[top_ofs]
	sub	ecx,SCRLW
	mcall	13,,,[cl_3d_normal]

	push	bx
	shl	ebx,16
	pop	bx
	add	ecx,[top_ofs]
	dec	ecx
	mcall	38,,,[cl_3d_inset]

	mov	ebx,[left_ofs]
	add	ebx,-3-LCHGW
	shl	ebx,16
	add	ebx,[top_ofs]
	mov	edi,[sc.work_text]
	mov	ecx,[top_line]
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
	cmp	ecx,[lines]
	jg	@f
	mov	esi,ecx
	sub	esi,[top_line]
	cmp	esi,[lines.scr]
	jbe	@b
    @@: add	esp,4*8*2

  .no_gutter:
;------------------------------------------------

	mov	ebx,[draw_blines]
	or	ebx,ebx
	js	@f
	imul	ebx,LINEH
	mov	ecx,[esp-8]
	shl	ecx,16
	mov	cx,bx
	mov	ebx,[p_info.client_box.width]
	add	ebx,[left_ofs-2]
	sub	ebx,[left_ofs]
	add	ebx,-1*65536-SCRLW+1 ; <OLEFT-1,LINE_WIDTH*6+2>
	mcall	13,,,[color_tbl+4*5]
    @@:

	add	esp,4
	cmp	[bot_mode],0
	jne	@f
	mov	ebx,[pos.x]
	sub	ebx,[left_col]
	js	@f
	cmp	ebx,[columns.scr]
	ja	@f
	imul	ebx,6
	add	ebx,[left_ofs]
	dec	ebx
	push	bx
	shl	ebx,16
	pop	bx
	mov	eax,[pos.y]
	sub	eax,[top_line]
	js	@f
	cmp	eax,[lines.scr]
	jge	@f
	imul	eax,LINEH
	add	eax,[top_ofs]
;       inc     eax
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
;------------------------------------------------
	cmp	[do_not_draw],2
	je	.exit

;        mov     ebx,[p_info.x_size]
;        shl     ebx,16
;        add     ebx,(-SCRLW-5+2)*65536+SCRLW-2
;        mov     ecx,[top_ofs-2]
;        mov     cx,SCRLW-1
;        mcall   8,,,'UP',[sc.work_button]
;!!!!!!!!!!!!!!!!!!
	mov	ebx,[p_info.client_box.width]
	shl	ebx,16
	add	ebx,(-SCRLW)*65536+SCRLW
	mov	ecx,[top_ofs-2]
	mov	cx,SCRLW
	sub	ecx,0x00020000
	mcall	8,,,'UP' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
	mov	eax,8
;!!!!!!!!!!!!!!!!!!

	pushad
	push	0x18
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

;        mov     ecx,[bot_ofs]
;        shl     ecx,16
;        add     ecx,(-SCRLW*2-1)*65536+SCRLW-1
;        mcall   ,,,'DN'
;!!!!!!!!!!!!!!!!!!
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(-SCRLW*2-1)*65536+SCRLW
	mcall	,,,'DN' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
	mov	eax,8
;!!!!!!!!!!!!!!!!!!

	pushad
	push	0x19
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad
;        sub     ebx,1*65536-2

	push	ebx
	mov	eax,[lines]
	mov	ebx,[lines.scr]
	mov	ecx,[top_line]
	mov	edx,[bot_ofs]
	sub	edx,[top_ofs]
	add	edx,-SCRLW*3+1
	call	get_scroll_vars
	mov	[vscrl_top],eax
	mov	[vscrl_size],ebx
	pop	ebx

	mov	ecx,eax
	add	ecx,[top_ofs]
	add	ecx,SCRLW-1
;        shl     ecx,16
;        mov     cx,word[vscrl_size]

;       mcall   13,,,[sc.work_button]
;!!!!!!!!!!!!!!!!!!
	pushad
	sar	ebx,16
;        rol     ecx,16
;        movsx   eax,cx
;        sar     ecx,16
	push	ebx ecx SCRLW [vscrl_size]
	dec	dword[esp]
	call	draw_3d_panel
	popad
	mov	eax,13
;!!!!!!!!!!!!!!!!!!
	inc	ebx

	mov	ecx,[top_ofs-2]
	mov	cx,word[vscrl_top]
	add	ecx,(SCRLW-1)*65536
	mov	edx,[sc.work];[color_tbl+4*5]
	or	cx,cx
	jle	@f
	mcall	13
    @@:
	mov	ecx,[top_ofs]
	add	ecx,[vscrl_top]
	add	ecx,[vscrl_size]
	add	ecx,SCRLW-1
	mov	di,cx
	shl	ecx,16
	mov	cx,word[bot_ofs]
	sub	cx,di
	sub	cx,SCRLW*2+1
	jle	@f
	mcall
    @@:
;-----------------------
;        mov     eax,ebx
;        shr     eax,16
;        add     bx,ax
;        mov     ecx,[top_ofs-2]
;        mov     cx,word[top_ofs]
;        add     ecx,SCRLW*65536+SCRLW
;        mcall   38,,,[sc.work_graph];[sc.work_text]
;        mov     ecx,[bot_ofs-2]
;        mov     cx,word[bot_ofs]
;        sub     ecx,(SCRLW*2+2)*65536+(SCRLW*2+2)
;        mcall
	rol	ebx,16
	push	bx
	rol	ebx,16
	pop	bx
	mov	ecx,[top_ofs-2]
	mov	cx,word[bot_ofs]
	add	ecx,(SCRLW-1)*65536-SCRLW*2-2
	mcall	38,,,[cl_3d_inset]
;------------------------------------------------
;        mov     ebx,5*65536+SCRLW-1
;        mov     ecx,[bot_ofs]
;        shl     ecx,16
;        add     ecx,(-SCRLW)*65536+SCRLW-2
;        mcall   8,,,'LT',[sc.work_button]
;!!!!!!!!!!!!!!!!!!
	mov	ebx,SCRLW
	mov	ecx,[bot_ofs]
	shl	ecx,16
	add	ecx,(-SCRLW-1)*65536+SCRLW
	mcall	8,,,'LT' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
;!!!!!!!!!!!!!!!!!!

	pushad
	push	0x1B
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad

;        mov     ebx,[p_info.x_size]
;        shl     ebx,16
;        add     ebx,(-SCRLW*2-5)*65536+SCRLW
;        mcall   ,,,'RT'
;!!!!!!!!!!!!!!!!!!
	mov	ebx,[p_info.client_box.width]
	shl	ebx,16
	add	ebx,(-SCRLW*2)*65536+SCRLW
	mcall	8,,,'RT' or 0x40000000
	pushad
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	popad
;!!!!!!!!!!!!!!!!!!

	pushad
	push	0x1A
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	esp,4
	popad
;       inc     ecx

	push	ecx
	mov	eax,[columns]
	mov	ebx,[columns.scr]
	mov	ecx,[left_col]
	mov	edx,[p_info.client_box.width]
	add	edx,-(SCRLW*3)
	call	get_scroll_vars
	mov	[hscrl_top],eax
	mov	[hscrl_size],ebx
	pop	ecx

	mov	ebx,eax
	add	ebx,1+SCRLW
	shl	ebx,16
	mov	bx,word[hscrl_size]

;        mcall   13,,,[sc.work_button]
;!!!!!!!!!!!!!!!!!!
	pushad
	sar	ecx,16
	rol	ebx,16
	movsx	eax,bx
	sar	ebx,16
	dec	ebx
	push	eax ecx ebx SCRLW
	call	draw_3d_panel
	popad
;!!!!!!!!!!!!!!!!!!

	mov	ebx,(1+SCRLW)*65536
	mov	bx,word[hscrl_top]
	mcall	13,,,[sc.work];[color_tbl+4*5]
	mov	ebx,1+SCRLW
	add	ebx,[hscrl_top]
	add	ebx,[hscrl_size]
	mov	di,bx
	shl	ebx,16
	mov	bx,word[p_info.client_box.width]
	sub	bx,di
	sub	bx,SCRLW*2
	jle	@f
	mcall
    @@:
;        mov     eax,ebx
;        shr     eax,16
;        add     bx,ax
;        mov     ecx,[bot_ofs-2]
;        mov     cx,word[bot_ofs]
;        sub     ecx,SCRLW*65536+2
;        mcall   38,<OLEFT+SCRLW-1,OLEFT+SCRLW-1>,,[sc.work_graph];[sc.work_text]
;        mov     ebx,[p_info.x_size-2]
;        mov     bx,word[p_info.x_size]
;        sub     ebx,(SCRLW*2+6)*65536+(SCRLW*2+6)
;        mcall
	mov	ebx,[p_info.client_box.width]
;       add     ebx,5*65536-5
	mov	ecx,[bot_ofs-2]
	mov	cx,word[bot_ofs]
	sub	ecx,(SCRLW+1)*65536+(SCRLW+1)
	mcall	38,,,[cl_3d_inset]
;------------------------------------------------
  .exit:
	popad
	ret
endf

;-----------------------------------------------------------------------------
func get_next_part ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; Input:
;  ECX = current letter
;  ESI = string
; Output:
;  ECX = color
;  EDX = string
;  ESI = length
;-----------------------------------------------------------------------------
	cmp	[asm_mode],0
	je	.plain.text
	xor	ebx,ebx
	mov	edx,ecx
	add	esi,ecx
	mov	edi,symbols
	mov	al,[esi]
	cmp	al,';'
	je	.comment
	mov	ecx,symbols.size
	repne	scasb
	je	.symbol
	cmp	al,'$'
	jne	@f
	mov	edi,symbols
	mov	al,[esi+1]
	mov	ecx,symbols.size
	repne	scasb
	je	.not_symbol
	jmp	.number
    @@: cmp	al,'0'
	jb	@f
	cmp	al,'9'
	jbe	.number
    @@: cmp	al,"'"
	je	.string
	cmp	al,'"'
	je	.string
  .not_symbol:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	cmp	al,';'
	je	@f
	mov	ecx,symbols.size
	repne	scasb
	jne	.not_symbol
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*0]
	ret
  .symbol:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	mov	ecx,symbols.size
	repne	scasb
	je	.symbol
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*4]
	ret
  .comment:
	neg	edx
	add	edx,[cur_line_len];LINE_WIDTH
	xchg	edx,esi
	mov	ecx,[cur_line_len];LINE_WIDTH
	mov	eax,[color_tbl+4*3]
	ret
  .number:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	mov	edi,symbols
	mov	al,[esi+ebx]
	cmp	al,';'
	je	@f
	mov	ecx,symbols.size
	repne	scasb
	jne	.number
    @@: mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*1]
	ret
  .string:
	inc	ebx
	inc	edx
	cmp	edx,[cur_line_len];LINE_WIDTH
	jge	@f
	cmp	[esi+ebx],al
	jne	.string
	inc	ebx
	inc	edx
    @@:
	mov	ecx,edx
	mov	edx,esi
	mov	esi,ebx
	mov	eax,[color_tbl+4*2]
	ret
  .plain.text:
	mov	edx,[cur_line_len];LINE_WIDTH
	xchg	edx,esi
	mov	ecx,[cur_line_len];LINE_WIDTH
	mov	eax,[color_tbl+4*0]
	ret
endf

;-----------------------------------------------------------------------------
func writepos ;///// WRITE POSITION //////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[do_not_draw],1  ; return if drawing is not permitted
	jae	.exit
	pusha

;       mcall   9,p_info,-1

	mov	ecx,[p_info.client_box.height-2]
	mov	cx,word[p_info.client_box.height]
	sub	ecx,STATH*65536
;        mpack   ebx,6*13,6*13
;        add     ebx,[left_ofs-2]
;        add     ebx,[left_ofs]
	mcall	38,<6*13,6*13>,,[cl_3d_inset]

	pushad
;       sub     ebx,(6*13+1)*65536-1
;       sub     ebx,[left_ofs]
	mov	cx,STATH+1
	mcall	13,<0,6*13>,,[cl_3d_normal]
	mcall	,<6*13+1,6*(s_modified.size+2)-1>
	mov	ebx,(6*(s_modified.size+15)+1)*65536
	mov	bx,word[p_info.client_box.width]
	sub	bx,6*(s_modified.size+15)
	mcall
	popad

	add	ebx,6*(s_modified.size+2)*65536+6*(s_modified.size+2)
	mcall

	and	ecx,0x0000FFFF
	push	ecx

	mov	eax,[pos.y]
	inc	eax
	mov	ecx,10
	mov	edi,p_info+0x100;htext2.pos1
	cld
	call	uint2str
	mov	al,','
	stosb
	mov	eax,[pos.x]
	inc	eax
	call	uint2str

	pop	ebx

	lea	esi,[edi-p_info-0x100]
	lea	edi,[esi*3]
	shl	edi,16

;       add     ebx,[left_ofs-2]
	add	ebx,(1+6*6+3)*65536-STATH/2-3
	sub	ebx,edi
	mcall	4,,[sc.work_text],p_info+0x100

	cmp	[modified],0
	je	@f
	and	ebx,0x0000FFFF
;       add     ebx,[left_ofs-2]
	add	ebx,(1+12*6+12+1)*65536
	mcall	,,,s_modified,s_modified.size

    @@: cmp	[s_status],0
	je	@f
	and	ebx,0x0000FFFF
	add	ebx,6*(s_modified.size+16)*65536
	or	ecx, 80000000h
	mcall	,,,[s_status]

    @@: popa

  .exit:
	ret
endf

;-----------------------------------------------------------------------------
func print_text ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pusha
;        mov     ebx,(LBTNW+5+2)*65536
	mov	bx,word[p_info.box.width]
;        sub     bx,LBTNW+RBTNW+10+3
	mov	ecx,[ya-2]
;        mov     cx,ABTNH+1
	mcall	13,,,[sc.work]
;        mov     ebx,(LBTNW+5+2+4)*65536+ABTNH/2-3
	add	ebx,[ya]
	mov	eax,[p_info.box.width]
;        sub     eax,LBTNW+RBTNW+10+8
	push	eax
	cdq
	mov	ecx,6
	div	ecx
	cmp	eax,PATHL
	jbe	@f
	mov	eax,PATHL
    @@: mov	esi,eax
	mcall	4,,[color_tbl+0],[addr]

	mov	eax,[ya]
	mov	ebx,eax
;        add     eax,ABTNH/2-6
	shl	eax,16
	add	eax,ebx
;        add     eax,ABTNH/2-6+11
	mov	ecx,eax
	imul	eax,[temp],6
	pop	ebx
	cmp	eax,ebx
	jae	@f
;        add     eax,LBTNW+5+2+4
	mov	ebx,eax
	shl	eax,16
	add	ebx,eax
	mcall	38,,,[color_tbl+0]

    @@: popa
	ret
endf

func draw_framerect ; ebx,ecx
	push	ebx ecx

	add	bx,[esp+6]
	mov	cx,[esp+2]
	dec	ebx
	mcall	38,,,[cl_3d_inset]
	add	cx,[esp]
	rol	ecx,16
	add	cx,[esp]
	sub	ecx,0x00010001
	mcall

	mov	ebx,[esp+4]
	mov	ecx,[esp]
	mov	bx,[esp+6]
	add	cx,[esp+2]
	dec	ecx
	mcall
	add	bx,[esp+4]
	rol	ebx,16
	add	bx,[esp+4]
	sub	ebx,0x00010001
	mcall

	add	esp,8
	ret
endf