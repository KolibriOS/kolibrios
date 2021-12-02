;-----------------------------------------------------------------------------
OS      =       0
Loader  =       1
;-----------------------------------------------------------------------------
SkipAddr        = 8
;-----------------------------------------------------------------------------
Bitness equ     32
;-----------------------------------------------------------------------------
include "globals!.inc"
include "fasm\fasm.asm"
include "disasm\disasm32.asm"
if ~OS
include "loadmap.inc"
end if
include "assemble.inc"
;-----------------------------------------------------------------------------
xglobal
even 16
StdMemStr       rb      4*1024
StdMemEnd:
AddMemStr       rb      4*1024
AddMemEnd:
endg
;-----------------------------------------------------------------------------
uglobal
even 16
Inst    TInstruction
even 16
DefInst TInstruction
endg
;-----------------------------------------------------------------------------
virtual at 0
        dq      ?
sizeof.dq       = $
end virtual
;---------------------------------------;
JccCalls:                               ;
        seto    al                      ; 0
        ret                             ;
        setno   al                      ; 1
        ret                             ;
        setc    al                      ; 2
        ret                             ;
        setnc   al                      ; 3
        ret                             ;
        setz    al                      ; 4
        ret                             ;
        setnz   al                      ; 5
        ret                             ;
        setbe   al                      ; 6
        ret                             ;
        seta    al                      ; 7
        ret                             ;
        sets    al                      ; 8
        ret                             ;
        setns   al                      ; 9
        ret                             ;
        setpe   al                      ; A
        ret                             ;
        setpo   al                      ; B
        ret                             ;
        setl    al                      ; C
        ret                             ;
        setge   al                      ; D
        ret                             ;
        setle   al                      ; E
        ret                             ;
        setg    al                      ; F
        ret                             ;
;-----------------------------------------------------------------------------
ShowCPU:
        mov     [XPos],3
        mov     [YPos],0
        mov     edx,Msg00
        call    PrintS
        movzx   edx,[CPUType]
        cmp     dl,6
        jb      @F
        mov     dl,6
      @@:
        lea     edx,[CPUTable+rdx*4]
        jmp     PrintS
;-----------------------------------------------------------------------------
ShowFPU:
        movzx   edx,[CPUType]
        cmp     dl,6
        jb      @F
        mov     dl,6
      @@:
        lea     edx,[FPUTable+rdx*8]
        jmp     PrintS
;-----------------------------------------------------------------------------
DetectCPU:
        mov     dl,1
        push    sp
        pop     ax
        cmp     ax,sp
        jnz     .RetCPU
        inc     edx
        pushfw
        pop     ax
        mov     cx,ax
        xor     ah,40h
        push    ax
        popfw
        pushfw
        pop     ax
        push    cx
        popfw
        xor     ah,ch
        jz      .RetCPU
        inc     edx
        pushfd
        pop     eax
        mov     ecx,eax
        btc     eax,18
        push    eax
        popfd
        pushfd
        pop     eax
        push    ecx
        popfd
        xor     eax,ecx
        jz      .RetCPU
        inc     edx
        mov     eax,ecx
        btc     eax,21
        push    eax
        popfd
        pushfd
        pop     eax
        push    ecx
        popfd
        xor     eax,ecx
        jz      .RetCPU
        inc     edx
        xor     eax,eax
        cpuid
        test    eax,eax
        jz      .RetCPU
        xor     eax,eax
        inc     eax
        cpuid
        mov     dl,ah
        and     dl,0Fh
.RetCPU:ret
;-----------------------------------------------------------------------------
DetectFPU:
        xor     eax,eax
        push    ax
        mov     ebp,esp
        fninit
        fnstcw  word [ebp]
        mov     dx,[ebp]
        cmp     dh,03h
        jnz     .Done           ; done: no FPU present
        inc     eax
        and     word [ebp],0FF7Fh
        wait
        fldcw   word [ebp]
        fdisi
        fstcw   word [ebp]
        wait
        test    byte [ebp],0080h
        jnz     .Done           ; done: 8087 is present
        inc     eax
        fninit
        wait
        fld1
        wait
        fldz
        wait
        fdivp   st1,st0
        wait
        fld     st0
        wait
        fchs
        wait
        fcompp
        wait
        fnstsw  ax
        wait
        fnclex
        wait
        sahf
        jz      .Done           ; done: 80287 is present
        inc     eax
        mov     ah,[CPUType]
        cmp     ah,4
        jb      .Done           ; done: 80387 is present
        mov     al,ah           ; done: 80487+ (build-in)
.Done:  or      al,al
        jz      .Exit
        mov     ecx,8
.Loop:  fldz                    ; set ST(0) to ST(7) to +ZERO
        loop    .Loop
        finit                   ; reinitialize FPU
.Exit:  pop     dx
        ret
;-----------------------------------------------------------------------------
ClearCmdLine:
        mov     [XPos],1
        mov     [YPos],MinHeight-1
        mov     [Color],7
        mov     al,32
        mov     ecx,[CurWidth]
        sub     ecx,2
.Loop:  call    PrintC
        loop    .Loop
        ret
;-----------------------------------------------------------------------------
PrintCmdLine:
        call    ClearCmdLine
        mov     [XPos],1
        lea     edx,[Msg50]
        jmp     PrintS
;-----------------------------------------------------------------------------
Decode: push    rax rcx rsi rdi
        xor     ecx,ecx
        mov     [Origin],rsi
        mov     rdi,rsi
      @@:
        mov     rsi,rdi
        add     rsi,rcx
        call    GetDataByte
        mov     [rcx+BufferI],al
        inc     ecx
        cmp     cl,15
        jb      @B
        pop     rdi rsi rcx rax
        pushad
        mov     [DefInst.Addr],BufferI
        mov     [TheBufferO],BufferO
        call    DisAsmLine
        mov     [rsp+1Ch],rax
        popad
        add     rsi,rax
        ret
DisAsmLine:
        push    rbp
        mov     al,[CodeType]
        mov     ah,0
        cmp     al,16
        jz      @F
        mov     ah,1
        cmp     al,32
        jz      @F
        mov     ah,2
      @@:
        mov     [DefInst.Arch],ah
        mov     [DefInst.Emulated],-1   ;#
        lea     ebp,[Inst+128]
        mov     ecx,sizeof.TInstruction
        lea     esi,[DefInst]
        lea     edi,[Inst]
        rep     movsb
        mov     [SpaceSize],8
        call    DisAsm
        push    rax
        call    PrnAsm
        pop     rax
        pop     rbp
        ret
;-----------------------------------------------------------------------------
ShowText:
MakeText:
        xor     ebp,ebp
        mov     [CPUXPtr],ebp
        mov     rsi,[CodeAddr]
        mov     [AddrBuffer],rsi
        mov     [YPos],0
.Loop:  mov     [Color],30h
        mov     [BufferO-1],byte 30h    ;Default Color
        inc     [YPos]
        mov     [XPos],1
        call    Decode
        mov     eax,[_EIP]
        inc     ebp
        cmp     [AddrBuffer+(rbp-1)*sizeof.dq],rax
        jnz     .1
        mov     [CPUXPtr],ebp
        mov     [BufferO+SkipAddr+1],byte 10h   ;'>>'
        cmp     [Inst.IsAddress],0
        je      .1
        call    CheckJump
        cmp     [JumpTaken],0
        jz      .1
        mov     [BufferO+SkipAddr+2*16+1],byte 18h
        mov     rax,[Inst.Arg1.Imm]
        cmp     rax,rsi
        jbe     .1
.0:     mov     [BufferO+SkipAddr+2*16+1],byte 19h

.1:     mov     [AddrBuffer+rbp*sizeof.dq],rsi
        lea     edx,[BufferO]
        cmp     byte [rdx-1],30h
        jnz     .2
        mov     ax,'+'
        xchg    ax,[rdx+9+9]
        cmp     al,32
ifz     mov     [rdx+9+9],al
        call    PrintS
        add     edx,9+15*2
        jmp     .3
.2:     mov     eax,edx
.N:     cmp     [eax],byte 32
        jz      .F
        inc     eax
        jmp     .N
.F:     mov     [rax],byte 0
        sub     eax,edx
        cmp     eax,(MinWidth-2)
        jbe     .X
        mov     [rdx+(MinWidth-2)],byte 0
.X:     add     edx,43
        mov     [Color],30h
        mov     [XPos],9+15*2-(9+9)+3
        call    PrintS
        sub     edx,43
        mov     al,[rdx-1]
        mov     [Color],al
        mov     [XPos],1
.3:     call    PrintS
        mov     rax,[AddrBuffer+(rbp-1)*sizeof.dq]
        call    BChk
        cmp     ebp,28
        jb      .Loop
        ret
;-----------------------------------------------------------------------------
BChk:   push    rbp rbx rcx rsi rdi
        xor     ebp,ebp
.001:   cmp     [DrXBreak+rbp*4],eax
        jnz     .NotFound
        push    rax
        mov     esi,1
        movzx   edi,[YPos]
        mov     ecx,[CurWidth]
        sub     ecx,25
        mov     ax,7F00h
        call    Cursor
        mov     eax,ebp                 ; EBP = breakpoint number
        shl     eax,16
        add     eax,60306023h           ; show #X (X=BrkPnt)
        add     esi,ecx                 ; adjust X-pos
        push    rdx
        mov     edx,[CurWidth]
        add     edx,edx
        imul    edx,edi
        lea     esi,[rdx+rsi*2]
        add     esi,ConsoleDataPtr
        mov     [rsi],eax
        pop     rdx
        pop     rax
.NotFound:
        inc     ebp
        cmp     ebp,4
        jnz     .001

        push    rax
        call    FindEnabledBreakPoint;FindBreakPoint
        inc     eax
        jz      .NoFound2
        mov     esi,1
        movzx   edi,[YPos]
        mov     ecx,[CurWidth]
        sub     ecx,21
        mov     ax,7F00h
        call    Cursor
.NoFound2:
        pop     rax

        pop     rdi rsi rcx rbx rbp
        ret
;-----------------------------------------------------------------------------
CheckJump:
        pushf
        push    rax rdx
        pushf
        and     dword [rsp],$FFFFF700
        mov     eax,[_EFL]
        and     eax,000008FFh
        or      [rsp],eax
        mov     rax,[Inst.SaveRSI]
        movzx   edx,byte [rax-2]
        movzx   eax,byte [rax-1]
        mov     [JumpTaken],ah  ;ah=0
        cmp     al,0xE8
        jz      .2
        cmp     al,0xE9
        jz      .2
        cmp     al,0xEB
        jz      .2
        cmp     al,0xE0         ;LOOPNZ
        jz      .3
        cmp     al,0xE1         ;LOOPZ
        jz      .4
        cmp     al,0xE2         ;LOOP
        jz      .5
        cmp     al,0xE3         ;JxCXZ
        jz      .6
        and     al,7Fh          ;70-7F
        cmp     al,0Fh          ;0F80-0F8F
        jbe     .1
        sub     al,70h
      .1:
        popf
        lea     eax,[JccCalls+rax*4]
        call    rax
        mov     [JumpTaken],al
        pop     rdx rax
        popf
        ret
      .2:
        mov     [JumpTaken],1
        popf
        pop     rdx rax
        popf
        ret
      .3:
        popf
        call    JccCalls+5*4
        mov     [JumpTaken],al
        call    TypeLoop
        pop     rdx rax
        popf
        ret
      .4:
        popf
        call    JccCalls+4*4
        mov     [JumpTaken],al
        call    TypeLoop
        pop     rdx rax
        popf
        ret
      .5:
        popf
        call    TypeLoop
        pop     rdx rax
        popf
        ret
      .6:
        popf
        call    TypeJxCXZ
        pop     rdx rax
        popf
        ret
