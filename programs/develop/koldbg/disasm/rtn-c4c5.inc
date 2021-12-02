r8d     equ     edi
r8      equ     rdi
;-----------------------------------------------------------------------------
R32RM32C5:
        mov     eax,R32RM32
        jmp     R16RM16C5.1
R16RM16C5:
        mov     eax,R16RM16
.1:     cmp     byte [rsi],0xC0
        jae     RtnAVX2C5
        or      byte [I.Flags],_O
        and     byte [I.Prefixes],not (bit P66)
        jmp     rax

RtnAVX2C5:
        ;F0/66/F2/F3/REX then #UD
        test    byte [I.Prefixes],bit PF0+bit P66+bit PF2+bit PF3+bit P4X
        jnz     ErrorDec

        FetchB
        mov     [I.VEX],al
        and     [I.VEX],7Fh     ;clear VEX.R

        mov     ah,al
        and     al,10000000b
        xor     al,10000000b
        shr     al,5            ;REX.R
        jz      .L1

        cmp     [IgnoreInvalid],True
        je      .L1
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
.L1:
        mov     [I.PreREX],al
        call    SetMagicBytes

        mov     [I.P],1

        mov     al,ah
        and     al,01111000b    ;vvvv
        xor     al,01111000b
        shr     al,3

        cmp     [IgnoreInvalid],True
        je      .L2
        cmp     al,7
        jbe     .L2
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
.L2:
        mov     [I.VVVV],al
        mov     al,ah
        and     eax,11b         ;pp
       ;mov     [PP],al
        mov     edx,[rax*4+C4C5Tab]
        mov     edi,[rax*3+CMPSuffixesX]
        jmp     C4C5XX
;-----------------------------------------------------------------------------
R32RM32C4:
        mov     eax,R32RM32
        jmp     R16RM16C4.1
R16RM16C4:
        mov     eax,R16RM16
.1:     cmp     byte [rsi],0xC0
        jae     RtnAVX2C4
        or      byte [I.Flags],_O
        and     byte [I.Prefixes],not (bit P66)
        jmp     rax
RtnAVX2C4:
        mov     [I.RandSize],0
        ;F0/66/F2/F3/REX then #UD
        test    byte [I.Prefixes],bit PF0+bit P66+bit PF2+bit PF3+bit P4X
        jnz     ErrorDec

        FetchB
        mov     ah,al
       ;and     al,11100000b
        xor     al,11100000b
        shr     al,5            ;REX.RXB
        jz      .L1

        cmp     [IgnoreInvalid],True
        je      .L1
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
.L1:
        mov     [I.PreREX],al
        call    SetMagicBytes

        mov     [I.P],1

        mov     al,ah
        and     al,11111b
        mov     [I.MMMMM],al    ;mmmmm

        FetchB
        mov     ah,al
        mov     [I.VEX],al
        and     al,01111000b    ;vvvv
        xor     al,01111000b
        shr     al,3

        cmp     [IgnoreInvalid],True
        je      .L2
        cmp     al,7
        jbe     .L2
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
.L2:
        mov     [I.VVVV],al
        mov     al,ah
        and     eax,11b         ;pp
       ;mov     [PP],al

        mov     edx,[rax*4+C4C5Tab]
        mov     ebx,[rax*4+C438Tab]
        mov     ecx,[rax*4+C43ATab]
        mov     edi,[rax*3+CMPSuffixesX]

        mov     al,[I.MMMMM]
        or      al,al
        jz      ErrorDec
        cmp     al,00011b
        ja      ErrorDec
       ;cmp     al,00011b
        jne     .L3
        mov     edx,ecx
.L3:
        cmp     al,00010b
        jne     .L4
        mov     edx,ebx
.L4:

C4C5XX:
        mov     word [I.Suffix3rd],di
        FetchB
        mov     [I.AfterVEX],al

        mov     [I.SaveRSI],rsi
.L1:
        cmp     dword [rdx],-1
        jz      ErrorDec
        cmp     al,[rdx+CXX.Imm]
        jz      .L2
        add     edx,CXX.Size
        jmp     .L1
.L2:
        mov     eax,dword [rdx+CXX.Flags]
        mov     word [I.FlagsVEX],ax

        mov     cl,VEXL

        and     al,cl
        mov     ah,[I.VEX]
        and     ah,al
        jnz     ErrorDec

        test    [I.FlagsVEX],VEXN
        jz      .L3
        mov     ah,[I.VEX]
        and     ah,cl
        jz      ErrorDec
