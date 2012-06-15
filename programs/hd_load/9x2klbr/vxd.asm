	.386p
WIN40COMPAT = 1
	include vmm.inc
	include v86mmgr.inc
	DECLARE_VIRTUAL_DEVICE LDKLBR,1,0,LDKLBR_Control,UNDEFINED_DEVICE_ID,1
	
;Begin_control_dispatch LDKLBR
;Control_Dispatch w32_DeviceIoControl, OnDeviceIoControl
;Control_Dispatch Sys_Dynamic_Device_Exit, OnExit
;End_control_dispatch LDKLBR

VxD_LOCKED_DATA_SEG
VkdControlProc	dd	0
vkdddb		dd	0
diskinfobuf:
	db	10h,0,0,0FFh
	db	0Ch dup (0)

oldidt label fword
	dw	03FFh
	dd	0

	include mtldr.inc

imgname	dd	0

VxD_LOCKED_DATA_ENDS

VxD_LOCKED_CODE_SEG

BeginProc NewControlProc
	cmp	eax, Reboot_Processor
	jz	short MyReboot
	jmp	[VkdControlProc]
EndProc NewControlProc

BeginProc MyReboot
	VMMCall _MapPhysToLinear,<0D000h,2000h,0>
	push	eax
	VMMCall	_MapPhysToLinear,<0,1000h,0>
	xchg	eax, ebx
	cli
	lea	esi, [ebx+53Ch]
	lodsd
	mov	[ebx+413h], ax
	shr	eax, 10h
	mov	[ebx+40Eh], ax
; restore BIOS IDT - vectors 00..1F
	mov	edi, ebx
	mov	ecx, 20h
	rep	movsd
; int 19
	mov	eax, [ebx+810h]
	mov	[ebx+64h], eax
; vectors 40,41,42,43,46,4B,4F
	lea	edi, [ebx+40h*4]
	movsd
	movsd
	movsd
	movsd
	scasd
	scasd
	movsd
	add	edi, 10h
	movsd
	add	edi, 0Ch
	movsd
; vectors 70..77
;	lea	esi, [ebx+5DCh]
	lea	edi, [ebx+70h*4]
	mov	ecx, 8
	rep	movsd

; reboot to mtldr
	mov	dword ptr [ebx+467h], 0D000007h	; 0D00:0007
	mov	al, 0Fh
	out	70h, al
	jecxz	$+2
	jecxz	$+2
	mov	al, 5
	out	71h, al
; copy mtldr code
	mov	esi, offset mtldr
;	mov	edi, 0D000h
	pop	edi
	push	edi
	mov	ecx, mtldr_size
	rep	movsb
; copy mtldr parameters
	mov	esi, [imgname]
	mov	edi, esi
	mov	al, 0
	xor	ecx, ecx
	dec	ecx
	repnz	scasb
	pop	edi
	not	ecx
	movzx	eax, word ptr [edi+5]
	add	edi, eax
	rep	movsb
; load old IDT
	lidt	[oldidt]
; reboot
	mov	al, 0FEh
	out	64h, al
	hlt
EndProc MyReboot

BeginProc LDKLBR_Control
	cmp	eax, w32_DeviceIoControl
	jz	short OnDeviceIoControl
	cmp	eax, Sys_Dynamic_Device_Exit
	jz	short OnExit
	cmp	eax, Reboot_Processor
	jz	MyReboot
	clc
	ret

OnExit:
; allow unload if and only if we are not hooking
	cmp	[VkdControlProc], 1
	cmc
	ret

OnDeviceIoControl:
	cmp	dword ptr [esi+12], DIOC_Open
	jz	@@open
	cmp	dword ptr [esi+12], 0Fh
	jnz	_exit
; request to set path of image
	mov	ecx, [esi+20]	; cbInBuffer
	cmp	ecx, 300
	ja	short @@paramerr
	test	ecx, ecx
	jnz	short @@param1ok
@@paramerr:
	xor	eax, eax
	inc	eax
@@errret:
	mov	ecx, [vkdddb]
	mov	edx, [VkdControlProc]
	mov	[ecx + VxD_Desc_Block.DDB_Control_Proc], edx
	mov	[VkdControlProc], 0
	ret
@@param1ok:
	mov	eax, [esi+16]	; lpvInBuffer
; set drive
	mov	dl, [eax]
	or	dl, 20h
	sub	dl, 60h
	jz	short @@paramerr
	cmp	dl, 'z'-60h
	ja	short @@paramerr
	push	esi
	Push_Client_State Uses_edi
	mov	ecx, 10h
	stc
	push	ds
	pop	fs
	mov	esi, offset diskinfobuf
	VMMCall Get_Cur_VM_Handle
	VxDCall V86MMGR_Allocate_Buffer
	VMMCall Begin_Nest_V86_Exec
	assume ebp:ptr Client_Reg_Struc
	mov	[ebp.Client_AX], 440Dh
	mov	[ebp.Client_BL], dl
	mov	[ebp.Client_CX], 086Fh
	mov	[ebp.Client_DX], di
	mov	eax, edi
	shr	eax, 10h
	mov	[ebp.Client_DS], ax
	mov	eax, 21h
	VMMCall	Exec_Int
	VMMCall End_Nest_Exec
	mov	ecx, 10h
	stc
	push	ds
	pop	fs
	VxDCall V86MMGR_Free_Buffer
	Pop_Client_State Uses_esi
	pop	esi
	mov	al, byte ptr [diskinfobuf+3]
	cmp	al, 0FFh
	jz	@@errret
	cmp	al, 80h
	jb	@@paramerr
	mov	byte ptr [mtldr+4], al
	mov	eax, dword ptr [diskinfobuf+8]
	mov	dword ptr [mtldr], eax
; set path
	mov	ecx, [imgname]
	jecxz	@f
	VMMCall _HeapFree, <ecx,0>
@@:
	mov	ecx, [esi+20]
	dec	ecx
	push	ecx
	VMMCall _HeapAllocate, <ecx,0>
	pop	ecx
	mov	[imgname], eax
	xchg	edi, eax
	mov	esi, [esi+16]
	inc	esi
@@1:
	lodsb
	cmp	al, 'A'
	jb	short @f
	cmp	al, 'Z'
	ja	short @f
	or	al, 20h
@@:
	stosb
	loop	@@1
	xor	eax, eax
	ret
@@open:
; don't hook if already hooked
	cmp	[VkdControlProc], 0
	jnz	short @f
	mov	eax, 0Dh
	VMMCall	Get_DDB
	mov	[vkdddb], ecx
	mov	eax, [ecx + VxD_Desc_Block.DDB_Control_Proc]
	mov	[VkdControlProc], eax
	mov	[ecx + VxD_Desc_Block.DDB_Control_Proc], NewControlProc
@@:
	xor	eax, eax
_exit:
	ret
EndProc LDKLBR_Control

VxD_LOCKED_CODE_ENDS

	end
