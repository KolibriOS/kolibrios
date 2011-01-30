; --------------------------------------------------------------------------
; FILE: TPhotons.Asm
; DATE: November 1, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   EAX = sector coordinate (X or Y)
; Output:
;   CF=1 if EAX is in range [1..10]
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhotons_IsValidSector:
    cmp     eax, 1
    jb      .invalid
    cmp     eax, 10
    ja      .invalid

    stc
    ret

.invalid:
    clc
    ret

; --------------------------------------------------------------------------
; PHOTONS
; --------------------------------------------------------------------------
virtual at 0
loc44:
    .pTrekData PVOID ?
    .pdbl_KPower PDOUBLE ?
    .nN COUNT ?
    .nTubes COUNT ?
    .bIHURT BOOL ?
    .nI INDEX ?
    .nJ INDEX ?
    .nLL INDEX ?
    .nL INDEX ?
    .nJX INT32 ?
    .nJY INT32 ?
    .torp_IX INT32 ?
    .torp_IY INT32 ?
    .byte_IX BYTE ?
    .byte_IY BYTE ?
    .byte_STARTX BYTE ?
    .byte_STARTY BYTE ?
    .arr_TARGX INTEGERS 6
    .arr_TARGY INTEGERS 6
    ;
    ; I have added here another 3 DOUBLE values (30 bytes) to "improve" the game.
    ; However, that caused a misalignment of the local variables, which
    ; in turn caused the failure of a Sleep API!! (MORALE: Always align the stack!)
    ; Thanks to 'Clerk' from WASM.RU forum I was able to fix the issue.
    ;
    .arr_COURSE DOUBLES 6
    .dbl_R DOUBLE ?
    .dbl_AC DOUBLE ?
    .dbl_TEMP DOUBLE ?
    .dbl_DELTX DOUBLE ?
    .dbl_DELTY DOUBLE ?
    .dbl_HIT DOUBLE ?
    .dbl_ANGLE DOUBLE ?
    .dbl_DELTAX DOUBLE ?
    .dbl_DELTAY DOUBLE ?
    .dbl_BIGGER DOUBLE ?
    .dbl_SINANG DOUBLE ?
    .dbl_X DOUBLE ?
    .dbl_Y DOUBLE ?
    .dbl_DIST DOUBLE ?
    .dbl_DISTN DOUBLE ?
    .dbl_KP DOUBLE ?
    .dbl_ANG DOUBLE ?
    .dbl_SPRANG DOUBLE ?
    .dbl_BETA DOUBLE ?
    .ch_IQUAD CHAR ?
    .ch_reserved CHAR ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TPhotons_Main:
    mcBeginLocals loc44.size
    call    TConsole_SetGameMsgAttr

    mcLoadGameDataPtr esi
    mcStoreLocal loc44.pTrekData, esi
    ;
    ; Check torpedo count and status of photon tubes
    ;
    mov     [esi + TREKDATA.IDIDIT], 1
    mov     [esi + TREKDATA.KSHOT], 0

    movzx   ecx, [esi + TREKDATA.NPTUBES]
    mcStoreLocal loc44.nTubes, ecx

    mov     cl, DEV_PHOTON_TUBES
    call    TArray_IsDamaged
    jnc     .L10

    mov     ecx, 340
    call    TConsole_Prout
    jmp     .L3005

.L10:
    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.TORPS], 0
    jne     .L15

    mov     ecx, 341
    call    TConsole_Prout
    jmp     .L3005

.L15:
    call    TCmdBuf_Scan
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mcStoreLocal loc44.nN, eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    je      .L30

.L16:
    mcLoadLocal esi, loc44.pTrekData
    movzx   eax, [esi + TREKDATA.TORPS]
    call    TConsole_CramInt

    mov     ecx, 342
    call    TConsole_Prout

    mov     ecx, 343
    call    TGame_Prompt
    jmp     .L15

.L30:
    mcLoadLocal eax, loc44.nN
    cmp     eax, 0
    jle     .L3005
    cmp     eax, [esp + loc44.nTubes]
    jle     .L31

    mov     ecx, 344
    call    TConsole_Cram
    mcLoadLocal esi, loc44.pTrekData
    movzx   eax, [esi + TREKDATA.NPTUBES]
    call    TConsole_CramInt
    mov     ecx, 704
    call    TConsole_Prout
    jmp     .L16

.L3004:
    call    TMove_BegPardon

.L3005:
    mcLoadLocal edi, loc44.pTrekData
    dec     [edi + TREKDATA.IDIDIT]
    jmp     KPHOTON.done

.L31:
    mcLoadLocal esi, loc44.pTrekData
    mcLoadLocal eax, loc44.nN
    cmp     al, [esi + TREKDATA.TORPS]
    jle     .L32
    jmp     .L16

.L3101:
    ;
    ; All torpedoes fired at same spot
    ;
    cmp     [esp + loc44.nN], 1
    je      .L36

    mcLoadLocalRef esi, loc44.arr_TARGX
    mcLoadLocalRef edi, loc44.arr_TARGY

    mov     eax, [esi]
    mov     edx, [edi]
    mov     [esi + 4], eax
    mov     [edi + 4], edx

    cmp     [esp + loc44.nN], 2
    je      .L36

    mov     [esi + 8], eax
    mov     [edi + 8], edx
    jmp     .L36

