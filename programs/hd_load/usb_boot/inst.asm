	format PE GUI 4.0
section '.text' code readable executable
entry start
start:
	xor	ebx, ebx
	mov	esi, a2_src
	mov	edi, a2
	movsd
	movsd
	movsd
	movsd
	movsd
	push	1
	call	[SetErrorMode]
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
	xchg	edi, eax
	push	0Ah	; OEM_FIXED_FONT
	call	[GetStockObject]
	push	ebx
	push	eax
	push	30h	; WM_SETFONT
	call	ListCommand
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

ListCommand:
	pop	eax
	push	edi
	push	eax
	jmp	[SendMessageA]

MyWndProc:
	push	edi ebx
	xor	ebx, ebx
	mov	edi, [esp+12]
	cmp	dword [esp+16], 2	; WM_DESTROY
	jnz	@f
	push	ebx
	call	[PostQuitMessage]
@@:
	cmp	dword [esp+16], 219h	; WM_DEVICECHANGE
	jnz	w
	cmp	dword [esp+20], 8000h	; DBT_DEVICEARRIVAL
	jz	@f
	cmp	dword [esp+20], 8004h	; DBT_DEVICEREMOVECOMPLETE
	jnz	w
@@:
	call	UpdateDrivesInfo
w:
	cmp	dword [esp+16], 203h	; WM_LBUTTONDBLCLK
	jnz	@f
	push	ebx
	push	ebx
	push	188h	; LB_GETCURSEL
	call	ListCommand
	cmp	eax, -1
	jz	@f
	push	n
	push	eax
	push	189h	; LB_GETTEXT
	call	ListCommand
	mov	eax, n
	mov	byte [eax+2], bl
	mov	edx, [eax]
	mov	[mtldr_out], dl
	mov	dword [eax], '\\.\'
	mov	dword [eax+4], edx
	call	install
@@:
	pop	ebx edi
	pop	eax
	push	[OldWndProc]
	push	eax
	jmp	[CallWindowProcA]

UpdateDrivesInfo:
	push	ebx
	push	ebx
	push	184h	; LB_RESETCONTENT
	call	ListCommand

CollectDrivesInfo:
	push	esi
	call	[GetLogicalDrives]
	mov	esi, eax
	mov	edx, a
	mov	byte [edx], 'A'
l:
	shr	esi, 1
	jnc	d
	mov	[edx+2], bl
	push	edx
	call	[GetDriveTypeA]
; Uncomment following lines to allow hard drives
;	cmp	eax, 3	; DRIVE_FIXED
;	jz	@f
	cmp	eax, 2	; DRIVE_REMOVABLE
	jnz	d
	push	ebx	; hTemplateFile
	push	ebx	; dwFlagsAndAttributes
	push	3	; dwCreationDisposition = OPEN_EXISTING
	push	ebx	; lpSecurityAttributes
	push	3	; dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE
	push	ebx	; dwDesiredAccess
	push	a2
	call	[CreateFileA]
	cmp	eax, -1
	jz	d
	push	eax
	push	ebx
	mov	ecx, esp
	push	ebx	; lpOverlapped
	push	ecx	; lpBytesReturned
	push	1024	; nOutBufferSize
	push	n	; lpOutBuffer
	push	ebx
	push	ebx
	push	70C00h	; IOCTL_DISK_GET_MEDIA_TYPES
	push	eax
	call	[DeviceIoControl]
	pop	ecx
	pop	eax
	push	ecx
	push	eax
	call	[CloseHandle]
	pop	ecx
	jecxz	@f	; not supported => OK
	cmp	byte [n+8], 11
	jnz	d
