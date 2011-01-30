; --------------------------------------------------------------------------
; FILE: TPlanet.Asm
; DATE: November 4, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; STARS
; --------------------------------------------------------------------------
align PROC_ALIGN
TPlanet_Stars:
    mov     ecx, 434
    call    TConsole_ProutGameMsg
    ret

; --------------------------------------------------------------------------
; KABOOM
; --------------------------------------------------------------------------
virtual at 0
loc46:
    .pTrekData PVOID ?
    .dbl_WHAMMO DOUBLE ?
    .byte_IsKilled BYTE ?
    .byte_2 BYTE ?
    .pKPower PDOUBLE ?
    .pKDist PDOUBLE ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPlanet_KaBoom:
    mcBeginLocals loc46.size

    call    TPlanet_Stars

    mcLoadGameDataPtr esi
    mcStoreLocal loc46.pTrekData, esi
    cmp     [esi + TREKDATA.SHIP], CHAR_ENTERPRISE
    jne     .print

    call    TConsole_Cram3Asterisks

.print:
    mov     ecx, 472
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     ecx, 473
    call    TConsole_Prout

    call    TPlanet_Stars
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc46.pTrekData
    cmp     [esi + TREKDATA.NENHERE], 0
    je      .L20

    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_25]
    fmulp
    fstp    [esp + loc46.dbl_WHAMMO]

.outer_loop:
    mcLoadLocal ebx, loc46.pTrekData
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mcStoreLocal loc46.pKX, esi
    mcStoreLocal loc46.pKY, edi

    movzx   ecx, [ebx + TREKDATA.NENHERE]
    jecxz   .L20

    mcLoadMemberRef esi, TREKDATA.KPOWER
    mcLoadMemberRef edi, TREKDATA.KDIST

.check_whammo:
    fld     tbyte [esi]
    fld     tbyte [edi]
    fmulp
    fld     [esp + loc46.dbl_WHAMMO]
    mc_CMP_ST0_ST1
    jc      .next_whammo

    mcLoadLocal esi, loc46.pKX
    mcLoadLocal edi, loc46.pKY
    mov     al, [esi]
    mov     dl, [edi]
    call    TPhasers_DeadKlingon
    invoke  Sleep, 500
    jmp     .outer_loop

.next_whammo:
    inc     [esp + loc46.pKX]
    inc     [esp + loc46.pKY]
    add     esi, 10
    add     edi, 10
    loop    .check_whammo

.L20:
    mov     al, 10
    call    TFinish_Main

    mcEndLocals loc46.size
    ret

; --------------------------------------------------------------------------
; PLANET
; --------------------------------------------------------------------------
virtual at 0
loc45:
    .pTrekData PVOID ?
    .pIWHERE PVOID ?
    .bHasDilithium BOOL ?
    .nPlanetClass INT32 ?
    .dbl_DAMAGE_10 DOUBLE ?
    .byte_1 BYTE ?
    .byte_2 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPlanet_Main:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr

    movzx   ecx, [esi + TREKDATA.INPLAN]
    lea     edi, [esi + TREKDATA.PLNETS]
    mcZeroBits eax

.check_planet_info:
    add     al, [edi + TPlanet.planet_SCANNED]
    add     edi, TPlanet.size
    loop    .check_planet_info
    mcOnRegNotZero eax, .L102

    mov     ecx, 380
    call    TConsole_Prout

.exit_main:
    mcEndLocals loc45.size
    ret

.L102:
    mov     ecx, 381
    call    TConsole_Prout
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr

    mcLoadLocal esi, loc45.pTrekData
    movzx   ecx, [esi + TREKDATA.INPLAN]
    lea     edi, [esi + TREKDATA.PLNETS]

