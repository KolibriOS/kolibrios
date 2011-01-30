; --------------------------------------------------------------------------
; FILE: TNova.Asm
; DATE: October 18, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; NOVA
; --------------------------------------------------------------------------
; Input:
;   AL = X sector of nova
;   DL = Y sector of nova
; --------------------------------------------------------------------------
virtual at 0
loc54:
    .pTrekData PVOID ?
    .arr_HitsX BYTES 12
    .arr_HitsY BYTES 12
    .bIHURT BOOL ?
    .nBot INT32 ?
    .nTop INT32 ?
    .nTop2 INT32 ?
    .nKOUNT INT32 ?
    .nII INT32 ?
    .nJJ INT32 ?
    .nJ INT32 ?
    .nMM INT32 ?
    .nNN INT32 ?
    .nLL INT32 ?
    .nIX BYTE ?
    .nICX BYTE ?
    .nIY BYTE ?
    .nICY BYTE ?
    .chIQUAD BYTE ?
    .chIQUAD1 BYTE ?
    .nNewCX BYTE ?
    .nNewCY BYTE ?
    .arr_DblCourse DOUBLES 9
    .dbl_HIT DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TNova_Main:
    mcBeginLocals loc54.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc54.pTrekData, esi

    mcStoreLocal loc54.nIX, al
    mcStoreLocal loc54.nIY, dl

    fld     [glb_dbl_0dot05]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .do_supernova

    mcLoadLocal al, loc54.nIX
    mcLoadLocal dl, loc54.nIY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mov     bl, CHAR_ASTERISK
    mcLoadLocal al, loc54.nIX
    mcLoadLocal dl, loc54.nIY
    call    TConsole_CramEnemyAt
    mov     ecx, 613
    call    TConsole_Prout

    call    TArray_MyGalaxyPtr
    dec     dword [ebx]
    ;
    ; IF (IPHWHO != 1) STARKL++
    ;
    mcLoadLocal esi, loc54.pTrekData
    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .setup_stack

    inc     [esi + TREKDATA.STARKL]

.setup_stack:
    mcZeroBits eax
    mcStoreLocal loc54.nKOUNT, eax
    mcStoreLocal loc54.nICX, al
    mcStoreLocal loc54.nICY, al
    inc     eax
    mcStoreLocal loc54.nBot, eax
    mcStoreLocal loc54.nTop, eax
    mcStoreLocal loc54.nTop2, eax
    ;
    ; Also setup COURSE array
    ;
    mcLoadLocalRef edi, loc54.arr_DblCourse
    fld     [glb_dbl_10dot5]
    fstp    tbyte [edi]
    fld     [glb_dbl_12]
    fstp    tbyte [edi + 10]
    fld     [glb_dbl_1dot5]
    fstp    tbyte [edi + 20]
    fld     [glb_dbl_9]
    fstp    tbyte [edi + 30]
    fldz
    fstp    tbyte [edi + 40]
    fld     [glb_dbl_3]
    fstp    tbyte [edi + 50]
    fld     [glb_dbl_7dot5]
    fstp    tbyte [edi + 60]
    fld     [glb_dbl_6]
    fstp    tbyte [edi + 70]
    fld     [glb_dbl_4dot5]
    fstp    tbyte [edi + 80]
    ;
    ; HITS (BOT,1) = IX
    ; HITS (BOT,2) = IY
    ;
    mcLoadLocalRef esi, loc54.arr_HitsX
    mcLoadLocalRef edi, loc54.arr_HitsY
    mcLoadLocal al, loc54.nIX
    mcLoadLocal dl, loc54.nIY
    mov     [esi], al
    mov     [edi], dl

.L78:
    ;
    ; DO 90 MM=BOT,TOP
    ;
    mcLoadLocal eax, loc54.nBot
    mcStoreLocal loc54.nMM, eax

.MM_incremented:
    ;
    ; DO 90 NN=1,3
    ;
    mcLoad1 eax
    mcStoreLocal loc54.nNN, eax

.NN_incremented:
    ;
    ; DO 90 J=1,3
    ;
    mcLoad1 eax
    mcStoreLocal loc54.nJ, eax

