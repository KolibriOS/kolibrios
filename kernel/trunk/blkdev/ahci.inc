;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


PCI_REG_STATUS_COMMAND = 0x0004
PCI_REG_BAR5 = 0x0024

AHCI_DBGLVL = 0 ; debug output verbosity level. 0 - less verbose, 1 - more verbose

; different SATA device signatures
SATA_SIG_ATA    = 0x00000101    ; SATA drive
SATA_SIG_ATAPI  = 0xEB140101    ; SATAPI drive
SATA_SIG_SEMB   = 0xC33C0101    ; Enclosure management bridge
SATA_SIG_PM     = 0x96690101    ; Port multiplier

; Device type constants
AHCI_DEV_NULL   = 0
AHCI_DEV_SATA   = 1
AHCI_DEV_SEMB   = 2
AHCI_DEV_PM     = 3
AHCI_DEV_SATAPI = 4

; ATA commands
ATA_IDENTIFY         = 0xEC
ATA_CMD_READ_DMA_EX  = 0x25
ATA_CMD_WRITE_DMA_EX = 0x35

; ATA constants
ATA_DEV_BUSY    = 0x80
ATA_DEV_DRQ     = 0x08

; ATAPI commands
ATAPI_IDENTIFY  = 0xA1

PRDT_MAX_ENTRIES = 16 ;65535

; bit_ prefix means that its index of bit
; format: bit_AHCI_STR_REG_BIT
bit_AHCI_HBA_CAP2_BOH   = 0        ; Supports BIOS/OS Handoff

bit_AHCI_HBA_BOHC_BOS  = 0         ; BIOS-Owned Semaphore (BIOS owns controller)
bit_AHCI_HBA_BOHC_OOS  = 1         ; OS-Owned Semaphore (OS owns controller)
bit_AHCI_HBA_BOHC_BB   = 4         ; BIOS Busy (polling bit while BIOS cleans up)

bit_AHCI_HBA_GHC_AHCI_ENABLE      = 31  ; Enable AHCI mode
bit_AHCI_HBA_GHC_RESET            = 0   ; Reset HBA
bit_AHCI_HBA_GHC_INTERRUPT_ENABLE = 1   ; Enable interrupts from the HBA

bit_AHCI_HBA_PxCMD_ST    = 0
bit_AHCI_HBA_PxCMD_FRE   = 4
bit_AHCI_HBA_PxCMD_FR    = 14
bit_AHCI_HBA_PxCMD_CR    = 15
bit_AHCI_HBA_PxIS_TFES   = 30

AHCI_HBA_PxCMD_ST    = 1 shl 0
AHCI_HBA_PxCMD_FRE   = 1 shl 4
AHCI_HBA_PxCMD_FR    = 1 shl 14
AHCI_HBA_PxCMD_CR    = 1 shl 15

bit_AHCI_H2D_FLAG_CMD    = 7

AHCI_HBA_PxSSTS_DET         = 0xF
AHCI_HBA_PORT_IPM_ACTIVE    = 1
AHCI_HBA_PxSSTS_DET_PRESENT = 3

AHCI_MAX_PORTS = 32        ;
;HBA_MEMORY_SIZE = 0x1100

AHCI_PORT_TIMEOUT = 100      ; in timer ticks (1 tick = 10 ms): 1 s to wait for BSY/DRQ clear
AHCI_CMD_TIMEOUT  = 500      ; in timer ticks: 5 s for a command to complete
AHCI_IRQ_MISS_LIMIT = 4      ; after that many completions without an interrupt fall back to polling

; Base busy-poll length. Short and medium commands (a typical SSD request is
; ~100-200 us, HDD random access is hundreds of us) are caught by polling the
; completion register - that costs neither the interrupt processing nor the
; interrupt-delivery latency. Only ports working on genuinely long commands
; (several multi-ms ones in a row) switch to interrupts and sleeping.
; The threshold is measured in TSC ticks computed from cpu_freq; until the
; frequency is measured (early boot) a conservative default is used.
AHCI_SPIN_US          = 200      ; base busy-poll length, in microseconds
AHCI_SPIN_TSC_DEFAULT = 400000   ; fallback threshold in TSC ticks (~200 us at 2 GHz)

; PxIS/PxIE bits: D2H register FIS, PIO setup FIS, DMA setup FIS, set device bits FIS
; + overflow, interface fatal, host bus data/fatal, task file error
AHCI_PORT_INTR_ERR_MASK = 0x79000000 ; TFES | HBFS | HBDS | IFS | OFS
AHCI_PORT_INTR_MASK     = AHCI_PORT_INTR_ERR_MASK or 0x0000000F

; Frame Information Structure Types
FIS_TYPE_REG_H2D    = 0x27 ; Register FIS - host to device
FIS_TYPE_REG_D2H    = 0x34 ; Register FIS - device to host
FIS_TYPE_DMA_ACT    = 0x39 ; DMA activate FIS - device to host
FIS_TYPE_DMA_SETUP  = 0x41 ; DMA setup FIS - bidirectional
FIS_TYPE_DATA       = 0x46 ; Data FIS - bidirectional
FIS_TYPE_BIST       = 0x58 ; BIST activate FIS - bidirectional
FIS_TYPE_PIO_SETUP  = 0x5F ; PIO setup FIS - device to host
FIS_TYPE_DEV_BITS   = 0xA1 ; Set device bits FIS - device to host

; Generic Host Control registers
struct HBA_MEM
        cap                   dd ?                    ; 0x00, Host capabilities
        ghc                   dd ?                    ; 0x04, Global host control
        is                    dd ?                    ; 0x08, Interrupt status
        pi                    dd ?                    ; 0x0C, Port implemented
        version               dd ?                    ; 0x10, Version
        ccc_ctl               dd ?                    ; 0x14, Command completion coalescing control
        ccc_pts               dd ?                    ; 0x18, Command completion coalescing ports
        em_loc                dd ?                    ; 0x1C, Enclosure management location
        em_ctl                dd ?                    ; 0x20, Enclosure management control
        cap2                  dd ?                    ; 0x24, Host capabilities extended
        bohc                  dd ?                    ; 0x28, BIOS/OS handoff control and status
        reserved              rb (0xA0-HBA_MEM.reserved)        ; 0x2C - 0x9F, Reserved
        vendor                rb (0x100-HBA_MEM.vendor)         ; 0xA0 - 0xFF, Vendor specific
        ports                 rb (sizeof.HBA_PORT*AHCI_MAX_PORTS) ; 0x100 - 0x10FF, Port control registers, max AHCI_MAX_PORTS
ends

; Port Control registers
struct HBA_PORT
        command_list_base_l      dd ?                 ; 0x00, command list base address, 1K-byte aligned
        command_list_base_h      dd ?                 ; 0x04, command list base address upper 32 bits, used on 64 bit systems
        fis_base_l               dd ?                 ; 0x08, FIS base address, 256-byte aligned
        fis_base_h               dd ?                 ; 0x0C, FIS base address upper 32 bits, used on 64 bit systems
        interrupt_status         dd ?                 ; 0x10
        interrupt_enable         dd ?                 ; 0x14
        command                  dd ?                 ; 0x18, command and status
        reserved0                dd ?                 ; 0x1C
        task_file_data           dd ?                 ; 0x20
        signature                dd ?                 ; 0x24
        sata_status              dd ?                 ; 0x28, SATA status (SCR0:SStatus)
        sata_control             dd ?                 ; 0x2C, SATA control (SCR2:SControl)
        sata_error               dd ?                 ; 0x30, SATA error (SCR1:SError)
        sata_active              dd ?                 ; 0x34, SATA active (SCR3:SActive)
        command_issue            dd ?                 ; 0x38
        sata_notification        dd ?                 ; 0x3C, SATA notification (SCR4:SNotification)
        fis_based_switch_control dd ?                 ; 0x40
        reserved1                rd 11                ; 0x44 - 0x6F
        vendor                   rd 4                 ; 0x70 - 0x7F, vendor specific
ends

; Command header structure, size = 32 bytes
struct HBA_CMD_HDR
    flags1       db ? ; 0bPWACCCCC, P - Prefetchable, W - Write (1: H2D, 0: D2H)
                       ; A - ATAPI, C - Command FIS length in DWORDS, 2 ~ 16

    flags2       db ? ; 0bPPPPRCB(Re), P - Port multiplier port, R - Reserved,
                       ; C - Clear busy upon R_OK, B - BIST, Re - Reset

    prdtl         dw ? ; Physical region descriptor table length in entries
    prdbc         dd ? ; Physical region descriptor byte count transferred
    ctba          dd ? ; Command table descriptor base address
    ctbau         dd ? ; Command table descriptor base address upper 32 bits
                  rd 4 ; Reserved
ends

; Physical region descriptor table entry, size = 16 bytes
struct HBA_PRDT_ENTRY
    dba           dd ?  ; Data base address
    dbau          dd ?  ; Data base address upper 32 bits
                  dd ?  ; Reserved
    flags        dd ?  ; 0bIR..RD..D, I (1 bit) - Interrupt on completion,
                        ; R (9 bits) - Reserved, D (22 bits) - Byte count, 4M max
ends

struct HBA_CMD_TBL
    cfis          rb 64 ; 0x00, Command FIS
    acmd          rb 16 ; 0x40, ATAPI command, 12 or 16 bytes
                  rb 48 ; 0x50, Reserved
    prdt_entry    HBA_PRDT_ENTRY  ; 0x80, Physical region descriptor table entries, 0 ~ 65535
                        ; so, this structure is variable-length
ends

; Contains virtual mappings for port phys memory regions
struct PORT_DATA
    clb           dd ? ; Command list base
    fb            dd ? ; FIS base
    ctba_arr      rd 32 ; ctba_arr[0] = clb[0].ctba, ... and so on.
    port          dd ? ; address of corresponding HBA_PORT structure
    portno        dd ? ; port index, 0..31
    drive_type    db ? ; drive type
    sector_count  dq ? ; number of sectors
    ctr_ptr       dd ? ; pointer to controller to which port belongs
    intr_status   dd ? ; PxIS bits accumulated by the irq handler
    event_ptr     dd ? ; EVENT of the thread waiting for command completion, 0 = nobody waits
    event_uid     dd ? ; uid of that event
    avg_cmd_tsc   dd ? ; smoothed command duration in TSC ticks, for the adaptive busy-poll
    ie_mode       dd ? ; current PxIE state: 1 = interrupts enabled on this port
    wake_slow     dd ? ; 1 = interrupt wakeups on this port are late/missing, do not sleep
    slow_streak   dd ? ; sleeps in a row that ended late (by timeout or a tick-late interrupt)
    fast_streak   dd ? ; sleeps in a row that were ended by the interrupt in time
    next_probe    dd ? ; timer tick after which to re-probe sleeping while wake_slow = 1
    long_run      dd ? ; consecutive commands longer than the sleep threshold
    short_run     dd ? ; consecutive commands shorter than the sleep threshold
    mutex         MUTEX ; serializes commands on this port
ends

