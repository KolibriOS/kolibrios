;* Call: ***************************************************************
lzma_decompress:
	push	esi
	push	edi
	push	ebx
	push	ebp
	mov	esi,[esp+0x14]
	xor	ebp,ebp
	mov	edi,0xD6B8
	inc	esi
	lodsd 
	bswap	eax
	stosd 
	xor	eax,eax
	dec	eax
	stosd 
	stosd 
	stosd 
	stosd 
	xchg	esi,eax
	stosd 
	mov	ecx,0x1F36
	mov	eax,0x400
	mov	edi,0x59E0
	rep	stosd 
	mov	edi,[esp+0x18]
	mov	ebx, edi
	add	ebx,[esp+0x1C]
;--------------------------------------------------------------------
.labl_00:
	cmp	edi,ebx
	jnb	.labl_19
	mov	edx,edi
	and	edx,3
	push	eax
	mov	eax,ebp
	shl	eax,6
	lea	eax,[eax+edx*4+0x59E0]
	call	cm_pr_20
	pop	eax
	jb	.labl_06
	movzx	eax,al
	shr	eax,5
	imul	eax,eax,0xC00
	add	eax,0x76B8
	mov	cl,1
	cmp	ebp,7
	jb	.labl_02
	mov	dl,[edi+esi]
;--------------------------------------------------------------------
.labl_01:
	add	dl,dl
	setb	ch
	push	eax
	lea	eax,[eax+ecx*4+0x400]
	call	cm_pr_20
	pop	eax
	adc	cl,cl
	jb	.labl_03
	xor	ch,cl
	test	ch,1
	mov	ch,0
	je	.labl_01
;--------------------------------------------------------------------
.labl_02:
	push	eax
	lea	eax,[eax+ecx*4]
	call	cm_pr_20
	pop	eax
	adc	cl,cl
	jnb	.labl_02
;--------------------------------------------------------------------
.labl_03:
	mov	eax,ebp
	cmp	al,4
	jb	.labl_04
	cmp	al,0xA
	mov	al,3
	jb	.labl_04
	mov	al,6
;--------------------------------------------------------------------
.labl_04:
	sub	ebp,eax
	xchg	ecx,eax
;--------------------------------------------------------------------
.labl_05:
	stosb 
	jmp	.labl_00
;--------------------------------------------------------------------
.labl_06:
	lea	eax,[0x5CE0+ebp*4]
	call	cm_pr_20
	jnb	.labl_09
	add	eax,0x30 ;'0'
	call	cm_pr_20
	jb	.labl_07
	mov	eax, ebp
	shl	eax,6
	lea	eax,[eax+edx*4+0x5DA0]
	call	cm_pr_20
	jb	.labl_08
	cmp	ebp,7
	sbb	ebp,ebp
	lea	ebp,[ebp+ebp+0xB]
	mov	al,[edi+esi]
	jmp	.labl_05
;--------------------------------------------------------------------
.labl_07:
	add	eax,0x30 ;'0'
	call	cm_pr_20
	xchg	esi,[0xD6C0]
	jnb	.labl_08
	add	eax,0x30 ;'0'
	call	cm_pr_20
	xchg	esi,[0xD6C4]
	jnb	.labl_08
	xchg	esi,[0xD6C8]
;--------------------------------------------------------------------
.labl_08:
	mov	eax,0x6EB0
	call	cm_pr_22
	push	8
	jmp	.labl_17
;--------------------------------------------------------------------
.labl_09:
	xchg	esi,[0xD6C0]
	xchg	esi,[0xD6C4]
	mov	[0xD6C8],esi
	mov	eax,0x66A8
	call	cm_pr_22
	push	3
	pop	eax
	cmp	eax,ecx
	jb	.labl_10
	mov	eax,ecx
;--------------------------------------------------------------------
.labl_10:
	push	ecx
	push	6
	pop	ecx
	shl	eax,cl
	shl	eax,2
	add	eax,0x60A0
	call	cm_pr_23
	mov	esi,ecx
	cmp	ecx,4
	jb	.labl_16
	push	ecx
	xor	eax,eax
	inc	eax
	shr	ecx,1
	adc	al,al
	dec	ecx
	shl	eax,cl
	mov	esi, eax
	pop	edx
	cmp	edx,0xE
	jnb	.labl_11
	sub	eax,edx
	shl	eax,2
	add	eax,0x649C
	jmp	.labl_14
