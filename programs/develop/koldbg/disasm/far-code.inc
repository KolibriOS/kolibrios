ClrMagicBytes:
        xor     al,al

SetMagicBytes:

        test    al,4
        setnz   dl
        shl     dl,3
        mov     [I.R],dl

        mov     bl,-1
        cmovnz  ebx,[NotP4X]
        mov     [I.NotR],bl

        test    al,2
        setnz   dl
        shl     dl,3
        mov     [I.X],dl

        mov     bl,-1
        cmovnz  ebx,[NotP4X]
        mov     [I.NotX],bl

        test    al,1
        setnz   dl
        shl     dl,3
        mov     [I.B],dl

        mov     bl,-1
        cmovnz  ebx,[NotP4X]
        mov     [I.NotB],bl

        test    al,8
        setnz   [I.W]

        mov     bl,-1
        cmovnz  ebx,[NotP4X]
        mov     [I.NotW],bl

        test    al,40h
        setnz   [I.P]

        mov     bl,-1
        cmovnz  ebx,[NotP4X]
        mov     [I.NotP],bl

        ret

FetchModRM:
        FetchB
        mov     [I.ModRM],al

        mov     cl,al
        mov     bl,al
        mov     dl,al
        shr     cl,6    ;cl = mod
        and     bl,7    ;bl = r/m
        shr     dl,3
        and     dl,7    ;dl = reg

        mov     [I.Mod],cl
        mov     [I.Reg],dl
        mov     [I.RM],bl

        call    ModRM2EA

        mov     al,[I.ModRM]
        ret

TWorFW: cmp     [I.Syntax],0
        jnz     .Exit
        mov     [I.RandSize],VTWORD
        cmp     [I.Arch],CPUX64
        jz      .Exit
        mov     [I.RandSize],VFWORD
.Exit:
        ret

SetArg2W:
        cmp     [I.Fasm],0
        jz      .Exit
        mov     [I.Arg2.Type],GPR16
.Exit:  ret

SetArg12X:
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg2.Type],XMMRG
        ret

RM32R32C:
        call    RM32R32M

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],CTRRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg1.Type],GPR32
        ;lock mov reg,cr0 = mov reg,cr8
        ;lock mov cr0,reg = mov cr8,reg
        cmp     [I.Reg],0
        jne     .L1
        BitTR   I.Prefixes,PF0
        jnc     .L1
        mov     [I.Reg],8
.L1:    ret
;-----------------------------------------------------------------------------
CheckPrevSSE4:
        mov     ecx,(EndSSE4-TabSSE4)/3
        mov     edx,TabSSE4
.L1:    cmp     al,[rdx]
        je      .L2
        add     edx,3
        loop    .L1
        stc
        ret
.L2:    mov     edx,[rdx+1]
        mov     [I.Name],dx
        call    rbx
        mov     [I.RandSize],VQWORD+80h
        clc
        ret
;-----------------------------------------------------------------------------
CheckNextSSE4:
        mov     ecx,(EndNxtSSE4-NxtSSE4)/4
        mov     edx,NxtSSE4
.L1:    cmp     al,[rdx]
        je      .L2
        add     edx,4
        loop    .L1
        stc
        ret
.L2:    mov     ecx,[rdx+1]
        mov     [I.RandSize],cl
        shr     ecx,8
        mov     [I.Name],cx

        cmp     al,2Ah
        jne     .2A
        cmp     byte [rsi],0xC0
        jae     ErrorDec
.2A:
        cmp     al,15h
        ja      .L3
        mov     [I.Arg3.Type],XMMRG
.L3:    call    R32RM32X
        clc
        ret
;-----------------------------------------------------------------------------
R32RM32C:
        call    R32RM32M

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg1.Type],CTRRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg2.Type],GPR32
        ;lock mov reg,cr0 = mov reg,cr8
        ;lock mov cr0,reg = mov cr8,reg
        cmp     [I.Reg],0
        jne     .L1
        BitTR   I.Prefixes,PF0
        jnc     .L1
        mov     [I.Reg],8
.L1:    ret

RM32R32D:
        call    RM32R32M

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],DBGRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg1.Type],GPR32
.L1:    ret

R32RM32D:
        call    R32RM32M

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg1.Type],DBGRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg2.Type],GPR32
.L1:    ret

RM32R32T:
        call    RM32R32M

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],TRXRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg1.Type],GPR32
.L1:    ret

