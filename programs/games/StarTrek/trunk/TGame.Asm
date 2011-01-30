; --------------------------------------------------------------------------
; FILE: TGame.Asm
; DATE: September 28, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; WAIT
; --------------------------------------------------------------------------
virtual at 0
loc48:
    .pTrekData PVOID ?
    .dbl_TEMP DOUBLE ?
    .dbl_ANUM DOUBLE ?
    .dbl_AITEM2 DOUBLE ?
    .dbl_ DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Wait:
    mcBeginLocals loc48.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc48.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0

.L10:
    call    TCmdBuf_Scan

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc48.dbl_ANUM]

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L20

    mov     ecx, 560
    call    TGame_Prompt
    jmp     .L10

.L20:
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L60

    fldz
    fld     [esp + loc48.dbl_ANUM]
    mc_CMP_ST0_ST1
    jc      .done
    jz      .done

    mcLoadLocal esi, loc48.pTrekData
    fld     [esi + TREKDATA.REMTIME]
    fld     [esp + loc48.dbl_ANUM]
    mc_CMP_ST0_ST1
    jnc     .verify

    cmp     [esi + TREKDATA.NENHERE], 0
    je      .L30

.verify:
    mcLoad8bitsToReg32 ecx, 158
    call    TGame_JA
    jnc     .done

.L30:
    mcLoadLocal esi, loc48.pTrekData
    inc     [esi + TREKDATA.RESTING]

    fld     [esp + loc48.dbl_ANUM]
    fstp    [esp + loc48.dbl_AITEM2]

.L40:
    fldz
    fld     [esp + loc48.dbl_ANUM]
    mc_CMP_ST0_ST1
    jc      .stop_rest
    jz      .stop_rest

.still_resting:
    mcLoadLocal esi, loc48.pTrekData
    cmp     [esi + TREKDATA.RESTING], 0
    je      .L50

    fld     [esp + loc48.dbl_ANUM]
    fstp    [esp + loc48.dbl_TEMP]

    cmp     [esi + TREKDATA.NENHERE], 0
    je      .set_time

    call    TRandom_Ranf
    fld1
    faddp
    fld     [esp + loc48.dbl_ANUM]
    call    TCommon_FPU_Min
    fstp    [esp + loc48.dbl_TEMP]

.set_time:
    fld     [esp + loc48.dbl_TEMP]
    mcLoadLocal edi, loc48.pTrekData
    fstp    [edi + TREKDATA.TIME]

    fld     [esp + loc48.dbl_ANUM]
    fld     [edi + TREKDATA.TIME]
    mc_CMP_ST0_ST1
    jnc     .check_tholian

    call    TAttack_Main

.check_tholian:
    mcLoadLocal esi, loc48.pTrekData
    cmp     [esi + TREKDATA.NENHERE], 0
    jne     .call_events

    call    TEvents_MoveTholian

.call_events:
    mcLoadLocal esi, loc48.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    call    TEvents_Main

    mcLoadLocal edi, loc48.pTrekData
    mov     [edi + TREKDATA.IDIDIT], 1

    cmp     [edi + TREKDATA.ALLDONE], 0
    jne     .done

    fld     [esp + loc48.dbl_ANUM]
    fld     [esp + loc48.dbl_TEMP]
    fsubp
    fstp    [esp + loc48.dbl_ANUM]

    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    jne     .L40

    mcLoadLocal esi, loc48.pTrekData
    mov     [esi + TREKDATA.RESTING], 0

    fldz
    fstp    [esi + TREKDATA.TIME]
    jmp     .L55

.stop_rest:
    mcLoadLocal esi, loc48.pTrekData
    mov     [esi + TREKDATA.RESTING], 0

.L50:
    call    TConsole_SetGameMsgAttr
    mcLoadLocal esi, loc48.pTrekData
    fld     [esi + TREKDATA.REMTIME]
    mov     cl, 2
    call    TConsole_CramFloat
    mov     ecx, 561
    call    TConsole_Prout

.L55:
    mcLoadLocal esi, loc48.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .done
    ;
    ; Repair Death Ray!
    ;
    fld     [glb_dbl_9dot99]
    fld     [esp + loc48.dbl_AITEM2]
    fld     [esp + loc48.dbl_ANUM]
    fsubp
    mc_CMP_ST0_ST1
    jc      .done

    fldz
    mov     cl, DEV_DEATHRAY
    call    TArray_SetDblDamage
    jmp     .done

.L60:
    call    TMove_BegPardon

.done:
    mcEndLocals loc48.size
    ret

; --------------------------------------------------------------------------
; GAME
; --------------------------------------------------------------------------
virtual at 0
loc42:
    .pTrekData PVOID ?
    .nKILLTOT COUNT ?
    .nDESTBAS COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Report:
    mcBeginLocals loc42.size

    mcLoadGameDataPtr eax
    mcStoreLocal loc42.pTrekData, eax

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr

    mov     ecx, 309
    call    TConsole_Cram

    mcLoadLocal esi, loc42.pTrekData
    mov     al, [esi + TREKDATA.LENGTH]
    mcOnRegEqu al, 1, .short_game
    mcOnRegEqu al, 2, .medium_game
    mcOnRegEqu al, 4, .long_game

.assertion_failed:
    ;int     3
    jmp     .done

.short_game:
    push    74
    jmp     .print_length

.medium_game:
    push    75
    jmp     .print_length

.long_game:
    push    76

.print_length:
    pop     ecx
    call    TConsole_Cram

    mov     al, CHAR_BLANK
    call    TConsole_PutChar

    mcLoadLocal esi, loc42.pTrekData
    movzx   eax, [esi + TREKDATA.SKILL]
    cmp     al, 1
    jb      .assertion_failed
    cmp     al, 5
    ja      .assertion_failed

    lea     ecx, [eax + 12]
    call    TConsole_Cram

    mov     ecx, 310
    call    TConsole_Prout

    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.IDIDIT], 0
    je      .L110

    mov     ecx, 311
    call    TConsole_Cram

    mcLoadLocal esi, loc42.pTrekData
    add     esi, TREKDATA.PASSWD
    mcZeroBits eax

.get_char:
    lodsb
    mcOnRegZero eax, .end_of_password

    call    TConsole_PutChar
    jmp     .get_char

.end_of_password:
    mov     ecx, 312
    call    TConsole_Prout

