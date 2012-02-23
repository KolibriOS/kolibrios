proc	xcf._.blend_rgb

	push		eax ebx

	xchg		al, bh
	mov		ah, bh
	neg		ax
	add		ax, 0xffff
	mul		ah
	neg		ah
	add		ah, 0xff
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	not		al
	div		bh
    @@:
	mov		ah, al
	movd		xmm1, eax

	pop		ebx eax
	push		ebx

	shr		eax, 8
	shr		ebx, 8

	xchg		al, bh
	mov		ah, bh
	neg		ax
	add		ax, 0xffff
	mul		ah
	neg		ah
	add		ah, 0xff
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	not		al
	div		bh
    @@:
	mov		ah, al
	movd		ebx, xmm1
	ror		ebx, 16
	mov		bx, ax
	rol		ebx, 16
	movd		xmm1, ebx

	pop		ebx

;	movdqu		xmm1, xword[xcf._.xmm_000000ff]
;	movdqa		xmm4, xmm1
;	movdqa		xmm5, xmm1
;	movdqa		xmm6, xmm2
;	psrldq		xmm6, 3
;	pand		xmm6, xmm1
;	psubw		xmm4, xmm6
;	movdqa		xmm6, xmm3
;	psrldq		xmm6, 3
;	pand		xmm6, xmm1
;	psubw		xmm5, xmm6
;	pmullw		xmm4, xmm5
;	psrlw		xmm4, 8
;	psubw		xmm1, xmm4
;	movdqa		xmm4, xmm1
;	movdqa		xmm1, xmm6
;	divps		xmm1, xmm4
;	packuswb	xmm1, xmm0
;	packuswb	xmm1, xmm0
;	punpcklbw	xmm1, xmm1

	punpcklbw	xmm1, xmm1
	punpcklbw	xmm1, xmm0

	movdqa		xmm7, xmm1
	psrlw		xmm7, 7
	paddw		xmm1, xmm7

	psubw		xmm3, xmm2
	pmullw		xmm3, xmm1
	psllw		xmm2, 8
	paddw		xmm3, xmm2
	pinsrw		xmm3, ebx, 3
	shr		ebx, 8
	pinsrw		xmm3, ebx, 7
	psrlw		xmm3, 8
	packuswb	xmm3, xmm0

	ret
endp


proc	xcf._.blend_gray

	xchg		al, bh
	mov		ah, bh
	neg		ax
	add		ax, 0xffff
	mul		ah
	neg		ah
	add		ah, 0xff
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	not		al
	div		bh
    @@:

	mov		ah, al

	movd		xmm1, eax
	punpcklbw	xmm1, xmm1
	punpcklbw	xmm1, xmm0

	movq		xmm7, xmm1
	psrlw		xmm7, 7
	paddw		xmm1, xmm7

	psubw		xmm3, xmm2
	pmullw		xmm3, xmm1
	psllw		xmm2, 8
	paddw		xmm3, xmm2
	pinsrw		xmm3, ebx, 1
	psrlw		xmm3, 8
	packuswb	xmm3, xmm0

	ret
endp


proc	xcf._.merge_32 _copy_width, _copy_height, _img_total_bpl, _bottom_total_bpl

	pxor		xmm0, xmm0

  .line:
	mov		ecx, [_copy_width]
	bt		ecx, 0
	jnc		.even
  .odd:
	movd		xmm2, [edi]
	movd		xmm3, [esi]
	add		esi, 4

	movdqa		xmm4, xmm2
	pminub		xmm4, xmm3
	pextrw		eax, xmm4, 3
	pextrw		ebx, xmm4, 1
	mov		al, bh

	push		eax
	pextrw		eax, xmm2, 3
	pextrw		ebx, xmm2, 1
	mov		bl, ah
	shl		ebx, 8
	pop		eax

	call		edx
	call		xcf._.blend_rgb
	movd		[edi], xmm3
	add		edi, 4

	cmp		ecx, 1
	je		.done

  .even:
	sub		ecx, 2
  .pixel:
	movq		xmm2, [edi]
	movq		xmm3, [esi]
	add		esi, 8

	movdqa		xmm4, xmm2
	pminub		xmm4, xmm3
	pextrw		eax, xmm4, 3
	pextrw		ebx, xmm4, 1
	mov		al, bh

	push		eax
	pextrw		eax, xmm2, 3
	pextrw		ebx, xmm2, 1
	mov		bl, ah
	shl		ebx, 8
	pop		eax

	call		edx
	call		xcf._.blend_rgb
	movq		[edi], xmm3
	add		edi, 8
	sub		ecx, 2
	jns		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line
  .done:
	ret
