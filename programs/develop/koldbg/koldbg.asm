        use32
        db      'MENUET01'
        dd      1
        dd      Start
        dd      I_END
MemSize dd      Mem
        dd      StackTop
        dd      0,AppPath

VERSION equ     '0.01+'

;-----------------------------------------------------------------------------
Bitness equ     32
;-----------------------------------------------------------------------------
bit     equ     1 shl
by      equ     shl 8 +

%B      =       32

macro   even    Value { rb (Value-1)-($+Value-1) mod Value }

macro   ifc    Opc&
{
local ..L
        jnc     ..L
        Opc
..L:
}

macro   ifa    Opc&
{
local ..L
        jna     ..L
        Opc
..L:
}

macro   ifae   Opc&
{
local ..L
        jnae    ..L
        Opc
..L:
}

macro   ifb    Opc&
{
local ..L
        jnb     ..L
        Opc
..L:
}

macro   ifbe   Opc&
{
local ..L
        jnbe    ..L
        Opc
..L:
}

macro   ifg     Opc&
{
local ..L
        jng     ..L
        Opc
..L:
}

macro   ifge   Opc&
{
local ..L
        jnge    ..L
        Opc
..L:
}

macro   ifl    Opc&
{
local ..L
        jnl     ..L
        Opc
..L:
}

macro   ifle   Opc&
{
local ..L
        jnle    ..L
        Opc
..L:
}

macro   ifno   Opc&
{
local ..L
        jo      ..L
        Opc
..L:
}

macro   ifnp   Opc&
{
local ..L
        jp      ..L
        Opc
..L:
}

macro   ifns   Opc&
{
local ..L
        js      ..L
        Opc
..L:
}

macro   ifnz   Opc&
{
local ..L
        jz      ..L
        Opc
..L:
}

macro   ifo    Opc&
{
local ..L
        jno     ..L
        Opc
..L:
}

macro   ifp    Opc&
{
local ..L
        jnp     ..L
        Opc
..L:
}

macro   ifs    Opc&
{
local ..L
        jns     ..L
        Opc
..L:
}

macro   ifz    Opc&
{
local ..L
        jnz     ..L
        Opc
..L:
}

macro   lea     Dst,Src
{
local ..L,..H
virtual at 0
        mov     al,byte Src
load ..L byte from 0x0
if ..L = 0xA0
  load ..H dword from 0x1
end if
end virtual
if ..L = 0xA0
        mov     Dst,..H
else
        lea     Dst,Src
end if
}

include 'kosfuncs.inc'
include '../../macros.inc'

include 'font.inc'
include 'tools.inc'
include 'kernel.inc'

virtual at rsp
All:
.edi    dd ?
.esi    dd ?
.ebp    dd ?
.res    dd ?
.ebx    dd ?
.edx    dd ?
.ecx    dd ?
.eax    dd ?
end virtual

macro   jecxnz  Target
{
        inc     ecx
        loop    Target
}

TestFile        db '/sys/develop/scancode',0

MinWidth = 90
MaxWidth = 255
MinHeight = 75
MaxHeight = 255

DATA_WIDTH      = 88                    ; in characters, multiple of 4

CMD_WIDTH       = DATA_WIDTH

CursorNormSize = 2
CursorBigsSize = FontHeight

;-----------------------------------------------------------------------------
;                           Fast load test file event
DoLoad: mov     esi,TestFile
        mov     edi,LoadName
        jmp     OnLoadInit
;-----------------------------------------------------------------------------
;                           Load executable event
OnLoad: mov     esi,[CurArg]
OnLoadInit:
        mov     edi,LoadName
        or      [PrgNameLen],-1
        mov     [PrgNamePtr],edi
    .CopyName:
        lodsb
        stosb
        inc     [PrgNameLen]
        cmp     al,'/'
        jnz     @F
        or      [PrgNameLen],-1
        mov     [PrgNamePtr],edi
    @@:
        cmp     al,' '
        ja      .CopyName
        mov     byte [edi-1],0
        and     [LoadParams],0
        dec     esi
        call    GetArg.SkipSpaces
        cmp     al,0
        jz      @F
        mov     [LoadParams],esi
    @@:
       ;and     [DumpPos],0
        mov     ecx,[Symbols]
        jecxz   DoReLoad
        mcall   68,13
        and     [Symbols],0
        and     [NumSymbols],0
DoReLoad:
        mcall   18,7
        mov     [DbgWnd],eax
        xchg    ecx,eax
        mcall   70,FN70LoadBlock
        test    eax,eax
        jns     .Loaded

    .LoadErr:
        push    eax
        mov     esi,LoadErrMsg
        call    PutMessage
        pop     eax
        not     eax
        cmp     eax,0x20
        jae     .UnkErr
        mov     esi,[LoadErrMsgs+eax*4]
        test    esi,esi
        jnz     PutMessage

    .UnkErr:
        mov     esi,UnkErrMsg
        inc     eax
        push    eax
        call    PutMessageNoDraw
        jmp     DrawMessages

.Loaded:
        mov     [DebuggeePID],eax
        mov     [bSuspended],1
        mcall   5,20
        push    ecx
        call    GetContext
        mov     edi,OldContext
        mov     ecx,(CtxEnd-Context)/4
        rep     movsd
        ; activate debugger window
        pop     ecx
        mcall   18,3
        call    GetDump
if 1
        push    dword [ShowNames]
        mov     [ShowNames],-1
end if
        mov     [AfterKey],0
        call    ShowImage
        mov     esi,LoadSuccMsg
        push    [DebuggeePID]
        call    PutMessageNoDraw
        call    DrawMessages
    ; try to load symbols
        mov     esi,LoadName
        mov     edi,SymbolsFile
        push    edi
    @@:
        lodsb
        stosb
        test    al,al
        jnz     @B
        lea     ecx,[edi-1]
    @@:
        dec     edi
        cmp     edi,SymbolsFile
        jb      @F
        cmp     byte [edi],'/'
        jz      @F
        cmp     byte [edi],'.'
        jnz     @B
        mov     ecx,edi
    @@:
        mov     dword [ecx],'.dbg'
        mov     byte [ecx+4],0
        pop     esi
        mov     ebp,esi
        push    ecx esi
        call    OnLoadSymbols.Silent    ; Try to load .dbg file
        pop     esi ecx
        xor     eax,eax
        cmp     [NumSymbols],eax
        jne     @F
        mov     dword [ecx],'.map'      ; If failed, try .map file too
        call    OnLoadSymbols.Silent
    @@:
if 1
        pop     eax
        mov     [ShowNames],al
        call    DrawMessages
end if
        ret

;-----------------------------------------------------------------------------
;                        Working with debug context
GetNewContext:
        mov     esi,Context
        mov     edi,OldContext
        mov     ecx,(CtxEnd-Context)/4
        rep     movsd

GetContext:
        mcall   69,1,[DebuggeePID],CtxEnd-Context,Context
        ret

SetContext:
        mcall   69,2,[DebuggeePID],28h,Context
        ret

;-----------------------------------------------------------------------------
;                        Resume process event
DoResume:
        mcall   69,5,[DebuggeePID]
        mov     [bSuspended],0
        ret

;-----------------------------------------------------------------------------
;                        Breakpoints manipulation
OnBp:   mov     esi,[CurArg]
        call    CalcExpression
        jc      .Ret
        xchg    eax,ebp
        push    eax
        call    FindBreakPoint
        inc     eax
        pop     eax
        jz      .NotFound
        mov     esi,aDuplicateBreakpoint
        jmp     .SayErr
    .NotFound:
        mov     bl,1
        call    AddBreakPoint
        jnc     .Ret
        mov     esi,aBreakpointLimitExceeded
    .SayErr:
        call    PutMessage
    .Ret:
        jmp     DrawMessages;DrawDisAsm

;-----------------------------------------------------------------------------
;                            Dump memory event
OnDump: mov     esi,[CurArg]
        cmp     byte [esi],0
        jnz     .Param
       ;add     [DumpPos],DUMP_HEIGHT*10h
        add     [DataAddr],8*8
        jmp     .DoIt

    .Param:
        call    CalcExpression
        jc      .Ret
       ;mov     [DumpPos],ebp
        mov     [DataAddr],ebp
    .DoIt:
       ;call    GetDump
       ;call    DrawDump.Redraw
        call    DrawMessages
    .Ret:
        ret

OnBpmb: mov     dh,0011b
        jmp     DoBpm

OnBpmw: mov     dh,0111b
        jmp     DoBpm

OnBpmd: mov     dh,1111b

DoBpm:  mov     esi,[CurArg]
        cmp     byte [esi],'w'
        jnz     @F
        and     dh,not 2
        inc     esi
    @@:
        push    edx
        call    CalcExpression
        pop     edx
        jnc     @F
        ret
    ; ebp = expression, dh = flags
    @@:
        movzx   eax,dh
        shr     eax,2
        test    ebp,eax
        jz      @F
        mov     esi,aUnaligned
        jmp     PutMessage
    @@:
        mov     eax,ebp
        mov     bl,0Bh
        call    AddBreakPoint
        jnc     @F
        mov     esi,aBreakpointLimitExceeded
        jmp     PutMessage
    ; now find index
    @@:
        push    eax
        xor     ecx,ecx
    .L1:
        cmp     [DrXBreak+ecx*4],0
        jnz     .L2
        push    ecx
        mov     dl,cl
        mov     esi,ebp
        mcall   69,9,[DebuggeePID]
        test    eax,eax
        jz      .OK
        pop     ecx
    .L2:
        inc     ecx
        cmp     ecx,4
        jb      .L1
        pop     eax
        call    ClearBreakPoint
        mov     esi,aBreakpointLimitExceeded
        jmp     PutMessage
    .OK:
        pop     ecx
        pop     eax
        and     byte [edi],not 2        ; breakpoint is enabled
        shl     dl,6
        or      dl,dh
        mov     byte [edi+1],dl
        inc     eax
        mov     [DrXBreak+ecx*4],eax
        ret

