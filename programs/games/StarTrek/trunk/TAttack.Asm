; --------------------------------------------------------------------------
; FILE: TAttack.Asm
; DATE: October 18, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   EBX = address of BYTE A
;   ECX = address of BYTE B
; --------------------------------------------------------------------------
; Performs the following statement:
;   IF (2*ABS(A) < ABS (B)) A=0
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Helper1:
    push    eax edx

    movsx   eax, byte [ebx]
    call    TCommon_AbsEAX
    lea     edx, [eax + eax]

    movsx   eax, byte [ecx]
    call    TCommon_AbsEAX

    cmp     edx, eax
    jnb     .done

.apply_zero:
    mov     byte [ebx], 0

.done:
    pop     edx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   EBX = address of BYTE A
;   EAX = value of MOTION
; --------------------------------------------------------------------------
; Performs the following statement:
;   IF (A != 0) A = ISIGN (1, MOTION*A)
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Helper2:
    cmp     byte [ebx], 0
    jne     .get_sign
    ret

.get_sign:
    push    eax ecx
    movsx   ecx, byte [ebx]
    imul    ecx, eax
    mcLoad1 eax
    call    TAttack_ISign
    mov     [ebx], al
    pop     ecx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   BL = QUAD
;   AL = LOOK
; Output:
;   AL = QUAD + (LOOK+9)/10 - 1
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Helper3:
    add     al, 9
    movzx   eax, al
    mcLoad8bitsToReg32 ecx, 10
    mcZeroBits edx
    div     ecx
    add     al, bl
    dec     al
    ret

; --------------------------------------------------------------------------
; ISIGN
; --------------------------------------------------------------------------
; Input:
;   EAX = Int32 value
;   ECX = any signed value
; Output:
;   EAX = same value with a sign of value in ECX
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_ISign:
    call    TCommon_AbsEAX

    add     ecx, 0
    js      .apply_negative
    ret

.apply_negative:
    neg     eax
    ret

; --------------------------------------------------------------------------
; CASULTY
; --------------------------------------------------------------------------
; Input:
;   ST(0) = total hit incurred by the ship
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Casualties:
    call    TRandom_Ranf
    fmulp
    fld     [glb_dbl_0dot015]
    fmulp
    call    TCommon_FP_Truncate

    cmp     eax, 2
    jb      .done

    mcLoadGameDataPtr edi
    add     [edi + TREKDATA.CASUAL], al
    push    eax

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 272
    call    TConsole_Cram

    pop     eax
    call    TConsole_CramInt

    mov     ecx, 273
    call    TConsole_Prout
    call    TConsole_SetGameMsgAttr

.done:
    ret

; --------------------------------------------------------------------------
; Check in debugger if all values in KPOWER array are intact
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_DbgVerifyKPower:
    pushad

    mcLoadGameDataPtr esi
    movzx   ecx, [esi + TREKDATA.NENHERE]
    lea     ebx, [esi + TREKDATA.KPOWER]

@@:
    fld     tbyte [ebx]
    mcFpuPop
    add     ebx, 10
    loop    @r

    popad
    ret

; --------------------------------------------------------------------------
; ZAP
; --------------------------------------------------------------------------
; Input:
;   ESI = address of HIT
;   EDI = address of IHURT
;   ECX = L (1-based klingon index)
; --------------------------------------------------------------------------
virtual at 0
loc37:
    .pTrekData PVOID ?
    .pIHURT PBOOL ?
    .pHIT PDOUBLE ?
    .dbl_HITSH DOUBLE ?
    .dbl_ABSORB DOUBLE ?
    .dbl_PROPOR DOUBLE ?
    .dbl_CHGFAC DOUBLE ?
    .dbl_EXTRADM DOUBLE ?
    .dbl_PFAC DOUBLE ?
    .nCRIT COUNT ?
    .nKTR COUNT ?
    .nJ INDEX ?
    .nL INDEX ?
    .nLL INDEX ?
    .arrCDAM BYTES 8
    .nCDAM COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Zap:
    mcBeginLocals loc37.size
    mcStoreLocal loc37.pHIT, esi
    mcStoreLocal loc37.pIHURT, edi
    mcStoreLocal loc37.nL, ecx

    mcLoadGameDataPtr ebx
    mcStoreLocal loc37.pTrekData, ebx

    fld1
    fld     [ebx + TREKDATA.INSHLD]
    fdivp
    fstp    [esp + loc37.dbl_PFAC]

    fld1
    fstp    [esp + loc37.dbl_CHGFAC]

    cmp     [ebx + TREKDATA.SHLDCHG], 0
    je      .check_defenses

    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    fmulp
    fld     [glb_dbl_DOCKFAC] ; Reusing 0.25
    faddp
    fstp    [esp + loc37.dbl_CHGFAC]

.check_defenses:
    cmp     [ebx + TREKDATA.SHLDUP], 0
    jne     .reduce_hit
    cmp     [ebx + TREKDATA.SHLDCHG], 0
    je      .L10

.reduce_hit:
    fld     [ebx + TREKDATA.SHLD]
    fld     [esp + loc37.dbl_PFAC]
    fmulp
    fld     [glb_dbl_0dot1]
    call    TCommon_FPU_Max
    fld     st
    fstp    [esp + loc37.dbl_PROPOR]

    fld     [esp + loc37.dbl_CHGFAC]
    fmulp
    mcLoadLocal esi, loc37.pHIT
    fld     tbyte [esi]
    fmulp
    fld1
    faddp
    fld     st
    fstp    [esp + loc37.dbl_HITSH]

    fld     [glb_dbl_0dot8]
    fmulp
    fstp    [esp + loc37.dbl_ABSORB]

    fld     [ebx + TREKDATA.SHLD]
    fld     [esp + loc37.dbl_ABSORB]
    mc_CMP_ST0_ST1
    jc      .reduce_shields

    fld     [ebx + TREKDATA.SHLD]
    fstp    [esp + loc37.dbl_ABSORB]

.reduce_shields:
    fld     [ebx + TREKDATA.SHLD]
    fld     [esp + loc37.dbl_ABSORB]
    fsubp
    fld     st
    fstp    [ebx + TREKDATA.SHLD]

    fldz
    mc_CMP_ST0_ST1
    jc      .reduce_hit2

    mov     [ebx + TREKDATA.SHLDUP], 0

