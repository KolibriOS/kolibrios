;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

PCI_REG_STATUS_COMMAND = 0x0004
PCI_REG_BAR5 = 0x0024

; different SATA device signatures
SATA_SIG_ATA	= 0x00000101	; SATA drive
SATA_SIG_ATAPI	= 0xEB140101	; SATAPI drive
SATA_SIG_SEMB	= 0xC33C0101	; Enclosure management bridge
SATA_SIG_PM	= 0x96690101	; Port multiplier

; Device type constants
AHCI_DEV_NULL   = 0
AHCI_DEV_SATA   = 1
AHCI_DEV_SEMB   = 2
AHCI_DEV_PM     = 3
AHCI_DEV_SATAPI = 4

; ATA commands
ATA_IDENTIFY    = 0xEC

; ATAPI commands
ATAPI_IDENTIFY  = 0xA1

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

; Frame Information Structure Types
FIS_TYPE_REG_H2D    = 0x27 ; Register FIS - host to device
FIS_TYPE_REG_D2H    = 0x34 ; Register FIS - device to host
FIS_TYPE_DMA_ACT    = 0x39 ; DMA activate FIS - device to host
FIS_TYPE_DMA_SETUP  = 0x41 ; DMA setup FIS - bidirectional
FIS_TYPE_DATA       = 0x46 ; Data FIS - bidirectional
FIS_TYPE_BIST       = 0x58 ; BIST activate FIS - bidirectional
FIS_TYPE_PIO_SETUP  = 0x5F ; PIO setup FIS - device to host
FIS_TYPE_DEV_BITS   = 0xA1 ; Set device bits FIS - device to host

struct AHCI_DATA
        abar    dd ?       ; pointer to HBA Memory (BAR5) mapped to virtual kernelspace memory
        pcidev  dd ?       ; pointer to corresponding PCIDEV structure
ends

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
        ahci_controller AHCI_DATA
        port_data_arr   rb (sizeof.PORT_DATA*AHCI_MAX_PORTS)
endg

; -----------------------------------------------------------------------
; detect ahci controller and initialize
align 4
ahci_init:
        mov     ecx, ahci_controller
        mov     esi, pcidev_list
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
        mov     [ahci_controller + AHCI_DATA.pcidev], esi

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
        mov     [ahci_controller + AHCI_DATA.abar], eax
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

        ; ; Print some register values to debug board
        ; mov     esi, [ahci_controller + AHCI_DATA.abar]
        ; DEBUGF  1, "K: AHCI: HBA.cap = %x, HBA.ghc = %x, HBA_MEM.version = %x\n", [esi + HBA_MEM.cap], [esi + HBA_MEM.ghc], [esi + HBA_MEM.version]

        ;-------------------------------------------------------
        ; Request BIOS/OS ownership handoff, if supported. (TODO check correctness)
        mov     esi, [ahci_controller + AHCI_DATA.abar]
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
        mov     ebx, 1 ; wait 10 ms
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
        add     ecx, port_data_arr
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
        mov     [ecx + PORT_DATA.drive_type], eax

        DEBUGF  1, "K: AHCI: found drive on port %u: TYPE = %u\n", ebx, [ecx + PORT_DATA.drive_type]

        stdcall ahci_port_identify, ecx

.continue_detect_drives:
        inc     ebx
        jmp     .detect_drives

        

.end_detect_drives:
        

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
        mov     edi, [esi + PORT_DATA.port] ; edi - address of HBA_PORT struct of port

        mov     eax, edi
        call    ahci_find_cmdslot

        cmp     eax, -1
        jne      .cmdslot_found

        DEBUGF  1, "No free cmdslot on port %u\n", [esi + PORT_DATA.portno]

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

        ; TODO Wait on previous command to complete. AHCIPortWait(bd->port_num, tS + 2);
        mov     ebx, 20 ;;;
        call    delay_hs ;;;

        mov     eax, [cmdslot]
        bts     [edi + HBA_PORT.command_issue], eax ; Issue the command

        ; TODO AHCIPortCmdWait(bd->port_num, cmd_slot);
        mov     ebx, 20 ;;;
        call    delay_hs ;;;

        ; DEBUGF  1, "sata_error register = 0x%x\n", [edi + HBA_PORT.sata_error]

;         mov     ecx, ecx
;         mov     esi, [buf_virt]
; .print_ident:
;         cmp     ecx, 512 - 1 ; why -1 ?
;         jae     .end_print_ident

;         mov     al, byte [esi + ecx]
;         mov     byte [modelstr], al
;         mov     byte [modelstr + 1], 0
;         DEBUGF  1, "(%s) ", modelstr

