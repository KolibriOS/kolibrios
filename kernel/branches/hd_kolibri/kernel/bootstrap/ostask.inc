; SET UP OS TASK

        mov  esi,boot_setostask
        call boot_log

;        mov eax, fpu_data
;        mov  dword [SLOT_BASE+APPDATA.fpu_state], eax
;        mov  dword [SLOT_BASE+APPDATA.fpu_handler], 0
;        mov  dword [SLOT_BASE+APPDATA.sse_handler], 0

        ; name for OS/IDLE process

        mov dword [SLOT_BASE+256+APPDATA.app_name],   dword 'OS/I'
        mov dword [SLOT_BASE+256+APPDATA.app_name+4], dword 'DLE '
        mov edi, [os_stack]
        mov dword [SLOT_BASE+256+APPDATA.pl0_stack], edi
        add edi, RING0_STACK_SIZE
        mov dword [SLOT_BASE+256+APPDATA.fpu_state], edi

        mov esi, fpu_data
        mov ecx, 512/4
        cld
        rep movsd

        mov dword [SLOT_BASE+256+APPDATA.fpu_handler], 0
        mov dword [SLOT_BASE+256+APPDATA.sse_handler], 0

        mov ebx, [def_cursor]
        mov dword [SLOT_BASE+256+APPDATA.cursor], ebx

        mov ebx, SLOT_BASE+256+APP_OBJ_OFFSET
        mov  dword [SLOT_BASE+256+APPDATA.fd_obj], ebx
        mov  dword [SLOT_BASE+256+APPDATA.bk_obj], ebx

        ; task list
        mov  [TASK_DATA+TASKDATA.wnd_number], 1 ; on screen number
        mov  [TASK_DATA+TASKDATA.pid], 1        ; process id number
        mov  [TASK_DATA+TASKDATA.mem_start], 0  ; process base address

        mov  edi,tss_data+tss_step
        mov ecx, (tss_step)/4
        xor eax, eax
        cld
        rep stosd

        mov  edi,tss_data+tss_step
        mov  [edi+TSS._ss0], os_data
        mov  eax,cr3
        mov  [edi+TSS._cr3],eax
        mov  [edi+TSS._eip],osloop
        mov  [edi+TSS._eflags],dword 0x11202 ; sti and resume
        mov eax, [os_stack]
        add eax, RING0_STACK_SIZE
        mov  [edi+TSS._esp], eax
        mov  [edi+TSS._cs],os_code
        mov  [edi+TSS._ss],os_data
        mov  [edi+TSS._ds],os_data
        mov  [edi+TSS._es],os_data
        mov  [edi+TSS._fs],os_data
        mov  [edi+TSS._gs],os_data

        mov  ax,tss0
        ltr  ax