; Register FIS - Host to Device
struct FIS_REG_H2D
        fis_type      db ?       ; FIS_TYPE_REG_H2D
        flags        db ?       ; 0bCRRRPPPP, C - 1: Command, 0: Control
                                 ; R - Reserved, P - Port multiplier

        command       db ?       ; Command register
        featurel      db ?       ; Feature register, 7:0

        lba0          db ?       ; LBA low register, 7:0
        lba1          db ?       ; LBA mid register, 15:8
        lba2          db ?       ; LBA high register, 23:16
        device        db ?       ; Device register

        lba3          db ?       ; LBA register, 31:24
        lba4          db ?       ; LBA register, 39:32
        lba5          db ?       ; LBA register, 47:40
        featureh      db ?       ; Feature register, 15:8

        countl        db ?       ; Count register, 7:0
        counth        db ?       ; Count register, 15:8
        icc           db ?       ; Isochronous command completion
        control       db ?       ; Control register

                      rb 4       ; Reserved
ends

; Register FIS - Device to Host
struct FIS_REG_D2H
    fis_type      db ?           ; FIS_TYPE_REG_D2H

    flags        db ?           ; 0bRIRPPPP, P - Port multiplier, R - Reserved
                                 ; I - Interrupt bit

    status        db ?           ; Status register
    error         db ?           ; Error register

    lba0          db ?           ; LBA low register, 7:0
    lba1          db ?           ; LBA mid register, 15:8
    lba2          db ?           ; LBA high register, 23:16
    device        db ?           ; Device register

    lba3          db ?           ; LBA register, 31:24
    lba4          db ?           ; LBA register, 39:32
    lba5          db ?           ; LBA register, 47:40
                  db ?           ; Reserved

    countl        db ?           ; Count register, 7:0
    counth        db ?           ; Count register, 15:8
                  rb 2           ; Reserved

                  rb 4           ; Reserved
ends

; Data FIS - Bidirectional
struct FIS_DATA
    fis_type      db ?           ; FIS_TYPE_DATA
    flags        db ?           ; 0bRRRRPPPP, R - Reserved, P - Port multiplier
                  rb 2           ; Reserved
    ; DWORD 1 ~ N (?)
    data          rd 1           ; Payload
ends

; PIO Setup - Device to Host
struct FIS_PIO_SETUP
    fis_type      db ?           ; FIS_TYPE_PIO_SETUP

    flags        db ?           ; 0bRIDRPPPP, P - Port multiplier, R - Reserved
                                 ; I - Interrupt bit, D - Data transfer direction, 1 - device to host

    status        db ?           ; Status register
    error         db ?           ; Error register

    lba0          db ?           ; LBA low register, 7:0
    lba1          db ?           ; LBA mid register, 15:8
    lba2          db ?           ; LBA high register, 23:16
    device        db ?           ; Device register

    lba3          db ?           ; LBA register, 31:24
    lba4          db ?           ; LBA register, 39:32
    lba5          db ?           ; LBA register, 47:40
                  db ?           ; Reserved

    countl        db ?           ; Count register, 7:0
    counth        db ?           ; Count register, 15:8
                  db ?           ; Reserved
    e_status      db ?           ; New value of status register

    tc            dw ?           ; Transfer count
                  rb 2           ; Reserved
ends

; DMA Setup - Device to Host
struct FIS_DMA_SETUP
    fis_type      db ?           ; FIS_TYPE_DMA_SETUP
    flags        db ?           ; 0bAIDRPPPP, A - Auto-activate. Specifies if DMA Activate FIS is needed,
                                 ; I - Interrupt bit, D - Data transfer direction, 1 - device to host,
                                 ; R - Reserved, P - Port multiplier

                  rb 2           ; Reserved
    DMAbufferID   dq ?           ; DMA Buffer Identifier.
                                 ; Used to Identify DMA buffer in host memory.
                                 ; SATA Spec says host specific and not in Spec.
                                 ; Trying AHCI spec might work.

                  dd ?           ; Reserved
    DMAbufOffset  dd ?           ; Byte offset into buffer. First 2 bits must be 0
    TransferCount dd ?           ; Number of bytes to transfer. Bit 0 must be 0
                  dd ?           ; Reserved
ends

; Set device bits FIS - device to host
struct FIS_DEV_BITS
    fis_type      db ?           ; FIS_TYPE_DEV_BITS
    flags        db ?           ; 0bNIRRPPPP, N - Notification, I - Interrupt,
                                 ; R - Reserved, P - Port multiplier

    status        db ?           ; Status register
    error         db ?           ; Error register

    protocol      dd ?           ; Protocol
ends

struct HBA_FIS
    dsfis         FIS_DMA_SETUP  ; 0x00, DMA Setup FIS
                  rb 4           ; padding

    psfis         FIS_PIO_SETUP  ; 0x20, PIO Setup FIS
                  rb 12          ; padding

    rfis          FIS_REG_D2H    ; 0x40, Register - Device to Host FIS
                  rb 4           ; padding

    sdbfis        FIS_DEV_BITS   ; 0x58, Set Device Bit FIS

    ufis          rb 64          ; 0x60

                  rb (0x100 - 0xA0) ; 0xA0, Reserved
ends

; --------------------------------------------------
uglobal

align 4

; AHCI controller structure
struct AHCI_CTR
        abar             dd ?    ; pointer to HBA Memory (BAR5) mapped to virtual kernelspace memory
        pcidev           dd ?    ; pointer to corresponding PCIDEV structure
        port_data_arr    rb (sizeof.PORT_DATA*AHCI_MAX_PORTS)
        irq_line         dd ?    ; PCI interrupt line, -1 if none
        polling_mode     dd ?    ; 1 = wait for commands by polling, do not rely on interrupts
        irq_miss_count   dd ?    ; commands completed without an interrupt (in a row)
        intr_count       dd ?    ; total interrupts received, diagnostics
        cmds_total       dd ?    ; commands completed, diagnostics
        sleeps_total     dd ?    ; times a thread went to sleep waiting, diagnostics
        miss_total       dd ?    ; wakeups by timeout with the command already done, diagnostics
ends

ctr1_data AHCI_CTR
ctr2_data AHCI_CTR
ctr3_data AHCI_CTR
ctr4_data AHCI_CTR
ctr5_data AHCI_CTR
ctr6_data AHCI_CTR
ctr7_data AHCI_CTR
ctr8_data AHCI_CTR

ctr_ptr dd ?
ahci_spin_tsc dd ? ; busy-poll threshold in TSC ticks, 0 = not computed yet
endg

iglobal
align 4
ahci_callbacks:
    dd  ahci_callbacks.end - ahci_callbacks
    dd  0   ; no close function
    dd  0   ; no closemedia function
    dd  ahci_querymedia
    dd  ahci_read
    dd  ahci_write
    dd  0   ; no flush function
    dd  0   ; use default cache size
.end:
hd_name db 'sd', 0, 0, 0
sata_dev_counter dd 0    ; sata devices counter
; TODO: satapi_dev_counter dd 0 ; satapi devices (optical drives) counter
ctr_counter dd 0         ; controllers counter
endg

; -----------------------------------------------------------------------
; detect ahci controller and initialize
align 4
ahci_init:
        mov     esi, pcidev_list
        mov     [ctr_ptr], ctr1_data
        mov     [sata_dev_counter], 0
.find_ahci_ctr:
        mov     esi, [esi + PCIDEV.fd]
        cmp     esi, pcidev_list
        jz      .ahci_ctr_not_found
        mov     eax, [esi + PCIDEV.class]
        ;DEBUGF  1, "K: device class = %x\n", eax
        shr     eax, 8 ; shift right because lowest 8 bits if ProgIf field
        cmp     eax, 0x0106 ; 0x01 - Mass Storage Controller class,  0x06 - Serial ATA Controller subclass
        jz      .ahci_ctr_found
        jmp     .find_ahci_ctr

.ahci_ctr_not_found:
        DEBUGF  1, "K: AHCI controller not found\n"
        ret

.ahci_ctr_found:
        push    esi

        mov     ecx, [ctr_ptr]
        mov     [ecx + AHCI_CTR.pcidev], esi

        mov     eax, [esi+PCIDEV.class]
        movzx   ebx, byte [esi+PCIDEV.bus]
        movzx   ecx, byte [esi+PCIDEV.devfn]
        shr     ecx, 3 ; get rid of 3 lowest bits (function code), the rest bits is device code
        movzx   edx, byte [esi+PCIDEV.devfn]
        and     edx, 00000111b ; get only 3 lowest bits (function code)
        DEBUGF  1, "K: found AHCI controller, (class, subcl, progif) = %x, bus = %x, device = %x, function = %x\n", eax, ebx, ecx, edx

        ; get BAR5 value, it is physical address
        movzx   ebx, [esi + PCIDEV.bus]
        movzx   ebp, [esi + PCIDEV.devfn]
        stdcall pci_read32, ebx, ebp, PCI_REG_BAR5
        DEBUGF  1, "K: AHCI controller MMIO = %x\n", eax
        mov     edi, eax

        ; get the size of MMIO region
        stdcall pci_write32, ebx, ebp, PCI_REG_BAR5, 0xFFFFFFFF
        stdcall pci_read32, ebx, ebp, PCI_REG_BAR5
        not     eax
        inc     eax
        DEBUGF  1, "K: AHCI: MMIO region size = 0x%x bytes\n", eax

        ; Map MMIO region to virtual memory
        stdcall map_io_mem, edi, eax, PG_SWR + PG_NOCACHE
        push    ecx
        mov     ecx, [ctr_ptr]
        mov     [ecx + AHCI_CTR.abar], eax
        pop     ecx
        DEBUGF  1, "K: AHCI controller BAR5 mapped to virtual addr %x\n", eax

        ; Restore the original BAR5 value
        stdcall pci_write32, ebx, ebp, PCI_REG_BAR5, edi

        ; Enable dma bus mastering, memory space access, clear the "disable interrupts" bit
        ; Usually, it is already done before us
        movzx   ebx, [esi + PCIDEV.bus]
        movzx   ebp, [esi + PCIDEV.devfn]
        stdcall pci_read32, ebx, ebp, PCI_REG_STATUS_COMMAND
        DEBUGF  1, "K: AHCI: pci_status_command = %x\nEnabling interrupts, DMA bus mastering and memory space access\n", eax
        or      eax, 0x06 ; pci.command |= 0x06 (dma bus mastering + memory space access)
        btr     eax, 10 ; clear the "disable interrupts" bit
        DEBUGF  1, "K: AHCI: pci_status_command = %x\n", eax
        stdcall pci_write32, ebx, ebp, PCI_REG_STATUS_COMMAND, eax

        ; Get the PCI interrupt line and install the interrupt handler.
        ; If that fails, fall back to completion by polling.
        mov     ecx, [ctr_ptr]
        mov     [ecx + AHCI_CTR.polling_mode], 0
        mov     [ecx + AHCI_CTR.irq_miss_count], 0
        stdcall pci_read8, ebx, ebp, 0x3C ; PCI interrupt line register
        and     eax, 0xFF
        mov     [ecx + AHCI_CTR.irq_line], eax
        cmp     eax, 0
        je      .no_irq
        cmp     eax, 0xFF
        je      .no_irq
        cmp     eax, IRQ_RESERVED
        jae     .no_irq
        stdcall attach_int_handler, eax, ahci_irq_handler, ecx
        mov     ecx, [ctr_ptr] ; attach_int_handler clobbers ecx
        test    eax, eax
        jnz     .irq_ok
