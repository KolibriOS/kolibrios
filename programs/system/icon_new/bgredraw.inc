BGRedrawThread:

	mcall	40,010000b

BGRTRedrawIcons:
	mcall	48,5

	mov	[wsXe],ax
	mov	[wsYe],bx

	mov	dx,ax
	shr	eax,16
	mov	[wsX],ax
	sub	dx,ax
	mov	[wsW],dx

	mov	ax,bx
	shr	ebx,16
	mov	[wsY],bx
	sub	ax,bx
	mov	[wsH],ax

	mcall	14
	mov	[ScreenH],ax
	shr	eax,16
	mov	[ScreenW],ax

	mov	ecx,[MaxNumIcon]
	test	ecx,ecx
	jz	.NoDraw
	xor	ebx,ebx
    @@: push	ecx
	cmp	[IconNoDraw],ebx
	je	.nd
	stdcall DrawIcon,ebx,0
      .nd:
	inc	ebx
	pop	ecx
	loop	@b
   .NoDraw:

BGRTmessages:
	mcall	10
	sub	eax,5
	jz	BGRTRedrawIcons

	jmp	BGRTmessages






