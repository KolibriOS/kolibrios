func color_24_to_4_bits
begin
	push	edx
	mov	dl,0
	cmp	al,85             ; blue
	jbe	.p13green
	or	dl,0x01
	cmp	al,170
	jbe	.p13green
	or	dl,0x08
  .p13green:
	shr	eax,8
	cmp	al,85             ; green
	jbe	.p13red
	or	dl,0x02
	cmp	al,170
	jbe	.p13red
	or	dl,0x08
  .p13red:
	cmp	ah,85             ; red
	jbe	.p13cont
	or	dl,0x04
	cmp	ah,170
	jbe	.p13cont
	or	dl,0x08
  .p13cont:
	mov	eax,edx
	pop	edx
	ret
endf

;-----------------------------------------------------------------------------
func vm_mike_draw_rect.04 ;///////////////////////////////////////////////////
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

	push	eax
	mov	eax,edi
	call	color_24_to_4_bits
	mov	edi,eax
	pop	eax

	mov	ebp,[0x3010]
	movsx	esi,word[ebp-0x3000+0]
	add	eax,esi
	add	ecx,esi
	movsx	esi,word[ebp-0x3000+4]
	add	ebx,esi
	add	edx,esi
;	add	eax,[ebp-0x3000+0]
;	add	ebx,[ebp-0x3000+4]
;	add	ecx,[ebp-0x3000+0]
;	add	edx,[ebp-0x3000+4]

	mov	esi,[0x00003000]
	mov	esi,[CLIP_RECTS+esi*4]
	mov	ebp,[esi]
	or	ebp,ebp
	jz	.exit
	add	esi,4
  .nx:	jif	ecx,le,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,le,[rr.top],.skip
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
  .put: sub	edx,ebx
	push	edx edi ebx eax
	mov	edi,ebx
	shl	edi,6
	shl	ebx,4
	add	edi,ebx
	shr	eax,3
	add	edi,eax
	add	edi,0x000A0000
	pop	eax ebx

	mov	ebx,eax
	mov	esi,ecx
	sub	esi,eax
	mov	dx,0x03CE

;edi = Offset in VMem
;esi = Length
;ebx = x
; dx = Graphix Controller

  mov   cl,bl		; Get StartBit
  and   ecx,07h

  mov   eax,esi
  add   eax,ecx
  cmp   eax,8		; Is x+Length<One Byte
  jb    .D_One

  mov   ax,0xff08	; 11111111b | BitMask Register
  shr   ah,cl		; BitMask
  out   dx,ax		; Write BitMask
  push  ecx
  mov   ah,[esp+4]
  mov   ecx,[esp+4+4]
  push  edi
.D_LL:			; Draw Left of Box
  mov   al,[edi]
  mov   [edi],ah
  add   edi,80		; edi:=edi+80
  dec   ecx
  jnz   .D_LL
  pop   edi
  inc   edi
  pop   ecx

  mov   ax,0xFF08	; BitMask Register
  out   dx,ax		; Write BitMask

  mov   eax,esi
  mov   ch,8
  sub   ch,cl
  movzx ecx,ch
  sub   eax,ecx
  shr   eax,3		; Length div 8

  push  edi ebx
  mov   ebx,[esp+8+4]
  mov   edx,eax
  mov   al,[esp+8]
.D_LC:
  mov   ecx,edx
  rep   stosb
  add   edi,80
  sub   edi,edx
  dec   ebx
  jnz   .D_LC
  pop   ebx edi
  add   edi,edx

  mov   ecx,ebx		; ecx:=x+Length
  add   ecx,esi
  and   ecx,07h		; ecx and 07
  mov   ah,0ffh
  shr   ah,cl		; BitMask
  jz    .D_End

  not   ah
  mov   al,8		; BitMask Register
  mov   dx,03ceh	; Graphics Controller
  out   dx,ax		; Write BitMask
  mov   ecx,[esp+4]
  mov   al,[esp]
.D_LR:
  mov   ah,[edi]
  mov   [edi],al
  add   edi,80
  dec   ecx
  jnz   .D_LR

  jmp   .D_End

.D_One:
  mov   ah,0ffh
  shr   ah,cl		; Left BitMask

  add   ebx,esi
  dec   ebx
  and   ebx,07h

  mov   ecx,7
  sub   ecx,ebx

  mov   bl,0ffh
  shl   bl,cl		; Right BitMask

  and   ah,bl		; Full  BitMask
  mov   al,8		; BitMask Register
  out   dx,ax		; Write BitMask
  mov   ecx,[esp+4]
  mov   al,[esp]
.D_L:
  mov   dl,[edi]	; Fill Latches
  mov   [edi],al	; Write Pixel
  add   edi,80
  dec   ecx
  jnz   .D_L