.L110:
    mcLoadLocal esi, loc42.pTrekData
    mov     al, [esi + TREKDATA.KILLK]
    add     al, [esi + TREKDATA.KILLC]
    add     al, [esi + TREKDATA.NSCKILL]

    movzx   eax, al
    mcStoreLocal loc42.nKILLTOT, eax
    call    TConsole_CramInt

    mov     ecx, 313
    call    TConsole_Cram

    mcLoad8bitsToReg32 ecx, 204
    mcLoadLocal esi, loc42.pTrekData
    movzx   eax, [esi + TREKDATA.INKLING]
    call    TConsole_CramSinglePlural

    mov     ecx, 314
    mcZeroBits eax
    cmp     [esp + loc42.nKILLTOT], 1
    sete    al
    add     ecx, eax
    call    TConsole_Cram

    mcLoad8bitsToReg32 ecx, 205
    mcLoadLocal esi, loc42.pTrekData
    movzx   eax, [esi + TREKDATA.KILLC]
    call    TConsole_CramSinglePlural

    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.SKILL], 2
    jbe     .L200

    mov     ecx, 316
    call    TConsole_Cram

    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.NSCREM], 1
    jne     .scom_news

    mov     ecx, 317
    call    TConsole_Cram

.scom_news:
    mov     ecx, 318
    call    TConsole_Prout

.L200:
    mcLoadLocal esi, loc42.pTrekData
    mov     al, [esi + TREKDATA.INBASE]
    sub     al, [esi + TREKDATA.REMBASE]
    movzx   eax, al
    mcStoreLocal loc42.nDESTBAS, eax
    mcOnRegZero eax, .L210

    mov     ecx, 319
    call    TConsole_CramSinglePlural
    mov     ecx, 320
    call    TConsole_Cram

.L210:
    mcLoadLocal esi, loc42.pTrekData
    mov     ecx, 319
    movzx   eax, [esi + TREKDATA.REMBASE]
    call    TConsole_CramSinglePlural

    cmp     [esp + loc42.nDESTBAS], 0
    je      .end_of_bases

    mov     ecx, 321
    call    TConsole_Cram

.end_of_bases:
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.ICSOS], 0
    je      .L240

    fld     [esi + TREKDATA.FUTURE5]
    fld     [glb_dbl_1E38]
    mc_CMP_ST0_ST1
    jc      .L240
    jz      .L240

    cmp     [esi + TREKDATA.REMCOM], 0
    je      .L240
    cmp     [esi + TREKDATA.REMBASE], 0
    je      .L240
    cmp     [esi + TREKDATA.BATX], 0
    je      .L240
    cmp     [esi + TREKDATA.BATY], 0
    je      .L240

    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    call    TArray_GetGalaxyValue
    mcLoad8bitsToReg32 eax, 100
    xchg    eax, ecx
    mcZeroBits edx
    div     ecx
    cmp     edx, 10
    jb      .L240

    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    movzx   ecx, [esi + TREKDATA.REMCOM]
    lea     edi, [esi + TREKDATA.CY]
    add     esi, TREKDATA.CX

.find_commander:
    cmp     al, [esi]
    jne     .next_commander
    cmp     dl, [edi]
    je      .L221

.next_commander:
    inc     esi
    inc     edi
    loop    .find_commander
    jmp     .L240

.L221:
    mov     ecx, 322
    call    TConsole_Cram

    mcLoadLocal esi, loc42.pTrekData
    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    mov     cl, 1
    call    TConsole_CramLoc

    mov     ecx, 323
    call    TConsole_Prout

    mov     ecx, 324
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal esi, loc42.pTrekData
    fld     [esi + TREKDATA.FUTURE5]
    call    TConsole_CramFloat

    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

.L240:
    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.ISSOS], 0
    je      .L245
    cmp     [esi + TREKDATA.ISATB], 1
    jne     .L245

    mov     ecx, 322
    call    TConsole_Cram

    mcLoadLocal esi, loc42.pTrekData
    mov     al, [esi + TREKDATA.ISX]
    mov     dl, [esi + TREKDATA.ISY]
    mov     cl, 1
    call    TConsole_CramLoc

    mov     ecx, 325
    call    TConsole_Prout

    mov     ecx, 324
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal esi, loc42.pTrekData
    fld     [esi + TREKDATA.FUTURE7]
    call    TConsole_CramFloat

    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

.L245:
    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.ISUBDAM], 0
    je      .L249

    mov     ecx, 328
    call    TConsole_Cram

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_GetDblDamage
    fldz
    mcZeroBits eax
    mc_CMP_ST0_ST1
    setz    al
    lea     ecx, [eax + 326]
    call    TConsole_Cram

    mov     ecx, 329
    call    TConsole_Prout
    mov     ecx, 330
    call    TConsole_Prout

.L249:
    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.CASUAL], 0
    je      .L250

    movzx   eax, [esi + TREKDATA.CASUAL]
    call    TConsole_CramInt

    mov     ecx, 331
    call    TConsole_Prout

.L250:
    mcLoadLocal esi, loc42.pTrekData
    cmp     [esi + TREKDATA.NHELP], 0
    je      .L260

    movzx   eax, [esi + TREKDATA.NHELP]
    mov     ecx, 332
    call    TConsole_CramSinglePlural

    mov     ecx, 333
    call    TConsole_Cram
    call    TConsole_ScrollUp

.L260:
    call    TConsole_ScrollUp

.done:
    mcEndLocals loc42.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = value in range [1..8]
; Output:
;   CF=TRUE if value is in range
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_IsValidQuadrant:
    cmp     eax, 1
    jb      .failed
    cmp     eax, 8
    ja      .failed

    stc
    ret

.failed:
    clc
    ret

; --------------------------------------------------------------------------
; GETFN
; --------------------------------------------------------------------------
; Input:
;   ESI = PCHAR - buffer to store the name into
; --------------------------------------------------------------------------
virtual at 0
loc88:
    .pszBuffer PCHAR ?
    .nAskThreeTimes COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_GetFileName:
    mcBeginLocals loc88.size
    mcStoreLocal loc88.pszBuffer, esi

    mcLoad8bitsToReg32 eax, 3
    mcStoreLocal loc88.nAskThreeTimes, eax

.L40:
    call    TCmdBuf_Scan
    mov     al, [ebx + TCmdBuf.cmdbuf_KEY]
    mcOnRegEqu al, CMD_TOKEN_EOL, .L75
    mcOnRegNotEqu al, CMD_TOKEN_ALPHA, .L100

    call    TCmdBuf_AnsiStrLen
    cmp     ecx, 27
    ja      .too_long

    mcLoadLocal edi, loc88.pszBuffer
    call    TString_AnsiCopy

    dec     edi
    mov     esi, str_TRK
    call    TString_AnsiCopy
    jmp     .done

.too_long:
    mov     ecx, 668
    call    TConsole_ProutGameMsg
    jmp     .ask

