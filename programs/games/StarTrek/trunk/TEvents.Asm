; --------------------------------------------------------------------------
; FILE: TEvents.Asm
; DATE: October 18, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   EDI = address of INT32 value
; --------------------------------------------------------------------------
; This helper function will limit the offset stored at EDI to one quadrant.
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_Helper1:
    cmp     dword [edi], 1
    jg      .set_to_one
    cmp     dword [edi], -1
    jl      .set_to_neg_one
    ret

.set_to_one:
    mcLoad1 eax
    stosd
    ret

.set_to_neg_one:
    mcLoadNeg1 eax
    stosd
    ret

; --------------------------------------------------------------------------
; SCOM
; --------------------------------------------------------------------------
virtual at 0
loc33:
    .pTrekData PVOID ?
    .pIWHERE PVOID ?
    .arr_BASEQX BYTES 8
    .arr_BASEQY BYTES 8
    .arr_BDIST DOUBLES 8
    .nDistance INT32 ?
    .nIFINDIT INT32 ?
    .nIWHICHB INT32 ?
    .nI INT32 ?
    .nREMBASE INT32 ?
    .bSwapped BOOL ?
    .iDELTX INT32 ?
    .iDELTY INT32 ?
    .iQX INT32 ?
    .iQY INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_MoveSuperCommander:
    mcBeginLocals loc33.size

    mcLoadGameDataPtr ebx
    mcStoreLocal loc33.pTrekData, ebx

    cmp     [ebx + TREKDATA.REMBASE], 0
    jle     .L60
    ;
    ; Fill local arrays with base coordinates
    ;
    lea     esi, [ebx + TREKDATA.BASEQX]
    lea     edi, [esp + loc33.arr_BASEQX]
    mcZeroBits ecx
    mov     cl, 5
    rep     movsb

    lea     esi, [ebx + TREKDATA.BASEQY]
    lea     edi, [esp + loc33.arr_BASEQY]
    mov     cl, 5
    rep     movsb
    ;
    ; Calculate distances to super-commander quadrant
    ;
    mcLoadMember cl, TREKDATA.REMBASE
    mcStoreLocal loc33.nREMBASE, ecx
    lea     esi, [esp + loc33.arr_BASEQX]
    lea     edi, [esp + loc33.arr_BASEQY]
    lea     edx, [esp + loc33.arr_BDIST]

.next_base:
    mov     al, [esi]
    sub     al, [ebx + TREKDATA.ISX]
    movsx   eax, al
    imul    eax, eax
    mcStoreLocal loc33.nDistance, eax

    mov     al, [edi]
    sub     al, [ebx + TREKDATA.ISY]
    movsx   eax, al
    imul    eax, eax
    add     eax, [esp + loc33.nDistance]

    call    TCommon_FPU_Load_EAX
    fsqrt
    fstp    tbyte [edx]

    inc     esi
    inc     edi
    add     edx, 10
    loop    .next_base
    ;
    ; Now, sort all three local base arrays by distance
    ;
    cmp     [ebx + TREKDATA.REMBASE], 1
    jbe     .L4

.bubble_sort:
    mcZeroBits eax
    mcStoreLocal loc33.bSwapped, eax

    movzx   ecx, [ebx + TREKDATA.REMBASE]
    dec     ecx

    lea     esi, [esp + loc33.arr_BASEQX]
    lea     edi, [esp + loc33.arr_BASEQY]
    lea     edx, [esp + loc33.arr_BDIST]

.sort_pair:
    fld     tbyte [edx + 10]
    fld     tbyte [edx]
    mc_CMP_ST0_ST1
    jc      .next_pair
    jz      .next_pair

    inc     [esp + loc33.bSwapped]

    mov     al, [esi]
    xchg    al, [esi + 1]
    mov     [esi], al

    mov     al, [edi]
    xchg    al, [edi + 1]
    mov     [edi], al

    fld     tbyte [edx + 10]
    fld     tbyte [edx]
    fstp    tbyte [edx + 10]
    fstp    tbyte [edx]

.next_pair:
    inc     esi
    inc     edi
    add     edx, 10
    loop    .sort_pair

    cmp     [esp + loc33.bSwapped], 0
    jne     .bubble_sort

.L4:
    ;
    ; Look for nearest base without a COMMANDER, no Enterprise,
    ; not too many klingons and not under attack.
    ;
    mcZeroBits eax
    mcStoreLocal loc33.nIFINDIT, eax
    mcStoreLocal loc33.nIWHICHB, eax

    inc     eax
    mcStoreLocal loc33.nI, eax

.load_base_quad:
    mcLoadLocalRef esi, loc33.arr_BASEQX
    mcLoadLocalRef edi, loc33.arr_BASEQY
    mcLoadLocal ecx, loc33.nI
    dec     ecx
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]

    mcLoadLocal ebx, loc33.pTrekData
    cmp     al, [ebx + TREKDATA.QUADX]
    jne     .check_battle_quad
    cmp     dl, [ebx + TREKDATA.QUADY]
    je      .check_next_base

.check_battle_quad:
    cmp     al, [ebx + TREKDATA.BATX]
    jne     .check_klingon_count
    cmp     dl, [ebx + TREKDATA.BATY]
    je      .check_next_base

.check_klingon_count:
    call    TArray_GetGalaxyValue
    cmp     ecx, 899
    ja      .check_next_base

    cmp     [ebx + TREKDATA.REMCOM], 0
    jle     .L10
    ;
    ; Check AL,DL quadrant against array of commanders
    ;
    mcLoadMemberRef esi, TREKDATA.CX
    mcLoadMemberRef edi, TREKDATA.CY
    movzx   ecx, [ebx + TREKDATA.REMCOM]

.check_this_commander:
    cmp     al, [esi]
    jne     .next_commander

    cmp     dl, [edi]
    je      .quad_with_commander

.next_commander:
    inc     esi
    inc     edi
    loop    .check_this_commander

    mcLoadLocal eax, loc33.nI
    mcStoreLocal loc33.nIWHICHB, eax
    jmp     .L10

.quad_with_commander:
    cmp     [esp + loc33.nIFINDIT], 2
    je      .check_next_base

    mcLoad8bitsToReg32 eax, 2
    mcStoreLocal loc33.nIFINDIT, eax

    mcLoadLocal eax, loc33.nI
    mcStoreLocal loc33.nIWHICHB, eax

