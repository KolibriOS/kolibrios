; --------------------------------------------------------------------------
; FILE: TCommon.Asm
; DATE: October 4, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AbsEAX:
    add     eax, 0
    js      .flip_negative
    ret

.flip_negative:
    neg     eax
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = DOUBLE value B
;   ST(1) = DOUBLE value A
; Output:
;   ST(0) = A with a sign of B
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_FPU_SignAB:
    mcZeroBits eax
    fldz
    mc_CMP_ST0_ST1
    adc     eax, eax                ; EAX=1 if B is positive

    fabs
    mcOnRegZero eax, .negative
    ret

.negative:
    fchs
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = DOUBLE value #1
;   ST(1) = DOUBLE value #2
; Output:
;   ST(0) = maximum of two values
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_FPU_Max:
    mcBeginLocals 16

    fld     st
    fstp    tbyte [esp]

    fcomip  st, st1
    jc      .done

    fstp    [glb_dbl_FpuPopper]
    fld     tbyte [esp]

.done:
    mcEndLocals 16
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = DOUBLE value #1
;   ST(1) = DOUBLE value #2
; Output:
;   ST(0) = minimum of two values
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_FPU_Min:
    mcBeginLocals 16

    fld     st
    fstp    tbyte [esp]

    fcomip  st, st1
    jnc     .done

    fstp    [glb_dbl_FpuPopper]
    fld     tbyte [esp]

.done:
    mcEndLocals 16
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_Allocate:
    mcAllocatePages TCommon.size
    mov     [glb_pCommon], eax

    add     eax, TCommon.GAMEDB
    mov     [glb_pGameData], eax
    ret

; --------------------------------------------------------------------------
; EXPRAN
; --------------------------------------------------------------------------
; Input:
;   ST(0) = AVERAGE
; Output:
;   ST(0) = -AVERAGE * LOG (RANF(0))
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_ExpRan:
    fchs
    fld1
    call    TRandom_Ranf
    fyl2x
    fldln2
    fmulp
    fmulp
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = byte lo load
; Output:
;   ST(0) = DOUBLE loaded for processing
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_LoadByteIntoFPU:
    movzx   eax, al
    mov     [glb_FPU_Int32], eax
    fild    [glb_FPU_Int32]
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = value lo load
; Output:
;   ST(0) = DOUBLE loaded for processing
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_FPU_Load_EAX:
    mov     [glb_FPU_Int32], eax
    fild    [glb_FPU_Int32]
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = DOUBLE value
; Output:
;   EAX = truncated Int32 value
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_FP_Truncate:
    fisttp  [glb_FPU_Int32]
    mov     eax, [glb_FPU_Int32]
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = points to DOUBLE X value
;   EDI = points to DOUBLE Y value
; Output:
;   ST(0) = result X**Y
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_X_exp_Y:
    fld     tbyte [edi]
    fld     tbyte [esi]

    fyl2x

    fld     st
    frndint

    fsub    st1, st
    fxch
    f2xm1
    fld1
    faddp

    fscale
    fxch
    mcFpuPop
    ret

; --------------------------------------------------------------------------
virtual at 0
loc12:
    .nQuadrants COUNT ?
    .nStars COUNT ?
    .pQuadrant PINT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateStars:
    mcBeginLocals loc12.size

    mcLoad8bitsToReg32 eax, 64
    mcStoreLocal loc12.nQuadrants, eax

    mcLoadGameDataPtr edx
    lea     edi, [edx + TREKDATA.GALAXY]
    mcStoreLocal loc12.pQuadrant, edi

    mcZeroBits ecx
    mcStoreLocal loc12.nStars, ecx

.stars_for_quadrant:
    mcLoad8bitsToReg32 edx, 9
    call    TRandom_IRan
    inc     eax

    add     [esp + loc12.nStars], eax
    mcLoadLocal edi, loc12.pQuadrant
    mov     [edi], eax
    add     [esp + loc12.pQuadrant], 4

    dec     [esp + loc12.nQuadrants]
    jnz     .stars_for_quadrant

    mcLoadLocal eax, loc12.nStars
    mcLoadGameDataPtr ebx
    mcStoreMember TREKDATA.INSTAR, eax

    mcEndLocals loc12.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc13:
    .base_IX INT32 ?
    .base_IY INT32 ?
    .base_DISTQ_max INT32 ?
    .loop_I INT32 ?
    .loop_J INT32 ?
    .loop_LIM INT32 ?
    .loop_INBASE INT32 ?
    .pBaseQX PINT32 ?
    .pBaseQY PINT32 ?
    .pGalaxyCell PINT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateBases:
    mcBeginLocals loc13.size

    mcLoadGameDataPtr esi
    movzx   eax, [esi + TREKDATA.INBASE]
    mcStoreLocal loc13.loop_INBASE, eax

    sub     eax, 6
    neg     eax
    imul    eax, 6
    mcStoreLocal loc13.base_DISTQ_max, eax

    lea     edx, [esi + TREKDATA.BASEQX]
    lea     ecx, [esi + TREKDATA.BASEQY]
    mcStoreLocal loc13.pBaseQX, edx
    mcStoreLocal loc13.pBaseQY, ecx

    mcLoad1 eax
    mcStoreLocal loc13.loop_I, eax