.L32:
    ;
    ; Read in target sectors
    ;
    mcLoad1 eax
    mcStoreLocal loc44.nI, eax

.scan_next_target:
    call    TCmdBuf_Scan

    mcLoadLocal ecx, loc44.nI
    dec     ecx
    mcLoadLocalRef esi, loc44.arr_TARGX
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mov     [esi + ecx*4], eax
    ;
    ; IF ((I == 1) AND (CMD == EOL)) GOTO 34
    ;
    cmp     [esp + loc44.nI], 1
    jne     .L3201
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .L34

.L3201:
    ;
    ; IF ((I == 2) AND (CMD == EOL)) GOTO 3101
    ;
    cmp     [esp + loc44.nI], 2
    jne     .L3202
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .L3101

.L3202:
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L3004

    call    TCmdBuf_Scan

    mcLoadLocal ecx, loc44.nI
    dec     ecx
    mcLoadLocalRef esi, loc44.arr_TARGY
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mov     [esi + ecx*4], eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L3004

    inc     [esp + loc44.nI]
    mov     ecx, [esp + loc44.nI]
    cmp     ecx, [esp + loc44.nN]
    jbe     .scan_next_target
    jmp     .L36

.L34:
    mcLoad1 eax
    mcStoreLocal loc44.nI, eax

.ask_target_sector:
    mov     cl, ATTR_PROMPT_TEXT
    call    TConsole_SetAttr

    mov     ecx, 345
    call    TConsole_Cram

    mcLoadLocal eax, loc44.nI
    call    TConsole_CramInt

    mov     ecx, 301
    call    TGame_Prompt

    call    TCmdBuf_Scan

    mcLoadLocal ecx, loc44.nI
    dec     ecx
    mcLoadLocalRef esi, loc44.arr_TARGX
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mov     [esi + ecx*4], eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L3004

    call    TCmdBuf_Scan

    mcLoadLocal ecx, loc44.nI
    dec     ecx
    mcLoadLocalRef esi, loc44.arr_TARGY
    mov     eax, [ebx + TCmdBuf.cmdbuf_INUM]
    mov     [esi + ecx*4], eax

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_REAL
    jne     .L3004

    inc     [esp + loc44.nI]
    mov     ecx, [esp + loc44.nI]
    cmp     ecx, [esp + loc44.nN]
    jbe     .ask_target_sector

.L36:
    ;
    ; Check for invalid targets
    ;
    mcLoadLocal ecx, loc44.nN
    mcLoadLocalRef esi, loc44.arr_TARGX
    mcLoadLocalRef edi, loc44.arr_TARGY

.validate_sector:
    mov     eax, [edi]
    add     edi, 4

    add     eax, 0
    js      .L3005

    call    TPhotons_IsValidSector
    jnc     .L3004

    lodsd

    add     eax, 0
    js      .L3005

    call    TPhotons_IsValidSector
    jnc     .L3004

    loop    .validate_sector
    ;
    ; Compute course for each torpedo
    ;
    mcLoad1 eax
    mcStoreLocal loc44.nI, eax

.next_course:
    mcLoadLocal ecx, loc44.nI
    dec     ecx

    mcLoadLocalRef esi, loc44.arr_TARGY
    mov     eax, [esi + ecx*4]

    mcLoadLocal ebx, loc44.pTrekData
    movzx   edx, [ebx + TREKDATA.SECTY]

    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTX]

    mcLoadLocalRef esi, loc44.arr_TARGX
    mov     edx, [esi + ecx*4]
    movzx   eax, [ebx + TREKDATA.SECTX]

    sub     eax, edx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTY]
    ;
    ; Do not shoot the ENTERPRISE!
    ;
    fld     [esp + loc44.dbl_DELTX]
    fldz
    mc_CMP_ST0_ST1
    jnz     .L40

    fld     [esp + loc44.dbl_DELTY]
    fldz
    mc_CMP_ST0_ST1
    jnz     .L40

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 346
    call    TConsole_Prout
    mov     ecx, 347
    call    TConsole_Prout
    mov     ecx, 348
    call    TConsole_Prout
    jmp     .L3005

.L40:
    fld     [esp + loc44.dbl_DELTX]
    fld     [esp + loc44.dbl_DELTY]
    fpatan
    fld     [glb_dbl_1dot90985932]
    fmulp
    mcLoadLocal ecx, loc44.nI
    dec     ecx
    imul    ecx, 10
    mcLoadLocalRef edi, loc44.arr_COURSE
    fstp    tbyte [edi + ecx]

    inc     [esp + loc44.nI]
    mov     ecx, [esp + loc44.nI]
    cmp     ecx, [esp + loc44.nN]
    jbe     .next_course
    ;
    ; Begin outer loop for moving torpedoes
    ;
    mcZeroBits eax
    mcStoreLocal loc44.nI, eax

.L50:
    inc     [esp + loc44.nI]
    mov     ecx, [esp + loc44.nI]
    cmp     ecx, [esp + loc44.nN]
    ja      KPHOTON.L115

    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .L50001

    dec     [esi + TREKDATA.TORPS]
    jnz     .L50001

    mov     [esi + TREKDATA.QTORPS], 0