.check_next_base:
    inc     [esp + loc33.nI]
    mcLoadLocal ecx, loc33.nI
    cmp     ecx, [esp + loc33.nREMBASE]
    jbe     .load_base_quad

    cmp     [esp + loc33.nIFINDIT], 0
    je      .done

.L10:
    ;
    ; Base found! (1-based index is stored in loc33.nIWHICHB)
    ;
    mcLoadLocal ebx, loc33.pTrekData
    mcLoadLocal ecx, loc33.nIWHICHB
    dec     ecx
    mcLoadLocalRef esi, loc33.arr_BASEQX
    mcLoadLocalRef edi, loc33.arr_BASEQY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    sub     al, [ebx + TREKDATA.ISX]
    sub     dl, [ebx + TREKDATA.ISY]
    movsx   eax, al
    movsx   edx, dl
    mcStoreLocal loc33.iDELTX, eax
    mcStoreLocal loc33.iDELTY, edx

    mcLoadLocalRef edi, loc33.iDELTX
    call    TEvents_Helper1

    mcLoadLocalRef edi, loc33.iDELTY
    call    TEvents_Helper1
    ;
    ; Attempt first to move in both directions
    ;
    movzx   eax, [ebx + TREKDATA.ISX]
    movzx   edx, [ebx + TREKDATA.ISY]
    add     eax, [esp + loc33.iDELTX]
    add     edx, [esp + loc33.iDELTY]
    mcStoreLocal loc33.iQX, eax
    mcStoreLocal loc33.iQY, edx

    mov     eax, .L23
    mcStoreLocal loc33.pIWHERE, eax
    jmp     .L15

.goto_IWHERE:
    jmp     [esp + loc33.pIWHERE]

.L15:
    ;
    ; Make checks on possible destination quadrant
    ;
    mcLoadLocal eax, loc33.iQX
    mcLoadLocal edx, loc33.iQY

    cmp     edx, 1
    jl      .goto_IWHERE
    cmp     edx, 8
    jg      .goto_IWHERE
    cmp     eax, 1
    jl      .goto_IWHERE
    cmp     eax, 8
    jg      .goto_IWHERE

    mcLoadLocal ebx, loc33.pTrekData
    cmp     al, [ebx + TREKDATA.QUADX]
    jne     .check_klingons
    cmp     dl, [ebx + TREKDATA.QUADY]
    je      .goto_IWHERE

.check_klingons:
    call    TArray_GetGalaxyValue
    cmp     ecx, 899
    ja      .goto_IWHERE
    ;
    ; Go ahead and move SUPER-COMMANDER!
    ;
    mov     al, [ebx + TREKDATA.ISX]
    mov     dl, [ebx + TREKDATA.ISY]
    mov     esi, ebx
    call    TArray_GalaxyPtr
    sub     dword [ebx], 100

    mcLoadLocal eax, loc33.iQX
    mcLoadLocal edx, loc33.iQY
    mov     [esi + TREKDATA.ISX], al
    mov     [esi + TREKDATA.ISY], dl
    call    TArray_GalaxyPtr
    add     dword [ebx], 100

    cmp     [esi + TREKDATA.ISCATE], 0
    je      .L40
    ;
    ; SUPER-COMMANDER left current quadrant
    ;
    mcZeroBits eax
    mov     [esi + TREKDATA.ISCATE], al
    mov     [esi + TREKDATA.ISATB], al
    mov     [esi + TREKDATA.ISHERE], al
    mov     [esi + TREKDATA.IENTESC], al

    fld     [glb_dbl_1E38]
    fstp    [esi + TREKDATA.FUTURE7]
    ;
    ; Find S.C. index in KX,KY arrays
    ;
    lea     edi, [esi + TREKDATA.KX]
    lea     ebx, [esi + TREKDATA.KY]
    movzx   ecx, [esi + TREKDATA.NENHERE]

    mcZeroBits eax
    mcStoreLocal loc33.nI, eax

.find_scom_in_quad:
    inc     [esp + loc33.nI]
    mov     al, [edi]
    mov     dl, [ebx]

    push    ebx
    call    TArray_QuadPtr
    mov     eax, ebx
    pop     ebx

    cmp     byte [eax], CHAR_SCOM
    je      .L22

    inc     edi
    inc     ebx
    loop    .find_scom_in_quad
    jmp     .L40

.L22:
    mov     byte [eax], CHAR_COSMOS
    mcLoadLocal ecx, loc33.nI
    call    TArray_Leave
    call    TCommon_SortKlingons
    jmp     .L40

.L23:
    ;
    ; Try some other maneuvers
    ;
    cmp     [esp + loc33.iDELTX], 0
    je      .L30
    cmp     [esp + loc33.iDELTY], 0
    je      .L30
    ;
    ; Try moving just in X direction
    ;
    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISY]
    mcStoreLocal loc33.iQY, eax

    mov     edx, .L25
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L25:
    ;
    ; Then try moving just in Y direction
    ;
    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISX]
    movzx   edx, [ebx + TREKDATA.ISY]
    add     edx, [esp + loc33.iDELTY]
    mcStoreLocal loc33.iQX, eax
    mcStoreLocal loc33.iQY, edx

    mov     edx, .L300
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L30:
    ;
    ; Attempt angle move
    ;
    cmp     [esp + loc33.iDELTX], 0
    jne     .L35

    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISX]
    inc     eax
    mcStoreLocal loc33.iQX, eax

    mov     edx, .L32
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L32:
    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISX]
    dec     eax
    mcStoreLocal loc33.iQX, eax

    mov     edx, .L300
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L35:
    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISY]
    inc     eax
    mcStoreLocal loc33.iQY, eax

    mov     edx, .L36
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L36:
    mcLoadLocal ebx, loc33.pTrekData
    movzx   eax, [ebx + TREKDATA.ISY]
    dec     eax
    mcStoreLocal loc33.iQY, eax

    mov     edx, .L300
    mcStoreLocal loc33.pIWHERE, edx
    jmp     .L15

.L40:
    ;
    ; SUPER-COMMANDER has moved - check/report situation
    ;
    mcLoadLocal ebx, loc33.pTrekData
    movzx   ecx, [ebx + TREKDATA.INPLAN]
    mcLoadMemberRef esi, TREKDATA.PLNETS
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY

.check_planet:
    cmp     al, [esi + TPlanet.planet_X]
    jne     .next_planet
    cmp     dl, [esi + TPlanet.planet_Y]
    jne     .next_planet

    cmp     [esi + TPlanet.planet_DILITHIUM], 1
    jne     .L45
    jmp     .scom_destroys_planet

.next_planet:
    add     esi, TPlanet.size
    loop    .check_planet
    jmp     .L45

.scom_destroys_planet:
    push    eax
    mov     edi, esi
    mov     cl, TPlanet.size
    mcZeroBits eax
    rep     stosb
    pop     eax

    call    TArray_NewStufPtr
    dec     byte [ebx]

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jc      .L45

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 212
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 213
    call    TConsole_Cram

    mcLoadLocal ebx, loc33.pTrekData
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY
    mov     cl, 1
    call    TConsole_CramLoc

    mcLoad8bitsToReg32 ecx, 214
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 215
    call    TConsole_Prout

.L45:
    ;
    ; Check for a base
    ;
    mcLoadLocal ebx, loc33.pTrekData
    cmp     [ebx + TREKDATA.REMBASE], 0
    jle     .L60

    movzx   ecx, [ebx + TREKDATA.REMBASE]
    mcLoadMemberRef esi, TREKDATA.BASEQX
    mcLoadMemberRef edi, TREKDATA.BASEQY
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY

.verify_scom_at_base:
    cmp     al, [esi]
    jne     .verify_next_base
    cmp     dl, [edi]
    jne     .verify_next_base

    cmp     al, [ebx + TREKDATA.BATX]
    jne     .L80
    cmp     dl, [ebx + TREKDATA.BATY]
    jne     .L80

.verify_next_base:
    inc     esi
    inc     edi
    loop    .verify_scom_at_base
    ;
    ; Check for intelligence report
    ;
    call    TRandom_Ranf
    fld     [glb_dbl_0dot2]
    mc_CMP_ST0_ST1
    jc      .done

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jc      .done

    mcLoadLocal ebx, loc33.pTrekData
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY
    call    TArray_GetStarChartValue
    cmp     ecx, 0
    jg      .done

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 216
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 217
    call    TConsole_Cram

    mcLoadLocal ebx, loc33.pTrekData
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY
    mov     cl, 1
    call    TConsole_CramLoc

    mcLoad8bitsToReg32 ecx, 154
    call    TConsole_Prout
    jmp     .done

.L60:
    mcLoadLocal edi, loc33.pTrekData
    fld     [glb_dbl_1E38]
    fstp    [edi + TREKDATA.FUTURE6]
    jmp     .done

.L80:
    ;
    ; Attack a base
    ;
    mcLoadLocal ebx, loc33.pTrekData
    mov     [ebx + TREKDATA.ISATB], 1

    call    TRandom_Ranf
    fld     st
    faddp
    fld1
    faddp
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE7]

    cmp     [ebx + TREKDATA.BATX], 0
    je      .cry_sos
    ;
    ; FUTURE7 += FUTURE5-DATE
    ;
    fld     [ebx + TREKDATA.FUTURE5]
    fld     [ebx + TREKDATA.DATE]
    fsubp
    fld     [ebx + TREKDATA.FUTURE7]
    faddp
    fstp    [ebx + TREKDATA.FUTURE7]

.cry_sos:
    mov     al, CHAR_SCOM
    call    TEvents_SOS

.L300:
.done:
    mcEndLocals loc33.size
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X quadrant
;   DL = Y quadrant
;   ESI = points to TREKDATA object
; Output:
;   CF=1 - base was removed
; --------------------------------------------------------------------------
; Find the base in BASEQX,BASEQY arrays and overwrite its coordinates with
; the last entry from these arrays. Do not decrement REMBASE! It is done
; outside of this function.
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_RemoveThisBase:
    lea     ebx, [esi + TREKDATA.BASEQX]
    lea     edi, [esi + TREKDATA.BASEQY]
    movzx   ecx, [esi + TREKDATA.REMBASE]

.check_base_coords:
    cmp     al, [ebx]
    jne     .next_base

    cmp     dl, [edi]
    je      .base_found

.next_base:
    inc     ebx
    inc     edi
    loop    .check_base_coords
    clc
    ret

.base_found:
    push    ebx
    push    edi

    lea     ecx, [esi + TREKDATA.BASEQX]
    lea     edx, [esi + TREKDATA.BASEQY]
    movzx   eax, [esi + TREKDATA.REMBASE]
    dec     eax

    mov     bl, [edx + eax]
    pop     esi
    mov     [esi], bl

    mov     bl, [ecx + eax]
    pop     esi
    mov     [esi], bl
    stc
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = points to TREKDATA object
; Output:
;   CF=1 if quadrant BATX,BATY contains a COMMANDER
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_VerifyBattleQuad:
    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]

    lea     ebx, [esi + TREKDATA.CX]
    lea     edi, [esi + TREKDATA.CY]

    movzx   ecx, [esi + TREKDATA.REMCOM]

.check_commander:
    cmp     al, [ebx]
    jne     .next_commander

    cmp     dl, [edi]
    je      .verified

.next_commander:
    inc     ebx
    inc     edi
    loop    .check_commander

    clc
    ret

.verified:
    stc
    ret

; --------------------------------------------------------------------------
; SOS
; --------------------------------------------------------------------------
; Input:
;   AL = which commander is attacking the base
; --------------------------------------------------------------------------
virtual at 0
loc34:
    .pTrekData PVOID ?
    .nESC BYTE ?
    .nIX BYTE ?
    .nIY BYTE ?
    .bRadioDamaged BYTE ?
    .dbl_DDAY DOUBLE ?
    .dbl_reserved DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_SOS:
    mcBeginLocals loc34.size
    mcStoreLocal loc34.nESC, al