.L6:
    call    TRandom_IRan8
    mcStoreLocal loc13.base_IX, eax
    mcStoreLocal loc13.base_IY, edx

    call    TArray_GalaxyPtr
    mcStoreLocal loc13.pGalaxyCell, ebx

    cmp     dword [ebx], 10
    jae     .L6

    cmp     [esp + loc13.loop_I], 1
    je      .L8

    mcLoadLocal ecx, loc13.loop_I
    dec     ecx
    mcStoreLocal loc13.loop_LIM, ecx
    ;
    ; Check distances to already set up bases
    ;
    mcLoad1 eax
    mcStoreLocal loc13.loop_J, eax

.calculate_distance:
    mcLoadLocal ecx, loc13.loop_J
    dec     ecx

    mcLoadLocal esi, loc13.pBaseQX
    mcLoadLocal edi, loc13.pBaseQY

    mcLoadLocal edx, loc13.base_IX
    mcLoadLocal eax, loc13.base_IY

    sub     dl, [esi + ecx]
    sub     al, [edi + ecx]

    movsx   ecx, dl
    imul    ecx, ecx

    movsx   edx, al
    imul    edx, edx

    add     edx, ecx
    cmp     edx, [esp + loc13.base_DISTQ_max]
    jae     .L8

    fld     [glb_dbl_0dot75]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L6

    inc     [esp + loc13.loop_J]
    mcLoadLocal ecx, loc13.loop_J
    cmp     ecx, [esp + loc13.loop_LIM]
    jbe     .calculate_distance

.L8:
    mcLoadLocal esi, loc13.pBaseQX
    mcLoadLocal edi, loc13.pBaseQY

    mcLoadLocal eax, loc13.base_IX
    mcLoadLocal edx, loc13.base_IY

    mcLoadLocal ecx, loc13.loop_I
    dec     ecx

    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    mcLoad8bitsToReg32 ecx, -1
    call    TArray_SetStarChartValue

    mcLoadLocal edi, loc13.pGalaxyCell
    add     dword [edi], 10
    ;
    ; Next base...
    ;
    inc     [esp + loc13.loop_I]
    mcLoadLocal ecx, loc13.loop_I
    cmp     ecx, [esp + loc13.loop_INBASE]
    jbe     .L6

    mcEndLocals loc13.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc14:
    .nKREM COUNT ?
    .nKLUMP COUNT ?
    .nKLUMPER COUNT ?
    .nNUM COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateKlingons:
    mcBeginLocals loc14.size

    mcLoadGameDataPtr esi
    mcZeroBits eax
    mov     al, [esi + TREKDATA.INKLING]
    sub     al, [esi + TREKDATA.INCOM]
    sub     al, [esi + TREKDATA.NSCREM]
    mcStoreLocal loc14.nKREM, eax

    movzx   edx, [esi + TREKDATA.SKILL]
    movzx   ecx, [esi + TREKDATA.LENGTH]
    sub     ecx, 9
    neg     ecx
    imul    ecx, edx
    mov     [glb_FPU_Int32], ecx
    fild    [glb_FPU_Int32]
    fld     [glb_dbl_DOCKFAC]
    fmulp
    fld1
    faddp
    call    TCommon_FP_Truncate

    cmp     eax, 9
    jbe     .store_KLUMPER

    mcLoad8bitsToReg32 eax, 9

.store_KLUMPER:
    mcStoreLocal loc14.nKLUMPER, eax

.L10:
    call    TRandom_Ranf
    fld     st
    fmulp
    fld1
    fsubrp
    fild    [esp + loc14.nKLUMPER]
    fmulp
    call    TCommon_FP_Truncate

    cmp     eax, [esp + loc14.nKREM]
    jbe     .store_KLUMP

    mcLoadLocal eax, loc14.nKREM

.store_KLUMP:
    mcStoreLocal loc14.nKLUMP, eax

    imul    eax, 100
    mcStoreLocal loc14.nNUM, eax

.L15:
    call    TRandom_IRan8
    call    TArray_GalaxyPtr

    mov     eax, [ebx]
    add     eax, [esp + loc14.nNUM]
    cmp     eax, 999
    ja      .L15

    mcLoadLocal ecx, loc14.nNUM
    add     [ebx], ecx

    mcLoadLocal edx, loc14.nKLUMP
    sub     [esp + loc14.nKREM], edx

    cmp     [esp + loc14.nKREM], 0
    jne     .L10

    mcEndLocals loc14.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc15:
    .loop_INCOM COUNT ?
    .loop_IY INT32 ?
    .loop_IX INT32 ?
    .loop_I COUNT ?
    .loop_JJ COUNT ?
    .loop_IM1 COUNT ?
    .pCX PBYTE ?
    .pCY PBYTE ?
    .pGalaxyCell PINT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateCommanders:
    mcBeginLocals loc15.size

    mcLoadGameDataPtr ebx

    lea     esi, [ebx + TREKDATA.CX]
    lea     edi, [ebx + TREKDATA.CY]
    mcStoreLocal loc15.pCX, esi
    mcStoreLocal loc15.pCY, edi

    movzx   ecx, [ebx + TREKDATA.INCOM]
    mcStoreLocal loc15.loop_INCOM, ecx

    mcLoad1 eax
    mcStoreLocal loc15.loop_I, eax

