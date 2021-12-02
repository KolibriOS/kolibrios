;-----------------------------------------------------------------------------
macro dr [Name]
{
if Name eq
 dw 0
else
 dw Name-RtnXX
end if
}
;-----------------------------------------------------------------------------
N0F1A:          dw TBNDCL,TBNDCU,TBNDMOV ,TBNDLDX
R0F1A:          dr RBNDCL,RBNDCU,RBNDMOV2,RBNDLDX
;-----------------------------------------------------------------------------
N0F1B:          dw TBNDMK,TBNDCN,TBNDMOV ,TBNDSTX
R0F1B:          dr RBNDMK,RBNDCN,RBNDMOV1,RBNDSTX
;-----------------------------------------------------------------------------
TTXXXX:
;-----------------------------------------------------------------------------
TT0F10: dw TMOVUPS      ;00
        dw TMOVUPD      ;66
        dw TMOVSD       ;F2
        dw TMOVSS       ;F3
        ;ch=3, dwdwdwdw
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F11: dw TMOVUPS      ;00
        dw TMOVUPD      ;66
        dw TMOVSD       ;F2
        dw TMOVSS       ;F3
        ;ch=3, dwdwdwdw
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr RM32R32X     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0F12: dw TMOVLPS      ;00
        dw TMOVLPD      ;66
        dw TMOVDDUP     ;F2
        dw TMOVSLDUP    ;F3
        dw TMOVHLPS     ;00 ch=3
        dw 0            ;66 ch=3
        dw TMOVDDUP     ;F2 ch=3
        dw TMOVSLDUP    ;F3 ch=3
        db VQWORD+80h   ;00
        db VQWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VXWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F13: dw TMOVLPS      ;00
        dw TMOVLPD      ;66
        dw 0            ;F2
        dw 0            ;F3
        dw 0            ;00 ch=3
        dw 0            ;66 ch=3
        dw 0            ;F2 ch=3
        dw 0            ;F3 ch=3
        db VQWORD+80h   ;00
        db VQWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr RM32R32X     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0F14: dw TUNPCKLPS    ;00
        dw TUNPCKLPD    ;66
        dw 0            ;F2
        dw 0            ;F3
        ;ch=3, dwdwdwdw
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr              ;F2
        dr              ;F3
;-----------------------------------------------------------------------------
TT0F15: dw TUNPCKHPS    ;00
        dw TUNPCKHPD    ;66
        dw 0            ;F2
        dw 0            ;F3
        ;ch=3, dwdwdwdw
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr              ;F2
        dr              ;F3
;-----------------------------------------------------------------------------
TT0F16: dw TMOVHPS      ;00
        dw TMOVHPD      ;66
        dw 0            ;F2
        dw TMOVSHDUP    ;F3
        dw TMOVLHPS     ;00 ch=3
        dw 0            ;66 ch=3
        dw 0            ;F2 ch=3
        dw TMOVSHDUP    ;F3 ch=3
        db VQWORD+80h   ;00
        db VQWORD+80h   ;66
        db 0            ;F2
        db VXWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr              ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F17: dw TMOVHPS      ;00
        dw TMOVHPD      ;66
        dw 0            ;F2
        dw 0            ;F3
        dw 0            ;00 ch=3
        dw 0            ;66 ch=3
        dw 0            ;F2 ch=3
        dw 0            ;F3 ch=3
        db VQWORD+80h   ;00
        db VQWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr RM32R32X     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0F28: dw TMOVAPS      ;00
        dw TMOVAPD      ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F29: dw TMOVAPS      ;00
        dw TMOVAPD      ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr RM32R32X     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0F2B: dw TMOVNTPS     ;00
        dw TMOVNTPD     ;66
        dw TMOVNTSD     ;F2
        dw TMOVNTSS     ;F3
        dw 0            ;00 ch=3
        dw 0            ;66 ch=3
        dw 0            ;F2 ch=3
        dw 0            ;F3 ch=3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr RM32R32X     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0F2E: dw TUCOMISS     ;00
        dw TUCOMISD     ;66
        dw 0            ;F2
        dw 0            ;F3
        db VDWORD+80h   ;00
        db VQWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F2F: dw TCOMISS      ;00
        dw TCOMISD      ;66
        dw 0            ;F2
        dw 0            ;F3
        db VDWORD+80h   ;00
        db VQWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F38: dw TNULL        ;00
        dw TNULL        ;66
        dw TNULL        ;F2
        dw TNULL        ;F3
        db 0            ;00
        db 0            ;66
        db 0            ;F2
        db 0            ;F3
        dr Rtn0F3800    ;00
        dr Rtn0F3866    ;66
        dr Rtn0F38F2    ;F2
        dr Rtn0F38F3    ;F3
;-----------------------------------------------------------------------------
TT0F51: dw TSQRTPS      ;00
        dw TSQRTPD      ;66
        dw TSQRTSD      ;F2
        dw TSQRTSS      ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F52: dw TRSQRTPS     ;00
        dw 0            ;66
        dw 0            ;F2
        dw TRSQRTSS     ;F3
        db VXWORD+80h   ;00
        db 0            ;66
        db 0            ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F53: dw TRCPPS       ;00
        dw 0            ;66
        dw 0            ;F2
        dw TRCPSS       ;F3
        db VXWORD+80h   ;00
        db 0            ;66
        db 0            ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F54: dw TANDPS       ;00
        dw TANDPD       ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F55: dw TANDNPS      ;00
        dw TANDNPD      ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F56: dw TORPS        ;00
        dw TORPD        ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F57: dw TXORPS       ;00
        dw TXORPD       ;66
        dw 0            ;F2
        dw 0            ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F58: dw TADDPS       ;00
        dw TADDPD       ;66
        dw TADDSD       ;F2
        dw TADDSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F59: dw TMULPS       ;00
        dw TMULPD       ;66
        dw TMULSD       ;F2
        dw TMULSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5A: dw TCVTPS2PD    ;00
        dw TCVTPD2PS    ;66
        dw TCVTSD2SS    ;F2
        dw TCVTSS2SD    ;F3
        db VQWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5B: dw TCVTDQ2PS    ;00
        dw TCVTPS2DQ    ;66
        dw 0            ;F2
        dw TCVTTPS2DQ   ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db VXWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5C: dw TSUBPS       ;00
        dw TSUBPD       ;66
        dw TSUBSD       ;F2
        dw TSUBSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5D: dw TMINPS       ;00
        dw TMINPD       ;66
        dw TMINSD       ;F2
        dw TMINSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5E: dw TDIVPS       ;00
        dw TDIVPD       ;66
        dw TDIVSD       ;F2
        dw TDIVSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F5F: dw TMAXPS       ;00
        dw TMAXPD       ;66
        dw TMAXSD       ;F2
        dw TMAXSS       ;F3
        db VXWORD+80h   ;00
        db VXWORD+80h   ;66
        db VQWORD+80h   ;F2
        db VDWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F6F: dw TMOVQ        ;00
        dw TMOVDQA      ;66
        dw 0            ;F2
        dw TMOVDQU      ;F3
        db VQWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db VXWORD+80h   ;F3
        dr RtnMM2MM     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F7C: dw 0            ;00
        dw THADDPD      ;66
        dw THADDPS      ;F2
        dw 0            ;F3
        db 0            ;00
        db VXWORD+80h   ;66
        db VXWORD+80h   ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F7D: dw 0            ;00
        dw THSUBPD      ;66
        dw THSUBPS      ;F2
        dw 0            ;F3
        db 0            ;00
        db VXWORD+80h   ;66
        db VXWORD+80h   ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0F7F: dw TMOVQ        ;00
        dw TMOVDQA      ;66
        dw 0            ;F2
        dw TMOVDQU      ;F3
        db VQWORD+80h   ;00
        db VXWORD+80h   ;66
        db 0            ;F2
        db VXWORD+80h   ;F3
        dr RtnMM4MM     ;00
        dr RM32R32X     ;66
        dr RM32R32X     ;F2
        dr RM32R32X     ;F3
;-----------------------------------------------------------------------------
TT0FD0: dw 0            ;00
        dw TADDSUBPD    ;66
        dw TADDSUBPS    ;F2
        dw 0            ;F3
        db 0            ;00
        db VXWORD+80h   ;66
        db VXWORD+80h   ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0FE6: dw 0            ;00
        dw TCVTTPD2DQ   ;66
        dw TCVTPD2DQ    ;F2
        dw TCVTDQ2PD    ;F3
        db 0            ;00
        db VXWORD+80h   ;66
        db VXWORD+80h   ;F2
        db VQWORD+80h   ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
TT0FF0: dw 0            ;00
        dw 0            ;66
        dw TLDDQU       ;F2
        dw 0            ;F3
        dw 0            ;00 ch=3
        dw 0            ;66 ch=3
        dw 0            ;F2 ch=3
        dw 0            ;F3 ch=3
        db 0            ;00
        db 0            ;66
        db VXWORD+80h   ;F2
        db 0            ;F3
        dr R32RM32X     ;00
        dr R32RM32X     ;66
        dr R32RM32X     ;F2
        dr R32RM32X     ;F3
;-----------------------------------------------------------------------------
; 80h - REX.W -> RD -> RQ
; 40h - REX.W -> QWORD
; 20h - REX.W -> ...Q
; 10h - IMMB
;-----------------------------------------------------------------------------
TT0F2A: dw TCVTPI2PS            ;00
        dw TCVTPI2PD            ;66
        dw TCVTSI2SD            ;F2
        dw TCVTSI2SS            ;F3
        ;ch=3 dwdwdwdw
        db VQWORD+80h           ;00
        db VQWORD+80h           ;66
        db VDWORD               ;F2
        db VDWORD               ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db XMMRG,MMXRG          ;00
        db XMMRG,MMXRG          ;66
        db XMMRG,GPR32+80h+40h  ;F2
        db XMMRG,GPR32+80h+40h  ;F3
;-----------------------------------------------------------------------------
TT0F2C: dw TCVTTPS2PI           ;00
        dw TCVTTPD2PI           ;66
        dw TCVTTSD2SI           ;F2
        dw TCVTTSS2SI           ;F3
        db VQWORD+80h           ;00
        db VXWORD+80h           ;66
        db VQWORD+80h           ;F2
        db VDWORD+80h           ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db MMXRG,XMMRG          ;00
        db MMXRG,XMMRG          ;66
        db GPR32+80h,XMMRG      ;F2
        db GPR32+80h,XMMRG      ;F3
;-----------------------------------------------------------------------------
TT0F2D: dw TCVTPS2PI            ;00
        dw TCVTPD2PI            ;66
        dw TCVTSD2SI            ;F2
        dw TCVTSS2SI            ;F3
        db VQWORD+80h           ;00
        db VXWORD+80h           ;66
        db VQWORD+80h           ;F2
        db VDWORD+80h           ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db MMXRG,XMMRG          ;00
        db MMXRG,XMMRG          ;66
        db GPR32+80h,XMMRG      ;F2
        db GPR32+80h,XMMRG      ;F3
;-----------------------------------------------------------------------------
TT0F50: dw 0                    ;00
        dw 0                    ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw TMOVMSKPS            ;00 ch=3
        dw TMOVMSKPD            ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db 0                    ;00
        db 0                    ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr                      ;F2
        dr                      ;F3
        db GPR32+80h,XMMRG      ;00
        db GPR32+80h,XMMRG      ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0F6E: dw TMOVD                ;00
        dw TMOVD                ;66
        dw 0                    ;F2
        dw 0                    ;F3
        db VDWORD+80h           ;00
        db VDWORD+80h           ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr                      ;F2
        dr                      ;F3
      db MMXRG,GPR32+20h+40h+80h;00
      db XMMRG,GPR32+20h+40h+80h;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0F70: dw TPSHUFW              ;00
        dw TPSHUFD              ;66
        dw TPSHUFLW             ;F2
        dw TPSHUFHW             ;F3
        db VQWORD+80h           ;00
        db VXWORD+80h           ;66
        db VXWORD+80h           ;F2
        db VXWORD+80h           ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db MMXRG,MMXRG+10h      ;00
        db XMMRG,XMMRG+10h      ;66
        db XMMRG,XMMRG+10h      ;F2
        db XMMRG,XMMRG+10h      ;F3
;-----------------------------------------------------------------------------
TT0F7E: dw TMOVD                ;00
        dw TMOVD                ;66
        dw 0                    ;F2
        dw TMOVQ                ;F3
        db VDWORD+80h           ;00
        db VDWORD+80h           ;66
        db 0                    ;F2
        db VQWORD+80h           ;F3
        dr RM32R32X             ;00
        dr RM32R32X             ;66
        dr RM32R32X             ;F2
        dr R32RM32X             ;F3
      db GPR32+20h+40h+80h,MMXRG;00
      db GPR32+20h+40h+80h,XMMRG;66
        db 0,0                  ;F2
        db XMMRG,XMMRG          ;F3
