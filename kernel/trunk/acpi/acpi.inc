;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; ACPI Generic Address Structure
struct GAS
        ASID       db ? ; address space id
        BitWidth   db ?
        BitOffset  db ?
        AccessSize db ?
        Address    DQ ?
ends

ASID.SYSTEM_MEMORY = 0
ASID.SYSTEM_IO     = 1
ASID.PCI_CONFIG    = 2
ASID.PCI_EC        = 3
ASID.PCI_SMBUS     = 4

ACCESS_SIZE.UNDEFINED = 0
ACCESS_SIZE.BYTE      = 1
ACCESS_SIZE.WORD      = 2
ACCESS_SIZE.DWORD     = 3
ACCESS_SIZE.QWORD     = 4


struct ACPI_RSDP
    Signature   DQ ?
    Checksum    db ?
    OEMID       rb 6
    Revision    db ?
    RsdtAddress dd ?
    ; for Revision >= 2
    Length      dd ?
    XsdtAddress DQ ?
    ExtChecksum db ?
    Reserved    rb 3
ends

struct ACPI_TABLE       ; DESCRIPTION_HEADER
    Signature       dd ?
    Length          dd ?
    Revision        db ?
    Checksum        db ?
    OEMID           rb 6
    OEMTableID      rb 8
    OEMRevision     rb 4
    CreatorID       rb 4
    CreatorRevision rb 4
ends

struct ACPI_RSDT ACPI_TABLE
    Entry           rd (0x1000-sizeof.ACPI_TABLE)/4
ends

struct ACPI_HPET ACPI_TABLE
    ID                  dd ?
    Base                GAS
    SeqNumber           db ?
    MainCounterMinimum  dw ?
    PageProtectionOEM   db ?
ends

struct ACPI_MADT ACPI_TABLE
    Local_IC_Addr dd ?
    Flags         dd ?
    IntController rb 0x1000-sizeof.ACPI_TABLE-ACPI_MADT.IntController
ends

struct ACPI_FADT ACPI_TABLE
    FirmwareCtrl        dd ?
    DSDT                dd ?
                        db ?
    PreferredPMProfile  db ?
    SCI_INT             dw ?
    SMI_CMD             dd ?
    ACPI_ENABLE         db ?
    ACPI_DISABLE        db ?
    S4BIOS_REQ          db ?
    PSTATE_CNT          db ?
    PM1a_EVT_BLK        dd ?
    PM1b_EVT_BLK        dd ?
    PM1a_CNT_BLK        dd ?
    PM1b_CNT_BLK        dd ?
    PM2_CNT_BLK         dd ?
    PM_TMR_BLK          dd ?
    GPE0_BLK            dd ?
    GPE1_BLK            dd ?
    PM1_EVT_LEN         db ?
    PM1_CNT_LEN         db ?
    PM2_CNT_LEN         db ?
    PM_TMR_LEN          db ?
    GPE0_BLK_LEN        db ?
    GPE1_BLK_LEN        db ?
    GPE1_BASE           db ?
    CST_CNT             db ?
    P_LVL2_LAT          dw ?
    P_LVL3_LAT          dw ?
    FLUSH_SIZE          dw ?
    FLUSH_STRIDE        dw ?
    DUTY_OFFSET         db ?
    DUTY_WIDTH          db ?
    DAY_ALRM            db ?
    MON_ALRM            db ?
    CENTURY             db ?
    IAPC_BOOT_ARCH      dw ?
                        db ?
    Flags               dd ?
    RESET_REG           GAS
    RESET_VALUE         db ?
    ARM_BOOT_ARCH       dw ?
    FADT_Minor_Version  db ?
    X_FIRMWARE_CTRL     DQ ?
    X_DSDT              DQ ?
    X_PM1a_EVT_BLK      GAS
    X_PM1b_EVT_BLK      GAS
    X_PM1a_CNT_BLK      GAS
    X_PM1b_CNT_BLK      GAS
    X_PM2_CNT_BLK       GAS
    X_PM_TMR_BLK        GAS
    X_GPE0_BLK          GAS
    X_GPE1_BLK          GAS
    SLEEP_CONTROL_REG   GAS
    SLEEP_STATUS_REG    GAS
    HypervisorVendorID  rb 8
ends

MAX_SSDTS = 32

iglobal
align 4
acpi_lapic_base   dd 0xfee00000   ; default local apic base
endg

uglobal
align 4
acpi_dev_data     rd 1
acpi_dev_size     rd 1

