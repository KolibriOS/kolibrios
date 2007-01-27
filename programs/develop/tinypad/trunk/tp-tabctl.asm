;-----------------------------------------------------------------------------
func flush_cur_tab ;///// SAVE CURRENT TAB DATA TO CONTROL ///////////////////
;-----------------------------------------------------------------------------
; EBP = TABITEM*
;-----------------------------------------------------------------------------
	push	ecx esi edi
	mov	esi,cur_tab
	mov	edi,[tab_bar.Current.Ptr]
	mov	ecx,sizeof.TABITEM/4
	cld
	rep	movsd
	pop	edi esi ecx
	ret
endf

;-----------------------------------------------------------------------------
func set_cur_tab ;///// SET SPECIFIED TAB CURRENT (FOCUS IT) /////////////////
;-----------------------------------------------------------------------------
; EBP = TABITEM*
;-----------------------------------------------------------------------------
	push	ecx esi edi
	cmp	[tab_bar.Current.Ptr],0
	je	@f
	call	flush_cur_tab
    @@: mov	esi,ebp
	mov	edi,cur_tab
	mov	ecx,sizeof.TABITEM/4
	rep	movsd
	mov	[tab_bar.Current.Ptr],ebp
;       call    update_caption
	pop	edi esi ecx
	ret
endf

;-----------------------------------------------------------------------------
func make_tab_visible ;///// MAKE SPECIFIED TAB VISIBLE IF IT'S OFFSCREEN ////
;-----------------------------------------------------------------------------
	call	flush_cur_tab
	imul	eax,[tab_bar.Items.Left],sizeof.TABITEM
	add	eax,[tab_bar.Items]
	cmp	eax,ebp
	jb	.go_right
	ja	.go_left
	ret

  .go_right:
	push	ebp
	call	get_hidden_tabitems_number
	cmp	ebp,[esp]
	ja	@f
    @@: inc	[tab_bar.Items.Left]
	call	get_hidden_tabitems_number
	cmp	ebp,[esp]
	jbe	@b
    @@: pop	ebp
	ret

  .go_left:
	mov	eax,ebp
	sub	eax,[tab_bar.Items]
	jz	@f
	cdq
	mov	ebx,sizeof.TABITEM
	div	ebx
    @@: mov	[tab_bar.Items.Left],eax
	ret
endf

;-----------------------------------------------------------------------------
func create_tab ;///// ADD TAB TO THE END ////////////////////////////////////
;-----------------------------------------------------------------------------
	push	eax ecx esi edi

	inc	[tab_bar.Items.Count]
	imul	ebx,[tab_bar.Items.Count],sizeof.TABITEM
	mov	eax,[tab_bar.Items]
	mov	ecx,eax
	call	mem.ReAlloc
	mov	[tab_bar.Items],eax
	sub	ecx,eax
	sub	[tab_bar.Current.Ptr],ecx
	cmp	[tab_bar.Default.Ptr],0
	je	@f
	sub	[tab_bar.Default.Ptr],ecx
    @@: lea	ebp,[eax+ebx-sizeof.TABITEM]
	call	set_cur_tab

	mov	eax,1024
	mov	[cur_editor.Lines.Size],eax
	call	mem.Alloc
	mov	[cur_editor.Lines],eax
	mov	[cur_editor.Lines.Count],1
	mov	[cur_editor.Columns.Count],1
	xor	eax,eax
	mov	[cur_editor.TopLeft.X],eax
	mov	[cur_editor.TopLeft.Y],eax
	mov	[cur_editor.Caret.X],eax
	mov	[cur_editor.Caret.Y],eax
	mov	[cur_editor.SelStart.X],eax
	mov	[cur_editor.SelStart.Y],eax
	mov	edi,[cur_editor.Lines]
	add	edi,4
	mov	ecx,10
	mov	[edi-4],ecx
	mov	[edi+10],eax
	mov	al,' '
	cld
	rep	stosb

	mov	esi,s_defname
	mov	edi,cur_editor.FilePath
	mov	ecx,s_defname.size
	rep	movsb
	mov	[cur_editor.FileName],0

	mov	[cur_editor.Modified],0
	mov	[cur_editor.AsmMode],0

	call	flush_cur_tab
	mov	ebp,[tab_bar.Current.Ptr]
	call	make_tab_visible
	call	update_caption
	cmp	[do_not_draw],0
	jne	@f
	call	align_editor_in_tab
	call	draw_editor
	call	draw_tabctl
	call	draw_statusbar
	call	update_caption
    @@:
	mov	ebp,cur_tab
	pop	edi esi ecx eax
	ret
