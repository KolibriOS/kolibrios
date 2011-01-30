; --------------------------------------------------------------------------
; FILE: TMove.Asm
; DATE: October 12, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; TIMEWRP
; --------------------------------------------------------------------------
virtual at 0
loc56:
    .pTrekData PVOID ?
    .dbl_XDATE DOUBLE ?
    ._pad_1 BYTE ?
    ._pad_2 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_TimeWarp:
    mcBeginLocals loc56.size

    mov     ecx, 627
    call    TConsole_ProutGameMsg

    mcLoadGameDataPtr esi
    mcStoreLocal loc56.pTrekData, esi

    mov     ebx, [glb_pCommon]
    cmp     [ebx + TCommon.SNAP], 0
    je      .travel_forward

    fld     [glb_dbl_0dot5]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L10

.travel_forward:
    mcLoadLocal esi, loc56.pTrekData
    fld     [esi + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fstp    [esi + TREKDATA.TIME]

    mov     ecx, 628
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal esi, loc56.pTrekData
    fld     [esi + TREKDATA.TIME]
    call    TConsole_CramFloat

    mov     ecx, 630
    call    TConsole_Prout

    mcLoadLocal esi, loc56.pTrekData
    fld     [esi + TREKDATA.FUTURE2]
    fld     [glb_dbl_1E38]
    mc_CMP_ST0_ST1
    jz      .L40

    fld     [esi + TREKDATA.FUTURE2]
    fld     [esi + TREKDATA.TIME]
    faddp
    fstp    [esi + TREKDATA.FUTURE2]
    jmp     .L40

.L10:
    mcLoadLocal esi, loc56.pTrekData
    fld     [esi + TREKDATA.DATE]
    fstp    [esp + loc56.dbl_XDATE]

    mov     ebx, [glb_pCommon]
    mov     [ebx + TCommon.SNAP], 0
    mcLoadMemberRef esi, TCommon.SNAPSHT
    mcLoadMemberRef edi, TCommon.GAMEDB
    mov     ecx, TREKDATA.size
    rep     movsb

    mov     ecx, 629
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal esi, loc56.pTrekData
    fld     [esp + loc56.dbl_XDATE]
    fld     [esi + TREKDATA.DATE]
    fsubp
    call    TConsole_CramFloat

    mov     ecx, 630
    call    TConsole_Prout

    mcLoadLocal esi, loc56.pTrekData
    cmp     [esi + TREKDATA.REMCOM], 0
    je      .L25

    mcLoadLocal ebx, loc56.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    movzx   eax, [ebx + TREKDATA.REMCOM]
    call    TCommon_FPU_Load_EAX
    fdivp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE2]

    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot3]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE4]

.L25:
    mcLoadLocal ebx, loc56.pTrekData
    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE1]

    fld     [ebx + TREKDATA.INTIME]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TCommon_ExpRan
    fld     [ebx + TREKDATA.DATE]
    faddp
    fstp    [ebx + TREKDATA.FUTURE3]

    cmp     [ebx + TREKDATA.NSCREM], 0
    je      .set_scom_features

    fld     [glb_dbl_0dot2777]
    fstp    [ebx + TREKDATA.FUTURE6]

.set_scom_features:
    mcZeroBits eax
    mcStoreMember TREKDATA.BATX, al
    mcStoreMember TREKDATA.BATY, al
    mcStoreMember TREKDATA.ISATB, al

    fld     [glb_dbl_1E38]
    fld     st
    fstp    [ebx + TREKDATA.FUTURE5]
    fstp    [ebx + TREKDATA.FUTURE7]
    ;
    ; Revert star chart to earlier era
    ;
    mcLoadMemberRef esi, TREKDATA.STARCH
    mcLoad8bitsToReg32 ecx, 64

.revert:
    cmp     dword [esi], 1
    jle     .next_revert

    mov     dword [esi], 1

.next_revert:
    add     esi, 4
    loop    .revert

    mov     ecx, 631
    call    TConsole_ProutGameMsg

.L40:
    call    TCommon_NewQuad

.done:
    mcEndLocals loc56.size
    ret

; --------------------------------------------------------------------------
; RAM
; --------------------------------------------------------------------------
; Input:
;   AL = IX sector of a collision
;   DL = IY sector of a collision
;   CL = IENM (object type symbol)
;   BL = IBUMPD (who rammed what)
; --------------------------------------------------------------------------
virtual at 0
loc43:
    .pTrekData PVOID ?
    .dbl_TYPE DOUBLE ?
    .dbl_EXTRADM DOUBLE ?
    .chIENM CHAR ?
    .sect_IX BYTE ?
    .sect_IY BYTE ?
    .bIBUMPED BYTE ?
    .nICAS COUNT ?
    .nL INDEX ?
    .pDAMAGE PDOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_Ram:
    mcBeginLocals loc43.size

    mcStoreLocal loc43.bIBUMPED, bl
    mcStoreLocal loc43.sect_IX, al
    mcStoreLocal loc43.sect_IY, dl
    mcStoreLocal loc43.chIENM, cl

    mcLoadGameDataPtr ebx
    mcStoreLocal loc43.pTrekData, ebx

    mcLoadMemberRef esi, TREKDATA.DAMAGE
    mcStoreLocal loc43.pDAMAGE, esi

    call    TMove_RedAlert
    call    TConsole_SetGameMsgAttr

    mov     ecx, 334
    call    TConsole_Prout

    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip

    movzx   eax, [esp + loc43.bIBUMPED]
    lea     ecx, [eax + 335]
    call    TConsole_Cram

    mcLoadLocal al, loc43.chIENM
    mcOnRegEqu al, CHAR_KLINGON, .klingon
    mcOnRegEqu al, CHAR_THOLIAN, .tholian
    mcOnRegEqu al, CHAR_ROMULAN, .romulan
    mcOnRegEqu al, CHAR_COMMANDER, .commander
    mcOnRegEqu al, CHAR_SCOM, .scommander

    ;int     3
    jmp     .done

