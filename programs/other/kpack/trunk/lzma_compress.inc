;* Call: ***************************************************************
lzma_compress:
	push	ebp
	mov	ebp,esp
	and	esp,0xFFFFFFF8
	sub	esp,0xC
	push	ebx
	push	esi
	push	edi
	push	2
	pop	esi
	xor	ebx,ebx
	mov	[esp+0x14],esi
	mov	[0x19FB0],bl
	mov	byte [0x19FB1],1
	mov	[esp+0x10],esi
;----------------------------------------------------------
.labl_00:
	mov	ecx,[esp+0x10]
	xor	edi,edi
	sar	ecx,1
	dec	ecx
	inc	edi
	shl	edi,cl
	cmp	edi,ebx
	jbe	.labl_01
	mov	eax,[esp+0x14]
	push	edi
	push	dword [esp+0x14]
	lea	eax,[eax+0x19FB0]
	push	eax
	call	_memset
	add	esp,0xC
	add	[esp+0x14],edi
;----------------------------------------------------------
.labl_01:
	inc	dword [esp+0x10]
	cmp	dword [esp+0x10],0x14
	jl	.labl_00
	mov	edi,[ebp+0x14]
	push	3
	pop	eax
	mov	ecx,edi
	and	ecx,eax
	mov	edx,0x80	;'€'
	mov	[0x1CC30],edx
	mov	[0x51A00],esi
	mov	[0x1CC34],eax
	mov	[0x1B234],eax
	mov	[0x2956C],ebx
	mov	[0x19FA9],bl
	mov	[0x19FA8],bl
	je	.labl_02
	sub	edi,ecx
	add	edi,4
;----------------------------------------------------------
.labl_02:
	mov	[0x519F4],eax
	mov	eax,[0x29568]
	mov	[0x519F0],edi
	add	edi,0x6000
	inc	eax
	mov	[0x519F8],ebx
	mov	[0x519FC],ebx
	mov	[0x52218],edx
	mov	[0x51A0C],eax
	mov	[0x52224],edi
	mov	dword [0x52220],0xFF	;'ÿ'
	call	lz_cm_01
	call	cm_pr_07
	call	cm_pr_08
	call	cm_pr_09
	push	0x7F	;''
	pop	esi
	mov	[0x19F64],esi
	mov	[esp+0x10],ebx
;----------------------------------------------------------
.labl_03:
	push	dword [esp+0x10]
	mov	eax,0xD6D8
	call	cm_pr_01
	inc	dword [esp+0x10]
	cmp	dword [esp+0x10],4
	jb	.labl_03
	mov	[0x294F4],esi
	xor	esi,esi
;----------------------------------------------------------
.labl_04:
	push	esi
	mov	eax,0x1CC68
	call	cm_pr_01
	inc	esi
	cmp	esi,4
	jb	.labl_04
	mov	eax,[ebp+8]
	mov	esi,[ebp+0xC]
	dec	eax
	mov	[0x52628],eax
	mov	eax,[ebp+0x10]
	inc	eax
	mov	[0x52630],eax
	mov	ecx,0x110000
	xor	eax,eax
	mov	[0x1CC28],ebx
	mov	[0x1CC2C],ebx
	mov	[0x1B400],ebx
	mov	[0x1B404],ebx
	mov	[0x52634],esi
	mov	dword [0x5262C],1
	rep	stosd	
	mov	[0x5221C],ebx
;----------------------------------------------------------
.labl_08:
	push	8
	mov	eax,ebx
	pop	ecx
;----------------------------------------------------------
.labl_07:
	test	al,1
	je	.labl_05
	shr	eax,1
	xor	eax,0xEDB88320
	jmp	.labl_06
;----------------------------------------------------------
.labl_05:
	shr	eax,1
;----------------------------------------------------------
.labl_06:
	dec	ecx
	jne	.labl_07
	mov	[0x52228+ebx*4],eax
	inc	ebx
	cmp	ebx,0x100
	jb	.labl_08
;----------------------------------------------------------
.labl_09:
	call	lz_cm_02
	test	al,al
	jne	.labl_09
	mov	eax,[0x52634]
	pop	edi
	sub	eax,esi
	pop	esi
	pop	ebx
	mov	esp,ebp
	pop	ebp
	ret	0x10
;*****************************************************************************

;* Call: ***************************************************************
_memset:
	push	edi
	mov	edi,[esp+8]
	mov	al,[esp+12]
	mov	ecx,[esp+16]
	rep	stosb	
	pop	edi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_01:
	and	dword [0x51A10],0
	and	dword [0x51A14],0
	or	dword [0x51A04],0xFFFFFFFF
	push	ebx
	push	esi
	push	edi
	xor	eax,eax
	mov	edi,0x1BC08
	stosd	
	stosd	
	xor	ebx,ebx
	stosd	
	inc	ebx
	mov	byte [0x519EC],0
	mov	byte [0x19FAB],0
	stosd	
	mov	[0x51A08],ebx
	mov	byte [0x519ED],0
	xor	ecx,ecx
;----------------------------------------------------------
.labl_00:
	mov	eax,ebx
	shl	eax,cl
	push	8
	pop	edx
	sub	edx,ecx
	lea	esi,[0x51A18+eax*4]
	shl	edx,6
;----------------------------------------------------------
.labl_01:
	mov	edi,eax
	shl	edi,6
	shr	edi,cl
	add	edi,edx
	mov	[esi],edi
	add	esi,4
	dec	eax
	jne	.labl_01
	inc	ecx
	cmp	ecx,9
	jl	.labl_00
	push	0xC
	pop	edx
	mov	esi,0x400
	mov	eax,esi
	mov	ecx,edx
	mov	edi,0x29538
	rep	stosd	
	mov	ecx,edx
	mov	edi,0x1AF00
	rep	stosd	
	mov	ecx,edx
	mov	edi,0x1CC38
	rep	stosd	
	mov	ecx,edx
	mov	edi,0x515B8
	rep	stosd	
	xor	ecx,ecx
;----------------------------------------------------------
.labl_02:
	lea	edi,[ecx+0x1A7B0]
	mov	eax,esi
	stosd	
	stosd	
	stosd	
	stosd	
	lea	edi,[ecx+0x1AF30]
	mov	eax,esi
	stosd	
	stosd	
	stosd	
	add	ecx,0x40	;'@'
	cmp	ecx,0x300
	stosd	
	jl	.labl_02
	mov	eax,[0x519F4]
	mov	ecx,[0x519F8]
	add	ecx,eax
	mov	eax,ebx
	shl	eax,cl
	test	eax,eax
	jbe	.labl_04
	xor	edx,edx
	mov	ebx,eax
;----------------------------------------------------------
.labl_03:
	mov	eax,[0x519F0]
	lea	edi,[edx+eax]
	mov	ecx,0x300
	mov	eax,esi
	add	edx,0xC00
	dec	ebx
	rep	stosd	
	jne	.labl_03
;----------------------------------------------------------
.labl_04:
	mov	edi,0x1BC18
;----------------------------------------------------------
.labl_05:
	push	6
	pop	ecx
	mov	eax,edi
	call	cm_pr_15
	add	edi,0x404
	cmp	edi,0x1CC28
	jl	.labl_05
	push	0x72	;'r'
	mov	eax,esi
	pop	ecx
	mov	edi,0x1B238
	mov	esi,0xD6D8
	rep	stosd
	call	cm_pr_00
	mov	esi,0x1CC68
	call	cm_pr_00
	push	4
	pop	ecx
	mov	eax,0x515E8
	call	cm_pr_15
	pop	edi
	xor	eax,eax
	pop	esi
	mov	byte [0x19FAA],0	;0x1B238
	mov	[0x29574],eax
	mov	[0x1AEF8],eax
	mov	[0x1AEFC],eax
	pop	ebx
	ret	
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_02:
	sub	esp,0x2C
	cmp	byte [0x19FA8],0
	push	ebx
	push	ebp
	push	esi
	push	edi
	je	.labl_01
;----------------------------------------------------------
.labl_00:
	xor	al,al
	jmp	.labl_28
;----------------------------------------------------------
.labl_01:
	mov	ebp,[0x1B400]
	mov	edi,[0x1B404]
	mov	eax,ebp
	or	eax,edi
	mov	byte [0x19FA8],1
	mov	[esp+0x34],ebp
	mov	[esp+0x38],edi
	jne	.labl_04
	mov	eax,[0x52630]
	sub	eax,[0x5262C]
	jne	.labl_03
	push	5
	pop	esi
;----------------------------------------------------------
.labl_02:
	call	cm_pr_13
	dec	esi
	jne	.labl_02
	jmp	.labl_00
;----------------------------------------------------------
.labl_03:
	call	cm_pr_06
	movzx	esi,byte [0x519EC]
	mov	edx,esi
	shl	edx,6
	xor	edi,edi
	push	edi
	add	edx,0x1AF30
	call	cm_pr_14
	mov	al,[esi+LiteralNextStates]
	mov	ecx,[0x5262C]
	mov	[0x519EC],al
	mov	eax,[0x52628]
	sub	eax,[0x1AEFC]
	mov	bl,[eax+ecx]
	mov	al,[0x19FAB]
	mov	[esp+0x18],bl
	push	dword [esp+0x18]
	push	edi
	call	cm_pr_04
	imul	eax,eax,0xC00
	add	eax,[0x519F0]
	push	eax
	call	cm_pr_03
	dec	dword [0x1AEFC]
	xor	ebp,ebp
	inc	ebp
	mov	[0x19FAB],bl
	mov	[0x1B400],ebp
	mov	[0x1B404],edi
;----------------------------------------------------------
.labl_04:
	mov	eax,[0x52630]
	sub	eax,[0x5262C]
	jne	.labl_06
	push	5
	pop	esi
;----------------------------------------------------------
.labl_05:
	call	cm_pr_13
	dec	esi
	jne	.labl_05
	jmp	.labl_00
;----------------------------------------------------------
.labl_06:
	lea	eax,[esp+0x14]
	push	eax
	lea	eax,[esp+0x14]
	push	eax
	mov	ebx,ebp
	push	ebp
	and	ebx,3
	call	lz_cm_03
	cmp	dword [esp+0x14],1
	jne	.labl_09
	cmp	dword [esp+0x10],0xFFFFFFFF
	jne	.labl_09
	movzx	eax,byte [0x519EC]
	shl	eax,4
	add	eax,ebx
	push	0
	lea	edx,[0x1AF30+eax*4]
	call	cm_pr_14
	mov	eax,[0x52628]
	mov	esi,[0x1AEFC]
	mov	ebx,[0x5262C]
	sub	eax,esi
	mov	al,[eax+ebx]
	mov	[esp+0x18],al
	mov	al,[0x19FAB]
	push	ebp
	call	cm_pr_04
	imul	eax,eax,0xC00
	add	eax,[0x519F0]
	cmp	byte [0x519EC],7
	push	dword [esp+0x18]
	jb	.labl_07
	mov	ecx,[0x52628]
	sub	ecx,[0x1BC08]
	sub	ecx,esi
	mov	cl,[ecx+ebx-1]
	mov	[esp+0x34],cl
	push	dword [esp+0x34]
	push	eax
	call	lz_cm_04
	jmp	.labl_08
;----------------------------------------------------------
.labl_07:
	push	eax
	call	cm_pr_03