endf

;-----------------------------------------------------------------------------
func delete_tab ;///// DELETE SPECIFIED TAB //////////////////////////////////
;-----------------------------------------------------------------------------
	cmp	[tab_bar.Default.Ptr],0
	je	@f
	cmp	ebp,[tab_bar.Default.Ptr]
	je	.lp1
	ja	@f
	sub	[tab_bar.Default.Ptr],sizeof.TABITEM
	jmp	@f
  .lp1:
	mov	[tab_bar.Default.Ptr],0

    @@: mov	eax,[ebp+TABITEM.Editor.Lines]
	call	mem.Free
	imul	ecx,[tab_bar.Items.Count],sizeof.TABITEM
	add	ecx,[tab_bar.Items]
	sub	ecx,ebp
	sub	ecx,sizeof.TABITEM
	jle	@f
	cld
	shr	ecx,2
	mov	edi,ebp
	lea	esi,[edi+sizeof.TABITEM]
	rep	movsd
    @@: dec	[tab_bar.Items.Count]
	jz	.no_tabs
	imul	ebx,[tab_bar.Items.Count],sizeof.TABITEM
	push	ebx
	mov	eax,[tab_bar.Items]
	mov	ecx,eax
	call	mem.ReAlloc
	mov	[tab_bar.Items],eax
	sub	ecx,eax
	sub	ebp,ecx
	cmp	[tab_bar.Default.Ptr],0
	je	@f
	sub	[tab_bar.Default.Ptr],ecx
    @@:
	pop	ecx
	add	ecx,[tab_bar.Items]
	sub	ecx,ebp
	ja	@f
	add	ebp,-sizeof.TABITEM

    @@: mov	[tab_bar.Current.Ptr],0
	call	set_cur_tab
	call	make_tab_visible
	call	align_editor_in_tab
	call	draw_editor
	call	draw_tabctl
	call	draw_statusbar
	ret

  .no_tabs:
	mov	eax,[tab_bar.Items]
	call	mem.Free
	xor	eax,eax
	mov	[tab_bar.Items],eax
	mov	[tab_bar.Current.Ptr],eax
	mov	[tab_bar.Default.Ptr],eax
	ret
endf

