
BufferSizeI     =       1*1024
BufferSizeO     =       1*1024

        use32

rax     equ     eax
rbx     equ     ebx
rcx     equ     ecx
rdx     equ     edx
rsi     equ     esi
rdi     equ     edi
rbp     equ     ebp
rsp     equ     esp

dq      equ     dd

macro   dv A,B
{
        db A
        dw B
}

macro   dy A,B,C
{
        db A,B
        dw C
}

macro   cmova   Dst,Src
{
local ..L
        jna     ..L
        mov     Dst,Src
..L:
}

macro   cmovae  Dst,Src
{
local ..L
        jnae    ..L
        mov     Dst,Src
..L:
}

macro   cmovb   Dst,Src
{
local ..L
        jnb     ..L
        mov     Dst,Src
..L:
}

macro   cmovbe  Dst,Src
{
local ..L
        jnbe    ..L
        mov     Dst,Src
..L:
}

macro   cmovg   Dst,Src
{
local ..L
        jng     ..L
        mov     Dst,Src
..L:
}

macro   cmovge  Dst,Src
{
local ..L
        jnge    ..L
        mov     Dst,Src
..L:
}

macro   cmovl   Dst,Src
{
local ..L
        jnl     ..L
        mov     Dst,Src
..L:
}

macro   cmovle  Dst,Src
{
local ..L
        jnle    ..L
        mov     Dst,Src
..L:
}

macro   cmovno  Dst,Src
{
local ..L
        jo      ..L
        mov     Dst,Src
..L:
}

macro   cmovnp  Dst,Src
{
local ..L
        jp      ..L
        mov     Dst,Src
..L:
}

macro   cmovns  Dst,Src
{
local ..L
        js      ..L
        mov     Dst,Src
..L:
}

macro   cmovnz  Dst,Src
{
local ..L
        jz      ..L
        mov     Dst,Src
..L:
}

macro   cmovo   Dst,Src
{
local ..L
        jno     ..L
        mov     Dst,Src
..L:
}

macro   cmovp   Dst,Src
{
local ..L
        jnp     ..L
        mov     Dst,Src
..L:
}

macro   cmovs   Dst,Src
{
local ..L
        jns     ..L
        mov     Dst,Src
..L:
}

macro   cmovz   Dst,Src
{
local ..L
        jnz     ..L
        mov     Dst,Src
..L:
}

macro   jrcxz   Dst
{
        jecxz   Dst
}

use32
InitDefault:
        mov     [DefInst.Arch],CPUX32
        mov     [DefInst.Size],15
        mov     [DefInst.Negate],True

        mov     [DefInst.Base],-1
        mov     [DefInst.Indx],-1
        mov     [DefInst.Scale],-1
        ret

include "disasm.asm"

CrLf            db 0
TheL            db 0
LastByte        db 0
IgnoreInvalid   db 0 ;Ignore invalid VEX/XOP
SpaceSize       dd 8
Origin          dd 0
TheBufferO      dd 0

_r8_    dd      ?
_r9_    dd      ?
_r10_   dq      ?
_r15_   dq      ?