;----------------------------------------------------------
.labl_08:
	movzx	eax,byte [0x519EC]
	mov	al,[eax+LiteralNextStates]
	mov	[0x519EC],al
	mov	al,[esp+0x18]
	jmp	.labl_24
;----------------------------------------------------------
.labl_09:
	movzx	edi,byte [0x519EC]
	mov	eax,edi
	shl	eax,4
	add	eax,ebx
	shl	eax,2
	push	1
	lea	edx,[eax+0x1AF30]
	mov	[esp+0x28],eax
	call	cm_pr_14
	cmp	dword [esp+0x10],4
	jnb	.labl_17
	mov	esi,edi
	shl	esi,2
	push	1
	lea	edx,[esi+0x515B8]
	call	cm_pr_14
	cmp	dword [esp+0x10],0
	lea	edx,[esi+0x1CC38]
	jne	.labl_10
	push	0
	call	cm_pr_14
	mov	edx,[esp+0x24]
	xor	eax,eax
	cmp	dword [esp+0x14],1
	lea	edx,[edx+0x1A7B0]
	setne	al
	jmp	.labl_12
;----------------------------------------------------------
.labl_10:
	push	1
	call	cm_pr_14
	cmp	dword [esp+0x10],1
	lea	edx,[esi+0x1AF00]
	jne	.labl_11
	push	0
	jmp	.labl_13
;----------------------------------------------------------
.labl_11:
	push	1
	call	cm_pr_14
	mov	eax,[esp+0x10]
	add	eax,0xFFFFFFFE
	lea	edx,[esi+0x29538]
;----------------------------------------------------------
.labl_12:
	push	eax
;----------------------------------------------------------
.labl_13:
	call	cm_pr_14
	mov	eax,[esp+0x14]
	cmp	eax,1
	jne	.labl_14
	mov	al,[edi+ShortRepNextStates]
	jmp	.labl_15
;----------------------------------------------------------
.labl_14:
	add	eax,0xFFFFFFFE
	mov	esi,0x1CC68
	call	cm_pr_02
	mov	al,[edi+RepNextStates]
;----------------------------------------------------------
.labl_15:
	mov	[0x519EC],al
	mov	eax,[esp+0x10]
	mov	ecx,[0x1BC08+eax*4]
	test	eax,eax
	je	.labl_23
;----------------------------------------------------------
.labl_16:
	dec	eax
	mov	edx,[0x1BC08+eax*4]
	mov	[0x1BC0C+eax*4],edx
	jne	.labl_16
	mov	[0x1BC08],ecx
	jmp	.labl_23
;----------------------------------------------------------
.labl_17:
	push	0
	lea	edx,[0x515B8+edi*4]
	call	cm_pr_14
	mov	al,[edi+MatchNextStates]
	mov	edi,[esp+0x14]
	mov	[0x519EC],al
	add	edi,0xFFFFFFFE
	mov	eax,edi
	mov	esi,0xD6D8
	call	cm_pr_02
	sub	dword [esp+0x10],4
	mov	eax,[esp+0x10]
	call	cm_pr_000
	cmp	dword [esp+0x14],6
	mov	ebx,eax
	jb	.labl_18
	push	3
	pop	edi
;----------------------------------------------------------
.labl_18:
	imul	edi,edi,0x404
	push	ebx
	add	edi,0x1BC18
	call	cm_pr_16
	cmp	ebx,4
	jb	.labl_21
	mov	esi,[esp+0x10]
	xor	eax,eax
	inc	eax
	mov	ecx,ebx
	mov	edi,ebx
	and	edi,eax
	shr	ecx,1
	dec	ecx
	or	edi,2
	shl	edi,cl
	sub	esi,edi
	cmp	ebx,0xE
	jnb	.labl_20
	mov	[esp+0x20],esi
	mov	[esp+0x1C],eax
	test	ecx,ecx
	jle	.labl_21
	mov	[esp+0x24],ecx
;----------------------------------------------------------
.labl_19:
	mov	edx,[esp+0x1C]
	mov	esi,[esp+0x20]
	sub	edx,ebx
	add	edx,edi
	and	esi,1
	push	esi
	lea	edx,[0x1B234+edx*4]
	call	cm_pr_14
	mov	eax,[esp+0x1C]
	shr	dword [esp+0x20],1
	dec	dword [esp+0x24]
	lea	eax,[esi+eax*2]
	mov	[esp+0x1C],eax
	jne	.labl_19
	jmp	.labl_21
;----------------------------------------------------------
.labl_20:
	lea	eax,[ecx-4]
	mov	ecx,esi
	shr	ecx,4
	push	ecx
	call	lz_cm_05
	and	esi,0xF
	push	esi
	call	lz_cm_06
	dec	dword [0x1B230]
	jne	.labl_21
	call	cm_pr_09
;----------------------------------------------------------
.labl_21:
	mov	eax,0x1BC14
;----------------------------------------------------------
.labl_22:
	mov	ecx,[eax-4]
	mov	[eax],ecx
	sub	eax,4
	cmp	eax,0x1BC08
	jne	.labl_22
	mov	eax,[esp+0x10]
	mov	[0x1BC08],eax
;----------------------------------------------------------
.labl_23:
	mov	eax,[0x52628]
	mov	esi,[0x1AEFC]
	mov	ebx,[0x5262C]
	mov	ecx,[esp+0x14]
	mov	edi,[0x1B404]
	sub	eax,esi
	add	eax,ebx
	mov	al,[eax+ecx-1]
;----------------------------------------------------------
.labl_24:
	sub	esi,[esp+0x14]
	add	ebp,[esp+0x14]
	mov	[0x19FAB],al
	adc	edi,0
	mov	eax,ebp
	sub	eax,[0x1CC28]
	mov	ecx,edi
	sbb	ecx,[0x1CC2C]
	mov	[0x1AEFC],esi
	mov	[0x1B400],ebp
	mov	[0x1B404],edi
	mov	[esp+0x28],ecx
	jne	.labl_25
	cmp	eax,0x200
	jb	.labl_26
;----------------------------------------------------------
.labl_25:
	call	cm_pr_07
	call	cm_pr_08
	mov	[0x1CC28],ebp
	mov	[0x1CC2C],edi
;----------------------------------------------------------
.labl_26:
	test	esi,esi
	jne	.labl_06
	mov	eax,[0x52630]
	sub	eax,ebx
	je	.labl_29
	mov	ecx,ebp
	sub	ecx,[esp+0x34]
	mov	eax,edi
	sbb	eax,[esp+0x38]
	test	eax,eax
	ja	.labl_27
	jb	.labl_06
	cmp	ecx,0x1000
	jb	.labl_06
;----------------------------------------------------------
.labl_27:
	mov	byte [0x19FA8],0
	mov	al,1
;----------------------------------------------------------
.labl_28:
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	add	esp,0x2C
	ret	
;----------------------------------------------------------
.labl_29:
	push	5
	pop	esi
;----------------------------------------------------------
.labl_30:
	call	cm_pr_13
	dec	esi
	jne	.labl_30
	jmp	.labl_00
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_03:
	push	ebp
	mov	ebp,esp
	mov	ecx,[0x1AEF8]
	sub	esp,0x74
	push	esi
	cmp	[0x29574],ecx
	je	.labl_00
	mov	eax,ecx
	imul	eax,eax,0x28
	add	eax,0x29578
	mov	edx,[eax+0x10]
	mov	eax,[eax+0x14]
	mov	esi,edx
	sub	esi,ecx
	mov	ecx,[ebp+0x10]
	mov	[ecx],esi
	mov	ecx,[ebp+0xC]
	mov	[ecx],eax
	mov	[0x1AEF8],edx
	jmp	.labl_76
;----------------------------------------------------------
.labl_00:
	push	edi
	xor	edi,edi
	cmp	byte [0x19FAA],0	;0x1B238
	mov	[0x29574],edi
	mov	[0x1AEF8],edi
	jne	.labl_01
	call	cm_pr_06
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	mov	eax,[0x19FAC]
	mov	byte [0x19FAA],0	;0x1B238
;----------------------------------------------------------
.labl_02:
	push	ebx
	mov	ebx,[ebp+8]
	mov	[ebp-0x10],eax
	mov	eax,ebx
	shl	eax,2
	mov	[ebp-0x4C],eax
;----------------------------------------------------------
.labl_03:
	mov	esi,edi
	shl	esi,2
	mov	eax,[esi+0x1BC08]
	push	eax
	mov	[ebp+esi-0x64],eax
	push	dword 0xFFFFFFFF
	mov	eax,0x111
	call	cm_pr_18
	mov	[ebp+esi-0x74],eax
	test	edi,edi
	je	.labl_04
	mov	ecx,[ebp-0x4C]
	cmp	eax,[ebp+ecx-0x74]
	jbe	.labl_05
;----------------------------------------------------------
.labl_04:
	mov	ebx,edi
	mov	[ebp-0x4C],esi
;----------------------------------------------------------
.labl_05:
	inc	edi
	cmp	edi,4
	jb	.labl_03
	mov	eax,[ebp+ebx*4-0x74]
	mov	ecx,0x80	;'€'
	mov	[ebp-0x44],eax
	cmp	eax,ecx
	jb	.labl_06
	mov	ecx,[ebp+0xC]
	mov	[ecx],ebx
	jmp	.labl_07
;----------------------------------------------------------
.labl_06:
	cmp	[ebp-0x10],ecx
	jb	.labl_08
	mov	eax,[0x1ACB0]
	mov	ecx,[ebp+0xC]
	add	eax,4
	mov	[ecx],eax
	mov	eax,[ebp-0x10]
;----------------------------------------------------------
.labl_07:
	mov	ecx,[ebp+0x10]
	mov	[ecx],eax
	dec	eax
	call	lz_cm_07
	jmp	.labl_75
;----------------------------------------------------------
.labl_08:
	mov	ecx,[0x52628]
	mov	eax,[0x5262C]
	mov	dl,[eax+ecx-1]
	sub	eax,[0x1BC08]
	mov	bl,[0x519EC]
	mov	[0x29578],bl
	mov	al,[eax+ecx-2]
	mov	[ebp-0x34],al
	mov	eax,[ebp+8]
	and	eax,3
	movzx	ecx,bl
	shl	ecx,4
	add	ecx,eax
	mov	esi,[0x1AF30+ecx*4]
	mov	[ebp-0x1C],eax
	cmp	bl,7
	sbb	al,al
	mov	[ebp-0x30],dl
	push	dword [ebp-0x30]
	inc	al
	push	dword [ebp-0x34]
	movzx	eax,al
	push	eax
	push	dword [ebp+8]
	mov	al,[0x19FAB]
	mov	[ebp-0x4C],esi
	call	cm_pr_05
	or	dword [0x295B4],0xFFFFFFFF
	and	dword [0x295B0],0
	shr	esi,2
	add	eax,[0x51A18+esi*4]
	mov	byte [0x295A1],0
	mov	[0x295AC],eax
	lea	esi,[ebp-0x64]
	mov	edi,0x29590
	movsd	
	movsd	
	mov	eax,0x800
	mov	ecx,eax
	sub	ecx,[ebp-0x4C]
	movsd	
	shr	ecx,2
	mov	edx,[0x51A18+ecx*4]
	movzx	ecx,bl
	movsd	
	mov	esi,[0x515B8+ecx*4]
	sub	eax,esi
	shr	eax,2
	mov	edi,[0x51A18+eax*4]
	mov	al,[ebp-0x30]
	add	edi,edx
	mov	[ebp-0x4C],edx
	cmp	[ebp-0x34],al
	jne	.labl_09
	push	dword [ebp-0x1C]
	mov	al,bl
	call	cm_pr_10
	add	eax,edi
	cmp	eax,[0x295AC]
	jnb	.labl_09
	and	dword [0x295B4],0
	mov	[0x295AC],eax
	mov	byte [0x295A1],0