.klingon:
    push    2
    jmp     .calc_type

.tholian:
    push    1
    jmp     .calc_type

.romulan:
    push    3
    jmp     .calc_type

.commander:
    push    4
    jmp     .calc_type

.scommander:
    push    5

.calc_type:
    pop     ecx
    fldz

.L11:
    fld     [glb_dbl_0dot5]
    faddp
    loop    .L11
    fstp    [esp + loc43.dbl_TYPE]

    mov     cl, 2
    mcLoadLocal bl, loc43.chIENM
    mcLoadLocal al, loc43.sect_IX
    mcLoadLocal dl, loc43.sect_IY
    call    TConsole_CramEnemyAt
    call    TConsole_ScrollUp

    mcLoadLocal al, loc43.sect_IX
    mcLoadLocal dl, loc43.sect_IY
    call    TPhasers_DeadKlingon

    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip
    mov     ecx, 337
    call    TConsole_Prout

    mov     ecx, 338
    call    TConsole_Cram

    call    TRandom_Ranf
    fld     [glb_dbl_20]
    fmulp
    call    TCommon_FP_Truncate
    add     eax, 10
    mcStoreLocal loc43.nICAS, eax
    call    TConsole_CramInt

    mov     ecx, 339
    call    TConsole_Prout

    mcLoadLocal esi, loc43.pTrekData
    mcLoadLocal eax, loc43.nICAS
    add     [esi + TREKDATA.CASUAL], al

    mcLoad1 ecx
    mcStoreLocal loc43.nL, ecx

.damage_device:
    mcLoadLocal esi, loc43.pDAMAGE
    fldz
    fld     tbyte [esi]
    mc_CMP_ST0_ST1
    jc      .L10

    call    TRandom_Ranf
    fld     [esp + loc43.dbl_TYPE]
    fmulp
    fld     [glb_dbl_Ten]
    fmulp
    fld1
    faddp
    mcLoadLocal esi, loc43.pTrekData
    fld     [esi + TREKDATA.DAMFAC]
    fmulp
    fld     [esi + TREKDATA.TIME]
    faddp

    mcLoadLocal esi, loc43.pDAMAGE
    fld     tbyte [esi]
    faddp
    fstp    tbyte [esi]

.L10:
    add     [esp + loc43.pDAMAGE], 10
    inc     [esp + loc43.nL]
    cmp     [esp + loc43.nL], 13
    jbe     .damage_device

    mcLoadLocal edi, loc43.pTrekData
    mov     [edi + TREKDATA.ISUBDAM], 1
    mov     [edi + TREKDATA.SHLDUP], 0

    cmp     [edi + TREKDATA.REMKL], 0
    je      .finished

    call    TDock_DamageReport
    jmp     .done

.finished:
    mov     al, 1
    call    TFinish_Main

.done:
    mcEndLocals loc43.size
    ret

; --------------------------------------------------------------------------
; BEGPARD
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_BegPardon:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 173
    call    TConsole_Prout
    ret

; --------------------------------------------------------------------------
; GETCD
; --------------------------------------------------------------------------
virtual at 0
loc28:
    .pTrekData PVOID ?
    .nIROWQ INT32 ?
    .nIROWS INT32 ?
    .nICOLQ INT32 ?
    .nICOLS INT32 ?
    .nITEMP INT32 ?
    .nIPROMPT INT32 ?
    .nXJ INT32 ?
    .nXK INT32 ?
    .nXL INT32 ?
    .nXI INT32 ?
    .dbl_DELTX DOUBLE ?
    .dbl_DELTY DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_GetCourseDistance:
    mcBeginLocals loc28.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc28.pTrekData, esi

    cmp     [esi + TREKDATA.LANDED], 1
    jne     .L1

    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 163
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 164
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp
    jmp     .L71

.L1:
    movzx   eax, [esi + TREKDATA.QUADX]
    movzx   edx, [esi + TREKDATA.QUADY]
    mcStoreLocal loc28.nIROWQ, eax
    mcStoreLocal loc28.nICOLQ, edx

    fldz
    fldz
    fstp    [esp + loc28.dbl_DELTX]
    fstp    [esp + loc28.dbl_DELTY]

    mcZeroBits ecx
    mcStoreLocal loc28.nITEMP, ecx
    mcStoreLocal loc28.nIPROMPT, ecx

.L10:
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    je      .L24
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    je      .L13

    mov     cl, DEV_COMPUTER
    call    TArray_IsDamaged
    jc      .L1301

.L11:
    mcLoad8bitsToReg32 ecx, 174
    call    TGame_Prompt
    inc     [esp + loc28.nIPROMPT]
    jmp     .L10

.L13:
    mcLoad8bitsToReg32 ecx, 165
    call    TCmdBuf_Crop
    jc      .L20

    mcLoad8bitsToReg32 ecx, 166
    call    TCmdBuf_Crop
    jnc     .L11
    ;
    ; Automatic movement requested.
    ; Check for computer damage.
    ;
    mov     cl, DEV_COMPUTER
    call    TArray_IsDamaged
    jnc     .L14

