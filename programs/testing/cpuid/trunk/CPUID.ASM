;******************************************************************************
; project name:  CPUID                                                        *
; platform:      KolibriOS, x86 (IA-32), x86-64 achitectures                  *
; compiler:      flat assembler 1.70.03                                       *
; version:       2.31                                                        *
; last update:   9 October 2018                                              *
; maintained by: Sergey Kuzmin aka Wildwest                                   *
; e-mail:        kuzmin_serg@list.ru                                          *
; site:          http://coolthemes.narod.ru/files.html                        *
; license:       Copyright 2004-2018 Sergey Kuzmin and co-authors             *
;                Rules:                                                       *
;                1)you can use pieces of code in your project, but should     *
;                mention the original author (include copyright notice);      *
;                2)if you modify CPUID (improve, port, translate,  etc) send   *
;                your changes to the maintainer or make about post changes    *
;                at forum  http://board.kolibrios.org/viewtopic.php?f=42&t=594*
;-----------------------------------------------------------------------------*
; English comments                                                            *
;------------------------------------------------------------------------------
use32
  org    0x0
  db     'MENUET01'
  dd     0x01
  dd     START
  dd     I_END
  dd U_END+4096
  dd U_END+4096
  dd     0x0
  dd     path

include '..\..\..\develop\libraries\box_lib\load_lib.mac'
include '..\..\..\macros.inc'
include '../../../gui_patterns.inc'
include '../../../KOSfuncs.inc'
include 'lang.inc'
include 'draw.inc'
include 'brand.inc'     ; brand ID decoding
include 'caches.inc'    ; L1 and L2 cashes decoding for Intel
include 'multipli.inc'  ; multiplier decoding
include 'features.inc'  ; features decoding
include 'logos.inc'     ; include file where gif's are stored
include 'rsatest.inc'
include 'variable.inc'
include 'gif_lite.inc'

@use_library

START:
        mcall   68,11
        mcall   66,1,1
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

load_libraries l_libs_start,end_l_libs
;-----------------------------------------------------------------------------
;OpenDialog     initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

        mov     edi,filename_area
        mov     esi,start_temp_file_name
        call    copy_file_name_path
;-----------------------------------------------------------------------------
        mcall   68,12,4096*8 ; 16 Kb - I hope this will be enough for store of data
        mov     [store_text_area_start],eax


;------------
CYCLES:
;   CPU speed
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
cpu:                  ;is CPUID supported?
        pushfd              ;push original EFLAGS
        pop eax             ;get original EFLAGS
        mov ebx, eax        ;save original EFLAGS
        xor eax, 00200000h  ;flip 21th bit in EFLAGS
        push eax            ;save new EFLAGS value on stack
        popfd               ;replace current EFLAGS value
        pushfd              ;get new EFLAGS
        pop eax             ;store new EFLAGS in EAX
        cmp eax, ebx        ;compare values of 21th bit
        jz NO_CPUID         ;CPUID isn't supported
;------------
CPUNAME:              ; VENDOR
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

        mov [che], al        ; number of calls
multik:

.eaxl:
        test  eax, eax       ;    Test bit 31
        js    .ebxl          ;    <> 0 =>invalid values
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
        inc eax   ; eax=1
        cpuid

        mov ecx, eax
        shr ecx,8         ;   shift it to the correct position
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
        mov ecx, eax      ; get Type
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

        mov ecx, eax          ; get Extended model
        shr ecx,16            ;shift it to the correct position
        and ecx, 0000000Fh
        shl ecx, 4
        mov dword[newpc],ecx     ;  this value for old pc=0 and for new pc>0
        add ecx, [m]
        mov dword[em],ecx     ;  effective    model

        mov ecx, eax          ; get Extended family
        shr ecx, 20           ;shift it to the correct position
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
        mov eax, 0x80000001               ; CPUID ext. function 0x80000001
        cpuid
        mov ecx, eax
        shr ecx,8         ;   shift it to the correct position
        and ecx,0000000Fh ;   get CPU family
        mov dword[ef],ecx

        mov ecx, eax
        shr ecx,4
        and ecx,0000000Fh ;    get CPU model
        mov dword[em],ecx

fut:

        call decode_sse3_5



        ;call decode_sse3_5
        ;-
        call decode_extended

        mov   eax,$80000000
        cpuid

        mov   [extc], eax  ; max number of calls

        test  eax, $80000000 ;// Test bit 31
        jz .noname

        cmp  eax,$80000003
        ja .mynameis
        jmp .noname

.mynameis:
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
        jmp   red

.noname:

red:

        ;mov byte [multiplier], 115;     ; for testing

        call decode_sse3

        call multipl                          ; get multiplier
        mov byte [multiplier], cl

        mov   dword [freqbb], 0
        mov   dword [freqll], 0

        mov   ebx, dword [multiplier]
        test  ebx, ebx
        jz output

        calc:

        mov eax,dword [ost]   ; example 166474
        imul eax, 10          ; example 1664740
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

;HRERE

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
        cmp dword[smallvendor], 'ntel'   ;1
        jz Intel
        cmp dword[smallvendor], 'cAMD'   ;2
        jz AMD
        cmp dword[smallvendor], 'tead'   ;3
        jz Cyrix
        cmp dword[smallvendor], 'auls'    ;4
        jz Centaur
        cmp dword[smallvendor], 'Mx86'    ;5
        jz Transmeta
        cmp dword[smallvendor], ' SoC'    ;6
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

        mov esi, other
        mov edi, [saveproc]
        call concatname

        jmp MMXtest
;-------------------------

AMD:

;-------------------------------------------------------------------------------

        Text 15, 190,0x00000000,cache, cachelen-cache

        Text 75,70,,AMDn, AMDnlen-AMDn
        mov esi, AMDnNew
        mov edi, saveproc
        call concatname
        mov     esi, amd
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        MOV [codeN], 2
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

;CACHE3: edx provides l3

        mov eax, 80000006h
        cpuid

        movzx eax, cl       ;mov cl to eax, zero extend; cl is counter reg for loop,shifts
        mov dword[linel3], eax

        push edx
        shr edx, 12+1
        and edx, 0x7
        mov eax, 1
        shl eax, cl
        mov dword [wayl3], eax
        pop edx

        shr edx, 18
        mov [L3],ecx

        cmp [f], $5
        jz .fiv
        cmp [f], $6
        jz .si
        cmp [f], $F
        jz fif
        cmp [f], $10   ;family 16, 010h,
        jz ten


.fiv:    ;     Family=5
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
        mov     [cname], edx
        jmp     MMXtest
A6:
;mov     [FRS], 266  ;!!!!!!
;Number  315,90,0,3,dword [FRS],0x000000; MHz

        call    newrating; !!!!

        Text    245,70,0x00000000,pr, prlen-pr
        Number  310,70,0,4,dword [rating],0x000000
        mov     edx, At6
        jmp     @b

A8:

;mov     [FRS], 266     ;!!!!!!
;Number  315,90,0,3,dword [FRS],0x000000; MHz

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


        cmp [m],$1   ;Dual-core Opteron
        jz   AF1.
        cmp [m],$3   ;Toledo 1024 0.09
        jz  AF3.
        cmp [m],$4   ;Athlon 64 Mobile Athlon 64 FX  ClawHammer (1024) 0.13
        jz AF4.
        cmp [m],$5   ;Opteron Athlon 64 FX 0.13 (1024)
        jz AF5.
        cmp [m],$7   ;Athlon 64 Athlon 64 FX  Clawhammer(1024) 0.13   Sledgehammer(1024)  0.13  // SSE3+ SanDiego(1024)
        jz AF7.
        cmp [m],$8   ;Athlon 64 Mobile Athlon 64 FX ClawHammer (1024) 0.13
        jz AF8.
        cmp [m],$B   ;Athlon 64
        jz AFB.
        cmp [m],$C   ;Athlon 64 Newcastle(512) 0.13  Sempron> Paris (256)   0.13  |SSE3+ Sempron >  Palermo FC0 0.09  // (Venice)
        jz AFC.
        cmp [m],$E   ;Athlon 64
        jz AFE.
        cmp [m],$F   ;Athlon 64 Winchester(512) |SSE3+ SanDiego(1024)  Venice (512)  Palermo (256) 0.09
        jz AFF.
        jmp next_generation

ten:      ;family = 10h

        cmp [m],$2
        jz AB23.
        cmp [m], $8
        jz AB83.
        cmp [m], $9
        jz AB9.
        cmp [m], $2
        jz AB8check2.