.J_incremented:
    ;
    ; IF (J*NN == 4) GOTO 90
    ;
    mcLoadLocal ecx, loc54.nJ
    imul    ecx, [esp + loc54.nNN]
    mcOnRegEqu ecx, 4, .next_J
    ;
    ; II = HITS (MM,1) + NN - 2
    ; JJ = HITS (MM,2) + J - 2
    ;
    mcLoadLocalRef esi, loc54.arr_HitsX
    mcLoadLocalRef edi, loc54.arr_HitsY
    mcLoadLocal ecx, loc54.nMM
    dec     ecx
    movzx   eax, byte [esi + ecx]
    movzx   edx, byte [edi + ecx]
    add     eax, [esp + loc54.nNN]
    add     edx, [esp + loc54.nJ]
    sub     eax, 2
    sub     edx, 2
    mcStoreLocal loc54.nII, eax
    mcStoreLocal loc54.nJJ, edx
    ;
    ; Validate sector II,JJ
    ;
    call    TPhotons_IsValidSector
    jnc     .next_J

    mov     eax, edx
    call    TPhotons_IsValidSector
    jnc     .next_J
    ;
    ; IQUAD = QUAD (II,JJ)
    ;
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TArray_QuadPtr
    mov     cl, [ebx]
    mcStoreLocal loc54.chIQUAD, cl
    ;
    ; Space, black hole, "thing", tholian or its web are not affected
    ;
    mcOnRegEqu cl, CHAR_COSMOS, .next_J
    mcOnRegEqu cl, CHAR_THING, .next_J
    mcOnRegEqu cl, CHAR_THOLIAN, .next_J
    mcOnRegEqu cl, CHAR_WEB, .next_J
    mcOnRegEqu cl, CHAR_BLACK_HOLE, .next_J
    ;
    ; IF (IQUAD != '*') GOTO 80
    ;
    mcOnRegNotEqu cl, CHAR_ASTERISK, .L80
    ;
    ; Another star affected by nova
    ;
    fld     [glb_dbl_0dot05]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .do_secondary_supernova
    ;
    ; TOP2++
    ; HITS (TOP2,1) = II
    ; HITS (TOP2,2) = JJ
    ;
    inc     [esp + loc54.nTop2]
    mov     ecx, [esp + loc54.nTop2]
    dec     ecx
    mcLoadLocalRef esi, loc54.arr_HitsX
    mcLoadLocalRef edi, loc54.arr_HitsY
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    mov     [esi + ecx], al
    mov     [edi + ecx], dl
    ;
    ; Decrease star count in the quadrant
    ;
    call    TArray_MyGalaxyPtr
    dec     dword [ebx]
    ;
    ; IF (IPHWHO != 1) STARKL++
    ;
    mcLoadLocal esi, loc54.pTrekData
    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .next_nova_msg

    inc     [esi + TREKDATA.STARKL]

.next_nova_msg:
    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mov     bl, CHAR_ASTERISK
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TConsole_CramEnemyAt
    mov     ecx, 613
    call    TConsole_Cram
    jmp     .L8905

.L80:
    cmp     [esp + loc54.chIQUAD], CHAR_PLANET
    jne     .L8002
    ;
    ; Planet destroyed by nova
    ;
    call    TArray_MyNewStufPtr
    dec     byte [ebx]
    ;
    ; IF (IPHWHO != 1) NPLANKL++
    ;
    mcLoadLocal esi, loc54.pTrekData
    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .planet_killed_msg

    inc     [esi + TREKDATA.NPLANKL]

.planet_killed_msg:
    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mov     bl, CHAR_PLANET
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TConsole_CramEnemyAt
    mov     ecx, 306
    call    TConsole_Cram

    mcLoadLocal esi, loc54.pTrekData
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr
    mcZeroBits eax
    stosd
    stosb

    mov     [esi + TREKDATA.IPLANET], al
    mov     [esi + TREKDATA.PLNETX], al
    mov     [esi + TREKDATA.PLNETY], al

    cmp     [esi + TREKDATA.LANDED], 1
    jne     .L8905

    mov     al, 16
    call    TFinish_Main
    jmp     .done

.L8002:
    cmp     [esp + loc54.chIQUAD], CHAR_STARBASE
    jne     .L82
    ;
    ; Nova destroys starbase
    ;
    call    TArray_MyGalaxyPtr
    sub     dword [ebx], 10

    mcLoadLocal esi, loc54.pTrekData
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TEvents_RemoveThisBase

    mcLoadLocal esi, loc54.pTrekData
    dec     [esi + TREKDATA.REMBASE]
    mov     [esi + TREKDATA.BASEX], 0
    mov     [esi + TREKDATA.BASEY], 0
    ;
    ; IF (IPHWHO != 1) BASEKL++
    ;
    mcLoadLocal esi, loc54.pTrekData
    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .base_killed_msg

    inc     [esi + TREKDATA.BASEKL]