;-----------------------------------------------------------------------------
func draw_tabctl ;///// DRAW TAB CONTROL /////////////////////////////////////
;-----------------------------------------------------------------------------

	dec	[tab_bar.Items.Left]
	js	.lp1
    @@: call	get_hidden_tabitems_number
	or	eax,eax
	jnz	.lp1
	dec	[tab_bar.Items.Left]
	jns	@b
  .lp1: inc	[tab_bar.Items.Left]
	mov	eax,[tab_bar.Items.Count]
	cmp	[tab_bar.Items.Left],eax
	jb	@f
	dec	eax
	mov	[tab_bar.Items.Left],eax
    @@:

	mov	eax,8
	mov	edx,[tab_bar.Buttons.First]
    @@: cmp	edx,[tab_bar.Buttons.Last]
	ja	@f
	push	edx
	or	edx,0x80000000
	mcall
	pop	edx
	inc	edx
	jmp	@b
    @@:

	mov	ebx,[tab_bar.Bounds.Left-2]
	mov	bx,word[tab_bar.Bounds.Right]
	sub	bx,word[tab_bar.Bounds.Left]
	inc	ebx
	mov	ecx,[tab_bar.Bounds.Top-2]
	mov	cx,word[tab_bar.Bounds.Bottom]
	sub	cx,word[tab_bar.Bounds.Top]
	inc	ecx
	mov	edx,[cl_3d_normal]
	call	draw_framerect

	mov	al,[tab_bar.Style]
	dec	al
	jz	.tabs_on_top
	dec	al
	jz	.tabs_on_bottom
	dec	al
	jz	.tabs_on_left
	dec	al
	jz	.tabs_on_right
	ret

  .tabs_on_top:
	add	ebx,1*65536-2
	mov	ecx,[tab_bar.Bounds.Top-2]
	xor	cx,cx
	add	ecx,1*65536+TBARH
	mcall	13
	add	ecx,(TBARH-1)*65536-(TBARH-1)
	mcall	,,,[sc.work]
	add	ecx,-1*65536+2
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	esi,[tab_bar.Bounds.Left]
	inc	esi
	mov	edi,[tab_bar.Bounds.Top]
	inc	edi
	push	.curr_top .check_horz .next_horz
	call	.draw_tabs
	ret

  .tabs_on_bottom:
	add	ebx,1*65536-2
	mov	ecx,[tab_bar.Bounds.Bottom-2]
	xor	cx,cx
	add	ecx,-TBARH*65536+TBARH
	mcall	13
	mov	cx,1
	mcall	,,,[sc.work]
	add	ecx,-1*65536+2
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	esi,[tab_bar.Bounds.Left]
	inc	esi
	mov	edi,[tab_bar.Bounds.Bottom]
	add	edi,-TBARH+1
	push	.curr_bottom .check_horz .next_horz
	call	.draw_tabs
	ret

  .tabs_on_left:
	call	get_max_tab_width
	mov	ebx,[tab_bar.Bounds.Left-2]
	mov	bx,ax
	add	ebx,1*65536-1
	add	ecx,1*65536-2
	push	eax
	mcall	13
	pop	ebx
	shl	ebx,16
	add	ebx,1*65536+1
	mcall	,,,[sc.work]
	add	ebx,-1*65536+2
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	esi,[tab_bar.Bounds.Left]
	inc	esi
	mov	edi,[tab_bar.Bounds.Top]
	inc	edi
	push	.curr_left .check_vert .next_vert
	call	.draw_tabs
	ret

  .tabs_on_right:
	call	get_max_tab_width
	push	eax
	mov	ebx,[tab_bar.Bounds.Right-2]
	mov	bx,ax
	shl	eax,16
	sub	ebx,eax
	add	ecx,1*65536-2
	mcall	13
	add	ebx,-1*65536
	mov	bx,1
	mcall	,,,[sc.work]
	add	ebx,-1*65536+2
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	esi,[tab_bar.Bounds.Right]
	pop	eax
	sub	esi,eax
	mov	edi,[tab_bar.Bounds.Top]
	inc	edi
	push	.curr_right .check_vert .next_vert
	call	.draw_tabs
	ret


  .draw_tabs:
	mov	ecx,[tab_bar.Items.Count]
	mov	ebx,[tab_bar.Items.Left]
	imul	ebp,ebx,sizeof.TABITEM
	add	ebp,[tab_bar.Items]
	push	ecx
	sub	[esp],ebx
	add	ebx,1000
	mov	[tab_bar.Buttons.First],ebx
	dec	ebx
	mov	[tab_bar.Buttons.Last],ebx
    @@: push	ecx

	call	get_tab_size

	call	dword[esp+(8+4)+4]
	jc	.draw_tabs.dontfit

	rol	ebx,16
	mov	bx,si
	rol	ebx,16
	rol	ecx,16
	mov	cx,di
	rol	ecx,16
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	edx,[sc.work_text]
	cmp	ebp,[tab_bar.Current.Ptr]
	jne	.draw_tabs.inactive
	push	ebx ecx
	call	dword[esp+(8+4)+8+8]
	mcall	13,,,[sc.work]
	pop	ecx ebx
	mov	edx,[color_tbl.text]
  .draw_tabs.inactive:

	cmp	ebp,[tab_bar.Default.Ptr]
	jne	.draw_tabs.notdefault
	push	ebx ecx edx
	add	ebx,3*65536
	add	ecx,(TBARH/2-6)*65536
	mov	bx,11
	mov	cx,bx
	call	draw_framerect
	add	ebx,1*65536-2
	add	ecx,1*65536-2
	mcall	13,,,[sc.work]
	shr	ebx,16
	shr	ecx,16
	add	ebx,2
	add	ecx,3
	mov	edx,[esp]
	call	draw_check
	pop	edx ecx ebx
  .draw_tabs.notdefault:

	push	ebx ecx esi edx
	lea	eax,[ebp+TABITEM.Editor.FilePath]
	add	eax,[ebp+TABITEM.Editor.FileName]
	mov	edx,eax
	call	strlen
	mov	esi,eax
	shr	ecx,16
	mov	bx,cx
	add	ebx,5*65536+TBARH/2-4
	pop	ecx
	cmp	ebp,[tab_bar.Default.Ptr]
	jne	.lp2
	add	ebx,13*65536
  .lp2: mcall	4
	pop	esi ecx ebx

	inc	[tab_bar.Buttons.Last]
	cmp	ebp,[tab_bar.Current.Ptr]
	je	.draw_tabs.active
	push	ebx ecx
	dec	ebx
	dec	ecx
	mov	edx,[tab_bar.Buttons.Last]
	or	edx,0x40000000
	mcall	8
	pop	ecx ebx
  .draw_tabs.active:

	call	dword[esp+(8+4)+0]
	add	ebp,sizeof.TABITEM

	pop	ecx
	dec	ecx
	dec	dword[esp]
	jnz	@b

	add	esp,4
	or	ecx,ecx
	jnz	@f

	ret	8

  .draw_tabs.dontfit:

	add	esp,8

    @@: mov	ebx,[tab_bar.Bounds.Right]
	shl	ebx,16
	mov	ecx,[tab_bar.Bounds.Bottom]
	shl	ecx,16
	add	ecx,(-SCRLW-1)*65536+SCRLW
	call	get_max_tab_width
	mov	edx,eax

	mov	al,[tab_bar.Style]
	dec	al
	jz	.scroll_on_top
	dec	al
	jz	.scroll_on_bottom
	dec	al
	jz	.scroll_on_left
	dec	al
	jz	.scroll_on_right
	ret

  .scroll_on_top:
	add	ebx,(-SCRLW*2-1)*65536+SCRLW
	mov	ecx,[tab_bar.Bounds.Top]
	shl	ecx,16
	add	ecx,1*65536+SCRLW
	jmp	.draw_tabs.draw_scroll
  .scroll_on_bottom:
	add	ebx,(-SCRLW*2-1)*65536+SCRLW
	jmp	.draw_tabs.draw_scroll
  .scroll_on_left:
	mov	ebx,[tab_bar.Bounds.Left]
	add	ebx,edx
	shl	ebx,16
	add	ebx,(-SCRLW*2)*65536+SCRLW
	jmp	.draw_tabs.draw_scroll
  .scroll_on_right:
	shl	edx,16
	sub	ebx,edx
	add	ebx,SCRLW

  .draw_tabs.draw_scroll:
	mcall	8,,,'TBG' or 0x40000000
	push	ebx
	add	ebx,SCRLW*65536
	mcall	8,,,'TBL' or 0x40000000
	pop	ebx
	push	ebx ecx
	sar	ebx,16
	sar	ecx,16
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	add	ebx,SCRLW
	push	ebx ecx SCRLW SCRLW
	call	draw_3d_panel
	pop	ecx ebx

	push	'<'
	shr	ecx,16
	mov	bx,cx
	add	ebx,(SCRLW/2-2)*65536+SCRLW/2-3
	mcall	4,,[sc.work_text],esp,1
	add	ebx,0x00020000
	mcall
	add	ebx,(SCRLW-2)*65536
	mov	byte[esp],'>'
	mcall
	add	ebx,0x00020000
	mcall
	add	esp,4

	ret	8

  .curr_left:
	add	ebx,0x00010000
	add	ecx,1*65536-2
	ret
  .curr_top:
	add	ebx,1*65536-2
	add	ecx,0x00010000
	ret
  .curr_right:
	dec	ebx
	add	ecx,1*65536-2
	ret
  .curr_bottom:
	add	ebx,1*65536-2
	dec	ecx
	ret

  .check_horz:
	lea	eax,[esi+ebx-1]
	sub	eax,[tab_bar.Bounds.Right]
	jge	.check.dontfit
	add	eax,SCRLW*2+2
	jl	.check.fit
	cmp	dword[esp+4],1
	jbe	.check.fit
  .check.dontfit:
	stc
	ret
  .check_vert:
	lea	eax,[edi+ecx-1]
	sub	eax,[tab_bar.Bounds.Bottom]
	jge	.check.dontfit
	add	eax,SCRLW+2
	jl	.check.fit
	cmp	dword[esp+4],1
	ja	.check.dontfit
  .check.fit:
	clc
	ret

  .next_horz:
	movzx	ebx,bx
	lea	esi,[esi+ebx+1]
	ret
  .next_vert:
	movzx	ecx,cx
	lea	edi,[edi+ecx+1]
	ret