;-----------------------------------------------------------------------------
TT0FC3: dw TMOVNTI              ;00
        dw 0                    ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw 0                    ;00 ch=3
        dw 0                    ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db VDWORD+80h           ;00
        db 0                    ;66
        db 0                    ;F2
        db 0                    ;F3
        dr RM32R32X             ;00
        dr RM32R32X             ;66
        dr RM32R32X             ;F2
        dr RM32R32X             ;F3
        db 0,GPR32+80h+40h      ;00
        db 0,0                  ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FC4: dw TPINSRW              ;00
        dw TPINSRW              ;66
        dw 0                    ;F2
        dw 0                    ;F3
        db VWORD+80h            ;00
        db VWORD+80h            ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db MMXRG,GPR32+80h+10h  ;00
        db XMMRG,GPR32+80h+10h  ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FC5: dw 0                    ;00
        dw 0                    ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw TPEXTRW              ;00 ch=3
        dw TPEXTRW              ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db 0                    ;00
        db 0                    ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db GPR32+80h+10h,MMXRG  ;00
        db GPR32+80h+10h,XMMRG  ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FC6: dw TSHUFPS              ;00
        dw TSHUFPD              ;66
        dw 0                    ;F2
        dw 0                    ;F3
        db VXWORD+80h           ;00
        db VXWORD+80h           ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db XMMRG,XMMRG+10h      ;00
        db XMMRG,XMMRG+10h      ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FD6: dw 0                    ;00
        dw TMOVQ                ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw 0                    ;00 ch=3
        dw TMOVQ                ;66 ch=3
        dw TMOVDQ2Q             ;F2 ch=3
        dw TMOVQ2DQ             ;F3 ch=3
        db 0                    ;00
        db VQWORD+80h           ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr RM32R32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db XMMRG,XMMRG          ;00
        db XMMRG,XMMRG          ;66
        db MMXRG,XMMRG          ;F2
        db XMMRG,MMXRG          ;F3
;-----------------------------------------------------------------------------
TT0FD7: dw 0                    ;00
        dw 0                    ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw TPMOVMSKB            ;00 ch=3
        dw TPMOVMSKB            ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db 0                    ;00
        db 0                    ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db GPR32+80h,MMXRG      ;00
        db GPR32+80h,XMMRG      ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FE7: dw TMOVNTQ              ;00
        dw TMOVNTDQ             ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw 0                    ;00 ch=3
        dw 0                    ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db VQWORD+80h           ;00
        db VXWORD+80h           ;66
        db 0                    ;F2
        db 0                    ;F3
        dr RM32R32X             ;00
        dr RM32R32X             ;66
        dr RM32R32X             ;F2
        dr RM32R32X             ;F3
        db 0,MMXRG              ;00
        db 0,XMMRG              ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
TT0FF7: dw 0                    ;00
        dw 0                    ;66
        dw 0                    ;F2
        dw 0                    ;F3
        dw TMASKMOVQ            ;00 ch=3
        dw TMASKMOVDQU          ;66 ch=3
        dw 0                    ;F2 ch=3
        dw 0                    ;F3 ch=3
        db 0                    ;00
        db 0                    ;66
        db 0                    ;F2
        db 0                    ;F3
        dr R32RM32X             ;00
        dr R32RM32X             ;66
        dr R32RM32X             ;F2
        dr R32RM32X             ;F3
        db MMXRG,MMXRG          ;00
        db XMMRG,XMMRG          ;66
        db 0,0                  ;F2
        db 0,0                  ;F3