.base_killed_msg:
    mov     [esi + TREKDATA.CONDIT], 0
    call    TCommon_NewCondition
    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mov     bl, CHAR_STARBASE
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TConsole_CramEnemyAt
    mov     ecx, 306
    call    TConsole_Cram
    jmp     .L8905
    ;
    ; HIT = 800.0 + 800.0*RANF(0)
    ;
.L82:
    call    TRandom_Ranf
    fld     [glb_dbl_800]
    fmulp
    fld     [glb_dbl_800]
    faddp
    fstp    [esp + loc54.dbl_HIT]

    mcLoadLocal esi, loc54.pTrekData
    mov     al, [esi + TREKDATA.SHIP]
    cmp     [esp + loc54.chIQUAD], al
    jne     .L87
    ;
    ; Starship hit by nova!
    ;
    call    TConsole_SetGameMsgAttr
    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip
    mov     ecx, 614
    call    TConsole_Cram

    mcLoadLocal esi, loc54.pTrekData
    mov     [esi + TREKDATA.KSHOT], 0

    mcLoadLocalRef esi, loc54.dbl_HIT
    mcLoadLocalRef edi, loc54.bIHURT
    mcZeroBits ecx
    call    TAttack_Zap

    mcLoadLocal esi, loc54.pTrekData
    fldz
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jnc     .L86

    mov     al, 7
    call    TFinish_Main
    jmp     .done

.L86:
    ;
    ; Nova kicks starship away
    ;
    mcLoadLocal ecx, loc54.nMM
    dec     ecx
    mcLoadLocalRef ebx, loc54.arr_HitsX
    mcLoadLocalRef edx, loc54.arr_HitsY
    mov     al, [ebx + ecx]
    mov     dl, [edx + ecx]

    mov     cl, [esi + TREKDATA.SECTX]
    sub     cl, al
    add     [esp + loc54.nICX], cl

    mov     cl, [esi + TREKDATA.SECTY]
    sub     cl, al
    add     [esp + loc54.nICY], cl

    inc     [esp + loc54.nKOUNT]
    jmp     .next_J

.L87:
    ;
    ; Enemy ship hit by nova!
    ;
    cmp     [esp + loc54.chIQUAD], CHAR_KLINGON
    je      .L88
    ;
    ; Find enemy index in KX,KY arrays
    ;
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    mcLoadLocal esi, loc54.pTrekData
    movzx   ecx, [esi + TREKDATA.NENHERE]
    lea     edi, [esi + TREKDATA.KY]
    add     esi, TREKDATA.KX
    mcZeroBits ebx

.find_enemy_ship:
    inc     ebx
    cmp     [esi], al
    jne     .next_ship
    cmp     [edi], dl
    jne     .next_ship

    mcStoreLocal loc54.nLL, ebx
    jmp     .hit_the_enemy

.next_ship:
    inc     esi
    inc     edi
    loop    .find_enemy_ship

    ;int     3
    jmp     .next_J

.hit_the_enemy:
    ;
    ; KPOWER (LL) -= HIT
    ;
    mcLoadLocal esi, loc54.pTrekData
    lea     ebx, [esi + TREKDATA.KPOWER]
    mcLoadLocal ecx, loc54.nLL
    dec     ecx
    imul    ecx, 10
    fld     tbyte [ebx + ecx]
    fld     [esp + loc54.dbl_HIT]
    fsubp
    fstp    tbyte [ebx + ecx]

    fldz
    fld     tbyte [ebx + ecx]
    mc_CMP_ST0_ST1
    jc      .L88
    ;
    ; NEWCX = II + II - HITS (MM,1)
    ; NEWCY = JJ + JJ - HITS (MM,2)
    ;
    mcLoadLocalRef esi, loc54.arr_HitsX
    mcLoadLocalRef edi, loc54.arr_HitsY
    mcLoadLocal ecx, loc54.nMM
    dec     ecx
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    add     eax, eax
    add     edx, edx
    sub     al, [esi + ecx]
    sub     dl, [edi + ecx]
    mcStoreLocal loc54.nNewCX, al
    mcStoreLocal loc54.nNewCY, dl

    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mcLoadLocal bl, loc54.chIQUAD
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TConsole_CramEnemyAt
    mov     ecx, 615
    call    TConsole_Cram

    movzx   eax, [esp + loc54.nNewCX]
    call    TPhotons_IsValidSector
    jnc     .L8703

    movzx   eax, [esp + loc54.nNewCY]
    call    TPhotons_IsValidSector
    jnc     .L8703
    ;
    ; IQUAD1 = QUAD (NEWCX,NEWCY)
    ;
    mcLoadLocal al, loc54.nNewCX
    mcLoadLocal dl, loc54.nNewCY
    call    TArray_QuadPtr
    mov     cl, [ebx]
    mcStoreLocal loc54.chIQUAD1, cl

    mcOnRegNotEqu cl, CHAR_BLACK_HOLE, .L87025
    ;
    ; Enemy displaced into black hole!
    ;
    mov     ecx, 616
    call    TConsole_Prout
    jmp     .L88