acpi_rsdp_base    rd 1
acpi_rsdt_base    rd 1
acpi_rsdt_size    rd 1
acpi_fadt_base    rd 1
acpi_fadt_size    rd 1
acpi_ssdt_base    rd MAX_SSDTS
acpi_ssdt_size    rd MAX_SSDTS
acpi_ssdt_cnt     rd 1
acpi_madt_base    rd 1
acpi_madt_size    rd 1
acpi_ioapic_base  rd MAX_IOAPICS
acpi_hpet_base    rd 1
acpi_hpet_size    rd 1
cpu_count         rd 1
smpt              rd 16
endg

align 4
; @returns ACPI Root System Description Pointer
acpi_get_root_ptr:
        mov     eax, [acpi_rsdp_base]
        ret

align 4
rsdt_find:           ;ecx= rsdt edx= SIG
        push    ebx
        push    esi

        lea     ebx, [ecx+ACPI_RSDT.Entry]
        mov     esi, [ecx+ACPI_RSDT.Length]
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
        cmp     [acpi_rsdp_base], 0
        jz      .done
        stdcall map_io_mem, [acpi_rsdp_base], sizeof.ACPI_RSDP, \
                PG_GLOBAL+PAT_WB+PG_READ
        mov     [acpi_rsdp_base], eax
.rsdp_done:
        cmp     [acpi_rsdt_base], 0
        jz      .rsdt_done
        stdcall map_io_mem, [acpi_rsdt_base], [acpi_rsdt_size], \
                PG_GLOBAL+PAT_WB+PG_READ
        mov     [acpi_rsdt_base], eax
.rsdt_done:
        cmp     [acpi_fadt_base], 0
        jz      .fadt_done
        stdcall map_io_mem, [acpi_fadt_base], [acpi_fadt_size], \
                PG_GLOBAL+PAT_WB+PG_READ
        mov     [acpi_fadt_base], eax
.fadt_done:
        cmp     [acpi_hpet_base], 0
        jz      .hpet_done
        stdcall map_io_mem, [acpi_hpet_base], [acpi_hpet_size], \
                PG_GLOBAL+PAT_WB+PG_READ
        mov     [acpi_hpet_base], eax
        mov     eax, [eax+ACPI_HPET.Base.Address.lo]
        mov     [hpet_base], eax
.hpet_done:
        cmp     [acpi_madt_base], 0
        jz      .madt_done
        stdcall map_io_mem, [acpi_madt_base], [acpi_madt_size], \
                PG_GLOBAL+PAT_WB+PG_READ
        mov     [acpi_madt_base], eax

        mov     ecx, [eax+ACPI_MADT.Local_IC_Addr]
        mov     [acpi_lapic_base], ecx
        push    eax
        stdcall map_io_mem, ecx, 0x1000, PG_GLOBAL+PG_NOCACHE+PG_SWR
        mov     [LAPIC_BASE], eax
        mov     ecx, eax
        pop     eax

        mov     edi, smpt
        mov     ebx, [ecx+APIC_ID]
        shr     ebx, 24              ; read APIC ID

        mov     [edi], ebx           ; bootstrap always first
        inc     [cpu_count]
        add     edi, 4

        mov     [ioapic_cnt], 0
        lea     edx, [eax+ACPI_MADT.IntController]
        mov     ecx, [eax+ACPI_MADT.Length]
        add     ecx, eax
.check:
        mov     eax, [edx]
        cmp     al, 0
        je      .lapic
        cmp     al, 1
        je      .io_apic
        jmp     .next
.lapic:
        shr     eax, 24              ; get APIC ID
        cmp     eax, ebx             ; skip self
        je      .next

        test    [edx+4], byte 1      ; is enabled ?
        jz      .next

        cmp     [cpu_count], 16
        jae     .next

        stosd                        ; store APIC ID
        inc     [cpu_count]
        jmp     .next

.io_apic:
        mov     eax, [ioapic_cnt]
        push    dword[edx+4]
        pop     [acpi_ioapic_base+eax*4]
        push    dword[edx+8]
        pop     [ioapic_gsi_base+eax*4]
        inc     [ioapic_cnt]
        jmp     .next

.next:
        mov     eax, [edx]
        movzx   eax, ah
        add     edx, eax
        cmp     edx, ecx
        jb      .check
.madt_done:

        xor     ecx, ecx
.next_ssdt:
        cmp     ecx, [acpi_ssdt_cnt]
        jz      .ssdt_done
        push    ecx
        stdcall map_io_mem, [acpi_ssdt_base+ecx*4], [acpi_ssdt_size+ecx*4], \
                PG_GLOBAL+PAT_WB+PG_READ
        pop     ecx
        mov     [acpi_ssdt_base+ecx*4], eax
        inc     ecx
        jmp     .next_ssdt
.ssdt_done:

.done:
        ret
