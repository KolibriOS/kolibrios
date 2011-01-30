; --------------------------------------------------------------------------
; FILE: TPhasers.Asm
; DATE: October 18, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   AL = quadrant X of a COMMANDER
;   DL = quadrant Y of a COMMANDER
; Output:
;   CF=1 if commander was removed
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhasers_RemoveCommander:
    pushad

    mcLoadGameDataPtr ebx
    cmp     [ebx + TREKDATA.REMCOM], 0
    je      .not_removed

    mcLoadMemberRef esi, TREKDATA.CX
    mcLoadMemberRef edi, TREKDATA.CY
    mcZeroBits ecx

.find_commander:
    cmp     [esi + ecx], al
    jne     .next_commander
    cmp     [edi + ecx], dl
    jne     .next_commander
    ;
    ; Replace commander by the last entry
    ;
    movzx   edx, [ebx + TREKDATA.REMCOM]
    dec     edx
    push    edx

    mov     al, [esi + edx]
    mov     dl, [edi + edx]
    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    mcZeroBits eax
    pop     edx
    mov     [esi + edx], al
    mov     [edi + edx], al

    dec     [ebx + TREKDATA.REMCOM]

    popad
    stc
    ret

.next_commander:
    inc     ecx
    cmp     cl, [ebx + TREKDATA.REMCOM]
    jb      .find_commander

.not_removed:
    popad
    clc
    ret

; --------------------------------------------------------------------------
; DEADKL
; --------------------------------------------------------------------------
; Input:
;   AL = sector X of an enemy
;   DL = sector Y of an enemy
; --------------------------------------------------------------------------
virtual at 0
loc41:
    .pTrekData PVOID ?
    .pQUAD PBYTE ?
    .nTYPE BYTE ?
    .nIXX BYTE ?
    .nIYY BYTE ?
    .nEnemies BYTE ?
    .nJ INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhasers_DeadKlingon:
    mcBeginLocals loc41.size

    mcStoreLocal loc41.nIXX, al
    mcStoreLocal loc41.nIYY, dl

    call    TArray_QuadPtr
    mcStoreLocal loc41.pQUAD, ebx

    mov     cl, [ebx]
    mcStoreLocal loc41.nTYPE, cl

    mcLoadGameDataPtr ebx
    mcStoreLocal loc41.pTrekData, ebx

    call    TConsole_SetGameMsgAttr
    mcLoadLocal al, loc41.nIXX
    mcLoadLocal dl, loc41.nIYY
    mcLoadLocal bl, loc41.nTYPE
    mov     cl, 2
    call    TConsole_CramEnemyAt

    mcLoadLocal al, loc41.nTYPE
    mcOnRegEqu al, CHAR_THOLIAN, .L30
    mcOnRegEqu al, CHAR_ROMULAN, .L27

    call    TArray_MyGalaxyPtr
    sub     dword [ebx], 100

    mcLoadLocal edi, loc41.pTrekData
    dec     [edi + TREKDATA.KLHERE]
    dec     [edi + TREKDATA.REMKL]

    mcLoadLocal al, loc41.nTYPE
    mcOnRegEqu al, CHAR_KLINGON, .L25
    mcOnRegEqu al, CHAR_SCOM, .L26
    ;
    ; COMMANDER finds his fate!
    ;
    mcLoadLocal edi, loc41.pTrekData
    mov     [edi + TREKDATA.COMHERE], 0

    mov     al, [edi + TREKDATA.QUADX]
    mov     dl, [edi + TREKDATA.QUADY]
    call    TPhasers_RemoveCommander

    fld     [glb_dbl_1E38]
    fstp    [edi + TREKDATA.FUTURE2]

    cmp     [edi + TREKDATA.REMCOM], 0
    je      .kill_commander

    mov     al, [edi + TREKDATA.INCOM]
    call    TCommon_LoadByteIntoFPU
    mov     al, [edi + TREKDATA.REMCOM]
    call    TCommon_LoadByteIntoFPU
    fdivp
    push    edi
    call    TCommon_ExpRan
    pop     edi
    fld     [edi + TREKDATA.DATE]
    faddp
    fstp    [edi + TREKDATA.FUTURE2]

.kill_commander:
    inc     [edi + TREKDATA.KILLC]
    jmp     .L30

.L25:
    ;
    ; Ordinary KLINGON wasted!
    ;
    inc     [edi + TREKDATA.KILLK]
    jmp     .L30

