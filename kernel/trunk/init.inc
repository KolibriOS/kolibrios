;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


MEM_WB     equ 6               ;write-back memory
MEM_WC     equ 1               ;write combined memory
MEM_UC     equ 0               ;uncached memory

align 4
proc mem_test
; if we have BIOS with fn E820, skip the test
        cmp     dword [BOOT_LO.memmap_block_cnt], 0
        jnz     .ret

        mov     eax, cr0
        and     eax, not (CR0_CD+CR0_NW)
        or      eax, CR0_CD       ;disable caching
        mov     cr0, eax
        wbinvd                    ;invalidate cache

        xor     edi, edi
        mov     ebx, 'TEST'
@@:
        add     edi, 0x100000
        xchg    ebx, dword [edi]
        cmp     dword [edi], 'TEST'
        xchg    ebx, dword [edi]
        je      @b

        and     eax, not (CR0_CD+CR0_NW) ;enable caching
        mov     cr0, eax
        inc     dword [BOOT_LO.memmap_block_cnt]
        xor     eax, eax
        mov     [BOOT_LO.memmap_blocks + e820entry.addr.lo], eax
        mov     [BOOT_LO.memmap_blocks + e820entry.addr.hi], eax
        mov     [BOOT_LO.memmap_blocks + e820entry.size.lo], edi
        mov     [BOOT_LO.memmap_blocks + e820entry.size.hi], eax
        inc     eax
        mov     [BOOT_LO.memmap_blocks + e820entry.type], eax
.ret:
        ret
endp

align 4
proc init_mem
; calculate maximum allocatable address and number of allocatable pages
        mov     edi, BOOT_LO.memmap_blocks
        mov     ecx, [edi-4]
        xor     esi, esi; esi will hold total amount of memory
        xor     edx, edx; edx will hold maximum allocatable address
.calcmax:
; round all to pages
        mov     eax, [edi]
        cmp     [edi+16], byte 1
        jne     .unusable

        test    eax, 0xFFF
        jz      @f
        neg     eax
        and     eax, 0xFFF
        add     [edi], eax
        adc     dword [edi+4], 0
        sub     [edi+8], eax
        sbb     dword [edi+12], 0
        jc      .unusable
@@:
        and     dword [edi+8], not 0xFFF
        jz      .unusable
; ignore memory after 4 Gb
        cmp     dword [edi+4], 0
        jnz     .unusable
        mov     eax, [edi]
        cmp     dword [edi+12], 0
        jnz     .overflow
        add     eax, [edi+8]
        jnc     @f
.overflow:
        mov     eax, 0xFFFFF000
@@:
        cmp     edx, eax
        jae     @f
        mov     edx, eax
@@:
        sub     eax, [edi]
        mov     [edi+8], eax
        add     esi, eax
        jmp     .usable
.unusable:
;        and     dword [edi+8], 0
.usable:
        add     edi, 20
        loop    .calcmax
.calculated:
        mov     [MEM_AMOUNT-OS_BASE], esi
        mov     [pg_data.mem_amount-OS_BASE], esi
        shr     esi, 12
        mov     [pg_data.pages_count-OS_BASE], esi

        shr     edx, 12
        add     edx, 31
        and     edx, not 31
        shr     edx, 3
        mov     [pg_data.pagemap_size-OS_BASE], edx

        add     edx, (sys_pgmap-OS_BASE)+4095
        and     edx, not 4095
        mov     [tmp_page_tabs], edx

        mov     edx, esi
        and     edx, -1024
        cmp     edx, (OS_BASE/4096)
        jbe     @F
        mov     edx, (OS_BASE/4096)
        jmp     .set
@@:
        cmp     edx, (HEAP_BASE-OS_BASE+HEAP_MIN_SIZE)/4096
        jae     .set
        mov     edx, (HEAP_BASE-OS_BASE+HEAP_MIN_SIZE)/4096