.reduce_hit2:
    mcLoadLocal edi, loc37.pHIT
    fld     tbyte [edi]
    fld     [esp + loc37.dbl_HITSH]
    fsubp
    fstp    tbyte [edi]

    fld     [esp + loc37.dbl_PROPOR]
    fld     [glb_dbl_0dot1]
    mc_CMP_ST0_ST1
    jz      .L10

    fld     [ebx + TREKDATA.ENERGY]
    fld     [glb_dbl_0dot005]
    fmulp
    fld     tbyte [edi]
    mc_CMP_ST0_ST1
    jc      .done

.L10:
    ;
    ; We've been hit!!
    ;
    mcLoadLocal edi, loc37.pIHURT
    inc     dword [edi]

    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mov     dl, 8
    mcLoadLocal esi, loc37.pHIT
    fld     tbyte [esi]

    call    TConsole_CramFloatWidth
    mcLoad8bitsToReg32 ecx, 254
    call    TConsole_Cram

    cmp     [esp + loc37.nL], 0
    je      .L15

    mcLoad8bitsToReg32 ecx, 255
    call    TConsole_Cram

    mcLoadLocal ebx, loc37.pTrekData
    mcLoadLocal ecx, loc37.nL
    dec     ecx
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    call    TConsole_CramEnemyAtEx

.L15:
    call    TConsole_ScrollUp
    invoke  Sleep, 350
    ;
    ; Decide if hit is critical
    ;
    mcLoadLocal ebx, loc37.pTrekData
    mov     eax, 275
    movzx   ecx, [ebx + TREKDATA.SKILL]
    imul    ecx, 25
    sub     eax, ecx

    push    eax
    call    TRandom_Ranf
    pop     eax

    fld     [glb_dbl_0dot5]
    fmulp
    fld1
    faddp
    call    TCommon_FPU_Load_EAX
    fmulp

    mcLoadLocal esi, loc37.pHIT
    fld     tbyte [esi]
    mc_CMP_ST0_ST1
    jc      .L60

    call    TRandom_Ranf
    fld     [glb_dbl_100]
    fmulp
    fld     [glb_dbl_500]
    faddp

    mcLoadLocal esi, loc37.pHIT
    fld     tbyte [esi]
    fdivrp
    fld1
    faddp
    call    TCommon_FP_Truncate
    mcStoreLocal loc37.nCRIT, eax

    call    TConsole_SetGameMsgAttr
    mov     ecx, 256
    call    TConsole_Cram
    ;
    ; Select devices and cause damage
    ;
    mcLoad1 eax
    mcStoreLocal loc37.nKTR, eax
    mcStoreLocal loc37.nLL, eax
    dec     eax
    mcStoreLocal loc37.nCDAM, eax

.L20:
    mcLoad8bitsToReg32 edx, 13
    call    TRandom_IRan
    inc     eax
    mov     ecx, eax
    mcStoreLocal loc37.nJ, eax
    call    TArray_GetDblDamage

    fldz
    fxch
    mc_CMP_ST0_ST1
    jc      .L20
    ;
    ; Prevent shuttle damage unless on ship
    ;
    cmp     [esp + loc37.nJ], 10
    jne     .calculate_damage

    mcLoadLocal esi, loc37.pTrekData
    cmp     [esi + TREKDATA.ISCRAFT], 1
    jne     .L20

.calculate_damage:
    mcLoadLocal ecx, loc37.nCDAM
    mcLoadLocalRef edi, loc37.arrCDAM
    inc     [esp + loc37.nCDAM]
    mcLoadLocal eax, loc37.nJ
    mov     [edi + ecx], al

    call    TRandom_Ranf
    fld     [glb_dbl_25]
    fmulp
    fld     [glb_dbl_75]
    faddp
    fild    [esp + loc37.nCRIT]
    fmulp

    mcLoadLocal esi, loc37.pTrekData
    mcLoadLocal ebx, loc37.pHIT
    fld     [esi + TREKDATA.DAMFAC]
    fld     tbyte [ebx]
    fmulp
    fdivrp
    fstp    [esp + loc37.dbl_EXTRADM]

    mcLoadLocal ecx, loc37.nJ
    call    TArray_GetDblDamage
    fld     [esp + loc37.dbl_EXTRADM]
    faddp

    mcLoadLocal ecx, loc37.nJ
    call    TArray_SetDblDamage

    cmp     [esp + loc37.nLL], 1
    je      .L40
    ;
    ; Check if device has been reported once
    ;
    mcLoadLocal ecx, loc37.nCDAM
    mcLoadLocal edx, loc37.nJ
    mcLoadLocalRef esi, loc37.arrCDAM

.check_if_reported:
    lodsb
    cmp     al, dl
    je      .L50
    loop    .check_if_reported
    ;
    ; Report the device
    ;
    inc     [esp + loc37.nKTR]
    cmp     [esp + loc37.nKTR], 3
    jne     .cram_AND

    call    TConsole_ScrollUp

.cram_AND:
    mov     ecx, 258
    call    TConsole_Cram

.L40:
    mcLoadLocal ecx, loc37.nJ
    call    TConsole_CramDevice

.L50:
    inc     [esp + loc37.nLL]
    mcLoadLocal ecx, loc37.nLL
    cmp     ecx, [esp + loc37.nCRIT]
    jbe     .L20

    mov     ecx, 259
    call    TConsole_Prout
    ;
    ; Print message if shields got knocked down
    ;
    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jnc     .L60

    mcLoadLocal esi, loc37.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L60

    dec     [esi + TREKDATA.SHLDUP]
    mov     ecx, 260
    call    TConsole_Prout

.L60:
    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jnc     .deplete

    mcLoadLocal edi, loc37.pTrekData
    mov     [edi + TREKDATA.ISUBDAM], 1

.deplete:
    mcLoadLocal edi, loc37.pTrekData
    mcLoadLocal esi, loc37.pHIT
    fld     [edi + TREKDATA.ENERGY]
    fld     tbyte [esi]
    fsubp
    fstp    [edi + TREKDATA.ENERGY]

.done:
    mcEndLocals loc37.size
    ret