.L1301:
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 167
    call    TConsole_Prout
    jmp     .L2001

.L14:
    ;
    ; Get quadrant and sector
    ;
    call    TCmdBuf_Scan
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc28.nXI, eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L15

.L1410:
    mcLoad8bitsToReg32 ecx, 168
    call    TGame_Prompt
    inc     [esp + loc28.nIPROMPT]
    jmp     .L14

.L15:
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L1410

    cmp     [ebx + TCmdBuf.cmdbuf_INUM], -1
    je      .L71

    call    TCmdBuf_Scan
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc28.nXJ, eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L1410

    call    TCmdBuf_Scan
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc28.nXK, eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L16

    call    TCmdBuf_Scan
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc28.nXL, eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L1410
    ;
    ; Quadrant and sector specified
    ;
    mcLoadLocal eax, loc28.nXI
    mcLoadLocal edx, loc28.nXJ
    mcStoreLocal loc28.nIROWQ, eax
    mcStoreLocal loc28.nICOLQ, edx

    mcLoadLocal eax, loc28.nXK
    mcLoadLocal edx, loc28.nXL
    mcStoreLocal loc28.nIROWS, eax
    mcStoreLocal loc28.nICOLS, edx
    jmp     .L30

.L16:
    ;
    ; Only sector specified
    ;
    mcLoadLocal eax, loc28.nXI
    mcLoadLocal edx, loc28.nXJ
    mcStoreLocal loc28.nIROWS, eax
    mcStoreLocal loc28.nICOLS, edx
    inc     [esp + loc28.nITEMP]
    jmp     .L30

.L20:
    ;
    ; Manual (DELTX,DELTY) movement
    ;
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    je      .L24

.L2001:
    mcLoad8bitsToReg32 ecx, 169
    call    TGame_Prompt
    inc     [esp + loc28.nIPROMPT]
    jmp     .L20

.L24:
    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc28.dbl_DELTX]

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    jne     .L29
    ;
    ; if (deltx == -1.0) && (key == EOL) goto 71
    ;
    fld     [esp + loc28.dbl_DELTX]
    fld1
    fchs
    mc_CMP_ST0_ST1
    jz      .L71

.L29:
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L70

    fld     [ebx + TCmdBuf.cmdbuf_FNUM]
    fstp    [esp + loc28.dbl_DELTY]
    jmp     .L40

.L30:
    ;
    ; Check for invalid input for automatic case
    ;
    mcLoadLocal eax, loc28.nICOLQ
    mcLoadLocal edx, loc28.nIROWQ
    cmp     eax, 1
    jl      .L70
    cmp     edx, 1
    jl      .L70
    cmp     eax, 8
    jg      .L70
    cmp     edx, 8
    jg      .L70

    mcLoadLocal eax, loc28.nICOLS
    mcLoadLocal edx, loc28.nIROWS
    cmp     eax, 1
    jl      .L70
    cmp     edx, 1
    jl      .L70
    cmp     eax, 10
    jg      .L70
    cmp     edx, 10
    jg      .L70
    ;
    ; Print message from appropriate officer
    ;
    cmp     [esp + loc28.nITEMP], 0
    jne     .L31

    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 170
    call    TConsole_Prout
    jmp     .L32

.L31:
    cmp     [esp + loc28.nIPROMPT], 0
    je      .L32

    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 171
    call    TConsole_Cram

    mov     cl, 2
    mcLoadLocal eax, loc28.nIROWS
    mcLoadLocal edx, loc28.nICOLS
    call    TConsole_CramLoc

    mcLoad8bitsToReg32 ecx, 154
    call    TConsole_Prout

.L32:
    ;
    ; Convert to DELTX,DELTY form
    ;
    mcLoadLocal esi, loc28.pTrekData

    mcLoadLocal eax, loc28.nICOLS
    movzx   edx, [esi + TREKDATA.SECTY]
    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    mcLoadLocal eax, loc28.nICOLQ
    movzx   edx, [esi + TREKDATA.QUADY]
    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    faddp
    fstp    [esp + loc28.dbl_DELTX]

    mcLoadLocal edx, loc28.nIROWS
    movzx   eax, [esi + TREKDATA.SECTX]
    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    mcLoadLocal edx, loc28.nIROWQ
    movzx   eax, [esi + TREKDATA.QUADX]
    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    faddp
    fstp    [esp + loc28.dbl_DELTY]

.L40:
    ;
    ; Check for a zero movement
    ;
    fld     [esp + loc28.dbl_DELTX]
    fldz
    mc_CMP_ST0_ST1
    jnz     .L42

    fld     [esp + loc28.dbl_DELTY]
    fldz
    mc_CMP_ST0_ST1
    jz      .L71

.L42:
    cmp     [esp + loc28.nIPROMPT], 0
    je      .L43

    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 172
    call    TConsole_Prout

.L43:
    ;
    ; Convert into course and distance
    ;
    fld     [esp + loc28.dbl_DELTX]
    fld     st
    fmulp

    fld     [esp + loc28.dbl_DELTY]
    fld     st
    fmulp

    faddp
    fsqrt
    mcLoadLocal edi, loc28.pTrekData
    fstp    [edi + TREKDATA.DIST]

    fld     [esp + loc28.dbl_DELTX]
    fld     [esp + loc28.dbl_DELTY]
    fpatan
    fld     [glb_dbl_1dot90985932]
    fmulp
    fstp    [edi + TREKDATA.DIREC]

    fldz
    fld     [edi + TREKDATA.DIREC]
    mc_CMP_ST0_ST1
    jnc     .done

    fld     [edi + TREKDATA.DIREC]
    fld     [glb_dbl_12]
    faddp
    fstp    [edi + TREKDATA.DIREC]
    jmp     .done

