DWORD equ dword
WORD  equ word
BYTE  equ byte

PTR   equ

_mem_counter   equ (BOOT_VAR + 0x9100)
_mem_table     equ (BOOT_VAR + 0x9104)

_spinlock_initialize:
	mov	eax, DWORD PTR [esp+4]
	mov	DWORD PTR [eax], 0
	ret

_buddy_find_block:
	push	ebx
	mov	eax, DWORD PTR [eax+12]
	mov	ebx, ecx
	sub	edx, eax
	sar	edx, 2
	imul	ecx, edx, -858993459
	lea	eax, [eax+edx*4]
.L4:
	cmp	DWORD PTR [eax+12], ebx
        jne     .L7
	sub	ecx, 1
	sub	eax, 20
	cmp	ecx, -1
        jne     .L4
	xor	eax, eax
.L7:
	pop	ebx
	ret

@buddy_system_free@8:
	push	ebp
	mov	ebp, edx
	push	edi
	push	esi
	mov	esi, edx
	push	ebx
	sub	esp, 8
	mov	ebx, DWORD PTR [edx+12]
	mov	DWORD PTR [esp+4], ecx
	cmp	BYTE PTR [ecx+24], bl
	mov	edi, ebx
        je      .L17
	mov	edx, DWORD PTR [ecx+12]
	mov	DWORD PTR [esp], edx
        jmp     .L15
.L30:
	mov	eax, 1
	sal	eax, cl
	add	edx, eax
	mov	eax, DWORD PTR [esp+4]
	cmp	edx, DWORD PTR [eax+8]
        jae     .L17
.L31:
	lea	eax, [edx+edx*4]
	mov	edx, DWORD PTR [esp]
	lea	ecx, [edx+eax*4]
	cmp	ebx, DWORD PTR [ecx+12]
        jne     .L17
	mov	eax, DWORD PTR [ecx+8]
	test	eax, eax
        jne     .L17
	mov	eax, DWORD PTR [ecx+4]
	cmp	esi, ecx
	mov	edx, DWORD PTR [ecx]
	mov	DWORD PTR [esi+12], 255
	mov	DWORD PTR [ecx+12], 255
	mov	DWORD PTR [eax], edx
	mov	edx, DWORD PTR [ecx]
	mov	DWORD PTR [ecx], 0
	mov	DWORD PTR [edx+4], eax
	mov	edx, ebp
	mov	DWORD PTR [ecx+4], 0
        jb      .L26
	mov	edx, ecx
.L26:
	mov	ecx, edi
	mov	esi, edx
	movzx	eax, cl
	mov	ebp, edx
	lea	ebx, [eax+1]
	mov	eax, DWORD PTR [esp+4]
	mov	DWORD PTR [edx+12], ebx
	movzx	edi, BYTE PTR [eax+24]
	mov	edx, edi
	cmp	dl, bl
        je      .L17
	mov	edi, ebx
.L15:
	mov	eax, esi
	mov	ecx, ebx
	sub	eax, DWORD PTR [esp]
	sar	eax, 2
	imul	edx, eax, -858993459
	mov	eax, edx
	shr	eax, cl
	test	al, 1
        je      .L30
	mov	eax, 1
	mov	ecx, ebx
	sal	eax, cl
	sub	edx, eax
	mov	eax, DWORD PTR [esp+4]
	cmp	edx, DWORD PTR [eax+8]
        jb      .L31
.L17:
	mov	ecx, edi
	movzx	edx, cl
	mov	ecx, DWORD PTR [esp+4]
	mov	eax, DWORD PTR [ecx+28+edx*8]
	mov	DWORD PTR [ebp], eax
	lea	eax, [ecx+28+edx*8]
	mov	DWORD PTR [ebp+4], eax
	mov	eax, DWORD PTR [ecx+28+edx*8]
	mov	DWORD PTR [ecx+28+edx*8], ebp
	mov	DWORD PTR [eax+4], ebp
	add	esp, 8
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

@buddy_system_alloc_block@8:
	push	ebp
	mov	ebp, ecx
	push	edi
	mov	ecx, 255
	push	esi
	mov	eax, ebp
	push	ebx
	sub	esp, 4
	mov	DWORD PTR [esp], edx
	call	_buddy_find_block
	mov	ebx, eax
	mov	eax, DWORD PTR [eax+4]
	mov	edx, DWORD PTR [ebx]
	mov	DWORD PTR [eax], edx
	mov	edx, DWORD PTR [ebx]
	mov	DWORD PTR [ebx], 0
	mov	DWORD PTR [edx+4], eax
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [ebx+4], 0
	test	eax, eax
        jne     .L38
        jmp     .L34
