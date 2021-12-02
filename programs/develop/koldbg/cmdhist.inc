InitCmdHistBuffer:
;-------------------------------------------------------------------------------
        push    eax ebx ecx

        mcall   68,12,4096      ;allocate 4KB block
        mov     [CmdHistBuffer.Ptr],eax
        mov     [CmdHistBuffer.Size],ecx

        mov     edi,eax
        xor     eax,eax
        stosd
        stosd
        mov     [CmdHistBuffer.NewNodeOffset],eax
        mov     [CmdHistBuffer.LastNodeOffset],eax
        mov     [CmdHistBuffer.CurNodeOffset],eax
        mov     [CmdHistBuffer.TmpLineFlag],al
        mov     [CmdHistBuffer.NFlag],al

        pop     ecx ebx eax
        ret
;-------------------------------------------------------------------------------

EnlargeCmdHistBuffer:
;-------------------------------------------------------------------------------
        pusha

        mov     ecx,[CmdHistBuffer.Size]
        add     ecx,4096
        mov     edx,[CmdHistBuffer.Ptr]
        mcall   68,20           ;reallocate the buffer (prev. size + 4KB)
        test    eax,eax
        jz      .Err
        mov     [CmdHistBuffer.Ptr],eax
        mov     [CmdHistBuffer.Size],ecx
.OK:
        clc
        popa
        ret
.Err:
        stc
        popa
        ret
;-------------------------------------------------------------------------------

AddCmdHistLine:
;-------------------------------------------------------------------------------
;in:
;eax = ptr to ASCIIZ string of line
;-------------------------------------------------------------------------------
        pusha

        mov     esi,eax
        mov     edx,eax
        xor     ecx,ecx
@@:
        inc     ecx
        lodsb
        test    al,al
        jz      @F
        jmp     @B
@@:
        dec     ecx
        jz      .OK

        inc     ecx

        mov     eax,[CmdHistBuffer.NewNodeOffset]
        lea     eax,[eax+ecx+2*4]
        cmp     [CmdHistBuffer.Size],eax
        jae     @F
        call    EnlargeCmdHistBuffer
        jc      .Err
@@:
        mov     edi,[CmdHistBuffer.NewNodeOffset]
        mov     eax,[CmdHistBuffer.Ptr]
        add     edi,eax
        cmp     eax,edi                 ;if eax = edi, then it's the first node
        jne     @F
        xor     eax,eax
        dec     eax
        stosd                           ;save previous node link = -1 for the first node
        stosd                           ;save next node link = -1 for the first node
        jmp     .CPStr
@@:
        mov     ebx,[CmdHistBuffer.LastNodeOffset]
        mov     [edi],ebx               ;save previous node link for a new node
        add     ebx,eax
        mov     eax,[CmdHistBuffer.NewNodeOffset]
        mov     [ebx+4],eax             ;update next node link for a last node
        mov     [CmdHistBuffer.LastNodeOffset],eax
        mov     [CmdHistBuffer.CurNodeOffset],eax
        add     edi,4
        xor     eax,eax
        dec     eax
        stosd
.CPStr:
        mov     esi,edx

        push    ecx
        rep     movsb
        pop     ecx

        add     ecx,2*4
        add     [CmdHistBuffer.NewNodeOffset],ecx
.OK:
        clc
.Err:
        mov     [CmdHistBuffer.TmpLineFlag],0
        popa
        ret
;-------------------------------------------------------------------------------

AddCmdHistTmpLine:
;-------------------------------------------------------------------------------
;in:
;eax = ptr to ASCIIZ string of line
;-------------------------------------------------------------------------------
        pusha

        mov     esi,eax
        mov     edx,eax
        xor     ecx,ecx
@@:
        inc     ecx
        lodsb
        test    al,al
        jz      @F
        jmp     @b
@@:
        mov     eax,[CmdHistBuffer.NewNodeOffset]
        lea     eax,[eax+ecx+2*4]
        cmp     [CmdHistBuffer.Size],eax
        jae     @F
        call    EnlargeCmdHistBuffer
        jc      .Err
@@:
        mov     edi,[CmdHistBuffer.NewNodeOffset]
        mov     [CmdHistBuffer.TmpLineFlag],1
        mov     eax,[CmdHistBuffer.Ptr]
        add     edi,eax
        xor     eax,eax
        stosd
        stosd   ;two zeroes - marker (prefix) of tmp line
.CPStr:
        mov     esi,edx
        rep     movsb
.OK:
        clc
.Err:
        popa
        ret
;-------------------------------------------------------------------------------