;-----------------------------------------------------------------------------
StrTabXOPA:
XOPc $10,0,0,XOPL                                       ,RtnTBM0,TNULL
XOPc $12,0,0,0                                          ,RtnLWPxx,TNULL
dd -1
;-----------------------------------------------------------------------------
StrTabXOP9:
XOPc $01,0,0,XOPL                                       ,RtnTBM1,TNULL
XOPc $02,0,0,XOPL                                       ,RtnTBM2,TNULL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $12,0,0,XOPV                                       ,RtnLWPCB,TNULL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $81,VOWORD+80h,VYWORD+80h,XOPV or XOPO or XOPW     ,RtnXOP,TFRCZPD
XOPc $80,VOWORD+80h,VYWORD+80h,XOPV or XOPO or XOPW     ,RtnXOP,TFRCZPS
XOPc $83,VQWORD+80h,0,XOPV or XOPO or XOPL              ,RtnXOP,TFRCZSD
XOPc $82,VDWORD+80h,0,XOPV or XOPO or XOPL              ,RtnXOP,TFRCZSS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $C1,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDBW
XOPc $C2,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDBD
XOPc $C3,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDBQ
XOPc $C6,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDWD
XOPc $C7,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDWQ
XOPc $CB,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDDQ
XOPc $D1,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUBW
XOPc $D2,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUBD
XOPc $D3,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUBQ
XOPc $D6,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUWD
XOPc $D7,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUWQ
XOPc $DB,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHADDUDQ
XOPc $E1,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHSUBBW
XOPc $E2,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHSUBWD
XOPc $E3,VOWORD+80h,0,XOPL or XOPW                      ,RtnXOP,TPHSUBDQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $90,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPROTB
XOPc $91,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPROTW
XOPc $92,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPROTD
XOPc $93,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPROTQ
XOPc $98,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHAB
XOPc $99,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHAW
XOPc $9A,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHAD
XOPc $9B,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHAQ
XOPc $94,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHLB
XOPc $95,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHLW
XOPc $96,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHLD
XOPc $97,VOWORD+80h,0,XOPL                              ,XOP3Rtn,TPSHLQ
dd -1
;-----------------------------------------------------------------------------
StrTabXOP8:
XOPc $A2,VOWORD+80h,VYWORD+80h,XOPI                     ,XOP4Rtn,TPCMOV
XOPc $CC,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMB,TPCOM
XOPc $CD,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMW,TPCOM
XOPc $CE,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMD,TPCOM
XOPc $CF,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMQ,TPCOM
XOPc $EC,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMUB,TPCOM
XOPc $ED,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMUW,TPCOM
XOPc $EE,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMUD,TPCOM
XOPc $EF,VOWORD+80h,0,XOPL or XOPW                      ,SuffixCOMUQ,TPCOM
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $9E,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSDD
XOPc $9F,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSDQH
XOPc $97,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSDQL
XOPc $8E,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSSDD
XOPc $8F,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSSDQH
XOPc $87,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSSDQL
XOPc $86,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSSWD
XOPc $85,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSSWW
XOPc $96,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSWD
XOPc $95,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMACSWW
XOPc $A6,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMADCSSWD
XOPc $B6,VOWORD+80h,0,XOPL or XOPW or XOPI              ,XOP4Rtn,TPMADCSWD
XOPc $A3,VOWORD+80h,0,XOPL         or XOPI              ,XOP4Rtn,TPPERM
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
XOPc $C0,VOWORD+80h,0,XOPL or XOPW or XOPV              ,XOPImm8,TPROTB
XOPc $C1,VOWORD+80h,0,XOPL or XOPW or XOPV              ,XOPImm8,TPROTW
XOPc $C2,VOWORD+80h,0,XOPL or XOPW or XOPV              ,XOPImm8,TPROTD
XOPc $C3,VOWORD+80h,0,XOPL or XOPW or XOPV              ,XOPImm8,TPROTQ
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
dd -1
;-----------------------------------------------------------------------------
C43866:
xC      RtnC4C5,        VEXV or VEX2 or VEXW,           $13,xQ,xO,TCVTPH2PS
xC      RtnC4C5,        VEX0,                           $DC,xO,xY,TAESENC
xC      RtnC4C5,        VEX0,                           $DD,xO,xY,TAESENCLAST
xC      RtnC4C5,        VEX0,                           $DE,xO,xY,TAESDEC
xC      RtnC4C5,        VEX0,                           $DF,xO,xY,TAESDECLAST
xC      RtnC4C5,        VEXV or VEXL,                   $DB,xO,xN,TAESIMC
xC      RtnC401,        VEXV or VEX0 or VEXW,           $18,xD,xD,TBROADCASTSS
xC      RtnC401,        VEXV or VEXN or VEXW,           $19,xN,xQ,TBROADCASTSD
xC      RtnC4C5,        VEXV or VEXN or VEXM,           $1A,xN,xO,TBROADCASTF128
xC      RtnC4C5,        VEXV or VEXN or VEXM or VEXW,   $5A,xN,xO,TBROADCASTI128
xC      RtnC4C5,        VEXM,                           $2C,xO,xY,TMASKMOVPS
xC      RtnC4C5,        VEXM,                           $2D,xO,xY,TMASKMOVPD
xC      RtnC402,        VEXM,                           $2E,xO,xY,TMASKMOVPS
xC      RtnC402,        VEXM,                           $2F,xO,xY,TMASKMOVPD
xC      RtnC4C5,        VEXV or VEXM,                   $2A,xO,xY,TMOVNTDQA
xC      RtnC4C5,        VEXV,                           $1C,xO,xY,TPABSB
xC      RtnC4C5,        VEXV,                           $1D,xO,xY,TPABSW
xC      RtnC4C5,        VEXV,                           $1E,xO,xY,TPABSD
xC      RtnC4C5,        VEX0,                           $2B,xO,xY,TPACKUSDW
xC      RtnC4C5,        VEX0,                           $29,xO,xY,TPCMPEQQ
xC      RtnC4C5,        VEX0,                           $37,xO,xY,TPCMPGTQ
xC      RtnC4C5,        VEX0,                           $0D,xO,xY,TPERMILPD
xC      RtnC4C5,        VEX0,                           $0C,xO,xY,TPERMILPS
xC      RtnC4C5,        VEX0,                           $01,xO,xY,TPHADDW
xC      RtnC4C5,        VEX0,                           $02,xO,xY,TPHADDD
xC      RtnC4C5,        VEX0,                           $03,xO,xY,TPHADDSW
xC      RtnC4C5,        VEXV or VEXL,                   $41,xO,xN,TPHMINPOSUW
xC      RtnC4C5,        VEX0,                           $05,xO,xY,TPHSUBW
xC      RtnC4C5,        VEX0,                           $06,xO,xY,TPHSUBD
xC      RtnC4C5,        VEX0,                           $07,xO,xY,TPHSUBSW
xC      RtnC4C5,        VEX0,                           $04,xO,xY,TPMADDUBSW
xC      RtnC4C5,        VEX0,                           $3C,xO,xY,TPMAXSB
xC      RtnC4C5,        VEX0,                           $3D,xO,xY,TPMAXSD
xC      RtnC4C5,        VEX0,                           $3E,xO,xY,TPMAXUW
xC      RtnC4C5,        VEX0,                           $3F,xO,xY,TPMAXUD
xC      RtnC4C5,        VEX0,                           $38,xO,xY,TPMINSB
xC      RtnC4C5,        VEX0,                           $39,xO,xY,TPMINSD
xC      RtnC4C5,        VEX0,                           $3A,xO,xY,TPMINUW
xC      RtnC4C5,        VEX0,                           $3B,xO,xY,TPMINUD
xC      RtnC4C5,        VEXV or VEX2,                   $20,xQ,xO,TPMOVSXBW
xC      RtnC4C5,        VEXV or VEX2,                   $21,xD,xQ,TPMOVSXBD
xC      RtnC4C5,        VEXV or VEX2,                   $22,xW,xD,TPMOVSXBQ
xC      RtnC4C5,        VEXV or VEX2,                   $23,xQ,xO,TPMOVSXWD
xC      RtnC4C5,        VEXV or VEX2,                   $24,xD,xQ,TPMOVSXWQ
xC      RtnC4C5,        VEXV or VEX2,                   $25,xQ,xO,TPMOVSXDQ
xC      RtnC4C5,        VEXV or VEX2,                   $30,xQ,xO,TPMOVZXBW
xC      RtnC4C5,        VEXV or VEX2,                   $31,xD,xQ,TPMOVZXBD
xC      RtnC4C5,        VEXV or VEX2,                   $32,xW,xD,TPMOVZXBQ
xC      RtnC4C5,        VEXV or VEX2,                   $33,xQ,xO,TPMOVZXWD
xC      RtnC4C5,        VEXV or VEX2,                   $34,xD,xQ,TPMOVZXWQ
xC      RtnC4C5,        VEXV or VEX2,                   $35,xQ,xO,TPMOVZXDQ
xC      RtnC4C5,        VEX0,                           $0B,xO,xY,TPMULHRSW
xC      RtnC4C5,        VEX0,                           $40,xO,xY,TPMULLD
xC      RtnC4C5,        VEX0,                           $28,xO,xY,TPMULDQ
xC      RtnC4C5,        VEX0,                           $00,xO,xY,TPSHUFB
xC      RtnC4C5,        VEX0,                           $08,xO,xY,TPSIGNB
xC      RtnC4C5,        VEX0,                           $09,xO,xY,TPSIGNW
xC      RtnC4C5,        VEX0,                           $0A,xO,xY,TPSIGND
xC      RtnC4C5,        VEXV,                           $17,xO,xY,TPTEST
xC      RtnC4C5,        VEXV,                           $0E,xO,xY,TTESTPS
xC      RtnC4C5,        VEXV,                           $0F,xO,xY,TTESTPD
xC      RtnC401,        VEXV or VEXW,                   $78,xB,xB,TPBROADCASTB
xC      RtnC401,        VEXV or VEXW,                   $79,xW,xW,TPBROADCASTW
xC      RtnC401,        VEXV or VEXW,                   $58,xD,xD,TPBROADCASTD
xC      RtnC401,        VEXV or VEXW,                   $59,xQ,xQ,TPBROADCASTQ
xC      RtnC4C5,        VEXN or VEXW,                   $36,xN,xY,TPERMD
xC      RtnC4C5,        VEXN or VEXW,                   $16,xN,xY,TPERMPS
xC      RtnC403,        VEX0,                           $47,xO,xY,TPSLLVD
xC      RtnC4C5,        VEXW,                           $46,xO,xY,TPSRAVD
xC      RtnC403,        VEX0,                           $45,xO,xY,TPSRLVD
xC      RtnC403,        VEXM,                           $8C,xO,xY,TPMASKMOVD
xC      RtnC404,        VEXM,                           $8E,xO,xY,TPMASKMOVD
xC      RtnC405,        VEXM,                           $90,xD,xD,TPGATHERDD
xC      RtnC405,        VEXM,                           $91,xQ,xQ,TPGATHERQD
xC      RtnC406,        VEXM,                           $92,xD,xD,TGATHERD
xC      RtnC406,        VEXM,                           $93,xQ,xQ,TGATHERQ
;;;;;;;;----------------------------------------------------------------------
xC      RtnC407,        VEX0 or VEXC,                   $96,xO,xY,TFMADDSUB132
xC      RtnC407,        VEX0 or VEXC,                   $97,xO,xY,TFMSUBADD132
xC      RtnC407,        VEX0 or VEXC,                   $98,xO,xY,TFMADD132
xC      RtnC408,        VEXL or VEXC,                   $99,xD,xN,TFMADD132
xC      RtnC407,        VEX0 or VEXC,                   $9A,xO,xY,TFMSUB132
xC      RtnC408,        VEXL or VEXC,                   $9B,xD,xN,TFMSUB132
xC      RtnC407,        VEX0 or VEXC,                   $9C,xO,xY,TFNMADD132
xC      RtnC408,        VEXL or VEXC,                   $9D,xD,xN,TFNMADD132
xC      RtnC407,        VEX0 or VEXC,                   $9E,xO,xY,TFNMSUB132
xC      RtnC408,        VEXL or VEXC,                   $9F,xD,xN,TFNMSUB132
xC      RtnC407,        VEX0 or VEXC,                   $A6,xO,xY,TFMADDSUB213
xC      RtnC407,        VEX0 or VEXC,                   $A7,xO,xY,TFMSUBADD213
xC      RtnC407,        VEX0 or VEXC,                   $A8,xO,xY,TFMADD213
xC      RtnC408,        VEXL or VEXC,                   $A9,xD,xN,TFMADD213
xC      RtnC407,        VEX0 or VEXC,                   $AA,xO,xY,TFMSUB213
xC      RtnC408,        VEXL or VEXC,                   $AB,xD,xN,TFMSUB213
xC      RtnC407,        VEX0 or VEXC,                   $AC,xO,xY,TFNMADD213
xC      RtnC408,        VEXL or VEXC,                   $AD,xD,xN,TFNMADD213
xC      RtnC407,        VEX0 or VEXC,                   $AE,xO,xY,TFNMSUB213
xC      RtnC408,        VEXL or VEXC,                   $AF,xD,xN,TFNMSUB213
xC      RtnC407,        VEX0 or VEXC,                   $B6,xO,xY,TFMADDSUB231
xC      RtnC407,        VEX0 or VEXC,                   $B7,xO,xY,TFMSUBADD231
xC      RtnC407,        VEX0 or VEXC,                   $B8,xO,xY,TFMADD231
xC      RtnC408,        VEXL or VEXC,                   $B9,xD,xN,TFMADD231
xC      RtnC407,        VEX0 or VEXC,                   $BA,xO,xY,TFMSUB231
xC      RtnC408,        VEXL or VEXC,                   $BB,xD,xN,TFMSUB231
xC      RtnC407,        VEX0 or VEXC,                   $BC,xO,xY,TFNMADD231
xC      RtnC408,        VEXL or VEXC,                   $BD,xD,xN,TFNMADD231
xC      RtnC407,        VEX0 or VEXC,                   $BE,xO,xY,TFNMSUB231
xC      RtnC408,        VEXL or VEXC,                   $BF,xD,xN,TFNMSUB231
xC      RtnC409,        VEXG or VEXH or VEXL,           $F7,xD,xQ,TSHLX
xC      RtnC4C5,        VEXW,                           $CF,xO,xY,TGF2P8MULB
;;;;;;;;----------------------------------------------------------------------
dd -1
;-----------------------------------------------------------------------------
C43A66:
xC      RtnC40A,        VEXL or VEXI or VEXH,           $30,xO,xY,TKSHIFTRB
xC      RtnC40B,        VEXL or VEXI or VEXH,           $31,xO,xY,TKSHIFTRD
xC      RtnC40A,        VEXL or VEXI or VEXH,           $32,xO,xY,TKSHIFTLB
xC      RtnC40B,        VEXL or VEXI or VEXH,           $33,xO,xY,TKSHIFTLD
xC      RtnC4C5,        VEXV or VEXL or VEXI,           $DF,xO,xN,TAESKEYGENASSIST
xC      RtnC4C5,        VEXI,                           $0C,xO,xY,TBLENDPS
xC      RtnC4C5,        VEXI,                           $0D,xO,xY,TBLENDPD
xC      RtnC4C5,        VEX4 or VEXW,                   $4A,xO,xY,TBLENDVPS
xC      RtnC4C5,        VEX4 or VEXW,                   $4B,xO,xY,TBLENDVPD
xC      RtnC4C5,        VEXI or VEXL,                   $41,xO,xN,TDPPD
xC      RtnC4C5,        VEXI,                           $40,xO,xY,TDPPS
xC      RtnC40C,        VEXI or VEXV or VEXN,           $19,xN,xO,TEXTRACTF128
xC      RtnC40D,        VEXI or VEXV or VEXW,           $1D,xQ,xO,TCVTPS2PH
xC      RtnC40E,        VEXI or VEXV or VEXL,           $17,xD,xN,TEXTRACTPS
xC      RtnC410,        VEXI or VEX0,                   $18,xN,xO,TINSERTF128
xC      RtnC4C5,        VEXI or VEXL,                   $21,xD,xN,TINSERTPS
xC      RtnC4C5,        VEXI,                           $42,xO,xY,TMPSADBW
xC      RtnC4C5,        VEXI,                           $0F,xO,xY,TPALIGNR
xC      RtnC4C5,        VEX4 or VEXW,                   $4C,xO,xY,TPBLENDVB
xC      RtnC4C5,        VEXI,                           $0E,xO,xY,TPBLENDW
xC      RtnCLMUL,       VEXI,                           $44,xO,xY,TPCLMULQDQ
xC      RtnC4C5,        VEXV or VEXL or VEXI,           $61,xO,xN,TPCMPESTRI
xC      RtnC4C5,        VEXV or VEXL or VEXI,           $60,xO,xN,TPCMPESTRM
xC      RtnC4C5,        VEXV or VEXL or VEXI,           $63,xO,xN,TPCMPISTRI
xC      RtnC4C5,        VEXV or VEXL or VEXI,           $62,xO,xN,TPCMPISTRM
xC      RtnC4C5,        VEXV or VEXI,                   $05,xO,xY,TPERMILPD
xC      RtnC4C5,        VEXV or VEXI,                   $04,xO,xY,TPERMILPS
xC      RtnC4C5,        VEXN or VEXI,                   $06,xN,xY,TPERM2F128
xC      RtnC40F,        VEXV or VEXL or VEXI,           $14,xB,xN,TPEXTRB
xC      RtnC40F,        VEXV or VEXL or VEXI,           $15,xW,xN,TPEXTRW
xC      RtnC411,        VEXV or VEXL or VEXI,           $16,xD,xN,TPEXTRD
xC      RtnC412,        VEXL or VEXI,                   $20,xB,xN,TPINSRB
xC      RtnC413,        VEXL or VEXI,                   $22,xD,xN,TPINSRD
xC      RtnC4C5,        VEXV or VEXI,                   $09,xO,xY,TROUNDPD
xC      RtnC4C5,        VEXV or VEXI,                   $08,xO,xY,TROUNDPS
xC      RtnC4C5,        VEXL or VEXI,                   $0B,xQ,xN,TROUNDSD
xC      RtnC4C5,        VEXL or VEXI,                   $0A,xD,xN,TROUNDSS
xC      RtnC4C5,        VEXI or VEXW,                   $02,xO,xY,TPBLENDD
xC      RtnC414,        VEXN or VEXC or VEXI,           $01,xN,xY,TPERMPD
xC      RtnC414,        VEXN or VEXC or VEXI,           $00,xN,xY,TPERMQ
xC      RtnC4C5,        VEXN or VEXW or VEXI,           $46,xN,xY,TPERM2I128
xC      RtnC40C,        VEXN or VEXV or VEXW or VEXI,   $39,xN,xO,TEXTRACTI128
xC      RtnC410,        VEXN or VEXW or VEXI,           $38,xN,xO,TINSERTI128
xC      RtnC4C5,        VEXC or VEXI,                   $CE,xO,xY,TGF2P8AFFINEQB
xC      RtnC4C5,        VEXC or VEXI,                   $CF,xO,xY,TGF2P8AFFINEINVQB
;;;;;;;;----------------------------------------------------------------------
xC      C4FMA,          VEX0 or VEXC,                   $69,xO,xY,TFMADDPD
xC      C4FMA,          VEX0 or VEXC,                   $68,xO,xY,TFMADDPS
xC      C4FMA,          VEXL or VEXC,                   $6B,xQ,xN,TFMADDSD
xC      C4FMA,          VEXL or VEXC,                   $6A,xD,xN,TFMADDSS
xC      C4FMA,          VEX0 or VEXC,                   $5D,xO,xY,TFMADDSUBPD
xC      C4FMA,          VEX0 or VEXC,                   $5C,xO,xY,TFMADDSUBPS
xC      C4FMA,          VEX0 or VEXC,                   $5F,xO,xY,TFMSUBADDPD
xC      C4FMA,          VEX0 or VEXC,                   $5E,xO,xY,TFMSUBADDPS
xC      C4FMA,          VEX0 or VEXC,                   $6D,xO,xY,TFMSUBPD
xC      C4FMA,          VEX0 or VEXC,                   $6C,xO,xY,TFMSUBPS
xC      C4FMA,          VEXL or VEXC,                   $6F,xQ,xN,TFMSUBSD
xC      C4FMA,          VEXL or VEXC,                   $6E,xD,xN,TFMSUBSS
xC      C4FMA,          VEX0 or VEXC,                   $79,xO,xY,TFNMADDPD
xC      C4FMA,          VEX0 or VEXC,                   $78,xO,xY,TFNMADDPS
xC      C4FMA,          VEXL or VEXC,                   $7B,xQ,xN,TFNMADDSD
xC      C4FMA,          VEXL or VEXC,                   $7A,xD,xN,TFNMADDSS
xC      C4FMA,          VEX0 or VEXC,                   $7D,xO,xY,TFNMSUBPD
xC      C4FMA,          VEX0 or VEXC,                   $7C,xO,xY,TFNMSUBPS
xC      C4FMA,          VEXL or VEXC,                   $7F,xQ,xN,TFNMSUBSD
xC      C4FMA,          VEXL or VEXC,                   $7E,xD,xN,TFNMSUBSS
dd -1
;;;;;;;;----------------------------------------------------------------------
C43800:
xC      RtnC4C5,        VEXG or VEXH or VEXL,           $F2,xD,xQ,TANDN
xC      RtnC409,        VEXG or VEXH or VEXL,           $F7,xD,xQ,TBEXTR
xC      RtnC415,        VEXG or VEXH or VEXL,           $F3,xD,xQ,TNULL
xC      RtnC409,        VEXG or VEXH or VEXL,           $F5,xD,xQ,TBZHI
dd -1
C438F3:
xC      RtnC4C5,        VEXG or VEXH or VEXL,           $F5,xD,xQ,TPEXT
xC      RtnC409,        VEXG or VEXH or VEXL,           $F7,xD,xQ,TSARX
dd -1
C438F2:
xC      RtnC4C5,        VEXG or VEXH or VEXL,           $F6,xD,xQ,TMULX
xC      RtnC4C5,        VEXG or VEXH or VEXL,           $F5,xD,xQ,TPDEP
xC      RtnC409,        VEXG or VEXH or VEXL,           $F7,xD,xQ,TSHRX
dd -1
C43A00:
xC      C40F3A48,       VEX4,                           $48,xO,xY,TPERMIL2PS
xC      C40F3A49,       VEX4,                           $49,xO,xY,TPERMIL2PD
dd -1
C43AF3:
C43AF2:
xC      RtnC416,        VEXG or VEXH or VEXL,           $F0,xD,xQ,TRORX
dd -1
;-----------------------------------------------------------------------------
C4C500:
xC      RtnC4C501,      VEXN+VEXH,                      $4A,xO,xY,TKADDW
xC      RtnC4C501,      VEXN+VEXH,                      $41,xO,xY,TKANDW
xC      RtnC4C501,      VEXN+VEXH,                      $42,xO,xY,TKANDNW
xC      RtnC4C501,      VEXN+VEXH,                      $45,xO,xY,TKORW
xC      RtnC4C501,      VEXN+VEXH,                      $46,xO,xY,TKXNORW
xC      RtnC4C501,      VEXN+VEXH,                      $47,xO,xY,TKXORW
xC      RtnC4C504,      VEXL+VEXH,                      $44,xO,xY,TKNOTW
xC      RtnC4C504,      VEXL+VEXH,                      $98,xO,xY,TKORTESTW
xC      RtnC4C504,      VEXL+VEXH,                      $99,xO,xY,TKTESTW
xC      RtnC4C505,      VEXL+VEXH,                      $90,xN,xN,TKMOVW
xC      RtnC4C507,      VEXL+VEXH,                      $91,xN,xN,TKMOVW
xC      RtnC4C508,      VEXN+VEXH,                      $4B,xN,xN,TKUNPCKWD
xC      RtnC4C509,      VEXL+VEXH+VEXW,                 $92,xN,xN,TKMOVW
xC      RtnC4C50A,      VEXL+VEXH+VEXW,                 $93,xN,xN,TKMOVW
xC      RtnC4C5,        VEX0,                           $58,xO,xY,TADDPS
xC      RtnC4C5,        VEX0,                           $54,xO,xY,TANDPS
xC      RtnC4C5,        VEX0,                           $55,xO,xY,TANDNPS
xC      RtnC4C5,        VEX8,                           $C2,xO,xY,TCMPPS
xC      RtnC4C5,        VEXV or VEXL,                   $2F,xD,xN,TCOMISS
xC      RtnC4C5,        VEXV or VEX0,                   $5B,xO,xY,TCVTDQ2PS
xC      RtnC4C5,        VEXV or VEX2,                   $5A,xQ,xO,TCVTPS2PD
xC      RtnC4C5,        VEX0,                           $5E,xO,xY,TDIVPS
xC      RtnC4C50B,      VEXL or VEXM,                   $AE,xD,xN,TLDMXCSR
xC      RtnC4C5,        VEX0,                           $5F,xO,xY,TMAXPS
xC      RtnC4C5,        VEX0,                           $5D,xO,xY,TMINPS
xC      RtnC4C5,        VEXV,                           $28,xO,xY,TMOVAPS
xC      RtnC4C50C,      VEXV,                           $29,xO,xY,TMOVAPS
xC      C4C5NB1,        VEXL,                           $12,xQ,xY,TMOVLPS
xC      C4C5NB2,        VEXL,                           $16,xQ,xN,TMOVHPS
xC      RtnC4C50C,      VEXV or VEXL or VEXM,           $17,xQ,xN,TMOVHPS
xC      RtnC4C50C,      VEXV or VEXL or VEXM,           $13,xQ,xN,TMOVLPS
xC      RtnC4C50D,      VEXV or VEXR,                   $50,xO,xY,TMOVMSKPS
xC      RtnC4C50C,      VEXV or VEXM,                   $2B,xO,xY,TMOVNTPS
xC      RtnC4C5,        VEXV,                           $10,xO,xY,TMOVUPS
xC      RtnC4C50C,      VEXV,                           $11,xO,xY,TMOVUPS
xC      RtnC4C5,        VEX0,                           $59,xO,xY,TMULPS
xC      RtnC4C5,        VEX0,                           $56,xO,xY,TORPS
xC      RtnC4C5,        VEXV,                           $53,xO,xY,TRCPPS
xC      RtnC4C5,        VEXV,                           $52,xO,xY,TRSQRTPS
xC      RtnC4C5,        VEXI,                           $C6,xO,xY,TSHUFPS
xC      RtnC4C5,        VEXV,                           $51,xO,xY,TSQRTPS
xC      RtnC4C5,        VEX0,                           $5C,xO,xY,TSUBPS
xC      RtnC4C5,        VEXV or VEXL,                   $2E,xD,xN,TUCOMISS
xC      RtnC4C5,        VEX0,                           $15,xO,xY,TUNPCKHPS
xC      RtnC4C5,        VEX0,                           $14,xO,xY,TUNPCKLPS
xC      RtnC4C5,        VEX0,                           $57,xO,xY,TXORPS
xC      RtnC4C50E,      VEXF or VEXV,                   $77,xN,xN,TZEROALL
dd -1
;-----------------------------------------------------------------------------
C4C566:
xC      RtnC4C502,      VEXN+VEXH,                      $41,xO,xY,TKANDB
xC      RtnC4C502,      VEXN+VEXH,                      $42,xO,xY,TKANDNB
xC      RtnC4C502,      VEXN+VEXH,                      $4A,xO,xY,TKADDB
xC      RtnC4C502,      VEXN+VEXH,                      $45,xO,xY,TKORB
xC      RtnC4C502,      VEXN+VEXH,                      $46,xO,xY,TKXNORB
xC      RtnC4C502,      VEXN+VEXH,                      $47,xO,xY,TKXORB
xC      RtnC4C503,      VEXL+VEXH,                      $44,xO,xY,TKNOTB
xC      RtnC4C503,      VEXL+VEXH,                      $98,xO,xY,TKORTESTB
xC      RtnC4C503,      VEXL+VEXH,                      $99,xO,xY,TKTESTB
xC      RtnC4C508,      VEXN+VEXH+VEXW,                 $4B,xN,xN,TKUNPCKBW
xC      RtnC4C503,      VEXL+VEXH,                      $90,xN,xN,TKMOVB
xC      RtnC4C506,      VEXL+VEXH,                      $91,xN,xN,TKMOVB
xC      RtnC4C509,      VEXL+VEXH+VEXW,                 $92,xN,xN,TKMOVB
xC      RtnC4C50A,      VEXL+VEXH+VEXW,                 $93,xN,xN,TKMOVB
xC      RtnC4C5,        VEX0,                           $58,xO,xY,TADDPD
xC      RtnC4C5,        VEX0,                           $D0,xO,xY,TADDSUBPD
xC      RtnC4C5,        VEX0,                           $54,xO,xY,TANDPD
xC      RtnC4C5,        VEX0,                           $55,xO,xY,TANDNPD
xC      RtnC4C5,        VEX8,                           $C2,xO,xY,TCMPPD
xC      RtnC4C5,        VEXV or VEXL,                   $2F,xQ,xN,TCOMISD
xC      RtnC4C5,        VEXV or VEX1,                   $5A,xO,xY,TCVTPD2PS
xC      RtnC4C5,        VEXV or VEX0,                   $5B,xO,xY,TCVTPS2DQ
xC      RtnC4C5,        VEXV or VEX0 or VEX1,           $E6,xO,xY,TCVTTPD2DQ
xC      RtnC4C5,        VEX0,                           $5E,xO,xY,TDIVPD
xC      RtnC4C5,        VEX0,                           $7C,xO,xY,THADDPD
xC      RtnC4C5,        VEX0,                           $7D,xO,xY,THSUBPD
xC      RtnC4C5,        VEXV or VEXL or VEXR,           $F7,xN,xN,TMASKMOVDQU
xC      RtnC4C5,        VEX0,                           $5F,xO,xY,TMAXPD
xC      RtnC4C5,        VEX0,                           $5D,xO,xY,TMINPD
xC      RtnC4C5,        VEXV,                           $28,xO,xY,TMOVAPD
xC      RtnC4C50C,      VEXV,                           $29,xO,xY,TMOVAPD
xC      RtnC4C5XRM,     VEXV or VEXL,                   $6E,xD,xN,TMOVD
xC      RtnC4C5RMX,     VEXV or VEXL,                   $7E,xD,xN,TMOVD
xC      RtnC4C50C,      VEXV or VEXL,                   $D6,xQ,xN,TMOVQ
xC      RtnC4C5,        VEXV,                           $6F,xO,xY,TMOVDQA
xC      RtnC4C50C,      VEXV,                           $7F,xO,xY,TMOVDQA
xC      RtnC4C5,        VEXL or VEXM,                   $16,xQ,xN,TMOVHPD
xC      RtnC4C50C,      VEXV or VEXL or VEXM,           $17,xQ,xN,TMOVHPD
xC      RtnC4C5,        VEXL or VEXM,                   $12,xQ,xN,TMOVLPD
xC      RtnC4C50C,      VEXV or VEXL or VEXM,           $13,xQ,xN,TMOVLPD
xC      RtnC4C50D,      VEXV or VEXR,                   $50,xO,xY,TMOVMSKPD
xC      RtnC4C50C,      VEXV or VEXM,                   $E7,xO,xY,TMOVNTDQ
xC      RtnC4C50C,      VEXV or VEXM,                   $2B,xO,xY,TMOVNTPD
xC      RtnC4C5,        VEXV,                           $10,xO,xY,TMOVUPD
xC      RtnC4C50C,      VEXV,                           $11,xO,xY,TMOVUPD
xC      RtnC4C5,        VEX0,                           $59,xO,xY,TMULPD
xC      RtnC4C5,        VEX0,                           $56,xO,xY,TORPD
xC      RtnC4C5,        VEX0,                           $63,xO,xY,TPACKSSWB
xC      RtnC4C5,        VEX0,                           $6B,xO,xY,TPACKSSDW
xC      RtnC4C5,        VEX0,                           $67,xO,xY,TPACKUSWB
xC      RtnC4C5,        VEX0,                           $FC,xO,xY,TPADDB
xC      RtnC4C5,        VEX0,                           $FD,xO,xY,TPADDW
xC      RtnC4C5,        VEX0,                           $FE,xO,xY,TPADDD
xC      RtnC4C5,        VEX0,                           $D4,xO,xY,TPADDQ
xC      RtnC4C5,        VEX0,                           $EC,xO,xY,TPADDSB
xC      RtnC4C5,        VEX0,                           $ED,xO,xY,TPADDSW
xC      RtnC4C5,        VEX0,                           $DC,xO,xY,TPADDUSB
xC      RtnC4C5,        VEX0,                           $DD,xO,xY,TPADDUSW
xC      RtnC4C5,        VEX0,                           $DB,xO,xY,TPAND
xC      RtnC4C5,        VEX0,                           $DF,xO,xY,TPANDN
xC      RtnC4C5,        VEX0,                           $E0,xO,xY,TPAVGB
xC      RtnC4C5,        VEX0,                           $E3,xO,xY,TPAVGW
xC      RtnC4C5,        VEX0,                           $74,xO,xY,TPCMPEQB
xC      RtnC4C5,        VEX0,                           $75,xO,xY,TPCMPEQW
xC      RtnC4C5,        VEX0,                           $76,xO,xY,TPCMPEQD
xC      RtnC4C5,        VEX0,                           $64,xO,xY,TPCMPGTB
xC      RtnC4C5,        VEX0,                           $65,xO,xY,TPCMPGTW
xC      RtnC4C5,        VEX0,                           $66,xO,xY,TPCMPGTD
xC      RtnC4C50F,      VEXV or VEXL or VEXR,           $C5,xN,xN,TPEXTRW
xC      RtnC412,        VEXL or VEXI,                   $C4,xW,xN,TPINSRW
xC      RtnC4C5,        VEX0,                           $F5,xO,xY,TPMADDWD
xC      RtnC4C5,        VEX0,                           $EE,xO,xY,TPMAXSW
xC      RtnC4C5,        VEX0,                           $DE,xO,xY,TPMAXUB
xC      RtnC4C5,        VEX0,                           $EA,xO,xY,TPMINSW
xC      RtnC4C5,        VEX0,                           $DA,xO,xY,TPMINUB
xC      RtnC4C50D,      VEXV or VEXR,                   $D7,xN,xN,TPMOVMSKB
xC      RtnC4C5,        VEX0,                           $E4,xO,xY,TPMULHUW
xC      RtnC4C5,        VEX0,                           $E5,xO,xY,TPMULHW
xC      RtnC4C5,        VEX0,                           $D5,xO,xY,TPMULLW
xC      RtnC4C5,        VEX0,                           $F4,xO,xY,TPMULUDQ
xC      RtnC4C5,        VEX0,                           $EB,xO,xY,TPOR
xC      RtnC4C5,        VEX0,                           $F6,xO,xY,TPSADBW
xC      RtnC4C5,        VEXV or VEXI,                   $70,xO,xY,TPSHUFD
xC      C4C50F71,       VEXS or VEXI or VEXR,           $71,xN,xN,TNULL
xC      C4C50F72,       VEXS or VEXI or VEXR,           $72,xN,xN,TNULL
xC      C4C50F73,       VEXS or VEXI or VEXR,           $73,xN,xN,TNULL
xC      RtnC4C510,      VEX0,                           $F1,xO,xO,TPSLLW
xC      RtnC4C510,      VEX0,                           $F2,xO,xO,TPSLLD
xC      RtnC4C510,      VEX0,                           $F3,xO,xO,TPSLLQ
xC      RtnC4C510,      VEX0,                           $E1,xO,xO,TPSRAW
xC      RtnC4C510,      VEX0,                           $E2,xO,xO,TPSRAD
xC      RtnC4C510,      VEX0,                           $D1,xO,xO,TPSRLW
xC      RtnC4C510,      VEX0,                           $D2,xO,xO,TPSRLD
xC      RtnC4C510,      VEX0,                           $D3,xO,xO,TPSRLQ
xC      RtnC4C5,        VEX0,                           $F8,xO,xY,TPSUBB
xC      RtnC4C5,        VEX0,                           $F9,xO,xY,TPSUBW
xC      RtnC4C5,        VEX0,                           $FA,xO,xY,TPSUBD
xC      RtnC4C5,        VEX0,                           $FB,xO,xY,TPSUBQ
xC      RtnC4C5,        VEX0,                           $E8,xO,xY,TPSUBSB
xC      RtnC4C5,        VEX0,                           $E9,xO,xY,TPSUBSW
xC      RtnC4C5,        VEX0,                           $D8,xO,xY,TPSUBUSB
xC      RtnC4C5,        VEX0,                           $D9,xO,xY,TPSUBUSW
xC      RtnC4C5,        VEX0,                           $68,xO,xY,TPUNPCKHBW
xC      RtnC4C5,        VEX0,                           $69,xO,xY,TPUNPCKHWD
xC      RtnC4C5,        VEX0,                           $6A,xO,xY,TPUNPCKHDQ
xC      RtnC4C5,        VEX0,                           $6D,xO,xY,TPUNPCKHQDQ
xC      RtnC4C5,        VEX0,                           $60,xO,xY,TPUNPCKLBW
xC      RtnC4C5,        VEX0,                           $61,xO,xY,TPUNPCKLWD
xC      RtnC4C5,        VEX0,                           $62,xO,xY,TPUNPCKLDQ
xC      RtnC4C5,        VEX0,                           $6C,xO,xY,TPUNPCKLQDQ
xC      RtnC4C5,        VEX0,                           $EF,xO,xY,TPXOR
xC      RtnC4C5,        VEXI,                           $C6,xO,xY,TSHUFPD
xC      RtnC4C5,        VEXV,                           $51,xO,xY,TSQRTPD
xC      RtnC4C5,        VEX0,                           $5C,xO,xY,TSUBPD
xC      RtnC4C5,        VEXV or VEXL,                   $2E,xQ,xN,TUCOMISD
xC      RtnC4C5,        VEX0,                           $15,xO,xY,TUNPCKHPD
xC      RtnC4C5,        VEX0,                           $14,xO,xY,TUNPCKLPD
xC      RtnC4C5,        VEX0,                           $57,xO,xY,TXORPD
dd -1
;-----------------------------------------------------------------------------
C4C5F2:
xC      RtnC4C5,        VEXL,                           $58,xQ,xN,TADDSD
xC      RtnC4C5,        VEX0,                           $D0,xO,xY,TADDSUBPS
xC      RtnC4C5,        VEX8 or VEXL,                   $C2,xQ,xN,TCMPSD
xC      RtnC4C5,        VEXV or VEX1,                   $E6,xO,xY,TCVTPD2DQ
xC      RtnC4C511,      VEXV or VEXL,                   $2D,xQ,xN,TCVTSD2SI
xC      RtnC4C5,        VEXL,                           $5A,xQ,xN,TCVTSD2SS
xC      RtnC4C512,      VEXL,                           $2A,xD,xN,TCVTSI2SD
xC      RtnC4C511,      VEXV or VEXL,                   $2C,xQ,xN,TCVTTSD2SI
xC      RtnC4C5,        VEXL,                           $5E,xQ,xN,TDIVSD
xC      RtnC4C5,        VEX0,                           $7C,xO,xY,THADDPS
xC      RtnC4C5,        VEX0,                           $7D,xO,xY,THSUBPS
xC      RtnC4C5,        VEXV or VEXM,                   $F0,xO,xY,TLDDQU
xC      RtnC4C5,        VEXL,                           $5F,xQ,xN,TMAXSD
xC      RtnC4C5,        VEXL,                           $5D,xQ,xN,TMINSD
xC      RtnC4C5,        VEXV,                           $12,xQ,xY,TMOVDDUP
xC      C4C5XMJ,        VEXL,                           $10,xQ,xN,TMOVSD
xC      C4C5MXJ,        VEXL,                           $11,xQ,xN,TMOVSD
xC      RtnC4C5,        VEXL,                           $59,xQ,xN,TMULSD
xC      RtnC4C5,        VEXV or VEXI,                   $70,xO,xY,TPSHUFLW
xC      RtnC4C5,        VEXL,                           $51,xQ,xN,TSQRTSD
xC      RtnC4C5,        VEXL,                           $5C,xQ,xN,TSUBSD
xC      RtnC4C509,      VEXL or VEXH,                   $92,xN,xN,TKMOVD
xC      RtnC4C50A,      VEXL or VEXH,                   $93,xN,xN,TKMOVD
dd -1
;-----------------------------------------------------------------------------
C4C5F3:
xC      RtnC4C5,        VEXL,                           $58,xD,xN,TADDSS
xC      RtnC4C5,        VEX8 or VEXL,                   $C2,xD,xN,TCMPSS
xC      RtnC4C5,        VEXV or VEX0 or VEX2,           $E6,xQ,xO,TCVTDQ2PD
xC      RtnC4C512,      VEXL,                           $2A,xD,xN,TCVTSI2SS
xC      RtnC4C5,        VEXL,                           $5A,xD,xN,TCVTSS2SD
xC      RtnC4C511,      VEXV or VEXL,                   $2D,xD,xN,TCVTSS2SI
xC      RtnC4C5,        VEXV,                           $5B,xO,xY,TCVTTPS2DQ
xC      RtnC4C511,      VEXV or VEXL,                   $2C,xD,xN,TCVTTSS2SI
xC      RtnC4C5,        VEXL,                           $5E,xD,xN,TDIVSS
xC      RtnC4C5,        VEXL,                           $5F,xD,xN,TMAXSS
xC      RtnC4C5,        VEXL,                           $5D,xD,xN,TMINSS
xC      RtnC4C5,        VEXV or VEXL,                   $7E,xQ,xN,TMOVQ
xC      RtnC4C5,        VEXV,                           $6F,xO,xY,TMOVDQU
xC      RtnC4C50C,      VEXV,                           $7F,xO,xY,TMOVDQU
xC      RtnC4C5,        VEXV,                           $16,xO,xY,TMOVSHDUP
xC      RtnC4C5,        VEXV,                           $12,xO,xY,TMOVSLDUP
xC      C4C5XMJ,        VEXL,                           $10,xD,xN,TMOVSS
xC      C4C5MXJ,        VEXL,                           $11,xD,xN,TMOVSS
xC      RtnC4C5,        VEXL,                           $59,xD,xN,TMULSS
xC      RtnC4C5,        VEXV or VEXI,                   $70,xO,xY,TPSHUFHW
xC      RtnC4C5,        VEXL,                           $53,xD,xN,TRCPSS
xC      RtnC4C5,        VEXL,                           $51,xD,xN,TSQRTSS
xC      RtnC4C5,        VEXL,                           $52,xD,xN,TRSQRTSS
xC      RtnC4C5,        VEXL,                           $5C,xD,xN,TSUBSS
dd -1
;-----------------------------------------------------------------------------
T620000:
                T62 T6201,54h,TANDPS,EVEXM+EVEXYW,xO,xD
                T62 T6201,55h,TANDNPS,EVEXM+EVEXYW,xO,xD
                T62 T6201,58h,TADDPS,EVEXM+EVEXYW+EVEXR,xO,xD
                T62 T6202,$C2,TCMPPS,EVEXM+EVEXYW+EVEXI,xO,xD
                T62 T6203,$2E,TUCOMISS,EVEXYW+EVEXX+EVEXS+EVEXV,xD,xD
                T62 T6203,$2F,TCOMISS,EVEXYW+EVEXX+EVEXS+EVEXV,xD,xD
                T62 T6201,5Eh,TDIVPS,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T62W0,$5B,TCVT?Q2PS,EVEX2+EVEXR+EVEXB+EVEXM+EVEXE+EVEXV,xO,xD
                T62 T62W1,$79,TCVTP?2UDQ,EVEX2+EVEXR+EVEXM+EVEXE+EVEXV,xO,xQ
                T62 T6206,$5A,TCVTPS2PD,EVEXM+EVEXV+EVEXYW,xO,xQ
                T62 T62W0,$78,TCVTTP?2UDQ,EVEX2+EVEXM+EVEXE+EVEXB+EVEXV,xO,xD
                T62 T6201,5Fh,TMAXPS,EVEXM+EVEXYW,xO,xD
                T62 T6201,5Dh,TMINPS,EVEXM+EVEXYW,xO,xD
                T62 T6204,28h,TMOVAPS,EVEXM+EVEXYW+EVEXV,xO,xO
                T62 T620B,29h,TMOVAPS,EVEXM+EVEXYW+EVEXV,xO,xO
                T62 T620C,12h,TMOVLPS,EVEXLX+EVEXM+EVEXYW,xQ,xQ
                T62 T620E,16h,TMOVHPS,EVEXM+EVEXLX+EVEXYW,xQ,xQ
                T62 T620F,17h,TMOVHPS,EVEXM+EVEXLX+EVEXV+EVEXYW,xQ,xQ
                T62 T620F,13h,TMOVLPS,EVEXM+EVEXLX+EVEXV+EVEXYW,xQ,xQ
                T62 T6210,2Bh,TMOVNTPS,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T6204,10h,TMOVUPS,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T620B,11h,TMOVUPS,EVEXM+EVEXYW+EVEXV,xO,xO
                T62 T6201,59h,TMULPS,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6201,56h,TORPS,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6201,57h,TXORPS,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6201,$C6,TSHUFPS,EVEXM+EVEXYW+EVEXI,xO,xD
                T62 T6204,51h,TSQRTPS,EVEXM+EVEXR+EVEXYW+EVEXV,xO,xD
                T62 T6201,5Ch,TSUBPS,EVEXM+EVEXYW+EVEXR,xO,xD
                T62 T6201,15h,TUNPCKHPS,EVEXM+EVEXYW,xO,xD
                T62 T6201,14h,TUNPCKLPS,EVEXM+EVEXYW,xO,xD
