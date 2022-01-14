;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;								;;
;;  Copyright (C) KolibriOS team 2016. All rights reserved.	;;
;; Distributed under terms of the GNU General Public License	;;
;;								;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../macros.inc'
include '../../../proc32.inc'


; calculate string width in pixels
proc	stringWidth, charQuantity, charHeight
	mov	eax,[charHeight]
	shr	eax,1
	mul	[charQuantity]
	ret
endp


; calculate amount of chars that fits given width
proc	charsFit, areaWidth, charHeight
	shr	[charHeight],1
	mov	eax,[areaWidth]
	xor	edx,edx
	div	[charHeight]
	ret
endp


; calculate amount of valid chars in UTF-8 string
; supports zero terminated string (set byteQuantity = -1)
cntUTF_8: ;old function name
proc	countUTF8Z, string, byteQuantity
	push	esi
	mov	edx,[byteQuantity]
	inc	edx
	xor	ecx,ecx
	dec	ecx
	mov	esi,[string]
@@:
	inc	ecx
	dec	edx
	jz	.done
	lodsb
	test	al,al
	jz	.done
	jns	@b
	dec	ecx
	shl	al,1
	jns	@b
.next:
	mov	ah,[esi]
	test	ah,ah
	jns	@b
	shl	ah,1
	js	@b
	inc	esi
	dec	edx
	jz	@f
	shl	al,1
	js	.next
	inc	ecx
	jmp	@b
@@:
	inc	ecx
.done:
	mov	eax,ecx
	pop	esi
	ret
endp


; draw text on 24bpp or 32bpp image
; autofits text between 'x' and 'xSize'
proc	drawText, canvas, x, y, string, charQuantity, fontColor, params
; [canvas]:
;  xSize	dd  ?
;  ySize	dd  ?
;  picture	rb  xSize * ySize * bpp

; fontColor	dd  AARRGGBB
;  AA = alpha channel	; 0 = transparent, FF = non transparent

; params	dd  ffeewwhh
;  hh = char height
;  ww = char width	; 0 = auto (proportional)
;  ee = encoding	; 1 = cp866, 2 = UTF-16LE, 3 = UTF-8
;  ff = flags		; 0001 = bold, 0010 = italic
			; 0100 = underline, 1000 = strike-through
; 00010000 = align right, 00100000 = align center
; 01000000 = set text area between higher and lower halfs of 'x'
; 10000000 = 32bpp canvas insted of 24bpp
; all flags combinable, except align right + align center

; returns: eax = char width (0 = error), ecx = text start X
	pusha
	movzx	eax,byte[params+1]
	test	eax,eax
	jnz	@f
	mov	al ,byte[params]
	shr	al ,1
	mov	byte[params+1],al
@@:
	cmp	al, 22
	jc	@f
	mov	al, 21
	mov	byte[params+1],21
@@:
	inc	[charQuantity]
	mul	[charQuantity]
	mov	ebx,eax
	mov	esi,[canvas]
	mov	esi,[esi]
	test	byte[params+3],64
	jz	.fit
	movzx	eax,word[x]
	movzx	ecx,word[x+2]
	cmp	eax,ecx
	jnc	@f
	xchg	eax,ecx
@@:
	mov	[x],ecx
	cmp	esi,eax
	jc	.fit
	mov	esi,eax
.fit:
	mov	eax,esi
	sub	eax,[x]
	jnc	@f
	popa
	xor	eax,eax
	jmp	.exit
@@:
	cmp	eax,ebx
	jnc	@f
	mov	ebx,eax
	div	[charQuantity]
	mov	byte[params+1],al
	sub	ebx,edx
@@:
	mov	eax,esi
	sub	eax,ebx
	test	byte[params+3],32
	jz	@f
	sub	eax,[x]
	shr	eax,1
	add	[x],eax
	jmp	.ok
@@:
	test	byte[params+3],16
	jz	.ok
	mov	[x],eax
.ok:
	movzx	eax,byte[params+1]
	lea	eax,[eax*2+eax]
	shr	eax,3
	test	byte[params+1],7
	jz	@f
	inc	eax
@@:
	mov	ecx,eax
	push	eax
	shl	eax,3
	mul	[charQuantity]
	shl	ecx,4
	push	ecx
	push	eax
	mul	ecx
	push	eax
	lea	ecx,[eax*4+8]
	mcall	68,12

	pop	ecx
	popd	[eax]
	popd	[eax+4]
	push	eax
	lea	edi,[eax+8]
	xor	eax,eax
	rep stosd
	pop	edi
	pop	ecx
	shl	ecx,4
	mov	ch, byte[params+2]
	shl	ecx,22
	shr	ecx,2
	dec	ecx
	bts	ecx,27
	mov	esi,[charQuantity]
	dec	esi
	xor	ebx,ebx
	mcall	4,,,[string]

	xor	eax,eax
	mov	ebx,[edi]
	mov	ecx,[edi+4]
	push	edi
	add	edi,8
	test	byte[params+3],1
	jnz	.bold
	movzx	esi,byte[params]
