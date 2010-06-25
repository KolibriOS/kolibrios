substr:
	push	si di cx
	movzx	cx,byte[ds:si-1]
    @@: mov	al,[ds:si]
	cmp	al,[es:di]
	jne	@f
	inc	si
	inc	di
	dec	cx
	jnz	@b
;	cld
;	repe	cmps byte[ds:si],[es:di]
    @@: pop	cx di si
	ret

int2str:
        dec	bl
	jz	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	int2str
	pop	eax
    @@: cmp	al,10
	sbb	al,$69
	das
	mov	[ds:di],al
	inc	di
	ret

int2strnz:
	cmp	eax,ecx
	jb	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	int2strnz
	pop	eax
    @@: cmp	al,10
	sbb	al,$69
	das
	mov	[ds:di],al
	inc	di
	ret