.L26:
    ;
    ; SUPER-COMMANDER finally brought down!
    ;
    mcZeroBits eax
    mov     [edi + TREKDATA.NSCREM], al
    mov     [edi + TREKDATA.ISHERE], al
    mov     [edi + TREKDATA.ISX], al
    mov     [edi + TREKDATA.ISY], al
    mov     [edi + TREKDATA.ISATB], al
    mov     [edi + TREKDATA.ISCATE], al

    inc     [edi + TREKDATA.NSCKILL]
    fld     [glb_dbl_1E38]
    fld     st
    fstp    [edi + TREKDATA.FUTURE6]
    fstp    [edi + TREKDATA.FUTURE7]
    jmp     .L30

.L27:
    ;
    ; ROMULAN destroyed!
    ;
    call    TArray_MyNewStufPtr
    sub     byte [ebx], 10

    dec     [edi + TREKDATA.IRHERE]
    dec     [edi + TREKDATA.NROMREM]
    inc     [edi + TREKDATA.NROMKL]

.L30:
    ;
    ; Finish message
    ;
    mov     ecx, 306
    call    TConsole_Prout

    mcLoadLocal ebx, loc41.pQUAD
    mov     byte [ebx], CHAR_COSMOS

    mcLoadLocal esi, loc41.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    je      .done

    mov     al, [esi + TREKDATA.REMCOM]
    shl     al, 2
    add     al, [esi + TREKDATA.REMKL]
    call    TCommon_LoadByteIntoFPU
    fld     [esi + TREKDATA.REMRES]
    fdivrp
    fstp    [esi + TREKDATA.REMTIME]
    ;
    ; Kill Tholian
    ;
    cmp     [esp + loc41.nTYPE], CHAR_THOLIAN
    je      .kill_tholian
    ;
    ; Remove enemy ship from arrays describing local conditions
    ;
    mov     cl, [esi + TREKDATA.NENHERE]
    mcStoreLocal loc41.nEnemies, cl
    lea     edi, [esi + TREKDATA.KY]
    add     esi, TREKDATA.KX
    mcZeroBits ecx
    mcLoadLocal al, loc41.nIXX
    mcLoadLocal dl, loc41.nIYY

.find_enemy_ship:
    cmp     al, [esi + ecx]
    jne     .next_ship
    cmp     dl, [edi + ecx]
    je      .L45

.next_ship:
    inc     ecx
    cmp     cl, [esp + loc41.nEnemies]
    jb      .find_enemy_ship

    ;int     3
    jmp     .done

.L45:
    inc     ecx

    mcLoadLocal ebx, loc41.pTrekData
    dec     [ebx + TREKDATA.NENHERE]
    dec     [esp + loc41.nEnemies]

    cmp     cl, [ebx + TREKDATA.NENHERE]
    ja      .L55
    ;
    ; Shift all ships up to delete the killed one
    ;
    mcStoreLocal loc41.nJ, ecx

.remove_ship:
    mcLoadLocal ecx, loc41.nJ
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    dec     ecx
    mov     al, [esi + ecx + 1]
    mov     dl, [edi + ecx + 1]
    mov     [esi + ecx], al
    mov     [edi + ecx], dl

    imul    ecx, 10
    mcLoadMemberRef esi, TREKDATA.KPOWER
    mcLoadMemberRef edi, TREKDATA.KDIST
    fld     tbyte [esi + ecx + 10]
    fld     tbyte [edi + ecx + 10]
    fstp    tbyte [edi + ecx]
    fstp    tbyte [esi + ecx]

    inc     [esp + loc41.nJ]
    mov     ecx, [esp + loc41.nJ]
    cmp     cl, [esp + loc41.nEnemies]
    jbe     .remove_ship

.L55:
    ;
    ; Cleanup the tail of arrays
    ;
    movzx   ecx, [esp + loc41.nEnemies]
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mcZeroBits eax
    mov     [esi + ecx], al
    mov     [edi + ecx], al

    imul    ecx, 10
    fldz
    fldz
    mcLoadMemberRef esi, TREKDATA.KPOWER
    mcLoadMemberRef edi, TREKDATA.KDIST
    fstp    tbyte [esi + ecx]
    fstp    tbyte [edi + ecx]
    jmp     .done

.kill_tholian:
    dec     [esi + TREKDATA.ITHERE]

.done:
    mcEndLocals loc41.size
    ret