.L3:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        test    [I.FlagsVEX+1],VEXG shr 8
        jz      .L4
        xor     cl,cl
        cmp     [I.Arch],CPUX64
        jne     .L4
        mov     cl,VEXW
.L4:
        mov     al,[rdx+CXX.MOS]
        test    [I.VEX],cl
        jz      .L5
        shr     al,4
.L5:    and     al,0Fh
        mov     [I.RandSize],al
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     al,byte [rdx+CXX.Flags]
        and     al,VEXW
        mov     ah,[I.VEX]

        cmp     [I.Arch],CPUX64
        je      .L6
        test    [I.FlagsVEX+1],VEXC shr 8
        jnz     .L6
        test    ah,VEXW
        jnz     ErrorDec
.L6:
        and     ah,al
        jnz     ErrorDec

        mov     al,byte [rdx+CXX.Flags]
        and     al,VEXV
        jz      .L7
        cmp     [I.VVVV],0      ;!1111b=0000b
        jnz     ErrorDec
.L7:
        movzx   r8d,[rdx+CXX.Rtn]
        add     r8d,RtnXX

        mov     edx,dword [rdx+CXX.Name]
        test    [I.FlagsVEX+1],VEXH shr 8
        jnz     .L8
        mov     [I.Prefix],'v'
.L8:    mov     [I.Name],dx

        test    [I.FlagsVEX+1],VEXF shr 8
        jnz     .L9
        call    FetchModRM
.L9:
        test    [I.FlagsVEX+1],VEXR shr 8
        jz      .LA
        cmp     [I.Mod],3
        jnz     ErrorDec
.LA:
        cmp     [I.Mod],3
        jnz     .LB
        test    [I.FlagsVEX],VEXM
        jnz     ErrorDec
.LB:    jmp     r8

RtnC4C5:
        lea     r8,[I.Arg1]

        mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     al,YMMRG
.L1:    mov     ah,al

        test    [I.FlagsVEX+1],VEX2 shr 8
        jz      .L2
        mov     ah,XMMRG
.L2:
        test    [I.FlagsVEX+1],VEX1 shr 8
        jz      .L3
        mov     al,XMMRG
.L3:
        test    [I.FlagsVEX+1],VEXG shr 8
        jz      .L4
        mov     ax,GPR32 by GPR32
        cmp     [I.Arch],CPUX64
        jne     .L4
        test    [I.VEX],VEXW
        jz      .L4
        mov     ax,GPR64 by GPR64
.L4:
        push    rax
        test    [I.FlagsVEX+1],VEXS shr 8
        jnz     .NDD
        mov     [r8+TArg.Type],al
        mov     [r8+TArg.Type2nd],InReg
        add     r8d,sizeof.TArg
.NDD:
        mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L5
        mov     al,YMMRG
.L5:
        test    [I.FlagsVEX+1],VEXG shr 8
        jz      .L6
        mov     al,GPR32
        cmp     [I.Arch],CPUX64
        jne     .L6
        test    [I.VEX],VEXW
        jz      .L6
        mov     al,GPR64
.L6:
        test    [I.FlagsVEX],VEXV
        jnz     .L7
        mov     [r8+TArg.Type],al
        mov     [r8+TArg.Type2nd],InVVVV
        add     r8d,sizeof.TArg
.L7:    pop     rax

       ;cmp     [I.Mod],3
       ;setnz   [r8+TArg.Mem]
        mov     dl,ah
        mov     [r8+TArg.Type],dl
        mov     [r8+TArg.Type2nd],InRM
        add     r8d,sizeof.TArg

        test    [I.FlagsVEX],VEXI
        jnz     .Imm

        test    [I.FlagsVEX],VEX8
        jz      .L8
        movzx   eax,byte [rsi]
        cmp     al,1Fh
        ja      .Imm
        FetchB

        cmp     [I.AfterVEX],0xC2
        jne     .L8
        mov     edx,[rax*2+CMPxxNames]
        mov     [I.Name],dx
        mov     eax,dword [I.Suffix3rd]
        mov     dword [I.Suffix2nd],eax
        ret

.Imm:   FetchB
        mov     [r8+TArg.Type],80h+1
        mov     [r8+TArg.ImmB],al
        ret
.L8:
        test    [I.FlagsVEX],VEX4
        jnz     .L9
        ret
.L9:
        FetchB
        shr     al,4

        cmp     [IgnoreInvalid],True
        je      .LA
        cmp     [I.Arch],CPUX64
        je      .LA
        and     al,111b
