;******************************************************************************
; project name:  CPUID                                                        *
; platform:      KolibriOS, x86 (IA-32), x86-64 achitectures                  *
; compiler:      flat assembler 1.70.03                                       *
; version:       2.28                                                         *
; last update:   23rd October 2013                                            *
; maintained by: Sergey Kuzmin aka Wildwest                                   *
; e-mail:        kuzmin_serg@list.ru                                          *
; site:          http://coolthemes.narod.ru/files.html                        *
; license:       Copyright 2004-2013 Sergey Kuzmin and co-authors             *
;                Rules:                                                       *
;                1)you can use pieces of code in your project, but should     *
;                mention the original author (include copyright notice);      *
;                2)if you modify CPUID (improve, port, translate, etc) send   *
;                your changes to the maintainer or make about post changes    *
;                at forum  http://board.kolibrios.org/viewtopic.php?f=42&t=594*
;-----------------------------------------------------------------------------*
; English comments                                                            *
;------------------------------------------------------------------------------
use32
  org	 0x0
  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd U_END+4096
  dd U_END+4096
  dd	 0x0
  dd	 0x0

include 'lang.inc'                              ; language support
include '..\..\..\macros.inc' ; useful macroses
include 'draw.inc'
include 'brand.inc'           ;Brand ID decoding
include 'caches.inc'          ;(L1 and L2 cashes decoding for Intel)
include 'multipli.inc'        ;(multiplier decoding)
include 'features.inc'        ;(features decoding)

include 'rsatest.inc'
include 'variable.inc'



START:			;   LET'S GO!!!
;------------
CYCLES:
;    CPU speed
    mov eax, 18
    mov ebx,5
    mcall
    mov [total1],eax  ;in Hz,  example 1600490000
    xor  edx,edx
    mov  ebx,1000000
    div  ebx
    mov [total], eax  ; in Mhz,  example 1600
    xor edx, edx
    mov eax, [total1]
    mov ebx, 10000
    div ebx
    mov [ost], eax    ; example 160049
    mov ecx, [total]
    imul ecx, 100
    neg  ecx
    add  ecx, eax
    mov [sot], ecx    ; example 49
;------------
cpu:		      ;is CPUID supported?
  pushfd	      ;push original EFLAGS
  pop eax	      ;get original EFLAGS
  mov ebx, eax	      ;save original EFLAGS
  xor eax, 00200000h  ;flip 21th bit in EFLAGS
  push eax	      ;save new EFLAGS value on stack
  popfd 	      ;replace current EFLAGS value
  pushfd	      ;get new EFLAGS
  pop eax	      ;store new EFLAGS in EAX
  cmp eax, ebx	      ;compare values of 21th bit
  jz NO_CPUID	      ;CPUID isn't supported
;------------
CPUNAME:	      ; VENDOR
    mov   eax,0       ; eax=0
    cpuid

    mov [stdc], eax   ; number of calls
    mov   [cpuname+ 12],ebx
    mov   [cpuname+ 16],edx
    mov   [cpuname+ 20],ecx
    mov   [smallvendor],ecx

; for various vendors we should later use different methods

;Decoding cache L1 and L2 for Intel

  cmp ecx, 'ntel'
  jne cpu1     ;is not Intel

;Starting L1, L2, L3 caches detection (Intel made it VERY HARD)

mov eax, 2
cpuid

mov [che], al	     ; number of calls
multik:

.eaxl:
test  eax, eax       ;    Test bit 31
js    .ebxl	     ;    <> 0 =>invalid values
call decodecache24
.ebxl:
test  ebx, ebx
js    .ecxl
mov eax, ebx
call decodecache32
.ecxl:
test  ecx, ecx
js    .edxl
mov eax, ecx
call decodecache32
.edxl:
test  edx, edx
js    cpu1
mov eax, edx
call decodecache32

dec [che]    ; we made all calls
je cpu1

multi: ; not yet

mov eax, 2  ; so we made call again
cpuid

jmp multik

;  FAMILY MODEL STEPPING
cpu1:
    xor eax, eax
    inc eax	  ; eax=1
    cpuid

mov ecx, eax
shr ecx,8	  ;   shift it to the correct position
and ecx,0000000Fh ;   get CPU family
mov dword[f],ecx

mov ecx, eax
shr ecx,4
and ecx,0000000Fh ;    get CPU model
mov dword[m],ecx

mov ecx, eax
and ecx,0000000Fh ;   get CPU stepping
mov dword[s],ecx

;-
mov ecx, eax	  ; get Type
shl ecx, 18
shr ecx,30
;and ecx, 0000000Fh ; only two lower bits can be nonzero
mov dword[t], ecx
;=======================================================

cmp dword[smallvendor], 'cAMD'
jz maybe_athlon
cmp dword[smallvendor], 'ntel'
jnz no_full   ; if not AMD or Intel

;detect_it:
;cmp [f], 0Fh
;jne no_full   fixed calculation of extended model for Intel

full:

mov ecx, eax	      ; get Extended model
shr ecx,16	      ;shift it to the correct position
and ecx, 0000000Fh 
shl ecx, 4
add ecx, [m]
mov dword[em],ecx     ;  effective    model

mov ecx, eax	      ; get Extended family
shr ecx, 20	      ;shift it to the correct position
and ecx, 000000FFh
add ecx, [f]
mov dword[ef],ecx     ; effective family


jmp fut

no_full:

mov ecx, [m]
mov [em], ecx

mov ecx, [f]
mov [ef], ecx

jmp fut

maybe_athlon:
mov eax, 0x80000001		  ; CPUID ext. function 0x80000001
cpuid				    
mov ecx, eax
shr ecx,8	  ;   shift it to the correct position
and ecx,0000000Fh ;   get CPU family
mov dword[ef],ecx

mov ecx, eax
shr ecx,4
and ecx,0000000Fh ;    get CPU model
mov dword[em],ecx

fut:

call decode_sse3
;-
    call decode_extended

mov  dword [myname], $612F6E

        cmp     [extc], $80000003
        jbe     .noname

    mov       eax,$80000002
    cpuid
    mov   [myname],eax
    mov   [myname+4],ebx
    mov   [myname+8],ecx
    mov   [myname+12],edx
    mov   eax,$80000003
    cpuid
    mov   [myname+16],eax
    mov   [myname+20],ebx
    mov   [myname+24],ecx
    mov   [myname+28],edx
    mov   eax,$80000004
    cpuid
    mov   [myname+32],eax
    mov   [myname+36],ebx
    mov   [myname+40],ecx
    mov   [myname+44],edx

.noname:
red:			

;mov byte [multiplier], 115;     ; for testing

call multipl			      ; get multiplier 
mov byte [multiplier], cl

mov   dword [freqbb], 0
mov   dword [freqll], 0 

mov   ebx, dword [multiplier]
test  ebx, ebx
jz output

calc:

mov eax,dword [ost]   ; example 166474
imul eax, 10	      ; example 1664740
xor edx,edx
div ebx               ; get system clock (if multiplier detected)

xor edx, edx                    ; example eax=16647
mov ebx, 100
div ebx
mov dword [freqbb], eax         ; example 166
mov dword [freqll], edx         ; example 47

xor edx, edx
    mov eax,dword[multiplier]  ; example 115
    mov  ebx,10
    div  ebx
    mov dword[multb], eax  ;   example 11
    mov dword[multa], edx    ; example 5