dd -1
T620066:
                T62 T6201,54h,TANDPD,EVEXM+EVEXNW,xO,xQ
                T62 T6201,55h,TANDNPD,EVEXM+EVEXNW,xO,xQ
                T62 T6201,58h,TADDPD,EVEXM+EVEXNW+EVEXR,xO,xQ
                T62 T6202,$C2,TCMPPD,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6203,2Eh,TUCOMISD,EVEXNW+EVEXX+EVEXS+EVEXV,xQ,xQ
                T62 T6203,2Fh,TCOMISD,EVEXNW+EVEXX+EVEXS+EVEXV,xQ,xQ
                T62 T6201,5Eh,TDIVPD,EVEXM+EVEXR+EVEXNW,xO,xQ
                T62 T6205,$5A,TCVTPD2PS,EVEXM+EVEXR+EVEXNW+EVEXV,xO,xQ
                T62 T6206,$7B,TCVTP?2QQ,EVEXM+EVEXR+EVEXE+EVEXV,xO,xQ
                T62 T6206,$79,TCVTP?2UQQ,EVEXM+EVEXR+EVEXE+EVEXV,xO,xQ
                T62 T6204,$5B,TCVTPS2DQ,EVEXM+EVEXR+EVEXYW+EVEXV,xO,xD
                T62 T6205,$E6,TCVTTPD2DQ,EVEXM+EVEXNW+EVEXV,xO,xQ
                T62 T6206,$7A,TCVTTP?2QQ,EVEXM+EVEXE+EVEXV,xO,xQ
                T62 T6206,$78,TCVTTP?2UQQ,EVEXM+EVEXE+EVEXV,xO,xQ
                T62 T6201,5Fh,TMAXPD,EVEXM+EVEXNW,xO,xQ
                T62 T6201,5Dh,TMINPD,EVEXM+EVEXNW,xO,xQ
                T62 T6204,28h,TMOVAPD,EVEXM+EVEXNW+EVEXV,xO,xO
                T62 T620B,29h,TMOVAPD,EVEXM+EVEXNW+EVEXV,xO,xO
                T62 T6211,6Eh,TMOVD,EVEXM+EVEXQ1+EVEXNW+EVEXYW+EVEXV,xD,xD
                T62 T6212,7Eh,TMOVD,EVEXM+EVEXQ1+EVEXNW+EVEXYW+EVEXV,xD,xD
                T62 T620B,$D6,TMOVQ,EVEXM+EVEXNW+EVEXV,xQ,xQ
                T62 T6204,6Fh,TMOVDQA??,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T620B,7Fh,TMOVDQA??,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T620D,16h,TMOVHPD,EVEXM+EVEXLX+EVEXNW,xQ,xQ
                T62 T620D,12h,TMOVLPD,EVEXM+EVEXLX+EVEXNW,xQ,xQ
                T62 T620F,17h,TMOVHPD,EVEXM+EVEXLX+EVEXV+EVEXNW,xQ,xQ
                T62 T620F,13h,TMOVLPD,EVEXM+EVEXLX+EVEXV+EVEXNW,xQ,xQ
                T62 T6210,$E7,TMOVNTDQ,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T6210,2Bh,TMOVNTPD,EVEXM+EVEXV+EVEXNW,xO,xO
                T62 T6204,10h,TMOVUPD,EVEXM+EVEXV+EVEXNW,xO,xO
                T62 T620B,11h,TMOVUPD,EVEXM+EVEXNW+EVEXV,xO,xO
                T62 T6201,$F6,TPSADBW,EVEXM,xO,xO
                T62 T6201,59h,TMULPD,EVEXM+EVEXR+EVEXNW,xO,xQ
                T62 T6201,56h,TORPD,EVEXM+EVEXR+EVEXNW,xO,xQ
                T62 T6201,57h,TXORPD,EVEXM+EVEXR+EVEXNW,xO,xQ
                T62 T6201,63h,TPACKSSWB,EVEXM,xO,xO
                T62 T6201,6Bh,TPACKSSDW,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6201,67h,TPACKUSWB,EVEXM,xO,xO
                T62 T6201,$FC,TPADDB,EVEXM,xO,xO
                T62 T6201,$FD,TPADDW,EVEXM,xO,xO
                T62 T6201,$FE,TPADDD,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6201,$D4,TPADDQ,EVEXM+EVEXR+EVEXNW,xO,xQ
                T62 T6201,$EC,TPADDSB,EVEXM,xO,xO
                T62 T6201,$ED,TPADDSW,EVEXM,xO,xO
                T62 T6201,$DC,TPADDUSB,EVEXM,xO,xO
                T62 T6201,$DD,TPADDUSW,EVEXM,xO,xO
                T62 T6201,$DB,TPANDD,EVEXM+EVEXQ1+EVEXB+EVEXR,xO,xD
                T62 T6201,$DF,TPANDND,EVEXM+EVEXQ1+EVEXB+EVEXR,xO,xD
                T62 T6201,$E0,TPAVGB,EVEXM,xO,xO
                T62 T6201,$E3,TPAVGW,EVEXM,xO,xO
                T62 T6202,$76,TPCMPEQD,EVEXM+EVEXYW,xO,xD
                T62 T6202,$74,TPCMPEQB,EVEXM,xO,xO
                T62 T6202,$75,TPCMPEQW,EVEXM,xO,xO
                T62 T6202,$66,TPCMPGTD,EVEXM+EVEXYW,xO,xD
                T62 T6202,$64,TPCMPGTB,EVEXM,xO,xO
                T62 T6202,$65,TPCMPGTW,EVEXM,xO,xO
                T62 T6213,$C5,TPEXTRW,EVEXV+EVEXI+EVEXM+EVEXLX,xW,xW
                T62 T6201,$F5,TPMADDWD,EVEXM,xO,xO
                T62 T6214,$C4,TPINSRW,EVEXM+EVEXLX+EVEXI,xW,xW
                T62 T6201,$EE,TPMAXSW,EVEXM,xO,xO
                T62 T6201,$DE,TPMAXUB,EVEXM,xO,xO
                T62 T6201,$EA,TPMINSW,EVEXM,xO,xO
                T62 T6201,$DA,TPMINUB,EVEXM,xO,xO
                T62 T6201,$E4,TPMULHUW,EVEXM,xO,xO
                T62 T6201,$E5,TPMULHW,EVEXM,xO,xO
                T62 T6201,$D5,TPMULLW,EVEXM,xO,xO
                T62 T6201,$F4,TPMULUDQ,EVEXNW+EVEXM+EVEXB,xO,xD
                T62 T6201,$EB,TPORD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6204,70h,TPSHUFD,EVEXYW+EVEXM+EVEXI+EVEXV,xO,xD
                T62 T6215,$F1,TPSLLW,EVEXM,xO,xO
                T62 T6215,$F2,TPSLLD,EVEXYW+EVEXM,xO,xO
                T62 T6215,$F3,TPSLLQ,EVEXNW+EVEXM,xO,xO
                T62 T6271,71h,TNULL,EVEXM+EVEXI,xO,xO
                T62 T6272,72h,TNULL,EVEXM+EVEXB+EVEXI,xO,xD
                T62 T6273,73h,TNULL,EVEXM+EVEXB+EVEXI,xO,xD
                T62 T6215,$E1,TPSRAW,EVEXM,xO,xO
                T62 T6215,$E2,TPSRAD,EVEXQ1+EVEXM,xO,xO
                T62 T6215,$D1,TPSRLW,EVEXM,xO,xO
                T62 T6215,$D2,TPSRLD,EVEXYW+EVEXQ1+EVEXM,xO,xO
                T62 T6215,$D3,TPSRLQ,EVEXNW+EVEXM,xO,xO
                T62 T6201,$F8,TPSUBB,EVEXM,xO,xD
                T62 T6201,$F9,TPSUBW,EVEXM,xO,xD
                T62 T6201,$FA,TPSUBD,EVEXM+EVEXYW+EVEXB,xO,xD
                T62 T6201,$FB,TPSUBQ,EVEXM+EVEXNW+EVEXB,xO,xD
                T62 T6201,$E8,TPSUBSB,EVEXM,xO,xD
                T62 T6201,$E9,TPSUBSW,EVEXM,xO,xD
                T62 T6201,$D8,TPSUBUSB,EVEXM,xO,xD
                T62 T6201,$D9,TPSUBUSW,EVEXM,xO,xD
                T62 T6201,68h,TPUNPCKHBW,EVEXM,xO,xD
                T62 T6201,69h,TPUNPCKHWD,EVEXM,xO,xD
                T62 T6201,6Ah,TPUNPCKHDQ,EVEXYW+EVEXM,xO,xD
                T62 T6201,6Dh,TPUNPCKHQDQ,EVEXNW+EVEXM,xO,xQ
                T62 T6201,60h,TPUNPCKLBW,EVEXM,xO,xD
                T62 T6201,61h,TPUNPCKLWD,EVEXM,xO,xD
                T62 T6201,62h,TPUNPCKLDQ,EVEXYW+EVEXM,xO,xD
                T62 T6201,6Ch,TPUNPCKLQDQ,EVEXNW+EVEXM,xO,xQ
                T62 T6201,$C6,TSHUFPD,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6206,51h,TSQRTPD,EVEXM+EVEXR+EVEXNW+EVEXV,xO,xQ
                T62 T6201,$EF,TPXORD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,5Ch,TSUBPD,EVEXM+EVEXNW+EVEXR,xO,xQ
                T62 T6201,15h,TUNPCKHPD,EVEXM+EVEXNW,xO,xQ
                T62 T6201,14h,TUNPCKLPD,EVEXM+EVEXNW,xO,xQ