.LA:    mov     [r8+TArg.Reg],al
        mov     [r8+TArg.Type],XMMRG
        test    [I.VEX],VEXL
        jz      .LB
        mov     [r8+TArg.Type],YMMRG
.LB:    ret
;-----------------------------------------------------------------------------
RtnC401:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InReg

        test    [I.VEX],VEXL
        jz      .L1
        mov     [I.Arg1.Type],YMMRG
.L1:    ret
;-----------------------------------------------------------------------------
RtnC404:test    [I.VEX],VEXW
        jz      .L1
        mov     [I.LastByte],'q'
.L1:   ;jmp     RtnC402
;-----------------------------------------------------------------------------
RtnC402:mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     al,YMMRG
.L1:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type],al
        mov     [I.Arg3.Type2nd],InReg
        ret
;-----------------------------------------------------------------------------
RtnC403:test    [I.VEX],VEXW
        jz      .L1
        mov     [I.LastByte],'q'
.L1:    jmp     RtnC4C5
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
RtnC405:test    [I.VEX],VEXW
        jz      .L1
        mov     [I.LastByte],'q'
.L1:   ;jmp     RtnC4VT
;-----------------------------------------------------------------------------
RtnC4VT:mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     al,YMMRG
.L1:    call    SetVT

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg3.Type],al
        mov     [I.Arg3.Type2nd],InVVVV
        ret
;-----------------------------------------------------------------------------
RtnC406:call    RtnC4VT
        mov     ax,'ps'
        test    [I.VEX],VEXW
        jz      .L1
        mov     ah,'d'  ;'pd'
.L1:    mov     word [I.Suffix2nd],ax
        ret
;-----------------------------------------------------------------------------
RtnC407:call    RtnC4C5
        mov     ax,'ps'
        test    [I.VEX],VEXW
        jz      .L1
        mov     ah,'d'  ;'pd'
.L1:    mov     word [I.Suffix2nd],ax
        ret
;-----------------------------------------------------------------------------
RtnC408:test    [I.VEX],VEXW
        jz      .L1
        mov     [I.RandSize],xQ
.L1:    call    RtnC4C5
        mov     ax,'ss'
        test    [I.VEX],VEXW
        jz      .L2
        mov     ah,'d'  ;'sd'
.L2:    mov     word [I.Suffix2nd],ax
        ret
;-----------------------------------------------------------------------------
RtnC409:
        mov     al,GPR32
        cmp     [I.Arch],CPUX64
        jne     .L1
        test    [I.VEX],VEXW
        jz      .L1
        mov     al,GPR64
.L1:
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg3.Type],al
        mov     [I.Arg3.Type2nd],InVVVV
        ret
;-----------------------------------------------------------------------------
RtnC40A:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'w'
.L0:
        mov     al,KXXRG
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InRM
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
;-----------------------------------------------------------------------------
RtnC40B:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
.L0:    jmp     RtnC40A.L0
;-----------------------------------------------------------------------------
RtnC40C:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],YMMRG
        mov     [I.Arg2.Type2nd],InReg
.L1:
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
;-----------------------------------------------------------------------------
RtnC40D:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InRM

        mov     al,XMMRG
        test    [I.XOP],XOPL
        jz      .L1
        mov     al,YMMRG
.L1:
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InReg
        jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
RtnC40F:
        mov     al,GPR64
        cmp     [I.Arch],CPUX64
        jne     RtnC40E.L1
        test    [I.VEX],VEXW
        jnz     RtnC40E.L2
RtnC40E:
.L1:    mov     al,GPR32
.L2:
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg2.Type2nd],InReg
        jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
RtnC410:
        mov     [I.Arg1.Type],YMMRG
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],YMMRG
        mov     [I.Arg2.Type2nd],InVVVV
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg3.Mem]
        mov     [I.Arg3.Type],XMMRG
        mov     [I.Arg3.Type2nd],InRM
        FetchB
        mov     [I.Arg4.Type],80h+1
        mov     [I.Arg4.ImmB],al
        ret
;-----------------------------------------------------------------------------
RtnCLMUL:
        call    RtnC4C5
        movzx   eax,byte [rsi-1]
        cmp     al,18
        ja      .L1
        mov     edx,[rax*2+TabCLMUL]
        or      dx,dx
        jz      .L1
        mov     [I.Name],dx
        mov     [I.Arg4.Type],0 ;Delete
.L1:    ret
;-----------------------------------------------------------------------------
C40F3A48:
        mov     ebx,TPERMIL2PSX
        jmp     C40F3A49.L1