;--------------------------------------------------------------------
.labl_11:
	sub	ecx,4
	xor	eax,eax
;--------------------------------------------------------------------
.labl_12:
	shr	dword [0xD6BC],1
	add	eax, eax
	mov	edx,[0xD6B8]
	sub	edx,[0xD6BC]
	jb	.labl_13
	mov	[0xD6B8],edx
	inc	eax
;--------------------------------------------------------------------
.labl_13:
	call	cm_pr_21
	loop	.labl_12
	mov	cl,4
	shl	eax,cl
	add	esi,eax
	mov	eax,0x6668
;--------------------------------------------------------------------
.labl_14:
	push	edi
	push	ecx
	xor	edx,edx
	inc	edx
	xor	edi,edi
;--------------------------------------------------------------------
.labl_15:
	push	eax
	lea	eax,[eax+edx*4]
	call	cm_pr_20
	lahf 
	adc	edx,edx
	sahf 
	rcr	edi,1
	pop	eax
	loop	.labl_15
	pop	ecx
	rol	edi,cl
	add	esi,edi
	pop	edi
;--------------------------------------------------------------------
.labl_16:
	pop	ecx
	not	esi
	push	7
;--------------------------------------------------------------------
.labl_17:
	cmp	ebp,7
	pop	ebp
	jb	.labl_18
	inc	ebp
	inc	ebp
	inc	ebp
;--------------------------------------------------------------------
.labl_18:
	inc	ecx
	push	esi
	add	esi,edi
	rep	movsb 
	lodsb 
	pop	esi
	jmp	.labl_05
;--------------------------------------------------------------------
.labl_19:
	pop	ebp
	pop	ebx
	pop	edi
	pop	esi
	ret	0xC
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_20:
	push	edx
	mov	edx,[0xD6BC]
	shr	edx,0xB
	imul	edx,[eax]
	cmp	[0xD6B8],edx
	jnb	.labl_01
	mov	[0xD6BC],edx
	mov	edx,0x800
	sub	edx,[eax]
	shr	edx,5
	add	[eax],edx
;--------------------------------------------------------------------
.labl_00:
	pushfd 
	call	cm_pr_21
	popfd 
	pop	edx
	ret 
;--------------------------------------------------------------------
.labl_01:
	sub	[0xD6BC],edx
	sub	[0xD6B8],edx
	mov	edx,[eax]
	shr	edx,5
	sub	[eax],edx
	stc 
	jmp	.labl_00
;***********************************************************************

;* Call: ***************************************************************
cm_pr_21:
	cmp	[0xD6BF],byte 0
	jne	.labl_00
	shl	dword [0xD6BC],8
	shl	dword [0xD6B8],8
	push	eax
	mov	eax,[0xD6CC]
	mov	al,[eax]
	inc	dword [0xD6CC]
	mov	[0xD6B8],al
	pop	eax
;--------------------------------------------------------------------
.labl_00:
	ret 
;***********************************************************************

;* Call: ***************************************************************
cm_pr_22:
	call	cm_pr_20
	jnb	.labl_01
	add	eax,4
	call	cm_pr_20
	jb	.labl_00
	mov	cl,3
	shl	edx,cl
	lea	eax,[eax+edx*4+0x204]
	call	cm_pr_23
	add	ecx,8
	ret 
;--------------------------------------------------------------------
.labl_00:
	add	eax,0x404
	mov	cl,8
	call	cm_pr_23
	add	ecx,0x10
	ret 
;--------------------------------------------------------------------
.labl_01:
	mov	cl,3
	shl	edx,cl
	lea	eax,[eax+edx*4+8]
;***********************************************************************

;* Call: ***************************************************************
cm_pr_23:
	push	edi
	xor	edx,edx
	inc	edx
	mov	edi,edx
	xchg	edi, eax
;--------------------------------------------------------------------
.labl_00:
	push	eax
	lea	eax,[edi+edx*4]
	call	cm_pr_20
	pop	eax
	adc	dl,dl
	add	al,al
	loop	.labl_00
	sub	dl,al
	pop	edi
	mov	ecx,edx
	ret 
;***********************************************************************

