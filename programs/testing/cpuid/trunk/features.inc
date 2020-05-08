;Addresses for bits
; edx cpuid(1)
FPU_FLAG equ 0001h    ;Floating-point unit on-Chip                                             
VME_FLAG equ 0002h    ;Virtual Mode Extension
DE_FLAG equ 0004h     ;Debugging Extension
PSE_FLAG equ 0008h    ;Page Size Extension
TSC_FLAG equ 0010h    ;Time-Stamp Counter
MSR_FLAG equ 0020h    ;Model Specific Registers
PAE_FLAG equ 0040h    ;Physical Address Extension
MCE_FLAG equ 0080h    ;Machine Check Exception
CX8_FLAG equ 0100h    ;CMPXCHG8 Instruction Supported
APIC_FLAG equ 0200h   ;On-chip APIC Hardware Supported
;10_FLAG equ 0400h   ;Reserved
SEP_FLAG equ 0800h	 ;Fast System Call
MTRR_FLAG equ 1000h	 ;Memory Type Range Registers
PGE_FLAG equ 2000h	 ;Page Global Enable
MCA_FLAG equ 4000h	 ;Machine Check Architecture
CMOV_FLAG equ 8000h	 ;Conditional Move Instruction Supported
PAT_FLAG equ 10000h	 ;Page Attribute Table
PSE36_FLAG equ 20000h	 ;36-bit Page Size Extension
PSNUM_FLAG equ 40000h	 ;Processor serial number present and enabled
CLFLUSH_FLAG equ 80000h  ;CLFLUSH Instruction supported
;20_FLAG equ 100000h  ;Reserved
DTS_FLAG equ 200000h	 ;Debug Store
ACPI_FLAG equ 400000h	 ;Thermal Monitor and Software Controlled Clock Facilities supported
MMX_FLAG equ 800000h	 ;Intel Architecture MMX technology supported
FXSR_FLAG equ 1000000h	 ;Fast floating point save and restore
SSE_FLAG equ 2000000h	 ;Streaming SIMD Extensions supported
SSE2_FLAG equ 4000000h	 ;Streaming SIMD Extensions 2
SS_FLAG equ 8000000h	 ;Self-Snoop
;HTT_FLAG equ 10000000h   ;Hyper-Threading Technology
TM_FLAG equ 20000000h	 ;Thermal Monitor supported
IA64_FLAG equ 40000000h  ; IA-64  
PBE_FLAG equ 80000000h	  ;Pending Break Enable

; ecx cpuid(1)
SSE3_FLAG equ 0001h	  ;SSE3 0
MON_FLAG equ 0008h	  ;MON -3
DS_CPL_FLAG equ 0010h	  ;DS-CPL -4
VMX_FLAG equ 0020h	  ;VMX -5    
SMX_FLAG equ 0040h	  ;SMX -6                  to  add
EST_FLAG equ 0080h	  ; EST-7
TM2_FLAG equ 0100h	  ; TM2-8
SSSE3_FLAG equ 0200h	  ;SSSE3 -9  
CNXT_ID_FLAG equ 0400h	  ;CID -10
CX16_FLAG equ 2000h	  ;CX16 - 13
ETPRD_FLAG equ 4000h	  ;xTPR - 14
PDCM_FLAG equ 8000h	  ;PDCM - 15                to  add
DCA_FLAG equ 40000h	  ;DCA - 18   Direct Cache Access
SSE41_FLAG equ 80000h	  ;SSE41 - 19               to  add
SSE42_FLAG equ 100000h	  ;SSE42 - 20               to  add
x2APIC_FLAG equ 200000h	  ;x2APIC - 21               to  add
PPCNT_FLAG equ 800000h	  ;PPCNT - 23              to  add