.L75:
    dec     [esp + loc88.nAskThreeTimes]
    jz      .L100

.ask:
    mov     ecx, 665
    call    TGame_Prompt
    jmp     .L40

.L100:
    mcLoadLocal esi, loc88.pszBuffer
    mov     byte [esi], 0

.done:
    mcEndLocals loc88.size
    ret

; --------------------------------------------------------------------------
; SCORE
; --------------------------------------------------------------------------
virtual at 0
loc57:
    .pTrekData PVOID ?
    .dbl_TIMUSED DOUBLE ?
    .dbl_PERDATE DOUBLE ?
    .dbl_ITHPERD DOUBLE ?
    .dbl_pad_1 DOUBLE ?
    .nISCORE INT32 ?
    .nIWON INT32 ?
    .nIDIED INT32 ?
    .nKLSHIP INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Score:
    mcBeginLocals loc57.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc57.pTrekData, esi

    fld     [esi + TREKDATA.DATE]
    fld     [esi + TREKDATA.INDATE]
    fsubp
    fstp    [esp + loc57.dbl_TIMUSED]

    fld     [esp + loc57.dbl_TIMUSED]
    fldz
    mc_CMP_ST0_ST1
    jz      .recalc_time_used

    cmp     [esi + TREKDATA.REMKL], 0
    je      .calc_per_date

.recalc_time_used:
    fld     [esp + loc57.dbl_TIMUSED]
    fld     [glb_dbl_5]
    call    TCommon_FPU_Max
    fstp    [esp + loc57.dbl_TIMUSED]

.calc_per_date:
    mov     al, [esi + TREKDATA.KILLC]
    add     al, [esi + TREKDATA.KILLK]
    add     al, [esi + TREKDATA.NSCKILL]
    call    TCommon_LoadByteIntoFPU
    fld     [esp + loc57.dbl_TIMUSED]
    fdivp
    fld     st
    fstp    [esp + loc57.dbl_PERDATE]
    fstp    [glb_dbl_KlingonsPerDate]

    fld     [esp + loc57.dbl_PERDATE]
    fld     [glb_dbl_500]
    fmulp
    fld     [glb_dbl_0dot5]
    faddp
    fstp    [esp + loc57.dbl_ITHPERD]

    mcZeroBits eax
    mcStoreLocal loc57.nIWON, eax

    cmp     [esi + TREKDATA.GAMEWON], 0
    je      .check_ship

    movzx   eax, [esi + TREKDATA.SKILL]
    imul    eax, 100
    mcStoreLocal loc57.nIWON, eax

.check_ship:
    mov     al, [esi + TREKDATA.SHIP]
    mcOnRegEqu al, CHAR_ENTERPRISE, .killed_zero_ships
    mcOnRegEqu al, CHAR_FQUEENE, .killed_one_ship
    mcOnRegZero al, .killed_two_ships

    ;int     3
    jmp     .done

.killed_zero_ships:
    push    0
    jmp     .store_killed_ships

.killed_one_ship:
    push    1
    jmp     .store_killed_ships

.killed_two_ships:
    push    2

.store_killed_ships:
    pop     edx
    mcStoreLocal loc57.nKLSHIP, edx

    mcZeroBits eax
    cmp     [esi + TREKDATA.ALIVE], 0
    jne     .I_died

    add     eax, 200

.I_died:
    mcStoreLocal loc57.nIDIED, eax

    cmp     [esi + TREKDATA.GAMEWON], 0
    jne     .calc_total_score

    mov     [esi + TREKDATA.NROMREM], 0

.calc_total_score:
    movzx   eax, [esi + TREKDATA.KILLK]
    imul    eax, 10
    mcStoreLocal loc57.nISCORE, eax

    movzx   eax, [esi + TREKDATA.KILLC]
    imul    eax, 50
    add     [esp + loc57.nISCORE], eax

    fld     [esp + loc57.dbl_ITHPERD]
    call    TCommon_FP_Truncate
    add     [esp + loc57.nISCORE], eax

    mcLoadLocal eax, loc57.nIWON
    add     [esp + loc57.nISCORE], eax

    mcLoadLocal eax, loc57.nIDIED
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.BASEKL]
    imul    eax, 100
    sub     [esp + loc57.nISCORE], eax

    mcLoadLocal eax, loc57.nKLSHIP
    imul    eax, 100
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.NHELP]
    imul    eax, 45
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.STARKL]
    imul    eax, 5
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.CASUAL]
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.NROMKL]
    imul    eax, 20
    add     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.NSCKILL]
    imul    eax, 200
    add     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.NPLANKL]
    imul    eax, 10
    sub     [esp + loc57.nISCORE], eax

    movzx   eax, [esi + TREKDATA.NROMREM]
    add     [esp + loc57.nISCORE], eax
    ;
    ; Report all that...
    ;
    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr
    call    TConsole_ScrollUp
    mov     ecx, 632
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.NROMKL], 0
    je      .L1

    movzx   eax, [esi + TREKDATA.NROMKL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 633
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.NROMKL]
    imul    eax, 20
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L1:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.NROMREM], 0
    je      .L5

    movzx   eax, [esi + TREKDATA.NROMREM]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 634
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.NROMREM]
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L5:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.KILLK], 0
    je      .L10

    movzx   eax, [esi + TREKDATA.KILLK]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 635
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.KILLK]
    imul    eax, 10
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L10:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.KILLC], 0
    je      .L12

    movzx   eax, [esi + TREKDATA.KILLC]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 636
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.KILLC]
    imul    eax, 50
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L12:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.NSCKILL], 0
    je      .L15

    movzx   eax, [esi + TREKDATA.NSCKILL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 637
    call    TConsole_Prout

.L15:
    fld     [esp + loc57.dbl_ITHPERD]
    fldz
    mc_CMP_ST0_ST1
    jz      .L20

    fld     [esp + loc57.dbl_PERDATE]
    mov     cl, 2
    mov     dl, 6
    call    TConsole_CramFloatWidth

    mov     ecx, 638
    call    TConsole_Cram

    fld     [esp + loc57.dbl_ITHPERD]
    call    TCommon_FP_Truncate
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L20:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.STARKL], 0
    je      .L30

    movzx   eax, [esi + TREKDATA.STARKL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 639
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.STARKL]
    imul    eax, 5
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L30:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.NPLANKL], 0
    je      .L32

    movzx   eax, [esi + TREKDATA.NPLANKL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 640
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.NPLANKL]
    imul    eax, 10
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L32:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.BASEKL], 0
    je      .L35

    movzx   eax, [esi + TREKDATA.BASEKL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 641
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.BASEKL]
    imul    eax, 100
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L35:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.NHELP], 0
    je      .L40

    movzx   eax, [esi + TREKDATA.NHELP]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 642
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.NHELP]
    imul    eax, 45
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L40:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.CASUAL], 0
    je      .L45

    movzx   eax, [esi + TREKDATA.CASUAL]
    mov     cl, 6
    call    TConsole_CramIntWidth

    mov     ecx, 643
    call    TConsole_Cram

    mcLoadLocal esi, loc57.pTrekData
    movzx   eax, [esi + TREKDATA.CASUAL]
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L45:
    cmp     [esp + loc57.nKLSHIP], 0
    je      .L50

    mov     cl, 6
    mcLoadLocal eax, loc57.nKLSHIP
    call    TConsole_CramIntWidth

    mov     ecx, 644
    call    TConsole_Cram

    mcLoadLocal eax, loc57.nKLSHIP
    imul    eax, 100
    mov     cl, 10
    neg     eax
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L50:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.ALIVE], 0
    jne     .L60

    mov     ecx, 645
    call    TConsole_Prout

