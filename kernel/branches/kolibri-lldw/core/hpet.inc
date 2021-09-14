;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

HPET_ID                 = 0x0000
HPET_PERIOD             = 0x0004
HPET_CFG_ENABLE         = 0x0001
HPET_CFG                = 0x0010
HPET_COUNTER            = 0x00f0
HPET_T0_CFG             = 0x0100

HPET_TN_LEVEL           = 0x0002
HPET_TN_ENABLE          = 0x0004
HPET_TN_FSB             = 0x4000

uglobal
hpet_base         rd 1
hpet_period       rd 1
hpet_timers       rd 1
hpet_tsc_start    rd 2
endg

align 4
init_hpet:
        mov     ebx, [hpet_base]
        test    ebx, ebx
        jz      .done

        mov     eax, [ebx]
        and     ah, 0x1F
        inc     ah
        movzx   eax, ah
        mov     [hpet_timers], eax
        mov     ecx, eax

        mov     eax, [ebx+HPET_PERIOD]
        xor     edx, edx
        shld    edx, eax, 10
        shl     eax, 10
        mov     esi, 1000000
        div     esi
        mov     [hpet_period], eax

        mov     esi, [ebx+HPET_CFG]
        and     esi, not HPET_CFG_ENABLE
        mov     [ebx+HPET_CFG], esi             ;stop main counter

        lea     edx, [ebx+HPET_T0_CFG]
@@:
        jcxz    @F
        mov     eax, [edx]
        and     eax, not (HPET_TN_ENABLE+HPET_TN_LEVEL+HPET_TN_FSB)
        mov     [edx], eax
        add     edx, 0x20
        dec     ecx
        jmp     @B
@@:
        mov     [ebx+HPET_COUNTER], ecx         ;reset main counter
        mov     [ebx+HPET_COUNTER+4], ecx

        or      esi, HPET_CFG_ENABLE
        mov     [ebx+HPET_CFG], esi             ;and start again

.done:
        rdtsc
        mov     [hpet_tsc_start], eax
        mov     [hpet_tsc_start+4], edx

        ret