;-----------------------------------------------------------------------------
TypeJxCXZ:
        cmp     [CodeType],32
        jz      JCXZ32
        cmp     [CodeType],16
        jz      JCXZ16
if %B=64
        cmp     dl,67h
        jnz     .1
        cmp     [Reg.ECX],0
        jmp     .2
.1:     cmp     [Reg.RCX],0
.2:     setz    [JumpTaken]
end if
        ret
;-----------------------------------------------------------------------------
JCXZ16: cmp     dl,67h
        jz      .1
        cmp     word [_ECX],0
        setz    [JumpTaken]
        jmp     .2
.1:     cmp     [_ECX],0
        setz    [JumpTaken]
.2:     ret
;-----------------------------------------------------------------------------
JCXZ32: cmp     dl,67h
        jz      .1
        cmp     [_ECX],0
        setz    [JumpTaken]
        jmp     .2
.1:     cmp     word [_ECX],0
        setz    [JumpTaken]
.2:     ret
;-----------------------------------------------------------------------------
TypeLoop:
        cmp     [CodeType],32
        jz      .32
        cmp     [CodeType],16
        jz      .16
if %B=64
        mov     rax,[_RCX]
        cmp     dl,67h          ; check for ECX or RCX == 0
        jnz     .1X
        mov     eax,eax
.1X:    dec     rax
        setnz   al
        or      [JumpTaken],al
end if
        ret
.32:    mov     eax,[_ECX]
        cmp     dl,67h          ; check for CX or ECX == 0
        jnz     .1Y
        movzx   eax,ax
.1Y:    dec     eax
        setnz   al
        or      [JumpTaken],al
        ret
.16:    mov     eax,[_ECX]
        cmp     dl,67h          ; check for CX or ECX == 0
        jz      .1Z
        movzx   eax,ax
.1Z:    dec     eax
        setnz   al
        or      [JumpTaken],al
        ret
;-----------------------------------------------------------------------------
CPUCursor:
        mov     esi,[CPUXPos]
        mov     edi,[CPUYPos]
        mov     ecx,[CurWidth]
        sub     ecx,23
        mov     ax,2F00h
        call    Cursor
        mov     eax,[CPUYPos]
        mov     rsi,[(rax-1)*sizeof.dq+AddrBuffer]
        call    Decode
        call    ShowAddress
        ret
;-----------------------------------------------------------------------------
RegCursor:
        mov     esi,[RegXPos]
        mov     edi,[RegYPos]
        mov     ecx,[RegLDat]
        jmp     CursorXY
;-----------------------------------------------------------------------------
MemCursor:
        mov     esi,[MemXPos]
        mov     edi,[MemYPos]
        mov     ecx,2
        mov     ax,2F00h
        jmp     Cursor
;-----------------------------------------------------------------------------
StkCursor:
        mov     esi,[StkXPos]
        mov     edi,[StkYPos]
        mov     ecx,20
        mov     ax,2F00h
        jmp     Cursor
;-----------------------------------------------------------------------------
CursorXY:
        pushad
        mov     eax,[CurWidth]
        add     eax,eax
        imul    eax,edi
        lea     esi,[rax+rsi*2]
        add     esi,ConsoleDataPtr
.Loop:  mov     al,[rsi+1]
        cmp     al,17h
        jz      .1
        cmp     al,1Fh
        jz      .2
        cmp     al,30h
        jz      .3
        cmp     al,3Fh
        jz      .4
        jmp     .Next
.1:     mov     al,3Fh
        jmp     .Next
.2:     mov     al,30h
        jmp     .Next
.3:     mov     al,1Fh
        jmp     .Next
.4:     mov     al,17h
.Next:  mov     [rsi+1],al
        add     esi,2
        dec     cl
        jnz     .Loop
        popad
        ret
;-----------------------------------------------------------------------------
Cursor: push    rcx rdx rsi
        mov     edx,eax
        mov     eax,[CurWidth]
        add     eax,eax
        imul    eax,edi
        xchg    edx,eax
        lea     esi,[rdx+rsi*2]
        add     esi,ConsoleDataPtr
.Loop:  xor     [rsi],ax
        add     esi,2
        dec     cl
        jnz     .Loop
        pop     rsi rdx rcx
        ret
;-----------------------------------------------------------------------------
ShowCursor:
        mov     ebp,CursorTab
GotoWindow:
        mov     ebx,[CurrentWindow]
        and     ebx,3
        jmp     dword [rbp+rbx*4]
;-----------------------------------------------------------------------------
WindowSwitch:
        test    [CtrlState],3
        jz      .1
        dec     [CurrentWindow]
        jmp     .2
.1:     inc     [CurrentWindow]
.2:     and     [CurrentWindow],3
        xor     eax,eax
WinSwitch:
        mov     [AfterKey],1
        call    ShowImage
        mov     ebp,WindowTab
        jmp     GotoWindow
;-----------------------------------------------------------------------------
DrawChar:
        mov     [rdi+rdx*2],ax
        inc     edx
        ret
;-----------------------------------------------------------------------------
DrawLine:
        mov     [rdi+rdx*2],ax
        inc     edx
        loop    DrawLine
        ret
;-----------------------------------------------------------------------------
DrawDown:
        mov     [rdi+rdx*2],ax
        add     edx,[CurWidth]
        loop    DrawDown
        ret
;-----------------------------------------------------------------------------
PrintAL:push    rdi
        mov     edi,HexBuf
        call    MakeHex
        lea     edi,[rdi+6]
        jmp     PrintHex
;-----------------------------------------------------------------------------
PrintAX:push    rdi
        mov     edi,HexBuf
        call    MakeHex
        lea     edi,[rdi+4]
        jmp     PrintHex
;-----------------------------------------------------------------------------
PrintEAX:
        push    rdi
        mov     edi,HexBuf
        call    MakeHex
PrintHex:
        push    rdx
        mov     edx,edi
        call    PrintS
        pop     rdx rdi
        ret
;-----------------------------------------------------------------------------
MakeHex:push    rax rdx rsi
        mov     edx,3
.Loop:  mov     esi,eax
        and     eax,$FF
        mov     ax,[rax*2+HexString]
        mov     [rdi+rdx*2],ax
        mov     eax,esi
        shr     eax,8
        sub     edx,1
        jnc     .Loop
        pop     rsi rdx rax
        ret
;-----------------------------------------------------------------------------
DrawFrame:
        mov     ah,3Fh          ; color
        mov     edi,ConsoleDataPtr
        xor     edx,edx
        xor     ecx,ecx

        mov     al,     'É'
        call    DrawChar
        mov     al,     'Í'
        mov     cl,byte [CurWidth]
        sub     cl,2
        call    DrawLine
        mov     al,     '»'
        call    DrawChar

        mov     al,     'º'     ; left border
        mov     edx,[CurWidth]
        mov     cl,byte [CurHeight]
        sub     cl,2
        call    DrawDown
        mov     al,     'È'
        call    DrawChar
        mov     al,     'º'     ; right border
        imul    edx,[CurWidth],2
        dec     edx
        mov     cl,byte [CurHeight]
        sub     cl,2
        call    DrawDown
        mov     al,     '¼'
        call    DrawChar

        mov     al,     'Ç'
        imul    edx,[CurWidth],29
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,byte [CurWidth]
        sub     cl,2
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar

        mov     al,     'Ç'
        imul    edx,[CurWidth],40
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,byte [CurWidth]
        sub     cl,(2+45)
        call    DrawLine
        mov     al,     '´'
        call    DrawChar

        mov     al,     'Ç'
        imul    edx,[CurWidth],49
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,byte [CurWidth]
        sub     cl,2
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar

        mov     al,     'Â'
        imul    edx,[CurWidth],30
        sub     edx,46
        call    DrawChar
        imul    edx,[CurWidth],31
        sub     edx,46
        mov     al,     '³'
        mov     cl,10
        call    DrawDown

        imul    edx,[CurWidth],40
        sub     edx,46
        mov     al,     '³'
        mov     cl,10
        call    DrawDown

        mov     al,     'Ã'
        imul    edx,[CurWidth],46
        sub     edx,46
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,44
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar
        mov     al,     'Ñ'
        mov     edx,[CurWidth]
        sub     edx,22
        call    DrawChar

        mov     al,     '³'
        imul    edx,[CurWidth],2
        sub     edx,22
        mov     cl,28
        call    DrawDown
        mov     al,     'Á'
        call    DrawChar

        mov     al,     'Ã'
        imul    edx,[CurWidth],42
        sub     edx,46
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,44
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar

        mov     al,     'Ã'
        imul    edx,[CurWidth],34
        sub     edx,46
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,44
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar

        mov     al,     'Ã'
        imul    edx,[CurWidth],38
        sub     edx,46
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,44
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar
        mov     al,     '´'
        imul    edx,[CurWidth],41
        sub     edx,46
        call    DrawChar
        mov     al,     'Á'
        imul    edx,[CurWidth],50
        sub     edx,46
        call    DrawChar
if 1
        mov     al,     'Ç'
        imul    edx,[CurWidth],MinHeight-2-(MSG_HEIGHT-1)-1
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,byte [CurWidth]
        sub     cl,2
        call    DrawLine
        mov     al,     '¶'
        call    DrawChar