OnBc:   mov     esi,[CurArg]
    @@:
        call    GetHexNumber
        jc      OnBp.Ret
        call    ClearBreakPoint
        jmp     @B
OnBd:
        mov     esi,[CurArg]
    @@:
        call    GetHexNumber
        jc      OnBp.Ret
        call    DisableBreakPoint
        jmp     @B
OnBe:
        mov     esi,[CurArg]
    @@:
        call    GetHexNumber
        jc      OnBp.Ret
        push    eax
        call    FindEnabledBreakPoint
        pop     eax
        jz      .Err
        call    EnableBreakPoint
        jmp     @B
    .Err:
        mov     esi,OnBeErrMsg
        jmp     PutMessage

GetHexNumber:
        call    GetArg.SkipSpaces
        xor     ecx,ecx
        xor     edx,edx
    @@:
        lodsb
        call    IsHexDigit
        jc      .Ret
        shl     edx,4
        or      dl,al
        inc     ecx
        jmp     @B
    .Ret:
        dec     esi
        cmp     ecx,1
        xchg    eax,edx
        ret

;-----------------------------------------------------------------------------
;                       Breakpoints list event
OnBl:   mov     esi,[CurArg]
        cmp     byte [esi],0
        jz      .ListAll
        call    GetHexNumber
        jc      .Ret
        cmp     eax,BreakPointsN
        jae     .Err
        push    eax
        add     eax,eax
        lea     edi,[BreakPoints+eax+eax*2]
        pop     eax
        test    byte [edi+4],1
        jz      .Err
        call    ShowBreakInfo
    .Ret:
        ret
    .Err:
        mov     esi,aInvalidBreak
        jmp     PutMessage

    .ListAll:
        mov     edi,BreakPoints
        xor     eax,eax
    @@:
        test    byte [edi+4],1
        jz      .Cont
        push    edi eax
        call    ShowBreakInfo
        pop     eax edi
    .Cont:
        add     edi,6
        inc     eax
        cmp     eax,BreakPointsN
        jb      @B
        ret

ShowBreakInfo:
        push    edi
        test    byte [edi+4],8
        jnz     .DR
        push    dword [edi]
        push    eax
        mov     esi,aBreakNum
        call    PutMessageNoDraw
        jmp     .CMN
    .DR:
        push    eax
        mov     esi,aMemBreak1
        call    PutMessageNoDraw
        pop     edi
        push    edi
        mov     esi,aMemBreak2
        test    byte [edi+5],2
        jz      @F
        mov     esi,aMemBreak3
    @@:
        call    PutMessageNoDraw
        pop     edi
        push    edi
        mov     esi,aMemBreak6
        test    byte [edi+5],8
        jnz     @F
        mov     esi,aMemBreak5
        test    byte [edi+5],4
        jnz     @F
        mov     esi,aMemBreak4
    @@:
        call    PutMessageNoDraw
        pop     edi
        push    edi
        push    dword [edi]
        mov     esi,aMemBreak7
        call    PutMessageNoDraw
    .CMN:
        pop     edi
        test    byte [edi+4],2
        jz      @F
        push    edi
        mov     esi,aDisabled
        call    PutMessageNoDraw
        pop     edi
    @@:
        test    byte [edi+4],4
        jz      @F
        mov     esi,aOneShot
        call    PutMessageNoDraw
    @@:
        mov     esi,NewLine
        jmp     PutMessage

;-----------------------------------------------------------------------------
;                       Unpack executable event
OnUnpack:
    ; program must be loaded - checked when command was parsed
    ; program must be stopped
        mov     esi,aRunningErr
        cmp     [bSuspended],0
        jz      PutMessage
   ; all breakpoints must be disabled
        mov     edi,BreakPoints
    @@:
        test    byte [edi+4],1
        jz      .Cont
        test    byte [edi+4],2
        jnz     .Cont
        mov     esi,aEnabledBreakErr
        jmp     PutMessage
    .Cont:
        add     edi,6
        cmp     edi,BreakPoints+BreakPointsN*6
        jb      @B
    ; ok, now do it
    ; set breakpoint on 0xC dword access
        push    9
        pop     ebx
        mov     ecx,[DebuggeePID]
        mov     dx,1111b*256
        push    0xC
        pop     esi
    @@:
        mcall   69
        test    eax,eax
        jz      .BreakOK
        inc     edx
        cmp     dl,4
        jb      @B
    .BreakOK:
        call    GoOn
    ; now wait for event
    .Wait:
        mcall   10
        dec     eax
        jz      .Redraw
        dec     eax
        jz      .Key
        dec     eax
        jnz     .Debug
    ; button; we have only one button, close
        or      eax,-1
        mcall

    .Redraw:
        mov     [DoDraw],1
        call    DrawWindow
        jmp     .Wait

    .Key:
        mov     al,2
        mcall
        cmp     ah,3   ; Ctrl+C
        jnz     .Wait

    .UserBreak:
        mov     esi,aInterrupted
    .X1:
        push    edx esi
        call    PutMessage
        pop     esi edx
        or      dh,80h
        mcall   69,9,[DebuggeePID]
        cmp     esi,aUnpacked
        jnz     OnSuspend
        jmp     AfterSuspend

    .Debug:
        cmp     [DbgBufLen],4*3
        jnz     .NotOur
        cmp     dword [DbgBuf],3
        jnz     .NotOur
        test    byte [DbgBuf+8],1
        jnz     .Our

    .NotOur:
        mov     esi,aInterrupted
        push    edx
        call    PutMessage
        pop     edx
        or      dh,80h
        mcall   69,9,[DebuggeePID]
        jmp     DebugMsg

    .Our:
        and     [DbgBufLen],0
        push    edx
        call    GetContext
        push    eax
        mcall   69,6,[DebuggeePID],4,0xC,esp
        pop     eax
        pop     edx
        cmp     eax,[_EIP]
        jz      .Done
        call    DoResume
        jmp     .Wait

    .Done:
        mov     esi,aUnpacked
        jmp     .X1

;-----------------------------------------------------------------------------
;                        Calculate expression event
OnCalc: mov     esi,[CurArg]
        call    CalcExpression
        jc      .Ret
        push    ebp
        mov     esi,CalcString
        call    PutMessageNoDraw
        jmp     DrawMessages
    .Ret:
        ret

;-----------------------------------------------------------------------------
;                       Access to register value event
OnReg:  mov     esi,[CurArg]
        call    GetArg.SkipSpaces
        call    FindReg
        jnc     @F
    .Err:
        mov     esi,RSyntax
        jmp     PutMessage
    @@:
        call    GetArg.SkipSpaces
        test    al,al
        jz      .Err
        cmp     al,'='
        jnz     @F
        inc     esi
        call    GetArg.SkipSpaces
        test    al,al
        jz      .Err
    @@:
        push    edi
        call    CalcExpression
        pop     edi
        jc      .Ret
    ; now edi=register id, ebp=value
        cmp     [bSuspended],0
        mov     esi,aRunningErr
        jz      PutMessage
        xchg    eax,ebp
        cmp     edi,24
        jz      .EIP
        sub     edi,4
        jb      .8LO
        sub     edi,4
        jb      .8HI
        sub     edi,8
        jb      .16
        mov     [_EAX+edi*4],eax
        jmp     .Ret
    .16:
        mov     word [_EAX+(edi+8)*4],ax
        jmp     .Ret
    .8LO:
        mov     byte [_EAX+(edi+4)*4],al
        jmp     .Ret
    .8HI:
        mov     byte [_EAX+(edi+4)*4+1],al
        jmp     .Ret
    .EIP:
        mov     [_EIP],eax
       ;call    UpdateDisAsmEIP
    .Ret:
        call    SetContext
        jmp     DrawMessages;DrawRegisters.ReDraw

;-----------------------------------------------------------------------------
;                       Step execution event
;Here we get [<number>] argument at do step <number> times
OnStepMultiple:
        cmp     [bSuspended],0
        jz      OnStep.Running
        mov     [StepNum],1
        mov     esi,[CurArg]
        test    esi,esi
        jz      .Do
        cmp     byte [esi],0
        jz      .Do
        call    GetHexNumber
        jc      .Ret
        or      eax,eax ; check if lesser or equal than 0
        jle     .Ret
        mov     [StepNum],eax
.Do:    call    OnStep
        dec     [StepNum]
        jnz     .Do
.Ret:   ret