; --------------------------------------------------------------------------
; HITEM
; --------------------------------------------------------------------------
; Input:
;   ESI = pointer to array of DOUBLE values with hits
; --------------------------------------------------------------------------
virtual at 0
loc40:
    .pTrekData PVOID ?
    .pHITS PDOUBLE ?
    .nNENHER2 COUNT ?
    .nK INDEX ?
    .nKK INDEX ?
    .dbl_KP DOUBLE ?
    .dbl_KPOW DOUBLE ?
    .pKPOWER PDOUBLE ?
    .pKDIST PDOUBLE ?
    .pKX PBYTE ?
    .pKY PBYTE ?
    .nII BYTE ?
    .nJJ BYTE ?
    .n1 BYTE ?
    .n2 BYTE ?
    .dbl_HIT DOUBLE ?
    .dbl_WHAM DOUBLE ?
    .dbl_DUSTFAC DOUBLE ?
    .dbl_1 DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhasers_HitThem:
    mcBeginLocals loc40.size

    mcLoadGameDataPtr edi
    mcStoreLocal loc40.pTrekData, edi
    mcStoreLocal loc40.pHITS, esi

    movzx   eax, [edi + TREKDATA.NENHERE]
    mcStoreLocal loc40.nNENHER2, eax

    lea     ebx, [edi + TREKDATA.KX]
    lea     ecx, [edi + TREKDATA.KY]
    lea     esi, [edi + TREKDATA.KPOWER]
    lea     eax, [edi + TREKDATA.KDIST]
    mcStoreLocal loc40.pKX, ebx
    mcStoreLocal loc40.pKY, ecx
    mcStoreLocal loc40.pKPOWER, esi
    mcStoreLocal loc40.pKDIST, eax

    call    TConsole_ScrollUp

    mcLoad1 eax
    mcStoreLocal loc40.nKK, eax
    mcStoreLocal loc40.nK, eax

.next_hit:
    invoke  Sleep, 500

    mcLoadLocal ecx, loc40.nK
    dec     ecx
    imul    ecx, 10
    mcLoadLocal esi, loc40.pHITS
    fld     tbyte [esi + ecx]
    fstp    [esp + loc40.dbl_WHAM]

    fldz
    fld     [esp + loc40.dbl_WHAM]
    mc_CMP_ST0_ST1
    jz      .L30

    call    TRandom_Ranf
    fld     [glb_dbl_0dot01]
    fmulp
    fld     [glb_dbl_0dot9]
    faddp
    fstp    [esp + loc40.dbl_DUSTFAC]

    mcLoadLocal ecx, loc40.nKK
    dec     ecx
    imul    ecx, 10
    mcLoadLocal esi, loc40.pKDIST
    lea     edi, [esi + ecx]
    mcLoadLocalRef esi, loc40.dbl_DUSTFAC
    call    TCommon_X_exp_Y
    fld     [esp + loc40.dbl_WHAM]
    fmulp
    fstp    [esp + loc40.dbl_HIT]

    mcLoadLocal ecx, loc40.nKK
    dec     ecx
    imul    ecx, 10
    mcLoadLocal esi, loc40.pKPOWER
    fld     tbyte [esi + ecx]
    fstp    [esp + loc40.dbl_KP]

    fld     [esp + loc40.dbl_KP]
    fabs
    fld     [esp + loc40.dbl_HIT]
    fld     st
    faddp
    call    TCommon_FPU_Min

    fld     [esp + loc40.dbl_KP]
    call    TCommon_FPU_SignAB

    fld     [esp + loc40.dbl_KP]
    fsubrp
    fld     st
    fstp    tbyte [esi + ecx]
    fstp    [esp + loc40.dbl_KPOW]

    mcLoadLocal ecx, loc40.nKK
    dec     ecx
    mcLoadLocal esi, loc40.pKX
    mcLoadLocal edi, loc40.pKY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    mcStoreLocal loc40.nII, al
    mcStoreLocal loc40.nJJ, dl

    call    TConsole_SetGameMsgAttr
    fld     [esp + loc40.dbl_HIT]
    fld     [glb_dbl_0dot005]
    mc_CMP_ST0_ST1
    jc      .L10

    mov     ecx, 302
    call    TConsole_Cram
    jmp     .L20

.L10:
    fld     [esp + loc40.dbl_HIT]
    mov     cl, 2
    call    TConsole_CramFloat

    mov     ecx, 303
    call    TConsole_Cram

.L20:
    mcLoadLocal al, loc40.nII
    mcLoadLocal dl, loc40.nJJ
    call    TConsole_CramEnemyAtEx
    call    TConsole_ScrollUp

    fldz
    fld     [esp + loc40.dbl_KPOW]
    mc_CMP_ST0_ST1
    jnz     .L25

    mcLoadLocal al, loc40.nII
    mcLoadLocal dl, loc40.nJJ
    call    TPhasers_DeadKlingon

    mcLoadLocal esi, loc40.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    je      .game_won

    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done
    jmp     .L40