output:

   call draw_window    ;     Draw window

typedetect:
        mov     edx, t1
        cmp     [t], 00b
        jz      @f
        mov     edx, t2
        cmp     [t], 01b
        jz      @f
        mov     edx, t3
        cmp     [t], 11b
        jz      @f
        mov     edx, t4
@@:
        mov     ebx, 290*65536 + 250
        mov     ecx, 0x80000000
        mcall   4

PROCCORE:    ;   Who are you?
; Intel - "GenuineIntel"           +
; AMD - "AuthenticAMD"             +
; Cyrix - "CyrixInstead"           +
; UMC - "UMC UMC UMC "
; NexGen - "NexGenDriven"
; Centaur - "CentaurHauls"         +
; Rise Technology - "RiseRiseRise"
; SiS - "SiS SiS SiS "
; Transmeta - "GenuineTMx86"       +
; National Semiconductor - "Geode by NSC"
; Vortex - "Vortex86 SoC"       + initial support
  cmp dword[smallvendor], 'ntel'
  jz Intel
  cmp dword[smallvendor], 'cAMD'
  jz AMD
  cmp dword[smallvendor], 'tead'
  jz Cyrix
  cmp dword[smallvendor], 'auls'
  jz Centaur
  cmp dword[smallvendor], 'Mx86'
  jz Transmeta
  cmp dword[smallvendor], ' SoC'
  jz Vortex

; cmp ecx, 'UMC '
; jz .UMC
; cmp ecx, 'iven'
; jz .NexGen
; cmp ecx, 'Rise'
;  jz .Rise
; cmp ecx, 'SiS '
; jz .SiS
; cmp ecx, ' NSC'
; jz .NSC
; jmp Other   ;  I don't know what to do with you...
Other:
Text 75,70,0x00000000,other, otherlen-other
    jmp MMXtest
;-------------------------

AMD:

Text 15, 190,0x00000000,cache, cachelen-cache

Text 75,70,,AMDn, AMDnlen-AMDn

	mov	esi, amd
	call	load_gif
PutImage 135,107,201,49,img_area+8
;         place   size

; Relax, man. AMD made PRETTY SIMPLE cache detection routine
;CACHE1:
mov eax, 80000005h
    cpuid

movzx eax, cl
mov [lineld], eax

mov eax, ecx
;shl eax, 8
;shr eax, 24

and eax,00FF0000h
shr eax, 16
mov [wayld], eax

shr ecx, 24
mov [L1d], ecx


movzx eax, dl
mov [lineli], eax

mov eax, edx
;shl eax, 8
;shr eax, 24

and eax,00FF0000h
shr eax, 16
mov [wayli], eax


shr edx, 24
mov [L1i], edx


;CACHE2:
mov eax, 80000006h
    cpuid

movzx eax, cl
mov dword[linel2], eax

push ecx
shr ecx, 12+1
and ecx, 0x7
mov eax, 1
shl eax, cl
mov dword [wayl2], eax
pop ecx

shr ecx, 16
mov [L2],ecx

    cmp [f], $5
    jz .fiv
    cmp [f], $6
    jz .si
    cmp [f], $F
    jz fif
.fiv:	 ;     Family=5
        mov     [micron], 50
        mov     edx, A50
        cmp     [m], $0
        jz      @f
        mov     [micron], 35
        mov     edx, A51
        cmp     [m], $1
        jz      @f
        mov     edx, A52
        cmp     [m], $2
        jz      @f
        mov     edx, A53
        cmp     [m], $3
        jz      @f
        mov     [micron], 30
        mov     edx, A56
        cmp     [m], $6
        jz      @f
        mov     [micron], 25
        mov     edx, A57
        cmp     [m], $7
        jz      @f
        mov     edx, A58
        cmp     [m], $8
        jz      @f
        mov     edx, A59
        cmp     [m], $9
        jz      @f
        mov     [micron], 18
        mov     edx, A5D
@@:
        jmp     @f

.si:   ;    Family=6
        mov     [micron], 25
        mov     edx, At1
        cmp     [m], $1
        jz      @f
        mov     [micron], 18
        mov     edx, At2
        cmp     [m], $2
        jz      @f
        mov     edx, At3
        cmp     [m], $3
        jz      @f
        mov     edx, At4
        cmp     [m], $4
        jz      @f
        cmp     [m], $6
        jz      A6
        mov     [micron], 13
        mov     edx, At7
        cmp     [m], $7
        jz      @f
        cmp     [m], $8
        jz      A8
        jmp     AA
@@:
        Text    100,70,0x80000000
        jmp     MMXtest
A6:
;        mov     [FRS], 266  ;!!!!!!

;        Number  315,90,0,3,dword [FRS],0x000000; MHz 

        call    newrating; !!!!

        Text    245,70,0x00000000,pr, prlen-pr

        Number  310,70,0,4,dword [rating],0x000000
        mov     edx, At6
        jmp     @b

A8:

;        mov     [FRS], 266	;!!!!!!

;        Number  315,90,0,3,dword [FRS],0x000000; MHz

        cmp     [L2], 256
        jl      .App  ; Applebred

        call    newrating;!!!!

        Text    245,70,0x00000000,pr, prlen-pr
        Number  310,70,0,4,dword [rating],0x000000
        mov     edx, At8
        jmp     @b

.App:
        mov     edx, At8a
        jmp     @b

AA:

;        mov     [FRS], 333; !!!!
        Text    245,70,0x00000000,pr, prlen-pr

;        Number  315,90,0,3,dword [FRS],0x000000; MHz 

        mov     edx, Atat
        cmp     [L2], 256
        jl      .Tho ; Thorton
        mov     edx, Ata
.Tho:
        push    edx

        call    newrating;!!!!!

        Number  310,70,0,4,dword [rating],0x000000
        pop     edx
        jmp     @b

fif:  ;  AMD-64    Family=15

;here is a need to rewrite detection of AMD F-th family according to "Revision Guide for
;AMD AthlonTM 64 and  AMD OpteronTM  Processors" 25759.pdf

; checking sse3 for new AMD's is needed
    cmp [m],$1	; Dual-core Opteron      
    jz .AF1
    cmp [m],$3	; Toledo 1024 0.09   // Manchester     ||Windsor Dual Core not supported
    jz .AF3
    cmp [m],$4	;Athlon 64 Mobile Athlon 64 FX  ClawHammer (1024) 0.13
    jz .AF4
    cmp [m],$5	; Opteron     Athlon 64 FX 0.13 (1024)
    jz .AF5
    cmp [m],$7	;Athlon 64 Athlon 64 FX  Clawhammer(1024) 0.13   Sledgehammer(1024)  0.13  // SSE3+ SanDiego(1024)
    jz .AF7
   cmp [m],$8 ; Athlon 64 Mobile Athlon 64 FX ClawHammer (1024) 0.13
    jz .AF8
   cmp [m],$B ; Athlon 64
    jz .AFB
   cmp [m],$C ;Athlon 64 Newcastle(512) 0.13  Sempron> Paris (256)   0.13  |SSE3+ Sempron >  Palermo FC0 0.09  // (Venice)
    jz .AFC
   cmp [m],$E  ; Athlon 64    //
    jz .AFE
   cmp [m],$F ; Athlon 64 Winchester(512) |SSE3+ SanDiego(1024)  Venice (512)  Palermo (256) 0.09
    jz .AFF
    jmp next_generation