;-----------------------------------------------------------------------------
C40F3A49:
        mov     ebx,TPERMIL2PDX
.L1:
        mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L2
        mov     al,YMMRG
.L2:
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type],al
        mov     [I.Arg4.Type],al
        FetchB
        mov     dl,al
        shr     dl,4

        cmp     [IgnoreInvalid],True
        je      .L3
        cmp     [I.Arch],CPUX64
        je      .L3
        and     dl,111b
.L3:
        lea     r8,[I.Arg3]
        test    [I.VEX],VEXW
        jnz     .L7
        lea     r8,[I.Arg4]
.L7:    mov     [r8+TArg.Reg],dl

        and     eax,1111b

        cmp     al,0011b
        ja      .L4
        mov     edx,[rax*2+rbx]
        mov     [I.Name],dx
        jmp     .L5
.L4:
        mov     [I.Arg5.Type],80h+1
        mov     [I.Arg5.ImmB],al
.L5:
        lea     r8,[I.Arg4]
        test    [I.VEX],VEXW
        jnz     .L6
        lea     r8,[I.Arg3]
.L6:
        mov     [r8+TArg.Type2nd],InRM
       ;cmp     [I.Mod],3
       ;setnz   [r8+TArg.Mem]
        ret
;-----------------------------------------------------------------------------
RtnC411:
        call    RM32R32M
        mov     [I.Arg2.Type],XMMRG
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     [I.Arg1.Type],GPR64
        mov     [I.RandSize],VQWORD+80h
        mov     [I.LastByte],'q'
.L1:    jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
RtnC413:
        push    RtnPINSR?
RtnC412:
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg2.Type2nd],InVVVV
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg3.Mem]
        mov     [I.Arg3.Type],GPR32
        mov     [I.Arg3.Type2nd],InRM
        FetchB
        mov     [I.Arg4.Type],80h+1
        mov     [I.Arg4.ImmB],al
        ret
;-----------------------------------------------------------------------------
RtnC414:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg2.Type],YMMRG
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],YMMRG
        mov     [I.Arg1.Type2nd],InReg
        jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
C4FMA:  mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     al,YMMRG
.L1:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],al
        mov     [I.Arg4.Type],al
        FetchB
        shr     al,4

        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV

        lea     r8,[I.Arg3]
        test    [I.VEX],VEXW
        jnz     .L2
        lea     r8,[I.Arg4]
.L2:
        mov     [r8+TArg.Reg],al

        lea     r8,[I.Arg4]
        test    [I.VEX],VEXW
        jnz     .L3
        lea     r8,[I.Arg3]
.L3:
       ;cmp     [I.Mod],3
       ;setnz   [r8+TArg.Mem]
        mov     [r8+TArg.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC415:
        movzx   eax,[I.Reg]
        mov     edx,[rax*2+BMI1]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        mov     [I.Prefix],0

        mov     al,GPR32
        cmp     [I.Arch],CPUX64
        jne     .L1
        test    [I.VEX],VEXW
        jz      .L1
        mov     al,GPR64
.L1:
        mov     [I.Arg2.Type],al
        mov     [I.Arg1.Type],al

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type2nd],InVVVV
        ret
;-----------------------------------------------------------------------------
RtnC416:
        call    R32RM32M

        cmp     [I.Arch],CPUX64
        jne     .L1
        test    [I.VEX],VEXW
        jz      .L1
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],GPR64
.L1:    jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
RtnC4C508:
        test    [I.VEX],VEXW
        jz      .L1
        mov     [I.Name],TKUNPCKDQ
.L1:    jmp     RtnC4C502.L0
;-----------------------------------------------------------------------------
RtnC4C501:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
.L0:    jmp     RtnC4C502.L0
;-----------------------------------------------------------------------------
RtnC4C502:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'d'
.L0:    ;
        cmp     [I.Mod],3
        jnz     ErrorDec
        ;
        mov     al,KXXRG
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],al

        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg3.Mem]
        mov     [I.Arg3.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C503:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'d'
.L0:    jmp     RtnC4C505.L0
;-----------------------------------------------------------------------------
RtnC4C504:
        cmp     [I.Mod],3
        jnz     ErrorDec
RtnC4C505:
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
.L0:
        mov     al,KXXRG
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        mov     [I.Arg1.Type2nd],InReg
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C506:
        cmp     [I.Mod],3
        jz      ErrorDec
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'d'
.L0:
        mov     al,KXXRG
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        mov     [I.Arg2.Type2nd],InReg
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C507:
        cmp     [I.Mod],3
        jz      ErrorDec
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
.L0:
        jmp     RtnC4C506.L0
;-----------------------------------------------------------------------------
RtnC4C509:
        cmp     [I.Mod],3
        jnz     ErrorDec
        mov     al,GPR32
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
        mov     al,GPR64
.L0:
        mov     [I.Arg1.Type],KXXRG
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C50A:
        cmp     [I.Mod],3
        jnz     ErrorDec
        mov     al,GPR32
        test    [I.VEX],VEXW
        jz      .L0
        mov     [I.LastByte],'q'
        mov     al,GPR64
.L0:
        mov     [I.Arg2.Type],KXXRG
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg1.Type],al
        ret
