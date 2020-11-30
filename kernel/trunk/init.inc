;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

align 4
proc mem_test
; if we have BIOS with fn E820, skip the test
        cmp     [BOOT_LO.memmap_block_cnt], 0
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
        mov     [BOOT_LO.memmap_blocks+e820entry.addr.lo], eax
        mov     [BOOT_LO.memmap_blocks+e820entry.addr.hi], eax
        mov     [BOOT_LO.memmap_blocks+e820entry.size.lo], edi
        mov     [BOOT_LO.memmap_blocks+e820entry.size.hi], eax
        inc     eax
        mov     [BOOT_LO.memmap_blocks+e820entry.type], eax
.ret:
        ret
endp

align 4
proc init_mem
; calculate maximum allocatable address and number of allocatable pages
        mov     edi, BOOT_LO.memmap_blocks
        mov     ecx, [edi-4]    ; memmap_block_cnt
        xor     esi, esi; esi will hold total amount of memory
        xor     edx, edx; edx will hold maximum allocatable address
.calcmax:
; round all to pages
        mov     eax, [edi+e820entry.addr.lo]
        cmp     byte [edi+e820entry.type], 1
        jne     .unusable

        test    eax, 0xFFF
        jz      @f
        neg     eax
        and     eax, 0xFFF
        add     [edi+e820entry.addr.lo], eax
        adc     [edi+e820entry.addr.hi], 0
        sub     [edi+e820entry.size.lo], eax
        sbb     [edi+e820entry.size.hi], 0
        jc      .unusable
@@:
        and     [edi+e820entry.size.lo], not 0xFFF
        jz      .unusable
; ignore memory after 4 GiB
        cmp     [edi+e820entry.addr.hi], 0
        jnz     .unusable
        mov     eax, [edi+e820entry.addr.lo]
        cmp     [edi+e820entry.size.hi], 0
        jnz     .overflow
        add     eax, [edi+e820entry.size.lo]
        jnc     @f
.overflow:
        mov     eax, 0xFFFFF000
@@:
        cmp     edx, eax
        jae     @f
        mov     edx, eax
@@:
        sub     eax, [edi+e820entry.addr.lo]
        mov     [edi+e820entry.size.lo], eax
        add     esi, eax
        jmp     .usable
.unusable:
;        and     dword [edi+e820entry.size.lo], 0
.usable:
        add     edi, sizeof.e820entry
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
        cmp     byte [ebx+e820entry.type], 1
        jne     .next

        mov     ecx, [ebx+e820entry.size.lo]
        shr     ecx, 12; ecx = number of pages
        jz      .next
        mov     edi, [ebx+e820entry.addr.lo]
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
        add     ebx, sizeof.e820entry
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

 ; descriptors for PCI BIOS are created here

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
        ; pci_emu_dat structure should be filled here
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

ACPI_HI_RSDP_WINDOW_START  = 0x000E0000
ACPI_HI_RSDP_WINDOW_END    = 0x00100000
ACPI_RSDP_CHECKSUM_LENGTH  = 20

proc acpi_locate_tables uses ebx esi edi
        mov     ebx, [ebx+ACPI_RSDP.RsdtAddress]
        mov     [acpi_rsdt_base-OS_BASE], ebx
        mov     eax, [ebx+ACPI_RSDT.Length]
        mov     [acpi_rsdt_size-OS_BASE], eax

        mov     esi, [acpi_rsdt_base-OS_BASE]
        mov     ecx, [esi+ACPI_RSDT.Length]
        lea     edi, [esi+ecx]
        add     esi, sizeof.ACPI_TABLE
        movi    ecx, 1
.next_table:
        cmp     esi, edi
        jae     .done
        lodsd
        cmp     [eax+ACPI_TABLE.Signature], 'SSDT'      ; skip DSDT if present
        jz      .ssdt                                   ; read it from FADT
        cmp     [eax+ACPI_TABLE.Signature], 'FACP'      ; this is FADT
        jz      .fadt
        cmp     [eax+ACPI_TABLE.Signature], 'APIC'      ; this is MADT
        jz      .madt
        cmp     [eax+ACPI_TABLE.Signature], 'HPET'
        jz      .hpet
        jmp     .next_table
.ssdt:
        mov     [acpi_ssdt_base+ecx*4-OS_BASE], eax
        mov     eax, [eax+ACPI_TABLE.Length]
        mov     [acpi_ssdt_size+ecx*4-OS_BASE], eax
        inc     ecx
        jmp     .next_table
.fadt:
        mov     [acpi_fadt_base-OS_BASE], eax
        cmp     [eax+ACPI_FADT.DSDT], 0
        jz      @f
        mov     edx, [eax+ACPI_FADT.DSDT]
        mov     [acpi_ssdt_base-OS_BASE], edx
        mov     edx, [edx+ACPI_TABLE.Length]
        mov     [acpi_ssdt_size-OS_BASE], edx
@@:
        mov     eax, [eax+ACPI_TABLE.Length]
        mov     [acpi_fadt_size-OS_BASE], eax
        jmp     .next_table
.madt:
        mov     [acpi_madt_base-OS_BASE], eax
        mov     eax, [eax+ACPI_TABLE.Length]
        mov     [acpi_madt_size-OS_BASE], eax
        jmp     .next_table
.hpet:
        mov     [acpi_hpet_base-OS_BASE], eax
        mov     eax, [eax+ACPI_TABLE.Length]
        mov     [acpi_hpet_size-OS_BASE], eax
        jmp     .next_table
.done:
        mov     [acpi_ssdt_cnt-OS_BASE], ecx
        ret
endp

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
        mov     [acpi_rsdp_base-OS_BASE], ebx
        test    ebx, ebx
        jz      @f
        call    acpi_locate_tables
@@:
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
