
 virtual at 0
_FILE:
.pos		dq	?
.bufpos 	dq	?
.bufsize	dd	?
.mode		dd	?
.hPlugin	dd	?
.hFile		dd	?
.fileinfo:
.fimode 	dd	?
.fioffset	dq	?
.fisize 	dd	?
.fibuf		dd	?
.finame 	rb	1024
.attr		rb	40
align 512
.buf		rb	2048
.size = $
end virtual

O_READ = 1	; allows read from file
O_WRITE = 2	; allows write to file
O_CREATE = 4	; if file does not exist and this flag is set, create file;
		; if file does not exist and this flag is not set, fail
O_TRUNCATE = 8	; truncate file if it exists
O_SEQUENTIAL_ONLY = 10h ; there will be no 'seek'/'setpos' calls


; void __stdcall close(HANDLE hFile);
close:
	pushad
	mov	ecx, [esp+24h]
	mov	eax, [ecx+_FILE.hPlugin]
	test	eax, eax
	jz	@f
	push	ecx
	push	[ecx+_FILE.hFile]
	call	[aClose]
	pop	ecx
@@:
	call	pgfree
	popad
	ret	4

; unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
read:
	xor	eax, eax
	pushad
	mov	ecx, [esp+36]
	test	[ecx+_FILE.mode], O_READ
	jnz	@f
.ret:
	popad
	ret	12
@@:
	cmp	dword [esp+44], eax
	jz	.ret
	mov	[ecx+_FILE.fimode], eax
	mov	ebx, [ecx+_FILE.bufsize]
	mov	eax, dword [ecx+_FILE.pos]
	and	eax, 2047
	sub	ebx, eax
	jbe	.nobuf0
	cmp	ebx, [esp+44]
	jbe	@f
	mov	ebx, [esp+44]
@@:
	push	ecx
	lea	esi, [ecx+eax+_FILE.buf]
	mov	ecx, ebx
	mov	edi, [esp+44]
	rep	movsb
	pop	ecx
	mov	[esp+40], edi
	add	[esp+28], ebx
	add	dword [ecx+_FILE.pos], ebx
	adc	dword [ecx+_FILE.pos+4], 0
	test	dword [ecx+_FILE.pos], 2047
	jnz	@f
	and	[ecx+_FILE.bufsize], 0
@@:
	sub	[esp+44], ebx
	jz	.ret
.nobuf0:
	test	dword [ecx+_FILE.pos], 2047
	jz	.aligned
	cmp	dword [ecx+_FILE.bufsize], 0
	jnz	.ret
	lea	ebx, [ecx+_FILE.fileinfo]
	mov	dword [ebx+12], 2048
	lea	eax, [ecx+_FILE.buf]
	mov	dword [ebx+16], eax
	mov	eax, dword [ecx+_FILE.fioffset]
	mov	dword [ecx+_FILE.bufpos], eax
	mov	eax, dword [ecx+_FILE.fioffset+4]
	mov	dword [ecx+_FILE.bufpos+4], eax
	call	.doread
	test	eax, eax
	jnz	.ret
	mov	[ecx+_FILE.bufsize], ebx
	mov	eax, dword [ecx+_FILE.pos]
	and	eax, 2047
	sub	ebx, eax
	jbe	.ret
	cmp	ebx, [esp+44]
	jbe	@f
	mov	ebx, [esp+44]
@@:
	push	ecx
	lea	esi, [ecx+eax+_FILE.buf]
	mov	ecx, ebx
	mov	edi, [esp+44]
	rep	movsb
	pop	ecx
	add	dword [ecx+_FILE.pos], ebx
	adc	dword [ecx+_FILE.pos+4], 0
	mov	[esp+40], edi
	add	[esp+28], ebx
	sub	[esp+44], ebx
	jz	.ret
	test	dword [ecx+_FILE.pos], 2047
	jnz	.ret