.L50001:
    ;
    ; J=I
    ;
    mcLoadLocal eax, loc44.nI
    mcStoreLocal loc44.nJ, eax

    mov     al, [esi + TREKDATA.SECTX]
    mov     dl, [esi + TREKDATA.SECTY]
    mcStoreLocal loc44.byte_STARTX, al
    mcStoreLocal loc44.byte_STARTY, dl

    call    TRandom_Ranf
    call    TRandom_Ranf
    faddp
    fld     [glb_dbl_0dot5]
    fmulp
    fld     [glb_dbl_0dot5]
    fsubp
    fstp    [esp + loc44.dbl_R]

    fld     [glb_dbl_0dot49]
    fld     [esp + loc44.dbl_R]
    fabs
    mc_CMP_ST0_ST1
    jc      .L5709

    call    TRandom_Ranf
    fld     [glb_dbl_1dot2]
    faddp
    fld     [esp + loc44.dbl_R]
    fmulp
    fstp    [esp + loc44.dbl_R]
    ;
    ; Torpedo misfires
    ;
    mov     cl, ATTR_HIT_DAMAGE
    call    TConsole_SetAttr

    cmp     [esp + loc44.nN], 1
    ja      .L5706

    mov     ecx, 349
    call    TConsole_Prout
    jmp     .L5707

.L5706:
    mov     ecx, 350
    call    TConsole_Cram
    mcLoadLocal eax, loc44.nI
    call    TConsole_CramInt
    mov     ecx, 351
    call    TConsole_Prout

    mov     eax, [esp + loc44.nI]
    cmp     eax, [esp + loc44.nN]
    je      .abort_burst

    mov     ecx, 352
    call    TConsole_Prout

.abort_burst:
    mcLoadLocal eax, loc44.nN
    mcStoreLocal loc44.nI, eax

.L5707:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot2]
    mc_CMP_ST0_ST1
    jc      .L5709

    call    TRandom_Ranf
    fld     st
    faddp
    fld1
    faddp
    mcLoadLocal esi, loc44.pTrekData
    fld     [esi + TREKDATA.DAMFAC]
    fmulp
    mov     cl, DEV_PHOTON_TUBES
    call    TArray_SetDblDamage
    jmp     KPHOTON.L115

.increase_deflection:
    fld     [esp + loc44.dbl_R]
    fld     [esi + TREKDATA.SHLD]
    fmulp
    fld     [glb_dbl_0dot001]
    fmulp
    fld     [esp + loc44.dbl_R]
    faddp
    fstp    [esp + loc44.dbl_R]
    jmp     .calc_AC

.L5709:
    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.QTORPS], 1
    je      .calc_quantum_R
    cmp     [esi + TREKDATA.SHLDUP], 0
    jne     .increase_deflection
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .increase_deflection
    jmp     .calc_AC

.calc_quantum_R:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot15]
    fmulp
    fstp    [esp + loc44.dbl_R]

.calc_AC:
    mcLoadLocal ecx, loc44.nI
    dec     ecx
    imul    ecx, 10
    mcLoadLocalRef edi, loc44.arr_COURSE
    fld     [glb_dbl_DOCKFAC]
    fld     [esp + loc44.dbl_R]
    fmulp
    fld     tbyte [edi + ecx]
    faddp
    fstp    [esp + loc44.dbl_AC]
    jmp     KPHOTON.L5710