.set:
        mov     [pg_data.kernel_pages-OS_BASE], edx
        shr     edx, 10
        mov     [pg_data.kernel_tables-OS_BASE], edx

        xor     eax, eax
        mov     edi, sys_proc-OS_BASE
        mov     ecx, 8192/4
        cld
        rep stosd

        mov     edx, (sys_proc-OS_BASE+PROC.pdt_0)+ 0x800; (OS_BASE shr 20)
        bt      [cpu_caps-OS_BASE], CAPS_PSE
        jnc     .no_PSE

        mov     ebx, cr4
        or      ebx, CR4_PSE
        mov     eax, PDE_LARGE+PG_SWR
        mov     cr4, ebx
        dec     [pg_data.kernel_tables-OS_BASE]

        mov     [edx], eax
        add     edx, 4

        mov     edi, [tmp_page_tabs]
        jmp     .map_kernel_heap        ; new kernel fits to the first 4Mb - nothing to do with ".map_low"
.no_PSE:
        mov     eax, PG_SWR
        mov     ecx, [tmp_page_tabs]
        shr     ecx, 12
.map_low:
        mov     edi, [tmp_page_tabs]
@@:                                   ;
        stosd
        add     eax, 0x1000
        dec     ecx
        jnz     @B

.map_kernel_heap:
        mov     ecx, [pg_data.kernel_tables-OS_BASE]
        shl     ecx, 10
        xor     eax, eax
        rep stosd

        mov     ecx, [pg_data.kernel_tables-OS_BASE]
        mov     eax, [tmp_page_tabs]
        or      eax, PG_SWR
        mov     edi, edx

.map_kernel_tabs:
        stosd
        add     eax, 0x1000
        dec     ecx
        jnz     .map_kernel_tabs

        mov     dword [sys_proc-OS_BASE+PROC.pdt_0+(page_tabs shr 20)], sys_proc+PROC.pdt_0+PG_SWR-OS_BASE

        mov     edi, (sys_proc+PROC.pdt_0-OS_BASE)
        lea     esi, [edi+(OS_BASE shr 20)]
        movsd
        movsd
        ret
endp

align 4
proc init_page_map
; mark all memory as unavailable
        mov     edi, sys_pgmap-OS_BASE
        mov     ecx, [pg_data.pagemap_size-OS_BASE]
        shr     ecx, 2
        xor     eax, eax
        cld
        rep stosd

; scan through memory map and mark free areas as available
        mov     ebx, BOOT_LO.memmap_blocks
        mov     edx, [ebx-4]
.scanmap:
        cmp     [ebx+16], byte 1
        jne     .next

        mov     ecx, [ebx+8]
        shr     ecx, 12; ecx = number of pages
        jz      .next
        mov     edi, [ebx]
        shr     edi, 12; edi = first page
        mov     eax, edi
        shr     edi, 5
        shl     edi, 2
        add     edi, sys_pgmap-OS_BASE
        and     eax, 31
        jz      .startok
        add     ecx, eax
        sub     ecx, 32
        jbe     .onedword
        push    ecx
        mov     ecx, eax
        or      eax, -1
        shl     eax, cl
        or      [edi], eax
        add     edi, 4
        pop     ecx
.startok:
        push    ecx
        shr     ecx, 5
        or      eax, -1
        rep stosd
        pop     ecx
        and     ecx, 31
        neg     eax
        shl     eax, cl
        dec     eax
        or      [edi], eax
        jmp     .next
.onedword:
        add     ecx, 32
        sub     ecx, eax
@@:
        bts     [edi], eax
        inc     eax
        loop    @b
.next:
        add     ebx, 20
        dec     edx
        jnz     .scanmap