.L60:
    mcLoadLocal esi, loc57.pTrekData
    cmp     [esi + TREKDATA.GAMEWON], 0
    je      .L70

    mov     ecx, 646
    call    TConsole_Cram

    movzx   ecx, [esi + TREKDATA.SKILL]
    add     ecx, 646
    call    TConsole_Cram

    mcLoadLocal eax, loc57.nIWON
    mov     cl, 25
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.L70:
    call    TConsole_ScrollUp

    mov     ecx, 652
    call    TConsole_Cram

    mcLoadLocal eax, loc57.nISCORE
    mov     [glb_Score], eax
    mov     cl, 10
    call    TConsole_CramIntWidth
    call    TConsole_ScrollUp

.done:
    mcEndLocals loc57.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = index of the message table
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Prompt:
    push    ebx

    push    ecx
    mov     cl, ATTR_PROMPT_TEXT
    call    TConsole_SetAttr
    pop     ecx

    call    TConsole_Cram
    call    TConsole_RefreshCaretLine
    lock    dec [glb_GameThreadBusy]

.check_if_cmd_complete:
    mcZeroBits eax
    lock    add [glb_GameThreadBusy], eax
    jnz     .done

    invoke  Sleep, eax
    jmp     .check_if_cmd_complete

.done:
    pop     ebx
    ret

; --------------------------------------------------------------------------
; JA
; --------------------------------------------------------------------------
; Input:
;   ECX = prompt text index
; Output:
;   CF=1 if user answered "YES"
; --------------------------------------------------------------------------
virtual at 0
loc27:
    .nMsgIndex INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_JA:
    mcBeginLocals loc27.size
    mcStoreLocal loc27.nMsgIndex, ecx

.ask:
    mcLoadLocal ecx, loc27.nMsgIndex
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .ask

    mcLoad8bitsToReg32 ecx, 159
    call    TCmdBuf_Crop
    jc      .return_yes

    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jc      .return_no

    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 160
    call    TConsole_Prout
    call    TConsole_ScrollUp
    jmp     .ask

.return_yes:
    mcEndLocals loc27.size
    stc
    ret

.return_no:
    mcEndLocals loc27.size
    clc
    ret

; --------------------------------------------------------------------------
; FREEZE
; --------------------------------------------------------------------------
; glb_bEmExit = 0 means normal freezing
; glb_bEmExit = 1 means freezing into EMSAVE.TRK
; --------------------------------------------------------------------------
virtual at 0
loc89:
    .strFileName CHARS 32
    .hFile HANDLE ?
    .dwSize COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Freeze:
    mcBeginLocals loc89.size

    mcLoadGameDataPtr edi
    mov     [edi + TREKDATA.IDIDIT], 0

    cmp     [glb_bEmExit], 1
    je      .emergency_exit

.ask_fname:
    mcLoadLocalRef esi, loc89.strFileName
    call    TGame_GetFileName

    cmp     byte [esp], 0
    je      .L920

    invoke  GetFileAttributes, esp
    mcOnRegEqu eax, -1, .save

    mov     cl, ATTR_PROMPT_TEXT
    call    TConsole_SetAttr

    mov     ecx, 669
    call    TConsole_Cram

    mov     esi, esp
    call    TConsole_CramAnsi

    mov     ecx, 670
    call    TGame_JA
    jnc     .ask_fname
    jmp     .save

.emergency_exit:
    mov     esi, str_EmSaveGame
    mov     edi, esp
    call    TString_AnsiCopy

.save:
    mcZeroBits eax
    mcStoreLocal loc89.dwSize, eax

    mov     esi, esp
    invoke  CreateFileAnsi, esi, GENERIC_WRITE, 0, 0,\
            CREATE_ALWAYS, FILE_FLAG_SEQ_SCAN, 0

    mcStoreLocal loc89.hFile, eax
    mcOnRegEqu eax, -1, .L920

    mov     esi, [glb_pCommon]
    mov     ecx, TCommon.size
    mcLoadLocal ebx, loc89.hFile
    mcLoadLocalRef edi, loc89.dwSize
    invoke  WriteFile, ebx, esi, ecx, edi, 0

    invoke  CloseHandle, ebx

    mov     ecx, TCommon.size
    cmp     [edi], ecx
    jne     .L920

.file_stored:
    mcLoadGameDataPtr edi
    inc     [edi + TREKDATA.IDIDIT]

    mov     ecx, 666
    call    TConsole_ProutGameMsg
    jmp     .done

.L920:
    mov     ecx, 667
    call    TConsole_ProutGameMsg

.done:
    mcEndLocals loc89.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Prelim:
    mcLoad8bitsToReg32 ecx, 92
    call    TConsole_ProutGameMsg
    dec     ecx
    call    TConsole_Prout

    mcLoad1 ecx
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 91
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    dec     ecx
    call    TConsole_Prout

    mov     ecx, 654
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 91
    call    TConsole_Prout

    mov     ecx, 657
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 91
    call    TConsole_Prout

    mov     ecx, 659
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 91
    call    TConsole_Prout
    call    TConsole_Prout

    mov     ecx, 663
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 92
    call    TConsole_Prout

    mov     cl, 5
    call    TConsole_Skip
    ret

; --------------------------------------------------------------------------
; ESI = points to file name
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_CutTheExtension:
    mov     edi, esi
    mov     al, '.'
    mcLoadNeg1 ecx
    repne   scasb
    dec     edi
    mov     byte [edi], 0
    ret

