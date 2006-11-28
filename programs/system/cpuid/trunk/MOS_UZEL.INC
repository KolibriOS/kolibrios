;Addresses for bits

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
SEP_FLAG equ 0800h       ;Fast System Call
MTRR_FLAG equ 1000h      ;Memory Type Range Registers
PGE_FLAG equ 2000h       ;Page Global Enable
MCA_FLAG equ 4000h       ;Machine Check Architecture
CMOV_FLAG equ 8000h      ;Conditional Move Instruction Supported
PAT_FLAG equ 10000h      ;Page Attribute Table
PSE36_FLAG equ 20000h    ;36-bit Page Size Extension
PSNUM_FLAG equ 40000h    ;Processor serial number present and enabled
CLFLUSH_FLAG equ 80000h  ;CLFLUSH Instruction supported
;20_FLAG equ 100000h  ;Reserved
DTS_FLAG equ 200000h     ;Debug Store
ACPI_FLAG equ 400000h    ;Thermal Monitor and Software Controlled Clock Facilities supported
MMX_FLAG equ 800000h     ;Intel Architecture MMX technology supported
FXSR_FLAG equ 1000000h   ;Fast floating point save and restore
SSE_FLAG equ 2000000h    ;Streaming SIMD Extensions supported
SSE2_FLAG equ 4000000h   ;Streaming SIMD Extensions 2
SS_FLAG equ 8000000h     ;Self-Snoop
;HTT_FLAG equ 10000000h   ;Hyper-Threading Technology
TM_FLAG equ 20000000h    ;Thermal Monitor supported
IA64_FLAG equ 40000000h  ; IA-64
PBE_FLAG equ 80000000h    ;Pending Break Enable

; ecx cpuid(1)
SSE3_FLAG equ 0001h       ;SSE3 0
MON_FLAG equ 0008h        ;MON -3
DS_CPL_FLAG equ 0010h         ;DS-CPL -4
EST_FLAG equ 0080h    ; EST-7
TM2_FLAG equ 0100h       ; TM2-8
CNXT_ID_FLAG equ 0400h    ;CID -10
CX16_FLAG equ 2000h       ;CX16 - 13
ETPRD_FLAG equ 4000h      ;xTPR - 14

; edx cpuid(8000_0001h)

SYS_FLAG equ 0800h       ;11

MP_FLAG equ 80000h       ;19
NX_FLAG equ 100000h      ;20
MMXPi_FLAG equ 400000h    ;22
MMXn_FLAG equ 800000h     ;23
FXSRn_FLAG equ 1000000h   ;24   для Сирикс это ММХ+
FFXSR_FLAG equ 2000000h  ;25
TSCP_FLAG equ 8000000h   ;27
LM_FLAG equ 20000000h    ;29
DNo_FLAG equ 40000000h  ;30
DN_FLAG equ 80000000h   ;31

; ecx cpuid(8000_0001h)
LAHF_FLAG equ 100000h  ;Reserved

; optimize the code for size
macro add arg1,arg2
 {
   if arg1 in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
      if arg2 eqtype 0
         if arg2 = 1
            inc arg1
         else
            add arg1,arg2
         end if
      else
         add arg1,arg2
      end if
   else
      add arg1,arg2
   end if
 }

macro sub arg1,arg2
 {
   if arg2 eqtype 0
      if arg2 = 1
         dec arg1
      else
         sub arg1,arg2
      end if
   else
      sub arg1,arg2
   end if
 }

macro mov arg1,arg2
 {
   if arg1 in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
      if arg2 eqtype 0
         if arg2 = 0
            xor arg1,arg1
         else if arg2 = 1
            xor arg1,arg1
            inc arg1
         else if arg2 = -1
            or  arg1,-1
         else if arg2 > -128 & arg2 < 128
            push arg2
            pop  arg1
         else
            mov  arg1,arg2
         end if
      else
         mov arg1,arg2
      end if
   else
      mov arg1,arg2
   end if
 }


; Interface

