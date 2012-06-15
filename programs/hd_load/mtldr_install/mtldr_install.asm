	format PE GUI 4.0
section '.text' code readable executable
entry start
start:
	xor	ebx, ebx
	push	ofn
	call	[GetOpenFileNameA]
	test	eax, eax
	jnz	@f
	push	ebx
	call	[ExitProcess]
@@:
	call	[GetVersion]
	test	eax, eax
	sets	[b9x]
	js	install_9x
	mov	[img_name+2], bl
	push	ebx
	push	ebx
	push	3
	push	ebx
	push	3
	push	80000000h
	push	dn
	call	[CreateFileA]
	inc	eax
	jnz	@f
norights:
	push	10h
	push	ebx
	push	norightsmsg
mbx:
	push	ebx
	call	[MessageBoxA]
	push	ebx
	call	[ExitProcess]
@@:
	lea	esi, [eax-1]
	push	ebx
	push	tmp
	push	12
	push	sdn
	push	ebx
	push	ebx
	push	0x2D1080
	push	esi
	call	[DeviceIoControl]
	test	eax, eax
	jnz	@f
cnr:
	push	esi
	call	[CloseHandle]
	jmp	norights
@@:
	push	ebx
	push	tmp
	push	20h
	push	pi
	push	ebx
	push	ebx
	push	0x74004
	push	esi
	call	[DeviceIoControl]
	test	eax, eax
	jz	cnr
	push	esi
	call	[CloseHandle]
	cmp	[sdn], 7
	jz	@f
	push	10h
	push	0
	push	nohd
	jmp	mbx
@@:
	mov	al, byte [sdn+4]
	or	al, 80h
	mov	[mtldr_code+7], al
	mov	eax, [pi]
	mov	edx, [pi+4]
	shrd	eax, edx, 9
	shr	edx, 9
	jz	@f
m1e:	push	10h
	push	ebx
	push	m1
	jmp	mbx
@@:
install_cmn:
	mov	[mtldr_code+8], eax
	mov	esi, img_name
	mov	edi, img_real_name
	mov	byte [esi+2], '\'
	push	256
	push	edi
	push	esi
	call	[GetShortPathNameA]
	cmp	eax, 256
	jb	@f
	push	10h
	push	ebx
	push	ptl
	jmp	mbx
@@:
	test	eax, eax
	jnz	@f
	push	esi edi
	mov	ecx, 256/4
	rep	movsd
	pop	edi esi
@@:
	cmp	byte [edi], 0
	jz	lcd
	cmp	byte [edi], 'A'
	jb	lcc
	cmp	byte [edi], 'Z'
	ja	lcc
	add	byte [edi], 20h
lcc:
	inc	edi
	jmp	@b
lcd:
	mov	esi, img_real_name
	cmp	[b9x], 0
	jnz	@f
	cmp	byte [esi], 'c'
	jnz	notc
@@:
	push	256/4
	pop	ecx
	lea	edi, [esi+ecx*4]
	rep	movsd
	mov	edi, esi
	xor	eax, eax
	or	ecx, -1
	repnz	scasb
	dec	edi
	std
	mov	al, '\'
	repnz	scasb
	cld
	inc	edi
	inc	edi
	mov	eax, 'mtld'
	stosd
	mov	al, 'r'
	stosb
	jmp	cmn
notc:
	mov	dword [mtldr_name], 'C:\m'
	mov	dword [mtldr_name+4], 'tldr'
	mov	edi, mtldr_name+8
cmn:
	and	word [edi], 0
mf:
	push	mtldr_name
	call	[GetFileAttributesA]
	inc	eax
	jnz	@f
	call	[GetLastError]
	cmp	eax, 2
	jz	fo
@@:
	cmp	byte [edi], 0
	jnz	@f
	mov	byte [edi], '0'
	jmp	mf
@@:
	cmp	byte [edi], '9'
	jae	@f
mfi:
	inc	byte [edi]
	jmp	mf
@@:
	ja	@f
	mov	byte [edi], 'A'
	jmp	mf
@@:
	cmp	byte [edi], 'Z'
	jb	mfi
nomx:	push	10h
	push	ebx
	push	nom
	jmp	mbx