KPHOTON:
    ;
    ; Enemy fires photon torpedo!
    ;
    mcBeginLocals loc44.size

    mcLoadGameDataPtr esi
    mcStoreLocal loc44.pTrekData, esi

    movzx   ecx, [esi + TREKDATA.KSHOT]
    dec     ecx
    lea     edi, [esi + TREKDATA.KY]
    add     esi, TREKDATA.KX
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    mcStoreLocal loc44.byte_IX, al
    mcStoreLocal loc44.byte_IY, dl
    mcStoreLocal loc44.byte_STARTX, al
    mcStoreLocal loc44.byte_STARTY, dl

    mcLoad1 eax
    mcStoreLocal loc44.nI, eax
    mcStoreLocal loc44.nN, eax

    mcLoadLocal esi, loc44.pTrekData
    mov     al, [esi + TREKDATA.SECTY]
    sub     al, [esp + loc44.byte_STARTY]
    movsx   eax, al
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTX]

    mov     al, [esp + loc44.byte_STARTX]
    sub     al, [esi + TREKDATA.SECTX]
    movsx   eax, al
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTY]

    fld     [esp + loc44.dbl_DELTX]
    fld     [esp + loc44.dbl_DELTY]
    fpatan
    fld     [glb_dbl_1dot90985932]
    fmulp
    fstp    [esp + loc44.dbl_AC]

    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    fsubp
    fstp    [esp + loc44.dbl_TEMP]

    mcLoadLocal esi, loc44.pTrekData
    movzx   ecx, [esi + TREKDATA.KSHOT]
    dec     ecx
    imul    ecx, 10
    lea     ebx, [esi + TREKDATA.KPOWER]
    fld     tbyte [ebx + ecx]
    fld     [glb_dbl_0dot001]
    fmulp
    fld1
    faddp
    fld     [esp + loc44.dbl_TEMP]
    fmulp

    call    TRandom_Ranf
    fld     [esp + loc44.dbl_TEMP]
    fmulp
    faddp
    fld     st
    fstp    [esp + loc44.dbl_R]

    fld     [glb_dbl_DOCKFAC]
    fmulp
    fld     [esp + loc44.dbl_AC]
    faddp
    fstp    [esp + loc44.dbl_AC]

    call    TConsole_SetGameMsgAttr
    mcLoadLocal al, loc44.byte_IX
    mcLoadLocal dl, loc44.byte_IY
    call    TConsole_CramEnemyAtEx
    mov     ecx, 354
    call    TConsole_Prout
    ;
    ; Check for klingon misfire
    ;
    fld     [glb_dbl_0dot45]
    fld     [esp + loc44.dbl_TEMP]
    fabs
    mc_CMP_ST0_ST1
    jc      .L5710
    ;
    ; Enemy torpedo misfired. Determine damage.
    ;
    fld     [glb_dbl_15]
    fld     [esp + loc44.dbl_AC]
    fsubp
    fld     [glb_dbl_0dot5235988]
    fmulp
    fstp    [esp + loc44.dbl_ANGLE]

    call    TRandom_Ranf
    fld     [glb_dbl_600]
    fmulp
    fld     [glb_dbl_200]
    faddp
    fstp    [esp + loc44.dbl_HIT]

    mcLoadLocal esi, loc44.pTrekData
    movzx   ecx, [esi + TREKDATA.KSHOT]
    mcStoreLocal loc44.nLL, ecx
    ;
    ; BUG FIX! Set these members as if torpedo hit him!
    ;
    dec     ecx
    movzx   eax, [esi + ecx + TREKDATA.KX]
    movzx   edx, [esi + ecx + TREKDATA.KY]
    mcStoreLocal loc44.torp_IX, eax
    mcStoreLocal loc44.torp_IY, edx

    call    TArray_QuadPtr
    mov     al, [ebx]
    mcStoreLocal loc44.ch_IQUAD, al

    mov     ecx, 355
    call    TConsole_Prout
    jmp     .L68

.L5710:
    fld     [glb_dbl_15]
    fld     [esp + loc44.dbl_AC]
    fsubp
    fld     [glb_dbl_0dot5235988]
    fmulp
    fstp    [esp + loc44.dbl_ANGLE]

    call    TConsole_SetGameMsgAttr
    cmp     [esp + loc44.nN], 1
    je      .L58

    call    TConsole_ScrollUp
    mov     ecx, 356
    call    TConsole_Cram
    mcLoadLocal eax, loc44.nJ
    call    TConsole_CramInt

.L5720:
    mov     ecx, 357
    call    TConsole_Cram
    jmp     .L59

.L58:
    call    TConsole_ScrollUp
    mov     ecx, 358
    call    TConsole_Cram

.L59:
    fld     [esp + loc44.dbl_ANGLE]
    fld     st
    fsin
    fchs
    fstp    [esp + loc44.dbl_DELTAX]
    fcos
    fstp    [esp + loc44.dbl_DELTAY]

    fld     [esp + loc44.dbl_DELTAX]
    fabs
    fld     [esp + loc44.dbl_DELTAY]
    fabs
    call    TCommon_FPU_Max
    fstp    [esp + loc44.dbl_BIGGER]

    fld     [esp + loc44.dbl_DELTAX]
    fld     [esp + loc44.dbl_BIGGER]
    fdivp
    fstp    [esp + loc44.dbl_DELTAX]

    fld     [esp + loc44.dbl_DELTAY]
    fld     [esp + loc44.dbl_BIGGER]
    fdivp
    fstp    [esp + loc44.dbl_DELTAY]

    mov     al, [esp + loc44.byte_STARTX]
    call    TCommon_LoadByteIntoFPU
    fstp    [esp + loc44.dbl_X]

    mov     al, [esp + loc44.byte_STARTY]
    call    TCommon_LoadByteIntoFPU
    fstp    [esp + loc44.dbl_Y]
    ;
    ; Begin inner loop for moving a single torpedo
    ;
    mcZeroBits eax
    mcStoreLocal loc44.nL, eax

.L5910:
    inc     [esp + loc44.nL]
    ;
    ; X += DELTAX
    ;
    fld     [esp + loc44.dbl_X]
    fld     [esp + loc44.dbl_DELTAX]
    faddp
    fld     st
    fstp    [esp + loc44.dbl_X]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc44.torp_IX, eax

    cmp     eax, 1
    jl      .L105
    cmp     eax, 10
    jg      .L105
    ;
    ; Y += DELTAY
    ;
    fld     [esp + loc44.dbl_Y]
    fld     [esp + loc44.dbl_DELTAY]
    faddp
    fld     st
    fstp    [esp + loc44.dbl_Y]
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc44.torp_IY, eax

    cmp     eax, 1
    jl      .L105
    cmp     eax, 10
    jg      .L105
    ;
    ; Check if new line must be done in track.
    ;
    cmp     [esp + loc44.nL], 4
    je      .new_line
    cmp     [esp + loc44.nL], 9
    jne     .print_track

