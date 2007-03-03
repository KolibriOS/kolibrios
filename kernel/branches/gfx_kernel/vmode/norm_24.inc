;-----------------------------------------------------------------------------
func vm_mike_draw_rect.24 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax - x start
; ebx - y start
; ecx - x end
; edx - y end
; edi - color
;-----------------------------------------------------------------------------
;- eax(ebx) [x start]*65536 + [x size]
;- ebx(ecx) [y start]*65536 + [y size]
;- ecx(edx) color 0x00RRGGBB
;-----------------------------------------------------------------------------
begin
	pushad
	cli
	jif	eax,e,ecx,.exit
	jif	ebx,e,edx,.exit

	call	get_cursor_rect

	mov	ebp,[TASK_BASE]
	movsx	esi,word[ebp-CURRENT_TASK+0]
	add	eax,esi
	add	ecx,esi
	movsx	esi,word[ebp-CURRENT_TASK+4]
	add	ebx,esi
	add	edx,esi
;       add     eax,[ebp-CURRENT_TASK+0]
;       add     ebx,[ebp-CURRENT_TASK+4]
;       add     ecx,[ebp-CURRENT_TASK+0]
;       add     edx,[ebp-CURRENT_TASK+4]

	mov	esi,[CURRENT_TASK]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	ebp,[esi]
	or	ebp,ebp
	jz	.exit
	add	esi,4
  .nx:	jif	ecx,l,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,l,[rr.top],.skip
	jif	ebx,ge,[rr.bottom],.skip
	pushad
	jif	eax,ge,[rr.left],@f
	mov	eax,[rr.left]
    @@: jif	ebx,ge,[rr.top],@f
	mov	ebx,[rr.top]
    @@: jif	ecx,l,[rr.right],@f
	mov	ecx,[rr.right]
    @@: jif	edx,l,[rr.bottom],@f
	mov	edx,[rr.bottom]
    @@: call	is_intersect_rc
	jc	.put
	cmp	byte[MOUSE_VISIBLE],0
	je	.put
	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0
  .put: mov	ebp,[BytesPerScanLine]
	imul	ebp,ebx
	push	eax
	lea	eax,[eax*3]
	add	ebp,eax
	pop	eax
	add	ebp,[LFBAddress]
  .xxx: push	eax ebp
    @@: push	eax
	mov	eax,edi
	mov	[ebp],ax
	shr	eax,16
	mov	[ebp+2],al
	pop	eax
	add	ebp,3
	inc	eax
	cmp	eax,ecx
	jl	@b
	pop	ebp eax
	add	ebp,[BytesPerScanLine]
	inc	ebx
	cmp	ebx,edx
	jl	.xxx
	popad
  .skip:
	add	esi,SR
	dec	ebp
	jnz	.nx

  .exit:
	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_draw_line.24 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax(ebx)  [x start] shl 16 + [x end]