end if
        mov     [Color],3Ah
        call    ShowCPU
        call    ShowFPU
        mov     [Color],3Ah
        mov     [XPos],3
        mov     [YPos],29
        lea     edx,[Msg10]
        call    PrintS
        mov     [Color],30h
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg11]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg12]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg13]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg14]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg15]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg16]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg17]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg18]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg19]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[Msg20]
        call    PrintS
        mov     [XPos],3
        inc     [YPos]
        mov     [Color],3Ah
        lea     edx,[Msg21]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,43
        mov     [XPos],al
        mov     [YPos],29
        lea     edx,[MsgX2]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,43
        mov     [XPos],al
        mov     [YPos],33
        lea     edx,[MsgX3]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,43
        mov     [XPos],al
        mov     [YPos],37
        lea     edx,[MsgX4]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,43
        mov     [XPos],al
        mov     [YPos],41
        lea     edx,[Msg22]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,43
        mov     [XPos],al
        mov     [YPos],45
        lea     edx,[Msg23]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,19
        mov     [XPos],al
        mov     [YPos],0
        lea     edx,[Msg30]
        call    PrintS
        mov     [XPos],3
        mov     [YPos],MinHeight-2-(MSG_HEIGHT-1)-1
        lea     edx,[Msg3M]
        call    PrintS
        mov     eax,[CurWidth]
        sub     eax,51-23
        mov     [XPos],al
        mov     [YPos],MinHeight-2-(MSG_HEIGHT-1)-1
        lea     edx,[Msg40]
        call    PrintS

        cmp     [DebuggeePID],0
        jz      .Exit

        call    MakeText
        btr     dword [AfterKey],0
        jc      .0

        cmp     [CPUXPtr],0
        jnz     .1
        mov     eax,[_EIP]
        mov     [CodeAddr],rax
        mov     [CPUYPos],1
.0:
        mov     ebx,28                  ;dY
        mov     ecx,[CurWidth]          ;\dX
        sub     ecx,23                  ;/
        mov     esi,1                   ;X
        mov     edi,1                   ;Y
        mov     edx,3F203F20h
        call    ClearWindow
        call    MakeText
        jmp     .2
.1:     mov     eax,[CPUXPtr]
        mov     [CPUYPos],eax
        mov     rsi,[AddrBuffer+(rax-1)*sizeof.dq]
        call    Decode
        call    ShowAddress
.2:
        call    ShowRegs
        call    ShowData
        call    ShowCursor
.Exit:
        mov     eax,PrintCmdLine
        cmp     [CmdLineActive],0
ifnz    mov     eax,DrawCmdLine
        call    eax

        mov     edx,Messages
        push    MSG_WIDTH
        pop     esi
        xor     ecx,ecx
    @@:
        mov     eax,MinHeight-2-(MSG_HEIGHT-1)
        add     eax,ecx
        mov     [XPos],1
        mov     [YPos],al
        mov     [Color],8Fh
        xor     ebp,ebp
    .Loop:
        mov     al,[rdx+rbp]
        cmp     al,8
        jnz     .NoColor
        inc     ebp
        mov     al,[rdx+rbp]
        mov     [Color],al
        jmp     .Next
   .NoColor:
        call    PrintC
   .Next:
        inc     ebp
        cmp     ebp,MSG_WIDTH
        jnz     .Loop
        inc     ecx
        add     edx,esi
        cmp     edx,Messages+MSG_WIDTH*MSG_HEIGHT
        jb      @B

        mov     esi,[PrgNamePtr]
        mov     edi,HeaderE-61
        mov     ecx,60
      @@:
        lodsb
        stosb
        or      al,al
        loopnz  @B

        mov     eax,71
        mov     ebx,2
        mov     dl,1    ;CP866
        mov     ecx,HeaderN
        cmp     [DebuggeePID],0
        jz      @F
        mov     ecx,HeaderY
    @@: int     40h
        ret
;-----------------------------------------------------------------------------
ShowRegs:
        call    ShowGRegs
        call    ShowSRegs
        call    ShowXRegs
        call    ShowSlide
if 0
        jmp     ShowMode
else
        ret
end if
;-----------------------------------------------------------------------------
PrintC: push    rax rdx rdi rbx
        mov     bl,byte [CurWidth]
        mov     bh,byte [CurHeight]
        cmp     al,09h
        jz      .09
        cmp     al,0Ah
        jz      .0A
        cmp     al,0Dh
        jz      .0D
        ;
        mov     edi,eax
        movzx   eax,[YPos]
        mul     [CurWidth]
        movzx   edx,[XPos]
        add     edx,eax
        mov     eax,edi
        mov     edi,ConsoleDataPtr
        ;
        mov     ah,[Color]
        mov     [rdi+rdx*2],ax
        inc     [XPos]
        cmp     [XPos],bl
        jb      .Done
        mov     [XPos],0
        jmp     .0A
.0D:    pop     rbx rdi rdx rax
        mov     [XPos],ah
        ret
.09:    and     [XPos],$F8
        add     [XPos],8
        cmp     [XPos],bl
        jb      .Done
        sub     [XPos],bl
.0A:    inc     [YPos]
        cmp     [YPos],bh
        jb      .Done
        mov     [YPos],0
.Done:  pop     rbx rdi rdx rax
        ret
;-----------------------------------------------------------------------------
PrintS: push    rax rdx
        mov     ah,[XPos]
.Loop:  mov     al,[rdx]
        inc     edx
        test    al,al
        jz      .Done
        cmp     al,08h
        jz      .1
        call    PrintC
        jmp     .Loop
.1:     mov     al,[rdx]
        inc     edx
        test    al,al
        jz      .Done
        mov     [Color],al
        jmp     .Loop
.Done:  pop     rdx rax
        ret
;-----------------------------------------------------------------------------
ShowGRegs:
        mov     dl,5
        mov     [YPos],30
        mov     [XPos],dl
        lea     rbp,[_EAX]
        call    DisplayIfChanged
        lea     rbp,[_EBX]
        call    DisplayIfChanged
        lea     rbp,[_ECX]
        call    DisplayIfChanged
        lea     rbp,[_EDX]
        call    DisplayIfChanged
        lea     rbp,[_ESI]
        call    DisplayIfChanged
        lea     rbp,[_EDI]
        call    DisplayIfChanged
        lea     rbp,[_EBP]
        call    DisplayIfChanged
        lea     rbp,[_ESP]
DisplayIfChanged:
        mov     rax,[rbp]
        cmp     rax,[rbp+Plus]
        mov     [Color],30h
        jz      @F
        mov     [Color],3Fh
@@:     call    PrintEAX
        inc     [YPos]
        mov     [XPos],dl
        ret
;-----------------------------------------------------------------------------
ShowXRegs:
        mov     dl,18
        mov     [YPos],30
        mov     [XPos],dl
        lea     rbp,[_EFL]
        call    DisplayIfChanged
        lea     rbp,[_EIP]
        call    DisplayIfChanged
        ret
;-----------------------------------------------------------------------------
ShowSlide:
        mov     [XPos],1
        mov     [YPos],39
        mov     esi,[_EFL]
        mov     edi,[_EFL+Plus]
        shl     esi,14
        shl     edi,14
        call    .Show           ; VM-flag
        call    .Show           ; RF-flag
        shl     esi,1
        shl     edi,1
        call    .Show           ; NT-flag
        call    .Show           ; IOPL-flag
        dec     [XPos]
        call    .Show
        call    .Show           ; OF-flag
        call    .Show           ; DF-flag
        call    .Show           ; IF-flag
        call    .Show           ; TF-flag
        call    .Show           ; SF-flag
        inc     [XPos]
        call    .Show           ; ZF-flag
        shl     esi,1
        shl     edi,1
        call    .Show           ; AF-flag
        shl     esi,1
        shl     edi,1
        call    .Show           ; PF-flag
        shl     esi,1
        shl     edi,1
.Show:  xor     ax,ax
        shl     esi,1
        adc     al,'0'
        shl     edi,1
        adc     ah,'0'
        cmp     al,ah
        mov     [Color],30h
        jz      .S1
        mov     [Color],3Fh
.S1:    call    PrintC
        inc     [XPos]
        ret
;-----------------------------------------------------------------------------
ShowSRegs:
        mov     dl,17
        mov     [YPos],32
        mov     eax,cs
        call    .DisplaySel
        mov     eax,ds
        call    .DisplaySel
        mov     eax,es
        call    .DisplaySel
        mov     eax,fs
        call    .DisplaySel
        mov     eax,gs
        call    .DisplaySel
        mov     eax,ss
.DisplaySel:
        mov     [Color],31h
        mov     [XPos],dl
        call    PrintAX
        add     [XPos],10
        lar     eax,ax
        shr     eax,8
        call    PrintAX
        inc     [YPos]
        ret
;-----------------------------------------------------------------------------
ShowData:
        mov     bl,8
        mov     [YPos],41
        mov     [Color],30h
        mov     rsi,[DataAddr]
        call    ShowDataX
        mov     bl,3
        xor     ebp,ebp
        mov     edi,[_EDX]
        mov     [YPos],30
        call    ShowDataRDX
        mov     bl,3
        xor     ebp,ebp
        mov     edi,[_EBX]
        mov     [YPos],34
        call    ShowDataRBX
        mov     bl,3
        xor     ebp,ebp
        mov     edi,[_ESI]
        mov     [YPos],42
        call    ShowDataRSI
        mov     bl,3
        xor     ebp,ebp
        mov     edi,[_EDI]
        mov     [YPos],46
        call    ShowDataRDI
        mov     bl,3
        xor     ebp,ebp
        mov     edi,[_EBP]
        mov     [YPos],38
        call    ShowDataRBP
        call    ShowStack32
        call    ShowFPURegs
        cmp     [MMXType],0
ifnz    call    ShowMMXRegs
        cmp     [XMMType],0
ifnz    call    ShowXMMRegs
        ret
;-----------------------------------------------------------------------------
ShowDataX:
.1:     mov     [Color],30h
        mov     [XPos],1
        mov     rax,rsi
        call    PrintEAX
        mov     al,':'
        call    PrintC
        inc     [XPos]
        mov     ecx,35 by 8
.2:     mov     [Color],30h
        call    GetDataByte
        jc      .3
        lea     eax,[Msg2ER]
        xchg    eax,edx
        call    PrintS
        xchg    eax,edx
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rsi
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
;-----------------------------------------------------------------------------
ShowDataRSI:
.1:     mov     eax,[CurWidth]
        sub     eax,29+16
        mov     [XPos],al
        mov     rax,rbp
        test    rbp,rbp
        lea     edx,[Msg2SIP]
        jns     .01
        lea     edx,[Msg2SIM]
        neg     rax
.01:    call    PrintS
        call    PrintAL
        mov     al,']'
        call    PrintC
        mov     al,32
        test    rbp,rbp
        jnz     .03
        mov     al,10h
.03:    call    PrintC
        inc     [XPos]
.04:    mov     ecx,[CurWidth]
        sub     ecx,5+5
        shl     ecx,8
        mov     cl,8
.2:     lea     rsi,[rdi+rbp]
        mov     al,3            ;ds
        call    GetDataByte
        jc      .3
        lea     edx,[Msg2ER]
        call    PrintS
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rbp
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
ShowDataRDI:
.1:     mov     eax,[CurWidth]
        sub     eax,29+16
        mov     [XPos],al
        mov     rax,rbp
        test    rbp,rbp
        lea     edx,[Msg2DIP]
        jns     .01
        lea     edx,[Msg2DIM]
        neg     rax
.01:    call    PrintS
        call    PrintAL
        mov     al,']'
        call    PrintC
        mov     al,32
        test    rbp,rbp
        jnz     .03
        mov     al,10h