OnStep:
        cmp     [bSuspended],0
        jz      .Running
        call    GetContext
        or      byte [_EFL+1],1 ; set TF
        call    SetContext
        and     byte [_EFL+1],not 1
    ; if instruction at eip is "int xx", set one-shot breakpoint immediately after
        mov     eax,[_EIP]
        call    FindEnabledBreakPoint
        jnz     @F
        cmp     byte [edi+5],0xCD
        jz      .Int
     @@:
        push    0
        mcall   69,6,[DebuggeePID],3,[_EIP],esp
        cmp     eax,edx
        pop     eax
        jnz     .DoIt
        cmp     al,0xCD
        jz      .Int
        cmp     ax,0x050F
        jz      .SysCall
        cmp     ax,0x340F
        jz      .SysEnter
    ; resume process
    .DoIt:
        call    GoOn
        cmp     [bAfterGo],0
        jz      @F
        mov     [bAfterGo],2
    @@:
        ret

    ; return address is [ebp-4]
    .SysEnter:
        push    0
        inc     edx     ; read 4 bytes
        mov     esi,[_EBP]
        sub     esi,4
        mcall   69
        cmp     eax,edx
        pop     eax
        jnz     .SysCall
        push    eax
        and     byte [_EFL+1],not 1
        call    SetContext
        pop     eax
        jmp     @F

    .SysCall:
        and     byte [_EFL+1],not 1     ; clear TF - avoid system halt (!)
        call    SetContext
    .Int:
        mov     eax,[_EIP]
        inc     eax
        inc     eax
    @@:
        push    eax
        call    FindEnabledBreakPoint
        pop     eax
        jz      .DoIt
    ; there is no enabled breakpoint yet; set temporary breakpoint
        mov     bl,5
        call    AddBreakPoint
        jmp     .DoIt

    .Running:
        mov     esi,aRunningErr
        jmp     PutMessage

;-----------------------------------------------------------------------------
;                       Proceed process event
;Here we get [<number>] argument at do step <number> times
OnProceedMultiple:
        cmp     [bSuspended],0
        jz      OnStep.Running
        mov     [ProcNum],1
        mov     esi,[CurArg]
        test    esi,esi
        jz      .Do
        cmp     byte [esi],0
        jz      .Do
        call    GetHexNumber
        jc      .Ret
        or      eax,eax ; check if lesser or equal than 0
        jle     .Ret
        mov     [ProcNum],eax
        and     [CurArg],0
.Do:
        call    OnProceed
        dec     [ProcNum]
        jnz     .Do
.Ret:
        ret

OnProceed:
        cmp     [bSuspended],0
        jz      OnStep.Running
        mov     esi,[_EIP]

    @@:
        call    GetByteNoBreak
        jc      OnStep
        inc     esi
    ; skip prefixes
        call    IsPrefix
        jz      @B
        cmp     al,0xE8         ; call
        jnz     @f
        add     esi,4
        jmp     .DoIt

    ; A4,A5 = movs; A6,A7 = cmps
    @@:
        cmp     al,0xA4
        jb      @F
        cmp     al,0xA8
        jb      .DoIt

    ; AA,AB = stos; AC,AD = lods; AE,AF = scas
    @@:
        cmp     al,0xAA
        jb      @F
        cmp     al,0xB0
        jb      .DoIt

    ; E0 = loopnz; E1 = loopz; E2 = loop
    @@:
        cmp     al,0xE0
        jb      .NoLoop
        cmp     al,0xE2
        ja      .NoLoop
        inc     esi
        jmp     .DoIt

    ; FF /2 = call
    .NoLoop:
        cmp     al,0xFF
        jnz     OnStep
        call    GetByteNoBreak
        jc      OnStep
        inc     esi
        mov     cl,al
        and     al,00111000b
        cmp     al,00010000b
        jnz     OnStep
    ; skip instruction
        mov     al,cl
        and     eax,7
        shr     cl,6
        jz      .Mod0
        jp      .DoIt
        cmp     al,4
        jnz     @F
        inc     esi
    @@:
        inc     esi
        dec     cl
        jz      @F
        add     esi,3
    @@:
        jmp     .DoIt
    .Mod0:
        cmp     al,4
        jnz     @F
        call    GetByteNoBreak
        jc      OnStep
        inc     esi
        and     al,7
    @@:
        cmp     al,5
        jnz     .DoIt
        add     esi,4
    .DoIt:
    ; insert one-shot breakpoint at esi and resume
        call    GetByteNoBreak
        jc      OnStep
        mov     eax,esi
        call    FindEnabledBreakPoint
        jz      @F
        mov     eax,esi
        mov     bl,5
        call    AddBreakPoint
        jmp     OnStep.DoIt
    @@:
        ret

;-----------------------------------------------------------------------------
;                        Read next byte of machine code
GetByteNoBreak:
        mov     eax,esi
        call    FindEnabledBreakPoint
        jnz     .NoBreak
        mov     al,[edi+5]
        clc
        ret

    .NoBreak:
        xor     edx,edx
        push    edx
        inc     edx
        mov     edi,esp
        mcall   69,6,[DebuggeePID]
        dec     eax
        clc
        jz      @F
        stc
    @@:
        pop     eax
        ret

IsPrefix:
        cmp     al,0x64         ; fs:
        jz      .Ret
        cmp     al,0x65         ; gs:
        jz      .Ret
        cmp     al,0x66         ; use16/32
        jz      .Ret
        cmp     al,0x67         ; addr16/32
        jz      .Ret
        cmp     al,0xF0         ; lock
        jz      .Ret
        cmp     al,0xF2         ; repnz
        jz      .Ret
        cmp     al,0xF3         ; rep(z)
        jz      .Ret
        cmp     al,0x2E         ; cs:
        jz      .Ret
        cmp     al,0x36         ; ss:
        jz      .Ret
        cmp     al,0x3E         ; ds:
        jz      .Ret
        cmp     al,0x26         ; es:
    .Ret:
        ret

OnResume:
        mov     esi,[CurArg]
        cmp     byte [esi],0
        jz      GoOn
        call    CalcExpression
        jc      .Ret
    .Jmp:
        mov     eax,ebp
        push    eax
        call    FindEnabledBreakPoint
        pop     eax
        jz      GoOn
        mov     bl,5   ; valid enabled one-shot
        call    AddBreakPoint
        jnc     GoOn
        mov     esi,aBreakpointLimitExceeded
        call    PutMessage
    .Ret:
        ret

;-----------------------------------------------------------------------------
;                                 Jump event
OnJump: mov     esi,[CurArg]
        cmp     byte [rsi],0
        jz      .Ret
        call    CalcExpression
        jc      .Ret
        mov     ecx,[CurrentWindow]
        mov     [CodeAddr+rcx*4],ebp
        jecxnz  .Next
        mov     [CPUYPos],1
.Next:
        cmp     ecx,1           ;Reg Window?
        jnz     .NReg
        mov     eax,[RegXPtr]
        cmp     eax,9
        jae     .NReg
        mov     eax,[RegRTab+rax*4]
        mov     [rax],ebp
        call    SetContext
.NReg:
        mov     [AfterKey],1
        call    ShowImage
.Ret:   ret

;-----------------------------------------------------------------------------
;                                 Help event
OnHelp: mov     esi,HelpMsg
        mov     edi,[CurArg]
        cmp     byte [edi],0
        jz      .X
        mov     esi,HelpGroups
        call    FindCmd
        jc      .NoCmd
        mov     esi,[esi+12]
    .X:
        jmp     PutMessage

    .NoCmd:
        mov     esi,aUnknownCommand
        jmp     .X

;-----------------------------------------------------------------------------
;                        Detach process event
OnDetach:
        mcall    69,3,[DebuggeePID]
        and     [DebuggeePID],0
        call    FreeSymbols
        mov     esi,aContinued
        jmp     PutMessage

;-----------------------------------------------------------------------------
;                         Reload executable event
OnReLoad:
        cmp     [DebuggeePID],0
        jnz     TerminateReLoad
        mov     esi,NeedDebuggee
        cmp     byte [LoadName],0
        jnz     DoReLoad
        jz      PutMessage

TerminateReLoad:
        mov     [bReload],1
;-----------------------------------------------------------------------------
;                        Terminate process event
OnTerminate:
        mcall   69,8,[DebuggeePID]
        ret

;-----------------------------------------------------------------------------
;                         Suspend process event
AfterSuspend:
        mov     [bSuspended],1
        call    GetNewContext
        call    GetDump
        call    ShowImage
        ret

OnSuspend:
        mcall   69,4,[DebuggeePID]
        call    AfterSuspend
        mov     esi,aSuspended
        jmp     PutMessage
GoOn:
    ; test for enabled breakpoint at eip
        mov     eax,[_EIP]
        call    FindEnabledBreakPoint
        jnz     .NoBreak
    ; temporarily disable breakpoint, make step, enable breakpoint, continue
        inc     eax
        mov     [TempBreak],eax
        mov     [bAfterGo],1
        dec     eax
        call    DisableBreakPoint
        call    GetContext
        or      byte [_EFL+1],1         ; set TF
        call    SetContext
        and     byte [_EFL+1],not 1
        call    DoResume
        ret
    .NoBreak:
        call    DoResume
ShowImage:
        pushad
        call    InitConsole
        call    DrawWindow
        popad
        ret

even 4
InitConsole:
        mov     eax,[CurWidth]
        mul     [CurHeight]
        mov     ecx,eax
        mov     edi,ConsoleDataPtr
        mov     eax,3F203F20h
        shr     ecx,1
        rep     stosd
        adc     cl,cl
        rep     stosw
        ;
        mov     ebx,MSG_HEIGHT                  ;dY
        mov     ecx,[CurWidth]                  ;\dX
        sub     ecx,2                           ;/
        mov     esi,1                           ;X
        mov     edi,MinHeight-MSG_HEIGHT-1      ;Y
        mov     edx,87208720h
        call    ClearWindow
        ;
        call    DrawFrame
        ret

