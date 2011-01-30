; --------------------------------------------------------------------------
; FILE: TDock.Asm
; DATE: October 25, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; ABANDON
; --------------------------------------------------------------------------
virtual at 0
loc86:
    .pTrekData PVOID ?
    .nAttempts COUNT ?
    .dbl_ShuttleDamage DOUBLE ?
    .dbl_padding DOUBLE ?
    .nBaseSectorX INT32 ?
    .nBaseSectorY INT32 ?
    .nFQSectorX INT32 ?
    .nFQSectorY INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_Abandon:
    mcBeginLocals loc86.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc86.pTrekData, esi

    mov     cl, DEV_SHUTTLE_CRAFT
    call    TArray_GetDblDamage
    fstp    [esp + loc86.dbl_ShuttleDamage]

    fld     [esp + loc86.dbl_ShuttleDamage]
    fldz
    mc_CMP_ST0_ST1
    jc      .L10
    jz      .L15

    fld     [esp + loc86.dbl_ShuttleDamage]
    fld1
    fchs
    mc_CMP_ST0_ST1
    jz      .L6

    mov     ecx, 443
    call    TConsole_ProutGameMsg
    jmp     .done

.L6:
    mov     ecx, 444
    call    TConsole_ProutGameMsg
    jmp     .done

.L10:
    mov     ecx, 445
    call    TConsole_ProutGameMsg
    jmp     .done

.L15:
    mcLoadLocal esi, loc86.pTrekData
    cmp     [esi + TREKDATA.LANDED], 1
    jne     .check1

.must_be_aboard:
    mov     ecx, 675
    call    TConsole_ProutGameMsg
    jmp     .done

.check1:
    cmp     [esi + TREKDATA.ICRAFT], 1
    je      .must_be_aboard

    cmp     [esi + TREKDATA.ISCRAFT], 1
    je      .L17

    mov     ecx, 674
    call    TConsole_ProutGameMsg
    jmp     .done

.L17:
    call    TConsole_ScrollUp
    mov     cl, ATTR_COND_RED
    call    TConsole_SetAttr
    mov     ecx, 676
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    call    TConsole_ScrollUp

    invoke  Sleep, 800

    mov     ecx, 678
    call    TConsole_ProutGameMsg
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mcLoadLocal esi, loc86.pTrekData
    cmp     [esi + TREKDATA.REMBASE], 0
    je      .finished

    invoke  Sleep, 800

    call    TConsole_ScrollUp
    mov     ecx, 681
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mcLoadLocal esi, loc86.pTrekData
    mcZeroBits eax
    mov     [esi + TREKDATA.ICRYSTL], al
    mov     [esi + TREKDATA.IMINE], al
    mov     [esi + TREKDATA.ISCATE], al

    movzx   edx, [esi + TREKDATA.REMBASE]
    call    TRandom_IRan
    mov     ecx, eax

    mcLoadLocal ebx, loc86.pTrekData
    mcLoadMemberRef esi, TREKDATA.BASEQX
    mcLoadMemberRef edi, TREKDATA.BASEQY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    mcStoreMember TREKDATA.QUADX, al
    mcStoreMember TREKDATA.QUADY, dl

.L40:
    mov     al, 5
    mcStoreMember TREKDATA.SECTX, al
    mcStoreMember TREKDATA.SECTY, al

    call    TCommon_NewQuad

    mov     al, 5
    mov     dl, al
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS
    ;
    ; Three attempts to put F.Q. beside the base
    ;
    mcLoad8bitsToReg32 eax, 3
    mcStoreLocal loc86.nAttempts, eax

    mcLoadLocal esi, loc86.pTrekData
    movzx   eax, [esi + TREKDATA.BASEX]
    movzx   edx, [esi + TREKDATA.BASEY]
    mcStoreLocal loc86.nBaseSectorX, eax
    mcStoreLocal loc86.nBaseSectorY, edx

.attempt:
    mcLoad8bitsToReg32 edx, 3
    call    TRandom_IRan
    dec     eax
    add     eax, [esp + loc86.nBaseSectorX]
    mcStoreLocal loc86.nFQSectorX, eax
    call    TPhotons_IsValidSector
    jnc     .next_attempt

    mcLoad8bitsToReg32 edx, 3
    call    TRandom_IRan
    dec     eax
    add     eax, [esp + loc86.nBaseSectorY]
    mcStoreLocal loc86.nFQSectorY, eax
    call    TPhotons_IsValidSector
    jnc     .next_attempt

    mcLoadLocal eax, loc86.nFQSectorX
    mcLoadLocal edx, loc86.nFQSectorY
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    je      .L60