.L35:
	mov	DWORD PTR [ebx+8], 1
	mov	edx, esi
	mov	ecx, ebp
	call	@buddy_system_free@8
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [ebx+8], 0
	test	eax, eax
        je      .L34
.L38:
	lea	ecx, [eax-1]
	mov	edx, DWORD PTR [esp]
	mov	eax, 20
	sal	eax, cl
	lea	edi, [ebx+eax]
	mov	eax, ebp
	mov	DWORD PTR [ebx+12], ecx
	mov	esi, edi
	mov	DWORD PTR [edi+12], ecx
	mov	ecx, 255
	call	_buddy_find_block
	cmp	edi, eax
        jne     .L35
	mov	esi, ebx
	mov	ebx, edi
        jmp     .L35
.L34:
	mov	DWORD PTR [ebx+8], 1
	mov	eax, ebx
	add	esp, 4
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

_zone_release:
	push	edi
	push	esi
	push	ebx
	mov	esi, DWORD PTR [esp+16]
	mov	eax, DWORD PTR [esp+20]
	mov	edi, DWORD PTR [esp+24]
	mov	edx, DWORD PTR [esi+4]
	add	edi, eax
	cmp	edi, edx
        jb      .L48
	mov	ebx, edx
	add	ebx, DWORD PTR [esi+8]
	cmp	eax, ebx
        ja      .L48
	cmp	eax, edx
	mov	ecx, eax
        jae     .L44
	mov	ecx, edx
.L44:
	cmp	edi, ebx
        jbe     .L45
	mov	edi, ebx
.L45:
	cmp	ecx, edi
        jae     .L48
	mov	ebx, ecx
.L47:
	mov	edx, DWORD PTR [esi+12]
	mov	eax, ecx
	add	ebx, 1
	sub	eax, DWORD PTR [esi+4]
	lea	eax, [eax+eax*4]
	mov	DWORD PTR [edx+8+eax*4], 0
	sub	ecx, DWORD PTR [esi+4]
	lea	edx, [ecx+ecx*4]
	mov	ecx, esi
	sal	edx, 2
	add	edx, DWORD PTR [esi+12]
	call	@buddy_system_free@8
	cmp	edi, ebx
	mov	ecx, ebx
        ja      .L47
.L48:
	pop	ebx
	pop	esi
	pop	edi
	ret

_zone_reserve:
	push	edi
	push	esi
	push	ebx
	mov	esi, DWORD PTR [esp+16]
	mov	eax, DWORD PTR [esp+20]
	mov	ebx, DWORD PTR [esp+24]
	mov	edx, DWORD PTR [esi+4]
	add	ebx, eax
	cmp	ebx, edx
        jb      .L61
	mov	ecx, edx
	add	ecx, DWORD PTR [esi+8]
	cmp	eax, ecx
        ja      .L61
	cmp	eax, edx
        jae     .L54
	mov	eax, edx
.L54:
	cmp	ebx, ecx
	mov	edi, ebx
        jbe     .L55
	mov	edi, ecx
.L55:
	cmp	eax, edi
        jae     .L61
	mov	ebx, eax
        jmp     .L57
.L58:
	add	ebx, 1
	cmp	edi, ebx
	mov	eax, ebx
        jbe     .L61
.L62:
	mov	edx, DWORD PTR [esi+4]
.L57:
	sub	eax, edx
	lea	eax, [eax+eax*4]
	lea	edx, [0+eax*4]
	add	edx, DWORD PTR [esi+12]
	mov	ecx, DWORD PTR [edx+8]
	test	ecx, ecx
        jne     .L58
	add	ebx, 1
	mov	ecx, esi
	call	@buddy_system_alloc_block@8
	mov	eax, ebx
	sub	DWORD PTR [esi+16], 1
	cmp	edi, ebx
        ja      .L62
.L61:
	pop	ebx
	pop	esi
	pop	edi
	ret

@buddy_system_alloc@8:
	sub	esp, 12
	mov	DWORD PTR [esp], ebx
	lea	eax, [ecx+28+edx*8]
	mov	ebx, edx
	mov	DWORD PTR [esp+4], esi
	mov	esi, ecx
	mov	DWORD PTR [esp+8], edi
	cmp	DWORD PTR [ecx+32+edx*8], eax
        je      .L64
	mov	ecx, DWORD PTR [ecx+32+edx*8]
	mov	eax, DWORD PTR [ecx+4]
	mov	edx, DWORD PTR [ecx]
	mov	DWORD PTR [ecx+8], 1
	mov	DWORD PTR [eax], edx
	mov	edx, DWORD PTR [ecx]
	mov	DWORD PTR [ecx], 0
	mov	DWORD PTR [edx+4], eax
	mov	DWORD PTR [ecx+4], 0