.L25:
    fldz
    fld     [esp + loc40.dbl_KPOW]
    mc_CMP_ST0_ST1
    jc      .L30

    fld     [glb_dbl_0dot9]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L30

    call    TRandom_Ranf
    fld     [glb_dbl_0dot4]
    fmulp
    fld     [glb_dbl_0dot4]
    faddp
    fld     [esp + loc40.dbl_KP]
    fmulp
    fld     [esp + loc40.dbl_KPOW]
    mc_CMP_ST0_ST1
    jnc     .L30

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 304
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal al, loc40.nII
    mcLoadLocal dl, loc40.nJJ
    call    TConsole_CramLoc

    call    TConsole_ScrollUp
    mov     ecx, 305
    call    TConsole_Prout

    fld     [esp + loc40.dbl_KPOW]
    fchs

    mcLoadLocal ecx, loc40.nKK
    dec     ecx
    imul    ecx, 10
    mcLoadLocal esi, loc40.pKPOWER
    fstp    tbyte [esi + ecx]

.L30:
    inc     [esp + loc40.nKK]

.L40:
    ;
    ; Next hit
    ;
    inc     [esp + loc40.nK]
    mov     ecx, [esp + loc40.nK]
    cmp     ecx, [esp + loc40.nNENHER2]
    jbe     .next_hit
    jmp     .done

.game_won:
    mov     al, 1
    call    TFinish_Main

.done:
    mcEndLocals loc40.size
    ret

; --------------------------------------------------------------------------
; PHASERS
; --------------------------------------------------------------------------
virtual at 0
loc39:
    .pTrekData PVOID ?
    .bIFAST BOOL ?
    .bIPOOP BOOL ?
    .bIDOIT BOOL ?
    .bNO BOOL ?
    .nK INDEX ?
    .nI INDEX ?
    .pIWHERE PVOID ?
    .nEnemies COUNT ?
    .nKEY UINT32 ?
    .nKEY1 UINT32 ?
    .nKEY2 UINT32 ?
    .nIREC UINT32 ?
    .pszAITEM PCHAR ?
    .dbl_POW DOUBLE ?
    .dbl_POWREM DOUBLE ?
    .dbl_EXTRA DOUBLE ?
    .dbl_TEMP DOUBLE ?
    .dbl_PART1 DOUBLE ?
    .dbl_PART2 DOUBLE ?
    .dbl_HITS_I_ DOUBLE ?
    .dbl_OVER DOUBLE ?
    .arr_HITS DOUBLES 20
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhasers_Main:
    mcBeginLocals loc39.size

    call    TConsole_SetGameMsgAttr

    mcLoadGameDataPtr esi
    mcStoreLocal loc39.pTrekData, esi

    movzx   ecx, [esi + TREKDATA.NENHERE]
    mcStoreLocal loc39.nEnemies, ecx

    mcZeroBits eax
    mcStoreLocal loc39.bIFAST, eax
    mcStoreLocal loc39.bNO, eax
    mcStoreLocal loc39.nIREC, eax

    inc     eax
    mcStoreLocal loc39.bIPOOP, eax
    mcStoreLocal loc39.bIDOIT, eax
    mov     [esi + TREKDATA.IDIDIT], al

    mov     cl, DEV_SR_SENSORS
    call    TArray_IsDamaged
    adc     [esp + loc39.nIREC], 0

    mov     cl, DEV_COMPUTER
    call    TArray_IsDamaged
    adc     [esp + loc39.nIREC], 0

    cmp     [esp + loc39.nIREC], 0
    je      .check_firing_solution

    dec     [esp + loc39.bIPOOP]

.check_firing_solution:
    mcLoadLocal esi, loc39.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L5

    mov     ecx, 274
    call    TConsole_Prout
    jmp     .L19

.L5:
    mov     cl, DEV_PHASERS
    call    TArray_IsDamaged
    jnc     .L10

    mov     ecx, 275
    call    TConsole_Prout
    jmp     .L19

.L10:
    ;
    ; Do checks for hi-speed shield control
    ;
    mcLoadLocal esi, loc39.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L20

    mov     cl, DEV_SHIELD_CONTROL
    call    TArray_IsDamaged
    jnc     .L13

    mov     ecx, 276
    call    TConsole_Prout
    jmp     .L19

.L13:
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_200]
    mc_CMP_ST0_ST1
    jc      .L16

    mov     ecx, 277
    call    TConsole_Prout

.L19:
    dec     [esp + loc39.bIDOIT]

    cmp     [esp + loc39.bIPOOP], 0
    je      .L15
    cmp     [esp + loc39.nEnemies], 0
    je      .L15

    mov     ecx, 278
    call    TConsole_Prout
    jmp     .L870

.L15:
    mcLoadLocal esi, loc39.pTrekData
    dec     [esi + TREKDATA.IDIDIT]

    cmp     [esp + loc39.bIFAST], 0
    je      .done

    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_200]
    faddp
    fstp    [esi + TREKDATA.ENERGY]
    jmp     .done

