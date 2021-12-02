
;-----------------------------------------------------------------------------
match =32,Bitness {
rax     equ     eax
rbx     equ     ebx
rcx     equ     ecx
rdx     equ     edx
rsi     equ     esi
rdi     equ     edi
rbp     equ     ebp
rsp     equ     esp
r9      equ     [r9v]
r15d    equ     [r5v]
r15     equ     [r5v]
r10     equ     [r1v]
dq      equ     dd
_8_     equ     4
}
;-----------------------------------------------------------------------------
match =64,Bitness {
_8_     equ     8
}
;-----------------------------------------------------------------------------
macro   jrcxz   Dst
{
if %B=32
        jecxz   Dst
else
        jrcxz   Dst
end if
}
;-----------------------------------------------------------------------------
; REX.W = 0 - CS.D, 1 - 64bit
; REX.R = ModR/M reg field (GPR, SSE, CRx, DRx)
; REX.X = SIB index field
; REX.B = ModR/M r/m field, SIB base field, opcode reg field
;-----------------------------------------------------------------------------
; In 64Bits
; REX.W = 1 & 66h -> 66h is ignored if not mandatory
;-----------------------------------------------------------------------------
; Prefixes VEX Opc3 ModRM SIB DISP IMM
;
;      REX & VEX -> #UD
;     LOCK & VEX -> #UD
; 66/F2/F3 & VEX -> #UD
;
; VEX3 - 0F / 0F 38 / 0F 3A
; 11000100 RXBmmmmm WvvvvLpp
;
; VEX2 - 0F
; 11000101 RvvvvLpp
;
; REX.R = !VEX.R
;VEX.R=0: Same as REX.R=1 (64-bit mode only)
;VEX.R=1: Same as REX.R=0 (must be 1 in 32-bit mode)
; REX.X = !VEX.X
;VEX.X=0: Same as REX.X=1 (64-bit mode only)
;VEX.X=1: Same as REX.X=0 (must be 1 in 32-bit mode
; REX.B = !VEX.B
;VEX.B=0: Same as REX.B=1 (64-bit mode only)
;VEX.B=1: Same as REX.B=0 (Ignored in 32-bit mode)
;
; REX.W = VEX.W, 4 operands
; In 32-bit VEX.W is silently ignored.
;
; mmmmm = 01b - 0F
;         10b - 0F 38
;         11b - 0F 3A
; vvvv  = register specifier / 1111 - unused
; L     = 0 - vector 128bit or scalar / 1 - 256bit vector
; pp    = opcode extension - 00b - None
;                            01b - 66
;                            10b - F3
;                            11b - F2
;
;NDS, NDD, DDS: specifies that VEX.vvvv field is valid for the encoding of a
;register operand:
; VEX.NDS: VEX.vvvv encodes the first source register in an instruction
;syntax where the content of source registers will be preserved.
; VEX.NDD: VEX.vvvv encodes the destination register that cannot be
;encoded by ModR/M:reg field.
; VEX.DDS: VEX.vvvv encodes the second source register in a three-
;operand instruction syntax where the content of first source register will
;be overwritten by the result.
; If none of NDS, NDD, and DDS is present, VEX.vvvv must be 1111b (i.e.
;VEX.vvvv does not encode an operand). The VEX.vvvv field can be
;encoded using either the 2-byte or 3-byte form of the VEX prefix.
; /is4: An 8-bit immediate byte is present containing a source register
;specifier in imm[7:4] and instruction-specific payload in imm[3:0].
; imz2: Part of the is4 immediate byte providing control functions that
;apply to two-source permute instructions
;-----------------------------------------------------------------------------
; EVEX 62h P0 P1 P2
;
;P0:     7 6  5 4  3  2 0 1
;        R X  B R' 0  0 m m      P[7:0]
;P1      7 6  5 4  3  2 0 1
;        W v  v v  v  1 p p      P[15:8]
;P2      7 6  5 4  3  2 0 1
;        z L' L b  V' a a a      P[23:16]
;EVEX.mm        Compressed legacy escape                P[1:0]          Identical to low two bits of VEX.mmmmm
;EVEX.pp        Compressed legacy prefix                P[9:8]          Identical to VEX.pp
;EVEX.RXB       Next-8 register specifier modifier      P[7:5]          Combine with ModR/M.reg, ModR/M.rm (base, index/vidx)
;EVEX.R'        High-16 register specifier modifier     P[4]            Combine with EVEX.R and ModR/M.reg
;EVEX.X         High-16 register specifier modifier     P[6]            Combine with EVEX.B and ModR/M.rm, when SIB/VSIB absent
;EVEX.vvvv      NDS register specifier                  P[14:11]        Same as VEX.vvvv
;EVEX.V'        High-16 NDS/VIDX register specifier     P[19]           Combine with EVEX.vvvv or when VSIB present
;EVEX.aaa       Embedded opmask register specifier      P[18:16]
;EVEX.W         Osize promotion/Opcode extension        P[15]
;EVEX.z         Zeroing/Merging                         P[23]
;EVEX.b         Broadcast/RC/SAE Context                P[20]
;EVEX.L'L       Vector length/RC                        P[22:21]
;-------------------------------------------------------------------------------------------------------
;Addressing mode Bit 4           Bit 3   Bits [2:0]      Register type          Common usage
;-------------------------------------------------------------------------------------------------------
;REG            EVEX.R'         EVEX.R  ModRM.reg       General purpose, Vector Destination or Source
;NDS/NDD        EVEX.V'         EVEX.v3v2v1v0           GPR, Vector             2nd Source or Destination
;RM             EVEX.X          EVEX.B  ModRM.r/m       GPR, Vector             1st Source or Destination
;BASE           0               EVEX.B  ModRM.r/m       GPR                     Memory addressing
;INDEX          0               EVEX.X  SIB.index       GPR                     Memory addressing
;VIDX           EVEX.V'         EVEX.X  SIB.index       Vector                  VSIB memory addressing
;IS4            Imm8[3]         Imm8[7:4]               Vector                  3rd Source
;-------------------------------------------------------------------------------------------------------
;XOP Bit Mnemonic Description
;Byte 0:
;7-0 8Fh XOP Prefix Byte for 3-byte XOP Prefix
;Byte 1:
;7 R Inverted one bit extension to ModRM.reg field
;6 X Inverted one bit extension of the SIB index field
;5 B Inverted one bit extension of the ModRM r/m field or the SIB base field
;4-0 mmmmm
;XOP opcode map select:
;08h-instructions with immediate byte;
;09h-instructions with no immediate;
;Byte 2:
;7 W Default operand size override for a general pur-
;pose register to 64-bit size in 64-bit mode; oper-
;and configuration specifier for certain XMM/YMM-based operations.
;6-3 vvvv Source or destination register specifier
;2 L Vector length for XMM/YMM-based operations.
;1-0 pp
;Specifies whether there's an implied 66, F2, or F3 opcode extension
;-----------------------------------------------------------------------------
RXB = 1         ;REX.B (extension to the Base)
RXX = 2         ;REX.X (extension to the SIB indeX)
RXR = 4         ;REX.R (extension to the ModRM/REG)
RXW = 8         ;REX.W (operand Width; 0 = default, 1 = 64bit)
RXP = 40h       ;REX prefix
;-----------------------------------------------------------------------------

MaxInstructionLength    = 15

MnemonicLength          = 17      ;maximum length of instruction name

;Unsigned Minimum eax,edx to eax
macro MinU
{
        cmp     rax,rdx
        sbb     rcx,rcx
        and     rax,rcx
        not     rcx
        and     rdx,rcx
        or      rax,rdx
}
;Unsigned Maximum eax,edx to eax
macro MaxU
{
        cmp     rdx,rax
        sbb     rcx,rcx
        and     rax,rcx
        not     rcx
        and     rdx,rcx
        or      rax,rdx
}
;-----------------------------------------------------------------------------
; I/O: eax - size
macro   Compress
{
        push    rbx rcx rdx rsi rdi
        mov     rbx,[TheBufferO]
        mov     ecx,eax
        xor     edi,edi
        xor     esi,esi
        mov     [TheK],rdi
.1:     mov     ah,[TabSize]
        xor     edx,edx
.2:     mov     al,[rbx+rsi]
        mov     [rbx+rdi],al
        inc     rsi
        cmp     rsi,rcx
        ja      .6
        inc     rdi
        inc     [TheK]
        cmp     al,32
        jne     .3
        inc     dl
        jmp     .4
.3:     xor     dl,dl
.4:     dec     ah
        jnz     .2
        or      dl,dl
        jz      .5
        dec     dl
        jz      .5
        sub     rdi,rdx
        sub     [TheK],rdx
        mov     al,9
        mov     [rbx+rdi-1],al
.5:     jmp     .1
.6:     mov     rax,[TheK]
        pop     rdi rsi rdx rcx rbx
}

Names:  file    "qopcodes.bin"
include "qopcodes.inc"

TNULL           =       ($-1-Names)

False           =       0
True            =       1

NIA             =       1       ;Not Intel/AMD
UND             =       2       ;Undocumented or abandon

RACC            =       RRAX

RRAX            =       0
RRCX            =       1
RRDX            =       2
RRBX            =       3
RRSP            =       4
RRBP            =       5
RRSI            =       6
RRDI            =       7
RR8             =       8
RR9             =       9
RR10            =       10
RR11            =       11
RR12            =       12
RR13            =       13
RR14            =       14
RR15            =       15

RES             =       0
RCS             =       1
RSS             =       2
RDS             =       3
RFS             =       4
RGS             =       5

VES             =       26h
VCS             =       2Eh
VSS             =       36h
VDS             =       3Eh
VFS             =       64h
VGS             =       65h

VNONE           =       0
VBYTE           =       1
VWORD           =       2
VDWORD          =       3
VQWORD          =       4
VOWORD          =       5
VXWORD          =       5
VYWORD          =       6
VZWORD          =       7
VFWORD          =       8
VTWORD          =       9
;-----------------------------------------------------------------------------
xN              =       VNONE
xB              =       VBYTE
xW              =       VWORD
xD              =       VDWORD
xQ              =       VQWORD
xO              =       VOWORD
xX              =       VXWORD
xY              =       VYWORD
xZ              =       VZWORD
xF              =       VFWORD
xT              =       VTWORD
;-----------------------------------------------------------------------------
CPUX16          =       0
CPUX32          =       1
CPUX64          =       2

AD16            =       0
AD32            =       1
AD64            =       2
ADXX            =       3
ADYY            =       4
ADZZ            =       5

;kolejno˜† ma znaczenie/the order is significant
PF3             =       0
PF2             =       1
P66             =       2

P67             =       3
P4X             =       4
P9B             =       5
PF0             =       6
PSEG            =       7

GPR08           =       1
GPR16           =       2
GPR32           =       3
GPR64           =       4
SEGRG           =       5
STXRG           =       6
CTRRG           =       7
DBGRG           =       8
MMXRG           =       9
XMMRG           =       10
YMMRG           =       11
ZMMRG           =       12
BNDRG           =       13
TRXRG           =       14
KXXRG           =       15
;-----------------------------------------------------------------------------
_R              =           1   ;Clear 66h prefix
_R64            =           2   ;Clear 66h prefix if CPUX64
_O              =           4   ;Do not show memory operand size
_R48            =           8   ;Clear 48h prefix
_J              =         10h   ;Show Branch Hint
_A              =         20h   ;Always call 3rd procedure
_T              =         40h   ;Extended = Table
_E              =         80h   ;Extended Table
_3              =        100h   ;MOD3 (I.Mod=3, CL=3 next 4 names in TABle)
_S              =       8000h   ;add "S"uffix in ATT
_XA             =       4000h   ;XACQUIRE
_XR             =       2000h   ;XRELEASE
_L              =       1000h   ;LOCK unnecessary
_B              =        800h   ;BND prefix instruction
;-----------------------------------------------------------------------------
VEXF            =       8000h   ;without third opcode
VEX2            =       4000h   ;2nd operand always = xmm
VEX1            =       2000h   ;1st operand always = xmm
VEXH            =       1000h   ;MoveName instead MoveNameV
VEXG            =        800h   ;operands - GPR32, VEXW - GPR64
VEXC            =        400h   ;VEXW=1 = no error
VEXS            =        200h   ;encoded NDS
VEXR            =        100h   ;only register operands
VEXW            =         80h   ;if W=1 then error
VEXI            =         40h   ;imm8
VEX4            =         20h   ;is4
VEXV            =         10h   ;must be vvvv=1111b
VEX8            =         08h   ;imm8=opcode's new name
VEXL            =         04h   ;if L=1 then error
VEXN            =         02h   ;if L=0 then error
VEXM            =         01h   ;only memory operands
VEX0            =         00h   ;undefined
;-----------------------------------------------------------------------------
XOPO            =         80h   ;00h=RM?R?,80h=R?RM?
XOP2            =         40h   ;2nd operand always = xmm
XOP1            =         20h   ;1st operand always = xmm
XOPV            =         10h   ;must be vvvv=1111b
XOPI            =         08h   ;imm8
;-----------------------------------------------------------------------------
XOPL            =         04h   ;if L=1 then error & flag like VEX.L
XOPW            =         80h   ;if W=1 then error & flag like REX.W
;-----------------------------------------------------------------------------
EVEXAA          =      10000h           ;for decorator
EVEXQ1          =      08000h           ;add suffix Q1 when W=1
EVEXD1          =      04000h           ;add suffix D1 when W=1
EVEXE           =      (EVEXQ1+EVEXD1)  ;name points to two names
EVEXI           =      02000h           ;imm8
EVEXM           =      01000h           ;call GetVectorAndMUL
EVEXB           =      00800h           ;inc MemorySize & MULT when (W=1 & BB!=0)
EVEXU           =      00400h           ;inc MemorySize & MULT when W=1
EVEXL0          =      00200h           ;if L'L=00b then error
EVEXLL          =      00100h           ;if L'L<10b then error
EVEXLX          =      (EVEXL0+EVEXLL)  ;if L'L>00b then error
;-----------------------------------------------------------------------------
EVEXW0          =      (EVEXYW+EVEXNW)  ;ignore W1 in non 64-bits
EVEXYW          =      00080h           ;if W=1 then error
EVEXNW          =      00040h           ;if W=0 then error
EVEXV           =      00020h           ;if VVVV!=1111b then error
EVEX2           =      00010h           ;proc points to two procs
;-----------------------------------------------------------------------------
EVEXS           =      00008h           ;skip {K1}
EVEXR           =      00004h           ;indicates support for embedded rounding control, which is only applicable to the register-register form of the instruction
EVEXO           =      00002h           ;EVEXR Only if W=1
EVEXX           =      00001h           ;always XMM, L'L ignore
;-----------------------------------------------------------------------------
EVEX0           =      00000h           ;undefined
;-----------------------------------------------------------------------------
include "scanitem.inc"
;-----------------------------------------------------------------------------
macro xx Name,Rtn,Rand=0,DefReg=0,Flags=0
{
local Temp
if Rtn-RtnXX<0
  err invalid procedure address
end if
        dw Name
virtual at 0
Temp::
        dw Rtn-RtnXX
if (Flags) and _E = _E
  if DefReg-EXT80<0
    err invalid table address
  end if
        dw DefReg-EXT80
else if (Flags) and _T = _T
  if DefReg-TTXXXX<0
    err invalid table address
  end if
        dw DefReg-TTXXXX
else
        db Rand shl 4+DefReg,?
end if
        dw Flags
end virtual
        AddElement Temp:
}

virtual at 0
xxh:
.Rtn    dw ?
.NxtTab rw 0
.DefReg db ?,?
.Flags  dw ?
.Size:
end virtual

virtual at 0
yyh:
.Name:  dw ?
.Addr:  dw ?
.Size:
end virtual
;-----------------------------------------------------------------------------
virtual at 0    ;XOP
XOP:
.Imm    db ?
.Flags  db ?
.MOS    db ?,?
.Rtn    dw ?
.Name   dw ?
.Size:
end virtual
;-----------------------------------------------------------------------------
macro XOPc Imm,MOS0,MOS1,Flags,Rtn,NameV
{
        db Imm
        db Flags
        db MOS0,MOS1
        dw Rtn-RtnXX
        dw NameV
}
;-----------------------------------------------------------------------------
virtual at 0    ;xC
CXX:
.Rtn    dw ?
.Flags  dw ?
.Imm    db ?
.MOS    db ?
.Name   dw ?
.Size:
end virtual
;-----------------------------------------------------------------------------
macro xC Routine,Flags,Imm,MOS0,MOS1,NameV
{
        dw Routine-RtnXX
        dw Flags
        db Imm
        db (MOS1)*16+MOS0
        dw NameV
}
;-----------------------------------------------------------------------------
virtual at 0
X62:
.Rtn    dw ?
.Name   dw ?
.Flags  dw ?
.Imm    db ?
.MOS    db ?
.Size:
end virtual
;-----------------------------------------------------------------------------
macro T62 Routine,Imm,Name,Flags,MemSizeH,MemSizeL
{
        dw Routine-RtnXX
  if ((Flags) and EVEXE = EVEXE)
        dw Name-T62Names
  else
        dw Name
  end if
        dw Flags
        db Imm
        db (MemSizeH*16)+MemSizeL
}
;-----------------------------------------------------------------------------
macro   FetchB
{
        lodsb
}

macro   FetchW
{
        lodsw
}

macro   FetchD
{
        lodsd
}

macro   FetchUD
{
        lodsd
        or      [I.MainFlags],80h
}

macro   FetchSB
{
        FetchB
        movsx   rax,al
}

macro   FetchSW
{
        FetchW
        movsx   rax,ax
}

macro   FetchSD
{
        FetchD
if %B=64
        movsxd  rax,eax
end if
}

macro   FetchQ
{
if %B=64
        lodsq
else
        lodsd
        mov     edx,eax
        lodsd
        xchg    edx,eax
end if
}

macro   BitT    Mem,Val
{
        bt      dword [Mem+Val/8],Val and 7
}

macro   BitTS   Mem,Val
{
        bts     dword [Mem+Val/8],Val and 7
}

macro   BitTR   Mem,Val
{
        btr     dword [Mem+Val/8],Val and 7
}

;Type2nd=
InNONE          = 0
InRM            = 1
InReg           = 2
InVVVV          = 3
Is1             = 4

struct  TArg
Type            db ?
Type2nd         db ?
Reg             db ?
Mem             db ?
ImmB            rb 0
Imm             dq ?
if %B=32
                dd ?
end if
ends

struct  TInstruction
;
Addr            dq ?
SaveRSP         dq ?
SaveRSI         dq ?
Size            dq ?
Item            dd ?
Table           dd ?
;
Arch            db ?
Only            db ?
NextByte        db ?
NewTable        db ?
;
NotR            db ?
NotX            db ?
NotB            db ?
NotW            db ?
NotP            db ?
;
Is62            db ?
;
R               db ?
X               db ?
B               db ?
W               db ?
P               db ?
V               db ?
Z               db ?
BB              db ?
PP              db ?
LL              db ?
XX              db ?
AAAA            db ?
MULT            db ?
;
XOP             db ?
VEX             db ?
VVVV            db ?
MMMMM           db ?
;
FlagsVEX        rb 0
FlagsXOP        db ?,?
FlagsEVEX       db ?,?,?
;
VT              db ?
AfterVEX        db ?
;
Fasm            db ?
Medium          db ?
UseDB           db ?
NoREX           db ?
FullHex         db ?
Dollar          db ?
HexPrefix       db ?
Negate          db ?
ShowRand        db ?    ;Always show operand size
ShowSize        db ?
RandSize        db ?
ShowScale       db ?
NoAddress       db ?
HideRIP         db ?
IsShort         db ?
Emulated        db ?
IsAddress       db ?
Sensitive       db ?
NoShowRand      db ?
DisplayHint     db ?
PossibleLOCK    db ?
PossibleF2F3    db ?
Compression     db ?
Intelligent     db ?
IsCALLJMP       db ?
Prefix          db ?
LastByte        db ?
;
Operand         db ?
Address         db ?
CurSeg          db ?
DefSeg          db ?
PreREX          db ?
IsFloat         db ?
IsRET           db ?
Syntax          db ?
;
ModRM           db ?
SIB             db ?
Flags           dw ?
Point           dw ?
;
RM              db ?
Reg             db ?
Mod             db ?
Relative        db ?
;
Base            db ?
Indx            db ?
Scale           db ?
DispSize        db ?
;
Pos66           db ?
Pos67           db ?
Pos4X           db ?
Pos9B           db ?
PosF0           db ?
PosF2           db ?
PosF3           db ?
PosSEG          db ?
;
Disp            dq ?
if %B=32
                dd ?
end if
;
IsLEA           db ?
PrefixByte      db ?
Mandatory66     db ?
MemSeparator    db ?
MainFlags       db ?
;
Name            dw ?
AltName         dw ?
SegmName        dw ?
;
Arg1            TArg
Arg2            TArg
Arg3            TArg
Arg4            TArg
Arg5            TArg
;
Suffix          rb 4
Suffix2nd       rb 4
Suffix3rd       rb 4
LastArg         dq ?
;
Prefixes        dd ?
PrefixesCpy     dd ?
PrefixNames     rw 16
PrefixBites     rb 16
PrefixCount     db ?
;
ends

virtual at rbp-128      ;maximum lower address
I       TInstruction
end virtual

macro   HexString
{
local A
virtual at 0
HexVal::db      '0123456789ABCDEF'
end virtual
even 4
HexString:
rept 256 n:0 {\
  load A byte from HexVal:(n shr 4)
        db      A
  load A byte from HexVal:(n and 15)
        db      A
  \}
}

HexString
Scales          db      '1248'
Suffixes        db      0,'bwlq'
;-----------------------------------------------------------------------------
TSAE            dw T?RN?SAE?
                dw T?RD?SAE?
                dw T?RU?SAE?
                dw T?RZ?SAE?
;-----------------------------------------------------------------------------
T1TO            dw T?1TO2?
                dw T?1TO4?
                dw T?1TO8?
                dw T?1TO16?
;-----------------------------------------------------------------------------
NGPR08          dw TAL,\
                   TCL,\
                   TDL,\
                   TBL,\
                   TAH,\
                   TCH,\
                   TDH,\
                   TBH,\
                   TAL,\
                   TCL,\
                   TDL,\
                   TBL,\
                   TAH,\
                   TCH,\
                   TDH,\
                   TBH

NGPRX8          dw TAL,\
                   TCL,\
                   TDL,\
                   TBL,\
                   TSPL,\
                   TBPL,\
                   TSIL,\
                   TDIL,\
                   TR8B,\
                   TR9B,\
                   TR10B,\
                   TR11B,\
                   TR12B,\
                   TR13B,\
                   TR14B,\
                   TR15B

NGPRL8          dw TAL,\
                   TCL,\
                   TDL,\
                   TBL,\
                   TSPL,\
                   TBPL,\
                   TSIL,\
                   TDIL,\
                   TR8L,\
                   TR9L,\
                   TR10L,\
                   TR11L,\
                   TR12L,\
                   TR13L,\
                   TR14L,\
                   TR15L

NGPR16          dw TAX,\
                   TCX,\
                   TDX,\
                   TBX,\
                   TSP,\
                   TBP,\
                   TSI,\
                   TDI,\
                   TR8W,\
                   TR9W,\
                   TR10W,\
                   TR11W,\
                   TR12W,\
                   TR13W,\
                   TR14W,\
                   TR15W

NGPR32          dw TEAX,\
                   TECX,\
                   TEDX,\
                   TEBX,\
                   TESP,\
                   TEBP,\
                   TESI,\
                   TEDI,\
                   TR8D,\
                   TR9D,\
                   TR10D,\
                   TR11D,\
                   TR12D,\
                   TR13D,\
                   TR14D,\
                   TR15D,\
                   TEIP?

NGPR64          dw TRAX,\
                   TRCX,\
                   TRDX,\
                   TRBX,\
                   TRSP,\
                   TRBP,\
                   TRSI,\
                   TRDI,\
                   TR8,\
                   TR9,\
                   TR10,\
                   TR11,\
                   TR12,\
                   TR13,\
                   TR14,\
                   TR15,\
                   TRIP?

NSTRXX          dw TST0,\
                   TST1,\
                   TST2,\
                   TST3,\
                   TST4,\
                   TST5,\
                   TST6,\
                   TST7

NSTRYY          dw TST?0?,\
                   TST?1?,\
                   TST?2?,\
                   TST?3?,\
                   TST?4?,\
                   TST?5?,\
                   TST?6?,\
                   TST?7?

NCTRXX          dw TCR0,\
                   TCR1,\
                   TCR2,\
                   TCR3,\
                   TCR4,\
                   TCR5,\
                   TCR6,\
                   TCR7,\
                   TCR8,\
                   TCR9,\
                   TCR10,\
                   TCR11,\
                   TCR12,\
                   TCR13,\
                   TCR14,\
                   TCR15

NDBGXX          dw TDR0,\
                   TDR1,\
                   TDR2,\
                   TDR3,\
                   TDR4,\
                   TDR5,\
                   TDR6,\
                   TDR7,\
                   TDR8,\
                   TDR9,\
                   TDR10,\
                   TDR11,\
                   TDR12,\
                   TDR13,\
                   TDR14,\
                   TDR15

NDBGYY          dw TDB0,\
                   TDB1,\
                   TDB2,\
                   TDB3,\
                   TDB4,\
                   TDB5,\
                   TDB6,\
                   TDB7,\
                   TDB8,\
                   TDB9,\
                   TDB10,\
                   TDB11,\
                   TDB12,\
                   TDB13,\
                   TDB14,\
                   TDB15

NXMMXX          dw TXMM0,\
                   TXMM1,\
                   TXMM2,\
                   TXMM3,\
                   TXMM4,\
                   TXMM5,\
                   TXMM6,\
                   TXMM7,\
                   TXMM8,\
                   TXMM9,\
                   TXMM10,\
                   TXMM11,\
                   TXMM12,\
                   TXMM13,\
                   TXMM14,\
                   TXMM15,\
                   TXMM16,\
                   TXMM17,\
                   TXMM18,\
                   TXMM19,\
                   TXMM20,\
                   TXMM21,\
                   TXMM22,\
                   TXMM23,\
                   TXMM24,\
                   TXMM25,\
                   TXMM26,\
                   TXMM27,\
                   TXMM28,\
                   TXMM29,\
                   TXMM30,\
                   TXMM31

NYMMXX          dw TYMM0,\
                   TYMM1,\
                   TYMM2,\
                   TYMM3,\
                   TYMM4,\
                   TYMM5,\
                   TYMM6,\
                   TYMM7,\
                   TYMM8,\
                   TYMM9,\
                   TYMM10,\
                   TYMM11,\
                   TYMM12,\
                   TYMM13,\
                   TYMM14,\
                   TYMM15,\
                   TYMM16,\
                   TYMM17,\
                   TYMM18,\
                   TYMM19,\
                   TYMM20,\
                   TYMM21,\
                   TYMM22,\
                   TYMM23,\
                   TYMM24,\
                   TYMM25,\
                   TYMM26,\
                   TYMM27,\
                   TYMM28,\
                   TYMM29,\
                   TYMM30,\
                   TYMM31

NZMMXX          dw TZMM0,\
                   TZMM1,\
                   TZMM2,\
                   TZMM3,\
                   TZMM4,\
                   TZMM5,\
                   TZMM6,\
                   TZMM7,\
                   TZMM8,\
                   TZMM9,\
                   TZMM10,\
                   TZMM11,\
                   TZMM12,\
                   TZMM13,\
                   TZMM14,\
                   TZMM15,\
                   TZMM16,\
                   TZMM17,\
                   TZMM18,\
                   TZMM19,\
                   TZMM20,\
                   TZMM21,\
                   TZMM22,\
                   TZMM23,\
                   TZMM24,\
                   TZMM25,\
                   TZMM26,\
                   TZMM27,\
                   TZMM28,\
                   TZMM29,\
                   TZMM30,\
                   TZMM31
;-----------------------------------------------------------------------------
NBNDX           dw TBND0,TBND1,TBND2,TBND3,TBND4?,TBND5?,TBND6?,TBND7?
;-----------------------------------------------------------------------------
NSEGR           dw TES,TCS,TSS,TDS,TFS,TGS,TS6,TS7
NKXXX           dw TK0,TK1,TK2,TK3,TK4,TK5,TK6,TK7
;-----------------------------------------------------------------------------
NMMXXX          dw TMM0,TMM1,TMM2,TMM3,TMM4,TMM5,TMM6,TMM7
NTRXXX          dw TTR0,TTR1,TTR2,TTR3,TTR4,TTR5,TTR6,TTR7
;-----------------------------------------------------------------------------
NSEGX           dw TSEGES,TSEGCS,TSEGSS,TSEGDS,TSEGFS,TSEGGS
;-----------------------------------------------------------------------------
DefCPU2AD       db AD16,AD32,AD64
XorCPU2AD       db AD32,AD16,AD32
DefCPU2OS       db 0,1,1
XorCPU2OS       db 1,0,0
;-----------------------------------------------------------------------------
if used DefArgSize
DefArgSize      db VWORD,GPR16,VDWORD,GPR32,VQWORD,GPR64
end if
;-----------------------------------------------------------------------------
NotP4X          dd not (bit P4X)
;-----------------------------------------------------------------------------
SZ2Mems         dw TBYTE,TWORD,TDWORD,TQWORD
SZ2Name         dw TNULL,TBYTE,TWORD,TDWORD,TQWORD,TXWORD,TYWORD,TZWORD,\
                   TFWORD,TTWORD
;-----------------------------------------------------------------------------
AD2Regs         dd NGPR16,NGPR32,NGPR64,NXMMXX,NYMMXX,NZMMXX
;-----------------------------------------------------------------------------
IntelName       dd NGPRX8
SZ2Regs         dd NGPR08,NGPR16,NGPR32,NGPR64,NSEGR,NSTRXX,NCTRXX,NDBGXX,\
                   NMMXXX,NXMMXX,NYMMXX,NZMMXX,NBNDX,NTRXXX,NKXXX
;-----------------------------------------------------------------------------
TBIT            dd 1 shl GPR08+\
                   1 shl GPR16+\
                   1 shl GPR32+\
                   1 shl GPR64+\
                   0 shl SEGRG+\
                   0 shl STXRG+\
                   1 shl CTRRG+\
                   1 shl DBGRG+\
                   0 shl MMXRG+\
                   1 shl XMMRG+\
                   1 shl YMMRG+\
                   1 shl ZMMRG+\
                   0 shl BNDRG+\
                   0 shl TRXRG+\
                   0 shl KXXRG
;-----------------------------------------------------------------------------
SZ2Mask         dq 0x00,0xFF,0xFFFF,0xFFFFFFFF
;-----------------------------------------------------------------------------
;One Table 0F (X0-XF)
EXTBITS dd 0 shl 0+\    ;-      0
           0 shl 1+\    ;-      1
           1 shl 2+\    ;+      2
           1 shl 3+\    ;+      3
           0 shl 4+\    ;-      4
           1 shl 5+\    ;+      5
           1 shl 6+\    ;+      6
           1 shl 7+\    ;+      7
           1 shl 8+\    ;+      8
           1 shl 9+\    ;+      9
           0 shl 10+\   ;-      A
           0 shl 11+\   ;-      B
           0 shl 12+\   ;-      C
           1 shl 13+\   ;+      D
           1 shl 14+\   ;+      E
           1 shl 15     ;+      F
;-----------------------------------------------------------------------------
EXT0F   dd EXT0F00,EXT0F10,EXT0F20,EXT0F30,EXT0F40,EXT0F50,EXT0F60,EXT0F70
        dd EXT0F80,EXT0F90,EXT0FA0,EXT0FB0,EXT0FC0,EXT0FD0,EXT0FE0,EXT0FF0
;-----------------------------------------------------------------------------
TFLDXTable:     dw TFLD1
                dw TFLDL2T
                dw TFLDL2E
                dw TFLDPI
                dw TFLDLG2
                dw TFLDLN2
                dw TFLDZ
                dw 0

TE110Table:     dw TF2XM1
                dw TFYL2X
                dw TFPTAN
                dw TFPATAN
                dw TFXTRACT
                dw TFPREM1
                dw TFDECSTP
                dw TFINCSTP

TE111Table:     dw TFPREM
                dw TFYL2XP1
                dw TFSQRT
                dw TFSINCOS
                dw TFRNDINT
                dw TFSCALE
                dw TFSIN
                dw TFCOS

TVMXXTable:     dw TVMRUN
                dw TVMMCALL
                dw TVMLOAD
                dw TVMSAVE
                dw TSTGI
                dw TCLGI
                dw TSKINIT
                dw TINVLPGA
;-----------------------------------------------------------------------------
Now3D:  dv 0Ch,TPI2FW
        dv 0Dh,TPI2FD
        dv 1Ch,TPF2IW
        dv 1Dh,TPF2ID
        dv 8Ah,TPFNACC
        dv 8Eh,TPFPNACC
        dv 90h,TPFCMPGE
        dv 94h,TPFMIN
        dv 96h,TPFRCP
        dv 97h,TPFRSQRT
        dv 9Ah,TPFSUB
        dv 9Eh,TPFADD
        dv $A0,TPFCMPGT
        dv $A4,TPFMAX
        dv $A6,TPFRCPIT1
        dv $A7,TPFRSQIT1
        dv $AA,TPFSUBR
        dv $AE,TPFACC
        dv $B0,TPFCMPEQ
        dv $B4,TPFMUL
        dv $B6,TPFRCPIT2
        dv $B7,TPMULHRW
        dv $BB,TPSWAPD
        dv $BF,TPAVGUSB
        ;
        dv $86,TPFRCPV  ;Cyrix
        dv $87,TPFRSQRTV;Cyrix
        ;
End3D:
;-----------------------------------------------------------------------------
TabSSE4 rb 0
        dv $00,TPSHUFB
        dv $01,TPHADDW
        dv $02,TPHADDD
        dv $03,TPHADDSW
        dv $04,TPMADDUBSW
        dv $05,TPHSUBW
        dv $06,TPHSUBD
        dv $07,TPHSUBSW
        dv $08,TPSIGNB
        dv $09,TPSIGNW
        dv $0A,TPSIGND
        dv $0B,TPMULHRSW
        dv $1C,TPABSB
        dv $1D,TPABSW
        dv $1E,TPABSD
EndSSE4 rb 0
;-----------------------------------------------------------------------------
NxtSSE4 rb 0
        dy $10,VXWORD+80h,TPBLENDVB
        dy $14,VXWORD+80h,TBLENDVPS
        dy $15,VXWORD+80h,TBLENDVPD
        dy $17,VXWORD+80h,TPTEST
        dy $20,VQWORD+80h,TPMOVSXBW
        dy $21,VDWORD+80h,TPMOVSXBD
        dy $22, VWORD+80h,TPMOVSXBQ
        dy $23,VQWORD+80h,TPMOVSXWD
        dy $24,VDWORD+80h,TPMOVSXWQ
        dy $25,VQWORD+80h,TPMOVSXDQ
        dy $28,VXWORD+80h,TPMULDQ
        dy $29,VXWORD+80h,TPCMPEQQ
        dy $2A,VXWORD+80h,TMOVNTDQA
        dy $2B,VXWORD+80h,TPACKUSDW
        dy $30,VQWORD+80h,TPMOVZXBW
        dy $31,VDWORD+80h,TPMOVZXBD
        dy $32, VWORD+80h,TPMOVZXBQ
        dy $33,VQWORD+80h,TPMOVZXWD
        dy $34,VDWORD+80h,TPMOVZXWQ
        dy $35,VQWORD+80h,TPMOVZXDQ
        dy $37,VXWORD+80h,TPCMPGTQ
        dy $38,VXWORD+80h,TPMINSB
        dy $39,VXWORD+80h,TPMINSD
        dy $3A,VXWORD+80h,TPMINUW
        dy $3B,VXWORD+80h,TPMINUD
        dy $3C,VXWORD+80h,TPMAXSB
        dy $3D,VXWORD+80h,TPMAXSD
        dy $3E,VXWORD+80h,TPMAXUW
        dy $3F,VXWORD+80h,TPMAXUD
        dy $40,VXWORD+80h,TPMULLD
        dy $41,VXWORD+80h,TPHMINPOSUW
        dy $CF,VXWORD+80h,TGF2P8MULB
        dy $DB,VXWORD+80h,TAESIMC
        dy $DC,VXWORD+80h,TAESENC
        dy $DD,VXWORD+80h,TAESENCLAST
        dy $DE,VXWORD+80h,TAESDEC
        dy $DF,VXWORD+80h,TAESDECLAST
EndNxtSSE4:
;-----------------------------------------------------------------------------
TableSXSSE:
        dy $08,VOWORD+80h,TROUNDPS
        dy $09,VOWORD+80h,TROUNDPD
        dy $0A,VDWORD+80h,TROUNDSS
        dy $0B,VQWORD+80h,TROUNDSD
        dy $0C,VOWORD+80h,TBLENDPS
        dy $0D,VOWORD+80h,TBLENDPD
        dy $0E,VOWORD+80h,TPBLENDW
        dy $0F,VOWORD+80h,TPALIGNR
        dy $40,VOWORD+80h,TDPPS
        dy $41,VOWORD+80h,TDPPD
        dy $42,VOWORD+80h,TMPSADBW
        dy $60,VOWORD+80h,TPCMPESTRM
        dy $61,VOWORD+80h,TPCMPESTRI
        dy $62,VOWORD+80h,TPCMPISTRM
        dy $63,VOWORD+80h,TPCMPISTRI
        dy $CE,VOWORD+80h,TGF2P8AFFINEQB
        dy $CF,VOWORD+80h,TGF2P8AFFINEINVQB
        dy $DF,VOWORD+80h,TAESKEYGENASSIST
TableEXSSE:
;-----------------------------------------------------------------------------
TabCLMUL:
        dw TPCLMULLQLQDQ        ;0000_0000b
        dw TPCLMULHQLQDQ        ;0000_0001b
        rw 14
        dw TPCLMULLQHQDQ        ;0001_0000b
        dw TPCLMULHQHQDQ        ;0001_0001b
;-----------------------------------------------------------------------------
T0F71Names:
        dw 0            ;/0
        dw 0            ;/1
        dw TPSRLW       ;/2
        dw 0            ;/3
        dw TPSRAW       ;/4
        dw 0            ;/5
        dw TPSLLW       ;/6
        dw 0            ;/7
;-----------------------------------------------------------------------------
T0F72Names:
        dw 0            ;/0
        dw 0            ;/1
        dw TPSRLD       ;/2
        dw 0            ;/3
        dw TPSRAD       ;/4
        dw 0            ;/5
        dw TPSLLD       ;/6
        dw 0            ;/7
;-----------------------------------------------------------------------------
T0F73Names:
        dw 0            ;/0
        dw 0            ;/1
        dw TPSRLQ       ;/2
        dw TPSRLDQ      ;/3
        dw 0            ;/4
        dw 0            ;/5
        dw TPSLLQ       ;/6
        dw TPSLLDQ      ;/7
;-----------------------------------------------------------------------------
TCentTable:
        dw TMONTMUL     ;C0
        dw TXSHA1       ;C8
        dw TXSHA256     ;D0
;-----------------------------------------------------------------------------
TCRYPTable:
        dw 0            ;C0
        dw TXCRYPTECB   ;C8
        dw TXCRYPTCBC   ;D0
        dw TXCRYPTCTR   ;D8
        dw TXCRYPTCFB   ;E0
        dw TXCRYPTOFB   ;E8
        dw 0            ;F0
        dw 0            ;F8
;-----------------------------------------------------------------------------
T0FAETable:
        dw TFXSAVE
        dw TFXRSTOR
        dw TLDMXCSR
        dw TSTMXCSR
        dw TXSAVE
        dw TXRSTOR
        dw TXSAVEOPT
        dw TCLFLUSH
;-----------------------------------------------------------------------------
T0FAETab64:
        dw TFXSAVE64
        dw TFXRSTOR64
        dw 0
        dw 0
        dw TXSAVE64
        dw TXRSTOR64
        dw TXSAVEOPT64
        dw 0
;-----------------------------------------------------------------------------
TF30FAETable:
        dw TRDFSBASE
        dw TRDGSBASE
        dw TWRFSBASE
        dw TWRGSBASE
        dw TPTWRITE
        dw TINCSSPD
        dw TUMONITOR
        dw 0
;-----------------------------------------------------------------------------
T660FAETable0:
        dw 0
        dw 0
        dw 0
        dw 0
        dw 0
        dw 0
        dw TCLWB
        dw TCLFLUSHOPT
;-----------------------------------------------------------------------------
T660FAETable3:
        dw 0
        dw 0
        dw 0
        dw 0
        dw 0
        dw 0
        dw TTPAUSE
        dw TPCOMMIT
;-----------------------------------------------------------------------------
T0FC7Table:
        dw 0
        dw 0
        dw 0
        dw TXRSTORS
        dw TXSAVEC
        dw TXSAVES
        dw 0
        dw 0
;-----------------------------------------------------------------------------
T0FC7Tab64:
        dw 0
        dw 0
        dw 0
        dw TXRSTORS64
        dw TXSAVEC64
        dw TXSAVES64
        dw 0
        dw 0
;-----------------------------------------------------------------------------
TablePCOM:
        dw TPCOMLT      ;0
        dw TPCOMLE      ;1
        dw TPCOMGT      ;2
        dw TPCOMGE      ;3
        dw TPCOMEQ      ;4
        dw TPCOMNEQ     ;5
        dw TPCOMFALSE   ;6
        dw TPCOMTRUE    ;7
;-----------------------------------------------------------------------------
CMPSuffixesY:
        db 'ps',VXWORD+80h;00
        db 'pd',VXWORD+80h;66
        db 'sd',VQWORD+80h;F2
        db 'ss',VDWORD+80h;F3
;-----------------------------------------------------------------------------
CMPSuffixesX:
        db 'ps',VXWORD+80h;00
        db 'pd',VXWORD+80h;66
        db 'ss',VDWORD+80h;F3
        db 'sd',VQWORD+80h;F2
;-----------------------------------------------------------------------------
CMPxxNames:
        dw TCMPEQ       ;0
        dw TCMPLT       ;1
        dw TCMPLE       ;2
        dw TCMPUNORD    ;3
        dw TCMPNEQ      ;4
        dw TCMPNLT      ;5
        dw TCMPNLE      ;6
        dw TCMPORD      ;7
        dw TCMPEQ?UQ    ;8
        dw TCMPNGE      ;9
        dw TCMPNGT      ;A
        dw TCMPFALSE    ;B
        dw TCMPNEQ?OQ   ;C
        dw TCMPGE       ;D
        dw TCMPGT       ;E
        dw TCMPTRUE     ;F
        dw TCMPEQ?OS    ;10
        dw TCMPLT?OQ    ;11
        dw TCMPLE?OQ    ;12
        dw TCMPUNORD?S  ;13
        dw TCMPNEQ?US   ;14
        dw TCMPNLT?UQ   ;15
        dw TCMPNLE?UQ   ;16
        dw TCMPORD?S    ;17
        dw TCMPEQ?US    ;18
        dw TCMPNGE?UQ   ;19
        dw TCMPNGT?UQ   ;1A
        dw TCMPFALSE?OS ;1B
        dw TCMPNEQ?OS   ;1C
        dw TCMPGE?OQ    ;1D
        dw TCMPGT?OQ    ;1E
        dw TCMPTRUE?US  ;1F
;-----------------------------------------------------------------------------
TBM1Table:
        dw 0            ;/0
        dw 0            ;/1
        dw TBLSFILL     ;/2
        dw TBLCS        ;/3
        dw TTZMSK       ;/4
        dw TBLCIC       ;/5
        dw TBLSIC       ;/6
        dw TT1MSKC      ;/7
;-----------------------------------------------------------------------------
TBM2Table:
        dw 0            ;/0
        dw TBLCMSK      ;/1
        dw 0            ;/2
        dw 0            ;/3
        dw 0            ;/4
        dw 0            ;/5
        dw TBLCI        ;/6
        dw 0            ;/7
;-----------------------------------------------------------------------------
TPERMIL2PDX:
        dw TPERMILTD2PD
        dw TPERMILTD2PD
        dw TPERMILMO2PD
        dw TPERMILMZ2PD
;-----------------------------------------------------------------------------
TPERMIL2PSX:
        dw TPERMILTD2PS
        dw TPERMILTD2PS
        dw TPERMILMO2PS
        dw TPERMILMZ2PS
;-----------------------------------------------------------------------------
BMI1:   dw 0
        dw TBLSR
        dw TBLSMSK
        dw TBLSI
        dw 0
        dw 0
        dw 0
        dw 0
;-----------------------------------------------------------------------------
T0F71N: dw 0            ;/0
        dw 0            ;/1
        dw TPSRLW       ;/2
        dw 0            ;/3
        dw TPSRAW       ;/4
        dw 0            ;/5
        dw TPSLLW       ;/6
        dw 0            ;/7
;-----------------------------------------------------------------------------
T0F72N: dw TPRORD       ;/0
        dw TPROLD       ;/1
        dw TPSRLD       ;/2
        dw 0            ;/3
        dw TPSRAD       ;/4
        dw 0            ;/5
        dw TPSLLD       ;/6
        dw 0            ;/7
;W1
        dw TPRORQ       ;/0
        dw TPROLQ       ;/1
        dw 0            ;/2
        dw 0            ;/3
        dw TPSRAQ       ;/4
        dw 0            ;/5
        dw 0            ;/6
        dw 0            ;/7
;-----------------------------------------------------------------------------
T0F73N: dw 0            ;/0
        dw 0            ;/1
        dw 0            ;/2
        dw TPSRLDQ      ;/3
        dw 0            ;/4
        dw 0            ;/5
        dw 0            ;/6
        dw TPSLLDQ      ;/7
;W1
        dw 0            ;/0
        dw 0            ;/1
        dw TPSRLQ       ;/2
        dw TPSRLDQ      ;/3
        dw 0            ;/4
        dw 0            ;/5
        dw TPSLLQ       ;/6
        dw TPSLLDQ      ;/7
;-----------------------------------------------------------------------------
T??C6N: dw 0                    ;/0
        dw TGATHERPF0DPS        ;/1
        dw TGATHERPF1DPS        ;/2
        dw 0                    ;/3
        dw 0                    ;/4
        dw TSCATTERPF0DPS       ;/5
        dw TSCATTERPF1DPS       ;/6
        dw 0                    ;/7
;W1
        dw 0                    ;/0
        dw TGATHERPF0DPD        ;/1
        dw TGATHERPF1DPD        ;/2
        dw 0                    ;/3
        dw 0                    ;/4
        dw TSCATTERPF0DPD       ;/5
        dw TSCATTERPF1DPD       ;/6
        dw 0                    ;/7
;-----------------------------------------------------------------------------
T??C7N: dw 0                    ;/0
        dw TGATHERPF0QPS        ;/1
        dw TGATHERPF1QPS        ;/2
        dw 0                    ;/3
        dw 0                    ;/4
        dw TSCATTERPF0QPS       ;/5
        dw TSCATTERPF1QPS       ;/6
        dw 0                    ;/7
;W1
        dw 0                    ;/0
        dw TGATHERPF0QPD        ;/1
        dw TGATHERPF1QPD        ;/2
        dw 0                    ;/3
        dw 0                    ;/4
        dw TSCATTERPF0QPD       ;/5
        dw TSCATTERPF1QPD       ;/6
        dw 0                    ;/7
;-----------------------------------------------------------------------------
C4C5Tab         dd C4C500,C4C566,C4C5F3,C4C5F2
C438Tab         dd C43800,C43866,C438F3,C438F2
C43ATab         dd C43A00,C43A66,C43AF3,C43AF2
;-----------------------------------------------------------------------------
T6200Tab        dd T620000,T620066,T6200F3,T6200F2
T6238Tab        dd T623800,T623866,T6238F3,T6238F2
T623ATab        dd T623A00,T623A66,T623AF3,T623AF2
;-----------------------------------------------------------------------------
if used StrLen
StrLen:
        push    rbx
        mov     edx,7
        add     rdx,rax
        mov     ebx,[rax]
        add     rax,4
.1:     lea     ecx,[rbx-01010101h]
        xor     ebx,-1
        and     ecx,ebx
        mov     ebx,[rax]
        add     rax,4
        and     ecx,80808080h
        jz      .1
        test    ecx,00008080h
        jnz     .2
        shr     ecx,16
        add     rax,2
.2:     shl     cl,1
        sbb     rax,rdx
        pop     rbx
        ret
end if

DisAsm:
        mov     [I.SaveRSP],rsp

        mov     rsi,[I.Addr]
        mov     rdx,[I.Size]
        mov     eax,MaxInstructionLength
        cmp     rdx,rax
        cmova   rdx,rax
        add     rdx,rsi
        mov     r9,rdx

        movzx   edx,[I.Arch]

        mov     al,[rdx+DefCPU2AD]
        mov     [I.Address],al
        mov     al,[rdx+DefCPU2OS]
        mov     [I.Operand],al

        mov     [I.Table],InstTab

        call    ClrMagicBytes

.NextByte:

        FetchB
        cmp     rsi,r9
        ja      ErrorDec
        mov     [LastByte],al

.NewTable:

        mov     ebx,[I.Table]

        movzx   eax,al
if bsf yyh.Size = bsr yyh.Size
        shl     eax,bsf yyh.Size
else
        imul    eax,yyh.Size
end if
        xor     ecx,ecx

        cmp     [I.Only],0      ;only one table?
        jnz     .SkipAdd
        cmp     [I.Operand],0
        jz      .SkipBBBB
        mov     ecx,1*yyh.Size
.SkipBBBB:
        lea     eax,[rax*3]
        cmp     [I.Arch],CPUX64
        jnz     .SkipX64
        mov     ecx,2*yyh.Size
        test    [I.PreREX],8    ;REX.W
        jnz     .SkipX64
        xor     ecx,ecx
        cmp     [I.Operand],0
        jz      .SkipAAA
        mov     ecx,1*yyh.Size
.SkipAAA:
        lea     edx,[rcx+rax]
        movzx   edx,word [rbx+rdx+yyh.Addr]
        add     edx,NextTab
        test    [rdx+xxh.Flags],_A
        jz      .SkipX64
        mov     ecx,2*yyh.Size
.SkipX64:
        add     eax,ecx
.SkipAdd:
        add     ebx,eax

        movzx   edx,word [rbx+yyh.Addr]
        add     edx,NextTab
        xchg    edx,ebx

        mov     [I.Item],ebx

        test    [rbx+xxh.Flags],_E+_T
        jnz     .SkipValue
        mov     al,[rbx+xxh.DefReg]
        mov     ah,al
        and     al,0xF
        mov     [I.Arg1.Reg],al
        shr     ah,4
        mov     [I.RandSize],ah
.SkipValue:
if 0
        test    [rbx+xxh.Flags+1],_D shr 8
        jz      .SkipDetect
        movzx   eax,[I.Operand]
        add     al,[I.W]
        mov     ax,word [rax*2+DefArgSize]
        mov     [I.Arg1.Type],al
        mov     [I.RandSize],ah
.SkipDetect:
end if
        mov     [I.SaveRSI],rsi

        or      eax,0xFFFFFFFF
        mov     ax,[rdx+yyh.Name]
        mov     dword [I.Name],eax

        movzx   eax,[rbx+xxh.Flags]
        mov     [I.Flags],ax

        test    al,_T
        jz      .SkipTableAddress
        mov     edx,[I.Item]
        movzx   edx,[rdx+xxh.NxtTab]
        add     edx,TTXXXX
.SkipTableAddress:

        movzx   eax,[rbx+xxh.Rtn]
        add     eax,RtnXX
        call    rax

        xor     cl,cl
        xchg    [I.NextByte],cl
        or      cl,cl
        jnz     .NextByte

        xor     cl,cl
        xchg    [I.NewTable],cl
        or      cl,cl
        jnz     .NewTable

        cmp     rsi,r9
        ja      ErrorDec

        cmp     [I.Mandatory66],0
        jnz     .IsMandatory66
        test    [I.PreREX],8    ;REX.W
        jz      .NoRXW
        BitT    I.Prefixes,P66
        jc      ErrorDec
.NoRXW:
.IsMandatory66:
if 1
        mov     al,[I.CurSeg]
        or      al,al
        jz      .SkipSegm
        cmp     al,[I.DefSeg]
        jnz     .SkipSegm
        BitTS   I.Prefixes,PSEG
        cmp     [I.PosSEG],0
        jz      ErrorDec
.SkipSegm:
end if
        call    SetArguments

        mov     al,[I.Arch]
        cmp     al,CPUX64
        jz      .SkipRand
        test    [I.Flags],_R
        jz      .SkipRand
        BitTR   I.Prefixes,P66
.SkipRand:
        cmp     al,CPUX64
        jnz     .SkipRand64
        test    [I.Flags],_R48
        jz      .SkipREX48
        mov     al,[I.NotW]
        and     byte [I.Prefixes],al
      .SkipREX48:
        test    [I.Flags],_R64
        jz      .SkipRand64
        BitTR   I.Prefixes,P66
.SkipRand64:

        BitT    I.Prefixes,PF0
        jnc     .SkipLOCK
        cmp     [I.PossibleLOCK],0
        jnz     .SkipLOCK
        cmp     [I.PrefixNames],TLOCK
        jz      ErrorDec
.SkipLOCK:

        BitT    I.Prefixes,P4X
        jnc     .Skip4X
        cmp     [I.PrefixNames],TREX??
        jz      ErrorDec
.Skip4X:

        BitT    I.Prefixes,P66
        jnc     .Skip66
        cmp     [I.PrefixNames],TRAND
        jz      ErrorDec
.Skip66:

        BitT    I.Prefixes,P67
        jnc     .Skip67
        cmp     [I.PrefixNames],TADDR
        jz      ErrorDec
.Skip67:

        BitT    I.Prefixes,P9B
        jnc     .SkipWAIT
        cmp     [I.PrefixNames],TWAIT
        jz      ErrorDec
.SkipWAIT:

        test    [I.Flags],_B
        jz      .SkipBND
        BitT    I.Prefixes,PF2
        jnc     .SkipBND
        movzx   eax,[I.PosF2]
        mov     [rax*2+I.PrefixNames],TBND
        mov     [I.PossibleF2F3],True
.SkipBND:

        test    [I.Flags],_XA+_XR
        jz      .SkipXAXR

        mov     al,00h
        test    [I.Flags],_XA
        jz      .SkipXA
        BitT    I.Prefixes,PF2
        setc    al
.SkipXA:
        mov     ah,00h
        test    [I.Flags],_XR
        jz      .SkipXR
        BitT    I.Prefixes,PF3
        setc    ah
.SkipXR:
        mov     cl,[I.PosF2]
        mov     dx,TXACQUIRE
        cmp     ax,0001h
        jz      .IsF2
        mov     cl,[I.PosF3]
        mov     dx,TXRELEASE
        cmp     ax,0100h
        jz      .IsF3
        cmp     ax,0101h
        jnz     .SkipXAXR
        mov     cl,[I.PosF2]
        mov     ch,[I.PosF3]
        mov     dx,TXACQUIRE
        cmp     cl,ch
        ja      .IsF2F3
        mov     cl,ch
        mov     dx,TXRELEASE
        jmp     .IsF2F3
.IsF2: .IsF3:
        mov     eax,[I.Prefixes]
        and     al,11b
        cmp     al,11b
        jnz     .IsF2F3
        mov     [I.PossibleF2F3],True
.IsF2F3:
        test    [I.Flags],_L
        jnz     .SkipTestF0
        BitT    I.Prefixes,PF0
        jnc     .SkipXAXR
.SkipTestF0:
        movzx   eax,cl
        mov     [rax*2+I.PrefixNames],dx
.SkipXAXR:

        BitT    I.Prefixes,PF2
        jnc     .SkipF2
        cmp     [I.PossibleF2F3],0
        jnz     .SkipF2
        cmp     [I.PrefixNames],TREPNE
        jz      ErrorDec
.SkipF2:

        BitT    I.Prefixes,PF3
        jnc     .SkipF3
        cmp     [I.PossibleF2F3],0
        jnz     .SkipF3
        cmp     [I.PrefixNames],TREP
        jz      ErrorDec
.SkipF3:

        cmp     [I.DisplayHint],0
        jz      .SkipHint
        test    [I.Flags],_J
        jz      .SkipHint
        cmp     [I.Arch],CPUX64
        jz      .SkipHint
        mov     al,[I.CurSeg]
        cmp     al,3Eh
        jz      .IsHint
        cmp     al,2Eh
        jz      .IsHint
        cmp     al,64h
        jnz     .SkipHint
      .IsHint:
        BitTR   I.Prefixes,PSEG
      .SkipHint:

        BitT    I.Prefixes,PSEG
        jnc     .SkipPSEG
        cmp     [I.PosSEG],0
        jz      ErrorDec
.SkipPSEG:

        mov     rax,rsi
        sub     rax,[I.Addr]
RtnXX:  ret

ErrorDec:
        mov     rsp,[I.SaveRSP]

        mov     eax,[I.PrefixesCpy]
        mov     [I.Prefixes],eax

        mov     [I.PrefixCount],1

        cmp     [I.Syntax],0
        jnz     .ForceDB
        cmp     [I.UseDB],0
        jnz     .ForceDB
        movzx   eax,[I.PrefixNames+0*2]
        or      eax,eax
        jnz     .Prefix
.ForceDB:

        mov     dword [I.Name],T?BYTE shl 16+TDB

        mov     [I.RandSize],0
        mov     [I.PrefixCount],0

        mov     rax,[I.Addr]
        mov     al,[rax]
        mov     [I.Arg1.ImmB],al
        mov     [I.Arg1.Type],80h+1
        jmp     .Continue

.Prefix:mov     [I.Name],TNULL
        mov     [I.Arg1.Type],0
.Continue:
        xor     eax,eax

        mov     [I.Arg2.Type],al
        mov     [I.Arg3.Type],al
        mov     [I.Arg4.Type],al
        mov     [I.Arg5.Type],al

        mov     [I.Arg1.Mem],al
        mov     [I.Arg2.Mem],al
        mov     [I.Arg3.Mem],al
        mov     [I.Arg4.Mem],al
        mov     [I.Arg5.Mem],al

        mov     [I.Prefix],al
        mov     [I.LastByte],al
        and     dword [I.Suffix],eax
        and     dword [I.Suffix2nd],eax

        mov     [I.AAAA],al
        mov     [I.Z],al

        mov     [I.Relative],al         ;* 23-06-2017
        mov     [I.IsAddress],al        ;* 23-06-2017

        mov     [I.RandSize],al
        mov     [I.Point],ax

        mov     eax,1
        ret

SetArguments:

        call    DetectMULT

        cmp     [I.VT],0
        jz      .L1
        cmp     [I.Indx],-1
        jnz     .L1
        mov     [I.Indx],4      ;fix for EVEX
.L1:
        lea     rdi,[I.Arg1]
        call    SetArgument
        lea     rdi,[I.Arg2]
        call    SetArgument
        lea     rdi,[I.Arg3]
        call    SetArgument
        lea     rdi,[I.Arg4]
        call    SetArgument
        lea     rdi,[I.Arg5]
SetArgument:
        movzx   eax,[rdi+TArg.Type2nd]
        and     al,7Fh
        mov     eax,[rax*4+SetArg]
        jmp     rax

SetArg  dd      .Exit,.RM,.Reg,.VVVV,.Exit

.RM:
        cmp     [I.Mod],3
        setnz   [rdi+TArg.Mem]

        mov     dl,[I.RM]

        mov     al,[rdi+TArg.Type]
        and     eax,0Fh
        bt      [TBIT],eax
        jnc     .XXXX

        add     dl,[I.B]
        add     dl,[I.XX]

        cmp     al,GPR08
        jnz     .NoB8
        call    ClearPRM
        jmp     .XXXX
.NoB8:  call    ClearBRM
        jmp     .XXXX

.Reg:   mov     dl,[I.Reg]

        mov     al,[rdi+TArg.Type]
        and     eax,0Fh
        bt      [TBIT],eax
        jnc     .XXXX

        add     dl,[I.R]

        cmp     al,GPR08
        jnz     .NoR8
        call    ClearP
        jmp     .XXXX
.NoR8:  call    ClearR
        jmp     .XXXX

.VVVV:  mov     dl,[I.VVVV]
.XXXX:  add     [rdi+TArg.Reg],dl

.Exit:  ret

ClearR: mov     al,[I.NotR]
        and     byte [I.Prefixes],al
        ret

ClearPRM:
        cmp     [I.Mod],3
        jnz     ClearP.NoClear
ClearP: cmp     dl,4
        jb      .NoClear
        mov     al,[I.NotP]
        and     byte [I.Prefixes],al
.NoClear:
        ret

ClearBRM:
        cmp     [I.Mod],3
        jnz     ClearB.NoClear
ClearB: mov     al,[I.NotB]
        and     byte [I.Prefixes],al
.NoClear:
        ret

ClearW: mov     al,[I.NotW]
        and     byte [I.Prefixes],al
        ret

MakeSpace:
        cmp     r10,0
        jnz     .SkipFill
        inc     r10
        push    rdx
        mov     rdx,rdi
        sub     rdx,r9
        mov     r9,rcx
        mov     ecx,[SpaceSize]
        sub     rcx,rdx
        pop     rdx
        ja      .DoFill
        mov     ecx,1
        cmp     byte [rdi-1],32
        jz      .ZeroFill
.DoFill:
        mov     al,32
        rep     stosb
.ZeroFill:
        mov     rcx,r9
.SkipFill:
        ret

ShowHint:
        cmp     [I.DisplayHint],0
        jz      .SkipHint
        test    [I.Flags],_J
        jz      .SkipHint
        cmp     [I.Arch],CPUX64
        jz      .SkipHint
        mov     si,TJ?
        cmp     [I.CurSeg],3Eh
        jz      .IsHint
        mov     si,TN?
        cmp     [I.CurSeg],2Eh
        jz      .IsHint
        mov     si,TA?
        cmp     [I.CurSeg],64h
        jnz     .SkipHint
      .IsHint:
        call    MoveStringData
      .SkipHint:
        ret

PrnAsm:
        mov     rdi,[TheBufferO]

        add     [DefInst.Addr],rax

        mov     rsi,rax
        mov     rax,[Origin]
        add     [Origin],rsi
        cmp     [I.NoAddress],0
        jnz     .SkipAddr

        cmp     [TheL],0
        jz      .SkipL
        mov     byte [rdi],'L'
        scasb
.SkipL:
if 0
        or      ecx,0xFFFFFFFF
        cmp     rax,rcx
        setbe   cl
        mov     ch,16
        shr     ch,cl
        mov     cl,ch
else
  if %B=32
        mov     cl,8
  else
        mov     cl,16
  end if
end if
if ~OS
        push    rdi
end if
        call    Hex
if ~OS
        push    [LastNames]
end if
        mov     word [rdi],': '
        scasw
.SkipAddr:
        mov     rax,rsi

        cmp     [I.ShowSize],0
        jz      .NoShowSize
        mov     byte [rdi],'('
        scasb
        mov     bl,10
        div     bl
        add     ax,'00'
        stosw
        mov     ax,') '
        stosw
        mov     rax,rsi
      .NoShowSize:

        cmp     [I.Medium],0
        jnz     .SkipCodeStr
        mov     edx,MaxInstructionLength+1
        sub     edx,eax
        jc      .SkipCodeStr
        mov     ecx,eax
        or      ecx,ecx
        jz      .Skip
        mov     r10,rcx
        mov     rsi,[I.Addr]
.Loop:
        lodsb
        mov     ecx,[rax*2+HexString]
        mov     [rdi],cx
        scasw

        dec     r10
        jnz     .Loop

        lea     ecx,[rdx*2+1]
        mov     al,32
        rep     stosb
.SkipCodeStr:
if ~OS
        pop     [LastNames]
        xchg    rdi,[rsp]
        mov     rsi,[LastNames]
        or      rsi,rsi
        jz      .SkipEmptyName
.LoopEmptyName:
        mov     al,[rsi]
        or      al,al
        jz      .LastEmptyName
        mov     [rdi],al
        inc     esi
        inc     edi
        jmp     .LoopEmptyName
.LastEmptyName:
if 1
        mov     [rdi],byte 32
        inc     edi
end if
.SkipEmptyName:
        pop     rdi
end if
        xor     edx,edx
        mov     r10,rdx                 ;no space
        movzx   ecx,[I.PrefixCount]
        jrcxz   .ZeroPrefixes
.LoopPrefixes:
        movzx   eax,[rdx*1+I.PrefixBites]
        mov     esi,dword [rdx*2+I.PrefixNames]

        cmp     [I.Syntax],0
        jz      .SkipSyn
        mov     ebx,dword [I.SegmName]
        cmp     al,PSEG
        jz      .MoveToESI
        mov     bx,TDATA
        cmp     al,P66
        jz      .MoveToESI
        mov     bx,TADDR
        cmp     al,P67
        jz      .MoveToESI
        cmp     al,PF0
        jz      .SkipSyn
        cmp     al,PF2
        jz      .SkipSyn
        cmp     al,PF3
        jz      .SkipSyn
        jmp     .SkipThisPrefix
      .MoveToESI:
        mov     esi,ebx
      .SkipSyn:

        cmp     al,P4X
        jnz     .SkipCheckREX
        cmp     [I.NoREX],0
        jnz     .SkipThisPrefix
      .SkipCheckREX:

        call    ShowHint

        bt      [I.Prefixes],eax
        jnc     .SkipThisPrefix
        mov     r9,rdi
        call    MoveStringData
        call    MakeSpace
      .SkipThisPrefix:
        inc     edx
        loop    .LoopPrefixes
.ZeroPrefixes:

        movzx   eax,[I.Syntax]
        mov     esi,dword [I.Name+rax*2]
        cmp     si,-1
        cmovz   esi,dword [I.Name]
        mov     r9,rdi

        mov     al,[I.Prefix]
        or      al,al
        jz      .SkipPrefix
        stosb
.SkipPrefix:
        call    MoveStringData

        mov     eax,dword [I.Suffix2nd]
        or      eax,eax
        jnz     .SyntaxSuffix
        mov     eax,dword [I.Suffix]
        cmp     [I.Syntax],0
        jz      .SyntaxSuffix
        test    [I.Flags],_S
        jz      .SyntaxSuffix
        movzx   eax,[I.RandSize]

        cmp     [I.IsFloat],0
        jz      .NoFloat
        and     al,7Fh
        mov     ah,al
        mov     al,'s'
        cmp     ah,VDWORD
        jz      .SyntaxSuffix
        mov     al,'l'
        cmp     ah,VQWORD
        jz      .SyntaxSuffix
        mov     al,'t'
        cmp     ah,VTWORD
        jz      .SyntaxSuffix
        xor     ah,ah
      .NoFloat:

        or      al,al
        jz      .SyntaxIntel
      .SelectPoint:
        cmp     al,VQWORD
        ja      .SyntaxIntel
        mov     al,[rax+Suffixes]
      .SyntaxSuffix:
        or      al,al
        jz      .SyntaxIntel
      .StoreChar:
        stosb
        shr     eax,8
        cmp     al,32
        jae     .StoreChar
.SyntaxIntel:

        mov     al,[I.LastByte]
        or      al,al
        jz      .NoModify
        mov     [rdi-1],al
.NoModify:

        push    r10
        call    MakeSpace
        pop     r10
        cmp     r10,0
        jz      .SkipSpace
        mov     al,32
        stosb
.SkipSpace:

        cmp     [I.Syntax],0
        jnz     .SkipPoint
        movzx   esi,[I.Point]
        or      esi,esi
        jz      .SkipPoint
        call    MoveStringData
.SkipPoint:

r8      equ     rbx

        cmp     [I.Syntax],0
        jnz     .SyntaxATT
        lea     r8,[I.Arg1]
        call    ParseArg
        call    MoveKandZ
        lea     r8,[I.Arg2]
        call    ParseArg
        lea     r8,[I.Arg3]
        call    ParseArg
        lea     r8,[I.Arg4]
        call    ParseArg
        lea     r8,[I.Arg5]
        call    ParseArg
        test    [I.MainFlags],00000001b
        jnz     .SkipSAE
        call    MoveSAE1
.SkipSAE:
        jmp     .Print

.SyntaxATT:
        call    MoveSAE2
        lea     r8,[I.Arg5]
        call    ParseArgATT
        lea     r8,[I.Arg4]
        call    ParseArgATT
        lea     r8,[I.Arg3]
        call    ParseArgATT
        lea     r8,[I.Arg2]
        call    ParseArgATT
        lea     r8,[I.Arg1]
        call    ParseArgATT
        call    MoveKandZ
.Print:

.ScanBack:
        dec     rdi
        cmp     byte [rdi],32
        jz      .ScanBack
        scasb

        lea     rsi,[CrLf]
        call    MoveStringDataRSI

        mov     byte [rdi],0
if 1
        mov     rax,rdi
        sub     rax,[TheBufferO]
else
        mov     rax,[TheBufferO]
        call    StrLen
end if
if 0
        cmp     [I.Compression],0
        jz      .NoComp
        Compress
      .NoComp:
        add     [TheBufferO],rax

        mov     rdi,[TheBufferO]
        cmp     rdi,Buffer+BufferSizeO-255
        jb      .Skip
        call    PrintBuffer
        mov     [TheBufferO],Buffer
end if
.Skip:
        ret
if 0
PrintBuffer:
        mov     edi,Buffer
        mov     rcx,[TheBufferO]
        sub     rcx,rdi
        jbe     .DoNotWrite
        mov     rdx,rdi
        call    WriteBlock
        jc      WriteErr
.DoNotWrite:
        ret
end if
MoveStringData:
        movzx   esi,si
        add     esi,Names
MoveStringDataRSI:
      @@:
        movsb
        cmp     byte [rdi-1],0
        jnz     @B
        dec     rdi
        ret

AddVT:
        cmp     [I.VT],0
        jz      .Old

        movzx   edx,[I.VT]
        mov     edx,[rdx*4+AD2Regs]
.Old:
        cmp     [I.VT],0
        jz      .TTTT
        add     al,[I.V]
.TTTT:
        cmp     [I.Arch],CPUX64
        jz      .VVVV
        and     al,00111b
.VVVV:
        ret

StoreSegment:
        cmp     [I.IsLEA],0
        jnz     .SkipSegm
        mov     al,[I.CurSeg]
        or      al,al
        jz      .SkipSegm
        cmp     al,[I.DefSeg]
        jz      .SkipSegm
        mov     si,[I.SegmName]
        cmp     si,TNULL
        jz      .SkipSegm

        cmp     [I.Syntax],0
        jz      .SkipPercent
        mov     al,'%'
        stosb
.SkipPercent:

        call    MoveStringData
        mov     al,':'
        stosb
.SkipSegm:
        ret

ParseArgATT:
        mov     [I.HexPrefix],True

        cmp     [r8+TArg.Mem],0
        jnz     .SkipType
        cmp     [r8+TArg.Type],0
        jz      ParseArg.SkipArg
.SkipType:

        cmp     [I.IsCALLJMP],0
        jz      .No
        mov     al,'*'
        stosb
.No:
        mov     rax,r8
        xchg    [I.LastArg],rax
        or      rax,rax
        jz      .Arg1st
        mov     al,','
        stosb
.Arg1st:

        cmp     [r8+TArg.Type2nd],Is1
        jz      .Put1
        test    [r8+TArg.Type],10h
        jnz     .PtrATT
        test    [r8+TArg.Type],80h
        jnz     ParseArg.ImmATT
        cmp     [r8+TArg.Mem],0
        jz      .Reg
.Mem:
        call    StoreSegment

        mov     cl,[I.MULT]
        cmp     [I.DispSize],1
        jnz     .NoMULT
        mov     ch,byte [I.Disp+1]
        sal     [I.Disp],cl
        cmp     ch,byte [I.Disp+1]
        jz      .NoMULT
        inc     [I.DispSize]
.NoMULT:

        mov     al,[I.DispSize]
        or      al,al
        jz      .SkipDisp

        mov     rax,[I.Disp]
if %B=32
        mov     edx,[I.Disp+4]
        test    [I.MainFlags],80h
        jnz     .SkipCDQ
        cmp     [I.DispSize],4
        ja      .SkipCDQ
        cdq
.SkipCDQ:
end if
        cmp     byte [rdi-1],'['
        jz      .SkipNegate
        cmp     [I.Negate],0
        jz      .SkipNegate
if %B=64
        or      rax,rax
else
        cmp     [I.Arch],CPUX64
        jz      .Arch64
.Arch64:or      eax,eax
        jmp     .ArchXX
        or      edx,edx
.ArchXX:
end if
        jns     .SkipNegate
        mov     byte [rdi],'-'
        scasb
.NoPlus:
        neg     rax
if %B=32
        adc     edx,0
        neg     edx
end if
.SkipNegate:

        mov     cl,[I.DispSize]
        add     cl,cl
if %B=32
        cmp     cl,8
        jbe     .SkipHigh
        push    eax
        mov     eax,edx
        mov     cl,8
        call    Bin2Hex32
        pop     eax
        mov     cl,8
        call    Hex
        jmp     .SkipDisp
.SkipHigh:
end if
        call    Bin2Hex
.SkipDisp:

        cmp     [I.Relative],0
        jnz     .SkipCheck
        mov     al,[I.Base]
        and     al,[I.Indx]
        cmp     al,-1
        jz      .SkipBaseIndx
.SkipCheck:

        mov     al,'('
        stosb

        movzx   eax,[I.Address]
        mov     edx,[rax*4+AD2Regs]

        cmp     [I.Relative],0
        jz      .SkipRels
        mov     byte [rdi],'%'
        scasb
        mov     esi,[16*2+rdx]
        call    MoveStringData
        dec     rdi
.SkipRels:

        mov     al,[I.Base]
        cmp     al,-1
        jz      .SkipBase

        call    AddVT.TTTT

        mov     byte [rdi],'%'
        scasb
        mov     esi,[rax*2+rdx]
        call    MoveStringData
.SkipBase:

        mov     al,[I.Indx]
        cmp     al,-1
        jz      .SkipIndx

        call    AddVT

        mov     word [rdi],',%'
        scasw
        mov     esi,[rax*2+rdx]
        call    MoveStringData
        mov     al,','
        stosb
        mov     al,[I.Scale]
        cmp     al,-1
        jz      .SkipScale
        mov     al,[rax+Scales]
        stosb
.SkipScale:

.SkipIndx:
        mov     al,')'
        stosb

.SkipBaseIndx:

        call    MoveXtoX

        ret
.Reg:
        mov     al,'%'
        stosb
        jmp     ParseArg.Reg

.PtrATT:
        mov     ax,'(%'
        stosw

        movzx   eax,[I.Address]
        mov     edx,[rax*4+AD2Regs]

        mov     al,[r8+TArg.Reg]
        mov     esi,dword [rax*2+rdx]
        call    MoveStringData

        jmp     .SkipIndx

ParseArg.SkipArg:
        ret

ParseArgATT.Put1:
        mov     al,'$'
        stosb
ParseArg.Put1:
        mov     al,'1'
        stosb
        ret

ParseArg:
        test    [r8+TArg.Type],80h
        jz      .NoIMM
        or      [I.MainFlags],00000001b
        call    MoveSAE1
.NoIMM:
        cmp     [r8+TArg.Mem],0
        jnz     .SkipType
        cmp     [r8+TArg.Type],0
        jz      .SkipArg
.SkipType:
        lea     rax,[I.Arg1]
        cmp     r8,rax
        jz      .Arg1st
        test    [r8+TArg.Type2nd],80h
        jz      .NoPlusReg
        mov     ax,'+3'
        stosw
.NoPlusReg:
        mov     al,','
        cmp     [I.MemSeparator],0
        jz      .NoSeparate
        mov     al,':'
     .NoSeparate:
        stosb
.Arg1st:

        cmp     [r8+TArg.Type2nd],Is1
        jz      .Put1
        test    [r8+TArg.Type],10h
        jnz     .Ptr
        test    [r8+TArg.Type],80h
        jnz     .Imm

        cmp     [r8+TArg.Mem],0
        jz      .Reg
.Mem:
        movzx   eax,[I.Address]
        mov     edx,[rax*4+AD2Regs]

        movzx   eax,[I.RandSize]

        cmp     [I.NoShowRand],0
        jnz     .SkipRand
        cmp     [I.ShowRand],0
        jnz     .DoShowIt
        test    [I.Flags],_O
        jnz     .SkipRand
        bt      eax,7
        jc      .SkipRand
.DoShowIt:
        and     al,7Fh
        mov     esi,dword [rax*2+SZ2Name]
        call    MoveStringData
.SkipRand:

        cmp     [I.Fasm],0
        jnz     .SkipSegm
        call    StoreSegment
.SkipSegm:

        mov     al,'['
        stosb

        cmp     [I.Fasm],0
        jz      .SegmSkip
        call    StoreSegment
.SegmSkip:

        cmp     [I.Fasm],0
        jz      .SkipSizeOvr
        BitT    I.PrefixesCpy,P67
        jnc     .NoAddress
        cmp     [I.Relative],0
        jnz     .NoAddress
        mov     al,[I.Base]
        and     al,[I.Indx]
        cmp     al,-1
        jnz     .NoAddress
        movzx   eax,[I.DispSize]
        bsf     eax,eax
        mov     esi,dword [rax*2+SZ2Mems]
        call    MoveStringData
        jmp     .SkipSizeOvr
.NoAddress:
        movzx   eax,[I.DispSize]
        or      al,al
        jz      .SkipSizeOvr
        mov     rcx,[I.Disp]
        bsf     eax,eax
        cmp     rcx,[rax*_8_+SZ2Mask]
        ja      .SkipSizeOvr
if %B=32
        cmp     eax,3
        jnz     .SkipSizeQWs
        cmp     [I.Disp+4],0
        ja      .SkipSizeOvr
.SkipSizeQWs:
end if
        mov     esi,dword [rax*2+SZ2Mems]
        call    MoveStringData
.SkipSizeOvr:

        mov     al,[I.Base]
        cmp     al,-1
        jz      .SkipBase
        call    AddVT.TTTT
        mov     esi,[rax*2+rdx]
        call    MoveStringData
.SkipBase:
        mov     al,[I.Indx]
        cmp     al,-1
        jz      .SkipIndx

        cmp     [I.Base],-1
        jz      .Skip1
        mov     byte [rdi],'+'
        scasb
     .Skip1:

        call    AddVT

        mov     esi,[rax*2+rdx]
        call    MoveStringData
.SkipIndx:
        mov     al,[I.Indx]
        cmp     al,-1
        jz      .SkipScale
        mov     al,[I.Scale]
        cmp     al,-1
        jz      .SkipScale
        cmp     [I.ShowScale],0
        jnz     .DoShowScale
        or      al,al
        jz      .SkipScale
.DoShowScale:
        mov     ah,[rax+Scales]
        mov     al,'*'
        stosw
.SkipScale:

        mov     cl,[I.MULT]
        cmp     [I.DispSize],1
        jnz     .NoMULT
        mov     ch,byte [I.Disp+1]
        sal     [I.Disp],cl
        cmp     ch,byte [I.Disp+1]
        jz      .NoMULT
        inc     [I.DispSize]
.NoMULT:

        mov     al,[I.DispSize]
        or      al,al
        jz      .SkipDisp

        mov     al,[I.Base]
        and     al,[I.Indx]
        cmp     al,-1
        jz      .Skip2
        mov     byte [rdi],'+'
        scasb
     .Skip2:

        cmp     [I.Relative],0
        jz      .SkipRels
        cmp     [I.HideRIP],0
        jnz     .HideRels
        mov     esi,[16*2+rdx]
        call    MoveStringData
        jmp     .SkipRels
.HideRels:
        mov     [I.DispSize],8
        mov     rax,[I.Disp]
        add     rax,[Origin]
if %B=32
        mov     edx,[I.Disp+4]
        adc     edx,0
end if
        cmp     [I.Address],AD64
        jz      .SkipCut
        mov     eax,eax
        mov     [I.DispSize],4
     .SkipCut:
        mov     [I.Disp],rax
if %B=32
        mov     [I.Disp+4],edx
end if
.SkipRels:
        mov     rax,[I.Disp]
if %B=32
        mov     edx,[I.Disp+4]
        test    [I.MainFlags],80h
        jnz     .SkipCDQ
        cmp     [I.DispSize],4
        ja      .SkipCDQ
        cdq
.SkipCDQ:
end if
        cmp     [TheL],0
        jz      .SkipL
        cmp     [I.DispSize],4
        jb      .SkipL
        cmp     byte [rdi-1],'+'
        jz      .SkipL
        mov     byte [rdi],'L'
        scasb
        jmp     .SkipNegate
.SkipL:

        cmp     byte [rdi-1],'['
        jz      .SkipNegate
        cmp     [I.Negate],0
        jz      .SkipNegate
        cmp     [I.DispSize],8
        jz      .SkipNegate
if %B=64
        or      rax,rax
else
        cmp     [I.Arch],CPUX64
        jz      .Arch64
.Arch64:or      eax,eax
        jmp     .ArchXX
        or      edx,edx
.ArchXX:
end if
        jns     .SkipNegate
        cmp     byte [rdi-1],'+'
        jnz     .NoPlus
        mov     byte [rdi-1],'-'
.NoPlus:
        neg     rax
if %B=32
        adc     edx,0
        neg     edx
end if
.SkipNegate:

        mov     cl,[I.DispSize]
        add     cl,cl
if %B=32
        cmp     cl,8
        jbe     .SkipHigh
        push    eax
        mov     eax,edx
        mov     cl,8
        call    Bin2Hex32
        pop     eax
        mov     cl,8
        call    Hex
        jmp     .SkipDisp
.SkipHigh:
end if
        call    Bin2Hex
.SkipDisp:

        mov     al,']'
        stosb

        call    MoveXtoX

        ret

.Ptr:
        test    [r8+TArg.Type],20h
        jz      .RandSkip
        movzx   eax,[I.RandSize]
        mov     esi,dword [rax*2+SZ2Name]
        call    MoveStringData
     .RandSkip:

        mov     al,'['
        stosb

        movzx   eax,[I.Address]
        mov     edx,[rax*4+AD2Regs]

        mov     al,[r8+TArg.Reg]
        mov     esi,dword [rax*2+rdx]
        call    MoveStringData
.SkipAddr:

        jmp     .SkipDisp

.Reg:
        movzx   eax,[r8+TArg.Type]
        mov     edx,[(rax-1)*4+SZ2Regs]

        cmp     al,GPR08
        jnz     .SkipByte
        cmp     [I.P],0
        jz      .SkipByte
        mov     edx,[IntelName]
.SkipByte:

        movzx   eax,[r8+TArg.Reg]
        mov     esi,[rax*2+rdx]
        call    MoveStringData
        ret

.ImmATT:
        mov     [I.Dollar],0
        cmp     [I.IsAddress],0
        jnz     .ImmContinue
        mov     byte [rdi],'$'
        scasb
        jmp     .ImmContinue
.Imm:
        cmp     [I.IsShort],0
        jz      .NoShort
        mov     si,TSHORT
        call    MoveStringData
      .NoShort:

        mov     cl,[r8+TArg.Type]
        test    cl,20h
        jz      .ImmContinue
        and     ecx,0Fh
        bsf     ecx,ecx
        mov     esi,dword [(rcx+1)*2+SZ2Name]
        call    MoveStringData
.ImmContinue:
        mov     rax,[r8+TArg.Imm]
if %B=32
        mov     edx,[r8+TArg.Imm+4]
        test    [r8+TArg.Type],8
        jnz     .SkipImmCDQ
        cdq
.SkipImmCDQ:
end if
        mov     cl,[r8+TArg.Type]
        test    cl,40h
        jz      .SkipNeg
if %B=64
        or      rax,rax
else
        or      edx,edx
end if
        jns     .SkipNeg
        mov     byte [rdi],'-'
        scasb
        neg     rax
if %B=32
        adc     edx,0
        neg     edx
end if
.SkipNeg:
        cmp     [I.IsAddress],0
if %B=32
        jnz     .Hex32
else
        jnz     .Hex
end if
        and     cl,0Fh
        add     cl,cl
if %B=32
        cmp     cl,8
        jbe     .SkipHigh32
        push    eax
        mov     eax,edx
        mov     cl,8
        call    Bin2Hex32
        pop     eax
        mov     cl,8
        call    Hex
        jmp     .SkipDisp32
.SkipHigh32:
end if
        call    Bin2Hex
.SkipDisp32:
        ret
if %B=32
.Hex32: or      edx,edx
        jz      .Hex
        push    eax
        mov     eax,edx
        call    .Hex
        pop     eax
        mov     cl,8
        jmp     Hex
end if
.Hex:   mov     cl,16
        mov     edx,-1
        cmp     rax,rdx
        ja      .DoHex
        mov     cl,8
.DoHex:
        cmp     [I.HexPrefix],0
        je      .L1
        mov     word [rdi],'0x'
        scasw
if 1
        cmp     [I.IsAddress],0
        jz      .L2
        cmp     [TheL],0
        jz      .L2
        dec     rdi
        mov     byte [rdi-1],'L'
end if
        jmp     .L2
.L1:
        cmp     [I.Dollar],0
        jne     .L2
        mov     byte [rdi],'$'
        scasb
if 1
        cmp     [I.IsAddress],0
        jz      .L2
        cmp     [TheL],0
        jz      .L2
        mov     byte [rdi-1],'L'
end if
.L2:
        call    Hex
        ret

PrefixErrorDec:
        cmp     [I.PosSEG],0
        jz      ErrorDec
        mov     [I.SegmName],TNULL      ;ignore this prefix
        ret

Rtn6465:
        movzx   eax,byte [rsi-1]
        mov     [I.CurSeg],al

        mov     dx,[(rax-60h)*2+NSEGR]
        mov     [I.SegmName],dx
        mov     dx,[(rax-60h)*2+NSEGX]
        jmp     Rtn262E363E.C6464

Rtn262E363E64:
        push    PrefixErrorDec
Rtn262E363E:
        movzx   eax,byte [rsi-1]
        mov     [I.CurSeg],al

        shr     al,3
        mov     dx,[(rax-4)*2+NSEGR]
        mov     [I.SegmName],dx
        mov     dx,[(rax-4)*2+NSEGX]
.C6464:
        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],dx
        mov     [rax+I.PrefixBites],PSEG
        inc     [I.PrefixCount]

        BitTS   I.PrefixesCpy,PSEG
        BitTS   I.Prefixes,PSEG
        jnc     @F
        cmp     [I.PosSEG],0
        jz      ErrorDec
      @@:
        mov     [I.PosSEG],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret

Rtn4X:
        mov     al,[rsi-1]
        mov     [I.PreREX],al

        call    SetMagicBytes

        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TREX??
        mov     [rax+I.PrefixBites],P4X
        inc     [I.PrefixCount]
        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,P4X
        BitTS   I.Prefixes,P4X
        jnc     @F
        cmp     [I.Pos4X],0
        jz      ErrorDec
      @@:
        mov     [I.Pos4X],al

        mov     al,[rsi-1]
        mov     ax,[rax*2+HexString]
        mov     word [Names+TREX??+3],ax

        mov     [I.NextByte],1
        ret
Rtn66:
        movzx   eax,[I.Arch]
        mov     al,[rax+XorCPU2OS]
        mov     [I.Operand],al

        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TRAND
        mov     [rax+I.PrefixBites],P66
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,P66
        BitTS   I.PrefixByte,P66
        BitTS   I.Prefixes,P66
        jnc     @F
        cmp     [I.Pos66],0
        jz      ErrorDec
      @@:
        mov     [I.Pos66],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret
Rtn67:
        movzx   eax,[I.Arch]
        mov     al,[rax+XorCPU2AD]
        mov     [I.Address],al

        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TADDR
        mov     [rax+I.PrefixBites],P67
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,P67
        BitTS   I.Prefixes,P67
        jnc     @F
        cmp     [I.Pos67],0
        jz      ErrorDec
      @@:
        mov     [I.Pos67],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret
Rtn9B:
        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TWAIT
        mov     [rax+I.PrefixBites],P9B
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,P9B
        BitTS   I.Prefixes,P9B
        jnc     @F
        cmp     [I.Pos9B],0
        jz      ErrorDec
      @@:
        mov     [I.Pos9B],al

        mov     [I.NextByte],1
        ret

RtnF0:
        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TLOCK
        mov     [rax+I.PrefixBites],PF0
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,PF0
        BitTS   I.Prefixes,PF0
        jnc     @F
        cmp     [I.PosF0],0
        jz      ErrorDec
      @@:
        mov     [I.PosF0],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret
RtnF2:
        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TREPNE
        mov     [rax+I.PrefixBites],PF2
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,PF2
        BitTS   I.PrefixByte,PF2
        BitTS   I.Prefixes,PF2
        jnc     @F
        cmp     [I.PosF2],0
        jz      ErrorDec
      @@:
        mov     [I.PosF2],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret
RtnF3:
        movzx   eax,[I.PrefixCount]
        mov     [rax*2+I.PrefixNames],TREP
        mov     [rax+I.PrefixBites],PF3
        inc     [I.PrefixCount]

        mov     [I.Name],TNULL

        BitTS   I.PrefixesCpy,PF3
        BitTS   I.PrefixByte,PF3
        BitTS   I.Prefixes,PF3
        jnc     @F
        cmp     [I.PosF3],0
        jz      ErrorDec
      @@:
        mov     [I.PosF3],al

        call    ClrMagicBytes

        mov     [I.NextByte],1
        ret