; ebx(ecx)  [y start] shl 16 + [y end]
; ecx(edx)  colour 0x00RRGGBB
; edi = 0x00000001 force
;-----------------------------------------------------------------------------
begin
	pushad
	cli

	call	get_cursor_rect

	movsx	eax,word[esp+4*7]  ; x end
	cmp	ax,[esp+4*7+2]	   ; x start
	je	dl.24.vert_line
	movsx	eax,word[esp+4*4]  ; y end
	cmp	ax,[esp+4*4+2]	   ; y start
	je	dl.24.horz_line

	sti
	popad
	retn

  dl.24.vert_line:
	test	edi,1
	jnz	.forced
	mov	esi,[CURRENT_TASK]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	edi,[esi]
	or	edi,edi
	jz	.exit
	add	esi,4
  .nx:	movsx	ebx,word[esp+4*4+2]  ; y start
	movsx	edx,word[esp+4*4]    ; y end
	cmp	ebx,edx
	je	.exit
	jl	@f
	xchg	ebx,edx
    @@: jif	eax,l,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,l,[rr.top],.skip
	jif	ebx,ge,[rr.bottom],.skip
	jif	ebx,ge,[rr.top],@f
	mov	ebx,[rr.top]
    @@: jif	edx,l,[rr.bottom],@f
	mov	edx,[rr.bottom]
	dec	edx
  .draw:
    @@: call	is_intersect_vln
	jc	.put
	cmp	byte[MOUSE_VISIBLE],0
	je	.put
	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0
  .put: mov	ebp,[BytesPerScanLine]
	imul	ebp,ebx
	push	eax
	lea	eax,[eax*3]
	add	ebp,eax
	pop	eax
	add	ebp,[LFBAddress]
    @@: test	ecx,0x01000000
	jz	.dr
	mov	ecx,[ebp]
	not	ecx
	or	ecx,0x01000000
  .dr:	mov	[ebp],cx
	ror	ecx,8
	mov	[ebp+2],ch
	rol	ecx,8
	add	ebp,[BytesPerScanLine]
	inc	ebx
	cmp	ebx,edx
	jle	@b
  .skip:
	add	esi,SR
	dec	edi
	jnz	.nx
  .exit:
	sti
	popad
	retn
  .forced:
	jif	eax,l,[viewport.left],.exit
	jif	eax,ge,[viewport.right],.exit
	movsx	ebx,word[esp+4*4+2]  ; y start
	movsx	edx,word[esp+4*4]    ; y end
	jif	ebx,e,edx,.exit
	jl	@f
	xchg	ebx,edx
    @@: jif	edx,l,[viewport.top],.exit
	jif	ebx,ge,[viewport.bottom],.exit
	jif	ebx,ge,[viewport.top],@f
	mov	ebx,[viewport.top]
    @@: jif	edx,l,[viewport.bottom],@f
	mov	edx,[viewport.bottom]
	dec	edx
    @@: mov	edi,1
	jmp	.draw

  dl.24.horz_line:
	test	edi,1
	jnz	.forced
	mov	esi,[CURRENT_TASK]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	edi,[esi]
	or	edi,edi
	jz	.exit
	add	esi,4
  .nx:	movsx	ebx,word[esp+4*7+2]  ; x start
	movsx	edx,word[esp+4*7]    ; x end
	cmp	ebx,edx
	je	.exit
	jl	@f
	xchg	ebx,edx
    @@: jif	eax,l,[rr.top],.skip
	jif	eax,ge,[rr.bottom],.skip
	jif	edx,l,[rr.left],.skip
	jif	ebx,ge,[rr.right],.skip
	jif	ebx,ge,[rr.left],@f
	mov	ebx,[rr.left]
    @@: jif	edx,l,[rr.right],@f
	mov	edx,[rr.right]
	dec	edx
  .draw:
    @@: call	is_intersect_hln
	jc	.put
	cmp	byte[MOUSE_VISIBLE],0
	je	.put
	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0
  .put: mov	ebp,[BytesPerScanLine]
	imul	ebp,eax
	push	eax
	lea	eax,[ebx*3]
	add	ebp,eax
	pop	eax
	add	ebp,[LFBAddress]
    @@: test	ecx,0x01000000
	jz	.dr
	mov	ecx,[ebp]
	not	ecx
	or	ecx,0x01000000
  .dr:	mov	[ebp],cx
	ror	ecx,8
	mov	[ebp+2],ch
	rol	ecx,8
	add	ebp,3
	inc	ebx
	cmp	ebx,edx
	jle	@b
  .skip:
	add	esi,SR
	dec	edi
	jnz	.nx
  .exit:
	sti
	popad
	retn
  .forced:
	jif	eax,l,[viewport.top],.exit
	jif	eax,ge,[viewport.bottom],.exit
	movsx	ebx,word[esp+4*7+2]  ; x start
	movsx	edx,word[esp+4*7]    ; x end
	cmp	ebx,edx
	je	.exit
	jl	@f
	xchg	ebx,edx
    @@: jif	edx,l,[viewport.left],.exit
	jif	ebx,ge,[viewport.right],.exit
	jif	ebx,ge,[viewport.left],@f
	mov	ebx,[viewport.left]
    @@: jif	edx,l,[viewport.right],@f
	mov	edx,[viewport.right]
	dec	edx
    @@: mov	edi,1
	jmp	.draw
endf