;WriteTextToWindow
macro Text xStart,yStart,rgbColor,pText,nTextLen
{
 mov ebx,xStart
 shl ebx,16
 add ebx,yStart
 mov ecx,rgbColor
 mov edx,pText
 mov esi,nTextLen
 mov eax,4
 int 0x40
}

;DisplayNumber
macro Number xStart,yStart,nPrintType,noOfDigits,Data,rgbColor
{
 mov edx,xStart
 shl edx,16
 add edx,yStart
 mov ebx,noOfDigits
 shl ebx,16
 or ebx,nPrintType
 mov ecx,Data
 mov esi,rgbColor
 mov eax,47
 int 0x40
}

macro DrawLine xStart,xEnd,yStart,yEnd,rgbColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xEnd
 mov ecx,yStart
 shl ecx,16
 add ecx, yEnd
 mov edx,rgbColor
 mov eax,38
 int 0x40
}

;//////////////////////////////////////////////
; Decoding cache L1,L2,L3 for Intel


decodecache:

comp0:
cmp al, 00h
je L000

;-------L2 inst

comp1:
cmp al, 41h
je L128


comp2:
cmp al, 42h
je L256

comp3:
cmp al, 43h
je L512

comp4:
cmp al, 44h
je L1024


comp5:
cmp al, 45h
je L2048

;------------L1 inst
comp6:
cmp al, 06h
je Li8


comp7:
cmp al, 08h
je Li16

comp8:
cmp al, 15h
je Li16

comp9:
cmp al, 77h
je Li16

comp10:
cmp al, 30h
je Li32
;---------------L1 data
comp11:
cmp al, 0Ah
je Ld8

comp12:
cmp al, 66h
je Ld8

comp13:
cmp al, 0Ch
je Ld16

comp14:
cmp al, 10h
je Ld16

comp15:
cmp al, 67h
je Ld16

comp16:
cmp al, 2Ch
je Ld32

comp17:
cmp al, 68h
je Ld32

;-------L2
comp18:
cmp al, 39h
je L128

comp19:
cmp al, 3Bh
je L128

comp20:
cmp al, 79h
je L128

comp21:
cmp al, 81h
je L128

comp22:
cmp al, 3Ch
je L256

comp23:
cmp al, 7Ah
je L256

comp24:
cmp al, 7Eh
je L256

comp25:
cmp al, 82h
je L256

comp26:
cmp al, 7Bh
je L512

comp27:
cmp al, 83h
je L512

comp28:
cmp al, 86h
je L512

comp29:
cmp al, 7Ch
je L1024

comp30:
cmp al, 84h
je L1024

comp31:
cmp al, 87h
je L1024

comp32:
cmp al, 85h
je L2048

;-----L1 Trace instr

comp33:
cmp al, 70h
je Li12

comp34:
cmp al, 71h
je Li16

comp35:
cmp al, 72h
je Li32

;----New codes

comp36:
cmp al, 60h
je Ld16

comp37:
cmp al, 78h
je L1024

comp38:
cmp al, 7Dh
je L2048

;---- L3
comp39:
cmp al, 22h
je L305

comp40:
cmp al, 23h
je L31

comp41:
cmp al, 25h
je L32

comp42:
cmp al, 29h
je L34

comp43:
cmp al, 88h
je L32

comp44:
cmp al, 89h
je L34

comp45:
cmp al, 8Ah
je L38

comp46:
cmp al, 8Dh
je L34

;============= v. 2.04
comp47:
cmp al, 73h
je Li64

comp48:
cmp al, 1Ah
je L96

comp49:
cmp al, 3Ah
je L192

comp50:
cmp al, 3Dh
je L384

comp51:
cmp al, 3Eh
je L512

comp52:
cmp al, 7Fh
je L512

comp53:
cmp al, 46h
je L34

comp54:
cmp al, 47h
je L38

comp55:
cmp al, 49h
je L34

comp56:
cmp al, 4Ah
je L36

comp57:
cmp al, 4Bh
je L38

comp58:
cmp al, 4Ch
je L312

comp59:
cmp al, 4Dh
je L316

jne L000

;------------------
Li8:
mov [L1i], 8
jmp L000