.L87025:
    mcOnRegNotEqu cl, CHAR_COSMOS, .L8703

    mov     ecx, 617
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal al, loc54.nNewCX
    mcLoadLocal dl, loc54.nNewCY
    call    TConsole_CramLoc

    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    mcLoadLocal al, loc54.nNewCX
    mcLoadLocal dl, loc54.nNewCY
    call    TArray_QuadPtr
    mcLoadLocal cl, loc54.chIQUAD
    mov     [ebx], cl

    mcLoadLocal al, loc54.nNewCX
    mcLoadLocal dl, loc54.nNewCY
    mcLoadLocal ecx, loc54.nLL
    dec     ecx
    mcLoadLocal esi, loc54.pTrekData
    lea     edi, [esi + TREKDATA.KY]
    add     esi, TREKDATA.KX
    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    imul    ecx, 10
    mcLoadLocal esi, loc54.pTrekData
    lea     edi, [esi + TREKDATA.KDIST]
    sub     al, [esi + TREKDATA.SECTX]
    sub     dl, [esi + TREKDATA.SECTY]
    movsx   eax, al
    movsx   edx, dl
    imul    eax, eax
    imul    edx, edx
    add     eax, edx
    call    TCommon_FPU_Load_EAX
    fsqrt
    fstp    tbyte [edi + ecx]

.L8703:
    call    TConsole_ScrollUp
    jmp     .next_J

.L88:
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TPhasers_DeadKlingon
    jmp     .next_J

.L8905:
    call    TConsole_ScrollUp
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

.next_J:
    inc     [esp + loc54.nJ]
    cmp     [esp + loc54.nJ], 3
    jbe     .J_incremented

    inc     [esp + loc54.nNN]
    cmp     [esp + loc54.nNN], 3
    jbe     .NN_incremented

    inc     [esp + loc54.nMM]
    mov     eax, [esp + loc54.nTop]
    cmp     [esp + loc54.nMM], eax
    jbe     .MM_incremented
    ;
    ; Check if more stars affected by nova
    ;
    mov     eax, [esp + loc54.nTop]
    cmp     [esp + loc54.nTop2], eax
    je      .L93

    inc     eax
    mcStoreLocal loc54.nBot, eax
    mov     eax, [esp + loc54.nTop2]
    mcStoreLocal loc54.nTop, eax
    jmp     .L78

.L93:
    cmp     [esp + loc54.nKOUNT], 0
    je      .done
    ;
    ; Starship affected by nova - kick it away
    ;
    mcLoadLocal esi, loc54.pTrekData
    fild    [esp + loc54.nKOUNT]
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esi + TREKDATA.DIST]

    cmp     [esp + loc54.nICX], 0
    je      .L9301

    mcLoad1 eax
    movsx   ecx, [esp + loc54.nICX]
    call    TAttack_ISign
    mcStoreLocal loc54.nICX, al

.L9301:
    cmp     [esp + loc54.nICY], 0
    je      .L9302

    mcLoad1 eax
    movsx   ecx, [esp + loc54.nICY]
    call    TAttack_ISign
    mcStoreLocal loc54.nICY, al

.L9302:
    ;
    ; INDEX = 3*(ICX+1) + ICY + 2
    ;
    movsx   ecx, [esp + loc54.nICX]
    inc     ecx
    imul    ecx, 3
    movsx   eax, [esp + loc54.nICY]
    add     ecx, eax
    add     ecx, 2

    cmp     ecx, 1
    jb      .assertion_failed
    cmp     ecx, 9
    ja      .assertion_failed

    dec     ecx
    imul    ecx, 10
    mcLoadLocalRef ebx, loc54.arr_DblCourse
    fld     tbyte [ebx + ecx]
    fstp    [esi + TREKDATA.DIREC]

    fld     [esi + TREKDATA.DIREC]
    fldz
    mc_CMP_ST0_ST1
    jnz     .check_distance

    fldz
    fstp    [esi + TREKDATA.DIST]

.check_distance:
    fld     [esi + TREKDATA.DIST]
    fldz
    mc_CMP_ST0_ST1
    jz      .done

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_12]
    fmulp
    fstp    [esi + TREKDATA.TIME]

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 618
    call    TConsole_Cram
    call    TConsole_CramShip

    call    TMove_Move
    jmp     .done

