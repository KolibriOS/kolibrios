diff16 'tp-draw.asm',0,$

;-----------------------------------------------------------------------------
proc drawwindow ;///// DRAW WINDOW ///////////////////////////////////////////
;-----------------------------------------------------------------------------

	cmp	[just_from_popup],1
	jne	@f
	ret
    @@:

	mcall	48,3,sc,sizeof.system_colors
	call	calc_3d_colors

	;mcall   12,1
	invoke	gfx.open,TRUE
	mov	[ctx],eax

	;m2m     [sc.work],dword[color_tbl.back]

	mov	edx,[sc.work]
	add	edx,0x73000000
	mov	ebx,[mainwnd_pos.x-2]
	mov	bx,word[mainwnd_pos.w]
	mov	ecx,[mainwnd_pos.y-2]
	mov	cx,word[mainwnd_pos.h]
	mcall	0,,,,,s_title

	mcall	9,p_info,-1
	
	mov	eax,[p_info+70] ;status of window
	test	eax,100b
	jne	.exit.2	
	
	mov	esi,p_info.box.left
	mov	edi,mainwnd_pos
	mov	ecx,4
	cld
	rep	movsd

	cmp	[p_info.client_box.height],LINEH
	jl	.exit.2

	mov	[tab_bar.Bounds.Left],0
	mov	[tab_bar.Bounds.Top],ATOPH
	mov	eax,[p_info.client_box.width]
	mov	[tab_bar.Bounds.Right],eax
	mov	eax,[p_info.client_box.height]
	sub	eax,[bot_dlg_height]
	add	eax,-STATH-1
	mov	[tab_bar.Bounds.Bottom],eax

	call	align_editor_in_tab

	mov	[top_ofs],ATOPH;+1

	mov	eax,[p_info.client_box.height]
	add	eax,-STATH+1;*3-2-2
	sub	eax,[bot_dlg_height]
	mov	[bot_ofs],eax

	call	draw_bottom_dialog

;        mov     [do_not_draw],1 ; do_not_draw = true

;        mov     ebx,eax
;        sub     ebx,[top_ofs]
;        sub     ebx,SCRLW*3+AMINS+5
;        js      .no_draw

;        dec     [do_not_draw]    ; do_not_draw = false
;        sub     eax,SCRLW+3
;        sub     eax,[top_ofs]
;        cdq
;        mov     ebx,LINEH
;        div     ebx
;        mov     [lines.scr],eax

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
	call	draw_editor
	call	draw_tabctl
  .exit.2:
	;mcall   12,2
	invoke	gfx.close,[ctx]
	ret
endp

;-----------------------------------------------------------------------------
proc draw_bottom_dialog ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[bot_dlg_height],0
	je	.exit
	pushad
	invoke	gfx.pen.color,[ctx],[cl_3d_inset]
	mov	ecx,[bot_ofs]
	dec	ecx
	invoke	gfx.line,[ctx],0,ecx,[p_info.client_box.width],ecx
	invoke	gfx.brush.color,[ctx],[cl_3d_normal]
	inc	ecx
	mov	eax,ecx
	add	eax,[bot_dlg_height]
	invoke	gfx.fillrect,[ctx],0,ecx,[p_info.client_box.width],eax
	;mov     ebx,[p_info.client_box.width]
	;mov     ecx,[bot_ofs]
	;dec     ecx
	;push    cx
	;shl     ecx,16
	;pop     cx
	;mcall   38,,,[cl_3d_inset]
	;mov     ecx,[bot_ofs-2]
	;mov     cx,word[bot_dlg_height]
	;dec     ecx
	;mov     ebx,[p_info.client_box.width]
	;inc     ebx
	;mcall   13,,,[cl_3d_normal]
	mov	al,1
	call	[bot_dlg_handler]
	popad

  .exit:
	ret
endp

mi_sel	 dd ?
mi_cur	 dd -1