Li12:
mov [L1i], 12
jmp L000

Li16:
mov [L1i], 16
jmp L000

Li32:
mov [L1i], 32
jmp L000

Li64:
mov [L1i], 64
jmp L000

Ld8:
mov [L1d], 8
jmp L000

Ld16:
mov [L1d], 16
jmp L000

Ld32:
mov [L1d], 32
jmp L000

L96:
mov [L2], 96
jmp L000

L128:
mov [L2], 128
jmp L000

L192:
mov [L2], 192
jmp L000

L256:
mov [L2], 256
jmp L000

L384:
mov [L2], 384
jmp L000

L512:
mov [L2], 512
jmp L000

L1024:
mov [L2], 1024
jmp L000

L2048:
mov [L2], 2048
jmp L000

L305:
mov [L3], 512
jmp L000

L31:
mov [L3], 1024
jmp L000

L32:
mov [L3], 2048
jmp L000

L34:
mov [L3], 4096
jmp L000

L36:
mov [L3], 6144
jmp L000

L38:
mov [L3], 8192
jmp L000

L312:
mov [L3], 12288
jmp L000

L316:
mov [L3], 16384
jmp L000

L000:
ret
;//////////////////////////////////////////////
;Decoding Brand ID for Intel



decodebrand:


cmp dword[smallvendor], 'cAMD'
jz amd_br
cmp dword[smallvendor], 'ntel'
jz intel_br
jmp Bi00   ; if not AMD or Intel

amd_br:      ;;;;;;;;;;;;;;;;;;;;; amd brand

xor eax,eax
inc eax
cpuid

cnnn0:
cmp bl, 00h
ja rr
je cnnn1

rr:
mov byte [brand], bl ;
Text 275,210,0x00000000,abrand00, abrand00len-abrand00
jmp B000
;jmp comppp

cnnn1:

mov     eax, 0x80000001               ; CPUID ext. function 0x80000001
cpuid

cmp ebx, 0
je Bi00
mov word [brand], bx ;

comppp:
cmp  [brand], 400h
jl res1
cmp  [brand], 500h
jl res2
jae res3


res1:
Text 275,210,0x00000000,abrand0, abrand0len-abrand0
jmp B000

res2:
Text 275,210,0x00000000,abrand1, abrand1len-abrand1
jmp B000

res3:
Text 275,210,0x00000000,abrand2, abrand2len-abrand2
jmp B000
             ;;;;;;;;;;;;;;;;;;;;; intel brand
intel_br:
      xor eax,eax
      inc eax
      cpuid
cmp0:
cmp bl, 00h
je Bi00

cmp1:
cmp bl, 01h
je Bi01

cmp2:
cmp bl, 02h
je Bi02

cmp3:
cmp bl, 03h
je Bi03

cmp4:
cmp bl, 04h
je Bi04

cmp6:
cmp bl, 06h
je Bi06

cmp7:
cmp bl, 07h
je Bi07

cmp8:
cmp bl, 08h
je Bi08

cmp9:
cmp bl, 09h
je Bi09

cmpA:
cmp bl, 0Ah
je Bi0A

cmpB:
cmp bl, 0Bh
je Bi0B

cmpC:
cmp bl, 0Ch
je Bi0C

cmpE:
cmp bl, 0Eh
je Bi0E

cmpF:
cmp bl, 0Fh
je Bi0F

cmp11:
cmp bl, 11h
je Bi11

cmp12:
cmp bl, 12h
je Bi12

cmp13:
cmp bl, 13h
je Bi13

cmp14:
cmp bl, 14h
je Bi14

cmp15:
cmp bl, 15h
je Bi15

cmp16:
cmp bl, 16h
je Bi16

cmp17:
cmp bl, 17h
je Bi17

jne Bi00

;------------------
Bi00:
mov byte [brand], bl ;00h
Text 275,210,0x00000000,brand0, brand0len-brand0
;Number 275,200,1*256,2,dword [brand],0x000000     was in v. 1.11
jmp B000

Bi01:
mov [brand], 01h
Text 275,210,0x00000000,brand01, brand01len-brand01
jmp B000

