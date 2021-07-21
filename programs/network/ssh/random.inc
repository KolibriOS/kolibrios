; ----------------------------- MOTHER.ASM -----------------------------
; Mother-of-All random number generator by Agner Fog 1998
; 32-bit mode version for 80x86 and compatible microprocessors
;
; MRandom returns a floating point number between 0 and 1.
; MRandomInit must be called before the first call to MRandom.
;
; C++ prototypes:
; extern "C" void MRandomInit (int seed);
; extern "C" double MRandom (void);
; extern "C" int MIRandom (int min, int max);
;
; © 1998, 2004 Agner Fog. 
; GNU General Public License www.gnu.org/copyleft/gpl.html
; ----------------------------------------------------------------------

; The MRandom function is optimized for the Pentium microprocessor.

iglobal
        mf3     dd 2111111111           ; factors
        mf2     dd 1492
        mf1     dd 1776
        mf0     dd 5115
endg


uglobal
        m0      dd ?                    ; history buffer
        m1      dd ?
        m2      dd ?
        m3      dd ?
        mc      dd ?
        temprng dq ?                    ; used for conversion to float
endg


proc init_random

        xor     ecx, ecx
        ; make random numbers and put them into buffer
  @@:
        imul    eax, 29943829
        dec     eax
        mov     [m0+ecx*4], eax
        inc     ecx
        cmp     ecx, 5
        jb      @r
        push    edi
        mov     edi, 19
  @@:
        call    MRandom
        fstp    st0
        dec     edi
        jnz     @r
        pop     edi
        ret

endp


proc MRandom

        call    MBRandom                ; random bits
        mov     edx, eax                ; fast conversion to float
        shr     eax, 12
        or      eax, 3ff00000h
        shl     edx, 20
        mov     dword[temprng+4], eax
        mov     dword[temprng], edx
        fld1
        fld     [temprng]               ; partial memory stall here
        fsubr   st0, st1
        ret

endp


proc MIRandom, max, min                 ; make random integer in desired interval

        call    MBRandom                ; make random number
        mov     edx, [max]
        mov     ecx, [min]
        sub     edx, ecx
        js      .error                  ; max < min
        inc     edx                     ; max - min + 1
        mul     edx                     ; multiply random number by interval and truncate
        lea     eax, [edx+ecx]          ; add min
        ret

  .error:
        mov     eax, 80000000h          ; error exit
        ret

endp


proc MBRandom

        push    edi
        mov     eax, [mf3]
        mul     [m3]                    ; x[n-4]
        mov     ecx, eax
        mov     eax, [m2]               ; x[n-3]
        mov     edi, edx
        mov     [m3], eax
        mul     [mf2]
        add     ecx, eax
        mov     eax, [m1]               ; x[n-2]
        adc     edi, edx
        mov     [m2], eax
        mul     [mf1]
        add     ecx, eax
        mov     eax, [m0]               ; x[n-1]
        adc     edi, edx
        mov     [m1], eax
        mul     [mf0]
        add     eax, ecx
        adc     edx, edi
        add     eax, [mc]
        adc     edx, 0
        mov     [m0], eax
        mov     [mc], edx
        pop     edi
        ret

endp