.new_line:
    call    TConsole_ScrollUp

.print_track:
    mov     cl, 1
    fld     [esp + loc44.dbl_X]
    call    TConsole_CramFloat

    mcLoad8bitsToReg32 ecx, 59
    call    TConsole_Cram

    mov     cl, 1
    fld     [esp + loc44.dbl_Y]
    call    TConsole_CramFloat

    mov     cl, 3
    call    TConsole_RepeatBlank

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     al, [ebx]
    mcStoreLocal loc44.ch_IQUAD, al

    cmp     al, CHAR_COSMOS
    je      .L5910

    call    TConsole_ScrollUp
    invoke  Sleep, 800
    ;
    ; Begin hit checks.
    ;
    movzx   ecx, [esp + loc44.byte_STARTX]
    sub     ecx, [esp + loc44.torp_IX]
    imul    ecx, ecx

    movzx   eax, [esp + loc44.byte_STARTY]
    sub     eax, [esp + loc44.torp_IY]
    imul    eax, eax

    add     eax, ecx
    call    TCommon_FPU_Load_EAX
    fsqrt
    fstp    [esp + loc44.dbl_DIST]

    mov     eax, [esp + loc44.torp_IY]
    movzx   ecx, [esp + loc44.byte_STARTY]
    sub     eax, ecx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTX]

    mov     ecx, [esp + loc44.torp_IX]
    movzx   eax, [esp + loc44.byte_STARTX]
    sub     eax, ecx
    call    TCommon_FPU_Load_EAX
    fld     [glb_dbl_0dot1]
    fmulp
    fstp    [esp + loc44.dbl_DELTY]
    ;
    ; BULSEYE
    ;
    fld     [esp + loc44.dbl_DELTX]
    fld     [esp + loc44.dbl_DELTY]
    fpatan
    fld     [glb_dbl_1dot90985932]
    fmulp
    fld     [glb_dbl_15]
    fsubrp
    fld     [glb_dbl_0dot5235988]
    fmulp
    fld     [esp + loc44.dbl_ANGLE]
    fsubrp
    fsin
    fld     st
    fstp    [esp + loc44.dbl_SINANG]

    fabs
    fld     [esp + loc44.dbl_DIST]
    fmulp
    fstp    [esp + loc44.dbl_DISTN]
    ;
    ; HIT=(700.0 + 100*RANF(0)) - DISTN*1000.0
    ;
    call    TRandom_Ranf
    fld     [glb_dbl_100]
    fmulp
    fld     [glb_dbl_700]
    faddp

    fld     [esp + loc44.dbl_DISTN]
    fld     [glb_dbl_1000]
    fmulp
    fsubp
    fstp    [esp + loc44.dbl_HIT]
    ;
    ; Quantum warhead packs a punch!
    ;
    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.QTORPS], 1
    jne     .check_object_type
    cmp     [esi + TREKDATA.KSHOT], 0
    jne     .check_object_type

    call    TRandom_Ranf
    fld     [glb_dbl_700]
    fmulp
    fld     [glb_dbl_700]
    faddp
    fld     [esp + loc44.dbl_HIT]
    faddp
    fstp    [esp + loc44.dbl_HIT]
    ;
    ; Test for a type of object hit by torpedo
    ;
.check_object_type:
    mcLoadLocal al, loc44.ch_IQUAD
    mcOnRegEqu al, CHAR_KLINGON, .L62
    mcOnRegEqu al, CHAR_ROMULAN, .L62
    mcOnRegEqu al, CHAR_COMMANDER, .check_anti_photon
    mcOnRegEqu al, CHAR_SCOM, .check_anti_photon
    jmp     .L70

.check_anti_photon:
    call    TRandom_Ranf
    fld     [glb_dbl_0dot05]
    mc_CMP_ST0_ST1
    jc      .L62

    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mcLoadLocal bl, loc44.ch_IQUAD
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAt
    mov     ecx, 360
    call    TConsole_Prout
    mov     ecx, 361
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L62:
    mcLoadLocal ebx, loc44.pTrekData
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    movzx   ecx, [ebx + TREKDATA.NENHERE]

    mcZeroBits eax
    mcStoreLocal loc44.nLL, eax

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY

.find_klingon:
    inc     [esp + loc44.nLL]

    cmp     al, [esi]
    jne     .next_klingon

    cmp     dl, [edi]
    je      .L68

.next_klingon:
    inc     esi
    inc     edi
    loop    .find_klingon

    ;int     3
    jmp     .done

.L68:
    mcLoadLocal ecx, loc44.nLL
    call    TArray_KlingonPowerPtr
    mcStoreLocal loc44.pdbl_KPower, edx
    fld     tbyte [edx]
    fstp    [esp + loc44.dbl_KP]

    fld     [esp + loc44.dbl_KP]
    fabs
    fld     [esp + loc44.dbl_HIT]
    call    TCommon_FPU_Min

    fld     [esp + loc44.dbl_KP]
    call    TCommon_FPU_SignAB

    fld     [esp + loc44.dbl_KP]
    fsubrp
    mcLoadLocal edi, loc44.pdbl_KPower
    fld     st
    fstp    tbyte [edi]

    fldz
    mc_CMP_ST0_ST1
    jnz     .L69

    mcLoadLocal ecx, loc44.nLL
    dec     ecx
    mcLoadLocal ebx, loc44.pTrekData
    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mov     al, [esi + ecx]
    mov     dl, [edi + ecx]
    call    TPhasers_DeadKlingon
    jmp     TPhotons_Main.L50