fo:
	cmp	[b9x], 0
	jnz	install_9x_2
	call	write_mtldr1
	push	ecx
	call	[GetVersion]
	pop	ecx
	cmp	al, 6
	jae	install_vista
	mov	al, 2
	mov	edi, tmp_data
	neg	ecx
	add	ecx, 2000h - mtldr_code_size
	push	ebx
	push	tmp
	push	ecx
	push	edi
	push	esi
	rep	stosb
	call	[WriteFile]
	push	esi
	call	[CloseHandle]
	push	bootini
	mov	edi, systitle+1
	mov	esi, ostitle
	mov	byte [edi-1], '"'
@@:
	lodsb
	test	al, al
	jz	@f
	stosb
	jmp	@b
@@:
	mov	word [edi], '"'
	push    bootini
	call    [GetFileAttributesA]
	push    eax
	and     al, not 1
	push    eax
	push    bootini
	call    [SetFileAttributesA]
	push    bootini
	push	systitle
	push	mtldr_name
	push	mtldr_name
	push	mtldr_name
	call	[CharToOemA]
	push	osstr
	call	[WritePrivateProfileStringA]
	xchg    eax, [esp]
	push    eax
	push    bootini
	call    [SetFileAttributesA]
	pop     eax
	test	eax, eax
	jnz	suci
; failed, delete written mtldr
	call	delete_mtldr
	push	10h
	push	ebx
	push	insterr
	jmp	mbx
suci:
	push	40h
	push	suct
	push	succ
	jmp	mbx

install_9x:
	mov	al, [img_name]
	or	al, 20h
	sub	al, 'a'-1
	mov	byte [regs], al
	push	ebx
	push	ebx
	push	3
	push	ebx
	push	3
	push	80000000h
	push	vwin32
	call	[CreateFileA]
	inc	eax
	jz	norights
	dec	eax
	xchg	eax, esi
	push	ebx
	push	tmp
	push	28
	push	regs
	push	28
	push	regs
	push	1
	push	esi
	call	[DeviceIoControl]
	push	eax
	push	esi
	call	[CloseHandle]
	pop	eax
	test	eax, eax
@@:	jz	norights
	mov	al, [diskinfobuf+3]
	cmp	al, 0xFF
	jz	@b
	cmp	al, 80h
	jb	norights
	mov	[mtldr_code+7], al
	cmp	dword [diskinfobuf+12], 0
	jnz	m1e
	mov	eax, [diskinfobuf+8]
	jmp	install_cmn

install_9x_2:
	push	ebx
	push	ebx
	push	3
	push	ebx
	push	1
	push	80000000h
	push	config
	call	[CreateFileA]
	inc	eax
	jnz	@f
ie2:
	push	10h
	push	ebx
	push	insterr2
	jmp	mbx
@@:
	dec	eax
	xchg	eax, esi
	push	ebx
	push	esi
	call	[GetFileSize]
	inc	eax
	jz	ie2
	dec	eax
	xchg	eax, ebp
	push	4
	push	1000h
	push	ebp
	push	ebx
	call	[VirtualAlloc]
	xchg	eax, edi
	test	edi, edi
	jz	ie2
	push	ebx
	push	tmp
	push	ebp
	push	edi
	push	esi
	call	[ReadFile]
	push	esi
	call	[CloseHandle]
	push	ebx
	push	80h
	push	2
	push	ebx
	push	ebx
	push	40000000h
	push	config
	call	[CreateFileA]
	inc	eax
	jz	ie2
	dec	eax
	xchg	eax, esi
	mov	eax, dword [edi]
	or	eax, 0x20202000
	cmp	eax, '[men'
	jz	menu
	push	ostitle
	call	[lstrlenA]
	cmp	eax, 17
	ja	bt1
	push	esi edi
	mov	esi, ostitle
	mov	edi, mtldr_code+23Ah
	mov	ecx, eax
	rep	movsb
	mov	dword [edi], '? [y'
	mov	dword [edi+4], '/n]:'
	mov	word [edi+8], ' '
	pop	edi esi
	jmp	ct1
bt1:
	push	img_real_name+3
	call	[lstrlenA]
	add	eax, mtldr_code_size+1+100h
	mov	word [mtldr_code+0x19], ax
ct1:
	push	ebx
	push	tmp
	push	8
	push	install
	push	esi
	call	[WriteFile]
