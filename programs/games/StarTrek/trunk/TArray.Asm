; --------------------------------------------------------------------------
; FILE: TArray.Asm
; DATE: October 4, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; LEAVE
; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index (IESC) of the leaving enemy
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_Leave:
    pushad

    mcLoadGameDataPtr ebx
    movzx   edx, [ebx + TREKDATA.NENHERE]
    dec     edx
    dec     ecx

    mcLoadMemberRef edi, TREKDATA.KX
    mov     al, [edi + edx]
    mov     [edi + ecx], al

    mcLoadMemberRef edi, TREKDATA.KY
    mov     al, [edi + edx]
    mov     [edi + ecx], al

    imul    ecx, 10
    imul    edx, 10

    mcLoadMemberRef edi, TREKDATA.KPOWER
    fld     tbyte [edi + edx]
    fstp    tbyte [edi + ecx]

    mcLoadMemberRef edi, TREKDATA.KDIST
    fld     tbyte [edi + edx]
    fstp    tbyte [edi + ecx]

    dec     [ebx + TREKDATA.KLHERE]
    dec     [ebx + TREKDATA.NENHERE]

    cmp     [ebx + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .done

    call    TCommon_NewCondition

.done:
    popad
    ret

; --------------------------------------------------------------------------
; Input:
;   EDI = array relative offset
;   ECX = 1-based index of DOUBLE cell
;   ST(0) = value to store
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetDouble:
    push    eax ecx
    movzx   ecx, cl
    dec     ecx
    imul    ecx, 10
    mcLoadGameDataPtr eax
    add     edi, eax
    fstp    tbyte [edi + ecx]
    pop     ecx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = 1-based index of KPOWER cell
; Output:
;   EDX = address of that DOUBLE cell
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_KlingonPowerPtr:
    push    ecx
    movzx   ecx, cl
    mcLoadGameDataPtr edx
    add     edx, TREKDATA.KPOWER
    dec     ecx
    imul    ecx, 10
    add     edx, ecx
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   EDI = array relative offset
;   CL = 1-based index of DOUBLE cell
; Output:
;   ST(0) = loaded value
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetDouble:
    push    eax ecx
    movzx   ecx, cl
    dec     ecx
    imul    ecx, 10
    mcLoadGameDataPtr eax
    add     edi, eax
    fld     tbyte [edi + ecx]
    pop     ecx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = 1-based index of DAMAGE cell
; Output:
;   ST(0) = loaded value
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetDblDamage:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.DAMAGE
    call    TArray_GetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = 1-based index of DEVICE
; Output:
;   CF=1 if device is damaged
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_IsDamaged:
    call    TArray_GetDblDamage
    fldz
    mc_CMP_ST0_ST1
    jz      .no_damage

    stc
    ret

.no_damage:
    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index of KPOWER cell
; Output:
;   ST(0) = loaded value
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetDblKPower:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.KPOWER
    call    TArray_GetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index of KDIST cell
; Output:
;   ST(0) = loaded value
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetDblKDist:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.KDIST
    call    TArray_GetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index of DAMAGE cell
;   ST(0) = value to store
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetDblDamage:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.DAMAGE
    call    TArray_SetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index of KPOWER cell
;   ST(0) = value to store
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetDblKPower:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.KPOWER
    call    TArray_SetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = 1-based index of KDIST cell
;   ST(0) = value to store
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetDblKDist:
    push    edi
    mcLoad8bitsToReg32 edi, TREKDATA.KDIST
    call    TArray_SetDouble
    pop     edi
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..10]
;   DL = Y coordinate in range [1..10]
; Output:
;   EBX = address of QUAD (X,Y)
; NOTE:
;   Coordinates are preserved during the call
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_QuadPtr:
    push    eax ecx edx
    ;
    ; I = (X-1)*10 + (Y-1)
    ;
    dec     al
    dec     dl
    cbw
    imul    ax, 10
    add     al, dl
    movzx   ecx, al
    ;
    ; Load address into EBX
    ;
    mov     ebx, [glb_pGameData]
    lea     ebx, [ebx + ecx + TREKDATA.QUAD]

    pop     edx ecx eax
    ret