R32RM32T:
        call    R32RM32M

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg1.Type],TRXRG

        cmp     [I.Arch],CPUX64
        je      .L1
        mov     [I.Arg2.Type],GPR32
.L1:    ret
;-----------------------------------------------------------------------------
BNDX4:  mov     [I.Arg2.Type],GPR64
        mov     [I.RandSize],VQWORD+80h
        cmp     [I.Arch],CPUX64
        jz      .Next
        mov     [I.RandSize],VDWORD+80h
        mov     [I.Arg2.Type],GPR32
.Next: ;jmp     BNDX1
;-----------------------------------------------------------------------------
BNDX1:  mov     [I.Arg1.Type],BNDRG
        ret
;-----------------------------------------------------------------------------
BNDX3:  mov     [I.Arg1.Type],BNDRG
BNDX2:  mov     [I.Arg2.Type],BNDRG
        ret
;-----------------------------------------------------------------------------
Rtn0F3A14:
        mov     [I.RandSize],VBYTE+80h
        mov     dx,TPEXTRB
.L0:    BitTR   I.Prefixes,P66
        jnc     ErrorDec
        mov     [I.Name],dx
        call    RM32R32X
        mov     [I.Arg1.Type],GPR32
        cmp     [I.W],0
        jz      .L1
        mov     [I.Arg1.Type],GPR64
.L1:    call    ClearW
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
Rtn0F3A15:
        mov     [I.RandSize],VWORD+80h
        mov     dx,TPEXTRW
        jmp     Rtn0F3A14.L0
Rtn0F3A16:
        mov     [I.RandSize],VDWORD+80h
        mov     dx,TPEXTRD
        test    [I.W],0
        jz      .L1
        mov     [I.RandSize],VQWORD+80h
        mov     dx,TPEXTRQ
.L1:    jmp     Rtn0F3A14.L0
Rtn0F3A17:
        mov     [I.RandSize],VDWORD+80h
        mov     dx,TEXTRACTPS
        jmp     Rtn0F3A14.L0
Rtn0F3A20:
        mov     [I.RandSize],VBYTE+80h
        BitTR   I.Prefixes,P66
        jnc     ErrorDec
        mov     [I.Name],TPINSRB
        call    R32RM32X
        FetchB
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
Rtn0F3A21:
        mov     [I.RandSize],VDWORD+80h
        BitTR   I.Prefixes,P66
        jnc     ErrorDec
        mov     [I.Name],TINSERTPS
        call    R32RM32X
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
Rtn0F3A22:
        call    R32RM32X
        mov     [I.RandSize],VDWORD+80h
        BitTR   I.Prefixes,P66
        jnc     ErrorDec
        mov     dx,TPINSRD
        mov     [I.Arg2.Type],GPR32
        cmp     [I.W],0
        jz      .L1
        mov     dx,TPINSRQ
        mov     [I.Arg2.Type],GPR64
.L1:    mov     [I.Name],dx
        call    ClearW
        FetchB
.L2:    mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
Rtn0F3A44:
        mov     [I.RandSize],VXWORD+80h
        mov     [I.Name],TPCLMULQDQ
        BitTR   I.Prefixes,P66
        jnc     ErrorDec
        call    R32RM32X
        xor     eax,eax
        FetchB
        cmp     al,18
        ja      Rtn0F3A22.L2
        mov     edx,[rax*2+TabCLMUL]
        or      dx,dx
        jz      Rtn0F3A22.L2
        mov     [I.Name],dx
        ret

SetArg3Imm:
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret

SetArg3CL:
        mov     [I.Arg3.Type],GPR08
        mov     [I.Arg3.Reg],RRCX
        ret

RtnXOP: mov     ebx,R32RM32X
        test    [I.FlagsXOP],XOPO
        jnz     .Z1
        mov     ebx,RM32R32X
.Z1:    call    rbx
        cmp     [I.MMMMM],8
        jnz     .Z2
        test    [I.FlagsXOP],XOPI
        jz      .Z2
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
.Z2:    ret
;-----------------------------------------------------------------------------
SetVT3: push    CheckVT3
SetVT:  mov     [I.Base],-1
        mov     [I.Indx],-1
        mov     [I.Scale],-1
        mov     [I.DispSize],0

        cmp     [I.Arch],CPUX64
        jz      .L1
        mov     [I.Address],AD32