; edx cpuid(8000_0001h)
SYS_FLAG equ 0800h	;11       - may be we already detect it as SEP_FLAG
MP_FLAG equ 80000h	;19       ???? not found in recent guides from AMD and Intel
NX_FLAG equ 100000h	;20
MMXPi_FLAG equ 400000h	;22
MMXn_FLAG equ 800000h	;23
FXSRn_FLAG equ 1000000h ;24   it is ллу+ for Cyrix
FFXSR_FLAG equ 2000000h ;25
PAGE_FLAG equ 4000000h  ;26 Page1Gb                to  add
TSCP_FLAG equ 8000000h	;27  
LM_FLAG equ 20000000h	;29  EM64T
DNo_FLAG equ 40000000h	;30   3dNow! Ext
DN_FLAG equ 80000000h	;31   3dNow!


; ecx cpuid(8000_0001h)
LAHF_FLAG equ 0001h  ; LAHF   0
CMPL_FLAG equ 0002h    ; core multi-processing legacy mode 1
SVM_FLAG equ 0004h     ; secure virtual machine 2
EAS_FLAG equ 0008h     ; ExtApicSpace  3         to  add
MOVCR8_FLAG equ 0010h  ; AltMovCr8  4
ABM_FLAG equ 0020h     ; ABM -5                  to  add
SSE4A_FLAG equ 0040h   ; SSE4A -6                to  add
MIS_FLAG equ 0080h     ; MissAlignSSE -7         to  add
3DNP_FLAG equ 0100h    ; 3DNowPrefetch-8         to  add
OSVW_FLAG equ 0200h    ; OS visible workaround -9    to  add
SSE5_FLAG equ 0800h     ;SSE5 - 11               to  add
SKINIT_FLAG equ 1000h  ;SKINIT- 12               to  add
WDT_FLAG equ 2000h     ;Watchdog timer - 13      to  add


;decoding standard features

decode_standard_features:

 xor eax,eax
 inc eax
 cpuid


Test0:
test edx, FPU_FLAG
jnz Test0e

mov dword  [FPU+6], $6F6E
jmp Test1

Test0e:
mov  dword [FPU+6], $736579

;;;;;;
Test1:
test edx, VME_FLAG
jnz Test1e

mov   dword [VME+ 7], $6F6E
jmp Test2

Test1e:
mov   dword [VME+ 7], $736579

;;;;;;
Test2:
test edx, DE_FLAG
jnz Test2e

mov   dword [DE+ 7], $6F6E
jmp Test3

Test2e:
mov   dword [DE+ 7], $736579
;;;;;;

Test3:
test edx, PSE_FLAG
jnz Test3e

mov   dword [PSE+ 8], $6F6E
jmp Test4

Test3e:
mov   dword [PSE+ 8], $736579

;;;;
Test4:
test edx, TSC_FLAG
jnz Test4e

mov   dword [TSC+ 6], $6F6E
jmp Test5

Test4e:
mov   dword [TSC+ 6], $736579

;;;;
Test5:
test edx, MSR_FLAG
jnz Test5e

mov   dword [MSR+ 7], $6F6E
jmp Test6

Test5e:
mov   dword [MSR+ 7], $736579

;;;;
Test6:
test edx, PAE_FLAG
jnz Test6e

mov   dword [PAE+ 7], $6F6E
jmp Test7

Test6e:
mov   dword [PAE+ 7], $736579

;;;;
Test7:
test edx, MCE_FLAG
jnz Test7e

mov   dword [MCE+ 8], $6F6E
jmp Test8

Test7e:
mov   dword [MCE+ 8], $736579

;;;;
Test8:
test edx, CX8_FLAG
jnz Test8e

mov   dword [CX8+ 6], $6F6E
jmp Test9

Test8e:
mov   dword [CX8+ 6], $736579
;;;;

Test9:
test edx, APIC_FLAG
jnz Test9e

mov   dword [APIC+ 7], $6F6E
jmp Test11

Test9e:
mov   dword [APIC+ 7], $736579
;;;;;