@@:
	mov	eax, a
	mov	ecx, n
	mov	byte [eax+2], '\'
	push	ecx
	push	ebx	; nFileSystemNameSize
	push	ebx	; lpFileSystemNameBuffer
	push	ebx	; lpFileSystemFlags
	push	ebx	; lpMaximumComponentLength
	push	ebx	; lpVolumeSerialNumber
	push	1024	; nVolumeNameSize
	mov	edx, [eax]
	mov	[ecx], edx
	mov	word [ecx+3], ' ['
	add	ecx, 5
	mov	byte [ecx], bl
	push	ecx	; lpVolumeNameBuffer
	push	eax	; lpRootPathName
	call	[GetVolumeInformationA]
	pop	eax
	push	eax
	cmp	byte [eax+5], bl
	jz	nol
@@:
	inc	eax
	cmp	byte [eax-1], bl
	jnz	@b
	mov	word [eax-1], ']'
;	jmp	@f
nol:
	mov	byte [eax+3], bl
@@:
	push	ebx
	push	180h	; LB_ADDSTRING
	call	ListCommand
d:
	mov	edx, a
	inc	byte [edx]
	test	esi, esi
	jnz	l
	pop	esi
	ret

install:
	push	ebx	; hTemplateFile
	push	ebx	; dwFlagsAndAttributes
	push	3	; dwCreationDisposition = OPEN_EXISTING
	push	ebx	; lpSecurityAttributes
	push	3	; dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE
	push	0C0000000h	; dwDesiredAccess = GENERIC_READ|GENERIC_WRITE
	push	eax
	call	[CreateFileA]
	cmp	eax, -1
	jz	deverre
	push	esi ebp
	mov	ebp, bootsect_dev
	xchg	esi, eax
	push	eax
	mov	eax, esp
	push	ebx
	push	eax
	push	512
	push	ebp
	push	esi
	call	[ReadFile]
	test	eax, eax
	jnz	@f
deverrl:
	push	esi
	call	[CloseHandle]
	pop	eax
	pop	ebp esi
deverre:
	push	10h
	push	ebx
	push	deverr
	push	edi
	call	[MessageBoxA]
	ret
@@:
; make sure that this is FAT32 volume
	cmp	word [ebp+0Bh], 200h	; bytes per sector
	jnz	bootinv
	cmp	word [ebp+0Eh], bx	; reserved sectors
	jz	bootinv
	cmp	byte [ebp+10h], bl	; number of FATs
	jz	bootinv
	cmp	word [ebp+11h], bx	; root dir entries
	jnz	bootinv			; must be 0 for FAT32
	cmp	word [ebp+16h], bx	; length of one copy of FAT1x
	jnz	bootinv
	cmp	dword [ebp+20h], ebx	; length of one copy of FAT32
	jz	bootinv
	cmp	byte [ebp+42h], ')'	; magic value
	jz	@f
bootinv:
	push	10h
	push	ebx
	push	nofat32
	jmp	re
@@:
; ok, this is really correct FAT32 volume, so start install
; copy file mtldr_f
	push	80h
	push	mtldr_out
	call	[SetFileAttributesA]
	push	ebx	; bFailIfExists
	push	mtldr_out	; lpNewFileName
	push	mtldr_in	; lpExistingFileName
	call	[CopyFileA]
	test	eax, eax
	jnz	@f
	push	10h
	push	ebx
	push	mterr
re:
	push	edi
	call	[MessageBoxA]
	jmp	r
@@:
	push	7
	push	mtldr_out
	call	[SetFileAttributesA]
; load bootsector
	push	ebx	; hTemplateFile
	push	ebx	; dwFlagsAndAttributes
	push	3	; dwCreationDisposition = OPEN_EXISTING
	push	ebx	; lpSecurityAttributes
	push	1	; dwShareMode = FILE_SHARE_READ
	push	80000000h	; dwDesiredAccess = GENERIC_READ
	push	btname
	call	[CreateFileA]
	cmp	eax, -1
	jnz	@f
bterrc:
	push	40h
	push	ebx
	push	bterr
	jmp	re