.planet_report:
    cmp     [edi + TPlanet.planet_SCANNED], 0
    je      .next_planet

    push    ecx edi
    mov     al, [edi + TPlanet.planet_X]
    mov     dl, [edi + TPlanet.planet_Y]
    mov     cl, 1
    call    TConsole_CramLoc

    mov     ecx, 382
    call    TConsole_Cram
    pop     edi ecx

    mov     al, 'L'
    add     al, [edi + TPlanet.planet_CLASS]
    push    ecx edi
    call    TConsole_PutChar
    pop     edi ecx

    push    ecx edi
    mov     ecx, 383
    movzx   eax, [edi + TPlanet.planet_DILITHIUM]
    add     ecx, eax
    call    TConsole_Cram
    mov     ecx, 385
    call    TConsole_Prout
    pop     edi ecx

.next_planet:
    add     edi, TPlanet.size
    loop    .planet_report
    jmp     .exit_main

; --------------------------------------------------------------------------
; ORBIT
; --------------------------------------------------------------------------
TPlanet_Orbit:
    mcBeginLocals loc45.size

    call    TConsole_ScrollUp

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0
    cmp     [esi + TREKDATA.INORBIT], 0
    je      .L2

    call    TConsole_SetGameMsgAttr
    mov     ecx, 386
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L2:
    mov     cl, DEV_IMPULSE_ENGINES
    call    TArray_IsDamaged
    jnc     .L3

    mov     cl, DEV_WARP_ENGINES
    call    TArray_IsDamaged
    jnc     .L3

    call    TConsole_SetGameMsgAttr
    mov     ecx, 387
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L3:
    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.PLNETX], 0
    je      .L5

    mov     al, [esi + TREKDATA.SECTX]
    sub     al, [esi + TREKDATA.PLNETX]
    movsx   eax, al
    call    TCommon_AbsEAX
    push    eax

    mov     al, [esi + TREKDATA.SECTY]
    sub     al, [esi + TREKDATA.PLNETY]
    movsx   eax, al
    call    TCommon_AbsEAX
    pop     edx
    add     eax, edx
    cmp     eax, 2
    jbe     .L10

.L5:
    call    TConsole_SetGameMsgAttr
    call    TConsole_CramShip
    mov     ecx, 388
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L10:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot03]
    fmulp
    fld     [glb_dbl_0dot02]
    faddp
    mcLoadLocal edi, loc45.pTrekData
    fstp    [edi + TREKDATA.TIME]

    mov     cl, DEV_WARP_ENGINES
    call    TArray_IsDamaged
    jnc     .go_ahead_sulu

    mcLoadLocal edi, loc45.pTrekData
    fld     [edi + TREKDATA.TIME]
    fld     [glb_dbl_Ten]
    fmulp
    fstp    [edi + TREKDATA.TIME]

.go_ahead_sulu:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 389
    call    TConsole_Prout
    call    TCommon_NewCondition

    mov     eax, .L17
    mcStoreLocal loc45.pIWHERE, eax

.L16:
    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.IDIDIT], 1

    call    TEvents_Main

    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 1
    je      TPlanet_Main.exit_main
    cmp     [esi + TREKDATA.JUSTIN], 1
    je      TPlanet_Main.exit_main

    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    je      TPlanet_Main.exit_main
    jmp     [esp + loc45.pIWHERE]

.L17:
    mov     ecx, 390
    call    TConsole_Cram

    call    TRandom_Ranf
    fld     [glb_dbl_7200]
    fmulp
    fld     [glb_dbl_1400]
    faddp
    fld     st
    mcLoadLocal edi, loc45.pTrekData
    fstp    [edi + TREKDATA.HEIGHT]

    mov     cl, 2
    call    TConsole_CramFloat

    mov     ecx, 391
    call    TConsole_Prout

    mcLoadLocal edi, loc45.pTrekData
    inc     [edi + TREKDATA.INORBIT]
    jmp     TPlanet_Main.exit_main

