;-----------------------------------------------------------------------------
func vm_mike_draw_rect.32 ;///////////////////////////////////////////////////
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
	lea	ebp,[ebp+eax*4]
	add	ebp,[LFBAddress]
  .xxx: push	eax ebp
    @@: push	ebp
	call	[set_bank]
	mov	[ebp],edi
	pop	ebp
;       mov     [ebp],edi
	add	ebp,4
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
func vm_mike_draw_line.32 ;///////////////////////////////////////////////////
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
	je	dl.32.vert_line
	movsx	eax,word[esp+4*4]  ; y end
	cmp	ax,[esp+4*4+2]	   ; y start
	je	dl.32.horz_line

	sti
	popad
	retn

  dl.32.vert_line:
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
	lea	ebp,[ebp+eax*4]
	add	ebp,[LFBAddress]
    @@: push	ebp
	call	[set_bank]
	test	ecx,0x01000000
	jz	.dr
	mov	ecx,[ebp]
	not	ecx
	or	ecx,0x01000000
  .dr:	mov	[ebp],ecx
	pop	ebp
;       test    ecx,0x01000000
;       jz      .dr
;       mov     ecx,[ebp]
;       not     ecx
;       or      ecx,0x01000000
;  .dr: mov     [ebp],ecx
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

  dl.32.horz_line:
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
	lea	ebp,[ebp+ebx*4]
	add	ebp,[LFBAddress]
    @@: push	ebp
	call	[set_bank]
	test	ecx,0x01000000
	jz	.dr
	mov	ecx,[ebp]
	not	ecx
	or	ecx,0x01000000
  .dr:	mov	[ebp],ecx
	pop	ebp
;       test    ecx,0x01000000
;       jz      .dr
;       mov     ecx,[ebp]
;       not     ecx
;       or      ecx,0x01000000
;  .dr: mov     [ebp],ecx
	add	ebp,4
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
func vm_mike_put_pixel.32 ;///////////////////////////////////////////////////
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
	lea	edx,[edx+eax*4]
	add	edx,[LFBAddress]
;       test    ecx,0x01000000
;       jz      @f
;       mov     ecx,[edx]
;       not     ecx
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
  .put: push	edx
	call	[set_bank]
	pop	edx
	test	ecx,0x01000000
	jz	.lp1
	not	dword[ebp]
	jmp	.exit
  .lp1: mov	[ebp],ecx
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
	push	edx
	call	[set_bank]
	pop	edx
	test	ecx,0x01000000
	jz	.lp2
	not	dword[ebp]
	jmp	.exit
  .lp2: mov	[ebp],ecx
	jmp	.exit
endf

;-----------------------------------------------------------------------------
func vm_mike_get_pixel.32 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
;-----------------------------------------------------------------------------
begin
	pushad
	cli
	imul	ebx,[BytesPerScanLine]
	shl	eax,2
	add	eax,ebx
	add	eax,[LFBAddress]
	push	eax
	call	[set_bank]
	pop	eax
	mov	eax,[ebp]
	and	eax,0x00FFFFFF
	mov	[esp+4*6],eax
	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_put_image.32 ;///////////////////////////////////////////////////
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
;       add     eax,[esi-CURRENT_TASK+0]
;       add     ebx,[esi-CURRENT_TASK+4]
;       add     ecx,[esi-CURRENT_TASK+0]
;       add     edx,[esi-CURRENT_TASK+4]

;       cmp     eax,640
;       jb      @f
;       SHFLOW  '%x %x %x %x',eax,ebx,ecx,edx
;   @@:

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
	lea	edi,[edi+eax*4]
	add	edi,[LFBAddress]
  .xxx: push	eax esi edi eax
    @@: lodsd

	push	ebp edi
	call	[set_bank]
	mov	[ebp],eax
	pop	edi ebp
	add	edi,4

	dec	esi
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
func vm_mike_draw_bg.32 ;/////////////////////////////////////////////////////
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
	lea	ebp,[ebp+eax*4]
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
	add	ebp,4
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
	call	vm_mike_put_image.32.direct
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
	call	vm_mike_draw_rect.32
	popad
	retn
endf