.L70:
    call    TConsole_ScrollUp
    call    TMove_BegPardon

.L71:
    fld1
    fchs
    mcLoadLocal edi, loc28.pTrekData
    fstp    [edi + TREKDATA.DIREC]

.done:
    mcEndLocals loc28.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = X sector
;   EDX = Y sector
; Output:
;   ST(0) = distance
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_DistanceAfterMove:
    mcLoadGameDataPtr ebx

    movzx   ecx, [ebx + TREKDATA.SECTX]
    sub     eax, ecx

    movzx   ecx, [ebx + TREKDATA.SECTY]
    sub     edx, ecx

    imul    edx, edx
    imul    eax, eax

    add     eax, edx
    call    TCommon_FPU_Load_EAX
    fsqrt
    fld     [glb_dbl_0dot1]
    fmulp
    ret

; --------------------------------------------------------------------------
; REDALRT
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_RedAlert:
    mov     cl, ATTR_COND_RED
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 180
    call    TConsole_Prout
    ret

; --------------------------------------------------------------------------
; MOVE
; --------------------------------------------------------------------------
virtual at 0
loc29:
    .pTrekData PVOID ?
    .bTRBEAM BOOL ?
    .bXTIMEW BOOL ?
    .dbl_DeltaX DOUBLE ?
    .dbl_DeltaY DOUBLE ?
    .dbl_Angle DOUBLE ?
    .dbl_Bigger DOUBLE ?
    .dbl_X DOUBLE ?
    .dbl_Y DOUBLE ?
    .dbl_Stopegy DOUBLE ?
    .dbl_align DOUBLE ?
    .nIX INT32 ?
    .nIY INT32 ?
    .nSteps INT32 ?
    .iStep INT32 ?
    .nIQUAD BYTE ?
    .nKINKS BYTE ?
    .nKINK BYTE ?
    .nReserved3 BYTE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_Move:
    mcBeginLocals loc29.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc29.pTrekData, esi

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .check_if_leaving_orbit

    mov     [esi + TREKDATA.CONDIT], CONDITION_GREEN
    call    TCommon_NewCondition

.check_if_leaving_orbit:
    cmp     [esi + TREKDATA.INORBIT], 0
    je      .L1

    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 175
    call    TConsole_Prout

    mcLoadLocal esi, loc29.pTrekData
    mov     [esi + TREKDATA.INORBIT], 0

.L1:
    fld     [glb_dbl_15]
    fld     [esi + TREKDATA.DIREC]
    fsubp
    fld     [glb_dbl_0dot5235998]
    fmulp
    fstp    [esp + loc29.dbl_Angle]
    ;
    ; DELTAX = -SIN (ANGLE)
    ;
    fld     [esp + loc29.dbl_Angle]
    fsin
    fchs
    fstp    [esp + loc29.dbl_DeltaX]
    ;
    ; DELTAY = COS (ANGLE)
    ;
    fld     [esp + loc29.dbl_Angle]
    fcos
    fstp    [esp + loc29.dbl_DeltaY]

    fld     [esp + loc29.dbl_DeltaX]
    fld     [esp + loc29.dbl_DeltaY]
    call    TMove_Bigger
    fstp    [esp + loc29.dbl_Bigger]

    fld     [esp + loc29.dbl_DeltaX]
    fld     [esp + loc29.dbl_Bigger]
    fdivp
    fstp    [esp + loc29.dbl_DeltaX]

    fld     [esp + loc29.dbl_DeltaY]
    fld     [esp + loc29.dbl_Bigger]
    fdivp
    fstp    [esp + loc29.dbl_DeltaY]

    mov     [esp + loc29.bTRBEAM], 0
    ;
    ; If tractor beam is to occur, do not
    ; move full distance
    ;
    fld     [esi + TREKDATA.FUTURE2]
    fld     [esi + TREKDATA.DATE]
    fld     [esi + TREKDATA.TIME]
    faddp
    mc_CMP_ST0_ST1
    jc      .L5
    ;
    ; Ship will be tractored!
    ;
    inc     [esp + loc29.bTRBEAM]
    mov     [esi + TREKDATA.CONDIT], CONDITION_RED

    fld     [esi + TREKDATA.FUTURE2]
    fld     [esi + TREKDATA.DATE]
    fsubp
    fld     [esi + TREKDATA.DIST]
    fmulp
    fld     [esi + TREKDATA.TIME]
    fdivp
    fld     [glb_dbl_0dot1]
    faddp
    fstp    [esi + TREKDATA.DIST]

    fld     [esi + TREKDATA.FUTURE2]
    fld     [esi + TREKDATA.DATE]
    fsubp
    fld     [glb_dbl_0dot00001]
    faddp
    fstp    [esi + TREKDATA.TIME]

.L5:
    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    mov     al, [esi + TREKDATA.SECTX]
    call    TCommon_LoadByteIntoFPU
    fstp    [esp + loc29.dbl_X]

    mov     al, [esi + TREKDATA.SECTY]
    call    TCommon_LoadByteIntoFPU
    fstp    [esp + loc29.dbl_Y]
    ;
    ; How many steps to move
    ;
    fld     [esi + TREKDATA.DIST]
    fld     [esp + loc29.dbl_Bigger]
    fmulp
    fld     [glb_dbl_Ten]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc29.nSteps, eax
    mcOnRegZero eax, .L100

    mcLoad1 ecx
    mcStoreLocal loc29.iStep, ecx

