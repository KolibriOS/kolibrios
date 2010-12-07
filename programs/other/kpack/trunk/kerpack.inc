; do not touch "purge mov"!!!
purge mov ; for the correct patch of loader
; do not touch "purge mov"!!!
;*********************************************************************
;die_with_err:
;	pop	esi
;@@:
;	lodsb
;	test	al,al
;	jz	@f

;	mov	cl,al
;	mcall	63,1
;	jmp	@b
;*********************************************************************
;@@:
;	mcall	63,,13
;	mcall	,,10
;	mcall	-1
;*********************************************************************
kerpack:
	call	clear_mess_and_displogo

	mov	ecx,200*1024*3+6A8000h
	mcall	68,12
	mov	[infile],eax
	mov	[infile3],eax
	add	eax,200*1024
	mov	[inbuftmp],eax
	add	eax,200*1024
	mov	[outfile],eax
	add	eax,200*1024
	mov	[workmem],eax
	
	
	mov	esi,inname
	call	get_full_name
	
	mov	[insize3],200*1024 ; max size of input file
	
	mcall	70,fn70_read
	cmp	eax,6
	jz	read_ok
;--------------------------------------
read_err:
;	call	die_with_err
;	db	'KerPack: cannot load kernel.mnt',0
	mov	esi,errload_str
	push	errload_len
	pop	ecx
	call	write_string
	jmp	read_ok.exit
;*********************************************************************
read_ok:
	mov	[insize3],ebx

	mov	edi,[infile]
	add	edi,[insize3]
	cmp	dword [edi-8],dword 'KERN'
	jnz	.lzma_set_dict_size

	cmp	dword [edi-4],dword 'PACK'
	jnz	.lzma_set_dict_size
;--------------------------------------
	mov	esi,already_str
	push	already_len
	pop	ecx
	call	write_string
	jmp	read_ok.exit
;*********************************************************************	
.not_kernel:
	mov	esi,notkernel_str
	push	notkernel_len
	pop	ecx
	call	write_string
	jmp	read_ok.exit	
;*********************************************************************	
.lzma_set_dict_size:
	push	18
	call	lzma_set_dict_size
; find jump to 32-bit code
;	mov	edi,infile - 1
	mov	edi,[infile]
	mov	eax,edi
	add	eax,[insize3]
	dec	edi
;--------------------------------------
@@:
	cmp	eax,edi
	je	.not_kernel
	
	inc	edi
	cmp	dword [edi],0xE88EE08E	 ; mov fs,ax/mov gs,ax
	jnz	@b

	cmp	dword [edi+4],0x00BCD08E	; mov ss,ax/mov esp,00xxxxxx
	jnz	@b

	add	edi,11
	mov	[inptr],edi
	sub	edi,[infile]	; infile
	mov	[indelta],edi
	lea	eax,[ebx+0x10000]
	mov	dword [loader_patch3+2],eax
	sub	ebx,edi
	mov	[insize1],ebx
	call	preprocess_calltrick3

	mov	al,[cti]
	mov	[loader_patch5-1],al
	mov	eax,[ctn]
	mov	[loader_patch4+1],eax
	mov	eax,[inptr]
;	add	eax, outfile - infile + loader_size - 5
	add	eax,[outfile]
	sub	eax,[infile]
	add	eax,loader_size - 5
	
	push	eax
	call	tell_compress_mess
	pop	eax
	
	push	[workmem]	;workmem
	push	[insize1]
	push	eax
	push	[inptr]
	call	lzma_compress

	add	eax, loader_size-5
	mov	[loader_patch1+6],eax
	add	eax,[indelta]
	mov	[outsize3],eax
	mov	eax,[indelta]
;	mov	ecx,dword [eax + outfile + loader_size - 4]
	mov	ecx,eax
	add	ecx,[outfile]
	mov	ecx,[ecx + loader_size - 4]
	
	bswap	ecx
	mov	[loader_patch2+4],ecx
	add	eax, 0x10000
	mov	[loader_patch1+1],eax
	mov	esi,[infile]	;infile
	mov	edi,[outfile]	;outfile
	mov	ecx,[indelta]
	rep	movsb

	mov	esi,loader_start
	mov	ecx,loader_size
	rep	movsb

	mov	eax,[outfile]
	add	eax,[outsize3]
	mov	[eax],dword 'KERN'
	mov	[eax+4],dword 'PACK'
	add	dword [outsize3],8
	
	mov	eax,[outsize3]
	mov	ecx,100
	mul	ecx
	div	[insize3]
	aam
	xchg	al,ah
	add	ax,'00'
	mov	[ratio],ax
	mov	esi,done_str
	push	done_len
	pop	ecx
	call	write_string
	
	mov	eax,[outfile]
	mov	[outfile3],eax

	mov	esi,outname
	call	get_full_name
	
	mcall	70,fn70_write
	test	eax,eax
	jz	.exit	;@f
;	call	die_with_err
;	db	'KerPack: cannot save kernel.mnt',0

	mov	esi,outfileerr_str
	push	outfileerr_len
	pop	ecx
	call	write_string
;*********************************************************************
.exit:
;	call	die_with_err
;	db	'KerPack: all is OK',0
	call	return_memory
	ret
;*********************************************************************
preprocess_calltrick3:
; input preprocessing
	mov	edi,ct1
	xor	eax,eax
	push	edi
	mov	ecx,256/4
	rep	stosd

	pop	edi
	mov	ecx,ebx
	mov	esi,[inptr]
	mov	ebx,[inbuftmp]	;inbuftmp
	xchg	eax,edx
;--------------------------------------
input_pre3:
	lodsb
;--------------------------------------
@@:
	cmp	al,0Fh
	jnz	ip3

	dec	ecx
	jz	input_pre_done3

	lodsb
	cmp	al,80h
	jb	@b

	cmp	al,90h
	jb	@f
;--------------------------------------
ip3:
	sub	al,0E8h
	cmp	al,1
	ja	input_pre_cont3
;--------------------------------------
@@:
	cmp	ecx,5
	jb	input_pre_done3

	lodsd
	add	eax,esi
	sub	eax,[inptr]
	cmp	eax,[insize1]
	jae	xxx3

	cmp	eax,1000000h
	jae	xxx3

	sub	ecx,4
	xchg	al,ah
	rol	eax,16
	xchg	al,ah
	mov	[esi-4],eax
	inc	edx
	mov	[ebx],esi
	add	ebx,4
	jmp	input_pre_cont3
;*********************************************************************
xxx3:
	sub	esi,4
	movzx	eax,byte [esi]
	mov	byte [eax+edi],1
;--------------------------------------
input_pre_cont3:
	loop	input_pre3
;--------------------------------------
input_pre_done3:
	mov	[ctn],edx
	xor	eax,eax
	mov	ecx,256
	repnz	scasb
	jnz	pack_calltrick_done

	not	cl
	mov	[cti],cl
;--------------------------------------
@@:
	cmp	ebx,[inbuftmp]	;inbuftmp
	jz	pack_calltrick_done

	sub	ebx,4
	mov	eax,[ebx]
	mov	[eax-4],cl
	jmp	@b
;*********************************************************************
pack_calltrick_done:
	ret
;*********************************************************************
include 'loader_lzma.inc'
;*********************************************************************