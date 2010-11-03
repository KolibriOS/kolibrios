;*********************************************************************
pack:
;	call	clear_edit_points
;	and	[curedit],0
; clear messages
	call	clear_messages
; display logo
	mov	esi,info_str
	push	info_len
	pop	ecx
	call	write_string
; load input file
	mov	esi,inname
	call	get_full_name
	mov	ebx,fn70block
	mov	[ebx],dword 5
	and	[ebx+4],dword 0
	and	[ebx+8],dword 0
	and     [ebx+12],dword 0
	mov	[ebx+16],dword  file_attr
	mcall	70
	test	eax,eax
	jz	inopened
;---------------------------------------------------------------------
infileerr:
	call	return_memory
	mov	esi,errload_str
	push	errload_len
	pop	ecx
	jmp	write_string
;---------------------------------------------------------------------
inopened:
        mov     ebx,[insize]
        test    ebx,ebx
        jz      infileerr
; maximum memory requests: 2*insize + 2*(maxoutsize+400h) + worksize
;	mov	esi,[memf]
	xor	esi,esi
;	mov	[infile],esi
	add	esi,ebx
	mov	[inbuftmp],esi
	add	esi,ebx
	mov	[outfile],esi
	mov	[outfile1],esi
	mov	[outfilebest],esi

	mov	ecx,ebx
	shr	ecx,3
	add	ecx,ebx
	add	ecx,400h
	add	esi,ecx
	mov	[outfile2],esi
	add	esi,ecx
	mov	[workmem],esi
	add	ecx,ebx
	add	ecx,ecx
;	add	ecx,[memf]
; LZMA requires 0x448000 + dictsize*9.5 bytes for workmem,
	and	[lzma_dictsize],0
	push	ecx
	mov	eax,ebx
	dec	eax
	bsr	ecx,eax
	inc	ecx
	cmp	ecx,28
	jb	@f

	mov	cl,28
;--------------------------------------
@@:
	mov	edx,ecx
	xor	eax,eax
	inc	eax
	shl	eax,cl
	imul	eax,19
	shr	eax,1
	add	eax,448000h
	pop	ecx
	add	ecx,eax
;	mcall	64,1
	
;	test	eax,eax
;	jz	mem_ok

	mcall	68,12

	mov	[infile],eax
	add	[inbuftmp],eax
	add	[outfile],eax
	add	[outfile1],eax
	add	[outfilebest],eax
	add	[outfile2],eax
	add	[workmem],eax
;--------------------------------------
; try to use smaller dictionary
;meml0:
;	cmp	edx,4
;	jbe	memf1
;
;	dec	edx
;	xor	eax,eax
;	inc	eax
;	mov	ecx,edx
;	shl	eax,cl
;	imul	eax,19
;	shr	eax,1
;	add	eax,509000h
;	pop	ecx
;	push	ecx
;	add	ecx,eax
;	mcall	64
;	test	eax,eax
;	jnz	meml0
;--------------------------------------
; ok, say warning and continue
;	mov	[lzma_dictsize],edx
;	mov	esi,lzma_memsmall_str
;	push	lzma_memsmall_len
;	pop	ecx
;	call	write_string
;	jmp	mem_ok
;---------------------------------------------------------------------
;memf1:
;	mov	esi,nomem_str
;	push	nomem_len
;	pop	ecx
;	jmp	write_string
;---------------------------------------------------------------------
mem_ok:
	mov	eax,[insize]
	mov	ebx,fn70block
	mov	[ebx],byte 0
	mov	[ebx+12],eax
	mov	esi,[infile]
	mov	[ebx+16],esi
	mcall	70
	test	eax,eax
	jnz	infileerr

	mov	eax,[outfile]
	mov	[eax],dword 'KPCK'	;'KCPK'
	mov     ecx,[insize]
	mov	[eax+4],dword ecx
	mov	edi,eax
; set LZMA dictionary size
	mov	eax,[lzma_dictsize]
	test	eax,eax
	js	no_lzma_setds
	jnz	lzma_setds

	mov	ecx,[insize]
	dec	ecx
	bsr	eax,ecx
	inc	eax
	cmp	eax,28
	jb	lzma_setds

	mov	eax,28