; mark kernel memory as allocated (unavailable)
        mov     ecx, [tmp_page_tabs]
        mov     edx, [pg_data.pages_count-OS_BASE]
        shr     ecx, 12
        add     ecx, [pg_data.kernel_tables-OS_BASE]
        sub     edx, ecx
        mov     [pg_data.pages_free-OS_BASE], edx

        mov     edi, sys_pgmap-OS_BASE
        mov     ebx, ecx
        shr     ecx, 5
        xor     eax, eax
        rep stosd

        not     eax
        mov     ecx, ebx
        and     ecx, 31
        shl     eax, cl
        and     [edi], eax
        add     edi, OS_BASE
        mov     [page_start-OS_BASE], edi;

        mov     ebx, sys_pgmap
        add     ebx, [pg_data.pagemap_size-OS_BASE]
        mov     [page_end-OS_BASE], ebx

        ret
endp

align 4

init_BIOS32:
        mov     edi, 0xE0000
.pcibios_nxt:
        cmp     dword[edi], '_32_'; "magic" word
        je      .BIOS32_found
.pcibios_nxt2:
        add     edi, 0x10
        cmp     edi, 0xFFFF0
        je      .BIOS32_not_found
        jmp     .pcibios_nxt
.BIOS32_found:                  ; magic word found, check control summ

        movzx   ecx, byte[edi + 9]
        shl     ecx, 4
        mov     esi, edi
        xor     eax, eax
        cld      ; paranoia
@@:
        lodsb
        add     ah, al
        loop    @b
        jnz     .pcibios_nxt2; control summ must be zero
    ; BIOS32 service found !
        mov     ebp, [edi + 4]
        mov     [bios32_entry], ebp
    ; check PCI BIOS present
        mov     eax, '$PCI'
        xor     ebx, ebx
        push    cs  ; special for 'ret far' from  BIOS
        call    ebp
        test    al, al
        jnz     .PCI_BIOS32_not_found

 ; здесь создаются дискрипторы для PCI BIOS

        add     ebx, OS_BASE
        dec     ecx
        mov     [(pci_code_32-OS_BASE)], cx   ;limit 0-15
        mov     [(pci_data_32-OS_BASE)], cx   ;limit 0-15

        mov     [(pci_code_32-OS_BASE)+2], bx ;base  0-15
        mov     [(pci_data_32-OS_BASE)+2], bx ;base  0-15

        shr     ebx, 16
        mov     [(pci_code_32-OS_BASE)+4], bl ;base  16-23
        mov     [(pci_data_32-OS_BASE)+4], bl ;base  16-23

        shr     ecx, 16
        and     cl, 0x0F
        mov     ch, bh
        add     cx, D32
        mov     [(pci_code_32-OS_BASE)+6], cx ;lim   16-19 &
        mov     [(pci_data_32-OS_BASE)+6], cx ;base  24-31

        mov     [(pci_bios_entry-OS_BASE)], edx
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
           endl

        xor     eax, eax
        mov     [cpu_type], eax
        mov     [cpu_caps-OS_BASE], eax
        mov     [cpu_caps+4-OS_BASE], eax
        mov     [cpu_phys_addr_width-OS_BASE], 32

        pushfd
        pop     eax
        mov     ecx, eax
        xor     eax, EFLAGS_AC
        push    eax
        popfd
        pushfd
        pop     eax
        xor     eax, ecx
        mov     [cpu_type], CPU_386
        jz      .end_cpuid
        push    ecx
        popfd

        mov     [cpu_type], CPU_486
        mov     eax, ecx
        xor     eax, EFLAGS_ID
        push    eax
        popfd
        pushfd
        pop     eax
        xor     eax, ecx
        je      .end_cpuid

        xor     eax, eax
        cpuid

        mov     [cpu_vendor-OS_BASE], ebx
        mov     [cpu_vendor+4-OS_BASE], edx
        mov     [cpu_vendor+8-OS_BASE], ecx

        cmp     eax, 1
        jl      .end_cpuid
        mov     eax, 1
        cpuid
        mov     [cpu_sign-OS_BASE], eax
        mov     [cpu_info-OS_BASE], ebx
        mov     [cpu_caps-OS_BASE], edx
        mov     [cpu_caps+4-OS_BASE], ecx

        bt      edx, CAPS_PAE
        jnc     @f
        mov     [cpu_phys_addr_width-OS_BASE], 36