; --------------------------------------------------------------------------
; BEAM
; --------------------------------------------------------------------------
TPlanet_Beam:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    call    TConsole_ScrollUp

    mov     cl, DEV_TRANSPORTER
    call    TArray_IsDamaged
    jnc     .L19

    call    TConsole_SetGameMsgAttr
    mov     ecx, 392
    call    TConsole_Prout

    mov     cl, DEV_SHUTTLE_CRAFT
    call    TArray_GetDblDamage
    fldz
    mc_CMP_ST0_ST1
    jnz     TPlanet_Main.exit_main

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 393
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L19:
    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.INORBIT], 0
    jne     .L1910

.L1901:
    call    TConsole_SetGameMsgAttr
    call    TConsole_CramShip
    mov     ecx, 394
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L1910:
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L1920

    call    TConsole_SetGameMsgAttr
    mov     ecx, 395
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L1920:
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr
    movzx   eax, [edi + TPlanet.planet_DILITHIUM]
    mcStoreLocal loc45.bHasDilithium, eax
    cmp     [edi + TPlanet.planet_SCANNED], 1
    je      .L1940

.L1930:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 396
    call    TConsole_Prout
    mov     ecx, 397
    call    TConsole_Prout
    mov     ecx, 398
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L1940:
    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.LANDED], 1
    je      .L30

    cmp     [esp + loc45.bHasDilithium], 1
    je      .L20

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 399
    call    TConsole_Prout
    mov     ecx, 400
    call    TConsole_Prout

    mov     eax, .L20
    mcStoreLocal loc45.pIWHERE, eax

.L1950:
    mcLoad8bitsToReg32 ecx, 158
    call    TGame_JA
    jnc     TPlanet_Main.exit_main
    jmp     [esp + loc45.pIWHERE]

.L20:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 401
    call    TConsole_Prout
    call    TConsole_ScrollUp

    call    TConsole_SetGameMsgAttr
    mov     ecx, 402
    call    TConsole_Prout
    call    TConsole_ScrollUp

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 403
    call    TConsole_Prout

.L21:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 404
    call    TConsole_Prout
    invoke  Sleep, 1200

    fld     [glb_dbl_0dot98]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jnc     .L35

    mov     ecx, 405
    call    TConsole_Prout
    invoke  Sleep, 1200

    mov     ecx, 406
    call    TConsole_Prout

    mcLoadLocal esi, loc45.pTrekData
    neg     [esi + TREKDATA.LANDED]

    cmp     [esi + TREKDATA.LANDED], 1
    je      .reset_mining_flag
    cmp     [esi + TREKDATA.IMINE], 1
    jne     .reset_mining_flag

    mov     [esi + TREKDATA.ICRYSTL], 1

.reset_mining_flag:
    mov     [esi + TREKDATA.IMINE], 0
    jmp     TPlanet_Main.exit_main

.L30:
    cmp     [esi + TREKDATA.ISCRAFT], 1
    je      .L32

    call    TConsole_SetGameMsgAttr
    mov     ecx, 407
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L32:
    call    TConsole_SetGameMsgAttr
    mov     ecx, 408
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mov     ecx, 409
    call    TConsole_Prout
    mov     ecx, 410
    call    TConsole_Prout
    call    TConsole_ScrollUp

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 411
    call    TConsole_Prout
    jmp     .L21

.L35:
    call    TConsole_ScrollUp
    mov     ecx, 412
    call    TConsole_Prout

    call    TConsole_SetCrewMsgAttr
    call    TConsole_ScrollUp
    mov     ecx, 413
    call    TConsole_Prout

    mov     al, 13

.finished:
    call    TFinish_Main
    jmp     TPlanet_Main.exit_main

; --------------------------------------------------------------------------
; MINE
; --------------------------------------------------------------------------
TPlanet_Mine:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.LANDED], 1
    je      .L50

    call    TConsole_SetGameMsgAttr
    mov     ecx, 414
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L50:
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr
    movzx   eax, [edi + TPlanet.planet_CLASS]
    mcStoreLocal loc45.nPlanetClass, eax
    cmp     [edi + TPlanet.planet_DILITHIUM], 1
    je      .L51

    call    TConsole_SetGameMsgAttr
    mov     ecx, 415
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L51:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot2]
    fmulp
    fld     [glb_dbl_0dot1]
    faddp
    fild    [esp + loc45.nPlanetClass]
    fmulp
    mcLoadLocal esi, loc45.pTrekData
    fstp    [esi + TREKDATA.TIME]

    mov     eax, .L52
    mcStoreLocal loc45.pIWHERE, eax
    jmp     TPlanet_Orbit.L16