.no_irq:
        DEBUGF  1, "K: AHCI: no usable IRQ (line = %d), working in polling mode\n", [ecx + AHCI_CTR.irq_line]
        mov     [ecx + AHCI_CTR.polling_mode], 1
        or      [ecx + AHCI_CTR.irq_line], -1
        jmp     @f
.irq_ok:
        DEBUGF  1, "K: AHCI: using IRQ %d\n", [ecx + AHCI_CTR.irq_line]
@@:

        mov     esi, [ctr_ptr]
        mov     esi, [esi + AHCI_CTR.abar]

        ; ; Print some register values to debug board
        ; DEBUGF  1, "K: AHCI: HBA.cap = %x, HBA.ghc = %x, HBA_MEM.version = %x\n", [esi + HBA_MEM.cap], [esi + HBA_MEM.ghc], [esi + HBA_MEM.version]

        ;-------------------------------------------------------
        ; Request BIOS/OS ownership handoff, if supported. (TODO check correctness)
        ;mov     ebx, [esi + HBA_MEM.cap2]
        ;DEBUGF  1, "K: AHCI: HBA_MEM.cap2 = %x\n", ebx
        bt      [esi + HBA_MEM.cap2], bit_AHCI_HBA_CAP2_BOH
        jnc     .end_handoff
        DEBUGF  1, "K: AHCI: requesting AHCI ownership change...\n"
        bts     [esi + HBA_MEM.bohc], bit_AHCI_HBA_BOHC_OOS

        mov     ecx, 200 ; wait up to 2 s for BIOS to release the controller
.wait_not_bos:
        bt      [esi + HBA_MEM.bohc], bit_AHCI_HBA_BOHC_BOS
        jnc     @f
        mov     ebx, 1
        call    delay_hs
        dec     ecx
        jnz     .wait_not_bos
        DEBUGF  1, "K: AHCI: BIOS did not release ownership, taking over anyway\n"
@@:
        mov     ebx, 3
        call    delay_hs

        ; if Bios Busy is still set after 30 mS, wait 2 seconds.
        bt      [esi + HBA_MEM.bohc], bit_AHCI_HBA_BOHC_BB
        jnc     @f

        mov     ebx, 200
        call    delay_hs
@@:
        DEBUGF  1, "K: AHCI: ownership change completed.\n"

.end_handoff:
        ;-------------------------------------------------------

        ; enable the AHCI and reset it
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_AHCI_ENABLE
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_RESET

        ; wait for reset to complete, up to 1 s by spec
        mov     ecx, 100
.wait_reset:
        bt      [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_RESET
        jnc     @f
        mov     ebx, 1
        call    delay_hs
        dec     ecx
        jnz     .wait_reset
        DEBUGF  1, "K: AHCI: controller reset failed, skipping it\n"
        jmp     .end_detect_drives
@@:

        ; enable the AHCI and interrupts
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_AHCI_ENABLE
        mov     [esi + HBA_MEM.is], 0xFFFFFFFF ; clear stale interrupt status
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_INTERRUPT_ENABLE
        mov     ebx, 2
        call    delay_hs

        DEBUGF  1, "K: AHCI: caps: %x %x, ver: %x, ghc: %x, pi: %x\n", [esi + HBA_MEM.cap], [esi + HBA_MEM.cap2], [esi + HBA_MEM.version], [esi + HBA_MEM.ghc], [esi + HBA_MEM.pi]

        xor     ebx, ebx
.detect_drives:
        cmp     ebx, AHCI_MAX_PORTS
        jae     .end_detect_drives

        ; if port with index ebx is not implemented then go to next
        mov     ecx, [esi + HBA_MEM.pi]
        bt      ecx, ebx
        jnc     .continue_detect_drives

        mov     edi, ebx
        imul    edi, sizeof.HBA_PORT
        add     edi, HBA_MEM.ports
        add     edi, esi
        ; now edi - base of HBA_MEM.ports[ebx]

        DEBUGF  1, "K: AHCI: port %d, cmd = %x, ssts = %x\n", ebx, [edi + HBA_PORT.command], [edi + HBA_PORT.sata_status]

        ; If port is not idle force it to be idle
        mov     eax, [edi + HBA_PORT.command]
        and     eax, (AHCI_HBA_PxCMD_ST or AHCI_HBA_PxCMD_CR or AHCI_HBA_PxCMD_FRE or AHCI_HBA_PxCMD_FR)
        test    eax, eax
        jz      @f

        mov     eax, edi
        DEBUGF  1, "ahci_stop_cmd..\n"
        call    ahci_stop_cmd
@@:
        ; TODO: what is purpose of this block of code ?
        ; Reset port, disable slumber and partial state
        ; mov     [edi + HBA_PORT.sata_control], 0x301
        ; push    ebx
        ; mov     ebx, 5 ; wait 50 ms
        ; call    delay_hs
        ; pop     ebx
        ; mov     [edi + HBA_PORT.sata_control], 0x300

        ; if(abar->cap & HBA_MEM_CAP_SSS)
        ; {
        ; abar->ports[i].cmd |= (HBA_PxCMD_SUD | HBA_PxCMD_POD | HBA_PxCMD_ICC);
        ; Sleep(10);
        ; }
        ; rewritten to:
        bt      [esi + HBA_MEM.cap], 27 ; check Supports Staggered Spin-up bit in capabilities
        jnc     @f
        DEBUGF  1, "Supports Staggered Spin-up, spinning up the port..\n"
        or      [edi + HBA_PORT.command], (0x0002 or 0x0004 or 0x10000000)
        push    ebx
        mov     ebx, 10 ; wait 100 ms
        call    delay_hs
        pop     ebx
@@:
        ; Clear interrupt status and error status
        mov     [edi + HBA_PORT.sata_error], 0xFFFFFFFF
        mov     [edi + HBA_PORT.interrupt_status], 0xFFFFFFFF

        ; ------------------------------------------
        ; Check whether anything can be attached to this port:
        ; DET = 3 - device present and phy communication established;
        ; DET = 1 - device present but no communication yet (will do COMRESET);
        ; IPM != 0 (partial/slumber/devsleep) - link is powered down, DET may
        ; read 0 in that state, so the device has to be woken up by COMRESET.
        ; Only DET = 0/4 with IPM = 0 reliably means "nothing here".

        mov     ecx, [edi + HBA_PORT.sata_status]
        test    ecx, 0xF00 ; IPM
        jnz     .try_port
        and     ecx, AHCI_HBA_PxSSTS_DET
        cmp     ecx, AHCI_HBA_PxSSTS_DET_PRESENT
        je      .try_port
        cmp     ecx, 1 ; presence detected, no communication
        jne     .continue_detect_drives
.try_port:

        mov     ecx, ebx
        imul    ecx, sizeof.PORT_DATA
        add     ecx, AHCI_CTR.port_data_arr
        add     ecx, [ctr_ptr]
        mov     eax, [ctr_ptr]
        mov     [ecx + PORT_DATA.ctr_ptr], eax ; to which controller the port belongs

        push    ecx
        add     ecx, PORT_DATA.mutex
        call    mutex_init
        pop     ecx

        stdcall ahci_port_rebase, edi, ebx, ecx
        test    eax, eax
        jnz     .continue_detect_drives ; no device answered on this port

        ; DEBUGF  1, "K: AHCI: After REBASING, signature = 0x%x\n", [edi + HBA_PORT.signature]

        ; Determine drive type by checking port signature
.switch_sig:
        cmp     [edi + HBA_PORT.signature], SATA_SIG_ATA
        mov     eax, AHCI_DEV_SATA
        jz      .end_switch_sig

        cmp     [edi + HBA_PORT.signature], SATA_SIG_ATAPI
        mov     eax, AHCI_DEV_SATAPI
        jz      .end_switch_sig

        cmp     [edi + HBA_PORT.signature], SATA_SIG_SEMB
        mov     eax, AHCI_DEV_SEMB
        jz      .end_switch_sig

        cmp     [edi + HBA_PORT.signature], SATA_SIG_PM
        mov     eax, AHCI_DEV_PM
        jz      .end_switch_sig

        ; The link is up but the signature is not any known one. Most likely
        ; the device signature FIS was simply lost (e.g. BIOS left the port
        ; uninitialized), so try to treat it as a plain SATA disk and let
        ; IDENTIFY decide whether a disk is really there.
        DEBUGF  1, "K: AHCI: unknown device signature %x, assuming SATA\n", [edi + HBA_PORT.signature]
        mov     eax, AHCI_DEV_SATA
.end_switch_sig:
        mov     [ecx + PORT_DATA.drive_type], al

        DEBUGF  1, "K: AHCI: found drive on port %u: TYPE = %u\n", ebx, [ecx + PORT_DATA.drive_type]

        stdcall ahci_port_identify, ecx
        test    eax, eax
        jnz     .continue_detect_drives ; identification failed - no disk to add

        cmp     [ecx + PORT_DATA.drive_type], AHCI_DEV_SATA
        jne     .after_add_disk ; skip adding disk code
        ; register disk in system:

        ;stdcall ahci_read_first_sector, ecx

        push    ecx
        mov     eax, [sata_dev_counter]
        inc     [sata_dev_counter]
        xor     edx, edx
        mov     ecx, 10
        div     ecx ; eax = sata_dev_counter / 10, edx = sata_dev_counter % 10
        test    eax, eax
        jz      .concat_one
        add     al, '0'
        mov     byte [hd_name + 2], al
        add     dl, '0'
        mov     byte [hd_name + 3], dl
        jmp     .endif1
.concat_one:
        add     dl, '0'
        mov     byte [hd_name + 2], dl
.endif1:
        pop     ecx

        DEBUGF  1, "adding '%s'\n", hd_name

        push    ecx
        stdcall disk_add, ahci_callbacks, hd_name, ecx, 0
        pop     ecx
        test    eax, eax
        jz      .disk_add_fail
        push    ecx
        stdcall disk_media_changed, eax, 1 ; system will scan for partitions on disk
        pop     ecx

        jmp     .after_add_disk

.disk_add_fail:
        DEBUGF  1, "Failed to add disk\n"
.after_add_disk:

.continue_detect_drives:
        inc     ebx
        jmp     .detect_drives



.end_detect_drives:
        pop     esi
        add     [ctr_ptr], sizeof.AHCI_CTR
        inc     [ctr_counter]
        cmp     [ctr_counter], 8
        jnz     .find_ahci_ctr
        DEBUGF  1, "AHCI: reached controllers number limit\n"
        ret
; -------------------------------------------------

modelstr  rb 42
; Identify drive on port
; in: pdata - address of PORT_DATA structure
; out: eax = 0 on success, 1 on error
proc ahci_port_identify stdcall, pdata: dword
        locals
            cmdslot dd ?
            cmdheader dd ?
            cmdtable  dd ?
            buf_phys  dd ?
            buf_virt  dd ?
            result    dd ?
        endl

        pushad

        mov     [result], 1

        mov     esi, [pdata] ; esi - address of PORT_DATA struct of port
        stdcall ahci_find_cmdslot, esi

        cmp     eax, -1
        jne     .cmdslot_found

        DEBUGF  1, "No free cmdslot on port %u\n", [esi + PORT_DATA.portno]
        jmp     .ret

.cmdslot_found:
        mov     [cmdslot], eax
        ; DEBUGF  1, "Found free cmdslot %u on port %u\n", [cmdslot], [esi + PORT_DATA.portno]

        shl     eax, BSF sizeof.HBA_CMD_HDR
        add     eax, [esi + PORT_DATA.clb]
        mov     [cmdheader], eax ; address of virtual mapping of command header
        mov     eax, [cmdslot]
        mov     eax, [esi + eax*4 + PORT_DATA.ctba_arr]
        mov     [cmdtable], eax ; address of virtual mapping of command table of command header

        stdcall _memset, eax, 0, sizeof.HBA_CMD_TBL

        call    alloc_page
        test    eax, eax
        jz      .ret
        mov     [buf_phys], eax

        stdcall map_io_mem, eax, 4096, PG_NOCACHE + PG_SWR  ; map to virt memory so we can work with it
        test    eax, eax
        jz      .free_phys
        mov     [buf_virt], eax

        mov     eax, [cmdtable]
        mov     ebx, [buf_phys]
        mov     dword [eax + HBA_CMD_TBL.prdt_entry + HBA_PRDT_ENTRY.dba], ebx
        mov     dword [eax + HBA_CMD_TBL.prdt_entry + HBA_PRDT_ENTRY.dbau], 0
        and     [eax + HBA_CMD_TBL.prdt_entry + HBA_PRDT_ENTRY.flags], not 0x3FFFFF ; zero out lower 22 bits, they used for byte count
        or      [eax + HBA_CMD_TBL.prdt_entry + HBA_PRDT_ENTRY.flags], 512 - 1 ; reason why -1 see in spec on this field
        ; or      [eax + HBA_CMD_TBL.prdt_entry + HBA_PRDT_ENTRY.flags], 1 shl 31 ; enable interrupt on completion

        mov     eax, [cmdheader]
        and     [eax + HBA_CMD_HDR.flags1], not 0x1F ; zero out lower 5 bits, they will be used for cfl
        or      [eax + HBA_CMD_HDR.flags1], (sizeof.FIS_REG_H2D / 4) ; set command fis length in dwords
        movzx   bx, [eax + HBA_CMD_HDR.flags1]
        btr     bx, 6 ; flag W = 0
        mov     [eax + HBA_CMD_HDR.flags1], bl
        movzx   bx, [eax + HBA_CMD_HDR.flags2]
        btr     bx, 2 ; flag C = 0
        mov     [eax + HBA_CMD_HDR.flags2], bl
        mov     [eax + HBA_CMD_HDR.prdtl], 1

        mov     eax, [cmdtable]
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.fis_type], FIS_TYPE_REG_H2D
        movzx   ebx, byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.flags]
        bts     ebx, bit_AHCI_H2D_FLAG_CMD ; Set Command bit in H2D FIS.
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.flags], bl

        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.command], ATA_IDENTIFY
        cmp     [esi + PORT_DATA.drive_type], AHCI_DEV_SATAPI
        jne     @f
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.command], ATAPI_IDENTIFY
@@:
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.device], 0

        mov     edi, [esi + PORT_DATA.port] ; edi - address of HBA_PORT struct of port

        ; Wait on previous command to complete, before issuing new command.
        stdcall ahci_port_wait, edi, AHCI_PORT_TIMEOUT
        test    eax, eax
        jnz     .fail

        stdcall ahci_port_update_ie, esi
        ; drop the PxIS bits accumulated by previous commands (with PxIE off
        ; nobody clears them), so that a stale error bit does not fail us
        mov     [edi + HBA_PORT.interrupt_status], 0xFFFFFFFF
        mov     [esi + PORT_DATA.intr_status], 0
        mov     eax, [cmdslot]
        bts     [edi + HBA_PORT.command_issue], eax ; Issue the command

        ; Wait for command completion
        stdcall ahci_port_cmd_wait, esi, eax
        test    eax, eax
        jnz     .fail

        ; DEBUGF  1, "sata_error register = 0x%x\n", [edi + HBA_PORT.sata_error]

        mov     esi, [buf_virt]
        add     esi, 27*2
        mov     edi, modelstr
        mov     ecx, ((46-27)+1)*2
        cld
        rep movsb
        mov     byte [edi], 0

        stdcall swap_bytes_in_words, modelstr, (46-27)+1
        DEBUGF  1, "IDENTIFICATION RESULT: MODEL = %s\n", modelstr

        mov     esi, [buf_virt]
        mov     eax, [esi + 200]
        mov     edx, [esi + 200 + 4]
        DEBUGF 1, "lba48 mode sector count = 0x%x:%x\n", edx, eax

        mov     ebx, [pdata]
        mov     dword [ebx + PORT_DATA.sector_count], eax
        mov     dword [ebx + PORT_DATA.sector_count + 4], edx

        ; a SATA disk with zero capacity did not really answer IDENTIFY
        cmp     [ebx + PORT_DATA.drive_type], AHCI_DEV_SATA
        jne     @f
        or      eax, edx
        jz      .fail
        mov     eax, dword [ebx + PORT_DATA.sector_count]
        mov     edx, dword [ebx + PORT_DATA.sector_count + 4]