AB8check2.:
        cmp [s],03h
        jz AB8right2.
        cmp [m],$2
        jz athlonCheck

athlonCheck:
        cmp [s],03h
        jz athlonKuma

        cmp [m], $5
        jz AB4.
        cmp [m],$1
        jz AB1.
        cmp [m],$4
        jz AB8check.

AB8check.:
        cmp [s],02h
        jz ABC2.
        cmp [s],01h
        jz ABC3.
        cmp [m], $9
        jz AB6.
        cmp [m], $2
        jz AB8check.
        cmp [m], 06h
        jz ABM.

athlonKuma:
        mov [micron], 65
        Text 100,70,0x00000000,AthlonKuma, AthlonKumalen-AthlonKuma
        mov esi, AthlonKuma
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB23.:
        mov [micron], 65
        Text 100,70,0x00000000,AB23, AB23len-AB23
        mov esi, AB23
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB83.:
        mov [micron], 65
        Text 100,70,0x00000000,AB83, AB83len-AB83
        mov esi, AB83
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB9.:
        mov [micron], 65
        Text 100,70,0x00000000,AB9, AB9len-AB9
        mov esi, AB9
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB8right2.:
        mov [micron], 65
        Text 100,70,0x00000000,AB8right2, AB8right2len-AB8right2
        mov esi, AB8right2
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB4.:
        mov [micron], 45
        Text 100,70,0x00000000,AB4, AB4len-AB4
        mov esi, AB4
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB1.:
        mov [micron], 65
        Text 100,70,0x00000000,AB1, AB1len-AB1
        mov esi, AB1
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

ABC2.:
        mov [micron], 45
        Text 100,70,0x00000000,ABC2, ABC2len-ABC2
        mov esi, ABC2
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AB6.:
        mov [micron], 45
        Text 100,70,0x00000000,AB6, AB6len-AB6
        mov esi, AB6
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

ABC3.:
        mov [micron], 45
        Text 100,70,0x00000000,ABC3, ABC3len-ABC3
        mov esi, ABC3
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

ABM.:
        mov [micron], 45
        Text 100,70,0x00000000,ABM2, ABM2len-ABM
        mov esi, ABM2
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AF1.:
        mov [micron], 09  ;?
        Text 100,70,0x00000000,AF1, AF1len-AF1
        mov esi, AF1
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest
AF3.:
        mov [micron], 09
        Text 100,70,0x00000000,AF3, AF3len-AF3
        mov esi, AF3
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest
AF4.:
        mov [micron], 13
        Text 100,70,0x00000000,AF4, AF4len-AF4
        mov esi, AF4
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest
AF5.:
        mov [micron], 13
        Text 100,70,0x00000000,AF5, AF5len-AF5
        jmp MMXtest

AF7.:
        mov [micron], 13
        Text 100,70,0x00000000,AF5, AF5len-AF5
        mov esi, AF5
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AF8.:
        mov [micron], 13
        Text 100,70,0x00000000,AF4, AF4len-AF4
        mov esi, AF4
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AFB.:
        mov [micron], 13
        Text 100,70,0x00000000,AF4, AF4len-AF4
        mov esi, AF4
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AFC.:
        cmp [L2], 512
        je AFC.n

        cmp [sse3sup], 1
        je AFC.npal

AFC.npar:  ; paris
        mov [micron], 13
        Text 100,70,0x00000000,AFCs, AFCslen-AFCs
        mov esi, AFCs
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AFC.npal: ; palermo
        mov [micron], 9
        Text 100,70,0x00000000,AFCsp, AFCsplen-AFCsp
        mov esi, AFCsp
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AFC.n: ;newcastle
        mov [micron], 13
        Text 100,70,0x00000000,AFC, AFClen-AFC
        mov esi, AFC
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

AFE.:   ; error in cpu
        jmp AFC.

AFF.:

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
        mov esi, AFS
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

.AFd:
        Text 100,70,0x00000000,AFV, AFVlen-AFV
        mov esi, AFV
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest

.AFp:
        Text 100,70,0x00000000,AFCsp, AFCsplen-AFCsp
        mov esi, AFCsp
        mov edi, saveproc + 0x4
        call concatname
        jmp MMXtest
;-----------------------------------------------
Intel:
        mov [codeN], $1
        Text 75,70,0x00000000,Inteln, Intelnlen-Inteln
        mov esi, IntelnNew
        mov edi, saveproc
        call concatname
        mov     esi, intel
        call    load_gif
        PutImage 135,107,201,49,img_area+8

det:
        cmp [f], $5
        jz five
        cmp [f], $6
        jz six
        cmp [f], $7
        jz sev
        cmp [f], $F
        jz fift
five:        ;Family=5

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
        mov [cname], P50
        mov esi, P50
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 80
        jmp MMXtest
.I1:
        mov [cname], P5
        mov esi, P5
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 50
        jmp MMXtest

.I2:
        mov [cname], P54C
        mov esi, P54C
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 50
        jmp MMXtest

.I3:
        mov [cname], P54T
        mov esi, P54T
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 50
        jmp MMXtest

.I4:
        mov [cname], P55C
        mov esi, P55C
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 35
        jmp MMXtest


.I7:
        mov [cname], P54C
        mov esi, P54C
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 8
        mov [L1i], 8
        mov [L2], 256
        mov [micron], 35
        jmp MMXtest

.I8:
        mov [cname], P55C
        mov esi, P55C
        mov edi, saveproc + 0x6
        call concatname
        mov [L1d], 16
        mov [L1i], 16
        mov [L2], 256
        mov [micron], 35
        jmp MMXtest

six:              ;Family=6

        Text 15, 190,0x00000000,cache, cachelen-cache
        cmp [newpc],$0
        jnz NEWintel

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
        mov [cname], P60
        mov esi, P60
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I61:
        mov [micron], 35
        mov [cname], P61
        mov esi, P61
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I63:
        mov [micron], 28
        mov [cname], P63
        mov esi, P63
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I65:
        mov [micron], 25
        cmp [L2], 0
        jne .pp65  ; Pentium
        mov [cname], P65c
        mov esi, P65c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.pp65:
        mov [cname], P65
        mov esi, P65
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.I66:
        mov [micron], 25
        mov [cname], P66
        mov esi, P66
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.I67:
        mov [micron], 25
        ;but if SSE4.1 supported then it is Intel Core (Penryn)
        mov [cname], P67
        mov esi, P67
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I68:
        mov [micron], 18
        cmp [L2], 128
        jne .pp68  ; Pentium
        mov [cname], P68c
        mov esi, P68c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

 .pp68:
        mov [cname], P68
        mov esi, P68
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I69:
        mov [micron], 13
        mov [cname], P69
        mov esi, P69
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I6A:
        mov [micron], 18
        ;but if SSE4.2 supported then it is Intel Core (Nehalem)
        mov [cname], P6A
        mov esi, P6A
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I6B:
        mov [micron], 13
        cmp [L2], 256
        jne .pp6B  ; Pentium
        mov [cname], P6Bc
        mov esi, P6Bc
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.pp6B:
        mov [cname], P6B
        mov esi, P6B
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I6D:
        mov [micron], 9
        mov [cname], P6D
        mov esi, P6D
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I6E:
        mov [micron], 6
        mov [cname], P6E
        mov esi, P6E
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.I6F:
        mov [micron], 6
        mov [cname], P6F
        mov esi, P6F
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

;06Ex - Pentium M Yonah 0.065
;06Fx - Pentium D Conroe 0.065, Xeon Woodcrest, Celeron D AllenDale, Core 2 Kentsfield

NEWintel:

        cmp [em],$3A   ;IvyBridge
        jz I3A
        cmp [em],$2A   ;SandyBridge
        jz I2A
        cmp [em],$2D  ;SandyBridge-E/EN/EP
        jz I2D
        cmp [em],$25   ;Arrandale/Clarksdale
        jz I25
        cmp [em],$2C   ;Gulftown/Westmere-EP
        jz I2C        ;westmere-EP stepping B1 -> micron 0.032
        cmp [em],$2F   ;Westmere-EX
        jz I2F
        cmp [em],$1E   ;Clarksfield/Lynnfield/Jasper Forest
        jz I1E
        cmp [em],$1A   ;Bloomfield/Nehalem-EP
        jz I1A
        cmp [em],$2E   ;Nehalem-EX
        jz I2E
        cmp [em],$17   ;Yorkfield/Wolfdale/Penryn/Harpertown (DP)
        jz I17
        cmp [em],$1D   ;Dunnington (MP)
        jz I1D
        cmp [em],$0F   ;Clovertown/Kentsfield/Conroe/Merom/Woodcrest
        jz I0F
        cmp [em],$16   ;Merom Conroe
        jz I16
        cmp [em],$06   ;Cedar Mill/Presler
        jz I06
        cmp [em],$03   ;Nocona Irwindale / Prescott
        jz I03
        cmp [em],$04   ;NoconaIrwindale / Prescott
        jz I03
        cmp [em],$0D   ;Dothan
        jz I0D
        cmp [em],$36   ;Cedarview
        jz I36
        cmp [em],$26   ;Lincroft
        jz I26
        cmp [em],$1C   ;Pineview/Silverthorne
        jz I1C
        mov [cname], no_known