;----------------------------------------------------------
.labl_09:
	push	2
	pop	eax
	cmp	[ebp-0x10],eax
	jnb	.labl_10
	mov	eax,[ebp+0xC]
	mov	ecx,[0x295B4]
	mov	[eax],ecx
	mov	eax,[ebp+0x10]
	mov	dword [eax],1
	jmp	.labl_75
;----------------------------------------------------------
.labl_10:
	mov	ecx,[ebp-0x10]
	shr	esi,2
	mov	esi,[0x51A18+esi*4]
	add	esi,[ebp-0x4C]
	mov	[ebp-0x4C],esi
	cmp	ecx,[ebp-0x44]
	ja	.labl_11
	and	dword [ebp-0x10],0
;----------------------------------------------------------
.labl_11:
	mov	[ebp-0x18],eax
	cmp	[ebp-0x10],eax
	jb	.labl_13
	mov	esi,0x295DC
;----------------------------------------------------------
.labl_12:
	mov	eax,[ebp-0x18]
	mov	edx,[0x1AAB0+eax*4]
	push	dword [ebp-0x1C]
	and	dword [esi-4],0
	lea	ecx,[edx+4]
	mov	[esi],ecx
	call	cm_pr_12
	add	eax,[ebp-0x4C]
	inc	dword [ebp-0x18]
	mov	[esi-8],eax
	mov	eax,[ebp-0x18]
	mov	byte [esi-0x13],0
	add	esi,0x28	;'('
	cmp	eax,[ebp-0x10]
	jbe	.labl_12
;----------------------------------------------------------
.labl_13:
	mov	ecx,[ebp-0x10]
	cmp	ecx,[ebp-0x44]
	jnb	.labl_14
	mov	ecx,[ebp-0x44]
	mov	[ebp-0x10],ecx
;----------------------------------------------------------
.labl_14:
	cmp	[ebp-0x18],ecx
	ja	.labl_16
	mov	eax,[ebp-0x18]
	sub	ecx,[ebp-0x18]
	imul	eax,eax,0x28
	add	eax,0x29584
	inc	ecx
;----------------------------------------------------------
.labl_15:
	mov	dword [eax],0x0FFFFFFF
	add	eax,0x28	;'('
	dec	ecx
	jne	.labl_15
;----------------------------------------------------------
.labl_16:
	and	dword [ebp-0x18],0
;----------------------------------------------------------
.labl_17:
	mov	eax,[ebp-0x18]
	mov	eax,[ebp+eax*4-0x74]
	push	2
	pop	ecx
	mov	[ebp-0x20],ecx
	cmp	eax,ecx
	jb	.labl_20
	mov	esi,0x295D4
;----------------------------------------------------------
.labl_18:
	push	dword [ebp-0x1C]
	mov	edx,[ebp-0x18]
	movzx	eax,bl
	push	eax
	mov	eax,[ebp-0x20]
	call	cm_pr_11
	add	eax,edi
	cmp	eax,[esi]
	jnb	.labl_19
	and	dword [esi+4],0
	mov	[esi],eax
	mov	eax,[ebp-0x18]
	mov	[esi+8],eax
	mov	byte [esi-0xB],0
;----------------------------------------------------------
.labl_19:
	inc	dword [ebp-0x20]
	mov	eax,[ebp-0x18]
	mov	ecx,[ebp-0x20]
	add	esi,0x28	;'('
	cmp	ecx,[ebp+eax*4-0x74]
	jbe	.labl_18
;----------------------------------------------------------
.labl_20:
	inc	dword [ebp-0x18]
	cmp	dword [ebp-0x18],4
	jb	.labl_17
	mov	eax,[ebp-0x10]
	xor	ecx,ecx
	inc	ecx
	mov	[ebp-8],eax
	mov	[ebp-0xC],ecx
	cmp	eax,ecx
	je	.labl_74
;----------------------------------------------------------
.labl_21:
	mov	edi,[ebp-0xC]
	inc	dword [ebp+8]
	mov	ebx,edi
	imul	ebx,ebx,0x28
	mov	dl,[ebx+0x29579]
	mov	esi,[ebx+0x29588]
	test	dl,dl
	je	.labl_25
	dec	esi
	cmp	byte [ebx+0x2957A],0
	je	.labl_23
	mov	eax,[ebx+0x2957C]
	imul	eax,eax,0x28
	cmp	dword [ebx+0x29580],4
	mov	al,[eax+0x29578]
	movzx	eax,al
	jnb	.labl_22
	mov	al,[eax+RepNextStates]
	jmp	.labl_24
;----------------------------------------------------------
.labl_22:
	mov	al,[eax+MatchNextStates]
	jmp	.labl_24
;----------------------------------------------------------
.labl_23:
	mov	eax,esi
	imul	eax,eax,0x28
	mov	al,[eax+0x29578]
;----------------------------------------------------------
.labl_24:
	movzx	eax,al
	mov	cl,[eax+LiteralNextStates]
	jmp	.labl_26
;----------------------------------------------------------
.labl_25:
	mov	eax,esi
	imul	eax,eax,0x28
	mov	cl,[eax+0x29578]
;----------------------------------------------------------
.labl_26:
	dec	edi
	cmp	esi,edi
	jne	.labl_29
	cmp	dword [ebx+0x2958C],0
	movzx	eax,cl
	jne	.labl_27
	mov	al,[eax+ShortRepNextStates]
	jmp	.labl_28
;----------------------------------------------------------
.labl_27:
	mov	al,[eax+LiteralNextStates]
;----------------------------------------------------------
.labl_28:
	mov	[ebp-0x14],al
	jmp	.labl_35
;----------------------------------------------------------
.labl_29:
	test	dl,dl
	je	.labl_31
	cmp	byte [ebx+0x2957A],0
	je	.labl_31
	mov	esi,[ebx+0x2957C]
	mov	eax,[ebx+0x29580]
	movzx	ecx,cl
;----------------------------------------------------------
.labl_30:
	mov	cl,[ecx+RepNextStates]
	jmp	.labl_32
;----------------------------------------------------------
.labl_31:
	mov	eax,[ebx+0x2958C]
	movzx	ecx,cl
	cmp	eax,4
	jb	.labl_30
	mov	cl,[ecx+MatchNextStates]
;----------------------------------------------------------
.labl_32:
	imul	esi,esi,0x28
	add	esi,0x29578
	mov	[ebp-0x14],cl
	mov	edx,esi
	cmp	eax,4
	jnb	.labl_34
	mov	ecx,[edx+eax*4+0x18]
	xor	esi,esi
	inc	esi
	mov	[ebp-0x64],ecx
	cmp	eax,esi
	jb	.labl_33
	lea	esi,[edx+0x18]
	mov	ecx,eax
	lea	edi,[ebp-0x60]
	rep	movsd	
	lea	esi,[eax+1]
	cmp	esi,4
	jnb	.labl_35
;----------------------------------------------------------
.labl_33:
	push	4
	lea	edx,[edx+esi*4+0x18]
	pop	ecx
	lea	edi,[ebp+esi*4-0x64]
	sub	ecx,esi
	mov	esi,edx
	rep	movsd	
	jmp	.labl_35
;----------------------------------------------------------
.labl_34:
	add	eax,0xFFFFFFFC
	mov	[ebp-0x64],eax
	lea	esi,[edx+0x18]
	lea	edi,[ebp-0x60]
	movsd	
	movsd	
	movsd	
;----------------------------------------------------------
.labl_35:
	mov	al,[ebp-0x14]
	mov	[ebx+0x29578],al
	lea	edi,[ebx+0x29590]
	lea	esi,[ebp-0x64]
	movsd	
	movsd	
	movsd	
	movsd	
	call	cm_pr_06
	mov	[ebp-0x2C],eax
	cmp	eax,0x80	;'€'
	jnb	.labl_73
	mov	eax,[ebx+0x29584]
	mov	edi,[0x5262C]
	mov	esi,[ebp+8]
	mov	[ebp-0x48],eax
	mov	eax,[0x52628]
	lea	eax,[edi+eax-1]
	mov	cl,[eax]
	mov	[ebp-0x30],cl
	push	dword [ebp-0x30]
	mov	ecx,eax
	sub	ecx,[ebp-0x64]
	and	esi,3
	mov	cl,[ecx-1]
	mov	[ebp-0x34],cl
	movzx	ecx,byte [ebp-0x14]
	push	dword [ebp-0x34]
	mov	[ebp-0x40],ecx
	shl	ecx,4
	add	ecx,esi
	mov	ecx,[0x1AF30+ecx*4]
	cmp	byte [ebp-0x14],7
	mov	[ebp-0x4C],ecx
	sbb	cl,cl
	inc	cl
	movzx	ecx,cl
	push	ecx
	push	dword [ebp+8]
	mov	[ebp-0x28],eax
	mov	al,[eax-1]
	mov	[ebp-0x1C],esi
	call	cm_pr_05
	mov	esi,eax
	mov	eax,[ebp-0x4C]
	shr	eax,2
	add	esi,[0x51A18+eax*4]
	mov	byte [ebp-1],0
	add	esi,[ebp-0x48]
	cmp	esi,[ebx+0x295AC]
	jnb	.labl_36
	mov	eax,[ebp-0xC]
	or	dword [ebx+0x295B4],0xFFFFFFFF
	mov	[ebx+0x295AC],esi
	mov	[ebx+0x295B0],eax
	mov	byte [ebx+0x295A1],0
	mov	byte [ebp-1],1
;----------------------------------------------------------
.labl_36:
	mov	edx,[ebp-0x40]
	mov	edx,[0x515B8+edx*4]
	mov	ecx,0x800
	mov	eax,ecx
	sub	eax,[ebp-0x4C]
	sub	ecx,edx
	shr	eax,2
	mov	eax,[0x51A18+eax*4]
	add	eax,[ebp-0x48]
	shr	ecx,2
	mov	ecx,[0x51A18+ecx*4]
	add	ecx,eax
	mov	[ebp-0x4C],eax
	mov	al,[ebp-0x30]
	mov	[ebp-0x54],edx
	mov	[ebp-0x48],ecx
	cmp	[ebp-0x34],al
	jne	.labl_38
	mov	eax,[ebp-0xC]
	cmp	[ebx+0x295B0],eax
	jnb	.labl_37
	cmp	dword [ebx+0x295B4],0
	je	.labl_38
;----------------------------------------------------------
.labl_37:
	push	dword [ebp-0x1C]
	mov	al,[ebp-0x14]
	call	cm_pr_10
	add	eax,[ebp-0x48]
	cmp	eax,[ebx+0x295AC]
	ja	.labl_38
	and	dword [ebx+0x295B4],0
	mov	[ebx+0x295AC],eax
	mov	eax,[ebp-0xC]
	mov	[ebx+0x295B0],eax
	mov	byte [ebx+0x295A1],0