@@:
        mov     [result], 0

        shrd    eax, edx, 11 ; i.e *512 / 1024 / 1024, 512 - sector size
        DEBUGF  1, "disk capacity = %u MiB ", eax
        shrd    eax, edx, 10 ; / 1024
        DEBUGF  1, "= %u GiB\n", eax
.free_buf:
        stdcall free_kernel_space, [buf_virt]
.free_phys:
        mov     eax, [buf_phys]
        call    free_page
.ret:
        popad
        mov     eax, [result]
        ret

.fail:
        mov     esi, [pdata]
        DEBUGF  1, "K: AHCI: IDENTIFY failed on port %u\n", [esi + PORT_DATA.portno]
        stdcall ahci_port_error_recovery, esi
        jmp     .free_buf
endp

proc ahci_querymedia stdcall, pdata, mediainfo
        push    ecx edx
        mov     eax, [mediainfo]
        mov     edx, [pdata]
        mov     [eax + DISKMEDIAINFO.Flags], 0
        mov     [eax + DISKMEDIAINFO.SectorSize], 512
        mov     ecx, dword[edx + PORT_DATA.sector_count]
        mov     dword [eax + DISKMEDIAINFO.Capacity], ecx
        mov     ecx, dword[edx + PORT_DATA.sector_count + 4]
        mov     dword [eax + DISKMEDIAINFO.Capacity + 4], ecx
        pop     edx ecx
        xor     eax, eax
        ret
endp

; Read/write sectors, note: currently work for SATA, not SATAPI
; return value: 0 = success, otherwise = error
proc ahci_rw_sectors stdcall pdata: dword, vbuf: dword, startsector: qword, numsectors: dword, is_write: dword
        locals
            cmdslot dd ?
            cmdheader dd ?
            cmdtable  dd ?
            vbuf_orig dd ?
            vbuf_len  dd ?
            phys_region_start dd ?
            new_phys_region_start dd ?
            cur_prd   dd ?
            cur_phys  dd ?
            dbc       dd ?
            cur_phys_page dd ?
            next_phys_page dd ?
            cur_antioffset dd ?
            prdt_bytes_total dd ?
        endl

        pushad

        DEBUGF  AHCI_DBGLVL, "    ahci_rw_sectors: buffer = 0x%x, startsector = 0x%x:%x, numsectors = %u, is_write = %u\n", [vbuf], [startsector], [startsector + 4], [numsectors], [is_write]:1

        ; the count field of the command FIS is 16 bit wide (0 means 65536)
        cmp     [numsectors], 65536
        jbe     @f
        mov     [numsectors], 65536
@@:
        mov     esi, [pdata] ; esi - address of PORT_DATA struct of port
        stdcall ahci_find_cmdslot, esi
        cmp     eax, -1
        jne     .cmdslot_found

        DEBUGF  AHCI_DBGLVL, "No free cmdslot on port %u\n", [esi + PORT_DATA.portno]
        jmp     .fail

.cmdslot_found:
        mov     [cmdslot], eax
        DEBUGF  AHCI_DBGLVL, "Found free cmdslot %u on port %u\n", [cmdslot], [esi + PORT_DATA.portno]

        shl     eax, BSF sizeof.HBA_CMD_HDR
        add     eax, [esi + PORT_DATA.clb]
        mov     [cmdheader], eax ; address of virtual mapping of command header
        mov     eax, [cmdslot]
        mov     eax, [esi + eax*4 + PORT_DATA.ctba_arr]
        mov     [cmdtable], eax ; address of virtual mapping of command table of command header

        mov     eax, [cmdheader]
        and     [eax + HBA_CMD_HDR.flags1], not 0x1F ; zero out lower 5 bits, they will be used for cfl
        or      [eax + HBA_CMD_HDR.flags1], (sizeof.FIS_REG_H2D / 4) ; set command fis length in dwords
        movzx   bx, [eax + HBA_CMD_HDR.flags1]
        btr     bx, 6 ; flag W = 0
        cmp     [is_write], 1 ; if is_write then set W flag
        jne     @f
        bts     bx, 6
@@:
        mov     [eax + HBA_CMD_HDR.flags1], bl
        movzx   bx, [eax + HBA_CMD_HDR.flags2]
        btr     bx, 2 ; flag C = 0
        mov     [eax + HBA_CMD_HDR.flags2], bl

        mov     eax, [vbuf]
        mov     [vbuf_orig], eax
        mov     ebx, [numsectors]
        shl     ebx, 9 ; *= 512
        mov     [vbuf_len], ebx
        DEBUGF  AHCI_DBGLVL, "vbuf_len = %u bytes\n", ebx

        mov     ebx, [vbuf]
        and     ebx, 0xFFF
        mov     eax, [vbuf]
        call    get_pg_addr
        add     eax, ebx
        mov     [phys_region_start], eax
        mov     [prdt_bytes_total], 0
        mov     [cur_prd], 0
.fill_prdt:
        cmp     [vbuf_len], 0
        jbe     .fill_prdt_end

        mov     eax, [vbuf]
        call    get_pg_addr
        mov     [cur_phys_page], eax
        mov     eax, [vbuf]
        add     eax, 4096
        call    get_pg_addr
        mov     [next_phys_page], eax
        mov     eax, 4096
        mov     ebx, [vbuf]
        and     ebx, 0xFFF
        sub     eax, ebx
        mov     [cur_antioffset], eax
        mov     eax, [cur_phys_page]
        add     eax, ebx
        mov     [cur_phys], eax