.AF1:
    mov [micron], 09  ;?
    Text 100,70,0x00000000,AF1, AF1len-AF1
    jmp MMXtest
.AF3:
    mov [micron], 09
    Text 100,70,0x00000000,AF3, AF3len-AF3
    jmp MMXtest
.AF4:
    mov [micron], 13
    Text 100,70,0x00000000,AF4, AF4len-AF4
    jmp MMXtest
.AF5:
    mov [micron], 13
    Text 100,70,0x00000000,AF5, AF5len-AF5
    jmp MMXtest
.AF7:
    mov [micron], 13
    Text 100,70,0x00000000,AF5, AF5len-AF5
    jmp MMXtest
.AF8:
   mov [micron], 13
   Text 100,70,0x00000000,AF4, AF4len-AF4  ; AF4, AF5len-AF4
   jmp MMXtest
.AFB:
    mov [micron], 13
Text 100,70,0x00000000,AF4, AF4len-AF4
 jmp MMXtest

.AFC:
cmp [L2], 512
je .AFCn

cmp [sse3sup], 1
je .AFCnpal

.AFCnpar:  ; paris
    mov [micron], 13
    Text 100,70,0x00000000,AFCs, AFCslen-AFCs
    jmp MMXtest

.AFCnpal: ; palermo
    mov [micron], 9
    Text 100,70,0x00000000,AFCsp, AFCsplen-AFCsp
    jmp MMXtest

.AFCn: ;newcastle
    mov [micron], 13
    Text 100,70,0x00000000,AFC, AFClen-AFC
    jmp MMXtest

.AFE:	; error in cpu
 jmp .AFC

.AFF:

cmp [sse3sup], 1
je .AFFsse

.win:
mov [micron], 9    ; winchester
jmp MMXtest

.AFFsse:
mov [micron], 9
cmp [L2], 1024
jz .AFs
cmp [L2], 512
jz .AFd
cmp [L2], 256
jz .AFp

.AFs:
Text 100,70,0x00000000,AFS, AFSlen-AFS
 jmp MMXtest

.AFd:
Text 100,70,0x00000000,AFV, AFVlen-AFV
 jmp MMXtest

.AFp:
Text 100,70,0x00000000,AFCsp, AFCsplen-AFCsp
 jmp MMXtest
;-----------------------------------------------
Intel:
Text 75,70,0x00000000,Inteln, Intelnlen-Inteln

	mov	esi, intel
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 125,107,201,49,img_area+8
;         place   size

det:
    cmp [f], $5
    jz .five
    cmp [f], $6
    jz .six
    cmp [f], $7
    jz .sev
    cmp [f], $F
    jz .fift
.five:	      ;Family=5

Text 15, 190,0x00000000,cache, cachelen-cache

    cmp [m],$0
    jz .I0
    cmp [m],$1
    jz .I1
    cmp [m],$2
    jz .I2
    cmp [m],$3
    jz .I3
    cmp [m],$4
    jz .I4
    cmp [m],$7
    jz .I7
    cmp [m],$8
    jz .I8
.I0:
Text 110,70,0x00000000,P50, P50len-P50
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 80
 jmp MMXtest
.I1:
Text 110,70,0x00000000,P5, P5len-P5
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 50
 jmp MMXtest
.I2:
Text 110,70,0x00000000,P54C, P54Clen-P54C
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 50
 jmp MMXtest
.I3:
Text 110,70,0x00000000,P54T, P54Tlen-P54T
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 50
 jmp MMXtest
.I4:
Text 110,70,0x00000000,P55C, P55Clen-P55C
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 35
 jmp MMXtest
.I7:
Text 110,70,0x00000000,P54C, P54Clen-P54C
   mov [L1d], 8
   mov [L1i], 8
   mov [L2], 256
   mov [micron], 35
 jmp MMXtest
.I8:
Text 110,70,0x00000000,P55C, P55Clen-P55C
   mov [L1d], 16
   mov [L1i], 16
   mov [L2], 256
   mov [micron], 35
 jmp MMXtest
.six:		   ;Family=6

Text 15, 190,0x00000000,cache, cachelen-cache

    cmp [m],$0
    jz .I60
    cmp [m],$1
    jz .I61
    cmp [m],$3
    jz .I63
    cmp [m],$5
    jz .I65
    cmp [m],$6
    jz .I66
    cmp [m],$7
    jz .I67
    cmp [m],$8
    jz .I68
    cmp [m],$9
    jz .I69
    cmp [m],$A
    jz .I6A
    cmp [m],$B
    jz .I6B
   cmp [m],$D
    jz .I6D
    cmp [m],$E
    jz .I6E
   cmp [m],$F
    jz .I6F
.I60:
    mov [micron], 50
Text 110,70,0x00000000,P60, P60len-P60
 jmp MMXtest
.I61:
    mov [micron], 35
Text 110,70,0x00000000,P61, P61len-P61
 jmp MMXtest
.I63:
    mov [micron], 28
Text 110,70,0x00000000,P63, P63len-P63
 jmp MMXtest
.I65:
    mov [micron], 25
    cmp [L2], 0
    jne .pp65  ; Pentium
Text 110,70,0x00000000,P65c, P65clen-P65c
    jmp MMXtest
.pp65:
Text 110,70,0x00000000,P65, P65len-P65
    jmp MMXtest
.I66:
    mov [micron], 25
Text 110,70,0x00000000,P66, P66len-P66
    jmp MMXtest
.I67:
    mov [micron], 25
Text 110,70,0x00000000,P67, P67len-P67 ;but if SSE4.1 supported then it is Intel Core (Penryn) 
    jmp MMXtest
.I68:
    mov [micron], 18
    cmp [L2], 128
    jne .pp68  ; Pentium
Text 110,70,0x00000000,P68c, P68clen-P68c
    jmp MMXtest
 .pp68:
Text 110,70,0x00000000,P68, P68len-P68
    jmp MMXtest
.I69:
    mov [micron], 13
Text 110,70,0x00000000,P69 , P69len-P69
    jmp MMXtest
.I6A:
    mov [micron], 18
Text 110,70,0x00000000,P6A, P6Alen-P6A ;but if SSE4.2 supported then it is Intel Core (Nehalem) 
    jmp MMXtest
.I6B:
    mov [micron], 13
    cmp [L2], 256
    jne .pp6B  ; Pentium
Text 110,70,0x00000000,P6Bc, P6Bclen-P6Bc
    jmp MMXtest
.pp6B:
Text 110,70,0x00000000,P6B, P6Blen-P6B
    jmp MMXtest
.I6D:
    mov [micron], 9
Text 110,70,0x00000000,P6D, P6Dlen-P6D
    jmp MMXtest
.I6E:
    mov [micron], 6
Text 110,70,0x00000000,P6E, P6Elen-P6E
    jmp MMXtest
.I6F:
    mov [micron], 6
Text 110,70,0x00000000,P6F, P6Flen-P6F
    jmp MMXtest

;06Ex - Pentium M Yonah 0.065
;06Fx - Pentium D Conroe 0.065, Xeon Woodcrest, Celeron D AllenDale, Core 2 Kentsfield

.sev:	 ;Family=7
.IS0:

Text 15, 190,0x00000000,cache, cachelen-cache ;?

    mov [micron], 18
Text 110,70,0x00000000,PS0, PS0len-PS0
 jmp MMXtest

.fift:	  ;Family=15

