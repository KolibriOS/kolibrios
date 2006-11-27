
init_fpu:
           clts
           fninit

           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

           stdcall kernel_alloc, 512*256
           mov [fpu_data], eax

           mov ebx, cr4
           mov ecx, cr0
           or ebx, CR4_OSFXSR+CR4_OSXMMEXPT
           mov cr4, ebx

           and ecx, not (CR0_MP+CR0_EM)
           or ecx, CR0_NE
           mov cr0, ecx

           mov dword [esp-4], SSE_INIT
           ldmxcsr [esp-4]

           xorps xmm0, xmm0
           xorps xmm1, xmm1
           xorps xmm2, xmm2
           xorps xmm3, xmm3
           xorps xmm4, xmm4
           xorps xmm5, xmm5
           xorps xmm6, xmm6
           xorps xmm7, xmm7
           fxsave [eax]
           ret
.no_SSE:
           stdcall kernel_alloc, 112*256
           mov [fpu_data], eax
           mov ecx, cr0
           and ecx, not CR0_EM
           or ecx, CR0_MP+CR0_NE
           mov cr0, ecx
           fnsave [eax]
           ret

align 4
proc fpu_save
           clts
           mov ebx, [fpu_owner]
           shl ebx, 8
           mov eax, [ebx+PROC_BASE+0x10]
           mov ebx, [CURRENT_TASK]
           mov [fpu_owner], ebx

           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

           fxsave [eax]
           fninit            ;re-init fpu
           ret
.no_SSE:
           fnsave [eax]
           ret
endp

align 4
proc fpu_restore
           mov ebx, [CURRENT_TASK]
           shl ebx, 8
           mov eax, [ebx+PROC_BASE+0x10]
           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

           fxrstor [eax]
           ret
.no_SSE:
           fnclex               ;fix possible problems
           frstor [eax]
           ret
endp

align 4
e7:                        ;#NM exception handler
        save_ring3_context
        clts
        mov ax, os_data
        mov ds, ax
        mov es, ax

        mov ebx, [fpu_owner]
        cmp ebx, [CURRENT_TASK]
        je .exit

        shl ebx, 8
        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        bt [cpu_caps], CAPS_SSE
        jnc .no_SSE

        fxsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        fxrstor [eax]
.exit:
        restore_ring3_context
        iret

.no_SSE:
        fnsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        frstor [eax]
        restore_ring3_context
        iret

iglobal
  fpu_owner dd 1
 endg



reg_eip       equ ebp+4
reg_cs        equ ebp+8
reg_eflags    equ ebp+12
reg_esp       equ ebp+16
reg_ss        equ ebp+20

align 4
except_16:           ;fpu native exceptions handler
           push ebp
           mov ebp, esp

           push eax
           push ebx
           push ecx
           push edx

           mov ebx, [ss:CURRENT_TASK]
           shl ebx, 8

           mov eax, [ss:ebx+PROC_BASE+APPDATA.fpu_handler]
           test eax, eax
           jz .default

           mov ecx, [reg_eip]
           mov edx, [reg_esp]
           sub edx, 4
           mov [ss:edx+new_app_base], ecx
           mov [reg_esp], edx
           mov dword [reg_eip], eax

           pop edx
           pop ecx
           pop ebx
           pop eax

           leave
           iretd

.default:
           pop edx
           pop ecx
           pop ebx
           pop eax
           leave

           save_ring3_context     ;debugger support

           mov bl, 16
           jmp exc_c

align 4
except_19:          ;sse exceptions handler
           push ebp
           mov ebp, esp

           push eax
           push ebx
           push ecx
           push edx

           mov ebx, [ss:CURRENT_TASK]
           shl ebx, 8

           mov eax, [ss:ebx+PROC_BASE+APPDATA.sse_handler]
           test eax, eax
           jz .default

           mov ecx, [reg_eip]
           mov edx, [reg_esp]
           sub edx, 4
           mov [ss:edx+new_app_base], ecx
           mov [reg_esp], edx
           mov dword [reg_eip], eax

           pop edx
           pop ecx
           pop ebx
           pop eax

           leave
           iretd

.default:
           pop edx
           pop ecx
           pop ebx
           pop eax
           leave

           save_ring3_context     ;debugger support

           mov bl, 19
           jmp exc_c

restore reg_eip
restore reg_cs
restore reg_eflags
restore reg_esp
restore reg_ss


