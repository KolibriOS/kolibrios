; --------------------------------------------------------------------------
; FILE: TShields.Asm
; DATE: October 18, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; SHIELDS
; --------------------------------------------------------------------------
virtual at 0
loc35:
    .pTrekData PVOID ?
    .dbl_ETRANS DOUBLE ?
    .dbl_reserved DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TShields_Main:
    mcBeginLocals loc35.size
    mcLoadGameDataPtr ebx
    mcStoreLocal loc35.pTrekData, ebx

    mov     [ebx + TREKDATA.IDIDIT], 0
    call    TCmdBuf_Scan

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     TShields_Up.L30

.L15:
    mcLoad8bitsToReg32 ecx, 225
    call    TGame_JA
    jc      TShields_Up.L8010

    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jc      TShields_Up.L60

    mcLoadLocal esi, loc35.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     TShields_Up.L20
    jmp     TShields_Up.ENTRY

; --------------------------------------------------------------------------
; SHLDSUP
; --------------------------------------------------------------------------
align PROC_ALIGN
TShields_Up:
    mcBeginLocals loc35.size
    mcLoadGameDataPtr ebx
    mcStoreLocal loc35.pTrekData, ebx

.ENTRY:
    mcLoad8bitsToReg32 ecx, 226
    call    TGame_JA
    jc      .L40
    jmp     .L90

.L20:
    mcLoad8bitsToReg32 ecx, 227
    call    TGame_JA
    jc      .L50
    jmp     .L90

.L30:
    mcLoad8bitsToReg32 ecx, 244
    call    TCmdBuf_Crop
    jc      .L80

    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jc      .L60

    mcLoad8bitsToReg32 ecx, 245
    call    TCmdBuf_Crop
    jc      .L40

    mcLoad8bitsToReg32 ecx, 246
    call    TCmdBuf_Crop
    jc      .L50
    jmp     TShields_Main.L15

.L40:
    ;
    ; Raising shields
    ;
    mcLoadLocal esi, loc35.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     .L45

    mov     [esi + TREKDATA.SHLDUP], 1
    mov     [esi + TREKDATA.SHLDCHG], 1

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L401

    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_50]
    fsubp
    fstp    [esi + TREKDATA.ENERGY]

.L401:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 228
    call    TConsole_Prout

    mcLoadLocal esi, loc35.pTrekData
    fldz
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L70

.did_it:
    mcLoadLocal edi, loc35.pTrekData
    inc     [edi + TREKDATA.IDIDIT]
    jmp     .done

.L45:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 230
    call    TConsole_Prout
    jmp     .done

.L50:
    ;
    ; Lowering shields
    ;
    mcLoadLocal esi, loc35.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L55

    dec     [esi + TREKDATA.SHLDUP]
    mov     [esi + TREKDATA.SHLDCHG], 1

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 229
    call    TConsole_Prout
    jmp     .did_it

.L55:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 231
    call    TConsole_Prout
    jmp     .done

.L60:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 232
    call    TConsole_Prout
    jmp     .done

.L70:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 233
    call    TConsole_Prout
    mov     al, 4
    call    TFinish_Main
    jmp     .done

.L80:
    call    TCmdBuf_Scan
    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc35.dbl_ETRANS]

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    je      .L81

.L8010:
    mcLoad8bitsToReg32 ecx, 234
    call    TGame_Prompt
    jmp     .L80

.L81:
    fld     [esp + loc35.dbl_ETRANS]
    fldz
    mc_CMP_ST0_ST1
    jz      .L90

    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc35.dbl_ETRANS]
    mc_CMP_ST0_ST1
    jc      .L82

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 235
    call    TConsole_Prout
    jmp     .L90

.L82:
    mcLoadLocal edi, loc35.pTrekData
    inc     [edi + TREKDATA.IDIDIT]

    fld     [edi + TREKDATA.INSHLD]
    fld     [edi + TREKDATA.SHLD]
    fld     [esp + loc35.dbl_ETRANS]
    faddp
    mc_CMP_ST0_ST1
    jc      .L83
    jz      .L83

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 236
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 237
    call    TConsole_Prout

    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.SHLD]
    fld     [esi + TREKDATA.INSHLD]
    fsubp
    fld     [esi + TREKDATA.ENERGY]
    faddp
    fstp    [esi + TREKDATA.ENERGY]

    fld     [esi + TREKDATA.INSHLD]
    fstp    [esi + TREKDATA.SHLD]
    jmp     .L90

.L83:
    fld     [esp + loc35.dbl_ETRANS]
    fldz
    mc_CMP_ST0_ST1
    jc      .L8310

    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.INENRG]
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc35.dbl_ETRANS]
    fsubp
    mc_CMP_ST0_ST1
    jc      .L8310
    jz      .L8310

    fld     [esi + TREKDATA.INENRG]
    fld     [esi + TREKDATA.ENERGY]
    fld     [esi + TREKDATA.SHLD]
    faddp
    mc_CMP_ST0_ST1
    jc      .L8310
    jz      .L8310

    call    TConsole_SetCrewMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 148
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 238
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 239
    call    TConsole_Prout

    mcLoadLocal edi, loc35.pTrekData
    dec     [edi + TREKDATA.IDIDIT]
    jmp     .L90

.L8310:
    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.SHLD]
    fld     [esp + loc35.dbl_ETRANS]
    faddp
    fldz
    mc_CMP_ST0_ST1
    jc      .L84
    jz      .L84

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 240
    call    TConsole_Prout

    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esi + TREKDATA.SHLD]
    faddp
    fstp    [esi + TREKDATA.ENERGY]

    fldz
    fstp    [esi + TREKDATA.SHLD]
    jmp     .L90

.L84:
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 242
    mcLoad8bitsToReg32 edx, 243
    fld     [esp + loc35.dbl_ETRANS]
    fldz
    mc_CMP_ST0_ST1
    cmovnc  ecx, edx
    call    TConsole_Prout

    mcLoadLocal ebx, loc35.pTrekData
    fld     [ebx + TREKDATA.SHLD]
    fld     [esp + loc35.dbl_ETRANS]
    faddp
    fstp    [ebx + TREKDATA.SHLD]

    fld     [ebx + TREKDATA.ENERGY]
    fld     [esp + loc35.dbl_ETRANS]
    fsubp
    fstp    [ebx + TREKDATA.ENERGY]

.L90:
    mcLoadLocal esi, loc35.pTrekData
    fld     [esi + TREKDATA.SHLD]
    fldz
    mc_CMP_ST0_ST1
    jc      .done

    dec     [esi + TREKDATA.SHLDUP]

.done:
    mcEndLocals loc35.size
    ret

; --- EOF ---
