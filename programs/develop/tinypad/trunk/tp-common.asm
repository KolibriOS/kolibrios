;-----------------------------------------------------------------------------
func check_cur_vis_inv ;//////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	push	eax ebx
	xor	bl,bl
  .chk_y:
	mov	eax,[pos.y]
	or	eax,eax
	jge	@f
	mov	[pos.y],0
	jmp	.chk_dy
    @@: cmp	eax,[lines]
	jl	.chk_dy
	mov	eax,[lines]
	dec	eax
	mov	[pos.y],eax
  .chk_dy:
	mov	eax,[top_line]
	cmp	eax,[pos.y]
	jle	@f
	push	[pos.y]
	pop	[top_line]
	inc	bl
    @@: add	eax,[lines.scr]
	cmp	eax,[pos.y]
	jg	.chk_x
	mov	eax,[pos.y]
	sub	eax,[lines.scr]
	inc	eax
	mov	[top_line],eax
	inc	bl
  .chk_x:
	mov	eax,[pos.x]
	or	eax,eax
	jge	@f
	mov	[pos.x],0
	jmp	.chk_dx
    @@: cmp	eax,[columns]
	jl	.chk_dx
	mov	eax,[columns]
	mov	[pos.x],eax
  .chk_dx:
	mov	eax,[left_col]
	cmp	eax,[pos.x]
	jle	@f
	push	[pos.x]
	pop	[left_col]
	inc	bl
    @@: add	eax,[columns.scr]
	cmp	eax,[pos.x]
	jg	@f
	mov	eax,[pos.x]
	sub	eax,[columns.scr]
	inc	eax
	mov	[left_col],eax
	inc	bl
    @@: cmp	[mev],MEV_LDOWN
	jne	.exit
	push	[pos.x] [pos.y]
	pop	[sel.y] [sel.x]
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
	mov	eax,[sel.y]
	mov	ebx,[pos.y]
	cmp	eax,ebx
	jle	@f
	xchg	eax,ebx
    @@: push	[pos.x] [pos.y]
	pop	[sel.y] [sel.x]
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
	mov	eax,[top_line]
	add	eax,[lines.scr]
	cmp	eax,[lines]
	jbe	.lp1
	mov	eax,[lines]
	sub	eax,[lines.scr]
	jns	@f
	xor	eax,eax
    @@: mov	[top_line],eax
  .lp1: mov	eax,[left_col]
	add	eax,[columns.scr]
	cmp	eax,[columns]
	jbe	.exit
	mov	eax,[columns]
	sub	eax,[columns.scr]
	jns	@f
	xor	eax,eax
    @@: mov	[left_col],eax
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
	mov	eax,[pos.y]
	mov	ecx,[top_line]
  .skip_init:
	call	check_cur_vis
	mov	[pos.y],eax
	mov	[top_line],ecx
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
	cmp	edx,[lines]
	jbe	@f
	mov	edx,[lines]
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
	cmp	edx,[lines]
	jbe	@f
	mov	ecx,[lines]
	sub	ecx,[lines.scr]
	jns	@f
	xor	ecx,ecx
    @@:;mov     [top_line],ecx

	pushad
	mov	eax,[pos.x]
	mov	ebx,[left_col]
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
    @@: mov	[left_col],ebx

  .exit:
	mov	[pos.x],eax
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
	mov	esi,AREA_EDIT
	jecxz	.exit
    @@: movzx	eax,word[esi]
	dec	ecx
	lea	esi,[esi+eax+4]
	jnz	@b
  .exit:
	pop	ecx eax
	ret
endf

;-----------------------------------------------------------------------------
func init_sel_vars ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	pushad
	mov	[sel.selected],1
	mov	eax,[sel.x]
	mov	ebx,[sel.y]
	mov	ecx,[pos.x]
	mov	edx,[pos.y]
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
; esi = line offset
; ecx = needed line length
;-----------------------------------------------------------------------------
	pushad
	movzx	edx,word[esi]
	cmp	ecx,edx
	jbe	.exit
	sub	ecx,edx
	push	ecx
	mov	edi,AREA_TEMP2
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
	mov	ecx,AREA_TEMP2
	sub	ecx,esi
	cld
	rep	movsb
	mov	eax,[sel.end.y]
	sub	eax,[sel.begin.y]
	sub	[lines],eax
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
	mov	ecx,AREA_TEMP2
	sub	ecx,esi
	cld
	rep	movsb

  .exit:
	mov	eax,[sel.begin.x]
	mov	[pos.x],eax
	mov	[sel.x],eax
	mov	eax,[sel.begin.y]
	mov	[pos.y],eax
	mov	[sel.y],eax
	popad
	mov	[modified],1
	clc
	ret

  .exit.2:
	stc
	ret
endf