;--------------------------------------
lzma_setds:
	push	eax
	call	lzma_set_dict_size
;--------------------------------------
no_lzma_setds:
	push	compressing_len
	pop	ecx
	mov	esi,compressing_str
	call	write_string
	mov	esi,[outfile1]
	mov     edi,[outfile2]
	movsd
	movsd
	movsd
	call	pack_lzma
	mov	[outsize],eax
	mov	eax,[outfile]
	mov	[outfilebest],eax
	mov	[method],use_lzma
;--------------------------------------
@@:
	call	preprocess_calltrick
	test	eax,eax
	jz	noct1

	call	set_outfile
	call	pack_lzma
	add	eax,5
	cmp	eax,[outsize]
	jae	@f

	mov	[outsize],eax
	mov	eax,[outfile]
	mov	[outfilebest],eax
	mov	[method],use_lzma or use_calltrick1
;--------------------------------------
@@:
noct1:
	call	set_outfile
	push	[ctn]
	mov	al,[cti]
	push	eax
	call	preprocess_calltrick2
	test	eax,eax
	jz	noct2

	call	set_outfile
	call	pack_lzma
	add	eax,5
	cmp	eax,[outsize]
	jae	@f

	mov	[outsize],eax
	mov	eax,[outfile]
	mov	[outfilebest],eax
	mov	[method],use_lzma or use_calltrick2
	pop	ecx
	pop	ecx
	push	[ctn]
	mov	al,[cti]
	push	eax
;--------------------------------------
@@:
noct2:
	pop	eax
	mov	[cti],al
	pop	[ctn]
	add     [outsize],12
	mov	eax,[outsize]
	cmp	eax,[insize]
	jb	packed_ok

	mov	esi,too_big_str
	push	too_big_len
	pop	ecx
	jmp	write_string
;---------------------------------------------------------------------
packed_ok:
; set header
        movzx	eax,[method]
	mov	edi,[outfilebest]
	mov     [edi+8],eax
	test	al,use_calltrick1 or use_calltrick2
	jz	@f

	mov	ecx,[outsize]
	add	ecx,edi
	mov	eax,[ctn]
	mov     [ecx-5],eax
	mov	al,[cti]
	mov     [ecx-1],al
;--------------------------------------
@@:
	mov	eax,[outsize]
	mov	ecx,100
	mul	ecx
	div	[insize]
	aam
	xchg	al,ah
	add	ax,'00'
	mov	[ratio],ax
	mov	esi,done_str
	push	done_len
	pop	ecx
	call	write_string
;--------------------------------------
; save output file
saveout:
	mov	esi,outname
	call	get_full_name
	mov	ebx,fn70block
	mov	[ebx],byte 2
	mov	eax,[outfilebest]
	mov	ecx,[outsize]
	mov	[ebx+12],ecx
	mov	[ebx+16],eax
	mcall	70
	test	eax,eax
	jz	@f
;--------------------------------------
outerr:
	mov	esi,outfileerr_str
	push	outfileerr_len
	pop	ecx
	jmp	write_string
;---------------------------------------------------------------------
@@:
	xor	eax,eax
	mov	ebx,fn70block
	mov	[ebx],byte 6
	mov	[ebx+4],eax
	mov	[ebx+8],eax
	mov	[ebx+12],eax
	mov	[ebx+16],dword file_attr
	mcall	70

	call	return_memory
	ret
;---------------------------------------------------------------------
set_outfile:
	mov	eax,[outfilebest]
	xor	eax,[outfile1]
	xor	eax,[outfile2]
	mov	[outfile],eax
	ret
;---------------------------------------------------------------------
pack_calltrick_fail:
	xor	eax,eax
	mov	[ctn],0
	ret
;---------------------------------------------------------------------
preprocess_calltrick:
; input preprocessing
	xor	eax,eax
	mov	edi,ct1
	mov	ecx,256/4
	push	edi
	rep	stosd
	pop	edi
	mov	ecx,[insize]
	mov	esi,[infile]
	xchg	eax,edx
	mov	ebx,[inbuftmp]
