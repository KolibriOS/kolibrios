; Threads management.

; int create_thread(int (*proc)(void *param), void *param, int stack_size)
; Creates a thread that executes the given function proc(param).
; Returns -1 on error, TID otherwise.
; If proc(param) returns, the returned value is passed to exit_thread().
; If stack_size is zero, uses the value from PE header of the executable.
proc create_thread stdcall uses ebx, thread_proc, param, stack_size
; 1. Determine stack size.
; Align stack_size up to page boundary;
        mov     ecx, [stack_size]
        add     ecx, 0xFFF
        and     ecx, not 0xFFF
        jnz     .stack_size_ok
; if this results in zero, read the value from the header of main module.
        mov     eax, [modules_list + MODULE.next]
        mov     eax, [eax + MODULE.base]
        mov     ecx, [eax+STRIPPED_PE_HEADER.SizeOfStackReserve]
        cmp     byte [eax], 'M'
        jnz     .stack_size_ok
        mov     ecx, [eax+3Ch]
        mov     ecx, [eax+ecx+IMAGE_NT_HEADERS.OptionalHeader.SizeOfStackReserve]
.stack_size_ok:
        mov     [stack_size], ecx
; 2. Allocate the stack.
        mov     eax, 68
        mov     ebx, 12
        call    FS_SYSCALL_PTR
        test    eax, eax
        jz      .fail
; 3. Copy parameters to the stack.
        lea     edx, [eax+ecx-16]
        mov     [edx], ecx
        mov     ebx, FS_SYSCALL_PTR
        mov     [edx+4], ebx
        mov     ebx, [thread_proc]
        mov     [edx+8], ebx
        mov     ebx, [param]
        mov     [edx+12], ebx
; 4. Call the kernel to create the thread.
        mov     eax, 51
        mov     ebx, 1
        mov     ecx, internal_thread_start
        call    FS_SYSCALL_PTR
        cmp     eax, -1
        jz      .fail_free
        ret
.fail_free:
        mov     eax, 68
        mov     ebx, 13
        lea     ecx, [edx+12]
        sub     ecx, [stack_size]
        call    FS_SYSCALL_PTR
        xor     eax, eax
.fail:
        dec     eax
        ret
endp

; void exit_thread(int exit_code)
; Terminates the current thread.
; exit_code is reserved; currently ignored
proc exit_thread stdcall, exit_code
; Use int 0x40 instead of call FS_SYSCALL_PTR, because we are freeing the stack.
        mov     eax, 68
        mov     ebx, 13
        mov     ecx, FS_STACK_MIN
        int     0x40
        or      eax, -1
        int     0x40
endp

; Real entry point of threads created by create_thread.
; Provides user-space initialization of the thread,
; calls user-provided thread routine,
; passes the returned value to exit_thread.
proc internal_thread_start
        pop     eax     ; stack_size
        lea     ecx, [esp+12]
        mov     FS_STACK_MAX, ecx
        sub     ecx, eax
        mov     FS_STACK_MIN, ecx
        pop     FS_SYSCALL_PTR ; from caller's FS_SYSCALL_PTR
        pop     eax     ; thread_proc
        call    eax     ; param is still on the stack
        push    eax     ; exit_code
        push    0       ; no return address
        jmp     exit_thread
endp