.03:    call    PrintC
        inc     [XPos]
.04:    mov     ecx,[CurWidth]
        sub     ecx,5+5
        shl     ecx,8
        mov     cl,8
.2:     lea     rsi,[rdi+rbp]
        mov     al,0            ;es
        call    GetDataByte
        jc      .3
        lea     edx,[Msg2ER]
        call    PrintS
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rbp
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
ShowDataRBX:
.1:     mov     eax,[CurWidth]
        sub     eax,29+16
        mov     [XPos],al
        mov     rax,rbp
        test    rbp,rbp
        lea     edx,[Msg2BXP]
        jns     .01
        lea     edx,[Msg2BXM]
        neg     rax
.01:    call    PrintS
        call    PrintAL
        mov     al,']'
        call    PrintC
        mov     al,32
        test    rbp,rbp
        jnz     .03
        mov     al,10h
.03:    call    PrintC
        inc     [XPos]
.04:    mov     ecx,[CurWidth]
        sub     ecx,5+5
        shl     ecx,8
        mov     cl,8
.2:     lea     rsi,[rdi+rbp]
        mov     al,3            ;ds
        call    GetDataByte
        jc      .3
        lea     edx,[Msg2ER]
        call    PrintS
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rbp
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
;-----------------------------------------------------------------------------
ShowDataRDX:
.1:     mov     eax,[CurWidth]
        sub     eax,29+16
        mov     [XPos],al
        mov     rax,rbp
        test    rbp,rbp
        lea     edx,[Msg2DXP]
        jns     .01
        lea     edx,[Msg2DXM]
        neg     rax
.01:    call    PrintS
        call    PrintAL
        mov     al,']'
        call    PrintC
        mov     al,32
        test    rbp,rbp
        jnz     .03
        mov     al,10h
.03:    call    PrintC
        inc     [XPos]
.04:    mov     ecx,[CurWidth]
        sub     ecx,5+5
        shl     ecx,8
        mov     cl,8
.2:     lea     rsi,[rdi+rbp]
        mov     al,3            ;ds
        call    GetDataByte
        jc      .3
        lea     edx,[Msg2ER]
        call    PrintS
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rbp
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
;-----------------------------------------------------------------------------
ShowDataRBP:
.1:     mov     eax,[CurWidth]
        sub     eax,29+16
        mov     [XPos],al
        mov     rax,rbp
        test    rbp,rbp
        lea     edx,[Msg2BPP]
        jns     .01
        lea     edx,[Msg2BPM]
        neg     rax
.01:    call    PrintS
        call    PrintAL
        mov     al,']'
        call    PrintC
        mov     al,32
        test    rbp,rbp
        jnz     .03
        mov     al,10h
.03:    call    PrintC
        inc     [XPos]
.04:    mov     ecx,[CurWidth]
        sub     ecx,5+5
        shl     ecx,8
        mov     cl,8
.2:     lea     rsi,[rdi+rbp]
        mov     al,2            ;ss
        call    GetDataByte
        jc      .3
        lea     edx,[Msg2ER]
        call    PrintS
        mov     al,'?'
        jmp     .5
.3:     call    PrintAL
        test    al,al
        jz      .4
        cmp     al,08h
        jz      .4
        cmp     al,09h
        jz      .4
        cmp     al,0Ah
        jz      .4
        cmp     al,0Dh
        jnz     .5
.4:     mov     al,'.'
.5:     xchg    [XPos],ch
        call    PrintC
        xchg    [XPos],ch
        inc     [XPos]
        inc     rbp
        dec     cl
        jnz     .2
        inc     [YPos]
        dec     bl
        jnz     .1
        ret
;-----------------------------------------------------------------------------
GetSeg: mov     edx,TDS
        cmp     [Inst.DefSeg],VSS
ifz     mov     edx,TSS
        mov     al,[Inst.CurSeg]
        or      al,al
        jz      .SkipSegm
        cmp     al,[Inst.DefSeg]
        jz      .SkipSegm
        mov     dx,[Inst.SegmName]
.SkipSegm:
        mov     dx,[rdx+Names]
        ret
;-----------------------------------------------------------------------------
GetSegm:mov     dx,TDS
        cmp     [Inst.DefSeg],VSS
ifz     mov     dx,TSS
        mov     al,[Inst.CurSeg]
        or      al,al
        jz      @F
        cmp     al,[Inst.DefSeg]
        jz      @F
        mov     dx,[Inst.SegmName]
      @@:
        mov     al,0
        cmp     dx,TES
        jz      @F
        mov     al,1
        cmp     dx,TCS
        jz      @F
        mov     al,2
        cmp     dx,TSS
        jz      @F
        mov     al,3
        cmp     dx,TDS
        jz      @F
        mov     al,4
        cmp     dx,TFS
        jz      @F
        mov     al,5
      @@:
        ret
;-----------------------------------------------------------------------------
GetMemAddress:
        ;
        xor     ebx,ebx
        mov     ecx,1
        ;
        movzx   eax,[Inst.Scale]
        cmp     al,-1
        jz      @F
        mov     cl,[eax+AnScale]
      @@:
        movzx   eax,[Inst.Base]
        cmp     al,-1
        jz      @F
        mov     eax,[rax*4+RegVTab]
        mov     rax,[rax]
        call    Address2Reg
        add     rbx,rax
      @@:
        movzx   eax,[Inst.Indx]
        cmp     al,-1
        jz      @F
        mov     eax,[rax*4+RegVTab]
        mov     rax,[rax]
        call    Address2Reg
        mul     rcx
        add     rbx,rax
      @@:
        cmp     [Inst.DispSize],0
        jz      @F
        mov     rax,[Inst.Disp]
        call    Address2Reg
        add     rbx,rax
      @@:
        mov     rax,rbx
        call    Address2Reg
        mov     rbx,rax
        ret
;-----------------------------------------------------------------------------
ShowAddress:
        ;
        mov     [Color],3Fh
        mov     [XPos],15
        mov     [YPos],ch
        mov     al,0xCD
        mov     ecx,[CurWidth]
        sub     ecx,21+16+1
.LoopX: call    PrintC
        loop    .LoopX
        ;
        mov     al,[Inst.Arg1.Mem]
        or      al,[Inst.Arg2.Mem]
        or      al,[Inst.Arg3.Mem]
        or      al,[Inst.Arg4.Mem]
        or      al,[Inst.Arg5.Mem]
ifz     ret
        ;
        call    GetSeg
        mov     [AddressStr],dx
        ;
        call    GetMemAddress
if %B=64
        rol     rax,32
        mov     edi,AddressStr+3
        call    MakeHex
        rol     rax,32
        mov     edi,AddressStr+3+8
        call    MakeHex
else
        mov     edi,AddressStr+3
        call    MakeHex
end if
        mov     edi,AddressFin

        movzx   edx,[Inst.RandSize]
        and     dl,7Fh
        movzx   ebp,[rdx*2+RndSize+0]
        movzx   ecx,[rdx*2+RndSize+1]
        jrcxz   .Exit
        lea     rsi,[rbx+rcx]
        ;
        cmp     edx,3
        jnz     @F
        cmp     [Inst.Point],0
        jz      @F
        mov     bp,2+1
        mov     cx,2+2
      @@:
        ;
      .Loop:
        dec     rsi
        sub     ebp,1
        jnz     @F
        mov     [rdi],byte ':'
        inc     edi
      @@:
        mov     [rdi],word '--'
        call    GetSegm
        call    GetDataByte
        jnc     @F
        movzx   eax,al
        mov     ax,[rax*2+HexString]
        mov     [rdi],ax
      @@:
        add     edi,2
        loop    .Loop
      .Exit:
        mov     [rdi],word 'h'

        sub     edi,AddressStr
        mov     eax,[CurWidth]
        sub     eax,24
        sub     eax,edi
ifc     ret
        mov     [XPos],al
        mov     [YPos],0
        mov     [Color],3Fh
        mov     edx,AddressStr
        jmp     PrintS
;-----------------------------------------------------------------------------
Address2Reg:
        cmp     [Inst.Address],2
        jz      @F
        cmp     [Inst.Address],1
        mov     eax,eax
        jz      @F
        movzx   eax,ax
      @@:
        ret
;-----------------------------------------------------------------------------
sglobal
if %B=32
AddressStr:     db '??:xxxxxxxx = '
AddressFin:     db 64*2+2 dup 0
else
AddressStr:     db '??:xxxxxxxxxxxxxxxx = '
AddressFin      db 64*2+2 dup 0
end if
endg
;-----------------------------------------------------------------------------
iglobal
RndSize db 0,0  ;NULL
        db 0,1  ;TWORD
        db 0,2  ;SWORD
        db 0,4  ;DWORD
        db 0,8  ;QWORD
        db 0,16 ;XWORD
        db 0,32 ;YWORD
        db 0,52 ;ZWORD -> 64 too large
        db 2+1,4+2;FWORD
        db 2+1,8+2;TWORD
AnScale db 1,2,4,8
endg
;-----------------------------------------------------------------------------
ClearWindow:
        mov     ebp,[CurWidth]
        mov     eax,edi
        imul    eax,ebp
        add     eax,eax
        lea     edi,[rax+rsi*2]
        add     edi,ConsoleDataPtr
        mov     eax,edx
        lea     edx,[rcx*2]
.Loop:  push    rcx
        shr     ecx,1
        rep     stosd
        adc     cl,cl
        rep     stosw
        pop     rcx
        sub     esi,edx
        sub     edi,edx
        lea     esi,[rsi+rbp*2]
        lea     edi,[rdi+rbp*2]
        dec     ebx
        jnz     .Loop
        ret
;-----------------------------------------------------------------------------
ShowStack32:
        mov     [YPos],28
        mov     [Color],30h
        mov     cl,28
        mov     edi,[_ESP]
        add     rdi,[StckAddr]
.1:     mov     rax,rdi
        sub     rax,[_ESP]
        mov     edx,Msg3SPP
        jns     .2
        neg     rax
        mov     edx,Msg3SPM
.2:     mov     ebx,[CurWidth]
        sub     ebx,21
        mov     [XPos],bl
        call    PrintS
        call    PrintAX
        mov     al,']'
        call    PrintC
.20:
        cmp     edi,[_ESP]
        mov     al,10h
        jz      .3
        cmp     edi,[_EBP]
        mov     al,'>'
        jz      .3
        mov     al,20h
.3:     call    PrintC
        lea     rsi,[rdi+3]
rept 3
{
        call    GetDataByte
        dec     rsi
        shl     rax,8
}
        call    GetDataByte
        add     rdi,4
        call    PrintEAX
       ;lea     edx,[Spaces32+6]
       ;call    PrintS
        dec     [YPos]
        dec     cl
        jnz     .1
        cmp     [CurrentWindow],3
        jz      ShowAddrStk
        ret
