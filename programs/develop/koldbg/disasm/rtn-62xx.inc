;-----------------------------------------------------------------------------
BNDEVEX62:
        cmp     byte [rsi],0xC0
        jae     EVEX62
        ;
        call    GetP0P1P2
        jmp     EVEX62.Loop
        ;
.Error: cmp     [I.Arch],CPUX64
        jz      ErrorDec
        ;
        xor     al,al
        ;
        mov     [I.R],al
        mov     [I.X],al
        mov     [I.B],al
        mov     [I.W],al
        mov     [I.V],al
        mov     [I.Z],al
        mov     [I.BB],al
        mov     [I.XX],al
        mov     [I.VVVV],al
        mov     [I.AAAA],al
        ;
        mov     rsi,[I.SaveRSI]
        ;
        BitTR   I.Prefixes,P66
        cmp     [I.Operand],0
        mov     [I.RandSize],VQWORD+80h ;32:32
        jnz     R32RM32
        mov     [I.RandSize],VDWORD+80h ;16:16
        jmp     R16RM16
;-----------------------------------------------------------------------------
EVEX62: cmp     [I.PreREX],0
        jnz     ErrorDec
        call    GetP0P1P2
.Loop:
        mov     [I.Is62],1

        cmp     dword [rbx],-1
        jz      ErrorDec
        cmp     al,[rbx+X62.Imm]
        je      .Found
        add     ebx,X62.Size
        jmp     .Loop
.Found:
        mov     al,[rbx+X62.MOS]
        cmp     [I.BB],0
        jnz     .L1
        shr     al,4
.L1:
        and     al,0Fh
        mov     [I.RandSize],al
        ;
        mov     eax,dword [rbx+X62.Flags]
        mov     word [I.FlagsEVEX],ax
        ;
        and     ah,EVEXLX shr 8
        cmp     ah,EVEXLX shr 8
        jnz     .L2
        cmp     [I.LL],00b
        jnz     ErrorDec
        jmp     .SkipLX
.L2:    ;
        test    ah,EVEXL0 shr 8
        jz      .L3
        cmp     [I.LL],00b
        jz      ErrorDec
.L3:    ;
        test    ah,EVEXLL shr 8
        jz      .L4
        cmp     [I.LL],10b
        jb      ErrorDec
.L4:
        ;
.SkipLX:
        ;
        mov     ah,al
        and     ah,EVEXW0
        cmp     ah,EVEXW0
        jnz     .L5
        cmp     [I.Arch],CPUX64
        jz      .SkipWNW
        mov     [I.W],0
        jmp     .SkipWNW
.L5:    ;
        test    al,EVEXYW
        jz      .L6
        cmp     [I.W],1
        jz      ErrorDec
.L6:    ;
        test    al,EVEXNW
        jz      .L7
        cmp     [I.W],0
        jz      ErrorDec
.L7:
;
.SkipWNW:
;
        test    al,EVEXV
        jz      .L8
        mov     al,[I.VVVV]
        and     al,1111b
        or      al,al
        jnz     ErrorDec
.L8:    ;
        mov     edx,dword [rbx+X62.Name]
        mov     al,byte [rbx+X62.Flags+1]
        and     al,EVEXE shr 8
        cmp     al,EVEXE shr 8
        jnz     .L9
        movzx   eax,[I.W]
        movzx   edx,dx
        add     edx,T62Names
        mov     edx,[rdx+rax*2]
.L9:
        mov     [I.Prefix],'v'
        mov     [I.Name],dx

        cmp     [I.W],1
        jnz     .LA
        mov     al,byte [rbx+X62.Flags+1]
        and     al,EVEXE shr 8
        cmp     al,EVEXE shr 8
        jz      .LA
        test    al,EVEXD1 shr 8
ifnz    mov     [I.LastByte],'d'
        test    al,EVEXQ1 shr 8
ifnz    mov     [I.LastByte],'q'
.LA:    ;
        movzx   r8d,word [rbx+X62.Rtn]
        add     r8d,RtnXX
        test    [rbx+X62.Flags],EVEX2
        jz      .LB
        movzx   eax,[I.W]
        movzx   r8d,word [r8+rax*2]
        add     r8d,RtnXX
.LB:    ;
        call    FetchModRM
        test    byte [I.FlagsEVEX+1],EVEXM shr 8
        jz      .LC
        call    GetVectorAndMUL
.LC:
        test    byte [I.FlagsEVEX+1],EVEXU shr 8
        jnz     .SkipBB
        test    byte [I.FlagsEVEX+1],EVEXB shr 8
        jz      .LD
        ;
        cmp     [I.BB],0
        jz      .LD
.SkipBB:
        cmp     [I.W],1
        jnz     .LD
       ;inc     [I.MULT]
        inc     [I.RandSize]