.L1:
        mov     rsi,[I.SaveRSI]
        push    rax
        sub     al,XMMRG-ADXX
        mov     [I.VT],al
        call    FetchModRM
        pop     rax

        cmp     [I.Arch],CPUX64
        jz      .L2
        BitT    I.PrefixesCpy,P67
        jnc     .L2
        BitTS   I.Prefixes,P67
.L2:
        cmp     [I.Relative],0
        jnz     ErrorDec
        ret
;-----------------------------------------------------------------------------
CheckVT3:
        cmp     [I.Mod],3
        jz      ErrorDec
        ret
;-----------------------------------------------------------------------------
RtnPINSR?:
        test    [I.VEX],VEXW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jnz     ErrorDec
        mov     [I.RandSize],VQWORD+80h
        mov     [I.LastByte],'q'
        mov     [I.Arg3.Type],GPR64
.L1:    ret
;-----------------------------------------------------------------------------
MoveKandZ:
        test    byte [I.FlagsEVEX],EVEXS
        jnz     .L1
        mov     al,[I.AAAA]
        or      al,al
        jz      .L1
        mov     al,'{'
        stosb
        cmp     [I.Syntax],0
        jz      .L0
        mov     al,'%'
        stosb
.L0:
        mov     al,'k'
        stosb
        mov     al,[I.AAAA]
        add     al,'0'
        stosb
        mov     al,'}'
        stosb
.L1:
        mov     al,[I.Z]
        or      al,al
        jz      .L2
        mov     al,'{'
        stosb
        mov     al,'z'
        stosb
        mov     al,'}'
        stosb
.L2:
        ret
;-----------------------------------------------------------------------------
GetP0P1P2:
        ;P0
        lodsb
        cmp     rsi,r9
        ja      .Error
        test    al,1100b
        jnz     .Error
        mov     ah,al
        and     al,11b
        mov     [I.MMMMM],al
        mov     al,ah

        test    al,10000000b
        setz    [I.R]
        test    al,00010000b
        setz    dl
        shl     dl,1
        or      [I.R],dl
        shl     [I.R],3

        test    al,01000000b
        setz    [I.X]
        shl     [I.X],3

        test    al,00100000b
        setz    [I.B]
        shl     [I.B],3

        mov     al,[I.X]
        shl     al,1
        mov     [I.XX],al

        mov     [I.P],1

        ;P1
        lodsb
        cmp     rsi,r9
        ja      .Error
        test    al,100b
        jz      .Error
        mov     ah,al
        and     al,11b
        mov     [I.PP],al
        mov     al,ah
        shr     al,3
        and     al,1111b
        xor     al,1111b
        mov     [I.VVVV],al
        mov     al,ah
        rol     al,1
        and     al,1b
        mov     [I.W],al
        ;P2
        lodsb
        cmp     rsi,r9
        ja      .Error
        mov     ah,al
        and     al,111b
        mov     [I.AAAA],al
        mov     al,ah
        xor     dl,dl
        shl     al,1
        rcl     dl,1
        mov     [I.Z],dl
        mov     al,ah
        and     al,1000b
        xor     al,1000b
        jz      .L1
        add     [I.V],16
.L1:
        shl     al,1
        or      [I.VVVV],al
        mov     al,ah
        and     al,1100000b
        cmp     al,1100000b
        shr     al,5
        mov     [I.LL],al
        test    ah,10000b
        setnz   [I.BB]
        ;
        movzx   eax,[I.PP]
        mov     edx,[rax*4+T6200Tab]
        mov     ebx,[rax*4+T6238Tab]
        mov     ecx,[rax*4+T623ATab]
        ;
        mov     al,[I.MMMMM]
        or      al,al
        jz      .Error
        cmp     al,00011b
        jne     .L2
        mov     edx,ecx
.L2:
        cmp     al,00010b
        jne     .L3
        mov     edx,ebx
.L3:
        lodsb
        cmp     rsi,r9
        ja      .Error
        mov     ebx,edx
        ;
        mov     [I.SaveRSI],rsi
        ret
        ;
.Error: pop     rax
        jmp     BNDEVEX62.Error