Test11:
test edx, SEP_FLAG
jnz Test11e

mov   dword [SEP+ 8], $6F6E
jmp Test12

Test11e:
mov   dword [SEP+ 8], $736579
;;;;

Test12:
test edx, MTRR_FLAG
jnz Test12e

mov   dword [MTRR+ 6], $6F6E
jmp Test13

Test12e:
mov   dword [MTRR+ 6], $736579
;;;;

Test13:
test edx, PGE_FLAG
jnz Test13e

mov   dword [PGE+ 7], $6F6E
jmp Test14

Test13e:
mov   dword [PGE+ 7], $736579
;;;;;

Test14:
test edx, MCA_FLAG
jnz Test14e

mov   dword [MCA+ 7], $6F6E
jmp Test15

Test14e:
mov   dword [MCA+ 7], $736579
;;;;

Test15:
test edx, CMOV_FLAG
jnz Test15e

mov   dword [CMOV+ 8], $6F6E
jmp Test16

Test15e:
mov   dword [CMOV+ 8], $736579
;;;;

Test16:
test edx, PAT_FLAG
jnz Test16e

mov   dword [PAT+ 6], $6F6E
jmp Test17

Test16e:
mov   dword [PAT+ 6], $736579
;;;;

Test17:
test edx, PSE36_FLAG
jnz Test17e

mov   dword [PSE36+ 7], $6F6E
jmp Test18

Test17e:
mov   dword [PSE36+ 7], $736579
;;;;

Test18:
test edx, PSNUM_FLAG
jnz Test18e

mov   dword [PSNUM+ 7], $6F6E
jmp Test19

Test18e:
mov   dword [PSNUM+ 7], $736579
;;;;

Test19:
test edx, CLFLUSH_FLAG
jnz Test19e

mov   dword [CLFLUSHn + 8], $6F6E
jmp Test21

Test19e:
mov   dword [CLFLUSHn + 8], $736579
;;;;

Test21:
test edx, DTS_FLAG
jnz Test21e

mov   dword [DTS+ 7], $6F6E
jmp Test22

Test21e:
mov   dword [DTS+ 7], $736579
;;;;

Test22:
test edx, ACPI_FLAG
jnz Test22e

mov   dword [ACPI+ 7], $6F6E
jmp Test23

Test22e:
mov   dword [ACPI+ 7], $736579
;;;;;

Test23:
test edx, MMX_FLAG
jnz Test23e

mov   dword [MMX+ 8], $6F6E
mov   dword [MMXs+ 7], $6F6E
jmp Test24

Test23e:
mov   dword [MMX+ 8], $736579
mov   dword [MMXs+ 7], $736579
;;;;;

Test24:
test edx, FXSR_FLAG
jnz Test24e

mov   dword [FXSR+ 6], $6F6E
jmp Test25

Test24e:
mov   dword [FXSR+ 6], $736579
;;;;;

Test25:
test edx, SSE_FLAG
jnz Test25e

mov   dword [SSE+ 7], $6F6E
jmp Test26

Test25e:
mov   dword [SSE+ 7], $736579

;;;;
Test26:
test edx, SSE2_FLAG
jnz Test26e

mov   dword [SSE2+ 7], $6F6E
jmp Test27

Test26e:
mov   dword [SSE2+ 7], $736579

;;;;

Test27:
test edx, SS_FLAG
jnz Test27e

mov   dword [SSn+ 8], $6F6E
jmp Test29;28

Test27e:
mov   dword [SSn+ 8], $736579

;;;;

;Test28:
;test edx, HTT_FLAG
;jnz Test28e
;
;mov   dword [HTT+ 8], $6F6E
;jmp Test29
;
;Test28e:
;mov   dword [HTT+ 8], $736579

;;;;

Test29:
test edx, TM_FLAG
jnz Test29e

mov   dword [TM+ 7], $6F6E
jmp Test30