endp


proc	xcf._.merge_8a _copy_width, _copy_height, _img_total_bpl, _bottom_total_bpl
  .gray_line:
	mov		ecx, [_copy_width]
  .gray_pixel:
	mov		bx,  word[edi]
	lodsw
	movd		xmm2, ebx
	movd		xmm3, eax
	shr		eax, 8
	cmp		al, bh
	jna		@f
	mov		al, bh
    @@:
	pxor		xmm0, xmm0
	call		edx
	call		xcf._.blend_gray
	movd		eax, xmm3
	stosw
	dec		ecx
	jnz		.gray_pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.gray_line
	ret
endp


proc	xcf._.composite_rgb_00 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl

	pxor		xmm0, xmm0

  .line:
	mov		ecx, [_copy_width]
	bt		ecx, 0
	jnc		.even
  .odd:
	movlpd		xmm2, [edi]
	movlpd		xmm3, [esi]
	add		esi, 4

	pextrw		eax, xmm3, 3
	pextrw		ebx, xmm3, 1
	mov		al, bh

	push		eax
	pextrw		eax, xmm2, 3
	pextrw		ebx, xmm2, 1
	mov		bl, ah
	shl		ebx, 8
	pop		eax

	xchg		al, bh
	mov		ah, bh
	neg		al
	neg		ah
	dec		al
	dec		ah
	mul		ah
	neg		ah
	dec		ah
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	inc		al
	div		bh
    @@:
	mov		ah, al
	movd		xmm1, eax

	punpcklbw	xmm1, xmm1
	punpcklbw	xmm1, xmm0
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	psubsw		xmm3, xmm2
	pmullw		xmm3, xmm1
	psllw		xmm2, 8
	paddw		xmm3, xmm2
	pinsrw		xmm3, ebx, 3
	shr		ebx, 8
	pinsrw		xmm3, ebx, 7
	psrlw		xmm3, 8
	packuswb	xmm3, xmm0

	movd		[edi], xmm3
	add		edi, 4

	cmp		ecx, 1
	je		.done

  .even:
	sub		ecx, 2
  .pixel:
	movlpd		xmm2, [edi]
	movlpd		xmm3, [esi]
	add		esi, 8

	pextrw		eax, xmm3, 3
	pextrw		ebx, xmm3, 1
	mov		al, bh

	push		eax
	pextrw		eax, xmm2, 3
	pextrw		ebx, xmm2, 1
	mov		bl, ah
	shl		ebx, 8
	pop		eax


	push		eax ebx

	xchg		al, bh
	mov		ah, bh
	neg		al
	neg		ah
	dec		al
	dec		ah
	mul		ah
	neg		ah
	dec		ah
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	inc		al
	div		bh
    @@:
	mov		ah, al
	movd		xmm1, eax

	pop		ebx eax
	push		ebx

	shr		eax, 8
	shr		ebx, 8

	xchg		al, bh
	mov		ah, bh
	neg		ax
	add		ax, 0xffff
	mul		ah
	neg		ah
	add		ah, 0xff
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	not		al
	div		bh
    @@:
	mov		ah, al
	movd		ebx, xmm1
	ror		ebx, 16
	mov		bx, ax
	rol		ebx, 16
	movd		xmm1, ebx

	pop		ebx

	punpcklbw	xmm1, xmm1
	punpcklbw	xmm1, xmm0
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	psubsw		xmm3, xmm2
	pmullw		xmm3, xmm1
	psllw		xmm2, 8
	paddw		xmm3, xmm2
	pinsrw		xmm3, ebx, 3
	shr		ebx, 8
	pinsrw		xmm3, ebx, 7
	psrlw		xmm3, 8
	packuswb	xmm3, xmm0

	movq		[edi], xmm3
	add		edi, 8
	sub		ecx, 2
	jns		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line
  .done:
	ret
endp


