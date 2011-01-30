; --------------------------------------------------------------------------
; FILE: TRandom.Asm
; DATE: October 4, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_Create:
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, TRandom.size
    mov     [glb_pMersenne], eax

    mcZeroBits ecx
    mov     [eax + TRandom.rand_NextIndex], ecx

    rdtsc
    call    TRandom_Seed
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = seed value
; --------------------------------------------------------------------------
virtual at 0
loc7:
    .index INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_Seed:
    mcBeginLocals loc7.size

    mcLoad1 ecx
    mcStoreLocal loc7.index, ecx

    mov     edi, [glb_pMersenne]
    stosd

.set_next_value:
    ;
    ; MT [i-1]
    ;
    mov     edx, [edi - 4]
    ;
    ; Right shift by 30 bits
    ;
    mov     eax, edx
    shr     eax, 30
    ;
    ; XOR it with MT [i-1]
    ;
    xor     edx, eax
    ;
    ; Add current index
    ;
    add     edx, [esp + loc7.index]
    ;
    ; Multiply by 6C078965h
    ;
    mov     eax, 6C078965h
    mul     edx
    ;
    ; Store as MT [i]
    ;
    stosd
    ;
    ; Continue until index reaches 623
    ;
    inc     [esp + loc7.index]
    cmp     [esp + loc7.index], 623
    jbe     .set_next_value

    mcEndLocals loc7.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = value
; Output:
;   EAX = value MOD 624
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_GetModulo624:
    push    ecx edx
    mcZeroBits edx
    mov     ecx, 624
    div     ecx
    mov     eax, edx
    pop     edx ecx
    ret

; --------------------------------------------------------------------------
virtual at 0
loc8:
    .index INDEX ?
    .y UINT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_Generate624:
    mcBeginLocals loc8.size
    ;
    ; i=0
    ;
    mcZeroBits eax
    mcStoreLocal loc8.index, eax
    ;
    ; MT array
    ;
    mov     esi, [glb_pMersenne]

.calc_y:
    ;
    ; MT [(i+1) MOD 624]
    ;
    mcLoadLocal eax, loc8.index
    inc     eax
    call    TRandom_GetModulo624
    mov     ecx, [esi + eax*4]
    ;
    ; Get last 31 bits of ECX
    ;
    and     ecx, 7FFFFFFFh
    ;
    ; 32nd bit of MT [i]
    ;
    mcLoadLocal eax, loc8.index
    mov     edx, [esi + eax*4]
    and     edx, 80000000h
    ;
    ; Combine them and store as Y
    ;
    or      edx, ecx
    mcStoreLocal loc8.y, edx
    ;
    ; MT [i] = MT [(i+397) MOD 624] xor (y >> 1)
    ;
    mcLoadLocal eax, loc8.index
    add     eax, 397
    call    TRandom_GetModulo624
    mov     ecx, [esi + eax*4]

    shr     edx, 1
    xor     ecx, edx
    mcLoadLocal eax, loc8.index
    mov     [esi + eax*4], ecx
    ;
    ; IF ((Y MOD 2) == 1) THEN MT [i] ^= 9908B0DFh
    ;
    test    [esp + loc8.y], 1
    jz      .continue

    xor     dword [esi + eax*4], 9908B0DFh

.continue:
    inc     [esp + loc8.index]
    cmp     [esp + loc8.index], 623
    jbe     .calc_y

    mcEndLocals loc8.size
    ret

; --------------------------------------------------------------------------
; Output:
;   EAX = next random value from a sequence
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_GetNextNumber:
    push    ebx
    mov     ebx, [glb_pMersenne]
    ;
    ; IF (index == 0) THEN generate next 624 values
    ;
    cmp     [ebx + TRandom.rand_NextIndex], 0
    jne     .get_it

    push    esi
    call    TRandom_Generate624
    pop     esi
    mov     ebx, [glb_pMersenne]

.get_it:
    mcLoadMember ecx, TRandom.rand_NextIndex
    mov     eax, [ebx + ecx*4]

    mov     edx, eax
    shr     edx, 11
    xor     eax, edx

    mov     edx, eax
    shl     edx, 7
    and     edx, 9D2C5680h
    xor     eax, edx

    mov     edx, eax
    shl     edx, 15
    and     edx, 0EFC60000h
    xor     eax, edx

    mov     edx, eax
    shr     edx, 18
    xor     eax, edx
    ;
    ; Index = (Index+1) MOD 624
    ;
    push    eax
    mov     eax, ecx
    call    TRandom_GetModulo624
    mcStoreMember TRandom.rand_NextIndex, eax
    pop     eax
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   EDX = N
; Output:
;   EAX = random value in range [0..N-1]
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_IRan:
    push    edx
    call    TRandom_GetNextNumber
    mcZeroBits edx
    pop     ecx
    div     ecx
    mov     eax, edx
    ret

; --------------------------------------------------------------------------
; Output:
;   ST(0) = random value in range [0..1]
; --------------------------------------------------------------------------
virtual at 0
loc9:
    .dbl_2e31 DOUBLE ?
    .dbl_padded DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_Ranf:
    pushad
    mcBeginLocals loc9.size

    fldz
    fstp    [esp + loc9.dbl_2e31]

    call    TRandom_GetNextNumber
    test    eax, 80000000h
    jz      .add_two_parts

    fld     [glb_dbl_2e31]
    fstp    [esp + loc9.dbl_2e31]
    and     eax, 7FFFFFFFh

.add_two_parts:
    call    TCommon_FPU_Load_EAX
    fld     [esp + loc9.dbl_2e31]
    faddp

    fld     [glb_dbl_2e32]
    fdivp

    mcEndLocals loc9.size
    popad
    ret

; --------------------------------------------------------------------------
; Output:
;   EAX = value 1 or 10 (X,Y sector of Tholian!)
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_TholianSect:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    faddp
    call    TCommon_FP_Truncate
    imul    eax, 9
    inc     eax
    ret

; --------------------------------------------------------------------------
; Output:
;   EAX = random X value in range [1..8]
;   EDX = random Y value in range [1..8]
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_IRan8:
    mcLoad8bitsToReg32 edx, 8
    call    TRandom_IRan
    inc     eax
    push    eax

    mcLoad8bitsToReg32 edx, 8
    call    TRandom_IRan
    inc     eax

    mov     edx, eax
    pop     eax
    ret

; --------------------------------------------------------------------------
; Output:
;   EAX = random X value in range [1..10]
;   EDX = random Y value in range [1..10]
; --------------------------------------------------------------------------
align PROC_ALIGN
TRandom_IRan10:
    mcLoad8bitsToReg32 edx, 10
    call    TRandom_IRan
    inc     eax
    push    eax

    mcLoad8bitsToReg32 edx, 10
    call    TRandom_IRan
    inc     eax

    mov     edx, eax
    pop     eax
    ret

; --- EOF ---