.do_secondary_supernova:
    mcLoadLocal eax, loc54.nII
    mcLoadLocal edx, loc54.nJJ
    jmp     .call_snova

.assertion_failed:
    ;int     3
    jmp     .done

.do_supernova:
    mcLoadLocal al, loc54.nIX
    mcLoadLocal dl, loc54.nIY

.call_snova:
    call    TNova_SuperNova

.done:
    mcEndLocals loc54.size
    ret

; --------------------------------------------------------------------------
; SNOVA
; --------------------------------------------------------------------------
; Input:
;   AL = X sector of supernova
;   DL = Y sector of supernova
; --------------------------------------------------------------------------
virtual at 0
loc55:
    .pTrekData PVOID ?
    .pChart PINT32 ?
    .pGalaxy PINT32 ?
    .iNSX INDEX ?
    .iNSY INDEX ?
    .iNUM INDEX ?
    .iNQX INDEX ?
    .iNQY INDEX ?
    .nInputSectorX BYTE ?
    .nInputSectorY BYTE ?
    .__pad_1 BYTE ?
    .__pad_2 BYTE ?
    .nKLDEAD COUNT ?
    .nCOMDEAD COUNT ?
    .nISCDEAD COUNT ?
    .nRMDEAD COUNT ?
    .nNPDEAD COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TNova_SuperNova:
    mcBeginLocals loc55.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc55.pTrekData, esi

    movzx   eax, al
    movzx   edx, dl
    mcStoreLocal loc55.iNSX, eax
    mcStoreLocal loc55.iNSY, edx

    mcStoreLocal loc55.nInputSectorX, al
    mcStoreLocal loc55.nInputSectorY, dl

    test    eax, edx
    jnz     .L50

    mov     edx, [esi + TREKDATA.INSTAR]
    call    TRandom_IRan
    inc     eax
    mcStoreLocal loc55.iNUM, eax

    mcLoad1 eax
    mcStoreLocal loc55.iNQX, eax

._NQX_incremented:
    mcLoad1 eax
    mcStoreLocal loc55.iNQY, eax

._NQY_incremented:
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_GetGalaxyValue
    mcOnRegEqu ecx, 1000, .done

    mov     eax, ecx
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 10
    div     ecx
    sub     [esp + loc55.iNUM], edx
    js      .L20
    jz      .L20

.next_quad:
    inc     [esp + loc55.iNQY]
    cmp     [esp + loc55.iNQY], 8
    jbe     ._NQY_incremented

    inc     [esp + loc55.iNQX]
    cmp     [esp + loc55.iNQX], 8
    jbe     ._NQX_incremented

    jmp     .done

.L20:
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_GetGalaxyValue
    mcOnRegEqu ecx, 1000, .done

    mcLoadLocal esi, loc55.pTrekData
    mcLoadLocal eax, loc55.iNQX
    cmp     al, [esi + TREKDATA.QUADX]
    jne     .L70

    mcLoadLocal eax, loc55.iNQY
    cmp     al, [esi + TREKDATA.QUADY]
    jne     .L70
    ;
    ; The supernova is in this quadrant!
    ;
    cmp     [esi + TREKDATA.JUSTIN], 0
    jne     .L70

    call    TArray_MyGalaxyPtr
    mov     eax, [ebx]
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 10
    div     ecx
    mcOnRegZero edx, .done
    call    TRandom_IRan
    inc     eax
    mcStoreLocal loc55.iNUM, eax

    mcLoadLocal esi, loc55.pTrekData
    add     esi, TREKDATA.QUAD
    mcLoad8bitsToReg32 ecx, 100
    mcZeroBits edx

.find_star:
    lodsb
    mcOnRegNotEqu al, CHAR_ASTERISK, .next_star

    dec     [esp + loc55.iNUM]
    jz      .L50

.next_star:
    inc     edx
    loop    .find_star
    jmp     .done