; --------------------------------------------------------------------------
; MOVECOM
; --------------------------------------------------------------------------
virtual at 0
loc52:
    .pTrekData PVOID ?
    .pNOEXIT PVOID ?
    .pKDIST PDOUBLE ?
    .pKPOWER PDOUBLE ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .nLOCCOM INDEX ?
    .nI INDEX ?
    .nLL INDEX ?
    .nSTEPS COUNT ?
    .bIRUN BYTE ?
    .chIENM BYTE ?
    .nKRAWLX BYTE ?
    .nKRAWLY BYTE ?
    .nIX BYTE ?
    .nIY BYTE ?
    .nCOMX BYTE ?
    .nCOMY BYTE ?
    .nMX BYTE ?
    .nMY BYTE ?
    .nNEXTX BYTE ?
    .nNEXTY BYTE ?
    .nLOOKX BYTE ?
    .nLOOKY BYTE ?
    .nIQX BYTE ?
    .nIQY BYTE ?
    .chIQUAD BYTE ?
    .pad_1 BYTE ?
    .pad_2 BYTE ?
    .pad_3 BYTE ?
    .nMOTION INT32 ?
    .dbl_TFAC DOUBLE ?
    .dbl_EFAC DOUBLE ?
    .dbl_MDIST DOUBLE ?
    .dbl_DIST1 DOUBLE ?
    .dbl_NBADDYS DOUBLE ?
    .dbl_FORCES DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_MoveCommanders:
    mcBeginLocals loc52.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc52.pTrekData, esi

    lea     eax, [esi + TREKDATA.KPOWER]
    lea     edx, [esi + TREKDATA.KDIST]
    lea     ebx, [esi + TREKDATA.KX]
    lea     ecx, [esi + TREKDATA.KY]
    mcStoreLocal loc52.pKPOWER, eax
    mcStoreLocal loc52.pKDIST, edx
    mcStoreLocal loc52.pKX, ebx
    mcStoreLocal loc52.pKY, ecx

    mov     al, 1
    mcStoreLocal loc52.nKRAWLX, al
    mcStoreLocal loc52.nKRAWLY, al
    ;
    ; IF ((NENHERE == 0) OR (JUSTIN == 1)) RETURN
    ;
    cmp     [esi + TREKDATA.NENHERE], 0
    je      .done
    cmp     [esi + TREKDATA.JUSTIN], al
    je      .done
    ;
    ; NBADDYS = COMHERE + ISHERE
    ;
    mov     al, [esi + TREKDATA.COMHERE]
    add     al, [esi + TREKDATA.ISHERE]
    call    TCommon_LoadByteIntoFPU
    fstp    [esp + loc52.dbl_NBADDYS]
    ;
    ; IF (KSTUF5 != 0)
    ;       NBADDYS = ((2*COMHERE)+(2*ISHERE)+(KLHERE*1.23)+(IRHERE*1.5))/2
    ;
    cmp     [esi + TREKDATA.KSTUF5], 0
    je      .begin_loop

    mov     al, [esi + TREKDATA.COMHERE]
    add     al, al
    mov     cl, [esi + TREKDATA.ISHERE]
    add     cl, cl
    add     al, cl
    call    TCommon_LoadByteIntoFPU

    mov     al, [esi + TREKDATA.KLHERE]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_1dot23]
    fmulp
    faddp

    mov     al, [esi + TREKDATA.IRHERE]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_1dot5]
    fmulp
    faddp

    fld     [glb_dbl_2]
    fdivp
    fstp    [esp + loc52.dbl_NBADDYS]

.begin_loop:
    mcLoad1 eax
    mcStoreLocal loc52.nI, eax

.L1:
    mov     [esp + loc52.bIRUN], 0
    ;
    ; IX = KX (I)
    ; IY = KY (I)
    ;
    mcLoadLocal ecx, loc52.nI
    mcStoreLocal loc52.nLOCCOM, ecx
    dec     ecx
    mcLoadLocal esi, loc52.pKX
    mcLoadLocal edi, loc52.pKY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    mcStoreLocal loc52.nIX, al
    mcStoreLocal loc52.nIY, dl
    mcStoreLocal loc52.nCOMX, al
    mcStoreLocal loc52.nCOMY, dl
    ;
    ; IENM = QUAD (IX,IY)
    ;
    call    TArray_QuadPtr
    mov     al, [ebx]
    mcStoreLocal loc52.chIENM, al
    ;
    ; IF (KSTUF5==0) AND ((IENM != 'S') OR (IENM != 'C')) GOTO 500
    ;
    mcLoadLocal esi, loc52.pTrekData
    cmp     [esi + TREKDATA.KSTUF5], 0
    jne     .check_if_scom

    cmp     [esp + loc52.chIENM], CHAR_SCOM
    jne     .L500
    cmp     [esp + loc52.chIENM], CHAR_COMMANDER
    jne     .L500

.check_if_scom:
    ;
    ; IF (IENM != 'S') GOTO 3
    ;
    cmp     [esp + loc52.chIENM], CHAR_SCOM
    jne     .L3
    ;
    ; Check with spy to see if S.C. should hi-tail it
    ; IF (KPOWER (LOCCOM) > 500.0) AND (!DOCKED OR TUBES DAMAGED) GOTO 3
    ;
    mcLoadLocal ecx, loc52.nLOCCOM
    mcLoadLocal ebx, loc52.pKPOWER
    dec     ecx
    imul    ecx, 10
    fld     [glb_dbl_500]
    fld     tbyte [ebx + ecx]
    mc_CMP_ST0_ST1
    jc      .enemy_runs

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L3

    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    jc      .L3

.enemy_runs:
    inc     [esp + loc52.bIRUN]
    mcLoad8bitsToReg32 eax, -10
    mcStoreLocal loc52.nMOTION, eax
    jmp     .L8

.L3:
    ;
    ; Decide whether to advance, retreat or hold position
    ; and by how much
    ;
    mcZeroBits eax
    mcStoreLocal loc52.nMOTION, eax
    ;
    ; DIST1 = KDIST (LOCCOM)
    ;
    mcLoadLocal ecx, loc52.nLOCCOM
    mcLoadLocal ebx, loc52.pKDIST
    dec     ecx
    imul    ecx, 10
    fld     tbyte [ebx + ecx]
    fstp    [esp + loc52.dbl_DIST1]
    ;
    ; MDIST = DIST1 + 0.5
    ;
    fld     [esp + loc52.dbl_DIST1]
    fld     [glb_dbl_0dot5]
    faddp
    fstp    [esp + loc52.dbl_MDIST]
    ;
    ; FORCES = KPOWER (LOCCOM) + NENHERE*100.0 + (NBADDYS-1.0)*400.0
    ;
    mcLoadLocal ecx, loc52.nLOCCOM
    mcLoadLocal ebx, loc52.pKPOWER
    dec     ecx
    imul    ecx, 10
    fld     tbyte [ebx + ecx]

    mcLoadLocal esi, loc52.pTrekData
    mov     al, [esi + TREKDATA.NENHERE]
    call    TCommon_LoadByteIntoFPU
    fld     [glb_dbl_100]
    fmulp
    faddp

    fld     [esp + loc52.dbl_NBADDYS]
    fld1
    fsubp
    fld     [glb_dbl_400]
    fmulp
    faddp
    fstp    [esp + loc52.dbl_FORCES]
    ;
    ; IF (SHLDUP == 0) FORCES += 1000.0
    ;
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     .check_damages

    fld     [esp + loc52.dbl_FORCES]
    fld     [glb_dbl_1000]
    faddp
    fstp    [esp + loc52.dbl_FORCES]

