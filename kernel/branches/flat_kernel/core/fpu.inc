
init_fpu:
           clts
           fninit

           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

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
           fxsave [fpu_data]  ;[eax]
           ret
.no_SSE:
           mov ecx, cr0
           and ecx, not CR0_EM
           or ecx, CR0_MP+CR0_NE
           mov cr0, ecx
           fnsave [fpu_data]
           ret

; param
;  eax= 512 bytes memory area

align 4
fpu_save:
           push ecx
           push esi
           push edi

           pushfd
           cli

           clts
           mov edi, eax

           mov ecx, [fpu_owner]
           mov eax, [CURRENT_TASK]
           cmp ecx, eax
           jne .save
.copy:
           shl eax, 8
           mov esi, [eax+SLOT_BASE+APPDATA.fpu_state]
           mov ecx, 512/4
           cld
           rep movsd
           fninit

           popfd
           pop edi
           pop esi
           pop ecx
           ret
.save:
           mov [fpu_owner], eax

           shl ecx, 8
           mov ecx, [ecx+SLOT_BASE+APPDATA.fpu_state]

           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

           fxsave [ecx]
           jmp .copy
.no_SSE:
           fnsave [ecx]
           jmp .copy

align 4
fpu_restore:
           push ecx
           push esi

           mov esi, eax

           pushfd
           cli

           mov ecx, [fpu_owner]
           mov eax, [CURRENT_TASK]
           cmp ecx, eax
           jne .copy

           clts

           bt [cpu_caps], CAPS_SSE
           jnc .no_SSE

           fxrstor [esi]
           popfd
           pop esi
           pop ecx
           ret
.no_SSE:
           fnclex               ;fix possible problems
           frstor [esi]
           popfd
           pop esi
           pop ecx
           ret
.copy:
           shl eax, 8
           mov edi, [eax+SLOT_BASE+APPDATA.fpu_state]
           mov ecx, 512/4
           cld
           rep movsd
           popfd
           pop esi
           pop ecx
           ret

align 4
e7:                        ;#NM exception handler
        save_ring3_context
        clts
        mov ax, app_data  ;
        mov ds, ax
        mov es, ax

        mov ebx, [fpu_owner]
        cmp ebx, [CURRENT_TASK]
        je .exit

        shl ebx, 8
        mov eax, [ebx+SLOT_BASE+APPDATA.fpu_state]
        bt [cpu_caps], CAPS_SSE
        jnc .no_SSE

        fxsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        mov eax, [ebx+SLOT_BASE+APPDATA.fpu_state]
        fxrstor [eax]
.exit:
        restore_ring3_context
        iret

.no_SSE:
        fnsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        mov eax, [ebx+SLOT_BASE+APPDATA.fpu_state]
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

           mov ebx, [CURRENT_TASK]
           shl ebx, 8

           mov eax, [ebx+SLOT_BASE+APPDATA.fpu_handler]
           test eax, eax
           jz .default

           mov ecx, [reg_eip]
           mov edx, [reg_esp]
           sub edx, 4
           mov [edx], ecx
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

           mov ebx, [current_slot]

           mov eax, [ebx+APPDATA.sse_handler]
           test eax, eax
           jz .default

           mov ecx, [reg_eip]
           mov edx, [reg_esp]
           sub edx, 4
           mov [edx], ecx
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