InitCoords:
        mov     [CPUXPos],1
        mov     [CPUYPos],1
        mov     [CPUXPtr],0
        mov     [MemXPos],11
        mov     [MemYPos],41
        mov     eax,[CurWidth]
        SUB     eax,21
        mov     [StkXPos],eax
        mov     [StkYPos],28
        mov     [RegXPos],1
        mov     [RegYPos],30
        mov     [RegLDat],12
        and     [RegXPtr],0
        ret

GetDataByte:
;I: esi - address
;O: al  - byte
        pushad
        mov     al,0
        push    eax
        mcall   69,6,[DebuggeePID],1,[All.esi+4],esp
if 1
        mov     esi,[All.esi+4]
        mov     edi,esp
        call    RestoreFromBreaksOne
end if
        cmp     eax,-1  ;invalid PID?
        jz      @F      ;Yes,CF=0
        cmp     eax,1   ;CF=1,eax=1 if OK
        cmc
@@:     pop     dword [esp+1Ch]
        popad
        ret

SafeStoreDD:
        pushad
        mov     eax,69
        mov     ebx,7
        mov     ecx,[DebuggeePID]
        mov     edx,4
        mov     esi,edi
        lea     edi,[All.eax]
        int     40h
        popad
        ret

SafeStore:
        pushad
        mov     eax,69
        mov     ebx,7
        mov     ecx,[DebuggeePID]
        mov     edx,1
        mov     esi,edi
        lea     edi,[All.eax]
        int     40h
        popad
        ret

DrawWindow:
        btr     dword [DoDraw],0
        jc      .CopyDraw
        mov     esi,ConsoleDataPtr
        mov     edi,ConsoleDataOld
        mov     ecx,[CurWidth]
        imul    ecx,[CurHeight]
        shr     ecx,1
        repz    cmpsd
        jnz     .CopyDraw
        adc     cl,cl
        repz    cmpsw
        jz      .SkipDraw
.CopyDraw:
        mov     esi,ConsoleDataPtr
        mov     edi,ConsoleDataOld
        mov     ecx,[CurWidth]
        imul    ecx,[CurHeight]
        shr     ecx,1
        rep     movsd
        adc     cl,cl
        rep     movsw

        push    SF_REDRAW
        pop     eax
        push    SSF_BEGIN_DRAW
        pop     ebx
        int     40h
        mov     al,SF_STYLE_SETTINGS
        mov     bl,SSF_GET_SKIN_HEIGHT
        int     40h
        mov     [SkinH],eax
        mov     ebx,[CurWidth]
        imul    ebx,FontWidth
        add     ebx,100*65536+5*2-1
        mov     ecx,[CurHeight]
        imul    ecx,FontHeight
        lea     ecx,[eax+ecx+5-1+100*65536]
        xor     eax,eax
        mov     edx,0x53000000

        mov     edi,HeaderN
        cmp     [DebuggeePID],0
        jz      @F
        mov     edi,HeaderY
    @@: int     40h

        mov     al,SF_DRAW_RECT
        xor     edx,edx
        cmp     [FillWidth],0
        jz      @F
        mov     ebx,[WndWidth]
        sub     ebx,[FillWidth]
        sub     ebx,5-1
        shl     ebx,16
        mov     bx,word [FillWidth]
        mov     ecx,[SkinH-2]
        mov     cx,word [WndHeight]
        sub     cx,word [SkinH]
        sub     cx,5-1
        int     40h
@@:
        cmp     [FillHeight],0
        jz      @F
        xor     edx,edx
        mov     ebx,50000h
        mov     bx,word [WndWidth]
        sub     ebx,9
        mov     ecx,[WndHeight]
        sub     ecx,[FillHeight]
        sub     ecx,5-1
        shl     ecx,16
        mov     cx,word [FillHeight]
        int     40h
@@:
        call    DrawImage
        mov     al,SF_PUT_IMAGE_EXT
        mov     ebx,[MemForImage]
        test    ebx,ebx
        jz      @F
        mov     ecx,[CurWidth]
        imul    ecx,FontWidth*10000h
        mov     cx,word [CurHeight]
        imul    cx,FontHeight
        mov     edx,[SkinH]
        add     edx,5*10000h
        mov     esi,8
        mov     edi,ConsoleColors
        xor     ebp,ebp
        int     40h
@@:
        mov     al,SF_REDRAW
        push    SSF_END_DRAW
        pop     ebx
        int     40h
.SkipDraw:
        ret

align 16
DrawImage:
        mov     [bMemForImageValidData],byte 1
        cmp     [MemForImage],0
        jnz     .Allocated
; allocate memory for image
        mov     ecx,[CurWidth]
        imul    ecx,[CurHeight]
        imul    ecx,FontWidth*FontHeight
        call    PGAlloc
        test    eax,eax
ifz     ret
        mov     [MemForImage],eax
        mov     [bMemForImageValidData],byte 0
.Allocated:
        push    ebp
        and     [MaxX],0
        or      [MinX],-1
        and     [MaxY],0
        or      [MinY],-1
        mov     eax,[CursorY]
        mul     [CurWidth]
        add     eax,[CursorX]
        add     eax,eax
        add     eax,ConsoleDataPtr
        xchg    [CurCursorPos],eax
        mov     [OldCursorPos],eax
        mov     edi,[MemForImage]
        mov     esi,ConsoleDataPtr
        mov     ecx,[CurHeight]
.LH:
        push    ecx
        mov     ecx,[CurWidth]
.LW:
        push    ecx
        mov     eax,[CurWidth]
        imul    eax,[CurHeight]
        mov     ebx,[esi]
        cmp     [bMemForImageValidData],0
        jz      @F
        cmp     esi,[CurCursorPos]
        jz      @F
        cmp     esi,[OldCursorPos]
        jz      @F
        cmp     bx,[eax*2+esi]
        jnz     @F
        inc     esi
        inc     esi
        jmp     .SkipSymbol
@@:
        mov     [eax*2+esi],bx
        cmp     ecx,[MinX]
        ja      @F
        mov     [MinX],ecx
@@:
        cmp     ecx,[MaxX]
        jb      @F
        mov     [MaxX],ecx
@@:
        mov     eax,[esp+4]
        mov     [MinY],eax
        cmp     eax,[MaxY]
        jb      @F
        mov     [MaxY],eax
@@:
        push    edi
        xor     eax,eax
        mov     al,[esi+1]
        and     al,0xF
        mov     ebx,eax
        mov     al,[esi+1]
        shr     al,4
        mov     ebp,eax
        sub     ebx,ebp
        lodsb
        inc     esi
if FontWidth > 8
        lea     edx,[eax+eax+Font]
else
        lea     edx,[eax+Font]
end if
.SH:
        mov     ecx,[edx]
repeat FontWidth
        shr     ecx,1
        sbb     eax,eax
        and     eax,ebx
        add     eax,ebp
        mov     [edi+%-1],al
end repeat
        mov     eax,[CurWidth]
if FontWidth = 6
        lea     eax,[eax*2+eax]
        lea     edi,[edi+eax*2]
else if FontWidth = 7
        lea     edi,[edi+eax*8]
        sub     edi,eax
else if FontWidth = 8
        lea     edi,[edi+eax*8]
else if FontWidth = 9
        lea     edi,[edi+eax*8]
        add     edi,eax
else if FontWidth = 10
        lea     eax,[eax*4+eax]
        lea     edi,[edi+eax*2]
else
Unknown FontWidth Value!
end if
if FontWidth > 8
        add     edx,256*2
        cmp     edx,Font+256*2*FontHeight
else
        add     edx,256
        cmp     edx,Font+256*FontHeight
end if
        jb      .SH
        pop     edi
.SkipSymbol:
        pop     ecx
        add     edi,FontWidth
        dec     ecx
        jnz     .LW
        mov     eax,[CurWidth]
        imul    eax,(FontHeight-1)*FontWidth
        add     edi,eax
        pop     ecx
        dec     ecx
        jnz     .LH
; cursor
        mov     eax,[CursorY]
        inc     eax
        jz      .NoCursor
        mul     [CurWidth]
        imul    eax,FontHeight*FontWidth
        mov     edx,[CursorX]
        inc     edx
        imul    edx,FontWidth
        add     eax,edx
        add     eax,[MemForImage]
        mov     edx,[CurWidth]
        imul    edx,FontWidth
        neg     edx
        mov     ecx,[CursorSize]
.CursorLoop:
        push    ecx
        mov     ecx,FontWidth
        add     eax,edx
        push    eax
@@:
        xor     byte [eax-1],7
        sub     eax,1
        loop    @B
        pop     eax
        pop     ecx
        loop    .CursorLoop
.NoCursor:
        cmp     [MinY],-1
        jz      .NoDraw
        mov     ecx,[CurWidth]
        mov     ebx,[CurHeight]
        mov     eax,ebx
        sub     ebx,[MaxY]
        sub     eax,[MinY]
        sub     eax,ebx
        inc     eax
        imul    ebp,eax,FontHeight
        mov     edx,ecx
        sub     edx,[MaxX]
        imul    edx,FontWidth
        mov     eax,edx
        shl     edx,16
        imul    dx,bx,FontHeight
        imul    ebx,[CurWidth]
        mov     ecx,[MaxX]
        sub     ecx,[MinX]
        inc     ecx
        imul    ecx,FontWidth*10000h
        add     ecx,ebp
        imul    ebx,FontWidth*FontHeight
        add     ebx,[MemForImage]
        add     ebx,eax
        add     edx,[SkinH]
        add     edx,5*10000h
        imul    esi,[CurWidth],FontWidth
        mov     ebp,ecx
        shr     ebp,16
        sub     esi,ebp
        mov     ebp,esi
        push    SF_PUT_IMAGE_EXT
        pop     eax
        mov     edi,ConsoleColors
        push    8
        pop     esi
        int     40h
