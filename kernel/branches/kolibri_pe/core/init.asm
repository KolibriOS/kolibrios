DWORD equ dword
WORD  equ word
BYTE  equ byte

PTR   equ

_init:
	push	esi
	push	ebx
	sub	esp, 4
        mov     eax, DWORD PTR [_boot_mbi]
	test	BYTE PTR [eax], 2
        je      .L2
	push	ecx
	push	ecx
	push	DWORD PTR [eax+12]
        push    .LC0
	call	_printf
	add	esp, 16
.L2:
        mov     eax, DWORD PTR [_boot_mbi]
	test	BYTE PTR [eax], 4
	je	L4
	push	edx
	push	edx
	push	DWORD PTR [eax+16]
        push    .LC1
	call	_printf
	add	esp, 16
.L4:
        mov     eax, DWORD PTR [_boot_mbi]
	test	BYTE PTR [eax], 8
        je      .L6
	push	esi
	xor	esi, esi
	push	DWORD PTR [eax+24]
	push	DWORD PTR [eax+20]
        push    .LC2
	call	_printf
        mov     eax, DWORD PTR [_boot_mbi]
	mov	ebx, DWORD PTR [eax+24]
        jmp     .L22
.L9:
	mov	eax, DWORD PTR [ebx-12]
	inc	esi
	push	DWORD PTR [ebx-8]
        mov     DWORD PTR [_pg_balloc], eax
	push	eax
	push	DWORD PTR [ebx-16]
        push    .LC3
	call	_printf
.L22:
        mov     eax, DWORD PTR [_boot_mbi]
	add	esp, 16
	mov	edx, ebx
	add	ebx, 16
	cmp	esi, DWORD PTR [eax+20]
        jb      .L9
	mov	edx, DWORD PTR [edx-16]
	push	ebx
	push	ebx
	sub	edx, 536870912
	lea	eax, [edx+512]
        mov     DWORD PTR [_rd_fat], eax
	lea	eax, [edx+4790]
	push	edx
        push    .LC4
        mov     DWORD PTR [_rd_fat_end], eax
	lea	eax, [edx+9728]
        mov     DWORD PTR [_rd_root], eax
	lea	eax, [edx+16896]
        mov     DWORD PTR [_rd_base], edx
        mov     DWORD PTR [_rd_root_end], eax
	call	_printf
	add	esp, 16
.L6:
        mov     eax, DWORD PTR [_boot_mbi]
	xor	esi, esi
	test	BYTE PTR [eax], 64
        je      .L13
	push	ecx
	push	DWORD PTR [eax+44]
	push	DWORD PTR [eax+48]
        push    .LC5
	call	_printf
        mov     eax, DWORD PTR [_boot_mbi]
	add	esp, 16
	mov	ebx, DWORD PTR [eax+48]
        jmp     .L14
.L15:
	push	edx
	push	DWORD PTR [ebx+20]
	push	DWORD PTR [ebx+12]
	push	DWORD PTR [ebx+16]
	push	DWORD PTR [ebx+4]
	push	DWORD PTR [ebx+8]
	push	DWORD PTR [ebx]
        push    .LC6
	call	_printf
	add	esp, 32
	cmp	DWORD PTR [ebx+20], 1
        jne     .L16
	mov	eax, DWORD PTR [ebx+4]
	add	eax, DWORD PTR [ebx+12]
	and	eax, -4096
	cmp	esi, eax
        jae     .L16
	mov	esi, eax
.L16:
	mov	eax, DWORD PTR [ebx]
	add	eax, 4
	lea	ebx, [eax+ebx]
.L14:
        mov     edx, DWORD PTR [_boot_mbi]
	mov	eax, DWORD PTR [edx+48]
	add	eax, DWORD PTR [edx+44]
	cmp	ebx, eax
        jb      .L15
	cmp	esi, 268435456
        jbe     .L13
	mov	esi, 268435456
.L13:
        mov     DWORD PTR [_mem_amount], esi
	pop	eax
	pop	ebx
	pop	esi
        ret


.LC0:
        db "boot_device = 0x%x",10,0
.LC1:
        db "cmdline = %s",10,0
.LC2:
        db "mods_count = %d, mods_addr = 0x%x",10,0
.LC3:
        db " mod_start = 0x%x, mod_end = 0x%x, string = %s",10,0
.LC4:
        db " rd_base = %x",10,0
.LC5:
        db "mmap_addr = 0x%x, mmap_length = 0x%x",10,0
.LC6:
        db " size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x",10,0


restore DWORD
restore WORD
restore BYTE

restore PTR

