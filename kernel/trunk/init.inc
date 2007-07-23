$Revision$

MEM_WB     equ 6               ;write-back memory
MEM_WC     equ 1               ;write combined memory
MEM_UC     equ 0               ;uncached memory

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
           add edi, 0x100000
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

           shr eax, 12
           mov edx, eax
           mov [pg_data.pages_count-OS_BASE], eax
           shr eax, 3
           mov [pg_data.pagemap_size-OS_BASE], eax

           add eax, (sys_pgmap-OS_BASE)+4095
           and eax, not 4095
           mov [tmp_page_tabs], eax

           cmp edx, (OS_BASE/4096)
           jbe @F
           mov edx, (OS_BASE/4096)
           jmp .set
@@:
           cmp edx, (HEAP_MIN_SIZE/4096)
           jae .set
           mov edx, (HEAP_MIN_SIZE/4096)
.set:
           mov [pg_data.kernel_pages-OS_BASE], edx
           shr edx, 10
           mov [pg_data.kernel_tables-OS_BASE], edx

           xor eax, eax
           mov edi, sys_pgdir-OS_BASE
           mov ecx, 4096/4
           cld
           rep stosd

           mov edx, (sys_pgdir-OS_BASE)+ 0x800; (OS_BASE shr 20)
           bt [cpu_caps-OS_BASE], CAPS_PSE
           jnc .no_PSE

           mov ebx, cr4
           or ebx, CR4_PSE
           mov eax, PG_LARGE+PG_SW
           mov cr4, ebx
           dec [pg_data.kernel_tables-OS_BASE]

           mov [edx], eax
           add eax, 0x00400000
           add edx, 4

           mov eax, 0x400000+PG_SW
           mov ecx, [tmp_page_tabs]
           sub ecx, 0x400000
           shr ecx, 12          ;ecx/=4096
           jmp .map_low
.no_PSE:
           mov eax, PG_SW
           mov ecx, [tmp_page_tabs]
           shr ecx, 12
.map_low:
           mov edi, [tmp_page_tabs]
@@:                                   ;
           stosd
           add eax, 0x1000
           dec ecx
           jnz @B

           mov ecx, [pg_data.kernel_tables-OS_BASE]
           shl ecx, 10
           xor eax, eax
           rep stosd

           mov ecx, [pg_data.kernel_tables-OS_BASE]
           mov eax, [tmp_page_tabs]
           or eax, PG_SW
           mov edi, edx

.map_kernel_tabs:

           stosd
           add eax, 0x1000
           dec ecx
           jnz .map_kernel_tabs

           mov dword [sys_pgdir-OS_BASE+(page_tabs shr 20)], sys_pgdir+PG_SW-OS_BASE

           mov edi, (sys_pgdir-OS_BASE)
           lea esi, [edi+(OS_BASE shr 20)]
           movsd
           movsd
           ret
endp

align 4
proc init_page_map

           mov edi, sys_pgmap-OS_BASE
           mov ecx, [pg_data.pagemap_size-OS_BASE]
           shr ecx, 2
           or eax, -1
           cld
           rep stosd

           mov ecx, [tmp_page_tabs]
           mov edx, [pg_data.pages_count-OS_BASE]
           shr ecx, 12
           add ecx, [pg_data.kernel_tables-OS_BASE]
           sub edx, ecx
           mov [pg_data.pages_free-OS_BASE], edx

           mov edi, sys_pgmap-OS_BASE
           mov ebx, ecx
           shr ecx, 5
           xor eax, eax
           rep stosd

           not eax
           mov ecx, ebx
           and ecx, 31
           shl eax, cl
           mov [edi], eax
           add edi, OS_BASE
           mov [page_start-OS_BASE], edi;

           mov ebx, sys_pgmap
           add ebx, [pg_data.pagemap_size-OS_BASE]
           mov [page_end-OS_BASE], ebx

           mov [pg_data.pg_mutex-OS_BASE], 0
           ret
endp

align 4

init_BIOS32:
           mov edi, 0xE0000
.pcibios_nxt:
           cmp dword[edi], '_32_' ; "magic" word
           je .BIOS32_found
.pcibios_nxt2:
           add edi, 0x10
           cmp edi, 0xFFFF0
           je .BIOS32_not_found
           jmp .pcibios_nxt
.BIOS32_found:			; magic word found, check control summ

           movzx ecx, byte[edi + 9]
           shl ecx, 4
           mov esi, edi
           xor eax, eax
           cld   ; paranoia
@@:	lodsb
           add ah, al
           loop @b
           jnz .pcibios_nxt2 ; control summ must be zero
    ; BIOS32 service found !
           mov ebp, [edi + 4]
           mov [bios32_entry], ebp
    ; check PCI BIOS present
           mov eax, '$PCI'
           xor ebx, ebx
           push cs  ; special for 'ret far' from  BIOS
           call ebp
           test al, al
           jnz .PCI_BIOS32_not_found

 ; здесь создаются дискрипторы для PCI BIOS

           add ebx, OS_BASE
           dec ecx
           mov [(pci_code_32-OS_BASE)], cx    ;limit 0-15
           mov [(pci_data_32-OS_BASE)], cx    ;limit 0-15

           mov [(pci_code_32-OS_BASE)+2], bx  ;base  0-15
           mov [(pci_data_32-OS_BASE)+2], bx  ;base  0-15

           shr ebx, 16
           mov [(pci_code_32-OS_BASE)+4], bl  ;base  16-23
           mov [(pci_data_32-OS_BASE)+4], bl  ;base  16-23

           shr ecx, 16
           and cl, 0x0F
           mov ch, bh
           add cx, D32
           mov [(pci_code_32-OS_BASE)+6], cx  ;lim   16-19 &
           mov [(pci_data_32-OS_BASE)+6], cx  ;base  24-31

           mov [(pci_bios_entry-OS_BASE)], edx
         ; jmp .end
.PCI_BIOS32_not_found:
	; здесь должна заполнятся pci_emu_dat
.BIOS32_not_found:
.end:


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