.L52:
    invoke  Sleep, 1200
    call    TConsole_SetGameMsgAttr
    mov     ecx, 416
    call    TConsole_Prout

    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.IMINE], 1
    jmp     TPlanet_Main.exit_main

; --------------------------------------------------------------------------
; CRYSTAL
; --------------------------------------------------------------------------
TPlanet_Crystal:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    call    TConsole_ScrollUp

    mcLoadLocal edi, loc45.pTrekData
    cmp     [edi + TREKDATA.ICRYSTL], 1
    je      .L55

    call    TConsole_SetGameMsgAttr
    mov     ecx, 417
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L55:
    fld     [glb_dbl_1000]
    fld     [edi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L5510

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 418
    call    TConsole_Prout
    mov     ecx, 419
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L5510:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 420
    call    TConsole_Prout
    mov     ecx, 421
    call    TConsole_Prout
    mov     ecx, 422
    call    TConsole_Prout

    mov     eax, .L56
    mcStoreLocal loc45.pIWHERE, eax
    jmp     TPlanet_Beam.L1950

.L56:
    call    TConsole_SetCrewMsgAttr
    call    TConsole_ScrollUp
    mov     ecx, 423
    call    TConsole_Prout
    mov     ecx, 424
    call    TConsole_Prout
    invoke  Sleep, 1200

    mcLoadLocal edi, loc45.pTrekData
    fld     [edi + TREKDATA.CRYPROB]
    fld     st
    faddp
    fstp    [edi + TREKDATA.CRYPROB]

    call    TConsole_ScrollUp
    mov     ecx, 425
    call    TConsole_Prout
    mov     ecx, 426
    call    TConsole_Prout
    invoke  Sleep, 1200

    call    TConsole_ScrollUp
    mov     ecx, 427
    call    TConsole_Prout
    call    TConsole_ScrollUp
    invoke  Sleep, 1200

    call    TRandom_Ranf
    mcLoadLocal esi, loc45.pTrekData
    fld     [esi + TREKDATA.CRYPROB]
    mc_CMP_ST0_ST1
    jc      .L57

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 428
    call    TConsole_Prout

.L5610:
    mov     cl, ATTR_COND_RED
    call    TConsole_SetAttr
    mov     ecx, 429
    call    TConsole_Prout

    call    TPlanet_Stars
    mov     ecx, 430
    call    TConsole_Prout

    call    TPlanet_KaBoom
    jmp     TPlanet_Main.exit_main

.L57:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot9]
    fmulp
    fld1
    faddp
    fld     [glb_dbl_5000]
    fmulp

    mcLoadLocal esi, loc45.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    faddp
    fstp    [esi + TREKDATA.ENERGY]

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 431
    call    TConsole_Prout
    mov     ecx, 432
    call    TConsole_Prout
    mov     ecx, 433
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

; --------------------------------------------------------------------------
; SENSOR
; --------------------------------------------------------------------------
TPlanet_Sensor:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    call    TConsole_ScrollUp

    mov     cl, DEV_SR_SENSORS
    call    TArray_IsDamaged
    jnc     .L60

    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L60

    call    TConsole_SetGameMsgAttr
    mov     ecx, 435
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L60:
    cmp     [esi + TREKDATA.PLNETX], 0
    jne     .L65

    call    TConsole_SetGameMsgAttr
    mov     ecx, 436
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

