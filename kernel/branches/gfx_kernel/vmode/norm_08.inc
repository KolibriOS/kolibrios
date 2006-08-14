;-----------------------------------------------------------------------------
func vm_mike_draw_rect.08 ;///////////////////////////////////////////////////
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

	mov	ebp,[0x3010]
	movsx	esi,word[ebp-0x3000+0]
	add	eax,esi
	add	ecx,esi
	movsx	esi,word[ebp-0x3000+4]
	add	ebx,esi
	add	edx,esi
;       add     eax,[ebp-0x3000+0]
;       add     ebx,[ebp-0x3000+4]
;       add     ecx,[ebp-0x3000+0]
;       add     edx,[ebp-0x3000+4]

	push	ecx
	mov	ecx,edi
	call	color_24_to_8_bits
	mov	edi,ecx
	pop	ecx

	mov	esi,[0x00003000]
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
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: mov	ebp,[bytes_per_scanline]
	imul	ebp,ebx
	add	ebp,eax
	add	ebp,[lfb_address]
  .xxx: push	eax ebp
	xchg	eax,edi
    @@: push	ebp
	call	[set_bank]
	mov	[ebp],al
	pop	ebp
;       mov     [ebp],di
	inc	ebp
	inc	edi
	cmp	edi,ecx
	jl	@b
	xchg	eax,edi
	pop	ebp eax
	add	ebp,[bytes_per_scanline]
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
func vm_mike_draw_line.08 ;///////////////////////////////////////////////////
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

	mov	eax,ecx
	call	color_24_to_8_bits
	and	eax,0x01000000
	or	ecx,eax

	movsx	eax,word[esp+4*7]  ; x end
	cmp	ax,[esp+4*7+2]	   ; x start
	je	dl.08.vert_line
	movsx	eax,word[esp+4*4]  ; y end
	cmp	ax,[esp+4*4+2]	   ; y start
	je	dl.08.horz_line

	sti
	popad
	retn

  dl.08.vert_line:
	test	edi,1
	jnz	.forced
	mov	esi,[0x00003000]
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
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: mov	ebp,[bytes_per_scanline]
	imul	ebp,ebx
	add	ebp,eax
	add	ebp,[lfb_address]
    @@: push	ebp
	call	[set_bank]
	test	ecx,0x01000000
	jz	.dr
	movzx	ecx,byte[ebp]
	mov	ecx,dword[palette_8_256+ecx*3]
	not	ecx
	call	color_24_to_8_bits
	or	ecx,0x01000000
  .dr:	mov	[ebp],cl
	pop	ebp
;       test    ecx,0x01000000
;       jz      .dr
;       mov     cx,[ebp]
;       not     cx
;  .dr: mov     [ebp],cx
	add	ebp,[bytes_per_scanline]
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

  dl.08.horz_line:
	test	edi,1
	jnz	.forced
	mov	esi,[0x00003000]
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
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: mov	ebp,[bytes_per_scanline]
	imul	ebp,eax
	add	ebp,ebx
	add	ebp,[lfb_address]
    @@: push	ebp
	call	[set_bank]
	test	ecx,0x01000000
	jz	.dr
	movzx	ecx,byte[ebp]
	mov	ecx,dword[palette_8_256+ecx*3]
	not	ecx
	call	color_24_to_8_bits
	or	ecx,0x01000000
  .dr:	mov	[ebp],cl
	pop	ebp
;       test    ecx,0x01000000
;       jz      .dr
;       mov     cx,[ebp]
;       not     cx
;  .dr: mov     [ebp],cx
	inc	ebp
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