.aligned:
	lea	ebx, [ecx+_FILE.fileinfo]
	mov	eax, [esp+44]
	and	eax, not 2047
	jz	.finish
	and	[ecx+_FILE.bufsize], 0
	mov	[ebx+12], eax
	mov	eax, [esp+40]
	mov	[ebx+16], eax
	call	.doread
	test	eax, eax
	jnz	.ret
	add	dword [ecx+_FILE.pos], ebx
	adc	dword [ecx+_FILE.pos+4], 0
	add	[esp+28], ebx
	add	[esp+40], ebx
	sub	[esp+44], ebx
	jz	.ret
	cmp	ebx, [ecx+_FILE.fisize]
	jb	.ret
.finish:
	lea	ebx, [ecx+_FILE.fileinfo]
	mov	dword [ebx+12], 2048
	lea	eax, [ecx+_FILE.buf]
	mov	[ebx+16], eax
	and	[ecx+_FILE.bufsize], 0
	mov	eax, dword [ecx+_FILE.fioffset]
	mov	dword [ecx+_FILE.bufpos], eax
	mov	eax, dword [ecx+_FILE.fioffset+4]
	mov	dword [ecx+_FILE.bufpos+4], eax
	call	.doread
	test	eax, eax
	jnz	.ret
	mov	[ecx+_FILE.bufsize], ebx
	cmp	ebx, [esp+44]
	jb	@f
	mov	ebx, [esp+44]
@@:
	add	[esp+28], ebx
	add	dword [ecx+_FILE.pos], ebx
	adc	dword [ecx+_FILE.pos+4], 0
	lea	esi, [ecx+_FILE.buf]
	mov	edi, [esp+40]
	mov	ecx, ebx
	rep	movsb
	popad
	ret	12
.doread:
	mov	eax, [ecx+_FILE.hPlugin]
	test	eax, eax
	jz	.native
	push	ecx
	push	[ecx+_FILE.fisize]
	push	[ecx+_FILE.fibuf]
	push	[ecx+_FILE.hFile]
	call	[eax+aRead]
	pop	ecx
	cmp	eax, -1
	jz	@f
	mov	ebx, eax
	xor	eax, eax
	jmp	.addpos
@@:
	ret
.native:
	push	70
	pop	eax
	int	0x40
	test	eax, eax
	jz	.addpos
	cmp	eax, 6
	jnz	@b
	xor	eax, eax
.addpos:
	add	dword [ecx+_FILE.fioffset], ebx
	adc	dword [ecx+_FILE.fioffset+4], 0
	ret

; void __stdcall seek(HANDLE hFile, int method, __int64 newpos);
seek:
	pushad
	mov	ecx, [esp+36]
	mov	eax, [esp+44]
	mov	edx, [esp+48]
	cmp	dword [esp+40], 1
	jb	.set
	ja	.end
	add	eax, dword [ecx+_FILE.pos]
	adc	edx, dword [ecx+_FILE.pos+4]
	jmp	.set
.end:
	add	eax, dword [ecx+_FILE.attr+32]
	adc	edx, dword [ecx+_FILE.attr+36]
.set:
	mov	dword [ecx+_FILE.pos], eax
	mov	dword [ecx+_FILE.pos+4], edx
	and	eax, not 2047
	cmp	eax, dword [ecx+_FILE.bufpos]
	jnz	@f
	cmp	edx, dword [ecx+_FILE.bufpos+4]
	jz	.bufposok
@@:
	and	[ecx+_FILE.bufsize], 0
	mov	dword [ecx+_FILE.bufpos], eax
	mov	dword [ecx+_FILE.bufpos+4], edx
.bufposok:
	cmp	[ecx+_FILE.bufsize], 0
	jnz	.ret
	cmp	eax, dword [ecx+_FILE.fioffset]
	jnz	@f
	cmp	edx, dword [ecx+_FILE.fioffset+4]
	jz	.ret
@@:
	mov	dword [ecx+_FILE.fioffset], eax
	mov	dword [ecx+_FILE.fioffset+4], edx
	mov	eax, [ecx+_FILE.hPlugin]
	test	eax, eax
	jz	@f
	push	dword [ecx+_FILE.fioffset+4]
	push	dword [ecx+_FILE.fioffset]
	push	[ecx+_FILE.hFile]
	call	[eax+aSetpos]