.L69:
    call    TConsole_SetGameMsgAttr
    mov     cl, 2
    mcLoadLocal bl, loc44.ch_IQUAD
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAt

.L6901:
    ;
    ; If enemy damaged, but not destroyed - try
    ; to displace him.
    ;
    call    TRandom_Ranf
    fld     [glb_dbl_0dot5]
    fsubp
    fld     [glb_dbl_2dot5]
    fmulp
    fld     [esp + loc44.dbl_ANGLE]
    faddp
    fstp    [esp + loc44.dbl_ANG]

    fld     [esp + loc44.dbl_ANG]
    fcos
    fabs
    fld     [esp + loc44.dbl_ANG]
    fsin
    fabs
    call    TCommon_FPU_Max
    fstp    [esp + loc44.dbl_TEMP]

    fld     [esp + loc44.dbl_ANG]
    fsin
    fchs
    fld     [esp + loc44.dbl_TEMP]
    fdivp
    fstp    [esp + loc44.dbl_X]

    fld     [esp + loc44.dbl_ANG]
    fcos
    fld     [esp + loc44.dbl_TEMP]
    fdivp
    fstp    [esp + loc44.dbl_Y]
    ;
    ; Calculate JX,JY - sector of displacement.
    ;
    fild    [esp + loc44.torp_IX]
    fld     [esp + loc44.dbl_X]
    faddp
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc44.nJX, eax

    fild    [esp + loc44.torp_IY]
    fld     [esp + loc44.dbl_Y]
    faddp
    frndint
    call    TCommon_FP_Truncate
    mcStoreLocal loc44.nJY, eax
    ;
    ; If displacement sector is out of quadrant range
    ; then simply report that enemy is not yet destroyed.
    ;
    cmp     eax, 1
    jl      .L6905
    cmp     eax, 10
    jg      .L6905

    mcLoadLocal eax, loc44.nJX
    cmp     eax, 1
    jl      .L6905
    cmp     eax, 10
    jg      .L6905
    ;
    ; Check if displaced into a black hole!
    ;
    mcLoadLocal edx, loc44.nJY
    call    TArray_QuadPtr
    mov     al, [ebx]
    mcOnRegNotEqu al, CHAR_BLACK_HOLE, .L6903
    ;
    ; Someone falls into a black hole!
    ;
    mov     ecx, 362
    call    TConsole_Prout

    mcLoadLocal esi, loc44.pTrekData
    mov     al, [esi + TREKDATA.SHIP]
    cmp     al, [esp + loc44.ch_IQUAD]
    jne     .L6902

    mov     al, 21
    call    TFinish_Main
    jmp     .done

.L6902:
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TPhasers_DeadKlingon
    jmp     TPhotons_Main.L50

.L6903:
    mcOnRegNotEqu al, CHAR_COSMOS, .L6905
    ;
    ; Displace it!
    ;
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    mcLoadLocal eax, loc44.nJX
    mcLoadLocal edx, loc44.nJY
    call    TArray_QuadPtr
    mcLoadLocal al, loc44.ch_IQUAD
    mov     [ebx], al

    mov     ecx, 363
    call    TConsole_Prout
    mov     ecx, 364
    call    TConsole_Cram
    mov     cl, 2
    mcLoadLocal eax, loc44.nJX
    mcLoadLocal edx, loc44.nJY
    call    TConsole_CramLoc
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc44.pTrekData
    mov     al, [esi + TREKDATA.SHIP]
    cmp     al, [esp + loc44.ch_IQUAD]
    jne     .L6904
    ;
    ; Starship displaced by a torpedo.
    ; Move it and reset enemy distances.
    ;
    mcLoadLocal eax, loc44.nJX
    mcLoadLocal edx, loc44.nJY
    mov     [esi + TREKDATA.SECTX], al
    mov     [esi + TREKDATA.SECTY], dl

.reset_distance:
    call    TCommon_ResetDist
    jmp     TPhotons_Main.L50

.L6904:
    ;
    ; Enemy displaced by photon blast!
    ;
    mcLoadLocal ebx, loc44.pTrekData
    mcLoadLocal ecx, loc44.nLL
    dec     ecx

    mcLoadMemberRef esi, TREKDATA.KX
    mcLoadMemberRef edi, TREKDATA.KY
    mcLoadLocal eax, loc44.nJX
    mcLoadLocal edx, loc44.nJY
    mov     [esi + ecx], al
    mov     [edi + ecx], dl
    jmp     .reset_distance

.L6905:
    mov     ecx, 365
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L70:
    cmp     [esp + loc44.ch_IQUAD], CHAR_THOLIAN
    jne     .L7002

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAtEx

    fld     [glb_dbl_600]
    fld     [esp + loc44.dbl_HIT]
    mc_CMP_ST0_ST1
    jc      .L7001

    mov     ecx, 306
    call    TConsole_Prout

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