.L66:
	mov	ebx, DWORD PTR [esp]
	mov	eax, ecx
	mov	esi, DWORD PTR [esp+4]
	mov	edi, DWORD PTR [esp+8]
	add	esp, 12
	ret
.L64:
	movzx	eax, BYTE PTR [ecx+24]
	cmp	eax, edx
        jne     .L71
.L67:
	xor	ecx, ecx
        jmp     .L66
.L71:
	lea	edx, [edx+1]
	call	@buddy_system_alloc@8
	test	eax, eax
	mov	edi, eax
        je      .L67
	mov	ecx, DWORD PTR [eax+12]
	mov	edx, 20
	mov	DWORD PTR [eax+12], ebx
	mov	DWORD PTR [eax+8], 1
	sub	ecx, 1
	sal	edx, cl
	mov	ecx, esi
	add	edx, eax
	mov	DWORD PTR [edx+12], ebx
	call	@buddy_system_free@8
	mov	ecx, edi
        jmp     .L66

_zone_frame_alloc:
	sub	esp, 8
	mov	ecx, eax
	mov	DWORD PTR [esp], ebx
	mov	ebx, eax
	mov	DWORD PTR [esp+4], esi
	mov	esi, edx
	call	@buddy_system_alloc@8
	mov	ecx, esi
	mov	edx, 1
	sal	edx, cl
	mov	esi, DWORD PTR [esp+4]
	sub	DWORD PTR [ebx+16], edx
	sub	eax, DWORD PTR [ebx+12]
	add	DWORD PTR [ebx+20], edx
	mov	ebx, DWORD PTR [esp]
	add	esp, 8
	sar	eax, 2
	imul	eax, eax, -858993459
	ret

_frame_set_parent:
	mov	eax, DWORD PTR [esp+4]
        sub     eax, DWORD PTR [_z_core+4]
	mov	ecx, DWORD PTR [esp+8]
        mov     edx, DWORD PTR [_z_core+12]
	lea	eax, [eax+eax*4]
	mov	DWORD PTR [edx+16+eax*4], ecx
	ret

@zone_free@8:
	sub	esp, 8
	lea	edx, [edx+edx*4]
	mov	DWORD PTR [esp], ebx
	sal	edx, 2
	mov	ebx, ecx
	mov	DWORD PTR [esp+4], esi
	add	edx, DWORD PTR [ecx+12]
	mov	eax, DWORD PTR [edx+8]
	mov	esi, DWORD PTR [edx+12]
	sub	eax, 1
	test	eax, eax
	mov	DWORD PTR [edx+8], eax
        jne     .L79
	call	@buddy_system_free@8
	mov	eax, 1
	mov	ecx, esi
	sal	eax, cl
	add	DWORD PTR [ebx+16], eax
	sub	DWORD PTR [ebx+20], eax
.L79:
	mov	ebx, DWORD PTR [esp]
	mov	esi, DWORD PTR [esp+4]
	add	esp, 8
	ret


@zone_alloc@8:
	sub	esp, 8
	mov	DWORD PTR [esp], ebx
	mov	ebx, ecx
	mov	DWORD PTR [esp+4], esi

	pushf
	pop esi
	cli

@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

	mov	eax, ecx
	call	_zone_frame_alloc
	mov	edx, DWORD PTR [ebx+4]
	mov	DWORD PTR [ebx], 0

	push esi
	popf

	mov	ebx, DWORD PTR [esp]
	add	eax, edx
	mov	esi, DWORD PTR [esp+4]
	sal	eax, 12
	add	esp, 8
	ret

alloc_page:
_alloc_page:
	push	ebx

	pushf
	pop ebx
	cli
@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

        push edx
	xor	edx, edx
        mov     eax, _z_core
	call	_zone_frame_alloc
        pop edx
        mov     [_z_core], 0

	push ebx
	popf

	pop	ebx
	sal	eax, 12
	ret

alloc_pages:
_alloc_pages@4:
	push	ebx

	pushf
	pop ebx
	cli