Text 15, 190,0x00000000,cacheP4, cacheP4len-cacheP4

    cmp [m],$0
    jz .IF0
    cmp [m],$1
    jz .IF1
    cmp [m],$2
    jz .IF2
    cmp [m],$3
    jz .IF3
    cmp [m],$4
    jz .IF3 ;identical to F3xh
    cmp [m],$5
    jz .IF5
    cmp [m],$6
    jz .IF6
    jmp next_generation
.IF0:
    mov [micron], 18
    cmp [L2], 128
    jne .ppF0  ; Pentium
Text 110,70,0x00000000,PF0c, PF0clen-PF0c
    jmp MMXtest
.ppF0:
Text 110,70,0x00000000,PF0, PF0len-PF0
    jmp MMXtest
.IF1:
    mov [micron], 18
    cmp [L2], 128
    je .IF0;jne.ppF1  ; Pentium
  ;  mov   eax,dword 0x00000004
  ;  mov   ebx,115*65536+80
  ;  mov   ecx,dword 0x00000000
  ;  mov   edx,PF0c
  ;  mov   esi,PF0clen-PF0c
  ;  mcall
  ;jmp MMXtest
;.ppF1:
Text 110,70,0x00000000,PF0, PF0len-PF0
 jmp MMXtest
.IF2:
    mov [micron], 13
    cmp [L2], 128
    jne .ppF2  ; Pentium
Text 110,70,0x00000000,PF2c, PF2clen-PF2c
 jmp MMXtest
.ppF2:
Text 110,70,0x00000000,PF2, PF2len-PF2
 jmp MMXtest
.IF3:
    mov [micron], 09
    cmp [L2], 256
    jne .ppF3  ; Pentium
Text 110,70,0x00000000,PF3c, PF3clen-PF3c
 jmp MMXtest
.ppF3:
Text 110,70,0x00000000,PF3, PF3len-PF3
 jmp MMXtest

.IF5:
    mov [micron], 09
    cmp [L2], 512
    jae .ppF5  ; Pentium
Text 110,70,0x00000000,PF5c, PF5clen-PF5c
 jmp MMXtest
.ppF5:
Text 110,70,0x00000000,PF5, PF5len-PF5
 jmp MMXtest

 .IF6:
    mov [micron], 06  ; 065
    cmp [L2], 512
    ja .ppF6  ; Pentium
Text 110,70,0x00000000,PF6c, PF6clen-PF6c
 jmp MMXtest
.ppF6:
Text 110,70,0x00000000,PF6, PF6len-PF6
 jmp MMXtest


 next_generation:
Text 110,70,0x00000000,NG, NGlen-NG
  jmp MMXtest
;----------------------------------
Cyrix:

Text 15, 190,0x00000000,cache, cachelen-cache

	mov	esi, cyrix
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 130,127,201,49,img_area+8
;         place   size

    cmp [f], $5
    jz .fivv
    cmp [f], $6
    jz .sixx
.fivv:	  ;Family=5
    cmp [m],$2
    jz .C52
    cmp [m],$4
    jz .C54
.C52:
    mov [micron], 50 ;35?
    mov [L1i], 8
    mov [L1d], 8
    mov [L2], 512
Text 75,70,0x00000000,Cyrixn, Cyrixnlen-Cyrixn
Text 110,70,0x00000000,C52, C52len-C52
    jmp MMXtest
.C54:
    mov [micron], 50
    mov [L1i], 8
    mov [L1d], 8
    mov [L2], 512
Text 75,70,0x00000000,Cyrixn, Cyrixnlen-Cyrixn
Text 110,70,0x00000000,C54, C54len-C54
    jmp MMXtest

.sixx:	   ;Family=6
   cmp [m],$0
   jz .C60
   cmp [m],$5
   jz .C65
.C60:
    mov [micron], 25
    mov [L1i], 32
    mov [L1d], 32
    mov [L2], 512
Text 75,70,0x00000000,Cyrixn, Cyrixnlen-Cyrixn
Text 110,70,0x00000000,C60, C60len-C60
    jmp MMXtest
.C65:
    mov [micron], 25 ;35?
    mov [L1i], 32
    mov [L1d], 32
    mov [L2], 512
Text 75,70,0x00000000,Centaurn, Centaurnlen-Centaurn
    mov edx,C65
    mov esi,C65len-C65
    jmp OutProcName
;---------------------
Centaur:

Text 15, 190,0x00000000,cache, cachelen-cache

;CACHE1:
mov eax, 80000005h
    cpuid
shr ecx, 24
mov [L1d], ecx
shr edx, 24
mov [L1i], edx



; cache detection routine
;CACHE1:
mov eax, 80000005h
    cpuid

movzx eax, cl
mov [lineld], eax

mov eax, ecx
shr eax, 16
and eax,000000FFh
mov [wayld], eax

shr ecx, 24
mov [L1d], ecx

movzx eax, dl
mov [lineli], eax

mov eax, edx

shr eax, 16
and eax,000000FFh
mov [wayli], eax

shr edx, 24
mov [L1i], edx


;CACHE2:
mov eax, 80000006h
    cpuid

cmp [f], $6
    jz vn
    jmp vl2old	; if not then old identification
vn:
    cmp [m],$9
    jl vl2old
; else  new
movzx eax, cl
mov dword[linel2], eax

mov eax, ecx
shl eax, 16
shr eax, 28

mov dword[wayl2], eax

shr ecx, 16  ; it is 16 bits now
mov [L2],ecx



vl2old:
movzx eax, cl
mov dword[linel2], eax

mov eax, ecx
shl eax, 8
shr eax, 24

mov dword[wayl2], eax

shr ecx, 24  ; it was 8 bits earlier
mov [L2],ecx


    cmp [f], $5
    jz fivC
    cmp [f], $6
    jz sixC

fivC:		   ;Family=5

	mov	esi, idt
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 125,107,201,49,img_area+8
;         place   size

Text 75,70,0x00000000,IDTn, IDTnlen-IDTn
    cmp [m],$4
    jz .V54
    cmp [m],$8
    jz .V58
    cmp [m],$9
    jz .V59
.V54:
   mov [micron], 35
   mov edx,V54
   mov esi,V54len-V54
    jmp OutProcName
.V58:
    mov [micron], 25
    mov edx,V58
    mov esi,V58len-V58
    jmp OutProcName
.V59:
    mov [micron], 25
    mov edx,V59
    mov esi,V59len-V59
    jmp OutProcName

sixC:	;Family=6

	mov	esi, via
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 125,107,201,49,img_area+8
;         place   size

Text 75,70,0x00000000,Centaurn, Centaurnlen-Centaurn
    cmp [m],$6
    jz .V66
    cmp [m],$7
    jz .V67
    cmp [m],$8
    jz .V68
    cmp [m],$9
    jz .V69
    cmp [m],$A
    jz .V6A
.V66:
   mov [micron], 18 ; 25?
   mov edx,V66
   mov esi,V66len-V66
    jmp OutProcName
.V67:
    mov [micron], 15
    mov edx,V67
    mov esi,V67len-V67
    jmp OutProcName
.V68:
    mov [micron], 13
    mov edx,V68
    mov esi,V68len-V68
    jmp OutProcName
.V69:
   mov [micron], 13
   mov edx,V69
   mov esi,V69len-V69
   jmp OutProcName