.cleanup_tholian:
    mcLoadLocal ebx, loc44.pTrekData
    mcZeroBits eax
    mcStoreMember TREKDATA.ITHERE, al
    mcStoreMember TREKDATA.ITHX, al
    mcStoreMember TREKDATA.ITHY, al
    jmp     TPhotons_Main.L50

.L7001:
    fld     [glb_dbl_0dot05]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jc      .tholian_left

.L7510:
    mov     ecx, 366
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.tholian_left:
    mov     ecx, 367
    call    TConsole_Prout

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_WEB

    mov     al, CHAR_BLACK_HOLE
    call    TCommon_DropIn
    jmp     .cleanup_tholian

.L7002:
    cmp     [esp + loc44.ch_IQUAD], CHAR_WEB
    jne     .L7005

    mov     ecx, 368
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L7005:
    cmp     [esp + loc44.ch_IQUAD], CHAR_STARBASE
    jne     .L75
    ;
    ; Starbase hit!
    ;
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAtEx

    fld     [glb_dbl_600]
    fld     [esp + loc44.dbl_HIT]
    mc_CMP_ST0_ST1
    jc      .L7510

    mov     ecx, 369
    call    TConsole_Cram

    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.IPHWHO], 1
    je      .end_msg

    mov     ecx, 370
    call    TConsole_Cram

.end_msg:
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp

    mcLoadLocal esi, loc44.pTrekData
    mov     al, [esi + TREKDATA.QUADX]
    mov     dl, [esi + TREKDATA.QUADY]
    call    TArray_StarChartPtr
    add     dword [ebx], 0
    jns     .clean_base

    mcZeroBits eax
    mov     [ebx], eax

.clean_base:
    mcLoadLocal ebx, loc44.pTrekData
    movzx   ecx, [ebx + TREKDATA.REMBASE]
    mov     al, [ebx + TREKDATA.QUADX]
    mov     dl, [ebx + TREKDATA.QUADY]
    mcLoadMemberRef esi, TREKDATA.BASEQX
    mcLoadMemberRef edi, TREKDATA.BASEQY

.check_base_quad:
    cmp     al, [esi]
    jne     .next_base
    cmp     dl, [edi]
    jne     .next_base
    jmp     .L71

.next_base:
    inc     esi
    inc     edi
    loop    .check_base_quad

    ;int     3
    jmp     .done

.L71:
    ;
    ; Replace the values at ESI,EDI with
    ; the last base coordinates.
    ;
    movzx   ecx, [ebx + TREKDATA.REMBASE]
    dec     ecx

    mcLoadMemberRef edx, TREKDATA.BASEQX
    mov     al, [edx + ecx]
    mov     [esi], al

    mcLoadMemberRef edx, TREKDATA.BASEQY
    mov     al, [edx + ecx]
    mov     [edi], al

    dec     [ebx + TREKDATA.REMBASE]
    mov     [ebx + TREKDATA.BASEX], 0
    mov     [ebx + TREKDATA.BASEY], 0

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    call    TArray_MyGalaxyPtr
    sub     dword [ebx], 10

    mcLoadLocal ebx, loc44.pTrekData
    cmp     [ebx + TREKDATA.IPHWHO], 1
    je      .refresh_condition

    inc     [ebx + TREKDATA.BASEKL]

.refresh_condition:
    mov     [ebx + TREKDATA.CONDIT], 0
    call    TCommon_NewCondition
    jmp     TPhotons_Main.L50

.L75:
    cmp     [esp + loc44.ch_IQUAD], CHAR_PLANET
    jne     .L80

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAtEx

    fld     [glb_dbl_450]
    fld     [esp + loc44.dbl_HIT]
    mc_CMP_ST0_ST1
    jc      .L7510

    mov     ecx, 306
    call    TConsole_Prout

    mcLoadLocal ebx, loc44.pTrekData
    cmp     [ebx + TREKDATA.IPHWHO], 1
    je      .clean_planet

    inc     [ebx + TREKDATA.NPLANKL]

.clean_planet:
    call    TArray_MyNewStufPtr
    dec     byte [ebx]

    mcLoadLocal ebx, loc44.pTrekData
    movzx   ecx, [ebx + TREKDATA.IPLANET]
    dec     ecx
    imul    ecx, TPlanet.size
    mcLoadMemberRef edi, TREKDATA.PLNETS
    add     edi, ecx
    mcLoad8bitsToReg32 ecx, TPlanet.size
    mcZeroBits eax
    rep     stosb

    mcStoreMember TREKDATA.IPLANET, al
    mcStoreMember TREKDATA.PLNETX, al
    mcStoreMember TREKDATA.PLNETY, al

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS

    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.LANDED], 1
    jne     TPhotons_Main.L50

    mov     al, 15
    call    TFinish_Main
    jmp     .done

.L80:
    cmp     [esp + loc44.ch_IQUAD], CHAR_ASTERISK
    jne     .L90
    ;
    ; Check for Nova results...
    ;
    fld     [glb_dbl_270]
    fld     [esp + loc44.dbl_HIT]
    mc_CMP_ST0_ST1
    jnc     .L85

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAtEx
    mov     ecx, 371
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L85:
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TNova_Main

    call    TArray_MyGalaxyPtr
    cmp     dword [ebx], 1000
    je      .done

    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.ALLDONE], 0
    jne     .done
    jmp     TPhotons_Main.L50