.L16:
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_200]
    fsubp
    fstp    [esi + TREKDATA.ENERGY]
    inc     [esp + loc39.bIFAST]

.L20:
    ;
    ; Read in amount of energy to expend on phaser fire
    ;
    call    TCmdBuf_Scan
    mcStoreLocal loc39.pszAITEM, esi

    movzx   ecx, [ebx + TCmdBuf.cmdbuf_KEY]
    mcStoreLocal loc39.nKEY, ecx

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc39.dbl_POW]

    cmp     [esp + loc39.nEnemies], 0
    je      .L35

    mcLoad1 edx
    mcStoreLocal loc39.nK, edx

    cmp     [esp + loc39.nKEY], CMD_TOKEN_ALPHA
    je      .L23
    cmp     [esp + loc39.nKEY], CMD_TOKEN_REAL
    je      .L28

    mcLoadLocal esi, loc39.pszAITEM
    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jnc     .L23

    inc     [esp + loc39.bNO]

    call    TCmdBuf_Scan
    mcStoreLocal loc39.pszAITEM, esi

    movzx   ecx, [ebx + TCmdBuf.cmdbuf_KEY]
    mcStoreLocal loc39.nKEY, ecx

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc39.dbl_POW]

.L23:
    cmp     [esp + loc39.nKEY], CMD_TOKEN_ALPHA
    jne     .check_devices

    mcLoadLocal esi, loc39.pszAITEM
    mcLoad8bitsToReg32 ecx, 165
    call    TCmdBuf_Crop
    jc      .L90

.check_devices:
    mov     cl, DEV_COMPUTER
    call    TArray_IsDamaged
    jc      .L80

    mov     cl, DEV_SR_SENSORS
    call    TArray_IsDamaged
    jc      .L81
    ;
    ; Is KEY = EOL, 'AUTO' or phaser power
    ;
    cmp     [esp + loc39.nKEY], CMD_TOKEN_ALPHA
    jne     .L2301

    mcLoadLocal esi, loc39.pszAITEM
    mcLoad8bitsToReg32 ecx, 166
    call    TCmdBuf_Crop
    jc      .L27
    jmp     .L24

.L2301:
    ;
    ; Differentiate between EOL and phaser power (command mode)
    ;
    cmp     [esp + loc39.nKEY], CMD_TOKEN_EOL
    jne     .L30

.L24:
    ;
    ; Request MANUAL/AUTO fire
    ;
    mcLoad8bitsToReg32 ecx, 174
    call    TGame_Prompt
    call    TConsole_SetGameMsgAttr

    call    TCmdBuf_Scan

    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    add     eax, 0
    js      .L15

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    jne     .L24

    mcLoad8bitsToReg32 ecx, 165
    call    TCmdBuf_Crop
    jc      .L90

    mcLoad8bitsToReg32 ecx, 166
    call    TCmdBuf_Crop
    jnc     .L24

.L2409:
    ;
    ; Inform user of available energy and read in
    ; desired phaser power
    ;
    cmp     [esp + loc39.nEnemies], 0
    je      .L26

    mov     ecx, 279
    call    TConsole_Cram

.L25:
    mcLoadLocal esi, loc39.pTrekData
    mov     cl, 2
    fld     [esi + TREKDATA.ENERGY]
    call    TConsole_CramFloat
    call    TConsole_ScrollUp

    cmp     [esp + loc39.bIPOOP], 0
    je      .L26

    mov     al, '('
    call    TConsole_PutChar

    mcZeroBits eax
    mcStoreLocal loc39.nIREC, eax

    inc     eax
    mcStoreLocal loc39.nK, eax

.sum_all_power:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    fmulp
    fld     [glb_dbl_1dot01]
    faddp
    fstp    [esp + loc39.dbl_PART2]

    mov     esi, glb_dbl_0dot9
    mcLoadLocal ebx, loc39.pTrekData
    mcLoadLocal ecx, loc39.nK
    dec     ecx
    imul    ecx, 10
    lea     edi, [ebx + ecx + TREKDATA.KDIST]
    call    TCommon_X_exp_Y
    fld     st
    faddp
    fstp    [esp + loc39.dbl_PART1]

    fld     [ebx + ecx + TREKDATA.KPOWER]
    fabs

    fld     [esp + loc39.dbl_PART1]
    fdivp

    fld     [esp + loc39.dbl_PART2]
    fmulp

    call    TCommon_FP_Truncate
    inc     eax
    add     [esp + loc39.nIREC], eax

    inc     [esp + loc39.nK]
    mcLoadLocal ecx, loc39.nK
    cmp     ecx, [esp + loc39.nEnemies]
    jbe     .sum_all_power

    mcLoadLocal eax, loc39.nIREC
    call    TConsole_CramInt
    mov     ecx, 280
    call    TConsole_Prout