proc	xcf._.composite_gray_00 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		bx, [edi]
	lodsw
	movd		xmm2, ebx
	movd		xmm3, eax

	shr		eax, 8

	xchg		al, bh
	mov		ah, bh
	neg		ax
	add		ax, 0xffff
	mul		ah
	neg		ah
	add		ah, 0xff
	xchg		ah, bh

	mov		al, 0xff
	cmp		ah, bh
	je		@f
	not		al
	div		bh
    @@:

	mov		ah, al

	movd		xmm1, eax
	pxor		xmm0, xmm0
	punpcklbw	xmm1, xmm1
	punpcklbw	xmm1, xmm0
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	psubw		xmm3, xmm2
	pmullw		xmm3, xmm1
	psllw		xmm2, 8
	paddw		xmm3, xmm2
	pinsrw		xmm3, ebx, 1
	psrlw		xmm3, 8
	packuswb	xmm3, xmm0
	movd		eax, xmm3
	stosw

	dec		ecx
	jnz		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line

	ret
endp


proc	xcf._.composite_indexed_00 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		bx, [edi]
	lodsw

	or		ah, 0x7f
	test		ah, 0x80
	jnz		@f
	mov		ax, bx
    @@:
	stosw

	dec		ecx
	jnz		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line
	ret
endp


proc	xcf._.composite_rgb_01 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl
	pushad

	pxor		xmm4, xmm4
	movd		xmm4, [xcf._.random_b]
	movd		xmm1, [xcf._.random_a]
	movd		xmm2, [xcf._.random_c]

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		ebx, [edi]
	lodsd

	movq		xmm0, xmm4
	pmuludq		xmm0, xmm1
	paddq		xmm0, xmm2
	movd		edx, xmm0
	movd		xmm4, edx
	pxor		xmm0, xmm0

	rol		eax, 8
	test		al, al
	jz		@f
	shr		edx, 17
	cmp		dl, al
	ja		@f
	ror		eax, 8
	or		eax, 0xff000000
	jmp		.done
    @@:
	mov		eax, ebx
  .done:
	stosd
	dec		ecx
	jnz		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line

  .quit:
	popad
	ret
endp


proc	xcf._.composite_gray_01 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl
	pushad

	pxor		xmm4, xmm4
	movd		xmm4, [xcf._.random_b]
	movd		xmm1, [xcf._.random_a]
	movd		xmm2, [xcf._.random_c]

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		ebx, [edi]
	lodsw

	movq		xmm0, xmm4
	pmuludq		xmm0, xmm1
	paddq		xmm0, xmm2
	movd		edx, xmm0
	movd		xmm4, edx
	pxor		xmm0, xmm0

	test		ah, ah
	jz		@f
	shr		edx, 17
	cmp		dl, ah
	ja		@f
	or		ax, 0xff00
	jmp		.done
    @@:
	mov		eax, ebx
  .done:
	stosw
	dec		ecx
	jnz		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line

  .quit:
	popad
	ret
endp


proc	xcf._.composite_rgb_03			; Multiply

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0
	pmullw		xmm3, xmm2
	psrlw		xmm3, 8

	ret
endp


proc	xcf._.composite_rgb_04			; Screen

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0
	movdqu		xmm5, xword[xcf._.xmm_00ff]
	movdqa		xmm4, xmm5
	psubw		xmm4, xmm2
	psubw		xmm3, xmm5
	pmullw		xmm3, xmm4
	psrlw		xmm3, 8
	paddw		xmm3, xmm5
	ret
endp


proc	xcf._.composite_rgb_05			; Overlay

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0
	movdqu		xmm4, xword[xcf._.xmm_00ff]
	psubw		xmm4, xmm2
	pmullw		xmm3, xmm4
	psrlw		xmm3, 7
	paddw		xmm3, xmm2
	pmullw		xmm3, xmm2
	psrlw		xmm3, 8

	ret
endp


proc	xcf._.composite_rgb_06			; Difference

	movdqa		xmm4, xmm3
	pminub		xmm4, xmm2
	pmaxub		xmm3, xmm2
	psubusb		xmm3, xmm4
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	ret
endp


proc	xcf._.composite_rgb_07			; Addition

	paddusb		xmm3, xmm2
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	ret
endp


proc	xcf._.composite_rgb_08			; Subtract

	movdqa		xmm4, xmm2
	psubusb		xmm4, xmm3
	movq		xmm3, xmm4
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	ret
endp


proc	xcf._.composite_rgb_09			; Darken Only

	pminub		xmm3, xmm2
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	ret
endp


proc	xcf._.composite_rgb_10			; Lighten Only

	pmaxub		xmm3, xmm2
	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	ret
endp


proc	xcf._.composite_rgb_11			; Hue (H of HSV)
	push		eax ebx ecx edx

	movd		eax, xmm3
	movd		ebx, xmm2

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	test		ah, ah
	jnz		@f
	ror		eax, 8
	ror		ebx, 8
	mov		ah, bh
	rol		eax, 8
	rol		ebx, 8
    @@:
	mov		ax, bx

	call		xcf._.hsv2rgb

	push		eax

	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	test		ah, ah
	jnz		@f
	ror		eax, 8
	ror		ebx, 8
	mov		ah, bh
	rol		eax, 8
	rol		ebx, 8
    @@:
	mov		ax, bx

	call		xcf._.hsv2rgb

	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_12			; Saturation (S of HSV)
	push		eax ebx ecx edx

	movd		eax, xmm3
	movd		ebx, xmm2

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	ror		eax, 8
	ror		ebx, 8
	mov		ah, bh
	rol		eax, 8
	rol		ebx, 8
	mov		al, bl

	call		xcf._.hsv2rgb

	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	ror		eax, 8
	ror		ebx, 8
	mov		ah, bh
	rol		eax, 8
	rol		ebx, 8
	mov		al, bl

	call		xcf._.hsv2rgb


	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_13			; Color (H and S of HSL)
	push		eax ebx ecx edx

	movd		eax, xmm3
	movd		ebx, xmm2

	call		xcf._.rgb2hsl
	xchg		eax,    ebx
	call		xcf._.rgb2hsl
	xchg		eax,    ebx

	mov		al, bl

	call		xcf._.hsl2rgb

	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1

	call		xcf._.rgb2hsl
	xchg		eax,    ebx
	call		xcf._.rgb2hsl
	xchg		eax,    ebx

	mov		al, bl

	call		xcf._.hsl2rgb

	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_14			; Value (V of HSV)
	push		eax ebx ecx edx

	movd		eax, xmm3
	movd		ebx, xmm2

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	ror		eax, 8
	ror		ebx, 8
	mov		ax, bx
	rol		eax, 8
	rol		ebx, 8

	call		xcf._.hsv2rgb

	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1

	call		xcf._.rgb2hsv
	xchg		eax, ebx
	call		xcf._.rgb2hsv
	xchg		eax, ebx

	ror		eax, 8
	ror		ebx, 8
	mov		ax, bx
	rol		eax, 8
	rol		ebx, 8

	call		xcf._.hsv2rgb

	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_15			; Divide
	push		eax ebx ecx

	movd		eax, xmm3
	movd		ebx, xmm2

	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	test		bl, bl
	jz		.clamp1
	cmp		ah, bl
	jae		.clamp2
	div		bl
	jmp		.done
  .clamp1:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done
  .clamp2:
	mov		al, 0xff
	jmp		.done
  .done:
	mov		ah, al
	loop		.color

	ror		eax, 8


	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1


	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color2:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	test		bl, bl
	jz		.clamp12
	cmp		ah, bl
	jae		.clamp22
	div		bl
	jmp		.done2
  .clamp12:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done2
  .clamp22:
	mov		al, 0xff
	jmp		.done2
  .done2:
	mov		ah, al
	loop		.color2

	ror		eax, 8


	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_16			; Dodge
	push		eax ebx ecx

	movd		eax, xmm3
	movd		ebx, xmm2

	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	neg		bl
	add		bl, 0xff
	test		bl, bl
	jz		.clamp1
	cmp		ah,  bl
	jae		.clamp2
	div		bl
	jmp		.done
  .clamp1:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done
  .clamp2:
	mov		al, 0xff
	jmp		.done
  .done:
	mov		ah, al
	loop		.color

	ror		eax, 8


	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1


	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color2:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	neg		bl
	add		bl, 0xff
	test		bl, bl
	jz		.clamp12
	cmp		ah,  bl
	jae		.clamp22
	div		bl
	jmp		.done2
  .clamp12:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done2
  .clamp22:
	mov		al, 0xff
	jmp		.done2
  .done2:
	mov		ah, al
	loop		.color2

	ror		eax, 8


	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_17			; Burn
	push		eax ebx ecx

	movd		eax, xmm3
	movd		ebx, xmm2

	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	neg		ah
	add		ah, 0xff
	test		bl, bl
	jz		.clamp1
	cmp		ah, bl
	jae		.clamp2
	div		bl
	jmp		.done
  .clamp1:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done
  .clamp2:
	mov		al, 0xff
	jmp		.done
  .done:
	mov		ah, al
	neg		ah
	add		ah, 0xff
	loop		.color

	ror		eax, 8


	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1


	rol		eax, 8
	rol		ebx, 8

	xchg		eax, ebx

	mov		ecx, 3

  .color2:
	rol		eax, 8
	rol		ebx, 8
	shl		ax, 8
	neg		ah
	add		ah, 0xff
	test		bl, bl
	jz		.clamp12
	cmp		ah, bl
	jae		.clamp22
	div		bl
	jmp		.done2
  .clamp12:
	mov		al, 0xff
	test		ah, ah
	jnz		@f
	not		al
    @@:
	jmp		.done2
  .clamp22:
	mov		al, 0xff
	jmp		.done2
  .done2:
	mov		ah, al
	neg		ah
	add		ah, 0xff
	loop		.color2

	ror		eax, 8


	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_18			; Hard Light
	push		eax ebx ecx

	movd		eax, xmm3
	movd		ebx, xmm2

	rol		eax, 8
	rol		ebx, 8

	mov		ecx, 3

  .color:
	rol		eax, 8
	rol		ebx, 8
	cmp		al, 127
	jna		.part1
	mov		ah, 0xff
	sub		ah, bl
	neg		al
	add		al, 0xff
	mul		ah
	shl		ax, 1
	neg		ah
	add		ah, 0xff
	jmp		.done
  .part1:
	mul		bl
	shl		ax, 1
  .done:
	loop		.color

	ror		eax, 8


	push		eax
	movq		xmm1, xmm3
	psrldq		xmm1, 4
	movd		eax, xmm1
	movq		xmm1, xmm2
	psrldq		xmm1, 4
	movd		ebx, xmm1


	rol		eax, 8
	rol		ebx, 8

	mov		ecx, 3

  .color2:
	rol		eax, 8
	rol		ebx, 8
	cmp		al, 127
	jna		.part12
	mov		ah, 0xff
	sub		ah, bl
	neg		al
	add		al, 0xff
	mul		ah
	shl		ax, 1
	neg		ah
	add		ah, 0xff
	jmp		.done2
  .part12:
	mul		bl
	shl		ax, 1
  .done2:
	loop		.color2

	ror		eax, 8


	movd		xmm3, eax
	pslldq		xmm3, 4
	pop		eax
	movd		xmm1, eax
	paddq		xmm3, xmm1

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_20			; Grain Extract

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0
	movdqu		xmm4, xmm2
	psubw		xmm3, xword[xcf._.xmm_0080]
	psubw		xmm4, xmm3
	movdqa		xmm3, xmm4
	packuswb	xmm3, xmm0
	punpcklbw	xmm3, xmm0
	ret
endp


proc	xcf._.composite_rgb_21			; Grain Merge

	punpcklbw	xmm2, xmm0
	punpcklbw	xmm3, xmm0
	paddw		xmm3, xmm2
	psubusw		xmm3, xword[xcf._.xmm_0080]
	packuswb	xmm3, xmm0
	punpcklbw	xmm3, xmm0
	ret
endp


; starting numbers for pseudo-random number generator
xcf._.random_a		dd	1103515245
xcf._.random_b		dd	777
xcf._.random_c		dd	12345

xcf._.xmm_8080		dq	0x8080808080808080, 0x8080808080808080
xcf._.xmm_0080		dq	0x0080008000800080, 0x0080008000800080
xcf._.xmm_00ff		dq	0x00ff00ff00ff00ff, 0x00ff00ff00ff00ff
xcf._.xmm_0100		dq	0x0100010001000100, 0x0100010001000100
xcf._.xmm_000000ff	dq	0x000000ff000000ff, 0x0000000000000000