@@:
    mcLoadGameDataPtr ebx
    mcStoreLocal loc34.pTrekData, ebx

    mcZeroBits eax
    mcStoreLocal loc34.bRadioDamaged, al

    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    adc     [esp + loc34.bRadioDamaged], 0

    cmp     [esp + loc34.nESC], CHAR_SCOM
    je      .L20

    mov     [ebx + TREKDATA.ICSOS], 0
    cmp     [esp + loc34.bRadioDamaged], 0
    jne     .done

    inc     [ebx + TREKDATA.ICSOS]
    mcLoadMember al, TREKDATA.BATX
    mcLoadMember dl, TREKDATA.BATY
    mcStoreLocal loc34.nIX, al
    mcStoreLocal loc34.nIY, dl

    fld     [ebx + TREKDATA.FUTURE5]
    fstp    [esp + loc34.dbl_DDAY]
    jmp     .L30

.L20:
    mov     [ebx + TREKDATA.ISSOS], 0
    cmp     [esp + loc34.bRadioDamaged], 0
    jne     .done

    inc     [ebx + TREKDATA.ISSOS]
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY
    mcStoreLocal loc34.nIX, al
    mcStoreLocal loc34.nIY, dl

    fld     [ebx + TREKDATA.FUTURE7]
    fstp    [esp + loc34.dbl_DDAY]

.L30:
    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr

    mcLoad8bitsToReg32 ecx, 218
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal al, loc34.nIX
    mcLoadLocal dl, loc34.nIY
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 219
    call    TConsole_Cram

    mcLoadLocal al, loc34.nESC
    call    TConsole_CramEnemy

    mcLoad8bitsToReg32 ecx, 220
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 221
    call    TConsole_Cram

    mov     cl, 1
    fld     [esp + loc34.dbl_DDAY]
    call    TConsole_CramFloat

    mcLoad8bitsToReg32 ecx, 222
    call    TConsole_Prout

    mcLoadLocal esi, loc34.pTrekData
    cmp     [esi + TREKDATA.RESTING], 0
    je      .done

    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 223
    call    TConsole_Prout

    mcLoad8bitsToReg32 ecx, 224
    call    TGame_JA
    jnc     .done

    mcLoadLocal edi, loc34.pTrekData
    dec     [edi + TREKDATA.RESTING]

.done:
    mcEndLocals loc34.size
    ret

; --------------------------------------------------------------------------
; Output:
;   ECX = 1-based index of a COMMANDER or zero
; --------------------------------------------------------------------------
; This function finds a quadrant where COMMANDER+BASE, but
; no Enterprise and no SUPER COMMANDER.
; --------------------------------------------------------------------------
virtual at 0
loc32:
    ;
    ; These are not changing
    ;
    .pBASEQX PBYTE ?
    .pBASEQY PBYTE ?
    ;
    ; These are incrementing at the end of outer loop
    ;
    .pCX PBYTE ?
    .pCY PBYTE ?
    .nCommanderIndex INDEX ?
    ;
    ; Loop limits
    ;
    .nCommanders COUNT ?
    .nBases COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_FindCmdrAtBase:
    mcBeginLocals loc32.size
    mcLoadGameDataPtr ebx

    movzx   ecx, [ebx + TREKDATA.REMBASE]
    movzx   edx, [ebx + TREKDATA.REMCOM]
    mcStoreLocal loc32.nBases, ecx
    mcStoreLocal loc32.nCommanders, edx

    mcLoad1 eax
    mcStoreLocal loc32.nCommanderIndex, eax

    mcLoadMemberRef esi, TREKDATA.CX
    mcLoadMemberRef edi, TREKDATA.CY
    mcStoreLocal loc32.pCX, esi
    mcStoreLocal loc32.pCY, edi

    mcLoadMemberRef esi, TREKDATA.BASEQX
    mcLoadMemberRef edi, TREKDATA.BASEQY
    mcStoreLocal loc32.pBASEQX, esi
    mcStoreLocal loc32.pBASEQY, edi

.load_commander_quad:
    mcLoadLocal esi, loc32.pCX
    mcLoadLocal edi, loc32.pCY
    mov     al, [esi]
    mov     dl, [edi]

    cmp     al, [ebx + TREKDATA.ISX]
    jne     .check_against_ship
    cmp     dl, [ebx + TREKDATA.ISY]
    je      .next_commander

.check_against_ship:
    cmp     al, [ebx + TREKDATA.QUADX]
    jne     .check_against_all_bases
    cmp     dl, [ebx + TREKDATA.QUADY]
    je      .next_commander

.check_against_all_bases:
    mcLoadLocal esi, loc32.pBASEQX
    mcLoadLocal edi, loc32.pBASEQY
    mcLoadLocal ecx, loc32.nBases

.check_against_base:
    cmp     al, [esi]
    jne     .next_base

    cmp     dl, [edi]
    je      .commander_located

.next_base:
    inc     esi
    inc     edi
    loop    .check_against_base

.next_commander:
    inc     [esp + loc32.nCommanderIndex]
    mcLoadLocal ecx, loc32.nCommanderIndex
    cmp     ecx, [esp + loc32.nCommanders]
    ja      .ret_zero

    inc     [esp + loc32.pCX]
    inc     [esp + loc32.pCY]
    jmp     .load_commander_quad

.commander_located:
    mcLoadLocal ecx, loc32.nCommanderIndex
    jmp     .done

.ret_zero:
    mcZeroBits ecx

.done:
    mcEndLocals loc32.size
    ret

; --------------------------------------------------------------------------
; EVENTS
; --------------------------------------------------------------------------
virtual at 0
loc31:
    .pTrekData PVOID ?
    .bICTBEAM BOOL ?
    .bISTRACT BOOL ?
    .nLINE INDEX ?
    .nL INDEX ?
    .nI INDEX ?
    .nYANK INT32 ?
    .dbl_DATEMIN DOUBLE ?
    .dbl_XTIME DOUBLE ?
    .dbl_REPAIR DOUBLE ?
    .dbl_Reserved DOUBLE ?
    .iXHOLD BYTE ?
    .iYHOLD BYTE ?
    .nDamage_3_and_4 BYTE ?
    .byte_Reserved BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_Main:
    mcBeginLocals loc31.size

    mcLoadGameDataPtr edx
    mcStoreLocal loc31.pTrekData, edx

    mcZeroBits eax
    mcStoreLocal loc31.bICTBEAM, eax
    mcStoreLocal loc31.bISTRACT, eax