@@:
	pusha
	call	verSub
	popa
	sub	esi,16
	jg	@b
	jmp	@f
.bold:
	imul	ecx,ebx
	dec	eax
	movzx	ebx,byte[params+1]
.loop:
	push	edi
	push	ecx
	call	horAdd
	pop	ecx
	pop	edi
	sub	ebx,8
	jg	.loop
@@:

	test	byte[params+3],2
	jz	@f
	mov	edi,[esp]
	mov	ecx,[edi]
	mov	ebx,[edi+4]
	add	edi,8
	mov	esi,edi
	call	italic
@@:

	mov	edi,[esp]
	mov	eax,[edi]
	mov	ebx,[edi+4]
	add	edi,8
	mov	esi,edi
	movzx	edx,byte[params]
	call	verScale

	mov	eax,[charQuantity]
	mul	byte[params+1]
	mov	esi,[esp]
	mov	edx,[esi]
	add	esi,8
	mov	edi,esi
	mov	ebx,eax
	push	eax
	movzx	eax,byte[params]
	call	ClearType

	test	byte[params+3],4
	jz	@f
	movzx	eax,byte[params]
	movzx	esi,byte[params+1]
	mov	ebx,eax
	dec	eax
	call	drawLine
@@:

	test	byte[params+3],8
	jz	@f
	movzx	eax,byte[params]
	movzx	esi,byte[params+1]
	mov	ebx,eax
	shr	eax,1
	call	drawLine
@@:

	mov	esi,[canvas]
	mov	eax,[esi]
	mul	[y]
	add	eax,[x]
	mov	edi,eax
	mov	eax,[esi]
	pop	ecx
	sub	eax,ecx
	mov	ebx,[fontColor]
	movzx	edx,byte[params]
	test	byte[params+3],128
	mov	ebp,3
	jnz	@f
	lea	edi,[edi*2+edi+8]
	lea	eax,[eax*2+eax]
	jmp	.go
@@:
	lea	edi,[edi*4+8]
	shl	eax,2
	inc	ebp
.go:
	add	edi,esi
	mov	esi,[esp]
	add	esi,8
	call	putOnPicture

	pop	ecx
	mcall	68,13
	popa
	movzx	eax,byte[params+1]
	mov	ecx,[x]
.exit:
	ret
endp


drawLine:
	mov	ecx,[esp+4]
	lea	ecx,[ecx*2+ecx]
	mul	ecx
	lea	esi,[esi*2+esi]
	sub	ecx,esi
	add	esi,ecx
	add	esi,ecx
	shr	ecx,2
	add	eax,[esp+8]
	lea	edi,[eax+8]
	mov	eax,-1
@@:
	push	ecx
	rep stosd
	sub	edi,esi
	pop	ecx
	sub	ebx,16
	jg	@b
	ret


; make horizontal lines thinner
; one color background only
verSub:
; edi -> buffer (32bpp)
; eax = background color
; ebx = width
; ecx = height
	push	ebp
	mov	edx,ebx
	mov	esi,edi
	mov	ebp,ecx
	shl	ebx,2
.start:
	cmp	[edi],eax
	jnz	@f
.loop:
	add	edi,ebx
	dec	ecx
	jnz	.start
	jmp	.next
@@:
	mov	[edi],eax
@@:
	add	edi,ebx
	dec	ecx
	jz	.next
	cmp	[edi],eax
	jnz	@b
	jmp	.loop
.next:
	add	esi,4
	mov	edi,esi
	mov	ecx,ebp
	dec	edx
	jnz	.start
	pop	ebp
	ret


; make vertical lines thicker
horAdd:
; edi -> buffer (32bpp)
; eax = font color
; ecx = total number of pixels
	repnz scasd
	jcxz	.end
	repz  scasd
	mov	[edi-4],eax
	jmp	horAdd
.end:
	ret


; esi=edi supported (32bpp)
italic:
; esi -> source buffer
; edi -> result buffer
; ebx = height
; ecx = width
	shl	ecx,2
	shr	ebx,2
	mov	eax,ecx
	mul	ebx
	shl	eax,2
	add	esi,eax
	add	edi,eax
	dec	ecx
	sub	esi,8
	sub	edi,4
	push	ebx
	std
@@:
	push	ecx
	rep movsd
	pop	ecx
	sub	esi,4
	dec	ebx
	jnz	@b
	pop	ecx
	mov	eax,[edi+4]
	rep stosd
	cld
	ret


; vertical downscale
; white-black-gray only
; esi=edi supported (32bpp)
verScale:
; esi -> source buffer
; edi -> result buffer
; eax = width
; ebx = source height
; edx = result height
	push	ebp
	dec	eax
	shl	eax,2
	push	eax
	add	eax,4
	push	edx
	push	esi
	push	edi
	push	eax
	mov	ecx,edx