.check_damages:
    ;
    ; IF ((DAMAGE (3) == 0) OR (DAMAGE (4) == 0)) GOTO 4
    ;
    mov     cl, DEV_PHASERS
    call    TArray_IsDamaged
    jnc     .L4

    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    jnc     .L4
    ;
    ; FORCES += 1000.0
    ;
    fld     [esp + loc52.dbl_FORCES]
    fld     [glb_dbl_1000]
    faddp
    fstp    [esp + loc52.dbl_FORCES]
    jmp     .L7

.L4:
    fld1
    fld1
    fstp    [esp + loc52.dbl_EFAC]
    fstp    [esp + loc52.dbl_TFAC]
    ;
    ; IF (DAMAGE (3) == 0) GOTO 5
    ;
    mov     cl, DEV_PHASERS
    call    TArray_IsDamaged
    jnc     .L5
    ;
    ; EFAC = 0.0
    ; FORCES += 300.0
    ;
    fldz
    fstp    [esp + loc52.dbl_EFAC]

    fld     [esp + loc52.dbl_FORCES]
    fld     [glb_dbl_300]
    faddp
    fstp    [esp + loc52.dbl_FORCES]

.L5:
    ;
    ; IF (DAMAGE (4) == 0) GOTO 6
    ;
    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    jnc     .L6
    ;
    ; TFAC = 0.0
    ; FORCES += 300.0
    ;
    fldz
    fstp    [esp + loc52.dbl_TFAC]

    fld     [esp + loc52.dbl_FORCES]
    fld     [glb_dbl_300]
    faddp
    fstp    [esp + loc52.dbl_FORCES]

.L6:
    ;
    ; FORCES = FORCES - 50.0*TORPS*TFAC + 0.2*EFAC*(2500.0-ENERGY) + 0.6*SHLDUP*(1250.0-SHLD)
    ;
    fld     [esp + loc52.dbl_FORCES]
    mcLoadLocal esi, loc52.pTrekData
    movzx   eax, [esi + TREKDATA.TORPS]
    imul    eax, 50
    call    TCommon_FPU_Load_EAX
    fld     [esp + loc52.dbl_TFAC]
    fmulp
    fsubp

    fld     [glb_dbl_2500]
    fld     [esi + TREKDATA.ENERGY]
    fsubp
    fld     [esp + loc52.dbl_EFAC]
    fmulp
    fld     [glb_dbl_0dot2]
    fmulp
    faddp

    fld     [glb_dbl_1250]
    fld     [esi + TREKDATA.SHLD]
    fsubp
    fld     [glb_dbl_0dot6]
    fmulp
    mov     al, [esi + TREKDATA.SHLDUP]
    call    TCommon_LoadByteIntoFPU
    fmulp
    faddp
    fstp    [esp + loc52.dbl_FORCES]

.L7:
    ;
    ; IF (FORCES > 1000.0) MOTION = (1.0-RANF(0)**2)*DIST1 + 1.0
    ;
    fld     [glb_dbl_1000]
    fld     [esp + loc52.dbl_FORCES]
    mc_CMP_ST0_ST1
    jc      .check_docked

    fld1
    call    TRandom_Ranf
    fld     st
    fmulp
    fsubp
    fld     [esp + loc52.dbl_DIST1]
    fmulp
    fld1
    faddp
    call    TCommon_FP_Truncate
    mcStoreLocal loc52.nMOTION, eax

.check_docked:
    ;
    ; IF (DOCKED) MOTION -= SKILL*(2.0-RANF(0)**2)
    ;
    mcLoadLocal esi, loc52.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .check_no_motion

    fld     [glb_dbl_2]
    call    TRandom_Ranf
    fld     st
    fmulp
    fsubp
    mov     al, [esi + TREKDATA.SKILL]
    call    TCommon_LoadByteIntoFPU
    fmulp
    call    TCommon_FP_Truncate
    sub     [esp + loc52.nMOTION], eax

.check_no_motion:
    ;
    ; IF (MOTION == 0) MOTION = ((FORCES + 200.0*RANF(0))/150.0) - 5.0
    ;
    cmp     [esp + loc52.nMOTION], 0
    jne     .we_have_motion

    call    TRandom_Ranf
    fld     [glb_dbl_200]
    fmulp
    fld     [esp + loc52.dbl_FORCES]
    faddp
    fld     [glb_dbl_150]
    fdivp
    call    TCommon_FP_Truncate
    sub     eax, 5
    mcStoreLocal loc52.nMOTION, eax
    mcOnRegZero eax, .L500

.we_have_motion:
    ;
    ; IF (ABS (MOTION) > SKILL) MOTION = ISIGN (SKILL, MOTION)
    ;
    mcLoadLocal eax, loc52.nMOTION
    call    TCommon_AbsEAX

    mcLoadLocal esi, loc52.pTrekData
    movzx   ecx, [esi + TREKDATA.SKILL]

    cmp     eax, ecx
    jbe     .L8

    xchg    eax, ecx
    call    TAttack_ISign
    mcStoreLocal loc52.nMOTION, eax

.L8:
    ;
    ; Calculate preferred number of steps to
    ; move a commander.
    ;
    mcLoadLocal eax, loc52.nMOTION
    call    TCommon_AbsEAX
    mcStoreLocal loc52.nSTEPS, eax
    ;
    ; IF (MOTION > 0) AND (NSTEPS > MDIST) NSTEPS=MDIST
    ;
    cmp     [esp + loc52.nMOTION], 0
    jle     .validate_step_count

    fld     [esp + loc52.dbl_MDIST]
    call    TCommon_FP_Truncate
    cmp     [esp + loc52.nSTEPS], eax
    jle     .validate_step_count

    mcStoreLocal loc52.nSTEPS, eax

.validate_step_count:
    cmp     [esp + loc52.nSTEPS], 1
    jb      .set_one_step
    cmp     [esp + loc52.nSTEPS], 10
    jbe     .compute_dx_dy

    push    10
    jmp     .apply_num_steps

.set_one_step:
    push    1

.apply_num_steps:
    pop     eax
    mcStoreLocal loc52.nSTEPS, eax