.next_attempt:
    dec     [esp + loc86.nAttempts]
    jnz     .attempt
    jmp     .L40

.L60:
    mov     byte [ebx], CHAR_FQUEENE
    mcLoadLocal esi, loc86.pTrekData
    mov     [esi + TREKDATA.SHIP], CHAR_FQUEENE
    mov     [esi + TREKDATA.CONDIT], CONDITION_DOCKED

    mcLoadLocal eax, loc86.nFQSectorX
    mcLoadLocal edx, loc86.nFQSectorY
    mov     [esi + TREKDATA.SECTX], al
    mov     [esi + TREKDATA.SECTY], dl

    lea     edi, [esi + TREKDATA.DAMAGE]
    mcLoad8bitsToReg32 ecx, 14

.clear_damages:
    fldz
    fstp    tbyte [edi]
    add     edi, 10
    loop    .clear_damages

    fld     [glb_dbl_3000]
    fld     st
    fstp    [esi + TREKDATA.INENRG]
    fstp    [esi + TREKDATA.ENERGY]

    fld     [glb_dbl_1250]
    fld     st
    fstp    [esi + TREKDATA.INSHLD]
    fstp    [esi + TREKDATA.SHLD]

    mov     al, 6
    mov     [esi + TREKDATA.INTORPS], al
    mov     [esi + TREKDATA.TORPS], al

    fld     [glb_dbl_3]
    fld     st
    fstp    [esi + TREKDATA.INLSR]
    fstp    [esi + TREKDATA.LSUPRES]

    mov     [esi + TREKDATA.SHLDUP], 0

    fld     [glb_dbl_5]
    fstp    [esi + TREKDATA.WARPFAC]

    fld     [glb_dbl_25]
    fstp    [esi + TREKDATA.WFACSQ]
    jmp     .done

.finished:
    mov     al, 9
    call    TFinish_Main

.done:
    mcEndLocals loc86.size
    ret

; --------------------------------------------------------------------------
; DREPORT
; --------------------------------------------------------------------------
virtual at 0
loc38:
    .nJDAM INT32 ?
    .pDAMAGE PDOUBLE ?
    .nL INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_DamageReport:
    mcBeginLocals loc38.size

    mcZeroBits eax
    mcStoreLocal loc38.nJDAM, eax

    inc     eax
    mcStoreLocal loc38.nL, eax

    mcLoadGameDataPtr esi
    add     esi, TREKDATA.DAMAGE
    mcStoreLocal loc38.pDAMAGE, esi

.check_device:
    fldz
    mcLoadLocal ebx, loc38.pDAMAGE
    fld     tbyte [ebx]
    mc_CMP_ST0_ST1
    jc      .L20
    jz      .L20

    cmp     [esp + loc38.nJDAM], 0
    jne     .L10

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    call    TConsole_ScrollUp
    mov     ecx, 270
    call    TConsole_Prout
    mov     ecx, 271
    call    TConsole_Prout
    inc     [esp + loc38.nJDAM]

.L10:
    mov     cl, 2
    call    TConsole_RepeatBlank

    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr

    mov     dl, 22
    mcLoadLocal ecx, loc38.nL
    add     ecx, 59
    call    TConsole_CramString

    mov     cl, ATTR_REPORT_VALUE
    call    TConsole_SetAttr

    cmp     [esp + loc38.nL], 14
    je      .deathray

    mcLoadLocal esi, loc38.pDAMAGE
    fld     tbyte [esi]
    fld     [glb_dbl_0dot005]
    faddp
    mov     cl, 2
    mov     dl, 5
    call    TConsole_CramFloatWidth
    jmp     .docked_value

.deathray:
    mov     ecx, 269
    call    TConsole_Cram

.docked_value:
    mcLoadLocal esi, loc38.pDAMAGE
    fld     tbyte [esi]
    fld     [glb_dbl_DOCKFAC]
    fmulp
    fld     [glb_dbl_0dot005]
    faddp
    mov     cl, 2
    mov     dl, 10
    call    TConsole_CramFloatWidth
    call    TConsole_ScrollUp