.LD:
        call    r8
        ;
        test    byte [I.FlagsEVEX+1],EVEXI shr 8
        jz      .LG
        lea     r8,[I.Arg1]
        mov     ecx,5
.LE:
        cmp     [r8+TArg.Type],0
        jz      .LF
        add     r8,sizeof.TArg
        loop    .LE
        ret
.LF:
        FetchB
        mov     [r8+TArg.Type],80h+1
        mov     [r8+TArg.ImmB],al
.LG:    ret
;-----------------------------------------------------------------------------
T6244:
        cmp     [I.ModRM],3
        jz      ErrorDec
        cmp     [I.BB],1
        jz      ErrorDec
        mov     [I.RandSize],VOWORD
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type2nd],InRM or 80h
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],ah
        ret
;-----------------------------------------------------------------------------
T6214:
        call    GetRDRQAH
       ;jmp     T6201
;-----------------------------------------------------------------------------
T6201:
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],ah
        ret
;-----------------------------------------------------------------------------
T6215:
        mov     [I.RandSize],xO
        mov     ah,XMMRG
        jmp     T6201
;-----------------------------------------------------------------------------
T6200:
        mov     [I.RandSize],xD

        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],ah
        mov     [I.Arg2.Type2nd],InRM

        cmp     [I.W],0
        jz      .L1

        inc     [I.RandSize]

        dec     al
        cmp     al,XMMRG
        jae     .L1
        mov     al,XMMRG
.L1:    jmp     SetVT3
;-----------------------------------------------------------------------------
T621B:
        mov     [I.RandSize],xQ
        ;
        cmp     [I.W],0
        jnz     .L1
        ;
        dec     [I.RandSize]
        ;
        dec     al
        cmp     al,XMMRG
        jae     .L1
        mov     al,XMMRG
        ;
.L1:
        ;
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type],ah
        mov     [I.Arg2.Type2nd],InRM
        ;
        mov     al,ah
        jmp     SetVT3
;-----------------------------------------------------------------------------
T6202:
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type2nd],InRM
        mov     [I.Arg1.Type],KXXRG
        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],ah
        ret
;-----------------------------------------------------------------------------
T6219:
        cmp     [I.Mod],3
        jz      ErrorDec
       ;jmp     T6203
;-----------------------------------------------------------------------------
T6203:
        mov     ah,XMMRG
        call    GetVectorAL
T6204:
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],ah
        ret
;-----------------------------------------------------------------------------
T621A:
        cmp     [I.Mod],3
        jz      ErrorDec
        mov     al,ZMMRG
        mov     ah,XMMRG
        jmp     T6204
;-----------------------------------------------------------------------------
T6208:
        cmp     [I.BB],0
        jz      .L1
        dec     [I.RandSize]
.L1:
        jmp     T6204
;-----------------------------------------------------------------------------
T6209:
        or      byte [I.FlagsEVEX+2],EVEXAA shr 16
        ;
        cmp     [I.BB],0
        jnz     .L1
        dec     [I.RandSize]
.L1:
T6207:
        dec     ah
        cmp     ah,XMMRG
        jae     .L1
        mov     ah,XMMRG
.L1:    jmp     T6204
;-----------------------------------------------------------------------------
T6205:
        dec     al
        cmp     al,XMMRG
        jae     .L1
        mov     al,XMMRG
.L1:
        jmp     T6204
;-----------------------------------------------------------------------------
T6206:
        cmp     [I.W],0
        jnz     T6204
        ;
        dec     [I.RandSize]
        ;
        or      byte [I.FlagsEVEX+2],EVEXAA shr 16
        ;
        jmp     T6207
;-----------------------------------------------------------------------------
T620A:
        dec     ah
        cmp     ah,XMMRG
        jae     .L1
        mov     ah,XMMRG
.L1:
T620B:
        mov     [I.Arg2.Type2nd],InReg
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type],al
        mov     [I.Arg1.Type],ah
        ret
;-----------------------------------------------------------------------------
T6223:
        mov     al,XMMRG
        mov     ah,GPR32
        jmp     T620B
;-----------------------------------------------------------------------------
T6221:  mov     [I.RandSize],xO
       ;call    DetectMULT
       ;jmp     T6220
;-----------------------------------------------------------------------------
T6220:
        mov     ah,XMMRG
        jmp     T620B
;-----------------------------------------------------------------------------
T6222:
        mov     [I.RandSize],xY
       ;call    DetectMULT
        mov     ah,YMMRG
        jmp     T620B
;-----------------------------------------------------------------------------
T620C:
        cmp     [I.Mod],3
        jnz     T6201
        mov     [I.Name],TMOVHLPS
        jmp     T6201
;-----------------------------------------------------------------------------
T620D:
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     T6201
;-----------------------------------------------------------------------------
T620E:
        cmp     [I.Mod],3
        jnz     .L1
        mov     [I.Name],TMOVLHPS