@@:
        mov     eax, 0x80000000
        cpuid
        cmp     eax, 0x80000008
        jb      @f
        mov     eax, 0x80000008
        cpuid
        mov     [cpu_phys_addr_width-OS_BASE], al
@@:

        mov     eax, [cpu_sign-OS_BASE]
        shr     eax, 8
        and     eax, 0x0f
        ret
.end_cpuid:
        mov     eax, [cpu_type]
        ret
endp

iglobal
align 4
acpi_lapic_base   dd 0xfee00000   ; default local apic base
endg

uglobal
align 4
acpi_rsdp         rd 1
acpi_rsdt         rd 1
acpi_madt         rd 1

acpi_dev_data     rd 1
acpi_dev_size     rd 1

acpi_rsdt_base    rd 1
acpi_fadt_base    rd 1
acpi_dsdt_base    rd 1
acpi_dsdt_size    rd 1
acpi_madt_base    rd 1
acpi_ioapic_base  rd 1
acpi_hpet_base    rd 1
hpet_base         rd 1
hpet_period       rd 1
hpet_timers       rd 1
hpet_tsc_start    rd 2
cpu_count         rd 1
smpt              rd 16
endg

ACPI_HI_RSDP_WINDOW_START  equ 0x000E0000
ACPI_HI_RSDP_WINDOW_END    equ 0x00100000
ACPI_RSDP_CHECKSUM_LENGTH  equ 20

ACPI_HPET_SIGN             equ 'HPET'
ACPI_MADT_SIGN             equ 'APIC'
ACPI_FADT_SIGN             equ 'FACP'


acpi_locate:
        push    ebx
        push    edi

if defined UEFI
        ; UEFI loader knows where RSDP is
        mov     ebx, [BOOT_LO.acpi_rsdp]
        test    ebx, ebx
        jz      .done
        call    .check
else
        movzx   ebx, word [0x40E]
        shl     ebx, 4
        lea     ecx, [ebx+1024]
        call    .check

        test    ebx, ebx
        jz      @F
        jmp     .done

@@:
        mov     ebx, ACPI_HI_RSDP_WINDOW_START
        mov     edi, ACPI_HI_RSDP_WINDOW_END
        call    .check
end if
.done:
        mov     eax, ebx
        pop     edi
        pop     ebx
        ret

.check:
        cmp     [ebx], dword 'RSD '
        jne     .next
        cmp     [ebx+4], dword 'PTR '
        jne     .next

        mov     edx, ebx
        mov     ecx, ACPI_RSDP_CHECKSUM_LENGTH
        xor     eax, eax
.sum:
        add     al, [edx]
        inc     edx
        loop    .sum

        test    al, al
        jnz     .next
        ret
.next:
        add     ebx, 16
        cmp     ebx, edi
        jb      .check
        xor     ebx, ebx
        ret

align 4
rsdt_find:           ;ecx= rsdt edx= SIG
        push    ebx
        push    esi

        lea     ebx, [ecx+36]
        mov     esi, [ecx+4]
        add     esi, ecx
align 4
.next:
        mov     eax, [ebx]
        cmp     [eax], edx
        je      .done

        add     ebx, 4
        cmp     ebx, esi
        jb      .next

        xor     eax, eax
        pop     esi
        pop     ebx
        ret

.done:
        mov     eax, [ebx]
        pop     esi
        pop     ebx
        ret

align 4
check_acpi:

        call    acpi_locate
        test    eax, eax
        jz      .done

        mov     [acpi_rsdp-OS_BASE], eax
        mov     ecx, [eax+16]
        mov     edx, ACPI_FADT_SIGN
        mov     [acpi_rsdt_base-OS_BASE], ecx
        call    rsdt_find
        mov     [acpi_fadt_base-OS_BASE], eax
        test    eax, eax
        jz      @f

        mov     eax, [eax+40]
        mov     [acpi_dsdt_base-OS_BASE], eax
        mov     eax, [eax+4]
        mov     [acpi_dsdt_size-OS_BASE], eax
