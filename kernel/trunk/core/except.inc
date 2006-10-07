

reg_eip       equ ebp+4
reg_cs        equ ebp+8
reg_eflags    equ ebp+12
reg_esp       equ ebp+16
reg_ss        equ ebp+20
;fpu_ctrl      equ ebp-28

align 4
except_16:
           push ebp
           mov ebp, esp
;           sub esp, 28

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

;           fnstenv [fpu_ctrl]
;           fnclex
;           or word [fpu_ctrl], 0111111b
;           fldenv [fpu_ctrl]

;          pop edx
;          pop ecx
;          pop ebx
;          pop eax

;           leave
;           iretd

align 16
except_19:
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
;restore fpu_ctrl