.L65:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 437
    call    TConsole_Cram

    mcLoadLocal esi, loc45.pTrekData
    mov     cl, 1
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TConsole_CramLoc
    mov     al, ':'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mov     ecx, 438
    call    TConsole_Cram

    mcLoadLocal esi, loc45.pTrekData
    mov     cl, 2
    mov     al, [esi + TREKDATA.PLNETX]
    mov     dl, [esi + TREKDATA.PLNETY]
    call    TConsole_CramLoc

    mov     ecx, 439
    call    TConsole_Cram

    mcLoadLocal esi, loc45.pTrekData
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr

    mov     [edi + TPlanet.planet_SCANNED], 1
    mov     al, [edi + TPlanet.planet_DILITHIUM]
    push    eax

    mov     al, [edi + TPlanet.planet_CLASS]
    add     al, 'L'
    call    TConsole_PutChar
    mov     al, '`'
    call    TConsole_PutChar
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mov     ecx, 440
    call    TConsole_Cram

    pop     eax
    mcOnRegNotZero al, .report

    mov     ecx, 441
    call    TConsole_Cram

.report:
    mov     ecx, 442
    call    TConsole_Prout
    jmp     TPlanet_Main.exit_main

; --------------------------------------------------------------------------
; GALILEO
; --------------------------------------------------------------------------
TPlanet_Galileo:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0
    call    TConsole_ScrollUp

    mov     cl, DEV_SHUTTLE_CRAFT
    call    TArray_GetDblDamage
    fld     st
    fstp    [esp + loc45.dbl_DAMAGE_10]

    fldz
    mc_CMP_ST0_ST1
    jz      .L72
    jc      .L71

    fld1
    fchs
    mc_CMP_ST0_ST1
    jz      .L70

    mov     ecx, 443
    call    TConsole_ProutGameMsg
    jmp     TPlanet_Main.exit_main

.L70:
    mov     ecx, 444
    call    TConsole_ProutGameMsg
    jmp     TPlanet_Main.exit_main

.L71:
    mov     ecx, 445
    call    TConsole_ProutGameMsg
    jmp     TPlanet_Main.exit_main

.L72:
    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.INORBIT], 1
    jne     TPlanet_Beam.L1901

    cmp     [esi + TREKDATA.SHLDUP], 1
    je      .craft_cant_thru_shields
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L80

.craft_cant_thru_shields:
    mov     ecx, 446
    call    TConsole_ProutGameMsg
    jmp     TPlanet_Main.exit_main

.L80:
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr
    cmp     [edi + TPlanet.planet_SCANNED], 1
    jne     TPlanet_Beam.L1930

    mcLoadLocal esi, loc45.pTrekData
    fld     [esi + TREKDATA.HEIGHT]
    fld     [glb_dbl_3E_neg_5]
    fmulp
    fstp    [esi + TREKDATA.TIME]

    cmp     [esi + TREKDATA.LANDED], 1
    jne     .L100
    cmp     [esi + TREKDATA.ISCRAFT], 1
    jne     .L98

    mov     cl, DEV_TRANSPORTER
    call    TArray_IsDamaged
    jc      .L95

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 447
    call    TConsole_Prout
    mov     ecx, 458
    call    TGame_JA
    jc      TPlanet_Main.exit_main

.L95:
    call    TConsole_SetGameMsgAttr
    mov     cl, DEV_TRANSPORTER
    call    TArray_IsDamaged
    mov     ecx, 448
    adc     ecx, 0
    call    TConsole_Cram

    mov     ecx, 450
    call    TConsole_Prout

    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.ISCRAFT], 0

    mov     eax, .L97
    mcStoreLocal loc45.pIWHERE, eax

.L96:
    call    TConsole_ScrollUp
    jmp     TPlanet_Orbit.L16

.L97:
    mov     ecx, 451
    call    TConsole_ProutGameMsg
    jmp     TPlanet_Main.exit_main

.L98:
    mov     ecx, 452
    call    TConsole_ProutGameMsg
    mov     ecx, 453
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 454
    call    TConsole_Prout

    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.ICRAFT], 1
    mov     [edi + TREKDATA.LANDED], -1

    mov     eax, .L99
    mcStoreLocal loc45.pIWHERE, eax
    jmp     .L96