GetCmdHistLine:
;-------------------------------------------------------------------------------
;out:
;eax = ptr to ASCIIZ string of current line
;-------------------------------------------------------------------------------
        push    ebx

        mov     eax,[CmdHistBuffer.Ptr]
        mov     ebx,[CmdHistBuffer.CurNodeOffset]

        lea     eax,[eax+ebx+2*4]

        pop     ebx
        ret
;-------------------------------------------------------------------------------

GetCmdHistTmpLine:
;-------------------------------------------------------------------------------
;out:
;eax = ptr to ASCIIZ string of current line or 0 (no tmp line)
;-------------------------------------------------------------------------------
        cmp     [CmdHistBuffer.TmpLineFlag],0
        je      @F

        mov     eax,[CmdHistBuffer.NewNodeOffset]
        add     eax,[CmdHistBuffer.Ptr]
        add     eax,2*4
@@:
        ret
;-------------------------------------------------------------------------------

PrevCmdHistLine:
;-------------------------------------------------------------------------------
;out:
;CF = 0 - OK, CF = 1 (no previous lines)
;-------------------------------------------------------------------------------
        push    eax ebx

        mov     eax,[CmdHistBuffer.Ptr]
        mov     ebx,[CmdHistBuffer.CurNodeOffset]

        lea     eax,[eax+ebx]
        mov     eax,[eax]
        inc     eax
        jz      .RetCF
        dec     eax
        mov     [CmdHistBuffer.CurNodeOffset],eax

        clc
        pop     ebx eax
        ret
.RetCF:
        stc
        pop     ebx eax
        ret
;-------------------------------------------------------------------------------

NextCmdHistLine:
;-------------------------------------------------------------------------------
;out:
;CF = 0 - OK, CF = 1 (no next lines)
;-------------------------------------------------------------------------------
        push    eax ebx

        mov     eax,[CmdHistBuffer.Ptr]
        mov     ebx,[CmdHistBuffer.CurNodeOffset]

        lea     eax,[eax+ebx]
        mov     eax,[eax+4]
        inc     eax
        jz      .RetCF
        dec     eax
        mov     [CmdHistBuffer.CurNodeOffset],eax

        clc
        pop     ebx eax
        ret
.RetCF:
        stc
        pop     ebx eax
ClearCmdLineEnd:
        ret
;-------------------------------------------------------------------------------

DrawCmdLine:
        pushad
        xor     ebx,ebx
        jmp     @F

DrawCmdLineEnd:
        pushad
        mov     ebx,[CmdLinePos]
    @@:
        call    ClearCmdLine

        mov     [XPos],1
        mov     [YPos],MinHeight-1
        mov     [Color],15
        mov     al,'>'
        call    PrintC
        mov     ecx,[CmdLineLen]
        sub     ecx,ebx
        jbe     .Skip
        lea     esi,[CmdLine+ebx]
    .Loop:
        lodsb
        call    PrintC
        loop    .Loop
    .Skip:
        popad
        ret

DrawCursor:
        pushad
        mov     eax,[CmdLinePos]
times 2 inc     eax
        mov     [CursorX],eax
        mov     [CursorY],MinHeight-1
        mov     [AfterKey],1
        call    ShowImage
        popad
        ret

HideCursor:
        pushad
        or      [CursorX],-1
        or      [CursorY],-1
        mov     [AfterKey],1
        call    ShowImage
        popad
        ret

;-----------------------------------------------------------------------------
;                          Find command in list
FindCmd:
; all commands are case-insensitive
        push    edi
    .X4:
        mov     al,[edi]
        cmp     al,0
        jz      .X5
        cmp     al,'A'
        jb      @F
        cmp     al,'Z'
        ja      @F
        or      al,20h
    @@:
        stosb
        jmp     .X4

    ; find command
    .X5:
        pop     edi
    .X6:
        cmp     dword [esi],0
        jz      .X7
        push    esi
        mov     esi,[esi]
        lodsb
        movzx   ecx,al
        push    edi
        repz    cmpsb
        pop     edi
        pop     esi
        jz      .X8
        add     esi,17
        jmp     .X6
    .X7:
        stc
    .X8:
        ret

HelpGroups:
        dd      aControl,0,0,HelpControlMsg
        db      0
        dd      aData,0,0,HelpDataMsg
        db      0
        dd      aBreakpoints,0,0,HelpBreaksMsg
        db      0