;-----------------------------------------------------------------------------
func vm_mike_put_pixel.24 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
; edi = 0x00000001 force
;-----------------------------------------------------------------------------
begin
	pushad
	cli
	mov	edx,[BytesPerScanLine]
	imul	edx,ebx
	add	edx,eax 		; lea esi,[eax*3]
	lea	edx,[edx+eax*2] 	; add edx,esi
	add	edx,[LFBAddress]
	test	ecx,0x01000000
	jz	@f
	mov	ecx,[edx]
	not	ecx
    @@: test	edi,1
	jnz	.forced
	mov	esi,[CURRENT_TASK]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	edi,[esi]
	or	edi,edi
	jz	.exit
	add	esi,4
    @@: jif	eax,l,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	ebx,l,[rr.top],.skip
	jif	ebx,ge,[rr.bottom],.skip
	call	get_cursor_rect
	call	is_intersect_pt
	jc	.put
	cmp	byte[MOUSE_VISIBLE],0
	je	.put
	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0
  .put: mov	[edx],cx
	shr	ecx,16
	mov	[edx+2],cl
  .exit:
	sti
	popad
	retn
  .skip:
	add	esi,SR
	dec	edi
	jnz	@b
	jmp	.exit
  .forced:
	jif	eax,l,[viewport.left],.exit
	jif	ebx,l,[viewport.top],.exit
	jif	eax,ge,[viewport.right],.exit
	jif	ebx,ge,[viewport.bottom],.exit
	mov	[edx],cx
	shr	ecx,16
	mov	[edx+2],cl
	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_get_pixel.24 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
;-----------------------------------------------------------------------------
begin
	pushad
	cli
	imul	ebx,[BytesPerScanLine]
	lea	eax,[eax*3]
	add	eax,ebx
	add	eax,[LFBAddress]
	mov	eax,[eax]
	and	eax,0x00FFFFFF
	mov	[esp+4*6],eax
	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_put_image.24 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax(ebx) pointer to image in memory - RRGGBBRRGGBB..
; ebx(ecx) image size [x]*65536+[y]
; ecx(edx) image position in window [x]*65536+[y]
; ret: eax 0 succesful, 1 overlapped
;-----------------------------------------------------------------------------
begin
	mov	eax,ebx
	mov	ebx,ecx
	mov	ecx,edx

  .direct:
	pushad
	cli
	jif	ebx,z,0x0000FFFF,.exit,test
	jif	ebx,z,0xFFFF0000,.exit,test

	call	get_cursor_rect

	mov	ebp,eax
	movsx	eax,word[esp+4*6+2]
	movsx	ebx,word[esp+4*6]
	movsx	ecx,word[esp+4*4+2]
	movsx	edx,word[esp+4*4]

	lea	edi,[ecx*3]
	push	edi
	add	ecx,eax
	add	edx,ebx
	mov	edi,[TASK_BASE]
	movsx	esi,word[edi-CURRENT_TASK+0]
	add	eax,esi
	add	ecx,esi
	movsx	esi,word[edi-CURRENT_TASK+4]
	add	ebx,esi
	add	edx,esi