.L26:
    mov     ecx, 281
    call    TGame_Prompt

.L27:
    call    TCmdBuf_Scan

    movzx   eax, [ebx + TCmdBuf.cmdbuf_KEY]
    mcStoreLocal loc39.nKEY, eax

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc39.dbl_POW]

.L28:
    mcLoadLocal eax, loc39.nKEY
    mcStoreLocal loc39.nKEY1, eax

    cmp     eax, CMD_TOKEN_EOL
    je      .L2409

.L30:
    call    TCmdBuf_Scan
    mcStoreLocal loc39.pszAITEM, esi

    movzx   eax, [ebx + TCmdBuf.cmdbuf_KEY]
    mcStoreLocal loc39.nKEY2, eax

    mcLoadLocal eax, loc39.nKEY1
    mcStoreLocal loc39.nKEY, eax

    cmp     [esp + loc39.nKEY2], CMD_TOKEN_EOL
    je      .L35

    mcLoadLocal esi, loc39.pszAITEM
    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jnc     .L34

    inc     [esp + loc39.bNO]
    jmp     .L35

.L34:
    call    TMove_BegPardon
    jmp     .L15

.L35:
    cmp     [esp + loc39.nKEY], CMD_TOKEN_REAL
    jne     .L26

    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc39.dbl_POW]
    mc_CMP_ST0_ST1
    jc      .L41

    mov     ecx, 282
    call    TConsole_Cram
    jmp     .L25

.L41:
    fld     [esp + loc39.dbl_POW]
    fld     [glb_dbl_5]
    mc_CMP_ST0_ST1
    jc      .L46
    jz      .L46

    fldz
    fld     [esp + loc39.dbl_POW]
    mc_CMP_ST0_ST1
    jc      .L15
    jz      .L15

    mov     ecx, 283
    call    TConsole_Prout
    jmp     .L15

.L42:
    ;
    ; Print message for shield control
    ; and decide if malfunction occurs
    ;
    call    TConsole_ScrollUp

    fld     [glb_dbl_0dot995]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L45
    ;
    ; Malfunction!
    ;
    call    TMove_RedAlert
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 284
    call    TConsole_Prout

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 285
    call    TConsole_Prout
    jmp     .done

.L45:
    call    TConsole_SetGameMsgAttr
    mov     ecx, 229
    call    TConsole_Prout
    jmp     [esp + loc39.pIWHERE]

.L46:
    ;
    ; Allocate energy among klingons according to
    ; nearest first strategy and compute hits
    ;
    cmp     [esp + loc39.bIFAST], 0
    je      .L47

    mov     eax, .L47
    mcStoreLocal loc39.pIWHERE, eax
    jmp     .L42

.L47:
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc39.dbl_POW]
    fsubp
    fld     st
    fstp    [esi + TREKDATA.ENERGY]
    fstp    [esp + loc39.dbl_EXTRA]

    cmp     [esp + loc39.nEnemies], 0
    je      .L75
    ;
    ; Loop that distribute hits in automatic mode
    ;
    fldz
    fstp    [esp + loc39.dbl_EXTRA]

    fld     [esp + loc39.dbl_POW]
    fstp    [esp + loc39.dbl_POWREM]

    mcLoad1 eax
    mcStoreLocal loc39.nI, eax

.compute_hit:
    fldz
    fstp    [esp + loc39.dbl_HITS_I_]

    fldz
    fld     [esp + loc39.dbl_POWREM]
    mc_CMP_ST0_ST1
    jc      .L50
    jz      .L50

    mcLoadLocal ebx, loc39.pTrekData
    mcLoadLocal ecx, loc39.nI
    dec     ecx
    imul    ecx, 10
    lea     edi, [ebx + ecx + TREKDATA.KDIST]
    mov     esi, glb_dbl_0dot9
    call    TCommon_X_exp_Y
    fld     st
    faddp

    fld     [ebx + ecx + TREKDATA.KPOWER]
    fabs
    fdivrp
    fld     st
    fstp    [esp + loc39.dbl_HITS_I_]
    fstp    [esp + ecx + loc39.arr_HITS]

    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    fmulp
    fld     [glb_dbl_0dot01]
    faddp
    fld     [esp + loc39.dbl_HITS_I_]
    fmulp
    fstp    [esp + loc39.dbl_OVER]

    fld     [esp + loc39.dbl_POWREM]
    fstp    [esp + loc39.dbl_TEMP]

    fld     [esp + loc39.dbl_POWREM]
    fld     [esp + loc39.dbl_HITS_I_]
    fsubp
    fld     [esp + loc39.dbl_OVER]
    fsubp
    fld     st
    fstp    [esp + loc39.dbl_POWREM]

    fldz
    mc_CMP_ST0_ST1
    jc      .extra_plus_over

    fld     [esp + loc39.dbl_TEMP]
    fld     [esp + loc39.dbl_HITS_I_]
    call    TCommon_FPU_Min
    fstp    [esp + loc39.dbl_HITS_I_]

    fldz
    fstp    [esp + loc39.dbl_OVER]

    mcLoadLocal ecx, loc39.nI
    dec     ecx
    imul    ecx, 10
    fld     [esp + loc39.dbl_HITS_I_]
    fstp    [esp + ecx + loc39.arr_HITS]