.NoDraw:
        pop     ebp
        ret
even 4
Redraw:
        mov     al,SF_THREAD_INFO
        mov     ebx,ProcInfo
        or      ecx,-1
        int     40h
; test if rolled up
; height of rolled up window is [skinh]+3
        mov     eax,[ebx+46]
        sub     eax,[SkinH]
        cmp     eax,5
        ja      @F
        mov     al,SF_REDRAW
        push    SSF_BEGIN_DRAW
        pop     ebx
        int     0x40
        xor     eax,eax
; ebx, ecx, edi are ignored by function 0 after first redraw
        mov     edx,0x53000000
        int     0x40
        mov     al,SF_REDRAW
        inc     ebx
        int     0x40
        jmp     WaitEvent
@@:
        xor     ecx,ecx
        mov     eax,[ebx+42]
        mov     [WndWidth],eax
        sub     eax,5*2-1
        jae     @F
        xor     eax,eax
@@:
        cdq
        mov     esi,FontWidth
        div     esi
        cmp     eax,MinWidth
        jae     @F
        mov     al,MinWidth
        mov     ch,1
@@:
        cmp     eax,MaxWidth
        jbe     @F
        mov     eax,MaxWidth
        mov     ch,1
@@:
        cmp     eax,[CurWidth]
        mov     [CurWidth],eax
        setnz   cl
        or      cl,ch
        test    edx,edx
        mov     [FillWidth],edx
        setnz   ch
        mov     eax,[ebx+46]
        mov     [WndHeight],eax
        sub     eax,[SkinH]
        sub     eax,5-1
        jns     @F
        xor     eax,eax
@@:
        cdq
        mov     esi,FontHeight
        div     esi
        cmp     eax,MinHeight
        jae     @F
        mov     al,MinHeight
        mov     cl,1
@@:
        cmp     eax,MaxHeight
        jbe     @F
        mov     eax,MaxHeight
        mov     cl,1
@@:
        mov     [FillHeight],edx
        cmp     eax,[CurHeight]
        mov     [CurHeight],eax
        jnz     .ReSize
        test    cl,cl
        jnz     .ReSize
        test    edx,edx
        setnz   cl
        or      cl,ch
        jz      @F
        test    byte [ebx+70],1
        jnz     @F
.ReSize:
        push    SF_CHANGE_WINDOW
        pop     eax
        or      ebx,-1
        or      ecx,-1
        mov     edx,[CurWidth]
        imul    edx,FontWidth
        add     edx,5*2-1
        mov     esi,[CurHeight]
        imul    esi,FontHeight
        add     esi,[SkinH]
        add     esi,5-1
        int     40h
.ReSizeDraw:
        mov     ecx,[MemForImage]
        call    PGFree
        and     [MemForImage],0
        call    InitConsole
        call    InitCoords
        jmp     WaitEvent
@@:     mov     [DoDraw],1
        call    DrawWindow
        jmp     WaitEvent

even 4
Start:
        xor     eax,eax
        mov     edi,NeedZeroStart
        mov     ecx,(NeedZeroEnd-NeedZeroStart+3)/4
        rep     stosd

        call    DetectCPU
        mov     [CPUType],dl
        call    DetectFPU
        mov     [FPUType],al

        cmp     [CPUType],5
        jb      @F
        xor     eax,eax
        cpuid
        mov     edi,MsgXMMX+4
        mov     [edi+0],ebx
        mov     [edi+4],edx
        mov     [edi+8],ecx
        cmp     eax,1
        jl      .L1
        xor     eax,eax
        inc     eax
        cpuid
        xchg    eax,edx
        jmp     .L2
.L1:    xor     eax,eax
.L2:    test    eax,00800000h   ; CPU have MMX?
        setnz   [MMXType]
        test    eax,02000000h   ; CPU have SSE?
        setnz   [XMMType]
      @@:

        push    SF_SYS_MISC
        pop     eax
        push    SSF_HEAP_INIT
        pop     ebx
        int     40h

        call    InitCmdHistBuffer
        call    InitDefault

        call    InitCoords
        call    ShowImage

        ; set event mask - default events and debugging events
        mcall   40,EVM_REDRAW or EVM_KEY or EVM_BUTTON or EVM_DEBUG
        ; set debug messages buffer
        mov     ecx,DbgBufSize
        mov     dword [ecx],256
        xor     ebx,ebx
        mov     [ecx+4],ebx
        mov     al,69
        mcall

        call    ShowImage

WaitEvent:
        push    SF_WAIT_EVENT
        pop     eax
        int     40h

        cmp     al,9
        jz      DebugMsg
        dec     eax
        jz      Redraw
        dec     eax
        jz      Key
        sub     eax,4
        jz      Mouse
; button - we have only one button, close
;-----------------------------------------------------------------------------
;                                Quit event
OnQuit:
        or      eax,-1
        int     40h
Key:
        mov     al,SF_KEYBOARD
        push    SSF_GET_CONTROL_KEYS
        pop     ebx
        int     0x40
        and     eax,0x3F
        mov     [CtrlState],al

        mov     al,SF_GET_KEY
        int     40h
        test    al,al
        jnz     WaitEvent

        shr     eax,8

        cmp     ah,5Dh
ifz     call    DumpScreen

        cmp     [DebuggeePID],0
        jz      DoCommandLine
        cmp     [CmdLineActive],0
        jnz     GetCommandLine

        jmp     WinSwitch

F7:     cmp     [DebuggeePID],0
        jz      .No
        call    OnStep
    .No:jmp     WaitEvent

F8:     cmp     [DebuggeePID],0
        jz      F7.No
        call    OnProceed
        jmp     F7.No

even 16
Mouse:
if 0
        mov     eax,SF_MOUSE_GET
        mov     ebx,SSF_BUTTON_EXT
        int     0x40
        mov     byte [MouseState],1
        bt      eax,24 ;left but. double click
        jc      @F
        mov     byte [MouseState],0
        bt      eax,8 ;left but. down
        jc      @F
        jmp     WaitEvent
@@:
        mov     eax,SF_MOUSE_GET
        mov     ebx,SSF_WINDOW_POSITION
        int     0x40
end if
        jmp     WaitEvent

DumpScreen:
        pushad
        mov     dword [DumpName+4],'0000'
.1:
        mov     [DumpBlock.Func],SSF_GET_INFO
        mcall   70,DumpBlock
        or      eax,eax
        jnz     .2
        inc     byte [DumpName+7]
        cmp     byte [DumpName+7],'9'
        jbe     .1
        mov     byte [DumpName+7],'0'
        inc     byte [DumpName+6]
        cmp     byte [DumpName+6],'9'
        jbe     .1
        mov     byte [DumpName+6],'0'
        popad
        ret
.2:     mov     [DumpBlock.Func],SSF_CREATE_FILE
        mcall   70,DumpBlock
        or      eax,eax
        jnz     .Err
        mov     ebx,[CurHeight]
        mov     esi,ConsoleDataPtr
        mov     edi,ConsoleDataOld
.3:     mov     ecx,[CurWidth]
.4:     mov     al,[rsi]
        test    al,al
        jz      .5
        cmp     al,0Ah
        jz      .5
        cmp     al,0Dh
        jz      .5
        cmp     al,10h
        jb      .X
        jmp     .6
.5:     mov     al,20h
        jmp     .6
.X:     mov     al,'.'
.6:     mov     [rdi],al
        add     esi,2
        inc     edi
        loop    .4
        mov     byte [rdi],13
        inc     edi
        dec     ebx
        jnz     .3
        mov     ecx,[CurWidth]
        inc     ecx
        imul    ecx,[CurHeight]
        mov     [DumpBlock.Size],ecx
        mov     [DumpBlock.Func],SSF_WRITE_FILE
        mcall   70,DumpBlock
.Err:   popad
        ret

AfterGoException:
        push    eax
        mov     eax,[TempBreak]
        dec     eax
        push    esi
        call    EnableBreakPoint
    ; in any case, clear TF and RF
        call    GetNewContext
        and     [_EFL],not 10100h       ; clear TF,RF
        call    SetContext
        xor     edx,edx
        mov     [TempBreak],edx
        xchg    dl,[bAfterGo]
        pop     esi
        pop     eax
        cmp     dl,2
        jnz     @F
        lodsd
        push    esi
        call    GetDump
        jmp     Exception.Done
    @@:
        test    eax,eax
        jz      .NotInt1
    ; if exception is result of single step, simply ignore it and continue
        test    dword [esi],0xF
        jnz     DbgMsgStart.5
        lodsd
        push    esi
        mov     esi,OldContext
        mov     edi,Context
        mov     ecx,28h/4
        rep     movsd
        call    DoResume
        jmp     DbgMsgEnd
    .NotInt1:
    ; in other case, work as without temp_break
        lodsd
        push    esi
        push    eax
        jmp     Exception.4

    .NoTour:

DebugMsg:
        neg     [DbgBufSize]
        mov     esi,DbgBuf
