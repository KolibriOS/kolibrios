; -------- Fast System Call init ----------
; Intel SYSENTER/SYSEXIT (AMD CPU support it too)
           bt [cpu_caps], CAPS_SEP
           jnc .SEnP   ; SysEnter not Present
           xor edx, edx
           mov ecx, MSR_SYSENTER_CS
           mov eax, os_code
           wrmsr
           mov ecx, MSR_SYSENTER_ESP
;           mov eax, sysenter_stack ; Check it
           xor     eax, eax
           wrmsr
           mov ecx, MSR_SYSENTER_EIP
           mov eax, sysenter_entry
           wrmsr
.SEnP:
; AMD SYSCALL/SYSRET
           cmp byte[cpu_vendor], 'A'
           jne .noSYSCALL
           mov eax, 0x80000001
           cpuid
           test edx, 0x800  ; bit_11 - SYSCALL/SYSRET support
           jz .noSYSCALL
           mov ecx, MSR_AMD_EFER
           rdmsr
           or eax, 1   ; bit_0 - System Call Extension (SCE)
           wrmsr

	; !!!! It`s dirty hack, fix it !!!
	; Bits of EDX :
	; Bit 31–16 During the SYSRET instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.
	; Bit 15–0 During the SYSCALL instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.

	; mov	edx, (os_code + 16) * 65536 + os_code
           mov edx, 0x1B0013

           mov eax, syscall_entry
           mov ecx, MSR_AMD_STAR
           wrmsr
.noSYSCALL:
; -----------------------------------------