Bi02:
mov [brand], 02h
Text 275,210,0x00000000,brand02, brand02len-brand02
jmp B000

Bi03:
mov [brand], 03h

cmp [m], 8
je E3
Text 275,210,0x00000000,brand03, brand03len-brand03
jmp B000
E3:
Text 275,210,0x00000000,brand03d, brand03dlen-brand03d
jmp B000

Bi04:
mov [brand], 04h
Text 275,210,0x00000000,brand04, brand04len-brand04
jmp B000

Bi06:
mov [brand], 06h
Text 275,210,0x00000000,brand06, brand06len-brand06
jmp B000

Bi07:
mov [brand], 07h
Text 275,210,0x00000000,brand07, brand07len-brand07
jmp B000

Bi08:
mov [brand], 08h
Text 275,210,0x00000000,brand08, brand08len-brand08
jmp B000

Bi09:
mov [brand], 09h
Text 275,210,0x00000000,brand09, brand09len-brand09
jmp B000

Bi0A:
mov [brand], 0Ah
Text 275,210,0x00000000,brand0A, brand0Alen-brand0A
jmp B000

Bi0B:
mov [brand], 0Bh

cmp [m], 13
jl Eb
Text 275,210,0x00000000,brand0B, brand0Blen-brand0B
jmp B000
Eb:
Text 275,210,0x00000000,brand0Bd, brand0Bdlen-brand0Bd
jmp B000

Bi0C:
mov [brand], 0Ch
Text 275,210,0x00000000,brand0C, brand0Clen-brand0C
jmp B000

Bi0E:
mov [brand], 0Eh

cmp [m], 13
jl Ed
Text 275,210,0x00000000,brand0E, brand0Elen-brand0E
jmp B000
Ed:
Text 275,210,0x00000000,brand0Ed, brand0Edlen-brand0Ed
jmp B000

Bi0F:
mov [brand], 0Fh
Text 275,210,0x00000000,brand0F, brand0Flen-brand0F
jmp B000

Bi11:
mov [brand], 11h
Text 275,210,0x00000000,brand11, brand11len-brand11
jmp B000

Bi12:
mov [brand], 12h
Text 275,210,0x00000000,brand12, brand12len-brand12
jmp B000

Bi13:
mov [brand], 13h
Text 275,210,0x00000000,brand13, brand13len-brand13
jmp B000

Bi14:
mov [brand], 14h
Text 275,210,0x00000000,brand14, brand14len-brand14
jmp B000

Bi15:
mov [brand], 15h
Text 275,210,0x00000000,brand15, brand15len-brand15
jmp B000

Bi16:
mov [brand], 16h
Text 275,210,0x00000000,brand16, brand16len-brand16
jmp B000

Bi17:
mov [brand], 17h
Text 275,210,0x00000000,brand17, brand17len-brand17
jmp B000

B000:

ret
;//////////////////////////////////////////////

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
decode_sse3:   ; is SS3 supported
      xor eax,eax
      inc eax
      cpuid
  test  ecx, $1 ; Test bit 1
  jnz   .EX; SSE3 technology is  supported
  jz .EXN

.EX:
   mov dword  [sse3+ 6],  $736579
   mov [sse3sup], 1
   jmp exitter
.EXN:
    mov dword [sse3+ 6], $6F6E
    mov [sse3sup],0

exitter:

ret
;//////////////////////////////////////////////
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

mov   dword [CNXT_ID+ 9], $6F6E
jmp Tes7

Tes6e:
mov   dword [CNXT_ID+ 9], $736579


Tes7:
test ecx, CX16_FLAG
jnz Tes7e

mov   dword [CX16+ 9], $6F6E
jmp Tes8

Tes7e:
mov   dword [CX16+ 9], $736579


Tes8:
test ecx, ETPRD_FLAG
jnz Tes8e

mov   dword [ETPRD+ 9], $6F6E
jmp Tes9

Tes8e:
mov   dword [ETPRD+ 9], $736579