RtnF1:  cmp     [I.Syntax],0
        jz      .2
        mov     [I.Name],TICEBP
.2:
        test    [I.Sensitive],UND
        jz      .1
        mov     [I.Name],TSMI
.1:     ret

r8b     equ     cl
r8      equ     rcx

Rtn8F:  mov     al,[rsi]
        and     al,38h
        shr     al,3
        jz      EXTINST

        cmp     [I.PreREX],0
        jnz     ErrorDec

        FetchB
        mov     ah,al
       ;and     al,11100000b    ;XOP.RXB
        xor     al,11100000b
        shr     al,5
        mov     [I.PreREX],al

        call    SetMagicBytes

        mov     [I.P],1

        mov     al,ah
        and     al,11111b
        mov     edx,StrTabXOP8
        cmp     al,8
        jb      ErrorDec
        je      .Z1
        mov     edx,StrTabXOP9
        cmp     al,9
        je      .Z1
        mov     edx,StrTabXOPA
        cmp     al,0Ah
        ja      ErrorDec
.Z1:    mov     [I.MMMMM],al

        FetchB
        mov     ah,al
        and     al,01111000b
        xor     al,01111000b
        shr     al,3
        mov     [I.VVVV],al

        mov     al,ah
        test    al,11b
        jnz     ErrorDec

        and     al,10000100b    ;Mask XOP.W & XOP.L
        mov     [I.XOP],al

        FetchB
        mov     [I.SaveRSI],rsi

.Z2:    cmp     dword [rdx],-1
        je      ErrorDec
        cmp     al,[rdx+XOP.Imm]
        jz      .Z3
        add     edx,XOP.Size
        jmp     .Z2
.Z3:
        mov     [I.Prefix],'v'

        mov     al,[rdx+XOP.Flags]
        mov     [I.FlagsXOP],al
        test    al,XOPV
        jz      .Z4
        cmp     [I.VVVV],0
        jnz     ErrorDec
.Z4:
        and     al,XOPW+XOPL
        test    [I.XOP],al
        jnz     ErrorDec

        mov     r8b,XMMRG shl 4+XMMRG

        mov     eax,dword [rdx+XOP.MOS]
        test    [I.XOP],XOPL
        jz      .Z5
        mov     r8b,YMMRG shl 4+YMMRG
        shr     eax,8
.Z5:    mov     [I.RandSize],al

        movzx   ebx,[rdx+XOP.Rtn]
        add     ebx,RtnXX

        mov     edx,dword [rdx+XOP.Name]
        mov     [I.Name],dx

        push    r8
        call    .Arg
        call    rbx
        pop     r8
.Arg:
        test    [I.FlagsXOP],XOP2
        jz      .Z6
        and     r8b,0x0F
        or      r8b,XMMRG shl 4
.Z6:
        test    [I.FlagsXOP],XOP1
        jz      .Z7
        and     r8b,0xF0
        or      r8b,XMMRG
.Z7:
        mov     al,r8b
        and     al,0Fh
        mov     [I.Arg1.Type],al
        shr     r8b,4
        mov     [I.Arg2.Type],r8b
        ret
;-----------------------------------------------------------------------------
XOP3Rtn:call    FetchModRM
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg3.Type],XMMRG
        test    [I.XOP],XOPW
        jz      .L1
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg3.Mem]
        mov     [I.Arg2.Type2nd],InVVVV
        mov     [I.Arg3.Type2nd],InRM
        ret
.L1:   ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg3.Type2nd],InVVVV
        mov     [I.Arg2.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
;encoded bh,vvvv,bl,imm7:4 / bh,vvvv,bl,imm7:4 - W0 / W1
;-----------------------------------------------------------------------------
XOP4Rtn:
        call    FetchModRM
        mov     [I.Arg1.Type2nd],InReg
        mov     [I.Arg2.Type2nd],InVVVV
        mov     al,[I.Arg1.Type]
        mov     [I.Arg3.Type],al
        mov     [I.Arg4.Type],al

        test    [I.XOP],XOPW
        jnz     .L1

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg3.Mem]
        mov     [I.Arg3.Type2nd],InRM
        test    [I.FlagsXOP],XOPI
        jnz     .I1
        ret
.I1:
        FetchB
        shr     al,4
        mov     [I.Arg4.Reg],al
        ret
.L1:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg4.Mem]
        mov     [I.Arg4.Type2nd],InRM
        test    [I.FlagsXOP],XOPI
        jnz     .I2
        ret
.I2:
        FetchB
        shr     al,4
        mov     [I.Arg3.Reg],al
        ret
;-----------------------------------------------------------------------------
XOPImm8:call    RtnXOP
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
;-----------------------------------------------------------------------------
SuffixCOMUQ:
        mov     bx,'uq'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMUD:
        mov     bx,'ud'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMUW:
        mov     bx,'uw'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMUB:
        mov     bx,'ub'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMQ:
        mov     bx,'q'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMD:
        mov     bx,'d'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMW:
        mov     bx,'w'
        jmp     SuffixCOMB.1
;-----------------------------------------------------------------------------
SuffixCOMB:
        mov     bx,'b'
.1:
        mov     word [I.Suffix2nd],bx
        call    XOP4Rtn
        xor     eax,eax
        FetchB
        cmp     al,7
        jbe     .L1
.L0:    mov     [I.Arg4.Type],80h+1
        mov     [I.Arg4.ImmB],al
        ret
.L1:    mov     eax,[rax*2+TablePCOM]
        mov     [I.Name],ax
        mov     [I.Arg4.Type],0 ;delete
        ret
;-----------------------------------------------------------------------------
RtnTBM0:
        add     rsp,2*_8_       ;remove r8 & return

        mov     [I.RandSize],VDWORD+80h
        mov     ebx,R32RM32
        test    [I.XOP],XOPW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jne     .L1
        mov     [I.RandSize],VQWORD+80h
        mov     ebx,R64RM64
.L1:
        cmp     [I.Arch],CPUX64
        je      .L2
        mov     [I.B],0
        mov     [I.R],0
        mov     [I.X],0
.L2:
        mov     [I.Prefix],0
        mov     [I.Name],TBEXTR
        call    rbx
        FetchD
        mov     [I.Arg3.Type],80h+4
        mov     [I.Arg3.Imm],rax
        ret
;-----------------------------------------------------------------------------
RtnLWPCB:
        add     rsp,2*_8_       ;remove r8 & return

        call    FetchModRM
        cmp     [I.Mod],3
        jne     ErrorDec
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg1.Type],GPR32
        test    [I.XOP],XOPW
        jz      .1
        mov     [I.Arg1.Type],GPR64
.1:
        mov     dx,TSLWPCB
        cmp     [I.Reg],1
        je      .2
        ja      ErrorDec
        mov     dx,TLLWPCB
.2:     mov     [I.Name],dx
        mov     [I.Arg2.Type],0 ;delete
        mov     [I.Prefix],0
        ret
;-----------------------------------------------------------------------------
RtnLWPxx:
        add     rsp,2*_8_       ;remove r8 & return

        call    FetchModRM

        mov     [I.RandSize],VDWORD+80h

        mov     dx,TLWPVAL
        cmp     [I.Reg],1
        je      .2
        ja      ErrorDec
        mov     dx,TLWPINS
.2:     mov     [I.Name],dx
        mov     [I.Prefix],0

        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg2.Type],GPR32
        test    [I.XOP],XOPW
        jz      .1
        mov     [I.Arg1.Type],GPR64
.1:
        mov     [I.Arg1.Type2nd],InVVVV
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type2nd],InRM

        FetchD
        mov     [I.Arg3.Type],80h+4
        mov     [I.Arg3.Imm],rax
        ret
;-----------------------------------------------------------------------------
r8d     equ     edi
r8      equ     rdi
RtnTBM2:mov     r15d,TBM2Table
        jmp     RtnTBM1.L0
RtnTBM1:mov     r15d,TBM1Table
.L0:
        add     rsp,2*_8_       ;remove r8 & return

        mov     [I.RandSize],VDWORD+80h
        mov     r8d,R32RM32
        test    [I.XOP],XOPW
        jz      .L1
        cmp     [I.Arch],CPUX64
        jne     .L1
        mov     [I.RandSize],VQWORD+80h
        mov     r8d,R64RM64