.extra_plus_over:
    fld     [esp + loc39.dbl_EXTRA]
    fld     [esp + loc39.dbl_OVER]
    faddp
    fstp    [esp + loc39.dbl_EXTRA]

.L50:
    inc     [esp + loc39.nI]
    mov     ecx, [esp + loc39.nI]
    cmp     ecx, [esp + loc39.nEnemies]
    jbe     .compute_hit

    fldz
    fld     [esp + loc39.dbl_POWREM]
    mc_CMP_ST0_ST1
    jc      .hit_them
    jz      .hit_them

    fld     [esp + loc39.dbl_EXTRA]
    fld     [esp + loc39.dbl_POWREM]
    faddp
    fstp    [esp + loc39.dbl_EXTRA]

.hit_them:
    mcLoadLocalRef esi, loc39.arr_HITS
    call    TPhasers_HitThem

    fldz
    fld     [esp + loc39.dbl_EXTRA]
    mc_CMP_ST0_ST1
    jz      .L200

    mcLoadLocal esi, loc39.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .L200

.L75:
    ;
    ; Inform of overkill
    ;
    mcLoadLocal esi, loc39.pTrekData
    cmp     [esi + TREKDATA.ITHERE], 0
    je      .L78

    call    TConsole_SetGameMsgAttr
    mov     ecx, 286
    call    TConsole_Cram

    cmp     [esp + loc39.nEnemies], 0
    je      .L7501

    mov     ecx, 287
    call    TConsole_Cram

.L7501:
    mov     ecx, 288
    call    TConsole_Prout
    jmp     .L200

.L78:
    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    fld     [esp + loc39.dbl_EXTRA]
    call    TConsole_CramFloat
    mov     ecx, 289
    call    TConsole_Prout
    jmp     .L200

.L80:
    ;
    ; Manual section begins here.
    ; Inform of imposed manual condition.
    ;
    call    TConsole_SetGameMsgAttr
    mov     ecx, 299
    call    TConsole_Prout
    jmp     .L84

.L81:
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mov     ecx, 290
    call    TConsole_Prout
    mov     ecx, 291
    call    TConsole_Prout
    mov     ecx, 292
    call    TConsole_Prout
    mov     ecx, 293
    call    TConsole_Prout
    call    TConsole_ScrollUp

.L84:
    call    TConsole_SetGameMsgAttr
    mov     ecx, 282
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [glb_dbl_0dot006]
    fsubp
    call    TConsole_CramFloat
    call    TConsole_ScrollUp

.L870:
    ;
    ; Loop for desired individual hits
    ;
    mcLoad1 eax
    mcStoreLocal loc39.nK, eax

.L87:
    mov     cl, ATTR_PROMPT_TEXT
    call    TConsole_SetAttr

    cmp     [esp + loc39.bIPOOP], 0
    je      .L88
    ;
    ; Print battle computer recommendation
    ;
    mov     al, '('
    call    TConsole_PutChar

    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    fmulp
    fld     [glb_dbl_1dot01]
    faddp
    fstp    [esp + loc39.dbl_PART2]

    mov     esi, glb_dbl_0dot9
    mcLoadLocal ebx, loc39.pTrekData
    mcLoadLocal ecx, loc39.nK
    dec     ecx
    imul    ecx, 10
    lea     edi, [ebx + ecx + TREKDATA.KDIST]
    call    TCommon_X_exp_Y
    fld     st
    faddp
    fstp    [esp + loc39.dbl_PART1]

    fld     [ebx + ecx + TREKDATA.KPOWER]
    fabs

    fld     [esp + loc39.dbl_PART1]
    fdivp

    fld     [esp + loc39.dbl_PART2]
    fmulp

    call    TCommon_FP_Truncate
    inc     eax
    call    TConsole_CramInt

    mov     ecx, 300
    call    TConsole_Cram