DbgMsgStart:
        lodsd
        add     esi,4
        dec     eax
        jz      Exception
        dec     eax
        jz      Terminated
        dec     eax
        jnz     DbgNotify

        mov     [bSuspended],1
        cmp     [bAfterGo],0
        jnz     AfterGoException
        push    esi
        call    GetNewContext
        and     [_EFL],not 10100h       ; clear TF,RF
        call    SetContext
        pop     esi
    .5:
        push    esi
        call    GetDump
        pop     esi
        lodsd
        xor     ecx,ecx
    .6:
        bt      eax,ecx
        jnc     .7
        mov     ebx,[DrXBreak+ecx*4]
        test    ebx,ebx
        jz      .7
        pushad
        dec     ebx
        push    ebx
        mov     esi,aBreakStop
        call    PutMessageNoDraw
        popad
    .7:
        inc     ecx
        cmp     cl,4
        jb      .6
        push    esi
        jmp     Exception.DoneDraw

DbgNotify:
        int3
        add     esi,32
        push    esi
        jmp     DbgMsgEnd

Terminated:
        push    esi
        mov     esi,TerminatedMsg
        call    PutMessage
        and     [DebuggeePID],0
        and     [TempBreak],0
        mov     [bAfterGo],0
        xor     eax,eax
        mov     ecx,BreakPointsN*6/4+4
        mov     edi,BreakPoints
        rep     stosd
        cmp     [bReload],1
        sbb     [bReload],-1
        jnz     Exception.Done
        call    FreeSymbols
        jmp     Exception.Done

Exception:
        mov     [bSuspended],1
        cmp     [bAfterGo],0
        jnz     AfterGoException
        lodsd
        push    esi
        push    eax
        call    GetNewContext
        and     [_EFL],not 10100h       ; clear TF,RF
        call    SetContext
    .4:
        call    GetDump
        pop     eax
    ; int3 command generates exception 0x0D, #GP
        push    eax
        cmp     al,0x0D
        jnz     .NotDbg
    ; check for 0xCC byte at eip
        push    0
        mcall   69,6,[DebuggeePID],1,[_EIP],esp
        pop     eax
        cmp     al,0xCC
        je      .Int3
    ; check for 0xCD03 word at eip
        push    0
        inc     edx
        mcall   69;,6,[DebuggeePID],2,[_EIP],esp
        pop     eax
        cmp     ax,0x03CD
        jne     .NotDbg
        mov     eax,[_EIP]
        inc     [_EIP]
        inc     [_EIP]
        jmp     .UserINT3
    .Int3:
    ; this is either dbg breakpoint or int3 cmd in debuggee
        mov     eax,[_EIP]
        call    FindEnabledBreakPoint
        jnz     .UserINT3
    ; dbg breakpoint; clear if one-shot
        pop     ecx
        push    eax
        mov     esi,aBreakStop
        test    byte [edi+4],4
        jz      .PutMsgEAX
        pop     ecx
        call    ClearBreakPoint
        jmp     .Done

    .UserINT3:
        mov     eax,[_EIP]
        inc     [_EIP]
    .UserINT3_:
        pop     ecx
        push    eax
        call    SetContext
        mov     esi,aUserBreak
        jmp     .PutMsgEAX

    .NotDbg:
        pop     eax
        push    eax
        push    eax
        mov     esi,aException
        call    PutMessageNoDraw
        pop     eax

        cmp     al,16
        ja      .Suspended
        mov     esi,[MsgFaultSel+eax*4]
    .ShowMess:
        call    PutMessageNoDraw
    .Suspended:
        mov     esi,aSuspended
    .PutMsgEAX:
        call    PutMessageNoDraw
    .DoneDraw:
        call    DrawMessages
    .Done:
        mcall   18,3,[DbgWnd]   ; activate dbg window
        call    ShowImage

DbgMsgEnd:
        pop     esi
        mov     ecx,[DbgBufLen]
        add     ecx,DbgBuf
        cmp     esi,ecx
        jnz     DbgMsgStart
        and     [DbgBufLen],0
        neg     [DbgBufSize]
        cmp     [bReload],2
        jnz     @F
        mov     [bReload],0
        call    DoReLoad
    @@:
        jmp     WaitEvent

;-----------------------------------------------------------------------------
;                        Add breakpoint
; in: EAX = address; BL = flags
; out: CF = 1 => error
;      CF = 0 and EAX = breakpoint number
AddBreakPoint:
        xor     ecx,ecx
        mov     edi,BreakPoints
    @@:
        test    byte [edi+4],1
        jz      .Found
        add     edi,6
        inc     ecx
        cmp     ecx,BreakPointsN
        jb      @B
        stc
        ret
    .Found:
        stosd
        xchg    eax,ecx
        mov     [edi],bl
        test    bl,2
        jnz     @F
        or      byte [edi],2
        push    eax
        call    EnableBreakPoint
        pop     eax
    @@:
        clc
        ret
;-----------------------------------------------------------------------------
;                         Remove breakpoint
ClearBreakPoint:
        cmp     eax,BreakPointsN
        jae     .Ret
        mov     ecx,4
        inc     eax
    .1:
        cmp     [DrXBreak-4+ecx*4],eax
        jnz     @F
        and     [DrXBreak-4+ecx*4],0
    @@:
        loop    .1
        dec     eax
        push    eax
        add     eax,eax
        lea     edi,[BreakPoints+eax+eax*2+4]
        test    byte [edi],1
        pop     eax
        jz      .Ret
        push    edi
        call    DisableBreakPoint
        pop     edi
        mov     byte [edi],0
    .Ret:
        ret
;-----------------------------------------------------------------------------
;                          Disable breakpoint
DisableBreakPoint:
        cmp     eax,BreakPointsN
        jae     .Ret
        add     eax,eax
        lea     edi,[BreakPoints+eax+eax*2+5]
        test    byte [edi-1],1
        jz      .Ret
        test    byte [edi-1],2
        jnz     .Ret
        or      byte [edi-1],2
        test    byte [edi-1],8
        jnz     .DR
        push    esi
        mcall   69,7,[DebuggeePID],1,[edi-5]
        pop     esi
    .Ret:
        ret
    .DR:
        mov     dl,[edi]
        shr     dl,6
        mov     dh,80h
        mcall   69,9,[DebuggeePID]
        ret
;-----------------------------------------------------------------------------
;                           Enable breakpoint
EnableBreakPoint:
        push    esi
        cmp     eax,BreakPointsN
        jae     .Ret
        add     eax,eax
        lea     edi,[BreakPoints+eax+eax*2+5]
        test    byte [edi-1],1
        jz      .Ret
        test    byte [edi-1],2
        jz      .Ret
        and     byte [edi-1],not 2
        test    byte [edi-1],8
        jnz     .DR
        mcall   69,6,[DebuggeePID],1,[edi-5]
        dec     eax
        jnz     .Err
        push    0xCC
        mov     edi,esp
        inc     ebx
        mcall   69
        pop     eax
    .Ret:
        pop     esi
        ret
    .Err:
        or      byte [edi-1],2
        mov     esi,aBreakErr
        call    PutMessage
        pop     esi
        ret
    .DR:
        mov     esi,[edi-5]
        mov     dl,[edi]
        shr     dl,6
        mov     dh,[edi]
        and     dh,0xF
        mcall   69,9,[DebuggeePID]
        test    eax,eax
        jnz     .Err
        pop     esi
        ret
;-----------------------------------------------------------------------------
;                             Find breakpoint
FindBreakPoint:
        xor     ecx,ecx
        xchg    eax,ecx
        mov     edi,BreakPoints
    @@:
        test    byte [edi+4],1
        jz      .Cont
        test    byte [edi+4],8
        jnz     .Cont
        cmp     [edi],ecx
        jz      .Found
    .Cont:
        add     edi,6
        inc     eax
        cmp     eax,BreakPointsN
        jb      @B
        or      eax,-1
    .Found:
        ret
;-----------------------------------------------------------------------------
;
FindEnabledBreakPoint:
        xor     ecx,ecx
        xchg    eax,ecx
        mov     edi,BreakPoints
    @@:
        test    byte [edi+4],1
        jz      .Cont
        test    byte [edi+4],2 or 8
        jnz     .Cont
        cmp     [edi],ecx
        jz      .Found
    .Cont:
        add     edi,6
        inc     eax
        cmp     eax,BreakPointsN
        jb      @B
        or      eax,-1
    .Found:
        ret

GetDump:
if 0
        mov     edi,DumpData
        mov     esi,[edi-4]
        mov     edx,DUMP_HEIGHT*10h
        mov     ecx,edx
        xor     eax,eax
        push    edi
        rep     stosb
        pop     edi
        mcall   69,6,[DebuggeePID]
        cmp     eax,-1
        jnz     @F
        mov     esi,ReadMemErr
        call    PutMessage
        xor     eax,eax
    @@:
        mov     [edi-8],eax
; in: edi=buffer,eax=size,esi=address
RestoreFromBreaks:
        mov     ebx,BreakPoints
    @@:
        test    byte [ebx+4],1
        jz      .Cont           ; ignore invalid
        test    byte [ebx+4],2 or 8
        jnz     .Cont           ; ignore disabled and memory breaks
        mov     ecx,[ebx]
        sub     ecx,esi
        cmp     ecx,eax
        jae     .Cont
        mov     dl,[ebx+5]
        mov     [edi+ecx],dl
    .Cont:
        add     ebx,6
        cmp     ebx,BreakPoints+BreakPointsN*6
        jb      @B
end if
        ret