Inewunknown:
        jmp MMXtest

I3A:
        mov [micron], 32
        mov [cname], P3A
        mov esi, P3A
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I2A:
        mov [micron], 32
        mov [cname], P2A
        mov esi, P2A
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I2D:
        mov [micron], 32
        mov [cname], P2D
        mov esi, P2D
        mov edi,saveproc + 0x10
        call concatname
        jmp MMXtest

I25:
        mov [micron], 32
        mov [cname], P25  ;P25
        mov esi, P25
        mov edi, saveproc + 0x10  ;12
        call concatname
        jmp MMXtest

I2C:
        mov [micron], 32
        mov [cname], P2C
        mov esi, P2C
        mov edi,saveproc + 0x10
        call concatname
        jmp MMXtest

I2F:
        mov [micron], 32
        mov [cname], P2F
        mov esi, P2F
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I1E:
        mov [micron], 32
        mov [cname], P1E
        mov esi, P1E
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I1A:
        mov [micron], 45
        mov [cname], P1A
        mov esi, P1A
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I2E:
        mov [micron], 45
        mov [cname], P2E
        mov esi, P2E
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I17:
        mov [micron], 45
        mov [cname], P17
        mov esi, P17
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I1D:
        mov [micron], 45
        mov [cname], P1D
        mov esi, P1D
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I0F:
        mov [micron], 65
        mov [cname], P0F
        mov esi, P0F
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I16:
        mov [micron], 65
        mov [cname], P16
        mov esi, P16
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I06:
        mov [micron], 32
        mov [cname], P06
        mov esi, P06
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I03:
        mov [micron], 32
        mov [cname], P03
        mov esi, P03
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I0D:
        mov [micron], 32
        mov [cname], P0D
        mov esi, P0D
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I36:
        mov [micron], 32
        mov [cname], P36
        mov esi, P36
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I26:
        mov [micron], 32
        mov [cname], P26
        mov esi, P26
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest

I1C:
        mov [micron], 32
        mov [cname], P1C
        mov esi, P1C
        mov edi, saveproc + 0x10
        call concatname
        jmp MMXtest


;;;;;;;;;;;;;;;;;;;
sev:    ;Family=7
.IS0:

        Text 15, 190,0x00000000,cache, cachelen-cache
        mov [micron], 18
        mov [cname], PS0
        mov esi, PS0
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

fift:    ;Family=15

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
        mov [cname], PF0c
        mov esi, PF0c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.ppF0:
        mov [cname], PF0
        mov esi, PF0
        mov edi, saveproc + 0x6
        call concatname
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
        mov [cname], PF0
        mov esi, PF0
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.IF2:
        mov [micron], 13
        cmp [L2], 128
        jne .ppF2  ; Pentium
        mov [cname], PF2c
        mov esi, PF2c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.ppF2:
        mov [cname], PF2
        mov esi, PF2
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.IF3:
        mov [micron], 09
        cmp [L2], 256
        jne .ppF3  ; Pentium
        mov [cname], PF3c
        mov esi, PF3c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.ppF3:
        mov [cname], PF3
        mov esi, PF3
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.IF5:
        mov [micron], 09
        cmp [L2], 512
        jae .ppF5  ; Pentium
        mov [cname], PF5c
        mov esi, PF5c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.ppF5:
        mov [cname], PF5
        mov esi, PF5
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

 .IF6:
        mov [micron], 06  ; 065
        cmp [L2], 512
        ja .ppF6  ; Pentium
        mov [cname], PF6c
        mov esi, PF6c
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.ppF6:
        mov [cname], PF6
        mov esi, PF6
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest


 next_generation:
        mov [cname], NG
        mov esi, NG
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
;----------------------------------
Cyrix:

        mov [codeN], 3
        Text 15, 190,0x00000000,cache, cachelen-cache

        mov     esi, cyrix
        call    load_gif
        PutImage 135,107,201,49,img_area+8


        cmp [f], $5
        jz .fivv
        cmp [f], $6
        jz .sixx
.fivv:    ;Family=5
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
        mov esi, Cyrixn
        mov edi, saveproc
        call concatname
        mov [cname], C52
        mov esi, C52
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.C54:
        mov [micron], 50
        mov [L1i], 8
        mov [L1d], 8
        mov [L2], 512
        Text 75,70,0x00000000,Cyrixn, Cyrixnlen-Cyrixn
        mov esi, Cyrixn
        mov edi, saveproc
        call concatname
        mov [cname], C54
        mov esi, C54
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest

.sixx:     ;Family=6
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
        mov esi, Cyrixn
        mov edi, saveproc
        call concatname
        mov [cname], C60
        mov esi, C60
        mov edi, saveproc + 0x6
        call concatname
        jmp MMXtest
.C65:
        mov [micron], 25 ;35?
        mov [L1i], 32
        mov [L1d], 32
        mov [L2], 512
        Text 75,70,0x00000000,Centaurn, Centaurnlen-Centaurn
        mov esi, Centaurn
        mov edi, saveproc
        call concatname
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
        jmp vl2old      ; if not then old identification
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

fivC:              ;Family=5

        mov     esi, idt
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        ;PutImage 125,107,201,49,img_area+8
        ;         place   size

        Text 75,70,0x00000000,IDTn, IDTnlen-IDTn
        mov esi, IDTn
        mov edi, saveproc + 0x4
        call concatname
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

sixC:   ;Family=6

        mov     esi, via
        call    load_gif
        PutImage 135,107,201,49,img_area+8


        Text 75,70,0x00000000,Centaurn, Centaurnlen-Centaurn
        mov esi, Centaurn
        mov edi, saveproc
        call concatname
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
        mov esi, Tranmsmetan
        mov edi, saveproc
        call concatname

        mov     esi, transmeta
        call    load_gif
        PutImage 135,107,201,49,img_area+8

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
.fivt:    ;     Family=5

        mov edx,T5
        mov esi,T5len-T5
        jmp @f

.fift:    ;     Family=F
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

        mov     esi, vortex
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        ;PutImage 130,127,201,49,img_area+8
        ;         place   size

        cmp [f], $5
        jz .V54 ;fivvtx
;.fivvtx:         ;Family=5
;       cmp [m],$4
;       jz .V54
.V54:
        mov [micron], 13
        mov [L1i], 16
        mov [L1d], 16
        mov [L2], 256
        mov [wayl2], 4
        mov [wayli], 4
        mov [wayld], 4
        Text 75,70,0x00000000,Vortexn, Vortexnlen-Vortexn
        mov esi, Vortexn
        mov edi, saveproc
        call concatname
        jmp MMXtest

;---------------------



MMXtest:             ; MMX test and Brand ID decoding

        call decodebrand  ; get Brand ID

        call decode_standard_features

        call decode_extended_features

        xor eax,eax
        inc eax
        cpuid
HTTtest:
        test  edx, $10000000; ;Test bit 28
        jz .ELN

.EL:   ;HTT technology is supported
        and ebx,00FF0000h ; numbers of logical processors
        cmp ebx, 1 shl 16
        ;   mov [number_of_log_cpus], ebx
        je .ELN  ; HHT not enabled (Celeron)

        mov  dword [HTTn+9], $736579
        mov  dword [HTT+ 6], $736579
        jmp TEXTOUT
.ELN:

        mov  dword [HTTn+ 9],  $6F6E
        mov  dword [HTT+ 6],  $6F6E

TEXTOUT:

        Text 275,290,0x00000000,HTT, HTTlen-HTT
        Text 275,310,,sse3, sse3len-sse3
        Text 15,290,,MMXs, MMXslen-MMXs
        Text 15,310,,SSE, SSElen-SSE
        Text 95,310,,SSE2, SSE2len-SSE2

