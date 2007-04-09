; ENABLE PAGING
           mov eax, sys_pgdir
           mov cr3, eax

           mov eax,cr0
           or eax,CR0_PG
           mov cr0,eax

           call init_kernel_heap
           stdcall kernel_alloc, RING0_STACK_SIZE+512
           mov [os_stack], eax

           call init_LFB
           call init_fpu

           call init_malloc

           stdcall alloc_kernel_space, 0x4F000
           mov [ipc_tmp], eax
           mov ebx, 0x1000

           add eax, 0x40000
           mov [proc_mem_map], eax

           add eax, 0x8000
           mov [proc_mem_pdir], eax

           add eax, ebx
           mov [proc_mem_tab], eax

           add eax, ebx
           mov [tmp_task_pdir], eax

           add eax, ebx
           mov [tmp_task_ptab], eax

           add eax, ebx
           mov [ipc_pdir], eax

           add eax, ebx
           mov [ipc_ptab], eax

           call init_events

           mov eax, srv.fd-SRV_FD_OFFSET
           mov [srv.fd], eax
           mov [srv.bk], eax

           mov edi, irq_tab
           xor eax, eax
           mov ecx, 16
           rep stosd