.D_End:
	pop	edi
	add	esp,4
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
func vm_mike_draw_line.04 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax(ebx)  [x start] shl 16 + [x end]
; ebx(ecx)  [y start] shl 16 + [y end]
; ecx(edx)  colour 0x00RRGGBB
; edi = 0x00000001 force
;-----------------------------------------------------------------------------
begin
	push	0
	pushad
	cli

	test	ecx,0x01000000
	jnz	.exit

	call	get_cursor_rect

	mov	eax,ecx
	call	color_24_to_4_bits
;	mov	[esp+4*8],ecx
	mov	[esp+4*8],al

	movsx	eax,word[esp+4*7]  ; x end
	cmp	ax,[esp+4*7+2]	   ; x start
	je	dl.vert_line
	movsx	eax,word[esp+4*4]  ; y end
	cmp	ax,[esp+4*4+2]	   ; y start
	je	dl.horz_line

  .exit:
	sti
	popad
	add	esp,4
	retn

  dl.vert_line:
	push	eax
	mov	cl,al
	and	cl,7
	mov	ax,0x8008
	shr	ah,cl
	mov	dx,0x03CE
	out	dx,ax
	pop	eax
	mov	ecx,[esp+4*8]
	
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
  .put: push	ebx eax
	mov	ebp,ebx
	shl	ebp,6
	shl	ebx,4
	add	ebp,ebx
	shr	eax,3
	add	ebp,eax
	add	ebp,0x000A0000
	pop	eax ebx
    @@: mov	ch,[ebp]
	mov	[ebp],cl
	add	ebp,80
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
	add	esp,4
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

  dl.horz_line:
	cld
	mov	ecx,[esp+4*8]
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
  .put:	push	edi ebx eax
	mov	edi,eax
	shl	edi,6
	shl	eax,4
	add	edi,eax
	shr	ebx,3
	add	edi,ebx
	add	edi,0x000A0000
	pop	eax ebx

	push	eax ebx edx esi ecx
	mov	esi,edx
	sub	esi,ebx
	inc	esi
	mov	dx,0x03CE
;edi = Offset in VMem
;esi = Length
;ebx = x
; dx = Graphix Controller
  mov   cl,bl		; Get StartBit
  and   ecx,07h

  mov   eax,esi
  add   eax,ecx
  cmp   eax,8		; Is x+Length<One Byte
  jb    .D_One

  mov   ax,0xFF08	; 11111111b | BitMask Register
  shr   ah,cl           ; BitMask
  out   dx,ax           ; Write BitMask
  mov   al,[edi]
  mov   eax,[esp]
  mov   [edi],al
  inc   edi

  mov   ax,0xFF08	; BitMask | BitMask Register
  out   dx,ax		; Write BitMask

  mov   eax,esi
  mov   ch,8
  sub   ch,cl
  mov   cl,ch
  xor   ch,ch
  sub   eax,ecx
  shr   eax,3		; Length div 8
  mov   ecx,eax
  mov   eax,[esp]
  rep   stosb

  mov   ecx,ebx		; ecx:=x+Length
  add   ecx,esi
  and   ecx,07h		; ecx and 07
  mov   ah,0ffh
  shr   ah,cl		; BitMask
  jz    .D_End

  not   ah
  mov   al,8		; BitMask Register
  out   dx,ax		; Write BitMask
  mov   cl,[edi]
  mov   eax,[esp]
  mov   [edi],al
  jmp   .D_End

.D_One:
  mov   ax,0xff08	; | BitMask Register
  shr   ah,cl		; Left BitMask

  add   ebx,esi
  dec   ebx
  and   ebx,07h

  mov   ecx,7
  sub   ecx,ebx

  mov   bl,0ffh
  shl   bl,cl		; Right BitMask

  and   ah,bl		; Full  BitMask
  out   dx,ax		; Write BitMask

  mov   dl,[edi]	; Fill Latches
  mov   eax,[esp]
  mov   [edi],al	; Write Pixel
.D_End:
	pop	ecx esi edx ebx eax edi

;	mov	ebp,[bytes_per_scanline]
;	imul	ebp,eax
;	lea	ebp,[ebp+ebx*4]
;	add	ebp,[lfb_address]
;    @@: test	ecx,0x01000000
;	jz	.dr
;	mov	ecx,[ebp]
;	not	ecx
;	or	ecx,0x01000000
;  .dr:	mov	[ebp],ecx
;	add	ebp,4
;	inc	ebx
;	cmp	ebx,edx
;	jle	@b
  .skip:
	add	esi,SR
	dec	edi
	jnz	.nx
  .exit:
	sti
	popad
	add	esp,4
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
func vm_mike_put_pixel.04 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
; edi = 0x00000001 force
;-----------------------------------------------------------------------------
begin
	pushad
	cli