;-------------------
TEST3DNOW:

        xor edx, edx
        cmp [smallvendor], 'ntel'
        ;  je @f  ;recent change
        jne .t

.t:

        mov   eax, $80000001 ;// Setup extended function 8000_0001h
        cpuid

        test  edx, $80000000 ;// Test bit 31
        jnz   .XIT

.NOEXTENDED: ;// 3DNow! technology is supported
        mov  dword [now+ 9], $6F6E
        jmp TEST3DNOWP
.XIT:
        mov  dword [now+ 9],  $736579
        jmp TEST3DNOWP

TEST3DNOWP:

        cmp [smallvendor], 'ntel'
        je .NOEXTENDEDP

.tp:

        mov   eax, $80000001 ;// Setup extended function 8000_0001h
        cpuid

        test    edx, $40000000 ;// Test bit 30
        jnz   .XITP  ;// 3DNow! technology is supported

.NOEXTENDEDP:
        mov  dword [nowp+ 9], $6F6E
        jmp TESTMMXP
.XITP:
        mov  dword [nowp+ 9],  $736579
        jmp TESTMMXP


TESTMMXP:

        mov   eax,$80000000
        cpuid

        test eax, 80000000h
        jna NOEXTENDEDM

        ;cmp   eax, $80000000 ;// Is 800_0001h supported?
        ;jz   .NOEXTENDEDM    ;// If not, 3DNow! technology is not supported
        mov   eax, $80000001 ;// Setup extended function 8000_0001h
        cpuid
        cmp [smallvendor], 'tead'
        jne noCyr
Cyrmx:
        test  edx, $01000000 ;// Test bit 24
        jnz   XITM  ;// 3DNow! technology is supported
        jz NOEXTENDEDM
noCyr:
        test  edx, $00400000 ;// Test bit 22
        jnz   XITM  ;// 3DNow! technology is supported
        ;jz   .NOEXTENDEDM

NOEXTENDEDM:
        mov  dword [mmxp+ 7], $6F6E
        mov  dword [MMXPi+ 8], $6F6E
        jmp text3d
XITM:
        mov  dword [mmxp+ 7],  $736579
        mov  dword [MMXPi+ 8],  $736579

text3d:

        Text 15,330,0x00000000,now, nowlen-now
        Text 95,330,,nowp, nowplen-nowp
        Text 95,290,,mmxp, mmxplen-mmxp
        Text 110,70,0x80000000,dword[cname]

        jmp still

;--------------------------
NO_CPUID:
        mov [nocpuid], 1
        Text 15,50,0x00000000,oblom, oblomlen-oblom

FREEZE:
        nop
        jmp FREEZE ; maybe we should close application or just made some Warning and jump to still:
;----------------
still:

; waiting for events
event_wait:

        mov     eax,23       ; function 23 - event wait
        mov     ebx,50       ; wait for 0.5 second
        mcall

        cmp  eax,1              ;
        je       red            ;  redraw
        cmp  eax,2              ;
        je       key            ;  key
        cmp  eax,3              ;
        je       button         ;  button
        jmp  still              ;
  key:                  ;
        mcall           ;
        jmp  still              ;
  button:               ;
        mov  eax,17     ;
        mcall           ;
        cmp  ah,1               ;  = 1 ?
        je      close           ; close

        cmp  ah,2               ;  = 2 ?
        je      thread_start    ;

        cmp  ah, 3
        je call_OpenDialog

         mov    eax,11       ; function 23 - event wait
        mov  ebx,1
        mcall

        mov  eax,15       ; function 23 - event wait
        mov  ebx,3
        mcall

;    cmp  ah,3          ;  = 3 ?
        jne  still

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
        cmp  eax,dword [esp]   ;Is time changed?
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
        je       window_2       ;  window_2
        cmp  eax,2              ;
        je       key_2          ;  key_2
        cmp  eax,3              ;
        je       button_2       ;  button_2

        jmp  still_2    ;

  key_2:                ;
        mcall           ;
        jmp  still_2    ;

  button_2:             ;
        mov  eax,17     ; 17
        mcall           ;

        cmp  ah,1               ; = 1 ?
        jne  still_2    ; noclose

        mov  [num_win2],0       ;

        or       eax,-1         ;
        mcall


draw_window_2:

        ;call prepare_text_area

        mov  eax,12                ; function 12:tell os about windowdraw
        mov  ebx,1h                 ; 1, start of draw
        mcall


        Window 250,250,420,520, 0x33FFFFFF, 0x805080d0, standard  ;460
        ; place size


        Text 15, 10,0x00000000, STDCA, STDCAlen-STDCA
        Text 215, 10,, EXTCA, EXTCAlen-EXTCA

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
        ;mov eax, $03020101  \A0for test of reaction
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

        Text 15,355,0x00000000,SMX, SMXlen-SMX
        Text 15,335,0x00000000,PAGE, PAGElen-PAGE
        Text 15,375,0x00000000,MIS, MISlen-MIS
        Text 115,355,0x00000000,WDT, WDTlen-WDT

        Text 115,195,,CNXT_ID, CNXT_IDlen-CNXT_ID
        Text 115,215,,CX16, CX16len-CX16
        Text 115,235,,ETPRD, ETPRDlen-ETPRD
        Text 115,255,,SYS, SYSlen-SYS
        Text 115,275,,LAF, LAFlen-LAF
        Text 115,295,,SSSE3, SSSE3len-SSSE3
        Text 115,315,,MCR8, MCR8len-MCR8


        Text 115,335,0x00000000,EAS, EASlen-EAS

        Text 115,375,0x00000000,DNP, DNPlen-DNP


        Text 315,375,0x00000000,SSE5, SSE5len-SSE5

        Text 215,195,,MP, MPlen-MP
        Text 215,215,,NX, NXlen-NX
        Text 215,235,,MMXPi, MMXPilen-MMXPi
        Text 215,255,,MMXn, MMXnlen-MMXn
        Text 215,275,,FXSRn, FXSRnlen-FXSRn
        Text 215,295,,DCA,DCAlen-DCA

        Text 315,295,0x00000000,SSE41,SSE41len-SSE41
        Text 215,335,0x00000000,x2APIC,x2APIClen-x2APIC
        Text 215,355,0x00000000,ABM,ABMlen-ABM
        Text 215,375,0x00000000,OSVW,OSVWlen-OSVW

        Text 315,195,,FFXSR, FFXSRlen-FFXSR
        Text 315,215,,TSCP, TSCPlen-TSCP
        Text 315,235,,LM, LMlen-LM
        Text 315,255,,DNo, DNolen-DNo
        Text 315,275,,DN, DNlen-DN
        Text 215,315,,CMPL, CMPLlen-CMPL
        Text 315,315,0x00000000,SS42,SS42len-SS42
        Text 315,335,0x00000000,PPCNT,PPCNTlen-PPCNT
        Text 315,295,0x00000000,SSE4A,SSE4Alen-SSE4A
        Text 315,355,0x00000000,SKINIT_,SKINIT_len-SKINIT_

too:
        DrawLine 10,  400, 430,430,0x8080FF  ;10

        Text 15,415,0x00000000,speed, speedlen-speed
        Text 130,415,,kbpersec, kbperseclen-kbpersec

        DrawLine 10, 80, 400, 400, 0x8080FF

        Text 90,400,0x80000000,performancestr, 0
        DrawLine 322, 400, 400, 400, 0x8080FF
        DrawLine 10, 10, 400, 490, 0x8080FF
        DrawLine 400, 400, 400, 490, 0x8080FF

        DrawLine 10, 400, 490, 490, 0x8080FF

          ; your proc
        Text 20,415,0x808080FF,currentcpu, 0
        Number 170,415,0,5,dword [iter],0x000000       ; + 15
        ;Text 115,370,,kbpersec, kbperseclen-kbpersec  ;+ 355
        mov eax, dword[iter]
        ;mov ebx, 100
        ;mul ebx
        add eax, 215
        mov word[linelen], ax   ; need to store it as drawline corrupts eax
        DrawLine 215,  [linelen], 416,416,0x8080FF  ;10 + 355 , 357
        DrawLine 215,  [linelen], 417,417,0x8080FF  ;10
        DrawLine 215,  [linelen], 418,418,0x8080FF  ;10
