; kpack = Kolibri Packer
; Kolibri version
; Written by diamond in 2006, 2007 specially for KolibriOS
;
; Disassemled and corrected in 2010 specially for FASM
;            by Marat Zakiyanov aka Mario79, aka Mario
;
; Uses LZMA compression library by Igor Pavlov
; (for more information on LZMA and 7-Zip visit http://www.7-zip.org)
; (plain-C packer and ASM unpacker are ported by diamond)
;---------------------------------------------------------------------
use32
	org	0

	db 'MENUET01'
	dd 1
	dd START	;_start		;0x239F	;0x23A4
	dd IM_END	;bss_start	;0x32C9 i_end ;0x32C4
memf	dd 0x53000 	;I_END	;bss_end	;memory;
	dd stacktop	;bss_end	;0x59DC esp ;0x4ÑA0
	dd params	;params		;0x4000	;0x32C4
	dd 0		;cur_dir_path
;---------------------------------------------------------------------
START:
	call	clear_messages
; set default path = /RD/1/
	mov	esi,defpath
	mov	edi,path
	mov	[edi-4],dword 6
	movsw
	movsd
; get system window info
	mov	al,48
	push	3
	pop	ebx
	mov	ecx,color_table
	push	40
	pop	edx
	int	40h
	inc	ebx
	int	40h
	mov	[skinheight],eax
; check command line
	mov	esi,params
	mov	[esi+100h],byte 0
parse_opt:
	call	skip_spaces
	test	al,al
	jz	default
	mov	edi,inname
	call	copy_name
	test	al,al
	jz	outeqin
	mov	edi,outname
	call	copy_name
	test	al,al
	jnz	default
doit:
	call	draw_window
	call	pack
	jmp	waitevent
;---------------------------------------------------------------------
clear_messages:
	xor	eax,eax
	mov	ecx,80*20/4+1
	mov	edi,message_mem
	rep	stosd
	ret
;---------------------------------------------------------------------
exit:
	xor	eax,eax
	dec	eax
	int	40h
outeqin:
	mov	ecx,48/4+1
	mov	esi,inname-4
	mov	edi,outname-4
	rep	movsd
	jmp	doit
;---------------------------------------------------------------------
default:
	mov	[curedit],inname
	mov	ecx,[skinheight]
	add	ecx,5
	mov	[curedit_y],ecx
	mov	esi,definoutname
	mov	edi,esi
	xor	ecx,ecx
	xor	eax,eax
	dec	ecx
	repnz	scasb
	not	ecx
	dec	ecx
	mov	[innamelen],ecx
	push	ecx
	push	esi
	mov	edi,inname
	rep	movsb
	pop	esi
	pop	ecx
	mov	[outnamelen],ecx
	mov	edi,outname
	rep	movsb
;---------------------------------------------------------------------
dodraw:
	call	draw_window
waitevent:
	push	10
	pop	eax
	int	40h
	dec	eax
	jz	dodraw
	dec	eax
	jz	keypressed
	dec	eax
	jnz	waitevent
; button pressed
	mov	al,17
	int	40h
	xchg	al,ah
	cmp	al,7
	jz	but7
	dec	eax
	jz	exit
	dec	eax
	jnz	nopack
	call	pack
	jmp	waitevent
;---------------------------------------------------------------------
nopack:
	dec	eax
	jnz	nounpack
	call	unpack
	jmp	waitevent
;---------------------------------------------------------------------
but7:
	call	clear_messages
; display logo
	mov	esi,info_str
	push	info_len
	pop	ecx
	call	write_string
; display info
	mov	esi,usage_str
	mov	ecx,usage_len
	call	write_string
	jmp	waitevent
;---------------------------------------------------------------------
nounpack:
; this is infile/outfile/path button
	call	clear_edit_points
	mov	esi,inname
	mov	ecx,[skinheight]
	add	ecx,5
	dec	eax
	jz	edit
	mov	esi,outname
	add	ecx,0Ch
	dec	eax
	jz	edit
	mov	esi,path
	add	ecx,0Ch
edit:
	cmp	esi,[curedit]
	mov	[curedit],0
	jz	waitevent
	mov	[curedit],esi
	mov	[curedit_y],ecx
	mov	al,1
	mov	ebx,[esi-4]
	mov	edi,ebx
	imul	ebx,6
	add	ebx,42h
	add	ecx,4
	xor	edx,edx
@@:
	cmp	edi,48
	jz	waitevent
	int	40h
	add	ebx,6
	inc	edi
	jmp	@b
;---------------------------------------------------------------------
keypressed:
	mov	al,2
	int	40h
	xchg	al,ah
	mov	edi,[curedit]
	test	edi,edi
	jz	waitevent
	mov	ebx,[edi-4]
	cmp	al,8
	jz	backspace
	cmp	al,13
	jz	onenter
	cmp	al,20h
	jb	waitevent
	cmp	ebx,48
	jz	waitevent
	mov	[edi+ebx],al
	inc	ebx
	mov	[edi-4],ebx
; clear point and draw symbol
	lea	edi,[edi+ebx-1]
	imul	ebx,6
	add	ebx,40h-6
	shl	ebx,16
	mov	al,13
	mov	bl,6
	mov	ecx,[curedit_y]
	push	ecx
	shl	ecx,16
	mov	cl,9
	mov	edx,[color_table+20]
	int	40h
	pop	ecx
	mov	bx,cx
	mov	edx,edi
	push	1
	pop	esi
	mov	ecx,[color_table+32]
	mov	al,4
	int	40h
	jmp	waitevent
;---------------------------------------------------------------------
backspace:
	test	ebx,ebx
	jz	waitevent
	dec	ebx
	mov	[edi-4],ebx
; clear symbol and set point
	imul	ebx,6
	add	ebx,40h
	shl	ebx,16
	mov	al,13
	mov	bl,6
	mov	ecx,[curedit_y]
	push	ecx
	shl	ecx,16
	mov	cl,9
	mov	edx,[color_table+20]
	int	40h
	xor	edx,edx
	shr	ebx,16
	inc	ebx
	inc	ebx
	pop	ecx
	add	ecx,4
	mov	al,1
	int	40h
	jmp	waitevent
;---------------------------------------------------------------------
onenter:
	cmp	[curedit],inname
	jnz	@f
	push	2
	pop	eax
	jmp	nounpack
;---------------------------------------------------------------------
@@:
	cmp	[curedit],outname
	jnz	@f
	call	pack
	jmp	waitevent
;---------------------------------------------------------------------
@@:
	call	clear_edit_points
	jmp	waitevent
;---------------------------------------------------------------------
pack:
	call	clear_edit_points
	and	[curedit],0
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
	push	70
	pop	eax
	int	40h
	test	eax,eax
	jz	inopened
;---------------------------------------------------------------------
infileerr:
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
	mov	esi,[memf]
	mov	[infile],esi
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
	add	ecx,[memf]
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
	push	64
	pop	eax
	push	1
	pop	ebx
	int	40h
	test	eax,eax
	jz	mem_ok
; try to use smaller dictionary
meml0:
	cmp	edx,4
	jbe	memf1
	dec	edx
	xor	eax,eax
	inc	eax
	mov	ecx,edx
	shl	eax,cl
	imul	eax,19
	shr	eax,1
	add	eax,509000h
	pop	ecx
	push	ecx
	add	ecx,eax
	push	64
	pop	eax
	int	40h
	test	eax,eax
	jnz	meml0
; ok, say warning and continue
	mov	[lzma_dictsize],edx
	mov	esi,lzma_memsmall_str
	push	lzma_memsmall_len
	pop	ecx
	call	write_string
	jmp	mem_ok
;---------------------------------------------------------------------
memf1:
	mov	esi,nomem_str
	push	nomem_len
	pop	ecx
	jmp	write_string
;---------------------------------------------------------------------
mem_ok:
	mov	eax,[insize]
	mov	ebx,fn70block
	mov	[ebx],byte 0
	mov	[ebx+12],eax
	mov	esi,[infile]
	mov	[ebx+16],esi
	push	70
	pop	eax
	int	40h
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
lzma_setds:
	push	eax
	call	lzma_set_dict_size
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
	push	70
	pop	eax
	int	40h
	test	eax,eax
	jz	@f
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
	mov	al,70
	int	40h
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
input_pre_cont:
	loop	input_pre
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
input_pre2:
	lodsb
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
ip1:
	sub	al,0E8h
	cmp	al,1
	ja	input_pre_cont2
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
input_pre_cont2:
	loop	input_pre2
input_pre_done2:
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
unpack:
	call	clear_edit_points
	and	[curedit],0
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
	and	[ebx+12],dword 0
	mov	[ebx+16],dword file_attr
	push	70
	pop	eax
	int	40h
	test	eax,eax
	jnz	infileerr
	mov	eax,[insize]
	test	eax,eax
	jz      infileerr
	mov	ecx,[memf]
	mov	[infile],ecx
	add	ecx,eax
	mov	[outfile],ecx
	mov	[outfilebest],ecx
	push	64
	pop	eax
	push	1
	pop	ebx
	int	40h
	test	eax,eax
	jnz	memf1
	mov	ebx,fn70block
	mov	[ebx],byte 0
	mov	eax,[insize]
	mov	[ebx+12],eax
	mov	esi,[infile]
	mov	[ebx+16],esi
	push	70
	pop	eax
	int	40h
	test	eax,eax
	jnz	infileerr
	mov	eax,[infile]
	cmp	[eax],dword 'KPCK'	;'KCPK'
	jz	@f
unpack_err:
	mov	esi,notpacked_str
	push	notpacked_len
	pop	ecx
	jmp	write_string
;---------------------------------------------------------------------
@@:
	mov	ecx,[outfile]
	add	ecx,dword [eax+4]
	push	64
	pop	eax
	push	1
	pop	ebx
	int	40h
	test	eax,eax
	jnz	memf1
	mov	esi,[infile]
	mov	eax,[esi+8]
	push	eax
	and	al,0C0h
	cmp	al,0C0h
	pop	eax
	jz	unpack_err
	and	al,not 0C0h
	dec	eax
	jnz	unpack_err
	mov	eax,[esi+4]
	mov	[outsize],eax
	push	eax
	push	[outfile]
	add	esi,11
	push	esi
	mov	eax,[esi+1]
	xchg	al,ah
	ror	eax,16
	xchg	al,ah
	mov	[esi+1],eax
	call	lzma_decompress
	mov	esi,[infile]
	test	[esi+8],byte 80h
	jnz	uctr1
	test	[esi+8],byte 40h
	jz	udone
	add	esi,[insize]
	sub	esi,5
	lodsd
	mov	ecx,eax
	jecxz	udone
	mov	dl,[esi]
	mov	esi,[outfile]
uc1:
	lodsb
	sub	al,0E8h
	cmp	al,1
	ja	uc1
	cmp	[esi],dl
	jnz	uc1
	lodsd
	shr	ax,8
	ror	eax,16
	xchg	al,ah
	sub	eax,esi
	add	eax,[outfile]
	mov	[esi-4],eax
	loop	uc1
	jmp	udone
;---------------------------------------------------------------------
uctr1:
	add	esi,[insize]
	sub	esi,5
	lodsd
	mov	ecx,eax
	jecxz	udone
	mov	dl,[esi]
	mov	esi,[outfile]
uc2:
	lodsb
@@:
	cmp	al,15
	jnz	uf
	lodsb
	cmp	al,80h
	jb	@b
	cmp	al,90h
	jb	@f
uf:
	sub	al,0E8h
	cmp	al,1
	ja	uc2
@@:
	cmp	[esi],dl
	jnz	uc2
	lodsd
	shr	ax,8
	ror	eax,16
	xchg	al,ah
	sub	eax,esi
	add	eax,[outfile]
	mov	[esi-4],eax
	loop	uc2
udone:
	mov	esi,unpacked_ok
	push	unpacked_len
	pop	ecx
	call	write_string
	jmp	saveout
;---------------------------------------------------------------------
get_full_name:
	push	esi
	mov	esi,path
	mov	ecx,[esi-4]
	mov	edi,fullname
	rep	movsb
	mov	al,'/'
	cmp	[edi-1],al
	jz	@f
	stosb
@@:
	pop	esi
	cmp	[esi],al
	jnz	@f
	mov	edi,fullname
@@:
	mov	ecx,[esi-4]
	rep	movsb
	xor	eax,eax
	stosb
	ret
;---------------------------------------------------------------------
wsret:
	ret
write_string:
; in: esi=pointer, ecx=length
	mov	edx,[message_cur_pos]
x1:
	lea	edi,[message_mem+edx]
do_write_char:
	lodsb
	cmp	al,10
	jz	newline
	stosb
	inc	edx
	loop	do_write_char
	jmp	x2
;---------------------------------------------------------------------
newline:
	xor	eax,eax
	stosb
	xchg	eax,edx
	push	ecx
	push	eax
	mov	ecx,80
	div	ecx
	pop	eax
	xchg	eax,edx
	sub	edx,eax
	add	edx,ecx
	pop	ecx
	loop	x1
x2:
	mov	[message_cur_pos],edx
; update window
	push	13
	pop	eax
	mov	ebx,901A1h
	mov	ecx,[skinheight]
	shl	ecx,16
	add	ecx,3700DEh
	mov	edx,[color_table+20]
	int	40h
draw_messages:
	mov	ebx,[skinheight]
	add	ebx,3Ch+12*10000h
	mov	edi,message_mem
@@:
	push	edi
	xor	eax,eax
	push	80
	pop	ecx
	repnz	scasb
	sub	ecx,79
	neg	ecx
	mov	esi,ecx
	mov	al,4
	pop	edi
	mov	edx,edi
	mov	ecx,[color_table+32]
	int	40h
	add	ebx,10
	add	edi,80
	cmp	edi,message_cur_pos
	jb	@b
	ret
;---------------------------------------------------------------------
draw_window:
; start redraw
	push	12
	pop	eax
	xor	ebx,ebx
	inc	ebx
	int	40h
	mov	edi,[skinheight]
; define window
	xor	eax,eax
	mov	ebx,6401B3h
	mov	ecx,64011Eh
	add	ecx,edi
	mov	edx,[color_table+20]
	add	edx,13000000h
	push	edi
	mov	edi,caption_str
	int	40h
	pop	edi
; lines - horizontal
	mov	edx,[color_table+36]
	mov	ebx,80160h
	mov	ecx,edi
	shl	ecx,16
	or	ecx,edi
	add	ecx,20002h
	mov	al,38
	int	40h
	add	ecx,0C000Ch
	int	40h
	add	ecx,0C000Ch
	int	40h
	add	ecx,0C000Ch
	int	40h
; lines - vertical
	mov	ebx,80008h
	sub	ecx,240000h
	int	40h
	add	ebx,340034h
	int	40h
	add	ebx,1240124h
	int	40h
; draw frame for messages data
	push	ecx
	mov	ebx,801AAh
	add	ecx,340010h
	int	40h
	add	ecx,0E0h*10001h
	int	40h
	mov	ebx,80008h
	sub	cx,0E0h
	int	40h
	mov	ebx,1AA01AAh
	int	40h
	pop	ecx
; define compress button
	mov	al,8
	mov	cx,12h
	mov	ebx,1620048h
	push	2
	pop	edx
	mov	esi,[color_table+36]
	int	40h
; uncompress button
	add	ecx,120000h
	inc	edx
	int	40h
	add	ecx,-12h+0Ah+140000h
; question button
	push	esi
	mov	ebx,1A10009h
	mov	dl,7
	int	40h
	mov	al,4
	mov	edx,aQuestion
	push	1
	pop	esi
	shr	ecx,16
	lea	ebx,[ecx+1A40002h]
	mov	ecx,[color_table+28]
	int	40h
	mov	al,8
	pop	esi
; define settings buttons
	mov	ebx,90032h
	lea	ecx,[edi+2]
	shl	ecx,16
	mov	cx,0Bh
	push	4
	pop	edx
@@:
	int	40h
	add	ecx,0C0000h
	inc	edx
	cmp	edx,6
	jbe	@b
; text on settings buttons
	lea	ebx,[edi+5+0C0000h]
	mov	al,4
	mov	ecx,[color_table+28]
	push	buttons1names
	pop	edx
	push	8
	pop	esi
@@:
	int	40h
	add	edx,esi
	add	ebx,0Ch
	cmp	[edx-6],byte ' '
	jnz	@b
; text on compress and decompress buttons
	lea	ebx,[edi+8+1720000h]
	push	aCompress
	pop	edx
	or	ecx,80000000h
	int	40h
	lea	ebx,[edi+1Ah+16A0000h]
	push	aDecompress
	pop	edx
	int	40h
; infile, outfile, path strings
	mov	edx,inname
	lea	ebx,[edi+400005h]
editdraw:
	mov	esi,[edx-4]
	mov	al,4
	mov	ecx,[color_table+32]
	int	40h
	cmp	edx,[curedit]
	jnz	cont
	mov	al,1
	push	ebx
	push	edx
	movzx	ecx,bx
	shr	ebx,16
	lea	edx,[esi*2]
	lea	edx,[edx+edx*2]
	lea	ebx,[ebx+edx+2]
	add	ecx,4
	xor	edx,edx
@@:
	cmp	esi,48
	jz	@f
	int	40h
	add	ebx,6
	inc	esi
	jmp	@b
;---------------------------------------------------------------------
@@:
	pop	edx
	pop	ebx
cont:
	add	edx,52
	add	ebx,0Ch
	cmp	edx,path+52
	jb	editdraw
; draw messages
	call	draw_messages
; end redraw
	push	12
	pop	eax
	push	2
	pop	ebx
	int	40h
	ret
;---------------------------------------------------------------------
copy_name:
	lea	edx,[edi+48]
@@:
	lodsb
	cmp	al,' '
	jbe	copy_name_done
	stosb
	cmp	edi,edx
	jb	@b
@@:
	lodsb
	cmp	al,' '
	ja	@b
copy_name_done:
	dec	esi
	sub	edx,48
	sub	edi,edx
	mov	[edx-4],edi

skip_spaces:
	lodsb
	cmp	al,0
	jz	@f
	cmp	al,' '
	jbe	skip_spaces
@@:
	dec	esi
	ret
;---------------------------------------------------------------------
clear_edit_points:
; clear edit points (if is)
	mov	esi,[curedit]
	test	esi,esi
	jz	cleared_edit_points
	push	eax
	mov	al,13
	mov	ebx,[esi-4]
	imul	ebx,6
	mov	edi,ebx
	add	ebx,40h
	shl	ebx,16
	add	ebx,48*6
	sub	bx,di
	mov	ecx,[curedit_y]
	shl	ecx,16
	or	cx,9
	mov	edx,[color_table+20]
	int	40h
	pop	eax
cleared_edit_points:
	ret
;---------------------------------------------------------------------
;lzma_compress:
include 'lzma_compress.inc'
;---------------------------------------------------------------------
;lzma_set_dict_size:
include 'lzma_set_dict_size.inc'
;---------------------------------------------------------------------
;lzma_decompress:
include	'lzma_decompress.inc'
;---------------------------------------------------------------------
aQuestion	db '?'
caption_str	db 'KPack',0
buttons1names	db ' InFile:'
		db 'OutFile:'
		db '   Path:'
aCompress	db 'COMPRESS',0
aDecompress	db 'DECOMPRESS',0
definoutname	db 0
defpath		db '/RD/1/'
curedit		dd 0

info_str	db 'KPack - Kolibri Packer, version 0.13',10
		db 'Uses LZMA v4.32 compression library',10,10
info_len	= $ - info_str
usage_str	db 'Written by diamond in 2006, 2007, 2009 specially for KolibriOS',10
		db 'LZMA  is copyright (c) 1999-2005 by Igor Pavlov',10
		db 10
		db 'Command-line usage:',10
		db ' kpack infile [outfile]',10
		db 'If no output file is specified,',10
		db '    packed data will be written back to input file',10
		db 10
		db 'Window usage:',10
		db " enter input file name, output file name and press needed button",10
usage_len	= $ - usage_str
errload_str	db 'Cannot load input file',10
errload_len	= $ - errload_str
outfileerr_str	db 'Cannot save output file',10
outfileerr_len	= $ - outfileerr_str
nomem_str	db 'No memory',10
nomem_len	= $ - nomem_str
too_big_str	db 'failed, output is greater than input.',10
too_big_len	= $ - too_big_str
compressing_str	db 'Compressing ... '
compressing_len = $ - compressing_str
lzma_memsmall_str db	'Warning: not enough memory for default LZMA settings,',10
		db '         will use less dictionary size',10
lzma_memsmall_len = $ - lzma_memsmall_str
notpacked_str	db 'Input file is not packed with KPack!',10
notpacked_len	= $ - notpacked_str
unpacked_ok	db 'Unpacked successful',10
unpacked_len	= $ - unpacked_ok

done_str	db 'OK! Compression ratio: '
ratio		dw	'00'
		db '%',10
done_len	= $ - done_str
;---------------------------------------------------------------------
align 4
LiteralNextStates:	;0x30C: ;Binary tree
db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:	;0x318:
db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:	;0x324:
db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:	;0x330:
db 9,9,9,9,9,9,9,11,11,11,11,11
;0x33C:
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
;rb	0xD3C  ;unknown space area
params:
	rb 256
;---------------------------------------------------------------------	
color_table	rd 10
skinheight	rd 1

innamelen	rd 1
inname		rb 48
outnamelen	rd 1
outname		rb 48
pathlen		rd 1
path		rb 48
curedit_y	rd 1

message_mem	rb 80*20
message_cur_pos	rd 1

outsize		rd 1
infile		rd 1
outfile		rd 1
outfile1	rd 1
outfile2	rd 1
outfilebest	rd 1
inbuftmp	rd 1
workmem		rd 1
lzma_dictsize	rd 1
ct1		rb 256
ctn		rd 1
cti		rb 1
use_lzma	= 1

use_no_calltrick = 0
use_calltrick1	= 40h
use_calltrick2	= 80h

method		rb 1

;---------------------------------------------------------------------
align 4
fn70block:
fn70op		rd 1
fn70start	rd 1
fn70size	rd 1
fn70zero	rd 1
fn70dest	rd 1
fullname	rb 100

;---------------------------------------------------------------------
align 4
file_attr	rd 8
insize		rd 1       ; last qword in file_attr
		rd 1
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------