.L1:
        cmp     [I.Arch],CPUX64
        je      .L2
        mov     [I.B],0
        mov     [I.R],0
        mov     [I.X],0
        and     [I.VVVV],111b
.L2:
        call    FetchModRM
        mov     rsi,[I.SaveRSI]

        movzx   eax,[I.Reg]
if %B=32
        mov     rdx,r15
        mov     edx,[rax*2+rdx]
else
        mov     edx,[rax*2+r15]
end if
        or      dx,dx
        jz      ErrorDec
        mov     [I.Prefix],0
        mov     [I.Name],dx

        call    r8
        mov     [I.Arg1.Type2nd],InVVVV
        ret
;-----------------------------------------------------------------------------
RtnC6:  mov     al,[rsi]
        and     al,38h
        shr     al,3
        jz      EXTINSONE
        FetchB
        cmp     al,0xF8
        jnz     ErrorDec
        mov     [I.Name],TXABORT
        FetchB
        mov     [I.Arg1.Type],80h+1
        mov     [I.Arg1.Imm],rax
        ret

RtnC7:  mov     al,[rsi]
        and     al,38h
        shr     al,3
        jz      EXTINST
        FetchB
        cmp     al,0xF8
        jnz     ErrorDec
        BitTR   I.Prefixes,P66
        mov     [I.Name],TXBEGIN
DODISPWD:
        cmp     [I.Operand],0
        jnz     DODISPD
        jmp     DODISPW

EXTINSTFF:
        mov     al,[rsi]
        and     al,38h
        shr     al,3
        ;
        cmp     al,2
        jb      EXTINST
        cmp     al,5
        ja      EXTINST
        ;
        mov     [I.IsCALLJMP],True
        jmp     EXTINST

EXTINSONEFLT:
        mov     [I.IsFloat],1
EXTINSONE:
        mov     [I.Only],1
EXTINST:
        mov     eax,[I.Item]
        movzx   eax,[rax+xxh.NxtTab]
        add     eax,EXT80
        mov     [I.Table],eax

        mov     al,[rsi]
        and     al,38h
        shr     al,3
        mov     [I.NewTable],1
        ret

Rtn0F:  movzx   eax,byte [rsi]
        shr     al,4                    ;Lose low nibble.

        bt      [EXTBITS],eax
        setc    [I.Only]

        mov     eax,[rax*4+EXT0F]       ;Get new table address.
        mov     [I.Table],eax

        FetchB
        and     al,0Fh                  ;Make sure only lower nibble counts.
        mov     [I.NewTable],1
        ret

Rtn9X16:
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg2.Type],GPR16
        jmp     Rtn9XXX
Rtn9X32:
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg2.Type],GPR32
        jmp     Rtn9XXX
Rtn9X64:
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],GPR64
Rtn9XXX:
        mov     al,[rsi-1]
        and     al,0Fh
        jnz     .No90

        cmp     [I.PrefixByte],bit PF3
        jz      .IsPause

        cmp     [I.PrefixByte],0
        jnz     .No90
        cmp     [I.B],0
        jnz     .No90
        mov     [I.Name],TNOP
        jmp     .PauseNOP
.IsPause:
        BitTR   I.Prefixes,PF3
        mov     [I.Name],TPAUSE
.PauseNOP:
        and     [I.Flags],not _S
        mov     [I.Arg1.Type],0
        mov     [I.Arg2.Type],0
        ret

.No90:
        add     al,[I.B]
        mov     [I.Arg1.Reg],al
        call    ClearB
        ret

Rtn9864:mov     [I.AltName],TCLTQ
        ret
Rtn9816:mov     [I.AltName],TCBTW
        ret
Rtn9832:mov     [I.AltName],TCWTL
        ret

Rtn9964:mov     [I.AltName],TCQTO
        ret
Rtn9916:mov     [I.AltName],TCWTD
        ret
Rtn9932:mov     [I.AltName],TCLTD
        ret

RtnEA:  mov     [I.AltName],TLJMP
        jmp     RtnXA
Rtn9A:  mov     [I.AltName],TLCALL
RtnXA:
r10     equ     rbx
        mov     [I.MemSeparator],True
        lea     r10,[I.Arg1]
        lea     rax,[I.Arg2]
        mov     r8,rax
        xor     r8,r10

        cmp     [I.Syntax],0
        jnz     .ATT
        xor     r10,r8
.ATT:
        mov     al,[I.Operand]
        or      al,al
        jz      .W
        jmp     .D
.W:     mov     [I.RandSize],VWORD
        mov     [I.Point],TDWORD
        FetchW
        movzx   eax,ax
        mov     [r10+TArg.Type],80h+2
        jmp     .X
.D:     mov     [I.RandSize],VDWORD
        mov     [I.Point],TFWORD
        FetchD
        mov     [r10+TArg.Type],80h+4
.X:     mov     [r10+TArg.Imm],rax
        FetchW
        movzx   eax,ax
        xor     r10,r8
        mov     [r10+TArg.Type],80h+2
        mov     [r10+TArg.Imm],rax
        ret

RtnF16: mov     [I.Suffix],'w'
        ret

RtnF32: mov     [I.Suffix],'d'
        ret

RtnF64: mov     [I.Suffix],'q'
        ret

RtnA0:  mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      RtnA016
        cmp     al,CPUX32
        jz      RtnA032
        jmp     RtnA064

RtnA016:
        mov     [I.Arg1.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA032:
        mov     [I.Arg1.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .W
        mov     [I.DispSize],4
        FetchUD
        jmp     .X
.W:     mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
.X:
        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA064:
        mov     [I.Arg1.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],8
        FetchQ
if %B=32
        mov     [I.Disp+4],edx
end if
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA1:  mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      RtnA116
        cmp     al,CPUX32
        jz      RtnA132
        jmp     RtnA164

RtnA116:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jnc     .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA132:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jc      .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .W
        mov     [I.DispSize],4
        FetchUD
        jmp     .X
.W:     mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
.X:
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA164:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jc      .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        cmp     [I.W],0
        jz      .XX
        mov     al,GPR64
        mov     [I.RandSize],VQWORD
.XX:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],8
        FetchQ
if %B=32
        mov     [I.Disp+4],edx
end if
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg2.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA2:  mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      RtnA216
        cmp     al,CPUX32
        jz      RtnA232
        jmp     RtnA264
        ret

RtnA216:
        mov     [I.Arg2.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA232:
        mov     [I.Arg2.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .W
        mov     [I.DispSize],4
        FetchUD
        jmp     .X
.W:     mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
.X:
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA264:
        mov     [I.Arg2.Type],GPR08

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],8
        FetchQ
if %B=32
        mov     [I.Disp+4],edx
end if
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA3:  mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      RtnA316
        cmp     al,CPUX32
        jz      RtnA332
        jmp     RtnA364
        ret

RtnA316:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jnc     .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA332:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jc      .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .W
        mov     [I.DispSize],4
        FetchUD
        jmp     .X
.W:     mov     [I.DispSize],2
        FetchW
        movzx   eax,ax
.X:
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnA364:
        mov     al,GPR16
        mov     [I.RandSize],VWORD
        BitTR   I.Prefixes,P66
        jc      .16
        mov     al,GPR32
        mov     [I.RandSize],VDWORD
.16:
        cmp     [I.W],0
        jz      .XX
        mov     al,GPR64
        mov     [I.RandSize],VQWORD
.XX:
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],al

        BitTR   I.Prefixes,P67
        jc      .D
        mov     [I.DispSize],8
        FetchQ
if %B=32
        mov     [I.Disp+4],edx
end if
        jmp     .X
.D:     mov     [I.DispSize],4
        FetchUD
.X:
        mov     [I.Arg1.Mem],True
        mov     [I.Disp],rax
        call    ClearSeg
        mov     [I.PossibleLOCK],True
        ret

RtnMOVX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TMOVS

        test    byte [I.Prefixes],bit PSEG+bit P67
        jz      .SkipATT

        BitTR   I.Prefixes,P67

        mov     [I.Name],TMOVS
        mov     [I.NoShowRand],True

        mov     [I.Base],RRSI

        mov     [I.Arg1.Type],10h+20h
        mov     [I.Arg1.Reg],RRDI

        mov     [I.Arg2.Type],1
        mov     [I.Arg2.Mem],True

        call    ClearSeg
.SkipATT:
        ret

RtnCMPX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TCMPS

        test    byte [I.Prefixes],bit PSEG+bit P67
        jz      .SkipATT

        BitTR   I.Prefixes,P67

        mov     [I.Name],TCMPS

        mov     [I.Base],RRSI

        mov     [I.Arg2.Type],10h
        mov     [I.Arg2.Reg],RRDI

        mov     [I.Arg1.Type],1
        mov     [I.Arg1.Mem],True

        call    ClearSeg
.SkipATT:
        ret

RtnSTOX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TSTOS

        BitTR   I.Prefixes,P67
        jnc     .SkipATT

        mov     [I.Name],TSTOS

        mov     [I.Arg1.Reg],RRDI
        mov     [I.Arg1.Type],10h+20h
.SkipATT:
        ret

RtnLODX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TLODS

        test    byte [I.Prefixes],bit PSEG+bit P67
        jz      .SkipATT

        BitTR   I.Prefixes,P67

        mov     [I.Name],TLODS

        mov     [I.Base],RRSI

        mov     [I.Arg1.Type],1
        mov     [I.Arg1.Mem],True

        call    ClearSeg
.SkipATT:
        ret

RtnSCAX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TSCAS

        BitTR   I.Prefixes,P67
        jnc     .SkipATT

        mov     [I.Name],TSCAS

        mov     [I.Arg1.Reg],RRDI
        mov     [I.Arg1.Type],10h+20h
.SkipATT:
        ret

R8IMM8:
        mov     al,[rsi-1]
        and     al,0Fh
        add     al,[I.B]
        mov     [I.Arg1.Reg],al
        call    ClearB

        mov     [I.Arg1.Type],GPR08

        FetchSB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.Imm],rax
        ret

R16IMM16:
        mov     al,[rsi-1]
        sub     al,0xB8
        add     al,[I.B]
        mov     [I.Arg1.Reg],al
        call    ClearB

        mov     [I.Arg1.Type],GPR16

        FetchSW
        mov     [I.Arg2.Type],80h+2
        mov     [I.Arg2.Imm],rax
        ret

R32IMM32:
        mov     al,[rsi-1]
        sub     al,0xB8
        add     al,[I.B]
        mov     [I.Arg1.Reg],al
        call    ClearB

        mov     [I.Arg1.Type],GPR32

        FetchSD
        mov     [I.Arg2.Type],80h+4
        mov     [I.Arg2.Imm],rax
        ret

R64IMM64:
        mov     al,[rsi-1]
        sub     al,0xB8
        add     al,[I.B]
        mov     [I.Arg1.Reg],al
        call    ClearB

        mov     [I.Arg1.Type],GPR64

        FetchQ
if %B=32
        mov     [I.Arg2.Imm+4],edx
end if
        mov     [I.Arg2.Type],80h+8
        mov     [I.Arg2.Imm],rax
        ret

RM8R8:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Type2nd],InReg
        ret

RM8IMM8:call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM

        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.Imm],rax
        ret

RM16IMM16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        FetchSW
        mov     [I.Arg2.Type],80h+2
        mov     [I.Arg2.Imm],rax
        ret

RM32IMM32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        FetchSD
        mov     [I.Arg2.Type],80h+4
        mov     [I.Arg2.Imm],rax
        ret

RM64IMM32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        FetchSD
        mov     [I.Arg2.Type],80h+40h+4
        mov     [I.Arg2.Imm],rax
        ret

RM16IMMS8:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        FetchSB
        mov     [I.Arg2.Type],80h+40h+20h+1
        mov     [I.Arg2.Imm],rax
        ret

RM32IMMS8:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        FetchSB
        mov     [I.Arg2.Type],80h+40h+20h+1
        mov     [I.Arg2.Imm],rax
        ret

RM64IMMS8:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        FetchSB
        mov     [I.Arg2.Type],80h+40h+20h+1
        mov     [I.Arg2.Imm],rax
        ret

RM16R16CL:
        push    SetArg3CL
        jmp     RM16R16

RM16R16I8:
        push    SetArg3Imm
RM16R16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InReg
        ret

RM32R32CL:
        push    SetArg3CL
        jmp     RM32R32

RM32R32I8:
        push    SetArg3Imm
        jmp     RM32R32

RM32R32X:
        push    SetArg12X
RM32R32:
        call    FetchModRM
RM32R32M:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InReg
        ret

RM64R64CL:
        push    SetArg3CL
        jmp     RM64R64
RM64R64I8:
        push    SetArg3Imm
RM64R64:
        call    FetchModRM
RM64R64M:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InReg
        ret

R8RM8:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InReg
        ret

RM8:    call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM
        ret

R16RM16LZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TLZCNT
.L1:    jmp     R16RM16
R16RM16TZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TTZCNT
.L1:    jmp     R16RM16

R16RM16W:
R16RM16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InReg
        ret

R32RM32LZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TLZCNT
.L1:    jmp     R32RM32
R32RM32TZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TTZCNT
.L1:    jmp     R32RM32

R32RM32X:
        push    SetArg12X
        jmp     R32RM32
R32RM32W:
        push    SetArg2W
R32RM32:
        call    FetchModRM
R32RM32M:
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InReg
        ret

R64RM64LZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TLZCNT
.L1:    jmp     R64RM64
R64RM64TZ:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        mov     [I.Name],TTZCNT
.L1:    jmp     R64RM64

R64RM64W:
        push    SetArg2W
R64RM64:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg
        ret

R3264RM3264:
        cmp     [I.W],0
        jz      R32RM32
        mov     [I.RandSize],VQWORD
        jmp     R64RM64

R16RM16L:
        mov     [I.IsLEA],True
R16RM163:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InReg
        ret

R32RM32L:
        mov     [I.IsLEA],True
R32RM323:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InReg
        ret

R64RM64L:
        mov     [I.IsLEA],True
R64RM643:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg
        ret

RM16R163:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InReg
        ret

RM32R323:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InReg
        ret

RM64R643:
        call    FetchModRM

        cmp     [I.Mod],3
        jz      ErrorDec

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InReg
        ret

RM16SEG:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],SEGRG
        mov     [I.Arg2.Type2nd],InReg
        ret

RM32SEG:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],SEGRG
        mov     [I.Arg2.Type2nd],InReg
        ret

RM64SEG:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],SEGRG
        mov     [I.Arg2.Type2nd],InReg
        ret

SEGRM16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],SEGRG
        mov     [I.Arg1.Type2nd],InReg
        ret

SEGRM32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],SEGRG
        mov     [I.Arg1.Type2nd],InReg
        ret

SEGRM64:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],SEGRG
        mov     [I.Arg1.Type2nd],InReg
        ret

RM16N:  mov     [I.Point],TNEAR
RM16:   call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM
        ret

RM32N:  mov     [I.Point],TNEAR
RM32:   call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM
        ret

RM64N:  mov     [I.Point],TNEAR
RM64:   call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM
        ret

RM163264W:
        call    FetchModRM

        cmp     [I.Mod],3
        jnz     .No3
        BitTR   I.Prefixes,P66
.No3:
        cmp     [I.Syntax],0
        jz      .SkipIntel
        mov     [I.RandSize],0
.SkipIntel:

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]

        mov     [I.Arg1.Type],GPR16
        cmp     [I.Operand],0
        jz      .XX
        mov     [I.Arg1.Type],GPR32
.XX:
        cmp     [I.W],0
        jz      .64
        BitTR   I.Prefixes,P4X
        mov     [I.Arg1.Type],GPR64
.64:
        mov     [I.Arg1.Type2nd],InRM
        ret

RMDW:   call    FetchModRM

        cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        jnz     .No3
        BitTR   I.Prefixes,P66
.No3:
        mov     [I.Arg1.Type],GPR16
        cmp     [I.Operand],0
        jz      .XX
        mov     [I.RandSize],VDWORD
        mov     [I.Arg1.Type],GPR32
.XX:
        mov     [I.Arg1.Type2nd],InRM
        ret

RMW0100:call    FetchModRM
        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jnz     .Exit
        mov     dx,TENCLV
        cmp     al,$C0
        jz      .L1
        mov     dx,TVMCALL
        cmp     al,$C1
        jz      .L1
        mov     dx,TVMLAUNCH
        cmp     al,$C2
        jz      .L1
        mov     dx,TVMRESUME
        cmp     al,$C3
        jz      .L1
        mov     dx,TVMXOFF
        cmp     al,$C4
        jz      .L1
        mov     dx,TPCONFIG
        cmp     al,$C5
        jnz     ErrorDec
.L1:    mov     [I.Name],dx
.Exit:  ret

RMW0101:call    FetchModRM
        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jnz     .Exit
        mov     dx,TMONITOR
        cmp     al,$C8
        je      .L1
        mov     dx,TMWAIT
        cmp     al,$C9
        je      .L1
        mov     dx,TCLAC
        cmp     al,$CA
        je      .L1
        mov     dx,TSTAC
        cmp     al,$CB
        jz      .L1
        mov     dx,TENCLS
        cmp     al,$CF
        jne     ErrorDec
.L1:    mov     [I.Name],dx
        ret
.Exit:  call    TWorFW
        ret

RMW0102:call    FetchModRM
        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jnz     .Exit
        mov     dx,TENCLU
        cmp     al,$D7
        je      .L1
        mov     dx,TXTEST
        cmp     al,$D6
        je      .L1
        mov     dx,TXEND
        cmp     al,$D5
        je      .L1
        mov     dx,TVMFUNC
        cmp     al,$D4
        je      .L1
        mov     dx,TXGETBV
        cmp     al,$D0
        je      .L1
        mov     dx,TXSETBV
        cmp     al,$D1
        jne     ErrorDec
.L1:    mov     [I.Name],dx
        ret
.Exit:  call    TWorFW
        ret

RMW0103:call    FetchModRM
        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jnz     .Quit
        movzx   eax,byte [rsi-1]
        mov     edx,[rax*2+TVMXXTable-$D8*2]
        mov     [I.Name],dx
        cmp     al,$D9
        je      .Exit
        cmp     al,$DC
        je      .Exit
        cmp     al,$DD
        je      .Exit
        ;
        cmp     [I.Syntax],0
        jz      .YY
        cmp     al,$DF
        je      .Exit
.YY:    ;
        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
        cmp     al,CPUX32
        jz      .32

.64:    mov     [I.Arg1.Type],GPR64
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Arg1.Type],GPR32
      @@:
        jmp     .XX
.16:    mov     [I.Arg1.Type],GPR16
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Arg1.Type],GPR32
      @@:
        jmp     .XX
.32:    mov     [I.Arg1.Type],GPR32
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Arg1.Type],GPR16
      @@:
.XX:    cmp     dx,TINVLPGA
        jnz     @F
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Reg],RRCX
      @@:
.Exit:  ret

.Quit:  call    TWorFW
        ret

RMW0104:call    FetchModRM

        cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        jnz     .Exit

        mov     [I.Arg1.Type],GPR16
        mov     al,[I.Operand]
        add     [I.Arg1.Type],al

        mov     [I.Arg1.Type2nd],InRM

        cmp     [I.W],0
        jz      .W0
        BitTR   I.Prefixes,P4X
        mov     [I.Arg1.Type],GPR64
.W0:
        BitTR   I.Prefixes,P66
.Exit:  ret

RtnPKRU:cmp     byte [rsi],0xC0
        jb      .IsRSTORSSP
        FetchB
        mov     dx,TSETSSBSY
        cmp     al,0xE8
        jz      .IsEA
        mov     dx,TSAVEPREVSSP
        cmp     al,0xEA
        jz      .IsEA
        mov     dx,TRDPKRU
        cmp     al,0xEE
        jz      .MoveName
        mov     dx,TWRPKRU
        cmp     al,0xEF
        jnz     ErrorDec
.MoveName:
        mov     [I.Name],dx
        ret
.IsEA:  BitTR   I.Prefixes,PF3
        jc      .MoveName
        jmp     ErrorDec
.IsRSTORSSP:
        BitTR   I.Prefixes,PF3
        jnc     ErrorDec
        setc    [I.Arg1.Mem]
        mov     [I.Name],TRSTORSSP
        jmp     FetchModRM

RMW0107:call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jnz     .Exit

        mov     dx,TRDPRU
        cmp     al,$FD
        je      .L1
        mov     dx,TCLZERO
        cmp     al,$FC
        je      .L1
        mov     dx,TMWAITX
        cmp     al,$FB
        je      .L1
        mov     dx,TMONITORX
        cmp     al,$FA
        je      .L1
        mov     dx,TRDTSCP
        cmp     al,$F9
        je      .L1
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
        mov     dx,TSWAPGS
        cmp     al,$F8
        jne     ErrorDec
.L1:    mov     [I.Name],dx
.Exit:  ret

LOADALL2:
        test    [I.Sensitive],UND
        jz      .L1
        mov     [I.Name],TLOADALL286
.L1:    ret

Rtn0F07:
        test    [I.Sensitive],UND
        jz      .L1
        mov     [I.Name],TLOADALL
.L1:    ret

PREFETCHRTN:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        jz      .Exit

        mov     al,[I.Reg]
        mov     dx,TPREFETCHWT1
        cmp     al,2
        jz      .L1
        cmp     al,1
        jne     .Exit
        mov     dx,TPREFETCHW
.L1:    mov     [I.Name],dx
.Exit:  ret

RtnMM4MM:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],MMXRG
        mov     [I.Arg2.Type],MMXRG

        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg2.Type2nd],InReg
        ret

RtnMM2MM:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],MMXRG
        mov     [I.Arg1.Type],MMXRG

        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg1.Type2nd],InReg
        ret

Rtn0F0F:call    RtnMM2MM
        mov     ecx,(End3D-Now3D)/3
        mov     edx,Now3D
        FetchB
.L1:    cmp     [rdx],al
        je      .L2
        add     edx,3
        loop    .L1
        jmp     ErrorDec
.L2:    mov     eax,[rdx+1]
        mov     [I.Name],ax
        ret
;-----------------------------------------------------------------------------
Rtn0F22:push    R32RM32C
        jmp     Rtn0F20.L1
;-----------------------------------------------------------------------------
Rtn0F20:push    RM32R32C
.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jne     ErrorDec
        ret
;-----------------------------------------------------------------------------
Rtn0F23:push    R32RM32D
        jmp     Rtn0F21.L1
;-----------------------------------------------------------------------------
Rtn0F21:push    RM32R32D
.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jne     ErrorDec
        ret
;-----------------------------------------------------------------------------
Rtn0F26:push    R32RM32T
        jmp     Rtn0F24.L1
;-----------------------------------------------------------------------------
Rtn0F24:push    RM32R32T
.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jne     ErrorDec
        ret
;-----------------------------------------------------------------------------
Rtn0F37:test    [I.Sensitive],NIA
ifz     ret
        mov     [I.Name],TWRSHR
Rtn0F36:test    [I.Sensitive],NIA
        jz      ErrorDec
        cmp     [I.Arch],CPUX64
        jz      ErrorDec
        call    FetchModRM
        cmp     [I.Reg],0
        jnz     ErrorDec
        BitTR   I.Prefixes,P66
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
Rtn0F3800:
        test    [I.Sensitive],NIA
        jz      .L1
        mov     [I.Name],TSMINT
        ret
.L1:    FetchB
        cmp     al,0xF0
        jb      .L0
        cmp     al,0xF1
        jbe     RtnMOVBE
        cmp     al,0xF6
        jz      RtnWRSSX
.L0:
        mov     dx,TSHA1NEXTE
        cmp     al,0xC8
        jz      .0F38XX
        mov     dx,TSHA1MSG1
        cmp     al,0xC9
        jz      .0F38XX
        mov     dx,TSHA1MSG2
        cmp     al,0xCA
        jz      .0F38XX
        mov     dx,TSHA256RNDS2
        cmp     al,0xCB
        jz      .0F38XX
        mov     dx,TSHA256MSG1
        cmp     al,0xCC
        jz      .0F38XX
        mov     dx,TSHA256MSG2
        cmp     al,0xCD
        jnz     .FUCK
.0F38XX:mov     [I.Name],dx
        mov     [I.RandSize],VXWORD+80h
        cmp     al,0xCB
ifz     mov     [I.Arg3.Type],XMMRG
        jmp     R32RM32X
.FUCK:  mov     ebx,RtnMM2MM
        call    CheckPrevSSE4
        jc      ErrorDec
        ret
