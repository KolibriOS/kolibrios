; Statistics shared by the tabs, once a second: the threads, their CPU and
; memory, the history of the graphs.

struct PROC                             ; an item of `procs`
        pid             dd ?
        cpu             dd ?            ; cycles / 256, then percent
        mem             dd ?            ; KB
        name            rb 12           ; zero padded
ends

struct RAM_INFO                         ; function 18.20
        total           dd ?            ; pages
        free            dd ?
        page_faults     dd ?
        heap_size       dd ?            ; the kernel heap, bytes
        heap_free       dd ?
        heap_blocks     dd ?
        heap_free_blocks dd ?
        heap_max_free   dd ?
        heap_max_used   dd ?
ends

; The threads, their CPU and memory, the history of the graphs.
sample:
        pusha
        mov     edi, procs
        xor     edx, edx                ; cycles of all threads / 256
        xor     ecx, ecx

.slot:
        inc     ecx
        mcall   SF_THREAD_INFO, pinfo
        mov     ebp, eax                ; the last slot
        cmp     [pinfo.slot_state], TSTATE_FREE
        je      .next

        mov     eax, [ebx + process_information.PID]
        stosd                           ; PROC.pid
        mov     eax, [ebx + process_information.cpu_usage]
        shr     eax, 8                  ; the sum fits 32 bits
        stosd                           ; PROC.cpu
        add     edx, eax
        mov     eax, [ebx + process_information.used_memory]
        inc     eax
        shr     eax, 10
        stosd                           ; PROC.mem

        lea     esi, [ebx + process_information.process_name]
        movsd                           ; PROC.name: 11 bytes and 0
        movsd
        movsd
        mov     byte [edi - 1], 0

        cmp     edi, procs + MAX_PROCS * sizeof.PROC
        jae     .done

.next:
        cmp     ecx, ebp
        jb      .slot

.done:
        mov     [procs_end], edi

; Percent of CPU = cycles of the thread / 1% of cycles of all threads.
        xchg    eax, edx
        cdq
        mov     ecx, 100
        div     ecx
        inc     eax
        xchg    eax, ecx

        mov     esi, procs
@@:
        mov     eax, [esi + PROC.cpu]
        cdq
        div     ecx
        mov     [esi + PROC.cpu], eax
        add     esi, sizeof.PROC
        cmp     esi, edi
        jb      @b

        inc     [hist_pos]              ; the place of this sample
        and     [hist_pos], HIST_LEN - 1

        mcall   SF_SYSTEM, SSF_GET_RAM_INFO, ram_info
        mov     eax, [ram_info.total]
        sub     eax, [ram_info.free]
        imul    eax, 100
        cdq
        div     [ram_info.total]

        call    sensors_read
        call    net_sample

        mov     ecx, [hist_pos]
        mov     [hist_mem + ecx*4], eax
        mov     eax, 100
        sub     eax, dword [procs + PROC.cpu] ; the first thread is IDLE
        mov     [hist_cpu + ecx*4], eax
        popa
        ret

uglobal
align 4
ram_info        RAM_INFO
hist_pos        dd      ?
hist_cpu        rd      HIST_LEN
hist_mem        rd      HIST_LEN
procs_end       dd      ?
procs           rb      MAX_PROCS * sizeof.PROC
endg