.move_one_step:
    fld     [esp + loc29.dbl_X]
    fld     [esp + loc29.dbl_DeltaX]
    faddp
    fld     st
    fstp    [esp + loc29.dbl_X]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc29.nIX, eax

    fld     [esp + loc29.dbl_Y]
    fld     [esp + loc29.dbl_DeltaY]
    faddp
    fld     st
    fstp    [esp + loc29.dbl_Y]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc29.nIY, eax

    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY

    cmp     eax, 1
    jl      .L40
    cmp     edx, 1
    jl      .L40
    cmp     eax, 10
    jg      .L40
    cmp     edx, 10
    jg      .L40

    call    TArray_QuadPtr
    mov     al, [ebx]
    mcStoreLocal loc29.nIQUAD, al

    cmp     al, CHAR_COSMOS
    jne     .L20

    inc     [esp + loc29.iStep]
    mov     ecx, [esp + loc29.iStep]
    cmp     ecx, [esp + loc29.nSteps]
    jbe     .move_one_step
    ;
    ; Arrived into sector in the same quadrant
    ;
    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    call    TMove_DistanceAfterMove
    mcLoadLocal edi, loc29.pTrekData
    fstp    [edi + TREKDATA.DIST]

    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    mov     [edi + TREKDATA.SECTX], al
    mov     [edi + TREKDATA.SECTY], dl
    jmp     .L100

.L20:
    ;
    ; Object encountered along flight path
    ;
    mcLoadLocal esi, loc29.pTrekData
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_50]
    fmulp
    fld     [esi + TREKDATA.TIME]
    fdivp
    fstp    [esp + loc29.dbl_Stopegy]

    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    call    TMove_DistanceAfterMove
    fstp    [esi + TREKDATA.DIST]

    mcLoadLocal al, loc29.nIQUAD
    mcOnRegEqu al, CHAR_KLINGON, .L30
    mcOnRegEqu al, CHAR_COMMANDER, .L30
    mcOnRegEqu al, CHAR_SCOM, .L30
    mcOnRegEqu al, CHAR_ROMULAN, .L30
    mcOnRegEqu al, CHAR_THOLIAN, .L30
    mcOnRegEqu al, CHAR_BLACK_HOLE, .L25
    ;
    ; Ship is blocked by some object
    ;
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    call    TConsole_CramShip

    mcLoad8bitsToReg32 edx, 177
    mcLoad8bitsToReg32 ecx, 176
    cmp     [esp + loc29.nIQUAD], CHAR_WEB
    cmove   ecx, edx
    call    TConsole_Cram

    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    mov     cl, 2
    call    TConsole_CramLoc

    mov     al, ';'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 178
    call    TConsole_Cram

    mov     cl, 2
    fld     [esp + loc29.dbl_Stopegy]
    call    TConsole_CramFloat

    mcLoad8bitsToReg32 ecx, 179
    call    TConsole_Prout
    ;
    ; Deplete energy due to emergency stop
    ;
    mcLoadLocal esi, loc29.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc29.dbl_Stopegy]
    fsubp
    fstp    [esi + TREKDATA.ENERGY]

    fld     [esp + loc29.dbl_X]
    fld     [esp + loc29.dbl_DeltaX]
    fsubp
    call    TCommon_FP_Truncate
    mov     [esi + TREKDATA.SECTX], al

    fld     [esp + loc29.dbl_Y]
    fld     [esp + loc29.dbl_DeltaY]
    fsubp
    call    TCommon_FP_Truncate
    mov     [esi + TREKDATA.SECTY], al
    ;
    ; Check if emergency stop depleted all remaining energy
    ;
    fld     [esi + TREKDATA.ENERGY]
    fldz
    mc_CMP_ST0_ST1
    jc      .L100

    mov     al, 4
    call    TFinish_Main
    jmp     .done

.L25:
    ;
    ; Object is a black hole! Swallow ship.
    ;
    call    TMove_RedAlert
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    call    TConsole_Cram3Asterisks
    call    TConsole_CramShip
    mcLoad8bitsToReg32 ecx, 182
    call    TConsole_Cram

    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    mov     cl, 2
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    mc_CMP_ST0_ST1
    jc      .L27
    ;
    ; Ship warped into another quadrant
    ; by a black hole.
    ;
    call    TRandom_IRan8
    mcLoadLocal ebx, loc29.pTrekData
    mcStoreMember TREKDATA.QUADX, al
    mcStoreMember TREKDATA.QUADY, dl

    call    TRandom_IRan10
    mcLoadLocal ebx, loc29.pTrekData
    mcStoreMember TREKDATA.SECTX, al
    mcStoreMember TREKDATA.SECTY, dl

    call    TRandom_Ranf
    fld     [glb_dbl_0dot65]
    mcZeroBits ecx
    mc_CMP_ST0_ST1
    setc    cl
    mcStoreLocal loc29.bXTIMEW, ecx

    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 183
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 184
    call    TConsole_Cram

    cmp     [esp + loc29.bXTIMEW], 0
    je      .engage_time_warp

    mcLoad8bitsToReg32 ecx, 185
    call    TConsole_Cram

.engage_time_warp:
    mcLoad8bitsToReg32 ecx, 186
    call    TConsole_Prout

    cmp     [esp + loc29.bXTIMEW], 0
    je      .L95

    call    TMove_TimeWarp

    mcLoadLocal edi, loc29.pTrekData
    mov     [edi + TREKDATA.KSTUF4], 1
    jmp     .L95