; --------------------------------------------------------------------------
virtual at 0
loc101:
    .fileInfo rb WIN32_FIND_DATA.size
    .hFind HANDLE ?
    .nFilesPerLine COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_ListFrozenGames:
    mcBeginLocals loc101.size

    invoke  FindFirstFile, str_FileMask, esp
    mcStoreLocal loc101.hFind, eax
    mcOnRegEqu eax, -1, .done

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    call    TConsole_ScrollUp
    mov     ecx, 709
    call    TConsole_Prout
    call    TConsole_ScrollUp
    mov     [esp + loc101.nFilesPerLine], 4

.list_that_file:
    mov     eax, [esp + WIN32_FIND_DATA.dwAttr]
    test    eax, FILE_ATTR_DIR
    jnz     .next

    mov     cl, 2
    call    TConsole_RepeatBlank

    lea     esi, [esp + WIN32_FIND_DATA.strName]
    call    TGame_CutTheExtension
    call    TConsole_CramWide

    dec     [esp + loc101.nFilesPerLine]
    jnz     .next

    mov     [esp + loc101.nFilesPerLine], 4
    call    TConsole_ScrollUp

.next:
    mcLoadLocal edx, loc101.hFind
    invoke  FindNextFile, edx, esp
    mcOnRegNotZero eax, .list_that_file

    call    TConsole_ScrollUp

    cmp     [esp + loc101.nFilesPerLine], 4
    je      .done

    call    TConsole_ScrollUp

.done:
    mcEndLocals loc101.size
    ret

; --------------------------------------------------------------------------
; THAW
; --------------------------------------------------------------------------
virtual at 0
loc87:
    .fileName CHARS 32
    .hFile HANDLE ?
    .dwSize COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Thaw:
    mcBeginLocals loc87.size

    mcLoadGameDataPtr edi
    mov     [edi + TREKDATA.IDIDIT], 0

.ask_fname:
    mov     esi, esp
    call    TGame_GetFileName

    cmp     byte [esp], 0
    je      .L800

    invoke  GetFileAttributes, esp
    mcOnRegEqu eax, -1, .no_such_file

    mov     esi, esp
    invoke  CreateFileAnsi, esi, GENERIC_READ, 0, 0,\
            OPEN_EXISTING, FILE_FLAG_SEQ_SCAN, 0

    mcStoreLocal loc87.hFile, eax
    mcOnRegEqu eax, -1, .L800

    mov     esi, [glb_pCommon]
    mov     ecx, TCommon.size
    mcLoadLocal ebx, loc87.hFile
    mcLoadLocalRef edi, loc87.dwSize
    mov     dword [edi], 0
    invoke  ReadFile, ebx, esi, ecx, edi, 0

    invoke  CloseHandle, ebx

    mov     ecx, TCommon.size
    cmp     [edi], ecx
    jne     .L800

    mov     ecx, 671
    call    TConsole_ProutGameMsg
    jmp     .done

.no_such_file:
    mov     esi, esp
    mov     edi, str_CmdListGames
    call    TString_AnsiEqual
    jc      .list_all_files

    mov     ecx, 673
    call    TConsole_ProutGameMsg
    jmp     .ask_fname

.list_all_files:
    call    TGame_ListFrozenGames
    jmp     .ask_fname

.L800:
    ;
    ; Game was not loaded
    ;
    mcLoadGameDataPtr edi
    add     edi, TREKDATA.PASSWD
    mov     byte [edi], 0

    mov     ecx, 672
    call    TConsole_ProutGameMsg

.done:
    mcEndLocals loc87.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = token for hashing and initializing the RNG
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_InitTournament:
    mcZeroBits eax
    mcZeroBits edx

.next_char:
    mov     dl, [esi]
    inc     esi
    mcOnRegZero edx, .seed_RNG_with_EAX

    shl     eax, 5
    xor     al, dl
    jmp     .next_char

.seed_RNG_with_EAX:
    call    TRandom_Seed

    mcLoadGameDataPtr ebx
    mcLoadNeg1 eax
    mcStoreMember TREKDATA.THINGX, al
    mcStoreMember TREKDATA.THINGY, al
    ret

; --------------------------------------------------------------------------
; Output:
;   ECX = TRUE if FROZEN game is selected
; --------------------------------------------------------------------------
virtual at 0
loc10:
    .bFrozen BOOL ?
    .pszToken PCHAR ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Choose:
    mcBeginLocals loc10.size

    mcZeroBits eax
    mcLoadGameDataPtr edi
    mov     ecx, TREKDATA.size
    rep     stosb

.L5:
    mcZeroBits eax
    mcStoreLocal loc10.bFrozen, eax

    mcLoad8bitsToReg32 ecx, 3
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .L5

    mcLoad8bitsToReg32 ecx, 10
    call    TCmdBuf_Crop
    jc      .L9

    mcLoad8bitsToReg32 ecx, 11
    call    TCmdBuf_Crop
    jc      .L100

    mcLoad8bitsToReg32 ecx, 12
    call    TCmdBuf_Crop
    jc      .L200
    jmp     .L5

.L9:
    mcZeroBits eax
    mcLoadGameDataPtr ebx
    mcStoreMember TREKDATA.SKILL, al
    mcStoreMember TREKDATA.LENGTH, al

.L10:
    call    TCmdBuf_Scan
    mcStoreLocal loc10.pszToken, esi
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    jne     .L20
    ;
    ; Check for different kinds of games
    ;
    mcZeroBits eax
    mov     ebx, [glb_pGameData]
    mcStoreMember TREKDATA.KSTUF5, al

    mcLoad8bitsToReg32 ecx, 74
    call    TCmdBuf_Crop
    jnc     .check_medium

    mov     [ebx + TREKDATA.LENGTH], 1

.check_medium:
    mcLoad8bitsToReg32 ecx, 75
    call    TCmdBuf_Crop
    jnc     .check_long

    mov     [ebx + TREKDATA.LENGTH], 2

.check_long:
    mcLoad8bitsToReg32 ecx, 76
    call    TCmdBuf_Crop
    jnc     .check_novice

    mov     [ebx + TREKDATA.LENGTH], 4

.check_novice:
    mcLoad8bitsToReg32 ecx, 13
    call    TCmdBuf_Crop
    jnc     .check_fair

    mov     [ebx + TREKDATA.SKILL], 1

.check_fair:
    mcLoad8bitsToReg32 ecx, 14
    call    TCmdBuf_Crop
    jnc     .check_good

    mov     [ebx + TREKDATA.SKILL], 2

.check_good:
    mcLoad8bitsToReg32 ecx, 15
    call    TCmdBuf_Crop
    jnc     .check_expert

    mov     [ebx + TREKDATA.SKILL], 3

.check_expert:
    mcLoad8bitsToReg32 ecx, 16
    call    TCmdBuf_Crop
    jnc     .check_emeritus

    mov     [ebx + TREKDATA.SKILL], 4
    mov     [ebx + TREKDATA.KSTUF5], 1

.check_emeritus:
    mcLoad8bitsToReg32 ecx, 17
    call    TCmdBuf_Crop
    jnc     .check_both_entered

    mov     [ebx + TREKDATA.SKILL], 5
    mov     [ebx + TREKDATA.KSTUF5], 2

.check_both_entered:
    cmp     [ebx + TREKDATA.LENGTH], 0
    je      .L10
    cmp     [ebx + TREKDATA.SKILL], 0
    je      .L10
    jmp     .L30

.L20:
    mov     ebx, [glb_pGameData]
    cmp     [ebx + TREKDATA.LENGTH], 0
    jne     .L25

    mcLoad8bitsToReg32 ecx, 4
    call    TGame_Prompt
    jmp     .L10

.L25:
    cmp     [ebx + TREKDATA.SKILL], 0
    jne     .L30

    mcLoad8bitsToReg32 ecx, 5
    call    TGame_Prompt
    jmp     .L10

.L30:
    ;
    ; Read in secret password
    ;
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L40

.ask_password:
    mcLoad8bitsToReg32 ecx, 6
    call    TGame_Prompt
    jmp     .L30

.L40:
    call    TCmdBuf_AnsiStrLen
    cmp     ecx, 8
    ja      .ask_password

    mov     edi, [glb_pGameData]
    push    edi
    add     edi, TREKDATA.PASSWD
    inc     ecx
    rep     movsb
    pop     edi
    ;
    ; Use parameters to generate initial game values
    ;
    mov     al, [edi + TREKDATA.SKILL]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_0dot5]
    fmulp
    fstp    [edi + TREKDATA.DAMFAC]

    mcLoad8bitsToReg32 edx, 3
    call    TRandom_IRan
    add     eax, 2
    mov     [edi + TREKDATA.REMBASE], al

    mcLoad8bitsToReg32 edx, 6
    call    TRandom_IRan
    add     eax, 5
    mov     [edi + TREKDATA.INPLAN], al

    call    TRandom_Ranf
    fld     [glb_dbl_2]
    faddp
    mov     al, [edi + TREKDATA.SKILL]
    call    TCommon_LoadByteIntoFPU
    fmulp
    call    TCommon_FP_Truncate
    mov     [edi + TREKDATA.NROMREM], al

    movzx   eax, [edi + TREKDATA.SKILL]
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 3
    div     ecx
    mov     [edi + TREKDATA.NSCREM], al

    mov     al, [edi + TREKDATA.LENGTH]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_7]
    fmulp
    fld     st
    fstp    [edi + TREKDATA.REMTIME]
    fstp    [edi + TREKDATA.INTIME]

    call    TRandom_Ranf
    fld     st
    faddp                           ; 2*RANF(0)

    mov     al, [edi + TREKDATA.SKILL]
    call    TCommon_LoadByteIntoFPU
    fsubrp                          ; SKILL - 2*RANF(0)

    fld1
    faddp                           ; SKILL - 2*RAND(0) + 1.0

    fild    [glb_FPU_Int32]
    fmulp                           ; (SKILL - 2*RANF(0) + 1.0)*SKILL

    fld     [glb_dbl_0dot1]
    fmulp                           ; (SKILL - 2*RANF(0) + 1.0)*SKILL*0.1

    fld     [glb_dbl_0dot15]
    faddp                           ; RATE = ... + 0.15

    fld     st
    faddp
    fld     [edi + TREKDATA.INTIME]
    fmulp
    call    TCommon_FP_Truncate
    mov     [edi + TREKDATA.REMKL], al
    mov     [edi + TREKDATA.INKLING], al

    call    TCommon_LoadByteIntoFPU
    call    TRandom_Ranf
    fmulp
    fld     [glb_dbl_0dot0625]
    fmulp
    call    TCommon_FP_Truncate
    add     al, [edi + TREKDATA.SKILL]

    cmp     al, 10
    jbe     .store_commander_count

    mov     al, 10

.store_commander_count:
    mov     [edi + TREKDATA.INCOM], al
    mov     [edi + TREKDATA.REMCOM], al

    shl     al, 2
    add     al, [edi + TREKDATA.INKLING]
    call    TCommon_LoadByteIntoFPU
    fld     [edi + TREKDATA.INTIME]
    fmulp
    fld     st
    fstp    [edi + TREKDATA.REMRES]
    fstp    [edi + TREKDATA.INRESOR]

    cmp     [edi + TREKDATA.INKLING], 50
    jbe     .store_initial_base_count

    inc     [edi + TREKDATA.REMBASE]

.store_initial_base_count:
    mov     al, [edi + TREKDATA.REMBASE]
    mov     [edi + TREKDATA.INBASE], al
    jmp     .done

.L100:
    ;
    ; Process a tournament request
    ;
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .ask_tnumber

    call    TGame_InitTournament
    jmp     .L9

.ask_tnumber:
    mcLoad8bitsToReg32 ecx, 7
    call    TGame_Prompt
    jmp     .L100

.L200:
    ;
    ; Frozen game requested
    ;
    call    TGame_Thaw
    mov     esi, [glb_pGameData]
    cmp     [esi + TREKDATA.PASSWD], 0
    je      .L5
    ;
    ; Game loaded
    ;
    inc     [esp + loc10.bFrozen]
    mov     ebx, [glb_pGameData]
    mcZeroBits eax
    mov     [ebx + TREKDATA.THINGX], al
    mov     [ebx + TREKDATA.THINGY], al

    lea     esi, [ebx + TREKDATA.QUAD]
    mcLoad8bitsToReg32 ecx, 100

.erase_things:
    cmp     byte [esi], CHAR_THING
    jne     .no_thing

    mov     byte [esi], CHAR_COSMOS

.no_thing:
    inc     esi
    loop    .erase_things

    mov     [ebx + TREKDATA.IDIDIT], 1
    call    TGame_Report

.done:
    mcLoadLocal ecx, loc10.bFrozen
    mcEndLocals loc10.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_ListAllCommands:
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 8
    call    TConsole_Prout

    mcLoad8bitsToReg32 edx, 8
    mcLoad8bitsToReg32 ecx, 76

.next:
    inc     ecx
    push    edx
    call    TConsole_Prout
    pop     edx
    dec     edx
    jnz     .next

    call    TConsole_ScrollUp
    ret

; --------------------------------------------------------------------------
virtual at 0
loc99:
    .OldSectX BYTE ?
    .OldSectY BYTE ?
    .NewSectX BYTE ?
    .NewSectY BYTE ?
    .CharEnemy BYTE ?
    .__align_1 BYTE ?
    .__align_2 BYTE ?
    .__align_3 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_DbgMoveEnemy:
    mcBeginLocals loc99.size

    mov     ecx, 714
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc99.OldSectX, al

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc99.OldSectY, al

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc99.NewSectX, al

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc99.NewSectY, al

    movzx   eax, [esp + loc99.NewSectX]
    call    TPhotons_IsValidSector
    jnc     .done

    movzx   eax, [esp + loc99.NewSectY]
    call    TPhotons_IsValidSector
    jnc     .done

    mcLoadGameDataPtr ebx
    mcZeroBits ecx
    mcLoadMember cl, TREKDATA.NENHERE
    jecxz   .done

    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY

    mcLoadLocal al, loc99.OldSectX
    mcLoadLocal dl, loc99.OldSectY

.check_sector:
    cmp     al, [esi]
    jne     .next

    cmp     dl, [edi]
    je      .move_klingon

.next:
    inc     esi
    inc     edi
    loop    .check_sector
    jmp     .done

.move_klingon:
    mcLoadLocal al, loc99.NewSectX
    mcLoadLocal dl, loc99.NewSectY

    mov     [esi], al
    mov     [edi], dl

    mcLoadLocal al, loc99.OldSectX
    mcLoadLocal dl, loc99.OldSectY
    call    TArray_QuadPtr

    mov     al, [ebx]
    mov     byte [ebx], CHAR_COSMOS
    mcStoreLocal loc99.CharEnemy, al

    mcLoadLocal al, loc99.NewSectX
    mcLoadLocal dl, loc99.NewSectY
    call    TArray_QuadPtr
    mcLoadLocal al, loc99.CharEnemy
    mov     [ebx], al

    call    TCommon_SortKlingons

.done:
    mcEndLocals loc99.size
    ret

; --------------------------------------------------------------------------
virtual at 0
loc98:
    .ShipQuadX BYTE ?
    .ShipQuadY BYTE ?
    .__align_1 BYTE ?
    .__align_2 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_DbgMoveShip:
    mcBeginLocals loc98.size

    mov     ecx, 168
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    call    TGame_IsValidQuadrant
    jnc     .done
    mcStoreLocal loc98.ShipQuadX, al

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .done

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    call    TGame_IsValidQuadrant
    jnc     .done
    mcStoreLocal loc98.ShipQuadY, al

    mcLoadGameDataPtr esi
    cmp     al, [esi + TREKDATA.QUADY]
    jne     .move_ship

    mov     al, [esi + TREKDATA.QUADX]
    cmp     [esp + loc98.ShipQuadX], al
    je      .done

.move_ship:
    mcLoadLocal al, loc98.ShipQuadX
    mcLoadLocal dl, loc98.ShipQuadY

    mov     [esi + TREKDATA.QUADX], al
    mov     [esi + TREKDATA.QUADY], dl

    call    TCommon_NewQuad

.done:
    mcEndLocals loc98.size
    ret

; --------------------------------------------------------------------------
; PROGRAM STARTRK from original FORTRAN source
; --------------------------------------------------------------------------
align PROC_ALIGN
TGame_Thread:
    mcProlog
    call    TGame_Prelim

.L10:
    call    TGame_Choose
    mcOnRegNotZero ecx, .L15

    call    TCommon_Setup

.L15:
    mov     [glb_MOVED], 0

.L20:
    mov     esi, [glb_pGameData]
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .L9999

.reset_before_cmd:
    mcZeroBits eax
    mov     [esi + TREKDATA.JUSTIN], al
    mov     [esi + TREKDATA.KDIDIT], al

    fldz
    fstp    [esi + TREKDATA.TIME]

    mcLoad8bitsToReg32 ecx, 2
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .L20

    mcLoad8bitsToReg32 ecx, 18
    call    TCmdBuf_Crop
    jc      .L100

    mcLoad8bitsToReg32 ecx, 26
    call    TCmdBuf_Crop
    jc      .L200

    mcLoad8bitsToReg32 ecx, 20
    call    TCmdBuf_Crop
    jc      .L300

    mcLoad8bitsToReg32 ecx, 24
    call    TCmdBuf_Crop
    jc      .L400

    mcLoad8bitsToReg32 ecx, 19
    call    TCmdBuf_Crop
    jc      .L500

    mcLoad8bitsToReg32 ecx, 28
    call    TCmdBuf_Crop
    jc      .L600

    mcLoad8bitsToReg32 ecx, 32
    call    TCmdBuf_Crop
    jc      .L700

    mcLoad8bitsToReg32 ecx, 34
    call    TCmdBuf_Crop
    jc      .L800

    mcLoad8bitsToReg32 ecx, 30
    call    TCmdBuf_Crop
    jc      .L900

    mcLoad8bitsToReg32 ecx, 23
    call    TCmdBuf_Crop
    jc      .L1000

    mcLoad8bitsToReg32 ecx, 31
    call    TCmdBuf_Crop
    jc      .L1100

    mcLoad8bitsToReg32 ecx, 27
    call    TCmdBuf_Crop
    jc      .L1200

    mcLoad8bitsToReg32 ecx, 22
    call    TCmdBuf_Crop
    jc      .L1300

    mcLoad8bitsToReg32 ecx, 36
    call    TCmdBuf_Crop
    jc      .L1400

    mcLoad8bitsToReg32 ecx, 37
    call    TCmdBuf_Crop
    jc      .L1450

    mcLoad8bitsToReg32 ecx, 38
    call    TCmdBuf_Crop
    jc      .L1500

    mcLoad8bitsToReg32 ecx, 39
    call    TCmdBuf_Crop
    jc      .L1550

    mcLoad8bitsToReg32 ecx, 40
    call    TCmdBuf_Crop
    jc      .L1600

    mcLoad8bitsToReg32 ecx, 41
    call    TCmdBuf_Crop
    jc      .L1650

    mcLoad8bitsToReg32 ecx, 42
    call    TCmdBuf_Crop
    jc      .L1670

    mcLoad8bitsToReg32 ecx, 43
    call    TCmdBuf_Crop
    jc      .L1680

    mcLoad8bitsToReg32 ecx, 45
    call    TCmdBuf_Crop
    jc      .L1685

    mcLoad8bitsToReg32 ecx, 46
    call    TCmdBuf_Crop
    jc      .L1690
    ;
    ; Non-abbreviated commands (checking)
    ;
    mcLoad8bitsToReg32 ecx, 21
    call    TCmdBuf_Crop
    jc      .L1700

    mcLoad8bitsToReg32 ecx, 33
    call    TCmdBuf_Crop
    jc      .L9000

    mcLoad8bitsToReg32 ecx, 25
    call    TCmdBuf_Crop
    jc      .L1900

    mcLoad8bitsToReg32 ecx, 29
    call    TCmdBuf_Crop
    jc      .L2000

    mcLoad8bitsToReg32 ecx, 35
    call    TCmdBuf_Crop
    jc      .L2100

    mcLoad8bitsToReg32 ecx, 44
    call    TCmdBuf_Crop
    jc      .L2200

    mov     ecx, 708
    call    TCmdBuf_Crop
    jc      .L2300
    ;
    ; Get help, mon!
    ;
    call    TGame_ListAllCommands
    jmp     .L20