.check_if1:
        mov     eax, [vbuf_len]
        cmp     eax, [cur_antioffset]
        ja      .check_if2

        mov     eax, [cur_phys]
        sub     eax, [phys_region_start]
        add     eax, [vbuf_len]
        dec     eax
        mov     [dbc], eax
        mov     eax, [next_phys_page]
        mov     [new_phys_region_start], eax
        jmp     .add_prd

.check_if2:
        mov     eax, [cur_phys]
        add     eax, [cur_antioffset]
        cmp     eax, [next_phys_page]
        je      .check_if3

        mov     eax, [cur_phys]
        add     eax, [cur_antioffset]
        sub     eax, [phys_region_start]
        dec     eax
        mov     [dbc], eax
        mov     eax, [next_phys_page]
        mov     [new_phys_region_start], eax
        jmp     .add_prd

.check_if3:
        mov     eax, [cur_phys]
        add     eax, [cur_antioffset]
        sub     eax, [phys_region_start]
        cmp     eax, 4*1024*1024
        jb      .after_ifs

        mov     [dbc], 4*1024*1024 - 1
        mov     eax, [phys_region_start]
        add     eax, 4*1024*1024
        jmp     .add_prd

.after_ifs:
        jmp     .step_next

.add_prd:
        mov     ebx, [cur_prd]
        shl     ebx, BSF sizeof.HBA_PRDT_ENTRY
        add     ebx, [cmdtable]
        add     ebx, HBA_CMD_TBL.prdt_entry ; now ebx - address of 'th prdt_entry

        DEBUGF  AHCI_DBGLVL, "Added PRDT entry: dba = 0x%x, dbc = %u\n", [phys_region_start], [dbc]
        mov     eax, [phys_region_start]
        mov     [ebx + HBA_PRDT_ENTRY.dba], eax
        mov     [ebx + HBA_PRDT_ENTRY.dbau], 0
        and     [ebx + HBA_PRDT_ENTRY.flags], not 0x3FFFFF ; zero out lower 22 bits, they used for byte count
        mov     eax, [dbc]
        or      [ebx + HBA_PRDT_ENTRY.flags], eax

        inc     [cur_prd]
        mov     eax, [dbc]
        inc     eax
        add     [prdt_bytes_total], eax
        mov     eax, [new_phys_region_start]
        mov     [phys_region_start], eax
        cmp     [cur_prd], PRDT_MAX_ENTRIES
        jne     @f
        jmp     .fill_prdt_end
@@:

.step_next:
        mov     eax, [vbuf_len]
        cmp     eax, [cur_antioffset]
        jbe     @f
        mov     eax, [cur_antioffset]
@@:
        add     [vbuf], eax
        sub     [vbuf_len], eax
        jmp     .fill_prdt

.fill_prdt_end:

        mov     eax, [cmdheader]
        mov     ebx, [cur_prd]
        DEBUGF  AHCI_DBGLVL, " PRDTL = %u\n", ebx
        mov     [eax + HBA_CMD_HDR.prdtl], bx

        mov     eax, [prdt_bytes_total]
        DEBUGF  AHCI_DBGLVL, " prdt_bytes_total = %u\n", eax
        shr     eax, 9 ; /= 512
        mov     [numsectors], eax

        mov     eax, [cmdtable]
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.fis_type], FIS_TYPE_REG_H2D
        movzx   ebx, byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.flags]
        bts     ebx, bit_AHCI_H2D_FLAG_CMD ; Set Command bit in H2D FIS.
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.flags], bl

        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.command], ATA_CMD_READ_DMA_EX
        cmp     [is_write], 1
        jne     @f
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.command], ATA_CMD_WRITE_DMA_EX
@@:
        mov     ebx, dword [startsector]
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba0], bl
        shr     ebx, 8
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba1], bl
        shr     ebx, 8
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba2], bl
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.device], 1 shl 6 ; LBA mode
        shr     ebx, 8
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba3], bl
        mov     ebx, dword [startsector + 4]
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba4], bl
        shr     ebx, 8
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.lba5], bl

        mov     ebx, [numsectors]
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.countl], bl
        shr     ebx, 8
        mov     byte [eax + HBA_CMD_TBL.cfis + FIS_REG_H2D.counth], bl

        mov     edi, [esi + PORT_DATA.port] ; edi - address of HBA_PORT struct of port

        ; Wait on previous command to complete, before issuing new command.
        stdcall ahci_port_wait, edi, AHCI_PORT_TIMEOUT
        test    eax, eax
        jnz     .fail_recover

        stdcall ahci_port_update_ie, esi
        ; drop the PxIS bits accumulated by previous commands (with PxIE off
        ; nobody clears them), so that a stale error bit does not fail us
        mov     [edi + HBA_PORT.interrupt_status], 0xFFFFFFFF
        mov     [esi + PORT_DATA.intr_status], 0
        mov     eax, [cmdslot]
        bts     [edi + HBA_PORT.command_issue], eax ; Issue the command

        ; Wait for command completion
        stdcall ahci_port_cmd_wait, esi, eax
        test    eax, eax
        jnz     .fail_recover

        DEBUGF  AHCI_DBGLVL, "R/W completed\n"

;         xor     ecx, ecx
;         mov     esi, [vbuf_orig]
; .print_data:
;         cmp     ecx, 512
;         jae     .end_print_data

;         mov     al, byte [esi + ecx]
;         mov     byte [tmpstr], al
;         mov     byte [tmpstr + 1], 0
;         DEBUGF  1, "0x%x(%s) ", al:2, tmpstr

;         inc     ecx
;         jmp     .print_data
; .end_print_data:
;         DEBUGF  1, "\n"

        popad
        ;mov     eax, [cmdheader]
        ;mov     eax, [eax + HBA_CMD_HDR.prdbc]
        mov     eax, [numsectors]
        shl     eax, 9 ; *= 512
        ret

.fail_recover:
        DEBUGF  1, "K: AHCI: command failed on port %u, tfd = %x, serr = %x\n", [esi + PORT_DATA.portno], [edi + HBA_PORT.task_file_data], [edi + HBA_PORT.sata_error]
        stdcall ahci_port_error_recovery, esi
.fail:
        popad
        xor     eax, eax
        ret
endp
tmpstr    rb 16

; Read sectors
; return value: 0 = success, otherwise = error
proc ahci_read stdcall pdata: dword, buffer: dword, startsector: qword, numsectors_ptr:dword
        locals
                numsectors dd ?
                errcode    dd ?
        endl

        pushad

        mov     [errcode], DISK_STATUS_OK

        mov     ecx, [pdata]
        lea     ecx, [ecx + PORT_DATA.mutex]
        call    mutex_lock

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mov     [numsectors], eax
        DEBUGF  AHCI_DBGLVL, "  ahci_read: buffer = 0x%x, startsector = 0x%x:%x, numsectors = %u\n", [buffer], [startsector], [startsector + 4], eax

        xor     ecx, ecx ; how many sectors have been read
.read_loop:
        cmp     ecx, [numsectors]
        jae     .read_loop_end

        mov     ebx, [numsectors]
        sub     ebx, ecx
        stdcall ahci_rw_sectors, [pdata], [buffer], dword [startsector], dword [startsector + 4], ebx, 0
        test    eax, eax
        jz      .read_fail

        DEBUGF  AHCI_DBGLVL, "    EAX = 0x%x\n", eax

        add     [buffer], eax
        shr     eax, 9 ; /=  512
        add     ecx, eax
        add     dword [startsector], eax
        adc     dword [startsector + 4], 0

        jmp     .read_loop
.read_fail:
        mov     [errcode], DISK_STATUS_GENERAL_ERROR
.read_loop_end:
        mov     eax, [numsectors_ptr]
        mov     [eax], ecx ; number of sectors actually read

        mov     ecx, [pdata]
        lea     ecx, [ecx + PORT_DATA.mutex]
        call    mutex_unlock

        popad
        mov     eax, [errcode]
        ret
endp

; Write sectors
; return value: 0 = success, otherwise = error
proc ahci_write stdcall pdata: dword, buffer: dword, startsector: qword, numsectors_ptr:dword
        locals
                numsectors dd ?
                errcode    dd ?
        endl

        pushad

        mov     [errcode], DISK_STATUS_OK

        mov     ecx, [pdata]
        lea     ecx, [ecx + PORT_DATA.mutex]
        call    mutex_lock

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mov     [numsectors], eax
        DEBUGF  AHCI_DBGLVL, "  ahci_write: buffer = 0x%x, startsector = 0x%x:%x, numsectors = %u\n", [buffer], [startsector], [startsector + 4], eax

        xor     ecx, ecx ; how many sectors have been written
.write_loop:
        cmp     ecx, [numsectors]
        jae     .write_loop_end

        mov     ebx, [numsectors]
        sub     ebx, ecx
        stdcall ahci_rw_sectors, [pdata], [buffer], dword [startsector], dword [startsector + 4], ebx, 1
        test    eax, eax
        jz      .write_fail

        DEBUGF  AHCI_DBGLVL, "    EAX = 0x%x\n", eax

        add     [buffer], eax
        shr     eax, 9 ; /=  512
        add     ecx, eax
        add     dword [startsector], eax
        adc     dword [startsector + 4], 0

        jmp     .write_loop
.write_fail:
        mov     [errcode], DISK_STATUS_GENERAL_ERROR
.write_loop_end:
        mov     eax, [numsectors_ptr]
        mov     [eax], ecx ; number of sectors actually written

        mov     ecx, [pdata]
        lea     ecx, [ecx + PORT_DATA.mutex]
        call    mutex_unlock

        popad
        mov     eax, [errcode]
        ret
endp

; Start command engine
; in: eax - address of HBA_PORT structure
ahci_start_cmd:
        push    edx
        mov     edx, [timer_ticks]
        add     edx, 100 ; wait up to 1 s
.wait_cr: ; Wait until CR (bit15) is cleared
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_CR
        jnc     @f
        cmp     [timer_ticks], edx
        jb      .wait_cr
        DEBUGF  1, "K: AHCI: CR did not clear before engine start\n"
@@:
        ; Set FRE (bit4) and ST (bit0)
        bts     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE
        bts     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_ST
        pop     edx
        ret

; Stop command engine
; in: eax - address of HBA_PORT structure
ahci_stop_cmd:
        push    edx
        btr     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_ST ; Clear ST (bit0)
        mov     edx, [timer_ticks]
        add     edx, 100 ; the spec gives the HBA 500 ms to stop, wait up to 1 s
.wait_cr: ; Wait until CR (bit15) is cleared
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_CR
        jnc     @f
        cmp     [timer_ticks], edx
        jb      .wait_cr
        DEBUGF  1, "K: AHCI: CR stuck while stopping the port\n"
@@:
        btr     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE ; Clear FRE (bit4)
        mov     edx, [timer_ticks]
        add     edx, 100
.wait_fr: ; Wait until FR (bit14) is cleared
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FR
        jnc     @f
        cmp     [timer_ticks], edx
        jb      .wait_fr
        DEBUGF  1, "K: AHCI: FR stuck while stopping the port\n"
@@:
        pop     edx
        ret