.compute_dx_dy:
    ;
    ; Compute preferred values of Delta X and Delta Y
    ; MX = SECTX - COMX
    ; MY = SECTY - COMY
    ;
    mcLoadLocal esi, loc52.pTrekData
    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    sub     al, [esp + loc52.nCOMX]
    sub     dl, [esp + loc52.nCOMY]
    mcStoreLocal loc52.nMX, al
    mcStoreLocal loc52.nMY, dl
    ;
    ; IF (2*ABS(MX) < ABS(MY)) MX=0
    ; IF (2*ABS(MY) < ABS(MX)) MY=0
    ;
    mcLoadLocalRef ebx, loc52.nMX
    mcLoadLocalRef ecx, loc52.nMY
    call    TAttack_Helper1

    xchg    ebx, ecx
    call    TAttack_Helper1
    ;
    ; IF (MX != 0) MX = ISIGN (1, MX*MOTION)
    ; IF (MY != 0) MY = ISIGN (1, MY*MOTION)
    ;
    mcLoadLocalRef ebx, loc52.nMX
    mcLoadLocal eax, loc52.nMOTION
    call    TAttack_Helper2

    mcLoadLocalRef ebx, loc52.nMY
    call    TAttack_Helper2
    ;
    ; Main loop to attempt to move COMMANDER <STEPS> steps
    ; NEXTX = COMX
    ; NEXTY = COMY
    ;
    mcLoadLocal al, loc52.nCOMX
    mcLoadLocal dl, loc52.nCOMY
    mcStoreLocal loc52.nNEXTX, al
    mcStoreLocal loc52.nNEXTY, dl
    ;
    ; QUAD (COMX,COMY) = IHDOT
    ;
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS
    ;
    ; DO 60 LL=1,NSTEPS
    ;
    mcLoad1 eax
    mcStoreLocal loc52.nLL, eax

.look_next:
    ;
    ; LOOKX = NEXTX + MX
    ; LOOKY = NEXTY + MY
    ;
    mcLoadLocal al, loc52.nNEXTX
    mcLoadLocal dl, loc52.nNEXTY
    add     al, [esp + loc52.nMX]
    add     dl, [esp + loc52.nMY]
    mcStoreLocal loc52.nLOOKX, al
    mcStoreLocal loc52.nLOOKY, dl

    mov     ecx, .L10
    mcStoreLocal loc52.pNOEXIT, ecx
    ;
    ; IF (LOOKX < 1) OR (LOOKX > 10) IF (MOTION) 70,30,30
    ;
    cmp     al, 1
    jl      .look_x_out_of_quad
    cmp     al, 10
    jg      .look_x_out_of_quad
    ;
    ; IF (LOOKY < 1) OR (LOOKY > 10) IF (MOTION) 70,10,10
    ;
    cmp     dl, 1
    jl      .look_y_out_of_quad
    cmp     dl, 10
    jg      .look_y_out_of_quad
    ;
    ; IQUAD = QUAD (LOOKX,LOOKY)
    ;
    call    TArray_QuadPtr
    mov     cl, [ebx]
    mcStoreLocal loc52.chIQUAD, cl
    ;
    ; IF (IQUAD != SHIP) GOTO 9010
    ;
    mcLoadLocal esi, loc52.pTrekData
    cmp     [esi + TREKDATA.SHIP], cl
    jne     .L9010
    ;
    ; Only let COMMANDERS ram the ship
    ;
    mcOnRegEqu cl, CHAR_COMMANDER, .whamo
    mcOnRegNotEqu cl, CHAR_SCOM, .L9010

.whamo:
    mov     bl, 1
    mcLoadLocal cl, loc52.chIENM
    mcLoadLocal al, loc52.nCOMX
    mcLoadLocal dl, loc52.nCOMY
    call    TMove_Ram
    jmp     .L500

.look_x_out_of_quad:
    cmp     [esp + loc52.nMOTION], 0
    jl      .L70
    jmp     .L30

.look_y_out_of_quad:
    cmp     [esp + loc52.nMOTION], 0
    jl      .L70
    jmp     .L10

.at_L10_out_of_quad:
    cmp     [esp + loc52.nMOTION], 0
    jl      .L70
    jmp     .L20

.L9010:
    cmp     [esp + loc52.chIQUAD], CHAR_COSMOS
    je      .L50

.L10:
    ;
    ; IF (MY == KRAWLY) OR (MX == 0) GOTO 30
    ;
    mcLoadLocal al, loc52.nMY
    cmp     al, [esp + loc52.nKRAWLY]
    je      .L30
    cmp     [esp + loc52.nMX], 0
    je      .L30
    ;
    ; LOOKY = NEXTY + KRAWLY
    ;
    mcLoadLocal al, loc52.nNEXTY
    add     al, [esp + loc52.nKRAWLY]
    mcStoreLocal loc52.nLOOKY, al

    mov     ecx, .L20
    mcStoreLocal loc52.pNOEXIT, ecx
    ;
    ; IF (LOOKY < 1) OR (LOOKY > 10) IF (MOTION) 70,20,20
    ;
    cmp     al, 1
    jl      .at_L10_out_of_quad
    cmp     al, 10
    jg      .at_L10_out_of_quad
    ;
    ; IF (LOOKX < 1) OR (LOOKX > 10) IF (MOTION) 70,20,20
    ;
    mcLoadLocal dl, loc52.nLOOKX
    cmp     dl, 1
    jl      .at_L10_out_of_quad
    cmp     dl, 10
    jg      .at_L10_out_of_quad
    ;
    ; IF (QUAD (LOOKX,LOOKY) == IHDOT) GOTO 50
    ;
    xchg    edx, eax
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    je      .L50

.L20:
    ;
    ; KRAWLY = -KRAWLY
    ;
    neg     [esp + loc52.nKRAWLY]

.L30:
    ;
    ; IF (MX == KRAWLX) OR (MY == 0) GOTO 60
    ;
    mcLoadLocal al, loc52.nMX
    cmp     al, [esp + loc52.nKRAWLX]
    je      .L60
    cmp     [esp + loc52.nMY], 0
    je      .L60
    ;
    ; LOOKX = NEXTX + KRAWLX
    ;
    mcLoadLocal al, loc52.nNEXTX
    add     al, [esp + loc52.nKRAWLX]
    mcStoreLocal loc52.nLOOKX, al

    mov     ecx, .L40
    mcStoreLocal loc52.pNOEXIT, ecx
    ;
    ; IF (LOOKX < 1) OR (LOOKX > 10) IF (MOTION) 70,40,40
    ;
    cmp     al, 1
    jl      .at_L30_out_of_quad
    cmp     al, 10
    jg      .at_L30_out_of_quad
    ;
    ; IF (LOOKY < 1) OR (LOOKY > 10) IF (MOTION) 70,40,40
    ;
    mcLoadLocal dl, loc52.nLOOKY
    cmp     dl, 1
    jl      .at_L30_out_of_quad
    cmp     dl, 10
    jg      .at_L30_out_of_quad
    ;
    ; IF (QUAD (LOOKX,LOOKY) == IHDOT) GOTO 50
    ;
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    je      .L50

