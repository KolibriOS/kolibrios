iglobal
  prev_user_of_fpu dd 0x1 ; set to OS
endg

label fpu_tss at 0xB080
label fpu_stack dword at 0xB060


align 4
fpu_handler:

        ; clear TS flag in CR0 -> for task switching
        clts

        ; save FPU context of the previous task
        mov    eax,[prev_user_of_fpu]
        shl    eax,8
        add    eax,0x80000+0x10
        fsave  [eax]

        ; next task switch save our FPU context
        ; now restore context of current task (if exists)
        mov    eax,[0x3000]
        mov    [prev_user_of_fpu],eax
        shl    eax,8
        add    eax,0x80000
        cmp    [eax+0x7f],byte 0
        je     bs7_first_fpu
        frstor [eax+0x10]
      bs7_first_fpu:
        mov    [eax+0x7f],byte 1

        ; prepare structure in stack for proper IRET
        movzx  eax,word [fpu_tss+l.ss-tss_sceleton]  ; push ss
        push   eax
        mov    eax,[fpu_tss+l.esp-tss_sceleton]      ; push esp
        push   eax
        mov    eax,[fpu_tss+l.eflags-tss_sceleton]   ; push eflags
        push   eax

        movzx  eax,word [fpu_tss+l.cs-tss_sceleton]  ; push cs
        push   eax
        mov    eax,[fpu_tss+l.eip-tss_sceleton]      ; push eip
        push   eax

        ; save eax
        push   dword [fpu_tss+l.eax-tss_sceleton]

        ; restore all segment registers
        mov    ax,[fpu_tss+l.es-tss_sceleton]
        mov    es,ax
        mov    ax,[fpu_tss+l.fs-tss_sceleton]
        mov    fs,ax
        mov    ax,[fpu_tss+l.gs-tss_sceleton]
        mov    gs,ax
        mov    ax,[fpu_tss+l.ds-tss_sceleton]
        mov    ds,ax

        ; restore eax
        pop    eax

        iret