Tes9:

ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
decode_extended:
    xor   eax, eax
    mov   eax,$80000000
    cpuid
                           ;cmp  eax,$80000000
test eax, 80000000h
jnz gooodd
;
jmp baaad

baaad:
;mov   dword [MP+8], $6F6E
;mov   dword [NX+ 8], $6F6E
;mov   dword [MMXPi+ 8], $6F6E
;mov   dword [MMXn+ 8], $6F6E
;mov   dword [FXSRn+ 8], $6F6E
;mov   dword [FFXSR+ 10], $6F6E
;mov   dword [TSCP+ 10], $6F6E
;mov   dword [LM+ 10], $6F6E
;mov   dword [DNo+ 10], $6F6E
;mov   dword [DN+ 10], $6F6E
;mov   dword [SYS+ 9], $6F6E
;mov   dword [LAF+ 9], $6F6E



jmp Tez13

gooodd:
  xor eax, eax
  mov   eax, $80000001 ;// Setup extended function 8000_0001h
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
;jmp Tez3 было, но из-за Сирикс приходится делать детект в лругом месте

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

mov   dword [FFXSR+ 10], $6F6E
jmp Tez7

Tez6e:
mov   dword [FFXSR+ 10], $736579

Tez7:
test edx, TSCP_FLAG
jnz Tez7e

mov   dword [TSCP+ 10], $6F6E
jmp Tez8

Tez7e:
mov   dword [TSCP+ 10], $736579


Tez8:
test edx, LM_FLAG
jnz Tez8e

mov   dword [LM+ 10], $6F6E
jmp Tez9

Tez8e:
mov   dword [LM+ 10], $736579

Tez9:
test edx, DNo_FLAG
jnz Tez9e

mov   dword [DNo+ 10], $6F6E
jmp Tez10

Tez9e:
mov   dword [DNo+ 10], $736579


Tez10:
test edx, DN_FLAG
jnz Tez10e

mov   dword [DN+ 10], $6F6E
jmp Tez11

Tez10e:
mov   dword [DN+ 10], $736579

;Intel

Tez11:
test edx, SYS_FLAG
jnz Tez11e

mov   dword [SYS+ 9], $6F6E
jmp Tez12

Tez11e:
mov   dword [SYS+ 9], $736579


Tez12:
test ecx, LAHF_FLAG
jnz Tez12e

mov   dword [LAF+ 9], $6F6E
jmp Tez13

Tez12e:
mov   dword [LAF+ 9], $736579

Tez13:

ret
;//////////////////////////////////////////////
newrating:

cmp [FRS], 266
jz f266
cmp [FRS], 333
jz f333
cmp [FRS], 400
jz f400