.V6A:
   mov [micron], 9
   mov edx,VA
   mov esi,VAlen-VA
   jmp OutProcName
;-----------
Transmeta:

Text 15, 190,0x00000000,cache, cachelen-cache

Text 75,70,,Tranmsmetan, Tranmsmetanlen-Tranmsmetan

	mov	esi, transmeta
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 125,107,201,49,img_area+8
;         place   size

; cache detection routine - it is the same as for AMD (almost)
;CACHE1:
mov eax, 80000005h
    cpuid

movzx eax, cl
mov [lineld], eax

mov eax, ecx

shr eax,16
and eax,000000FFh
mov [wayld], eax

shr ecx, 24
mov [L1d], ecx

movzx eax, dl
mov [lineli], eax

mov eax, edx

shr eax, 16
and eax,000000FFh
mov [wayli], eax

shr edx, 24
mov [L1i], edx


;CACHE2:
mov eax, 80000006h
    cpuid

movzx eax, cl
mov dword[linel2], eax

mov eax, ecx
shl eax, 16
shr eax, 28

mov dword[wayl2], eax

shr ecx, 16
mov [L2],ecx


    cmp [f], $5
    jz .fivt
    cmp [f], $F
    jz .fift
.fivt:	  ;     Family=5

    mov edx,T5
    mov esi,T5len-T5
    jmp @f

.fift:	  ;     Family=F
    mov edx,TF
    mov esi,TFlen-TF
@@:
    mov [micron], 13 ;
    Text 140,70,0
    jmp MMXtest

OutProcName:
        Text    100,70,0

;----
;---------------------------------- new 
Vortex:

Text 15, 190,0x00000000,cache, cachelen-cache

	mov	esi, vortex
	call	load_gif
PutImage 135,107,201,49,img_area+8
;PutImage 130,127,201,49,img_area+8
;         place   size

    cmp [f], $5
    jz .V54 ;fivvtx
;.fivvtx:	  ;Family=5
;    cmp [m],$4
;    jz .V54
.V54:
    mov [micron], 13
    mov [L1i], 16
    mov [L1d], 16
    mov [L2], 256
    mov [wayl2], 4
    mov [wayli], 4
    mov [wayld], 4
Text 75,70,0x00000000,Vortexn, Vortexnlen-Vortexn
;Text 110,70,0x00000000,V54, V54len-V54
    jmp MMXtest

;---------------------



MMXtest:	     ; MMX test and Brand ID decoding

call decodebrand  ; get Brand ID

call decode_standard_features

call decode_extended_features
      xor eax,eax
      inc eax
      cpuid
HTTtest:
  test	edx, $10000000; ;Test bit 28
  jz .ELN

.EL:   ;HTT technology is supported
   and ebx,00FF0000h ; numbers of logical processors
   cmp ebx, 1 shl 16
   ;   mov [number_of_log_cpus], ebx
   je .ELN  ; HHT not enabled (Celeron)

   mov	dword [HTTn+9], $736579
   mov	dword [HTT+ 6], $736579
   jmp TEXTOUT
.ELN:

   mov	dword [HTTn+ 9],  $6F6E
   mov	dword [HTT+ 6],  $6F6E

TEXTOUT:

Text 15,110,0x00000000,fam, famlen-fam
Text 15,130,,mode, modelen-mode
Text 15,150,,step, steplen-step
;--------L1  L2
Number 75,170,0,3,dword [L1d],0x000000;
Number 75,190,,,dword [L1i]
Number 41,210,0,4,dword[L2]
Number 35,230,0,5,dword[L3]
;-----------Features
Number 258,50,0,2,dword [micron]  ; micron

Text 275,290,0x00000000,HTT, HTTlen-HTT
Text 275,310,,sse3, sse3len-sse3

Text 15,70,,name, namelen-name

Text 15,290,,MMXs, MMXslen-MMXs
Text 15,310,,SSE, SSElen-SSE
Text 95,310,,SSE2, SSE2len-SSE2

    Number 140,170,0,2,dword [wayld],0x000000
    Number 218,170,,,dword [lineld]

    Number 140,190,,,dword [wayli]
    Number 218,190,,,dword [lineli]

    Number 140,210,,,dword [wayl2]
    Number 218,210,,,dword [linel2]

    Number 140,230,,,dword [wayl3]
    Number 218,230,,,dword [linel3]

;-------------------
TEST3DNOW:

  xor edx, edx
  cmp [smallvendor], 'ntel'
  je @f

  mov	eax, $80000001 ;// Setup extended function 8000_0001h
  cpuid

@@:
  mov   eax, now+9
  add   edx, edx
  call  write_yes_no_cf

  mov   eax, nowp+9
  add   edx, edx
  call  write_yes_no_cf

TESTMMXP:

    mov   eax,$80000000
    cpuid

    test eax, eax
    jns NOEXTENDEDM

  ;cmp   eax, $80000000 ;// Is 800_0001h supported?
  ;jz   .NOEXTENDEDM    ;// If not, 3DNow! technology is not supported
  mov	eax, $80000001 ;// Setup extended function 8000_0001h
  cpuid
  cmp [smallvendor], 'tead'
  jne noCyr
Cyrmx:
  test	edx, $01000000 ;// Test bit 24
  jnz	XITM  ;// 3DNow! technology is supported
  jz NOEXTENDEDM
noCyr:
  test	edx, $00400000 ;// Test bit 22
  jnz	XITM  ;// 3DNow! technology is supported
  ;jz   .NOEXTENDEDM

NOEXTENDEDM:
   mov	dword [mmxp+ 7], $6F6E
   mov	dword [MMXPi+ 8], $6F6E
   jmp text3d
XITM:
   mov	dword [mmxp+ 7],  $736579
   mov	dword [MMXPi+ 8],  $736579

text3d:

Text 175,290,0x00000000,now, nowlen-now
Text 175,310,,nowp, nowplen-nowp
Text 95,290,,mmxp, mmxplen-mmxp

jmp still

;--------------------------
NO_CPUID:
 Text 15,50,0x00000000,oblom, oblomlen-oblom

FREEZE:
nop
jmp FREEZE ; maybe we should close application or just made some Warning and jump to still:
;----------------
still:

; waiting for events
event_wait:

    ;================_RAM_==============
  Number 200,340,0,4,dword [ram_size_a],0xFFFFFF

  mov eax, 18
  mov ebx, 16
  mcall

  shr eax, 10

  mov [ram_size_a], eax

  mov eax, 18
  mov ebx, 17
  mcall

  shr eax, 10

  mov [ram_size_t], eax

  Text 115,340,0x00000000,ram, ramlen-ram
  Text 300,340,,mb, mblen-mb

  Number 200,340,0,4,dword [ram_size_a],0x000000

  Number 270,340,,,dword [ram_size_t]

;==============================

	mov	eax,23	     ; function 23 - event wait
	mov	ebx,50	     ; wait for 0.5 second
	mcall

    cmp  eax,1		; 
    je	 red		;  redraw
    cmp  eax,2		; 
    je	 key		;  key
    cmp  eax,3		; 
    je	 button 	;  button
    jmp  still		; 
  key:			; 
    mcall		; 
    jmp  still		; 
  button:		; 
    mov  eax,17 	; 
    mcall		; 
    cmp  ah,1		;  = 1 ?
    je	close		; close

    cmp  ah,2		;  = 2 ?
    je	thread_start	; 
			; 