;-----------------------------------------------------------------------------
proc draw_main_menu ;/////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	ebx,[p_info.client_box.width]
	inc	ebx
	mcall	13,,ATOPH-1,[cl_3d_normal]

	mcall	38,[p_info.client_box.width],<ATOPH-1,ATOPH-1>,[cl_3d_inset]

	mov	edx,main_menu
	mov	ebx,9*65536+ATOPH/2-3;4
	mov	[mi_sel],0
	mov	edi,[mi_cur]
    @@: inc	[mi_sel]
	cmp	[mi_sel],main_menu.cnt_item
	ja	.exit
	mov	ecx,[sc.work_text]
	cmp	edi,[mi_sel]
	jne	.lp1
	pushad
	push	edx
	mov	ecx,[edx+4]
	add	ecx,2*65536-2
	mcall	13,[edx+0],,[sc.work]
	mov	edx,[esp]
	mov	cx,[edx+6]
	add	ecx,-1*65536+1
	add	bx,[edx+2]
	mcall	38,,,[cl_3d_inset]

	mov	edx,[esp]
	add	cx,[edx+4]
	add	cx,-2
	mov	bx,[edx+2]
	mcall	,,,[cl_3d_inset]
	pop	edx
	movzx	eax,word[edx]
	add	ebx,eax
	shl	eax,16
	add	ebx,eax
	mcall	38,,,[cl_3d_inset]
	popad
	mov	ecx,[color_tbl.text]
  .lp1: add	edx,8+1
	movzx	esi,byte[edx-1]
	mcall	4
	add	edx,esi
	add	esi,2
	imul	esi,6*65536
	add	ebx,esi
	jmp	@b

  .exit:
	mov	ebx,[mainwnd_pos.w]
	add	ebx,-10-(ATOPH-6)-3
	push	ebx 2 (ATOPH-6) (ATOPH-6)
	call	draw_3d_panel
	shl	ebx,16
	add	ebx,ATOPH-6
	mcall	8,,<2,ATOPH-6>,<0x4000,2>
	and	ebx,0xFFFF0000
	add	ebx,(ATOPH-8)/2*65536+(ATOPH-8)/2
	mcall	4,,[sc.work_text],.cross,1

	ret

.cross db 'x'
endp

;-----------------------------------------------------------------------------
proc draw_statusbar ;///// DRAW POSITION, MODIFIED STATE, HINT ///////////////
;-----------------------------------------------------------------------------
	cmp	[do_not_draw],1  ; return if drawing is not permitted
	jae	.exit
	pusha

       mcall   9,p_info,-1

	mov	eax,[p_info+70] ;status of window
	test	eax,100b
	jne	.exit_1	
	
	mov	ecx,[p_info.client_box.height-2]
	mov	cx,word[p_info.client_box.height]
	sub	ecx,STATH*65536+STATH
	mcall	38,[p_info.client_box.width],,[cl_3d_inset]

;       mcall   9,p_info,-1

	mov	ecx,[p_info.client_box.height-2]
	mov	cx,word[p_info.client_box.height]
	sub	ecx,STATH*65536
	mcall	38,<6*13,6*13>,,[cl_3d_inset]

	pushad
	add	ecx,1*65536
	mov	cx,STATH
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

	mov	eax,[cur_editor.Caret.Y]
	inc	eax
	mov	ecx,10
	mov	edi,p_info+0x100
	cld
	call	uint2str
	mov	al,','
	stosb
	mov	eax,[cur_editor.Caret.X]
	inc	eax
	call	uint2str

	pop	ebx

	lea	esi,[edi-p_info-0x100]
	lea	edi,[esi*3]
	shl	edi,16

	add	ebx,(1+6*6+3)*65536-STATH/2-3
	sub	ebx,edi
	mcall	4,,[sc.work_text],p_info+0x100

	cmp	[cur_editor.Modified],0
	je	@f
	and	ebx,0x0000FFFF
	add	ebx,(1+12*6+12+1)*65536
	mcall	,,,s_modified,s_modified.size

    @@: cmp	[s_status],0
	je	@f
	and	ebx,0x0000FFFF
	add	ebx,6*(s_modified.size+16)*65536
	or	ecx, 80000000h
	mcall	,,,[s_status]
.exit_1:
    @@: popa

  .exit:
	ret
endp

proc draw_fillrect ; ebx,ecx,edx
	; ebx = <left,width>
	; ecx = <top,height>
	push	ebx ecx edx
	call	draw_framerect
	add	ebx,1*65536-2
	add	ecx,1*65536-2
	mcall	13,,,esi
	pop	edx ecx ebx
	ret