cfgd:
	mov	eax, mtldr_name
	push	eax
	push	eax
	push	eax
	call	[CharToOemA]
	call	[lstrlenA]
	push	ebx
	push	tmp
	push	eax
	push	mtldr_name
	push	esi
	call	[WriteFile]
	push	ebx
	push	tmp
	push	2
	push	newline
	push	esi
	call	[WriteFile]
	push	ebx
	push	tmp
	push	ebp
	push	edi
	push	esi
	call	[WriteFile]
	push	esi
	call	[CloseHandle]
	call	write_mtldr1
	push	ostitle
	call	[lstrlenA]
	cmp	eax, 11
	jbe	@f
	push	ebx
	push	tmp
	push	ld2sz
	push	ld2
	push	esi
	push	ebx
	push	tmp
	push	eax
	push	ostitle
	push	esi
	push	ebx
	push	tmp
	push	ld1sz
	push	ld1
	push	esi
	call	[WriteFile]
	call	[WriteFile]
	call	[WriteFile]
@@:
	push	esi
	call	[CloseHandle]
	jmp	suci
menu:
	push	edi
	or	ecx, -1
mes:
	mov	al, 0xA
	repnz	scasb
	cmp	byte [edi], '['
	jz	med
	cmp	dword [edi], 'menu'
	jnz	mes
	cmp	dword [edi+4], 'item'
	jnz	mes
	cmp	byte [edi+8], '='
	jnz	mes
	mov	eax, [edi+9]
	or	eax, '    '
	cmp	eax, 'koli'
	jnz	mes
	mov	eax, [edi+13]
	and	eax, 0xFFFFFF
	or	eax, '   '
	cmp	eax, 'bri'
	jnz	mes
	movzx	eax, byte [edi+16]
	or	al, 0x20
	mov	[menuitems+eax], 1
	jmp	mes
med:
	cmp	word [edi-4], 0x0A0D
	jnz	@f
	dec	edi
	dec	edi
	jmp	med
@@:
	sub	edi, [esp]
	push	ebx
	push	tmp
	push	edi
	push	dword [esp+12]
	push	esi
	call	[WriteFile]
	add	[esp], edi
	sub	ebp, edi
	mov	ecx, 7
	cmp	[menuitems+0x20], 0
	jnz	@f
	cmp	[menuitems+','], 0
	jz	mef
@@:
	mov	eax, '0'
mel1:
	cmp	[menuitems+eax], 0
	jz	med1
	inc	eax
	cmp	al, '9'+1
	jb	mel1
	jnz	@f
	mov	al, 'a'
	jmp	mel1
@@:
	cmp	al, 'z'
	jbe	mel1
	push	ebx
	push	tmp
	push	ebp
	push	dword [esp+12]
	push	esi
	call	[WriteFile]
	push	esi
	call	[CloseHandle]
	jmp	nomx
med1:
	mov	[menuitem+7], al
	mov	ecx, 8
mef:
	push	ebx
	push	tmp
	push	ecx
	push	menuitem
	push	esi
	push	ebx
	push	tmp
	push	ecx
	push	menuitem
	push	esi
	push	ebx
	push	tmp
	push	9
	push	mis
	push	esi
	call	[WriteFile]
	call	[WriteFile]
	push	ebx
	push	tmp
	push	title9xsz
	push	title9x
	push	esi
	call	[WriteFile]
	push	ebx
	push	tmp
	push	ostitle
	call	[lstrlenA]
	push	eax
	push	ostitle
	push	esi
	call	[WriteFile]
	push	ebx
	push	tmp
	push	title9x2sz
	push	title9x2
	push	esi
	call	[WriteFile]
	call	[WriteFile]
	push	ebx
	push	tmp
	push	11
	push	sec9x2
	push	esi
	call	[WriteFile]
	mov	byte [mtldr_code+1], 37h
	pop	edi
	jmp	cfgd

install_vista:
	push	esi
	call	[CloseHandle]
	mov	edi, sbn
	call	adjust_privilege
	mov	edi, srn
	call	adjust_privilege
	push	ebx
	push	ebx
	call	[CoInitializeEx]
	test	eax, eax
	js	we
	push	ebx
	push	ebx
	push	ebx
	push	3
	push	ebx
	push	ebx
	push	ebx
	push	-1
	push	ebx
	call	[CoInitializeSecurity]
	test	eax, eax
	jns	@f
we2:
	call	[CoUninitialize]
we:
	call	delete_mtldr
	push	10h
	push	ebx
	push	wmierr
	jmp	mbx
@@:
	push	ebx
	push	esp
	push	IID_IWbemLocator
	push	1
	push	ebx
	push	CLSID_WbemLocator
	call	[CoCreateInstance]
	pop	edi
	test	eax, eax
	js	we2
	push	ebx
	push	esp
	push	ebx
	push	ebx
	push	ebx
	push	ebx
	push	ebx
	push	ebx
	push	ns
	push	edi
	mov	esi, [edi]
	call	dword [esi+12]
	push	eax
	push	edi
	call	dword [esi+8]
	pop	eax
	pop	edi
	test	eax, eax
	js	we2
	push	ebx
	push	ebx
	push	3
	push	3
	push	ebx
	push	ebx
	push	10
	push	edi
	call	[CoSetProxyBlanket]
	test	eax, eax
	jns	@f
we3:
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	jmp	we2
@@:
	xor	esi, esi
	push	osp
	push	osn
	push	bs
	call	call_method
	test	eax, eax
	js	we3
	mov	esi, guid
	mov	ebp, menuitems
	push	esi
	call	[CoCreateGuid]
	push	2000h/2
	push	ebp
	push	esi
	call	[StringFromGUID2]
	mov	esi, [varout+8]
	push	con
	push	bs
	call	call_method
	jns	@f
wecei:
	mov	ebp, coerr
wece:
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	call	[CoUninitialize]
	call	delete_mtldr
	push	10h
	push	ebx
	push	ebp
	jmp	mbx
@@:
	pop	eax
	push	esi
	push	eax
	mov	ebp, tmp_data
	mov	dword [vartmpstr+8], ebp
	mov	dword [vari32+8], 0x12000004
	push	2000h/2
	push	ebp
	push	-1
	push	ostitle
	push	ebx
	push	ebx
	call	[MultiByteToWideChar]
	mov	esi, [varout+8]
	push	ssen
	push	bo
	call	call_method
	mov	ebp, setproperr
	js	wece2
	sub	dword [esp], 24
	mov	byte [vari32+8], 2
	push	2000h/2
	push	tmp_data
	push	-1
	push	mtldr_name+2
	push	ebx
	push	ebx
	call	[MultiByteToWideChar]
	push	ssen
	push	bo
	call	call_method
	js	wece2
	mov	dword [vari32+8], 0x11000001
	mov	ecx, tmp_data
	mov	dword [ecx], '\' + ('?' shl 16)
	mov	dword [ecx+4], '?' + ('\' shl 16)
	xor	eax, eax
	mov	dword [ecx+12], eax
	mov	al, [mtldr_name+1]
	shl	eax, 16
	mov	al, [mtldr_name]
	mov	dword [ecx+8], eax
	push	spden
	push	bo
	call	call_method
	js	wece2
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	pop	eax
	pop	esi
	push	eax
	push	oon
	push	bs
	call	call_method
	mov	ebp, orerr
	js	wece3
	pop	eax
	push	esi
	push	eax
	mov	esi, [varout+8]
	mov	dword [vari32+8], 0x24000001
	push	gen
	push	bo
	call	call_method
	js	wece2
	push	esi
	mov	esi, [varout+8]
	push	ebx
	push	ebx
	push	varout
	push	ebx
	push	idsn
	mov	eax, [esi]
	push	esi
	call	dword [eax+16]
	push	eax
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	pop	eax
	pop	esi
	test	eax, eax
	js	wece2
	push	esi
	cmp	word [varout], 2008h
	jnz	wece4
	mov	esi, [varout+8]
	cmp	word [esi], 1
	jnz	wece4
	push	dword [esi+20]
	mov	eax, [esi+16]
	inc	eax
	push	eax
	push	esp
	push	esi
	call	[SafeArrayRedim]
	pop	ecx
	pop	ecx
	test	eax, eax
	js	wece4
	push	menuitems
	call	[SysAllocString]
	test	eax, eax
	jz	wece4
	push	eax
	mov	ecx, [esi+16]
	add	ecx, [esi+20]
	dec	ecx
	push	ecx
	mov	ecx, esp
	push	eax
	push	ecx
	push	esi
	call	[SafeArrayPutElement]
	pop	ecx
	call	[SysFreeString]
	pop	esi
	push	solen
	push	bo
	call	call_method
	js	wece2
	push	varout
	call	[VariantClear]
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	pop	eax
	pop	esi
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	call	[CoUninitialize]
	jmp	suci
wece4:
	pop	esi
wece2:
	mov	eax, [esi]
	push	esi
	call	dword [eax+8]
	pop	eax
	pop	esi
	push	eax
wece3:
	mov	dword [vartmpstr+8], menuitems
	pop	eax
	push	dop
	push	don
	push	bs
	call	call_method
	pop	eax
	jmp	wece

write_mtldr1:
	push	ebx
	push	80h
	push	2
	push	ebx
	push	ebx
	push	40000000h
	push	mtldr_name
	call	[CreateFileA]
	inc	eax
	jnz	@f
	push	10h
	push	ebx
	push	noc
	jmp	mbx
@@:
	dec	eax
	xchg	eax, esi
	push	ebx
	push	tmp
	push	mtldr_code_size
	push	mtldr_code
	push	esi
	call	[WriteFile]
	push	img_real_name
	push	img_real_name
	call	[CharToOemA]
	mov	edi, img_real_name+3
	push	edi
	call	[lstrlenA]
	inc	eax
	push	eax
	push	ebx
	push	tmp
	push	eax
	push	edi
	push	esi
	call	[WriteFile]
	pop	ecx
	ret
delete_mtldr:
        push    mtldr_name
        push    mtldr_name
        push    mtldr_name
        call    [OemToCharA]
        call    [DeleteFileA]
	ret

adjust_privilege:
	cmp	[advapi32], 0
	jnz	@f
	push	advapi32_name
	call	[LoadLibraryA]
	mov	[advapi32], eax
	mov	esi, eax
	test	esi, esi
	jz	ape
	push	opts
	push	esi
	call	[GetProcAddress]
	mov	[OpenProcessToken], eax
	test	eax, eax
	jz	ape
	push	lpvs
	push	esi
	call	[GetProcAddress]
	mov	[LookupPrivilegeValueA], eax
	test	eax, eax
	jz	ape
	push	atps
	push	esi
	call	[GetProcAddress]
	mov	[AdjustTokenPrivileges], eax
	test	eax, eax
	jz	ape
@@:
	push	ebx
	push	esp
	push	28h
	call	[GetCurrentProcess]
	push	eax
	call	[OpenProcessToken]
	test	eax, eax
	pop	esi
	jz	ape
	push	2
	push	ebx
	push	ebx
	mov	eax, esp
	push	1
	push	eax
	push	edi
	push	ebx
	call	[LookupPrivilegeValueA]
	test	eax, eax
	jz	ape2
	mov	eax, esp
	push	ebx
	push	ebx
	push	ebx
	push	eax
	push	ebx
	push	esi
	call	[AdjustTokenPrivileges]
	test	eax, eax
	jz	ape2
	add	esp, 10h
	push	esi
	call	[CloseHandle]
	ret
ape2:
	add	esp, 10h
	push	esi
	call	[CloseHandle]
ape:
	push	10h
	push	ebx
	push	apf
	jmp	mbx

call_method:
	push	ebx
	mov	eax, esp
	push	ebx
	push	eax
	push	ebx
	push	ebx
	push	dword [eax+8]
	mov	eax, [edi]
	push	edi
	call	dword [eax+24]
	xchg	edi, [esp]
	test	eax, eax
	js	r
	push	ebx
	mov	eax, esp
	push	ebx
	push	eax
	push	ebx
	push	dword [eax+16]
	mov	eax, [edi]
	push	edi
	call	dword [eax+76]
	push	eax
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	pop	eax
	pop	edi
	test	eax, eax
	js	r
	push	ebx
	push	esp
	push	ebx
	mov	eax, [edi]
	push	edi
	call	dword [eax+60]
	push	eax
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	pop	eax
	pop	edi
	test	eax, eax
	js	r
cml1:
	mov	eax, [esp+16]
	add	dword [esp+16], 8
	cmp	dword [eax], 0
	jz	cme1
	push	ebx
	push	dword [eax+4]
	push	ebx
	push	dword [eax]
	mov	eax, [edi]
	push	edi
	call	dword [eax+20]
	test	eax, eax
	js	r2
	jmp	cml1
cme1:
	and	dword [varout], 0
	mov	ecx, [esp+8]
	test	esi, esi
	jz	cms
	push	ebx
	push	ebx
	push	varout
	push	ebx
	push	rpn
	mov	eax, [esi]
	push	esi
	call	dword [eax+16]
	test	eax, eax
	js	r2
	cmp	word [varout], 8
	jnz	r2
	mov	ecx, [varout+8]
cms:
	pop	edx
	push	edx
	push	ebx
	mov	eax, esp
	push	ebx
	push	eax
	push	edi
	push	ebx
	push	ebx
	push	dword [eax+16]
	push	ecx
	mov	eax, [edx]
	push	edx
	call	dword [eax+96]
	push	eax
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	push	varout
	call	[VariantClear]
	pop	eax
	pop	edi
	test	eax, eax
	js	r
	push	ebx
	push	ebx
	push	varout
	push	ebx
	push	retvaln
	mov	eax, [edi]
	push	edi
	call	dword [eax+16]
	test	eax, eax
	js	r2
	mov	eax, 80000000h
	cmp	word [varout], 11
	jnz	r2
	cmp	word [varout+8], 0
	jz	r2
	mov	eax, [esp+16]
	mov	eax, [eax-4]
	test	eax, eax
	jz	r2
	push	ebx
	push	ebx
	push	varout
	push	ebx
	push	eax
	mov	eax, [edi]
	push	edi
	call	dword [eax+16]
	test	eax, eax
	js	r2
	cmp	word [varout], 13
	setnz	al
	shl	eax, 31
r2:
	push	eax
	mov	eax, [edi]
	push	edi
	call	dword [eax+8]
	pop	eax
r:
	pop	edi
	test	eax, eax
	ret	8

ofn_hook:
	cmp	dword [esp+8], 2
	jnz	@f
	push	260
	push	ostitle
	push	23
	push	dword [esp+12+4]
	call	[GetDlgItemTextA]
@@:
	xor	eax, eax
	ret	10h

section '.data' data readable writable
data resource from 'rsrc.res'
end data

	align	4
ofn:
	dd	76
	dd	0
	dd	ofn_title_template
	dd	filter
	dd	0
	dd	0
	dd	0
	dd	img_name
	dd	100h
	dd	0
	dd	0
	dd	0
	dd	ofn_title
	dd	818A4h
	dd	0
	dd	aImg
	dd	0
	dd	ofn_hook
	dd	0
ofn_title_template:
	dw	1,-1
	dd	0
	dd	0
	dd	56000444h
	dw	2
	dw	0,0,275,28
	dw	0,0,0
	dw	8
	dd	0
	du	'MS Sans Serif',0
	align	4
	dd	0
	dd	0
	dd	50010000h
	dw	5,12,45,9
	dw	-1
	dw	0
	dw	-1,82h
	du	'Title:',0
	dw	0
	align	4
	dd	0
	dd	204h
	dd	50010080h
	dw	54,10,218,12
	dw	23
	dw	0
	dw	-1,81h
	du	'KolibriOS',0
	dw	0

filter	db	'Image files (*.img)',0,'*.img',0,'All files',0,'*.*',0,0
ofn_title db	'Select KolibriOS image file',0
aImg	db	'img',0
norightsmsg	db	'Cannot query drive info.',10
		db	'Probably it is invalid drive or you are not administrator',0
nohd	db	'Image must be on hard disk!',0
m1	db	'Please mail to diamondz@land.ru',0
nom	db	"Too many mtldr's found!",0
noc	db	'Cannot create mtldr file!',0
osstr	db	'operating systems',0
bootini	db	'c:\boot.ini',0
insterr db	'Cannot write to boot.ini. Probably you are not administrator.',0
insterr2 db	'Cannot open config.sys',0
ptl	db	'Path is too long',0
succ	db	'Installation successful!',0
suct	db	'Success',0
vwin32	db	'\\.\vwin32',0
config	db	'C:\config.sys',0
sec9x2	db	']',13,10
install db	'install='
newline	db	13,10
menuitem db	'kolibri',0
mis	db	'menuitem='
title9x db	',Load '
title9xsz = $ - title9x
title9x2 db	13,10,13,10,'['
title9x2sz = $ - title9x2
ld1	db	'Load '
ld1sz = $ - ld1
ld2	db	'? [y/n]: ',0
ld2sz = $ - ld2
apf	db	'Cannot adjust backup and restore privileges',0
opts	db	'OpenProcessToken',0
lpvs	db	'LookupPrivilegeValueA',0
atps	db	'AdjustTokenPrivileges',0
sbn	db	'SeBackupPrivilege',0
srn	db	'SeRestorePrivilege',0
wmierr	db	'BCD WMI API: initialization error',0
coerr	db	'Cannot create BCD object for KolibriOS loader',0
setproperr db	'Cannot create BCD element in object for KolibriOS loader',0
orerr	db	'Cannot add KolibriOS loader in BCD display list',0
ns	du	'root\wmi',0
retvaln	du	'ReturnValue'
emptystr du	0
rpn	du	'__Relpath',0
bs	du	'BcdStore',0
bo	du	'BcdObject',0
osn	du	'OpenStore',0
con	du	'CreateObject',0
don	du	'DeleteObject',0
oon	du	'OpenObject',0
ssen	du	'SetStringElement',0
spden	du	'SetPartitionDeviceElement',0
gen	du	'GetElement',0
solen	du	'SetObjectListElement',0
fn	du	'File',0
storen	du	'Store',0
idn	du	'Id',0
idsn	du	'Ids',0
tn	du	'Type',0
obn	du	'Object',0
sn	du	'String',0
dtn	du	'DeviceType',0
aon	du	'AdditionalOptions',0
pn	du	'Path',0
en	du	'Element',0
bg	du	'{9dea862c-5cdd-4e70-acc1-f32b344d4795}',0

align 4
advapi32	dd	0

regs:
	dd	0
	dd	diskinfobuf
	dd	86Fh
	dd	440Dh
	dd	0
	dd	0
	dd	1

diskinfobuf:
	db	10h,0,0,0FFh
	times 0Ch db 0

IID_IWbemLocator:
	dd	0DC12A687h
	dw	737Fh
	dw	11CFh
	db	88h, 4Dh, 00h, 0AAh, 00h, 4Bh, 2Eh, 24h
CLSID_WbemLocator:
	dd	4590F811h
	dw	1D3Ah
	dw	11D0h
	db	89h, 1Fh, 00h, 0AAh, 00h, 4Bh, 2Eh, 24h
IID_IWbemClassObject:
	dd	0DC12A681h
	dw	737Fh
	dw	11CFh
	db	88h, 4Dh, 00h, 0AAh, 00h, 4Bh, 2Eh, 24h
varemptystr:
	dd	8, 0, emptystr, 0
vartmpstr:
	dd	8, 0, menuitems, 0
varbootmgr:
	dd	8, 0, bg, 0
vari32:
	dd	3, 0, 10400008h, 0
vari32_pd:
	dd	3, 0, 2, 0
osp:
	dd	fn, varemptystr
	dd	0, storen
	dd	idn, vartmpstr
	dd	tn, vari32
	dd	0, obn
	dd	tn, vari32
	dd	sn, vartmpstr
	dd	0, 0
	dd	tn, vari32
	dd	dtn, vari32_pd
	dd	aon, varemptystr
	dd	pn, vartmpstr
	dd	0, 0
	dd	idn, varbootmgr
	dd	0, obn
	dd	tn, vari32
	dd	0, en
	dd	tn, vari32
	dd	idsn, varout
	dd	0, 0

dop:
	dd	idn, vartmpstr
	dd	0, 0

data import
macro thunk a
{a#_thunk:dw 0
db `a,0}
	dd	0,0,0, rva kernel32_name, rva kernel32_thunks
	dd	0,0,0, rva user32_name, rva user32_thunks
	dd	0,0,0, rva comdlg32_name, rva comdlg32_thunks
	dd	0,0,0, rva ole32_name, rva ole32_thunks
	dd	0,0,0, rva oleaut32_name, rva oleaut32_thunks
	dd	0,0,0,0,0
kernel32_name	db	'kernel32.dll',0
user32_name	db	'user32.dll',0
advapi32_name	db	'advapi32.dll',0
comdlg32_name	db	'comdlg32.dll',0
ole32_name	db	'ole32.dll',0
oleaut32_name	db	'oleaut32.dll',0

kernel32_thunks:
GetVersion	dd	rva GetVersion_thunk
CreateFileA	dd	rva CreateFileA_thunk
DeviceIoControl	dd	rva DeviceIoControl_thunk
CloseHandle	dd	rva CloseHandle_thunk
GetFileAttributesA dd	rva GetFileAttributesA_thunk
SetFileAttributesA dd   rva SetFileAttributesA_thunk
GetLastError	dd	rva GetLastError_thunk
ReadFile	dd	rva ReadFile_thunk
WriteFile	dd	rva WriteFile_thunk
ExitProcess	dd	rva ExitProcess_thunk
WritePrivateProfileStringA dd rva WritePrivateProfileStringA_thunk
GetShortPathNameA dd	rva GetShortPathNameA_thunk
lstrlenA	dd	rva lstrlenA_thunk
VirtualAlloc	dd	rva VirtualAlloc_thunk
GetFileSize	dd	rva GetFileSize_thunk
DeleteFileA     dd      rva DeleteFileA_thunk
MultiByteToWideChar dd	rva MultiByteToWideChar_thunk
GetCurrentProcess dd	rva GetCurrentProcess_thunk
LoadLibraryA	dd	rva LoadLibraryA_thunk
GetProcAddress	dd	rva GetProcAddress_thunk
	dw	0
thunk GetVersion
thunk CreateFileA
thunk DeviceIoControl
thunk CloseHandle
thunk GetFileAttributesA
thunk SetFileAttributesA
thunk GetLastError
thunk ReadFile
thunk WriteFile
thunk ExitProcess
thunk WritePrivateProfileStringA
thunk GetShortPathNameA
thunk lstrlenA
thunk VirtualAlloc
thunk GetFileSize
thunk DeleteFileA
thunk MultiByteToWideChar
thunk GetCurrentProcess
thunk LoadLibraryA
thunk GetProcAddress

user32_thunks:
MessageBoxA	dd	rva MessageBoxA_thunk
CharToOemA	dd	rva CharToOemA_thunk
OemToCharA      dd      rva OemToCharA_thunk
GetDlgItemTextA	dd	rva GetDlgItemTextA_thunk
	dw	0
thunk MessageBoxA
thunk CharToOemA
thunk OemToCharA
thunk GetDlgItemTextA

comdlg32_thunks:
GetOpenFileNameA	dd	rva GetOpenFileNameA_thunk
	dw	0
thunk GetOpenFileNameA

ole32_thunks:
CoInitializeEx		dd	rva CoInitializeEx_thunk
CoUninitialize		dd	rva CoUninitialize_thunk
CoInitializeSecurity	dd	rva CoInitializeSecurity_thunk
CoCreateInstance	dd	rva CoCreateInstance_thunk
CoSetProxyBlanket	dd	rva CoSetProxyBlanket_thunk
CoCreateGuid		dd	rva CoCreateGuid_thunk
StringFromGUID2		dd	rva StringFromGUID2_thunk
	dw	0
thunk CoInitializeEx
thunk CoUninitialize
thunk CoInitializeSecurity
thunk CoCreateInstance
thunk CoSetProxyBlanket
thunk CoCreateGuid
thunk StringFromGUID2

oleaut32_thunks:
VariantClear	dd	rva VariantClear_thunk
SafeArrayRedim	dd	rva SafeArrayRedim_thunk
SafeArrayPutElement dd	rva SafeArrayPutElement_thunk
SysAllocString	dd	rva SysAllocString_thunk
SysFreeString	dd	rva SysFreeString_thunk
	dw	0
thunk VariantClear
thunk SafeArrayRedim
thunk SafeArrayPutElement
thunk SysAllocString
thunk SysFreeString
end data

mtldr_code:
	file	'mtldr_for_installer'
mtldr_code_size = $ - mtldr_code

dn	db	'\\.\'
img_name	rb	256
img_real_name	rb	256
mtldr_name	rb	256
tmp_data	rb	2000h
ostitle		rb	260
systitle	rb	262

align 4
OpenProcessToken	dd	?
LookupPrivilegeValueA	dd	?
AdjustTokenPrivileges	dd	?
tmp	dd	?
sdn	rd	3
pi	rd	8
varout	rd	4
guid	rd	4
b9x	db	?
menuitems	rb	100h
