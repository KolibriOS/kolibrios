checkidle:
        pushad

        cmp  [check_idle_semaphore],0
        jne  no_idle_state

        call change_task
        mov  eax,[idlemem]
        mov  ebx,[timer_ticks] ;[0xfdf0]
        cmp  eax,ebx
        jnz  idle_exit
        call _rdtsc
        mov  ecx,eax
      idle_loop:
        hlt
        cmp  [check_idle_semaphore],0
        jne  idle_loop_exit
        mov  eax,[timer_ticks] ;[0xfdf0]
        cmp  ebx,eax
        jz   idle_loop
      idle_loop_exit:
        mov  [idlemem],eax
        call _rdtsc
        sub  eax,ecx
        mov  ebx,[idleuse]
        add  ebx,eax
        mov  [idleuse],ebx

        popad
        ret

      idle_exit:

        mov  ebx,[timer_ticks] ;[0xfdf0]
        mov  [idlemem],ebx
        call change_task

        popad
        ret

      no_idle_state:

        dec  [check_idle_semaphore]

        mov  ebx,[timer_ticks] ;[0xfdf0]
        mov  [idlemem],ebx
        call change_task

        popad
        ret

uglobal
  idlemem               dd   0x0
  idleuse               dd   0x0
  idleusesec            dd   0x0
  check_idle_semaphore  dd   0x0
endg