ShowAddrStk:
        mov     eax,[CurWidth]
        sub     eax,51-23-9
        mov     [XPos],al
        mov     [YPos],60-2-9+10
        mov     [Color],3Fh
        mov     rax,[StckAddr]
        add     eax,[_ESP]
        call    PrintEAX
        add     [XPos],3
        mov     rsi,rax
        call    GetDataByte
        jc      PrintAL
        lea     edx,[Msg2ER]
        jmp     PrintS
;-----------------------------------------------------------------------------
FindCodeUp:
        mov     rax,[AddrBuffer]
        sub     rax,16                  ; average opcode length
.1:     inc     rax
        mov     rbx,rax
        mov     rsi,rax
.2:     mov     rsi,rbx
        mov     rdx,rbx
        call    Decode
        cmp     rbx,rsi
        ja      .Done
        mov     rbx,rsi
        sub     rsi,[AddrBuffer]
        jb      .2
.Done:  mov     rax,rdx
        ret
;-----------------------------------------------------------------------------
CPUWindow:
        cmp     ah,0x0F
        jz      WindowSwitch
        cmp     ah,0x3B                 ; F1 - Toggle Show
        jz      ToggleShow
        cmp     ah,0x3C                 ; F2 - Toggle BP
        jz      ToggleBP
        cmp     ah,0x3D                 ; F3 - Return To
        jz      ReturnTo
        cmp     ah,0x3E                 ; F4 - Here
        jz      ComeHere
        cmp     ah,0x40                 ; F6 - New EIP
        jz      NewAddress
        cmp     ah,0x41                 ; F7 - Trace
        jz      F7
        cmp     ah,0x42                 ; F8 - Step over
        jz      F8
        cmp     ah,0x43                 ; F9 - Proceed
        jz      Proceed
        cmp     ah,0x44                 ; F10 - Jump over
        jz      JumpOver
        cmp     al,0xB1
        jz      CPUDn
        cmp     al,0xB2
        jz      CPUUp
        cmp     al,0xB4
        jz      CPUHome
        cmp     al,0xB7
        jz      CPUPgDn
        cmp     al,0xB8
        jz      CPUPgUp
        cmp     ah,0x4B
        jz      CPULeft
        cmp     ah,0x4D
        jz      CPURight
        cmp     ah,0xE
        jz      CPUGoToMem
;
        cmp     al,'a'
        jb      .Skip1
        cmp     al,'z'
        jbe     DoCommandLine
      .Skip1:
        ;
        cmp     al,'A'
        jb      .Skip2
        cmp     al,'Z'
        jbe     DoCommandLine
      .Skip2:
;
        jmp     WaitEvent
RegWindow:
        cmp     ah,0x0F
        jz      WindowSwitch
        cmp     ah,0x01
        jz      SwitchCPU
        cmp     ah,0x3B                 ; F1 - Toggle Show
        jz      ToggleShow
;
        cmp     al,0xB1                 ; dn
        jz      RegDn
        cmp     al,0xB2                 ; up
        jz      RegUp
        cmp     al,0xB4                 ; home
        jz      RegHome
        cmp     al,0xB5                 ; end
        jz      RegEnd
        cmp     ah,0x4B                 ; left
        jz      RegLeft
        cmp     ah,0x4D                 ; right
        jz      RegRight
        cmp     al,'+'                  ; '+'
        jz      RegIncR
        cmp     al,'-'                  ; '-'
        jz      RegDecR
        cmp     al,'*'                  ; '*'
        jz      RegNot
        cmp     al,'/'                  ; '/'
        jz      RegRes
        cmp     ah,0xE                  ; BackSpace - GoTo MemAddr
        jz      RegGoToMem
        cmp     ah,0x39                 ; Space
        jz      RegXor
;
        cmp     al,'a'
        jb      .Skip1
        cmp     al,'z'
        jbe     DoCommandLine
      .Skip1:
        ;
        cmp     al,'A'
        jb      .Skip2
        cmp     al,'Z'
        jbe     DoCommandLine
      .Skip2:
;
        jmp     WaitEvent
MemWindow:
        cmp     ah,0x0F
        jz      WindowSwitch
        cmp     ah,0x01
        jz      SwitchCPU
        cmp     ah,0x3B                 ; F1 - Toggle Show
        jz      ToggleShow
;
        cmp     al,0xB1                 ; down
        jz      MemDn
        cmp     al,0xB2                 ; up
        jz      MemUp
        cmp     ah,0x4B                 ; left
        jz      MemLeft
        cmp     ah,0x4D                 ; right
        jz      MemRight
        cmp     al,0xB4                 ; home
        jz      MemHome
        cmp     al,0xB7                 ; page-dn
        jz      MemPageDn
        cmp     al,0xB8                 ; page-up
        jz      MemPageUp
;
        cmp     al,'+'
        jz      MemIncB
        cmp     al,'-'
        jz      MemDecB
        cmp     al,'*'
        jz      MemNotB
        cmp     al,'/'
        jz      MemResB
;
        cmp     al,'a'
        jb      .Skip1
        cmp     al,'z'
        jbe     DoCommandLine
      .Skip1:
        ;
        cmp     al,'A'
        jb      .Skip2
        cmp     al,'Z'
        jbe     DoCommandLine
      .Skip2:
;
        jmp     WaitEvent
StkWindow:
        cmp     ah,0x0F
        jz      WindowSwitch
        cmp     ah,0x01
        jz      SwitchCPU
        cmp     ah,0x3B                 ; F1 - Toggle Show
        jz      ToggleShow
        ;
        cmp     al,0xB1                 ; dn
        jz      StkDn
        cmp     al,0xB2                 ; up
        jz      StkUp
        cmp     al,0xB4                 ; home
        jz      StkHome
        cmp     al,0xB7                 ; page-dn
        jz      StkPgDn
        cmp     al,0xB8                 ; page-up
        jz      StkPgUp
        cmp     al,'+'                  ; '+'
        jz      StkIncV
        cmp     al,'-'                  ; '-'
        jz      StkDecV
        cmp     al,'*'                  ; '*'
        jz      StkNotV
        cmp     al,'/'                  ; '/'
        jz      StkResV
        cmp     al,0xE                  ; BackSpace - GoTo MemAddr
        jz      StkGoToMem
;
        cmp     al,'a'
        jb      .Skip1
        cmp     al,'z'
        jbe     DoCommandLine
      .Skip1:
        ;
        cmp     al,'A'
        jb      .Skip2
        cmp     al,'Z'
        jbe     DoCommandLine
      .Skip2:
;
        jmp     WaitEvent
;-----------------------------------------------------------------------------
DoCommandLine:
        mov     [CmdLineActive],1
        jmp     GetCommandLine
;-----------------------------------------------------------------------------
Proceed:call    GoOn
        jmp     WaitEvent
;-----------------------------------------------------------------------------
SwitchCPU:
        and     [CurrentWindow],0
        jmp     WinSwitch
;-----------------------------------------------------------------------------
ToggleShow:
        xor     [ShowNames],-1
        jmp     CPUCommon
;-----------------------------------------------------------------------------
StkUp:  mov     eax,2
        cmp     [CodeType],16
        je      .1
        mov     eax,4
        cmp     [CodeType],32
        je      .1
        mov     eax,8
      .1:
        add     [StckAddr],rax
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkDn:  mov     eax,2
        cmp     [CodeType],16
        je      .1
        mov     eax,4
        cmp     [CodeType],32
        je      .1
        mov     eax,8
      .1:
        sub     [StckAddr],rax
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkPgUp:mov     eax,2*4
        cmp     [CodeType],16
        je      .1
        mov     eax,4*4
        cmp     [CodeType],32
        je      .1
        mov     eax,8*4
      .1:
        add     [StckAddr],rax
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkPgDn:mov     eax,2*4
        cmp     [CodeType],16
        je      .1
        mov     eax,4*4
        cmp     [CodeType],32
        je      .1
        mov     eax,8*4
      .1:
        sub     [StckAddr],rax
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkHome:
        and     [StckAddr],0
StkCommon:
        jmp     CPUCommon
;-----------------------------------------------------------------------------
StkGetAddress:
        mov     rax,[StckAddr]
        add     eax,[_ESP]
        ret
;-----------------------------------------------------------------------------
StkIncV:call    StkGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    SafeLoadDD
        inc     rax
        call    SafeStoreDD
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkDecV:call    StkGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    SafeLoadDD
        dec     rax
        call    SafeStoreDD
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkNotV:call    StkGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    SafeLoadDD
        not     rax
        call    SafeStoreDD
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkResV:call    StkGetAddress
        mov     rdi,rax
        xor     eax,eax
        call    SafeStoreDD
        jmp     StkCommon
;-----------------------------------------------------------------------------
StkGoToMem:
        call    StkGetAddress
        mov     [DataAddr],rax
        jmp     StkCommon
;-----------------------------------------------------------------------------
RegUp:  mov     ebp,[RegXPtr]
        dec     ebp
        jns     RegCommon
        jmp     WaitEvent
;-----------------------------------------------------------------------------
RegDn:  mov     ebp,[RegXPtr]
        inc     ebp
        cmp     ebp,17
        jb      RegCommon
        jmp     WaitEvent
;-----------------------------------------------------------------------------
RegLeft:
        mov     ebp,[RegXPtr]
        sub     ebp,8
        jns     RegCommon
        xor     ebp,ebp
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegRight:
        mov     ebp,[RegXPtr]
        add     ebp,8
        cmp     ebp,17
        jb      RegCommon
        jmp     WaitEvent
;-----------------------------------------------------------------------------
RegHome:xor     ebp,ebp
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegEnd: mov     ebp,16
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegXor: mov     ebp,[RegXPtr]
        cmp     ebp,17-8
        jae     RegIncR.0
        jmp     WaitEvent
;-----------------------------------------------------------------------------
RegIncR:mov     ebp,[RegXPtr]
        cmp     ebp,17-8
        jb      .1
.0:     mov     eax,[RegRTab+rbp*4]
        xor     [_EFL],eax
        jmp     RegCommon
.1:     mov     eax,[RegRTab+rbp*4]
        inc     dword [rax]
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegDecR:mov     ebp,[RegXPtr]
        cmp     ebp,17-8
        jb      .1
        mov     eax,[RegRTab+rbp*4]
        xor     [_EFL],eax
        jmp     RegCommon
.1:     mov     eax,[RegRTab+rbp*4]
        dec     dword [rax]
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegRes: mov     ebp,[RegXPtr]
        cmp     ebp,17-8
        jb      .1
        mov     eax,[RegRTab+rbp*4]
        not     eax
        and     [_EFL],eax
        jmp     RegCommon
.1:     mov     eax,[RegRTab+rbp*4]
        and     dword [rax],0
        jmp     RegCommon
;-----------------------------------------------------------------------------
RegNot: mov     ebp,[RegXPtr]
        cmp     ebp,17-8
        jb      .1
        mov     eax,[RegRTab+rbp*4]
        xor     [_EFL],eax
        jmp     RegCommon