;-----------------------------------------------------------------------------
RtnWRSSX:
        mov     [I.Name],TWRSSD
        test    [I.PreREX],8    ;REX.W
        jz      RM32R32
        call    ClearW
        mov     [I.Name],TWRSSQ
        jmp     RM64R64
;-----------------------------------------------------------------------------
Rtn0F3X:test    [I.Sensitive],NIA
        jz      ErrorDec
        ret
;-----------------------------------------------------------------------------
Rtn0F50:test    [I.Sensitive],NIA
        jz      Rtn0066F2F3X
.L0:    mov     [I.RandSize],VQWORD+80h
        jmp     RtnMM2MM
;-----------------------------------------------------------------------------
RtnIAMM:test    [I.Sensitive],NIA
        jz      Rtn0066F2F3
        jmp     Rtn0F50.L0
;-----------------------------------------------------------------------------
RtnIAM3:test    [I.Sensitive],NIA
        jz      Rtn0066F2F3
        cmp     byte [rsi],0xC0
        jae     ErrorDec
        jmp     Rtn0F50.L0
;-----------------------------------------------------------------------------
Rtn0F6X:cmp     [I.PrefixByte],bit P66
        jnz     .L1
        and     byte [I.Prefixes],not (bit P66)
        mov     [I.Mandatory66],1
        mov     [I.RandSize],VXWORD+80h
        jmp     R32RM32X
.L1:    cmp     [I.PrefixByte],0
        jnz     ErrorDec
        cmp     byte [rsi-1],6Ch
        jae     ErrorDec
        jmp     RtnMM2MM
;-----------------------------------------------------------------------------
Rtn0F72:mov     r8d,T0F72Names
        jmp     Rtn0F71.L0
;-----------------------------------------------------------------------------
Rtn0F71:mov     r8d,T0F71Names
.L0:    call    FetchModRM
.0L:    movzx   eax,[I.Reg]
        mov     edx,[rax*2+r8]
        or      dx,dx
        jz      .L5
.L1:    cmp     [I.Mod],3
        jne     .L5
        mov     [I.Name],dx
        mov     al,[I.PrefixByte]
        cmp     al,bit P66
        jnz     .L4
        and     byte [I.Prefixes],not (bit P66)
        mov     [I.Mandatory66],1
.L2:    mov     [I.Arg1.Type],XMMRG
.L3:    mov     [I.Arg1.Type2nd],InRM
        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.ImmB],al
        ret
.L4:    mov     [I.Arg1.Type],MMXRG
        or      al,al
        jz      .L3
.L5:    jmp     ErrorDec
;-----------------------------------------------------------------------------
Rtn0F73:mov     r8d,T0F73Names
        call    FetchModRM
        cmp     [I.PrefixByte],0
        jnz     Rtn0F71.0L
        ;MMX /2 /6
        cmp     [I.Reg],2
        je      Rtn0F71.0L
        cmp     [I.Reg],6
        je      Rtn0F71.0L
        jmp     ErrorDec
;-----------------------------------------------------------------------------
RtnMMXM:mov     edx,RtnMM2MM
        mov     al,[I.PrefixByte]
        or      al,al
        jz      .L1
        cmp     al,bit P66
        jnz     ErrorDec
        mov     edx,R32RM32X
        and     byte [I.Prefixes],not (bit P66)
        mov     [I.Mandatory66],1
        mov     [I.RandSize],VXWORD+80h
.L1:    jmp     rdx
;-----------------------------------------------------------------------------
Rtn0F78:
        test    [I.Sensitive],NIA
        jz      .L0
        cmp     byte [rsi],0xC0
        jae     ErrorDec
        mov     [I.RandSize],VTWORD+80h
        mov     [I.Name],TSVDC
        jmp     RM16SEG
.L0:    mov     al,[I.PrefixByte]
        or      al,al
        jnz     .L2
        mov     edx,RM32R32
        mov     [I.RandSize],VDWORD+80h
        cmp     [I.Arch],CPUX64
        jnz     .L1
        mov     edx,RM64R64
        mov     [I.RandSize],VQWORD+80h
.L1:    jmp     rdx
.L2:    cmp     byte [rsi],0xC0
        jb      ErrorDec
        cmp     al,bit P66
        jnz     .L3
        call    FetchModRM
        cmp     [I.Reg],0
        jnz     ErrorDec
        and     byte [I.Prefixes],not (bit P66)
        mov     [I.Mandatory66],1
        mov     [I.Name],TEXTRQ
        mov     [I.Arg1.Type],XMMRG
        mov     [I.Arg1.Type2nd],InRM
        lea     r8,[I.Arg2]
.LX:    FetchW
        mov     [r8+TArg.Type],80h+1
        mov     [r8+TArg.ImmB],al
        add     r8d,sizeof.TArg
        mov     [r8+TArg.Type],80h+1
        mov     al,ah
        mov     [r8+TArg.ImmB],al
        ret
.L3:    cmp     al,bit PF2
        jnz     ErrorDec
        and     byte [I.Prefixes],not (bit PF2)
        mov     [I.Name],TINSERTQ
        call    R32RM32X
        lea     r8,[I.Arg3]
        jmp     .LX
;-----------------------------------------------------------------------------
Rtn0F79:test    [I.Sensitive],NIA
        jz      .L0
        cmp     byte [rsi],0xC0
        jae     ErrorDec
        mov     [I.RandSize],VTWORD+80h
        mov     [I.Name],TRSDC
        jmp     SEGRM16
.L0:    mov     al,[I.PrefixByte]
        or      al,al
        jnz     .L2
        mov     edx,R32RM32
        mov     [I.RandSize],VDWORD+80h
        cmp     [I.Arch],CPUX64
        jnz     .L1
        mov     edx,R64RM64
        mov     [I.RandSize],VQWORD+80h
.L1:    jmp     rdx
.L2:    cmp     byte [rsi],0xC0
        jb      ErrorDec
        cmp     al,bit P66
        jnz     .L3
        and     byte [I.Prefixes],not (bit P66)
        mov     [I.Mandatory66],1
        mov     [I.Name],TEXTRQ
        jmp     R32RM32X
.L3:    cmp     al,bit PF2
        jnz     ErrorDec
        and     byte [I.Prefixes],not (bit PF2)
        mov     [I.Name],TINSERTQ
        jmp     R32RM32X
;-----------------------------------------------------------------------------
Rtn0F7B:
Rtn0F7A:test    [I.Sensitive],NIA
        jz      ErrorDec
.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jz      ErrorDec
        cmp     [I.Reg],0
        jnz     ErrorDec
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
Rtn0F7D:
Rtn0F7C:
        test    [I.Sensitive],NIA
        jz      Rtn0066F2F3
        jmp     Rtn0F7A.L1
;-----------------------------------------------------------------------------
Rtn0F7E:test    [I.Sensitive],NIA
        jz      Rtn0066F2F3X
        ret
;-----------------------------------------------------------------------------
Rtn0FA6:test    [I.Sensitive],NIA
        jnz     .L1
        cmp     [I.Arch],CPUX64
        je      .L0
        mov     [I.RandSize],VBYTE+80h
        test    [I.Sensitive],UND
        jnz     RM8R8
.L0:    jmp     ErrorDec

.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jnz     .L0
        cmp     [I.RM],0
        jnz     .L0
        movzx   eax,[I.Reg]
        cmp     al,2
        ja      .L0
        BitTR   I.Prefixes,PF3
        jnc     .L0
        mov     eax,[rax*2+TCentTable]
        mov     [I.Name],ax
        ret
;-----------------------------------------------------------------------------
Rtn0FA7:test    [I.Sensitive],NIA
        jnz     .L1
        test    [I.Sensitive],UND
        jz      .L4
        cmp     [I.Arch],CPUX64
        je      .L4
        BitTR   I.Prefixes,P66
        cmp     [I.Operand],0
        mov     [I.RandSize],VDWORD+80h
        jnz     RM32R32
        mov     [I.RandSize],VWORD+80h
        jmp     RM16R16

.L1:    call    FetchModRM
        cmp     [I.Mod],3
        jnz     .L4
        cmp     [I.RM],0
        jnz     .L4
        cmp     [I.PrefixByte],bit PF3
        jnz     .L3
        BitTR   I.Prefixes,PF3
        movzx   eax,[I.Reg]
        mov     eax,[rax*2+TCRYPTable]
        or      ax,ax
        jz      .L4
.L2:    mov     [I.Name],ax
        ret
.L3:    cmp     al,$C0
        mov     ax,TXSTORE
        jz      .L2
.L4:    jmp     ErrorDec
;-----------------------------------------------------------------------------
Rtn0FAE:call    FetchModRM
        mov     rsi,[I.SaveRSI]

        movzx   ebx,[I.Reg]

        cmp     [I.PrefixByte],bit P66
        je      Rtn660FAE

        cmp     [I.Mod],3
        je      .L2

        mov     edx,T0FAETable
        cmp     [I.W],0
        jz      .L0
        mov     edx,T0FAETab64
        call    ClearW
.L0:
        cmp     [I.PrefixByte],bit PF3
        je      RtnF30FAE

        mov     edx,[rbx*2+rdx]
.L1:    or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        jmp     RM32
.L2:
        cmp     [I.PrefixByte],bit PF3
        je      RtnF30FAE
        cmp     [I.PrefixByte],bit PF2
        je      RtnF20FAE
        mov     dx,TLFENCE
        cmp     al,$E8
        je      .L3
        mov     dx,TMFENCE
        cmp     al,$F0
        je      .L3
        mov     dx,TSFENCE
        cmp     al,$F8
        jne     ErrorDec
.L3:    lodsb
        mov     [I.Name],dx
        ret

Rtn660FAE:
        and     byte [I.Prefixes],not (bit P66)
        mov     edx,T660FAETable3
        cmp     [I.Mod],3
        jz      .L1
        mov     edx,T660FAETable0
.L1:    mov     edx,[rbx*2+rdx]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        jmp     RM32

RtnF20FAE:
        and     byte [I.Prefixes],not (bit PF2)
        cmp     byte [rsi],0xC0
        jb      ErrorDec
        mov     [I.Name],TUMWAIT
        jmp     RM32

RtnF30FAE:
        cmp     bl,06h
        jz      .L0
        cmp     [I.Arch],CPUX64
        jne     ErrorDec
.L0:
        mov     edx,[rbx*2+TF30FAETable]
        or      dx,dx
        jz      ErrorDec
        mov     [I.Name],dx
        and     byte [I.Prefixes],not (bit PF3)
        cmp     bl,06h
        jz      IsUMONITOR
        call    ClearW
        mov     [I.RandSize],VDWORD
        cmp     [I.W],0
        jz      RM32
        cmp     bl,5
ifz     mov     [I.Name],TINCSSPQ
        mov     [I.RandSize],VQWORD
        jmp     RM64
IsUMONITOR:
        cmp     byte [rsi],0xC0
        jbe     .1
        BitTR   I.Prefixes,P67
        call    FetchModRM
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     al,[I.Address]
        add     al,GPR16
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InRM
        ret
.1:     mov     [I.Name],TCLRSSBSY
        call    FetchModRM
        mov     [I.Arg1.Mem],1
        ret
;-----------------------------------------------------------------------------
Rtn0FC2:xor     ebx,ebx
        mov     al,[I.PrefixByte]
        or      al,al
        jz      .L1
        inc     ebx
        cmp     al,bit P66
        setz    [I.Mandatory66]
        jz      .L1
        inc     ebx
        cmp     al,bit PF2
        jz      .L1
        inc     ebx
        cmp     al,bit PF3
        jnz     ErrorDec
.L1:    not     al
        and     byte [I.Prefixes],al

        mov     eax,[rbx*3+CMPSuffixesY]
        mov     word [I.Suffix2nd],ax
        shr     eax,16
        mov     [I.RandSize],al

        call    R32RM32X
        xor     eax,eax
        FetchB
        mov     edx,[rax*2+CMPxxNames]
        cmp     al,7
        ja      .L2
        mov     [I.Name],dx
        ret
.L2:    mov     [I.Name],TCMP
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
;-----------------------------------------------------------------------------
Rtn0FC73:
        mov     ax,TRDPID
        cmp     [I.Reg],7
        jnz     .M7
        BitTR   I.Prefixes,PF3
        jc      .M0
.M7:    cmp     [I.Reg],6
        jb      ErrorDec
        mov     ax,TRDRAND
        jz      .M0
        mov     ax,TRDSEED
.M0:    mov     [I.Name],ax
        cmp     [I.W],0
        jz      .M1
        call    ClearW
        jmp     RM64
.M1:    BitTR   I.Prefixes,P66
        cmp     [I.Operand],0
        jnz     RM32
        jmp     RM16
;-----------------------------------------------------------------------------
Rtn0FC7:call    FetchModRM
        mov     rsi,[I.SaveRSI]
        cmp     [I.Mod],3
        jz      Rtn0FC73
        cmp     [I.Reg],1
        jne     .L0
        or      [I.Flags],_XA+_XR
        mov     [I.RandSize],VQWORD+80h
        cmp     [I.W],0
        jz      .RM
        mov     [I.RandSize],VXWORD+80h
        mov     dx,TCMPXCHG16B
.WWWW:  call    ClearW
.EA:    mov     [I.Name],dx
.RM:    jmp     RM32
.L0:
        mov     edx,T0FC7Table
        cmp     [I.W],0
        jz      .W0
        mov     edx,T0FC7Tab64
.W0:    movzx   eax,[I.Reg]
        mov     dx,[rax*2+rdx]
        or      dx,dx
        jnz     .WWWW

        mov     [I.RandSize],VQWORD+80h
        mov     ah,[I.Reg]
        mov     al,[I.PrefixByte]
        or      al,al
        jne     .L1
        mov     dx,TVMPTRLD
        cmp     ah,6
        je      .EA
        mov     dx,TVMPTRST
        cmp     ah,7
        je      .EA
.ED:    jmp     ErrorDec

.L1:    cmp     [I.Reg],6
        jne     .ED
        cmp     al,bit P66
        jne     .L2
        mov     dx,TVMCLEAR
        mov     [I.Mandatory66],1
.AN:    not     al
        and     byte [I.Prefixes],al
        jmp     .EA

.L2:    cmp     al,bit PF3
        jne     .ED
        mov     dx,TVMXON
        jmp     .AN
;-----------------------------------------------------------------------------
RtnBSWAP:
        mov     al,[rsi-1]
        sub     al,0xC8
        mov     [I.RM],al
        mov     [I.Mod],3
        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg1.Type],GPR64
        cmp     [I.W],0
        jnz     .Exit
        BitTR   I.Prefixes,P66
        mov     [I.Arg1.Type],GPR32
        cmp     [I.Operand],0
        jnz     .Exit
        mov     [I.Arg1.Type],GPR16
.Exit:  ret
;-----------------------------------------------------------------------------
MOVZXWB:call    R16RM16
        mov     [I.AltName],TMOVZBW
        mov     [I.Arg2.Type],GPR08
        ret
MOVZXDB:call    R32RM32
        mov     [I.AltName],TMOVZBL
        mov     [I.Arg2.Type],GPR08
        ret
MOVZXQB:call    R64RM64
        mov     [I.AltName],TMOVZBQ
        mov     [I.Arg2.Type],GPR08
        ret
MOVZXWW:call    R16RM16
        mov     [I.AltName],TMOVZWW
        ret
MOVZXDW:call    R32RM32
        mov     [I.AltName],TMOVZWL
        mov     [I.Arg2.Type],GPR16
        ret
MOVZXQW:call    R64RM64
        mov     [I.AltName],TMOVZWQ
        mov     [I.Arg2.Type],GPR16
        ret
;-----------------------------------------------------------------------------
MOVSXWB:call    R16RM16
        mov     [I.AltName],TMOVSBW
        mov     [I.Arg2.Type],GPR08
        ret
MOVSXDB:call    R32RM32
        mov     [I.AltName],TMOVSBL
        mov     [I.Arg2.Type],GPR08
        ret
MOVSXQB:call    R64RM64
        mov     [I.AltName],TMOVSBQ
        mov     [I.Arg2.Type],GPR08
        ret
MOVSXWW:call    R16RM16
        mov     [I.AltName],TMOVSWW
        ret
MOVSXDW:call    R32RM32
        mov     [I.AltName],TMOVSWL
        mov     [I.Arg2.Type],GPR16
        ret
MOVSXQW:call    R64RM64
        mov     [I.AltName],TMOVSWQ
        mov     [I.Arg2.Type],GPR16
        ret
;-----------------------------------------------------------------------------
R16RM16B8:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        call    R16RM16
        mov     [I.RandSize],VWORD+80h
        ret
.L1:    mov     [I.Name],TJMPE
        or      [I.Flags],_J
        jmp     DODISPW
;-----------------------------------------------------------------------------
R32RM32B8:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        call    R32RM32
        mov     [I.RandSize],VDWORD+80h
        ret
.L1:    mov     [I.Name],TJMPE
        or      [I.Flags],_J
        jmp     DODISPD
;-----------------------------------------------------------------------------
R64RM64B8:
        BitTR   I.Prefixes,PF3
        jnc     .L1
        call    ClearW
        call    R64RM64
        mov     [I.RandSize],VQWORD+80h
        ret
.L1:    mov     [I.Name],TJMPE
        or      [I.Flags],_J
        jmp     DODISPD
;-----------------------------------------------------------------------------
RtnCRC32:
        mov     [I.Table],TableCRC32
        jmp     RtnMOV32
;-----------------------------------------------------------------------------
RtnMOVBE:
        mov     [I.Table],TableMOVBE
RtnMOV32:
        mov     [I.NewTable],1
        mov     [I.Only],0
        sub     al,0xF0
        ret
;-----------------------------------------------------------------------------
RtnADOX:mov     [I.Table],TableADOX
        jmp     RtnADXX
RtnADCX:mov     [I.Table],TableADCX
RtnADXX:mov     [I.NewTable],1
        mov     [I.Only],1
        xor     al,al
        ret
;-----------------------------------------------------------------------------
RtnINVXX:
        mov     [I.Table],TableINVx
        mov     [I.NewTable],1
        mov     [I.Only],1
        sub     al,80h
        ret
;-----------------------------------------------------------------------------
Rtn0F388X:
        call    R32RM32
        cmp     [I.Mod],3
        jz      ErrorDec
        cmp     [I.Arch],CPUX64
        jnz     .L1
        mov     [I.Arg1.Type],GPR64
.L1:    ret
;-----------------------------------------------------------------------------
Rtn0F3866:
        FetchB
        cmp     al,0x80
        jb      .M0
        cmp     al,0x82
        jbe     RtnINVXX
.M0:
        cmp     al,0xF0
        jb      .L0
        cmp     al,0xF1
        jbe     RtnMOVBE

        cmp     al,0xF6
        jz      RtnADCX

.L0:    mov     ebx,R32RM32X
        call    CheckPrevSSE4
        jnc     .LX
        call    CheckNextSSE4
        jc      ErrorDec
.LX:    ret
;-----------------------------------------------------------------------------
Rtn0F38F2:
        FetchB
        cmp     al,0xF0
        jb      .L0
        cmp     al,0xF1
        jbe     RtnCRC32
.L0:    jmp     ErrorDec
;-----------------------------------------------------------------------------
Rtn0F38F3:
        FetchB
        cmp     al,0xF6
        jz      RtnADOX
.L0:    jmp     ErrorDec
;-----------------------------------------------------------------------------
CRC32R: call    R32RM32
        mov     [I.Arg2.Type],GPR08
        cmp     [I.W],0
        jz      .L1
        mov     [I.Arg1.Type],GPR64
.L1:    ret
;-----------------------------------------------------------------------------
CRC32X: call    R32RM32
        BitTR   I.Prefixes,P66
        jnc     .L0
        mov     [I.Arg2.Type],GPR16
.L0:
        cmp     [I.W],0
        jz      .L1
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg2.Type],GPR64
.L1:    ret
;-----------------------------------------------------------------------------
Rtn0F39:test    [I.Sensitive],NIA
        jz      ErrorDec
        ret
;-----------------------------------------------------------------------------
Rtn0F3A:test    [I.Sensitive],NIA
        jnz     .L0
        test    [I.Sensitive],UND
        jz      .L1
        mov     [I.Name],TRDM
        ret
.L0:    mov     [I.Name],TBB0?RESET
        ret
.L1:    FetchB
        cmp     al,14h
        jz      Rtn0F3A14
        cmp     al,15h
        jz      Rtn0F3A15
        cmp     al,16h
        jz      Rtn0F3A16
        cmp     al,17h
        jz      Rtn0F3A17
        cmp     al,20h
        jz      Rtn0F3A20
        cmp     al,21h
        jz      Rtn0F3A21
        cmp     al,22h
        jz      Rtn0F3A22
        cmp     al,44h
        jz      Rtn0F3A44
        cmp     al,$CC
        jz      .CC

        mov     edx,TableSXSSE
.L5:    cmp     al,[rdx]
        je      .L4
        add     edx,4
        cmp     edx,TableEXSSE
        jb      .L5
        jmp     ErrorDec
.L4:    mov     edx,[rdx+1]
        mov     [I.RandSize],dl
        shr     edx,8
        mov     [I.Name],dx

        BitTR   I.Prefixes,P66
        jnc     .No66
        mov     [I.Mandatory66],1
.L3:    call    R32RM32X
.L2:    FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
        ret
.No66:  cmp     al,0Fh
        jnz     ErrorDec
        mov     [I.RandSize],VQWORD+80h
        call    RtnMM2MM
        jmp     .L2
.CC:    mov     [I.Name],TSHA1RNDS4
        mov     [I.RandSize],VXWORD+80h
        jmp     .L3
;-----------------------------------------------------------------------------
Rtn0F1012:
        test    [I.Sensitive],UND
        jnz     RM8R8
        jmp     Rtn0066F2F3

Rtn0F11X:
        test    [I.Sensitive],UND
        jz      Rtn0066F2F3
        cmp     [I.Operand],0
        jz      RM16R16
        jmp     RM32R32

Rtn0F13X:
        test    [I.Sensitive],UND
        jz      Rtn0066F2F3
        cmp     [I.Operand],0
        jz      R16RM16
        jmp     R32RM32

Rtn0F18:call    FetchModRM
        cmp     [I.Mod],3
        jz      ErrorDec
        setnz   [I.Arg1.Mem]
        mov     al,[I.Reg]
        mov     dx,TPREFETCHNTA
        or      al,al
        jz      .L1
        mov     dx,TPREFETCHT0
        cmp     al,1
        jz      .L1
        mov     dx,TPREFETCHT1
        cmp     al,2
        jz      .L1
        mov     dx,TPREFETCHT2
        cmp     al,3
        jnz     ErrorDec
.L1:    mov     [I.Name],dx
        mov     [I.RandSize],0
        ret


Rtn0F1C:call    FetchModRM
        cmp     [I.Mod],3
        jz      .Exit
        cmp     [I.Reg],0
        jnz     .Exit
        stc
        ret
.Exit:  mov     rsi,[I.SaveRSI]
        clc
        ret

Is0F1C: mov     [I.RandSize],VBYTE+80h
        mov     [I.Name],TCLDEMOTE
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM
        ret

Rtn0F1C16:
        call    Rtn0F1C
        jc      Is0F1C
Rtn0F1X16:
        mov     ebx,RM16
        jmp     Rtn0F18XX.L1
Rtn0F1C32:
        call    Rtn0F1C
        jc      Is0F1C
Rtn0F1X32:
        mov     ebx,RM32
        jmp     Rtn0F18XX.L1
Rtn0F1C64:
        call    Rtn0F1C
        jc      Is0F1C
Rtn0F1X64:
        mov     ebx,RM64
        jmp     Rtn0F18XX.L1

Rtn0F1816:
        mov     ebx,RM16
        jmp     Rtn0F18XX
Rtn0F1832:
        mov     ebx,RM32
        jmp     Rtn0F18XX
Rtn0F1864:
        mov     ebx,RM64
       ;jmp     Rtn0F18XX
Rtn0F18XX:
.L0:    test    [I.Sensitive],UND
        jz      Rtn0F18
.L1:    BitTR   I.Prefixes,P66
        call    ClearW

        movzx   eax,word [rsi-1]
        sub     al,18h
        shl     al,3
        and     ah,111000b
        shr     ah,3
        add     al,ah
        xor     ah,ah
        mov     cl,10
        div     cl
        add     ax,'00'
        cmp     al,'0'
        jne     @F
        mov     al,ah
        xor     ah,ah
@@:     mov     [Names+THINT?NOPXX+8],ax
        mov     [I.Name],THINT?NOPXX
        jmp     rbx