;-----------------------------------------------------------------------------
RtnC4C50B:
        cmp     [I.Reg],2               ;/2
        jb      ErrorDec
        cmp     [I.Reg],3               ;/3
        ja      ErrorDec
        jne     .L1
        mov     [I.Name],TSTMXCSR
.L1:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg1.Type],GPR32
        ret
;-----------------------------------------------------------------------------
RtnC4C50C:
        mov     al,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     al,YMMRG
.L1:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type2nd],InReg
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al
        ret
;-----------------------------------------------------------------------------
C4C5NB2:
        mov     dx,TMOVLHPS
        jmp     C4C5NB1.L1
;-----------------------------------------------------------------------------
C4C5NB1:mov     dx,TMOVHLPS
.L1:    cmp     [I.Mod],3
        jne     .L2
        mov     [I.Name],dx
.L2:    jmp     RtnC4C5
;-----------------------------------------------------------------------------
RtnC4C50D:
        mov     ah,XMMRG
        test    [I.VEX],VEXL
        jz      .L1
        mov     ah,YMMRG
.L1:
        mov     al,GPR32
        cmp     [I.Arch],CPUX64
        jnz     .L2
        mov     al,GPR64
.L2:
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],ah
        ret
;-----------------------------------------------------------------------------
RtnC4C50E:
        test    [I.VEX],VEXL
        jnz     .L1
        mov     [I.Name],TZEROUPPER
.L1:    ret
;-----------------------------------------------------------------------------
RtnC4C5XRM:
        mov     al,XMMRG
        mov     ah,GPR32
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     [I.LastByte],'q'
        mov     ah,GPR64
.L1:
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],ah
        ret
;-----------------------------------------------------------------------------
RtnC4C5RMX:
        mov     al,XMMRG
        mov     ah,GPR32
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     [I.LastByte],'q'
        mov     ah,GPR64
.L1:
        mov     [I.Arg2.Type2nd],InReg
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type],al
        mov     [I.Arg1.Type],ah
        ret
;-----------------------------------------------------------------------------
RtnC4C50F:
        mov     al,GPR64
        cmp     [I.Arch],CPUX64
        jz      .L1
        mov     al,GPR32
.L1:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InRM
        jmp     RtnC40C.L1
;-----------------------------------------------------------------------------
C4C50F71:
        mov     edx,T0F71Names
        jmp     C4C5NDSX
;-----------------------------------------------------------------------------
C4C50F72:
        mov     edx,T0F72Names
        jmp     C4C5NDSX
;-----------------------------------------------------------------------------
C4C50F73:
        mov     edx,T0F73Names
C4C5NDSX:
        movzx   eax,[I.Reg]
        mov     edx,[rdx+rax*2]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        jmp     RtnC4C5
;-----------------------------------------------------------------------------
RtnC4C510:
        mov     al,YMMRG
        test    [I.VEX],VEXL
        jnz     .L1
        mov     al,XMMRG
.L1:
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],al
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type],XMMRG
        mov     [I.Arg3.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C511:
        mov     al,GPR32
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     al,GPR64
.L1:
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg2.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
RtnC4C512:
        mov     al,GPR32
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     [I.RandSize],VQWORD
        mov     al,GPR64
.L1:
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],XMMRG
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type],al
        mov     [I.Arg3.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
C4C5MXJ:cmp     [I.Mod],3
        jz      RtnC4C5
        cmp     [I.VVVV],0              ;!1111b=0000b
        jnz     ErrorDec
        jmp     RtnC4C50C
;-----------------------------------------------------------------------------
C4C5XMJ:cmp     [I.Mod],3
        jz      .L1
        or      [I.FlagsVEX],VEXV
        cmp     [I.VVVV],0              ;!1111b=0000b
        jnz     ErrorDec
.L1:    jmp     RtnC4C5