.L20:
    ;
    ; Next device
    ;
    add     [esp + loc38.pDAMAGE], 10
    inc     [esp + loc38.nL]
    cmp     [esp + loc38.nL], 14
    jbe     .check_device

    cmp     [esp + loc38.nJDAM], 0
    jne     .done

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 268
    call    TConsole_Prout

.done:
    call    TConsole_ScrollUp
    mcEndLocals loc38.size
    ret

; --------------------------------------------------------------------------
; This function will check if sufficient time passed between dockings to
; prevent the cheating by docking a few times in a row and getting
; additional photon tubes, quantum charges, etc.
; --------------------------------------------------------------------------
; Returns CF=1 if additional devices are allowed.
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_IsTime4Goodies:
    mcLoadGameDataPtr esi

    fld     [esi + TREKDATA.DATE]
    fld     [esi + TREKDATA.DOCKDATE]
    fsubp
    fld     [glb_dbl_0dot8]
    mc_CMP_ST0_ST1

    ret

; --------------------------------------------------------------------------
; Check if enhanced torpedoes available
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_GetQuantumCharges:
    call    TDock_IsTime4Goodies
    jnc     .done

    mcLoadGameDataPtr esi

    cmp     [esi + TREKDATA.QTORPS], 1
    je      .done
    cmp     [esi + TREKDATA.SKILL], 5
    jb      .done

    mcLoad8bitsToReg32 edx, 100
    call    TRandom_IRan
    inc     eax
    cmp     eax, 25
    ja      .done

    mcLoadGameDataPtr edi
    inc     [edi + TREKDATA.QTORPS]

    mcZeroBits ecx
    cmp     [edi + TREKDATA.SHIP], CHAR_ENTERPRISE
    sete    cl
    mov     al, 8
    shl     al, cl
    mov     [edi + TREKDATA.TORPS], al

.report:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 707
    call    TConsole_Prout

.done:
    ret

; --------------------------------------------------------------------------
; Check if additional photon tubes were installed
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_InstallTubes:
    mcBeginLocals 4

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.SKILL], 5
    jb      .done

    cmp     [esi + TREKDATA.NPTUBES], 3
    ja      .done

    mcLoad8bitsToReg32 edx, 100
    call    TRandom_IRan
    inc     eax
    cmp     eax, 20
    ja      .done

    call    TRandom_Ranf
    fld     [glb_dbl_3]
    fmulp
    call    TCommon_FP_Truncate
    inc     eax
    mov     [esp], eax

    mcLoadGameDataPtr edi
    add     [edi + TREKDATA.NPTUBES], al

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 701
    call    TConsole_Cram

    mov     eax, [esp]
    call    TConsole_CramInt

    mov     ecx, 702
    call    TConsole_Cram

    cmp     dword [esp], 1
    je      .skip

    mov     al, 'S'
    call    TConsole_PutChar

.skip:
    mov     ecx, 703
    call    TConsole_Prout

.done:
    mcEndLocals 4
    ret

; --------------------------------------------------------------------------
; DOCK
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_Main:
    call    TConsole_SetGameMsgAttr

    mcLoadGameDataPtr esi
    mov     [esi + TREKDATA.IDIDIT], 0

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L30

    cmp     [esi + TREKDATA.INORBIT], 0
    jne     .L40

    cmp     [esi + TREKDATA.BASEX], 0
    je      .L5

    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    sub     al, [esi + TREKDATA.BASEX]
    sub     dl, [esi + TREKDATA.BASEY]
    movsx   eax, al
    movsx   edx, dl

    call    TCommon_AbsEAX
    cmp     eax, 1
    ja      .L5

    mov     eax, edx
    call    TCommon_AbsEAX
    cmp     eax, 1
    jbe     .L10

.L5:
    call    TConsole_CramShip
    mov     ecx, 264
    jmp     .msg_end