.L10:
    mcZeroBits eax
    mcStoreLocal loc31.nLINE, eax

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done
    ;
    ; Pick earliest event
    ;
    fld     [esi + TREKDATA.DATE]
    fld     [esi + TREKDATA.TIME]
    faddp
    fstp    [esp + loc31.dbl_DATEMIN]

    mcLoad8bitsToReg32 ecx, 7
    lea     edi, [esi + TREKDATA.FUTURE1]

    mcLoad1 eax
    mcStoreLocal loc31.nL, eax

.check_event:
    fld     tbyte [edi]
    fld     [esp + loc31.dbl_DATEMIN]
    mc_CMP_ST0_ST1
    jc      .L20

    mcLoadLocal eax, loc31.nL
    mcStoreLocal loc31.nLINE, eax

    fld     tbyte [edi]
    fstp    [esp + loc31.dbl_DATEMIN]

.L20:
    add     edi, 10
    inc     [esp + loc31.nL]
    loop    .check_event

    fld     [esp + loc31.dbl_DATEMIN]
    fld     [esi + TREKDATA.DATE]
    fsubp
    fstp    [esp + loc31.dbl_XTIME]

    fld     [esp + loc31.dbl_DATEMIN]
    fstp    [esi + TREKDATA.DATE]
    ;
    ; Deplete Federation Resources
    ;
    movzx   eax, [esi + TREKDATA.REMCOM]
    movzx   edx, [esi + TREKDATA.REMKL]
    shl     eax, 2
    add     eax, edx
    call    TCommon_FPU_Load_EAX
    fld     [esp + loc31.dbl_XTIME]
    fmulp
    fld     [esi + TREKDATA.REMRES]
    fsubrp
    fld     st
    fstp    [esi + TREKDATA.REMRES]

    fild    [glb_FPU_Int32]
    fdivp
    fld     st
    fstp    [esi + TREKDATA.REMTIME]

    fldz
    mc_CMP_ST0_ST1
    jc      .L30

    mov     al, 2

.finished:
    call    TFinish_Main
    jmp     .done

.L30:
    ;
    ; Verify Life Support is OK
    ;
    mov     cl, DEV_LIFE_SUPPORT
    call    TArray_IsDamaged
    jnc     .L50

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L50

    fld     [esi + TREKDATA.LSUPRES]
    fld     [esp + loc31.dbl_XTIME]
    mc_CMP_ST0_ST1
    jc      .L40
    jz      .L40

    fld     [esi + TREKDATA.LSUPRES]
    mov     cl, DEV_LIFE_SUPPORT
    call    TArray_GetDblDamage
    mc_CMP_ST0_ST1
    jc      .L40
    jz      .L40

    mov     al, 3
    jmp     .finished

.L40:
    mcLoadLocal esi, loc31.pTrekData
    fld     [esi + TREKDATA.LSUPRES]
    fld     [esp + loc31.dbl_XTIME]
    fsubp
    fstp    [esi + TREKDATA.LSUPRES]

    mov     cl, DEV_LIFE_SUPPORT
    call    TArray_GetDblDamage
    fld     [esp + loc31.dbl_XTIME]
    mc_CMP_ST0_ST1
    jc      .L50

    mcLoadLocal esi, loc31.pTrekData
    fld     [esi + TREKDATA.INLSR]
    fstp    [esi + TREKDATA.LSUPRES]

.L50:
    ;
    ; Fix devices...
    ;
    fld     [esp + loc31.dbl_XTIME]
    fstp    [esp + loc31.dbl_REPAIR]

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .fix_devices

    fld     [esp + loc31.dbl_XTIME]
    fld     [glb_dbl_DOCKFAC]
    fdivp
    fstp    [esp + loc31.dbl_REPAIR]

.fix_devices:
    lea     ebx, [esi + TREKDATA.DAMAGE]
    mcLoad8bitsToReg32 ecx, 13

.fix_it:
    fldz
    fld     tbyte [ebx]
    mc_CMP_ST0_ST1
    jz      .L60

    fld     tbyte [ebx]
    fld     [esp + loc31.dbl_REPAIR]
    fsubp
    fstp    tbyte [ebx]

    fldz
    fld     tbyte [ebx]
    mc_CMP_ST0_ST1
    jnc     .L60

    fldz
    fstp    tbyte [ebx]

.L60:
    add     ebx, 10
    loop    .fix_it
    ;
    ; Jump to event IF (LINE != 0)
    ;
    fld     [esi + TREKDATA.TIME]
    fld     [esp + loc31.dbl_XTIME]
    fsubp
    fstp    [esi + TREKDATA.TIME]

    cmp     [esp + loc31.nLINE], 0
    je      .L5000

    mcLoadLocal eax, loc31.nLINE
    mcOnRegEqu eax, 1, .L100
    mcOnRegEqu eax, 2, .L200
    mcOnRegEqu eax, 3, .L300
    mcOnRegEqu eax, 4, .L400
    mcOnRegEqu eax, 5, .L500
    mcOnRegEqu eax, 6, .L600
    mcOnRegEqu eax, 7, .L700

    ;int     3
    jmp     .done                   ; Something is broken in the code!

.L100:
    ;
    ; Supernova!
    ;
    mcZeroBits eax
    mcZeroBits edx
    call    TNova_SuperNova

    mcLoadLocal ebx, loc31.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE1]

    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    je      .done
    jmp     .L10

.L200:
    ;
    ; Tractor Beam!
    ;
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    je      .L220

    cmp     [esp + loc31.bISTRACT], 0
    jne     .L210

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L210
    ;
    ; Select a Commander who will beam
    ;
    movzx   edx, [esi + TREKDATA.REMCOM]
    call    TRandom_IRan
    mcStoreLocal loc31.nI, eax

    mcLoadLocal esi, loc31.pTrekData
    movzx   ecx, [esi + TREKDATA.QUADX]
    movzx   edx, [esi + TREKDATA.QUADY]

    movzx   ebx, [esi + eax + TREKDATA.CX]
    sub     ebx, ecx
    imul    ebx, ebx
    mcStoreLocal loc31.nYANK, ebx

    movzx   ebx, [esi + eax + TREKDATA.CY]
    sub     ebx, edx
    imul    ebx, ebx
    add     [esp + loc31.nYANK], ebx

    movzx   eax, [esi + TREKDATA.JUSTIN]
    add     eax, [esp + loc31.nYANK]
    mcOnRegZero eax, .L210

    cmp     [esp + loc31.bISTRACT], 0
    je      .L201

