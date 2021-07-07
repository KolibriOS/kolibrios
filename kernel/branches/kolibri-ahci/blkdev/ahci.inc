;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

PCI_REG_STATUS_COMMAND = 0x0004
PCI_REG_BAR5 = 0x0024

AHCI_BOHCf_BOS  = 0        ; BIOS-Owned Semaphore (BIOS owns controller)
AHCI_BOHCf_OOS  = 1        ; OS-Owned Semaphore (OS owns controller)
AHCI_BOHCf_BB   = 4        ; BIOS Busy (polling bit while BIOS cleans up 

AHCI_CAP2_BOH = 0          ; number of bit in BOH which shows Supports BIOS/OS Handoff

HBA_MEMORY_SIZE = 0x1100

struct AHCI_DATA
        ;;
        abar    dd ?       ; pointer to HBA Memory (BAR5) mapped to virtual kernelspace memory
        pcidev  dd ?       ; pointer to corresponding PCIDEV structure
ends

; Generic Host Control registers
struct HBA_MEM
        capability            dd ?                    ; 0x00
        global_host_control   dd ?                    ; 0x04
        interrupt_status      dd ?                    ; 0x08
        port_implemented      dd ?                    ; 0x0C
        version               dd ?                    ; 0x10
        ccc_ctl               dd ?                    ; 0x14, Command completion coalescing control
        ccc_pts               dd ?                    ; 0x18, Command completion coalescing ports
        em_loc                dd ?                    ; 0x1C, Enclosure management location
        em_ctl                dd ?                    ; 0x20, Enclosure management control
        capability2           dd ?                    ; 0x24, Host capabilities extended
        bohc                  dd ?                    ; 0x28, BIOS/OS handoff control and status
        reserved              rb (0xA0-0x2C)          ; 0x2C - 0x9F, Reserved
        vendor                rb (0x100-0xA0)         ; 0xA0 - 0xFF, Vendor specific 
        ports                 rb (sizeof.HBA_PORT*32) ; 0x100 - 0x10FF, Port control registers, max 32
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

uglobal
align 4
        ahci_controller AHCI_DATA
endg


; detect ahci controller and initialize
align 4
init_ahci:
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
        
        mov     ah, [esi + PCIDEV.bus]
        mov     al, 2 ; read dword
        mov     bh, [esi + PCIDEV.devfn]
        mov     bl, PCI_REG_BAR5
        call    pci_read_reg
        DEBUGF  1, "K: AHCI controller BAR5 = %x\n", eax

        stdcall map_io_mem, eax, HBA_MEMORY_SIZE, PG_SWR + PG_NOCACHE
        mov     [ahci_controller + AHCI_DATA.abar], eax
        DEBUGF  1, "K: AHCI controller BAR5 mapped to virtual addr %x\n", eax

        mov     ah, [esi + PCIDEV.bus]
        mov     al, 2 ; read dword
        mov     bh, [esi + PCIDEV.devfn]
        mov     bl, PCI_REG_STATUS_COMMAND
        call    pci_read_reg
        DEBUGF  1, "K: AHCI: pci_status_command = %x\nEnabling interrupts, DMA bus mastering and memory space access\n", eax
        or      eax, 0x06 ; pci.command |= 0x06 (dma bus mastering + memory space access)
        btr     eax, 10 ; clear the "disable interrupts" bit
        DEBUGF  1, "K: AHCI: pci_status_command = %x\n", eax
        mov     ecx, eax
        mov     ah, [esi + PCIDEV.bus]
        mov     al, 2 ; write dword
        mov     bh, [esi + PCIDEV.devfn]
        mov     bl, PCI_REG_STATUS_COMMAND
        call    pci_write_reg

        mov     eax, [ahci_controller + AHCI_DATA.abar]
        mov     ebx, [eax + HBA_MEM.capability]
        mov     ecx, [eax + HBA_MEM.global_host_control]
        mov     edx, [eax + HBA_MEM.version]
        DEBUGF  1, "K: AHCI: HBA.cap = %x, HBA.ghc = %x, HBA_MEM.version = %x\n", ebx, ecx, edx

        ; //Transferring ownership from BIOS if supported.
	; if (Bt(&hba->caps_ext, AHCI_CAPSEXTf_BOH))
	; {
	; 	Bts(&hba->bohc, AHCI_BOHCf_OOS);
	; 	"AHCI: Transferring ownership from BIOS\n";

	; 	while (Bt(&hba->bohc, AHCI_BOHCf_BOS));

	; 	Sleep(25);
	; 	if (Bt(&hba->bohc, AHCI_BOHCf_BB)) //if Bios Busy is still set after 25 mS, wait 2 seconds.
	; 		Sleep(2000);
	; }

        ; Transferring ownership from BIOS if supported. (TODO check)
        mov     eax, [ahci_controller + AHCI_DATA.abar]
        mov     ebx, [eax + HBA_MEM.capability2]
        DEBUGF  1, "K: AHCI: HBA_MEM.cap2 = %x\n", ebx
        bt      dword [eax + HBA_MEM.capability2], AHCI_CAP2_BOH
        jnc     .end_handoff
        DEBUGF  1, "K: AHCI: Transferring ownership from BIOS...\n"
        bts     dword [eax + HBA_MEM.bohc], AHCI_BOHCf_OOS

.wait_not_bos:
        bt      dword [eax + HBA_MEM.bohc], AHCI_BOHCf_BOS
        jc      .wait_not_bos

        mov     ebx, 3
        call    delay_hs

        ; if Bios Busy is still set after 25 mS, wait 2 seconds.
        bt      dword [eax + HBA_MEM.bohc], AHCI_BOHCf_BB
        jnc     @f

        mov     ebx, 200
        call    delay_hs
@@:
        DEBUGF  1, "K: AHCI: Done.\n"

.end_handoff:

        ;; TODO: Reset controller ?

        ;; TODO enble AHCI ? (see https://github.com/ZenithOS/ZenithOS/blob/4ea8b133613ab95a8b53ed543d8e63525a21954e/src/Kernel/BlkDev/DiskAHCI.CC#L719)

        ;; TODO: find drives (see https://github.com/ZenithOS/ZenithOS/blob/4ea8b133613ab95a8b53ed543d8e63525a21954e/src/Kernel/BlkDev/DiskAHCI.CC#L742)

        ret