.L16:
    call    TRandom_IRan8
    mcStoreLocal loc15.loop_IY, edx
    mcStoreLocal loc15.loop_IX, eax

    call    TArray_GalaxyPtr
    mcStoreLocal loc15.pGalaxyCell, ebx

    cmp     dword [ebx], 99
    jae     .check_if_quad_full

    fld     [glb_dbl_0dot75]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L16

.check_if_quad_full:
    mcLoadLocal esi, loc15.pGalaxyCell
    cmp     dword [esi], 899
    ja      .L16

    cmp     [esp + loc15.loop_I], 1
    je      .L17
    ;
    ; Verify that quadrant IX,IY does not contain commander already
    ;
    mcLoadLocal ecx, loc15.loop_I
    dec     ecx

    mcLoadLocal esi, loc15.pCX
    mcLoadLocal edi, loc15.pCY
    mcZeroBits ebx
    mcZeroBits eax
    mcZeroBits edx

.check_cx_cy:
    mov     al, [esi + ebx]
    mov     dl, [edi + ebx]

    cmp     eax, [esp + loc15.loop_IX]
    jne     .next

    cmp     edx, [esp + loc15.loop_IY]
    je      .L16

.next:
    inc     ebx
    loop    .check_cx_cy

.L17:
    mcLoadLocal edi, loc15.pGalaxyCell
    add     dword [edi], 100

    mcLoadLocal ecx, loc15.loop_I
    dec     ecx

    mcLoadLocal esi, loc15.pCX
    mcLoadLocal edi, loc15.pCY

    mcLoadLocal eax, loc15.loop_IX
    mcLoadLocal edx, loc15.loop_IY

    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    inc     [esp + loc15.loop_I]
    mcLoadLocal ecx, loc15.loop_I
    cmp     ecx, [esp + loc15.loop_INCOM]
    jbe     .L16

    mcEndLocals loc15.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc16:
    .loop_INPLAN COUNT ?
    .pPlanet PTPlanet ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocatePlanets:
    mcBeginLocals loc16.size

    mcLoadGameDataPtr esi
    movzx   ecx, [esi + TREKDATA.INPLAN]
    add     esi, TREKDATA.PLNETS
    mcStoreLocal loc16.loop_INPLAN, ecx
    mcStoreLocal loc16.pPlanet, esi

.L19:
    call    TRandom_IRan8
    call    TArray_NewStufPtr

    cmp     byte [ebx], 0
    jne     .L19

    inc     byte [ebx]
    mcLoadLocal ebx, loc16.pPlanet

    mov     [ebx + TPlanet.planet_X], al
    mov     [ebx + TPlanet.planet_Y], dl

    mcLoad8bitsToReg32 edx, 3
    call    TRandom_IRan
    inc     eax
    mov     [ebx + TPlanet.planet_CLASS], al

    call    TRandom_Ranf
    fld     [glb_dbl_1dot2]
    fmulp
    call    TCommon_FP_Truncate
    mov     [ebx + TPlanet.planet_DILITHIUM], al

    add     [esp + loc16.pPlanet], TPlanet.size
    dec     [esp + loc16.loop_INPLAN]
    jnz     .L19

    mcEndLocals loc16.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateRomulans:
    mcLoadGameDataPtr esi
    movzx   ecx, [esi + TREKDATA.NROMREM]

.L21:
    push    ecx

    call    TRandom_IRan8
    call    TArray_NewStufPtr
    add     byte [ebx], 10

    pop     ecx
    loop    .L21
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateSuperCommander:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.NSCREM], 0
    je      .done

.L22:
    call    TRandom_IRan8
    call    TArray_GalaxyPtr

    cmp     dword [ebx], 899
    ja      .L22

    mcLoadGameDataPtr edi
    mov     [edi + TREKDATA.ISX], al
    mov     [edi + TREKDATA.ISY], dl
    add     dword [ebx], 100

.done:
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_AllocateThing:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot04]
    mc_CMP_ST0_ST1
    jc      .no_thing

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.THINGX], -1
    je      .no_thing

    call    TRandom_IRan8
    mcLoadGameDataPtr ebx
    mcStoreMember TREKDATA.THINGX, al
    mcStoreMember TREKDATA.THINGY, dl
    jmp     .done

.no_thing:
    mcZeroBits eax
    mcLoadGameDataPtr ebx
    mcStoreMember TREKDATA.THINGX, al
    mcStoreMember TREKDATA.THINGY, al

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X sector coordinate
;   DL = Y sector coordinate
;   CH = value to check
;   CL = replacement value
; NOTE:
;   All registers are saved
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_ReserveSector:
    push    ebx

    call    TArray_QuadPtr
    cmp     [ebx], ch
    jne     .done

    mov     [ebx], cl