.L20010:
    mcLoadLocal esi, loc31.pTrekData
    movzx   ecx, [esi + TREKDATA.QUADX]
    movzx   edx, [esi + TREKDATA.QUADY]

    movzx   ebx, [esi + TREKDATA.ISX]
    sub     ebx, ecx
    imul    ebx, ebx
    mcStoreLocal loc31.nYANK, ebx

    movzx   ebx, [esi + TREKDATA.ISY]
    sub     ebx, edx
    imul    ebx, ebx
    add     [esp + loc31.nYANK], ebx

.L201:
    mcLoadLocal eax, loc31.nYANK
    call    TCommon_FPU_Load_EAX
    fsqrt
    fld     [glb_dbl_0dot1777777777]
    fmulp
    mcLoadLocal edi, loc31.pTrekData
    fstp    [edi + TREKDATA.TIME]

    inc     [esp + loc31.bICTBEAM]

    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip
    mcLoad8bitsToReg32 ecx, 191
    call    TConsole_Prout

    mov     al, 14
    mov     cl, 1
    call    TNova_AutomaticOverride

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done

    cmp     [esi + TREKDATA.ICRAFT], 1
    jne     .L203

    mov     al, 18
    jmp     .finished

.L203:
    cmp     [esi + TREKDATA.ISCRAFT], 0
    jne     .L204

    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 192
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mov     cl, DEV_SHUTTLE_CRAFT
    fld     [glb_dbl_Ten]
    fchs
    call    TArray_SetDblDamage

    mcLoadLocal edi, loc31.pTrekData
    mov     [edi + TREKDATA.ISCRAFT], -1

.L204:
    cmp     [esp + loc31.bISTRACT], 0
    je      .L205

    mcLoadLocal ebx, loc31.pTrekData
    mcLoadMember al, TREKDATA.ISX
    mcLoadMember dl, TREKDATA.ISY
    mcStoreMember TREKDATA.QUADX, al
    mcStoreMember TREKDATA.QUADY, dl
    jmp     .L206

.L205:
    mcLoadLocal ebx, loc31.pTrekData
    mcLoadLocal ecx, loc31.nI
    mov     al, [ebx + ecx + TREKDATA.CX]
    mov     dl, [ebx + ecx + TREKDATA.CY]
    mcStoreMember TREKDATA.QUADX, al
    mcStoreMember TREKDATA.QUADY, dl

.L206:
    call    TRandom_IRan10
    mcLoadLocal ebx, loc31.pTrekData
    mcStoreMember TREKDATA.SECTX, al
    mcStoreMember TREKDATA.SECTY, dl

    mcLoad8bitsToReg32 ecx, 194
    call    TConsole_Cram

    mov     cl, 1
    mcLoadLocal ebx, loc31.pTrekData
    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY
    call    TConsole_CramLoc

    mcLoad8bitsToReg32 ecx, 53
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal ebx, loc31.pTrekData
    mcLoadMember al, TREKDATA.SECTX
    mcLoadMember dl, TREKDATA.SECTY
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.RESTING], 0
    je      .check_shields

    dec     [esi + TREKDATA.RESTING]

    mcLoad8bitsToReg32 ecx, 195
    call    TConsole_ProutGameMsg

.check_shields:
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     .L208

    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jc      .no_shields

    mcLoadLocal esi, loc31.pTrekData
    fld     [esi + TREKDATA.SHLD]
    fldz
    mc_CMP_ST0_ST1
    jc      .L207

.no_shields:
    mcLoad8bitsToReg32 ecx, 196
    call    TConsole_ProutGameMsg
    jmp     .L208

.L207:
    call    TShields_Up

    mcLoadLocal ebx, loc31.pTrekData
    mcZeroBits eax
    mcStoreMember TREKDATA.SHLDCHG, al

.L208:
    call    TCommon_NewQuad

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    jle     .L220

.L210:
    mcLoadLocal ebx, loc31.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_1dot5]
    fmulp
    movzx   eax, [ebx + TREKDATA.REMCOM]
    call    TCommon_FPU_Load_EAX
    fdivp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fld     [ebx + TREKDATA.TIME]
    faddp
    fstp    [ebx + TREKDATA.FUTURE2]
    jmp     .L10

.L220:
    fld     [glb_dbl_1E38]
    mcLoadLocal edi, loc31.pTrekData
    fstp    [edi + TREKDATA.FUTURE2]
    jmp     .L10

.L300:
    ;
    ; Snapshot of Universe (for Time Warp)!
    ;
    mov     esi, [glb_pCommon]
    inc     [esi + TCommon.SNAP]
    lea     edi, [esi + TCommon.SNAPSHT]
    add     esi, TCommon.GAMEDB
    mov     ecx, TREKDATA.size
    rep     movsb

    mcLoadLocal ebx, loc31.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE3]
    jmp     .L10

.L400:
    ;
    ; Commander attacks starbase!
    ;
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    je      .disable_com_at_base

    cmp     [esi + TREKDATA.REMBASE], 0
    jne     .L410

.disable_com_at_base:
    fld     [glb_dbl_1E38]
    fld     st
    mcLoadLocal edi, loc31.pTrekData
    fstp    [edi + TREKDATA.FUTURE4]
    fstp    [edi + TREKDATA.FUTURE5]
    jmp     .L10

.L410:
    ;
    ; Find a quadrant where COMMANDER+BASE, but
    ; no Enterprise and no SUPER COMMANDER.
    ;
    call    TEvents_FindCmdrAtBase
    mcOnRegNotZero ecx, .L430

    mcLoadLocal ebx, loc31.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot3]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE4]

    fld     [glb_dbl_1E38]
    fstp    [ebx + TREKDATA.FUTURE5]
    jmp     .L10

.L430:
    mcLoadGameDataPtr esi
    dec     ecx
    mov     al, [esi + ecx + TREKDATA.CX]
    mov     dl, [esi + ecx + TREKDATA.CY]
    mov     [esi + TREKDATA.BATX], al
    mov     [esi + TREKDATA.BATY], dl

    call    TRandom_Ranf
    fld     [glb_dbl_3]
    fmulp
    fld1
    faddp
    fld     [esi + TREKDATA.DATE]
    faddp
    fstp    [esi + TREKDATA.FUTURE5]

    cmp     [esi + TREKDATA.ISATB], 0
    je      .L431
    ;
    ; FUTURE(5) += FUTURE(7)-DATE
    ;
    fld     [esi + TREKDATA.FUTURE7]
    fld     [esi + TREKDATA.DATE]
    fsubp
    fld     [esi + TREKDATA.FUTURE5]
    faddp
    fstp    [esi + TREKDATA.FUTURE5]