@@:
        mov     edx, ACPI_HPET_SIGN
        mov     ecx, [acpi_rsdt_base-OS_BASE]
        call    rsdt_find
        test    eax, eax
        jz      @F

        mov     [acpi_hpet_base-OS_BASE], eax
        mov     eax, [eax+44]
        mov     [hpet_base-OS_BASE], eax
@@:
        mov     edx, ACPI_MADT_SIGN
        mov     ecx, [acpi_rsdt_base-OS_BASE]
        call    rsdt_find
        test    eax, eax
        jz      .done

        mov     [acpi_madt_base-OS_BASE], eax
        mov     ecx, [eax+36]
        mov     [acpi_lapic_base-OS_BASE], ecx

        mov     edi, smpt-OS_BASE
        mov     ebx, [ecx+0x20]
        shr     ebx, 24              ; read APIC ID

        mov     [edi], ebx           ; bootstrap always first
        inc     [cpu_count-OS_BASE]
        add     edi, 4

        lea     edx, [eax+44]
        mov     ecx, [eax+4]
        add     ecx, eax
.check:
        mov     eax, [edx]
        cmp     al, 0
        jne     .io_apic

        shr     eax, 24              ; get APIC ID
        cmp     eax, ebx             ; skip self
        je      .next

        test    [edx+4], byte 1      ; is enabled ?
        jz      .next

        cmp     [cpu_count-OS_BASE], 16
        jae     .next

        stosd                        ; store APIC ID
        inc     [cpu_count-OS_BASE]
.next:
        mov     eax, [edx]
        movzx   eax, ah
        add     edx, eax
        cmp     edx, ecx
        jb      .check
.done:
        ret

.io_apic:
        cmp     al, 1
        jne     .next

        mov     eax, [edx+4]
        mov     [acpi_ioapic_base-OS_BASE], eax
        jmp     .next

HPET_PERIOD             equ 0x0004
HPET_CFG_ENABLE         equ 0x0001
HPET_CFG                equ 0x0010
HPET_COUNTER            equ 0x00f0
HPET_T0_CFG             equ 0x0100

HPET_TN_LEVEL           equ 0x0002
HPET_TN_ENABLE          equ 0x0004
HPET_TN_FSB             equ 0x4000

align 4
init_hpet:
        mov     ebx, [hpet_base-OS_BASE]
        test    ebx, ebx
        jz      .done

        mov     eax, [ebx]
        and     ah, 0x1F
        inc     ah
        movzx   eax, ah
        mov     [hpet_timers-OS_BASE], eax
        mov     ecx, eax

        mov     eax, [ebx+HPET_PERIOD]
        xor     edx, edx
        shld    edx, eax, 10
        shl     eax, 10
        mov     esi, 1000000
        div     esi
        mov     [hpet_period-OS_BASE], eax

        mov     esi, [ebx+HPET_CFG]
        and     esi, not HPET_CFG_ENABLE
        mov     [ebx+HPET_CFG], esi             ;stop main counter

        lea     edx, [ebx+HPET_T0_CFG]
@@:
        jcxz    @F
        mov     eax, [edx]
        and     eax, not (HPET_TN_ENABLE+HPET_TN_LEVEL+HPET_TN_FSB)
        mov     [edx], eax
        add     edx, 0x20
        dec     ecx
        jmp     @B
@@:
        mov     [ebx+HPET_COUNTER], ecx         ;reset main counter
        mov     [ebx+HPET_COUNTER+4], ecx

        or      esi, HPET_CFG_ENABLE
        mov     [ebx+HPET_CFG], esi             ;and start again

.done:
        rdtsc
        mov     [hpet_tsc_start-OS_BASE], eax
        mov     [hpet_tsc_start+4-OS_BASE], edx

        ret