.L40:
    neg     [esp + loc52.nKRAWLX]
    jmp     .L60

.at_L30_out_of_quad:
    cmp     [esp + loc52.nMOTION], 0
    jl      .L70
    jmp     .L40

.L50:
    ;
    ; NEXTX = LOOKX
    ; NEXTY = LOOKY
    ;
    mcLoadLocal al, loc52.nLOOKX
    mcLoadLocal dl, loc52.nLOOKY
    mcStoreLocal loc52.nNEXTX, al
    mcStoreLocal loc52.nNEXTY, dl

.L60:
    inc     [esp + loc52.nLL]
    mov     ecx, [esp + loc52.nLL]
    cmp     ecx, [esp + loc52.nSTEPS]
    jbe     .look_next
    ;
    ; Put COMMANDER in new place within same quadrant
    ; QUAD (NEXTX,NEXTY) = IENM
    ;
    mcLoadLocal al, loc52.nNEXTX
    mcLoadLocal dl, loc52.nNEXTY
    call    TArray_QuadPtr
    mcLoadLocal cl, loc52.chIENM
    mov     [ebx], cl
    ;
    ; IF ((NEXTX == COMX) AND (NEXTY == COMY)) GOTO 500
    ;
    mcLoadLocal dl, loc52.nCOMX
    cmp     [esp + loc52.nNEXTX], dl
    jne     .do_reposition

    mcLoadLocal dl, loc52.nCOMY
    cmp     [esp + loc52.nNEXTY], dl
    je      .L500

.do_reposition:
    ;
    ; KX (LOCCOM) = NEXTX
    ; KY (LOCCOM) = NEXTY
    ;
    mcLoadLocal al, loc52.nNEXTX
    mcLoadLocal dl, loc52.nNEXTY
    mcLoadLocal ecx, loc52.nLOCCOM
    dec     ecx
    mcLoadLocal esi, loc52.pKX
    mcLoadLocal edi, loc52.pKY
    mov     [esi + ecx], al
    mov     [edi + ecx], dl
    ;
    ; KDIST (LOCCOM) = sqrt ((SECTX-NEXTX)**2 + (SECTY-NEXTY)**2)
    ;
    imul    ecx, 10
    mcLoadLocal esi, loc52.pTrekData
    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    sub     al, [esp + loc52.nNEXTX]
    sub     dl, [esp + loc52.nNEXTY]
    movsx   eax, al
    movsx   edx, dl
    imul    eax, eax
    imul    edx, edx
    add     eax, edx
    call    TCommon_FPU_Load_EAX
    fsqrt
    mcLoadLocal edi, loc52.pKDIST
    fstp    tbyte [edi + ecx]

    mcLoadNeg1 eax
    mcStoreLocal loc52.nMOTION, eax
    ;
    ; IF (KDIST (LOCCOM) < DIST1) MOTION=1
    ;
    fld     [esp + loc52.dbl_DIST1]
    fld     tbyte [edi + ecx]
    mc_CMP_ST0_ST1
    jnc     .advance_retreat_msg

    neg     [esp + loc52.nMOTION]

.advance_retreat_msg:
    call    TConsole_SetGameMsgAttr
    call    TConsole_Cram3Asterisks
    mcLoadLocal al, loc52.chIENM
    call    TConsole_CramEnemy

    mcZeroBits eax
    add     [esp + loc52.nMOTION], 0
    sets    al
    lea     ecx, [eax + 596]
    call    TConsole_Cram

    mcLoadLocal al, loc52.nNEXTX
    mcLoadLocal dl, loc52.nNEXTY
    mov     cl, 2
    call    TConsole_CramLoc
    call    TConsole_ScrollUp
    invoke  Sleep, 350
    jmp     .L500

.goto_no_exit:
    jmp     [esp + loc52.pNOEXIT]

.L70:
    ;
    ; Try to move into adjacent quadrant, avoiding
    ; negative energy barrier, supernovae and >8 Klingons.
    ;
    ; IQX = QUADX + (LOOKX+9)/10 - 1
    ; IQY = QUADY + (LOOKY+9)/10 - 1
    ;
    mcLoadLocal esi, loc52.pTrekData
    mov     bl, [esi + TREKDATA.QUADX]
    mcLoadLocal al, loc52.nLOOKX
    call    TAttack_Helper3
    mcStoreLocal loc52.nIQX, al

    mov     bl, [esi + TREKDATA.QUADY]
    mcLoadLocal al, loc52.nLOOKY
    call    TAttack_Helper3
    mcStoreLocal loc52.nIQY, al
    ;
    ; IF (IQX,IQY is out of galaxy) GOTO NOEXIT
    ;
    cmp     al, 1
    jb      .goto_no_exit
    cmp     al, 8
    ja      .goto_no_exit

    mcLoadLocal dl, loc52.nIQX
    cmp     dl, 1
    jb      .goto_no_exit
    cmp     dl, 8
    ja      .goto_no_exit
    ;
    ; IF (GALAXY (IQX,IQY) > 899) GOTO NOEXIT
    ;
    xchg    eax, edx
    call    TArray_GetGalaxyValue
    cmp     ecx, 899
    ja      .goto_no_exit

    cmp     [esp + loc52.bIRUN], 0
    jne     .L86

    cmp     [esp + loc52.chIENM], CHAR_SCOM
    je      .L85
    ;
    ; Check if quadrant IQX,IQY will be an intrusion to another COMMANDER
    ;
    mcLoadLocal esi, loc52.pTrekData
    movzx   ecx, [esi + TREKDATA.REMCOM]
    lea     edi, [esi + TREKDATA.CY]
    add     esi, TREKDATA.CX
    mcLoadLocal al, loc52.nIQX
    mcLoadLocal dl, loc52.nIQY

.check_intrusion:
    cmp     [esi], al
    jne     .next_commander

    cmp     [edi], dl
    je      .goto_no_exit

.next_commander:
    inc     esi
    inc     edi
    loop    .check_intrusion
    ;
    ; Romulans do not leave!
    ;
    cmp     [esp + loc52.chIENM], CHAR_ROMULAN
    je      .goto_no_exit
    ;
    ; Do not leave if attacking a base
    ;
    mcLoadLocal esi, loc52.pTrekData
    mov     al, [esi + TREKDATA.QUADX]
    cmp     al, [esi + TREKDATA.BATX]
    jne     .L85

    mov     al, [esi + TREKDATA.QUADY]
    cmp     al, [esi + TREKDATA.BATY]
    je      .goto_no_exit