endf

;-----------------------------------------------------------------------------
func get_tab_size ;///// GET TAB WIDTH ///////////////////////////////////////
;-----------------------------------------------------------------------------
; EBP = TABITEM*
;-----------------------------------------------------------------------------
	push	eax
	cmp	[tab_bar.Style],3
	jae	.lp1
	lea	eax,[ebp+TABITEM.Editor.FilePath]
	add	eax,[ebp+TABITEM.Editor.FileName]
	cmp	byte[eax],0
	jne	@f
	int3
    @@:
	call	strlen
	imul	ebx,eax,6
	add	ebx,9
	cmp	ebp,[tab_bar.Default.Ptr]
	jne	.lp2
	add	ebx,13
	jmp	.lp2
  .lp1: call	get_max_tab_width
	mov	ebx,eax
  .lp2: mov	ecx,TBARH-1
	pop	eax
	ret
endf

;-----------------------------------------------------------------------------
func get_max_tab_width ;///// GET WIDTH OF LONGEST TAB ///////////////////////
;-----------------------------------------------------------------------------
	push	ebx ecx ebp
	mov	ecx,[tab_bar.Items.Count]
	mov	ebp,[tab_bar.Items]
	xor	ebx,ebx
    @@: dec	ecx
	js	@f

	lea	eax,[ebp+TABITEM.Editor.FilePath]
	add	eax,[ebp+TABITEM.Editor.FileName]
	call	strlen
	imul	eax,6
	add	eax,9

	add	ebp,sizeof.TABITEM
	cmp	ebx,eax
	jae	@b
	mov	ebx,eax
	jmp	@b
    @@: mov	eax,ebx
	cmp	eax,SCRLW*2+2
	jae	@f
	mov	eax,SCRLW*2+2
    @@: cmp	[tab_bar.Default.Ptr],0
	je	@f
	add	eax,13
    @@: pop	ebp ecx ebx
	ret
