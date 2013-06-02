; Disassemled and corrected in 2010 specially for FASM
;            by Marat Zakiyanov aka Mario79, aka Mario
;            All sources only assembler is now
;            GCC using for linking
;---------------------------------------------------------------------
; Linux port by mike.dld in 2007
;---------------------------------------------------------------------
; kpack = Kolibri Packer
; Written by diamond in 2006 specially for KolibriOS

; Uses LZMA compression library by Igor Pavlov
; (for more information on LZMA and 7-Zip visit http://www.7-zip.org)
; (plain-C packer and ASM unpacker are ported by diamond)

	format ELF
	public _start

;*********************************************************************
section ".text" executable align 16

_start:
	mov	ecx,[esp+0]
	lea	ebx,[esp+4]
	call	combine_program_arguments

	xor	ebx, ebx
	push	eax	; reserve dword on the stack

	xor	eax,eax
	inc	eax

	xchg	eax, ebp
; parse command line
	mov	eax,program_arguments

	xchg	eax, esi
	call	skip_spaces
	call	get_file_name
	test	al, al
	jz	short usage

	call	get_file_name

	mov	[infilename], edi
	test	al, al
	jnz	short two_files

	mov	[outfilename], edi
	jmp	short cont
;*********************************************************************
write_string:
	push	eax	; reserve dword on the stack
	mov	eax, esp
	push	ebx
	push	eax
	push	dword [eax+12]
	push	dword [eax+8]
	push	ebp
	call	WriteConsoleA
	pop	eax
	ret	8
;*********************************************************************
write_exit:
	call	write_string
;---------------------------------------------------------------------
doexit:
	push	ebx
	call	ExitProcess
;---------------------------------------------------------------------
usage:
        cmp     [bNoLogo], 0
        jnz     doexit

	push	info_len + usage_len
	push	info_str
	jmp	write_exit
;*********************************************************************
two_files:
	call	get_file_name
	mov	[outfilename], edi
	test	al, al
	jnz	short usage
;---------------------------------------------------------------------
cont:
        cmp     [bNoLogo], 0
        jnz     @f

	push	info_len
	push	info_str
	call	write_string
;---------------------------------------------------------------------
@@:
; Input file
	push	ebx
	push	ebx
	push	3	; OPEN_EXISTING
	push	ebx
	push	1	; FILE_SHARE_READ
	push	80000000h	; GENERIC_READ
	push	[infilename]
	call	CreateFileA
	inc	eax
	jnz	short inopened
;---------------------------------------------------------------------
infileerr:
	push	errload_len
	push	errload_str
	jmp	write_exit
;*********************************************************************
inopened:
	dec	eax
	xchg	eax, esi
	push	ebx
	push	esi
	call	GetFileSize
	inc	eax
	jz	short infileerr

	dec	eax
	jz	short infileerr

	mov	[insize], eax
	push	eax
	push	4	; PAGE_READWRITE
	push	1000h	; MEM_COMMIT
	push	eax
	push	ebx
	call	VirtualAlloc
	test	eax, eax
	jz	nomem

	mov	[infile], eax
	pop	edx
	mov	ecx, esp
	push	ebx
	push	ecx
	push	edx
	push	eax
	push	esi
	call	ReadFile
	test	eax, eax
	jz	short infileerr

	push	esi
	call	CloseHandle

	mov	eax, [insize]
	shr	eax, 3
	add	eax, [insize]
	add	eax, 400h	; should be enough for header
	mov	esi, eax
	add	eax, eax
	push	4
	push	1000h
	push	eax
	push	ebx
	call	VirtualAlloc
	test	eax, eax
	jnz	short outmemok
;---------------------------------------------------------------------
nomem:
	push	nomem_len
	push	nomem_str
	jmp	write_exit
;*********************************************************************
outmemok:
	mov	[outfile], eax
	mov	[outfile1], eax
	mov	[outfilebest], eax
	add	eax, esi
	mov	[outfile2], eax
	sub	eax, esi
	mov	dword [eax], 'KPCK'
	mov	ecx, [insize]
	mov     dword [eax+4], ecx
	dec	ecx
	bsr	eax, ecx
	inc	eax
	cmp	eax, 28
	jb	short @f

	mov	eax, 28
;---------------------------------------------------------------------
@@:
	push	eax
	push	eax
	call	lzma_set_dict_size

	pop	ecx
	mov	eax, 1
	shl	eax, cl
	mov	[lzma_dictsize], eax
	imul	eax, 19
	shr	eax, 1
	add	eax, 509000h
	push	4
	push	1000h
	push	eax
	push	ebx
	call	VirtualAlloc
	test	eax, eax
	jz	nomem

	mov	[workmem], eax
	push	compressing_len
	push	compressing_str
	call	write_string

	mov	eax, [outfile2]
	mov	[outfile], eax
	xchg	eax, edi
	mov	esi, [outfile1]
	movsd
	movsd
	call	pack_lzma

	mov	[outsize], eax
	mov	eax, [outfile]
	mov	[outfilebest], eax
	mov	[method], use_lzma
	call	preprocess_calltrick
	test	eax, eax
	jz	short noct1

	call	set_outfile
	call	pack_lzma

	add     eax, 5
	cmp	eax, [outsize]
	jae	short @f
	mov	[outsize], eax
	mov	eax, [outfile]
	mov	[outfilebest], eax
	mov	[method], use_lzma or use_calltrick1
;---------------------------------------------------------------------
@@:
noct1:
	call	set_outfile

	push	[ctn]
	mov	al, [cti]
	push	eax
	call	preprocess_calltrick2
	test	eax, eax
	jz	noct2

	call	set_outfile
	call	pack_lzma

	add     eax, 5
	cmp	eax, [outsize]
	jae	short @f

	mov	[outsize], eax
	mov	eax, [outfile]
	mov	[outfilebest], eax
	mov	[method], use_lzma or use_calltrick2
	pop	ecx
	pop	ecx
	push	[ctn]
	mov	al, [cti]
	push	eax
;---------------------------------------------------------------------
@@:
noct2:
	pop	eax
	mov	[cti], al
	pop	[ctn]
	add     [outsize], 12
	mov     eax, [outsize]
	cmp     eax, [insize]
	jb      short packed_ok

	push	too_big_len
	push	too_big_str
	jmp	write_exit
;*********************************************************************
packed_ok:
	push	8000h	; MEM_RELEASE
	push	ebx
	push	[workmem]
	call	VirtualFree
; set header
        movzx   eax, [method]
	mov	edi, [outfilebest]
	mov     [edi+8], eax
	test    al, use_calltrick1 or use_calltrick2
	jz      short @f

	mov     ecx, [outsize]
	add     ecx, edi
	mov     eax, [ctn]
	mov     [ecx-5], eax
	mov     al, [cti]
	mov     [ecx-1], al
@@:
	mov     eax, [outsize]
	mov	ecx, 100
	mul	ecx
	div	[insize]
	aam
	xchg	al, ah
	add	ax, '00'
	mov	[ratio], ax
	push	done_len
	cmp     [bNoLogo], 0
	jz      @f

	sub     dword [esp], 2
;---------------------------------------------------------------------
@@:
	push	done_str
	call	write_string
; Output file
	push	ebx
	push	80h	; FILE_ATTRIBUTE_NORMAL
	push	2	; CREATE_ALWAYS
	push	ebx
	push	ebx
	push	40000000h	; GENERIC_WRITE
	push	[outfilename]
	call	CreateFileA
	inc	eax
	jnz	short @f
;---------------------------------------------------------------------
outerr:
	push	outfileerr_len
	push	outfileerr_str
	jmp	write_exit
;*********************************************************************
@@:
	dec	eax
	xchg	eax, esi
	mov	eax, esp
	push	ebx
	push	eax
	push	[outsize]
	push	edi
	push	esi
	call	WriteFile
	test	eax, eax
	jz	short outerr

	push	esi
	call	CloseHandle

	push	ebx
	call	ExitProcess
;---------------------------------------------------------------------
get_file_name:
	mov	edi, esi
	lodsb
	cmp	al, 0
	jz	short _ret

	cmp	al, '"'
	setz	dl
	jz	short @f

	dec	esi
;---------------------------------------------------------------------
@@:
	mov	edi, esi
;---------------------------------------------------------------------
@@loop:
	lodsb
	cmp	al, 0
	jz	short _ret

	cmp	al, ' '
	ja	short @f

	test	dl, 1
	jz	short @@end
;---------------------------------------------------------------------
@@:
	cmp	al, '"'
	jnz	short @@loop

	test	dl, 1
	jz	short @@loop
;---------------------------------------------------------------------
@@end:
	mov	byte [esi-1], 0
;---------------------------------------------------------------------
skip_spaces:
	lodsb
	cmp	al, 0
	jz	short @f

	cmp	al, ' '
	jbe	short skip_spaces
;---------------------------------------------------------------------
@@:
	dec	esi
	cmp     dword [esi+0],'--no'
	jnz	@f

	cmp     dword [esi+4],'logo'
	jnz     @f

	mov     [bNoLogo], 1
	add     esi, 8
	jmp     skip_spaces
;*********************************************************************
@@:
        mov     al, [esi]
;---------------------------------------------------------------------
_ret:
	ret
;*********************************************************************
set_outfile:
	mov	eax,[outfilebest]
	xor	eax,[outfile1]
	xor	eax,[outfile2]
	mov	[outfile],eax
	ret
;*********************************************************************
pack_calltrick_fail:
	xor	eax,eax
	xor	ebx,ebx
	mov	[ctn],eax
	ret
;*********************************************************************
preprocess_calltrick:
; input preprocessing
	push	4	; PAGE_READWRITE
	push	1000h	; MEM_COMMIT
	push	[insize]
	push	ebx
	call	VirtualAlloc
	test	eax, eax
	jz	pack_calltrick_fail

	push	eax
	xor	eax, eax
	mov	edi, ct1
	mov	ecx, 256/4
	push	edi
	rep	stosd

	pop	edi
	mov	ecx, [insize]
	mov	esi, [infile]
	xchg	eax, edx
	pop	eax
	xchg	eax, ebx
	push	ebx
;---------------------------------------------------------------------
input_pre:
	lodsb
	sub	al, 0E8h
	cmp	al, 1
	ja	short input_pre_cont

	cmp	ecx, 5
	jb	short input_pre_done

	lodsd
	add	eax, esi
	sub	eax, [infile]
	cmp	eax, [insize]
	jae	short xxx

	cmp	eax, 1000000h
	jae	short xxx

	sub	ecx, 4
	bswap	eax
	mov	[esi-4], eax
	inc	edx
	mov	[ebx], esi
	add	ebx, 4
	jmp	short input_pre_cont
;*********************************************************************
xxx:
	sub	esi, 4
	movzx	eax, byte [esi]
	mov	byte [eax+edi], 1
;---------------------------------------------------------------------
input_pre_cont:
	loop	input_pre
;---------------------------------------------------------------------
input_pre_done:
	mov	[ctn], edx
	pop	edx
	xor	eax, eax
	mov	ecx, 256
	repnz	scasb
	jnz	pack_calltrick_fail
	not	cl

	mov	[cti], cl
;---------------------------------------------------------------------
@@:
	cmp	ebx, edx
	jz	@f

	sub	ebx, 4
	mov	eax, [ebx]
	mov	[eax-4], cl
	jmp	@b
;*********************************************************************
@@:
	xor	ebx, ebx
	push	8000h
	push	ebx
	push	edx
	call	VirtualFree
	ret
;*********************************************************************
pack_lzma:
	mov	eax, [outfile]
	add	eax, 11
	push	[workmem]
	push	[insize]
	push	eax
	push	[infile]
	call	lzma_compress

	mov	ecx, [outfile]
	mov     edx, [ecx+12]
	bswap	edx
	mov     [ecx+12], edx
	dec     eax
	ret
;*********************************************************************
preprocess_calltrick2:
; restore input
	mov	esi, [infile]
	mov	ecx, [ctn]
	jecxz	pc2l2
;---------------------------------------------------------------------
pc2l1:
	lodsb
	sub	al, 0E8h
	cmp	al, 1
	ja	short pc2l1

	mov	al, [cti]
	cmp	[esi], al
	jnz	short pc2l1

	lodsd
	mov	al, 0
	bswap	eax
	sub	eax, esi
	add	eax, [infile]
	mov	[esi-4], eax
	loop	pc2l1
;---------------------------------------------------------------------
pc2l2:
; input preprocessing
	push	4	; PAGE_READWRITE
	push	1000h	; MEM_COMMIT
	push	[insize]
	push	ebx
	call	VirtualAlloc
	test	eax, eax
	jz	pack_calltrick_fail

	mov	edi, ct1
	xchg	eax, ebx
	xor	eax, eax
	push	edi
	mov	ecx, 256/4
	rep	stosd

	pop	edi
	mov	ecx, [insize]
	mov	esi, [infile]
	xchg	eax, edx
	push	ebx
;---------------------------------------------------------------------
input_pre2:
	lodsb
;---------------------------------------------------------------------
@@:
	cmp	al, 0Fh
	jnz	short ip1

	dec	ecx
	jz	short input_pre_done2

	lodsb
	cmp	al, 80h
	jb	short @b

	cmp	al, 90h
	jb	short @f
;---------------------------------------------------------------------
ip1:
	sub	al, 0E8h
	cmp	al, 1
	ja	short input_pre_cont2
;---------------------------------------------------------------------
@@:
	cmp	ecx, 5
	jb	short input_pre_done2

	lodsd
	add	eax, esi
	sub	eax, [infile]
	cmp	eax, [insize]
	jae	short xxx2

	cmp	eax, 1000000h
	jae	short xxx2

	sub	ecx, 4
	bswap	eax
	mov	[esi-4], eax
	inc	edx
	mov	[ebx], esi
	add	ebx, 4
	jmp	short input_pre_cont2
;*********************************************************************
xxx2:
	sub	esi, 4
	movzx	eax, byte [esi]
	mov	byte [eax+edi], 1
;---------------------------------------------------------------------
input_pre_cont2:
	loop	input_pre2
;---------------------------------------------------------------------
input_pre_done2:
	mov	[ctn], edx
	pop	edx
	xor	eax, eax
	mov	ecx, 256
	repnz	scasb
	jnz	pack_calltrick_fail

	not	cl
	mov	[cti], cl
;---------------------------------------------------------------------
@@:
	cmp	ebx, edx
	jz	@f

	sub	ebx, 4
	mov	eax, [ebx]
	mov	[eax-4], cl
	jmp	@b
;*********************************************************************
@@:
	xor	ebx, ebx
	push	8000h
	push	ebx
	push	edx
	call	VirtualFree
	ret
;*********************************************************************
extrn exit
extrn putchar
extrn fopen
extrn fread
extrn fwrite
extrn fclose
extrn fseek
extrn ftell
extrn malloc
extrn free

open_mode	db	"rb",0
create_mode	db	"wb",0
;*********************************************************************
combine_program_arguments: ; ecx = argc, ebx = argv
	mov	edi,program_arguments
	cld
	mov	al,'"'
	stosb
	mov	esi,[ebx]
;---------------------------------------------------------------------
@@:
	lodsb
	or	al,al
	jz	@f
	stosb
	jmp	@b
;*********************************************************************
@@:
	add	ebx,4
	mov	al,'"'
	stosb
	dec	ecx
	jz	.no_args
;---------------------------------------------------------------------
.next_arg:
	dec	ecx
	js	.no_args

	mov	al,' '
	stosb
	mov	esi,[ebx]
;---------------------------------------------------------------------
@@:
	lodsb
	or	al,al
	jz	@f
	stosb
	jmp	@b
;*********************************************************************
@@:
	add	ebx,4
	jmp	.next_arg
;*********************************************************************
.no_args:
	mov	al,0
	stosb
	ret
;*********************************************************************
WriteConsoleA: ; handle, buf, buf_size, num_wrote, NULL
	push	ebx esi edi
	mov	esi,[esp+16+4]
	mov	ecx,[esp+16+8]
	cld
;---------------------------------------------------------------------
@@:
	push	ecx
	movzx	edx,byte[esi]
	push	edx
	call	putchar
	add	esp,4
	inc	esi
	pop	ecx
	loop	@b

	pop	edi esi ebx
	ret 	20
;*********************************************************************
CreateFileA: ; filename, access_mode, share_mode, security_attr, creation_disposition, flags, template
	push	ebx esi edi
	cmp	byte[esp+16+16],3
	push	open_mode
	je	@f

	mov	dword[esp],create_mode
;---------------------------------------------------------------------
@@:
	pushd	[esp+16+0+4]
	call	fopen
	add	esp,8
	or	eax,eax
	jnz	@f

	or	eax,-1
;---------------------------------------------------------------------
@@:
	pop	edi esi ebx
	ret 	28
;*********************************************************************
GetFileSize: ; handle, high_part
	push	ebx esi edi
	pushd	[esp+16+0]
	call	ftell

	mov	[esp],eax
	pushd	2 0 [esp+16+0+12] ; go to EOF
	call	fseek

	add	esp,12
	pushd	[esp+16+0+4]
	call	ftell

	add	esp,4
	xchg	eax,[esp]
	pushd	0 eax [esp+16+0+12] ; go to BOF
	call	fseek

	add	esp,12
	pop	eax
	pop	edi esi ebx
	ret 	8
;*********************************************************************
VirtualAlloc: ; address, size, alloc_type, mem_protection
	push	ebx esi edi
	mov	eax,[esp+16+4]
	add	eax,4095
	and	eax,not 4095
	push	eax
	call	malloc

	add	esp,4
	pop	edi esi ebx
	ret	16
;*********************************************************************
ReadFile: ; handle, buf, buf_size, num_read, overlapped
	push	ebx esi edi
	pushd	[esp+16+0] 1 [esp+16+8+8] [esp+16+4+12]
	call	fread

	add	esp,16
	mov	[esp+16+12],eax
	cmp	eax,1
	je	@f

	xor	eax,eax
;---------------------------------------------------------------------
@@:
	pop	edi esi ebx
	ret	20
;*********************************************************************
WriteFile: ; handle, buf, buf_size, num_wrote, overlapped
	push	ebx esi edi
	pushd	[esp+16+0] 1 [esp+16+8+8] [esp+16+4+12]
	call	fwrite

	add	esp,16
	mov	[esp+16+12],eax
	cmp	eax,1
	je	@f

	xor	eax,eax
;---------------------------------------------------------------------
@@:
	pop	edi esi ebx
	ret	20
;*********************************************************************
CloseHandle: ; handle
	push	ebx esi edi
	pushd	[esp+16+0]
	call	fclose
	add	esp,4
	pop	edi esi ebx
	ret	4
;*********************************************************************
VirtualFree: ; address, size, free_type
	push	ebx esi edi
	pushd	[esp+16+0]
	call	free
	add	esp,4
	pop	edi esi ebx
	ret	12
;*********************************************************************
ExitProcess:
	pushd	[esp+4]
	call	exit
	ret	4
;*********************************************************************
include 'lzma_compress.inc'
include 'lzma_set_dict_size.inc'
;*********************************************************************
section ".const" align 4
;---------------------------------------------------------------------
align 4
LiteralNextStates:
db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:
db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:
db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:
db 9,9,9,9,9,9,9,11,11,11,11,11
;---------------------------------------------------------------------
info_str	db	'KPack - Kolibri Packer, version 0.11',13,10
		db	'Uses LZMA v4.32 compression library',13,10,13,10
info_len	=	$ - info_str
usage_str	db	'Written by diamond in 2006, 2007 specially for KolibriOS',13,10
		db	'LZMA compression library is copyright (c) 1999-2005 by Igor Pavlov',13,10
		db	13,10
		db	'Usage: kpack [--nologo] <infile> [<outfile>]',13,10
usage_len	=	$ - usage_str
errload_str	db	'Cannot load input file',13,10
errload_len	=	$ - errload_str
outfileerr_str	db	'Cannot save output file',13,10
outfileerr_len	=	$ - outfileerr_str
nomem_str	db	'No memory',13,10
nomem_len	=	$ - nomem_str
too_big_str	db	'failed, output is greater than input.',13,10
too_big_len	=	$ - too_big_str
compressing_str	db	'Compressing ... '
compressing_len = $ - compressing_str
;*********************************************************************
section ".data" writeable align 4
bNoLogo         db      0
done_str	db	'OK! Compression ratio: '
ratio		dw	'00'
		db	'%',13,10,13,10
done_len	=	$ - done_str

use_lzma	=	1

use_no_calltrick =	0
use_calltrick1	=	40h
use_calltrick2	=	80h

method			db	1
;*********************************************************************
section ".bss" writeable align 4
infilename	dd	?
outfilename	dd	?
infile		dd	?
outfile1	dd	?
outfile2	dd	?
outfile		dd	?
outfilebest	dd	?
workmem		dd	?
insize		dd	?
outsize		dd	?
LastWriteTime	dq	?
LastAccessTime	dq	?
CreationTime	dq	?
lzma_dictsize	dd	?
ct1		db	256 dup (?)
ctn		dd	?
cti		db	?

program_arguments	db	512 dup (?)

include 'data.inc'
;*********************************************************************