.done:
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = object character
; Output:
;   AL = X sector coordinate
;   DL = Y sector coordinate
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_DropIn:
    mcBeginLocals 4
    mov     [esp], eax

.try_sector:
    call    TRandom_IRan10
    call    TArray_QuadPtr

    cmp     byte [ebx], CHAR_COSMOS
    jne     .try_sector

    mov     ecx, [esp]
    mov     [ebx], cl

    mcEndLocals 4
    ret

; --------------------------------------------------------------------------
; RESETD
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_ResetDist:
    call    TCommon_NewCondition

    mcLoadGameDataPtr ebx
    cmp     [ebx + TREKDATA.NENHERE], 0
    je      .done

    mcZeroBits ecx
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY

.calc_dist:
    movzx   eax, [ebx + TREKDATA.SECTX]
    movzx   edx, byte [esi + ecx]
    sub     eax, edx
    imul    eax, eax
    push    eax

    movzx   eax, [ebx + TREKDATA.SECTY]
    movzx   edx, byte [edi + ecx]
    sub     eax, edx
    imul    eax, eax
    pop     edx
    add     eax, edx

    call    TCommon_FPU_Load_EAX
    fsqrt
    inc     ecx
    call    TArray_SetDblKDist

    cmp     cl, [ebx + TREKDATA.NENHERE]
    jbe     .calc_dist

.done:
    ret

; --------------------------------------------------------------------------
; SORTKL
; --------------------------------------------------------------------------
virtual at 0
loc30:
    .nEnemies COUNT ?
    .pKPower PDOUBLE ?
    .pKDist PDOUBLE ?
    .bSwitch BOOL ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_SortKlingons:
    mcBeginLocals loc30.size

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.NENHERE], 1
    jbe     .done

    lea     eax, [esi + TREKDATA.KDIST]
    lea     ecx, [esi + TREKDATA.KPOWER]
    lea     edx, [esi + TREKDATA.KX]
    lea     ebx, [esi + TREKDATA.KY]

    mcStoreLocal loc30.pKDist, eax
    mcStoreLocal loc30.pKPower, ecx
    mcStoreLocal loc30.pKX, edx
    mcStoreLocal loc30.pKY, ebx

    movzx   ecx, [esi + TREKDATA.NENHERE]
    dec     ecx
    mcStoreLocal loc30.nEnemies, ecx

.L10:
    mcZeroBits eax
    mcStoreLocal loc30.bSwitch, eax

    mcZeroBits ecx
    mcZeroBits edx

.check_distance:
    mcLoadLocal esi, loc30.pKDist
    fld     tbyte [esi + edx + 10]
    fld     tbyte [esi + edx]
    mc_CMP_ST0_ST1
    jc      .L20
    jz      .L20
    ;
    ; Swap items from all arrays
    ;
    fld     tbyte [esi + edx + 10]
    fld     tbyte [esi + edx]
    fstp    tbyte [esi + edx + 10]
    fstp    tbyte [esi + edx]

    mcLoadLocal esi, loc30.pKPower
    fld     tbyte [esi + edx + 10]
    fld     tbyte [esi + edx]
    fstp    tbyte [esi + edx + 10]
    fstp    tbyte [esi + edx]

    mcLoadLocal esi, loc30.pKX
    mov     al, [esi + ecx]
    xchg    al, [esi + ecx + 1]
    mov     [esi + ecx], al

    mcLoadLocal esi, loc30.pKY
    mov     al, [esi + ecx]
    xchg    al, [esi + ecx + 1]
    mov     [esi + ecx], al

    inc     [esp + loc30.bSwitch]

.L20:
    inc     ecx
    add     edx, 10
    cmp     ecx, [esp + loc30.nEnemies]
    jb      .check_distance

    cmp     [esp + loc30.bSwitch], 0
    jne     .L10

.done:
    mcEndLocals loc30.size
    ret