.L50:
    call    TConsole_ScrollUp
    call    TMove_RedAlert
    call    TConsole_SetGameMsgAttr
    mov     ecx, 619
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal eax, loc55.iNSX
    mcLoadLocal edx, loc55.iNSY
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc55.pTrekData
    movzx   eax, [esi + TREKDATA.QUADX]
    movzx   edx, [esi + TREKDATA.QUADY]
    mcStoreLocal loc55.iNQX, eax
    mcStoreLocal loc55.iNQY, edx
    ;
    ; Supernova adjacent to ship ends game!
    ;
    movzx   eax, [esi + TREKDATA.SECTX]
    sub     eax, [esp + loc55.iNSX]
    imul    eax, eax

    movzx   ecx, [esi + TREKDATA.SECTY]
    sub     ecx, [esp + loc55.iNSY]
    imul    ecx, ecx

    add     eax, ecx
    cmp     ecx, 2
    ja      .L80

    mov     ecx, 620
    call    TConsole_ProutGameMsg
    call    TPlanet_Stars

    mcLoadLocal edi, loc55.pTrekData
    mov     [edi + TREKDATA.ALLDONE], 1
    jmp     .L80

.L70:
    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jc      .L80

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 621
    call    TConsole_Cram

    mcLoadLocal esi, loc55.pTrekData
    mov     cl, 1
    fld     [esi + TREKDATA.DATE]
    call    TConsole_CramFloat
    call    TConsole_ScrollUp

    mov     ecx, 622
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TConsole_CramLoc

    mov     ecx, 623
    call    TConsole_Prout

.L80:
    ;
    ; A wave of total destruction in the quadrant!!
    ;
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_GetGalaxyValue
    mcStoreLocal loc55.iNUM, ecx

    mcLoad8bitsToReg32 eax, 100
    xchg    eax, ecx
    mcZeroBits edx
    div     ecx
    mcStoreLocal loc55.nKLDEAD, eax

    mcZeroBits eax
    mcStoreLocal loc55.nCOMDEAD, eax
    mcStoreLocal loc55.nISCDEAD, eax

    mcLoadLocal esi, loc55.pTrekData
    mcLoadLocal eax, loc55.iNQX
    cmp     al, [esi + TREKDATA.ISX]
    jne     .L85

    mcLoadLocal eax, loc55.iNQY
    cmp     al, [esi + TREKDATA.ISY]
    jne     .L85

    mcZeroBits eax
    mov     [esi + TREKDATA.NSCREM], al
    mov     [esi + TREKDATA.ISX], al
    mov     [esi + TREKDATA.ISY], al
    mov     [esi + TREKDATA.ISATB], al
    mov     [esi + TREKDATA.ISCATE], al
    inc     [esp + loc55.nISCDEAD]

    fld     [glb_dbl_1E38]
    fld     st
    fstp    [esi + TREKDATA.FUTURE6]
    fstp    [esi + TREKDATA.FUTURE7]

.L85:
    cmp     [esp + loc55.nKLDEAD], 0
    je      .L100

    mcLoadLocal eax, loc55.nKLDEAD
    sub     [esi + TREKDATA.REMKL], al

    cmp     [esi + TREKDATA.REMCOM], 0
    je      .L100

    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TPhasers_RemoveCommander
    jnc     .L100

    dec     [esp + loc55.nKLDEAD]
    inc     [esp + loc55.nCOMDEAD]

    mcLoadLocal esi, loc55.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    jne     .L100

    fld     [glb_dbl_1E38]
    fstp    [esi + TREKDATA.FUTURE2]

.L100:
    ;
    ; Destroy Romulans and planets
    ;
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_NewStufPtr
    movzx   ecx, byte [ebx]
    mcStoreLocal loc55.iNUM, ecx
    mcZeroBits eax
    mov     [ebx], al
    ;
    ; NRMDEAD = NUM/10
    ;
    mov     eax, ecx
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 10
    div     ecx
    mcStoreLocal loc55.nRMDEAD, eax
    mcStoreLocal loc55.nNPDEAD, edx

    mcLoadLocal esi, loc55.pTrekData
    sub     [esi + TREKDATA.NROMREM], al

    cmp     [esp + loc55.nNPDEAD], 0
    je      .L109
    ;
    ; Remove planet in quadrant NQX,NQY
    ;
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_FindPlanet
    jecxz   .L109

    mov     edi, esi
    mcZeroBits eax
    stosd
    stosb

.L109:
    ;
    ; Destroy base (if any) in quadrant NQX,NQY
    ;
    mcLoadLocal esi, loc55.pTrekData
    cmp     [esi + TREKDATA.REMBASE], 0
    je      .L120

    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TEvents_RemoveThisBase
    jnc     .L120

    mcLoadLocal esi, loc55.pTrekData
    dec     [esi + TREKDATA.REMBASE]