dd -1
T6200F3:
                T62 T6201,5Ch,TSUBSS,EVEXM+EVEXYW+EVEXR+EVEXX,xD,xD
                T62 T6201,58h,TADDSS,EVEXM+EVEXYW+EVEXR+EVEXX,xD,xD
                T62 T6201,59h,TMULSS,EVEXM+EVEXYW+EVEXR+EVEXX,xD,xD
                T62 T6202,$C2,TCMPSS,EVEXM+EVEXYW+EVEXI+EVEXX,xD,xD
                T62 T6201,5Eh,TDIVSS,EVEXM+EVEXYW+EVEXR+EVEXX,xD,xD
                T62 T62W2,$E6,TCVT?Q2PD,EVEX2+EVEXO+EVEXR+EVEXM+EVEXE+EVEXB+EVEXV,xO,xD
                T62 T6201,5Ah,TCVTSS2SD,EVEXM+EVEXX+EVEXYW,xD,xD
                T62 T6213,2Dh,TCVTSS2SI,EVEXM+EVEXR+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T6214,2Ah,TCVTSI2SS,EVEXU+EVEXR+EVEXM+EVEXX+EVEXNW+EVEXYW,xD,xD
                T62 T6213,79h,TCVTSS2USI,EVEXM+EVEXX+EVEXW0+EVEXV,xD,xD
                T62 T6213,2Ch,TCVTTSS2SI,EVEXM+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T6213,78h,TCVTTSS2USI,EVEXM+EVEXX+EVEXW0+EVEXV,xD,xD
                T62 T6204,$5B,TCVTTPS2DQ,EVEXM+EVEXYW+EVEXV,xO,xD
                T62 T6206,$7A,TCVTU?Q2PD,EVEXM+EVEXO+EVEXR+EVEXE+EVEXV,xO,xQ
                T62 T6214,7Bh,TCVTUSI2SS,EVEXU+EVEXR+EVEXX+EVEXM+EVEXNW+EVEXYW,xD,xD
                T62 T6201,5Fh,TMAXSS,EVEXM+EVEXX+EVEXYW,xD,xD
                T62 T6201,5Dh,TMINSS,EVEXM+EVEXX+EVEXYW,xD,xD
                T62 T6204,7Eh,TMOVQ,EVEXM+EVEXNW+EVEXV,xQ,xQ
                T62 T6204,6Fh,TMOVDQU?2,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T620B,7Fh,TMOVDQU?2,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T6204,16h,TMOVSHDUP,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T6204,12h,TMOVSLDUP,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T6216,10h,TMOVSS,EVEXM+EVEXX+EVEXYW,xD,xD
                T62 T6217,11h,TMOVSS,EVEXM+EVEXX+EVEXYW,xD,xD
                T62 T6204,70h,TPSHUFHW,EVEXM+EVEXI+EVEXV,xO,xO
                T62 T6201,51h,TSQRTSS,EVEXX+EVEXM+EVEXYW+EVEXR+EVEXX,xD,xD
