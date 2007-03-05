
MEM_WB     equ 6               ;write-back memory
MEM_WC     equ 1               ;write combined memory
MEM_UC     equ 0               ;uncached memory

tmp_page_tab equ 0x1000000        ; HEAP_BASE

align 4
proc mem_test

           mov eax, cr0
           and eax, not (CR0_CD+CR0_NW)
           or eax, CR0_CD         ;disable caching
           mov cr0, eax
           wbinvd                 ;invalidate cache

           xor edi, edi
           mov ebx, 'TEST'
@@:
           add edi, 0x400000
           xchg ebx, dword [edi]
           cmp dword [edi], 'TEST'
           xchg ebx, dword [edi]
           je @b
           mov [MEM_AMOUNT-OS_BASE], edi

           and eax, not (CR0_CD+CR0_NW)  ;enable caching
           mov cr0, eax
           mov eax, edi
           ret
endp

align 4
proc init_mem

           mov eax, [MEM_AMOUNT-OS_BASE]
           mov [pg_data.mem_amount-OS_BASE], eax
           mov [pg_data.kernel_max-OS_BASE], eax

           shr eax, 12
           mov edx, eax
           mov [pg_data.pages_count-OS_BASE], eax
           mov [pg_data.kernel_pages-OS_BASE], eax

           shr eax, 3
           mov [pg_data.pagemap_size-OS_BASE], eax

           shr edx, 10
           cmp edx, 3
           ja @f
           inc edx       ;at least 4Mb for kernel heap
@@:
           mov [pg_data.kernel_tables-OS_BASE], edx

           xor eax, eax
           mov edi, sys_pgdir-OS_BASE
           mov ecx, 2048
           cld
           rep stosd

           mov edx, sys_pgdir-OS_BASE
;           bt [cpu_caps], CAPS_PSE
;           jnc .no_PSE

           mov ebx, cr4
           or ebx, CR4_PSE
           mov eax, PG_LARGE+PG_SW
           bt [cpu_caps-OS_BASE], CAPS_PGE
           jnc @F
           or eax, PG_GLOBAL
           or ebx, CR4_PGE
@@:
           mov cr4, ebx
           sub [pg_data.kernel_tables-OS_BASE], 2

           mov [edx], eax
           mov [edx+16], eax
           add eax, 0x00400000

           mov [edx+4], eax
           mov [edx+20], eax
           add eax, 0x00400000

           mov [edx+8], eax
           mov [edx+24], eax
           add eax, 0x00400000

           mov [edx+12], eax
           mov [edx+28], eax
           add edx, 32
           add eax, 0x00400000

;           mov eax, 0x800000+PG_SW
;           mov ecx, (HEAP_BASE-0x800000)/4096
;           jmp .map_low
;.no_PSE:
;           mov eax, PG_SW
;           mov ecx, HEAP_BASE/4096
;.map_low:
           mov edi, tmp_page_tab
;@@:                                   ;
;           stosd
;           add eax, 0x1000
;           dec ecx
;           jnz @B

           mov ecx, [pg_data.kernel_tables-OS_BASE]
           shl ecx, 10
           xor eax, eax
           rep stosd

           mov ecx, [pg_data.kernel_tables-OS_BASE]
           mov eax, tmp_page_tab+PG_SW
           mov edi, edx

.map_kernel_tabs:

           stosd
           add eax, 0x1000
           dec ecx
           jnz .map_kernel_tabs

           mov dword [sys_pgdir-OS_BASE+(page_tabs shr 20)], sys_pgdir+PG_SW-OS_BASE
           ret
endp

align 4
proc init_page_map

           mov edi, sys_pgmap-OS_BASE
           mov ecx, (HEAP_BASE/4096)/32      ;384/4
           mov ebx, ecx
           xor eax,eax
           cld
           rep stosd

           not eax
           mov ecx, [pg_data.pagemap_size-OS_BASE]
           sub ecx, ebx
           shr ecx, 2
           rep stosd

           lea edi, [sys_pgmap-OS_BASE+ebx*4]         ;+384
           mov edx, [pg_data.pages_count-OS_BASE]
           mov ecx, [pg_data.kernel_tables-OS_BASE]
           add ecx, (HEAP_BASE/4096) and 31
           sub edx, HEAP_BASE/4096
           sub edx, ecx
           mov [pg_data.pages_free-OS_BASE], edx

           xor eax, eax
           mov ebx, ecx
           shr ecx, 5
           rep stosd

           not eax
           mov ecx, ebx
           and ecx, 31
           shl eax, cl
           add edi, OS_BASE
           mov [page_start-OS_BASE], edi;     sys_pgmap+384
           stosd

           mov ebx, sys_pgmap
           add ebx, [pg_data.pagemap_size-OS_BASE]
           mov [page_end-OS_BASE], ebx

           mov [pg_data.pg_mutex-OS_BASE], 0

           ret
endp

align 4
proc test_cpu
           locals
              cpu_type   dd ?
              cpu_id     dd ?
              cpu_Intel  dd ?
              cpu_AMD    dd ?
           endl

           mov [cpu_type], 0
           xor eax, eax
           mov [cpu_caps-OS_BASE], eax
           mov [cpu_caps+4-OS_BASE], eax

           pushfd
           pop eax
           mov ecx, eax
           xor eax, 0x40000
           push eax
           popfd
           pushfd
           pop eax
           xor eax, ecx
           mov [cpu_type], CPU_386
           jz .end_cpuid
           push ecx
           popfd

           mov [cpu_type], CPU_486
           mov eax, ecx
           xor eax, 0x200000
           push eax
           popfd
           pushfd
           pop eax
           xor eax, ecx
           je .end_cpuid
           mov [cpu_id], 1

           xor eax, eax
           cpuid

           mov [cpu_vendor-OS_BASE], ebx
           mov [cpu_vendor+4-OS_BASE], edx
           mov [cpu_vendor+8-OS_BASE], ecx
           cmp ebx, dword [intel_str-OS_BASE]
           jne .check_AMD
           cmp edx, dword [intel_str+4-OS_BASE]
           jne .check_AMD
           cmp ecx, dword [intel_str+8-OS_BASE]
           jne .check_AMD
           mov [cpu_Intel], 1
           cmp eax, 1
           jl .end_cpuid
           mov eax, 1
           cpuid
           mov [cpu_sign-OS_BASE], eax
           mov [cpu_info-OS_BASE],  ebx
           mov [cpu_caps-OS_BASE],  edx
           mov [cpu_caps+4-OS_BASE],ecx

           shr eax, 8
           and eax, 0x0f
           ret
.end_cpuid:
           mov eax, [cpu_type]
           ret

.check_AMD:
           cmp ebx, dword [AMD_str-OS_BASE]
           jne .unknown
           cmp edx, dword [AMD_str+4-OS_BASE]
           jne .unknown
           cmp ecx, dword [AMD_str+8-OS_BASE]
           jne .unknown
           mov [cpu_AMD], 1
           cmp eax, 1
           jl .unknown
           mov eax, 1
           cpuid
           mov [cpu_sign-OS_BASE], eax
           mov [cpu_info-OS_BASE],  ebx
           mov [cpu_caps-OS_BASE],  edx
           mov [cpu_caps+4-OS_BASE],ecx
           shr eax, 8
           and eax, 0x0f
           ret
.unknown:
           mov eax, 1
           cpuid
           mov [cpu_sign-OS_BASE], eax
           mov [cpu_info-OS_BASE],  ebx
           mov [cpu_caps-OS_BASE],  edx
           mov [cpu_caps+4-OS_BASE],ecx
           shr eax, 8
           and eax, 0x0f
           ret
endp