.L431:
    fld     [esi + TREKDATA.INTIME]
    fld     [glb_dbl_0dot3]
    fmulp
    call    TCommon_ExpRan
    fld     [esi + TREKDATA.FUTURE5]
    faddp
    fstp    [esi + TREKDATA.FUTURE4]

    mov     al, CHAR_COMMANDER
    call    TEvents_SOS
    jmp     .L10

.L500:
    ;
    ; Commander destroys a starbase!
    ;
    mcLoadLocal edi, loc31.pTrekData
    fld     [glb_dbl_1E38]
    fstp    [edi + TREKDATA.FUTURE5]

.L502:
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.ISATB], 2
    jne     .L505

    mov     al, [esi + TREKDATA.ISX]
    mov     dl, [esi + TREKDATA.ISY]
    call    TArray_GetGalaxyValue
    mov     eax, ecx
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 100
    div     ecx
    cmp     edx, 10
    jb      .done

    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    mcStoreLocal loc31.iXHOLD, al
    mcStoreLocal loc31.iYHOLD, dl

    mov     al, [esi + TREKDATA.ISX]
    mov     dl, [esi + TREKDATA.ISY]
    mov     [esi + TREKDATA.BATX], al
    mov     [esi + TREKDATA.BATY], dl
    jmp     .L520

.L505:
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    je      .L515
    cmp     [esi + TREKDATA.REMBASE], 0
    je      .L515

    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    call    TArray_GetGalaxyValue
    mov     eax, ecx
    mcZeroBits edx
    mcLoad8bitsToReg32 ecx, 100
    div     ecx
    cmp     edx, 10
    jb      .L515

    call    TEvents_VerifyBattleQuad
    jc      .L520

.L515:
    mcLoadLocal edi, loc31.pTrekData
    mcZeroBits eax
    mov     [edi + TREKDATA.BATX], al
    mov     [edi + TREKDATA.BATY], al
    jmp     .L10

.L520:
    mcLoadLocal esi, loc31.pTrekData
    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    call    TArray_StarChartPtr

    cmp     dword [ebx], -1
    jne     .L5201

    mcZeroBits eax
    mov     [ebx], eax

.L5201:
    cmp     dword [ebx], 1000
    jb      .L5202

    sub     dword [ebx], 10

.L5202:
    mcLoadLocal esi, loc31.pTrekData
    mov     al, [esi + TREKDATA.BATX]
    cmp     al, [esi + TREKDATA.QUADX]
    jne     .L545

    mov     dl, [esi + TREKDATA.BATY]
    cmp     dl, [esi + TREKDATA.QUADY]
    jne     .L545
    ;
    ; Base is destroyed with ship present!
    ;
    mov     al, [esi + TREKDATA.BASEX]
    mov     dl, [esi + TREKDATA.BASEY]
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    mcZeroBits eax
    mov     [esi + TREKDATA.BASEX], al
    mov     [esi + TREKDATA.BASEY], al

    call    TCommon_NewCondition

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 197
    call    TConsole_Prout
    jmp     .L550

.L545:
    mov     cl, DEV_SUBSPACE_RADIO
    call    TArray_IsDamaged
    jc      .L550

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.REMBASE], 1
    je      .L550

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 198
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 199
    call    TConsole_Cram

    mcLoadLocal esi, loc31.pTrekData
    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    mov     cl, 1
    call    TConsole_CramLoc

    mcLoad8bitsToReg32 ecx, 200
    call    TConsole_Prout

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.ISATB], 2
    jne     .L547

    mcLoad8bitsToReg32 ecx, 201
    call    TConsole_Prout
    jmp     .L550

.L547:
    mcLoad8bitsToReg32 ecx, 202
    call    TConsole_Prout

.L550:
    ;
    ; Remove starbase from Galaxy
    ;
    mcLoadLocal esi, loc31.pTrekData
    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    call    TArray_GalaxyPtr
    sub     dword [ebx], 10

    cmp     [esi + TREKDATA.REMBASE], 1
    jle     .L580

    mov     al, [esi + TREKDATA.BATX]
    mov     dl, [esi + TREKDATA.BATY]
    call    TEvents_RemoveThisBase

.L580:
    mcLoadLocal esi, loc31.pTrekData
    dec     [esi + TREKDATA.REMBASE]
    cmp     [esi + TREKDATA.ISATB], 2
    jne     .L515
    ;
    ; Reinstate a COMMANDER's base attack
    ;
    mcLoadLocal al, loc31.iXHOLD
    mcLoadLocal dl, loc31.iYHOLD
    mov     [esi + TREKDATA.BATX], al
    mov     [esi + TREKDATA.BATY], dl
    mov     [esi + TREKDATA.ISATB], 0
    jmp     .L10

.L600:
    ;
    ; Super-Commander moves!
    ;
    mcLoadLocal esi, loc31.pTrekData
    fld     [esi + TREKDATA.DATE]
    fld     [glb_dbl_0dot2777]
    faddp
    fstp    [esi + TREKDATA.FUTURE6]

    movzx   eax, [esi + TREKDATA.IENTESC]
    add     eax, [esp + loc31.bISTRACT]
    mcOnRegNotZero eax, .L10

    cmp     [esi + TREKDATA.ISATB], 1
    je      .L10

    cmp     [esi + TREKDATA.ISCATE], 1
    jne     .move_scom

    cmp     [esi + TREKDATA.JUSTIN], 1
    jne     .L10

.move_scom:
    call    TEvents_MoveSuperCommander
    jmp     .L10

.L700:
    ;
    ; Super-Commander destroys base!
    ;
    mcLoadLocal edi, loc31.pTrekData
    fld     [glb_dbl_1E38]
    fstp    [edi + TREKDATA.FUTURE7]
    mov     [edi + TREKDATA.ISATB], 2
    jmp     .L502