;----------------------------------------------------------
.labl_38:
	mov	eax,[0x52630]
	sub	eax,edi
	mov	ecx,0xFFF
	sub	ecx,[ebp-0xC]
	inc	eax
	mov	[ebp-0x38],eax
	cmp	eax,ecx
	jbe	.labl_39
	mov	eax,ecx
	mov	[ebp-0x38],ecx
;----------------------------------------------------------
.labl_39:
	mov	[ebp-0x24],eax
	cmp	eax,2
	jb	.labl_72
	mov	ecx,0x80	;'€'
	cmp	eax,ecx
	jbe	.labl_40
	mov	[ebp-0x24],ecx
;----------------------------------------------------------
.labl_40:
	cmp	dword [ebp-0x24],3
	jb	.labl_45
	cmp	byte [ebp-1],0
	jne	.labl_45
	mov	ecx,[ebp-0x64]
	xor	eax,eax
	inc	eax
	inc	ecx
	cmp	[ebp-0x24],eax
	jbe	.labl_42
	mov	edi,[ebp-0x28]
	mov	edx,edi
	sub	edx,ecx
;----------------------------------------------------------
.labl_41:
	mov	cl,[eax+edi]
	cmp	cl,[edx+eax]
	jne	.labl_42
	inc	eax
	cmp	eax,[ebp-0x24]
	jb	.labl_41
;----------------------------------------------------------
.labl_42:
	dec	eax
	mov	[ebp-0x20],eax
	cmp	eax,2
	jb	.labl_45
	mov	eax,[ebp-0x40]
	mov	al,[eax+LiteralNextStates]
	mov	ecx,[ebp+8]
	movzx	edx,al
	mov	[ebp-0x3C],al
	mov	eax,edx
	shl	eax,4
	inc	ecx
	mov	edi,0x800
	and	ecx,3
	add	eax,ecx
	mov	ebx,edi
	sub	ebx,[0x1AF30+eax*4]
	mov	eax,edi
	sub	eax,[0x515B8+edx*4]
	mov	edx,[ebp-0xC]
	shr	ebx,2
	mov	edi,[0x51A18+ebx*4]
	shr	eax,2
	add	edi,[0x51A18+eax*4]
	mov	eax,[ebp-0x20]
	add	edi,esi
	lea	esi,[edx+eax]
	mov	eax,[ebp-8]
	lea	edx,[esi+1]
	cmp	eax,edx
	jnb	.labl_44
	sub	edx,[ebp-8]
	imul	eax,eax,0x28
	add	eax,0x29584
	add	[ebp-8],edx
;----------------------------------------------------------
.labl_43:
	add	eax,0x28	;'('
	dec	edx
	mov	dword [eax],0x0FFFFFFF
	jne	.labl_43
;----------------------------------------------------------
.labl_44:
	mov	eax,[ebp-0x20]
	push	ecx
	push	dword [ebp-0x3C]
	xor	edx,edx
	call	cm_pr_11
	imul	esi,esi,0x28
	add	eax,edi
	add	esi,0x295A0
	cmp	eax,[esi+0xC]
	jnb	.labl_45
	mov	[esi+0xC],eax
	mov	eax,[ebp-0xC]
	inc	eax
	and	dword [esi+0x14],0
	mov	[esi+0x10],eax
	mov	byte [esi+1],1
	mov	byte [esi+2],0
;----------------------------------------------------------
.labl_45:
	and	dword [ebp-0x18],0
;----------------------------------------------------------
.labl_46:
	mov	eax,[ebp-0x18]
	mov	eax,[ebp+eax*4-0x64]
	mov	edx,[ebp-0x28]
	inc	eax
	mov	ecx,edx
	sub	ecx,eax
	mov	[ebp-0x44],eax
	mov	al,[edx]
	mov	[ebp-0x50],ecx
	cmp	al,[ecx]
	jne	.labl_58
	mov	al,[edx+1]
	cmp	al,[ecx+1]
	jne	.labl_58
	push	2
	pop	esi
	cmp	[ebp-0x24],esi
	jbe	.labl_48
;----------------------------------------------------------
.labl_47:
	mov	al,[edx+esi]
	cmp	al,[ecx+esi]
	jne	.labl_48
	inc	esi
	cmp	esi,[ebp-0x24]
	jb	.labl_47
;----------------------------------------------------------
.labl_48:
	mov	eax,[ebp-0xC]
	lea	ebx,[eax+esi]
	mov	[ebp-0x20],ebx
	imul	ebx,ebx,0x28
	mov	edi,esi
	add	ebx,0x29584
;----------------------------------------------------------
.labl_49:
	mov	eax,[ebp-8]
	mov	ecx,[ebp-0x20]
	cmp	eax,ecx
	jnb	.labl_51
	sub	ecx,[ebp-8]
	imul	eax,eax,0x28
	add	eax,0x29584
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_50:
	add	eax,0x28	;'('
	dec	ecx
	mov	dword [eax],0x0FFFFFFF
	jne	.labl_50
;----------------------------------------------------------
.labl_51:
	push	dword [ebp-0x1C]
	mov	edx,[ebp-0x18]
	push	dword [ebp-0x14]
	mov	eax,esi
	call	cm_pr_11
	add	eax,[ebp-0x48]
	cmp	eax,[ebx]
	jnb	.labl_52
	mov	[ebx],eax
	mov	eax,[ebp-0xC]
	mov	[ebx+4],eax
	mov	eax,[ebp-0x18]
	mov	[ebx+8],eax
	mov	byte [ebx-0xB],0
;----------------------------------------------------------
.labl_52:
	dec	esi
	sub	ebx,0x28	;'('
	dec	dword [ebp-0x20]
	cmp	esi,2
	jnb	.labl_49
	lea	eax,[edi+1]
	lea	esi,[eax+0x80]
	cmp	esi,[ebp-0x38]
	jbe	.labl_53
	mov	esi,[ebp-0x38]
;----------------------------------------------------------
.labl_53:
	cmp	eax,esi
	jnb	.labl_55
	mov	edx,[ebp-0x28]
	mov	ecx,eax
	sub	ecx,[ebp-0x44]
	add	ecx,edx
;----------------------------------------------------------
.labl_54:
	mov	bl,[eax+edx]
	cmp	bl,[ecx]
	jne	.labl_55
	inc	eax
	inc	ecx
	cmp	eax,esi
	jb	.labl_54
;----------------------------------------------------------
.labl_55:
	or	ecx,0xFFFFFFFF
	sub	ecx,edi
	add	eax,ecx
	mov	[ebp-0x10],eax
	cmp	eax,2
	jb	.labl_58
	mov	eax,[ebp-0x40]
	mov	al,[eax+RepNextStates]
	mov	edx,[ebp+8]
	movzx	eax,al
	mov	ebx,eax
	mov	al,[eax+LiteralNextStates]
	lea	ecx,[edi+edx]
	mov	[ebp-0x3C],al
	movzx	eax,al
	mov	esi,ecx
	and	esi,3
	shl	ebx,4
	add	esi,ebx
	mov	esi,[0x1AF30+esi*4]
	lea	ebx,[edi+edx+1]
	mov	[ebp-0x20],eax
	mov	eax,[ebp-0x28]
	movzx	edx,byte [edi+eax]
	mov	al,[eax+edi-1]
	push	edx
	mov	edx,[ebp-0x50]
	movzx	edx,byte [edx+edi]
	push	edx
	shr	esi,02
	mov	esi,[0x51A18+esi*4]
	push	1
	push	ecx
	mov	[ebp-0x44],esi
	and	ebx,3
	call	cm_pr_05
	push	dword [ebp-0x1C]
	mov	edx,[ebp-0x18]
	push	dword [ebp-0x14]
	mov	esi,eax
	mov	eax,edi
	call	cm_pr_11
	mov	ecx,[ebp-0x20]
	add	esi,eax
	shl	ecx,4
	add	ecx,ebx
	mov	eax,0x800
	mov	edx,eax
	sub	edx,[0x1AF30+ecx*4]
	mov	ecx,[ebp-0x20]
	sub	eax,[0x515B8+ecx*4]
	mov	ecx,[ebp-0xC]
	shr	edx,2
	add	esi,[0x51A18+edx*4]
	shr	eax,2
	add	esi,[0x51A18+eax*4]
	mov	eax,[ebp-0x10]
	add	esi,[ebp-0x44]
	lea	eax,[eax+edi+1]
	add	esi,[ebp-0x48]
	add	eax,ecx
	mov	[ebp-0x44],eax
	cmp	[ebp-8],eax
	jnb	.labl_57
	mov	eax,[ebp-8]
	mov	ecx,[ebp-0x44]
	imul	eax,eax,0x28
	sub	ecx,[ebp-8]
	add	eax,0x29584
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_56:
	add	eax,0x28	;'('
	dec	ecx
	mov	dword [eax],0x0FFFFFFF
	jne	.labl_56
;----------------------------------------------------------
.labl_57:
	mov	eax,[ebp-0x10]
	push	ebx
	push	dword [ebp-0x3C]
	xor	edx,edx
	call	cm_pr_11
	mov	ecx,[ebp-0x44]
	imul	ecx,ecx,0x28
	add	eax,esi
	add	ecx,0x29578
	cmp	eax,[ecx+0xC]
	jnb	.labl_58
	and	dword [ecx+0x14],0
	mov	[ecx+0xC],eax
	mov	eax,[ebp-0xC]
	lea	edx,[edi+eax+1]
	mov	[ecx+4],eax
	mov	eax,[ebp-0x18]
	mov	[ecx+0x10],edx
	mov	byte [ecx+1],1
	mov	byte [ecx+2],1
	mov	[ecx+8],eax
;----------------------------------------------------------
.labl_58:
	inc	dword [ebp-0x18]
	cmp	dword [ebp-0x18],4
	jb	.labl_46
	mov	eax,[ebp-0x24]
	cmp	[ebp-0x2C],eax
	jbe	.labl_59
	mov	[ebp-0x2C],eax
;----------------------------------------------------------
.labl_59:
	cmp	dword [ebp-0x2C],2
	jb	.labl_72
	mov	esi,[ebp-0x2C]
	cmp	esi,2
	jne	.labl_60
	cmp	dword [0x1AAB8],0x80	;'€'
	jnb	.labl_72
;----------------------------------------------------------
.labl_60:
	mov	eax,[ebp-0x54]
	shr	eax,2
	mov	eax,[0x51A18+eax*4]
	add	eax,[ebp-0x4C]
	mov	[ebp-0x4C],eax
	mov	eax,[ebp-0xC]
	lea	ecx,[esi+eax]
	cmp	[ebp-8],ecx
	jnb	.labl_62
	mov	eax,[ebp-8]
	imul	eax,eax,0x28
	mov	edx,ecx
	sub	edx,[ebp-8]
	add	eax,0x29584
	add	[ebp-8],edx
;----------------------------------------------------------
.labl_61:
	add	eax,0x28	;'('
	dec	edx
	mov	dword [eax],0x0FFFFFFF
	jne	.labl_61
;----------------------------------------------------------
.labl_62:
	imul	ecx,ecx,0x28
	add	ecx,0x29584
	lea	eax,[0x1AAB4+esi*4]
	lea	edi,[esi+1]
	mov	[ebp-0x24],eax
	mov	ebx,ecx
	mov	[ebp-0x48],ecx