@@:
	mov	ecx, esp
	push	eax
	push	ebx
	push	ecx
	push	512
	push	bootsect_new
	push	eax
	call	[ReadFile]
	pop	ecx
	push	eax
	push	ecx
	call	[CloseHandle]
	pop	eax
	test	eax, eax
	jz	bterrc
	cmp	dword [esp], 512
	jnz	bterrc
; patch bootsector with real values
	push	esi edi
	mov	esi, bootsect_new
	mov	edi, bootsect_dev
	movsb
	movsb
	movsb
	add	esi, 57h
	add	edi, 57h
	mov	ecx, 200h-5Ah
	rep	movsb
	pop	edi esi
; write bootsector
	push	ebx
	push	ebx
	push	ebx
	push	esi
	call	[SetFilePointer]
	test	eax, eax
	jnz	deverrl
	mov	eax, esp
	push	ebx
	push	eax
	push	512
	push	ebp
	push	esi
	call	[WriteFile]
	test	eax, eax
	jz	deverrl
	cmp	dword [esp], 512
	jnz	deverrl
; Patch backup copy of boot sector, ignore errors
	movzx	eax, word [ebp+50]
	test	eax, eax
	jz	done_succ
; sanity check: it must be in the reserved area, not in data
	cmp	ax, word [ebp+14]
	jae	done_succ
	shl	eax, 9
	push	ebx
	push	ebx
	push	eax
	push	esi
	call	[SetFilePointer]
	cmp	eax, -1
	jz	done_succ
	mov	eax, esp
	push	ebx
	push	eax
	push	512
	push	ebp
	push	esi
	call	[WriteFile]
; done!
done_succ:
	push	40h
	push	ok
	push	succ
	push	edi
	call	[MessageBoxA]
	push	ebx
	call	[PostQuitMessage]
r:
	pop	eax
	push	esi
	call	[CloseHandle]
	pop	ebp esi
	ret

section '.rdata' data readable

data resource from 'rsrc.res'
end data

ClassName db	'LISTBOX',0
WndName	db	'Select drive',0
deverr	db	'Cannot open physical device or device error (no administrator rights?)',0
nofat32	db	'Not FAT32 volume. Sorry, only FAT32 is supported at moment.',0
ok	db	'Success',0
succ	db	'Kolibri flash loader was successfully installed!',10
	db	'Now you can copy the image kolibri.img and boot!',0
mterr	db	'Cannot copy MTLD_F32',0
bterr	db	'Cannot load '
btname	db	'BOOT_F32.BIN',0

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
GetVolumeInformationA	dd	rva GetVolumeInformationA_thunk
CreateFileA		dd	rva CreateFileA_thunk
ReadFile		dd	rva ReadFile_thunk
WriteFile		dd	rva WriteFile_thunk
SetFilePointer		dd	rva SetFilePointer_thunk
CloseHandle		dd	rva CloseHandle_thunk
SetErrorMode		dd	rva SetErrorMode_thunk
CopyFileA		dd	rva CopyFileA_thunk
SetFileAttributesA	dd	rva SetFileAttributesA_thunk
DeviceIoControl		dd	rva DeviceIoControl_thunk
	dw	0
thunk GetLogicalDrives
thunk GetDriveTypeA
thunk GetVolumeInformationA
thunk CreateFileA
thunk ReadFile
thunk WriteFile
thunk SetFilePointer
thunk CloseHandle
thunk SetErrorMode
thunk CopyFileA
thunk SetFileAttributesA
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

a2_src:
	db	'\\.\'
	db	'?:',0,0
	db	'?:\'
	db	'MTLD_F32',0

section '.data' data readable writable

;a2	db	'\\.\'
;a	db	'?:',0,0
;mtldr_out	db	'?:\'
;mtldr_in	db	'MTLD_F32',0
a2	rb	4
a	rb	4
mtldr_out	rb	3
mtldr_in	rb	9

align 4
OldWndProc	dd	?
devpath		rb	1024
n		rb	1032
bootsect_dev	rb	512
bootsect_new	rb	512