@@:
.ret:
	popad
	ret	16


; __int64 __stdcall tell(HANDLE hFile);
tell:
	mov	eax, [esp+4]
	mov	edx, dword [eax+_FILE.pos+4]
	mov	eax, dword [eax+_FILE.pos]
	ret	4

; HANDLE __stdcall open(const char* name, int mode);
; Opens physical file
open:
	pushad
	mov	ecx, _FILE.size
	call	xpgalloc
	test	eax, eax
	jz	.ret0z
	mov	[esp+28], eax
	mov	ecx, eax
	mov	esi, [esp+36]
	lea	edi, [eax+_FILE.finame]
	lea	edx, [eax+_FILE.finame+1024]
@@:
	lodsb
	stosb
	test	al, al
	jz	@f
	cmp	edi, edx
	jb	@b
.ret0:
	call	pgfree
.ret0z:
	popad
	xor	eax, eax
	ret	8
@@:
	mov	eax, [esp+40]
	mov	[ecx+_FILE.mode], eax
.getattr:
	lea	edi, [ecx+_FILE.fileinfo]
	mov	ebx, edi
	push	5
	pop	eax
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd
	lea	eax, [ecx+_FILE.attr]
	stosd
	push	70
	pop	eax
	int	0x40
	test	eax, eax
	jz	.found
	cmp	eax, 5
	jnz	.ret0
; file does not exist
	test	[ecx+_FILE.mode], O_CREATE
	jz	.ret0
.truncate:
	lea	ebx, [ecx+_FILE.fileinfo]
	mov	byte [ebx], 2
	push	70
	pop	eax
	int	0x40
	test	eax, eax
	jz	.getattr
	jmp	.ret0
.found:
	test	[ecx+_FILE.mode], O_TRUNCATE
	jz	@f
	cmp	dword [ecx+_FILE.attr+36], eax
	jnz	.truncate
	cmp	dword [ecx+_FILE.attr+32], eax
	jnz	.truncate
@@:
	mov	dword [ecx+_FILE.pos], eax
	mov	dword [ecx+_FILE.pos+4], eax
	mov	dword [ecx+_FILE.bufpos], eax
	mov	dword [ecx+_FILE.bufpos+4], eax
	mov	[ecx+_FILE.bufsize], eax
	mov	[ecx+_FILE.hPlugin], eax
	mov	[ecx+_FILE.hFile], eax
	mov	dword [ecx+_FILE.fioffset], eax
	mov	dword [ecx+_FILE.fioffset+4], eax
	mov	[esp+28], ecx
	popad
	ret	8

; HANDLE __stdcall open2(int plugin_id, HANDLE plugin_instance, const char* name, int mode);
; Opens file on plugin panel
open2:
	cmp	dword [esp+4], 0
	jnz	.plugin
	pop	eax
	add	esp, 8
	push	eax
	jmp	open
.plugin:
	pushad
	mov	ecx, _FILE.size
	call	xpgalloc
	test	eax, eax
	jz	.ret0z
	mov	[esp+28], eax
	mov	ecx, eax
	mov	esi, [esp+44]
	lea	edi, [eax+_FILE.finame]
	lea	edx, [eax+_FILE.finame+1024]
@@:
	lodsb
	stosb
	test	al, al
	jz	@f
	cmp	edi, edx
	jb	@b
.ret0:
	call	pgfree
.ret0z:
	popad
	xor	eax, eax
	ret	8