; in: edi=buffer,esi=address
RestoreFromBreaksOne:
        mov     ebx,BreakPoints
    @@:
        test    byte [ebx+4],1
        jz      .Cont           ; ignore invalid
        test    byte [ebx+4],2 or 8
        jnz     .Cont           ; ignore disabled and memory breaks
        mov     ecx,[ebx]
        sub     ecx,esi
        cmp     ecx,1
        jae     .Cont
        mov     dl,[ebx+5]
        mov     [edi+ecx],dl
        jmp     .Exit
    .Cont:
        add     ebx,6
        cmp     ebx,BreakPoints+BreakPointsN*6
        jb      @B
    .Exit:
        ret

GetCommandLine:
        cmp     ah,0x01
        jz      .Esc
        cmp     al,8
        jz      .Backspace
        cmp     al,0xB0
        jz      .Left
        cmp     al,0xB3
        jz      .Right
        cmp     al,0x0D
        jz      .Enter
        cmp     al,0xB6
        jz      .Del
        cmp     al,0xB4
        jz      .Home
        cmp     al,0xB5
        jz      .End
        cmp     al,0xB1
        jz      .Dn
        cmp     al,0xB2
        jz      .Up
        mov     [CmdHistBuffer.TmpLineFlag],0
        cmp     [CmdLineLen],CMD_WIDTH
        jae     WaitEvent
        push    eax
        call    ClearCmdLineEnd
        pop     eax
        mov     edi,CmdLine
        mov     ecx,[CmdLineLen]
        add     edi,ecx
        lea     esi,[edi-1]
        sub     ecx,[CmdLinePos]
        std
        rep     movsb
        cld
        stosb
        inc     [CmdLineLen]
        call    DrawCmdLineEnd
        inc     [CmdLinePos]
        call    DrawCursor
        jmp     WaitEvent


    .Esc:
        xor     eax,eax
        mov     [CmdLinePos],eax
        mov     [CmdLineLen],eax
        mov     [CmdLineActive],al
        call    DrawCursor
        jmp     WaitEvent

    .Backspace:
        mov     [CmdHistBuffer.TmpLineFlag],0
        cmp     [CmdLinePos],0
        jz      WaitEvent
        dec     [CmdLinePos]

    .DelChar:
        mov     [CmdHistBuffer.TmpLineFlag],0
        call    ClearCmdLineEnd
        mov     edi,[CmdLinePos]
        dec     [CmdLineLen]
        mov     ecx,[CmdLineLen]
        sub     ecx,edi
        add     edi,CmdLine
        lea     esi,[edi+1]
        rep     movsb
        call    DrawCmdLineEnd
        call    DrawCursor
        jmp     WaitEvent

    .Del:
        mov     eax,[CmdLinePos]
        cmp     eax,[CmdLineLen]
        jae     WaitEvent
        jmp     .DelChar

    .Left:
        cmp     [CmdLinePos],0
        jz      WaitEvent
        call    HideCursor
        dec     [CmdLinePos]
        call    DrawCursor
        jmp     WaitEvent

    .Right:
        mov     eax,[CmdLinePos]
        cmp     eax,[CmdLineLen]
        jae     WaitEvent
        call    HideCursor
        inc     [CmdLinePos]
        call    DrawCursor
        jmp     WaitEvent

    .Home:
        call    HideCursor
        and     [CmdLinePos],0
        call    DrawCursor
        jmp     WaitEvent

    .End:
        call    HideCursor
        mov     eax,[CmdLineLen]
        mov     [CmdLinePos],eax
        call    DrawCursor
        jmp     WaitEvent

        .Up:
                xor     edx,edx
                jmp     .Hist
        .Dn:
                xor     edx,edx
                inc     edx
        .Hist:
                cmp     [CmdHistBuffer.TmpLineFlag],1
                je      @F
                mov     eax,CmdLine
                mov     ecx,[CmdLineLen]
                mov     byte [eax+ecx],0
                call    AddCmdHistTmpLine
        @@:
                test    edx,edx
                jnz     .Hist.Next
                cmp     [CmdHistBuffer.NFlag],0
                jne     @F
                call    GetCmdHistLine
                inc     [CmdHistBuffer.NFlag]
                jmp     .Hist.CPLine

        @@:
                call    PrevCmdHistLine
                jc      WaitEvent
                call    GetCmdHistLine
                inc     [CmdHistBuffer.NFlag]
                jmp     .Hist.CPLine

        .Hist.Next:
                cmp     [CmdHistBuffer.NFlag],0
                je      WaitEvent

                call    NextCmdHistLine
                call    GetCmdHistLine
                jnc     .Hist.CPLine
                call    GetCmdHistTmpLine
                mov     [CmdHistBuffer.TmpLineFlag],0
                mov     [CmdHistBuffer.NFlag],0

        .Hist.CPLine:

                mov     esi,eax
                mov     edi,CmdLine
                xor     ecx,ecx
        @@:
                inc     ecx
                lodsb
                stosb
                test    al,al
                jnz     @B

                dec     ecx
                mov     [CmdLinePos],0
                call    ClearCmdLineEnd
                mov     [CmdLineLen],ecx
                mov     [CmdLinePos],ecx
                call    DrawCmdLine
                call    DrawCursor

                jmp     WaitEvent

        ; We also trying to execute previous command, if empty command_line
    .Enter:
        mov     [CmdLineActive],0

        mov     ecx,[CmdLineLen]
        test    ecx,ecx
        jnz     .ExecCur
        mov     cl,byte [CmdLinePrev]
        cmp     cl,0
        jz      WaitEvent

    .ExecPrev:
        mov     esi,CmdLinePrev
        jmp     .Exec

    .ExecCur:
        mov     esi,CmdLine

    .Exec:
        mov     byte [esi+ecx],0

        mov     eax,esi
        call    AddCmdHistLine
        mov     [CmdHistBuffer.NFlag],0

        and     [CmdLinePos],0
        push    esi
        call    ClearCmdLineEnd
        call    DrawCursor
        pop     esi
        and     [CmdLineLen],0
        ; skip leading spaces
        call    GetArg.SkipSpaces
        cmp     al,0
        jz      WaitEvent
        ; now esi points to command
        push    esi
        mov     esi,Prompt
        call    PutMessageNoDraw
        pop     esi
        push    esi
        call    PutMessageNoDraw
Z1:
        mov     esi,NewLine
        call    PutMessage
        pop     esi
        push    esi
        call    GetArg
        mov     [CurArg],esi
        pop     edi
        mov     esi,Commands
        call    FindCmd
        mov     eax,aUnknownCommand
        jc      .X11

        ; check command requirements
        ; flags field:
        ; &1: command may be called without parameters
        ; &2: command may be called with parameters
        ; &4: command may be called without loaded program
        ; &8: command may be called with loaded program
        mov     eax,[esi+8]
        mov     ecx,[CurArg]
        cmp     byte [ecx],0
        jz      .NoArgs
        test    byte [esi+16],2
        jz      .X11
        jmp     @F

    .NoArgs:
        test    byte [esi+16],1
        jz      .X11
    @@:
        cmp     [DebuggeePID],0
        jz      .NoDebuggee
        mov     eax,aAlreadyLoaded
        test    byte [esi+16],8
        jz      .X11
        jmp     .X9

    .NoDebuggee:
        mov     eax,NeedDebuggee
        test    byte [esi+16],4
        jnz     .X9
    .X11:
        xchg    esi,eax
        call    PutMessage
        ; store cmdline for repeating
    .X10:
        mov     esi,CmdLine
        mov     ecx,[CmdLineLen]
    @@:
        or      ecx,ecx
        jle     .We
        mov     al,[esi+ecx]
        mov     [CmdLinePrev+ecx],al
        dec     ecx
        jmp     @B

    .We:
        mov     [CmdLineLen],0
        jmp     WaitEvent

    .X9:
        call    dword [esi+4]
        jmp     .X10

PutMessage:
        call    PutMessageNoDraw
DrawMessages:
        mov     [AfterKey],1
        call    ShowImage
        ret

include "cmdhist.inc"
include "parser.inc"
include "message.inc"

HeaderN db      'SunSys Debugger ver ',VERSION,' - No program loaded',0
HeaderY db      'SunSys Debugger ver ',VERSION,' - ',60 dup 32,0
HeaderE:
;
LoadErrMsg      db      'Cannot load program. ',0
UnkErrMsg       db      'Unknown error code -%4X',10,0
aCannotLoadFile db      'Cannot load file. ',0
UnkErrMsg2    db      'Unknown error code %4X.',10,0
LoadErrMsgs:
        dd      .1,0,.3,0,.5,.6,0,0,.9,.A,0,0,0,0,0,0
        dd      0,0,0,0,0,0,0,0,0,0,0,0,0,.1E,.1F,.20
.1              db      'HD undefined.',10,0
.3              db      'Unknown FS.',10,0
.5              db      'File not found.',10,0
.6              db      'Unexpected EOF.',10,0
.9              db      'FAT table corrupted.',10,0
.A              db      'Access denied.',10,0
.1E             db      'No memory.',10,0
.1F             db      'Not Menuet/Kolibri executable.',10,0
.20             db      'Too many processes.',10,0
LoadSuccMsg     db      'Program loaded successfully! PID=%4X. Use "g" to run.',10,0
TerminatedMsg   db      'Program terminated.',10,0
MsgFaultSel     dd  aDivide,aDebug,aNonMask,aUndefined,aOverflow
                dd  aBounds,aInvalid,aCoProcessorNA,aDoubleFault
                dd  aUndefined,aInvalidTSS,aSegment,aStack
                dd  aProtection,aPageFault,aUndefined,aCoProcessor