;----------------------------------------------------------
.labl_63:
	mov	eax,[ebp-0x24]
	mov	esi,[eax-4]
	push	dword [ebp-0x1C]
	lea	eax,[edi-1]
	mov	edx,esi
	call	cm_pr_12
	add	eax,[ebp-0x4C]
	mov	[ebp-0x50],eax
	cmp	eax,[ebx]
	jnb	.labl_64
	mov	[ebx],eax
	mov	eax,[ebp-0xC]
	mov	[ebx+4],eax
	lea	eax,[esi+4]
	mov	[ebx+8],eax
	mov	byte [ebx-0xB],0
;----------------------------------------------------------
.labl_64:
	lea	eax,[edi-1]
	cmp	eax,[ebp-0x2C]
	je	.labl_65
	mov	eax,[ebp-0x24]
	cmp	esi,[eax]
	je	.labl_71
;----------------------------------------------------------
.labl_65:
	inc	esi
	lea	ecx,[edi+0x80]
	mov	[ebp-0x44],esi
	mov	[ebp-0x10],edi
	cmp	ecx,[ebp-0x38]
	jbe	.labl_66
	mov	ecx,[ebp-0x38]
;----------------------------------------------------------
.labl_66:
	cmp	edi,ecx
	jnb	.labl_68
	mov	eax,edi
	sub	eax,esi
	add	eax,[ebp-0x28]
;----------------------------------------------------------
.labl_67:
	mov	edx,[ebp-0x10]
	mov	esi,[ebp-0x28]
	mov	dl,[edx+esi]
	cmp	dl,[eax]
	jne	.labl_68
	inc	dword [ebp-0x10]
	inc	eax
	cmp	[ebp-0x10],ecx
	jb	.labl_67
;----------------------------------------------------------
.labl_68:
	or	ecx,0xFFFFFFFF
	lea	eax,[edi-1]
	sub	ecx,eax
	add	[ebp-0x10],ecx
	cmp	dword [ebp-0x10],2
	jb	.labl_71
	mov	eax,[ebp-0x40]
	mov	al,[eax+MatchNextStates]
	mov	ecx,[ebp+8]
	movzx	eax,al
	mov	esi,eax
	mov	al,[eax+LiteralNextStates]
	lea	edx,[edi+ecx-1]
	mov	[ebp-0x3C],al
	movzx	eax,al
	shl	esi,4
	mov	ebx,edx
	and	ebx,3
	add	esi,ebx
	mov	esi,[0x1AF30+esi*4]
	lea	ebx,[edi+ecx]
	mov	[ebp-0x20],eax
	mov	eax,[ebp-0x28]
	movzx	ecx,byte [edi+eax-1]
	push	ecx
	mov	ecx,edi
	sub	ecx,[ebp-0x44]
	shr	esi,2
	movzx	ecx,byte [ecx+eax-1]
	mov	esi,[0x51A18+esi*4]
	mov	al,[edi+eax-2]
	push	ecx
	push	1
	push	edx
	mov	[ebp-0x54],esi
	and	ebx,3
	call	cm_pr_05
	mov	ecx,[ebp-0x20]
	mov	esi,eax
	shl	ecx,4
	add	ecx,ebx
	mov	eax,0x800
	mov	edx,eax
	sub	edx,[0x1AF30+ecx*4]
	mov	ecx,[ebp-0x20]
	sub	eax,[0x515B8+ecx*4]
	mov	ecx,[ebp-0xC]
	shr	edx,2
	add	esi,[0x51A18+edx*4]
	shr	eax,2
	add	esi,[0x51A18+eax*4]
	mov	eax,[ebp-0x10]
	add	esi,[ebp-0x54]
	add	eax,edi
	add	esi,[ebp-0x50]
	add	eax,ecx
	mov	[ebp-0x20],eax
	cmp	[ebp-8],eax
	jnb	.labl_70
	mov	eax,[ebp-8]
	mov	ecx,[ebp-0x20]
	imul	eax,eax,0x28
	sub	ecx,[ebp-8]
	add	eax,0x29584
	add	[ebp-8],ecx
;----------------------------------------------------------
.labl_69:
	add	eax,0x28	;'('
	dec	ecx
	mov	dword [eax],0x0FFFFFFF
	jne	.labl_69
;----------------------------------------------------------
.labl_70:
	mov	eax,[ebp-0x10]
	push	ebx
	push	dword [ebp-0x3C]
	xor	edx,edx
	call	cm_pr_11
	mov	ecx,[ebp-0x20]
	mov	ebx,[ebp-0x48]
	imul	ecx,ecx,0x28
	add	eax,esi
	add	ecx,0x29578
	cmp	eax,[ecx+0xC]
	jnb	.labl_71
	and	dword [ecx+0x14],0
	mov	[ecx+0xC],eax
	mov	eax,[ebp-0xC]
	lea	edx,[edi+eax]
	mov	[ecx+4],eax
	mov	eax,[ebp-0x44]
	add	eax,3
	mov	[ecx+0x10],edx
	mov	byte [ecx+1],1
	mov	byte [ecx+2],1
	mov	[ecx+8],eax
;----------------------------------------------------------
.labl_71:
	sub	dword [ebp-0x24],4
	sub	ebx,0x28	;'('
	dec	edi
	lea	eax,[edi-1]
	mov	[ebp-0x48],ebx
	cmp	eax,2
	jnb	.labl_63
;----------------------------------------------------------
.labl_72:
	inc	dword [ebp-0xC]
	mov	eax,[ebp-0xC]
	cmp	eax,[ebp-8]
	jne	.labl_21
	jmp	.labl_74
;----------------------------------------------------------
.labl_73:
	mov	eax,[ebp-0x2C]
	mov	[0x19FAC],eax
	mov	byte [0x19FAA],1	;0x1B238
;----------------------------------------------------------
.labl_74:
	push	dword [ebp-0xC]
	push	dword [ebp+0xC]
	call	lz_cm_08
	mov	ecx,[ebp+0x10]
	mov	[ecx],eax
;----------------------------------------------------------
.labl_75:
	pop	ebx
	pop	edi
;----------------------------------------------------------
.labl_76:
	pop	esi
	leave	
	ret	0xC
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_04:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ebx
	push	esi
	xor	ebx,ebx
	push	edi
	inc	ebx
	mov	dword [ebp-4],8
;----------------------------------------------------------
.labl_00:
	dec	dword [ebp-4]
	movzx	ecx,byte [ebp-4]
	movzx	esi,byte [ebp+0xC]
	movzx	edi,byte [ebp+0x10]
	shr	esi,cl
	shr	edi,cl
	mov	ecx,[ebp+8]
	and	esi,1
	lea	eax,[esi+1]
	shl	eax,8
	add	eax,ebx
	and	edi,1
	push	edi
	lea	edx,[ecx+eax*4]
	call	cm_pr_14
	add	ebx,ebx
	or	ebx,edi
	cmp	esi,edi
	jne	.labl_02
	cmp	dword [ebp-4],0
	jne	.labl_00
	jmp	.labl_03
;----------------------------------------------------------
.labl_01:
	dec	dword [ebp-4]
	movzx	esi,byte [ebp+0x10]
	mov	cl,[ebp-4]
	mov	eax,[ebp+8]
	shr	esi,cl
	lea	edx,[eax+ebx*4]
	and	esi,1
	push	esi
	call	cm_pr_14
	add	ebx,ebx
	or	ebx,esi
;----------------------------------------------------------
.labl_02:
	cmp	dword [ebp-4],0
	jne	.labl_01
;----------------------------------------------------------
.labl_03:
	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	0xC
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_05:
	push	esi
	mov	esi,eax
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+8]
	shr	dword [0x51A04],1
	dec	esi
	mov	ecx,esi
	shr	eax,cl
	test	al,1
	je	.labl_01
	mov	eax,[0x51A04]
	add	[0x51A10],eax
	adc	dword [0x51A14],0
;----------------------------------------------------------
.labl_01:
	cmp	dword [0x51A04],0x1000000
	jnb	.labl_02
	shl	dword [0x51A04],8
	call	cm_pr_13
;----------------------------------------------------------
.labl_02:
	test	esi,esi
	jne	.labl_00
	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_06:
	push	esi
	xor	esi,esi
	push	edi
	mov	edi,[0x519E8]
	inc	esi
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+0xC]
	and	eax,1
	lea	edx,[0x515E8+esi*4]
	push	eax
	lea	esi,[eax+esi*2]
	call	cm_pr_14
	shr	dword [esp+0xC],1
	dec	edi
	jne	.labl_00
	pop	edi
	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_07:
	push	esi
	mov	esi,eax
	test	esi,esi
	je	.labl_01
	add	[0x1AEFC],esi
;----------------------------------------------------------
.labl_00:
	dec	esi
	call	lz_cm_09
	call	cm_pr_19
	test	esi,esi
	jne	.labl_00
;----------------------------------------------------------
.labl_01:
	pop	esi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_08:
	push	ebp
	mov	ebp,esp
	push	ecx
	mov	ecx,[ebp+0xC]
	mov	eax,ecx
	imul	eax,eax,0x28
	mov	edx,[eax+0x29588]
	mov	eax,[eax+0x2958C]
	push	ebx
	push	esi
	mov	[0x29574],ecx
	mov	[ebp-4],eax
	push	edi
;----------------------------------------------------------
.labl_00:
	imul	ecx,ecx,0x28
	cmp	byte [ecx+0x29579],0
	je	.labl_01
	mov	eax,edx
	imul	eax,eax,0x28
	or	dword [eax+0x2958C],0xFFFFFFFF
	lea	esi,[eax+0x29579]
	lea	edi,[eax+0x29588]
	lea	ebx,[edx-1]
	mov	byte [esi],0
	mov	[edi],ebx
	cmp	byte [ecx+0x2957A],0
	je	.labl_01
	mov	byte [esi-0x28],0
	mov	esi,[ecx+0x2957C]
	mov	[edi-0x28],esi
	mov	ecx,[ecx+0x29580]
	mov	[eax+0x29564],ecx
;----------------------------------------------------------
.labl_01:
	mov	ecx,[ebp-4]
	mov	eax,edx
	imul	eax,eax,0x28
	mov	esi,edx
	mov	edx,[eax+0x2958C]
	mov	[eax+0x2958C],ecx
	mov	ecx,[ebp+0xC]
	mov	[ebp-4],edx
	mov	edx,[eax+0x29588]
	mov	[eax+0x29588],ecx
	mov	ecx,esi
	mov	[ebp+0xC],ecx
	test	esi,esi
	jne	.labl_00
	mov	eax,[0x2958C]
	mov	ecx,[ebp+8]
	pop	edi
	mov	[ecx],eax
	mov	eax,[0x29588]
	pop	esi
	mov	[0x1AEF8],eax
	pop	ebx
	leave
	ret	8
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_09:
	push	ebp
	mov	ebp,esp
	mov	edx,[0x52218]
	mov	eax,[0x5262C]
	mov	ecx,[0x52630]
	sub	esp,0x2C
	push	esi
	lea	esi,[edx+eax]
	cmp	esi,ecx
	ja	.labl_00
	mov	[ebp-0x18],edx
	jmp	.labl_01