.1:     mov     eax,[RegRTab+rbp*4]
        not     dword [rax]
       ;jmp     RegCommon
RegCommon:
        mov     [RegXPtr],ebp
        mov     eax,RegXTab
        movzx   eax,byte [rax+rbp]
        mov     [RegXPos],eax
        mov     eax,RegYTab
        movzx   eax,byte [rax+rbp]
        mov     [RegYPos],eax
        mov     eax,RegLTab
        movzx   eax,byte [rax+rbp]
        mov     [RegLDat],eax
        call    SetContext
        jmp     CPUCommon
;-----------------------------------------------------------------------------
RegGoToMem:
        mov     ebp,[RegXPtr]
        cmp     ebp,9
        jae     GoToMemRAX.Next
        mov     eax,[RegRTab+rbp*4]
        mov     rax,[rax]
GoToMemRAX:
        mov     [DataAddr],rax
.Next:  jmp     CPUCommon
;-----------------------------------------------------------------------------
MemUp:  dec     [MemYPos]
        cmp     [MemYPos],40
        ja      MemCommon
        inc     [MemYPos]
        sub     [DataAddr],8
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemDn:  inc     [MemYPos]
        cmp     [MemYPos],49
        jb      MemCommon
        dec     [MemYPos]
        add     [DataAddr],8
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemPageUp:
        sub     [DataAddr],8*8
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemPageDn:
        add     [DataAddr],8*8
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemLeft:sub     [MemXPos],3
        cmp     [MemXPos],11-3
        ja      MemCommon
        add     [MemXPos],3*8
        jmp     MemUp
;-----------------------------------------------------------------------------
MemRight:
        add     [MemXPos],3
        cmp     [MemXPos],11-1+3*8
        jb      MemCommon
        sub     [MemXPos],3*8
        jmp     MemDn
;-----------------------------------------------------------------------------
MemHome:and     [DataAddr],0
        mov     [MemXPos],11
MemCommon:
        jmp     CPUCommon
;-----------------------------------------------------------------------------
MemGetAddress:
        xor     edx,edx
        mov     ebx,3
        mov     eax,[MemXPos]
        sub     al,11
        div     ebx
        add     rax,[DataAddr]
        mov     ecx,[MemYPos]
        sub     cl,41
        add     ecx,ecx
        lea     rax,[rax+rcx*8]
        ret
;-----------------------------------------------------------------------------
MemIncB:call    MemGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    GetDataByte
        inc     al
        call    SafeStore
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemDecB:call    MemGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    GetDataByte
        dec     al
        call    SafeStore
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemNotB:call    MemGetAddress
        mov     rsi,rax
        mov     rdi,rax
        call    GetDataByte
        not     al
        call    SafeStore
        jmp     MemCommon
;-----------------------------------------------------------------------------
MemResB:call    MemGetAddress
        mov     rdi,rax
        mov     al,0
        call    SafeStore
        jmp     MemCommon
;-----------------------------------------------------------------------------
ToggleBP:
        mov     eax,[CPUYPos]
        mov     rax,[AddrBuffer+(rax-1)*sizeof.dq]
        push    rax
        call    FindBreakPoint
        mov     edx,eax
        inc     eax
        pop     rax
        jz      .NotFound
        mov     eax,edx
        call    ClearBreakPoint
        jmp     .Ret
.NotFound:
        mov     bl,1
        call    AddBreakPoint
        jnc     .Ret
        mov     esi,aBreakpointLimitExceeded
    .SayErr:
        call    PutMessage
    .Ret:
        jmp     CPUCommon
;-----------------------------------------------------------------------------
ReturnTo:
        mov     ecx,1024                        ; try 1024 instructions
        mov     esi,[_EIP]
.0:     mov     ebp,esi
        call    Decode
        mov     eax,[BufferO+35+SkipAddr]
        mov     ebx,'iret'
        cmp     eax,ebx                         ; check for IRET
        jz      .1
        and     eax,00FFFFFFh
        mov     ebx,00746572h                   ;' ter'
        cmp     eax,ebx                         ; check for RET
        jz      .1
        mov     ebx,00706D6Ah                   ;' pmj'
        cmp     eax,ebx                         ; check for JMP
        jz      .1
        loop    .0
        jmp     WaitEvent
.1:     call    OnResume.Jmp
        jmp     WaitEvent
;-----------------------------------------------------------------------------
JumpOver:
        mov     esi,[_EIP]
        call    Decode
        mov     eax,[BufferO+35+SkipAddr]
        cmp     eax,'call'              ; check for CALL
        jz      .DoIt
        cmp     eax,'loop'              ; check for LOOP..
        jz      .DoIt
        and     eax,00FFFFFFh
        mov     ebx,00746E69h           ;' tni'
        cmp     eax,ebx                 ; check for INT
        jz      .DoIt
        mov     ebx,00706572h           ;' per'
        cmp     eax,ebx                 ; check for REP
        jz      .DoIt
        mov     ebx,00706D6Ah           ;' pmj'
        cmp     eax,ebx                 ; check for JMP
        jz      .DoIt
        mov     ebx,006F6E6Ah           ;' onj'
        cmp     eax,ebx                 ; check for JNO
        jz      .DoIt
        mov     ebx,0065616Ah           ;' eaj'
        cmp     eax,ebx                 ; check for JAE
        jz      .DoIt
        mov     ebx,007A6E6Ah           ;' znj'
        cmp     eax,ebx                 ; check for JNZ
        jz      .DoIt
        mov     ebx,0065626Ah           ;' ebj'
        cmp     eax,ebx                 ; check for JBE
        jz      .DoIt
        mov     ebx,00736E6Ah           ;' snj'
        mov     ebx,00736E6Ah           ;' snj'
        cmp     eax,ebx                 ; check for JNS
        jz      .DoIt
        mov     ebx,0065706Ah           ;' epj'
        cmp     eax,ebx                 ; check for JPE
        jz      .DoIt
        mov     ebx,006F706Ah           ;' opj'
        cmp     eax,ebx                 ; check for JPO
        jz      .DoIt
        mov     ebx,0065676Ah           ;' egj'
        cmp     eax,ebx                 ; check for JGE
        jz      .DoIt
        mov     ebx,00656C6Ah           ;' elj'
        cmp     eax,ebx                 ; check for JLE
        jz      .DoIt
        and     eax,0000FFFFh
        mov     ebx,00006F6Ah           ;' oj'
        cmp     eax,ebx                 ; check for JO
        jz      .DoIt
        mov     ebx,0000626Ah           ;' bj'
        cmp     eax,ebx                 ; check for JB
        jz      .DoIt
        mov     ebx,00007A6Ah           ;' zj'
        cmp     eax,ebx                 ; check for JZ
        jz      .DoIt
        mov     ebx,0000616Ah           ;' aj'
        cmp     eax,ebx                 ; check for JA
        jz      .DoIt
        mov     ebx,0000736Ah           ;' sj'
        cmp     eax,ebx                 ; check for JS
        jz      .DoIt
        mov     ebx,00006C6Ah           ;' lj'
        cmp     eax,ebx                 ; check for JL
        jz      .DoIt
        mov     ebx,0000676Ah           ;' gj'
        cmp     eax,ebx                 ; check for JG
        jz      .DoIt
        jmp     F7
.DoIt:  mov     ebp,esi
        call    OnResume.Jmp
        jmp     WaitEvent
;-----------------------------------------------------------------------------
ComeHere:
        mov     eax,[CPUYPos]
        mov     rax,[AddrBuffer+(rax-1)*sizeof.dq]
        push    eax
        call    FindEnabledBreakPoint
        pop     eax
        jz      .GoOn
        mov     bl,5    ; valid enabled one-shot
        call    AddBreakPoint
        jnc     .GoOn
        mov     esi,aBreakpointLimitExceeded
        call    PutMessage
        jmp     WaitEvent
.GoOn:  call    GoOn
        jmp     WaitEvent
;-----------------------------------------------------------------------------
NewAddress:
        mov     eax,[CPUYPos]
        mov     rax,[AddrBuffer+(rax-1)*sizeof.dq]
        mov     [_EIP],eax
        call    SetContext
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPUUp:  cmp     [CPUYPos],1
        jbe     .1
        dec     [CPUYPos]
        mov     ebx,[CPUYPos]
        mov     rsi,[AddrBuffer+(rbx-1)*sizeof.dq]
        call    Decode
        jmp     CPUCommon
.1:     call    FindCodeUp
        mov     [CodeAddr],rax
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPUDn:  cmp     [CPUYPos],28
        jae     .1
        inc     [CPUYPos]
        mov     ebx,[CPUYPos]
        mov     rsi,[AddrBuffer+(rbx-1)*sizeof.dq]
        call    Decode
        jmp     CPUCommon
.1:     mov     rax,[AddrBuffer+sizeof.dq]
        mov     [CodeAddr],rax
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPULeft:dec     [CodeAddr]
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPURight:
        inc     [CodeAddr]
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPUPgUp:mov     ecx,28
.1:     call    FindCodeUp
        mov     [AddrBuffer],rax
        loop    .1
        mov     [CodeAddr],rax
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPUPgDn:mov     rsi,[AddrBuffer+27*sizeof.dq]
        call    Decode
        mov     [CodeAddr],rsi
        jmp     CPUCommon
;-----------------------------------------------------------------------------
CPUHome:mov     eax,[_EIP]
        mov     [CodeAddr],rax
        mov     [CPUYPos],1
CPUCommon:
        mov     [AfterKey],1
        call    ShowImage
        jmp     WaitEvent
;-----------------------------------------------------------------------------
SafeLoadDD:
        push    rcx rsi
        mov     ecx,4
.Loop:  call    GetDataByte
        ror     rax,8
        inc     rsi
        loop    .Loop
        pop     rsi rcx
        ret
