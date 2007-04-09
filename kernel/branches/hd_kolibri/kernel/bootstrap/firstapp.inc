; LOAD FIRST APPLICATION
        mov   [CURRENT_TASK],dword 1
        mov   [TASK_COUNT],dword 1
        cli
        cmp   byte [0x2f0000+0x9030],1
        jne   no_load_vrr_m

        mov ebp, vrr_m
        xor ebx, ebx
        xor edx, edx
        call fs_execute
        cmp   eax,2                  ; if vrr_m app found (PID=2)
        je    first_app_found

no_load_vrr_m:
        mov ebp, firstapp
        xor ebx, ebx
        xor edx, edx
        call fs_execute
        cmp   eax,2                  ; continue if a process has been loaded
        je    first_app_found
        mov   eax, 0xDEADBEEF        ; otherwise halt
        hlt
first_app_found:
        cli

        ;mov   [TASK_COUNT],dword 2
        mov   [CURRENT_TASK],dword 1       ; set OS task fisrt