@@:
	mov	edx, [esp+36]
	mov	[ecx+_FILE.hPlugin], edx
	mov	ebx, [esp+40]
	mov	eax, [esp+48]
	mov	[ecx+_FILE.mode], eax
	push	ebx ecx
	push	eax
	lea	eax, [ecx+_FILE.finame]
	push	eax
	push	ebx
	call	[edx+aOpen]
	pop	ecx ebx
	test	eax, eax
	jz	.ret0
	mov	[ecx+_FILE.hFile], eax
	mov	edx, [esp+36]
	push	ecx
	lea	edi, [ecx+_FILE.fileinfo]
	push	edi
	xor	eax, eax
	push	ecx
	push	10
	pop	ecx
	rep	stosd
	pop	ecx
	lea	eax, [ecx+_FILE.finame]
	push	eax
	push	ebx
	call	[edx+aGetattr]
	pop	ecx
	xor	eax, eax
	mov	dword [ecx+_FILE.pos], eax
	mov	dword [ecx+_FILE.pos+4], eax
	mov	dword [ecx+_FILE.bufpos], eax
	mov	dword [ecx+_FILE.bufpos+4], eax
	mov	[ecx+_FILE.bufsize], eax
	mov	dword [ecx+_FILE.fioffset], eax
	mov	dword [ecx+_FILE.fioffset+4], eax
	mov	[esp+28], ecx
	popad
	ret	16

; __int64 __stdcall filesize(HANDLE hFile);
filesize:
	mov	eax, [esp+4]
	mov	edx, dword [eax+_FILE.attr+36]
	mov	eax, dword [eax+_FILE.attr+32]
	ret	4


;
;makedir:
;; create directory with name from CopyDestEditBuf+12
;; destroys eax
;        push    ebx
;        push    70
;        pop     eax
;        mov     ebx, mkdirinfo
;        int     0x40
;        pop     ebx
;        test    eax, eax
;        jz      .ret
;        cmp     dword [esp+8], DeleteErrorBtn
;        jnz     @f
;        cmp     [copy_bSkipAll], 0
;        jz      @f
;        push    1
;        pop     eax
;        jmp     .ret
;@@:
;        push    dword CopyDestEditBuf+12
;        push    dword aCannotMakeFolder
;        call    get_error_msg
;        push    eax
;        mov     eax, esp
;        push    dword [eax+20]
;        push    dword [eax+16]
;        push    eax
;        push    3
;        call    SayErr
;        add     esp, 3*4
;        test    eax, eax
;        jz      makedir
;.ret:
;        ret     8
;



;######################################################################################################
;######################################################################################################
;######################################################################################################
;######################################################################################################


CHECK_FOR_LEAKS = 0
if CHECK_FOR_LEAKS
uglobal
allocatedregions rd 1024
endg
iglobal
numallocatedregions dd 0
endg
end if
pgalloc:
; in: ecx=size
; out: eax=pointer or NULL
	push	ebx
	push	68
	pop	eax
	push	12
	pop	ebx
	int	0x40
if CHECK_FOR_LEAKS
	test	eax, eax
	jz	.no
.b:
	mov	ebx, [numallocatedregions]
	cmp	ebx, 1024
	jb	@f
	int3
	jmp	$
@@:
	mov	[allocatedregions+ebx*4], eax
	inc	[numallocatedregions]
.no:
end if
	pop	ebx
	ret

pgfree:
; in: ecx=pointer
; destroys eax
if CHECK_FOR_LEAKS
	jecxz	.no
	mov	eax, [numallocatedregions]
@@:
	dec	eax
	js	.a
	cmp	[allocatedregions+eax*4], ecx
	jnz	@b
	jmp	@f
.a:
	int3
	jmp	$
@@:
	dec	[numallocatedregions]
@@:
	cmp	eax, [numallocatedregions]
	jae	@f
	push	[allocatedregions+eax*4+4]
	pop	[allocatedregions+eax*4]
	inc	eax
	jmp	@b
@@:
.no:
end if
	push	ebx
	push	68
	pop	eax
	push	13
	pop	ebx
	int	0x40
	pop	ebx
	ret




pgrealloc:
; in: ecx=size, edx=pointer
; out: eax=pointer
	push	ebx
	push	68
	pop	eax
	push	20
	pop	ebx
	int	0x40
if CHECK_FOR_LEAKS
	test	edx, edx
	jz	pgalloc.b
	test	eax, eax
	jz	.no
	cmp	eax, edx
	jz	.no
	xor	ebx, ebx