;--------------------------------------
input_pre:
	lodsb
	sub	al,0E8h
	cmp	al,1
	ja	input_pre_cont

	cmp	ecx,5
	jb	input_pre_done

	lodsd
	add	eax,esi
	sub	eax,[infile]
	cmp	eax,[insize]
	jae	xxx

	cmp	eax,1000000h
	jae	xxx

	sub	ecx,4
; bswap is not supported on i386
	xchg	al,ah
	ror	eax,16
	xchg	al,ah
	mov	[esi-4],eax
	inc	edx
	mov	[ebx],esi
	add	ebx,4
	jmp	input_pre_cont
;---------------------------------------------------------------------
xxx:
	sub	esi,4
	movzx	eax,byte [esi]
	mov	[eax+edi],byte 1
;--------------------------------------
input_pre_cont:
	loop	input_pre
;--------------------------------------
input_pre_done:
	mov	[ctn],edx
	xor	eax,eax
	mov	ecx,256
	repnz	scasb
	jnz	pack_calltrick_fail

	not	cl
	mov	[cti],cl
@@:
	cmp	ebx,[inbuftmp]
	jz	@f

	sub	ebx,4
	mov	eax,[ebx]
	mov	[eax-4],cl
	jmp	@b
;---------------------------------------------------------------------
@@:
	mov	al,1
	ret
;---------------------------------------------------------------------
pack_lzma:
	mov	eax,[outfile]
	add	eax,11
	push	[workmem]	;workmem
	push    [insize]	;length
	push	eax		;destination
	push	[infile]	;source
	call	lzma_compress
	mov	ecx,[outfile]
	mov	edx,[ecx+12]
	xchg	dl,dh
	ror	edx,16
	xchg	dl,dh
	mov     [ecx+12],edx
	dec     eax
	ret
;---------------------------------------------------------------------
preprocess_calltrick2:
; restore input
	mov	esi,[infile]
	mov	ecx,[ctn]
	jecxz	pc2l2
;--------------------------------------
pc2l1:
	lodsb
	sub	al,0E8h
	cmp	al,1
	ja	pc2l1

	mov	al,[cti]
	cmp	[esi],al
	jnz	pc2l1

	lodsd
	shr	ax,8
	ror	eax,16
	xchg	al,ah
	sub	eax,esi
	add	eax,[infile]
	mov	[esi-4],eax
	loop	pc2l1
;--------------------------------------
pc2l2:
; input preprocessing
	mov	edi,ct1
	xor	eax,eax
	push	edi
	mov	ecx,256/4
	rep	stosd
	pop	edi
	mov	ecx,[insize]
	mov	esi,[infile]
	mov	ebx,[inbuftmp]
	xchg	eax,edx
;--------------------------------------
input_pre2:
	lodsb
;--------------------------------------
@@:
	cmp	al,0Fh
	jnz	ip1

	dec	ecx
	jz	input_pre_done2

	lodsb
	cmp	al,80h
	jb	@b

	cmp	al,90h
	jb	@f
;--------------------------------------
ip1:
	sub	al,0E8h
	cmp	al,1
	ja	input_pre_cont2
;--------------------------------------
@@:
	cmp	ecx,5
	jb	input_pre_done2

	lodsd
	add	eax,esi
	sub	eax,[infile]
	cmp	eax,[insize]
	jae	xxx2

	cmp	eax,1000000h
	jae	xxx2

	sub	ecx,4
	xchg	al,ah
	rol	eax,16
	xchg	al,ah
	mov	[esi-4],eax
	inc	edx
	mov	[ebx],esi
	add	ebx,4
	jmp	input_pre_cont2
;---------------------------------------------------------------------
xxx2:	sub	esi,4
	movzx	eax,byte [esi]
	mov	[eax+edi],byte 1
;--------------------------------------
input_pre_cont2:
	loop	input_pre2
;--------------------------------------
input_pre_done2:
	mov	[ctn],edx
	xor	eax,eax
	mov	ecx,256
	repnz	scasb
	jnz	pack_calltrick_fail

	not	cl
	mov	[cti],cl
;--------------------------------------
@@:
	cmp	ebx,[inbuftmp]
	jz	@f

	sub	ebx,4
	mov	eax,[ebx]
	mov	[eax-4],cl
	jmp	@b
;---------------------------------------------------------------------
@@:
	mov	al,1
	ret
;*********************************************************************