;;;     DrawLine 20,  390, 371,371,0x8080FF  ;10

        ; sample proc 1
        Text 20,435,0x80000000,samplename1,0       ; 10 + 40 + 40 +40 + 380
        Number 170,435,0,5,dword [samplespeed1],0x000000;   ; 25 + 40 + 40 +40 + 380
        ;Text 115,470,0,kbpersec, kbperseclen-kbpersec  ; 25 + 40 + 40 +40 + 380
        mov eax, dword[samplespeed1]
        add eax, 215
        mov dword[linelen], eax   ; need to store it as drawline corrupts eax
        DrawLine 215,  [linelen], 436,436,0x8080FF  ;27+40 + 40 +40  + 380
        DrawLine 215,  [linelen], 437,437,0x8080FF  ;10
        DrawLine 215,  [linelen], 438,438,0x8080FF  ;10

        ; sample proc 2
        Text 20,455,0x80000000,samplename2,0     ; 10 + 40 + 350
        Number 170,455,0,5,dword [samplespeed2],0x000000;   ; 25 + 40 + 350
        ;Text 115,410,0,kbpersec, kbperseclen-kbpersec  ; 25 + 40 + 350
        mov eax, dword[samplespeed2]
        add eax, 215
        mov dword[linelen], eax   ; need to store it as drawline corrupts eax
        DrawLine 215,  [linelen], 456,456,0x8080FF  ;27+40 + 355
        DrawLine 215,  [linelen], 457,457,0x8080FF  ;10
        DrawLine 215,  [linelen], 458,458,0x8080FF  ;10

           ; sample proc 3
        Text 20,475,0x80000000,samplename3,0     ; 10 + 40 + 40 + 38
        Number 170,475,0,5,dword [samplespeed3],0x000000;   ; 25 + 40 + 40 + 355
        ;Text 115,440,0,kbpersec, kbperseclen-kbpersec  ; 25 + 40 + 40 + 380
        mov eax, dword[samplespeed3]
        add eax, 215
        mov dword[linelen], eax   ; need to store it as drawline corrupts eax
        DrawLine 215,  [linelen], 476,476,0x8080FF  ;27+40 + 40
        DrawLine 215,  [linelen], 477,477,0x8080FF  ;10
        DrawLine 215,  [linelen], 478,478,0x8080FF  ;10

        mov  eax,12
        mov  ebx,2h
        mcall

        ret

linelen  dd 0
currentcpu db 'Current CPU',0
samplename1 db 'Intel Core i5 CPU', 0x000000    ; not real results!
samplespeed1 dd 62
samplename2 db 'Intel Core i3 CPU', 0
samplespeed2 dd 48
samplename3 db 'Intel Pentium Dual CPU', 0
samplespeed3 dd 35
performancestr db 'PERFORMANCE (KB/S in RSA test 256 bit)',0
ptsstring db ''
num_win2 db 0

draw_window:
        mcall SF_REDRAW, SSF_BEGIN_DRAW

        Window 150, 150, 350, 405, 0x34FFFFFF, 0x805080d0, title

        mov   esi, [sc.work_button]
        mcall SF_DEFINE_BUTTON, <138,92>, <345,24>, 2
        mcall SF_DEFINE_BUTTON, <238,92>, <345,24>, 3

        mov   ecx, [sc.work_button_text]
        or    ecx, 0x90000000
        mov   edx, btn_more_cap
        mcall SF_DRAW_TEXT, (138 + (92 - 7 * 8) / 2) shl 16 + 350
        mov   edx, btn_save_cap
        mcall SF_DRAW_TEXT, (238 + (92 - 4 * 8) / 2) shl 16 + 350

        Text 130,270,0x00000000,instruct, instructlen-instruct
        DrawLine  10,  330, 340,340,0x8080FF
        DrawLine 330,  330, 275,340;,0x8080FF
        DrawLine  10,   10, 275,340;,0x8080FF
        DrawLine  10,  125, 275,275;,0x8080FF
        DrawLine 230,  330, 275,275;,0x8080FF

        cmp dword[smallvendor], 'cAMD'
        jne cont
        cmp [f], $6
        jne cont

;       Button 240,85,69,15,3,0x030000FF  ;  button for rating
;       Text 245,90,0x00FFFFFF,FR, FRlen-FR ; text at button

        call newrating; !!!!

cont:
        ;Number 82,50,0,4,dword [total],0x000000; MHz
        mov esi, total
        mov edi, tsum + 0xB  ;0xA
        call savenumber

        ;Number 110,50,0,2,dword [sot]; KHz
        mov esi, sot
        mov edi, tsum + 0x10
        call savenumber

        ;Number 75,110,1*256,1,dword [f],0x000000 ;
        mov esi, f
        mov edi, fam + 0x8     ;0x9
        call savenumber

        ;Number 75,130,,,dword [m]
        mov esi, m
        mov edi, mode + 0x7
        call savenumber

        ;Number 75,150,,,dword [s]
        mov esi, s
        mov edi, step + 0xa   ;0x9
        call savenumber

        ;Number 110,110,1*256,2,dword [ef]
        mov esi, ef
        mov edi, fam + 0xE    ;0x9
        call savenumber

        ;Number 110,130,,,dword [em]
        mov esi, em
        mov edi, mode + 0xE   ;0xD
        call savenumber

        mov esi, multb
        mov edi, multil + 0xB
        call savenumber

        ;Number 105,30,0,1,dword [multa]
        mov esi, multa
        mov edi, multil + 0xf
        call savenumber

        ;Number 140,170,0,2,dword [wayld],0x000000
        mov esi, wayld
        mov edi, cache2 + 0x14   ;0x14
        call savenumber

        ;Number 218,170,,,dword [lineld]
        mov esi, lineld
        mov edi, cache2 + 0x21
        call savenumber

        ;Number 140,190,,,dword [wayli]
        mov esi, wayli
        mov edi, cache + 0x14
        call savenumber
        ;Number 218,190,,,dword [lineli]
        mov esi, lineli
        mov edi, cache + 0x21
        call savenumber

        ;Number 140,210,,,dword [wayl2]
        mov esi, wayl2
        mov edi, cache3 + 0x14
        call savenumber
        ;Number 218,210,,,dword [linel2]
        mov esi, linel2
        mov edi, cache3 + 0x21
        call savenumber

        ;Number 140,230,,,dword [wayl3]
        mov esi, wayl3
        mov edi, cache4 + 0x14
        call savenumber
        ;Number 218,230,,,dword [linel3]
        mov esi, linel3
        mov edi, cache4 + 0x21
        call savenumber

        mov esi, L1d
        mov edi, cache2 + 0xa
        call savenumber

        ;Number 75,190,,,dword [ L1i]
        mov esi, L1i
        mov edi, cache + 0xa
        call savenumber

        ;Number 41,210,0,4,dword[L2]
        mov esi, L2
        mov edi, cache3 + 0x4    ;0x3
        call savenumber

        ;Number 35,230,0,5,dword[L3]
        mov esi, L3
        mov edi, cache4 + 0x4   ;0x3
        call savenumber

        ;-----------Features
        ;Number 258,50,0,2,dword [micron]  ; micron
        mov esi, micron
        mov edi, tech + 0xE
        call savenumber

        mov esi, stdc
        mov edi, STDCA + 0x14
        call savenumber

        ;Number 335,10,,,dword [extc],
        mov esi, extc
        mov edi, EXTCA + 0x14
        call savenumber

        Text 15,90,,cpuname, cpunamelen-cpuname;
        Text 255,250,,typen, typenlen-typen
        Text 175, 50,,tech, techlen-tech;

   red2:

    ;;;;;;;;;;;;;;;;;;;;;;;;
    goon:

        call decodebrand

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

        Text 15,250,,brandid, brandidlen-brandid

        Text 15,50,0x00000000,tsum, tsumlen-tsum
        Text 15,110,0x00000000,fam, famlen-fam
        Text 15,130,0x00000000,mode, modelen-mode
        Text 15,150,0x00000000,step, steplen-step

        Text 275,290,0x00000000,HTT, HTTlen-HTT
        Text 275,310,,sse3, sse3len-sse3

        Text 175,290,0x00000000,SSE41, SSE41len-SSE41
        Text 175,310,0x00000000,SSE42, SSE42len-SSE42
        Text 175,330,0x00000000,SSE5, SSE5len-SSE5


        Text 15,70,,name, namelen-name
        Text 15,290,,MMXs, MMXslen-MMXs
        Text 15,310,,SSE, SSElen-SSE
        Text 95,310,,SSE2, SSE2len-SSE2

        Text 95,290,,mmxp, mmxplen-mmxp
        Text 15,330,0x00000000,now, nowlen-now
        Text 95,330,,nowp, nowplen-nowp

        Text 15, 190,0x00000000,cache, cachelen-cache
        Text 15,170,0x00000000,cache2, cache2len-cache2
        Text 15,210,,cache3, cache3len-cache3
        Text 15,230,,cache4, cache4len-cache4
        call    load_gif

        cmp [nomultiplier], $1
        je nomultip
        Text   15,30,0x00000000,multil2, multil2len-multil2
        Text   175,30,0x00000000,freql2, freql2len-freql2
        Number 85,30,0,2,dword [multb],0x000000;
        Number 105,30,0,1,dword [multa]
        Number 259,30,0,4,dword [freqbb]
        Number 289,30,0,2,dword [freqll]