; flags field:
; &1: command may be called without parameters
; &2: command may be called with parameters
; &4: command may be called without loaded program
; &8: command may be called with loaded program
Commands:
        dd      aHlp,OnHelp,HelpSyntax,HelpHelp
        db      0Fh
        dd      aHelp,OnHelp,HelpSyntax,HelpHelp
        db      0Fh
        dd      aQuit,OnQuit,QuitSyntax,QuitHelp
        db      0Dh
        dd      aExit,OnQuit,ExitSyntax,ExitHelp
        db      0Dh
        dd      aLoad,OnLoad,LoadSyntax,LoadHelp
        db      6
        dd      aReload,OnReLoad,ReloadSyntax,ReloadHelp
        db      0Dh
        dd      aTerminate,OnTerminate,TerminateSyntax,TerminateHelp
        db      9
        dd      aDetach,OnDetach,DetachSyntax,DetachHelp
        db      9
        dd      aSuspend,OnSuspend,SuspendSyntax,SuspendHelp
        db      9
        dd      aResume,OnResume,ResumeSyntax,ResumeHelp
        db      0Bh
        dd      aStep,OnStepMultiple,StepSyntax,StepHelp
        db      0Bh
        dd      aProceed,OnProceedMultiple,ProceedSyntax,ProceedHelp
        db      0Bh
        dd      aCalc,OnCalc,CalcSyntax,CalcHelp
        db      0Eh
        dd      aDump,OnDump,DumpSyntax,DumpHelp
        db      0Bh
       ;dd      aUnassemble,OnUnassemble,UnassembleSyntax,UnassembleHelp
       ;db      0Bh
        dd      aBp,OnBp,BpSyntax,BpHelp
        db      0Ah
        dd      aBpm,OnBpmb,BpmSyntax,BpmHelp
        db      0Ah
        dd      aBpmb,OnBpmb,BpmSyntax,BpmHelp
        db      0Ah
        dd      aBpmw,OnBpmw,BpmSyntax,BpmHelp
        db      0Ah
        dd      aBpmd,OnBpmd,BpmSyntax,BpmHelp
        db      0Ah
        dd      aBl,OnBl,BlSyntax,BlHelp
        db      0Bh
        dd      aBc,OnBc,BcSyntax,BcHelp
        db      0Ah
        dd      aBd,OnBd,BdSyntax,BdHelp
        db      0Ah
        dd      aBe,OnBe,BeSyntax,BeHelp
        db      0Ah
        dd      aReg,OnReg,RSyntax,RHelp
        db      0Ah
        dd      aUnpack,OnUnpack,UnpackSyntax,UnpackHelp
        db      9
        dd      aLoadSymbols,OnLoadSymbols,LoadSymbolsSyntax,LoadSymbolsHelp
        db      0Ah
        ;
        dd      aAsm,OnAsm,AsmSyntax,AsmHelp
        db      2+8
        dd      aJump,OnJump,JumpSyntax,JumpHelp
        db      2+8
        dd      aDoLoad,DoLoad,DoLoadSyntax,DoLoadHelp
        db      1+4
        ;
        dd      0

aHelp   db      5,'help',0
aHlp    db      2,'h',0
HelpHelp db     'Help on specified function',10
HelpSyntax db   'Usage: h or help [group | command]',10,0

HelpMsg db     'List of known command groups:',10
        db      '"help control"     - display list of control commands',10
        db      '"help data"        - display list of commands concerning data',10
        db      '"help breakpoints" - display list of commands concerning breakpoints',10,0

aAsm    db      2,'a',0
AsmHelp db      'Assemble instruction at current position',10
AsmSyntax db    'Usage: a instruction (',8,8Ah,'mov al,[ebx+12h]',8,8Fh,')',10,0

aJump   db      2,'j',0
JumpHelp db     'Sets current window or current register new address',10
JumpSyntax db   'Usage: Jump <expression>',10,0

aDetach db      7,'detach',0
DetachHelp db   'Detach from debugged program',10
DetachSyntax db 'Usage: detach',10,0

aSuspend db     5,'stop',0
SuspendHelp db  'Suspend execution of debugged program',10
SuspendSyntax db 'Usage: stop',10,0

aQuit   db      5,'quit',0
QuitHelp db     'Quit from debugger',10
QuitSyntax db   'Usage: quit',10,0

aExit   db      5,'exit',0
ExitHelp db     'Exit from debugger',10
ExitSyntax db   'Usage: exit',10,0

aLoad   db      5,'load',0
LoadHelp db     'Load program for debugging',10
LoadSyntax db   'Usage: load <program-name> [parameters]',10,0

aDoLoad  db     7,'doload',0
DoLoadHelp db   'Load test program for debugging',10
DoLoadSyntax db 'Usage: doload',10,0

aReload db      7,'reload',0
ReloadHelp db   'Reload debugging program (restart debug session)',10
ReloadSyntax db 'Usage: reload',10,0

aTerminate db   10,'terminate',0
TerminateHelp db 'Terminate debugged program',10
TerminateSyntax db 'Usage: terminate',10,0

aResume db      2,'g',0
ResumeHelp db   'Go (resume execution of debugged program)',10
ResumeSyntax db 'Usage: g',10
        db      '   or: g <expression> - wait until specified address is reached',10,0

aStep   db      2,'s',0
StepHelp db     'Make step in debugged program',10
StepSyntax db   'Usage: s [<number>]',10,0

aProceed db     2,'p',0
ProceedHelp db  'Make wide step in debugged program (step over CALL, REPxx, LOOP)',10
ProceedSyntax db 'Usage: p [<number>]',10,0

aDump   db      2,'d',0
DumpHelp db     'Dump data of debugged program',10
DumpSyntax db   'Usage: d <expression> - dump data at specified address',10
        db      '   or: d              - continue current dump',10,0

aCalc   db      2,'?',0
CalcHelp db     'Calculate value of expression',10
CalcSyntax db   'Usage: ? <expression>',10,0

aReg    db      2,'r',0
RHelp   db      'Set register value',10
RSyntax db      'Usage: r <register> <expression>',10
        db      '   or: r <register>=<expression> - set value of <register> to <expression>',10,0

aBp     db      3,'bp',0
BpHelp  db      'set BreakPoint on execution',10
BpSyntax db     'Usage: bp <expression>',10,0

aBpm    db      4,'bpm',0
aBpmb   db      5,'bpmb',0
aBpmw   db      5,'bpmw',0
aBpmd   db      5,'bpmd',0
BpmHelp db      'set BreakPoint on Memory access',10
        db      'Maximum 4 breakpoints of this type are allowed',10
        db      'Note that for this breaks debugger is activated after access',10
BpmSyntax db    'Usage: bpmb [w] <expression>',10
        db      '       bpmw [w] <expression>',10
        db      '       bpmd [w] <expression>',10
        db      '       bpm is synonym for bpmd',10
        db      '"w" means break only on writes (default is on read/write)',10,0

aBl     db      3,'bl',0
BlHelp  db      'Breakpoint List',10
BlSyntax db     'Usage: bl          - list all breakpoints',10
        db      '       bl <number> - display info on particular breakpoint',10,0

aBc     db      3,'bc',0
BcHelp  db      'Breakpoint Clear',10
BcSyntax db     'Usage: bc <number-list>',10
        db      'Examples: bc 2',10
        db      '          bc 1 3 4 A',10,0

aBd     db      3,'bd',0
BdHelp  db      'Breakpoint Disable',10
BdSyntax db     'Usage: bd <number-list>',10
        db      'Examples: bd 2',10
        db      '          bd 1 3 4 A',10,0

aBe     db      3,'be',0
BeHelp  db      'Breakpoint Enable',10
BeSyntax db     'Usage: be <number-list>',10
        db      'Examples: be 2',10
        db      '          be 1 3 4 A',10,0

aUnpack db      7,'unpack',0
UnpackHelp db   'Try to bypass unpacker code',10
UnpackSyntax db 'Usage: unpack',10,0

aLoadSymbols db 13,'load-symbols',0
LoadSymbolsHelp db 'Load symbolic information for executable',10
LoadSymbolsSyntax db 'Usage: load-symbols <symbols-file-name>',10,0

;               Control commands group
aControl db     8,'control',0
HelpControlMsg db     'List of control commands:',10
        db      'h = help             - help',10
        db      'exit/quit            - exit from debugger',10
        db      'load <name> [params] - load program for debugging',10
        db      'reload               - reload debugging program',10
        db      'load-symbols <name>  - load information on symbols for program',10
        db      'terminate            - terminate loaded program',10
        db      'detach               - detach from debugging program',10
        db      'stop                 - suspend execution of debugging program',10
        db      'a <instruction>      - assemble instruction at current position',10
        db      'g [<expression>]     - go on (resume execution of debugging program)',10
        db      's [<num>]            - program step, also <F7>',10
        db      'p [<num>]            - program wide step, also <F8>',10
        db      'unpack               - try to bypass unpacker code (heuristic)',10,0

;               Data commands group
aData   db      5,'data',0
HelpDataMsg db        'List of data commands:',10
        db      '? <expression>       - calculate value of expression',10
        db      'd [<expression>]     - dump data at given address',10
        db      'j <expression>       - jump, current window or current register new address',10
       ;db      'u [<expression>]     - unassemble instructions at given address',10
        db      'r <register> <expression> or',10
        db      'r <register>=<expression> - set register value',10,0

;               Breakpoints commands group
aBreakpoints db 12,'breakpoints',0
HelpBreaksMsg db      'List of breakpoints commands:',10
        db      'bp <expression>      - set breakpoint on execution',10
        db      'bpm[b|w|d] <type> <expression> - set breakpoint on memory access',10
        db      'bl [<number>]        - breakpoint(s) info',10
        db      'bc <number>...       - clear breakpoint',10
        db      'bd <number>...       - disable breakpoint',10
        db      'be <number>...       - enable breakpoint',10,0
;