;-----------------------------------------------------------------------------
DetectMULT:
        cmp     [I.Is62],0
        jz      .L2
        mov     al,[I.RandSize]
        mov     ah,1
        cmp     al,xW
        jz      .L1
        mov     ah,2
        cmp     al,xD
        jz      .L1
        mov     ah,3
        cmp     al,xQ
        jz      .L1
        mov     ah,4
        cmp     al,xO
        jz      .L1
        mov     ah,5
        cmp     al,xY
        jz      .L1
        mov     ah,6
        cmp     al,xZ
        jz      .L1
        xor     ah,ah
.L1:
        mov     [I.MULT],ah
.L2:
        ret
;-----------------------------------------------------------------------------
MoveXtoX:
        cmp     [I.Mod],3
        jz      .V1
        cmp     [I.BB],0
        jz      .V1
        movzx   eax,[I.LL]
        test    byte [I.FlagsEVEX+2],EVEXAA shr 16
        jnz     .V0
        cmp     [I.RandSize],xQ
        jz      .V0
        inc     eax
.V0:    mov     esi,dword [rax*2+T1TO]
        call    MoveStringData
.V1:    ret
;-----------------------------------------------------------------------------
MoveSAE1:
        cmp     [I.Mod],3
        jnz     .E1
        test    byte [I.FlagsEVEX],EVEXO
        jz      .L1
        cmp     [I.W],0
        jz      .E1
.L1:
        test    byte [I.FlagsEVEX],EVEXR
        jz      .E2
        cmp     [I.BB],0
        jz      .E1
        mov     al,','
        stosb
        movzx   eax,[I.LL]
        mov     esi,dword [rax*2+TSAE]
        call    MoveStringData
.E1:    ret
.E2:    cmp     [I.BB],0
        jz      .E3
        cmp     [I.LL],00b
        jnz     .E3
if %B=64
        mov     rax,',{sae}'
        stosq
        sub     rdi,2
else
        mov     ax,',{'
        stosw
        mov     eax,'sae}'
        stosd
end if
.E3:    ret
;-----------------------------------------------------------------------------
MoveSAE2:
        cmp     [I.Mod],3
        jnz     .E1
        test    byte [I.FlagsEVEX],EVEXO
        jz      .L1
        cmp     [I.W],0
        jz      .E1
.L1:
        test    byte [I.FlagsEVEX],EVEXR
        jz      .E2
        cmp     [I.BB],0
        jz      .E1
        movzx   eax,[I.LL]
        mov     esi,dword [rax*2+TSAE]
        call    MoveStringData
        mov     al,','
        stosb
.E1:    ret
.E2:    cmp     [I.BB],0
        jz      .E3
        cmp     [I.LL],00b
        jnz     .E3
if %B=64
        mov     rax,'{sae},'
        stosq
        sub     rdi,2
else
        mov     eax,'{sae'
        stosd
        mov     ax,'},'
        stosw
end if
.E3:    ret
;-----------------------------------------------------------------------------
GetVectorAL:
        test    byte [I.FlagsEVEX],EVEXX
        jnz     .L1
        mov     al,ZMMRG
        cmp     [I.BB],0
        jnz     .L0
        test    [I.LL],10b
        jnz     .L0
        mov     al,YMMRG
        test    [I.LL],01b
        jnz     .L0
.L1:    mov     al,XMMRG
.L0:    ret
;-----------------------------------------------------------------------------
GetVectorAndMUL:
        test    byte [I.FlagsEVEX],EVEXX
        jnz     .L4
        ;
        mov     ah,[I.LL]

        mov     al,ZMMRG
        ;
        cmp     [I.Mod],3
        jnz     .LX
        cmp     [I.BB],0
        jnz     .L0
.LX:    ;
        test    ah,10b
        jnz     .L0
        mov     al,YMMRG
        test    ah,01b
        jnz     .L0
        mov     al,XMMRG
        ;
.L0:    mov     ah,al
        ;
        cmp     [I.BB],0
        jnz     .L3
        cmp     al,YMMRG
        jz      .L2
        ja      .L1
        ret
        ;
.L1:    call    .L2
.L2:    inc     [I.RandSize]
.L3:    ret
.L4:    mov     al,XMMRG
        mov     ah,al
        ret
;-----------------------------------------------------------------------------
GetRDRQAH:
        mov     ah,GPR32
        cmp     [I.W],1
        jnz     .L1
        mov     ah,GPR64
.L1:    ret
;-----------------------------------------------------------------------------
GetRDRQAL:
        mov     al,GPR32
        cmp     [I.W],1
        jnz     .L1
        mov     al,GPR64
.L1:    ret