;       add     eax,[edi-CURRENT_TASK+0]
;       add     ebx,[edi-CURRENT_TASK+4]
;       add     ecx,[edi-CURRENT_TASK+0]
;       add     edx,[edi-CURRENT_TASK+4]

	mov	esi,[CURRENT_TASK]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	edi,[esi]
	or	edi,edi
	jz	.exit
	add	esi,4
	cld
  .nx:	jif	ecx,l,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,l,[rr.top],.skip
	jif	ebx,ge,[rr.bottom],.skip
	pushad
	jif	eax,ge,[rr.left],@f
	mov	eax,[rr.left]
    @@: jif	ebx,ge,[rr.top],@f
	mov	ebx,[rr.top]
    @@: jif	ecx,l,[rr.right],@f
	mov	ecx,[rr.right]
    @@: jif	edx,l,[rr.bottom],@f
	mov	edx,[rr.bottom]
    @@: call	is_intersect_rc
	jc	.put
	cmp	byte[MOUSE_VISIBLE],0
	je	.put
	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0
  .put: mov	esi,ebx
	sub	esi,[esp+4*4]
	imul	esi,[esp+4*8]
	mov	edi,eax
	sub	edi,[esp+4*7]
	lea	edi,[edi*3]
	add	esi,edi
	add	esi,ebp
	mov	edi,[BytesPerScanLine]
	imul	edi,ebx
	push	eax
	lea	eax,[eax*3]
	add	edi,eax
	pop	eax
	add	edi,[LFBAddress]
  .xxx: push	eax esi edi eax
    @@: ;movsd
	;dec    esi
	;dec    edi
	movsw
	movsb
	inc	dword[esp]
	cmp	[esp],ecx
	jl	@b
	pop	eax edi esi eax
	add	esi,[esp+4*8]
	add	edi,[BytesPerScanLine]
	inc	ebx
	cmp	ebx,edx
	jl	.xxx
	popad
  .skip:
	add	esi,SR
	dec	edi
	jnz	.nx

  .exit:
	add	esp,4
	sti
	popad
	xor	eax,eax
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_draw_bg.24 ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
begin
	pushad

	cmp	byte[bg_type],BGT_TILE
	je	.tiled

	mov	eax,[bg_width]
	cmp	eax,[ScreenWidth]
	jne	@f
	mov	eax,[bg_height]
	cmp	eax,[ScreenHeight]
	je	.tiled
    @@:
	imul	eax,[bg_width],3
	mov	[bg_BytesPerScanLine],eax

	mov	eax,[viewport.left]
	mov	ebx,[viewport.top]
	mov	ecx,[viewport.right]
	mov	edx,[viewport.bottom]

	cmp	[bg_width],1
	jne	@f
	cmp	[bg_height],1
	je	.color
    @@:
	cli

	call	[SF.draw_mouse_under]
	mov	byte[MOUSE_VISIBLE],0

	mov	esi,[CLIP_RECTS+4]
	mov	ebp,[esi]
	or	ebp,ebp
	jz	.exit
	add	esi,4
  .nx:	jif	ecx,l,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,l,[rr.top],.skip
	jif	ebx,ge,[rr.bottom],.skip
	pushad
	jif	eax,ge,[rr.left],@f
	mov	eax,[rr.left]
    @@: jif	ebx,ge,[rr.top],@f
	mov	ebx,[rr.top]
    @@: jif	ecx,l,[rr.right],@f
	mov	ecx,[rr.right]
    @@: jif	edx,l,[rr.bottom],@f
	mov	edx,[rr.bottom]
    @@: ;call   is_intersect_rc
	;jc     .put
	;cmp    [mouse_invisible],0
	;jne    .put
	;call   [SF.draw_mouse_under]
	;mov    [mouse_invisible],1
  .put: mov	ebp,[BytesPerScanLine]
	imul	ebp,ebx
	push	eax
	lea	eax,[eax*3]
	add	ebp,eax
	pop	eax
	add	ebp,[LFBAddress]
  .xxx: push	eax ebp
    @@: push	ebp
	call	[set_bank]
	push	eax edx
	mul	[bg_width]
	div	dword[ScreenWidth]
	lea	edi,[eax*3]
	mov	eax,ebx
	mul	[bg_height]
	div	dword[ScreenHeight]
	mul	[bg_BytesPerScanLine]
	add	edi,eax
	add	edi,IMG_BACKGROUND
	mov	eax,[edi]
	mov	[ebp],eax
	pop	edx eax 
	pop	ebp
	add	ebp,3
	inc	eax
	cmp	eax,ecx
	jl	@b
	pop	ebp eax
	add	ebp,[BytesPerScanLine]
	inc	ebx
	cmp	ebx,edx
	jl	.xxx
	popad
  .skip:
	add	esi,SR
	dec	ebp
	jnz	.nx

  .exit:
	sti
	popad
	retn

  .tiled:
	mov	eax,IMG_BACKGROUND
	mov	ebx,[bg_width-2]
	mov	bx,word[bg_height]
	xor	ecx,ecx
  .lp1: push	eax
	call	vm_mike_put_image.24.direct
	pop	eax
	rol	ecx,16
	add	cx,word[bg_width]
	cmp	cx,word[ScreenWidth]
	jae	@f
	rol	ecx,16
	jmp	.lp1
    @@: shr	ecx,16
	add	ecx,[bg_height]
	cmp	ecx,[ScreenHeight]
	jb	.lp1
	popad
	retn

  .color:
	mov	edi,[IMG_BACKGROUND]
	and	edi,0x00FFFFFF
	call	vm_mike_draw_rect.24
	popad
	retn
endf