.L99:
    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.ICRAFT], 0
    mov     [edi + TREKDATA.ISCRAFT], 1

    cmp     [edi + TREKDATA.IMINE], 0
    je      .L97

    mov     [edi + TREKDATA.ICRYSTL], 1
    mov     [edi + TREKDATA.IMINE], 0
    jmp     .L97

.L100:
    mov     ecx, 455
    call    TConsole_ProutGameMsg
    mov     ecx, 456
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 457
    call    TConsole_Prout

    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.ICRAFT], 1
    mov     [edi + TREKDATA.ISCRAFT], 0

    mov     eax, .L110
    mcStoreLocal loc45.pIWHERE, eax
    jmp     .L96

.L110:
    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.LANDED], 1
    mov     [edi + TREKDATA.ICRAFT], 0
    jmp     .L97

; --------------------------------------------------------------------------
; DEATHRA
; --------------------------------------------------------------------------
TPlanet_DeathRay:
    mcBeginLocals loc45.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc45.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0
    call    TConsole_ScrollUp

    cmp     [esi + TREKDATA.SHIP], CHAR_ENTERPRISE
    je      .L113

    mov     ecx, 459
    call    TConsole_ProutGameMsg
    jmp     .done

.L113:
    cmp     [esi + TREKDATA.NENHERE], 1
    jae     .L115

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 460
    call    TConsole_Prout
    jmp     .done

.L115:
    mov     cl, DEV_DEATHRAY
    call    TArray_IsDamaged
    jnc     .L116

    mov     ecx, 461
    call    TConsole_ProutGameMsg
    jmp     .done

.L116:
    mcLoadLocal edi, loc45.pTrekData
    mov     [edi + TREKDATA.IDIDIT], 1

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 462
    call    TConsole_Prout
    call    TConsole_ScrollUp
    invoke  Sleep, 1600

    mov     ecx, 463
    call    TConsole_Prout
    invoke  Sleep, 1000

    mov     ecx, 464
    call    TConsole_Prout
    call    TConsole_ScrollUp
    invoke  Sleep, 1200

    mov     ecx, 465
    call    TConsole_ProutGameMsg
    invoke  Sleep, 1200

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 466
    call    TConsole_Prout
    invoke  Sleep, 1200

    call    TRandom_Ranf
    fld     [glb_dbl_0dot3]
    mc_CMP_ST0_ST1
    jc      .L130

    call    TMove_RedAlert
    mov     ecx, 467
    call    TConsole_ProutGameMsg
    invoke  Sleep, 1200
    jmp     TPlanet_Crystal.L5610

.L130:
    call    TConsole_ScrollUp

    mcLoadLocal edx, loc45.pTrekData
    movzx   ecx, [edx + TREKDATA.NENHERE]

    lea     esi, [edx + TREKDATA.KX]
    lea     edi, [edx + TREKDATA.KY]

.next_enemy:
    push    edi esi ecx

    mov     al, [esi]
    mov     dl, [edi]
    call    TPhasers_DeadKlingon

    pop     ecx esi edi
    loop    .next_enemy

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 468
    call    TConsole_Prout

    mcLoadLocal esi, loc45.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    je      .game_won

    call    TConsole_ScrollUp
    mov     ecx, 469
    call    TConsole_Prout

    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    mc_CMP_ST0_ST1
    jc      .deathray_damaged

    mov     ecx, 470
    call    TConsole_Prout
    jmp     .done

.deathray_damaged:
    mov     ecx, 471
    call    TConsole_Prout

    mov     cl, DEV_DEATHRAY
    fld     [glb_dbl_39dot95]
    call    TArray_SetDblDamage
    jmp     .done

.game_won:
    mov     al, 1
    jmp     TPlanet_Beam.finished

.done:
    mcEndLocals loc45.size
    ret

; --- EOF ---