JumpForCodename:
        cmp [codeN], $1
        je codeNIntel
        cmp [codeN], $2
        je codeNAMD
        cmp [codeN], $3
        je codeNCyrix
        cmp [codeN], $4
        je codeNCentaur
        cmp [codeN],$5
        je codeNTransmeta
        cmp [codeN], $6
        je codeNVortex
        jmp nnn

codeNIntel:
        Text 75,70,0x00000000,Inteln, Intelnlen-Inteln
        mov     esi, intel
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

codeNAMD:
        Text 75,70,,AMDn, AMDnlen-AMDn
        mov     esi, amd
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

codeNCyrix:
        Text 75,70,0x00000000,Cyrixn, Cyrixnlen-Cyrixn
        mov     esi, cyrix
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

codeNCentaur:
        Text 75,70,0x00000000,IDTn, IDTnlen-IDTn
        mov     esi, idt
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

codeNTransmeta:
        Text 75,70,,Tranmsmetan, Tranmsmetanlen-Tranmsmetan
        mov     esi, transmeta
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

codeNVortex:
        Text 75,70,0x00000000,Vortexn, Vortexnlen-Vortexn
        mov     esi, vortex
        call    load_gif
        PutImage 135,107,201,49,img_area+8
        jmp nnn

nomultip:
        Text   15,30,0x00000000,multi3, multi3len-multi3
        Text   175,30,0x00000000,freql3, freql3len-freql3
        Text 259,30,0x00000000, clock0, clock0len-clock0

        jmp JumpForCodename

 nnn:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        Text 15,10,0x00000000,stm, stmlen-stm
        ;  Fix for deleting leading whitespaces
        ;  in Intel P4's internal name
        ;  by Madis Calme
        ;  23.12.2004 ver. 0.81
        cld
        mov  edi,myname
        mov  al,20h
        or       ecx,-1
        repe scasb
        dec  edi
        mov  esi,mynamelen
        sub  esi,edi
        Text 105, 10, 0x00000000, edi, esi
;-
        Text 15,250,,brandid, brandidlen-brandid

        mcall SF_REDRAW, SSF_END_DRAW

        ret             ;

call_OpenDialog:

        mov     [OpenDialog_data.type],1            ; Save

        push    dword OpenDialog_data
        call    [OpenDialog_Start]

        cmp     [OpenDialog_data.status],2          ; OpenDialog does not start
        je          .save_file_default_path

        cmp     [OpenDialog_data.status],1
        jne     still

        call    store_data
        jmp     still

.save_file_default_path:
         mov     edi,file_name
         mov     esi,file_default_path
         call    copy_file_name_path
         call    store_data
         jmp     still

copy_file_name_path:
        xor     eax,eax
        cld
@@:
        lodsb
        stosb
        test    eax,eax
        jnz     @r
        ret

;-----------------------------------------------------------------------------
prepare_text_area:
        mov     edi,[store_text_area_start]

        push    edi
        mov     ecx,4096
        mov     eax,dword '    '
        cld
        rep     stosd
        pop     edi
        mov     [store_text_area_end], edi

        mov esi,title
        call addstring

        mov esi, stm
        call addstring

        mov esi, myname
        call addstring

        cmp [nomultiplier], $1
        je noMult
        jne detMulti
         ;jne detFreq

        detMulti:
        mov esi,multil
        call addstring
        jmp detFreq

        detFreq:
        mov esi, freql
        call addstring

        noMult:
        mov esi,multi3
        call addstring

        mov esi,freql3
        call addstring

        mov esi, tech
        call addstring

        mov esi, saveproc
        call addstring

        mov esi, cpuname
        call addstring

        mov esi, fam
        call addstring

        mov esi, mode
        call addstring

        mov esi, step
        call addstring

        mov esi, cache2
        call addstring

        mov esi, cache
        call addstring

        mov esi, cache3
        call addstring

        mov esi, cache4
        call addstring

        mov esi, brandid
        call addstring

        mov esi, MMXs
        call addstring

        mov esi, mmxp
        call addstring

        mov esi, now
        call addstring

        mov esi, HTT
        call addstring

        mov esi, SSE
        call addstring

        mov esi, SSE41
        call addstring

        mov esi, SSE42
        call addstring

        mov esi, SSE5
        call addstring

        mov esi, SSE2
        call addstring

        mov esi, nowp
        call addstring

        mov esi, sse3
        call addstring

        mov esi, standard
        call addstring

        mov esi, STDCA
        call addstring

        mov esi, EXTCA
        call addstring

        mov esi, FPU
        call addstring

        mov esi, VME
        call addstring

        mov esi, DE
        call addstring

        mov esi, PSE
        call addstring

        mov esi, TSC
        call addstring

        mov esi, MSR
        call addstring

        mov esi, PAE
        call addstring

        mov esi, MCE
        call addstring

        mov esi, CX8
        call addstring

        mov esi, APIC
        call addstring

        mov esi, Res
        call addstring

        mov esi, SEP
        call addstring

        mov esi, MTRR
        call addstring

        mov esi, PGE
        call addstring

        mov esi, MCA
        call addstring

        mov esi, CMOV
        call addstring

        mov esi, PAT
        call addstring

        mov esi, PSE36
        call addstring

        mov esi, PSNUM
        call addstring

        mov esi, CLFLUSHn
        call addstring

        mov esi, Res
        call addstring

        mov esi, DTS
        call addstring

        mov esi, ACPI
        call addstring

        mov esi, MMX
        call addstring

        mov esi, FXSR
        call addstring

        mov esi, SSE
        call addstring


        mov esi, SSn
        call addstring

        mov esi, HTT
        call addstring

        mov esi, TM
        call addstring

        mov esi, IA64
        call addstring

        mov esi, PBE
        call addstring

        mov esi, SS3
        call addstring

        mov esi, CNXT_ID
        call addstring

        mov esi, MP
        call addstring

        mov esi, FFXSR
        call addstring

        mov esi, MON
        call addstring

        mov esi, CX16
        call addstring

        mov esi, NX
        call addstring

        mov esi, TSCP
        call addstring

        mov esi, DS_CPL
        call addstring

        mov esi, ETPRD
        call addstring

        mov esi, MMXPi
        call addstring

        mov esi, LM
        call addstring

        mov esi, EST
        call addstring

        mov esi, SYS
        call addstring

        mov esi, MMXn
        call addstring

        mov esi, DNo
        call addstring

        mov esi, TM2
        call addstring

        mov esi, LAF
        call addstring

        mov esi, FFXSR
        call addstring

        mov esi, DN
        call addstring

        mov esi, VMX
        call addstring

        mov esi, SSSE3
        call addstring

        mov esi, DCA
        call addstring

        mov esi, CMPL
        call addstring

        mov esi, SVM
        call addstring

        mov esi, MCR8
        call addstring

        mov esi, SMX
        call addstring

        mov esi, x2APIC
        call addstring

        mov esi, PPCNT
        call addstring

        mov esi, PAGE
        call addstring

        mov esi, EAS
        call addstring

        mov esi, ABM
        call addstring

        mov esi, MIS
        call addstring

        mov  esi,OSVW
        call addstring

        mov esi,SKINIT_
        call addstring

        mov esi, WDT
        call addstring

        ret

addstring:
        mov edi, [store_text_area_end]
        xor eax, eax
        xor ecx, ecx
        cld
@@:
        lodsb
        stosb
        inc     [store_text_area_end]
        cmp     [esi], byte 0x0
        jnz     @r
        mov     al,0Ah
        stosb

        mov     [store_text_area_end],edi
        xor     edi,edi
        xor     esi, esi
        ret

savenumber:
        xor eax, eax
        cld
        lodsw
        call numbertostring
        xor esi, esi
        xor edi, edi

        ret

savestring:
        xor eax, eax
        cld
        lodsw
        ;call numbertostring
        xor esi, esi
        xor edi, edi

        ret

