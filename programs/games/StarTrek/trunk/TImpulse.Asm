; --------------------------------------------------------------------------
; FILE: TImpulse.Asm
; DATE: November 9, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; IMPULSE
; --------------------------------------------------------------------------
virtual at 0
loc50:
    .pTrekData PVOID ?
    .dbl_POWER DOUBLE ?
    .__padded__ BYTES 2
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TImpulse_Main:
    mcBeginLocals loc50.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc50.pTrekData, esi

    mov     [esi + TREKDATA.IDIDIT], 0

    mov     cl, DEV_IMPULSE_ENGINES
    call    TArray_IsDamaged
    jc      .L40

    fld     [glb_dbl_30]
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L5
    jz      .L5

    call    TMove_GetCourseDistance

    mcLoadLocal esi, loc50.pTrekData
    fldz
    fld     [esi + TREKDATA.DIREC]
    mc_CMP_ST0_ST1
    jc      .done

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_100]
    fmulp
    fld     [glb_dbl_20]
    faddp
    fstp    [esp + loc50.dbl_POWER]

    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc50.dbl_POWER]
    mc_CMP_ST0_ST1
    jc      .L20

.L5:
    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 576
    call    TConsole_Prout
    mov     ecx, 577
    call    TConsole_Prout

    mcLoadLocal esi, loc50.pTrekData
    fld     [glb_dbl_30]
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jnc     .L10

    mov     ecx, 578
    call    TConsole_Prout
    jmp     .done

.L10:
    mov     ecx, 579
    call    TConsole_Prout

    mcLoadLocal esi, loc50.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_20]
    fsubp
    fld     [glb_dbl_0dot01]
    fmulp
    fld     [glb_dbl_0dot05]
    fsubp

    mov     cl, 1
    call    TConsole_CramFloat

    mov     ecx, 580
    call    TConsole_Prout
    jmp     .done

.L20:
    mcLoadLocal esi, loc50.pTrekData
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_0dot095]
    fdivp
    fstp    [esi + TREKDATA.TIME]

    fld     [esi + TREKDATA.REMTIME]
    fld     [esi + TREKDATA.TIME]
    mc_CMP_ST0_ST1
    jc      .L30

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 581
    call    TConsole_Prout
    mov     ecx, 582
    call    TConsole_Prout
    mov     ecx, 583
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 158
    call    TGame_JA
    jnc     .done

.L30:
    call    TMove_Move
    mcLoadLocal esi, loc50.pTrekData
    inc     [esi + TREKDATA.IDIDIT]

    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_100]
    fmulp
    fld     [glb_dbl_20]
    faddp
    fld     [esi + TREKDATA.ENERGY]
    fsubrp
    fstp    [esi + TREKDATA.ENERGY]

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_0dot095]
    fdivp
    fstp    [esi + TREKDATA.TIME]

    fld     [esi + TREKDATA.ENERGY]
    fldz
    mc_CMP_ST0_ST1
    jc      .done

    mov     al, 4
    call    TFinish_Main
    jmp     .done

.L40:
    call    TConsole_ScrollUp
    mov     ecx, 584
    call    TConsole_ProutGameMsg

.done:
    mcEndLocals loc50.size
    ret

; --- EOF ---