; --------------------------------------------------------------------------
; NEWCOND
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_NewCondition:
    mcLoadGameDataPtr ebx
    cmp     [ebx + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .done
    mov     [ebx + TREKDATA.CONDIT], CONDITION_GREEN

    fld     [glb_dbl_1000]
    fld     [ebx + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jnc     .check_enemies

    mov     [ebx + TREKDATA.CONDIT], CONDITION_YELLOW

.check_enemies:
    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 99
    ja      .condition_red

    call    TArray_MyNewStufPtr
    cmp     byte [ebx], 9
    ja      .condition_red

.done:
    ret

.condition_red:
    mcLoadGameDataPtr ebx
    mov     [ebx + TREKDATA.CONDIT], CONDITION_RED
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_DropBlackHole:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot89]
    mc_CMP_ST0_ST1
    jc      .drop
    ret

.drop:
    mov     al, CHAR_BLACK_HOLE
    call    TCommon_DropIn
    ret

; --------------------------------------------------------------------------
; NEWQUAD
; --------------------------------------------------------------------------
virtual at 0
loc23:
    .pTrekData PVOID ?
    .nQUADNUM INT32 ?
    .nPLAN INT32 ?
    .nI INT32 ?
    .nIX INT32 ?
    .nIY INT32 ?
    .nSKILL INT32 ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .nKLHERE COUNT ?
    .nNENHERE COUNT ?
    .pKPOWER PDOUBLE ?
    .nQuadValue INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_NewQuad:
    mcBeginLocals loc23.size
    mcLoadGameDataPtr ebx
    mcStoreLocal loc23.pTrekData, ebx

    mcZeroBits eax
    dec     eax
    mcStoreMember TREKDATA.LANDED, al

    inc     eax
    mcStoreMember TREKDATA.BASEX, al
    mcStoreMember TREKDATA.BASEY, al
    mcStoreMember TREKDATA.KLHERE, al
    mcStoreMember TREKDATA.COMHERE, al
    mcStoreMember TREKDATA.PLNETX, al
    mcStoreMember TREKDATA.PLNETY, al
    mcStoreMember TREKDATA.ISHERE, al
    mcStoreMember TREKDATA.IRHERE, al
    mcStoreMember TREKDATA.IPLANET, al
    mcStoreMember TREKDATA.NENHERE, al
    mcStoreMember TREKDATA.NEUTZ, al
    mcStoreMember TREKDATA.INORBIT, al
    mcStoreMember TREKDATA.IENTESC, al
    mcStoreMember TREKDATA.ITHERE, al

    inc     eax
    mcStoreMember TREKDATA.JUSTIN, al
    dec     eax

    cmp     [ebx + TREKDATA.ISCATE], al
    je      .L5

    mcStoreMember TREKDATA.ISCATE, al
    inc     [ebx + TREKDATA.IENTESC]

.L5:
    call    TArray_MyGalaxyPtr
    mov     eax, [ebx]
    mcStoreLocal loc23.nQUADNUM, eax

    cmp     eax, 999
    ja      .L70

    mcLoad8bitsToReg32 ecx, 100
    mcZeroBits edx
    div     ecx
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.KLHERE, al

    call    TArray_MyNewStufPtr
    movzx   eax, byte [ebx]
    mcLoad8bitsToReg32 ecx, 10
    mcZeroBits edx
    div     ecx
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.IRHERE, al
    mcStoreLocal loc23.nPLAN, edx

    mcLoadMember cl, TREKDATA.KLHERE
    add     cl, al
    mcStoreMember TREKDATA.NENHERE, cl

    lea     edi, [ebx + TREKDATA.QUAD]
    mcLoad8bitsToReg32 ecx, 100
    mov     al, CHAR_COSMOS
    rep     stosb

    mcLoadMember al, TREKDATA.SECTX
    mcLoadMember dl, TREKDATA.SECTY
    mcLoadMember cl, TREKDATA.SHIP
    call    TArray_QuadPtr
    mov     [ebx], cl

    fld     [glb_dbl_0dot08]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L17

    mcLoadLocal ebx, loc23.pTrekData
    mcLoadMemberRef esi, TREKDATA.PASSWD
    mov     edi, str_THOLIANX
    call    TString_AnsiEqual
    jnc     .L23

.L17:
    call    TRandom_TholianSect
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.ITHX, al

    call    TRandom_TholianSect
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.ITHY, al

    mov     dl, al
    mcLoadMember al, TREKDATA.ITHX
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    jne     .L17

    mov     byte [ebx], CHAR_THOLIAN
    mcLoadLocal edi, loc23.pTrekData
    inc     [edi + TREKDATA.ITHERE]

    mov     cl, CHAR_WEB
    mov     ch, CHAR_COSMOS
    mov     al, 1
    mov     dl, al
    call    TCommon_ReserveSector

    mov     dl, 10
    call    TCommon_ReserveSector

    mov     al, 10
    call    TCommon_ReserveSector

    mov     dl, 1
    call    TCommon_ReserveSector

.L23:
    ;
    ; Klingons!
    ;
    mcLoadLocal ebx, loc23.pTrekData
    movzx   ecx, [ebx + TREKDATA.KLHERE]
    mcStoreLocal loc23.nKLHERE, ecx

    imul    eax, ecx, 100
    mcStoreLocal loc23.nQuadValue, eax

    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mcLoad1 eax
    mcStoreLocal loc23.pKX, esi
    mcStoreLocal loc23.pKY, edi
    mcStoreLocal loc23.nI, eax

    mcLoadMemberRef esi, TREKDATA.KPOWER
    mcStoreLocal loc23.pKPOWER, esi

    movzx   eax, [ebx + TREKDATA.SKILL]
    mcStoreLocal loc23.nSKILL, eax

    cmp     [esp + loc23.nQUADNUM], 100
    jb      .L34

    mcLoadLocal eax, loc23.nQuadValue
    sub     [esp + loc23.nQUADNUM], eax

.put_klingon:
    mov     al, CHAR_KLINGON
    call    TCommon_DropIn
    mcLoadLocal esi, loc23.pKX
    mcLoadLocal edi, loc23.pKY
    mcLoadLocal ecx, loc23.nI
    dec     ecx
    mov     [esi + ecx], al
    mov     [edi + ecx], dl
    mcStoreLocal loc23.nIX, eax
    mcStoreLocal loc23.nIY, edx

    call    TRandom_Ranf
    fld     [glb_dbl_150]
    fmulp
    mcLoadLocal eax, loc23.nSKILL
    imul    eax, 25
    add     eax, 300
    call    TCommon_FPU_Load_EAX
    faddp

    mcLoadLocal edi, loc23.pKPOWER
    mcLoadLocal ecx, loc23.nI
    dec     ecx
    imul    ecx, 10
    fstp    tbyte [edi + ecx]

    inc     [esp + loc23.nI]
    mcLoadLocal edx, loc23.nI
    cmp     edx, [esp + loc23.nKLHERE]
    jbe     .put_klingon
    ;
    ; Commander!
    ;
    mcLoadLocal ebx, loc23.pTrekData
    cmp     [ebx + TREKDATA.REMCOM], 0
    je      .L32

    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY
    call    TArray_FindCommander
    jecxz   .L32

    mcLoadLocal eax, loc23.nIX
    mcLoadLocal edx, loc23.nIY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COMMANDER

    mcLoadLocal eax, loc23.nSKILL
    imul    eax, 50
    add     eax, 950
    call    TCommon_FPU_Load_EAX
    call    TRandom_Ranf
    fld     [glb_dbl_400]
    fmulp
    faddp
    mcLoadLocal ecx, loc23.nKLHERE
    call    TArray_SetDblKPower

    mcLoadLocal ebx, loc23.pTrekData
    inc     [ebx + TREKDATA.COMHERE]

.L32:
    ;
    ; Super-Commander!!
    ;
    mcLoadLocal ebx, loc23.pTrekData

    mcLoadMember al, TREKDATA.QUADX
    cmp     al, [ebx + TREKDATA.ISX]
    jne     .L34

    mcLoadMember dl, TREKDATA.QUADY
    cmp     dl, [ebx + TREKDATA.ISY]
    jne     .L34
    ;
    ; He's here!!
    ;
    mcLoadLocal eax, loc23.nKLHERE
    mcStoreLocal loc23.nI, eax

    cmp     [ebx + TREKDATA.COMHERE], 0
    je      .L33

    dec     [esp + loc23.nI]
    mcLoadLocal esi, loc23.pKX
    mcLoadLocal edi, loc23.pKY
    mcLoadLocal ecx, loc23.nI
    dec     ecx
    movzx   eax, byte [esi + ecx]
    movzx   edx, byte [edi + ecx]
    mcStoreLocal loc23.nIX, eax
    mcStoreLocal loc23.nIY, edx

.L33:
    mcLoadLocal eax, loc23.nIX
    mcLoadLocal edx, loc23.nIY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_SCOM

    mcLoadLocal eax, loc23.nSKILL
    imul    eax, 125
    add     eax, 1175
    call    TCommon_FPU_Load_EAX
    call    TRandom_Ranf
    fld     [glb_dbl_400]
    fmulp
    faddp
    mcLoadLocal ecx, loc23.nI
    call    TArray_SetDblKPower

    mcLoadLocal ebx, loc23.pTrekData
    inc     [ebx + TREKDATA.ISCATE]
    inc     [ebx + TREKDATA.ISHERE]

.L34:
    ;
    ; Romulans!
    ;
    mcLoadLocal ebx, loc23.pTrekData
    cmp     [ebx + TREKDATA.IRHERE], 0
    je      .L37

    movzx   eax, [ebx + TREKDATA.KLHERE]
    inc     eax
    mcStoreLocal loc23.nI, eax

    movzx   eax, [ebx + TREKDATA.NENHERE]
    mcStoreLocal loc23.nNENHERE, eax

.drop_a_romulan:
    mov     al, CHAR_ROMULAN
    call    TCommon_DropIn

    mcLoadLocal ecx, loc23.nI
    dec     ecx
    mcLoadLocal esi, loc23.pKX
    mcLoadLocal edi, loc23.pKY
    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    mcLoadLocal eax, loc23.nSKILL
    imul    eax, 50
    add     eax, 450
    call    TCommon_FPU_Load_EAX
    call    TRandom_Ranf
    fld     [glb_dbl_400]
    fmulp
    faddp

    mcLoadLocal ecx, loc23.nI
    call    TArray_SetDblKPower

    inc     [esp + loc23.nI]
    mcLoadLocal edx, loc23.nI
    cmp     edx, [esp + loc23.nNENHERE]
    jbe     .drop_a_romulan

.L37:
    call    TCommon_ResetDist
    call    TCommon_SortKlingons
    ;
    ; Starbase
    ;
    cmp     [esp + loc23.nQUADNUM], 10
    jb      .L50

    sub     [esp + loc23.nQUADNUM], 10
    mov     al, CHAR_STARBASE
    call    TCommon_DropIn
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.BASEX, al
    mcStoreMember TREKDATA.BASEY, dl

.L50:
    ;
    ; Planet
    ;
    cmp     [esp + loc23.nPLAN], 0
    je      .L54

    mcLoadLocal ebx, loc23.pTrekData
    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY
    call    TArray_FindPlanet
    mcStoreMember TREKDATA.IPLANET, cl
    jecxz   .L54

    mov     al, CHAR_PLANET
    call    TCommon_DropIn
    mcLoadLocal ebx, loc23.pTrekData
    mcStoreMember TREKDATA.PLNETX, al
    mcStoreMember TREKDATA.PLNETY, dl

.L54:
    ;
    ; Stars like dust!..
    ;
    call    TCommon_NewCondition
    cmp     [esp + loc23.nQUADNUM], 0
    je      .L62

    mcLoadLocal ecx, loc23.nQUADNUM

.drop_star:
    push    ecx
    mov     al, CHAR_ASTERISK
    call    TCommon_DropIn
    pop     ecx
    loop    .drop_star

.L62:
    ;
    ; Check for Neutral Zone
    ;
    mcLoadLocal ebx, loc23.pTrekData
    mcZeroBits eax
    cmp     [ebx + TREKDATA.IRHERE], al
    je      .L66
    cmp     [ebx + TREKDATA.KLHERE], al
    jne     .L66
    cmp     [ebx + TREKDATA.BASEX], al
    jne     .L66

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jc      .L64

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 85
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 86
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 87
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 88
    call    TConsole_Prout
    call    TConsole_ScrollUp

.L64:
    mcLoadLocal ebx, loc23.pTrekData
    inc     [ebx + TREKDATA.NEUTZ]

.L66:
    ;
    ; Check for THING!!
    ;
    mcLoadLocal ebx, loc23.pTrekData
    cmp     [ebx + TREKDATA.SHUTUP], 0
    jne     .L67

    mcLoadMember al, TREKDATA.THINGX
    cmp     al, [ebx + TREKDATA.QUADX]
    jne     .L67

    mcLoadMember al, TREKDATA.THINGY
    cmp     al, [ebx + TREKDATA.QUADY]
    jne     .L67

    mov     al, CHAR_THING
    call    TCommon_DropIn

    mcLoadLocal ebx, loc23.pTrekData
    mcZeroBits eax
    mcStoreMember TREKDATA.THINGX, al
    mcStoreMember TREKDATA.THINGY, al

    mov     cl, DEV_SR_SENSORS
    call    TArray_IsDamaged
    jc      .L67

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 89
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 90
    call    TConsole_Prout

.L67:
    call    TCommon_DropBlackHole
    call    TCommon_DropBlackHole
    call    TCommon_DropBlackHole

    mcLoadLocal ebx, loc23.pTrekData
    cmp     [ebx + TREKDATA.ITHERE], 0
    je      .done

    mov     ch, CHAR_WEB
    mov     cl, CHAR_COSMOS
    mov     al, 1
    mov     dl, al
    call    TCommon_ReserveSector

    mov     dl, 10
    call    TCommon_ReserveSector

    mov     al, 10
    call    TCommon_ReserveSector

    mov     dl, 1
    call    TCommon_ReserveSector
    jmp     .done

.L70:
    ;
    ; Quadrant contains supernova
    ;
    mcLoadLocal edi, loc23.pTrekData
    add     edi, TREKDATA.QUAD
    mcLoad8bitsToReg32 ecx, 100
    mov     al, CHAR_COSMOS
    rep     stosb

.done:
    mcEndLocals loc23.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc22:
    .pBaseX PBYTE ?
    .pBaseY PBYTE ?
    .nBases COUNT ?
    .iBase INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_PrintBaseQuads:
    mcBeginLocals loc22.size

    mcLoadGameDataPtr ebx
    mcLoadMemberRef esi, TREKDATA.BASEQX
    mcLoadMemberRef edi, TREKDATA.BASEQY
    movzx   ecx, [ebx + TREKDATA.INBASE]
    mcLoad1 eax

    mcStoreLocal loc22.pBaseX, esi
    mcStoreLocal loc22.pBaseY, edi
    mcStoreLocal loc22.nBases, ecx
    mcStoreLocal loc22.iBase, eax

.print_location:
    mcLoadLocal esi, loc22.pBaseX
    mcLoadLocal edi, loc22.pBaseY
    mcLoadLocal ecx, loc22.iBase

    dec     ecx
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    mcZeroBits ecx
    call    TConsole_CramLoc

    mcLoadLocal ecx, loc22.iBase
    cmp     ecx, [esp + loc22.nBases]
    je      .done

    mcLoad8bitsToReg32 ecx, 53
    call    TConsole_Cram

    inc     [esp + loc22.iBase]
    jmp     .print_location

.done:
    mcEndLocals loc22.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_InitialMsg:
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 47
    call    TConsole_Cram
    mov     cl, 1
    mcLoadGameDataPtr esi
    fld     [esi + TREKDATA.DATE]
    push    esi
    call    TConsole_CramFloat
    call    TConsole_ScrollUp
    pop     esi

    movzx   eax, [esi + TREKDATA.INKLING]
    movzx   ecx, [esi + TREKDATA.NSCREM]
    sub     eax, ecx
    mov     cl, 5
    push    esi
    call    TConsole_CramIntWidth
    mcLoad8bitsToReg32 ecx, 48
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    pop     esi

    cmp     [esi + TREKDATA.NSCREM], 0
    je      .L25

    mcLoad8bitsToReg32 ecx, 50
    push    esi
    call    TConsole_Prout
    pop     esi

.L25:
    fld     [esi + TREKDATA.INTIME]
    call    TCommon_FP_Truncate
    mov     cl, 5
    push    esi
    call    TConsole_CramIntWidth
    mcLoad8bitsToReg32 ecx, 51
    call    TConsole_Prout
    pop     esi

    movzx   eax, [esi + TREKDATA.INBASE]
    mov     cl, 5
    push    esi
    call    TConsole_CramIntWidth
    mcLoad8bitsToReg32 ecx, 52
    call    TConsole_Cram
    call    TCommon_PrintBaseQuads
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 54
    call    TConsole_Cram
    pop     esi

    mov     cl, 1
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    push    esi
    call    TConsole_CramLoc

    mov     al, ','
    call    TConsole_PutChar
    pop     esi

    mov     cl, 2
    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    push    esi
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 55
    call    TConsole_Cram
    pop     esi

    cmp     [esi + TREKDATA.NSCREM], 0
    je      .done

    mcLoad8bitsToReg32 ecx, 56
    call    TConsole_Cram

.done:
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    ret

; --------------------------------------------------------------------------
; Subroutine SETUP from original source
; --------------------------------------------------------------------------
virtual at 0
loc11:
    .dbl_IDATE DOUBLE ?
    .dbl_Reserved DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCommon_Setup:
    mcBeginLocals loc11.size
    mcLoadGameDataPtr ebx

    mcStoreMember TREKDATA.SHIP, CHAR_ENTERPRISE

    fld     [glb_dbl_5000]
    fld     st
    fstp    [ebx + TREKDATA.INENRG]
    fstp    [ebx + TREKDATA.ENERGY]

    fld     [glb_dbl_2500]
    fld     st
    fstp    [ebx + TREKDATA.INSHLD]
    fstp    [ebx + TREKDATA.SHLD]

    fld     [glb_dbl_4]
    fld     st
    fstp    [ebx + TREKDATA.INLSR]
    fstp    [ebx + TREKDATA.LSUPRES]

    mcStoreMember TREKDATA.INTORPS, 10
    mcStoreMember TREKDATA.TORPS, 10
    mcStoreMember TREKDATA.NPTUBES, 3

    fld     [glb_dbl_5]
    fld     [glb_dbl_25]
    fstp    [ebx + TREKDATA.WFACSQ]
    fstp    [ebx + TREKDATA.WARPFAC]

    call    TRandom_Ranf
    fld     [glb_dbl_31]
    fmulp
    fld     [glb_dbl_20]
    faddp
    fld     [glb_dbl_100]
    fmulp
    fld     st
    fld     st
    fstp    [ebx + TREKDATA.DATE]
    fstp    [ebx + TREKDATA.INDATE]
    fstp    [ebx + TREKDATA.DOCKDATE]

    call    TRandom_IRan8
    mcStoreMember TREKDATA.QUADX, al
    mcStoreMember TREKDATA.QUADY, dl

    call    TRandom_IRan10
    mcStoreMember TREKDATA.SECTX, al
    mcStoreMember TREKDATA.SECTY, dl

    mcStoreMember TREKDATA.ISCRAFT, 1
    mcStoreMember TREKDATA.ALIVE, 1
    mcStoreMember TREKDATA.LANDED, -1

    fld     [glb_dbl_0dot05]
    fstp    [ebx + TREKDATA.CRYPROB]
    ;
    ; Events
    ;
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE1]

    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_1dot5]
    fmulp
    movzx   ecx, [ebx + TREKDATA.REMCOM]
    mov     [glb_FPU_Int32], ecx
    fild    [glb_FPU_Int32]
    fdivp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE2]

    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE3]

    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot3]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE4]

    fld     [glb_dbl_1E38]
    fld     st
    fld     st
    fstp    [ebx + TREKDATA.FUTURE5]
    fstp    [ebx + TREKDATA.FUTURE6]
    fstp    [ebx + TREKDATA.FUTURE7]

    cmp     [ebx + TREKDATA.NSCREM], 0
    je      .allocate_objects

    fld     [ebx + TREKDATA.DATE]
    fld     [glb_dbl_0dot2777]
    faddp
    fstp    [ebx + TREKDATA.FUTURE6]

.allocate_objects:
    call    TCommon_AllocateStars
    call    TCommon_AllocateBases
    call    TCommon_AllocateKlingons
    call    TCommon_AllocateCommanders
    call    TCommon_AllocatePlanets
    call    TCommon_AllocateRomulans
    call    TCommon_AllocateSuperCommander
    call    TCommon_AllocateThing

    call    TCommon_InitialMsg
    call    TCommon_NewQuad

    mcEndLocals loc11.size
    ret

; --- EOF ---