Test29e:
mov   dword [TM+ 7], $736579

;;;;

Test30:
test edx, IA64_FLAG
jnz Test30e

mov   dword [IA64+ 7], $6F6E
jmp Test31

Test30e:
mov   dword [IA64+ 7], $736579

;;;;
Test31:
test edx, PBE_FLAG
jnz Test31e

mov   dword [PBE+ 8], $6F6E
jmp Standart_out

Test31e:
mov   dword [PBE+ 8], $736579

Standart_out:

ret
;//////////////////////////////////////////////
decode_sse3_5:   ; is SS3-5 supported
      xor eax,eax
      inc eax
      cpuid
  test	ecx, $1 ; Test bit 1
  jnz	.EX; SSE3 technology is  supported
  jz .EXN

.EX:
   mov dword  [sse3+ 6],  $736579
   mov [sse3sup], 1
   jmp exitter
.EXN:
    mov dword [sse3+ 6], $6F6E
    mov [sse3sup],0

exitter:


  test	ecx, $80000 ; Test bit 19
  jnz	.EX1; SSE41 technology is  supported
  jz .EXN1

.EX1:
   mov dword  [SSE41+ 11],  $736579
   mov [sse41sup], 1
   jmp exitter1
.EXN1:
    mov dword [SSE41+ 11], $6F6E
    mov [sse41sup],0

exitter1:


  test	ecx, $100000 ; Test bit 20
  jnz	.EX2; SSE42 technology is  supported
  jz .EXN2

.EX2:
   mov dword  [SSE42+ 10],  $736579
   mov dword  [SS42+ 11],  $736579
   mov [sse42sup], 1
   jmp exitter2
.EXN2:
   mov dword [SSE42+ 10], $6F6E
   mov dword [SS42+ 11], $6F6E
   mov [sse42sup],0

exitter2:


      xor eax,eax
      mov eax, 80000001h
      cpuid
  test	ecx, $800 ; Test bit 11
  jnz	.EX3; SS5 technology is  supported
  jz .EXN3

.EX3:
   mov dword  [SSE5+ 12],  $736579
   mov [sse5sup], 1
   jmp exitter3
.EXN3:
    mov dword [SSE5+ 12], $6F6E
    mov [sse5sup],0

exitter3: 

ret


decode_extended_features:
      xor eax,eax
      inc eax
      cpuid
Tes1:
test ecx, SSE3_FLAG
jnz Tes1e

mov   dword [SS3+ 8], $6F6E
jmp Tes2

Tes1e:
mov   dword [SS3+ 8], $736579


Tes2:
test ecx, MON_FLAG
jnz Tes2e

mov   dword [MON+ 8], $6F6E
jmp Tes3

Tes2e:
mov   dword [MON+ 8], $736579


Tes3:
test ecx, DS_CPL_FLAG
jnz Tes3e

mov   dword [DS_CPL+ 8], $6F6E
jmp Tes4

Tes3e:
mov   dword [DS_CPL+ 8], $736579

Tes4:
test ecx, EST_FLAG
jnz Tes4e

mov   dword [EST+ 8], $6F6E
jmp Tes5

Tes4e:
mov   dword [EST+ 8], $736579


Tes5:
test ecx, TM2_FLAG
jnz Tes5e

mov   dword [TM2+ 8], $6F6E
jmp Tes6

Tes5e:
mov   dword [TM2+ 8], $736579


Tes6:
test ecx, CNXT_ID_FLAG
jnz Tes6e

mov   dword [CNXT_ID+ 12], $6F6E
jmp Tes7

Tes6e:
mov   dword [CNXT_ID+ 12], $736579


Tes7:
test ecx, CX16_FLAG
jnz Tes7e

mov   dword [CX16+ 12], $6F6E
jmp Tes8

Tes7e:
mov   dword [CX16+ 12], $736579


Tes8:
test ecx, ETPRD_FLAG
jnz Tes8e

