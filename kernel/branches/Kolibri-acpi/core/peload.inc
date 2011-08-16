;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

include 'export.inc'

align 4

proc load_PE stdcall, file_name:dword
           locals
             image  dd ?
             entry  dd ?
             base   dd ?
           endl

           stdcall load_file, [file_name]
           test eax, eax
           jz .fail

           mov [image], eax

           mov edx, [eax+60]

           stdcall kernel_alloc, [eax+80+edx]
           test eax, eax
           jz .cleanup

           mov [base], eax

           stdcall map_PE, eax, [image]

           mov [entry], eax
           test eax, eax
           jnz .cleanup

           stdcall kernel_free, [base]
.cleanup:
           stdcall kernel_free, [image]
           mov eax, [entry]
           ret
.fail:
           xor eax, eax
           ret
endp

DWORD equ dword
PTR   equ

align 4
map_PE:                    ;stdcall base:dword, image:dword
           cld
        push    ebp
	push	edi
	push	esi
	push	ebx
	sub	esp, 60
	mov	ebx, DWORD PTR [esp+84]
	mov	ebp, DWORD PTR [esp+80]
	mov	edx, ebx
	mov	esi, ebx
	add	edx, DWORD PTR [ebx+60]
	mov	edi, ebp
	mov	DWORD PTR [esp+32], edx
	mov	ecx, DWORD PTR [edx+84]

	shr ecx, 2
	rep movsd

	movzx	eax, WORD PTR [edx+6]
	mov	DWORD PTR [esp+36], 0
	mov	DWORD PTR [esp+16], eax
	jmp	L2
L3:
	mov	eax, DWORD PTR [edx+264]
	test	eax, eax
	je	L4
	mov	esi, ebx
	mov	edi, ebp
	add	esi, DWORD PTR [edx+268]
	mov	ecx, eax
	add	edi, DWORD PTR [edx+260]

	shr ecx, 2
	rep movsd

L4:
	mov	ecx, DWORD PTR [edx+256]
	add	ecx, 4095
	and	ecx, -4096
	cmp	ecx, eax
	jbe	L6
	sub	ecx, eax
	add	eax, DWORD PTR [edx+260]
	lea	edi, [eax+ebp]

	xor eax, eax
	rep stosb

L6:
	inc	DWORD PTR [esp+36]
	add	edx, 40
L2:
	mov	esi, DWORD PTR [esp+16]
	cmp	DWORD PTR [esp+36], esi
	jne	L3
	mov	edi, DWORD PTR [esp+32]
	cmp	DWORD PTR [edi+164], 0
	je	L9
	mov	esi, ebp
	mov	ecx, ebp
	sub	esi, DWORD PTR [edi+52]
	add	ecx, DWORD PTR [edi+160]
	mov	eax, esi
	shr	eax, 16
	mov	DWORD PTR [esp+12], eax
	jmp	L11
L12:
	lea	ebx, [eax-8]
	xor	edi, edi
        shr     ebx,1
	jmp	L13
L14:
	movzx	eax, WORD PTR [ecx+8+edi*2]
	mov	edx, eax
	shr	eax, 12
	and	edx, 4095
	add	edx, DWORD PTR [ecx]
	cmp	ax, 2
	je	L17
	cmp	ax, 3
	je	L18
	dec	ax
	jne	L15
	mov	eax, DWORD PTR [esp+12]
	add	WORD PTR [edx+ebp], ax
L17:
	add	WORD PTR [edx+ebp], si
L18:
	add	DWORD PTR [edx+ebp], esi
L15:
	inc	edi
L13:
	cmp	edi, ebx
	jne	L14
	add	ecx, DWORD PTR [ecx+4]
L11:
	mov	eax, DWORD PTR [ecx+4]
	test	eax, eax
	jne	L12
L9:
	mov	edx, DWORD PTR [esp+32]
	cmp	DWORD PTR [edx+132], 0
	je	L20
	mov	eax, ebp
	add	eax, DWORD PTR [edx+128]
	mov	DWORD PTR [esp+40], 0
	add	eax, 20
	mov	DWORD PTR [esp+56], eax
L22:
	mov	ecx, DWORD PTR [esp+56]
	cmp	DWORD PTR [ecx-16], 0
	jne	L23
	cmp	DWORD PTR [ecx-8], 0
	je	L25
L23:
        mov     edi, DWORD PTR [__exports+32]
        mov     esi, DWORD PTR [__exports+28]
	mov	eax, DWORD PTR [esp+56]
	mov	DWORD PTR [esp+20], edi
        add     edi, OS_BASE
        add     esi, OS_BASE
	mov	DWORD PTR [esp+44], esi
	mov	ecx, DWORD PTR [eax-4]
	mov	DWORD PTR [esp+48], edi
	mov	edx, DWORD PTR [eax-20]
	mov	DWORD PTR [esp+52], 0
	add	ecx, ebp
	add	edx, ebp
	mov	DWORD PTR [esp+24], edx
	mov	DWORD PTR [esp+28], ecx