; --------------------------------------------------------------------------
; Output:
;   CF=TRUE if CHAR_WEB is the only element encountered around
;           the quadrant edges.
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_IsWebComplete:
    ;
    ; Check the way from sector 1-1 to sector 1-10
    ;
    mcLoad1 eax
    mov     edx, eax
    call    TArray_QuadPtr

    mcLoad8bitsToReg32 ecx, 10
    mov     al, CHAR_WEB

.check_1:
    cmp     [ebx], al
    jne     .return_false
    inc     ebx
    loop    .check_1
    ;
    ; Check the way from sector 10-1 to sector 10-10
    ;
    mov     al, 10
    mov     dl, 1
    call    TArray_QuadPtr

    mcLoad8bitsToReg32 ecx, 10
    mov     al, CHAR_WEB

.check_2:
    cmp     [ebx], al
    jne     .return_false
    inc     ebx
    loop    .check_2
    ;
    ; Check the vertical ways:
    ;   1. From sector 1-1 to sector 10-1
    ;   2. From sector 1-10 to sector 10-10
    ;
    mov     al, 2
    mov     dl, 1
    call    TArray_QuadPtr

    mcLoad8bitsToReg32 ecx, 8
    mov     al, CHAR_WEB

.check_3:
    cmp     [ebx], al
    jne     .return_false
    cmp     [ebx + 9], al
    jne     .return_false

    add     ebx, 10
    loop    .check_3
    ;
    ; All plugged!
    ;
    stc
    ret