func color_24_to_8_bits
begin
	push	ecx
	cmp	byte[esp+2],0 ; red=0 ?
	pop	ecx
	jne	.lp1
	cmp	ch,0 ; green=0 ?
	jne	@f
	shr	cl,2
	add	cl,192
	ret
    @@: cmp	cl,0 ; blue=0 ?
	jne	.lp1
	shr	cx,8+2
	add	cl,128
	ret
  .lp1: cmp	cx,0 ; green=0 && blue=0 ?
	jne	@f
	shr	ecx,16+2
	and	cl,0x3F
	add	cl,64
	ret
    @@: and	ecx,0x00FFFFFF
	shr	ecx,6
	shl	cx,6
	shr	ecx,6
	shl	cl,6
	shr	ecx,6
	ret
endf

;-----------------------------------------------------------------------------
func vm_mike_put_pixel.08 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
; edi = 0x00000001 force
;-----------------------------------------------------------------------------
begin
	pushad
	cli
	mov	edx,[bytes_per_scanline]
	imul	edx,ebx
	add	edx,eax
	add	edx,[lfb_address]
	test	edi,1
	jnz	.forced
	mov	esi,[0x00003000]
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
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: push	edx
	call	[set_bank]
	pop	edx
	test	ecx,0x01000000
	jz	.lp1
	not	byte[ebp]
	jmp	.exit
  .lp1: call	color_24_to_8_bits
	mov	[ebp],cl
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
	jz	@f
	movzx	eax,byte[ebp]
	mov	ecx,dword[palette_8_256+eax*3]
	not	ecx
	call	color_24_to_8_bits
	mov	byte[ebp],cl
	;not	byte[ebp]
	jmp	.exit
    @@: call	color_24_to_8_bits
	mov	[ebp],cl
	jmp	.exit
endf

;-----------------------------------------------------------------------------
func vm_mike_get_pixel.08 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
;-----------------------------------------------------------------------------
begin
	pushad
	cli

	imul	ebx,[bytes_per_scanline]
	add	eax,ebx
	add	eax,[lfb_address]
	push	eax
	call	[set_bank]
	pop	eax
	movzx	eax,byte[ebp]
	mov	eax,dword[palette_8_256+eax*3]
	and	eax,0x00FFFFFF
	mov	[esp+4*6],eax

	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_put_image.08 ;///////////////////////////////////////////////////
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
	mov	edi,[0x00003010]
	movsx	esi,word[edi-0x3000+0]
	add	eax,esi
	add	ecx,esi
	movsx	esi,word[edi-0x3000+4]
	add	ebx,esi
	add	edx,esi
;       add     eax,[esi-0x3000+0]
;       add     ebx,[esi-0x3000+4]
;       add     ecx,[esi-0x3000+0]
;       add     edx,[esi-0x3000+4]

	mov	esi,[0x00003000]
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
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: mov	esi,ebx
	sub	esi,[esp+4*4]
	imul	esi,[esp+4*8]
	mov	edi,eax
	sub	edi,[esp+4*7]
	lea	edi,[edi*3]
	add	esi,edi
	add	esi,ebp
	mov	edi,[bytes_per_scanline]
	imul	edi,ebx
	add	edi,eax
	add	edi,[lfb_address]
  .xxx: push	eax esi edi eax
    @@: lodsd
	xchg	eax,ecx
	call	color_24_to_8_bits
	xchg	eax,ecx

	push	ebp edi
	call	[set_bank]
	mov	[ebp],al
	pop	edi ebp
	inc	edi

	dec	esi
	inc	dword[esp]
	cmp	[esp],ecx
	jl	@b
	pop	eax edi esi eax
	add	esi,[esp+4*8]
	add	edi,[bytes_per_scanline]
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
func vm_mike_draw_bg.08 ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
begin
	pushad

	mov	eax,[viewport.left]
	mov	ebx,[viewport.top]
	mov	ecx,[viewport.right]
	mov	edx,[viewport.bottom]

	cli

	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1

	mov	esi,[CLIP_RECTS+4]
	mov	ebp,[esi]
	or	ebp,ebp
	jz	.exit
	add	esi,4
  .nx:	pushad
