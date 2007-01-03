
sz s_defname,'Untitled',0

;-----------------------------------------------------------------------------
func flush_cur_tab ;//////////////////////////////////////////////////////////
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
func set_cur_tab ;////////////////////////////////////////////////////////////
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
	call	update_caption
	pop	edi esi ecx
	ret
endf

;-----------------------------------------------------------------------------
func create_tab ;/////////////////////////////////////////////////////////////
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
	lea	ebp,[eax+ebx-sizeof.TABITEM]
	call	set_cur_tab

	mov	eax,1024
	call	mem.Alloc
	mov	[cur_tab.Editor.Data],eax
	mov	[cur_tab.Editor.Lines],1
	mov	[cur_tab.Editor.Columns],1
	xor	eax,eax
	mov	[cur_tab.Editor.TopLeft.X],eax
	mov	[cur_tab.Editor.TopLeft.Y],eax
	mov	[cur_tab.Editor.Caret.X],eax
	mov	[cur_tab.Editor.Caret.Y],eax
	mov	[cur_tab.Editor.SelStart.X],eax
	mov	[cur_tab.Editor.SelStart.Y],eax
	mov	edi,[cur_tab.Editor.Data]
	add	edi,4
	mov	ecx,10
	mov	[edi-4],ecx
	mov	[edi+10],eax
	mov	al,' '
	cld
	rep	stosb

	mov	esi,s_defname
	mov	edi,cur_tab.Editor.FilePath
	mov	ecx,s_defname.size
	rep	movsb
	mov	[cur_tab.Editor.FileName],0

	mov	[f_info.length],0
	mov	[cur_tab.Editor.Modified],0
	mov	[cur_tab.Editor.AsmMode],0

	call	flush_cur_tab
	call	update_caption
	call	drawwindow

	mov	ebp,cur_tab
	pop	edi esi ecx eax
	ret
endf

;-----------------------------------------------------------------------------
func delete_tab ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
	ret
endf

;-----------------------------------------------------------------------------
func get_tab_size ;///////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; EBP = TABITEM*
;-----------------------------------------------------------------------------
	push	eax
	cmp	[tab_bar.Style],3
	jae	.lp1
	lea	eax,[ebp+TABITEM.Editor.FilePath]
	add	eax,[ebp+TABITEM.Editor.FileName]
	call	strlen
	imul	ebx,eax,6
	add	ebx,9
	jmp	.lp2
  .lp1: call	get_max_tab_width
	mov	ebx,eax
  .lp2: mov	ecx,TBARH-1
	pop	eax
	ret
endf

;-----------------------------------------------------------------------------
func draw_tabctl ;////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------

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
	add	ecx,(TBARH-2)*65536-(TBARH-3)
	mov	edx,[cl_3d_inset]
	call	draw_framerect
	ret

  .tabs_on_bottom:
	add	ebx,1*65536-2
	mov	ecx,[tab_bar.Bounds.Bottom-2]
	xor	cx,cx
	add	ecx,-TBARH*65536+TBARH
	mcall	13
	mov	cx,1
	mcall	,,,[sc.work]
	add	ecx,-1*65536+2;-(TBARH-3)
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	mov	ecx,[tab_bar.Items.Count]
	mov	ebp,[tab_bar.Items]
	mov	esi,[tab_bar.Bounds.Left]
	inc	esi
	mov	edi,[tab_bar.Bounds.Bottom]
	add	edi,-TBARH+1
    @@: push	ecx

	call	get_tab_size
	rol	ebx,16
	mov	bx,si
	rol	ebx,16
	rol	ecx,16
	mov	cx,di
	rol	ecx,16
	mov	edx,[cl_3d_inset]
	call	draw_framerect

	cmp	ebp,[tab_bar.Current.Ptr]
	jne	.lp1
	push	ebx ecx
	add	ebx,1*65536-2
	dec	ecx
	mcall	13,,,[sc.work]
	pop	ecx ebx
  .lp1:
	pushad
	lea	eax,[ebp+TABITEM.Editor.FilePath]
	add	eax,[ebp+TABITEM.Editor.FileName]
	mov	edx,eax
	call	strlen
	mov	esi,eax
	shr	ecx,16
	mov	bx,cx
	add	ebx,0x00050005
	mcall	4,,0x00000000
	popad

	movzx	ebx,bx
	lea	esi,[esi+ebx+1]
	add	ebp,sizeof.TABITEM

	pop	ecx
	dec	ecx
	jnz	@b

	ret

  .tabs_on_left:
	call	get_max_tab_width
	mov	ebx,[tab_bar.Bounds.Left-2]
	mov	bx,ax
	add	ebx,1*65536
	add	ecx,1*65536-2
	push	eax
	mcall	13
	pop	eax
	add	eax,-2
	shl	eax,16
	add	ebx,eax
	mov	bx,3
	mov	edx,[cl_3d_inset]
	call	draw_framerect
	ret

  .tabs_on_right:
	call	get_max_tab_width
	mov	ebx,[tab_bar.Bounds.Right-2]
	mov	bx,ax
	shl	eax,16
	sub	ebx,eax
	add	ecx,1*65536-2
	mcall	13
	add	ebx,-1*65536
	mov	bx,3
	mov	edx,[cl_3d_inset]
	call	draw_framerect
	ret
endf

func get_max_tab_width
	mov	eax,100
	ret
endf

func align_editor_in_tab
	m2m	[cur_tab.Editor.Bounds.Left],[tab_bar.Bounds.Left]
	m2m	[cur_tab.Editor.Bounds.Top],[tab_bar.Bounds.Top]
	m2m	[cur_tab.Editor.Bounds.Right],[tab_bar.Bounds.Right]
	m2m	[cur_tab.Editor.Bounds.Bottom],[tab_bar.Bounds.Bottom]

	inc	[cur_tab.Editor.Bounds.Left]
	inc	[cur_tab.Editor.Bounds.Top]
	dec	[cur_tab.Editor.Bounds.Right]
	dec	[cur_tab.Editor.Bounds.Bottom]

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
	add	[cur_tab.Editor.Bounds.Top],TBARH
	ret

  .tabs_on_bottom:
	sub	[cur_tab.Editor.Bounds.Bottom],TBARH
	ret

  .tabs_on_left:
	call	get_max_tab_width
	add	[cur_tab.Editor.Bounds.Left],eax
	ret

  .tabs_on_right:
	call	get_max_tab_width
	sub	[cur_tab.Editor.Bounds.Right],eax
	ret
endf