L26:
	mov	esi, DWORD PTR [esp+52]
	mov	edi, DWORD PTR [esp+24]
	mov	eax, DWORD PTR [edi+esi*4]
	test	eax, eax
	je	L27
	test	eax, eax
	js	L27
	lea	edi, [ebp+eax]
	mov	eax, DWORD PTR [esp+28]
	mov	DWORD PTR [eax+esi*4], 0
	lea	esi, [edi+2]
	push	eax
	push	32
	movzx	eax, WORD PTR [edi]
	mov	edx, DWORD PTR [esp+56]
	mov	eax, DWORD PTR [edx+eax*4]
        add     eax, OS_BASE
	push	eax
	push	esi
	call	strncmp
	pop	ebx
	xor	ebx, ebx
	test	eax, eax
	jne	L32
	jmp	L30
L33:
	push	ecx
	push	32
	mov	ecx, DWORD PTR [esp+28]
        mov     eax, DWORD PTR [ecx+OS_BASE+ebx*4]
        add     eax, OS_BASE
	push	eax
	push	esi
	call	strncmp
	pop	edx
	test	eax, eax
	jne	L34
	mov	esi, DWORD PTR [esp+44]
	mov	edx, DWORD PTR [esp+52]
	mov	ecx, DWORD PTR [esp+28]
	mov	eax, DWORD PTR [esi+ebx*4]
        add     eax, OS_BASE
	mov	DWORD PTR [ecx+edx*4], eax
	jmp	L36
L34:
	inc	ebx
L32:
        cmp     ebx, DWORD PTR [__exports+24]
	jb	L33
L36:
        cmp     ebx, DWORD PTR [__exports+24]
	jne	L37

        mov     esi, msg_unresolved
        call    sys_msg_board_str
	lea	esi, [edi+2]
        call    sys_msg_board_str
        mov     esi, msg_CR
        call    sys_msg_board_str

	mov	DWORD PTR [esp+40], 1
	jmp	L37
L30:
	movzx	eax, WORD PTR [edi]
	mov	esi, DWORD PTR [esp+44]
	mov	edi, DWORD PTR [esp+52]
	mov	edx, DWORD PTR [esp+28]
	mov	eax, DWORD PTR [esi+eax*4]
        add     eax, OS_BASE
	mov	DWORD PTR [edx+edi*4], eax
L37:
	inc	DWORD PTR [esp+52]
	jmp	L26
L27:
	add	DWORD PTR [esp+56], 20
	jmp	L22
L25:
	xor	eax, eax
	cmp	DWORD PTR [esp+40], 0
	jne	L40
L20:
	mov	ecx, DWORD PTR [esp+32]
	mov	eax, ebp
	add	eax, DWORD PTR [ecx+40]
L40:
	add	esp, 60
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
        ret     8

 align 16
__exports:
          export 'KERNEL',   \
          alloc_kernel_space,    'AllocKernelSpace',   \      ; stdcall
          alloc_page,            'AllocPage',          \      ; gcc ABI
          alloc_pages,           'AllocPages',         \      ; stdcall
          commit_pages,          'CommitPages',        \      ; eax, ebx, ecx
\
          create_event,          'CreateEvent',        \      ; ecx, esi
          raise_event,           'RaiseEvent',         \      ; eax, ebx, edx, esi
          wait_event,            'WaitEvent',          \      ; eax, ebx
          get_event_ex,          'GetEvent',           \      ; edi
\
          create_kernel_object,  'CreateObject',       \
          create_ring_buffer,    'CreateRingBuffer',   \      ; stdcall
          destroy_kernel_object, 'DestroyObject',      \
          free_kernel_space,     'FreeKernelSpace',    \      ; stdcall
          kernel_alloc,          'KernelAlloc',        \      ; stdcall
          kernel_free,           'KernelFree',         \      ; stdcall
          malloc,                'Kmalloc',            \
          free,                  'Kfree',              \
          map_io_mem,            'MapIoMem',           \      ; stdcall
          get_pg_addr,           'GetPgAddr',          \      ; eax
\
          mutex_init,            'MutexInit',          \      ; gcc fastcall
          mutex_lock,            'MutexLock',          \      ; gcc fastcall
          mutex_unlock,          'MutexUnlock',        \      ; gcc fastcall
\
          get_display,           'GetDisplay',         \
          set_screen,            'SetScreen',          \
          pci_api_drv,           'PciApi',             \
          pci_read8,             'PciRead8',           \      ; stdcall
          pci_read16,            'PciRead16',          \      ; stdcall
          pci_read32,            'PciRead32',          \      ; stdcall
          pci_write8,            'PciWrite8',          \      ; stdcall
          pci_write16,           'PciWrite16',         \      ; stdcall
          pci_write32,           'PciWrite32',         \      ; stdcall
\
          get_service,           'GetService',         \      ;
          reg_service,           'RegService',         \      ; stdcall
          attach_int_handler_ex, 'AttachIntHandler',   \      ; stdcall
          user_alloc,            'UserAlloc',          \      ; stdcall
          user_free,             'UserFree',           \      ; stdcall
          unmap_pages,           'UnmapPages',         \      ; eax, ecx
          sys_msg_board_str,     'SysMsgBoardStr',     \
          get_timer_ticks,       'GetTimerTicks',      \
          get_stack_base,        'GetStackBase',       \
          delay_hs,              'Delay',              \      ; ebx
          set_mouse_data,        'SetMouseData',       \      ;
          set_keyboard_data,     'SetKeyboardData'            ; gcc fastcall



