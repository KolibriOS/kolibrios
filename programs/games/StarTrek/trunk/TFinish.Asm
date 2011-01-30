; --------------------------------------------------------------------------
; FILE: TFinish.Asm
; DATE: October 13, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; FINISH
; --------------------------------------------------------------------------
; Input:
;   AL = game ending code
; --------------------------------------------------------------------------
virtual at 0
loc47:
    .pTrekData PVOID ?
    .bIGotIt BOOL ?
    .nEndCode INT32 ?
    .dbl_DateDiff DOUBLE ?
    .dbl_RateMax DOUBLE ?
    .dbl_PerDate DOUBLE ?
    .dbl_Goodies DOUBLE ?
    .dbl_Baddies DOUBLE ?
    .dbl_Advantage DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TFinish_Main:
    mcBeginLocals loc47.size

    movzx   eax, al
    mcStoreLocal loc47.nEndCode, eax

    invoke  Sleep, 1000

    mcLoadGameDataPtr edi
    mcStoreLocal loc47.pTrekData, edi
    inc     [edi + TREKDATA.ALLDONE]

    mcZeroBits eax
    mcStoreLocal loc47.bIGotIt, eax

    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    mov     cl, ATTR_SHIP
    call    TConsole_SetAttr

    mov     ecx, 477
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal esi, loc47.pTrekData
    fld     [esi + TREKDATA.DATE]
    call    TConsole_CramFloat

    mov     ecx, 478
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mcLoadLocal eax, loc47.nEndCode
    mcOnRegEqu eax, 1, .L100
    mcOnRegEqu eax, 2, .L200
    mcOnRegEqu eax, 3, .L300
    mcOnRegEqu eax, 4, .L400
    mcOnRegEqu eax, 5, .L500
    mcOnRegEqu eax, 6, .L600
    mcOnRegEqu eax, 7, .L700
    mcOnRegEqu eax, 8, .L800
    mcOnRegEqu eax, 9, .L900
    mcOnRegEqu eax, 10, .L1000
    mcOnRegEqu eax, 11, .L1100
    mcOnRegEqu eax, 12, .L5000
    mcOnRegEqu eax, 13, .L1300
    mcOnRegEqu eax, 14, .L1400
    mcOnRegEqu eax, 15, .L1500
    mcOnRegEqu eax, 16, .L1600
    mcOnRegEqu eax, 17, .L1700
    mcOnRegEqu eax, 18, .L1800
    mcOnRegEqu eax, 21, .L2100
    mcOnRegEqu eax, 22, .L2200

    ;int     3
    jmp     .done

.L100:
    mcLoadLocal esi, loc47.pTrekData
    cmp     [esi + TREKDATA.NROMREM], 0
    je      .L105

    movzx   eax, [esi + TREKDATA.NROMREM]
    push    eax

    mov     ecx, 479
    call    TConsole_Cram
    pop     eax
    call    TConsole_CramInt
    mov     ecx, 480
    call    TConsole_Prout
    call    TConsole_ScrollUp

.L105:
    mov     ecx, 481
    call    TConsole_Prout
    mov     ecx, 482
    call    TConsole_Prout

    mcLoadLocal esi, loc47.pTrekData
    mov     [esi + TREKDATA.GAMEWON], 1

    cmp     [esi + TREKDATA.ALIVE], 0
    je      .L130
    cmp     [esi + TREKDATA.BASEKL], 0
    jne     .L130
    cmp     [esi + TREKDATA.SHIP], CHAR_ENTERPRISE
    jne     .L130

    movzx   eax, [esi + TREKDATA.STARKL]
    imul    eax, 5

    movzx   ecx, [esi + TREKDATA.CASUAL]
    add     eax, ecx

    movzx   ecx, [esi + TREKDATA.NPLANKL]
    imul    ecx, 10
    add     eax, ecx

    movzx   ecx, [esi + TREKDATA.NHELP]
    imul    ecx, 45
    add     eax, ecx

    cmp     eax, 100
    jae     .L130

    fld     [glb_dbl_5]
    fld     [esi + TREKDATA.DATE]
    fld     [esi + TREKDATA.INDATE]
    fsubp
    fld     st
    fstp    [esp + loc47.dbl_DateDiff]
    mc_CMP_ST0_ST1
    jc      .L110

    mov     al, [esi + TREKDATA.SKILL]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_0dot1]
    fmulp
    inc     al
    call    TCommon_LoadByteIntoFPU
    fmulp
    fld     [glb_dbl_0dot1]
    faddp
    fstp    [esp + loc47.dbl_RateMax]

    mov     al, [esi + TREKDATA.KILLK]
    add     al, [esi + TREKDATA.KILLC]
    add     al, [esi + TREKDATA.NSCKILL]
    fld     [esp + loc47.dbl_DateDiff]
    fdivp
    fstp    [esp + loc47.dbl_PerDate]

    fld     [esp + loc47.dbl_RateMax]
    fld     [esp + loc47.dbl_PerDate]
    mc_CMP_ST0_ST1
    jc      .L130

.L110:
    call    TConsole_ScrollUp
    mov     ecx, 483
    call    TConsole_Prout

    mcLoadLocal esi, loc47.pTrekData
    cmp     [esi + TREKDATA.SKILL], 4
    je      .L120
    cmp     [esi + TREKDATA.SKILL], 5
    je      .L125

    mov     ecx, 484
    call    TConsole_Cram

    mov     ecx, 484
    mcLoadLocal esi, loc47.pTrekData
    movzx   eax, [esi + TREKDATA.SKILL]
    add     ecx, eax
    call    TConsole_Prout

    mov     ecx, 488
    call    TConsole_Prout
    jmp     .L130

.L120:
    mov     ecx, 489
    call    TConsole_Prout
    inc     [esp + loc47.bIGotIt]

    call    TConsole_ScrollUp
    mov     ecx, 490
    call    TConsole_Prout
    mov     ecx, 491
    call    TConsole_Prout
    mov     ecx, 492
    call    TConsole_Prout
    jmp     .L130

.L125:
    call    TConsole_ScrollUp
    mov     ecx, 493
    call    TConsole_Prout

    call    TConsole_ScrollUp
    mov     ecx, 494
    call    TConsole_Prout

    mov     ecx, 495
    call    TConsole_Prout
    mov     ecx, 495
    call    TConsole_Prout
    mov     ecx, 495
    call    TConsole_Prout
    mov     ecx, 496
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mov     ecx, 497
    call    TConsole_Prout

    inc     [esp + loc47.bIGotIt]

.L130:
    call    TConsole_ScrollUp
    mov     ecx, 498
    call    TConsole_Prout
    call    TGame_Score

    cmp     [esp + loc47.bIGotIt], 0
    je      .done

    call    TApp_PlaqueProcessor
    jmp     .done

.L200:
    mov     ecx, 499
    call    TConsole_Prout
    mov     ecx, 500
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mov     ecx, 501
    call    TConsole_Prout
    mov     ecx, 502
    call    TConsole_Prout

    mcLoadLocal esi, loc47.pTrekData
    movzx   eax, [esi + TREKDATA.REMKL]
    movzx   edx, [esi + TREKDATA.INKLING]
    imul    eax, 3
    cmp     eax, edx
    ja      .L210

    mov     ecx, 503
    call    TConsole_Prout

    mcLoadLocal esi, loc47.pTrekData
    mov     [esi + TREKDATA.ALIVE], 0

.score_and_return:
    call    TGame_Score
    jmp     .done

.L210:
    mov     ecx, 504
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 498
    call    TConsole_Prout
    jmp     .score_and_return

.L300:
    mov     ecx, 505
    call    TConsole_Prout
    mov     ecx, 506
    call    TConsole_Prout

.L310:
    call    TConsole_ScrollUp
    mov     ecx, 507
    call    TConsole_Prout
    jmp     .L5000

.L400:
    mov     ecx, 508
    call    TConsole_Prout
    jmp     .L310

.L500:
    mov     ecx, 509
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     ecx, 510
    call    TConsole_Prout

    call    TConsole_ScrollUp
    mov     ecx, 511
    call    TConsole_Prout
    jmp     .L5000

.L600:
    mov     ecx, 512
    call    TConsole_Prout
    mov     ecx, 513
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 514
    call    TConsole_Prout
    jmp     .score_and_return

.L700:
    mov     ecx, 515
    call    TConsole_Prout

.L705:
    mov     ecx, 516
    call    TConsole_Prout
    jmp     .L5000

.L800:
    mov     ecx, 509
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     ecx, 517
    call    TConsole_Prout
    mov     ecx, 518
    call    TConsole_Prout
    jmp     .L5000

.L900:
    mov     ecx, 519
    call    TConsole_Prout
    mov     ecx, 520
    call    TConsole_Prout
    mov     ecx, 521
    call    TConsole_Prout
    mov     ecx, 522
    call    TConsole_Prout
    jmp     .L5000

.L1000:
    mov     ecx, 523
    call    TConsole_Prout
    jmp     .L5000

.L1100:
    mov     ecx, 524
    call    TConsole_Prout
    mov     ecx, 525
    call    TConsole_Prout
    jmp     .L5000

.L1300:
    mov     ecx, 526
    call    TConsole_Prout
    mov     ecx, 527
    call    TConsole_Prout
    jmp     .L1410

.L1400:
    mcLoadLocal esi, loc47.pTrekData
    movzx   ecx, [esi + TREKDATA.IPLANET]
    call    TArray_PlanetPtr
    mov     al, [edi + TPlanet.planet_CLASS]
    mcOnRegNotEqu al, 1, .L1401

    mov     ecx, 528
    call    TConsole_Prout
    mov     ecx, 529
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 530
    call    TConsole_Prout
    jmp     .L1410

.L1401:
    push    eax
    mov     ecx, 531
    call    TConsole_Prout
    mov     ecx, 532
    call    TConsole_Cram
    pop     eax
    add     al, 'L'
    call    TConsole_PutChar
    mov     ecx, 533
    call    TConsole_Prout
    mov     ecx, 534
    call    TConsole_Prout
    mov     ecx, 535
    call    TConsole_Prout

.L1410:
    call    TConsole_ScrollUp
    mov     ecx, 536
    call    TConsole_Prout
    call    TConsole_CramShip
    mov     ecx, 537
    call    TConsole_Prout
    jmp     .L5000

.L1500:
    mov     ecx, 538
    call    TConsole_Prout
    jmp     .L705

.L1600:
    mov     ecx, 539
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mov     ecx, 540
    call    TConsole_Cram
    call    TConsole_CramShip
    call    TConsole_ScrollUp

    mov     ecx, 541
    call    TConsole_Prout
    jmp     .L5000

.L1700:
    mov     ecx, 542
    call    TConsole_Prout
    jmp     .L1600

.L1800:
    mov     ecx, 543
    call    TConsole_Prout
    mov     ecx, 544
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 545
    call    TConsole_Prout
    jmp     .L1410

.L2100:
    mov     ecx, 546
    call    TConsole_Prout
    mov     ecx, 547
    call    TConsole_Prout
    jmp     .L5000

.L2200:
    mov     ecx, 548
    call    TConsole_Prout
    mov     ecx, 549
    call    TConsole_Prout
    jmp     .L705

.L5000:
    call    TConsole_ScrollUp
    mcLoadLocal esi, loc47.pTrekData
    mov     [esi + TREKDATA.ALIVE], 0

    mcZeroBits edx
    mcZeroBits eax
    mov     al, CHAR_FQUEENE
    cmp     [esi + TREKDATA.SHIP], CHAR_ENTERPRISE
    cmove   edx, eax
    mov     [esi + TREKDATA.SHIP], dl

    cmp     [esi + TREKDATA.REMKL], 0
    je      .L5050

    fld     [esi + TREKDATA.REMRES]
    fld     [esi + TREKDATA.INRESOR]
    fdivp
    fstp    [esp + loc47.dbl_Goodies]

    mov     al, [esi + TREKDATA.REMCOM]
    add     al, al
    add     al, [esi + TREKDATA.REMKL]
    call    TCommon_LoadByteIntoFPU

    mov     al, [esi + TREKDATA.INCOM]
    add     al, al
    add     al, [esi + TREKDATA.INKLING]
    call    TCommon_LoadByteIntoFPU

    fdivp
    fstp    [esp + loc47.dbl_Baddies]

    fld     [esp + loc47.dbl_Goodies]
    fld     [esp + loc47.dbl_Baddies]
    fdivp
    fstp    [esp + loc47.dbl_Advantage]

    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    fmulp
    fld1
    faddp
    fld     [esp + loc47.dbl_Advantage]
    mc_CMP_ST0_ST1
    jc      .L5020

    mov     ecx, 550
    call    TConsole_Prout
    mov     ecx, 551
    call    TConsole_Prout

    call    TRandom_Ranf
    fld     [glb_dbl_3]
    faddp
    fld     [esp + loc47.dbl_Advantage]
    mc_CMP_ST0_ST1
    jc      .L5010

    mov     ecx, 552
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     ecx, 555
    call    TConsole_Prout
    jmp     .L5030

.L5010:
    mov     ecx, 553
    call    TConsole_Prout
    jmp     .L5030

.L5020:
    mov     ecx, 554
    call    TConsole_Prout
    jmp     .L5030

.L5050:
    mov     ecx, 556
    call    TConsole_Prout
    mov     ecx, 557
    call    TConsole_Prout
    mov     ecx, 558
    call    TConsole_Prout
    mov     ecx, 559
    call    TConsole_Prout

    mcLoadLocal ebx, loc47.pTrekData
    mov     [ebx + TREKDATA.GAMEWON], 1
    mov     [ebx + TREKDATA.ALIVE], 0

.L5030:
    call    TGame_Score

.done:
    mcEndLocals loc47.size
    ret

; --- EOF ---