mov   dword [ETPRD+ 12], $6F6E
jmp Tes9

Tes8e:
mov   dword [ETPRD+ 12], $736579

Tes9:
test ecx, VMX_FLAG
jnz Tes9e

mov   dword [VMX+ 8], $6F6E
jmp Tes10

Tes9e:
mov   dword [VMX+ 8], $736579

Tes10:
test ecx, SSSE3_FLAG
jnz Tes10e

mov   dword [SSSE3+ 12], $6F6E
jmp Tes11

Tes10e:
mov   dword [SSSE3+ 12], $736579

Tes11:
test ecx, DCA_FLAG
jnz Tes11e

mov   dword [DCA+ 8], $6F6E
jmp Tes12

Tes11e:
mov   dword [DCA+ 8], $736579

Tes12:
test ecx, SMX_FLAG
jnz Tes12e

mov   dword [SMX+ 8], $6F6E
jmp Tes13

Tes12e:
mov   dword [SMX+ 8], $736579

Tes13:
test ecx, PDCM_FLAG
jnz Tes13e

mov   dword [newLabel+ 8], $6F6E
jmp Tes14

Tes13e:
mov   dword [newLabel+ 8], $736579

Tes14:
test ecx, x2APIC_FLAG
jnz Tes14e

mov   dword [x2APIC+ 8], $6F6E
jmp Tes15

Tes14e:
mov   dword [x2APIC+ 8], $736579

Tes15:
test ecx, PPCNT_FLAG
jnz Tes15e

mov   dword [PPCNT+ 12], $6F6E
jmp Tes16

Tes15e:
mov   dword [PPCNT+ 12], $736579

Tes16:

ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
decode_extended:
    xor   eax, eax
    mov   eax,$80000000
    cpuid

test eax, 80000000h
jnz gooodd

jmp baaad

baaad:

jmp Tez13

gooodd:
  xor eax, eax
  mov	eax, $80000001 ;// Setup extended function 8000_0001h
  cpuid

Tez1:
test edx, MP_FLAG
jnz Tez1e

mov   dword [MP+8], $6F6E
jmp Tez2

Tez1e:
mov   dword [MP+ 8], $736579

Tez2:
test edx, NX_FLAG
jnz Tez2e

mov   dword [NX+ 8], $6F6E
jmp Tez4
;jmp Tez3 we do detection in another place, because of Cyrix specific MMX+ detection 

Tez2e:
mov   dword [NX+ 8], $736579

;Tez3:
;test edx, MMXPi_FLAG
;jnz Tez3e

;mov   dword [MMXPi+ 8], $6F6E
;jmp Tez4


;Tez3e:
;mov   dword [MMXPi+ 8], $736579

Tez4:
test edx, MMXn_FLAG
jnz Tez4e

mov   dword [MMXn+ 8], $6F6E
jmp Tez5

Tez4e:
mov   dword [MMXn+ 8], $736579

Tez5:
test edx, FXSRn_FLAG
jnz Tez5e

mov   dword [FXSRn+ 8], $6F6E
jmp Tez6

Tez5e:
mov   dword [FXSRn+ 8], $736579

Tez6:
test edx, FFXSR_FLAG
jnz Tez6e

mov   dword [FFXSR+ 12], $6F6E
jmp Tez7

Tez6e:
mov   dword [FFXSR+ 12], $736579

Tez7:
test edx, TSCP_FLAG
jnz Tez7e

mov   dword [TSCP+ 12], $6F6E
jmp Tez8

Tez7e:
mov   dword [TSCP+ 12], $736579


Tez8:
test edx, LM_FLAG
jnz Tez8e

mov   dword [LM+ 12], $6F6E
jmp Tez9

Tez8e:
mov   dword [LM+ 12], $736579

Tez9:
test edx, DNo_FLAG
jnz Tez9e