;	mov	edx,[bytes_per_scanline]
;	imul	edx,ebx
;	lea	edx,[edx+eax*4]
;	add	edx,[lfb_address]
;	test	ecx,0x01000000
;	jz	@f
;	mov	ecx,[edx]
;	not	ecx
    @@: test	edi,1
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
  .put:;mov	[edx],ecx
;  mov   ax,SegA000      {Calculate Offset}
;  mov   es,ax
; mov   bx,[y]
  mov   edi,ebx
  shl   edi,6            ; 80*y
  shl   ebx,4
  add   edi,ebx

  push  ecx
  mov   cl,al
  shr   eax,3            ; /8
  add   edi,eax          ; 80*y + (x/8)

  and   cl,7             ; Get Bit that Changes
  mov   ax,0x8008
  shr   ah,cl
  mov   dx,0x03CE
  out   dx,ax
  add   edi,0x000A0000

  call	color_24_to_4_bits
  mov   ah,[edi]           ; dummy read
  mov   [edi],al

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

;  mov   ax,SegA000      {Calculate Offset}
;  mov   es,ax
; mov   bx,[y]
  mov   edi,ebx
  shl   edi,6            ; 80*y
  shl   ebx,4
  add   edi,ebx

  push  ecx
  mov   cl,al
  shr   eax,3            ; /8
  add   edi,eax          ; 80*y + (x/8)

  and   cl,7             ; Get Bit that Changes
  mov   ax,0x8008
  shr   ah,cl
  mov   dx,0x03CE
  out   dx,ax
  add   edi,0x000A0000

	pop	eax
	call	color_24_to_4_bits
	mov	ah,[edi]           ; dummy read
	mov	[edi],al

	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_get_pixel.04 ;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
;-----------------------------------------------------------------------------

clr_table dd \
  0x00000000,0x00000080,0x00008000,0x00008080,\
  0x00800000,0x00800080,0x00808000,0x00808080,\
  0x00CCCCCC,0x000000FF,0x0000FF00,0x0000FFFF,\
  0x00FF0000,0x00FF00FF,0x00FFFF00,0x00FFFFFF

begin
	pushad
	cli

  mov   edi,ebx
  shl   edi,6            ; 80*y
  shl   ebx,4
  add   edi,ebx
  mov   cl,al
  shr   eax,3            ; /8
  add   edi,eax          ; 80*y + (x/8)
  add   edi,0x000A0000

  and     ecx,7
  neg     ecx
  add     cl,7
  mov     dx,0x03CE
  xor     bl,bl
  mov     ah,3
@1:
  mov     al,4
  out     dx,al
  inc     dx
  mov     al,ah
  out     dx,al
  dec     dx
  mov     al,[edi]
  shr     al,cl
  and     al,1
  xchg    cl,ah
  shl     al,cl
  xchg    cl,ah
  or      bl,al
  dec     ah
  jns     @1

	and	ebx,0x0000000F
	mov	eax,[ebx*4+clr_table]
	mov	[esp+4*6],eax

	sti
	popad
	retn
endf

;-----------------------------------------------------------------------------
func vm_mike_put_image.04 ;///////////////////////////////////////////////////
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

	cld

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
;	add	eax,[esi-0x3000+0]
;	add	ebx,[esi-0x3000+4]
;	add	ecx,[esi-0x3000+0]
;	add	edx,[esi-0x3000+4]

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
  .put:
	mov	esi,ebx
	sub	esi,[esp+4*4]
	imul	esi,[esp+4*8]
	mov	edi,eax
	sub	edi,[esp+4*7]
	lea	edi,[edi*3]
	add	esi,edi
	add	esi,ebp
  mov   edi,ebx
  mov   ebp,ebx
  shl   edi,6            ; 80*y
  shl   ebp,4
  add   edi,ebp
  add   edi,0x000A0000
  .xxx: push	eax edx esi edi ebx eax
	or	ebp,-1
	mov	edx,0x03CE
    @@: mov	eax,[esp]
	push	ecx edi
  mov   cl,al
  shr   eax,3            ; /8
  add   edi,eax          ; 80*y + (x/8)

  mov   eax,0x8008
  and   cl,7             ; Get Bit that Changes
  shr   ah,cl
  out   dx,ax

	lodsd
	dec	esi
	and	eax,0x00FFFFFF
	cmp	eax,ebp
	jne	.ppp
	mov	cl,bl
	jmp	.ppp.2
  .ppp:
	mov	ebp,eax
	call	color_24_to_4_bits
	mov	bl,al
  .ppp.2:
 mov    al,[edi]           ; dummy read
 mov    [edi],cl
	pop	edi ecx
	inc	dword[esp]
	cmp	[esp],ecx
	jl	@b
	pop	eax ebx edi esi edx eax
	inc	ebx
	add	esi,[esp+4*8]
	add	edi,80
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
func vm_mike_draw_bg.04 ;/////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
begin
	pushad

	cmp	byte[0x460000-12],1
	je	.tiled

	mov	eax,[viewport.left]
	mov	ebx,[viewport.top]
	mov	ecx,[viewport.right]
	mov	edx,[viewport.bottom]
	mov	edi,[0x00300000]

	cli
	jif	eax,e,ecx,.exit
	jif	ebx,e,edx,.exit

	call	get_cursor_rect

	push	eax
	mov	eax,edi
	call	color_24_to_4_bits
	mov	edi,eax
	pop	eax

	mov	esi,[CLIP_RECTS+4]
	mov	ebp,[esi]
	or	ebp,ebp
	jz	.exit
	add	esi,4
  .nx:	jif	ecx,le,[rr.left],.skip
	jif	eax,ge,[rr.right],.skip
	jif	edx,le,[rr.top],.skip
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
  .put: sub	edx,ebx
	push	edx edi ebx eax
	mov	edi,ebx
	shl	edi,6
	shl	ebx,4
	add	edi,ebx
	shr	eax,3
	add	edi,eax
	add	edi,0x000A0000
	pop	eax ebx

	mov	ebx,eax
	mov	esi,ecx
	sub	esi,eax
	mov	dx,0x03CE