dd -1
T6200F2:
                T62 T6201,5Ch,TSUBSD,EVEXM+EVEXNW+EVEXR+EVEXX,xQ,xQ
                T62 T6201,58h,TADDSD,EVEXM+EVEXNW+EVEXR+EVEXX,xQ,xQ
                T62 T6201,59h,TMULSD,EVEXM+EVEXNW+EVEXR+EVEXX,xQ,xQ
                T62 T6202,$C2,TCMPSD,EVEXM+EVEXNW+EVEXI+EVEXX,xQ,xQ
                T62 T6201,5Eh,TDIVSD,EVEXM+EVEXNW+EVEXR+EVEXX,xQ,xQ
                T62 T6205,$E6,TCVTPD2DQ,EVEXM+EVEXR+EVEXNW+EVEXV,xO,xQ
                T62 T6213,2Dh,TCVTSD2SI,EVEXM+EVEXR+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T6213,79h,TCVTSD2USI,EVEXM+EVEXR+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T6201,5Ah,TCVTSD2SS,EVEXM+EVEXR+EVEXNW,xQ,xQ
                T62 T6214,2Ah,TCVTSI2SD,EVEXU+EVEXR+EVEXM+EVEXX+EVEXNW+EVEXYW,xD,xD
                T62 T6213,2Ch,TCVTTSD2SI,EVEXM+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T6213,78h,TCVTTSD2USI,EVEXM+EVEXX+EVEXW0+EVEXV,xQ,xQ
                T62 T62W1,7Ah,TCVTU?Q2PS,EVEX2+EVEXR+EVEXM+EVEXE+EVEXV,xO,xQ
                T62 T6214,7Bh,TCVTUSI2SD,EVEXU+EVEXR+EVEXX+EVEXM+EVEXNW+EVEXYW,xD,xD
                T62 T6201,5Fh,TMAXSD,EVEXM+EVEXNW,xQ,xQ
                T62 T6201,5Dh,TMINSD,EVEXM+EVEXNW,xQ,xQ
                T62 T6218,12h,TMOVDDUP,EVEXM+EVEXNW+EVEXV,xQ,xQ
                T62 T6204,6Fh,TMOVDQU??,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T620B,7Fh,TMOVDQU??,EVEXM+EVEXE+EVEXV,xO,xO
                T62 T6216,10h,TMOVSD,EVEXM+EVEXX+EVEXNW,xQ,xQ
                T62 T6217,11h,TMOVSD,EVEXM+EVEXX+EVEXNW,xQ,xQ
                T62 T6204,70h,TPSHUFLW,EVEXM+EVEXI+EVEXV,xO,xO
                T62 T6201,51h,TSQRTSD,EVEXM+EVEXNW+EVEXR+EVEXX,xQ,xQ