;    cmp  ah,3		;  = 3 ?
    jne  still


;vybor:

; Number 310,70,0,4,dword [rating],0xFFFFFF ;

; Number 315,90,0,3,dword [FRS]; MHz 

;cmp [FRS], 266
;jz .s1
;cmp [FRS], 333
;jz .s2
;cmp [FRS], 400
;jz .s3

;.s1:
;mov [FRS], 333
;.rating:;
;call newrating
; Number 310,70,0,4,dword [rating],0x000000

; Number 315,90,0,3,dword [FRS]; MHz 
;jmp  still
;
;.s2:
;mov [FRS], 400
;jmp .rating
;
;.s3:
;mov [FRS], 266
;jmp .rating

close:
    mov   eax,-1	
    mcall	       

;**************************** THREAD-SECOND WINDOW
thread_start:

    cmp  [num_win2],0	

    jne  still		

;================================================RSA test

;test rsa coding speed
        call    init_test
;length of module - 256 bit
  mov  eax,26
  mov  ebx,9
  mcall
  add  eax,100 ;test lasts 1 second.
  push eax
.loop:
  push 4     ;do 4 iterations - this reduces number of calls mcall.
.loop1:
  call rsa_test   ;this procedure change all registers
  dec  dword [esp]
  jnz  .loop1
  pop  ecx
  mov  eax,26
  mov  ebx,9
  mcall
  cmp  eax,dword [esp]	 ;Is time changed?
  jl   .loop
  pop  eax
  shr  dword [iter],4 ;[iter] - speed in Kb/sec. (every iteration codes 64 bytes)
CreateTread window_2,thread2_esp
  jmp  still

window_2:
    mov  [num_win2],1	
    call draw_window_2	

still_2:		

    mov  eax,10 	
    mcall		

    cmp  eax,1		
    je	 window_2	;  window_2
    cmp  eax,2		; 
    je	 key_2		;  key_2
    cmp  eax,3		; 
    je	 button_2	;  button_2

    jmp  still_2	; 

  key_2:		; 
    mcall		; 
    jmp  still_2	; 

  button_2:		; 
    mov  eax,17 	; 17 
    mcall		; 

    cmp  ah,1		; = 1 ?
    jne  still_2	; noclose

    mov  [num_win2],0	; 

    or	 eax,-1 	; 
    mcall

draw_window_2:
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1h 		    ; 1, start of draw
    mcall


  Window 250,250,420,390, 0x33FFFFFF, 0x805080d0, standard
	  ; place size

Text 15, 10,0x00000000, STDCA, STDCAlen-STDCA
Text 215, 10,, EXTCA, EXTCAlen-EXTCA

Number 135,10,1*256,8,dword [stdc],0x000000
Number 335,10,,,dword [extc],

Text 15, 30,0x00000000, FPU, FPUlen-FPU
Text 115, 30,, VME, VMElen-VME
Text 215, 30,, DE,  DElen-DE
Text 315, 30,, PSE, PSElen-PSE

Text 15, 50,,TSC, TSClen-TSC
Text 115,50,,MSR, MSRlen-MSR
Text 215,50,,PAE, PAElen-PAE
Text 315,50,,MCE, MCElen-MCE

Text 15,70,,CX8, CX8len-CX8
Text 115,70,,APIC, APIClen-APIC
Text 215,70,,Res, Reslen-Res
Text 315,70,,SEP, SEPlen-SEP

Text 15,90,,MTRR, MTRRlen-MTRR
Text 115,90,,PGE, PGElen-PGE
Text 215,90,,MCA, MCAlen-MCA
Text 315,90,,CMOV, CMOVlen-CMOV

Text 15,110,,PAT, PATlen-PAT
Text 115,110,,PSE36, PSE36len-PSE36
Text 215,110,,PSNUM, PSNUMlen-PSNUM
Text 315,110,,CLFLUSHn, CLFLUSHnlen-CLFLUSHn

Text 15,130,,Res, Reslen-Res
Text 115,130,,DTS, DTSlen-DTS
Text 215,130,,ACPI, ACPIlen-ACPI
Text 315,130,,MMX, MMXlen-MMX

Text 15,150,,FXSR, FXSRlen-FXSR
Text 115,150,,SSE, SSElen-SSE
Text  215,150,,SSE2, SSE2len-SSE2
Text 315,150,,SSn, SSnlen-SSn

Text 15,170,,HTT, HTTnlen-HTTn
Text 115,170,,TM, TMlen-TM
Text 215,170,,IA64, IA64len-IA64
Text 315,170,,PBE, PBElen-PBE
;---------------
DrawLine 0,  410, 185,185,0x8080FF  ;10

mov   eax,$80000000
cpuid
;mov eax, $03020101   for test of reaction
test eax, eax
js  goooddd

baaadd:
Text 95,235,0x00000000,NEF, NEFlen-NEF
jmp too

goooddd:
Text 15,195,0x00000000,SS3, SS3len-SS3
Text 15,215,,MON, MONlen-MON
Text 15,235,,DS_CPL, DS_CPLlen-DS_CPL
Text 15,255,,EST, ESTlen-EST
Text 15,275,,TM2, TM2len-TM2
Text 15,295,,VMX, VMXlen-VMX
Text 15,315,,SVM, SVMlen-SVM

Text 115,195,,CNXT_ID, CNXT_IDlen-CNXT_ID
Text 115,215,,CX16, CX16len-CX16
Text 115,235,,ETPRD, ETPRDlen-ETPRD
Text 115,255,,SYS, SYSlen-SYS
Text 115,275,,LAF, LAFlen-LAF
Text 115,295,,SSSE3, SSSE3len-SSSE3
Text 115,315,,MCR8, MCR8len-MCR8

Text 215,195,,MP, MPlen-MP
Text 215,215,,NX, NXlen-NX
Text 215,235,,MMXPi, MMXPilen-MMXPi
Text 215,255,,MMXn, MMXnlen-MMXn
Text 215,275,,FXSRn, FXSRnlen-FXSRn
Text 215,295,,DCA,DCAlen-DCA

Text 315,195,,FFXSR, FFXSRlen-FFXSR
Text 315,215,,TSCP, TSCPlen-TSCP
Text 315,235,,LM, LMlen-LM
Text 315,255,,DNo, DNolen-DNo
Text 315,275,,DN, DNlen-DN
Text 315,295,,CMPL, CMPLlen-CMPL


too:
DrawLine 0,  410, 335,335,0x8080FF  ;10

Text 15,350,0x00000000,speed, speedlen-speed
Text 130,350,,kbpersec, kbperseclen-kbpersec

Number 95,350,0,5,dword [iter],0x000000; RSA test results

    mov  eax,12
    mov  ebx,2h
    mcall

    ret

num_win2 db 0

;   *******  main window *******

draw_window:
   mov eax,12
   mov	ebx,1h
   mcall

  Window 150,150,350,385, 0x34FFFFFF, 0x805080d0, title
	  ; place size

  Button 15,330,92,23,2+0x40000000,0x03FFFFFF   ;  button "press for more"

	mov	esi, knopka
	mov	edi, img_area2
	call	load_gif2