;edi = Offset in VMem
;esi = Length
;ebx = x
; dx = Graphix Controller

  mov   cl,bl		; Get StartBit
  and   ecx,07h

  mov   eax,esi
  add   eax,ecx
  cmp   eax,8		; Is x+Length<One Byte
  jb    .D_One

  mov   ax,0xff08	; 11111111b | BitMask Register
  shr   ah,cl		; BitMask
  out   dx,ax		; Write BitMask
  push  ecx
  mov   ah,[esp+4]
  mov   ecx,[esp+4+4]
  push  edi
.D_LL:			; Draw Left of Box
  mov   al,[edi]
  mov   [edi],ah
  add   edi,80		; edi:=edi+80
  dec   ecx
  jnz   .D_LL
  pop   edi
  inc   edi
  pop   ecx

  mov   ax,0xFF08	; BitMask Register
  out   dx,ax		; Write BitMask

  mov   eax,esi
  mov   ch,8
  sub   ch,cl
  movzx ecx,ch
  sub   eax,ecx
  shr   eax,3		; Length div 8

  push  edi ebx
  mov   ebx,[esp+8+4]
  mov   edx,eax
  mov   al,[esp+8]
.D_LC:
  mov   ecx,edx
  rep   stosb
  add   edi,80
  sub   edi,edx
  dec   ebx
  jnz   .D_LC
  pop   ebx edi
  add   edi,edx

  mov   ecx,ebx		; ecx:=x+Length
  add   ecx,esi
  and   ecx,07h		; ecx and 07
  mov   ah,0ffh
  shr   ah,cl		; BitMask
  jz    .D_End

  not   ah
  mov   al,8		; BitMask Register
  mov   dx,03ceh	; Graphics Controller
  out   dx,ax		; Write BitMask
  mov   ecx,[esp+4]
  mov   al,[esp]
.D_LR:
  mov   ah,[edi]
  mov   [edi],al
  add   edi,80
  dec   ecx
  jnz   .D_LR

  jmp   .D_End

.D_One:
  mov   ah,0ffh
  shr   ah,cl		; Left BitMask

  add   ebx,esi
  dec   ebx
  and   ebx,07h

  mov   ecx,7
  sub   ecx,ebx

  mov   bl,0ffh
  shl   bl,cl		; Right BitMask

  and   ah,bl		; Full  BitMask
  mov   al,8		; BitMask Register
  out   dx,ax		; Write BitMask
  mov   ecx,[esp+4]
  mov   al,[esp]
.D_L:
  mov   dl,[edi]	; Fill Latches
  mov   [edi],al	; Write Pixel
  add   edi,80
  dec   ecx
  jnz   .D_L
.D_End:
	pop	edi
	add	esp,4
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
	xor	edx,edx
  .lp1: push	eax
	call	vm_mike_put_image.04.direct
	pop	eax
	add	edx,[bg_width]
	cmp	edx,[screen_width]
	jae	@f
	shl	edx,16
	add	ecx,edx
	shr	edx,16
	jmp	.lp1
    @@: and	ecx,0x0000FFFF
	xor	edx,edx
	add	ecx,[bg_height]
    	cmp	ecx,[screen_height]
    	jb	.lp1
	popad
	retn
endf