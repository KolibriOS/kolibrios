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
EST_FLAG equ 0080h	  ; EST-7
TM2_FLAG equ 0100h	  ; TM2-8
SSSE3_FLAG equ 0200h	  ;SSSE3 -9  
CNXT_ID_FLAG equ 0400h	  ;CID -10
CX16_FLAG equ 2000h	  ;CX16 - 13
ETPRD_FLAG equ 4000h	  ;xTPR - 14
DCA_FLAG equ 40000h	  ;DCA - 18  


; edx cpuid(8000_0001h)

SYS_FLAG equ 0800h	 ;11

MP_FLAG equ 80000h	 ;19
NX_FLAG equ 100000h	 ;20
MMXPi_FLAG equ 400000h	  ;22
MMXn_FLAG equ 800000h	  ;23
FXSRn_FLAG equ 1000000h   ;24   it is ллу+ for Cyrix
FFXSR_FLAG equ 2000000h  ;25
TSCP_FLAG equ 8000000h	 ;27
LM_FLAG equ 20000000h	 ;29  EM64T
DNo_FLAG equ 40000000h	;30
DN_FLAG equ 80000000h	;31

; ecx cpuid(8000_0001h)
LAHF_FLAG equ 0001h  ; LAHF   0
CMPL_FLAG equ 0002h    ; core multi-processing legacy mode 1
SVM_FLAG equ 0004h     ; secure virtual machine 2
MOVCR8_FLAG equ 0010h    ;AltMovCr8  4

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

;decoding standard features

decode_standard_features:

 xor eax,eax
 inc eax
 cpuid

        mov     eax, FPU+6
        call    show_next_bit

        mov     eax, VME+7
        call    show_next_bit

        mov     eax, DE+7
        call    show_next_bit

        mov     eax, PSE+8
        call    show_next_bit

        mov     eax, TSC+6
        call    show_next_bit

        mov     eax, MSR+7
        call    show_next_bit

        mov     eax, PAE+7
        call    show_next_bit

        mov     eax, MCE+8
        call    show_next_bit

        mov     eax, CX8+6
        call    show_next_bit

        mov     eax, APIC+7
        call    show_next_bit

        shr     edx, 1          ; skip reserved bit

        mov     eax, SEP+8
        call    show_next_bit

        mov     eax, MTRR+6
        call    show_next_bit

        mov     eax, PGE+7
        call    show_next_bit

        mov     eax, MCA+7
        call    show_next_bit

        mov     eax, CMOV+8
        call    show_next_bit

        mov     eax, PAT+6
        call    show_next_bit

        mov     eax, PSE36+7
        call    show_next_bit

        mov     eax, PSNUM+7
        call    show_next_bit

        mov     eax, CLFLUSHn+8
        call    show_next_bit

        shr     edx, 1          ; skip reserved bit

        mov     eax, DTS+7
        call    show_next_bit

        mov     eax, ACPI+7
        call    show_next_bit

        mov     eax, MMX+8
        call    show_next_bit
        mov     eax, [eax]
        mov     [MMXs+7], eax

        mov     eax, FXSR+6
        call    show_next_bit

        mov     eax, SSE+7
        call    show_next_bit

        mov     eax, SSE2+7
        call    show_next_bit

        mov     eax, SSn+8
        call    show_next_bit

        shr     edx, 1
;       mov     eax, HTT+8
;       test    edx, HTT_FLAG
;       call    write_yes_no

        mov     eax, TM+7
        call    show_next_bit

        mov     eax, IA64+7
        call    show_next_bit

        mov     eax, PBE+8
        call    show_next_bit

        ret

;//////////////////////////////////////////////
decode_extended_features:
; is called immediately after decode_standard_features
;        xor     eax, eax
;        inc     eax
;        cpuid

        mov     eax, SS3+8
        test    ecx, SSE3_FLAG
        call    write_yes_no

        mov     eax, MON+8
        test    ecx, MON_FLAG
        call    write_yes_no

        mov     eax, DS_CPL+8
        test    ecx, DS_CPL_FLAG
        call    write_yes_no

        mov     eax, EST+8
        test    ecx, EST_FLAG
        call    write_yes_no

        mov     eax, TM2+8
        test    ecx, TM2_FLAG
        call    write_yes_no

        mov     eax, CNXT_ID+12
        test    ecx, CNXT_ID_FLAG
        call    write_yes_no

        mov     eax, CX16+12
        test    ecx, CX16_FLAG
        call    write_yes_no

        mov     eax, ETPRD+12
        test    ecx, ETPRD_FLAG
        call    write_yes_no

        mov     eax, VMX+8
        test    ecx, VMX_FLAG
        call    write_yes_no

        mov     eax, SSSE3+12
        test    ecx, SSSE3_FLAG
        call    write_yes_no

        mov     eax, DCA+8
        test    ecx, DCA_FLAG
        call    write_yes_no
@@:
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
decode_extended:
        mov     eax, $80000000
        cpuid
        mov     [extc], eax     ; max number of calls
        test    eax, eax
        jns     @b

        mov     eax, $80000001 ;// Setup extended function 8000_0001h
        cpuid

        mov     eax, MP+8
        test    edx, MP_FLAG
        call    write_yes_no

        mov     eax, NX+8
        test    edx, NX_FLAG
        call    write_yes_no

;jmp Tez3 we do detection in another place, because of Cyrix specific MMX+ detection 
;       mov     eax, MMXPi+8
;       test    edx, MMXPi_FLAG
;       call    write_yes_no

        mov     eax, MMXn+8
        test    edx, MMXn_FLAG
        call    write_yes_no

        mov     eax, FXSRn+8
        test    edx, FXSRn_FLAG
        call    write_yes_no

        mov     eax, FFXSR+12
        test    edx, FFXSR_FLAG
        call    write_yes_no

        mov     eax, TSCP+12
        test    edx, TSCP_FLAG
        call    write_yes_no

        mov     eax, LM+12
        test    edx, LM_FLAG
        call    write_yes_no

        mov     eax, DNo+12
        test    edx, DNo_FLAG
        call    write_yes_no

        mov     eax, DN+12
        test    edx, DN_FLAG
        call    write_yes_no

;Intel
        mov     eax, SYS+12
        test    edx, SYS_FLAG
        call    write_yes_no

        mov     eax, LAF+12
        test    ecx, LAHF_FLAG
        call    write_yes_no

        mov     eax, CMPL+12
        test    ecx, CMPL_FLAG
        call    write_yes_no

        mov     eax, SVM+8
        test    ecx, SVM_FLAG
        call    write_yes_no

        mov     eax, MCR8+12
        test    ecx, MOVCR8_FLAG
        call    write_yes_no

        ret