.L10:
    mcLoadGameDataPtr edi
    mov     [edi + TREKDATA.CONDIT], CONDITION_DOCKED

    mov     ecx, 265
    call    TConsole_Prout

    call    TDock_InstallTubes
    call    TDock_GetQuantumCharges

    mcLoadGameDataPtr edi
    inc     [edi + TREKDATA.IDIDIT]

    fld     [edi + TREKDATA.INSHLD]
    fstp    [edi + TREKDATA.SHLD]

    fld     [edi + TREKDATA.INLSR]
    fstp    [edi + TREKDATA.LSUPRES]

    cmp     [edi + TREKDATA.QTORPS], 1
    je      .check_power

    mov     al, [edi + TREKDATA.INTORPS]
    mov     [edi + TREKDATA.TORPS], al

.check_power:
    fld     [edi + TREKDATA.INENRG]
    fld     [edi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .refuel
    ret

.refuel:
    fld     [edi + TREKDATA.INENRG]
    fstp    [edi + TREKDATA.ENERGY]
    ret

.L30:
    mov     ecx, 266
    jmp     .msg_end

.L40:
    mov     ecx, 267

.msg_end:
    call    TConsole_Prout

    mcLoadGameDataPtr esi
    fld     [esi + TREKDATA.DATE]
    fstp    [esi + TREKDATA.DOCKDATE]
    ret

; --------------------------------------------------------------------------
; HELP
; --------------------------------------------------------------------------
virtual at 0
loc49:
    .pTrekData PVOID ?
    .dbl_DIST DOUBLE ?
    .dbl_XDIST DOUBLE ?
    .dbl_TEMP1 DOUBLE ?
    .dbl_TEMP2 DOUBLE ?
    .dbl_PROBF DOUBLE ?
    .nIX BYTE ?
    .nIY BYTE ?
    .nLINE INDEX ?
    .nL INDEX ?
    .nLL INDEX ?
    .nSUM INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TDock_Help:
    mcBeginLocals loc49.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc49.pTrekData, esi

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L10

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 562
    call    TConsole_Prout
    jmp     .done

.L10:
    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jnc     .L20

    mov     ecx, 563
    call    TConsole_ProutGameMsg
    jmp     .done

.L20:
    mcLoadLocal esi, loc49.pTrekData
    cmp     [esi + TREKDATA.REMBASE], 0
    jne     .L30

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 564
    call    TConsole_Prout
    jmp     .done

.L30:
    cmp     [esi + TREKDATA.LANDED], 1
    jne     .L31

    call    TConsole_SetGameMsgAttr
    mov     ecx, 565
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp
    jmp     .done

.L31:
    cmp     [esi + TREKDATA.ISCRAFT], 0
    jne     .L32

    mov     ecx, 407
    call    TConsole_ProutGameMsg
    jmp     .done

.L32:
    inc     [esi + TREKDATA.NHELP]
    cmp     [esi + TREKDATA.BASEX], 0
    je      .L40

    mov     al, [esi + TREKDATA.BASEX]
    sub     al, [esi + TREKDATA.SECTX]
    movzx   eax, al
    imul    eax, eax

    mov     dl, [esi + TREKDATA.BASEY]
    sub     dl, [esi + TREKDATA.SECTY]
    movzx   edx, dl
    imul    edx, edx

    add     eax, edx
    call    TCommon_FPU_Load_EAX
    fsqrt
    fstp    [esp + loc49.dbl_DIST]
    jmp     .L60

.L40:
    fld     [glb_dbl_1000]
    fstp    [esp + loc49.dbl_DIST]

    movzx   ecx, [esi + TREKDATA.REMBASE]
    lea     edi, [esi + TREKDATA.BASEQX]
    lea     ebx, [esi + TREKDATA.BASEQY]
    mcZeroBits edx

.check_base_distance:
    mov     al, [edi]
    sub     al, [esi + TREKDATA.QUADX]
    movsx   eax, al
    imul    eax, eax
    mcStoreLocal loc49.nSUM, eax

    mov     al, [ebx]
    sub     al, [esi + TREKDATA.QUADY]
    movsx   eax, al
    imul    eax, eax
    add     eax, [esp + loc49.nSUM]

    call    TCommon_FPU_Load_EAX
    fsqrt
    fld     [glb_dbl_Ten]
    fmulp
    fld     st
    fstp    [esp + loc49.dbl_XDIST]

    fld     [esp + loc49.dbl_DIST]
    mc_CMP_ST0_ST1
    jc      .next_base

    fld     [esp + loc49.dbl_XDIST]
    fstp    [esp + loc49.dbl_DIST]
    mcStoreLocal loc49.nLINE, edx

.next_base:
    inc     edi
    inc     ebx
    inc     edx
    loop    .check_base_distance

    lea     edi, [esi + TREKDATA.BASEQX]
    lea     ebx, [esi + TREKDATA.BASEQY]
    mcLoadLocal ecx, loc49.nLINE

    mov     al, [edi + ecx]
    mov     dl, [ebx + ecx]
    mov     [esi + TREKDATA.QUADX], al
    mov     [esi + TREKDATA.QUADY], dl

    inc     [esi + TREKDATA.SHUTUP]
    push    esi
    call    TCommon_NewQuad
    pop     esi
    dec     [esi + TREKDATA.SHUTUP]

.L60:
    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    call    TConsole_SetGameMsgAttr
    mov     ecx, 199
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal esi, loc49.pTrekData
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TConsole_CramLoc
    mov     ecx, 566
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     ecx, 567
    call    TConsole_Prout

    mov     esi, glb_dbl_0dot98
    mcLoadLocalRef edi, loc49.dbl_DIST
    call    TCommon_X_exp_Y
    fld1
    fsubrp
    fstp    [esp + loc49.dbl_TEMP1]

    fld1
    fld     [glb_dbl_3]
    fdivp
    fstp    [esp + loc49.dbl_TEMP2]

    mcLoadLocalRef esi, loc49.dbl_TEMP1
    mcLoadLocalRef edi, loc49.dbl_TEMP2
    call    TCommon_X_exp_Y
    fstp    [esp + loc49.dbl_PROBF]

    mcLoad1 eax
    mcStoreLocal loc49.nL, eax
    call    TConsole_SetGameMsgAttr

.attempt:
    mov     ecx, 567
    add     ecx, [esp + loc49.nL]
    call    TConsole_Cram

    mov     ecx, 571
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     ecx, 572
    call    TConsole_Cram
    invoke  Sleep, 1200

    call    TRandom_Ranf
    fld     [esp + loc49.dbl_PROBF]
    mc_CMP_ST0_ST1
    jc      .L90

.L70:
    mov     ecx, 573
    call    TConsole_Prout

    inc     [esp + loc49.nL]
    cmp     [esp + loc49.nL], 3
    jbe     .attempt

    mov     al, 11
    call    TFinish_Main
    jmp     .done

.L90:
    mcLoad1 eax
    mcStoreLocal loc49.nLL, eax

    mcLoadLocal esi, loc49.pTrekData

.find_adjacent_pos:
    call    TRandom_Ranf
    fld     [glb_dbl_3]
    fmulp
    call    TCommon_FP_Truncate
    add     al, [esi + TREKDATA.BASEX]
    dec     eax
    call    TPhotons_IsValidSector
    jnc     .next_adjacent_try
    mcStoreLocal loc49.nIX, al

    call    TRandom_Ranf
    fld     [glb_dbl_3]
    fmulp
    call    TCommon_FP_Truncate
    add     al, [esi + TREKDATA.BASEY]
    dec     eax
    call    TPhotons_IsValidSector
    jnc     .next_adjacent_try
    mcStoreLocal loc49.nIY, al

    mcLoadLocal al, loc49.nIX
    mcLoadLocal dl, loc49.nIY
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    je      .L110

.next_adjacent_try:
    inc     [esp + loc49.nLL]
    cmp     [esp + loc49.nLL], 5
    jbe     .find_adjacent_pos
    jmp     .L70

.L110:
    push    ebx
    mov     ecx, 574
    call    TConsole_Prout
    pop     ebx

    mcLoadLocal esi, loc49.pTrekData
    mcLoadLocal al, loc49.nIX
    mcLoadLocal dl, loc49.nIY
    mov     [esi + TREKDATA.SECTX], al
    mov     [esi + TREKDATA.SECTY], dl

    mov     al, [esi + TREKDATA.SHIP]
    mov     byte [ebx], al

    call    TCommon_ResetDist
    call    TCommon_SortKlingons
    call    TDock_Main

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 575
    call    TConsole_Prout

.done:
    mcEndLocals loc49.size
    ret

; --- EOF ---