;----------------------------------------------------------
.labl_00:
	sub	ecx,eax
	mov	[ebp-0x18],ecx
	cmp	ecx,3
	jb	.labl_13
;----------------------------------------------------------
.labl_01:
	mov	ecx,[0x51A0C]
	mov	edx,eax
	sub	edx,ecx
	cmp	ecx,eax
	mov	ecx,[0x52628]
	lea	esi,[ecx+eax]
	movzx	ecx,byte [esi+2]
	push	ebx
	movzx	ebx,word [esi]
	push	edi
	sbb	edi,edi
	and	dword [ebp-0x1C],0
	and	dword [ebp-0x28],0
	shl	ecx,8
	and	edi,edx
	movzx	edx,byte [esi]
	xor	ecx,[0x52228+edx*4]
	movzx	edx,byte [esi+1]
	xor	ecx,edx
	mov	edx,[0x52224]
	and	ecx,0xFFFFF
	lea	ecx,[edx+ecx*4]
	mov	[edx+ebx*4+0x400000],eax
	mov	ebx,[ecx]
	mov	[ecx],eax
	mov	ecx,[0x5221C]
	lea	ecx,[edx+ecx*8+0x440000]
	mov	[ebp-4],ebx
	lea	ebx,[ecx+4]
	mov	[ebp-0xC],ecx
	mov	ecx,[0x52220]
	mov	[ebp-0x10],ebx
	mov	[ebp-0x14],ecx
	cmp	[ebp-4],edi
	jbe	.labl_11
;----------------------------------------------------------
.labl_02:
	mov	ecx,[ebp-0x14]
	dec	dword [ebp-0x14]
	test	ecx,ecx
	je	.labl_11
	mov	ecx,[0x52628]
	mov	ebx,[ebp-4]
	add	ecx,ebx
	mov	[ebp-0x24],ecx
	mov	ecx,[ebp-0x28]
	cmp	ecx,[ebp-0x1C]
	jb	.labl_03
	mov	ecx,[ebp-0x1C]
;----------------------------------------------------------
.labl_03:
	mov	ebx,[ebp-0x24]
	mov	[ebp-8],ecx
	add	ecx,esi
	sub	ebx,esi
	mov	[ebp-0x20],ebx
	jmp	.labl_05
;----------------------------------------------------------
.labl_04:
	mov	ecx,[ebp-0x2C]
	mov	ebx,[ebp-0x20]
;----------------------------------------------------------
.labl_05:
	mov	bl,[ebx+ecx]
	cmp	bl,[ecx]
	jne	.labl_06
	inc	dword [ebp-8]
	inc	ecx
	mov	[ebp-0x2C],ecx
	mov	ecx,[ebp-8]
	cmp	ecx,[ebp-0x18]
	jne	.labl_04
;----------------------------------------------------------
.labl_06:
	mov	ecx,[0x5221C]
	mov	ebx,eax
	sub	ebx,[ebp-4]
	cmp	ebx,ecx
	ja	.labl_07
	sub	ecx,ebx
	jmp	.labl_08
;----------------------------------------------------------
.labl_07:
	sub	ecx,ebx
	add	ecx,[0x51A0C]
;----------------------------------------------------------
.labl_08:
	lea	ecx,[edx+ecx*8+0x440000]
	mov	[ebp-0x20],ecx
	mov	ecx,[ebp-8]
	cmp	ecx,[ebp-0x18]
	je	.labl_14
	mov	ebx,[ebp-0x24]
	mov	bl,[ecx+ebx]
	cmp	bl,[ecx+esi]
	mov	ecx,[ebp-4]
	jnb	.labl_09
	mov	ebx,[ebp-0xC]
	mov	[ebx],ecx
	mov	ecx,[ebp-0x20]
	mov	ebx,[ebp-8]
	add	ecx,4
	mov	[ebp-0xC],ecx
	mov	[ebp-0x1C],ebx
	jmp	.labl_10
;----------------------------------------------------------
.labl_09:
	mov	ebx,[ebp-0x10]
	mov	[ebx],ecx
	mov	ecx,[ebp-0x20]
	mov	ebx,[ebp-8]
	mov	[ebp-0x10],ecx
	mov	[ebp-0x28],ebx
;----------------------------------------------------------
.labl_10:
	mov	ecx,[ecx]
	mov	[ebp-4],ecx
	cmp	ecx,edi
	ja	.labl_02
;----------------------------------------------------------
.labl_11:
	mov	eax,[ebp-0xC]
	and	dword [eax],0
	mov	eax,[ebp-0x10]
	and	dword [eax],0
;----------------------------------------------------------
.labl_12:
	pop	edi
	pop	ebx
;----------------------------------------------------------
.labl_13:
	pop	esi
	leave	
	ret	
;----------------------------------------------------------
.labl_14:
	mov	eax,[ebp-0x20]
	mov	ecx,[eax]
	mov	edx,[ebp-0xC]
	mov	[edx],ecx
	mov	eax,[eax+4]
	mov	ecx,[ebp-0x10]
	mov	[ecx],eax
	jmp	.labl_12
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_000:
	cmp	eax,0x400
	jnb	.labl_00
	movzx	eax,byte [eax+0x19FB0]
	ret 
;----------------------------------------------------------
.labl_00:
	cmp	eax,0x80000
	jnb	.labl_01
	shr	eax,9
	movzx	eax,byte [eax+0x19FB0]
	add	eax,0x12
	ret 
;----------------------------------------------------------
.labl_01:
	shr	eax,0x12
	movzx	eax,byte [eax+0x19FB0]
	add	eax,0x24 ;'$'
	ret 
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_00:
	push	ebx
	push	edi
	mov	eax,0x400
	push	4
	mov	[esi],eax
	mov	[esi+4],eax
	lea	edi,[esi+0x4048]
	pop	ebx
;----------------------------------------------------------
.labl_00:
	push	3
	lea	eax,[edi-0x4040]
	pop	ecx
	call	cm_pr_15
	push	3
	pop	ecx
	mov	eax,edi
	call	cm_pr_15
	add	edi,0x404
	dec	ebx
	jne	.labl_00
	push	8
	pop	ecx
	pop	edi
	lea	eax,[esi+0x8088]
	pop	ebx
	jmp	cm_pr_15
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_01:
	push	ebx
	push	edi
	mov	edi,eax
	xor	ebx,ebx
	cmp	[edi+0xC88C],ebx
	jbe	.labl_01
	mov	eax,[esp+0xC]
	push	esi
	lea	esi,[edi+eax*4+0x848C]
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+0x10]
	mov	edx,ebx
	call	lz_cm_10
	mov	[esi],eax
	inc	ebx
	add	esi,0x40	;'@'
	cmp	ebx,[edi+0xC88C]
	jb	.labl_00
	pop	esi
;----------------------------------------------------------
.labl_01:
	mov	eax,[edi+0xC88C]
	mov	ecx,[esp+0xC]
	mov	[edi+ecx*4+0xC890],eax
	pop	edi
	pop	ebx
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_10:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	esi
	cmp	edx,8
	jnb	.labl_00
	imul	eax,eax,0x404
	lea	esi,[eax+edi+8]
	call	cm_pr_17
	mov	ecx,[edi]
	shr	ecx,2
	add	eax,[0x51A18+ecx*4]
	jmp	.labl_03
;----------------------------------------------------------
.labl_00:
	push	ebx
	mov	ebx,0x800
	mov	ecx,ebx
	sub	ecx,[edi]
	shr	ecx,2
	mov	ecx,[0x51A18+ecx*4]
	mov	[ebp-4],ecx
	cmp	edx,0x10
	jnb	.labl_01
	imul	eax,eax,0x404
	add	edx,0xFFFFFFF8
	lea	esi,[eax+edi+0x4048]
	call	cm_pr_17
	mov	ecx,[edi+4]
	shr	ecx,2
	add	eax,[0x51A18+ecx*4]
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	add	edx,0xFFFFFFF0
	lea	esi,[edi+0x8088]
	call	cm_pr_17
	sub	ebx,[edi+4]
	shr	ebx,2
	add	eax,[0x51A18+ebx*4]
;----------------------------------------------------------
.labl_02:
	add	[ebp-4],eax
	mov	eax,[ebp-4]
	pop	ebx
;----------------------------------------------------------
.labl_03:
	pop	esi
	leave	
	ret	
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_02:
	push	edi
	mov	edi,eax
	mov	edx,esi
	cmp	edi,8
	jnb	.labl_00
	push	0
	call	cm_pr_14
	mov	eax,ebx
	imul	eax,eax,0x404
	push	edi
	lea	edi,[eax+esi+8]
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	push	1
	call	cm_pr_14
	lea	edx,[esi+4]
	cmp	edi,0x10
	jnb	.labl_01
	push	0
	call	cm_pr_14
	add	edi,0xFFFFFFF8
	mov	eax,ebx
	imul	eax,eax,0x404
	push	edi
	lea	edi,[eax+esi+0x4048]
	jmp	.labl_02
;----------------------------------------------------------
.labl_01:
	push	1
	call	cm_pr_14
	add	edi,0xFFFFFFF0
	push	edi
	lea	edi,[esi+0x8088]
;----------------------------------------------------------
.labl_02:
	call	cm_pr_16
	dec	dword [esi+ebx*4+0xC890]
	pop	edi
	jne	.labl_03
	push	ebx
	mov	eax,esi
	call	cm_pr_01
;----------------------------------------------------------
.labl_03:
	ret	
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_03:
	push	ebx
	push	esi
	push	edi
	xor	edi,edi
	push	8
	inc	edi
	pop	ebx
;----------------------------------------------------------
.labl_00:
	movzx	esi,byte [esp+0x14]
	mov	eax,[esp+0x10]
	dec	ebx
	mov	cl,bl
	shr	esi,cl
	lea	edx,[eax+edi*4]
	and	esi,1
	push	esi
	call	cm_pr_14
	add	edi,edi
	or	edi,esi
	test	ebx,ebx
	jne	.labl_00
	pop	edi
	pop	esi
	pop	ebx
	ret	8
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_04:
	mov	edx,[0x519F4]
	push	esi
	mov	esi,[0x519FC]
	and	esi,[esp+8]
	push	8
	pop	ecx
	movzx	eax,al
	sub	cl,dl
	shr	eax,cl
	mov	ecx,edx
	shl	esi,cl
	add	eax,esi
	pop	esi
	ret	4
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_05:
	push	dword [esp+0x10]
	push	dword [esp+0x10]
	push	dword [esp+0x10]
	push	dword [esp+0x10]
	call	cm_pr_04
	imul	eax,eax,0xC00
	add	eax,[0x519F0]
	push	eax
	call	lz_cm_11
	ret	0x10
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_11:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ebx
	push	esi
	xor	esi,esi
	xor	eax,eax
	inc	esi
	push	edi
	mov	dword [ebp-4],8
	cmp	[ebp+0xC],al
	je	.labl_02
	movzx	ecx,byte [ebp+0x14]
	mov	[ebp+0xC],ecx