; waits until the port is no longer busy before issuing a new command
; in: [port] - address of HBA_PORT structure
; [timeout] - timeout in timer ticks
; out: eax = 0 if success, 1 if timeout expired
proc ahci_port_wait stdcall, port: dword, timeout: dword
        push    ebx ecx
        mov     ebx, [port]
        mov     ecx, [timer_ticks]
        add     ecx, [timeout]
.wait:
        mov     eax, [ebx + HBA_PORT.task_file_data]
        and     eax, ATA_DEV_BUSY or ATA_DEV_DRQ
        jz      .done
        cmp     [timer_ticks], ecx
        jb      .wait
        DEBUGF  1, "K: AHCI: port is stuck busy, tfd = %x\n", [ebx + HBA_PORT.task_file_data]
        mov     eax, 1
        pop     ecx ebx
        ret
.done:
        xor     eax, eax
        pop     ecx ebx
        ret
endp


; Wait for command completion.
; Ports working on short/medium commands (ie_mode = 0) are busy-polled: the
; completion is caught by register at the exact moment it happens, like the
; old driver did. Ports working on genuinely long commands (ie_mode = 1, set
; by ahci_port_update_ie after several multi-ms commands in a row) release
; the CPU: the thread sleeps and is woken up by the port interrupt, with
; several self-defense fallbacks for broken/late interrupt delivery.
; in: [pdata] - address of PORT_DATA structure
;     [cmdslot] - number of command slot
; out: eax = 0 if success, 1 if device error, 2 if timeout
proc ahci_port_cmd_wait stdcall, pdata: dword, cmdslot: dword
        locals
            deadline    dd ?
            ev_ptr      dd ?
            ev_uid      dd ?
            spin_start  dd ?
            cmd_start   dd ?
            sleep_start dd ?
        endl
        push    ebx ecx edx esi edi
        mov     esi, [pdata]
        mov     edi, [esi + PORT_DATA.port]
        mov     eax, [timer_ticks]
        add     eax, AHCI_CMD_TIMEOUT
        mov     [deadline], eax
        rdtsc
        mov     [cmd_start], eax
.check:
        ; busy-poll first: fast completions cost neither an interrupt
        ; latency nor a reschedule
        mov     ecx, [ahci_spin_tsc]
        test    ecx, ecx
        jnz     .have_limit
        mov     eax, dword [cpu_freq]
        mov     edx, dword [cpu_freq + 4]
        mov     ecx, eax
        or      ecx, edx
        jz      .default_limit ; frequency is not measured yet (early boot)
        mov     ecx, 1000000
        div     ecx ; eax = frequency in MHz, always fits
        imul    eax, AHCI_SPIN_US
        mov     [ahci_spin_tsc], eax
        mov     ecx, eax
        jmp     .have_limit
.default_limit:
        mov     ecx, AHCI_SPIN_TSC_DEFAULT
.have_limit:
        ; In the interrupt/sleep mode (ie_mode=1, used for genuinely long
        ; commands) keep the pre-sleep poll short: the command is expected
        ; to take milliseconds, so busy-waiting for it would just burn CPU.
        ; In the poll mode (ie_mode=0) extend the poll up to twice the
        ; typical command duration (capped at 25x base, ~5 ms) so that all
        ; short and medium commands complete inside the poll without paying
        ; the interrupt-delivery latency, which on some boards is high and
        ; unstable between boots.
        cmp     [esi + PORT_DATA.ie_mode], 0
        jnz     .limit_done
        mov     eax, [esi + PORT_DATA.avg_cmd_tsc]
        add     eax, eax
        cmp     eax, ecx
        jbe     .limit_done
        lea     edx, [ecx*4 + ecx]
        lea     edx, [edx*4 + edx] ; edx = base * 25
        cmp     eax, edx
        jbe     @f
        mov     eax, edx
@@:
        mov     ecx, eax
.limit_done:
        rdtsc
        mov     [spin_start], eax ; low dword is enough for sub-second intervals
        mov     ebx, [cmdslot]
.spin:
        bt      [edi + HBA_PORT.command_issue], ebx
        jnc     .completed
        test    [esi + PORT_DATA.intr_status], AHCI_PORT_INTR_ERR_MASK
        jnz     .error
        test    [edi + HBA_PORT.interrupt_status], AHCI_PORT_INTR_ERR_MASK
        jnz     .error
        rdtsc
        sub     eax, [spin_start]
        cmp     eax, ecx
        jb      .spin

        mov     eax, [timer_ticks]
        cmp     eax, [deadline]
        jae     .timeout

        mov     eax, [esi + PORT_DATA.ctr_ptr]
        cmp     [eax + AHCI_CTR.polling_mode], 0
        jnz     .yield
        ; with the port interrupts off (short/medium commands) there is
        ; nothing to wake us up: keep busy-polling the completion by register,
        ; exactly like the old driver. This catches the completion at the
        ; moment it happens, with none of the interrupt-delivery latency that
        ; on this board is high and unstable between boots and wrecked the
        ; random tests. The deadline check above bounds a truly hung command.
        cmp     [esi + PORT_DATA.ie_mode], 0
        jz      .check
        ; if interrupt wakeups on this port have proven to be late or
        ; missing, don't sleep: poll politely instead (old-driver speed);
        ; every couple of seconds probe the interrupt path again
        cmp     [esi + PORT_DATA.wake_slow], 0
        jz      .can_sleep
        mov     eax, [timer_ticks]
        cmp     eax, [esi + PORT_DATA.next_probe]
        jb      .yield
        add     eax, 200
        mov     [esi + PORT_DATA.next_probe], eax
.can_sleep:

        ; sleep until the interrupt handler wakes us up
        push    esi edi
        xor     ecx, ecx
        xor     esi, esi
        call    create_event ; -> eax = event, edx = uid
        pop     edi esi
        test    eax, eax
        jz      .yield ; no free events - poll
        mov     [ev_ptr], eax
        mov     [ev_uid], edx
        mov     [esi + PORT_DATA.event_uid], edx
        mov     [esi + PORT_DATA.event_ptr], eax

        ; the command may have completed while the event was being prepared;
        ; re-check to avoid sleeping through an already-delivered interrupt
        mov     edx, [cmdslot]
        bt      [edi + HBA_PORT.command_issue], edx
        jnc     .cancel_wait

        ; Sleep in 1-tick quanta: if the interrupt gets lost or is delivered
        ; on a wrong IRQ line (weird routing - has been seen in the wild),
        ; the completed command is still picked up on the next tick, and such
        ; completions are counted below to switch the controller to polling.
        mov     ecx, [esi + PORT_DATA.ctr_ptr]
        inc     [ecx + AHCI_CTR.sleeps_total]
        rdtsc
        mov     [sleep_start], eax
        mov     eax, [ev_ptr]
        mov     ebx, [ev_uid]
        mov     ecx, 1
        call    wait_event_timeout
        mov     esi, [pdata]
        mov     edi, [esi + PORT_DATA.port]
        mov     [esi + PORT_DATA.event_ptr], 0
        test    eax, eax
        jz      .wait_expired
        ; the interrupt has woken us up; the event is already destroyed.
        ; A sleep much longer than the typical command on this port means
        ; the completion interrupt was delivered late (e.g. piggybacked on
        ; the timer tick by the wrong-IRQ recovery path) - treat it the
        ; same as a missing interrupt.
        rdtsc
        sub     eax, [sleep_start]
        mov     ecx, [esi + PORT_DATA.avg_cmd_tsc]
        mov     edx, [ahci_spin_tsc]
        test    edx, edx
        jnz     @f
        mov     edx, AHCI_SPIN_TSC_DEFAULT
@@:
        imul    edx, edx, 35 ; ~0.7 of a scheduler tick, in TSC ticks
        add     ecx, edx ; sleeping longer than the typical command plus a
                         ; good part of a tick = the wakeup came a tick late
        cmp     eax, ecx
        jbe     .wake_was_fast
        call    .note_slow_wakeup
        jmp     .after_streaks
.wake_was_fast:
        mov     [esi + PORT_DATA.slow_streak], 0
        inc     [esi + PORT_DATA.fast_streak]
        cmp     [esi + PORT_DATA.fast_streak], 4
        jne     .after_streaks
        cmp     [esi + PORT_DATA.wake_slow], 0
        jz      .after_streaks
        mov     [esi + PORT_DATA.wake_slow], 0
        DEBUGF  1, "K: AHCI: port %u: interrupt wakeups are fine again\n", [esi + PORT_DATA.portno]
.after_streaks:
        mov     eax, [esi + PORT_DATA.ctr_ptr]
        mov     [eax + AHCI_CTR.irq_miss_count], 0
        jmp     .check

.wait_expired:
        ; the event was not raised, destroy it manually
        mov     eax, [ev_ptr]
        mov     ebx, [ev_uid]
        call    destroy_event
        ; if the command has actually completed, its interrupt got lost;
        ; count such completions and switch the controller to polling mode
        ; when it becomes clear that interrupts do not work
        mov     edx, [cmdslot]
        bt      [edi + HBA_PORT.command_issue], edx
        jc      .check
        cmp     [esi + PORT_DATA.ie_mode], 0
        jz      .check ; interrupts were legitimately off - not a miss
        ; the command was done, yet no interrupt woke us up
        call    .note_slow_wakeup
        mov     eax, [esi + PORT_DATA.ctr_ptr]
        inc     [eax + AHCI_CTR.miss_total]
        inc     [eax + AHCI_CTR.irq_miss_count]
        cmp     [eax + AHCI_CTR.irq_miss_count], AHCI_IRQ_MISS_LIMIT
        jb      .check
        cmp     [eax + AHCI_CTR.polling_mode], 0
        jnz     .check
        mov     [eax + AHCI_CTR.polling_mode], 1
        ; silence the controller so that a possibly shared and unhandled
        ; INTx line is not held active by accumulating PxIS bits
        mov     eax, [eax + AHCI_CTR.abar]
        btr     [eax + HBA_MEM.ghc], bit_AHCI_HBA_GHC_INTERRUPT_ENABLE
        mov     [edi + HBA_PORT.interrupt_enable], 0
        mov     [esi + PORT_DATA.ie_mode], 0
        DEBUGF  1, "K: AHCI: interrupts are not arriving, falling back to polling\n"
        jmp     .check

.cancel_wait:
        ; completion raced with event setup - drop the event and re-check
        mov     [esi + PORT_DATA.event_ptr], 0
        mov     eax, [ev_ptr]
        mov     ebx, [ev_uid]
        call    destroy_event
        jmp     .check

.yield:
        call    change_task
        mov     esi, [pdata]
        mov     edi, [esi + PORT_DATA.port]
        jmp     .check

; local helper: a sleep ended without a timely interrupt; after a couple of
; those in a row stop sleeping on this port and poll politely instead
.note_slow_wakeup:
        mov     [esi + PORT_DATA.fast_streak], 0
        inc     [esi + PORT_DATA.slow_streak]
        cmp     [esi + PORT_DATA.slow_streak], 2
        jb      .nsw_ret
        cmp     [esi + PORT_DATA.wake_slow], 0
        jnz     .nsw_ret
        mov     [esi + PORT_DATA.wake_slow], 1
        mov     eax, [timer_ticks]
        add     eax, 200
        mov     [esi + PORT_DATA.next_probe], eax
        DEBUGF  1, "K: AHCI: port %u: interrupt wakeups are late, polling instead of sleeping\n", [esi + PORT_DATA.portno]