@@:
	cmp	ebx, [numallocatedregions]
	jae	.a
	cmp	[allocatedregions+ebx*4], edx
	jz	@f
	inc	ebx
	jmp	@b
@@:
	mov	[allocatedregions+ebx*4], eax
	jmp	.no
.a:
	int3
	jmp	$
.no:
end if
	pop	ebx
	ret

xpgalloc:
; in: ecx=size
; out: eax=pointer or NULL
	call	pgalloc
.common:
	test	eax, eax
	jnz	@f
	;call    SayNoMem

@@:
	ret

xpgrealloc:
; in: edx=pointer, ecx=new size
; out: eax=pointer or NULL
	call	pgrealloc
	jmp	xpgalloc.common

getfreemem:
; out: eax=size of free RAM in Kb
	push	ebx
	mcall	18,16
	pop	ebx
	ret



proc myAddFile name:dword, bdfe_info:dword, hFile:dword
locals
   beginFile dd 0
   pos	dd 0
endl

;dps 'file unpack '
;dpsP [name]
;dnl
	pusha
	mov	[pos],0

	push	ebp
	stdcall [aRead], [hFile], copy_buf, SIZE_COPY_BUF
	pop	ebp

	mov	[fsWrite.cmd],2
	mov	[fsWrite.pos],0
	mov	[fsWrite.size],eax
	mov	eax,[name]
	mov	[fsWrite.path],eax
	mcall	70,fsWrite

 .loop:
	mov	eax, [fsWrite.size]
	add	[pos], eax


	push	ebp
	stdcall [aRead], [hFile], copy_buf, SIZE_COPY_BUF
	pop	ebp

	cmp	eax,0
	jz	.end

	mov	[fsWrite.cmd],3
	mov	[fsWrite.size],eax
	mov	eax,[pos]
	mov	[fsWrite.pos],eax
	mcall	70,fsWrite
	jmp	.loop
   .end:


	popa
	mov	eax,1
	ret
endp


proc myAddDir name:dword, bdfe_info:dword
;	dps 'log adddir'
	mov	eax,[name]
;	dpsP eax
;	dnl

	pusha
	mov	eax,[name]
	mov	[fsNewDir.path],eax
	mcall	70,fsNewDir
   ;
   ;     mov     edi,[name]
   ;     xor     al,al
   ;     mov     ecx,256
   ;     repne scasb
   ;     mov     edx,256
   ;     sub     edx,ecx
   ;     mov     eax,[name]
   ;     mcall   30,1,eax
	popa
	mov	eax,1
	ret
endp


;копирует последнее имя в полном или относительном пути без расширения
proc cpLastName inp:dword, outp:dword
	push	edi esi
	mov	edi,[inp]
	xor	al,al
	mov	ecx,256 	;                V
	repne scasb		;/fol1/file1.zip

	mov	al,'/'
	mov	ecx,256
	std			;   V
	repne scasb		;/fol1/file1.zip
	cld

	mov	esi,edi
	add	esi,2
	mov	ecx,256 	;/fol1/file1.zip
	mov	edi,[outp]	;file1.zip
    @@: lodsb
	stosb
	test	al,al
	jz	@f
	loop	@b
    @@:

	mov	al,'.'
	mov	ecx,256
	std			;    V
	repne scasb		;file1.zip
	cld
	mov	[edi+1], byte 0

	pop	esi edi
	ret
endp


proc cpAllExcludeLastName inp:dword, outp:dword
	push	edi esi
	mov	edi,[inp]
	xor	al,al
	mov	ecx,256 	;                V
	repne scasb		;/fol1/file1.zip

	mov	al,'/'
	mov	ecx,256
	std			;   V
	repne scasb		;/fol1/file1.zip
	cld
	mov	edx,edi
	add	edx, 2

	mov	esi,[inp]
	mov	ecx,256 	;/fol1/file1.zip
	mov	edi,[outp]	;file1.zip
    @@: lodsb
	stosb
	test	al,al
	jz	@f
	cmp	esi,edx
	jae	@f
	loop	@b
    @@:
	xor	al, al
	stosb

	pop	esi edi
	ret
endp