;-----------------------------------------------------------------------------
CPUGoToMem:
        mov     eax,[CPUYPos]
        mov     rsi,[(rax-1)*sizeof.dq+AddrBuffer]
        call    Decode
        mov     al,[Inst.Arg1.Mem]
        or      al,[Inst.Arg2.Mem]
        or      al,[Inst.Arg3.Mem]
        or      al,[Inst.Arg4.Mem]
        or      al,[Inst.Arg5.Mem]
        jnz     .Mem
        ;
        mov     rdx,[Inst.Arg1.Imm]
        cmp     [Inst.IsAddress],0      ;relative
        jnz     .Ofs

        cmp     [Inst.MemSeparator],0
        jz      .NoFar
        mov     ax,[Inst.Point]
        cmp     ax,TDWORD
        jz      .DW2
        cmp     ax,TFWORD
        jz      .FW2
      .NoFar:
        jmp     WaitEvent
      .DW2:
        sub     rsi,4
        call    SafeLoadDD
        movzx   edx,ax
        shr     eax,16
        jmp     .All
      .FW2:
        sub     rsi,6
        call    SafeLoadDD
        mov     edx,eax
        add     rsi,4
        call    SafeLoadDD
        movzx   eax,ax
        jmp     .All
        ;
      .Mem:
        ;
        cmp     [Inst.Point],0
        jz      .Ptr
        movzx   edx,[Inst.RandSize]
        and     dl,7Fh
        push    rdx
        call    GetLinAddr
        pop     rdx
        cmp     dl,3
        jz      .DW
        cmp     dl,8
        jz      .FW
        cmp     dl,9
        jz      .TW
        jmp     WaitEvent
      .DW:
        mov     rsi,rax
        call    SafeLoadDD
        movzx   edx,ax
        shr     eax,16
        jmp     .All
      .FW:
        mov     rsi,rax
        call    SafeLoadDD
        mov     edx,eax
        add     rsi,4
        call    SafeLoadDD
        movzx   eax,ax
        jmp     .All
      .TW:
        mov     rsi,rax
        call    SafeLoadDD
        mov     rdx,rax
        add     rsi,8
        call    SafeLoadDD
        movzx   eax,ax
      .All:
       ;verr    eax
       ;jnz     .Err
       ;mov     [Reg.CS],eax
       ;call    SelectorInit
      .Ofs:
        mov     [CodeAddr],rdx
        mov     [CPUYPos],1
if 0
        call    RepaintText
        jmp     ShowRegs
else
        jmp     CPUCommon
end if
      .Err:
        jmp     WaitEvent
      .Ptr:
        call    GetMemAddress
        jmp     GoToMemRAX
;-----------------------------------------------------------------------------
GetLinAddr:
        call    GetSeg
        mov     [AddressStr],dx
        ;
        xor     ebx,ebx
        mov     ecx,1
        ;
        movzx   eax,[Inst.Scale]
        cmp     al,-1
        jz      @F
        mov     cl,[rax+AnScale]
      @@:
        movzx   eax,[Inst.Base]
        cmp     al,-1
        jz      @F
        mov     eax,[rax*4+RegVTab]
        mov     rax,[rax]
        call    Address2Reg
        add     rbx,rax
      @@:
        movzx   eax,[Inst.Indx]
        cmp     al,-1
        jz      @F
        mov     eax,[rax*4+RegVTab]
        mov     rax,[rax]
        call    Address2Reg
        mul     rcx
        add     rbx,rax
      @@:
        cmp     [Inst.DispSize],0
        jz      @F
        mov     rax,[Inst.Disp]
        call    Address2Reg
        add     rbx,rax
      @@:
        mov     rax,rbx
        call    Address2Reg
        mov     rbx,rax
      AddSeg:
if 0
        call    GetSegm
        movzx   eax,al
        mov     rax,[rax*sizeof.dq+ESBase]
        add     rax,rbx
end if
        ret
;-----------------------------------------------------------------------------
ShowFPURegs:
if 1
        mov     ah,3Fh
        xor     ecx,ecx
        mov     edi,ConsoleDataPtr
        mov     al,     'Â'
        imul    edx,[CurWidth],49+1
        sub     edx,31
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,28
        call    DrawLine
        imul    edx,[CurWidth],50+1
        sub     edx,31
        mov     al,     '³'
        mov     cl,9
        call    DrawDown
        mov     al,     'º'
        imul    edx,[CurWidth],55
        dec     edx
        call    DrawChar
  if 0
        mov     al,     'À'
        imul    edx,[CurWidth],49+1+10
        sub     edx,31
        call    DrawChar
        mov     al,     'Ä'
        mov     cl,28+1
        call    DrawLine
  end if
end if
        mov     ebp,[CurWidth]
        lea     eax,[ebp-28]
        mov     [Color],3Ah
        mov     [XPos],al
        mov     [YPos],49
        lea     edx,[Msg10F]
        call    PrintS
        mov     [Color],30h
        lea     eax,[ebp-30]
        mov     [XPos],al
        mov     [YPos],50
        lea     edx,[Msg11F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg12F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg13F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg14F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg15F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg16F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg17F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg18F]
        call    PrintS
        lea     eax,[ebp-30]
        mov     [XPos],al
        inc     [YPos]
        lea     edx,[Msg19F]
        call    PrintS
        fninit                  ; reinitialize FPU
.1:     lea     eax,[ebp-26]
        mov     [XPos],al
        mov     [YPos],50
        xor     edi,edi         ; displacement
        mov     bl,8            ; counter
        movzx   eax,word [_FTW]
        mov     [Temp],eax
        movzx   eax,[_FSW]
        shr     eax,11
        and     eax,07h
        lea     ecx,[rax*2]
        ror     word [Temp],cl
.Loop:  ;
        mov     [Color],30h
        mov     eax,dword [_ST0+rdi]
        cmp     eax,dword [_ST0+rdi+Plus]
        jnz     .NotEQ
        mov     eax,dword [_ST0+rdi+4]
        cmp     eax,dword [_ST0+rdi+Plus+4]
        jnz     .NotEQ
        mov     ax,word [_ST0+rdi+8]
        cmp     ax,word [_ST0+rdi+Plus+8]
        jz      .YeaEQ
.NotEQ: mov     [Color],3Fh
.YeaEQ:
        mov     eax,[Temp]
        ror     word [Temp],02h
        and     eax,0003h
        cmp     eax,0003h
        jz      .__ftoa_empty
        fld     tbyte [_ST0+rdi]
        fxam
        fstsw   ax
        mov     al,ah
;       C=          3   210
        and     al,01000101b
        jz      .__ftoa_err     ; unsupported number
        cmp     al,00000001b
        jz      .__ftoa_NaN     ; NaN number
        cmp     al,00000101b
        jz      .__ftoa_inf     ; infinity
        cmp     al,01000000b
        jz      .__ftoa_zero    ; zero
        cmp     al,01000100b
        jz      .__ftoa_den     ; denormal
        cmp     al,01000001b
        jz      .__ftoa_unu     ; unused
        cmp     al,01000101b
        jz      .__ftoa_unu     ; unused
        mov     eax,57
        call    _float2dec
        add     eax,17
        mov     edx,eax         ; save exponent in edx

        sub     esp,12
        fbstp   tbyte [rsp]
        mov     cx,[rsp+8]
        mov     esi,[rsp+4]
        mov     ebp,[rsp+0]
        add     esp,12

        mov     bh,15
        and     ch,80h
        mov     al,'+'
        jz      .2
        mov     al,'-'
.2:     call    PrintC
        call    __ftoa_char
        call    PrintC
        mov     al,'.'
        call    PrintC
.3:     call    __ftoa_char
        call    PrintC
        dec     bh
        jnz     .3

        mov     al,'E'
        call    PrintC
        test    edx,edx
        mov     al,'+'
        jns     .4
        mov     al,'-'
        neg     edx
.4:     call    PrintC

        mov     bh,4
        mov     eax,edx
        xor     ebp,ebp
.5:     mov     ecx,10
        xor     edx,edx
        idiv    ecx
        shl     ebp,8
        or      ebp,edx
        dec     bh
        jnz     .5

        mov     bh,4
.6:     mov     eax,ebp
        add     al,30h
        shr     ebp,8
        call    PrintC
        dec     bh
        jnz     .6

.Next:  add     edi,16
        mov     eax,[CurWidth]
        sub     eax,26
        mov     [XPos],al
        inc     [YPos]
        dec     bl
        jnz     .Loop

        mov     eax,[CurWidth]
        sub     eax,27
        mov     [XPos],al
        mov     [Color],30h
        mov     ax,[_FCW]       ; CW
        cmp     ax,[_FCW+Plus]
ifnz    mov     [Color],3Fh
        call    PrintAX
        add     [XPos],5
        mov     [Color],30h
        mov     ax,[_FSW]       ; SW
        cmp     ax,[_FSW+Plus]
ifnz    mov     [Color],3Fh
        call    PrintAX
        add     [XPos],6
        mov     [Color],30h
        mov     ax,word [_FTW]  ; Tag
        cmp     ax,word [_FTW+Plus]
ifnz    mov     [Color],3Fh
        call    PrintAX
        fwait
        ret

.__ftoa_err:
        lea     edx,[Msg1EF1]
        jmp     .__ftoa_
.__ftoa_NaN:
        and     ah,00000010h
        mov     al,'+'
        jz      .__ftoa_NaN_
        mov     al,'-'
.__ftoa_NaN_:
        lea     edx,[Msg1EF2]
        mov     [rdx],al
        jmp     .__ftoa_
.__ftoa_inf:
        and     ah,00000010h
        mov     al,'+'
        jz      .__ftoa_inf_
        mov     al,'-'
.__ftoa_inf_:
        lea     edx,[Msg1EF3]
        mov     [rdx],al
        jmp     .__ftoa_
.__ftoa_zero:
        and     ah,00000010h
        mov     al,'+'
        jz      .__ftoa_zero_
        mov     al,'-'
.__ftoa_zero_:
        lea     edx,[Msg1EF4]
        mov     [rdx],al
        jmp     .__ftoa_
.__ftoa_den:
        and     ah,00000010h
        mov     al,'+'
        jz      .__ftoa_den_
        mov     al,'-'
.__ftoa_den_:
        lea     edx,[Msg1EF5]
        mov     [rdx],al
        jmp     .__ftoa_
.__ftoa_unu:
        lea     edx,[Msg1EF6]
        jmp     .__ftoa_
.__ftoa_empty:
        lea     edx,[Msg1EF7]
.__ftoa_:
        call    PrintS
        jmp     .Next

__ftoa_char:
        mov     al,cl
        shl     ebp,1
        rcl     esi,1
        rcl     cl,1
        shl     ebp,1
        rcl     esi,1
        rcl     cl,1
        shl     ebp,1
        rcl     esi,1
        rcl     cl,1
        shl     ebp,1
        rcl     esi,1
        rcl     cl,1
        shr     al,4
        add     al,30h
        ret

_float2dec:
        sub     esp,3*4
        fstcw   word [rsp]
        mov     word [rsp+2],03BFh
        fldcw   word [rsp+2]
        mov     [rsp+4],eax
        fld     st0
        fxtract
        fstp    st0
        fisubr  dword [rsp+4]
        fldl2t
        fdivp   st1,st0
        frndint
        fist    dword [rsp+8]
        call    _exp10
        fmulp   st1,st0
        fldcw   word [rsp]
        mov     eax,[rsp+2*4]
        add     esp,3*4
        neg     eax
        ret

_exp10: fldl2t
        fmulp   st1,st0
_exp2:  sub     esp,2*4
        fstcw   word [rsp]
        mov     word [rsp+2],03BFh
        fldcw   word [rsp+2]
        fld     st0
        frndint
        fxch
        fsub    st0,st1
        ftst
        fstsw   word [rsp+4]
        and     byte [rsp+5],45h
        cmp     byte [rsp+5],01h
        ja      .Err
        je      .Neg
        f2xm1
        fld1
        faddp   st1,st0
        fxch
        fld1
        fscale
        fmulp   st2,st0
        fstp    st0
        jmp     .Done