@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

	mov	eax, DWORD PTR [esp+8]
	add	eax, 7
	and	eax, -8

	xor ecx, ecx
	bsr ecx, eax
        inc ecx

        push edx

        mov     edx, ecx
        mov     eax, _z_core
	call	_zone_frame_alloc
        pop edx

        mov     [_z_core], 0
	push ebx
	popf

	pop	ebx
	sal	eax, 12
	ret	4

_zone_create:
	push	esi
	push	ebx
	sub	esp, 4
	mov	ebx, DWORD PTR [esp+16]
	mov	esi, DWORD PTR [esp+24]
	mov	DWORD PTR [esp], ebx
	call	_spinlock_initialize
	mov	eax, DWORD PTR [esp+20]
	mov	DWORD PTR [ebx+8], esi
	mov	DWORD PTR [ebx+16], esi
	mov	DWORD PTR [ebx+20], 0
	mov	DWORD PTR [ebx+4], eax

        xor eax, eax
	bsr eax, esi

        xor     edx, edx
	mov	BYTE PTR [ebx+24], al
.L81:
	lea	eax, [ebx+28+edx*8]
	mov	DWORD PTR [ebx+28+edx*8], eax
	mov	DWORD PTR [ebx+32+edx*8], eax
	movzx	eax, BYTE PTR [ebx+24]
	add	edx, 1
	cmp	eax, edx
        jae     .L81
	lea	ecx, [esi+esi*4]
	sal	ecx, 2
	call	@balloc@4
	test	esi, esi
	mov	DWORD PTR [ebx+12], eax
        je      .L83
	xor	ecx, ecx
	xor	edx, edx
.L85:
	mov	eax, edx
	add	ecx, 1
	add	eax, DWORD PTR [ebx+12]
	add	edx, 20
	cmp	ecx, esi
	mov	DWORD PTR [eax+8], 1
	mov	DWORD PTR [eax+12], 0
        jne     .L85
.L83:
	add	esp, 4
	mov	eax, 1
	pop	ebx
	pop	esi
	ret

_init_mm:
	push	ebx
	sub	esp, 24
        mov     eax, DWORD PTR [_mem_amount]
        mov     DWORD PTR [esp], .LC3
	mov	ebx, eax
	shr	ebx, 12
	mov	DWORD PTR [esp+8], ebx
	mov	DWORD PTR [esp+4], eax
	call	_printf
        mov     eax, DWORD PTR [_pg_balloc]
        mov     DWORD PTR [esp], .LC4
	mov	DWORD PTR [esp+8], eax
	lea	eax, [ebx+ebx*4]
	sal	eax, 2
	mov	DWORD PTR [esp+4], eax
	call	_printf
	mov	DWORD PTR [esp+8], ebx
	mov	DWORD PTR [esp+4], 0
        mov     DWORD PTR [esp], _z_core
	call	_zone_create
	mov	DWORD PTR [esp+8], ebx
	mov	DWORD PTR [esp+4], 0
        mov     DWORD PTR [esp], _z_core
	call	_zone_release
        mov     eax, DWORD PTR [_pg_balloc]
	mov	DWORD PTR [esp+4], 0
        mov     DWORD PTR [esp], _z_core
	shr	eax, 12
	mov	DWORD PTR [esp+8], eax
	call	_zone_reserve
	add	esp, 24
	pop	ebx
        ret
.LC3:
        db "last page = %x total pages =  %x",10,0
.LC4:
        db "conf_size = %x  free mem start =%x",10,0


_frame_free:
	push	ebx
	mov	edx, DWORD PTR [esp+8]

	pushf
	pop ebx
	cli
@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

        mov     ecx, _z_core
	shr	edx, 12
	call	@zone_free@8
        mov     [_z_core], 0

	push ebx
	popf

	pop	ebx
	ret

_core_free:
	push	ebx
	mov	edx, DWORD PTR [esp+8]

	pushf
	pop ebx
	cli
@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

        mov     ecx, _z_core
	shr	edx, 12
	call	@zone_free@8
        mov     [_z_core], 0

	push ebx
	popf

	pop	ebx
	ret

_core_alloc:
	push	ebx
	pushf
	pop ebx
	cli
@@:
        pause
        mov eax, [_z_core]
        test eax, eax
        jnz @b
        inc eax
        xchg [_z_core], eax
        test eax, eax
        jnz @b

	mov	edx, DWORD PTR [esp+8]
        mov     eax, _z_core
	call	_zone_frame_alloc
        mov     [_z_core], 0

	push ebx
	popf

	pop	ebx
	sal	eax, 12
	ret


restore DWORD
restore WORD
restore BYTE

restore PTR