.L27:
    mov     al, 21
    call    TFinish_Main
    jmp     .done

.L30:
    ;
    ; Object is an enemy vessel. Ram him!
    ;
    mcLoadLocal eax, loc29.nIX
    mcLoadLocal edx, loc29.nIY
    mcLoadLocal ebx, loc29.pTrekData
    mcStoreMember TREKDATA.SECTX, al
    mcStoreMember TREKDATA.SECTY, dl

    mcLoadLocal cl, loc29.nIQUAD
    mcZeroBits ebx
    call    TMove_Ram
    jmp     .L100

.L40:
    ;
    ; Compute final position -- new quadrant and sector
    ;
    mcLoadLocal esi, loc29.pTrekData
    movzx   eax, [esi + TREKDATA.QUADX]
    movzx   ecx, [esi + TREKDATA.SECTX]
    dec     eax
    imul    eax, 10
    add     eax, ecx
    mcStoreLocal loc29.nIX, eax

    movzx   eax, [esi + TREKDATA.QUADY]
    movzx   ecx, [esi + TREKDATA.SECTY]
    dec     eax
    imul    eax, 10
    add     eax, ecx
    mcStoreLocal loc29.nIY, eax

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esp + loc29.dbl_Bigger]
    fmulp
    fld     [esp + loc29.dbl_DeltaX]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    add     [esp + loc29.nIX], eax

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esp + loc29.dbl_Bigger]
    fmulp
    fld     [esp + loc29.dbl_DeltaY]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    add     [esp + loc29.nIY], eax
    ;
    ; Check for edge of the galaxy
    ;
    mcZeroBits ecx
    mcStoreLocal loc29.nKINKS, cl

.L45:
    mcZeroBits ecx
    mcStoreLocal loc29.nKINK, cl

    cmp     [esp + loc29.nIX], 0
    jg      .L50

    neg     [esp + loc29.nIX]
    inc     [esp + loc29.nIX]
    inc     [esp + loc29.nKINK]

.L50:
    cmp     [esp + loc29.nIY], 0
    jg      .L55

    neg     [esp + loc29.nIY]
    inc     [esp + loc29.nIY]
    inc     [esp + loc29.nKINK]

.L55:
    cmp     [esp + loc29.nIX], 80
    jle     .L60

    mcLoad8bitsToReg32 eax, 161
    sub     eax, [esp + loc29.nIX]
    mcStoreLocal loc29.nIX, eax
    inc     [esp + loc29.nKINK]

.L60:
    cmp     [esp + loc29.nIY], 80
    jle     .L65

    mcLoad8bitsToReg32 eax, 161
    sub     eax, [esp + loc29.nIY]
    mcStoreLocal loc29.nIY, eax
    inc     [esp + loc29.nKINK]

.L65:
    cmp     [esp + loc29.nKINK], 0
    je      .L70

    inc     [esp + loc29.nKINKS]
    jmp     .L45

.L70:
    cmp     [esp + loc29.nKINKS], 0
    je      .L90

    mcLoadLocal esi, loc29.pTrekData
    inc     [esi + TREKDATA.NKINKS]
    cmp     [esi + TREKDATA.NKINKS], 3
    je      .L80

    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 187
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 188
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 189
    call    TConsole_Prout
    jmp     .L90

.L80:
    mov     al, 6
    call    TFinish_Main
    jmp     .done

.L90:
    ;
    ; Compute final position of starship
    ; in new quadrant.
    ;
    mcLoadLocal eax, loc29.nIX
    add     eax, 9
    mcLoad8bitsToReg32 ecx, 10
    mcZeroBits edx
    div     ecx
    mcLoadLocal ebx, loc29.pTrekData
    mcStoreMember TREKDATA.QUADX, al

    mcLoadLocal eax, loc29.nIY
    add     eax, 9
    mcZeroBits edx
    div     ecx
    mcStoreMember TREKDATA.QUADY, al

    movzx   ecx, [ebx + TREKDATA.QUADX]
    dec     ecx
    imul    ecx, 10
    mcLoadLocal eax, loc29.nIX
    sub     eax, ecx
    mcStoreMember TREKDATA.SECTX, al

    movzx   ecx, [ebx + TREKDATA.QUADY]
    dec     ecx
    imul    ecx, 10
    mcLoadLocal eax, loc29.nIY
    sub     eax, ecx
    mcStoreMember TREKDATA.SECTY, al

    cmp     [esp + loc29.bTRBEAM], 0
    jne     .done

.L95:
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 190
    call    TConsole_Cram

    mcLoadLocal ebx, loc29.pTrekData
    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY
    mov     cl, 1
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    call    TArray_SetShip
    call    TCommon_NewQuad
    jmp     .done

.L100:
    call    TArray_SetShip
    call    TCommon_ResetDist
    call    TCommon_SortKlingons

.done:
    mcEndLocals loc29.size
    ret

; --------------------------------------------------------------------------
; ST0 = MAX (ABS (ST0), ABS (ST1))
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_Bigger:
    fabs
    fxch
    fabs
    call    TCommon_FPU_Max
    ret