;----------------------------------------------------------
.labl_00:
	dec	dword [ebp-4]
	movzx	ecx,byte [ebp-4]
	movzx	edi,byte [ebp+0x10]
	mov	edx,[ebp+0xC]
	mov	ebx,[ebp+8]
	shr	edi,cl
	shr	edx,cl
	and	edi,1
	mov	ecx,edi
	shl	ecx,8
	add	ecx,esi
	mov	ecx,[ebx+ecx*4+0x400]
	and	edx,1
	sub	ecx,edx
	mov	ebx,edx
	neg	ebx
	xor	ecx,ebx
	shr	ecx,2
	and	ecx,0x1FF
	add	eax,[0x51A18+ecx*4]
	add	esi,esi
	or	esi,edx
	cmp	edi,edx
	jne	.labl_01
	cmp	dword [ebp-4],0
	jne	.labl_00
	jmp	.labl_04
;----------------------------------------------------------
.labl_01:
	cmp	dword [ebp-4],0
	je	.labl_04
;----------------------------------------------------------
.labl_02:
	movzx	edi,byte [ebp+0x14]
;----------------------------------------------------------
.labl_03:
	dec	dword [ebp-4]
	mov	cl,[ebp-4]
	mov	edx,edi
	shr	edx,cl
	mov	ecx,[ebp+8]
	mov	ecx,[ecx+esi*4]
	add	esi,esi
	and	edx,1
	sub	ecx,edx
	mov	ebx,edx
	neg	ebx
	xor	ecx,ebx
	shr	ecx,2
	and	ecx,0x1FF
	add	eax,[0x51A18+ecx*4]
	or	esi,edx
	cmp	dword [ebp-4],0
	jne	.labl_03
;----------------------------------------------------------
.labl_04:
	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	0x10
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_06:
	push	esi
	call	lz_cm_12
	mov	ecx,eax
	mov	esi,0x80	;'€'
	cmp	ecx,esi
	jne	.labl_00
	push	dword [0x1ACB0]
	lea	eax,[esi+0x11]
	push	esi
	call	cm_pr_18
	mov	ecx,eax
	add	ecx,esi
;----------------------------------------------------------
.labl_00:
	inc	dword [0x1AEFC]
	call	cm_pr_19
	mov	eax,ecx
	pop	esi
	ret	
;*****************************************************************************

;* Call: ***************************************************************
lz_cm_12:
	push	ebp
	mov	ebp,esp
	sub	esp,0x34
	mov	edx,[0x52218]
	mov	ecx,[0x5262C]
	mov	eax,[0x52630]
	and	dword [ebp-0x18],0
	push	esi
	lea	esi,[edx+ecx]
	cmp	esi,eax
	ja	.labl_02
	mov	[ebp-0x20],edx
;----------------------------------------------------------
.labl_00:
	mov	eax,[0x51A0C]
	mov	edx,ecx
	sub	edx,eax
	cmp	eax,ecx
	sbb	eax,eax
	and	eax,edx
	mov	[ebp-0x28],eax
	mov	eax,[0x52628]
	push	ebx
	push	edi
	lea	edi,[eax+ecx]
	movzx	edx,byte [edi+2]
	movzx	esi,byte [edi]
	movzx	eax,word [edi]
	shl	edx,8
	xor	edx,[0x52228+esi*4]
	movzx	esi,byte [edi+1]
	xor	edx,esi
	mov	esi,[0x52224]
	and	edx,0xFFFFF
	lea	edx,[esi+edx*4]
	mov	ebx,[edx]
	lea	eax,[esi+eax*4+0x400000]
	mov	[ebp-8],ebx
	mov	ebx,[eax]
	mov	[eax],ecx
	or	dword [0x01AAB8],0xFFFFFFFF
	cmp	ebx,[ebp-0x28]
	jbe	.labl_01
	mov	eax,ecx
	sub	eax,ebx
	dec	eax
	mov	[0x01AAB8],eax
	mov	dword [ebp-0x18],2
;----------------------------------------------------------
.labl_01:
	mov	ebx,[0x5221C]
	and	dword [ebp-0x24],0
	mov	[edx],ecx
	or	dword [0x1AABC],0xFFFFFFFF
	and	dword [ebp-0x30],0
	lea	edx,[esi+ebx*8+0x440000]
	lea	eax,[edx+4]
	mov	[ebp-0x14],eax
	mov	eax,[0x52220]
	mov	[ebp-0x1C],eax
	mov	eax,[ebp-8]
	mov	[ebp-0x10],edx
	jmp	.labl_13
;----------------------------------------------------------
.labl_02:
	sub	eax,ecx
	mov	[ebp-0x20],eax
	cmp	eax,3
	jnb	.labl_00
	xor	eax,eax
	jmp	.labl_16
;----------------------------------------------------------
.labl_03:
	mov	eax,[ebp-0x1C]
	dec	dword [ebp-0x1C]
	test	eax,eax
	je	.labl_14
	mov	eax,[0x52628]
	mov	edx,[ebp-8]
	add	eax,edx
	mov	[ebp-0x2C],eax
	mov	eax,[ebp-0x30]
	cmp	eax,[ebp-0x24]
	jb	.labl_04
	mov	eax,[ebp-0x24]
;----------------------------------------------------------
.labl_04:
	mov	edx,[ebp-0x2C]
	mov	[ebp-4],eax
	add	eax,edi
	sub	edx,edi
	mov	[ebp-0xC],edx
	jmp	.labl_06
;----------------------------------------------------------
.labl_05:
	mov	edx,[ebp-0xC]
	mov	eax,[ebp-0x34]
;----------------------------------------------------------
.labl_06:
	mov	dl,[edx+eax]
	cmp	dl,[eax]
	jne	.labl_07
	inc	dword [ebp-4]
	inc	eax
	mov	[ebp-0x34],eax
	mov	eax,[ebp-4]
	cmp	eax,[ebp-0x20]
	jne	.labl_05
;----------------------------------------------------------
.labl_07:
	mov	eax,ecx
	sub	eax,[ebp-8]
	mov	[ebp-0xC],eax
	mov	eax,[ebp-0x18]
	cmp	eax,[ebp-4]
	jnb	.labl_09
;----------------------------------------------------------
.labl_08:
	mov	edx,[ebp-0xC]
	inc	eax
	dec	edx
	mov	[0x1AAB0+eax*4],edx
	cmp	eax,[ebp-4]
	jb	.labl_08
	mov	[ebp-0x18],eax
;----------------------------------------------------------
.labl_09:
	mov	eax,ebx
	sub	eax,[ebp-0xC]
	cmp	[ebp-0xC],ebx
	jbe	.labl_10
	add	eax,[0x51A0C]
;----------------------------------------------------------
.labl_10:
	lea	eax,[esi+eax*8+0x440000]
	mov	[ebp-0xC],eax
	mov	eax,[ebp-4]
	cmp	eax,[ebp-0x20]
	je	.labl_17
	mov	edx,[ebp-0x2C]
	mov	dl,[eax+edx]
	cmp	dl,[eax+edi]
	mov	eax,[ebp-8]
	jnb	.labl_11
	mov	edx,[ebp-0x10]
	mov	[edx],eax
	mov	eax,[ebp-0xC]
	mov	edx,[ebp-4]
	add	eax,4
	mov	[ebp-0x10],eax
	mov	[ebp-0x24],edx
	jmp	.labl_12
;----------------------------------------------------------
.labl_11:
	mov	edx,[ebp-0x14]
	mov	[edx],eax
	mov	eax,[ebp-0xC]
	mov	edx,[ebp-4]
	mov	[ebp-0x14],eax
	mov	[ebp-0x30],edx
;----------------------------------------------------------
.labl_12:
	mov	eax,[eax]
	mov	[ebp-8],eax
;----------------------------------------------------------
.labl_13:
	cmp	eax,[ebp-0x28]
	ja	.labl_03
;----------------------------------------------------------
.labl_14:
	mov	eax,[ebp-0x10]
	and	dword [eax],0
	mov	eax,[ebp-0x14]
	and	dword [eax],0
;----------------------------------------------------------
.labl_15:
	mov	eax,[ebp-0x18]
	pop	edi
	pop	ebx
;----------------------------------------------------------
.labl_16:
	pop	esi
	leave	
	ret	
;----------------------------------------------------------
.labl_17:
	mov	eax,[ebp-0xC]
	mov	ecx,[eax]
	mov	edx,[ebp-0x10]
	mov	[edx],ecx
	mov	eax,[eax+4]
	mov	ecx,[ebp-0x14]
	mov	[ecx],eax
	jmp	.labl_15
;*****************************************************************************

;* Call: ***************************************************************
cm_pr_07:
	push	ecx
	push	ebx
	push	ebp
	push	esi
	push	edi
	xor	ebp,ebp
	mov	esi,0x1BC18
	mov	dword [esp+0x10],4
;----------------------------------------------------------
.labl_00:
	xor	edi,edi
;----------------------------------------------------------
.labl_01:
	mov	ebx,[0x29570]
	cmp	edi,ebx
	jnb	.labl_03
	mov	edx,edi
	call	cm_pr_17
	lea	ecx,[edi+ebp]
	inc	edi
	mov	[0x1A3B0+ecx*4],eax
	cmp	edi,0xE
	jb	.labl_01
	cmp	edi,ebx
	jnb	.labl_03
	lea	ebx,[edi+ebp]
	lea	ebx,[0x1A3B0+ebx*4]
;----------------------------------------------------------
.labl_02:
	mov	edx,edi
	call	cm_pr_17
	mov	ecx,edi
	shr	ecx,1
	sub	ecx,5
	shl	ecx,6
	add	eax,ecx
	mov	[ebx],eax
	inc	edi
	add	ebx,4
	cmp	edi,[0x29570]
	jb	.labl_02
;----------------------------------------------------------
.labl_03:
	add	esi,0x404
	add	ebp,0x40	;'@'
	dec	dword [esp+0x10]
	jne	.labl_00
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	ecx
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_08:
	push	ebp
	mov	ebp,esp
	sub	esp,0x24
	and	dword [ebp-0x14],0
	push	ebx
	push	esi
	push	edi
	push	4
	pop	ebx
	mov	dword [ebp-0x18],0x1B418
	mov	dword [ebp-0x10],0x1A3B0
	mov	[ebp-0x24],ebx
;----------------------------------------------------------
.labl_00:
	mov	eax,[ebp-0x18]
	mov	esi,[ebp-0x10]
	lea	edi,[eax-0x10]
	movsd	
	movsd	
	movsd	
	movsd	
	mov	[ebp-4],ebx
	mov	[ebp-0xC],eax
;----------------------------------------------------------
.labl_01:
	mov	esi,[ebp-4]
	mov	eax,esi
	call	cm_pr_000
	mov	edx,eax
	mov	ecx,edx
	xor	eax,eax
	inc	eax
	mov	[ebp-0x20],edx
	and	edx,eax
	shr	ecx,1
	dec	ecx
	or	edx,2
	shl	edx,cl
	xor	edi,edi
	mov	[ebp-8],eax
	mov	[ebp-0x1C],ecx
	sub	esi,edx
	test	ecx,ecx
	je	.labl_03
;----------------------------------------------------------
.labl_02:
	mov	eax,[ebp-8]
	sub	eax,[ebp-0x20]
	mov	ecx,esi
	add	eax,edx
	mov	eax,[0x1B234+eax*4]
	and	ecx,1
	sub	eax,ecx
	mov	ebx,ecx
	neg	ebx
	xor	eax,ebx
	shr	eax,2
	and	eax,0x1FF
	add	edi,[0x51A18+eax*4]
	mov	eax,[ebp-8]
	shr	esi,1
	dec	dword [ebp-0x1C]
	lea	eax,[ecx+eax*2]
	mov	[ebp-8],eax
	jne	.labl_02
	push	4
	pop	ebx