PutImage 15,330,93,24,img_area2+8   ; image "press for more"
;         place   size


    mov  eax,12
    mov  ebx,2h
    mcall

    Text 130,270,0x00000000,instruct, instructlen-instruct

    DrawLine  10,  330, 325,325,0x8080FF
    DrawLine 330,  330, 275,325;,0x8080FF
    DrawLine  10,   10, 275,325;,0x8080FF
    DrawLine  10,  125, 275,275;,0x8080FF
    DrawLine 230,  330, 275,275;,0x8080FF

  cmp dword[smallvendor], 'cAMD'
  jne cont
  cmp [f], $6
  jne cont

;   Button 240,85,69,15,3,0x030000FF  ;  button for rating

;    Text 245,90,0x00FFFFFF,FR, FRlen-FR ; text at button

   call newrating; !!!!

     cont:


    Text 15,50,0x00000000,tsum, tsumlen-tsum   ; 
    Text 15,90,,cpuname, cpunamelen-cpuname; 
    Text 255,250,,typen, typenlen-typen;
    Text 175, 50,,tech, techlen-tech; 

    Number 82,50,0,4,dword [total],0x000000; MHz
    Number 110,50,0,2,dword [sot]; KHz

    Text 15,170,0x00000000,cache2, cache2len-cache2
    Text 15,210,,cache3, cache3len-cache3
    Text 15,230,,cache4, cache4len-cache4


    Number 75,110,1*256,1,dword [f],0x000000 ; 
    Number 75,130,,,dword [m]
    Number 75,150,,,dword [s]

    Number 110,110,1*256,2,dword [ef]
    Number 110,130,,,dword [em]

Text   15,30,0x00000000,multil, multillen-multil
Text   175,30,,freql, freqllen-freql
Number 85,30,0,2,dword [multb],0x000000;
Number 105,30,0,1,dword [multa]
Number 259,30,0,4,dword [freqbb]
Number 289,30,0,2,dword [freqll]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Text 115,280,0x00000000,logcpus, logcpuslen-logcpus
;Number 250,280,0,2,dword [number_of_log_cpus],0x000000


    Text 15,10,0x00000000,stm, stmlen-stm
;  Fix for deleting leading whitespaces
;  in Intel P4's internal name
;  by Madis Calme
;  23.12.2004 ver. 0.81
    cld
    mov  edi,myname
    mov  al,20h
    or	 ecx,-1
    repe scasb
    dec  edi
    mov  esi,mynamelen
    sub  esi,edi
    Text 105, 10, 0x00000000, edi, esi
;    Text 105,20,0x00000000,myname, mynamelen-myname
;-
Text 15,250,,brandid, brandidlen-brandid

    ret 		; 

load_gif:
        mov     edi, img_area
load_gif2:

COLOR_ORDER equ MENUETOS
include 'gif_lite.inc'        ; parse GIF files

; DATA AREA

title	 db   'CPUID 2.28 by S.Kuzmin & the KolibriOS team',0


tsum:

if lang eq it

                db 'Frequenza:     .   MHz'

else

                db 'Frequency:     .   MHz'

end if

 

 

tsumlen:

 

total dd 0x0

total1 dd 0x0

rating dd 0x0

rat dd 0x0  ;

 

ram:

if lang eq it

                db 'RAM libera:        su'

else

                db 'Available RAM:     out of'

end if

 

 

ramlen:

 

NEF:

db 'EXTENDED FEATURES ARE NOT AVAILABLE'

NEFlen:

 

mb :

db 'MB'

mblen:

 

;logcpus :

;db 'Number of logical CPU:'

;logcpuslen:

 

speed :

if lang eq it

                db 'Performance'

else

                db 'PERFORMANCE:'

end if

 

 

speedlen:

 

kbpersec:

db 'KB/SEC'

kbperseclen:

 

instruct:

if lang eq it

                db 'Set istruzioni'

else

                db 'Instruction sets'

end if

 

 

instructlen:

 

standard    db 'Standard and Extended features plus Performance test',0

 

;FR:

;    db 'Choose FSB:'

;FRlen:

 

STDCA:

    db 'Highest STD call is         '

STDCAlen:

 

EXTCA:

    db 'Highest EXT call is         h'

EXTCAlen:

 

brandid:

    db 'Brand:'

brandidlen:

 

oblom:

if lang eq it

                db 'CPUID non e disponibile'

else

                db 'SORRY, CPUID IS NOT AVAILABLE'

end if

 

oblomlen:

other:

if lang eq it

                db 'Questo vendor non e supportato'

else

                db 'SORRY, THIS VENDOR IS NOT SUPPORTED YET'

end if

 

otherlen:

 

cpuname:

if lang eq it

                db 'Vendor CPU              '

else

                db 'CPU VENDOR:             '

end if

 

cpunamelen:

fam:

if lang eq it

                db 'Famiglia:   std    ext'

else

                db 'FAMILY:     std    ext'

end if

 

 

famlen:

mode:

if lang eq it

                db 'Modello:    std    ext'

else

                db 'MODEL:      std    ext'

end if

 

modelen:

step:

if lang eq it

                db 'Stepping:'

else

                db 'STEPPING:'

end if

 

steplen:

 

cache2:

 

    db 'L1(data):     KB       -way set     -byte line size'

cache2len:

 

cache:

    db 'L1(inst):     KB       -way set     -byte line size'

cachelen:

 

cache3:

 

    db 'L2:      KB            -way set     -byte line size'

cache3len:

 

cache4:

    db 'L3:      KB            -way set     -byte line size'

cache4len:

 

cacheP4:

 

    db 'L1(inst):     Kuops    -way set     -byte line size'

cacheP4len:

 

tech:

if lang eq it

                db 'Tecnologia: 0.   micron '

else

                db 'Technology: 0.   micron '

end if

 

 

techlen:

 

typen:

if lang eq it

                db 'Tipo:'

else

                db 'Type:'

end if

 

typenlen:

 

pr:

  db 'P-rating:'

prlen:

 

multil:

  db 'Multiplier:   .'

multillen:

 

freql:

  db 'System clock:     .   MHz'

freqllen:

 

name:

if lang eq it

                db 'Codename:'

else

                db 'CODENAME:'

end if

 

namelen:

 

AMDn:

    db 'AMD'

AMDnlen:

Inteln:

    db 'Intel'

Intelnlen:

Cyrixn:

    db 'Cyrix'

Cyrixnlen:

IDTn:

     db 'IDT/Centaur'

IDTnlen:

Centaurn:

     db 'VIA'

Centaurnlen:

 

Tranmsmetan:

     db 'Transmeta'

Tranmsmetanlen:


Vortexn:
    db 'Vortex86'
Vortexnlen:


MMXs:

    db 'MMX:         '

MMXslen:

 

mmxp:

    db 'MMX+:         '

mmxplen:

 

HTT:

    db 'HTT:          '

HTTlen:

 

HTTn:

    db 'HTT:         '

HTTnlen:

 

sse3:

    db 'SSE3:         '

sse3len:

now:

    db '3DNOW!:         '

nowlen:

nowp:

    db '3DNOW!+:         '

nowplen:

 

;-Type

 

t1      db      'OEM',0

t2      db      'Overdrive',0

t3      db      'Dual',0

t4      db      'Unknown',0

 

;----------Intel

P50:

db 'P5 A-step'

P50len:

P5:

db 'P5'

P5len:

P54T:

db 'P24T Overdrive'

P54Tlen:

P54C:

db 'P54C'

P54Clen:

P55C:

db 'P55C (with MMX)'

P55Clen:

; ---

P60:

db 'Pentium Pro A-step'

P60len:

P61:

db 'Pentium Pro'

