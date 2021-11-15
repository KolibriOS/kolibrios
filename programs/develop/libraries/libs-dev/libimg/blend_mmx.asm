;;================================================================================================;;
;;//// blend_mmx.asm //// (c) dunkaist, 2011-2012 ////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;

proc	xcf._.blend_rgb

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

	movd		mm1, eax
	punpcklbw	mm1, mm1
	punpcklbw	mm1, mm0

	movq		mm7, mm1
	psrlw		mm7, 7
	paddw		mm1, mm7

	psubw		mm3, mm2
	pmullw		mm3, mm1
	psllw		mm2, 8
	paddw		mm3, mm2
	psrlw		mm3, 8
	packuswb	mm3, mm0
	movd		eax, mm3
	rol		eax, 8
	mov		al, bh
	ror		eax, 8
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

	movd		mm1, eax
	punpcklbw	mm1, mm1
	punpcklbw	mm1, mm0

	movq		mm7, mm1
	psrlw		mm7, 7
	paddw		mm1, mm7

	psubw		mm3, mm2
	pmullw		mm3, mm1
	psllw		mm2, 8
	paddw		mm3, mm2
	psrlw		mm3, 8
	packuswb	mm3, mm0
	movd		eax, mm3
	mov		ah, bh

	ret
endp


proc	xcf._.merge_32 _copy_width, _copy_height, _img_total_bpl, _bottom_total_bpl
  .rgb_line:
	mov		ecx, [_copy_width]
  .rgb_pixel:
	mov		ebx, [edi]
	lodsd

	movd		mm2, ebx
	movd		mm3, eax
	shr		eax, 24
	shr		ebx, 16
	cmp		al, bh
	jna		@f
	mov		al, bh
    @@:
	pxor		mm0, mm0
	call		edx
	call		xcf._.blend_rgb
	stosd
	dec		ecx
	jnz		.rgb_pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.rgb_line
	emms
	ret
endp


proc	xcf._.merge_8a _copy_width, _copy_height, _img_total_bpl, _bottom_total_bpl
  .gray_line:
	mov		ecx, [_copy_width]
  .gray_pixel:
	mov		bx,  word[edi]
	lodsw
	movd		mm2, ebx
	movd		mm3, eax
	shr		eax, 8
	cmp		al, bh
	jna		@f
	mov		al, bh
    @@:
	pxor		mm0, mm0
	call		edx
	call		xcf._.blend_gray
	stosw
	dec		ecx
	jnz		.gray_pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.gray_line
	emms
	ret
endp


proc	xcf._.composite_rgb_00 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		ebx, [edi]
	lodsd
	movd		mm2, ebx
	movd		mm3, eax

	shr		eax, 24
	shr		ebx, 16

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

	movd		mm1, eax
	pxor		mm0, mm0
	punpcklbw	mm1, mm1
	punpcklbw	mm1, mm0
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	psubsw		mm3, mm2
	pmullw		mm3, mm1
	psllw		mm2, 8
	paddw		mm3, mm2
	psrlw		mm3, 8
	packuswb	mm3, mm0
	movd		eax, mm3
	rol		eax, 8
	mov		al, bh
	ror		eax, 8
	stosd

	dec		ecx
	jnz		.pixel
	add		esi, [_img_total_bpl]
	add		edi, [_bottom_total_bpl]
	dec		[_copy_height]
	jnz		.line

	ret
endp


proc	xcf._.composite_gray_00 _copy_width, _copy_height, _bottom_total_bpl, _img_total_bpl

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		bx, [edi]
	lodsw
	movd		mm2, ebx
	movd		mm3, eax

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

	movd		mm1, eax
	pxor		mm0, mm0
	punpcklbw	mm1, mm1
	punpcklbw	mm1, mm0
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	psubw		mm3, mm2
	pmullw		mm3, mm1
	psllw		mm2, 8
	paddw		mm3, mm2
	psrlw		mm3, 8
	packuswb	mm3, mm0
	movd		eax, mm3
	mov		ah, bh
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

	pxor		mm4, mm4
	movd		mm4, [xcf._.random_b]
	movd		mm1, [xcf._.random_a]
	movd		mm2, [xcf._.random_c]

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		ebx, [edi]
	lodsd

	movq		mm0, mm4
	pmuludq		mm0, mm1
	paddq		mm0, mm2
	movd		edx, mm0
	movd		mm4, edx
	pxor		mm0, mm0

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

	pxor		mm4, mm4
	movd		mm4, [xcf._.random_b]
	movd		mm1, [xcf._.random_a]
	movd		mm2, [xcf._.random_c]

  .line:
	mov		ecx, [_copy_width]
  .pixel:
	mov		ebx, [edi]
	lodsw

	movq		mm0, mm4
	pmuludq		mm0, mm1
	paddq		mm0, mm2
	movd		edx, mm0
	movd		mm4, edx
	pxor		mm0, mm0

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

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0
	pmullw		mm3, mm2
	psrlw		mm3, 8

	ret