.L100:
    call    TSrScan_Main
    jmp     .L20

.L200:
    call    TLrScan_Main
    jmp     .L20

.L300:
    call    TPhasers_Main

.L305:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.IDIDIT], 0
    je      .L20

.L310:
    call    TAttack_Main

.L320:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.KDIDIT], 0
    jne     .L2500
    jmp     .L15

.L400:
    call    TPhotons_Main

.L410:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.IDIDIT], 0
    je      .L20

    mov     [glb_MOVED], 0
    jmp     .L2500

.L500:
    cmp     [glb_MOVED], 0
    je      .L510

.L505:
    mov     [glb_MOVED], 2

.L510:
    call    TMove_Warp

.L520:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.IDIDIT], 0
    jne     .L521

    cmp     [glb_MOVED], 2
    jne     .L521

    dec     [glb_MOVED]

.L521:
    cmp     [esi + TREKDATA.IDIDIT], 0
    je      .L20

    cmp     [glb_MOVED], 2
    jne     .L522

    cmp     [esi + TREKDATA.JUSTIN], 0
    jne     .L522

    call    TAttack_Main

.L522:
    mov     [glb_MOVED], 1
    jmp     .L2500

.L600:
    call    TShields_Main
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.IDIDIT], 0
    je      .L20

    call    TAttack_Main
    mcLoadGameDataPtr esi
    mov     [esi + TREKDATA.SHLDCHG], 0
    jmp     .L320

.L700:
    call    TDock_Main
    jmp     .L305

.L800:
    call    TDock_DamageReport
    jmp     .L20

.L900:
    call    TChart_Main
    jmp     .L20

.L1000:
    cmp     [glb_MOVED], 0
    jne     .L505

    call    TImpulse_Main
    jmp     .L520

.L1100:
    call    TGame_Wait
    jmp     .L410

.L1200:
    call    TChart_SetWarp
    jmp     .L20

.L1300:
    call    TSrScan_Status
    jmp     .L20

.L1400:
    call    TPlanet_Sensor

.skip_before_prompt:
    call    TConsole_ScrollUp
    jmp     .L20

.L1450:
    call    TPlanet_Orbit
    jmp     .L410

.L1500:
    call    TPlanet_Beam
    jmp     .L20

.L1550:
    call    TPlanet_Mine
    jmp     .L410

.L1600:
    call    TPlanet_Crystal
    jmp     .L20

.L1650:
    call    TPlanet_Galileo
    jmp     .L410

.L1670:
    call    TPlanet_Main
    jmp     .skip_before_prompt

.L1680:
    call    TSrScan_Request
    jmp     .L20

.L1685:
    inc     [glb_bEmExit]
    call    TGame_Freeze
    jmp     .quit_now_fast

.L1690:
    mcLoadGameDataPtr edi
    mov     [edi + TREKDATA.IDIDIT], 0

    call    TGame_Report
    jmp     .L20
    ;
    ; Non-abbreviated commands (executing)
    ;
.L1700:
    mov     cl, 21
    call    TCmdBuf_VerifyToken
    jnc     .L20

    call    TDock_Help
    jmp     .L20

.L9000:
    mov     cl, 33
    call    TCmdBuf_VerifyToken
    jnc     .L20
    jmp     .L9001

.L1900:
    mov     cl, 25
    call    TCmdBuf_VerifyToken
    jnc     .L20

    call    TDock_Abandon
    jmp     .L20

.L2000:
    mov     cl, 29
    call    TCmdBuf_VerifyToken
    jnc     .L20

    call    TApp_SelfDestruct
    jmp     .L20

.L2100:
    mov     cl, 35
    call    TCmdBuf_VerifyToken
    jnc     .L20

    call    TGame_Freeze

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.IDIDIT], 1
    je      .L9999
    jmp     .L20

.L2200:
    mov     cl, 44
    call    TCmdBuf_VerifyToken
    jnc     .L20

    call    TPlanet_DeathRay
    jmp     .L305

.L2300:
    mov     ecx, 708
    call    TCmdBuf_VerifyToken_ECX
    jnc     .L20

    call    TLrScan_Log
    jmp     .L20

.L2500:
    ;
    ; After command which may use time invoke events module
    ;
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .L9999

    fld     [esi + TREKDATA.TIME]
    fldz
    mc_CMP_ST0_ST1
    jz      .L2501

    call    TEvents_Main

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .L9999

.L2501:
    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    jne     .L2510

    mov     al, 16
    mcZeroBits ecx
    call    TNova_AutomaticOverride

    mcZeroBits eax
    mov     [glb_MOVED], al
    jmp     .L2500

.L2510:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.NENHERE], 0
    jne     .L2511

    call    TEvents_MoveTholian

.L2511:
    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.KDIDIT], 1
    je      .L15
    cmp     [esi + TREKDATA.NENHERE], 0
    je      .L15

    cmp     [glb_MOVED], 0
    je      .L310
    cmp     [esi + TREKDATA.JUSTIN], 1
    je      .L310
    jmp     .L20

.L9001:
    call    TGame_Score

.L9999:
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    call    TPlanet_Stars
    call    TConsole_ScrollUp

    mov     ecx, 474
    call    TGame_JA
    jnc      .end_game

    mov     cl, 40
    call    TConsole_Skip
    jmp     .L10

.end_game:
    call    TConsole_ScrollUp
    mov     ecx, 475
    call    TConsole_ProutGameMsg
    mov     ecx, 476
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mov     ecx, 653
    call    TGame_Prompt

.quit_now_fast:
    invoke  PostMessage, [glb_MainWnd], WM_CLOSE, 0, 0

    mcEpilog
    ret     4

; --- EOF ---