endp

proc draw_framerect ; ebx,ecx,edx
	; ebx = <left,width>
	; ecx = <top,height>
	push	ebx ecx

	add	bx,[esp+6]	 ; ebx = <left,right>
	mov	cx,[esp+2]	 ; ecx = <top,top>
	dec	ebx
	mcall	38
	add	cx,[esp]	 ; ecx = <top,bottom>
	rol	ecx,16
	add	cx,[esp]	 ; ecx = <bottom,bottom>
	sub	ecx,0x00010001
	mcall

	mov	ebx,[esp+4]	 ; ebx = <width,left>
	mov	ecx,[esp]	 ; ecx = <height,top>
	mov	bx,[esp+6]	 ; ebx = <width,>
	add	cx,[esp+2]
	dec	ecx
	mcall
	add	bx,[esp+4]
	rol	ebx,16
	add	bx,[esp+4]
	sub	ebx,0x00010001
	mcall

	pop	ecx ebx
	ret
endp

proc draw_check
	push	bx
	shl	ebx,16
	pop	bx
	add	ebx,0x00010000
	push	cx
	shl	ecx,16
	pop	cx
	add	ecx,0x00020001
	mcall	38
	add	ecx,0x00010001
	mcall
	add	ebx,4
	sub	ecx,2
	mcall
	sub	ecx,0x00010001
	mcall
	ret
endp

proc calc_middle
	shr	eax,1
	shr	ebx,1
	and	eax,0x007F7F7F
	and	ebx,0x007F7F7F
	add	eax,ebx
	ret
endp

proc calc_3d_colors
	pushad
	m2m	[cl_3d_normal],[sc.work]
	m2m	[cl_3d_inset],[sc.work_graph]
	push	[cl_3d_normal]
	add	byte[esp],48
	jnc	@f
	mov	byte[esp],255
    @@: add	byte[esp+1],48
	jnc	@f
	mov	byte[esp+1],255
    @@: add	byte[esp+2],48
	jnc	@f
	mov	byte[esp+2],255
    @@: pop	[cl_3d_outset]
	mov	eax,[cl_3d_inset]
	mov	ebx,[cl_3d_outset]
	call	calc_middle
	mov	ebx,[cl_3d_normal]
	call	calc_middle
	mov	[cl_3d_pushed],eax
	mov	eax,[cl_3d_normal]
	mov	ebx,[sc.work_text]
	call	calc_middle
	mov	[cl_3d_grayed],eax
	popad
	ret
endp

proc draw_3d_panel ; x,y,w,h
	push	eax ebx ecx edx
	cmp	dword[esp+16+8],4
	jl	.exit
	cmp	dword[esp+16+4],4
	jl	.exit
	mov	ebx,[esp+16+16-2]
	mov	bx,[esp+16+8]
	inc	ebx
	mov	ecx,[esp+16+12-2]
	mov	cx,[esp+16+4]
	inc	ecx
	mcall	13,,,[cl_3d_normal]
	dec	ebx
	add	bx,[esp+16+16]
	mov	cx,[esp+16+12]
	mcall	38,,,[cl_3d_inset]
	add	ecx,[esp+16+4-2]
	add	cx,[esp+16+4]
	mcall
	mov	bx,[esp+16+16]
	mov	ecx,[esp+16+12-2]
	mov	cx,[esp+16+4]
	add	cx,[esp+16+12]
	mcall
	add	ebx,[esp+16+8-2]
	add	bx,[esp+16+8]
	mcall
	mov	ebx,[esp+16+16-2]
	mov	bx,[esp+16+8]
	add	bx,[esp+16+16]
	add	ebx,1*65536-1
	mov	ecx,[esp+16+12-2]
	mov	cx,[esp+16+12]
	add	ecx,0x00010001
	mcall	,,,[cl_3d_outset]
	mov	bx,[esp+16+16]
	inc	ebx
	mov	ecx,[esp+16+12-2]
	mov	cx,[esp+16+4]
	add	cx,[esp+16+12]
	add	ecx,2*65536-1
	mcall
  .exit:
	pop	edx ecx ebx eax
	ret	4*4
endp