mov   dword [DNo+ 12], $6F6E
jmp Tez10

Tez9e:
mov   dword [DNo+ 12], $736579


Tez10:
test edx, DN_FLAG
jnz Tez10e

mov   dword [DN+ 12], $6F6E
jmp Tez11

Tez10e:
mov   dword [DN+ 12], $736579

;Intel

Tez11:
test edx, SYS_FLAG
jnz Tez11e

mov   dword [SYS+ 12], $6F6E
jmp Tez12

Tez11e:
mov   dword [SYS+ 12], $736579


Tez12:
test ecx, LAHF_FLAG
jnz Tez12e

mov   dword [LAF+ 12], $6F6E
jmp Tez13

Tez12e:
mov   dword [LAF+ 12], $736579

Tez13:
test ecx, CMPL_FLAG
jnz Tez13e

mov   dword [CMPL+ 8], $6F6E
jmp Tez14

Tez13e:
mov   dword [CMPL+ 8], $736579

Tez14:
test ecx, SVM_FLAG
jnz Tez14e

mov   dword [SVM+ 8], $6F6E
jmp Tez15

Tez14e:
mov   dword [SVM+ 8], $736579

Tez15:
test ecx, MOVCR8_FLAG
jnz Tez15e

mov   dword [MCR8+ 12], $6F6E
jmp Tez16

Tez15e:
mov   dword [MCR8+ 12], $736579

Tez16:
test edx, PAGE_FLAG
jnz Tez16e

mov   dword [PAGE+ 8], $6F6E
jmp Tez17

Tez16e:
mov   dword [PAGE+ 8], $736579

Tez17:
test ecx, EAS_FLAG
jnz Tez17e

mov   dword [EAS+ 12], $6F6E
jmp Tez18

Tez17e:
mov   dword [EAS+ 12], $736579

Tez18:
test ecx, ABM_FLAG
jnz Tez18e

mov   dword [ABM+ 8], $6F6E
jmp Tez19

Tez18e:
mov   dword [ABM+ 8], $736579

Tez19:
test ecx, SSE4A_FLAG
jnz Tez19e

mov   dword [SSE4A+ 8], $6F6E
jmp Tez20

Tez19e:
mov   dword [SSE4A+ 8], $736579

Tez20:
test ecx, MIS_FLAG
jnz Tez20e

mov   dword [MIS+ 8], $6F6E
jmp Tez21

Tez20e:
mov   dword [MIS+ 8], $736579

Tez21:
test ecx, 3DNP_FLAG
jnz Tez21e

;mov   dword [DNP+ 8], $6F6E
;jmp Tez22

Tez21e:
;mov   dword [DNP+ 8], $736579

Tez22:
test ecx, OSVW_FLAG
jnz Tez22e

mov   dword [OSVW+ 8], $6F6E
jmp Tez23

Tez22e:
mov   dword [OSVW+ 8], $736579

Tez23:
test ecx, SKINIT_FLAG
jnz Tez23e

mov   dword [SKINIT_+ 12], $6F6E
jmp Tez24

Tez23e:
mov   dword [SKINIT_+ 12], $736579

Tez24:
test ecx, WDT_FLAG
jnz Tez24e

mov   dword [WDT+ 12], $6F6E
jmp Tez25

Tez24e:
mov   dword [WDT+ 12], $736579

Tez25:

ret

;//////////////////////////////////////////////
decode_sse3:   ; is SS3 supported
        xor     eax, eax
        inc     eax
        cpuid
        test    ecx, 1
        setnz   [sse3sup]
        mov     eax, sse3+6

write_yes_no:
        mov     dword [eax], 'no'
        jz      @f
        mov     dword [eax], 'yes'
@@:
        ret

show_next_bit:
        shr     edx, 1
write_yes_no_cf:
        mov     dword [eax], 'no'
        jnc     @f
        mov     dword [eax], 'yes'
@@:
        ret