endp


proc	xcf._.composite_rgb_04			; Screen

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0
	movq		mm4, [xcf._.mmx_00ff]
	movq		mm5, mm4
	psubw		mm5, mm3
	movq		mm3, mm4
	psubw		mm4, mm2
	pmullw		mm4, mm5
	psrlw		mm4, 8
	psubw		mm3, mm4

	ret
endp


proc	xcf._.composite_rgb_05			; Overlay

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0
	movq		mm4, [xcf._.mmx_00ff]
	psubw		mm4, mm2
	pmullw		mm3, mm4
	psrlw		mm3, 7
	paddw		mm3, mm2
	pmullw		mm3, mm2
	psrlw		mm3, 8

	ret
endp


proc	xcf._.composite_rgb_06			; Difference

	movq		mm4, mm3
	pminub		mm4, mm2
	pmaxub		mm3, mm2
	psubusb		mm3, mm4
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	ret
endp


proc	xcf._.composite_rgb_07			; Addition

	paddusb		mm3, mm2
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	ret
endp


proc	xcf._.composite_rgb_08			; Subtract

	movq		mm4, mm2
	psubusb		mm4, mm3
	movq		mm3, mm4
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	ret
endp


proc	xcf._.composite_rgb_09			; Darken Only

	pminub		mm3, mm2
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	ret
endp


proc	xcf._.composite_rgb_10			; Lighten Only

	pmaxub		mm3, mm2
	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	ret
endp


proc	xcf._.composite_rgb_11			; Hue (H of HSV)
	push		eax ebx ecx edx

	movd		eax, mm3
	movd		ebx, mm2

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

	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_12			; Saturation (S of HSV)
	push		eax ebx ecx edx

	movd		eax, mm3
	movd		ebx, mm2

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


	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_13			; Color (H and S of HSL)
	push		eax ebx ecx edx

	movd		eax, mm3
	movd		ebx, mm2

	call		xcf._.rgb2hsl
	xchg		eax,    ebx
	call		xcf._.rgb2hsl
	xchg		eax,    ebx

	mov		al, bl

	call		xcf._.hsl2rgb


	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_14			; Value (V of HSV)
	push		eax ebx ecx edx

	movd		eax, mm3
	movd		ebx, mm2

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


	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

  .quit:
	pop		edx ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_15			; Divide
	push		eax ebx ecx

	movd		eax, mm3
	movd		ebx, mm2

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
	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_16			; Dodge
	push		eax ebx ecx

	movd		eax, mm3
	movd		ebx, mm2

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
	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_17			; Burn
	push		eax ebx ecx

	movd		eax, mm3
	movd		ebx, mm2

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
	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_18			; Hard Light
	push		eax ebx ecx

	movd		eax, mm3
	movd		ebx, mm2

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
	movd		mm3, eax

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0

	pop		ecx ebx eax
	ret
endp


proc	xcf._.composite_rgb_20			; Grain Extract

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0
	movq		mm4, mm2
	psubw		mm3, [xcf._.mmx_0080]
	psubw		mm4, mm3
	movq		mm3, mm4
	packuswb	mm3, mm0
	punpcklbw	mm3, mm0
	ret
endp


proc	xcf._.composite_rgb_21			; Grain Merge

	punpcklbw	mm2, mm0
	punpcklbw	mm3, mm0
	paddw		mm3, mm2
	psubusw		mm3, [xcf._.mmx_0080]
	packuswb	mm3, mm0
	punpcklbw	mm3, mm0
	ret
endp


; starting numbers for pseudo-random number generator
xcf._.random_a		dd	1103515245
xcf._.random_b		dd	777
xcf._.random_c		dd	12345

xcf._.mmx_0080		dq	0x0080008000800080
xcf._.mmx_00ff		dq	0x00ff00ff00ff00ff
xcf._.mmx_0100		dq	0x0100010001000100