.nsw_ret:
        retn

.completed:
        rdtsc
        sub     eax, [cmd_start] ; eax = command duration in TSC ticks
        ; classify the command as long or short relative to the sleep
        ; threshold (10x base poll, ~2 ms) and keep run lengths. The sleep
        ; decision is made from these runs, not from the smoothed average:
        ; a single huge outlier (disk spin-up, sector remap - seen at 150 ms)
        ; would poison an EWMA and wrongly flip the whole port into sleep
        ; mode, where the random workload then eats the interrupt-delivery
        ; latency. Requiring several long commands in a row ignores outliers.
        mov     ecx, [ahci_spin_tsc]
        test    ecx, ecx
        jnz     @f
        mov     ecx, AHCI_SPIN_TSC_DEFAULT
@@:
        imul    ecx, ecx, 10
        cmp     eax, ecx
        jb      .cmd_short
        inc     [esi + PORT_DATA.long_run]
        mov     [esi + PORT_DATA.short_run], 0
        jmp     .upd_avg
.cmd_short:
        inc     [esi + PORT_DATA.short_run]
        mov     [esi + PORT_DATA.long_run], 0
.upd_avg:
        ; smoothed duration for the busy-poll length: avg = avg*3/4 + dur/4,
        ; with the sample clipped to the threshold so one outlier cannot blow
        ; the poll budget up either
        cmp     eax, ecx
        jb      @f
        mov     eax, ecx
@@:
        mov     ecx, [esi + PORT_DATA.avg_cmd_tsc]
        mov     edx, ecx
        shr     edx, 2
        sub     ecx, edx
        mov     edx, eax
        shr     edx, 2
        add     ecx, edx
        mov     [esi + PORT_DATA.avg_cmd_tsc], ecx
        ; diagnostics: report the counters to the debug board once in a while
        mov     ecx, [esi + PORT_DATA.ctr_ptr]
        inc     [ecx + AHCI_CTR.cmds_total]
        test    byte [ecx + AHCI_CTR.cmds_total], 0xFF
        jnz     @f
        DEBUGF  1, "K: AHCI stats: cmds=%u sleeps=%u misses=%u irqs=%u avg_tsc=%u ie=%u slow=%u\n", [ecx + AHCI_CTR.cmds_total], [ecx + AHCI_CTR.sleeps_total], [ecx + AHCI_CTR.miss_total], [ecx + AHCI_CTR.intr_count], [esi + PORT_DATA.avg_cmd_tsc], [esi + PORT_DATA.ie_mode], [esi + PORT_DATA.wake_slow]
@@:
        test    [esi + PORT_DATA.intr_status], AHCI_PORT_INTR_ERR_MASK
        jnz     .error
        test    [edi + HBA_PORT.interrupt_status], AHCI_PORT_INTR_ERR_MASK
        jnz     .error
        xor     eax, eax
        jmp     .ret
.error:
        mov     eax, 1
        jmp     .ret
.timeout:
        DEBUGF  1, "K: AHCI: command timeout on port %u, ci = %x, tfd = %x\n", [esi + PORT_DATA.portno], [edi + HBA_PORT.command_issue], [edi + HBA_PORT.task_file_data]
        mov     eax, 2
.ret:
        pop     edi esi edx ecx ebx
        ret
endp


; Decide whether the port should have its interrupts enabled before a
; command is issued. Ports with typically long commands (mechanical seeks)
; sleep and need the interrupt to be woken up; ports with fast commands are
; caught by the busy-poll, and processing an interrupt for every command
; would only slow them down. Hysteresis keeps the switching rare.
; Note: the switch must happen before the command is issued - at least some
; chipsets do not generate an interrupt when PxIE gets enabled while the
; command is already in flight (seen on real hardware).
; in: [pdata] - address of PORT_DATA structure
proc ahci_port_update_ie stdcall, pdata: dword
        push    eax edx
        mov     edx, [pdata]
        cmp     [edx + PORT_DATA.ie_mode], 0
        jnz     .currently_on
        ; interrupts are off: enable them (and start sleeping) only after
        ; several genuinely long commands in a row (real seeks / sequential
        ; transfers). Short and medium commands - all of HDD random access,
        ; hundreds of us - are kept in the busy-poll, which catches the
        ; completion at the exact moment it happens, without the interrupt-
        ; delivery latency that on this board varies a lot between boots and
        ; wrecked the random tests. Using a run length (not an average)
        ; makes a rare huge outlier unable to flip the port into sleep mode.
        mov     eax, [edx + PORT_DATA.ctr_ptr]
        cmp     [eax + AHCI_CTR.polling_mode], 0
        jnz     .done
        cmp     [edx + PORT_DATA.long_run], 3
        jb      .done
        mov     eax, [edx + PORT_DATA.port]
        mov     [eax + HBA_PORT.interrupt_status], 0xFFFFFFFF ; drop stale bits first
        mov     [eax + HBA_PORT.interrupt_enable], AHCI_PORT_INTR_MASK
        mov     [edx + PORT_DATA.ie_mode], 1
        jmp     .done
.currently_on:
        ; interrupts are on: disable them after several short commands in a
        ; row (the long burst is over, back to random/idle access)
        cmp     [edx + PORT_DATA.short_run], 3
        jb      .done
        mov     eax, [edx + PORT_DATA.port]
        mov     [eax + HBA_PORT.interrupt_enable], 0
        mov     [edx + PORT_DATA.ie_mode], 0
.done:
        pop     edx eax
        ret
endp


; AHCI interrupt handler
; in: [esp+4] - AHCI_CTR structure of the controller (arg of attach_int_handler)
; out: eax = 1 if the interrupt came from this controller, 0 otherwise
align 4
ahci_irq_handler:
        mov     esi, [esp + 4]
        test    esi, esi
        jz      .not_our
        mov     edx, [esi + AHCI_CTR.abar]
        test    edx, edx
        jz      .not_our
        mov     eax, [edx + HBA_MEM.is]
        test    eax, eax
        jz      .not_our
        inc     [esi + AHCI_CTR.intr_count]
        cmp     [esi + AHCI_CTR.intr_count], 1
        jne     @f
        DEBUGF  1, "K: AHCI: first interrupt received\n"
@@:
        push    8 ; bound for the handle-ack-recheck loop, just in case
.handle_loop:
        push    eax ; snapshot of HBA_MEM.is
        xor     ecx, ecx
.port_loop:
        bt      eax, ecx
        jnc     .next_port

        mov     edi, ecx
        imul    edi, sizeof.HBA_PORT
        lea     edi, [edx + HBA_MEM.ports + edi]
        mov     ebx, [edi + HBA_PORT.interrupt_status]
        mov     [edi + HBA_PORT.interrupt_status], ebx ; clear what has been seen (RWC)

        ; accumulate the status for the waiting thread and wake it up
        push    eax edx
        mov     eax, ecx
        imul    eax, sizeof.PORT_DATA
        lea     eax, [esi + AHCI_CTR.port_data_arr + eax]
        or      [eax + PORT_DATA.intr_status], ebx
        mov     edx, [eax + PORT_DATA.event_ptr]
        test    edx, edx
        jz      .no_waiter
        pushad
        mov     ebx, [eax + PORT_DATA.event_uid]
        mov     eax, edx
        xor     edx, edx
        xor     esi, esi
        call    raise_event
        popad
.no_waiter:
        pop     edx eax
.next_port:
        inc     ecx
        cmp     ecx, AHCI_MAX_PORTS
        jb      .port_loop

        ; acknowledge IS after the PxIS bits are cleared - acknowledging
        ; before that makes the controller re-latch IS from the still-set
        ; PxIS and fire a second, spurious interrupt (seen on ICH);
        ; then re-check IS to not lose an event that arrived meanwhile
        pop     eax
        mov     [edx + HBA_MEM.is], eax
        mov     eax, [edx + HBA_MEM.is]
        test    eax, eax
        jz      @f
        dec     dword [esp]
        jnz     .handle_loop
@@:
        pop     eax
        mov     eax, 1
        ret
.not_our:
        xor     eax, eax
        ret


; Issue COMRESET on the port link and wait for a device to be detected.
; The port command engine must be stopped; FRE should be enabled so that the
; signature FIS sent by the device after the reset is received.
; in: [port] - address of HBA_PORT structure
; out: eax = 0 - device detected, 1 - no device
proc ahci_port_comreset stdcall, port: dword
        push    ebx ecx edx
        mov     edx, [port]
        mov     eax, [edx + HBA_PORT.sata_control]
        and     eax, not 0xFFF
        or      eax, 0x301 ; DET = 1 (start COMRESET), IPM = 3 (disable partial/slumber)
        mov     [edx + HBA_PORT.sata_control], eax
        mov     ebx, 2 ; keep the reset active for 20 ms (spec minimum is 1 ms)
        call    delay_hs
        mov     eax, [edx + HBA_PORT.sata_control]
        and     eax, not 0xF ; DET = 0, resume communication
        mov     [edx + HBA_PORT.sata_control], eax

        mov     ecx, 30 ; wait up to 300 ms for the phy to come up
.wait_det:
        mov     eax, [edx + HBA_PORT.sata_status]
        and     eax, AHCI_HBA_PxSSTS_DET
        cmp     eax, AHCI_HBA_PxSSTS_DET_PRESENT
        je      .present
        mov     ebx, 1
        call    delay_hs
        dec     ecx
        jnz     .wait_det
        mov     [edx + HBA_PORT.sata_error], 0xFFFFFFFF
        mov     eax, 1
        jmp     .ret
.present:
        mov     [edx + HBA_PORT.sata_error], 0xFFFFFFFF ; clear diagnostic bits set by the reset
        xor     eax, eax
.ret:
        pop     edx ecx ebx
        ret
endp