.L85:
    ;
    ; IF (KPOWER (LOCCOM) > 1000.0) GOTO NOEXIT
    ;
    mcLoadLocal esi, loc52.pTrekData
    mcLoadLocal ecx, loc52.nLOCCOM
    lea     ebx, [esi + TREKDATA.KPOWER]
    dec     ecx
    imul    ecx, 10
    fld     tbyte [ebx + ecx]
    fld     [glb_dbl_1000]
    mc_CMP_ST0_ST1
    jc      .goto_no_exit

.L86:
    ;
    ; Print escape message and bail out of quadrant!
    ;
    call    TConsole_SetGameMsgAttr
    call    TConsole_Cram3Asterisks
    mcLoadLocal al, loc52.chIENM
    call    TConsole_CramEnemy

    mov     ecx, 598
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal al, loc52.nIQX
    mcLoadLocal dl, loc52.nIQY
    call    TConsole_CramLoc

    mov     ecx, 599
    call    TConsole_Prout

    mcLoadLocal ecx, loc52.nLOCCOM
    call    TArray_Leave
    dec     [esp + loc52.nI]
    ;
    ; Handle galaxy matrix
    ;
    call    TArray_MyGalaxyPtr
    sub     dword [ebx], 100

    mcLoadLocal al, loc52.nIQX
    mcLoadLocal dl, loc52.nIQY
    call    TArray_GalaxyPtr
    add     dword [ebx], 100

    cmp     [esp + loc52.chIENM], CHAR_COMMANDER
    je      .L87
    cmp     [esp + loc52.chIENM], CHAR_SCOM
    jne     .L500

    mcLoadLocal esi, loc52.pTrekData
    mcZeroBits eax
    mov     [esi + TREKDATA.ISHERE], al
    mov     [esi + TREKDATA.ISCATE], al
    mov     [esi + TREKDATA.IENTESC], al
    mov     [esi + TREKDATA.ISATB], al

    fld     [esi + TREKDATA.DATE]
    fld     [glb_dbl_0dot2777]
    faddp
    fstp    [esi + TREKDATA.FUTURE6]

    fld     [glb_dbl_1E38]
    fstp    [esi + TREKDATA.FUTURE7]

    mcLoadLocal al, loc52.nIQX
    mcLoadLocal dl, loc52.nIQY
    mov     [esi + TREKDATA.ISX], al
    mov     [esi + TREKDATA.ISY], dl
    jmp     .L500

.L87:
    mcLoadLocal ebx, loc52.pTrekData
    mcLoadMemberRef esi, TREKDATA.CX
    mcLoadMemberRef edi, TREKDATA.CY
    movzx   ecx, [ebx + TREKDATA.REMCOM]
    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY

.find_commander:
    cmp     [esi], al
    jne     .get_next_commander

    cmp     [edi], dl
    je      .L100

.get_next_commander:
    inc     esi
    inc     edi
    loop    .find_commander

    ;int     3
    jmp     .L500

.L100:
    mcLoadLocal al, loc52.nIQX
    mcLoadLocal dl, loc52.nIQY
    mov     [esi], al
    mov     [edi], dl

    mcLoadLocal edi, loc52.pTrekData
    mov     [edi + TREKDATA.COMHERE], 0

.L500:
    mcLoadLocal esi, loc52.pTrekData
    movzx   ecx, [esi + TREKDATA.NENHERE]
    inc     [esp + loc52.nI]
    cmp     [esp + loc52.nI], ecx
    jbe     .L1

    call    TCommon_SortKlingons

.done:
    mcEndLocals loc52.size
    ret

; --------------------------------------------------------------------------
; ATTACK
; --------------------------------------------------------------------------
virtual at 0
loc36:
    .pTrekData PVOID ?
    .bIHURT BOOL ?
    .bATTACKED BOOL ?
    .dbl_HITMAX DOUBLE ?
    .dbl_HITTOT DOUBLE ?
    .dbl_DUSTFAC DOUBLE ?
    .dbl_HIT DOUBLE ?
    .nL INDEX ?
    .nNENHERE COUNT ?
    .pKPOWER PDOUBLE ?
    .pKDIST PDOUBLE ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .chIQUAD BYTE ?
    .ch1 BYTE ?
    .ch2 BYTE ?
    .ch3 BYTE ?
    .nPERCENT INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TAttack_Main:
    mcBeginLocals loc36.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc36.pTrekData, esi

    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    cmp     [esi + TREKDATA.ITHERE], 0
    je      .check_neutral_zone

    push    esi
    call    TEvents_MoveTholian
    pop     esi

.check_neutral_zone:
    cmp     [esi + TREKDATA.NEUTZ], 0
    je      .L1

    dec     [esi + TREKDATA.NEUTZ]
    jmp     .done

.L1:
    cmp     [esi + TREKDATA.NENHERE], 0
    je      .done

    movzx   eax, [esi + TREKDATA.NENHERE]
    mcStoreLocal loc36.nNENHERE, eax

    lea     eax, [esi + TREKDATA.KPOWER]
    lea     edx, [esi + TREKDATA.KDIST]
    lea     edi, [esi + TREKDATA.KX]
    lea     ecx, [esi + TREKDATA.KY]
    mcStoreLocal loc36.pKPOWER, eax
    mcStoreLocal loc36.pKDIST, edx
    mcStoreLocal loc36.pKX, edi
    mcStoreLocal loc36.pKY, ecx

    mov     al, [esi + TREKDATA.ISHERE]
    add     al, [esi + TREKDATA.COMHERE]
    mcOnRegZero al, .check_skill

    cmp     [esi + TREKDATA.JUSTIN], 0
    jne     .check_skill

.move_enemies:
    call    TAttack_MoveCommanders
    invoke  Sleep, 200

    mcLoadLocal esi, loc36.pTrekData
    movzx   eax, [esi + TREKDATA.NENHERE]
    mcStoreLocal loc36.nNENHERE, eax
    jmp     .hit_the_ship

.check_skill:
    cmp     [esi + TREKDATA.KSTUF5], 2
    je      .move_enemies

.hit_the_ship:
    mcZeroBits eax
    mcStoreLocal loc36.bIHURT, eax
    mcStoreLocal loc36.bATTACKED, eax
    mcStoreLocal loc36.nL, eax
    call    TConsole_ScrollUp

    fldz
    fldz
    fstp    [esp + loc36.dbl_HITMAX]
    fstp    [esp + loc36.dbl_HITTOT]
    jmp     .L5