.return_false:
    clc
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetShip:
    mcLoadGameDataPtr ebx
    mcLoadMember al, TREKDATA.SECTX
    mcLoadMember dl, TREKDATA.SECTY
    push    ebx
    call    TArray_QuadPtr
    pop     esi

    mov     al, [esi + TREKDATA.SHIP]
    mov     [ebx], al
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   ECX = index into array (NOT OFFSET)
; NOTE:
;   Coordinates are preserved during the call
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_8x8Index:
    ;
    ; In STAR TREK cosmos: coordinate X is row and
    ; coordinate Y is column in the matrix, so
    ; index is calculated as:
    ;
    ; I = (X-1)*8 + (Y-1)
    ;
    push    eax edx
    dec     al
    dec     dl
    shl     al, 3
    add     al, dl
    movzx   ecx, al
    pop     edx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   EBX = address of SCANNED (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_ScannedQuadPtr:
    push    ecx
    call    TArray_8x8Index
    mov     ebx, [glb_pGameData]
    lea     ebx, [ebx + ecx + TREKDATA.SCANNED]
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetScanned:
    push    ebx
    call    TArray_ScannedQuadPtr
    mov     byte [ebx], 1
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   CF=TRUE if quadrant was scanned by SRSCAN command
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_IsScanned:
    push    eax
    push    ebx
    call    TArray_ScannedQuadPtr
    mov     al, [ebx]
    pop     ebx
    shr     al, 1
    pop     eax
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   EBX = address of NEWSTUF (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_NewStufPtr:
    push    ecx
    call    TArray_8x8Index
    mov     ebx, [glb_pGameData]
    lea     ebx, [ebx + ecx + TREKDATA.NEWSTUF]
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Output:
;   EBX = address of NEWSTUF (QUADX,QUADY) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_MyNewStufPtr:
    push    eax edx esi
    mov     esi, [glb_pGameData]
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TArray_NewStufPtr
    pop     esi edx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   EBX = address of GALAXY (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GalaxyPtr:
    push    ecx
    call    TArray_8x8Index
    mov     ebx, [glb_pGameData]
    lea     ebx, [ebx + ecx*4 + TREKDATA.GALAXY]
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = quadrant X coordinate in range [1..8]
;   DL = quadrant Y coordinate in range [1..8]
;   ESI = TPlanet address
; Output:
;   CF=1 if location is matching PLNETS array item
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_IsPlanetHere:
    cmp     [esi + TPlanet.planet_X], al
    jne     .ret_false

    cmp     [esi + TPlanet.planet_Y], dl
    jne     .ret_false

    stc
    ret

.ret_false:
    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = quadrant X coordinate in range [1..8]
;   DL = quadrant Y coordinate in range [1..8]
;   ECX = commander index (1-based)
; Output:
;   CF=1 if location is matching CX,CY arrays
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_IsCommanderHere:
    push    ebx ecx esi edi
    mcLoadGameDataPtr ebx

    mcLoadMemberRef esi, TREKDATA.CX
    mcLoadMemberRef edi, TREKDATA.CY
    dec     ecx

    cmp     [esi + ecx], al
    jne     .ret_false

    cmp     [edi + ecx], dl
    jne     .ret_false

    pop     edi esi ecx ebx
    stc
    ret

.ret_false:
    pop     edi esi ecx ebx
    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = quadrant X coordinate in range [1..8]
;   DL = quadrant Y coordinate in range [1..8]
; Output:
;   ECX = 1-based index into PLNETS array or zero
;   ESI = address of TPlanet structure
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_FindPlanet:
    mcLoadGameDataPtr ebx
    mcLoadMemberRef esi, TREKDATA.PLNETS
    mcLoad1 ecx

.check_planet:
    call    TArray_IsPlanetHere
    jc      .done

    inc     ecx
    cmp     cl, [ebx + TREKDATA.INPLAN]
    ja      .ret_false

    add     esi, TPlanet.size
    jmp     .check_planet

.ret_false:
    mcZeroBits ecx

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = quadrant X coordinate in range [1..8]
;   DL = quadrant Y coordinate in range [1..8]
; Output:
;   ECX = 1-based index into CX,CY arrays or zero
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_FindCommander:
    mcLoadGameDataPtr esi
    mcLoad1 ecx

.scan:
    call    TArray_IsCommanderHere
    jc      .found_it

    inc     ecx
    cmp     cl, [esi + TREKDATA.REMCOM]
    jbe     .scan

    mcZeroBits ecx

.found_it:
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = planet index (1-based)
; Output:
;   EDI = address of TPlanet structure
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_PlanetPtr:
    dec     ecx
    mcLoadGameDataPtr edi
    imul    ecx, TPlanet.size
    lea     edi, [edi + ecx + TREKDATA.PLNETS]
    ret

; --------------------------------------------------------------------------
; Output:
;   EBX = address of GALAXY (QUADX,QUADY) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_MyGalaxyPtr:
    push    eax edx esi
    mov     esi, [glb_pGameData]
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TArray_GalaxyPtr
    pop     esi edx eax
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   EBX = address of STARCH (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_StarChartPtr:
    push    ecx
    call    TArray_8x8Index
    mov     ebx, [glb_pGameData]
    lea     ebx, [ebx + ecx*4 + TREKDATA.STARCH]
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   ECX = value of STARCH (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetStarChartValue:
    push    ebx
    call    TArray_StarChartPtr
    mov     ecx, [ebx]
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
; Output:
;   ECX = value of GALAXY (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_GetGalaxyValue:
    push    ebx
    call    TArray_GalaxyPtr
    mov     ecx, [ebx]
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
;   ECX = value to set at STARCH (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetStarChartValue:
    push    ebx
    call    TArray_StarChartPtr
    mov     [ebx], ecx
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate in range [1..8]
;   DL = Y coordinate in range [1..8]
;   ECX = value to set at GALAXY (8,8) cell
; NOTE:
;   All registers are preserved
; --------------------------------------------------------------------------
align PROC_ALIGN
TArray_SetGalaxyValue:
    push    ebx
    call    TArray_GalaxyPtr
    mov     [ebx], ecx
    pop     ebx
    ret


; --- EOF ---