P61len:

P63:

db 'Pentium II (Klamath)'

P63len:

P65:

db 'Pentium II (Deschutes)'

P65len:

P66:

db 'Celeron (Medocino)'

P66len:

P67:

db 'Pentium III (Katmai)'

P67len:

P68:

db 'Pentium III (Coppermine)'

P68len:

P69:

db 'Pentium M (Banias)'

P69len:

P6A:

db 'Pentium III Xeon (Cascades)'

P6Alen:

P6B:

db 'Pentium III (Tualatin)'

P6Blen:

P6D:

db 'Pentium M (Dothan)'

P6Dlen:

P6E:

db 'Pentium M (Yonah)/ Core'

P6Elen:

P6F:

db 'Pentium D (Conroe)/ Core 2 (Kentsfield)'

P6Flen:

;---

PS0:

db 'Itanium (IA-64)'

PS0len:

;------------

PF0:

db 'Pentium 4 (Willamete)'

PF0len:

PF2:

db 'Pentium 4 (Northwood)'

PF2len:

PF3:

db 'Pentium 4 (Prescott)'

PF3len:

PF5:

db 'Pentium 4 (Tejas)'

PF5len:

PF6:

db 'Pentium 4 (Presler)'

PF6len:

;----------------Intel Celerons

P65c:

db 'Celeron (Covington)'

P65clen:

P68c:

db 'Celeron (Coppermine)'

P68clen:

P6Bc:

db 'Celeron (Tualatin)'

P6Bclen:

PF0c:

db 'Celeron (Willamete)'

PF0clen:

PF2c:

db 'Celeron (Northwood)'

PF2clen:

PF3c:

db 'Celeron (Prescott)'

PF3clen:

PF5c:

db 'Celeron D (Texas)'

PF5clen:

PF6c:

db 'Celeron D (Presler)'

PF6clen:

;---------AMD

A50     db 'K5 (PR75, PR90, PR100)',0

A51     db '5k86 (PR120, PR133)',0

A52     db '5k86 (PR166)',0

A53     db '5k86 (PR200)',0

A56     db 'K6',0

A57     db 'K6',0

A58     db 'K6-2',0

A59     db 'K6-III',0

A5D     db 'K6-2+ or K6-III+',0

;-------------------

At1     db 'Athlon',0

At2     db 'Athlon',0

At3     db 'Duron (Spitfire)',0

At4     db 'Athlon (Thunderbird)',0

At6     db 'AthlonXP (Palomino)',0

At7     db 'Duron (Morgan)',0

At8     db 'AthlonXP (Thoroughbred)',0

At8a    db 'Duron (Applebred)',0

Ata     db 'AthlonXP (Barton)',0

Atat    db 'AthlonXP (Thorton)',0

;-------------------

AF1:

db 'Dual-core Opteron'

AF1len:

AF3:

db 'Athlon 64 (Toledo)'

AF3len:

AF4:

db 'Athlon 64 (ClawHammer)'

AF4len:

AF5:

db 'Opteron/Athlon 64 FX (SledgeHammer)'

AF5len:

 

AFC:

db 'Athlon 64 (Newcastle)'

AFClen:

 

AFF:

db 'Athlon 64 (Winchester)'

AFFlen:

 

AFS:

db 'Athlon 64 (San Diego)'

AFSlen:

 

AFV:

db 'Athlon 64 (Venice)'

AFVlen:

 

AFCs:

db 'Sempron (Paris)'

AFCslen:

 

AFCsp:

db 'Sempron (Palermo)'

AFCsplen:

 

;---------Cyrix

C52:

db '6x86 M1'

C52len:

C54:

db 'MediaGX'

C54len:

C60:

db '6x86MX M2'

C60len:

C65:

db 'C3 (Cyrix M2)' ;?

C65len:

;--------IDT

V54:

db 'WinChip C6'

V54len:

V58:

db 'WinChip 2'

V58len:

V59:

db 'WinChip 3'

V59len:

;-------VIA

V66:

db 'C3 (Samuel)'  ; Joshua is unreleased 065

V66len:

V67:

db 'C3 (Samuel2/Ezra)' ; ?

V67len:

V68:

db 'C3 (Ezra-T/Eden)' ;?

V68len:

V69:

db 'C3 (Antaur/Nehemiah)' ;?

V69len:

VA:

db 'C7 (Esther)' ;?

VAlen:

;---------Transmeta

T5:

db 'Crusoe' ;

T5len:

TF:

db 'Efficeon' ;

TFlen:

;---------

NG:

    db 'Next generation CPU'

NGlen:

 

stm:

if lang eq it

                db 'Nome:  '

else

                db 'Internal name:'

end if

 

stmlen:

 

athloncoef      db      110, 115, 120, 125, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 120

                db      190, 120, 200, 130, 135, 140, 210, 150, 220, 160, 165, 170, 180, 230, 240

athlonmcoef:    db      110, 115, 120, 125, 50, 55, 60, 65,  70, 75, 80, 85, 90, 95, 100, 105

                db      30, 190, 40, 200, 130, 135, 14, 210, 150, 220, 160, 165, 170, 230, 240

athloncoef3     db      45, 50, 40, 55, 25, 30, 60, 35

p4coef          db      160, 170, 180, 190, 200, 210, 220, 230, 80, 90, 100, 110, 120, 130, 140, 150    ; Pentium 4 (Willamete)

coppercoeff     db       50, 30, 40, 20, 55, 35,  45, 25,  35, 70, 80, 60, 20, 75, 15, 65, 90, 110, 120, 20, 95, 115, 85, 25, 35, 70,  80, 100,  20, 75,  15, 105

tualatcoeff     db      120, 35, 35, 40, 55, 35, 115, 35, 160, 70, 80, 60, 40, 75, 35, 65, 90, 110,  35, 35, 95,  35, 85, 35, 35, 35, 130, 100, 140, 35, 150, 105

 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;

;  include images and unpacking- and hasharea

;

include 'logos.inc' ; include file where gif's are stored

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
I_END:

img_area:          ; image is going to be unpacked to here
rb 201*49*3+8      ; image resolution (bits to reserve)

img_area2:         ; image is going to be unpacked to here
rb 93*24*3+8       ; image resolution (bits to reserve)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; RSA test data

align 4

  num1 rd 40

  num2 rd 40

  num3 rd 40

  iter rd 1

  openkey rd 1

 

IncludeUGlobals

 

ost dd ?

sot dd ?

f dd ?

m dd ?

s dd ?

t dd ?

 

ef dd ?

em dd ?

 

multiplier dd ?

multa dd ?

multb dd ?

 

smallvendor dd ?

L1d  dd ?

L1i  dd ?

L2   dd ?

L3   dd ?

micron dd ?

brand dd ?

 

ram_size_a dd ?

ram_size_t dd ?

 

stdc dd ?

extc dd ?

 

FRS dd ?

freqsel db ?

sse3sup db ?

 

freqbb dd ?

freqll dd ?

 

wayli dd ?

lineli dd ?

 

wayld dd ?

lineld dd ?

 

wayl2 dd ?

linel2 dd ?

 

wayl3 dd ?

linel3 dd ?

 

;number_of_log_cpus dd ?

 

che db ? ; numbers of calls for Intel caches detection

 

myname:

   rb 48

mynamelen:

 

align 4

 

  thread2_stack_area rb 64

  thread2_esp = $

U_END: