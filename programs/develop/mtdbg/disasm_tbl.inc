disasm_table_1:
        dd      cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0           ; 0x
        dd      cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cF
        dd      cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0           ; 1x
        dd      cop22, cop22, cop22, cop22, cop21, cop21, cop0,  cop0
        dd      cop22, cop22, cop22, cop22, cop21, cop21, cseges,cop0           ; 2x
        dd      cop22, cop22, cop22, cop22, cop21, cop21, csegcs,cop0
        dd      cop22, cop22, cop22, cop22, cop21, cop21, csegss,cop0           ; 3x
        dd      cop22, cop22, cop22, cop22, cop21, cop21, csegds,cop0
        dd      cinc1, cinc1, cinc1, cinc1, cinc1, cinc1, cinc1, cinc1          ; 4x
        dd      cdec1, cdec1, cdec1, cdec1, cdec1, cdec1, cdec1, cdec1
        dd      cpush1,cpush1,cpush1,cpush1,cpush1,cpush1,cpush1,cpush1         ; 5x
        dd      cpop1, cpop1, cpop1, cpop1, cpop1, cpop1, cpop1, cpop1
        dd      cop0,  cop0,  cbound,carpl, csegfs,cseggs,c66,   c67            ; 6x
        dd      cpush21,cimul1,cpush22,cimul1,cunk,cunk,  cunk,  cunk
        dd      cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1          ; 7x
        dd      cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1, cjcc1
        dd      cop23, cop23, cop23, cop23, cop22, cop22, cop22, cop22          ; 8x
        dd      cop22, cop22, cop22, cop22, cunk,  cop22, cunk,  cpop2
        dd      cop0,  cxchg1,cxchg1,cxchg1,cxchg1,cxchg1,cxchg1,cxchg1         ; 9x
        dd      ccbw,  ccwd,  ccallf,cop0,  cop0,  cop0,  cop0,  cop0
        dd      cmov3, cmov3, cmov3, cmov3, cop0,  cop0,  cop0,  cop0           ; Ax
        dd      cop21, cop21, cop0,  cop0,  cop0,  cop0,  cop0,  cop0
        dd      cmov11,cmov11,cmov11,cmov11,cmov11,cmov11,cmov11,cmov11         ; Bx
        dd      cmov12,cmov12,cmov12,cmov12,cmov12,cmov12,cmov12,cmov12
        dd      cshift1,cshift1,cret2,cop0, cunk,  cunk,  cmov2, cmov2          ; Cx
        dd      center,cop0,  cunk,  cunk,  cop0,  cint,  cunk,  cunk
        dd      cshift2,cshift2,cshift3,cshift3,caam,caad,cunk,  cxlat          ; Dx
        dd      cD8,   cD9,   cDA,   cDB,   cDC,   cDD,   cDE,   cDF
        dd      cloopnz,cloopz,cloop,cjcxz, cunk,  cunk,  cunk,  cunk           ; Ex
        dd      ccall1,cjmp1, cunk,  cjmp2, cunk,  cunk,  cunk,  cunk
        dd      clock, cunk,  crepnz,crep,  cunk,  cop0,  cop1,  cop1           ; Fx
        dd      cop0,  cop0,  cop0,  cop0,  cop0,  cop0,  cop1,  cop1

disasm_table_2:
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cop0_F,cop0_F,cunk           ; 0x
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk           ; 1x
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk           ; 2x
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  csse1
        dd      cunk,  crdtsc,cunk,  cunk,  cop0_F,cunk,  cunk,  cunk           ; 3x
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
        dd      cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc         ; 4x
        dd      cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc,cmovcc
        dd      cunk,  cunk,  cunk,  cunk,  csse1, csse1, cunk,  cunk           ; 5x
        dd      csse1, cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cunk
        dd      cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn          ; 6x
        dd      cpcmn, cpcmn, cpcmn, cpcmn, cunk,  cunk,  cmovd1,cmovq1
        dd      cunk,  cpshift,cpshift,cpshift,cpcmn,cpcmn,cpcmn,cemms          ; 7x
        dd      cunk,  cunk,  cunk,  cunk,  cunk,  cunk,  cmovd2,cmovq2
        dd      cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2          ; 8x
        dd      cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2, cjcc2
        dd      csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc         ; 9x
        dd      csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc,csetcc
        dd      cunk,  cunk,  ccpuid,cbtx2, cshld, cshld, cunk,  cunk           ; Ax
        dd      cunk,  cunk,  cunk,  cbtx2, cshrd, cshrd, cgrp15,cop22
        dd      ccmpxchg,ccmpxchg,cunk,cbtx2,cunk, cunk,  cmovzx,cmovzx         ; Bx
        dd      cunk,  cunk,  cbtx1, cbtx2, cbsf,  cbsr,  cmovsx,cmovsx
        dd      cunk,  cunk,  csse1, cunk,  cunk,  cunk,  cunk,  ccmpxchg8b     ; Cx
        dd      cbswap,cbswap,cbswap,cbswap,cbswap,cbswap,cbswap,cbswap
        dd      csse2, cpsrlw,cpsrlw,cpsrlq,cpcmn, cpcmn, cunk,  cunk           ; Dx
        dd      cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn
        dd      cpcmn, cpsraw,cpsrad,cpcmn, cpcmn, cpcmn, cunk,  cunk           ; Ex
        dd      cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn
        dd      cunk,  cpsllw,cpslld,cpsllq,cpcmn, cpcmn, cpcmn, cunk           ; Fx
        dd      cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cpcmn, cunk

; vim: ft=fasm tabstop=4