numbertostring:
        mov bx, 10
        xor ecx, ecx
@@m1:
        xor dx, dx
        div bx
        push dx
        inc cx
        test ax, ax
        jnz @@m1
@@m2:
        pop ax
        add al, '0'
        stosb
        loop @@m2
        ret

concatname:
        ;mov edi, [saveproc]
        xor eax, eax
        xor ecx, ecx
        cld
@@:
        lodsb
        stosb
        cmp     [esi], byte 0x0
        jnz     @r

        xor     edi,edi
        xor     esi, esi
        ret

store_data:
        call prepare_text_area
        mov     eax,[store_text_area_start]
        mov     [fileinfo.return],eax
        mov     ebx,[store_text_area_end]
        sub     ebx,eax
        inc     ebx
        mov     [fileinfo.size],ebx
        mcall   70,fileinfo
        ret

load_gif:
        mov     edi, img_area
        jmp     ReadGIF


; DATA AREA

title    db   'CPUID 2.31 by S.Kuzmin & KolibriOS Team',0

stm:
   db 'Internal name:', 0

stmlen:

SS42:
        db 'SSE4.2:       ',0
SS42len:

SMX:
        db 'SMX:       ',0
SMXlen:

x2APIC:
        db 'x2APIC:       '
x2APIClen:

PPCNT:
        db 'POPCNT:          '
PPCNTlen:

PAGE:
        db 'Page1Gb:     '
PAGElen:

EAS:
        db 'EAS:                       ',0
EASlen:

newLabel:
        db '3DNP:                       ',0
newLabellen:


ABM:
        db 'ABM:      '
ABMlen:

SSE4A:
        db 'SSE4A:       '
SSE4Alen:

MIS:
        db 'MIS:      ',0
MISlen:

DNP:
        db '3DNP:       '
DNPlen:

OSVW:
        db 'OSVW:       ',0
OSVWlen:

SKINIT_:
        db 'SKINIT:          ',0
SKINIT_len:

WDT:
        db 'WDT:            ',0
WDTlen:


saveproc:
   db '                                                   ',0

multil:
  db 'Multiplier:   .  ', 0

multillen:

multil2:

  db 'Multiplier:   .  '

multil2len:

multi3:

  db 'Multiplier: n/a',0

multi3len:

freql:

  db 'System clock:     .   MHz', 0
freqllen:

freql2:

  db 'System clock:     .   MHz'


freql2len:

freql3:

  db 'System clock: n/a '


freql3len:

tsum:

        db 'Frequency:     .    MHz  ',0

tsumlen:

tech:

        db 'Technology: 0.   micron ', 0

techlen:

name:

if lang eq it

        db 'Codename:',0

else

        db 'CODENAME:',0


end if

namelen:

vendorname:

if lang eq it

         db 'Vendor CPU              ', 0

else

         db 'CPU VENDOR:             ', 0

end if

vendornamelen:

cpuname:

if lang eq it

         db 'Vendor CPU              ', 0

else

         db 'CPU VENDOR:             ', 0

end if

cpunamelen:


fam:

if lang eq it

         db 'Famiglia:   std   ext', 0

else

         db 'FAMILY:   std   ext', 0

end if

famlen:

mode:

if lang eq it

          db 'Modello:    std    ext', 0

else

          db 'MODEL:    std    ext', 0

end if

modelen:

step:

if lang eq it

           db 'Stepping:        ', 0

else

           db 'STEPPING:        ', 0

end if

steplen:

cache:

        db 'L1(inst):     KB       -way set     -byte line size',0

cachelen:

cache2:

         db 'L1(data):     KB       -way set     -byte line size',0

cache2len:


cache3:

        db 'L2:      KB            -way set     -byte line size',0

cache3len:

cache4:

   db 'L3:      KB            -way set     -byte line size',0

cache4len:

brandid:

        db 'Brand:', 0

brandidlen:

MMXs:

        db 'MMX:         ',0

MMXslen:


total dd 0x0

total1 dd 0x0

rating dd 0x0

rat dd 0x0  ;

NEF:

db 'EXTENDED FEATURES ARE NOT AVAILABLE',0

NEFlen:



mb :

db 'MB'

mblen:



logcpus :

db 'Number of logical CPU:'

logcpuslen:



speed :

if lang eq it

;               db 'Performance',0

else

;               db 'PERFORMANCE:',0

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


STDCA:

        db 'Highest STD call is         ',0

STDCAlen:



EXTCA:

        db 'Highest EXT call is         h',0

EXTCAlen:


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

cacheP4:

        db 'L1(inst):     Kuops    -way set     -byte line size'

cacheP4len:




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




AMDn:

        db 'AMD',0

AMDnlen:

AMDnNew:

        db 'CODENAME: AMD',0

AMDnNewlen:

Inteln:

        db 'Intel',0

Intelnlen:

IntelnNew:
 db 'CODENAME: Intel',0

IntelnNewlen:


Cyrixn:

        db 'Cyrix',0

Cyrixnlen:

IDTn:

         db 'IDT/Centaur',0

IDTnlen:

Centaurn:

         db 'VIA',0

Centaurnlen:



Tranmsmetan:

         db 'Transmeta',0

Tranmsmetanlen:


Vortexn:
        db 'Vortex86',0
Vortexnlen:


mmxp:

        db 'MMX+:         ',0

mmxplen:



HTT:

        db 'HTT:          ',0

HTTlen:


HTTn:

        db 'HTT:         ',0

HTTnlen:


sse3:

        db 'SSE3:         ',0

sse3len:

SSE41:
        db 'SSE4.1:          ',0
SSE41len:

SSE42:
        db 'SSE4.2:             ',0
SSE42len:

SSE5:
        db 'SSE5:               ',0
SSE5len:

now:

        db '3DNOW!:         ',0

nowlen:

nowp:

        db '3DNOW!+:         ',0

nowplen:

;-Type

t1      db      'OEM',0

t2      db      'Overdrive',0

t3      db      'Dual',0

t4      db      'Unknown',0



;----------Intel

P50:

db 'P5 A-step',0

P50len:

P5:

db 'P5',0

P5len:

P54T:

db 'P24T Overdrive',0

P54Tlen:

P54C:

db 'P54C',0

P54Clen:

P55C:

db 'P55C (with MMX)',0

P55Clen:

; ---

P60:

db 'Pentium Pro A-step',0

P60len:

P61:

db 'Pentium Pro',0

P61len:

P63:

db 'Pentium II (Klamath)',0

P63len:

P65:

db 'Pentium II (Deschutes)',0

P65len:

P66:

db 'Celeron (Medocino)',0

P66len:

P67:

db 'Pentium III (Katmai)',0

P67len:

P68:

db 'Pentium III (Coppermine)',0

P68len:

P69:

db 'Pentium M (Banias)',0

P69len:

P6A:

db 'Pentium III Xeon (Cascades)',0

P6Alen:

P6B:

db 'Pentium III (Tualatin)',0

P6Blen:

P6D:

db 'Pentium M (Dothan)',0

P6Dlen:

P6E:

db 'Pentium M (Yonah)/ Core',0

P6Elen:

P6F:

db 'Pentium D (Conroe)/ Core 2 (Kentsfield)',0

P6Flen:

;---

PS0:

db 'Itanium (IA-64)',0

PS0len:

;------------

PF0:

db 'Pentium 4 (Willamete)',0

PF0len:

PF2:

db 'Pentium 4 (Northwood)',0

PF2len:

PF3:

db 'Pentium 4 (Prescott)',0

PF3len:

PF5:

db 'Pentium 4 (Tejas)',0

PF5len:

PF6:

db 'Pentium 4 (Presler)',0

PF6len:

;----------------Intel Celerons

P65c:

db 'Celeron (Covington)',0

P65clen:

P68c:

db 'Celeron (Coppermine)',0

P68clen:

P6Bc:

db 'Celeron (Tualatin)',0

P6Bclen:

PF0c:

db 'Celeron (Willamete)',0

PF0clen:

PF2c:

db 'Celeron (Northwood)',0

PF2clen:

PF3c:

db 'Celeron (Prescott)',0

PF3clen:

PF5c:

db 'Celeron D (Texas)',0

PF5clen:

PF6c:

db 'Celeron D (Presler)',0

PF6clen:

;---------New Intel
P3A:

db 'IvyBridge',0

P3Alen:

P2A:

db 'Sandy Bridge',0

P2Alen:

P2D:
db 'Sandy bridge-E',0

P2Dlen:


P25:
db 'Arrandale',0
P25len:

P2C:
db 'Gulftown',0
P2Clen:

P2F:
db 'Westmere-EX',0
P2Flen:

P1E:
db 'Clarksfield',0
P1Elen:

P1A:
db 'Bloomfield',0
P1Alen:

P2E:
db 'Nehalem-EX',0
P2Elen:

P17:
db 'Yorkfield',0
P17len:

P1D:
db 'Dunnington',0
P1Dlen:

P0F:
db 'Clovertown',0
P0Flen:

P16:
db 'Merom Conroe',0
P16len:

P06:
db 'Cedar Mill',0
P06len:

P03:
db 'Nocona Irwindale',0
P03len:

P04:
db 'NoconaIrwindale',0
P04len:

P0D:
db 'Dothan',0
P0Dlen:

P36:
db 'Cedarview',0
P36len:

P26:
db 'Lincroft',0
P26len:

P1C:
db 'Pineview',0
P1Clen:

no_known:
db 'SORRY, CODENAME IS NOT SUPPORTED YET ', 0

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
AthlonKuma:

db 'AMD Athlon 7750 Black Edition',0

AthlonKumalen:

AB23:

db 'Opteron 2300-series',0

AB23len:

AB83:

db 'Opteron 8300-series',0

AB83len:

AB9:
db 'Phenom X4',0

AB9len:

AB8right2:

db 'Phenom X3',0

AB8right2len:

AB4:

db 'Athlon X2',0

AB4len:

AB1:

db 'Sempron',0

AB1len:

ABC2:

db 'Opteron (Shanghai) 2387',0

ABC2len:

AB6:

db 'Opteron (Magny-Cours)',0

AB6len:

ABC3:

db 'Opteron (Shanghai) 8300 series',0

ABC3len:

ABM2:

db 'Turion II ',0

ABM2len:

;---

AF1:

db 'Dual-core Opteron',0

AF1len:

AF3:

db 'Athlon 64 (Toledo)',0

AF3len:

AF4:

db 'Athlon 64 (ClawHammer)',0

AF4len:

AF5:

db 'Opteron/Athlon 64 FX (SledgeHammer)',0

AF5len:


AFC:

db 'Athlon 64 (Newcastle)',0

AFClen:


AFF:

db 'Athlon 64 (Winchester)',0

AFFlen:


AFS:

db 'Athlon 64 (San Diego)',0

AFSlen:


AFV:

db 'Athlon 64 (Venice)',0

AFVlen:


AFCs:

db 'Sempron (Paris)',0

AFCslen:



AFCsp:

db 'Sempron (Palermo)',0

AFCsplen:



;---------Cyrix

C52:

db '6x86 M1',0

C52len:

C54:

db 'MediaGX',0

C54len:

C60:

db '6x86MX M2',0

C60len:

C65:

db 'C3 (Cyrix M2)',0 ;?

C65len:

;--------IDT

V54:

db 'WinChip C6',0

V54len:

V58:

db 'WinChip 2',0

V58len:

V59:

db 'WinChip 3',0

V59len:

;-------VIA

V66:

db 'C3 (Samuel)',0  ; Joshua is unreleased 065

V66len:

V67:

db 'C3 (Samuel2/Ezra)',0 ; ?

V67len:

V68:

db 'C3 (Ezra-T/Eden)',0 ;?

V68len:

V69:

db 'C3 (Antaur/Nehemiah)',0 ;?

V69len:

VA:

db 'C7 (Esther)',0 ;?

VAlen:

;---------Transmeta

T5:

db 'Crusoe',0 ;

T5len:

TF:

db 'Efficeon',0 ;

TFlen:

;---------

NG:

        db 'Next generation CPU',0

NGlen:


athloncoef      db      110, 115, 120, 125, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 120

                db      190, 120, 200, 130, 135, 140, 210, 150, 220, 160, 165, 170, 180, 230, 240

athlonmcoef:    db      110, 115, 120, 125, 50, 55, 60, 65,  70, 75, 80, 85, 90, 95, 100, 105

                db      30, 190, 40, 200, 130, 135, 14, 210, 150, 220, 160, 165, 170, 230, 240

athloncoef3     db      45, 50, 40, 55, 25, 30, 60, 35

p4coef          db      160, 170, 180, 190, 200, 210, 220, 230, 80, 90, 100, 110, 120, 130, 140, 150    ; Pentium 4 (Willamete)

coppercoeff     db       50, 30, 40, 20, 55, 35,  45, 25,  35, 70, 80, 60, 20, 75, 15, 65, 90, 110, 120, 20, 95, 115, 85, 25, 35, 70,  80, 100,  20, 75,  15, 105

tualatcoeff     db      120, 35, 35, 40, 55, 35, 115, 35, 160, 70, 80, 60, 40, 75, 35, 65, 90, 110,  35, 35, 95,  35, 85, 35, 35, 35, 130, 100, 140, 35, 150, 105

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
I_END:

img_area:          ; image is going to be unpacked to here
rb 201*49*3+8      ; image resolution (bits to reserve)

img_area2:         ; image is going to be unpacked to here
rb 93*24*3+8       ; image resolution (bits to reserve)

img_area3:         ; image is going to be unpacked to here
rb 93*24*3+8       ; image resolution (bits to reserve)

gif_hash_area:
rd 4096+1          ;hash area size for unpacking gif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

system_dir_ProcLib      db '/sys/lib/proc_lib.obj',0

err_message_found_lib2  db 'proc_lib.obj - Not found!',0

err_message_import2     db 'proc_lib.obj - Wrong import!',0

head_f_i:
head_f_l        db 'error',0
;---------------------------------------------------------------------
l_libs_start:

library02  l_libs system_dir_ProcLib+9, path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init         dd aOpenDialog_Init
OpenDialog_Start        dd aOpenDialog_Start
;OpenDialog__Version    dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init        db 'OpenDialog_init',0
aOpenDialog_Start       db 'OpenDialog_start',0
;aOpenDialog_Version    db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type                   dd 0
.procinfo               dd Proc_Info    ;+4
.com_area_name          dd communication_area_name      ;+8
.com_area               dd 0    ;+12
.opendir_pach           dd temp_dir_pach        ;+16
.dir_default_pach       dd communication_area_default_pach      ;+20
.start_path             dd open_dialog_path     ;+24
.draw_window            dd draw_window  ;+28
.status                 dd 0    ;+32
.openfile_pach          dd file_name    ;+36
.filename_area          dd filename_area        ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10 ;+54 ; Window Y position

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
open_dialog_path:
        db '/sys/File Managers/opendial',0
communication_area_default_pach:
        db '/sys',0
Filter:
dd      Filter.end - Filter.1
.1:
db      'TXT',0
db      'LOG',0
.end:
dd      0

file_default_path:
        db '/sys/'
start_temp_file_name:
        db 'CPUID.txt',0
btn_more_cap:
        db 'Details', 0
btn_save_cap:
        db 'Save', 0
;---------------------------------------------------------------------
align   4
fileinfo:
.subfunction    dd 2
.Offset         dd 0
.Offset_1       dd 0
.size           dd 4096
.return         dd 0
                db 0
.name:          dd file_name

align 4
sc system_colors

store_text_area_start   dd ?
store_text_area_end     dd ?
store_text_size         dd ?

;---------------------------------------------------------------------
library_path:
        rb 4096
;---------------------------------------------------------------------
path:
        rb 4096
;---------------------------------------------------------------------
temp_dir_pach:
        rb 4096
;---------------------------------------------------------------------
file_name:
        rb 4096
;---------------------------------------------------------------------
file_name_1:
        rb 4096
;---------------------------------------------------------------------
filename_area:
        rb 256
;---------------------------------------------------------------------
        rb 4096
stacktop:
;---------------------------------------------------------------------
Proc_Info       process_information
; RSA test data
align 4
  num1 rd 40
  num2 rd 40
  num3 rd 40
  iter rd 1
  openkey rd 1


IncludeUGlobals

nocpuid db ?
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
brand  dd ?
newpc  dd ?
nomultiplier dd ?
ram_size_a dd ?
ram_size_t dd ?

stdc dd ?
extc dd ?

FRS dd ?
freqsel db ?
sse3sup db ?
sse41sup dd ?
sse42sup dd ?
sse5sup dd ?

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
che db ? ; numbers of calls for Intel caches detection
cname dd ?
codeN dd ?

myname:
   rb 48

mynamelen:
   db ?

align 4
  thread2_stack_area rb 64
  thread2_esp = $
U_END:
