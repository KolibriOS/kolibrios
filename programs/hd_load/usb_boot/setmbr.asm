	format PE GUI 4.0
section '.text' code readable executable
entry start
start:
	xor	ebx, ebx
	push	ebx	; lpParam
	push	400000h	; hInstance
	push	ebx	; hMenu
	push	ebx	; hWndParent
	push	100	; nHeight
	push	200	; nWidth
	mov	eax, 80000000h
	push	eax	; y
	push	eax	; x
	push	10EF0140h	; dwStyle
	push	WndName
	push	ClassName
	push	388h	; dwExStyle
	call	[CreateWindowExA]
	mov	edi, eax
	push	0Ah	; OEM_FIXED_FONT
	call	[GetStockObject]
	push	ebx
	push	eax
	push	30h	; WM_SETFONT
	push	edi
	call	[SendMessageA]
	call	CollectDrivesInfo
	push	MyWndProc
	push	-4	; GWL_WNDPROC
	push	edi
	call	[SetWindowLongA]
	mov	[OldWndProc], eax
	sub	esp, 20h
	mov	esi, esp
@@:
	push	ebx
	push	ebx
	push	ebx
	push	esi
	call	[GetMessageA]
	test	eax, eax
	jz	@f
	push	esi
	call	[TranslateMessage]
	push	esi
	call	[DispatchMessageA]
	jmp	@b
@@:
	add	esp, 20h
	ret

MyWndProc:
	push	edi
	mov	edi, [esp+8]
	cmp	dword [esp+12], 2	; WM_DESTROY
	jnz	@f
	push	0
	call	[PostQuitMessage]
@@:
	cmp	dword [esp+12], 219h	; WM_DEVICECHANGE
	jnz	w
	cmp	dword [esp+16], 8000h	; DBT_DEVICEARRIVAL
	jz	@f
	cmp	dword [esp+16], 8004h	; DBT_DEVICEREMOVECOMPLETE
	jnz	w
@@:
	call	UpdateDrivesInfo
w:
	cmp	dword [esp+12], 203h	; WM_LBUTTONDBLCLK
	jnz	@f
	push	0
	push	0
	push	188h	; LB_GETCURSEL
	push	edi
	call	[SendMessageA]
	cmp	eax, -1
	jz	@f
	push	n+4
	push	eax
	push	189h	; LB_GETTEXT
	push	edi
	call	[SendMessageA]
	mov	dword [n], '\\.\'
	mov	byte [n+4+aPhysicalDrive.sz], 0
	call	install
@@:
	pop	edi
	pop	eax
	push	[OldWndProc]
	push	eax
	jmp	[CallWindowProcA]

UpdateDrivesInfo:
	push	0
	push	0
	push	184h	; LB_RESETCONTENT
	push	edi
	call	[SendMessageA]

CollectDrivesInfo:
	xor	eax, eax
	mov	ecx, 32
	push	edi
	mov	edi, PhysicalDrives
	rep	stosd
	pop	edi
	push	esi
	call	[GetLogicalDrives]
	mov	esi, eax
	mov	[a], 'A'
l:
	shr	esi, 1
	jnc	d
	mov	[a+2], 0
	push	a
	call	[GetDriveTypeA]
; Uncomment following lines to allow hard drives
;	cmp	eax, 3	; DRIVE_FIXED
;	jz	@f
	cmp	eax, 2	; DRIVE_REMOVABLE
	jnz	d
@@:
	push	0	; hTemplateFile
	push	0	; dwFlagsAndAttributes
	push	3	; dwCreationDisposition = OPEN_EXISTING
	push	0	; lpSecurityAttributes
	push	3	; dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE
	push	0	; dwDesiredAccess
	push	a2
	call	[CreateFileA]
	cmp	eax, -1
	jz	d
	push	eax
	push	0
	mov	ecx, esp
	push	0	; lpOverlapped
	push	ecx	; lpBytesReturned
	push	12	; nOutBufferSize
	push	sdn	; lpOutBuffer
	push	0
	push	0
	push	2D1080h	; IOCTL_STORAGE_GET_DEVICE_NUMBER
	push	eax
	call	[DeviceIoControl]
	pop	ecx
	pop	edx
	push	eax
	push	edx
	call	[CloseHandle]
	pop	eax
	test	eax, eax
	jz	d	; probably it is floppy
	mov	eax, [sdn+4]
	cmp	eax, 32
	jae	d
	movzx	ecx, byte [a]
	sub	cl, 'A'
	bts	[PhysicalDrives+eax*4], ecx
d:
	inc	[a]
	test	esi, esi
	jnz	l
	xor	esi, esi
.physloop:
	push	esi
	mov	esi, [PhysicalDrives+esi*4]
	test	esi, esi
	jz	.physnext
	push	edi esi
	mov	esi, aPhysicalDrive
	mov	edi, n
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	pop	esi
	dec	edi
	mov	eax, [esp+4]
	cmp	al, 10
	jb	.1dig
	aam
	add	ah, '0'
	mov	byte [edi], ah
	inc	edi
.1dig:
	add	al, '0'
	stosb
	mov	al, ':'
	stosb
	mov	cl, 'A'-1
.logloop:
	mov	al, ' '
	stosb
	mov	al, cl
	stosb
@@:
	inc	byte [edi-1]
	shr	esi, 1
	jnc	@b
	mov	cl, [edi-1]
	mov	al, ':'
	stosb
	mov	al, '\'
	stosb
	test	esi, esi
	jnz	.logloop
	mov	al, 0
	stosb
	pop	edi
	push	n
	push	0
	push	180h	; LB_ADDSTRING
	push	edi
	call	[SendMessageA]
.physnext:
	pop	esi
	inc	esi
	cmp	esi, 32
	jb	.physloop
	pop	esi
	ret

install:
	push	0	; hTemplateFile
	push	0	; dwFlagsAndAttributes
	push	3	; dwCreationDisposition = OPEN_EXISTING
	push	0	; lpSecurityAttributes
	push	3	; dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE
	push	0C0000000h	; dwDesiredAccess = GENERIC_READ|GENERIC_WRITE
	push	n
	call	[CreateFileA]
	cmp	eax, -1
	jnz	@f
deverre:
	push	10h
	push	0
	push	deverr
	push	edi
	call	[MessageBoxA]
	ret
@@:
	push	esi
	mov	esi, eax
	push	eax
	mov	eax, esp
	push	0
	push	eax
	push	512
	push	mbr_dev
	push	esi
	call	[ReadFile]
	test	eax, eax
	jnz	@f
deverrl:
	push	esi
	call	[CloseHandle]
	pop	eax
	pop	esi
	jmp	deverre
@@:
	push	esi edi
	mov	esi, mbr_new
	mov	edi, mbr_dev
	mov	ecx, 1B8h
	rep	movsb
	mov	al, [edi+6]
	or	al, [edi+16h]
	or	al, [edi+26h]
	or	al, [edi+36h]
	test	al, al
	js	@f
	or	byte [edi+6], 80h
@@:
	pop	edi esi
	push	0
	push	0
	push	0
	push	esi
	call	[SetFilePointer]
	test	eax, eax
	jnz	deverrl
	mov	eax, esp
	push	0
	push	eax
	push	512
	push	mbr_dev
	push	esi
	call	[WriteFile]
	test	eax, eax
	jz	deverrl
	cmp	dword [esp], 512
	jnz	deverrl
; done!
done_succ:
	push	40h
	push	ok
	push	succ
	push	edi
	call	[MessageBoxA]
	push	0
	call	[PostQuitMessage]
r:
	pop	eax
	push	esi
	call	[CloseHandle]
	pop	esi
	ret

section '.data' data readable writable
data resource from 'rsrc.res'
end data

ClassName db	'LISTBOX',0
WndName	db	'Select drive',0
deverr	db	'Cannot open physical device or device error (no administrator rights?)',0
ok	db	'Success',0
succ	db	'Standard MBR has been installed',0
a2	db	'\\.\'
a	db	'?:',0,0
aPhysicalDrive	db	'PhysicalDrive',0
.sz = $ - aPhysicalDrive

data import
macro thunk a
{a#_thunk:dw 0
db `a,0}
	dd	0,0,0, rva kernel32_name, rva kernel32_thunks
	dd	0,0,0, rva user32_name, rva user32_thunks
	dd	0,0,0, rva gdi32_name, rva gdi32_thunks
	dd	0,0,0,0,0
kernel32_name	db	'kernel32.dll',0
user32_name	db	'user32.dll',0
gdi32_name	db	'gdi32.dll',0
kernel32_thunks:
GetLogicalDrives	dd	rva GetLogicalDrives_thunk
GetDriveTypeA		dd	rva GetDriveTypeA_thunk
CreateFileA		dd	rva CreateFileA_thunk
ReadFile		dd	rva ReadFile_thunk
WriteFile		dd	rva WriteFile_thunk
SetFilePointer		dd	rva SetFilePointer_thunk
CloseHandle		dd	rva CloseHandle_thunk
DeviceIoControl		dd	rva DeviceIoControl_thunk
	dw	0
thunk GetLogicalDrives
thunk GetDriveTypeA
thunk CreateFileA
thunk ReadFile
thunk WriteFile
thunk SetFilePointer
thunk CloseHandle
thunk DeviceIoControl
user32_thunks:
CreateWindowExA		dd	rva CreateWindowExA_thunk
GetMessageA		dd	rva GetMessageA_thunk
TranslateMessage	dd	rva TranslateMessage_thunk
DispatchMessageA	dd	rva DispatchMessageA_thunk
PostQuitMessage		dd	rva PostQuitMessage_thunk
CallWindowProcA		dd	rva CallWindowProcA_thunk
SetWindowLongA		dd	rva SetWindowLongA_thunk
SendMessageA		dd	rva SendMessageA_thunk
MessageBoxA		dd	rva MessageBoxA_thunk
	dw	0
thunk CreateWindowExA
thunk GetMessageA
thunk TranslateMessage
thunk DispatchMessageA
thunk PostQuitMessage
thunk CallWindowProcA
thunk SetWindowLongA
thunk SendMessageA
thunk MessageBoxA
gdi32_thunks:
GetStockObject		dd	rva GetStockObject_thunk
	dw	0
thunk GetStockObject
end data

align 4
mbr_new:
	file	'mbr'

align 4
OldWndProc	dd	?
PhysicalDrives	rd	32
sdn		rd	3
n		rb	1024
mbr_dev		rb	512
