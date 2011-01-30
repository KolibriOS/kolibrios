; --------------------------------------------------------------------------
; FILE: TChart.Asm
; DATE: October 12, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   EAX = X quadrant coordinate
;   EDX = Y quadrant coordinate
; --------------------------------------------------------------------------
align PROC_ALIGN
TChart_SetQuadAttr:
    push    ebx ecx

    mcLoadGameDataPtr ebx
    cmp     al, [ebx + TREKDATA.QUADX]
    jne     .no_ship_here
    cmp     dl, [ebx + TREKDATA.QUADY]
    je      .ship_here

.no_ship_here:
    call    TArray_IsScanned
    jc      .set_scanned_color

    mov     cl, ATTR_REPORT_VALUE
    jmp     .apply

.set_scanned_color:
    mov     cl, ATTR_PLANET
    jmp     .apply

.ship_here:
    mov     cl, ATTR_SHIP_QUAD

.apply:
    call    TConsole_SetAttr
    pop     ecx ebx
    ret

; --------------------------------------------------------------------------
; CHART
; --------------------------------------------------------------------------
virtual at 0
loc25:
    .nI INDEX ?
    .nJ INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TChart_Main:
    mcBeginLocals loc25.size

    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 136
    call    TConsole_Prout

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 137
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 138
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 139
    call    TConsole_Prout

    mcLoad1 eax
    mcStoreLocal loc25.nI, eax

.row_begins:
    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr

    mcLoadLocal eax, loc25.nI
    call    TConsole_CramInt
    mcLoad8bitsToReg32 ecx, 140
    call    TConsole_Cram

    mcLoad1 ecx
    mcStoreLocal loc25.nJ, ecx

.load_chart_value:
    mcLoadLocal eax, loc25.nI
    mcLoadLocal edx, loc25.nJ

    call    TChart_SetQuadAttr

    call    TArray_GetStarChartValue
    add     ecx, 0
    js      .L10
    jz      .L20

    cmp     ecx, 999
    ja      .L35

    call    TArray_GetGalaxyValue
    mov     eax, ecx

.put_value:
    mov     cl, 5
    call    TConsole_CramIntWidth
    jmp     .L40

.L35:
    lea     eax, [ecx - 1000]
    jmp     .put_value

.L40:
    inc     [esp + loc25.nJ]
    cmp     [esp + loc25.nJ], 8
    jbe     .load_chart_value

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 139
    call    TConsole_Prout

    inc     [esp + loc25.nI]
    cmp     [esp + loc25.nI], 8
    jbe     .row_begins

    call    TConsole_ScrollUp
    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr
    mov     cl, 2
    call    TConsole_RepeatBlank
    call    TConsole_CramShip
    mcLoad8bitsToReg32 ecx, 143
    call    TConsole_Cram
    mov     cl, 1
    mcLoadGameDataPtr esi
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TConsole_CramLoc
    jmp     .done

.L10:
    mcLoad8bitsToReg32 ecx, 141

.L101:
    call    TConsole_Cram
    jmp     .L40

.L20:
    mcLoad8bitsToReg32 ecx, 142
    jmp     .L101

.done:
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp

    mcEndLocals loc25.size
    ret

; --------------------------------------------------------------------------
; SETWARP
; --------------------------------------------------------------------------
virtual at 0
loc51:
    .pTrekData PVOID ?
    .dbl_OLDFAC DOUBLE ?
    .dbl_FNUM DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TChart_SetWarp:
    mcBeginLocals loc51.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc51.pTrekData, esi

.L10:
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L20

    mov     ecx, 585
    call    TGame_Prompt
    jmp     .L10

.L20:
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L40

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc51.dbl_FNUM]

    mov     cl, DEV_WARP_ENGINES
    call    TArray_GetDblDamage
    fld     [glb_dbl_Ten]
    mc_CMP_ST0_ST1
    jc      .L70

    mov     cl, DEV_WARP_ENGINES
    call    TArray_IsDamaged
    jnc     .validate_warp

    fld     [esp + loc51.dbl_FNUM]
    fld     [glb_dbl_4]
    mc_CMP_ST0_ST1
    jc      .L80

.validate_warp:
    fld1
    fld     [esp + loc51.dbl_FNUM]
    mc_CMP_ST0_ST1
    jc      .L50

    fld     [esp + loc51.dbl_FNUM]
    fld     [glb_dbl_Ten]
    mc_CMP_ST0_ST1
    jc      .L60

    mcLoadLocal esi, loc51.pTrekData
    fld     [esi + TREKDATA.WARPFAC]
    fstp    [esp + loc51.dbl_OLDFAC]

    fld     [esp + loc51.dbl_FNUM]
    fstp    [esi + TREKDATA.WARPFAC]

    fld     [esp + loc51.dbl_FNUM]
    fld     st
    fmulp
    fstp    [esi + TREKDATA.WFACSQ]

    fld     [esp + loc51.dbl_OLDFAC]
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L31
    jz      .L31

    fld     [glb_dbl_6]
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L31
    jz      .L31

    fld     [glb_dbl_8]
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L32
    jmp     .L33

.L31:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 586
    call    TConsole_Cram

    mcLoadLocal esi, loc51.pTrekData
    fld     [esi + TREKDATA.WARPFAC]
    mov     cl, 1
    call    TConsole_CramFloat

    mov     ecx, 587
    call    TConsole_Prout
    jmp     .done

.L32:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 588
    call    TConsole_Prout
    jmp     .done

.L33:
    mcLoadLocal esi, loc51.pTrekData
    fld     [esi + TREKDATA.WARPFAC]
    fld     [glb_dbl_Ten]
    mc_CMP_ST0_ST1
    jz      .L36

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 589
    call    TConsole_Prout
    jmp     .done

.L36:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 590
    call    TConsole_Prout
    jmp     .done

.L40:
    call    TMove_BegPardon
    jmp     .done

.L50:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 591
    call    TConsole_Prout
    jmp     .done

.L60:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 592
    call    TConsole_Prout
    jmp     .done

.L70:
    mov     ecx, 593
    call    TConsole_ProutGameMsg
    jmp     .done

.L80:
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 594
    call    TConsole_Prout
    mov     ecx, 595
    call    TConsole_Prout

.done:
    mcEndLocals loc51.size
    ret

; --- EOF ---