;       jif     eax,ge,[rr.left],@f
;       mov     eax,[rr.left]
;    @@: jif    ebx,ge,[rr.top],@f
;       mov     ebx,[rr.top]
;    @@: jif    ecx,l,[rr.right],@f
;       mov     ecx,[rr.right]
;    @@: jif    edx,l,[rr.bottom],@f
;       mov     edx,[rr.bottom]
;    @@:
	mov	eax,[rr.left]
	mov	ebx,[rr.top]
	mov	ecx,[rr.right]
	mov	edx,[rr.bottom]
	dec	ecx
	dec	edx
	mov	edi,0x007F7F7F
	call	vm_mike_draw_rect.08
	xor	edi,edi
	pushad
	shl	eax,16
	mov	ax,cx
	push	bx
	shl	ebx,16
	pop	bx
	mov	ecx,0x00CCCCCC
	call	vm_mike_draw_line.08
	add	ebx,0x00010001
	call	vm_mike_draw_line.08
	popad
	pushad
	shl	ebx,16
	mov	bx,dx
	push	ax
	shl	eax,16
	pop	ax
	mov	ecx,0x00CCCCCC
	call	vm_mike_draw_line.08
	add	eax,0x00010001
	call	vm_mike_draw_line.08
	popad
	pushad
	shl	eax,16
	mov	ax,cx
	mov	ebx,edx
	shl	ebx,16
	mov	bx,dx
	mov	ecx,0x00333333
	call	vm_mike_draw_line.08
	sub	ebx,0x00010001
	add	eax,0x00010000
	call	vm_mike_draw_line.08
	popad
	pushad
	mov	eax,ecx
	shl	eax,16
	mov	ax,cx
	shl	ebx,16
	mov	bx,dx
	mov	ecx,0x00333333
	call	vm_mike_draw_line.08
	sub	eax,0x00010001
	add	ebx,0x00010000
	call	vm_mike_draw_line.08
	popad
	popad
  .skip:
	add	esi,SR
	dec	ebp
	jnz	.nx

  .exit:
	sti
	popad
	retn
macro sdfsdSAFSDFG {
	pushad

	cmp	byte[bg_type],BGT_TILE
	je	.tiled

	mov	eax,[bg_width]
	cmp	eax,[screen_width]
	jne	@f
	mov	eax,[bg_height]
	cmp	eax,[screen_height]
	je	.tiled
    @@:
	imul	eax,[bg_width],3
	mov	[bg_bytes_per_scanline],eax

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
	mov	[mouse_invisible],1

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
    @@: call	is_intersect_rc
	jc	.put
	cmp	[mouse_invisible],0
	jne	.put
	call	[SF.draw_mouse_under]
	mov	[mouse_invisible],1
  .put: mov	ebp,[bytes_per_scanline]
	imul	ebp,ebx
	add	ebp,eax
	add	ebp,[lfb_address]
  .xxx: push	eax ebp
    @@: push	ebp
	call	[set_bank]
	mov	[ebp],di
	pop	ebp
;       mov     [ebp],di
	inc	ebp
	inc	eax
	cmp	eax,ecx
	jl	@b
	pop	ebp eax
	add	ebp,[bytes_per_scanline]
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
	mov	eax,bg_address
	mov	ebx,[bg_width-2]
	mov	bx,word[bg_height]
	xor	ecx,ecx
  .lp1: push	eax
	call	vm_mike_put_image.08.direct
	pop	eax
	rol	ecx,16
	add	cx,word[bg_width]
	cmp	cx,word[screen_width]
	jae	@f
	rol	ecx,16
	jmp	.lp1
    @@: shr	ecx,16
	add	ecx,[bg_height]
	cmp	ecx,[screen_height]
	jb	.lp1
	popad
	retn

  .color:
	mov	edi,[bg_address]
	and	edi,0x00FFFFFF
	call	vm_mike_draw_rect.08
	popad
	retn
}
endf