.L120:
    ;
    ; If starship caused supernova, tally up the destruction
    ;
    cmp     [esp + loc55.iNSX], 0
    je      .L130

    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_GetGalaxyValue
    mov     eax, ecx
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 100
    div     ecx
    mcStoreLocal loc55.iNUM, edx

    mcLoadLocal eax, loc55.nKLDEAD
    mcLoadLocal edx, loc55.nCOMDEAD
    mcLoadLocal ecx, loc55.nRMDEAD
    mcLoadLocal ebx, loc55.nISCDEAD

    mcLoadLocal esi, loc55.pTrekData
    add     [esi + TREKDATA.KILLK], al
    add     [esi + TREKDATA.KILLC], dl
    add     [esi + TREKDATA.NROMKL], cl
    add     [esi + TREKDATA.NSCKILL], bl

    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .L130

    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 10
    mcLoadLocal eax, loc55.iNUM
    div     ecx

    add     [esi + TREKDATA.BASEKL], al
    add     [esi + TREKDATA.STARKL], dl

    mcLoadLocal eax, loc55.nNPDEAD
    add     [esi + TREKDATA.NPLANKL], al

.L130:
    ;
    ; Mark supernova in galaxy and in star chart
    ;
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_StarChartPtr
    mcStoreLocal loc55.pChart, ebx

    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TArray_GalaxyPtr
    mcStoreLocal loc55.pGalaxy, ebx

    mcLoadLocal esi, loc55.pChart
    cmp     dword [esi], 0
    jle     .L131

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jnc     .L131

    mcLoadLocal esi, loc55.pGalaxy
    mov     eax, [esi]
    add     eax, 1000
    mcLoadLocal edi, loc55.pChart
    mov     [edi], eax

.L131:
    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jnc     .mark_chart

    mcLoadLocal esi, loc55.pTrekData
    mcLoadLocal eax, loc55.iNQX
    cmp     [esi + TREKDATA.QUADX], al
    jne     .L132

    mcLoadLocal eax, loc55.iNQY
    cmp     [esi + TREKDATA.QUADY], al
    jne     .L132

.mark_chart:
    mcLoadLocal edi, loc55.pChart
    mov     dword [edi], 1

.L132:
    mcLoadLocal edi, loc55.pGalaxy
    mov     dword [edi], 1000
    ;
    ; If supernova destroys last klingons, give special message
    ;
    mcLoadLocal esi, loc55.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    jne     .L140

    mcLoadLocal eax, loc55.iNQX
    cmp     [esi + TREKDATA.QUADX], al
    jne     .win_by_supernova

    mcLoadLocal eax, loc55.iNQY
    cmp     [esi + TREKDATA.QUADY], al
    je      .L140

.win_by_supernova:
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    mov     ecx, 624
    call    TConsole_ProutGameMsg

    mov     ecx, 625
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal eax, loc55.iNQX
    mcLoadLocal edx, loc55.iNQY
    call    TConsole_CramLoc

    mov     ecx, 626
    call    TConsole_Prout

    mov     al, 1
    call    TFinish_Main
    jmp     .done

.L140:
    mcLoadLocal esi, loc55.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    je      .done

    mov     al, 8
    call    TFinish_Main

.done:
    mcEndLocals loc55.size
    ret

; --------------------------------------------------------------------------
; AUTOVER, GRAB
; --------------------------------------------------------------------------
; Input:
;   AL = finishing code if game will end (II in original source)
;   CL =0 if AUTOVER called, =1 if GRAB called (IGRAB in original source)
; --------------------------------------------------------------------------
virtual at 0
loc53:
    .pTrekData PVOID ?
    .bIGRAB BYTE ?
    .bII BYTE ?
    .pad_1 BYTE ?
    .pad_2 BYTE ?
    .dbl_POWER DOUBLE ?
    .dbl_DISTREQ DOUBLE ?
    .dbl_DISTMAX DOUBLE ?
    .pad_11 BYTE ?
    .pad_22 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TNova_AutomaticOverride:
    mcBeginLocals loc53.size

    mcStoreLocal loc53.bIGRAB, cl
    mcStoreLocal loc53.bII, al

    mcLoadGameDataPtr esi
    mcStoreLocal loc53.pTrekData, esi

    cmp     [esi + TREKDATA.LANDED], 1
    jne     .L45

    mov     cl, DEV_TRANSPORTER
    call    TArray_IsDamaged
    jc      .L25

    mov     ecx, 600
    call    TConsole_ProutGameMsg

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L20

    mov     ecx, 601
    call    TConsole_Prout
    jmp     .L25

.L20:
    mov     ecx, 602
    call    TConsole_Cram
    invoke  Sleep, 1200

    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    mc_CMP_ST0_ST1
    jc      .L30

    mov     ecx, 573
    call    TConsole_Prout

.L25:
    mcLoadLocal al, loc53.bII