; --------------------------------------------------------------------------
; WARP
; --------------------------------------------------------------------------
virtual at 0
loc26:
    .pTrekData PVOID ?
    .dblPower DOUBLE ?
    .dblAngle DOUBLE ?
    .dblDeltaX DOUBLE ?
    .dblDeltaY DOUBLE ?
    .dblBigger DOUBLE ?
    .dbl_align DOUBLE ?
    .dblSectX DOUBLE ?
    .dblSectY DOUBLE ?
    .nIWarp INT32 ?
    .nSteps INT32 ?
    .iStep INT32 ?
    .iSectX INT32 ?
    .iSectY INT32 ?
    .Blooey BOOL ?
    .TWarp BOOL ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMove_Warp:
    mcBeginLocals loc26.size

    mcLoadGameDataPtr edi
    mcStoreLocal loc26.pTrekData, edi
    mov     [edi + TREKDATA.IDIDIT], 0

    mov     cl, DEV_WARP_ENGINES
    call    TArray_GetDblDamage
    fld     [glb_dbl_Ten]
    mc_CMP_ST0_ST1
    jc      TMove_WarpX.L90

    mov     cl, DEV_WARP_ENGINES
    call    TArray_IsDamaged
    jnc     .L3

    fld     [glb_dbl_4]
    mcLoadLocal esi, loc26.pTrekData
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L3
    jz      .L3

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 146
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 147
    call    TConsole_Prout
    call    TConsole_ScrollUp
    jmp     TMove_WarpX.done

.L3:
    call    TMove_GetCourseDistance

    fldz
    mcLoadLocal esi, loc26.pTrekData
    fld     [esi + TREKDATA.DIREC]
    mc_CMP_ST0_ST1
    jc      TMove_WarpX.done
    ;
    ; Check energy required to use warp engines
    ;
    fld     [esi + TREKDATA.WARPFAC]
    fld     st
    fld     st
    fmulp
    fmulp
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_0dot05]
    faddp
    fmulp
    movzx   eax, [esi + TREKDATA.SHLDUP]
    inc     eax
    call    TCommon_FPU_Load_EAX
    fmulp
    fstp    [esp + loc26.dblPower]

    fld     [esi + TREKDATA.ENERGY]
    fld     [esp + loc26.dblPower]
    mc_CMP_ST0_ST1
    jc      .L10
    ;
    ; Not enough energy
    ;
    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 148
    call    TConsole_Prout

    mcLoadLocal esi, loc26.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    je      .L5

    fld     [esp + loc26.dblPower]
    fld     [glb_dbl_0dot5]
    fmulp
    fld     [esi + TREKDATA.ENERGY]
    mc_CMP_ST0_ST1
    jc      .L5

    mcLoad8bitsToReg32 ecx, 149
    call    TConsole_Prout
    jmp     TMove_WarpX.done

.L5:
    fld     [esi + TREKDATA.ENERGY]
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_0dot05]
    faddp
    fdivp
    mcFpuPop
    mov     esi, glb_dbl_FpuPopper
    mov     edi, glb_dbl_0dot333
    call    TCommon_X_exp_Y
    call    TCommon_FP_Truncate
    mcStoreLocal loc26.nIWarp, eax
    mcOnRegZero eax, .L8

    mcLoad8bitsToReg32 ecx, 152
    call    TConsole_Cram
    mcLoadLocal eax, loc26.nIWarp
    call    TConsole_CramInt

    mcLoadLocal esi, loc26.pTrekData
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     .L6

    mcLoad8bitsToReg32 ecx, 154
    call    TConsole_Prout
    jmp     TMove_WarpX.done

.L6:
    mov     al, CHAR_COMMA
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 153
    call    TConsole_Prout
    jmp     TMove_WarpX.done

.L8:
    mcLoad8bitsToReg32 ecx, 151
    call    TConsole_Prout
    jmp     TMove_WarpX.done

.L10:
    ;
    ; Check the trip duration
    ;
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esi + TREKDATA.WFACSQ]
    fdivp
    fstp    [esi + TREKDATA.TIME]

    fld     [esi + TREKDATA.REMTIME]
    fld     [glb_dbl_0dot8]
    fmulp
    fld     [esi + TREKDATA.TIME]
    mc_CMP_ST0_ST1
    jc      TMove_WarpX.L20

    call    TConsole_SetCrewMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 155
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 156
    call    TConsole_Cram

    mcLoadLocal esi, loc26.pTrekData
    fld     [esi + TREKDATA.TIME]
    fld     [esi + TREKDATA.REMTIME]
    fdivp
    fld     [glb_dbl_100]
    fmulp

    mov     cl, 2
    call    TConsole_CramFloat
    mcLoad8bitsToReg32 ecx, 157
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 158
    call    TGame_JA
    jc      TMove_WarpX.L20
    jmp     TMove_WarpX.done

; --------------------------------------------------------------------------
; WARPX
; --------------------------------------------------------------------------
TMove_WarpX:
    mcBeginLocals loc26.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc26.pTrekData, esi

.L20:
    mcZeroBits eax
    mcStoreLocal loc26.Blooey, eax
    mcStoreLocal loc26.TWarp, eax

    mcLoadLocal esi, loc26.pTrekData
    fld     [glb_dbl_6]
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L50
    jz      .L50
    ;
    ; Check if engines will get damaged by high warp
    ;
    fld     [esi + TREKDATA.WARPFAC]
    fld     [glb_dbl_6]
    fsubp
    fld     st
    fmulp
    fld     [esi + TREKDATA.DIST]
    fmulp
    fld     [glb_dbl_66dot666]
    fdivp

    call    TRandom_Ranf
    fxch
    mc_CMP_ST0_ST1
    jc      .check_time_warp
    ;
    ; Engines will get damaged
    ;
    inc     [esp + loc26.Blooey]
    call    TRandom_Ranf
    mcLoadLocal esi, loc26.pTrekData
    fld     [esi + TREKDATA.DIST]
    fmulp
    fstp    [esi + TREKDATA.DIST]

