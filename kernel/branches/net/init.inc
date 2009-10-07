;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 983 $


MEM_WB     equ 6               ;write-back memory
MEM_WC     equ 1               ;write combined memory
MEM_UC     equ 0               ;uncached memory

align 4
proc mem_test
; if we have BIOS with fn E820, skip the test
           cmp dword [BOOT_VAR-OS_BASE + 0x9100], 0
           jnz .ret

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

           and eax, not (CR0_CD+CR0_NW)  ;enable caching
           mov cr0, eax
           inc dword [BOOT_VAR-OS_BASE + 0x9100]
           xor eax, eax
           mov [BOOT_VAR-OS_BASE + 0x9104], eax
           mov [BOOT_VAR-OS_BASE + 0x9108], eax
           mov [BOOT_VAR-OS_BASE + 0x910C], edi
           mov [BOOT_VAR-OS_BASE + 0x9110], eax
.ret:
           ret
endp

align 4
proc init_mem
; calculate maximum allocatable address and number of allocatable pages
           mov edi, BOOT_VAR-OS_BASE + 0x9104
           mov ecx, [edi-4]
           xor esi, esi ; esi will hold total amount of memory
           xor edx, edx ; edx will hold maximum allocatable address
.calcmax:
; round all to pages
           mov eax, [edi]
           test eax, 0xFFF
           jz @f
           neg eax
           and eax, 0xFFF
           add [edi], eax
           adc dword [edi+4], 0
           sub [edi+8], eax
           sbb dword [edi+12], 0
           jc .unusable
@@:
           and dword [edi+8], not 0xFFF
           jz .unusable
; ignore memory after 4 Gb
           cmp dword [edi+4], 0
           jnz .unusable
           mov eax, [edi]
           cmp dword [edi+12], 0
           jnz .overflow
           add eax, [edi+8]
           jnc @f
.overflow:
           mov eax, 0xFFFFF000
@@:
           cmp edx, eax
           jae @f
           mov edx, eax
@@:
           sub eax, [edi]
           mov [edi+8], eax
           add esi, eax
           jmp .usable
.unusable:
           and dword [edi+8], 0
.usable:
           add edi, 20
           loop .calcmax
.calculated:
           mov [MEM_AMOUNT-OS_BASE], esi
           mov [pg_data.mem_amount-OS_BASE], esi
           shr esi, 12
           mov [pg_data.pages_count-OS_BASE], esi

           shr edx, 12
           add edx, 31
           and edx, not 31
           shr edx, 3
           mov [pg_data.pagemap_size-OS_BASE], edx

           add edx, (sys_pgmap-OS_BASE)+4095
           and edx, not 4095
           mov [tmp_page_tabs], edx

           mov edx, esi
           and edx, -1024
           cmp edx, (OS_BASE/4096)
           jbe @F
           mov edx, (OS_BASE/4096)
           jmp .set
@@:
           cmp edx, (HEAP_BASE+HEAP_MIN_SIZE)/4096
           jae .set
           mov edx, (HEAP_BASE+HEAP_MIN_SIZE)/4096
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
; mark all memory as unavailable
           mov edi, sys_pgmap-OS_BASE
           mov ecx, [pg_data.pagemap_size-OS_BASE]
           shr ecx, 2
           xor eax, eax
           cld
           rep stosd

; scan through memory map and mark free areas as available
           mov ebx, BOOT_VAR-OS_BASE + 0x9104
           mov edx, [ebx-4]
.scanmap:
           mov ecx, [ebx+8]
           shr ecx, 12 ; ecx = number of pages
           jz .next
           mov edi, [ebx]
           shr edi, 12 ; edi = first page
           mov eax, edi
           neg eax
           shr edi, 5
           add edi, sys_pgmap-OS_BASE
           and eax, 31
           jz .startok
           sub ecx, eax
           jbe .onedword
           push ecx
           mov ecx, eax
           xor eax, eax
           inc eax
           shl eax, cl
           dec eax
           or [edi], eax
           add edi, 4
           pop ecx
.startok:
           push ecx
           shr ecx, 5
           or eax, -1
           rep stosd
           pop ecx
           and ecx, 31
           not eax
           shl eax, cl
           or [edi], eax
           jmp .next
.onedword:
           add ecx, eax
@@:
           dec eax
           bts [edi], eax
           loop @b
.next:
           add ebx, 20
           dec edx
           jnz .scanmap

; mark kernel memory as allocated (unavailable)
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
           and [edi], eax
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
           ret	

align 4
proc test_cpu
           locals
              cpu_type   dd ?
              cpu_id     dd ?
              cpu_Intel  dd ?
              cpu_AMD    dd ?
           endl

           xor eax, eax
           mov [cpu_type], eax
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

