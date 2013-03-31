;----------------------------------------------------------------------
draw_rectangle:
; in
; ebx = [coordinate on axis x]*65536 + [size on axis x]
; ecx = [coordinate on axis y]*65536 + [size on axis y]
	push	edx eax
;--------------------------------------
; top
	push	ebx ecx
	mov	eax,ebx
	shr	eax,16
	add	bx,ax
	
	mov	eax,ecx
	shr	eax,16
	mov	cx,ax

	sub	ecx,1 shl 16 +1
	sub	ebx,1 shl 16
	mcall	38,,,0xffffff
	
	sub	ecx,1 shl 16 +1
	sub	ebx,1 shl 16
	inc	ebx
	xor	edx,edx		; color black
	mcall
	pop	ecx ebx
;--------------------------------------
; bottom
	push	ebx ecx
	mov	eax,ebx
	shr	eax,16
	add	bx,ax
	
	mov	eax,ecx
	shr	eax,16
	add	cx,ax
	
	mov	eax,ecx
	ror	ecx,16
	mov	cx,ax
	rol	ecx,16
	
	sub	ebx,1 shl 16
	mcall	38,,,0xffffff
	
	add	ecx,1 shl 16 +1
	sub	ebx,1 shl 16
	inc	ebx
	xor	edx,edx
	mcall
	pop	ecx ebx	
;--------------------------------------
; left
	push	ebx ecx
	
	mov	eax,ebx
	shr	eax,16
	mov	bx,ax
	
	mov	eax,ecx
	shr	eax,16
	add	cx,ax
	
	sub	ebx,1 shl 16 +1
	sub	ecx,1 shl 16
	mcall	38,,,0xffffff
	
	sub	ebx,1 shl 16 +1
	xor	edx,edx
	mcall
	pop	ecx ebx	
;--------------------------------------
; right
	push	ebx ecx

	mov	eax,ebx
	shr	eax,16
	add	bx,ax
	
	mov	eax,ebx
	ror	ebx,16
	mov	bx,ax
	rol	ebx,16

	mov	eax,ecx
	shr	eax,16
	add	cx,ax
	
	sub	ecx,1 shl 16
	mcall	38,,,0xffffff
	
	add	ebx,1 shl 16 +1
	sub	ecx,1 shl 16
	inc	ecx
	xor	edx,edx
	mcall
	pop	ecx ebx	
;--------------------------------------
	pop	eax edx
	ret
;----------------------------------------------------------------------
	