.next_evil_doer:
    inc     [esp + loc36.pKX]
    inc     [esp + loc36.pKY]
    add     [esp + loc36.pKPOWER], 10
    add     [esp + loc36.pKDIST], 10

.L5:
    inc     [esp + loc36.nL]
    mcLoadLocal ecx, loc36.nL
    cmp     ecx, [esp + loc36.nNENHERE]
    ja      .L80
    ;
    ; If ship is docked - no hits!
    ;
    mcLoadLocal esi, loc36.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L6

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 241
    call    TConsole_Cram
    call    TConsole_CramShip
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    jmp     .done

.L6:
    ;
    ; If enemy's power is negative - they can't attack
    ;
    mcLoadLocal esi, loc36.pKPOWER
    fldz
    fld     tbyte [esi]
    mc_CMP_ST0_ST1
    jc      .next_evil_doer
    jz      .next_evil_doer

    inc     [esp + loc36.bATTACKED]

    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    fmulp
    fld     [glb_dbl_0dot8]
    faddp
    fstp    [esp + loc36.dbl_DUSTFAC]

    mcLoadLocalRef esi, loc36.dbl_DUSTFAC
    mcLoadLocal edi, loc36.pKDIST

    call    TCommon_X_exp_Y
    mcLoadLocal ebx, loc36.pKPOWER
    fld     tbyte [ebx]
    fmulp
    fstp    [esp + loc36.dbl_HIT]

    mcLoadLocal esi, loc36.pKX
    mcLoadLocal edi, loc36.pKY
    mov     al, [esi]
    mov     dl, [edi]
    call    TArray_QuadPtr
    mov     al, [ebx]
    mcStoreLocal loc36.chIQUAD, al
    ;
    ; Decide if enemy will fire a torpedo.
    ;
    fld     [esp + loc36.dbl_HIT]
    fld     [glb_dbl_400]
    mc_CMP_ST0_ST1
    jc      .L10

    mcLoadLocal esi, loc36.pTrekData
    cmp     [esi + TREKDATA.JUSTIN], 1
    je      .L10

    fld     [glb_dbl_300]
    fld     [esp + loc36.dbl_HIT]
    mc_CMP_ST0_ST1
    jc      .check_enemy_type

    mcLoadLocal edi, loc36.pKDIST
    fld     tbyte [edi]
    fld     [glb_dbl_5]
    mc_CMP_ST0_ST1
    jc      .L10

.check_enemy_type:
    cmp     [esp + loc36.chIQUAD], CHAR_KLINGON
    jne     .torpedo_fired

    call    TRandom_Ranf
    fld     [glb_dbl_0dot1]
    mc_CMP_ST0_ST1
    jc      .L10

.torpedo_fired:
    mcLoadLocal edi, loc36.pTrekData
    mov     [edi + TREKDATA.IPHWHO], 1

    mcLoadLocal eax, loc36.nL
    mov     [edi + TREKDATA.KSHOT], al
    push    edi
    call    KPHOTON
    pop     edi

    mov     [edi + TREKDATA.IPHWHO], 127
    mov     [edi + TREKDATA.KDIDIT], 1

    cmp     [edi + TREKDATA.ALLDONE], 0
    jne     .done

    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    je      .done
    jmp     .next_evil_doer

.L10:
    ;
    ; Enemy fires a disruptor weapon
    ;
    mcLoadLocal ecx, loc36.nL
    mcLoadLocalRef esi, loc36.dbl_HIT
    mcLoadLocalRef edi, loc36.bIHURT
    call    TAttack_Zap

    fld     [esp + loc36.dbl_HIT]
    fld     [esp + loc36.dbl_HITTOT]
    faddp
    fstp    [esp + loc36.dbl_HITTOT]

    fld     [esp + loc36.dbl_HITTOT]
    fld     [esp + loc36.dbl_HITMAX]
    call    TCommon_FPU_Max
    fstp    [esp + loc36.dbl_HITMAX]
    ;
    ; Loop for enemy attacks ends here
    ;
    jmp     .next_evil_doer

.L80:
    cmp     [esp + loc36.bATTACKED], 0
    je      .done

    fld     [esp + loc36.dbl_HITTOT]
    fldz
    mc_CMP_ST0_ST1
    jz      .done

    mcLoadLocal esi, loc36.pTrekData
    fldz
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L100
    jz      .L100

    fld     [esi + TREKDATA.SHLD]
    fld     [esi + TREKDATA.INSHLD]
    fdivp
    fld     [glb_dbl_100]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc36.nPERCENT, eax

    cmp     [esp + loc36.bIHURT], 0
    jne     .L85

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 247
    call    TConsole_Cram
    jmp     .L90

.L85:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr

    mcLoad8bitsToReg32 ecx, 248
    call    TConsole_Cram

    mcLoadLocal esi, loc36.pTrekData
    mov     cl, 2
    fld     [esi + TREKDATA.ENERGY]
    call    TConsole_CramFloat

    mcLoad8bitsToReg32 ecx, 249
    call    TConsole_Cram

    mcLoadLocal esi, loc36.pTrekData
    movzx   eax, [esi + TREKDATA.TORPS]
    call    TConsole_CramInt

    mcLoad8bitsToReg32 ecx, 250
    call    TConsole_Cram

    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jc      .shields_damaged

    mcLoadLocal esi, loc36.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .shields_down

    push    251
    jmp     .cram_shield_state

.shields_down:
    push    252
    jmp     .cram_shield_state

.shields_damaged:
    push    253

.cram_shield_state:
    pop     ecx
    call    TConsole_Cram

.L90:
    mcLoadLocal eax, loc36.nPERCENT
    call    TConsole_CramInt
    mov     al, '%'
    call    TConsole_PutChar
    call    TConsole_ScrollUp
    ;
    ; Check for casualties.
    ;
    fld     [glb_dbl_200]
    fld     [esp + loc36.dbl_HITMAX]
    mc_CMP_ST0_ST1
    jc      .check_total_hit

.get_casualties:
    fld     [esp + loc36.dbl_HITTOT]
    call    TAttack_Casualties
    call    TConsole_SetGameMsgAttr

.L120:
    call    TCommon_SortKlingons
    jmp     .done

.check_total_hit:
    fld     [glb_dbl_500]
    fld     [esp + loc36.dbl_HITTOT]
    mc_CMP_ST0_ST1
    jc      .L120
    jmp     .get_casualties

.L100:
    mov     al, 5
    call    TFinish_Main

.done:
    mcEndLocals loc36.size
    ret

; --- EOF ---
