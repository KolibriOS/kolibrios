; READ TSC / SECOND

        mov   esi,boot_tsc
        call  boot_log
        call  _rdtsc
        mov   ecx,eax
        mov   esi,250               ; wait 1/4 a second
        call  delay_ms
        call  _rdtsc
        sub   eax,ecx
        shl   eax,2
        mov   [CPU_FREQ],eax          ; save tsc / sec
        mov ebx, 1000000
        div ebx
        mov [stall_mcs], eax