.finished:
    call    TFinish_Main
    jmp     .done

.L30:
    mov     ecx, 574
    call    TConsole_Prout

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.IMINE], 0
    je      .L45

    mov     [esi + TREKDATA.IMINE], 0

    mov     ecx, 603
    call    TConsole_Cram

    call    TRandom_Ranf
    fld     [glb_dbl_DOCKFAC]
    mc_CMP_ST0_ST1
    jc      .L40

    mov     ecx, 604
    call    TConsole_Prout
    jmp     .L45

.L40:
    mov     ecx, 605
    call    TConsole_Prout

    mcLoadLocal esi, loc53.pTrekData
    mov     [esi + TREKDATA.ICRYSTL], 1

.L45:
    cmp     [esp + loc53.bIGRAB], 0
    jne     .done

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.ICRAFT], 0
    je      .try_override

    mov     al, 17
    jmp     .finished

.try_override:
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.JUSTIN], 0
    je      .L50

.L47:
    call    TMove_RedAlert
    call    TConsole_SetGameMsgAttr

    mov     ecx, 606
    call    TConsole_Cram
    call    TConsole_CramShip

    mov     ecx, 607
    call    TConsole_Prout
    mov     ecx, 608
    call    TConsole_Prout
    call    TConsole_ScrollUp

.L50:
    call    TConsole_SetGameMsgAttr
    mov     ecx, 609
    call    TConsole_Cram
    call    TConsole_CramShip
    call    TConsole_ScrollUp

    mov     ecx, 610
    call    TConsole_Prout
    ;
    ; STARCH (QUADX,QUADY) = 1
    ;
    mcLoad1 ecx
    mcLoadLocal esi, loc53.pTrekData
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TArray_SetStarChartValue
    ;
    ; Try to use warp engines...
    ;
    mov     cl, DEV_WARP_ENGINES
    call    TArray_IsDamaged
    jnc     .L100

    call    TConsole_ScrollUp
    mov     ecx, 150
    call    TConsole_ProutGameMsg
    mov     al, 8
    jmp     .finished

.L100:
    call    TRandom_Ranf
    fld     [glb_dbl_2]
    fmulp
    fld     [glb_dbl_6]
    faddp
    mcLoadLocal esi, loc53.pTrekData
    fstp    [esi + TREKDATA.WARPFAC]

    fld     [esi + TREKDATA.WARPFAC]
    fld     st
    fmulp
    fstp    [esi + TREKDATA.WFACSQ]

    call    TConsole_SetGameMsgAttr
    mov     ecx, 611
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal esi, loc53.pTrekData
    fld     [esi + TREKDATA.WARPFAC]
    call    TConsole_CramFloat
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc53.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_0dot75]
    fmulp
    fstp    [esp + loc53.dbl_POWER]

    fld     [esi + TREKDATA.WARPFAC]
    fld     st
    fld     st
    fmulp
    fmulp

    mov     al, [esi + TREKDATA.SHLDUP]
    call    TCommon_LoadByteIntoFPU
    fmulp

    fld     [esp + loc53.dbl_POWER]
    fdivrp
    fstp    [esp + loc53.dbl_DISTMAX]

    call    TRandom_Ranf
    fld     [glb_dbl_2]
    fmulp
    fld     [glb_dbl_1dot4142]
    faddp
    fstp    [esp + loc53.dbl_DISTREQ]

    fld     [esp + loc53.dbl_DISTREQ]
    fld     [esp + loc53.dbl_DISTMAX]
    call    TCommon_FPU_Min
    mcLoadLocal esi, loc53.pTrekData
    fstp    [esi + TREKDATA.DIST]

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esi + TREKDATA.WFACSQ]
    fdivp
    fstp    [esi + TREKDATA.TIME]

    call    TRandom_Ranf
    fld     [glb_dbl_12]
    fmulp
    mcLoadLocal esi, loc53.pTrekData
    fstp    [esi + TREKDATA.DIREC]

    mcZeroBits eax
    mov     [esi + TREKDATA.JUSTIN], al
    mov     [esi + TREKDATA.INORBIT], al

    invoke  Sleep, 1000
    call    TMove_WarpX

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    cmp     [esi + TREKDATA.JUSTIN], 0
    jne     .L200

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip
    mov     ecx, 612
    call    TConsole_Prout
    mov     al, 8
    jmp     .finished

.L200:
    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    je      .L47

    mcLoadLocal esi, loc53.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    jne     .done

    mov     al, 1
    jmp     .finished

.done:
    mcEndLocals loc53.size
    ret

; --- EOF ---