.scale:
	mov	al, [esi]
	add	esi,[esp]
	mul	cl
	neg	ecx
	add	ecx,ebx
	mov	ebp,eax
	mov	al, [esi]
@@:
	cmp	edx,ecx
	jnc	@f
	add	esi,[esp]
	mul	dl
	sub	ecx,edx
	add	ebp,eax
	mov	al, [esi]
	jmp	@b
@@:
	mul	cl
	add	eax,ebp
	div	bl
	mov	[edi],al
	mov	[edi+1],al
	mov	[edi+2],al
	add	edi,[esp]
	neg	ecx
	add	ecx,edx
	jnz	@f
	add	ecx,edx
	add	esi,[esp]
@@:
	dec	dword[esp+12]
	jnz	.scale
	mov	edi,[esp+4]
	mov	esi,[esp+8]
	mov	[esp+12],edx
	add	edi,[esp+16]
	add	esi,[esp+16]
	sub	dword[esp+16],4
	jnc	.scale
	add	esp,20
	pop	ebp
	ret


; horizontal downscale
; minimum — x3, maximum — x6
; white-black-gray only
; esi=edi supported
ClearType:
; esi -> source buffer (32bpp)
; edi -> result buffer (24bpp)
; eax = height
; edx = source width
; ebx = result width
	push	ebp
	lea	ebx,[ebx*2+ebx]
	imul	eax,ebx
	push	eax
	push	edi
	push	eax
	push	edx
	mov	ecx,ebx
.scale:
	movzx	eax,byte[esi]
	add	esi,4
	mul	ecx
	neg	ecx
	add	ecx,[esp]
	mov	ebp,eax
	movzx	eax,byte[esi]
	cmp	ebx,ecx
	jnc	@f
	add	esi,4
	mul	ebx
	sub	ecx,ebx
	add	ebp,eax
	movzx	eax,byte[esi]
@@:
	mul	ecx
	add	eax,ebp
	div	dword[esp]
	stosb
	neg	ecx
	add	ecx,ebx
	jnz	@f
	add	ecx,ebx
	add	esi,4
@@:
	dec	dword[esp+4]
	jnz	.scale
	pop	edi
	pop	edi
	mov	edi,[esp]
	mov	ecx,[esp+4]
	movzx	ebx,byte[edi]
	xor	eax,eax
	dec	ecx
.degradation:
	mov	al, [edi]
	shl	eax,1
	lea	eax,[eax*2+eax]
	lea	edx,[ebx*4+ebx]
	mov	bl, [edi+1]
	add	eax,edx
	lea	edx,[ebx*4+ebx]
	mov	bl, [edi]
	add	eax,edx
	shr	eax,4
	stosb
	dec	ecx
	jnz	.degradation
	pop	edi
	pop	ecx
.colRev:
	mov	al,[edi]
	xchg	al,[edi+2]
	mov	[edi],al
	add	edi,3
	sub	ecx,3
	jnz	.colRev
	pop	ebp
	ret


; apply color on font, put font on picture
; white font on black background only, smoothing allowed
putOnPicture:
; esi -> font buffer (24bpp)
; edi -> picture buffer
; ebx = font color
; ecx = width
; edx = height
; eax = picture buffer line gap in bytes
; ebp = picture buffer bytes per pixel
	push	edx
	push	eax
	push	ecx
	push	ebp
	xor	eax,eax
	rol	ebx,8
	mov	ebp,ecx
.start:
	cmp	byte[esi], 0
	jz	@f
	mov	al, [esi]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	mul	bh
	mov	edx,eax
	mov	al, [edi]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi],al
@@:
	cmp	byte[esi+1], 0
	jz	@f
	mov	al, [esi+1]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	rol	ebx,16
	mul	bl
	rol	ebx,16
	mov	edx,eax
	mov	al, [edi+1]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi+1],al
@@:
	cmp	byte[esi+2], 0
	jz	@f
	mov	al, [esi+2]
	mul	bl
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	cl, 255
	sub	cl, al
	rol	ebx,16
	mul	bh
	rol	ebx,16
	mov	edx,eax
	mov	al, [edi+2]
	mul	cl
	add	eax,edx
	mov	al, ah
	shr	ah, 7
	add	al, ah
	mov	[edi+2],al
@@:
	add	esi,3
	add	edi,[esp]
	dec	ebp
	jnz	.start
	mov	ebp,[esp+4]
	add	edi,[esp+8]
	dec	dword[esp+12]
	jnz	.start
	add	esp,16
	ret

align 4
@EXPORT:
export	drawText,	'drawText', \
	cntUTF_8,	'cntUTF-8', \  ;old function name
	countUTF8Z,	'countUTF8Z', \
	charsFit,	'charsFit', \
	stringWidth,	'strWidth'
