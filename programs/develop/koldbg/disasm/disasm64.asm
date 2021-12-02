
BufferSizeI     =       1k
BufferSizeO     =       1k

use64
InitDefault:
        mov     [DefInst.Arch],CPUX64
        mov     [DefInst.Size],15
        mov     [DefInst.Negate],True

        mov     [DefInst.HideRIP],True

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
Origin          dq 0
TheBufferO      dq 0