.L90:
    ;
    ; Check for a boomerang torpedo
    ;
    mcLoadLocal esi, loc44.pTrekData
    mov     al, [esi + TREKDATA.SHIP]
    cmp     [esp + loc44.ch_IQUAD], al
    jne     .L93

    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L9010

    call    TRandom_Ranf
    fld     [glb_dbl_0dot77]
    mc_CMP_ST0_ST1
    jc      .L9010

    call    TConsole_SetGameMsgAttr
    mov     ecx, 372
    call    TConsole_Cram
    call    TConsole_CramShip
    mov     al, '.'
    call    TConsole_PutChar
    call    TConsole_ScrollUp
    jmp     TPhotons_Main.L50

.L9010:
    call    TConsole_SetGameMsgAttr
    call    TConsole_CramShip
    mov     ecx, 373
    call    TConsole_Prout

    call    TCommon_NewCondition

    mcLoadLocal esi, loc44.pTrekData
    movzx   ecx, [esi + TREKDATA.KSHOT]
    mcLoadLocalRef esi, loc44.dbl_HIT
    mcLoadLocalRef edi, loc44.bIHURT
    call    TAttack_Zap

    mcLoadLocal esi, loc44.pTrekData
    fld     [esi + TREKDATA.ENERGY]
    fldz
    mc_CMP_ST0_ST1
    jc      .L91

    cmp     [esi + TREKDATA.KSHOT], 0
    jne     .L9020

    mov     al, 22
    jmp     .call_finish

.L9020:
    mov     al, 5
    jmp     .call_finish

.L91:
    fld     [esp + loc44.dbl_HIT]
    call    TAttack_Casualties
    ;
    ; Check to see if ship displaced
    ;
    call    TConsole_CramShip
    jmp     .L6901

.L93:
    cmp     [esp + loc44.ch_IQUAD], CHAR_THING
    jne     .L95

    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mov     ecx, 374
    call    TConsole_Prout
    mov     ecx, 375
    call    TConsole_Prout

    call    TConsole_ScrollUp
    call    TConsole_SetCrewMsgAttr
    mov     ecx, 376
    call    TConsole_Prout

    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TArray_QuadPtr
    mov     byte [ebx], CHAR_COSMOS
    jmp     TPhotons_Main.L50

.L95:
    ;
    ; Torpedo enters vicinity of a black hole.
    ; Check for deflection.
    ;
    mcLoadLocal eax, loc44.torp_IX
    mcLoadLocal edx, loc44.torp_IY
    call    TConsole_CramEnemyAtEx

    fld     [glb_dbl_0dot1]
    fld     [esp + loc44.dbl_DISTN]
    mc_CMP_ST0_ST1
    jc      .L97
    ;
    ; Torpedo deflected!
    ;
    fld     [glb_dbl_0dot5]
    fld     [esp + loc44.dbl_DISTN]
    fsubp
    fld     [glb_dbl_7dot853981634]
    fmulp
    fstp    [esp + loc44.dbl_SPRANG]

    fld     [glb_dbl_1dot57079633]
    fld     [esp + loc44.dbl_ANGLE]
    faddp
    fld     [esp + loc44.dbl_SPRANG]
    fsubp
    fstp    [esp + loc44.dbl_BETA]

    fld     [esp + loc44.dbl_DISTN]
    fld     [esp + loc44.dbl_BETA]
    fsin
    fmulp
    fild    [esp + loc44.torp_IX]
    fsubrp
    call    TCommon_FP_Truncate

    mcStoreLocal loc44.byte_STARTX, al
    call    TPhotons_IsValidSector
    jnc     .L97

    fld     [esp + loc44.dbl_DISTN]
    fld     [esp + loc44.dbl_BETA]
    fcos
    fmulp
    fild    [esp + loc44.torp_IY]
    faddp
    call    TCommon_FP_Truncate

    mcStoreLocal loc44.byte_STARTY, al
    call    TPhotons_IsValidSector
    jnc     .L97

    fld     [esp + loc44.dbl_SPRANG]
    fld     [esp + loc44.dbl_SINANG]
    call    TCommon_FPU_SignAB
    fld     [esp + loc44.dbl_ANGLE]
    fsubrp
    fstp    [esp + loc44.dbl_ANGLE]

    mov     ecx, 378
    call    TConsole_Prout
    mov     ecx, 379
    call    TConsole_Cram
    jmp     .L5720

.L97:
    mov     ecx, 377
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L105:
    ;
    ; End inner loop for moving one torpedo
    ;
    call    TConsole_ScrollUp

.L106:
    mov     ecx, 359
    call    TConsole_Prout
    jmp     TPhotons_Main.L50

.L115:
    mcLoadLocal esi, loc44.pTrekData
    cmp     [esi + TREKDATA.REMKL], 0
    je      .finished

    call    TCommon_SortKlingons
    jmp     .done

.finished:
    mov     al, 1

.call_finish:
    call    TFinish_Main

.done:
    mcEndLocals loc44.size
    ret

; --- EOF ---