dd -1
T623800:
dd -1
T623866:
                T62 T6201,64h,TPBLENDMD,EVEXB+EVEXM+EVEXQ1+EVEX0,xX,xD
                T62 T6201,65h,TBLENDMPS,EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,66h,TPBLENDM?,EVEXM+EVEXE,xX,0
                T62 T6203,19h,TBROADCAST??,EVEXL0+EVEXE+EVEX0,xQ,xQ
                T62 T6203,18h,TBROADCASTSS,EVEXYW,xD,xD
                T62 T6219,1Ah,TBROADCASTF??X2,EVEXL0+EVEXE+EVEXV,xO,xO
                T62 T621A,1Bh,TBROADCASTF??X?,EVEXLL+EVEXE+EVEXV,xY,xY
                T62 T62RB,7Ah,TPBROADCASTB,EVEXYW+EVEXV,xN,xN
                T62 T62RW,7Bh,TPBROADCASTW,EVEXYW+EVEXV,xN,xN
                T62 T62RD,7Ch,TPBROADCASTD,EVEXQ1+EVEXV,xN,xN
                T62 T6203,78h,TPBROADCASTB,EVEXYW+EVEXV,xB,xB
                T62 T6203,79h,TPBROADCASTW,EVEXYW+EVEXV,xW,xW
                T62 T6203,58h,TPBROADCASTD,EVEXYW+EVEXV,xD,xD
                T62 T6203,59h,TBROADCASTI??,EVEXE+EVEXV,xQ,xQ
                T62 T6219,5Ah,TBROADCASTI??X2,EVEXL0+EVEXE+EVEXV,xX,xX
                T62 T621A,5Bh,TBROADCASTI??X?,EVEXLL+EVEXE+EVEXV,xY,xY
                T62 T620B,8Ah,TCOMPRESSPS,EVEXM+EVEXD1+EVEXV,xO,xO
                T62 T6207,13h,TCVTPH2PS,EVEXM+EVEXYW+EVEXV,xQ,xQ
                T62 T6204,88h,TEXPANDPS,EVEXM+EVEXD1+EVEXV,xO,xO
                T62 T6201,98h,TFMADD132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$A8,TFMADD213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$B8,TFMADD231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$96,TFMADDSUB132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$A6,TFMADDSUB213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$B6,TFMADDSUB231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$97,TFMSUBADD132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$A7,TFMSUBADD213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$B7,TFMSUBADD231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$9A,TFMSUB132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$AA,TFMSUB213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$BA,TFMSUB231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$9C,TFNMADD132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$AC,TFNMADD213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$BC,TFNMADD231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$9E,TFNMSUB132PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$AE,TFNMSUB213PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$BE,TFNMSUB231PS,EVEXR+EVEXB+EVEXM+EVEXD1,xX,xD
                T62 T6201,$99,TFMADD132SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$A9,TFMADD213SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$B9,TFMADD231SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$9B,TFMSUB132SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$AB,TFMSUB213SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$BB,TFMSUB231SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$9D,TFNMADD132SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$AD,TFNMADD213SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$BD,TFNMADD231SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$9F,TFNMSUB132SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$AF,TFNMSUB213SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6201,$BF,TFNMSUB231SS,EVEXR+EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T6200,$90,TPGATHERDD,EVEXM+EVEXQ1,xN,xN
                T62 T621B,$91,TPGATHERQD,EVEXM+EVEXQ1,xN,xN
                T62 T6200,$92,TGATHERDPS,EVEXM+EVEXD1,xN,xN
                T62 T621B,$93,TGATHERQPS,EVEXM+EVEXD1,xN,xN
                T62 T6204,$42,TGETEXPPS,EVEXB+EVEXM+EVEXD1+EVEXV,xO,xD
                T62 T6201,$43,TGETEXPSS,EVEXU+EVEXX+EVEXM+EVEXD1,xD,xD
                T62 T621C,2Ah,TMOVNTDQA,EVEXM+EVEXV+EVEXYW,xO,xO
                T62 T6204,1Ch,TPABSB,EVEXM+EVEXV,xO,xO
                T62 T6204,1Dh,TPABSW,EVEXM+EVEXV,xO,xO
                T62 T6204,1Eh,TPABSD,EVEXM+EVEXR+EVEXYW+EVEXV,xO,xD
                T62 T6204,1Fh,TPABSQ,EVEXM+EVEXR+EVEXNW+EVEXV,xO,xQ
                T62 T6201,2Bh,TPACKUSDW,EVEXM+EVEXR+EVEXYW,xO,xD
                T62 T6202,29h,TPCMPEQQ,EVEXM+EVEXNW,xO,xQ
                T62 T6202,37h,TPCMPGTQ,EVEXM+EVEXNW,xO,xQ
                T62 T620B,8Bh,TPCOMPRESSD,EVEXM+EVEXQ1+EVEXV,xO,xO
                T62 T6204,$C4,TPCONFLICTD,EVEXM+EVEXB+EVEXQ1+EVEXV,xO,xD
                T62 T6201,8Dh,TPERM?,EVEXM+EVEXE,xO,xO
                T62 T6201,36h,TPERMD,EVEXM+EVEXQ1+EVEXB+EVEXL0,xO,xD
                T62 T6201,75h,TPERMI2?,EVEXM+EVEXE,xO,xO
                T62 T6201,76h,TPERMI2D,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6201,77h,TPERMI2PS,EVEXM+EVEXB+EVEXD1,xO,xD
                T62 T6201,7Dh,TPERMI2?2,EVEXM+EVEXE,xO,xO
                T62 T6201,7Eh,TPERMT2D,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6201,7Fh,TPERMT2PS,EVEXM+EVEXB+EVEXD1,xO,xD
                T62 T6201,0Dh,TPERMILPD,EVEXNW+EVEXM+EVEXB,xO,xD
                T62 T6201,0Ch,TPERMILPS,EVEXYW+EVEXM+EVEXB,xO,xD
                T62 T6201,16h,TPERMPS,EVEXD1+EVEXM+EVEXB,xO,xD
                T62 T6204,89h,TPEXPANDD,EVEXM+EVEXQ1+EVEXV,xO,xO
                T62 T6204,44h,TPLZCNTD,EVEXQ1+EVEXM+EVEXB+EVEXV,xO,xD
                T62 T6201,04h,TPMADDUBSW,EVEXM,xO,xO
                T62 T6201,$B4,TPMADD52LUQ,EVEXNW+EVEXM+EVEXB,xO,xD
                T62 T6201,$B5,TPMADD52HUQ,EVEXNW+EVEXM+EVEXB,xO,xD
                T62 T6201,3Ch,TPMAXSB,EVEXM,xO,xO
                T62 T6201,3Dh,TPMAXSD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,3Eh,TPMAXUW,EVEXM,xO,xO
                T62 T6201,3Fh,TPMAXUD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,38h,TPMINSB,EVEXM,xO,xO
                T62 T6201,39h,TPMINSD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,3Ah,TPMINUW,EVEXM,xO,xO
                T62 T6201,3Bh,TPMINUD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6207,20h,TPMOVSXBW,EVEXM+EVEXV,xQ,xQ
                T62 T6207,23h,TPMOVSXWD,EVEXM+EVEXV,xQ,xQ
                T62 T6207,25h,TPMOVSXDQ,EVEXYW+ EVEXM+EVEXV,xQ,xQ
                T62 T6203,21h,TPMOVSXBD,EVEXM+EVEXV,xD,xD
                T62 T6203,22h,TPMOVSXBQ,EVEXM+EVEXV,xW,xW
                T62 T6203,24h,TPMOVSXWQ,EVEXM+EVEXV,xD,xD
                T62 T6207,30h,TPMOVZXBW,EVEXM+EVEXV,xQ,xQ
                T62 T6207,33h,TPMOVZXWD,EVEXM+EVEXV,xQ,xQ
                T62 T6207,35h,TPMOVZXDQ,EVEXYW+EVEXM+EVEXV,xQ,xQ
                T62 T6203,31h,TPMOVZXBD,EVEXM+EVEXV,xD,xD
                T62 T6203,32h,TPMOVZXBQ,EVEXM+EVEXV,xW,xW
                T62 T6203,34h,TPMOVZXWQ,EVEXM+EVEXV,xD,xD
                T62 T6201,28h,TPMULDQ,EVEXM+EVEXNW,xO,xQ
                T62 T6201,0Bh,TPMULHRSW,EVEXM,xO,xO
                T62 T6201,40h,TPMULLD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,83h,TPMULTISHIFTQB,EVEXNW+EVEXM+EVEXB,xO,xD
                T62 T6201,15h,TPROLVD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,14h,TPRORVD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T62V1XYZ,$A0,TPSCATTERDD,EVEXM+EVEXQ1,xD,xD
                T62 T62V2XYZ,$A1,TPSCATTERQD,EVEXM+EVEXQ1,xD,xD
                T62 T6201,00h,TPSHUFB,EVEXM,xO,xO
                T62 T6201,12h,TPSLLVW,EVEXNW+EVEXM,xO,xO
                T62 T6201,47h,TPSLLVD,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6201,10h,TPSRLVW,EVEXNW+EVEXM,xO,xO
                T62 T6201,45h,TPSRLVD,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6202,26h,TPTESTM?,EVEXM+EVEXE,xO,xO
                T62 T6202,27h,TPTESTMD,EVEXQ1+EVEXM+EVEXB,xO,xD
                T62 T6201,11h,TPSRAVW,EVEXM+EVEXNW,xO,xO
                T62 T6201,46h,TPSRAVD,EVEXQ1+EVEXB+EVEXM,xO,xD
                T62 T6204,4Ch,TRCP14PS,EVEXM+EVEXB+EVEXD1,xO,xD
                T62 T6201,4Dh,TRCP14SS,EVEXX+EVEXM+EVEXU+EVEXD1,xD,xD
                T62 T6204,4Eh,TRSQRT14PS,EVEXM+EVEXB+EVEXD1,xO,xD
                T62 T6201,2Ch,TSCALEFPS,EVEXM+EVEXB+EVEXR+EVEXD1,xO,xD
                T62 T6201,4Fh,TRSQRT14SS,EVEXX+EVEXU+EVEXM+EVEXD1,xD,xD
                T62 T6201,2Dh,TSCALEFSS,EVEXX+EVEXU+EVEXM+EVEXR+EVEXD1,xD,xD
                T62 T62V1XYZ,$A2,TSCATTERDPS,EVEXM+EVEXD1,xD,xD
                T62 T62V2XYZ,$A3,TSCATTERQPS,EVEXM+EVEXD1,xD,xD
                T62 T6204,$C8,TEXP2PS,EVEXLL+EVEXM+EVEXB+EVEXD1+EVEXV,xO,xD
                T62 T6204,$CA,TRCP28PS,EVEXLL+EVEXM+EVEXB+EVEXD1+EVEXV,xO,xD
                T62 T6204,$CC,TRSQRT28PS,EVEXLL+EVEXM+EVEXB+EVEXD1+EVEXV,xO,xD
                T62 T6201,$CB,TRCP28SS,EVEXX+EVEXU+EVEXM+EVEXD1,xD,xD
                T62 T6201,$CD,TRSQRT28SS,EVEXX+EVEXU+EVEXM+EVEXD1,xD,xD
                T62 T62XYZVV1,$C6,TNULL,EVEXU,xD,xD
                T62 T62XYZVV2,$C7,TNULL,EVEXU,xD,xD
                T62 T6201,$CF,TGF2P8MULB,EVEXR+EVEXB+EVEXM+EVEXYW,xX,xD
                T62 T6201,$DE,TAESDEC,EVEXM,xO,xO
                T62 T6201,$DF,TAESDECLAST,EVEXM,xO,xO
                T62 T6201,$DC,TAESENC,EVEXM,xO,xO
                T62 T6201,$DD,TAESENCLAST,EVEXM,xO,xO
                T62 T6201,50h,TPDPBUSD,EVEXM+EVEXB,xO,xD
                T62 T6201,51h,TPDPBUSDS,EVEXM+EVEXB,xO,xD
                T62 T6201,52h,TPDPWSSD,EVEXM+EVEXB,xO,xD
                T62 T6201,53h,TPDPWSSDS,EVEXM+EVEXB,xO,xD
                T62 T6204,54h,TPOPCNT?A,EVEXM+EVEXV+EVEXE,xO,xO
                T62 T6204,55h,TPOPCNTD,EVEXM+EVEXV+EVEXQ1,xO,xO
                T62 T6201,70h,TPSHLDVW,EVEXM+EVEXNW,xO,xQ
                T62 T6201,71h,TPSHLDVD,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6201,72h,TPSHRDVW,EVEXM+EVEXNW,xO,xQ
                T62 T6201,73h,TPSHRDVD,EVEXM+EVEXB+EVEXQ1,xO,xD
                T62 T6202,8Fh,TPSHUFBITQMB,EVEXM,xO,xO
                T62 T6204,62h,TPEXPAND?,EVEXM+EVEXV+EVEXE,xO,xO
                T62 T620B,63h,TPCOMPRESS?,EVEXM+EVEXV+EVEXE,xO,xO