f266:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Print the unpacked img to screen
;    gif2img i333,img_area3
;    mov  eax,7      ; sysfunc7: putimage
;    mov  ebx,img_area3+8   ; pointer to image
;    mov  ecx,33*65536+30  ; resolution (all gif's included are 64*81)
;    mov  edx,240*65536+90  ; plce to print image on screen (first number is
                            ;pixels from left, second pixels from top)
;    int  0x40      ; execute function

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;формулы вычисления XP-рейтинга Athlon-ов (Tbred, Barton):
;FSB266/256kB:   сделано
;XP+ (Freq<2000MHz) = 1000 + (Freq-1000)*1,5 = Freq*1.5-500
;XP+ (Freq>=2000MHz) = 900 + (Freq-1000)*1,5 = Freq*1.5-600


   cmp [total], 2000
   jl pal

    ; Рейтинг для Паломино 2000+
    mov eax,[total]
    imul eax, 3
    shr eax,1
    sub eax, 600
    mov [rating], eax

jmp vyxod

pal:

    ; Рейтинг для Паломино 2000-
    mov eax,[total]
    imul eax, 3
    shr eax,1
    sub eax, 500
    mov [rating], eax

jmp vyxod


f333:




 cmp [L2], 256
 jl .Th ; Thorton

;FSB333/512kB: сделано
;XP+ (Freq<2100MHz) = Freq*1.2 + 300
;XP+ (Freq>=2100MHz) = Freq*1.2 + 400

cmp [total], 2100
jl .r2100
   mov eax,[total]
   xor edx, edx
   imul eax, 600000
   mov  ebx, 500000
   div ebx
   mov  [rating], eax
   add [rating], 400

; Rating for Barton F<2000 MHz
jmp vyxod

.r2100:
    mov eax,[total]
    xor edx, edx
    imul eax, 600000
    mov  ebx, 500000
    div ebx
    mov  [rating], eax
    add [rating], 300


 ; Rating for Barton F>2000 MHz
jmp vyxod

;FSB333/256kB: сделано
;XP+ = Freq*1.2 + 100

.Th:

    mov eax,[total]
    xor edx, edx
    imul eax, 600000
    mov  ebx, 500000
    div ebx
    mov  [rating], eax
    add [rating], 100


;Rating for Thorton


jmp vyxod

f400:


;FSB400/512kB:
;XP+ = Freq*1.5 - 100

  mov eax,[total]
  imul eax, 3
  shr eax,1
  sub eax, 100
  mov [rating], eax

vyxod:


ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

multipl:
;--- вычисление коэффициента умножения -
; портит регистры
; out : CL = коэфф.умножения * 10, или 0
  xor  ecx, ecx

            cmp dword[smallvendor], 'cAMD'  ; Check for Advanced Micro Devices CPU
  jne  noAMD
  cmp  byte [f], 5
  jne  noAMDK6
  mov  eax, 68    ; AMD-K6 (p.30)
  mov  ebx, 3
  mov  edx, 0x0C0000087
  int  0x40    ; eax - low dword
  and  eax, 111b
  mov  cl, [athloncoef3 + eax]
  cmp  eax, 6
  jne  @f
  cmp  byte[m], 8
  jae  @f
  mov  cl, 20
@@:  ret

noAMDK6:
           cmp  byte [f], 6
  jne  noAMDK7
  mov  eax, 68    ; Athlon/AthlonXP
  mov  ebx, 3
  mov  edx, 0x0C0010015
  int  0x40
  mov  ebx, eax
  shr  ebx, 24
  and  ebx, 0x0F
  shr  eax, 20
  jc  @f
  mov  cl, [athloncoef + ebx]
  ret
@@:  mov  cl, [athloncoef2 + ebx]
  ret

noAMD:
           cmp dword[smallvendor], 'ntel'  ; Check for International Electronics CPU
  jne  noIntel
  cmp  byte[f], 0x0F
  jne  noIntelP4
  cmp  byte [t], 6
  jne  @f
  mov  eax, 68    ; Pentium M
  mov  ebx, 3
  mov  edx, 0x2A
  int  0x40
  shr  eax, 22
  and  eax, 0x1F
  mov  dl, 10
  mul  dl
  mov  cl, al
  ret
@@:  cmp  byte [m], 2
  jae  @f
  mov  eax, 68    ; Pentium 4 / Xeon (model < 2)
  mov  ebx, 3
  mov  edx, 0x2A
  int  0x40
  shr  eax, 8
  and  eax, 0x0F
  mov  cl, [p4coef + eax]
  ret
@@:  mov  eax, 68    ; Pentium 4 / Xeon (model >= 2)
  mov  ebx, 3
  mov  edx, 0x2C
  int  0x40

  ; shr  eax, 24
  ; and  eax, 0x1F
  shr  eax, 27
  and  al, 0Fh
  add  al, 8

  mov  dl, 10
  mul  dl
  mov  cl, al
  ret
noIntelP4:
  cmp  byte[f], 6
  jne  noIntelP6
  mov  eax, 68    ; Pentium Pro / Pentium II / Pentium III
  mov  ebx, 3
  mov  edx, 0x2A
  int  0x40
  shr  eax, 22
  test  al, 0x20
  jz  @f
  or  al, 0x10
@@:  and  eax, 0x1f
  mov  cl, [coppercoeff + eax]
  cmp  byte[m], 0x0B
  jb  @f
  mov  cl, [tualatcoeff + eax]
@@:  ret

noIntel:
noIntelP6:
noAMDK7:
  ret