;----------------------------------------------------------
.labl_03:
	mov	eax,[ebp-0x20]
	mov	ecx,[ebp-0x14]
	add	ecx,eax
	mov	eax,[0x1A3B0+ecx*4]
	mov	ecx,[ebp-0xC]
	add	[ebp-0xC],ebx
	add	eax,edi
	inc	dword [ebp-4]
	cmp	dword [ebp-4],0x80	;'€'
	mov	[ecx],eax
	jb	.labl_01
	add	dword [ebp-0x10],0x100
	add	dword [ebp-0x14],0x40	;'@'
	add	dword [ebp-0x18],0x200
	dec	dword [ebp-0x24]
	jne	.labl_00
	pop	edi
	pop	esi
	pop	ebx
	leave	
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_09:
	push	ebp
	mov	ebp,esp
	push	ecx
	push	ecx
	push	ebx
	push	esi
	xor	esi,esi
	push	edi
	mov	[ebp-8],esi
;----------------------------------------------------------
.labl_00:
	mov	eax,[0x519E8]
	xor	edx,edx
	xor	ecx,ecx
	inc	edx
	mov	[ebp-4],eax
;----------------------------------------------------------
.labl_01:
	mov	edi,[0x515E8+edx*4]
	mov	eax,esi
	and	eax,1
	sub	edi,eax
	mov	ebx,eax
	neg	ebx
	xor	edi,ebx
	shr	edi,2
	and	edi,0x1FF
	add	ecx,[0x51A18+edi*4]
	shr	esi,1
	dec	dword [ebp-4]
	lea	edx,[eax+edx*2]
	jne	.labl_01
	mov	esi,[ebp-8]
	push	0x10
	mov	[0x51578+esi*4],ecx
	inc	esi
	pop	eax
	mov	[ebp-8],esi
	cmp	esi,eax
	jb	.labl_00
	pop	edi
	pop	esi
	mov	[0x1B230],eax
	pop	ebx
	leave	
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_10:
	movzx	eax,al
	mov	edx,[0x1CC38+eax*4]
	mov	ecx,eax
	shl	ecx,4
	add	ecx,[esp+4]
	shr	edx,2
	mov	ecx,[0x1A7B0+ecx*4]
	shr	ecx,2
	mov	eax,[0x51A18+ecx*4]
	add	eax,[0x51A18+edx*4]
	ret	4
;***********************************************************************

;* Call: ***************************************************************
cm_pr_11:
	movzx	ecx,byte [esp+4]
	shl	eax,4
	add	eax,[esp+8]
	push	esi
	mov	eax,[0x25074+eax*4]
	mov	esi,0x800
	test	edx,edx
	jne	.labl_00
	mov	edx,ecx
	mov	ecx,[0x1CC38+ecx*4]
	shl	edx,4
	add	edx,[esp+0xC]
	sub	esi,[0x1A7B0+edx*4]
	shr	ecx,2
	jmp	.labl_02
;----------------------------------------------------------
.labl_00:
	push	edi
	shl	ecx,2
	mov	edi,esi
	sub	edi,[ecx+0x1CC38]
	shr	edi,2
	add	eax,[0x51A18+edi*4]
	pop	edi
	cmp	edx,1
	jne	.labl_01
	mov	ecx,[ecx+0x1AF00]
	shr	ecx,2
	add	eax,[0x51A18+ecx*4]
	jmp	.labl_03
;----------------------------------------------------------
.labl_01:
	sub	esi,[ecx+0x1AF00]
	mov	ecx,[ecx+0x29538]
	add	edx,0xFFFFFFFE
	sub	ecx,edx
	neg	edx
	xor	ecx,edx
	shr	ecx,2
	and	ecx,0x1FF
;----------------------------------------------------------
.labl_02:
	shr	esi,2
	mov	edx,[0x51A18+esi*4]
	add	edx,[0x51A18+ecx*4]
	add	eax,edx
;----------------------------------------------------------
.labl_03:
	pop	esi
	ret	8
;***********************************************************************

;* Call: ***************************************************************
cm_pr_12:
	push	esi
	mov	esi,eax
	mov	ecx,0x80	;'€'
	cmp	esi,2
	jne	.labl_00
	cmp	edx,ecx
	jb	.labl_01
	mov	eax,0x0FFFFFFF
	jmp	.labl_09
;----------------------------------------------------------
.labl_00:
	cmp	esi,6
	jnb	.labl_02
;----------------------------------------------------------
.labl_01:
	lea	eax,[esi-2]
	jmp	.labl_03
;----------------------------------------------------------
.labl_02:
	push	3
	pop	eax
;----------------------------------------------------------
.labl_03:
	cmp	edx,ecx
	jnb	.labl_04
	shl	eax,7
	add	eax,edx
	mov	ecx,[0x1B408+eax*4]
	jmp	.labl_08
;----------------------------------------------------------
.labl_04:
	mov	ecx,edx
	cmp	edx,0x10000
	jnb	.labl_05
	shr	ecx,6
	movzx	ecx,byte [ecx+0x19FB0]
	add	ecx,0xC
	jmp	.labl_07
;----------------------------------------------------------
.labl_05:
	cmp	edx,0x2000000
	jnb	.labl_06
	shr	ecx,0xF
	movzx	ecx,byte [ecx+0x19FB0]
	add	ecx,0x1E
	jmp	.labl_07
;----------------------------------------------------------
.labl_06:
	shr	ecx,0x18
	movzx	ecx,byte [ecx+0x19FB0]
	add	ecx,0x30	;'0'
;----------------------------------------------------------
.labl_07:
	shl	eax,6
	add	eax,ecx
	mov	ecx,[0x1A3B0+eax*4]
	and	edx,0xF
	add	ecx,[0x51578+edx*4]
;----------------------------------------------------------
.labl_08:
	shl	esi,4
	add	esi,[esp+8]
	mov	eax,[0x15AE4+esi*4]
	add	eax,ecx
;----------------------------------------------------------
.labl_09:
	pop	esi
	ret	4
;***********************************************************************

;* Call: ***************************************************************
cm_pr_13:
	push	ecx
	push	ecx
	mov	eax,[0x51A10]
	push	esi
	mov	esi,[0x51A14]
	cmp	eax,0xFF000000
	jb	.labl_00
	test	esi,esi
	je	.labl_02
;----------------------------------------------------------
.labl_00:
	mov	cl,[0x519ED]
	and	dword [esp+8],0
	mov	edx,esi
	push	ebx
;----------------------------------------------------------
.labl_01:
	mov	bl,dl
	add	bl,cl
	mov	ecx,[0x52634]
	inc	dword [0x52634]
	mov	[ecx],bl
	or	cl,0xFF
	dec	dword [0x51A08]
	jne	.labl_01
	mov	cl,[0x51A13]
	mov	[0x519ED],cl
	pop	ebx
;----------------------------------------------------------
.labl_02:
	inc	dword [0x51A08]
	shl	eax,8
	and	dword [0x51A14],0
	mov	[0x51A10],eax
	pop	esi
	pop	ecx
	pop	ecx
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_14:
	mov	ecx,[0x51A04]
	mov	eax,[edx]
	shr	ecx,0xB
	imul	ecx,eax
	cmp	dword [esp+4],0
	jne	.labl_00
	mov	[0x51A04],ecx
	mov	ecx,0x800
	sub	ecx,eax
	shr	ecx,5
	add	ecx,eax
	mov	[edx],ecx
	jmp	.labl_01
;----------------------------------------------------------
.labl_00:
	add	[0x51A10],ecx
	adc	dword [0x51A14],0
	sub	[0x51A04],ecx
	mov	ecx,eax
	shr	ecx,5
	sub	eax,ecx
	mov	[edx],eax
;----------------------------------------------------------
.labl_01:
	cmp	dword [0x51A04],0x1000000
	jnb	.labl_02
	shl	dword [0x51A04],8
	call	cm_pr_13
;----------------------------------------------------------
.labl_02:
	ret	4
;***********************************************************************

;* Call: ***************************************************************
cm_pr_15:
	xor	edx,edx
	inc	edx
	shl	edx,cl
	mov	[eax+0x400],ecx
	lea	ecx,[edx-1]
	test	ecx,ecx
	jbe	.labl_00
	push	edi
	lea	edi,[eax+4]
	mov	eax,0x400
	rep	stosd	
	pop	edi
;----------------------------------------------------------
.labl_00:
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_16:
	mov	eax,[esp+4]
	push	ebx
	mov	ebx,[edi+0x400]
	push	esi
	xor	esi,esi
	inc	esi
	add	eax,eax
	mov	[esp+0xC],eax
;----------------------------------------------------------
.labl_00:
	mov	eax,[esp+0xC]
	mov	ecx,ebx
	shr	eax,cl
	lea	edx,[edi+esi*4]
	and	eax,1
	push	eax
	lea	esi,[eax+esi*2]
	call	cm_pr_14
	dec	ebx
	jne	.labl_00
	pop	esi
	pop	ebx
	ret	4
;***********************************************************************

;* Call: ***************************************************************
cm_pr_17:
	mov	ecx,[esi+0x400]
	push	edi
	xor	edi,edi
	inc	edi
	shl	edi,cl
	xor	eax,eax
	or	edx,edi
;----------------------------------------------------------
.labl_00:
	mov	ecx,edx
	shr	edx,1
	mov	edi,[esi+edx*4]
	and	ecx,1
	sub	edi,ecx
	neg	ecx
	xor	edi,ecx
	shr	edi,2
	and	edi,0x1FF
	add	eax,[0x51A18+edi*4]
	cmp	edx,1
	jne	.labl_00
	pop	edi
	ret	
;***********************************************************************

;* Call: ***************************************************************
cm_pr_18:
	mov	ecx,[0x52630]
	push	edi
	mov	edi,eax
	mov	eax,[0x5262C]
	sub	ecx,eax
	sub	ecx,[esp+8]
	cmp	edi,ecx
	jbe	.labl_00
	mov	edi,ecx
;----------------------------------------------------------
.labl_00:
	mov	ecx,[0x52628]
	inc	dword [esp+0xC]
	add	ecx,eax
	add	ecx,[esp+8]
	xor	eax,eax
	test	edi,edi
	jbe	.labl_03
	push	esi
	mov	esi,ecx
	sub	esi,[esp+0x10]
;----------------------------------------------------------
.labl_01:
	mov	dl,[eax+ecx]
	cmp	dl,[esi+eax]
	jne	.labl_02
	inc	eax
	cmp	eax,edi
	jb	.labl_01
;----------------------------------------------------------
.labl_02:
	pop	esi
;----------------------------------------------------------
.labl_03:
	pop	edi
	ret	8
;***********************************************************************

;* Call: ***************************************************************
cm_pr_19:
	inc	dword [0x5221C]
	mov	eax,[0x5221C]
	cmp	eax,[0x51A0C]
	jne	.labl_00
	and	dword [0x5221C],0
;----------------------------------------------------------
.labl_00:
	inc	dword [0x5262C]
	ret	
;***********************************************************************