dd -1
T6238F3:
                T62 T621D,2Ah,TPBROADCASTMB2Q,EVEXV+EVEXM+EVEXNW,xN,xN
                T62 T621D,3Ah,TPBROADCASTMW2D,EVEXV+EVEXM+EVEXYW,xN,xN
                T62 T621D,28h,TPMOVM2?,EVEXE+EVEXV+EVEXM,xN,xN
                T62 T621D,38h,TPMOVM2D,EVEXQ1+EVEXV+EVEXM,xN,xN
                T62 T621E,29h,TPMOVB?2M,EVEXE+EVEXV+EVEXM,xN,xN
                T62 T621E,39h,TPMOVD?2M,EVEXE+EVEXV+EVEXM,xN,xN
                T62 T6220,12h,TPMOVUSQB,EVEXYW+EVEXV+EVEXM+EVEXB,xW,xW
                T62 T6220,22h,TPMOVSQB,EVEXYW+EVEXV+EVEXM+EVEXB,xW,xW
                T62 T6220,32h,TPMOVQB,EVEXYW+EVEXV+EVEXM+EVEXB,xW,xW
                T62 T6220,14h,TPMOVUSQW,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T6220,24h,TPMOVSQW,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T6220,34h,TPMOVQW,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T620A,15h,TPMOVUSQD,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,25h,TPMOVSQD,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,35h,TPMOVQD,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T6220,11h,TPMOVUSDB,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T6220,21h,TPMOVSDB,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T6220,31h,TPMOVDB,EVEXYW+EVEXV+EVEXM+EVEXB,xD,xD
                T62 T620A,13h,TPMOVUSDW,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,23h,TPMOVSDW,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,33h,TPMOVDW,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,10h,TPMOVUSWB,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,20h,TPMOVSWB,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T620A,30h,TPMOVWB,EVEXYW+EVEXV+EVEXM+EVEXB,xQ,xQ
                T62 T6202,26h,TPTESTNM?,EVEXM+EVEXE,xO,xO
                T62 T6202,27h,TPTESTNMD,EVEXQ1+EVEXM+EVEXB,xO,xD
dd -1
T6238F2:
                T62 T6244,9Ah,T4FMADDPS,EVEXYW+EVEXM+EVEXLL,xO,xO
                T62 T6244,$AA,T4FNMADDPS,EVEXYW+EVEXM+EVEXLL,xO,xO
                T62 T6244,9Bh,T4FMADDSS,EVEXYW+EVEXM+EVEXX,xO,xO
                T62 T6244,$AB,T4FNMADDSS,EVEXYW+EVEXM+EVEXX,xO,xO
                T62 T6244,53h,TP4DPWSSDS,EVEXYW+EVEXM+EVEXLL,xO,xO
                T62 T6244,52h,TP4DPWSSD,EVEXYW+EVEXM+EVEXLL,xO,xO
dd -1
T623A00:
dd -1
T623A66:
                T62 T6201,03h,TALIGND,EVEXB+EVEXM+EVEXQ1+EVEXI,xO,xD
                T62 T620A,1Dh,TCVTPS2PH,EVEXM+EVEXI+EVEXYW+EVEXV,xQ,xQ
                T62 T6201,42h,TDBPSADBW,EVEXM+EVEXYW+EVEXI,xO,xO
                T62 T6221,19h,TEXTRACTF??X2,EVEXE+EVEXM+EVEXV+EVEXI,xO,xO
                T62 T6222,1Bh,TEXTRACTF??X?,EVEXE+EVEXM+EVEXV+EVEXI,xY,xY
                T62 T6221,39h,TEXTRACTI??X2,EVEXE+EVEXM+EVEXV+EVEXI,xO,xO
                T62 T6222,3Bh,TEXTRACTI??X?,EVEXE+EVEXM+EVEXV+EVEXI,xY,xY
                T62 T6223,17h,TEXTRACTPS,EVEXV+EVEXI,xD,xD
                T62 T6201,54h,TFIXUPIMMPS,EVEXB+EVEXM+EVEXD1+EVEXI,xO,xD
                T62 T6201,55h,TFIXUPIMMSS,EVEXU+EVEXX+EVEXM+EVEXD1+EVEXI,xD,xD
                T62 T621F,66h,TFPCLASSPS,EVEXB+EVEXM+EVEXD1+EVEXV+EVEXI,xO,xD
                T62 T621F,67h,TFPCLASSSS,EVEXU+EVEXX+EVEXM+EVEXD1+EVEXV+EVEXI,xD,xD
                T62 T6204,26h,TGETMANTPS,EVEXB+EVEXM+EVEXD1+EVEXV+EVEXI,xO,xD
                T62 T6201,$27,TGETMANTSS,EVEXU+EVEXX+EVEXM+EVEXD1+EVEXI,xD,xD
                T62 T6224,18h,TINSERTF??X?,EVEXE+EVEXM+EVEXI,xX,xX
                T62 T6225,1Ah,TINSERTF??X2,EVEXE+EVEXM+EVEXI,xX,xX
                T62 T6224,38h,TINSERTI??X2,EVEXE+EVEXM+EVEXI,xX,xX
                T62 T6225,3Ah,TINSERTI??X?,EVEXE+EVEXM+EVEXI,xX,xX
                T62 T6201,$21,TINSERTPS,EVEXU+EVEXM+EVEXD1+EVEXYW+EVEXI,xD,xD
                T62 T6201,0Fh,TPALIGNR,EVEXM+EVEXI,xO,xO
                T62 T6202,3Fh,TPCMP?,EVEXM+EVEXE+EVEXI,xO,xO
                T62 T6202,3Eh,TPCMPU?,EVEXM+EVEXE+EVEXI,xO,xO
                T62 T6202,1Fh,TPCMPD,EVEXQ1+EVEXB+EVEXM+EVEXI,xO,xD
                T62 T6202,1Eh,TPCMPUD,EVEXQ1+EVEXB+EVEXM+EVEXI,xO,xD
                T62 T6204,05h,TPERMILPD,EVEXNW+EVEXV+EVEXB+EVEXM+EVEXI,xO,xD
                T62 T6204,04h,TPERMILPS,EVEXYW+EVEXB+EVEXM+EVEXV+EVEXI,xO,xD
                T62 T6204,01h,TPERMPD,EVEXNW+EVEXL0+EVEXV+EVEXB+EVEXM+EVEXI,xO,xD
                T62 T6204,00h,TPERMQ,EVEXNW+EVEXL0+EVEXV+EVEXB+EVEXM+EVEXI,xO,xD
                T62 T6212,14h,TPEXTRB,EVEXV+EVEXI+EVEXM+EVEXLX,xB,xB
                T62 T6212,15h,TPEXTRW,EVEXV+EVEXI+EVEXM+EVEXLX,xW,xW
                T62 T6212,16h,TPEXTRD,EVEXV+EVEXI+EVEXM+EVEXLX+EVEXU+EVEXQ1,xD,xD
                T62 T6214,20h,TPINSRB,EVEXM+EVEXLX+EVEXI,xB,xB
                T62 T6214,22h,TPINSRD,EVEXQ1+EVEXU+EVEXM+EVEXLX+EVEXI,xD,xD
                T62 T6201,23h,TSHUFF?,EVEXE+EVEXM+EVEXL0+EVEXB+EVEXI,xO,xD
                T62 T6201,43h,TSHUFI?,EVEXE+EVEXM+EVEXL0+EVEXB+EVEXI,xO,xD
                T62 T6201,25h,TPTERNLOGD,EVEXM+EVEXB+EVEXQ1+EVEXI,xO,xD
                T62 T6201,50h,TRANGEPS,EVEXM+EVEXB+EVEXD1+EVEXI,xO,xD
                T62 T6201,51h,TRANGESS,EVEXX+EVEXM+EVEXU+EVEXD1+EVEXI,xD,xD
                T62 T6204,56h,TREDUCEPS,EVEXD1+EVEXB+EVEXM+EVEXI+EVEXV,xO,xD
                T62 T6201,57h,TREDUCESS,EVEXX+EVEXM+EVEXU+EVEXD1+EVEXI,xD,xD
                T62 T6204,09h,TRNDSCALEPD,EVEXB+EVEXM+EVEXNW+EVEXI+EVEXV,xO,xD
                T62 T6204,08h,TRNDSCALEPS,EVEXB+EVEXM+EVEXYW+EVEXI+EVEXV,xO,xD
                T62 T6201,0Bh,TRNDSCALESD,EVEXX+EVEXM+EVEXNW+EVEXI,xQ,xQ
                T62 T6201,0Ah,TRNDSCALESS,EVEXX+EVEXM+EVEXYW+EVEXI,xD,xD
                T62 T6201,$CE,TGF2P8AFFINEQB,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6201,$CF,TGF2P8AFFINEINVQB,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6201,44h,TPCLMULQDQ,EVEXM+EVEXI,xO,xO
                T62 T6201,70h,TPSHLDW,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6201,71h,TPSHLDD,EVEXM+EVEXB+EVEXI+EVEXQ1,xO,xD
                T62 T6201,72h,TPSHRDW,EVEXM+EVEXNW+EVEXI,xO,xQ
                T62 T6201,73h,TPSHRDD,EVEXM+EVEXB+EVEXI+EVEXQ1,xO,xD
dd -1
T623AF3:
dd -1
T623AF2:
dd -1
;-----------------------------------------------------------------------------
T62Names:
TEXTRACTI??X?:  dw TEXTRACTI32X8   ,TEXTRACTI64X4
TEXTRACTF??X?:  dw TEXTRACTF32X8   ,TEXTRACTF64X4
TEXTRACTF??X2:  dw TEXTRACTF32X4   ,TEXTRACTF64X2
TEXTRACTI??X2:  dw TEXTRACTI32X4   ,TEXTRACTI64X2
TINSERTI??X?:   dw TINSERTI32X8    ,TINSERTI64X4
TINSERTI??X2:   dw TINSERTI32X4    ,TINSERTI64X2
TINSERTF??X?:   dw TINSERTF32X4    ,TINSERTF64X2
TINSERTF??X2:   dw TINSERTF32X8    ,TINSERTF64X4
TMOVDQA??:      dw TMOVDQA32       ,TMOVDQA64
TCVTTP?2UDQ:    dw TCVTTPS2UDQ     ,TCVTTPD2UDQ
TMOVDQU??:      dw TMOVDQU8        ,TMOVDQU16
TMOVDQU?2:      dw TMOVDQU32       ,TMOVDQU64
TCVT?Q2PS:      dw TCVTDQ2PS       ,TCVTQQ2PS
TCVTP?2QQ:      dw TCVTPS2QQ       ,TCVTPD2QQ
TCVTTP?2QQ:     dw TCVTTPS2QQ      ,TCVTTPD2QQ
TCVTTP?2UQQ:    dw TCVTTPS2UQQ     ,TCVTTPD2UQQ
TCVTU?Q2PD:     dw TCVTUDQ2PD      ,TCVTUQQ2PD
TCVT?Q2PD:      dw TCVTDQ2PD       ,TCVTQQ2PD
TCVTP?2UQQ:     dw TCVTPS2UQQ      ,TCVTPD2UQQ
TCVTP?2UDQ:     dw TCVTPS2UDQ      ,TCVTPD2UDQ
TCVTU?Q2PS:     dw TCVTUDQ2PS      ,TCVTUQQ2PS
TBROADCASTI??:  dw TBROADCASTI32X2 ,TPBROADCASTQ
TBROADCASTF??X?:dw TBROADCASTF32X8 ,TBROADCASTF64X4
TBROADCASTI??X?:dw TBROADCASTI32X8 ,TBROADCASTI64X4
TBROADCASTF??X2:dw TBROADCASTF32X4 ,TBROADCASTF64X2
TBROADCASTI??X2:dw TBROADCASTI32X4 ,TBROADCASTI64X2
TBROADCAST??:   dw TBROADCASTF32X2 ,TBROADCASTSD
TPBLENDM?:      dw TPBLENDMB       ,TPBLENDMW
TPCMP?:         dw TPCMPB          ,TPCMPW
TPCMPU?:        dw TPCMPUB         ,TPCMPUW
TPERM?:         dw TPERMB          ,TPERMW
TPERMI2?:       dw TPERMI2B        ,TPERMI2W
TPERMI2?2:      dw TPERMT2B        ,TPERMT2W
TPMOVM2?:       dw TPMOVM2B        ,TPMOVM2W
TPMOVB?2M:      dw TPMOVB2M        ,TPMOVW2M
TPMOVD?2M:      dw TPMOVD2M        ,TPMOVQ2M
TPTESTNM?:      dw TPTESTNMB       ,TPTESTNMW
TPTESTM?:       dw TPTESTMB        ,TPTESTMW
TSHUFF?:        dw TSHUFF32X4      ,TSHUFF64X2
TSHUFI?:        dw TSHUFI32X4      ,TSHUFI64X2
TPOPCNT?A:      dw TPOPCNTB        ,TPOPCNTW
TPCOMPRESS?:    dw TPCOMPRESSB     ,TPCOMPRESSW
TPEXPAND?:      dw TPEXPANDB       ,TPEXPANDW
;-----------------------------------------------------------------------------
T62W0:          dr T6204,T6205
T62W1:          dr T6208,T6205
T62W2:          dr T6209,T6204
;-----------------------------------------------------------------------------