Rtn0F1A:mov     r8d,N0F1A
        jmp     Rtn0F1B.L0
Rtn0F1B:mov     r8d,N0F1B
.L0:    xor     edx,edx
        mov     dl,3
        mov     ecx,edx
        movzx   eax,[I.PrefixByte]
        bsf     edx,eax
        bsr     ecx,eax
        cmp     edx,ecx
        jnz     ErrorDec
        push    rdx
        mov     edx,[rdx*2+r8]
        mov     [I.Name],dx
        BitTR   I.Prefixes,P67          ; 0x48 & 0x67 have no effect
        pushf
        call    FetchModRM
        popf
        jnc     @F
        BitTS   I.Prefixes,P67
@@:
        pop     rdx
        movzx   eax,word [rdx*2+r8+4*2]
        add     eax,RtnXX
        mov     [I.RandSize],0
        jmp     rax
;-----------------------------------------------------------------------------
Rtn0F1E:
        BitTR   I.Prefixes,PF3
        jz      ErrorDec
        mov     al,[rsi]
        mov     dx,TENDBR32
        cmp     al,0xFB
        jz      .2
        mov     dx,TENDBR64
        cmp     al,0xFA
        jz      .2
        mov     [I.Arg1.Type],GPR64
        mov     dx,TRDSSPQ
        test    [I.PreREX],8    ;REX.W
        jnz     .1
        mov     [I.Arg1.Type],GPR32
        mov     dx,TRDSSPD
.1:     mov     [I.Name],dx
        call    FetchModRM
        cmp     [I.Mod],3
        jnz     ErrorDec
        cmp     [I.Reg],1
        jnz     ErrorDec
        mov     [I.Arg1.Type2nd],InRM
        ret
.2:     mov     [I.Name],dx
        lodsb
        ret
;-----------------------------------------------------------------------------
RBNDLDX:push    BNDX1
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     R32RM32M
;-----------------------------------------------------------------------------
RBNDSTX:push    BNDX2
        cmp     [I.Mod],3
        jz      ErrorDec
        jmp     RM32R32M
;-----------------------------------------------------------------------------
RBNDMOV1:
        push    BNDX3
        BitTR   I.Prefixes,P66
        jmp     RM32R32M
;-----------------------------------------------------------------------------
RBNDMOV2:
        push    BNDX3
        BitTR   I.Prefixes,P66
        jmp     R32RM32M
;-----------------------------------------------------------------------------
RBNDCU: RBNDCN:
        push    BNDX4
        BitTR   I.Prefixes,PF2
        jmp     R32RM32M
;-----------------------------------------------------------------------------
RBNDMK: cmp     [I.Mod],3
        jz      ErrorDec
RBNDCL: push    BNDX4
        BitTR   I.Prefixes,PF3
        jmp     R32RM32M
;-----------------------------------------------------------------------------
Rtn0066F2F3:
        call    Start66F2F3
.L1:
        movzx   ebx,word [rdx+rbx+4*1+4*2]
        add     ebx,RtnXX

        mov     edx,[rdx]
        or      dx,dx
        jz      ErrorDec
        not     al
        and     byte [I.Prefixes],al
        mov     [I.Name],dx

        jmp     rbx
;-----------------------------------------------------------------------------
RtnWB:  BitTR   I.Prefixes,PF3
        jnc     .Exit
        mov     [I.Name],TWBNOINVD
.Exit:  ret
;-----------------------------------------------------------------------------
YaMOVDIRI:
        mov     [I.Name],TMOVDIRI
        mov     [I.RandSize],VDWORD+80h
        cmp     [I.W],0
        jz      RM32R32
        BitTR   I.Prefixes,P4X
        mov     [I.RandSize],VQWORD+80h
        jmp     RM64R64
;-----------------------------------------------------------------------------
YaMOVDIR64B:
        BitTR   I.Prefixes,P66
        mov     [I.Mandatory66],True
        mov     [I.Name],TMOVDIR64B
        call    FetchModRM
       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     al,[I.Address]
        add     al,GPR16
        mov     [I.Arg1.Type],al
        mov     [I.Arg1.Type2nd],InReg

        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM
        ret
;-----------------------------------------------------------------------------
Rtn0066F2F3Y:
        lodsb
        cmp     [I.PrefixByte],bit P66
        jnz     NoMOVDIR64B
        cmp     al,0xF8
        jnz     NoMOVDIR64B
        cmp     byte [rsi],0xC0
        jb      YaMOVDIR64B
NoMOVDIR64B:
        cmp     al,0xF9
        jnz     NoMOVDIRI
        cmp     byte [rsi],0xC0
        jb      YaMOVDIRI
NoMOVDIRI:
        dec     rsi

        mov     ecx,[rdx+4*2]

        mov     al,[I.PrefixByte]
        or      al,al
        jz      .L1
        shr     ecx,8
        add     edx,2
        cmp     al,bit P66
        setz    [I.Mandatory66]
        jz      .L1
        shr     ecx,8
        add     edx,2
        cmp     al,bit PF2
        jz      .L1
        cmp     al,bit PF2+bit P66
        jz      .L0
        shr     ecx,8
        add     edx,2
        cmp     al,bit PF3
        jz      .L1
        jmp     ErrorDec
.L0:    mov     al,bit PF2
.L1:    mov     [I.RandSize],cl

        xor     ebx,ebx
        jmp     Rtn0066F2F3.L1
;-----------------------------------------------------------------------------
Rtn0066F2F3X:
r8w     equ     di
        call    Start66F2F3

        mov     r8d,[rdx+rbx+2*2*4+4]

        movzx   ebx,word [rdx+rbx+1*2*4+4]
        add     ebx,RtnXX

        mov     edx,[rdx]
        or      dx,dx
        jz      ErrorDec
        not     al
        and     byte [I.Prefixes],al
        mov     [I.Name],dx

        push    r8
        call    rbx
        pop     r8

        mov     eax,r8d
        and     eax,0F0Fh
        mov     [I.Arg1.Type],al
        mov     [I.Arg2.Type],ah

        test    r8w,8080h
        jz      .L1
        cmp     [I.W],0
        jz      .L1
        call    ClearW

        test    r8b,0080h
ifnz    mov     [I.Arg1.Type],GPR64
        test    r8w,8000h
ifnz    mov     [I.Arg2.Type],GPR64

        test    r8w,2020h
ifnz    mov     [I.LastByte],'q'

        test    r8w,4040h
        jz      .L1
        and     [I.RandSize],0xF0
        or      [I.RandSize],VQWORD
.L1:
        test    r8w,1010h
        jz      .L2
        FetchB
        mov     [I.Arg3.Type],80h+1
        mov     [I.Arg3.ImmB],al
.L2:
        ret

RM16F5: mov     [I.AltName],TLJMP
        jmp     RM16F35
RM16F3: mov     [I.AltName],TLCALL
RM16F35:mov     [I.Point],TFAR
        call    RM16
        cmp     [I.Mod],3
        jz      ErrorDec
        cmp     [I.Syntax],0
        jnz     .SkipATT
        mov     [I.RandSize],VDWORD
.SkipATT:
        ret

RM32F5: mov     [I.AltName],TLJMP
        jmp     RM32F35
RM32F3: mov     [I.AltName],TLCALL
RM32F35:mov     [I.Point],TFAR
        call    RM32
        cmp     [I.Mod],3
        jz      ErrorDec
        cmp     [I.Syntax],0
        jnz     .SkipATT
        mov     [I.RandSize],VFWORD
.SkipATT:
        ret

RM64F5: mov     [I.AltName],TLJMP
        jmp     RM64F35
RM64F3: mov     [I.AltName],TLCALL
RM64F35:mov     [I.Point],TFAR
        call    RM64
        cmp     [I.Mod],3
        jz      ErrorDec
        cmp     [I.Syntax],0
        jnz     .SkipATT
        mov     [I.RandSize],VTWORD
.SkipATT:
        ret

R8I08:
        FetchB

        mov     [I.Arg1.Type],GPR08

        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.ImmB],al

        ret

R16I16:
        FetchW

        mov     [I.Arg1.Type],GPR16

        mov     [I.Arg2.Type],80h+2
        mov     [I.Arg2.Imm],rax

        ret

R32I32:
        FetchSD

        mov     [I.Arg1.Type],GPR32

        mov     [I.Arg2.Type],80h+4
        mov     [I.Arg2.Imm],rax

        ret

R64I32:
        FetchSD

        mov     [I.Arg1.Type],GPR64

        mov     [I.Arg2.Type],80h+40h+4
        mov     [I.Arg2.Imm],rax

        ret

R16RM16I16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InReg

        FetchSW
        mov     [I.Arg3.Type],80h+40h+20h+2
        mov     [I.Arg3.Imm],rax
        ret

R32RM32I32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InReg

        FetchSD
        mov     [I.Arg3.Type],80h+40h+20h+4
        mov     [I.Arg3.Imm],rax
        ret

R64RM64I32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg

        FetchSD
        mov     [I.Arg3.Type],80h+40h+20h+4
        mov     [I.Arg3.Imm],rax
        ret

R16RM16SI16:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InReg

        FetchSB
        mov     [I.Arg3.Type],80h+40h+20h+1
        mov     [I.Arg3.Imm],rax
        ret

R32RM32SI32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InReg

        FetchSB
        mov     [I.Arg3.Type],80h+40h+20h+1
        mov     [I.Arg3.Imm],rax
        ret

R64RM64SI32:
        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR64
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg

        FetchSB
        mov     [I.Arg3.Type],80h+40h+20h+1
        mov     [I.Arg3.Imm],rax
        ret

RtnINSX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TINS

        BitTR   I.Prefixes,P67
        jnc     .SkipATT

        mov     [I.Name],TINS

        mov     [I.Base],RRDI

        mov     [I.Arg1.Type],1
        mov     [I.Arg1.Mem],True

        mov     [I.Arg2.Type],2
        mov     [I.Arg2.Reg],RRDX
.SkipATT:
        ret

RtnOUTSX:
        mov     [I.PossibleF2F3],True

        mov     [I.AltName],TOUTS

        test    byte [I.Prefixes],bit PSEG+bit P67
        jz      .SkipATT

        BitTR   I.Prefixes,P67

        mov     [I.Name],TOUTS

        mov     [I.Base],RRSI

        mov     [I.Arg2.Type],1
        mov     [I.Arg2.Mem],True

        mov     [I.Arg1.Type],2
        mov     [I.Arg1.Reg],RRDX

        call    ClearSeg
.SkipATT:
        ret

RM16I8: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.Imm],rax
        ret

RM32I8: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.Imm],rax
        ret

RM64I8: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.Imm],rax
        ret

RtnC216:
        mov     [I.IsRET],True
        FetchW
        mov     [I.Arg1.Type],80h+2
        mov     [I.Arg1.Imm],rax

        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.Suffix],'w'
.SkipSuffix:
        ret

RtnC232:
        mov     [I.IsRET],True
        FetchW
        mov     [I.Arg1.Type],80h+2
        mov     [I.Arg1.Imm],rax
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.Suffix],'d'
.SkipSuffix:
        ret

RtnC264:
        mov     [I.IsRET],True
        FetchW
        mov     [I.Arg1.Type],80h+2
        mov     [I.Arg1.Imm],rax
AddSuffixX:
        cmp     [I.W],0
        jz      .SkipSuffix
        BitTR   I.Prefixes,P4X
        mov     [I.Suffix],'q'
.SkipSuffix:
        ret

AddSuffixQ:
        cmp     [I.Syntax],0
        jz      AddSuffixX
        ret

RtnC316:
        mov     [I.IsRET],True
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.Suffix],'w'
.SkipSuffix:
        ret

RtnC332:
        mov     [I.IsRET],True
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.Suffix],'d'
.SkipSuffix:
        ret

RtnC364:
        mov     [I.IsRET],True
        cmp     [I.W],0
        jz      .SkipSuffix
        BitTR   I.Prefixes,P4X
        mov     [I.Suffix],'q'
.SkipSuffix:
        ret

RtnC816:
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.RandSize],VWORD
        mov     [I.Suffix],'w'
.SkipSuffix:
        jmp     RtnC864

RtnC832:
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.RandSize],VDWORD
        mov     [I.Suffix],'d'
.SkipSuffix:
        jmp     RtnC864

RtnC864:
        lea     r10,[I.Arg2]
        lea     rax,[I.Arg1]
        mov     r8,rax
        xor     r8,r10

        cmp     [I.Syntax],0
        jnz     .ATT
        xor     r10,r8
.ATT:
        FetchW
        mov     [r10+TArg.Type],80h+2
        mov     [r10+TArg.Imm],rax
        FetchB
        xor     r10,r8
        mov     [r10+TArg.Type],80h+1
        mov     [r10+TArg.Imm],rax
        ret

RtnC916:
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.RandSize],VWORD
        mov     [I.Suffix],'w'
.SkipSuffix:
        jmp     RtnC964
RtnC932:
        BitT    I.Prefixes,P66
        jnc     .SkipSuffix
        mov     [I.RandSize],VDWORD
        mov     [I.Suffix],'d'
.SkipSuffix:
RtnC964:
        ret

RtnCD:  cmp     [I.Emulated],0
        jnz     .Emul
        FetchB
.L4:    mov     [I.Name],TINT
        mov     [I.Arg1.Type],80h+1
        mov     [I.Arg1.ImmB],al
        ret
.Emul:
        FetchB
        cmp     al,34h
        jb      .L4
        cmp     al,3Bh
        ja      .L1
        add     al,$D8-34h
        pop     rdx
        jmp     DisAsm.NewTable
.L1:    cmp     al,3Ch
        jne     .L2
        FetchB
        mov     [I.SegmName],TES
        cmp     al,$C0
        jae     .V9
        cmp     al,$3F
        jb      .V9
        mov     [I.SegmName],TSS
        cmp     al,7Fh
        ja      .V9
        mov     [I.SegmName],TCS
      .V9:
        mov     [I.CurSeg],0xFF
        and     al,8
        add     al,$D0
        pop     rdx
        jmp     DisAsm.NewTable
.L2:    cmp     al,3Dh
        jne     .L3
        mov     [I.Name],TWAIT
        ret
.L3:    cmp     al,3Eh
        jne     .L4
        FetchW
        mov     al,3Eh
        jmp     .L4

RM81:   call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM

        mov     word [I.Arg2.Type],Is1 shl 8+80h+1
        ret

RM161:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        mov     word [I.Arg2.Type],Is1 shl 8+80h+1
        ret

RM321:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        mov     word [I.Arg2.Type],Is1 shl 8+80h+1
        ret

RM641:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        mov     word [I.Arg2.Type],Is1 shl 8+80h+1
        ret

RM8CL:  call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR08
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Reg],RRCX
        ret

RM16CL: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Reg],RRCX
        ret

RM32CL: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR32
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Reg],RRCX
        ret

RM64CL: call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InRM

        mov     [I.Arg2.Type],GPR08
        mov     [I.Arg2.Reg],RRCX
        ret

LoseByte:
        FetchB
        cmp     al,0Ah
        je      .Default
        mov     [I.Arg1.Type],80h+1
        mov     [I.Arg1.ImmB],al
.Default:
        ret

RtnD7:
        test    byte [I.Prefixes],bit PSEG+bit P67
        jz      .SkipATT

        BitTR   I.Prefixes,P67

        mov     [I.Name],TXLAT

        mov     [I.RandSize],VBYTE

        mov     [I.Base],RRBX

        mov     [I.Arg1.Type],1
        mov     [I.Arg1.Mem],True

        call    ClearSeg
.SkipATT:
        ret

RtnE0:  push    DODISPB
        mov     al,[I.Arch]
        cmp     al,CPUX32
        jz      .32
.16:
.64:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     dword [I.Name],TLOOPNZL shl 16+TLOOPNZD
      @@:
        ret
.32:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TLOOPNZW
      @@:
        ret

RtnE1:  push    DODISPB
        mov     al,[I.Arch]
        cmp     al,CPUX32
        jz      .32
.16:
.64:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     dword [I.Name],TLOOPZL shl 16+TLOOPZD
      @@:
        ret
.32:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TLOOPZW
      @@:
        ret

RtnE2:  push    DODISPB
        mov     al,[I.Arch]
        cmp     al,CPUX32
        jz      .32
.16:
.64:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     dword [I.Name],TLOOPL shl 16+TLOOPD
      @@:
        ret
.32:
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TLOOPW
      @@:
        ret

RtnE3:  push    DODISPB
        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
        cmp     al,CPUX32
        jz      .32
.64:
        mov     [I.Name],TJRCXZ
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TJECXZ
      @@:
        ret
.32:
        mov     [I.Name],TJECXZ
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TJCXZ
      @@:
        ret
.16:
        mov     [I.Name],TJCXZ
        BitTR   I.Prefixes,P67
        jnc     @F
        mov     [I.Name],TJECXZ
      @@:
        ret

RtnE4:  mov     [I.Arg1.Type],GPR08
RtnEXVV:
        FetchB
        mov     [I.Arg2.Type],80h+1
        mov     [I.Arg2.ImmB],al
        ret

RtnE516:mov     [I.Arg1.Type],GPR16
        jmp     RtnEXVV

RtnE532:mov     [I.Arg1.Type],GPR32
        jmp     RtnEXVV

RtnEC:  mov     [I.Arg1.Type],GPR08
RtnEXXX:mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Reg],RRDX
        ret

RtnED16:mov     [I.Arg1.Type],GPR16
        jmp     RtnEXXX

RtnED32:mov     [I.Arg1.Type],GPR32
        jmp     RtnEXXX

RtnE6:  FetchB
        mov     [I.Arg2.Type],GPR08
RtnEXYY:mov     [I.Arg1.Type],80h+1
        mov     [I.Arg1.ImmB],al
        ret

RtnE716:FetchB
        mov     [I.Arg2.Type],GPR16
        jmp     RtnEXYY

RtnE732:FetchB
        mov     [I.Arg2.Type],GPR32
        jmp     RtnEXYY

RtnEE:  mov     [I.Arg2.Type],GPR08
RtnEXZZ:
        mov     [I.Arg1.Type],GPR16
        mov     [I.Arg1.Reg],RRDX
        ret

RtnEF16:mov     [I.Arg2.Type],GPR16
        jmp     RtnEXZZ

RtnEF32:mov     [I.Arg2.Type],GPR32
        jmp     RtnEXZZ

ESC_0_000:
ESC_0_001:
ESC_0_010:
ESC_0_011:
ESC_0_100:
ESC_0_101:
ESC_0_110:
ESC_0_111:
        call    FetchModRM

        lea     r8,[I.Arg1]

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0

        cmp     [I.Reg],2
        jz      .Exit
        cmp     [I.Reg],3
        jz      .Exit

        lea     r8,[I.Arg2]
        mov     [r8+TArg.Type],STXRG
.Exit:
        mov     [r8+TArg.Type2nd],InRM
        ret

ESC_1_000:
        call    FetchModRM

        mov     [I.Arg1.Type2nd],InRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0
.Exit:
        ret

ESC_1_001:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     ErrorDec

        mov     [I.RandSize],0

        mov     [I.Arg1.Type2nd],InRM
        ret

ESC_1_010:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0

        cmp     byte [rsi-1],0xD0
        jnz     ErrorDec
        mov     [I.Arg1.Type],0
        mov     [I.Name],TFNOP
.Exit:
        ret

ESC_1_011:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0

        mov     [I.Name],TFSTP1

        mov     [I.Arg1.Type2nd],InRM
.Exit:
        ret

ESC_1_100:
        call    FetchModRM

        mov     [I.Name],TFLDENV

        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
.64:
.32:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     dword [I.Name],TFLDENVS shl 16+TFLDENVW
      @@:
        jmp     .XX
.16:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     dword [I.Name],TFLDENVS shl 16+TFLDENVD
      @@:

.XX:    cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     al,[I.RM]
        mov     [I.Name],TFCHS
        or      al,al
        jz      .Quit
        mov     [I.Name],TFABS
        cmp     al,1
        jz      .Quit
        mov     [I.Name],TFTST
        cmp     al,4
        jz      .Quit
        mov     [I.Name],TFXAM
        cmp     al,5
        jnz     ErrorDec
.Quit:  mov     [I.Arg1.Type],0
.Exit:  ret

ESC_1_101:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.Arg1.Type],0
        movzx   eax,[I.RM]
        mov     ax,[TFLDXTable+rax*2]
        or      eax,eax
        jz      ErrorDec
        mov     [I.Name],ax
.Exit:
        ret

ESC_1_110:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jz      .Mod3
if %B=64
        mov     rdx,(TFSTENV shl 16+TFNSTENV)shl 32+(TFSTENV shl 16+TFNSTENV)
else
        mov     edx,(TFSTENV shl 16+TFNSTENV)
        mov     edi,(TFSTENV shl 16+TFNSTENV)
end if
        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
.64:
.32:
        BitTR   I.Prefixes,P66
        jnc     @F
if %B=64
        mov     rdx,(TFSTENVS shl 16+TFNSTENVS)shl 32+(TFSTENVW shl 16+TFNSTENVW)
else
        mov     edx,(TFSTENVW shl 16+TFNSTENVW)
        mov     edi,(TFSTENVS shl 16+TFNSTENVS)
end if
      @@:
        jmp     .XX
.16:
        BitTR   I.Prefixes,P66
        jnc     @F
if %B=64
        mov     rdx,(TFSTENVS shl 16+TFNSTENVS)shl 32+(TFSTENVD shl 16+TFNSTENVD)
else
        mov     edx,(TFSTENVD shl 16+TFNSTENVD)
        mov     edi,(TFSTENVS shl 16+TFNSTENVS)
end if
      @@:

.XX:
        cmp     [I.Syntax],0
        jz      .YY
if %B=64
        rol     rdx,32
else
        mov     edx,edi
end if
.YY:
        BitTR   I.Prefixes,P9B
        jnc     @F
        rol     edx,16
      @@:
        mov     [I.Name],dx
        ret

.Mod3:
        mov     [I.Arg1.Type],0
        movzx   eax,[I.RM]
        mov     ax,[TE110Table+rax*2]
        mov     [I.Name],ax

.Exit:  ret

ESC_1_111:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jz      .Mod3

        mov     ax,TFNSTCW
        BitTR   I.Prefixes,P9B
        jnc     @F
        mov     ax,TFSTCW
      @@:
        jmp     .Name
.Mod3:
        mov     [I.Arg1.Type],0
        movzx   eax,[I.RM]
        mov     ax,[TE111Table+rax*2]
.Name:
        mov     [I.Name],ax
        ret

ESC_2_000:
        mov     di,TFCMOVB
        jmp     ESC_2_XXX
ESC_2_001:
        mov     di,TFCMOVE
        jmp     ESC_2_XXX
ESC_2_010:
        mov     di,TFCMOVBE
        jmp     ESC_2_XXX
ESC_2_011:
        mov     di,TFCMOVU
        jmp     ESC_2_XXX
ESC_2_100:
ESC_2_101:
ESC_2_110:
ESC_2_111:
        xor     edi,edi
ESC_2_XXX:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0

        mov     [I.Name],TFUCOMPP
        cmp     byte [rsi-1],0xE9
        jz      .Quit

        or      di,di
        jz      ErrorDec
        mov     [I.Name],di
        mov     [I.Arg2.Type],STXRG
        mov     [I.Arg2.Type2nd],InRM
.Exit:  ret
.Quit:
        mov     [I.Arg1.Type],0
        ret

ESC_3_000:
        mov     di,TFCMOVNB
        jmp     ESC_3_XXX
ESC_3_001:
        mov     di,TFCMOVNE
        jmp     ESC_3_XXX
ESC_3_010:
        mov     di,TFCMOVNBE
        jmp     ESC_3_XXX
ESC_3_011:
        mov     di,TFCMOVNU
ESC_3_XXX:
ESC_3_YYY:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.RandSize],0

        or      di,di
        jz      ErrorDec
        mov     [I.Name],di
        mov     [I.Arg2.Type],STXRG
        mov     [I.Arg2.Type2nd],InRM
.Exit:
        ret

ESC_3_100:
        call    FetchModRM
        cmp     [I.Mod],3
        jnz     ErrorDec
        mov     al,[I.RM]
        mov     edx,TFENI shl 16 +TFNENI
        or      al,al
        jz      .L1
        mov     edx,TFDISI shl 16 +TFNDISI
        dec     al
        jz      .L1
        mov     edx,TFCLEX shl 16 +TFNCLEX
        dec     al
        jz      .L1
        mov     edx,TFINIT shl 16 +TFNINIT
        dec     al
        jz      .L1
        mov     dx,TFSETPM
        dec     al
        jz      .L2
        mov     dx,TFRSTPM
        dec     al
        jnz     ErrorDec
        jmp     .L2
