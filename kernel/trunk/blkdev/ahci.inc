;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

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
bit_AHCI_HBA_BOHC_BB   = 4         ; BIOS Busy (polling bit while BIOS cleans up 

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

AHCI_PORT_TIMEOUT = 1000000

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
    port          dd ? ; address of correspoding HBA_PORT structure
    portno        dd ? ; port index, 0..31
    drive_type    db ? ; drive type
    sector_count  dq ? ; number of sectors
    ctr_ptr       dd ? ; pointer to controller to which port belongs
ends

; Register FIS – Host to Device
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

; Register FIS – Device to Host
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

; Data FIS – Bidirectional
struct FIS_DATA
    fis_type      db ?           ; FIS_TYPE_DATA
    flags        db ?           ; 0bRRRRPPPP, R - Reserved, P - Port multiplier
                  rb 2           ; Reserved
    ; DWORD 1 ~ N (?)
    data          rd 1           ; Payload
ends

; PIO Setup – Device to Host
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

; DMA Setup – Device to Host
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
        mutex            MUTEX
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
        add     ecx, AHCI_CTR.mutex
        call    mutex_init

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

.wait_not_bos:
        bt      [esi + HBA_MEM.bohc], bit_AHCI_HBA_BOHC_BOS
        jc      .wait_not_bos

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

        ; wait for reset to complete
.wait_reset:
        bt      [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_RESET
        jc      .wait_reset

        ; enable the AHCI and interrupts
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_AHCI_ENABLE
        bts     [esi + HBA_MEM.ghc], bit_AHCI_HBA_GHC_INTERRUPT_ENABLE
        mov     ebx, 2
        call    delay_hs
        
        DEBUGF  1, "K: AHCI: caps: %x %x, ver: %x, ghc: %x, pi: %x\n", [esi + HBA_MEM.cap], [esi + HBA_MEM.cap2], [esi + HBA_MEM.version], [esi + HBA_MEM.ghc], [esi + HBA_MEM.pi]

        ; TODO:
        ; calculate irq line
        ; ahciHBA->ghc |= AHCI_GHC_IE;
        ; IDT::RegisterInterruptHandler(irq, InterruptHandler);
        ; ahciHBA->is = 0xffffffff;

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

        mov     ecx, [edi + HBA_PORT.sata_status]
        shr     ecx, 8
        and     ecx, 0x0F
        cmp     ecx, AHCI_HBA_PORT_IPM_ACTIVE
        jne     .continue_detect_drives

        mov     ecx, [edi + HBA_PORT.sata_status]
        and     ecx, AHCI_HBA_PxSSTS_DET
        cmp     ecx, AHCI_HBA_PxSSTS_DET_PRESENT
        jne     .continue_detect_drives

        ; DEBUGF  1, "K: AHCI: found drive at port %d, cmd = 0x%x, ssts = 0x%x, signature = 0x%x\n", ebx, [edi + HBA_PORT.command], [edi + HBA_PORT.sata_status], [edi + HBA_PORT.signature]  

        mov     ecx, ebx
        imul    ecx, sizeof.PORT_DATA
        add     ecx, AHCI_CTR.port_data_arr
        add     ecx, [ctr_ptr]
        mov     eax, [ctr_ptr]
        mov     [ecx + PORT_DATA.ctr_ptr], eax ; to which controller the port belongs
        stdcall ahci_port_rebase, edi, ebx, ecx

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

        DEBUGF  1, "Unknown device signature\n"
        mov     eax, AHCI_DEV_NULL
.end_switch_sig:
        mov     [ecx + PORT_DATA.drive_type], al

        DEBUGF  1, "K: AHCI: found drive on port %u: TYPE = %u\n", ebx, [ecx + PORT_DATA.drive_type]

        stdcall ahci_port_identify, ecx

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
; Identify drive on port ; TODO check
; in: pdata - address of PORT_DATA structure
proc ahci_port_identify stdcall, pdata: dword
        locals
            cmdslot dd ?
            cmdheader dd ?
            cmdtable  dd ?
            buf_phys  dd ?
            buf_virt  dd ?
        endl

        pushad
        
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
        mov     [buf_phys], eax

        stdcall map_io_mem, eax, 4096, PG_NOCACHE + PG_SWR  ; map to virt memory so we can work with it
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
        ; DEBUGF  1, "eax = %x\n", eax
        ; TODO check eax error value

        mov     eax, [cmdslot]
        bts     [edi + HBA_PORT.command_issue], eax ; Issue the command

        ; Wait for command completion
        stdcall ahci_port_cmd_wait, edi, eax;, AHCI_PORT_CMD_TIMEOUT
        ; DEBUGF  1, " eax = %x\n", eax
        ; TODO check eax error value

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

        shrd    eax, edx, 11 ; i.e *512 / 1024 / 1024, 512 - sector size
        DEBUGF  1, "disk capacity = %u MiB ", eax
        shrd    eax, edx, 10 ; / 1024
        DEBUGF  1, "= %u GiB\n", eax
.ret:
        popad
        ret
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

        mov     eax, [cmdslot]
        bts     [edi + HBA_PORT.command_issue], eax ; Issue the command

        ; Wait for command completion
        stdcall ahci_port_cmd_wait, edi, eax;, AHCI_PORT_CMD_TIMEOUT

        DEBUGF  AHCI_DBGLVL, "sata_error register = 0x%x\n", [edi + HBA_PORT.sata_error]

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
        endl

        pushad
 
        mov     ecx, [pdata]
        mov     ecx, [ecx + PORT_DATA.ctr_ptr]
        mov     ecx, [ecx + AHCI_CTR.mutex]
        call    mutex_lock

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mov     [numsectors], eax
        DEBUGF  AHCI_DBGLVL, "  ahci_read: buffer = 0x%x, startsector = 0x%x:%x, numsectors = %u\n", [buffer], [startsector], [startsector + 4], eax

        xor     ecx, ecx ; how many sectors have been read
.read_loop:
        cmp     ecx, [numsectors]
        jae     .read_loop_end

        ; mov     eax, [buffer]
        ; call    get_pg_addr
        ; DEBUGF  1, "buf phys = 0x%x\n", eax
        ; mov     eax, [buffer]
        ; add     eax, 4096
        ; call    get_pg_addr
        ; DEBUGF  1, "buf + 4096 phys = 0x%x\n", eax

        mov     ebx, [numsectors]
        sub     ebx, ecx
        ; DEBUGF  1, "buffer = 0x%x\n", [buffer]
        stdcall ahci_rw_sectors, [pdata], [buffer], dword [startsector], dword [startsector + 4], ebx, 0
        ;; TODO check if eax == 0 ?

        DEBUGF  AHCI_DBGLVL, "    EAX = 0x%x\n", eax

        add     [buffer], eax
        shr     eax, 9 ; /=  512
        add     ecx, eax
        add     dword [startsector], eax
        adc     dword [startsector + 4], 0

        jmp     .read_loop
.read_loop_end:

        mov     ecx, [pdata]
        mov     ecx, [ecx + PORT_DATA.ctr_ptr]
        mov     ecx, [ecx + AHCI_CTR.mutex]
        call    mutex_unlock
 
        popad
        xor     eax, eax
        ret
endp

; Write sectors
; return value: 0 = success, otherwise = error
proc ahci_write stdcall pdata: dword, buffer: dword, startsector: qword, numsectors_ptr:dword
        locals
                numsectors dd ?
        endl

        pushad
 
        mov     ecx, [pdata]
        mov     ecx, [ecx + PORT_DATA.ctr_ptr]
        mov     ecx, [ecx + AHCI_CTR.mutex]
        call    mutex_lock

        mov     eax, [numsectors_ptr]
        mov     eax, [eax]
        mov     [numsectors], eax
        DEBUGF  AHCI_DBGLVL, "  ahci_write: buffer = 0x%x, startsector = 0x%x:%x, numsectors = %u\n", [buffer], [startsector], [startsector + 4], eax

        xor     ecx, ecx ; how many sectors have been read
.write_loop:
        cmp     ecx, [numsectors]
        jae     .write_loop_end

        mov     ebx, [numsectors]
        sub     ebx, ecx
        stdcall ahci_rw_sectors, [pdata], [buffer], dword [startsector], dword [startsector + 4], ebx, 1
        ;; TODO check if eax == 0 ?

        DEBUGF  AHCI_DBGLVL, "    EAX = 0x%x\n", eax

        add     [buffer], eax
        shr     eax, 9 ; /=  512
        add     ecx, eax
        add     dword [startsector], eax
        adc     dword [startsector + 4], 0

        jmp     .write_loop
.write_loop_end:

        mov     ecx, [pdata]
        mov     ecx, [ecx + PORT_DATA.ctr_ptr]
        mov     ecx, [ecx + AHCI_CTR.mutex]
        call    mutex_unlock
 
        popad
        xor     eax, eax
        ret
endp

; Start command engine
; in: eax - address of HBA_PORT structure
ahci_start_cmd:
.wait_cr: ; Wait until CR (bit15) is cleared
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_CR
        jc      .wait_cr
 
        ; Set FRE (bit4) and ST (bit0)
        bts     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE
        bts     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_ST
        ; maybe here call ahci flush cmd ? TODO (see seakernel)
        ret

; Stop command engine
; in: eax - address of HBA_PORT structure
ahci_stop_cmd:
        btr     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_ST ; Clear ST (bit0)
        btr     [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FRE ; Clear FRE (bit4)
.wait_fr_cr: ; Wait until FR (bit14), CR (bit15) are cleared
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_FR
        jc      .wait_fr_cr
        bt      [eax + HBA_PORT.command], bit_AHCI_HBA_PxCMD_CR
        jc      .wait_fr_cr

        ret

; waits until the port is no longer busy before issuing a new command
; in: [port] - address of HBA_PORT structure
; [timeout] - timeout (in iterations)
; out: eax = 0 if success, 1 if timeout expired
proc ahci_port_wait stdcall, port: dword, timeout: dword
        push    ebx ecx
        mov     ebx, [port]
        xor     ecx, ecx
.wait:
        cmp     ecx, [timeout]
        jae     .wait_end
        mov     eax, [ebx + HBA_PORT.task_file_data]
        and     eax, ATA_DEV_BUSY or ATA_DEV_DRQ
        test    eax, eax
        jz      .wait_end
        inc     ecx
        jmp     .wait
.wait_end:
        xor     eax, eax
        DEBUGF  AHCI_DBGLVL, "port wait counter = %u\n", ecx
        cmp     ecx, [timeout] ; if they equal it means port is hung
        setz    al
        pop     ecx ebx
        ret
endp


; Wait for command completion
; in: [port] - address of HBA_PORT structure
;     [cmdslot] - number of command slot
; out: eax = 0 if success, 1 if error
proc ahci_port_cmd_wait stdcall, port: dword, cmdslot: dword ;, timeout: dword
        push    ebx ecx edx
        mov     ebx, [port]
        mov     edx, [cmdslot]
        xor     eax, eax
        xor     ecx, ecx
.wait:
        bt      [ebx + HBA_PORT.command_issue], edx
        jnc     .wait_end
        bt      [ebx + HBA_PORT.interrupt_status], bit_AHCI_HBA_PxIS_TFES ; check for Task File Error
        jc      .error
        inc     ecx
        jmp     .wait
.wait_end:
        DEBUGF  AHCI_DBGLVL, "port cmd wait counter = %u\n", ecx
        bt      [ebx + HBA_PORT.interrupt_status], bit_AHCI_HBA_PxIS_TFES ; check for Task File Error
        jc      .error
        jmp     .ret
.error:
        mov     eax, 1
.ret:
        pop     edx ecx ebx
        ret
endp

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
        endl

        pushad

        DEBUGF  1, "Rebasing port %u\n", [portno]

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
        
        mov     eax, [port]
        call    ahci_start_cmd

        DEBUGF  1, "End rebasing port %u\n", [portno]
        popad
        ret    
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


proc _memset stdcall, dest:dword, val:byte, cnt:dword ; doesnt clobber any registers
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
; doesnt clobber any registers
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