.L5000:
    ;
    ; Check with spy to see if SUPER COMMANDER
    ; will tractor beam the ship
    ;
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.NSCREM], 0
    je      .done

    mcLoadLocal eax, loc31.bICTBEAM
    add     eax, [esp + loc31.bISTRACT]
    mcOnRegNotZero eax, .done

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .done
    cmp     [esi + TREKDATA.ISATB], 1
    je      .done
    cmp     [esi + TREKDATA.ISCATE], 1
    je      .done

    cmp     [esi + TREKDATA.IENTESC], 0
    jne     .L5100

    fld     [glb_dbl_2500]
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jnc     .L5001

    cmp     [esi + TREKDATA.TORPS], 4
    jae     .L5001

    fld     [glb_dbl_1250]
    fld     [esi + TREKDATA.SHLD]
    mc_CMP_ST0_ST1
    jc      .L5100

.L5001:
    mcZeroBits eax
    mcStoreLocal loc31.nDamage_3_and_4, al

    mov     cl, DEV_PHASERS
    call    TArray_IsDamaged
    adc     [esp + loc31.nDamage_3_and_4], 0

    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    adc     [esp + loc31.nDamage_3_and_4], 0

    cmp     [esp + loc31.nDamage_3_and_4], 2
    jne     .L5002

    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.TORPS], 4
    jb      .L5100

.L5002:
    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jnc     .done

    mcLoadLocal esi, loc31.pTrekData
    fld     [glb_dbl_3000]
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L5003

    mov     cl, DEV_PHASERS
    call    TArray_IsDamaged
    jnc     .done

.L5003:
    mcLoadLocal esi, loc31.pTrekData
    cmp     [esi + TREKDATA.TORPS], 5
    jb      .L5100

    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    jnc     .done

.L5100:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot65]
    mc_CMP_ST0_ST1
    jc      .done

    inc     [esp + loc31.bISTRACT]
    jmp     .L20010

.done:
    mcEndLocals loc31.size
    ret

; --------------------------------------------------------------------------
; MOVETHO
; --------------------------------------------------------------------------
virtual at 0
loc90:
    .pTrekData PVOID ?
    .nIDX BYTE ?
    .nIDY BYTE ?
    .nDeltaX BYTE ?
    .nDeltaY BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TEvents_MoveTholian:
    mcBeginLocals loc90.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc90.pTrekData, esi

    cmp     [esi + TREKDATA.ITHERE], 0
    je      .done
    cmp     [esi + TREKDATA.JUSTIN], 1
    je      .done

    mcZeroBits eax
    mcStoreLocal loc90.nDeltaX, al
    mcStoreLocal loc90.nDeltaY, al

    mov     al, [esi + TREKDATA.ITHX]
    mov     dl, [esi + TREKDATA.ITHY]

    mcOnRegEqu al, 1, .ITHX_is_1
    mcOnRegEqu al, 10, .ITHX_is_10
    jmp     .get_rid_of_tholian

.ITHX_is_1:
    mcOnRegEqu dl, 1, .L10
    mcOnRegEqu dl, 10, .L20
    jmp     .get_rid_of_tholian

.ITHX_is_10:
    mcOnRegEqu dl, 10, .L30
    mcOnRegEqu dl, 1, .L40

.get_rid_of_tholian:
    mov     [esi + TREKDATA.ITHERE], 0
    ;int     3
    jmp     .done
    ;
    ; Set destination sector
    ;
.L10:
    mov     [esp + loc90.nIDX], 1
    mov     [esp + loc90.nIDY], 10
    inc     [esp + loc90.nDeltaY]
    jmp     .L50

.L20:
    mov     [esp + loc90.nIDX], 10
    mov     [esp + loc90.nIDY], 10
    inc     [esp + loc90.nDeltaX]
    jmp     .L50

.L30:
    mov     [esp + loc90.nIDX], 10
    mov     [esp + loc90.nIDY], 1
    dec     [esp + loc90.nDeltaY]
    jmp     .L50

.L40:
    mov     [esp + loc90.nIDX], 1
    mov     [esp + loc90.nIDY], 1
    dec     [esp + loc90.nDeltaX]

.L50:
    ;
    ; Make sure destination is empty or web occupies it
    ;
    mcLoadLocal al, loc90.nIDX
    mcLoadLocal dl, loc90.nIDY
    call    TArray_QuadPtr

    mov     al, [ebx]
    mcOnRegEqu al, CHAR_COSMOS, .move
    mcOnRegNotEqu al, CHAR_WEB, .done

.move:
    ;
    ; Leave a web where Tholian is now
    ;
    mcLoadLocal esi, loc90.pTrekData
    mov     al, [esi + TREKDATA.ITHX]
    mov     dl, [esi + TREKDATA.ITHY]
    call    TArray_QuadPtr

    mov     cl, [ebx]
    mcOnRegEqu cl, CHAR_COSMOS, .put_web
    mcOnRegNotEqu cl, CHAR_THOLIAN, .add_deltas

.put_web:
    mov     byte [ebx], CHAR_WEB

.add_deltas:
    mcLoadLocal al, loc90.nDeltaX
    mcLoadLocal dl, loc90.nDeltaY
    mcLoadLocal esi, loc90.pTrekData
    add     [esi + TREKDATA.ITHX], al
    add     [esi + TREKDATA.ITHY], dl
    ;
    ; Check if Tholian arrived into destination sector
    ;
    mov     al, [esi + TREKDATA.ITHX]
    mov     dl, [esi + TREKDATA.ITHY]

    cmp     al, [esp + loc90.nIDX]
    jne     .move
    cmp     dl, [esp + loc90.nIDY]
    jne     .move
    ;
    ; Check if web around quadrant is complete
    ;
    call    TArray_IsWebComplete
    jc      .tholian_leaves

    mcLoadLocal al, loc90.nIDX
    mcLoadLocal dl, loc90.nIDY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_THOLIAN
    jmp     .done

.tholian_leaves:
    mcLoadLocal esi, loc90.pTrekData
    mcZeroBits eax
    mov     [esi + TREKDATA.ITHERE], al
    mov     [esi + TREKDATA.ITHX], al
    mov     [esi + TREKDATA.ITHY], al

    mov     ecx, 664
    call    TConsole_ProutGameMsg

    mov     al, CHAR_BLACK_HOLE
    call    TCommon_DropIn

.done:
    mcEndLocals loc90.size
    ret

; --- EOF ---