; Try to bring the port back to a working state after a command error/timeout
; (AHCI spec 6.2.2.1)
; in: [pdata] - address of PORT_DATA structure
proc ahci_port_error_recovery stdcall, pdata: dword
        push    eax esi edi
        mov     esi, [pdata]
        mov     edi, [esi + PORT_DATA.port]
        DEBUGF  1, "K: AHCI: recovering port %u\n", [esi + PORT_DATA.portno]
        mov     eax, edi
        call    ahci_stop_cmd
        mov     [edi + HBA_PORT.sata_error], 0xFFFFFFFF
        mov     [edi + HBA_PORT.interrupt_status], 0xFFFFFFFF
        mov     [edi + HBA_PORT.interrupt_enable], 0
        mov     [esi + PORT_DATA.intr_status], 0
        mov     [esi + PORT_DATA.ie_mode], 0
        ; if the device is stuck in busy state, reset the link
        mov     eax, [edi + HBA_PORT.task_file_data]
        test    eax, ATA_DEV_BUSY or ATA_DEV_DRQ
        jz      @f
        ; FRE must be on for the comreset, ahci_stop_cmd has cleared it
        bts     [edi + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE
        stdcall ahci_port_comreset, edi
@@:
        mov     eax, edi
        call    ahci_start_cmd
        pop     edi esi eax
        ret
endp


; Check whether the port signature belongs to a known device type
; in: eax - PxSIG value
; out: ZF set if the signature is a known one
ahci_is_sig_known:
        cmp     eax, SATA_SIG_ATA
        je      .known
        cmp     eax, SATA_SIG_ATAPI
        je      .known
        cmp     eax, SATA_SIG_SEMB
        je      .known
        cmp     eax, SATA_SIG_PM
.known:
        ret

; ; The commands may not take effect until the command
; ; register is read again by software, because reasons.
; ; in: eax - address of HBA_PORT structure
; ; out: eax - command register value
; ahci_flush_cmd:
;         mov     eax, [eax + HBA_PORT.command]
;         ret

; ; Send command to port
; ; in: eax - address of HBA_PORT structure
; ;     ebx - index of command slot
; ahci_send_cmd:
;         push    ecx
;         mov     [eax + HBA_PORT.interrupt_status], 0xFFFFFFFF

;         mov     cl, bl
;         mov     [eax + HBA_PORT.command_issue], 1
;         shl     [eax + HBA_PORT.command_issue], cl

;         call    ahci_flush_cmd
;         pop     ecx
;         ret

; ---------------------------------------------------------------------------
; in: port - address of HBA_PORT structure
;     portno - port index (0..31)
;     pdata - address of PORT_DATA structure
; out:
;     rebases port and fills pdata with mappings
proc ahci_port_rebase stdcall, port: dword, portno: dword, pdata: dword
        locals
            phys_page1  dd ?
            virt_page1  dd ?
            phys_page23 dd ?
            virt_page23 dd ?
            tmp         dd ?
            result      dd ?
        endl

        pushad

        DEBUGF  1, "Rebasing port %u\n", [portno]

        mov     [result], 1

        mov     eax, [port]
        call    ahci_stop_cmd

        ; Command list entry size = 32
        ; Command list entry maxim count = 32
        ; Command list maxim size = 32*32 = 1K per port
        call    alloc_page
        mov     [phys_page1], eax

        stdcall map_io_mem, eax, 4096, PG_NOCACHE + PG_SWR  ; map to virt memory so we can work with it
        mov     [virt_page1], eax

        mov     esi, [port]
        mov     ebx, [phys_page1]
        mov     [esi + HBA_PORT.command_list_base_l], ebx ; set the command list base
        mov     [esi + HBA_PORT.command_list_base_h], 0  ; zero upper 32 bits of addr cause we are 32 bit os

        mov     edi, [pdata]
        mov     ebx, [virt_page1]
        mov     [edi + PORT_DATA.clb], ebx ; set pdata->clb

        mov     eax, [port]
        mov     [edi + PORT_DATA.port], eax ; set pdata->port
        mov     eax, [portno]               ; set pdata->portno
        mov     [edi + PORT_DATA.portno], eax

        stdcall _memset, ebx, 0, 1024 ; zero out the command list

        ; FIS entry size = 256 bytes per port
        mov     eax, [phys_page1]
        add     eax, 1024
        mov     [esi + HBA_PORT.fis_base_l], eax
        mov     [esi + HBA_PORT.fis_base_h], 0

        mov     eax, [virt_page1]
        add     eax, 1024
        mov     [edi + PORT_DATA.fb], eax ; set pdata->fb
        stdcall _memset, eax, 0, 256 ; zero out

        ; The FIS base is set up now, enable FIS receive so that the device
        ; signature FIS is accepted (also needed for COMRESET detection below)
        mov     eax, [port]
        bts     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE

        stdcall alloc_pages, 32*(64 + 16 + 48 + PRDT_MAX_ENTRIES*16)/4096
        mov     [phys_page23], eax
        stdcall map_io_mem, eax, 32*(64 + 16 + 48 + PRDT_MAX_ENTRIES*16), PG_NOCACHE + PG_SWR
        mov     [virt_page23], eax

        ; Command table size = N*32 per port
        mov     edx, [edi + PORT_DATA.clb] ; cmdheader array base
        xor     ecx, ecx

.for1:
        cmp     ecx, 32
        jae     .for1_end

        mov     ebx, ecx
        shl     ebx, BSF sizeof.HBA_CMD_HDR
        add     ebx, edx ; ebx = cmdheader[ecx]

        mov     [ebx + HBA_CMD_HDR.prdtl], PRDT_MAX_ENTRIES ; prdt entries per command table

        ; bytes per command table = 64+16+48+PRDT_MAX_ENTRIES*16 = N

        push    edx

        ; cmdheader[ecx].ctba = phys_page23 + ecx*N
        mov     [ebx + HBA_CMD_HDR.ctba], ecx
        mov     edx, [ebx + HBA_CMD_HDR.ctba]
        imul    edx, (64+16+48+PRDT_MAX_ENTRIES*16) ; *= N
        mov     [ebx + HBA_CMD_HDR.ctba], edx
        mov     eax, [ebx + HBA_CMD_HDR.ctba]
        mov     edx, [phys_page23]
        add     [ebx + HBA_CMD_HDR.ctba], edx

        add     eax, [virt_page23]
        mov     [tmp], eax  ; tmp = virt_page23 + ecx*N
        lea     eax, [ecx*4 + edi + PORT_DATA.ctba_arr] ; eax = pdata->ctba_arr[ecx]
        mov     edx, [tmp]
        mov     [eax], edx  ; pdata->ctba_arr[ecx] = virt_page23 + ecx*N

        pop     edx

        mov     [ebx + HBA_CMD_HDR.ctbau], 0
        stdcall _memset, [eax], 0, 64+16+48+PRDT_MAX_ENTRIES*16 ; zero out

        inc     ecx
        jmp     .for1
.for1_end:

        ; If the link is not established or the port reports a bogus device
        ; signature (both happen when BIOS leaves the port uninitialized),
        ; reset the link: the device will resend its signature FIS.
        mov     esi, [port]
        mov     eax, [esi + HBA_PORT.sata_status]
        and     eax, AHCI_HBA_PxSSTS_DET
        cmp     eax, AHCI_HBA_PxSSTS_DET_PRESENT
        jne     .link_reset
        mov     eax, [esi + HBA_PORT.signature]
        call    ahci_is_sig_known
        jz      .link_ok
.link_reset:
        DEBUGF  1, "K: AHCI: port %u: link not ready (ssts = %x, sig = %x), doing COMRESET\n", [portno], [esi + HBA_PORT.sata_status], [esi + HBA_PORT.signature]
        stdcall ahci_port_comreset, esi
        test    eax, eax
        jnz     .no_device
.link_ok:
        ; wait for the device to become ready and send its signature
        mov     edx, [timer_ticks]
        add     edx, 100 ; up to 1 s (the drive may be spinning up)
.wait_ready:
        mov     eax, [esi + HBA_PORT.task_file_data]
        test    eax, ATA_DEV_BUSY or ATA_DEV_DRQ
        jnz     .not_ready_yet
        mov     eax, [esi + HBA_PORT.signature]
        call    ahci_is_sig_known
        jz      .dev_ready
.not_ready_yet:
        cmp     [timer_ticks], edx
        jae     .ready_timeout
        mov     ebx, 1
        call    delay_hs
        jmp     .wait_ready
.ready_timeout:
        DEBUGF  1, "K: AHCI: port %u: device not ready, tfd = %x, sig = %x\n", [portno], [esi + HBA_PORT.task_file_data], [esi + HBA_PORT.signature]
        ; proceed anyway if the link is up: IDENTIFY will be the final judge
        mov     eax, [esi + HBA_PORT.sata_status]
        and     eax, AHCI_HBA_PxSSTS_DET
        cmp     eax, AHCI_HBA_PxSSTS_DET_PRESENT
        jne     .no_device
.dev_ready:
        ; clear everything accumulated during the bring-up; per-port
        ; interrupts start disabled - ahci_port_update_ie enables them
        ; later, only when the port works on genuinely long commands,
        ; so fast commands do not pay for interrupt processing at all
        mov     [esi + HBA_PORT.sata_error], 0xFFFFFFFF
        mov     [esi + HBA_PORT.interrupt_status], 0xFFFFFFFF
        mov     [esi + HBA_PORT.interrupt_enable], 0
        mov     eax, [pdata]
        mov     [eax + PORT_DATA.intr_status], 0
        mov     [eax + PORT_DATA.ie_mode], 0

        mov     eax, [port]
        call    ahci_start_cmd

        mov     [result], 0
        DEBUGF  1, "End rebasing port %u\n", [portno]
.ret:
        popad
        mov     eax, [result]
        ret
.no_device:
        DEBUGF  1, "K: AHCI: port %u: no device detected\n", [portno]
        jmp     .ret
endp

; ----------------------------------------------------------- ; TODO check
; Find a free command list slot
; in: pdata - address of HBA_PORT structure
; out: eax - if not found -1, else slot index
proc ahci_find_cmdslot stdcall, pdata: dword
        push    ebx ecx edx esi
        mov     esi, [pdata]
        mov     eax, [esi + PORT_DATA.port]

        ; If not set in SACT and CI, the slot is free
        mov     ebx, [eax + HBA_PORT.sata_active]
        or      ebx, [eax + HBA_PORT.command_issue] ; ebx = slots

        mov     esi, [esi + PORT_DATA.ctr_ptr]
        mov     esi, [esi + AHCI_CTR.abar]
        mov     edx, [esi + HBA_MEM.cap]
        shr     edx, 8
        and     edx, 0xf
        ; DEBUGF  1, "Number of Command Slots on each port = %u\n", edx
        xor     ecx, ecx
.for1:
        cmp     ecx, edx
        jae     .for1_end

        ; if ((slots&1) == 0) return i;
        bt      ebx, 0
        jc      .cont1

        mov     eax, ecx
        jmp     .ret

.cont1:
        shr     ebx, 1
        inc     ecx
        jmp     .for1
.for1_end:
        DEBUGF  1, "Cannot find free command list entry\n"
        mov     eax, -1
.ret:
        pop     esi edx ecx ebx
        ret
endp


proc _memset stdcall, dest:dword, val:byte, cnt:dword ; doesn't clobber any registers
        ;DEBUGF  DBG_INFO, "memset(%x, %u, %u)\n", [dest], [val], [cnt]
        push    eax ecx edi
        mov     edi, dword [dest]
        mov     al, byte [val]
        mov     ecx, dword [cnt]
        rep stosb
        pop     edi ecx eax
        ret
endp

; Swaps byte order in words
; base - address of first word
; len - how many words to swap bytes in
; doesn't clobber any registers
proc swap_bytes_in_words stdcall, base: dword, len: dword
        push    eax ebx ecx
        xor     ecx, ecx
        mov     ebx, [base]
.loop:
        cmp     ecx, [len]
        jae     .loop_end
        mov     ax, word [ebx + ecx*2]
        xchg    ah, al
        mov     word [ebx + ecx*2], ax
        inc     ecx
        jmp     .loop
.loop_end:
        pop     ecx ebx eax
        ret
endp
