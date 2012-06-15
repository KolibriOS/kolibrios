	format PE GUI 4.0
section '.text' code readable executable
entry start
start:
	push	ebp
	mov	ebp, info
	xor	ebx, ebx
; set current directory to exe dir
	push	300
	push	ebp
	push	ebx
	call	[GetModuleFileNameA]
	lea	edi, [ebp+eax]
	xchg	eax, ecx
	mov	al, '\'
	std
	repnz	scasb
	cld
	jz	@f
	pop	ebp
	ret
@@:
	mov	byte [edi+1], bl
	push	ebp
	call	[SetCurrentDirectoryA]
; parse command line
	call	[GetCommandLineA]
	xchg	eax, esi
; skip leading spaces
@@:
	lodsb
	cmp	al, 0
	jz	cmdlineend
	cmp	al, ' '
	jbe	@b
	cmp	al, '"'
	jz	p
; skip EXE name
@@:
	lodsb
	cmp	al, ' '
	ja	@b
	dec     esi
	jmp	q
p:
	lodsb
	cmp	al, 0
	jz	cmdlineend
	cmp	al, '"'
	jnz	p
q:
; skip leading spaces
	lodsb
	cmp	al, 0
	jz	cmdlineend
	cmp	al, ' '
	jbe	q
	dec	esi
; now esi points to 1st argument
	jmp	@f
cmdlineend:
	mov	esi, def
@@:
	mov	al, 'c'
	cmp	byte [esi+1], ':'
	jnz	@f
	lodsb
	inc	esi
@@:
	mov	[ebp], al
	lodsb
	cmp	al, '\'
	jz	paramsok
usage:
	mov	esi, usagemsg
	jmp	failmsg
paramsok:
	cmp	byte [esi], '\'
	jz	usage
	mov	edi, esi
	mov	al, 0
	xor	ecx, ecx
	dec	ecx
	repnz	scasb
	not	ecx
	cmp	ecx, 290
	jae	usage
	lea	edi, [ebp+1]
	cmp     ecx, 1
	rep	movsb
	jz      copydefname
	cmp	byte [edi-2], '\'
	jnz	namegiven
copydefname:
	dec	edi
	mov	ecx, defnamesz
	mov	esi, defname
	rep	movsb
namegiven:
	push	ebx		; hTemplateFile
;	push	0x04000000	; dwFlagsAndAttributes = FILE_FLAG_DELETE_ON_CLOSE
	push	ebx
	push	ebx		; dwCreationDisposition
	push	ebx		; lpSecurityAttributes
	push	ebx		; dwShareMode
	push	ebx		; dwDesiredAccess
	push	name		; lpFileName
	call	[CreateFileA]
	inc	eax
	mov	esi, errmsg
	jz	failmsg
	dec	eax
	push	ebx
	push	ebx
	push	ebx
	push	ebx
	sub	edi, ebp
	push	edi
	push	ebp
	push	0Fh
	push	eax
	call	[DeviceIoControl]
	test	eax, eax
	mov	esi, errmsg2
	jz	failmsg
	push	ebx
	push	2		; EWX_REBOOT
	call	[ExitWindowsEx]
	pop	ebp
	ret
failmsg:
	push	ebx
	push	ebx
	push	esi
	push	ebx
	call	[MessageBoxA]
	pop	ebp
	ret

align 4
data import
	dd	0,0,0
	dd	rva kernel32_name
	dd	rva kernel32_thunks
	dd	0,0,0
	dd	rva user32_name
	dd	rva user32_thunks
	dd	0,0,0,0,0
kernel32_thunks:
CreateFileA	dd	rva CreateFileA_thunk
CloseHandle	dd	rva CloseHandle_thunk
DeviceIoControl	dd	rva DeviceIoControl_thunk
GetCommandLineA	dd	rva GetCommandLineA_thunk
SetCurrentDirectoryA dd	rva SetCurrentDirectoryA_thunk
GetModuleFileNameA dd	rva GetModuleFileNameA_thunk
	dd	0
user32_thunks:
MessageBoxA	dd	rva MessageBoxA_thunk
ExitWindowsEx	dd	rva ExitWindowsEx_thunk
	dw	0
CreateFileA_thunk:
	dw	0
	db	'CreateFileA'
CloseHandle_thunk:
	dw	0
	db	'CloseHandle'
DeviceIoControl_thunk:
	dw	0
	db	'DeviceIoControl'
GetCommandLineA_thunk:
	dw	0
	db	'GetCommandLineA'
SetCurrentDirectoryA_thunk:
	dw	0
	db	'SetCurrentDirectoryA'
GetModuleFileNameA_thunk:
	dw	0
	db	'GetModuleFileNameA'
MessageBoxA_thunk:
	dw	0
	db	'MessageBoxA'
ExitWindowsEx_thunk:
	dw	0
	db	'ExitWindowsEx',0
kernel32_name	db	'kernel32.dll',0
user32_name	db	'user32.dll',0
end data

section '.data' data readable writable
data resource from 'klbrico.res'
end data

name	db	'\\.\'
vxdfilename db	'ldklbr.vxd',0
errmsg	db	'Cannot load driver',0
errmsg2	db	'Invalid parameter',0
usagemsg db	'Usage: 9x2klbr [[drive:]\[path\][imagename]]',0
def	db	'\'
defname	db	'kolibri.img',0
defnamesz = $ - defname

info	rb	300