.L88:
    mov     ecx, 294
    call    TConsole_Cram

    mcLoadLocal ecx, loc39.nK
    mcLoadLocal ebx, loc39.pTrekData
    dec     ecx
    lea     esi, [ebx + TREKDATA.KX]
    lea     edi, [ebx + TREKDATA.KY]
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    call    TConsole_CramEnemyAtEx

    cmp     [esp + loc39.bIDOIT], 1
    je      .L89
    ;
    ; Phasers can't actually be fired - loop back for
    ; next computer readout or terminate if that's all.
    ;
    call    TConsole_ScrollUp

    inc     [esp + loc39.nK]
    mov     ecx, [esp + loc39.nK]
    cmp     ecx, [esp + loc39.nEnemies]
    jbe     .L87
    jmp     .L15

.L89:
    mov     ecx, 301
    call    TGame_Prompt

.L90:
    cmp     [esp + loc39.nK], 1
    jne     .scan_hit

    fldz
    fstp    [esp + loc39.dbl_POW]

.scan_hit:
    call    TCmdBuf_Scan

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    mcLoadLocal ecx, loc39.nK
    dec     ecx
    imul    ecx, 10
    fld     st
    fstp    [esp + ecx + loc39.arr_HITS]
    fstp    [esp + loc39.dbl_HITS_I_]

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    je      .L95
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L91

    cmp     [esp + loc39.nK], 1
    je      .L84
    jmp     .L87

.L91:
    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jnc     .L84

    inc     [esp + loc39.bNO]
    jmp     .L90

.L95:
    fldz
    fld     [esp + loc39.dbl_HITS_I_]
    mc_CMP_ST0_ST1
    jc      .L15

    fld     [esp + loc39.dbl_POW]
    fld     [esp + loc39.dbl_HITS_I_]
    faddp
    fstp    [esp + loc39.dbl_POW]
    ;
    ; If total amount is too much, then
    ; inform and start over.
    ;
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc39.dbl_POW]
    mc_CMP_ST0_ST1
    jc      .L97

    call    TConsole_SetGameMsgAttr
    mov     ecx, 295
    call    TConsole_Prout
    jmp     .L84

.L97:
    inc     [esp + loc39.nK]
    mov     ecx, [esp + loc39.nK]
    cmp     ecx, [esp + loc39.nEnemies]
    jbe     .L90
    ;
    ; If total requested is 0.0 - abort phasers.
    ;
    fldz
    fld     [esp + loc39.dbl_POW]
    mc_CMP_ST0_ST1
    jz      .L15

    call    TCmdBuf_Scan

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    jne     .L9701

    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    adc     [esp + loc39.bNO], 0

.L9701:
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc39.dbl_POW]
    fsubp
    fstp    [esi + TREKDATA.ENERGY]

    cmp     [esp + loc39.bIFAST], 0
    je      .L98

    mov     eax, .L98
    mcStoreLocal loc39.pIWHERE, eax
    jmp     .L42

.L98:
    mcLoadLocalRef esi, loc39.arr_HITS
    call    TPhasers_HitThem

    mcLoadLocal edi, loc39.pTrekData
    inc     [edi + TREKDATA.IDIDIT]

.L200:
    ;
    ; Raising shields after phaser fire.
    ;
    mcLoadLocal esi, loc39.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    cmp     [esp + loc39.bIFAST], 0
    je      .L210

    call    TConsole_ScrollUp
    cmp     [esp + loc39.bNO], 0
    jne     .L202

    fld     [glb_dbl_0dot99]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L205

    call    TConsole_SetCrewMsgAttr
    mov     ecx, 296
    call    TConsole_Prout
    mov     ecx, 297
    call    TConsole_Prout

.L202:
    mcLoadLocal edi, loc39.pTrekData
    mov     [edi + TREKDATA.SHLDUP], 0
    jmp     .L210

.L205:
    call    TConsole_SetGameMsgAttr
    mov     ecx, 228
    call    TConsole_Prout

.L210:
    ;
    ; Check for phaser overheat
    ;
    fld     [glb_dbl_1500]
    fld     [esp + loc39.dbl_POW]
    mc_CMP_ST0_ST1
    jc      .done
    jz      .done

    fld     [esp + loc39.dbl_POW]
    fld     [glb_dbl_1500]
    fsubp
    fld     [glb_dbl_0dot00038]
    fmulp
    fstp    [esp + loc39.dbl_TEMP]

    call    TRandom_Ranf
    fld     [esp + loc39.dbl_TEMP]
    mc_CMP_ST0_ST1
    jc      .done

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 298
    call    TConsole_Prout

    call    TRandom_Ranf
    fld1
    faddp
    fld     [esp + loc39.dbl_TEMP]
    fld1
    faddp
    fmulp
    mcLoadLocal esi, loc39.pTrekData
    fld     [esi + TREKDATA.DAMFAC]
    fmulp
    mov     cl, DEV_PHASERS
    call    TArray_SetDblDamage

.done:
    mcEndLocals loc39.size
    ret

; --- EOF ---