;         inc     ecx
;         jmp     .print_ident
; .end_print_ident:
        ; DEBUGF  1, "\n"

        ; DEBUGF  1, "after identification: signature = 0x%x\n", [edi + HBA_PORT.signature]

        mov     esi, [buf_virt]
        add     esi, 27*2
        mov     edi, modelstr
        mov     ecx, ((46-27)+1)*2
        cld
        rep movsb
        mov     byte [edi], 0
        
        xor     ecx, ecx
.reverse1:
        cmp     ecx, ((46-27)+1)*2
        jae     .reverse1_end
        mov     bl, byte [modelstr + ecx]
        mov     dl, byte [modelstr + ecx + 1]
        mov     byte [modelstr + ecx], dl
        mov     byte [modelstr + ecx + 1], bl
        add     ecx, 2
        jmp     .reverse1
.reverse1_end:
        DEBUGF  1, "IDENTIFICATION RESULT: MODEL = %s\n", modelstr

.ret:
        popad
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

; The commands may not take effect until the command
; register is read again by software, because reasons.
; in: eax - address of HBA_PORT structure
; out: eax - command register value
ahci_flush_cmd:
        mov     eax, [eax + HBA_PORT.command]
        ret

; Send command to port
; in: eax - address of HBA_PORT structure
;     ebx - index of command slot
ahci_send_cmd:
        push    ecx
        mov     [eax + HBA_PORT.interrupt_status], 0xFFFFFFFF
        
        mov     cl, bl
        mov     [eax + HBA_PORT.command_issue], 1
        shl     [eax + HBA_PORT.command_issue], cl

        call    ahci_flush_cmd
        pop     ecx
        ret

; ---------------------------------------------------------------------------
; TODO: check correctness
; in: port - address of HBA_PORT structure
;     portno - port index (0..31)
;     pdata - address of PORT_DATA structure
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
        
        stdcall alloc_pages, 2
        mov     [phys_page23], eax
        stdcall map_io_mem, eax, 2*4096, PG_NOCACHE + PG_SWR
        mov     [virt_page23], eax

        ; Command table size = 256*32 = 8K per port
        mov     edx, [edi + PORT_DATA.clb] ; cmdheader array base
        xor     ecx, ecx

.for1:
        cmp     ecx, 32
        jae     .for1_end

        mov     ebx, ecx
        shl     ebx, BSF sizeof.HBA_CMD_HDR
        add     ebx, edx ; ebx = cmdheader[ecx]

        mov     [ebx + HBA_CMD_HDR.prdtl], 8 ; 8 prdt entries per command table

        ; 256 bytes per command table, 64+16+48+16*8

        push    edx
        
        ; cmdheader[ecx].ctba = phys_page23 + ecx*256
        mov     [ebx + HBA_CMD_HDR.ctba], ecx
        shl     [ebx + HBA_CMD_HDR.ctba], BSF 256 ; *= 256
        mov     eax, [ebx + HBA_CMD_HDR.ctba]
        mov     edx, [phys_page23]
        add     [ebx + HBA_CMD_HDR.ctba], edx

        add     eax, [virt_page23]
        mov     [tmp], eax  ; tmp = virt_page23 + ecx*256
        lea     eax, [ecx*4 + edi + PORT_DATA.ctba_arr] ; eax = pdata->ctba_arr[ecx]
        mov     edx, [tmp]
        mov     [eax], edx  ; pdata->ctba_arr[ecx] = virt_page23 + ecx*256

        pop     edx

        mov     [ebx + HBA_CMD_HDR.ctbau], 0
        stdcall _memset, [eax], 0, 256 ; zero out

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
; in: eax - address of HBA_PORT structure
; out: eax - if not found -1, else slot index
ahci_find_cmdslot:
        push    ebx ecx edx esi
        ; If not set in SACT and CI, the slot is free
        mov     ebx, [eax + HBA_PORT.sata_active]
        or      ebx, [eax + HBA_PORT.command_issue] ; ebx = slots

        mov     esi, [ahci_controller + AHCI_DATA.abar]
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


proc _memset stdcall, dest:dword, val:byte, cnt:dword ; doesnt clobber any registers
        ;DEBUGF  DBG_INFO, "memset(%x, %u, %u)\n", [dest], [val], [cnt]
        push    eax ecx edi
        mov     edi, dword [dest]
        mov     al,  byte [val]
        mov     ecx, dword [cnt]
        rep stosb   
        pop     edi ecx eax
        ret 
endp