.L1:    BitTR   I.Prefixes,P9B
        jnc     .L2
        rol     edx,16
.L2:    mov     [I.Name],dx
        ret

ESC_3_101:
        mov     di,TFUCOMI
        mov     [I.IsFloat],1
        test    [I.Sensitive],NIA
        jz      ESC_3_YYY
        mov     [I.RandSize],0
        call    FetchModRM
        cmp     [I.Mod],3
        jnz     ErrorDec
        mov     al,[I.RM]
        mov     dx,TFSTB0
        or      al,al
        jz      .L1
        dec     al
        jz      ErrorDec
        mov     dx,TFSTB2
        dec     al
        jz      .L1
        mov     dx,TFSTB1
        dec     al
        jnz     ErrorDec
.L1:    mov     [I.Name],dx
        ret

ESC_3_110:
        call    FetchModRM
        cmp     [I.Mod],3
        jnz     ErrorDec

        test    [I.Sensitive],NIA
        jz      .YIA

        cmp     [I.RM],1
        jnz     ErrorDec
        mov     [I.Name],TF4X4
        ret
.YIA:
        mov     [I.Arg1.Type],STXRG
        mov     [I.Arg2.Type],STXRG
        mov     [I.Arg2.Type2nd],InRM
        ret

ESC_3_111:
        call    FetchModRM
        test    [I.Sensitive],NIA
        jz      .YIA

        cmp     [I.Mod],3
        jnz     ErrorDec
        cmp     [I.RM],4
        jnz     ErrorDec
        mov     [I.Name],TFRINT2
        ret
.YIA:
        cmp     [I.Mod],3
        jz      ErrorDec
        mov     [I.IsFloat],1
        mov     [I.Arg1.Mem],True
        mov     [I.Arg1.Type],STXRG
        mov     [I.RandSize],VTWORD
        ret

ESC_4_010:
        mov     edi,TFCOM2 shl 16+TFCOM2
        jmp     ESC_4_XXX
ESC_4_011:
        mov     edi,TFCOMP3 shl 16+TFCOMP3
        jmp     ESC_4_XXX
ESC_4_000:
ESC_4_001:
        xor     edi,edi
        jmp     ESC_4_XXX
ESC_4_100:
        mov     edi,TFSUB shl 16+TFSUBR
        jmp     ESC_4_XXX
ESC_4_101:
        mov     edi,TFSUBR shl 16+TFSUB
        jmp     ESC_4_XXX
ESC_4_110:
        mov     edi,TFDIV shl 16+TFDIVR
        jmp     ESC_4_XXX
ESC_4_111:
        mov     edi,TFDIVR shl 16+TFDIV
ESC_4_XXX:
        call    FetchModRM

        mov     [I.Arg1.Type2nd],InRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        or      edi,edi
        jz      .Null
        mov     dword [I.Name],edi
.Null:

        mov     [I.RandSize],0

        cmp     [I.Reg],2
        jz      .Exit
        cmp     [I.Reg],3
        jz      .Exit

        mov     [I.Arg2.Type],STXRG
.Exit:  ret

ESC_5_001:
        mov     di,TFXCH4
        jmp     ESC_5_XXX
ESC_5_000:
        mov     di,TFFREE
        jmp     ESC_5_XXX
ESC_5_010:
ESC_5_011:
        xor     edi,edi
ESC_5_XXX:
        call    FetchModRM
ESC_5_YYY:

        mov     [I.Arg1.Type2nd],InRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        or      di,di
        jz      .Null
        mov     [I.Name],di
.Null:

        mov     [I.RandSize],0
.Exit:  ret


ESC_5_100:
        mov     di,TFUCOM
        call    FetchModRM
        cmp     [I.Mod],3
        jz      ESC_5_YYY

        mov     [I.Name],TFRSTOR

        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
.64:
.32:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     dword [I.Name],TFRSTORS shl 16+TFRSTORW
      @@:
        jmp     .XX
.16:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     dword [I.Name],TFRSTORS shl 16+TFRSTORD
      @@:

.XX:
        mov     [I.Arg1.Mem],True
        mov     [I.Arg1.Type],STXRG
        ret

ESC_5_101:
        call    FetchModRM
        cmp     [I.Mod],3
        jnz     ErrorDec

        mov     [I.Name],TFUCOMP

        mov     [I.Arg1.Type2nd],InRM
        mov     [I.Arg1.Type],STXRG
        ret

ESC_5_110:
        call    FetchModRM
        cmp     [I.Mod],3
        jz      ErrorDec

        mov     edi,TFSAVE shl 16+TFNSAVE

        mov     al,[I.Arch]
        cmp     al,CPUX16
        jz      .16
.64:
.32:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     edi,TFSAVEW shl 16+TFNSAVEW
      @@:
        jmp     .XX
.16:
        BitTR   I.Prefixes,P66
        jnc     @F
        mov     edi,TFSAVED shl 16+TFNSAVED
      @@:
.XX:
        BitTR   I.Prefixes,P9B
        jnc     .YY
        rol     edi,16
.YY:
        mov     [I.Name],di
        mov     [I.Arg1.Mem],True
        mov     [I.Arg1.Type],STXRG
        ret

ESC_5_111:
        call    FetchModRM

        test    [I.Sensitive],NIA
        jnz     .XX

        cmp     [I.Mod],3
        jz      ErrorDec

        mov     edi,TFSTSW shl 16+TFNSTSW
        BitTR   I.Prefixes,P9B
        jnc     .YY
        rol     edi,16
.YY:
        mov     [I.Name],di
        mov     [I.Arg1.Mem],True
        mov     [I.Arg1.Type],STXRG
        ret
.XX:    cmp     [I.Mod],3
        jnz     ErrorDec
        cmp     byte [rsi-1],0xFC
        jnz     ErrorDec
        mov     [I.Name],TFRICHOP
        ret

ESC_6_111:
        mov     edi,TFDIVRP shl 16+TFDIVP
        jmp     ESC_6_XXX
ESC_6_110:
        mov     edi,TFDIVP shl 16+TFDIVRP
        jmp     ESC_6_XXX
ESC_6_101:
        mov     edi,TFSUBRP shl 16+TFSUBP
        jmp     ESC_6_XXX
ESC_6_100:
        mov     edi,TFSUBP shl 16+TFSUBRP
        jmp     ESC_6_XXX
ESC_6_010:
        mov     edi,TFCOMP5 shl 16+TFCOMP5
        jmp     ESC_7_XXX       ;Yes = ESC_7_XXX
ESC_6_001:
        mov     edi,TFMULP shl 16+TFMULP
        jmp     ESC_6_XXX
ESC_6_000:
        mov     edi,TFADDP shl 16+TFADDP
        jmp     ESC_6_XXX
ESC_6_011:
        xor     edi,edi
ESC_6_XXX:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.Arg1.Type2nd],InRM

        mov     [I.RandSize],0

        cmp     byte [rsi-1],0xD9
        jz      .Quit

        or      edi,edi
        jz      .Null
        mov     dword [I.Name],edi
.Null:
        mov     [I.Arg2.Type],STXRG
.Exit:
        ret
.Quit:
        mov     [I.Name],TFCOMPP
        mov     [I.Arg1.Type],0
        ret

ESC_7_011:
        mov     di,TFSTP9
        jmp     ESC_7_XXX
ESC_7_010:
        mov     di,TFSTP8
        jmp     ESC_7_XXX
ESC_7_001:
        mov     di,TFXCH7
        jmp     ESC_7_XXX
ESC_7_000:
        mov     di,TFFREEP
ESC_7_XXX:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.Arg1.Type2nd],InRM

        mov     [I.RandSize],0

        or      di,di
        jz      .Null
        mov     [I.Name],di
.Null:

.Exit:
        ret

ESC_7_100:
        call    FetchModRM

        cmp     [I.Syntax],0
        jz      .SkipATT
        mov     [I.RandSize],0
.SkipATT:

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     al,[I.Reg]

        mov     edx,TFSTSW shl 16 +TFNSTSW
        or      al,al
        jz      .L1

        test    [I.Sensitive],UND
        jz      .L1

        mov     edx,TFSTDW shl 16 +TFNSTDW
        cmp     al,1
        jz      .L1

        mov     edx,TFSTSG shl 16 +TFNSTSG
        cmp     al,2
.L1:
        jnz     ErrorDec

        BitTR   I.Prefixes,P9B
        jnc     .L2
        rol     edx,16
.L2:    mov     [I.Name],dx

        mov     [I.Arg1.Type],GPR16
        mov     [I.RandSize],0
.Exit:
        ret

ESC_7_110:
        cmp     [I.Syntax],0
        jz      .SkipATT
        mov     [I.RandSize],0
.SkipATT:

        mov     di,TFCOMIP
        jmp     ESC_7_YYY
ESC_7_101:
        mov     [I.IsFloat],0
        mov     di,TFUCOMIP
ESC_7_YYY:
        call    FetchModRM

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        mov     [I.Name],di

        mov     [I.Arg2.Type2nd],InRM
        mov     [I.Arg2.Type],STXRG

        mov     [I.RandSize],0
.Exit:
        ret

ESC_7_111:
        call    FetchModRM

        mov     [I.IsFloat],0

        cmp     [I.Mod],3
        setnz   [I.Arg1.Mem]
        mov     [I.Arg1.Type],STXRG
        jnz     .Exit

        test    [I.Sensitive],NIA
        jz      ErrorDec
        cmp     byte [rsi-1],0xFC
        jnz     ErrorDec

        mov     [I.Name],TFRINEAR
        mov     [I.Arg1.Type],0

        mov     [I.RandSize],0
.Exit:
        ret

DODISPBS:
        mov     [I.IsShort],True
DODISPB:
        mov     [I.IsAddress],True
if %B=64
        FetchSB
        add     rax,[Origin]
        add     rax,rsi
        sub     rax,[I.Addr]

        mov     cl,[I.Arch]
        or      cl,cl
        jnz     @F
        movzx   eax,ax
      @@:
        cmp     cl,CPUX32
        jnz     @F
        mov     eax,eax
      @@:

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],rax
        ret
else
        FetchSB
        cdq
        add     eax,[Origin]
        adc     edx,0
        add     eax,esi
        adc     edx,0
        sub     eax,[I.Addr]
        sbb     edx,0

        mov     cl,[I.Arch]
        or      cl,cl
        jnz     @F
        xor     edx,edx
        movzx   eax,ax
      @@:
        cmp     cl,CPUX32
        jnz     @F
        xor     edx,edx
      @@:

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],eax
        mov     [I.Arg1.Imm+4],edx
        ret
end if
DODISPW:
        mov     [I.IsAddress],True
        mov     [I.RandSize],VWORD
if %B=64
        FetchSW
        add     rax,[Origin]
        add     rax,rsi
        sub     rax,[I.Addr]

        movzx   eax,ax

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],rax
        ret
else
        FetchSW
        cdq
        add     eax,[Origin]
        adc     edx,0
        add     eax,esi
        adc     edx,0
        sub     eax,[I.Addr]
        sbb     edx,0

        xor     edx,edx
        movzx   eax,ax

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],eax
        mov     [I.Arg1.Imm+4],edx
        ret
end if
DODISPD:
        mov     [I.IsAddress],True
if %B=64
        FetchSD
        add     rax,[Origin]
        add     rax,rsi
        sub     rax,[I.Addr]

        cmp     [I.Arch],CPUX64
        jz      @F
        mov     eax,eax
      @@:

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],rax
        ret
else
        FetchSD
        cdq
        add     eax,[Origin]
        adc     edx,0
        add     eax,esi
        adc     edx,0
        sub     eax,[I.Addr]
        sbb     edx,0

        cmp     [I.Arch],CPUX64
        jz      @F
        xor     edx,edx
      @@:

        mov     [I.Arg1.Type],80h+8
        mov     [I.Arg1.Imm],eax
        mov     [I.Arg1.Imm+4],edx
        ret
end if
RtnArgQ:
        mov     [I.Arg1.Type],GPR64
RtnArgX:
        mov     al,[I.B]
        add     [I.Arg1.Reg],al
        call    ClearB
        ret

RtnArgD:
        mov     [I.Arg1.Type],GPR32
        jmp     RtnArgX

RtnArgW:
        mov     [I.Arg1.Type],GPR16
        jmp     RtnArgX

RndSizeD60:
        mov     [I.AltName],TPUSHA
        ret
RndSizeD61:
        mov     [I.AltName],TPOPA
        ret

AddPointD:
        mov     ax,TDWORD
        jmp     AddPointCommon
AddPointW:
        mov     ax,TWORD
AddPointCommon:
        BitTR   I.Prefixes,P66
        jnc     .Skip66
        mov     [I.Point],ax
.Skip66:
        mov     [I.Arg1.Type],SEGRG
        ret

RtnMOVSXD:
        mov     dword [I.Name],TMOVSLQ shl 16+TMOVSXD
        cmp     [I.W],0
        jnz     RtnMOVZXD.Continue
RtnMOVZXD:
        mov     dword [I.Name],TMOVZLQ shl 16+TMOVZXD
        BitTR   I.Prefixes,P66
        jnc     .Continue
        mov     dword [I.Name],TMOVZWQ shl 16+TMOVZXDW

        call    FetchModRM

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR16
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg
        ret

.Continue:
        call    FetchModRM

        cmp     [I.Syntax],0
        jnz     @F
        mov     [I.RandSize],VDWORD
      @@:

       ;cmp     [I.Mod],3
       ;setnz   [I.Arg2.Mem]
        mov     [I.Arg2.Type],GPR32
        mov     [I.Arg2.Type2nd],InRM

        mov     [I.Arg1.Type],GPR64
        mov     [I.Arg1.Type2nd],InReg
        ret

PushI16:
        FetchW
        mov     [I.Arg1.Type],80h+2
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TWORD
        ret

PushI32:
        FetchSD
        mov     [I.Arg1.Type],80h+4
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TDWORD
        ret

PushI64:
        FetchSD
        mov     [I.Arg1.Type],80h+40h+4
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TQWORD
        ret

PushSI16:
        FetchSB
        mov     [I.Arg1.Type],80h+40h+2
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TWORD
        ret

PushSI32:
        FetchSB
        mov     [I.Arg1.Type],80h+40h+4
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TDWORD
        ret

PushSI64:
        FetchSB
        mov     [I.Arg1.Type],80h+40h+4
        mov     [I.Arg1.Imm],rax

        mov     [I.Point],TQWORD
        ret

include "rtn-c4c5.inc"
include "rtn-62xx.inc"
include "far-code.inc"

ClearSeg:
        cmp     [I.IsLEA],0
        jnz     .NoClear
        mov     ax,[I.SegmName]
        cmp     [I.Arch],CPUX64
        jnz     .Clear
        cmp     ax,TFS
        jz      .Clear
        cmp     ax,TGS
        jz      .Clear
.NoClear:
        ret
.Clear: BitTR   I.Prefixes,PSEG

ModRM2EA.Exit:
        ret

ModRM2EA:
        cmp     [I.Mod],3
        jz      .Exit

        mov     [I.DefSeg],VDS

        call    ClearSeg
        BitTR   I.Prefixes,P67

        mov     [I.PossibleLOCK],True

        cmp     [I.Address],AD16
        jz      .Address16
.SkipJmp:
        mov     al,[I.RM]
        add     al,[I.B]

        cmp     al,04h
        jz      .J4
        cmp     al,0Ch
        jz      .JC
        cmp     al,05h
        jz      .J5
        cmp     al,0Dh
        jz      .JD

        mov     [I.Base],al
        jmp     .AddressSizeCont

.JC:
.J4:    call    SIBByte
        jmp     .AddressSizeCont
.JD:
.J5:    cmp     [I.Mod],0
        jne     .ModeX
        mov     [I.DispSize],4
        FetchSD
        mov     [I.Disp],rax
        cmp     [I.Arch],CPUX64
        jnz     .No64X
        mov     [I.Relative],True
.No64X:
        jmp     .AddressSizeCont
.ModeX:
        mov     [I.Base],al
        mov     [I.DefSeg],VSS
.AddressSizeCont:
        mov     al,[I.Mod]
        cmp     al,1
        jne     .ModeNEQ1
        mov     [I.DispSize],1
        FetchSB
        mov     [I.Disp],rax
        jmp     .ModeNEQ2
.ModeNEQ1:
        cmp     al,2
        jne     .ModeNEQ2
        mov     [I.DispSize],4
        FetchSD
        mov     [I.Disp],rax
.ModeNEQ2:

        cmp     [I.Base],-1
        jz      .NoBase
        mov     al,[I.NotB]
        and     byte [I.Prefixes],al
.NoBase:
        cmp     [I.Indx],-1
        jz      .NoIndx
        mov     al,[I.NotX]
        and     byte [I.Prefixes],al
.NoIndx:
        ret

.Jump16:dd      .I0,.I1,.I2,.I3,.I4,.I5,.I6,.I7

.Address16:
        movzx   eax,[I.RM]
        mov     eax,[rax*4+.Jump16]
        jmp     rax
.I0:
        mov     [I.Base],RRBX
        mov     [I.Indx],RRSI
        jmp     .AddressSizeCont0
.I1:
        mov     [I.Base],RRBX
        mov     [I.Indx],RRDI
        jmp     .AddressSizeCont0
.I2:
        mov     [I.Base],RRBP
        mov     [I.Indx],RRSI
        mov     [I.DefSeg],VSS
        jmp     .AddressSizeCont0
.I3:
        mov     [I.Base],RRBP
        mov     [I.Indx],RRDI
        mov     [I.DefSeg],VSS
        jmp     .AddressSizeCont0
.I4:
        mov     [I.Base],RRSI
        jmp     .AddressSizeCont0
.I5:
        mov     [I.Base],RRDI
        jmp     .AddressSizeCont0
.I6:
        cmp     [I.Mod],0
        jne     .ModeY
        mov     [I.DispSize],2
        FetchSW
        mov     [I.Disp],rax
        jmp     .AddressSizeCont0
.ModeY:
        mov     [I.Base],RRBP
        mov     [I.DefSeg],VSS
        jmp     .AddressSizeCont0
.I7:
        mov     [I.Base],RRBX
.AddressSizeCont0:
        mov     al,[I.Mod]
        cmp     al,1
        jne     .NotModeEQ1
        mov     [I.DispSize],1
        FetchSB
        mov     [I.Disp],rax
        ret
.NotModeEQ1:
        cmp     al,2
        jne     .NotModeEQ2
        mov     [I.DispSize],2
        FetchSW
        mov     [I.Disp],rax
.NotModeEQ2:
        ret

SIBByte:
        FetchB

        mov     [I.SIB],al
        shr     al,6
        mov     [I.Scale],al

        mov     al,[I.SIB]
        shr     al,3
        and     al,7                    ;Index
        add     al,[I.X]
        cmp     al,4
        jz      .NoIndx
        mov     [I.Indx],al
.NoIndx:
        mov     al,[I.SIB]
        and     al,7                    ;Base
        add     al,[I.B]

        cmp     al,05h
        jz      .X5
        cmp     al,0Dh
        jz      .XD

        mov     [I.Base],al

        cmp     al,04h
        jz      .X4

        ret
.XD:
        cmp     [I.Mod],0
        jne     .ModeNX
.LD:    mov     [I.DispSize],4
        FetchSD
        mov     [I.Disp],rax
        ret
.ModeNX:
        mov     [I.Base],al
        ret
.X5:
        cmp     [I.Mod],0
        jz      .LD
        mov     [I.Base],RRBP
.X4:    mov     [I.DefSeg],VSS
        ret

Hex16:  mov     cl,16
        jmp     Hex
Hex8:   mov     cl,8
Hex:    movzx   ecx,cl
        jrcxz   .L2
if ~OS
        push    rdi
        call    FindEAX
        pop     rdi
        jc      .No
        add     rdi,rcx
        ret
.No:
end if
        push    rcx
        push    rdx
        xchg    rdx,rax
.L1:    movzx   eax,dl
        shr     rdx,8
        mov     ax,[rax*2+HexString]
        sub     cl,2
        mov     [rdi+rcx],ax
        ja      .L1
        pop     rdx
        pop     rcx
        add     rdi,rcx
.L2:    ret

if %B=32
Bin2Hex32:
        or      eax,eax
        jnz     Bin2Hex
        mov     cl,0
        jmp     Bin2Hex
end if

Bin2Hex16:
        mov     cl,16
        jmp     Bin2Hex
Bin2Hex8:
        mov     cl,8
        jmp     Bin2Hex
;
Bin2Hex4:
        mov     cl,4
        jmp     Bin2Hex
;
Bin2Hex2:
        mov     cl,2
;
Bin2Hex:
if ~OS
        call    FindEAX
        jc      .No
        ret
.No:
end if
        push    rcx
        cmp     cl,16
        ja      .L0
        push    rdx
        cmp     [I.FullHex],True
        je      .L3
        cmp     rax,$FF
        ja      .SB
        cmp     cl,2
        jb      .L3
        mov     cl,2
        jmp     .L3
.SB:
        cmp     rax,$FFFF
        ja      .SW
        cmp     cl,4
        jb      .L3
        mov     cl,4
        jmp     .L3
.SW:
        cmp     rax,$FFFFFF
        ja      .SF
        cmp     cl,6
        jb      .L3
        mov     cl,6
        jmp     .L3
.SF:
        or      edx,$FFFFFFFF
        cmp     rax,rdx
        ja      .SD
        cmp     cl,8
        jb      .L3
        mov     cl,8
        jmp     .L3
.SD:

.L3:
if 1
        cmp     cl,16
        jnz     .L4
        or      edx,$FFFFFFFF
        cmp     rax,rdx
        ja      .L4
        mov     cl,8
.L4:
end if
        movzx   ecx,cl
        push    rdi rcx

        cmp     [I.HexPrefix],0
        je      @F
if 1
        cmp     byte [rdi-1],'L'
        jz      .L2
end if
        mov     word [rdi],'0x'
        scasw
        jmp     .L2
@@:
        cmp     [I.Dollar],0
        jne     .L2
if 1
        cmp     byte [rdi-1],'L'
        jz      .L2
end if
        mov     byte [rdi],'$'
        scasb
.L2:    xchg    rdx,rax
.L1:    movzx   eax,dl
        shr     rdx,8
        mov     ax,[rax*2+HexString]
        sub     cl,2
        mov     [rdi+rcx],ax
        ja      .L1
        pop     rcx rdi rdx
        add     rdi,rcx

        cmp     [I.HexPrefix],0
        je      @F
if 1
        neg     rcx
        cmp     byte [rdi+rcx-1],'L'
        jz      .L0
end if
        scasw
        jmp     .L0
@@:
        cmp     [I.Dollar],0
        jne     .L0
if 1
        neg     rcx
        cmp     byte [rdi+rcx-1],'L'
        jz      .L0
end if
        scasb
.L0:    pop     rcx
        ret

Start66F2F3:
        FetchB
        dec     rsi

        xor     ebx,ebx
        test    [I.Flags],_3
        jz      @F
        mov     bl,2*4
@@:
        mov     ecx,[rdx+rbx+4*2]

        cmp     al,0xC0
        jb      @F
        test    [I.Flags],_3
        jz      @F
        add     edx,ebx
        xor     ebx,ebx
@@:
        mov     al,[I.PrefixByte]
        or      al,al
        jz      .L1
        shr     ecx,8
        add     edx,2
        cmp     al,bit P66
        setz    [I.Mandatory66]
        jz      .L1
        shr     ecx,8
        add     edx,2
        cmp     al,bit PF2
        jz      .L1
        shr     ecx,8
        add     edx,2
        cmp     al,bit PF3
        jz      .L1
        jmp     ErrorDec
.L1:    mov     [I.RandSize],cl
        ret

include "selbat.inc"
include "tables.inc"

        CpyElements NextTab

vglobal
Params  rd      ParamSize/4     ;Parameters buffer
BufferI rb      BufferSizeI
Buffer  rb      BufferSizeO
endg

if %B=32
r9v     dd      ?
r5v     dd      ?
r1v     dd      ?
end if
LastHex db      ?

rept 16
{
restore r8b,r8w,r8d,r8
restore r9b,r9w,r9d,r9
restore r10b,r10w,r10d,r10
restore r11b,r11w,r11d,r11
restore r12b,r12w,r12d,r12
restore r13b,r13w,r13d,r13
restore r14b,r14w,r14d,r14
restore r15b,r15w,r15d,r15
}