endf

;-----------------------------------------------------------------------------
func get_hidden_tabitems_number ;/////////////////////////////////////////////
;-----------------------------------------------------------------------------
	mov	al,[tab_bar.Style]
	dec	al
	dec	al
	jle	.tabs_horz
	dec	al
	dec	al
	jle	.tabs_vert
	ret

  .tabs_horz:
	push	draw_tabctl.check_horz draw_tabctl.next_horz
	call	.calc_tabs
	ret

  .tabs_vert:
	push	draw_tabctl.check_vert draw_tabctl.next_vert
	call	.calc_tabs
	ret

  .calc_tabs:
	mov	esi,[tab_bar.Bounds.Left]
	inc	esi
	mov	edi,[tab_bar.Bounds.Top]
	inc	edi
	mov	ecx,[tab_bar.Items.Count]
	mov	ebx,[tab_bar.Items.Left]
	imul	ebp,ebx,sizeof.TABITEM
	add	ebp,[tab_bar.Items]
	push	ecx
	sub	[esp],ebx
    @@: push	ecx

	call	get_tab_size

	call	dword[esp+(8+4)+4]
	jc	.calc_tabs.dontfit

	call	dword[esp+(8+4)+0]
	add	ebp,sizeof.TABITEM

	pop	ecx
	dec	ecx
	dec	dword[esp]
	jnz	@b

	jmp	@f

  .calc_tabs.dontfit:

	add	esp,4
    @@: pop	ecx
	mov	eax,ecx
	ret	8
endf

;-----------------------------------------------------------------------------
func align_editor_in_tab ;///// ADJUST EDITOR POSITION TO FIT IN TAB /////////
;-----------------------------------------------------------------------------
	m2m	[cur_editor.Bounds.Left],[tab_bar.Bounds.Left]
	m2m	[cur_editor.Bounds.Top],[tab_bar.Bounds.Top]
	m2m	[cur_editor.Bounds.Right],[tab_bar.Bounds.Right]
	m2m	[cur_editor.Bounds.Bottom],[tab_bar.Bounds.Bottom]

	inc	[cur_editor.Bounds.Left]
	inc	[cur_editor.Bounds.Top]
	dec	[cur_editor.Bounds.Right]
	dec	[cur_editor.Bounds.Bottom]

	call	get_max_tab_width
	lea	ebx,[eax+1]

	mov	al,[tab_bar.Style]
	dec	al
	jz	.tabs_on_top
	dec	al
	jz	.tabs_on_bottom
	dec	al
	jz	.tabs_on_left
	dec	al
	jz	.tabs_on_right
	ret

  .tabs_on_top:
	add	[cur_editor.Bounds.Top],TBARH
	ret

  .tabs_on_bottom:
	sub	[cur_editor.Bounds.Bottom],TBARH
	ret

  .tabs_on_left:
	add	[cur_editor.Bounds.Left],ebx
	ret

  .tabs_on_right:
	sub	[cur_editor.Bounds.Right],ebx
	ret
endf