.Neg:   fabs
        f2xm1
        fld1
        faddp   st1,st0
        fxch
        fld1
        fscale
        fdivrp  st2,st0
        fstp    st0
.Done:  fldcw   word [rsp]
        add     esp,2*4
        ret
.Err:   fstp    st0
        fstp    st0
        fld1
        jmp     .Done
;-----------------------------------------------------------------------------
ShowMMXRegs:
if 1
        mov     ah,3Fh
        xor     ecx,ecx
        mov     edi,ConsoleDataPtr
        mov     al,     'Â'
        imul    edx,[CurWidth],49+1
        sub     edx,52
        call    DrawChar
        imul    edx,[CurWidth],50+1
        sub     edx,52
        mov     al,     '³'
        mov     cl,9
        call    DrawDown
        mov     al,     'º'
        imul    edx,[CurWidth],54
        dec     edx
        call    DrawChar
end if
        mov     ebx,[CurWidth]
        mov     bh,bl
        sub     bh,47
        sub     bl,51

        mov     [Color],3Ah
        mov     [XPos],bl
        add     [XPos],2
        mov     [YPos],49
        lea     edx,[MsgMMX]
        call    PrintS
        mov     [Color],30h
        mov     [XPos],bl
        mov     [YPos],50
        lea     edx,[MsgMM0]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM1]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM2]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM3]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM4]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM5]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM6]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgMM7]
        call    PrintS
        mov     [XPos],bl
        inc     [YPos]
        lea     edx,[MsgXMMX]
        call    PrintS
        inc     [XPos]
        add     edx,4
        mov     [Color],3Eh
        call    PrintS
        mov     [YPos],50
        mov     bl,8            ; counter
        lea     esi,[_MM0]
.Skip:  lea     ecx,[_MM0+Plus]
.Loop:  mov     [XPos],bh
        mov     [Color],30h
        mov     eax,[rsi+0]
        cmp     eax,[rcx+0]
        jne     @F
        mov     eax,[rsi+4]
        cmp     eax,[rcx+4]
        je      .@
@@:     mov     [Color],3Fh
.@:     mov     eax,[rsi+4]
        call    PrintEAX
        mov     eax,[rsi+0]
        call    PrintEAX
        inc     [YPos]
        add     esi,16
        add     ecx,16
        dec     bl
        jnz     .Loop
        ret
;-----------------------------------------------------------------------------
ShowXMMRegs:
        mov     [Color],3Ah
        mov     [XPos],3
        mov     [YPos],49
        lea     edx,[MsgXMM]
        call    PrintS
        mov     [Color],30h
        mov     [XPos],1
        mov     [YPos],50
        lea     edx,[MsgXM0]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM1]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM2]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM3]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM4]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM5]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM6]
        call    PrintS
        mov     [XPos],1
        inc     [YPos]
        lea     edx,[MsgXM7]
        call    PrintS
        mov     [YPos],50
        mov     bl,8            ; counter
        lea     esi,[_XMM0]
.Skip:  lea     ecx,[_XMM0+Plus]
.Loop:  mov     [XPos],6
        mov     [Color],30h
        mov     eax,[rsi+4*0]
        cmp     eax,[rcx+4*0]
        jne     @F
        mov     eax,[rsi+4*1]
        cmp     eax,[rcx+4*1]
        jne     @F
        mov     eax,[rsi+4*2]
        cmp     eax,[rcx+4*2]
        jne     @F
        mov     eax,[rsi+4*3]
        cmp     eax,[rcx+4*3]
        je      .@
@@:     mov     [Color],3Fh
.@:     mov     eax,[rsi+4*3]
        call    PrintEAX
        mov     eax,[rsi+4*2]
        call    PrintEAX
        mov     eax,[rsi+4*1]
        call    PrintEAX
        mov     eax,[rsi+4*0]
        call    PrintEAX
        inc     [YPos]
        add     esi,16
        add     ecx,16
        dec     bl
        jnz     .Loop
        mov     [XPos],1
        lea     edx,[MsgXCSR]
        call    PrintS
        mov     [XPos],1+6+9
        lea     edx,[MsgMCSR]
        call    PrintS
        mov     [XPos],7
        mov     eax,[_MXCSR]
        cmp     eax,[_MXCSR+Plus]
        mov     [Color],30h
ifnz    mov     [Color],3Fh
        call    PrintEAX
        mov     [XPos],7+9+11
        mov     eax,[_MXCSRM]
        cmp     eax,[_MXCSRM+Plus]
        mov     [Color],30h
ifnz    mov     [Color],3Fh
        call    PrintEAX
        ret
;-----------------------------------------------------------------------------
Msg10   db 'CPU Registers',0
;-----------------------------------------------------------------------------
Msg11   db 'EAX 00000000 EFL 00000000          ',0
Msg12   db 'EBX 00000000 EIP 00000000          ',0
Msg13   db 'ECX 00000000 CS 0000/00000000/0000 ',0
Msg14   db 'EDX 00000000 DS 0000/00000000/0000 ',0
Msg15   db 'ESI 00000000 ES 0000/00000000/0000 ',0
Msg16   db 'EDI 00000000 FS 0000/00000000/0000 ',0
Msg17   db 'EBP 00000000 GS 0000/00000000/0000 ',0
Msg18   db 'ESP 00000000 SS 0000/00000000/0000 ',0
Msg19   db 'V R N IO O D I T S  Z A P C ',0
Msg20   db '0 0 0 00 0 0 0 0 0  0 0 0 0 ',0
;-----------------------------------------------------------------------------
Msg22   db '[ds:esi]',0
Msg23   db '[es:edi]',0
MsgX2   db '[ds:edx]',0
MsgX3   db '[ds:ebx]',0
MsgX4   db '[ss:ebp]',0
MsgX5   db '[ds:ecx]',0
;-----------------------------------------------------------------------------
Msg2SIP db '[esi+',0
Msg2SIM db '[esi-',0
Msg2DIP db '[edi+',0
Msg2DIM db '[edi-',0
Msg2BXP db '[ebx+',0
Msg2BXM db '[ebx-',0
Msg2DXP db '[edx+',0
Msg2DXM db '[edx-',0
Msg2BPP db '[ebp+',0
Msg2BPM db '[ebp-',0
Msg3SPP db '[esp+',0
Msg3SPM db '[esp-',0
Msg2ER  db '--',0
;-----------------------------------------------------------------------------
Msg00   db 'CPU ',0
CPUTable \
        dd '8086'
        dd '186'
        dd '286'
        dd '386'
        dd '486'
        dd '586'
        dd '686'
FPUTable \
        db '/None',0,0,0
        db '/8087',0,0,0
        dd '/287',0
        dd '/387',0
        dd '/487',0
        dd '/587',0
        dd '/687',0
;-----------------------------------------------------------------------------
Msg21   db 'Memory',0
;-----------------------------------------------------------------------------
Msg30   db 'Stack',0
;-----------------------------------------------------------------------------
Msg3M   db 'Messages',0
;-----------------------------------------------------------------------------
Msg40   db 'Address: ',8,3Fh,'00000000 ',8,3Ah,'=',8,3Fh,' 00h',0
;-----------------------------------------------------------------------------
Msg10F  db 'FPU Registers',0
Msg11F  db 'ST0',0
Msg12F  db 'ST1',0
Msg13F  db 'ST2',0
Msg14F  db 'ST3',0
Msg15F  db 'ST4',0
Msg16F  db 'ST5',0
Msg17F  db 'ST6',0
Msg18F  db 'ST7',0
Msg19F  db 'CW=0000  SW=0000  TAG=0000',0
;-----------------------------------------------------------------------------
MsgMMX  db 'MMX Registers',0
MsgMM0  db 'MM0',0
MsgMM1  db 'MM1',0
MsgMM2  db 'MM2',0
MsgMM3  db 'MM3',0
MsgMM4  db 'MM4',0
MsgMM5  db 'MM5',0
MsgMM6  db 'MM6',0
MsgMM7  db 'MM7',0
;-----------------------------------------------------------------------------
MsgXMM  db 'XMM Registers',0
MsgXM0  db 'XMM0',0
MsgXM1  db 'XMM1',0
MsgXM2  db 'XMM2',0
MsgXM3  db 'XMM3',0
MsgXM4  db 'XMM4',0
MsgXM5  db 'XMM5',0
MsgXM6  db 'XMM6',0
MsgXM7  db 'XMM7',0
;-----------------------------------------------------------------------------
Msg1EF1 db 'Unsupported             ',0
Msg1EF2 db '+NaN                    ',0
Msg1EF3 db '+Infinity               ',0
Msg1EF4 db '+0.0                    ',0
Msg1EF5 db '+Denormal               ',0
Msg1EF6 db 'Unused                  ',0
Msg1EF7 db 'Empty                   ',0
;-----------------------------------------------------------------------------
MsgXMMX db 'CPU',0,'GenuineIntel',0
;-----------------------------------------------------------------------------
MsgXCSR db 'MXCSR=',0
MsgMCSR db 'MXCSR_MASK=',0
;-----------------------------------------------------------------------------
Msg50   db 8,74h,'F1',8,70h,'-Show '
        db 8,74h,'F2',8,70h,'-BreakPnt '
        db 8,74h,'F3',8,70h,'-Return '
        db 8,74h,'F4',8,70h,'-Here '
        db 8,74h,'F5',8,70h,'-Screen '
        db 8,74h,'F6',8,70h,'-New '
        db 8,74h,'F7',8,70h,'-Trace '
        db 8,74h,'F8',8,70h,'-Step '
        db 8,74h,'F9',8,70h,'-Run '
        db 8,74h,'F10',8,70h,'-Over '
        db 0
;-----------------------------------------------------------------------------
even 4
RegVTab dd _EAX,_ECX,_EDX,_EBX,_ESP,_EBP,_ESI,_EDI,_EIP
RegRTab dd _EAX,_EBX,_ECX,_EDX,_ESI,_EDI,_EBP,_ESP,_EIP
        dd 0800h,0400h,0200h,0080h,0040h,0010h,0004h,0001h
;-----------------------------------------------------------------------------
RegXTab db  1, 1, 1, 1, 1, 1, 1, 1, 14, 10,12,14,18, 21,23,25,27
RegYTab db 30,31,32,33,34,35,36,37, 31, 39,39,39,39, 39,39,39,39
RegLTab db 12,12,12,12,12,12,12,12, 12,  1, 1, 1, 1,  1, 1, 1, 1
;-----------------------------------------------------------------------------
CursorTab       dd CPUCursor,RegCursor,MemCursor,StkCursor
WindowTab       dd CPUWindow,RegWindow,MemWindow,StkWindow
;-----------------------------------------------------------------------------
HexBuf: db 8 dup 20h,0
;-----------------------------------------------------------------------------
XPos    db      0
YPos    db      0
Color   db      7
;-----------------------------------------------------------------------------