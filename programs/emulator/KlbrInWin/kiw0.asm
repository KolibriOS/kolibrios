; kiw0.sys - KlbrInWin ring-0 component
; (c) diamond, 2006, 2007, 2008
; Cb-n#%li.-# @l$i Lkbnbe
DRIVER_VERSION = 1
	format PE native
section '.text' code data readable writable executable
;section '.text' code readable executable
	entry start
start:
	push	eax
	push	esp
	push	0
	push	0
	push	22h	; FILE_DEVICE_UNKNOWN
	push	devname
	push	0
	push	dword [esp+20h]
	call	[IoCreateDevice]
	pop	ecx
	test	eax, eax
	js	.ret
	push	ecx
	push	devname
	push	symlinkname
	call	[IoCreateSymbolicLink]
	pop	ecx
	test	eax, eax
	jns	@f
	push	eax
	push	ecx
	call	[IoDeleteDevice]
	pop	eax
	jmp	.ret
@@:
	mov	eax, [esp+4]
	mov	dword [eax+38h], DispatchCreateClose
	mov	dword [eax+40h], DispatchCreateClose
	mov	dword [eax+70h], DispatchControl
	mov	dword [eax+34h], DriverUnload
	push	'kiw0'
	push	2000h
	push	0
	call	[ExAllocatePoolWithTag]
	mov	[oldiomap], eax
	push	eax
	push	1
	call	[Ke386QueryIoAccessMap]
	xor	eax, eax
.ret:
	ret	8

DriverUnload:
	push	symlinkname
	call	[IoDeleteSymbolicLink]
	mov	eax, [esp+4]
	push	dword [eax+4]
	call	[IoDeleteDevice]
	push	[oldiomap]
	push	1
	call	[Ke386SetIoAccessMap]
	push	[oldiomap]
	call	[ExFreePool]
	ret	4

DispatchCreateClose:
	mov	ecx, [esp+8]
	xor	edx, edx
	mov	[ecx+18h], edx
	mov	[ecx+1Ch], edx
	call	[IofCompleteRequest]
	xor	eax, eax
	ret	8

DispatchControl:
	mov	ecx, [esp+8]
	mov	eax, [ecx+60h]
	cmp     dword [eax+0Ch], 0x222000
	jz      .readmsr
	cmp	dword [eax+0Ch], 0x222004
	jz	.readpci
	cmp	dword [eax+0Ch], 0x222008
	jz	.getversion
	cmp	dword [eax+0Ch], 0x22203C
	jnz	.invreq
	cmp	dword [eax+8], 2000h
	jnz	.invreq
	push	ecx
	push	dword [ecx+0Ch]
	push	1
	call	[Ke386SetIoAccessMap]
	push	1
	call	[IoGetCurrentProcess]
	push	eax
	call	[Ke386IoSetAccessProcess]
	str     ax
	sub     esp, 6
	sgdt    [esp]
	pop     cx
	pop     ecx
	movzx   eax, ax
	mov     dh, [eax+ecx+7]
	mov     dl, [eax+ecx+4]
	shl     edx, 16
	mov     dx, [eax+ecx+2]
	mov     word [edx+66h], 88h
	pop	ecx
	xor	eax, eax
	mov	[ecx+1Ch], eax
	jmp	.ret
.getversion:
	cmp	dword [eax+4], 4
	jnz	.invreq
	mov	eax, [ecx+0Ch]
	mov	dword [eax], DRIVER_VERSION
	mov	dword [ecx+1Ch], 4
	jmp	.ret0
.readmsr:
        cmp     dword [eax+8], 4
        jnz     .invreq
        cmp     dword [eax+4], 9
        jnz     .invreq
        push	ecx
        mov     ecx, [ecx+0Ch]
        mov     byte [ecx+8], 0         ; assume OK
; rdmsr may throw exception
        push    .exception_handler
        push    dword [fs:0]
        mov     [fs:0], esp
        push    ecx
        mov     ecx, [ecx]
        rdmsr
        pop     ecx
        mov     [ecx], eax
        mov     [ecx+4], edx
.msr_common:
        pop     dword [fs:0]
        pop     ecx
        pop	ecx
        mov	dword [ecx+1Ch], 9
.ret0:
        xor	eax, eax
        jmp     .ret
.invreq2:
	pop	ecx
.invreq:
	mov	eax, 0xC0000010		; STATUS_INVALID_DEVICE_REQUEST
.ret:
	push	eax
	mov	[ecx+18h], eax
	xor	edx, edx
	call	[IofCompleteRequest]
	pop	eax
	ret	8
.exception_handler:
        mov     eax, [esp+12]
        mov     dword [eax+0xB8], .no_msr
        xor     eax, eax
        ret
.no_msr:
        pop     ecx
        mov     byte [ecx+8], 1
        jmp     .msr_common
.readpci:
	cmp	dword [eax+8], 4
	jnz	.invreq
	cmp	dword [eax+4], 4
	jnz	.invreq
	push	ecx
	mov	ecx, [ecx+0Ch]
	movzx	eax, byte [ecx]
	cmp	al, 2
	ja	.invreq2
	jb	@f
	inc	eax
@@:
	test	byte [ecx+2], al
	jnz	.readpci.unaligned
	inc	eax
	push	eax
	push	eax			; Length
	movzx	eax, byte [ecx+2]
	push	eax			; Offset
	push	ecx			; Buffer
	movzx	eax, byte [ecx+3]
	ror	al, 3
	push	eax			; SlotNumber
	movzx	eax, byte [ecx+1]
	push	eax			; BusNumber
	push	4	; PCIConfiguration
	or	dword [ecx], -1
	call	[HalGetBusDataByOffset]
	pop	edx
	pop	ecx
	mov	dword [ecx+1Ch], edx
	jmp	.ret0
.readpci.unaligned:
	or	dword [ecx], -1
	pop	ecx
	mov	dword [ecx+1Ch], 4
	jmp	.ret0

include 'd:\program files\fasm\fasmw16723\include\win32a.inc'
data import
	library ntoskrnl,'ntoskrnl.exe',hal,'hal.dll'
	import ntoskrnl, \
IoCreateDevice, 'IoCreateDevice', \
IoCreateSymbolicLink, 'IoCreateSymbolicLink', \
IoDeleteDevice, 'IoDeleteDevice', \
IoDeleteSymbolicLink, 'IoDeleteSymbolicLink', \
IoGetCurrentProcess, 'IoGetCurrentProcess', \
Ke386QueryIoAccessMap, 'Ke386QueryIoAccessMap', \
Ke386SetIoAccessMap, 'Ke386SetIoAccessMap', \
Ke386IoSetAccessProcess, 'Ke386IoSetAccessProcess', \
IofCompleteRequest, 'IofCompleteRequest', \
ExAllocatePoolWithTag, 'ExAllocatePoolWithTag', \
ExFreePool, 'ExFreePool'
	import hal, HalGetBusDataByOffset, 'HalGetBusDataByOffset'
end data

str1	db	'control code 0x%X',13,10,0
str2	db	'kiw0 loaded',13,10,0

devname:
	dw	12*2
	dw	13*2
	dd	@f
@@	du	'\Device\kiw0',0
symlinkname:
	dw	16*2
	dw	17*2
	dd	@f
@@	du	'\DosDevices\kiw0',0

data fixups
end data

;section '.data' data readable writable
oldiomap	dd	?