.L1:    jmp     T6201
;-----------------------------------------------------------------------------
T620F:
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     T620B
;-----------------------------------------------------------------------------
T6210:
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     T620B
;-----------------------------------------------------------------------------
T621C:
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     T6204
;-----------------------------------------------------------------------------
T6211:
        call    GetRDRQAH
        jmp     T6204
;-----------------------------------------------------------------------------
T6212:
        call    GetRDRQAH
        jmp     T620B
;-----------------------------------------------------------------------------
T6213:
        call    GetRDRQAL
        jmp     T6204
;-----------------------------------------------------------------------------
T6271:
        mov     ebx,T0F71N
        movzx   edx,[I.Reg]
.L0:    mov     dx,[rdx*2+rbx]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        mov     [I.Arg1.Type2nd],InVVVV
        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],ah
        ret
;-----------------------------------------------------------------------------
T6272:
        mov     ebx,T0F72N
.L0:    movzx   edx,[I.Reg]
        cmp     [I.W],1
        jnz     .L1
        add     dl,8
.L1:    jmp     T6271.L0
;-----------------------------------------------------------------------------
T6273:
        mov     ebx,T0F73N
        jmp     T6272.L0
;-----------------------------------------------------------------------------
T6216:
        cmp     [I.VVVV],00000b
        jnz     T6201
        jmp     T6203
;-----------------------------------------------------------------------------
T6217:
        cmp     [I.VVVV],00000b
        jnz     T6201
        jmp     T620B
;-----------------------------------------------------------------------------
T6218:
        cmp     al,XMMRG
        jz      .L1
        inc     [I.RandSize]
.L1:    jmp     T6204
;-----------------------------------------------------------------------------
T62RD:  call    GetRDRQAH
        jmp     T62RB.L0
;-----------------------------------------------------------------------------
T62RW:  mov     ah,GPR16
        jmp     T62RB.L0
;-----------------------------------------------------------------------------
T62RB:  mov     ah,GPR08
.L0:    cmp     [I.Mod],3
        jnz     ErrorDec
        call    GetVectorAL
        mov     [I.XX],0
        jmp     T6204
;-----------------------------------------------------------------------------
T62V1XYZ:
        mov     [I.RandSize],xD
        ;
        cmp     [I.W],0
        jz      .L1
        ;
        inc     [I.RandSize]
        ;
        dec     al
        cmp     al,XMMRG
        jae     .L1
        mov     al,XMMRG
.L1:    ;
        call    SetVT3
        ;
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type2nd],InReg
        mov     [I.Arg1.Type],ah
        ;
        mov     dl,[I.RandSize]
        call    GetVectorAndMUL
        mov     [I.RandSize],dl
        mov     [I.Arg2.Type],al
        ret
;-----------------------------------------------------------------------------
T62V2XYZ:
        mov     [I.RandSize],xQ
                ;
        cmp     [I.W],0
        jnz     .L1
                ;
        dec     [I.RandSize]
                ;
        dec     al
        cmp     al,XMMRG
        jae     .L1
        mov     al,XMMRG
        ;
.L1:    push    rax
        mov     al,ah
        call    SetVT3
        pop     rax
        ;
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type2nd],InReg
        mov     [I.Arg1.Type],ah
        mov     [I.Arg2.Type],al
        ret
;-----------------------------------------------------------------------------
T62XYZVV1:
        movzx   edx,[I.Reg]
        cmp     [I.W],0
        jz      .L1
        add     dl,8
.L1:
        mov     edx,[rdx*2+T??C6N]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx

        mov     al,ZMMRG
        cmp     [I.W],0
        jz      .L2
        mov     al,YMMRG
.L2:
        mov     [I.Arg1.Type],ah
        mov     [I.Arg1.Type2nd],InRM
        jmp     SetVT3
;-----------------------------------------------------------------------------
T62XYZVV2:
        movzx   edx,[I.Reg]
        cmp     [I.W],0
        jz      .L1
        add     dl,8
.L1:
        mov     edx,[rdx*2+T??C7N]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        mov     al,ZMMRG

        mov     [I.Arg1.Type],ah
        mov     [I.Arg1.Type2nd],InRM
        jmp     SetVT3
;-----------------------------------------------------------------------------
T621D:
        cmp     [I.Mod],3
        jnz     ErrorDec
        mov     ah,KXXRG
        jmp     T6204
;-----------------------------------------------------------------------------
T621E:
        cmp     [I.Mod],3
        jnz     ErrorDec
T621F:
        mov     al,KXXRG
        jmp     T6204
;-----------------------------------------------------------------------------
T6224:
        mov     [I.RandSize],xO
        mov     ah,XMMRG
        jmp     T6225.L0
;-----------------------------------------------------------------------------
T6225:
        mov     [I.RandSize],xY
        mov     ah,YMMRG
.L0:    jmp     T6201
;-----------------------------------------------------------------------------