aDivide         db      '(Divide error)',10,0
aDebug          db      '(Single-step/debug exception)',10,0
aNonMask        db      '(Nonmaskable interrupt)',10,0
aOverflow       db      '(Overflow)',10,0
aBounds         db      '(Bounds check)',10,0
aInvalid        db      '(Invalid opcode)',10,0
aCoProcessorNA  db      '(Coprocessor not available)',10,0
aDoubleFault    db      '(Double fault)',10,0
aUndefined      db      '(Undefined fault)',10,0
aInvalidTSS     db      '(Invalid TSS)',10,0
aSegment        db      '(Segment not present)',10,0
aStack          db      '(Stack fault)',10,0
aProtection     db      '(General protection fault)',10,0
aPageFault      db      '(Page fault)',10,0
aCoProcessor    db      '(Coprocessor error)',10,0
aSuspended      db      'Suspended',10,0
aContinued      db      'Continuing',10,0
aRunningErr     db      'Program is running',10,0
aException      db      'Debugged program caused an exception %2X. ',0
aBreakErr       db      'Cannot activate breakpoint, it will be disabled',10,0
aDuplicateBreakpoint db 'Duplicate breakpoint',10,0
aInvalidBreak   db      'Invalid breakpoint number',10,0
aBreakNum       db      '%2X: at %8X',0
aMemBreak1      db      '%2X: on ',0
aMemBreak2      db      'read from ',0
aMemBreak3      db      'access of ',0
aMemBreak4      db      'byte',0
aMemBreak5      db      'word',0
aMemBreak6      db      'dword',0
aMemBreak7      db      ' at %8X',0
aOneShot        db      ', one-shot',0
aDisabled       db      ', disabled',0
aBreakStop      db      'Breakpoint #%2X',10,0
aUserBreak      db      'int3 command at %8X',10,0
ReadMemErr      db      'ERROR: cannot read process memory!!!',10,0
aBreakpointLimitExceeded db 'Breakpoint limit exceeded',10,0
aUnknownCommand db      'Unknown command',10,0
NeedDebuggee    db      'No program loaded. Use "load" command.',10,0
aAlreadyLoaded  db      'Program is already loaded. Use "terminate" or "detach" commands',10,0
aParseError     db      'Parse error',10,0
aDivByZero      db      'Division by 0',10,0
CalcString      db      '%8X',10,0
aNoMemory       db      'No memory',10,0
aSymbolsLoaded  db      'Symbols loaded',10,0
aUnaligned      db      'Unaligned address',10,0
aEnabledBreakErr db     'Enabled breakpoints are not allowed',10,0
aInterrupted    db      'Interrupted',10,0
aUnpacked       db      'Unpacked successful!',10,0
OnBeErrMsg      db      'There is already enabled breakpoint on this address',10,0

DumpPath        db      '/TMP0/1/'
DumpName        db      'DUMP0000.TXT',0

even 4
ConsoleColors   dd      0x000000,0x000080,0x008000,0x008080
                dd      0x800000,0x800080,0x808000,0xC0C0C0
                dd      0x1D272F,0x0000FF,0x00FF00,0x00FFFF;0x808080,0x0000FF,0x00FF00,0x00FFFF
                dd      0xFF0000,0xFF00FF,0xFFFF00,0xFFFFFF

CurWidth        dd      MinWidth
CurHeight       dd      MinHeight
FillWidth       dd      0
FillHeight      dd      0

MemForImage     dd      0

bWasE0          db      0
CtrlState       db      0
MouseState      db      0
bMemForImageValidData   db 0

bReload         db      0
bAfterGo        db      0
bSuspended      db      0
CodeType        db      32

DoDraw          db      0
SymbolSection   db      0
CmdLineActive   db      0,?

CursorX         dd      -1
CursorY         dd      -1
CursorSize      dd      CursorNormSize
CurCursorPos    dd      -1
OldCursorPos    dd      -1

DebuggeePID     dd      0

DumpBlock:
.Func           dd      0
                dd      0
                dd      0
.Size           dd      0
                dd      ConsoleDataOld
                db      0
                dd      DumpPath

FN70LoadBlock:  dd 7
                dd 1
LoadParams      dd 0
                dd 0
                dd 0
LoadName:       db 0
                rb 255

FN70ReadBlock:  dd 0
                rq 1
                dd ?
                dd ?
                db 0
                dd ?

FN70AttrBlock:  dd 5
                dd 0,0,0
                dd FileAttr
                db 0
                dd ?

IncludeAllGlobals

DbgWnd          dd      ?
TempBreak       dd      ?

WndWidth        dd      ?
WndHeight       dd      ?

MinY            dd      ?
MaxY            dd      ?
MinX            dd      ?
MaxX            dd      ?

Tmp             dd      ?
SkinH           dd      ?
StdColors       rd      10

AppPath         rb      4096
ProcInfo        rb      1024

even 16
ConsoleDataPtr  rw      (MaxWidth+0)*MaxHeight
even 16
ConsoleDataOld  rw      (MaxWidth+1)*MaxHeight

even 16
NeedZeroStart:

DbgBufSize      dd      ?
DbgBufLen       dd      ?
DbgBuf          rb      256

FileAttr        rb      40

even 4
Context:
_EIP    dd ?
_EFL    dd ?
_EAX    dd ?
_ECX    dd ?
_EDX    dd ?
_EBX    dd ?
_ESP    dd ?
_EBP    dd ?
_ESI    dd ?
_EDI    dd ?

_CTX_FLAGS:
        dd ?
        dd ?

_SSE_FLAGS:

_FCW    dw ?
_FSW    dw ?
_FTW    db ?
        db ?
_FOP    dw ?
_FPU_IP dd ?
        dw ?
        dw ?

_FPU_DP dd ?
        dw ?
        dw ?

_MXCSR  dd ?
_MXCSRM dd ?

FPU_CONTEXT:
MMX_CONTEXT:
_ST0:
_MM0:   rq 2
_ST1:
_MM1:   rq 2
_ST2:
_MM2:   rq 2
_ST3:
_MM3:   rq 2
_ST4:
_MM4:   rq 2
_ST5:
_MM5:   rq 2
_ST6:
_MM6:   rq 2
_ST7:
_MM7:   rq 2

SSE_CONTEXT:
_XMM0   rq 2
_XMM1   rq 2
_XMM2   rq 2
_XMM3   rq 2
_XMM4   rq 2
_XMM5   rq 2
_XMM6   rq 2
_XMM7   rq 2
CtxEnd:

OldContext      rb      (CtxEnd-Context)

Plus = (OldContext-Context)

StepNum         dd      ?
ProcNum         dd      ?
if 0
DUMP_HEIGHT     = 6                     ; in text lines
;
DumpRead        dd      ?
DumpPos         dd      ?
DumpData        rb      DUMP_HEIGHT*10h
end if

CmdLine         rb      CMD_WIDTH+1
CmdLineLen      dd      ?
CmdLinePos      dd      ?
CurArg          dd      ?

CmdLinePrev     rb      CMD_WIDTH+1

SymbolsFile     rb      260

PrgNamePtr      dd      ?
PrgNameLen      dd      ?

Symbols         dd      ?
NumSymbols      dd      ?

CmdHistBuffer:
.Ptr                    dd      ?
.Size                   dd      ?
.LastNodeOffset         dd      ?
.NewNodeOffset          dd      ?
.CurNodeOffset          dd      ?
.TmpLineFlag            db      ?
.NFlag                  db      ?

; breakpoint structure:
; dword +0: address
; byte +4: flags
; bit 0: 1 <=> breakpoint valid
; bit 1: 1 <=> breakpoint disabled
; bit 2: 1 <=> one-shot breakpoint
; bit 3: 1 <=> DRx breakpoint
; byte +5: overwritten byte
;          for DRx breaks: flags + (index shl 6)
BreakPointsN = 256
BreakPoints     rb      BreakPointsN*6
DrXBreak        rd      4
;-----------------------------------------------------------------------------
CurrentWindow   dd      ?
CPUXPos         dd      ?               ; coordinates of cursor in windows
CPUYPos         dd      ?
CPUXPtr         dd      ?
MemXPos         dd      ?
MemYPos         dd      ?
StkXPos         dd      ?
StkYPos         dd      ?
RegXPos         dd      ?
RegYPos         dd      ?
RegLDat         dd      ?
RegXPtr         dd      ?
CPUType         db      ?
FPUType         db      ?
MMXType         db      ?
XMMType         db      ?
JumpTaken       db      ?
AfterKey        db      ?
;-----------------------------------------------------------------------------
TmpB            rb      0
Temp            dd      ?
;-----------------------------------------------------------------------------
CodeAddr        dd      ?       ;0 order
RegsAddr        dd      ?       ;1
DataAddr        dd      ?       ;2
StckAddr        dd      ?       ;3
OriginPtr:      rd      2
AddrBuffer:     rd      30
;-----------------------------------------------------------------------------
NeedZeroEnd:
;-----------------------------------------------------------------------------
BufferI:        rb      1024
BufferO:        rb      1024
;-----------------------------------------------------------------------------
; stack
even 4
                rb      32768
StackTop:
;-----------------------------------------------------------------------------
Mem:
;-----------------------------------------------------------------------------
I_END = UDataStr        ;%v
;-----------------------------------------------------------------------------