.check_time_warp:
    mcLoadLocal esi, loc26.pTrekData
    fld     [glb_dbl_Ten]
    fld     [esi + TREKDATA.WARPFAC]
    mc_CMP_ST0_ST1
    jc      .L40

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_0dot5]
    fmulp
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .L40

    inc     [esp + loc26.TWarp]

.L40:
    mcLoadLocal ecx, loc26.Blooey
    add     ecx, [esp + loc26.TWarp]
    mcOnRegZero ecx, .L50
    ;
    ; If ship will collide with something then cancel
    ; engine damages or time warp.
    ;
    mcLoadLocal esi, loc26.pTrekData
    fld     [glb_dbl_0dot15]
    fld     [esi + TREKDATA.DIREC]
    fsubp
    fld     [glb_dbl_0dot5235998]
    fmulp
    fstp    [esp + loc26.dblAngle]
    ;
    ; DELTAX = -SIN (ANGLE)
    ;
    fld     [esp + loc26.dblAngle]
    fsin
    fchs
    fstp    [esp + loc26.dblDeltaX]
    ;
    ; DELTAY = COS (ANGLE)
    ;
    fld     [esp + loc26.dblAngle]
    fcos
    fstp    [esp + loc26.dblDeltaY]

    fld     [esp + loc26.dblDeltaX]
    fld     [esp + loc26.dblDeltaY]
    call    TMove_Bigger
    fstp    [esp + loc26.dblBigger]

    fld     [esp + loc26.dblDeltaX]
    fld     [esp + loc26.dblBigger]
    fdivp
    fstp    [esp + loc26.dblDeltaX]

    fld     [esp + loc26.dblDeltaY]
    fld     [esp + loc26.dblBigger]
    fdivp
    fstp    [esp + loc26.dblDeltaY]

    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esp + loc26.dblBigger]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc26.nSteps, eax
    mcOnRegZero eax, .L50

    movzx   eax, [esi + TREKDATA.SECTX]
    call    TCommon_FPU_Load_EAX
    fstp    [esp + loc26.dblSectX]

    movzx   eax, [esi + TREKDATA.SECTY]
    call    TCommon_FPU_Load_EAX
    fstp    [esp + loc26.dblSectY]

    mcLoad1 ecx
    mcStoreLocal loc26.iStep, ecx

.move_one_step:
    fld     [esp + loc26.dblSectX]
    fld     [esp + loc26.dblDeltaX]
    faddp
    fld     st
    fstp    [esp + loc26.dblSectX]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc26.iSectX, eax

    cmp     eax, 1
    jl      .L50
    cmp     eax, 10
    jg      .L50

    fld     [esp + loc26.dblSectY]
    fld     [esp + loc26.dblDeltaY]
    faddp
    fld     st
    fstp    [esp + loc26.dblSectY]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc26.iSectY, eax

    cmp     eax, 1
    jl      .L50
    cmp     eax, 10
    jg      .L50

    mcLoadLocal eax, loc26.iSectX
    mcLoadLocal edx, loc26.iSectY
    call    TArray_QuadPtr
    cmp     byte [ebx], CHAR_COSMOS
    je      .move_one_step

    mcZeroBits eax
    mov     [esp + loc26.Blooey], eax
    mov     [esp + loc26.TWarp], eax

.L50:
    mcLoadLocal edi, loc26.pTrekData
    mov     [edi + TREKDATA.KSTUF4], 0

    call    TMove_Move

    mcLoadLocal esi, loc26.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done
    ;
    ; Deplete energy spent on move
    ;
    fld     [esi + TREKDATA.WARPFAC]
    fld     st
    fld     st
    fmulp
    fmulp
    fld     [esi + TREKDATA.DIST]
    fmulp
    movzx   eax, [esi + TREKDATA.SHLDUP]
    inc     eax
    call    TCommon_FPU_Load_EAX
    fmulp
    fld     [esi + TREKDATA.ENERGY]
    fsubrp
    fld     st
    fstp    [esi + TREKDATA.ENERGY]

    fldz
    mc_CMP_ST0_ST1
    jc      .L55

    mov     al, 4
    call    TFinish_Main
    jmp     .done

.L55:
    cmp     [esi + TREKDATA.KSTUF4], 0
    jne     .try_enter_time_warp
    ;
    ; Time needed for a trip
    ;
    fld     [esi + TREKDATA.DIST]
    fld     [glb_dbl_Ten]
    fmulp
    fld     [esi + TREKDATA.WFACSQ]
    fdivp
    fstp    [esi + TREKDATA.TIME]

.try_enter_time_warp:
    cmp     [esp + loc26.TWarp], 0
    je      .damage_warp_engines

    call    TMove_TimeWarp

.damage_warp_engines:
    cmp     [esp + loc26.Blooey], 0
    je      .L60

    call    TRandom_Ranf
    fld     [glb_dbl_3]
    fmulp
    fld1
    faddp
    mcLoadLocal esi, loc26.pTrekData
    fld     [esi + TREKDATA.DAMFAC]
    fmulp
    mov     cl, DEV_WARP_ENGINES
    call    TArray_SetDblDamage

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mcLoad8bitsToReg32 ecx, 148
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 161
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 162
    call    TConsole_Prout

.L60:
    mcLoadLocal edi, loc26.pTrekData
    inc     [edi + TREKDATA.IDIDIT]
    jmp     .done

.L90:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 150
    call    TConsole_Prout

.done:
    mcEndLocals loc26.size
    ret

; --- EOF ---
