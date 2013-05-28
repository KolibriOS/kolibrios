;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  i8254x driver for KolibriOS                                    ;;
;;                                                                 ;;
;;  based on i8254x.asm from baremetal os                          ;;
;;                                                                 ;;
;;    Written by hidnplayr (hidnplayr@gmail.com)                   ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ; TODO: make better use of the available descriptors

format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        MAX_DEVICES             = 16

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2

        MAX_PKT_SIZE            = 16384         ; Maximum packet size


include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../struct.inc'
include '../netdrv.inc'

public START
public service_proc
public version



; Register list
REG_CTRL                = 0x0000 ; Control Register
REG_STATUS              = 0x0008 ; Device Status Register
REG_CTRLEXT             = 0x0018 ; Extended Control Register
REG_MDIC                = 0x0020 ; MDI Control Register
REG_FCAL                = 0x0028 ; Flow Control Address Low
REG_FCAH                = 0x002C ; Flow Control Address High
REG_FCT                 = 0x0030 ; Flow Control Type
REG_VET                 = 0x0038 ; VLAN Ether Type
REG_ICR                 = 0x00C0 ; Interrupt Cause Read
REG_ITR                 = 0x00C4 ; Interrupt Throttling Register
REG_ICS                 = 0x00C8 ; Interrupt Cause Set Register
REG_IMS                 = 0x00D0 ; Interrupt Mask Set/Read Register
REG_IMC                 = 0x00D8 ; Interrupt Mask Clear Register
REG_RCTL                = 0x0100 ; Receive Control Register
REG_FCTTV               = 0x0170 ; Flow Control Transmit Timer Value
REG_TXCW                = 0x0178 ; Transmit Configuration Word
REG_RXCW                = 0x0180 ; Receive Configuration Word
REG_TCTL                = 0x0400 ; Transmit Control Register
REG_TIPG                = 0x0410 ; Transmit Inter Packet Gap

REG_LEDCTL              = 0x0E00 ; LED Control
REG_PBA                 = 0x1000 ; Packet Buffer Allocation

REG_RDBAL               = 0x2800 ; RX Descriptor Base Address Low
REG_RDBAH               = 0x2804 ; RX Descriptor Base Address High
REG_RDLEN               = 0x2808 ; RX Descriptor Length
REG_RDH                 = 0x2810 ; RX Descriptor Head
REG_RDT                 = 0x2818 ; RX Descriptor Tail
REG_RDTR                = 0x2820 ; RX Delay Timer Register
REG_RXDCTL              = 0x3828 ; RX Descriptor Control
REG_RADV                = 0x282C ; RX Int. Absolute Delay Timer
REG_RSRPD               = 0x2C00 ; RX Small Packet Detect Interrupt

REG_TXDMAC              = 0x3000 ; TX DMA Control
REG_TDBAL               = 0x3800 ; TX Descriptor Base Address Low
REG_TDBAH               = 0x3804 ; TX Descriptor Base Address High
REG_TDLEN               = 0x3808 ; TX Descriptor Length
REG_TDH                 = 0x3810 ; TX Descriptor Head
REG_TDT                 = 0x3818 ; TX Descriptor Tail
REG_TIDV                = 0x3820 ; TX Interrupt Delay Value
REG_TXDCTL              = 0x3828 ; TX Descriptor Control
REG_TADV                = 0x382C ; TX Absolute Interrupt Delay Value
REG_TSPMT               = 0x3830 ; TCP Segmentation Pad & Min Threshold

REG_RXCSUM              = 0x5000 ; RX Checksum Control

; Register list for i8254x
I82542_REG_RDTR         = 0x0108 ; RX Delay Timer Register
I82542_REG_RDBAL        = 0x0110 ; RX Descriptor Base Address Low
I82542_REG_RDBAH        = 0x0114 ; RX Descriptor Base Address High
I82542_REG_RDLEN        = 0x0118 ; RX Descriptor Length
I82542_REG_RDH          = 0x0120 ; RDH for i82542
I82542_REG_RDT          = 0x0128 ; RDT for i82542
I82542_REG_TDBAL        = 0x0420 ; TX Descriptor Base Address Low
I82542_REG_TDBAH        = 0x0424 ; TX Descriptor Base Address Low
I82542_REG_TDLEN        = 0x0428 ; TX Descriptor Length
I82542_REG_TDH          = 0x0430 ; TDH for i82542
I82542_REG_TDT          = 0x0438 ; TDT for i82542

; CTRL - Control Register (0x0000)
CTRL_FD                 = 0x00000001 ; Full Duplex
CTRL_LRST               = 0x00000008 ; Link Reset
CTRL_ASDE               = 0x00000020 ; Auto-speed detection
CTRL_SLU                = 0x00000040 ; Set Link Up
CTRL_ILOS               = 0x00000080 ; Invert Loss of Signal
CTRL_SPEED_MASK         = 0x00000300 ; Speed selection
CTRL_SPEED_SHIFT        = 8
CTRL_FRCSPD             = 0x00000800 ; Force Speed
CTRL_FRCDPLX            = 0x00001000 ; Force Duplex
CTRL_SDP0_DATA          = 0x00040000 ; SDP0 data
CTRL_SDP1_DATA          = 0x00080000 ; SDP1 data
CTRL_SDP0_IODIR         = 0x00400000 ; SDP0 direction
CTRL_SDP1_IODIR         = 0x00800000 ; SDP1 direction
CTRL_RST                = 0x04000000 ; Device Reset
CTRL_RFCE               = 0x08000000 ; RX Flow Ctrl Enable
CTRL_TFCE               = 0x10000000 ; TX Flow Ctrl Enable
CTRL_VME                = 0x40000000 ; VLAN Mode Enable
CTRL_PHY_RST            = 0x80000000 ; PHY reset

; STATUS - Device Status Register (0x0008)
STATUS_FD               = 0x00000001 ; Full Duplex
STATUS_LU               = 0x00000002 ; Link Up
STATUS_TXOFF            = 0x00000010 ; Transmit paused
STATUS_TBIMODE          = 0x00000020 ; TBI Mode
STATUS_SPEED_MASK       = 0x000000C0 ; Link Speed setting
STATUS_SPEED_SHIFT      = 6
STATUS_ASDV_MASK        = 0x00000300 ; Auto Speed Detection
STATUS_ASDV_SHIFT       = 8
STATUS_PCI66            = 0x00000800 ; PCI bus speed
STATUS_BUS64            = 0x00001000 ; PCI bus width
STATUS_PCIX_MODE        = 0x00002000 ; PCI-X mode
STATUS_PCIXSPD_MASK     = 0x0000C000 ; PCI-X speed
STATUS_PCIXSPD_SHIFT    = 14

; CTRL_EXT - Extended Device Control Register (0x0018)
CTRLEXT_PHY_INT         = 0x00000020 ; PHY interrupt
CTRLEXT_SDP6_DATA       = 0x00000040 ; SDP6 data
CTRLEXT_SDP7_DATA       = 0x00000080 ; SDP7 data
CTRLEXT_SDP6_IODIR      = 0x00000400 ; SDP6 direction
CTRLEXT_SDP7_IODIR      = 0x00000800 ; SDP7 direction
CTRLEXT_ASDCHK          = 0x00001000 ; Auto-Speed Detect Chk
CTRLEXT_EE_RST          = 0x00002000 ; EEPROM reset
CTRLEXT_SPD_BYPS        = 0x00008000 ; Speed Select Bypass
CTRLEXT_RO_DIS          = 0x00020000 ; Relaxed Ordering Dis.
CTRLEXT_LNKMOD_MASK     = 0x00C00000 ; Link Mode
CTRLEXT_LNKMOD_SHIFT    = 22

; MDIC - MDI Control Register (0x0020)
MDIC_DATA_MASK          = 0x0000FFFF ; Data
MDIC_REG_MASK           = 0x001F0000 ; PHY Register
MDIC_REG_SHIFT          = 16
MDIC_PHY_MASK           = 0x03E00000 ; PHY Address
MDIC_PHY_SHIFT          = 21
MDIC_OP_MASK            = 0x0C000000 ; Opcode
MDIC_OP_SHIFT           = 26
MDIC_R                  = 0x10000000 ; Ready
MDIC_I                  = 0x20000000 ; Interrupt Enable
MDIC_E                  = 0x40000000 ; Error

; ICR - Interrupt Cause Read (0x00c0)
ICR_TXDW                = 0x00000001 ; TX Desc Written back
ICR_TXQE                = 0x00000002 ; TX Queue Empty
ICR_LSC                 = 0x00000004 ; Link Status Change
ICR_RXSEQ               = 0x00000008 ; RX Sence Error
ICR_RXDMT0              = 0x00000010 ; RX Desc min threshold reached
ICR_RXO                 = 0x00000040 ; RX Overrun
ICR_RXT0                = 0x00000080 ; RX Timer Interrupt
ICR_MDAC                = 0x00000200 ; MDIO Access Complete
ICR_RXCFG               = 0x00000400
ICR_PHY_INT             = 0x00001000 ; PHY Interrupt
ICR_GPI_SDP6            = 0x00002000 ; GPI on SDP6
ICR_GPI_SDP7            = 0x00004000 ; GPI on SDP7
ICR_TXD_LOW             = 0x00008000 ; TX Desc low threshold hit
ICR_SRPD                = 0x00010000 ; Small RX packet detected

; RCTL - Receive Control Register (0x0100)
RCTL_EN                 = 0x00000002 ; Receiver Enable
RCTL_SBP                = 0x00000004 ; Store Bad Packets
RCTL_UPE                = 0x00000008 ; Unicast Promiscuous Enabled
RCTL_MPE                = 0x00000010 ; Xcast Promiscuous Enabled
RCTL_LPE                = 0x00000020 ; Long Packet Reception Enable
RCTL_LBM_MASK           = 0x000000C0 ; Loopback Mode
RCTL_LBM_SHIFT          = 6
RCTL_RDMTS_MASK         = 0x00000300 ; RX Desc Min Threshold Size
RCTL_RDMTS_SHIFT        = 8
RCTL_MO_MASK            = 0x00003000 ; Multicast Offset
RCTL_MO_SHIFT           = 12
RCTL_BAM                = 0x00008000 ; Broadcast Accept Mode
RCTL_BSIZE_MASK         = 0x00030000 ; RX Buffer Size
RCTL_BSIZE_SHIFT        = 16
RCTL_VFE                = 0x00040000 ; VLAN Filter Enable
RCTL_CFIEN              = 0x00080000 ; CFI Enable
RCTL_CFI                = 0x00100000 ; Canonical Form Indicator Bit
RCTL_DPF                = 0x00400000 ; Discard Pause Frames
RCTL_PMCF               = 0x00800000 ; Pass MAC Control Frames
RCTL_BSEX               = 0x02000000 ; Buffer Size Extension
RCTL_SECRC              = 0x04000000 ; Strip Ethernet CRC

; TCTL - Transmit Control Register (0x0400)
TCTL_EN                 = 0x00000002 ; Transmit Enable
TCTL_PSP                = 0x00000008 ; Pad short packets
TCTL_SWXOFF             = 0x00400000 ; Software XOFF Transmission

; PBA - Packet Buffer Allocation (0x1000)
PBA_RXA_MASK            = 0x0000FFFF ; RX Packet Buffer
PBA_RXA_SHIFT           = 0
PBA_TXA_MASK            = 0xFFFF0000 ; TX Packet Buffer
PBA_TXA_SHIFT           = 16

; Flow Control Type
FCT_TYPE_DEFAULT        = 0x8808

; === TX Descriptor fields ===

; TX Packet Length (word 2)
TXDESC_LEN_MASK         = 0x0000ffff

; TX Descriptor CMD field (word 2)
TXDESC_IDE              = 0x80000000 ; Interrupt Delay Enable
TXDESC_VLE              = 0x40000000 ; VLAN Packet Enable
TXDESC_DEXT             = 0x20000000 ; Extension
TXDESC_RPS              = 0x10000000 ; Report Packet Sent
TXDESC_RS               = 0x08000000 ; Report Status
TXDESC_IC               = 0x04000000 ; Insert Checksum
TXDESC_IFCS             = 0x02000000 ; Insert FCS
TXDESC_EOP              = 0x01000000 ; End Of Packet

; TX Descriptor STA field (word 3)
TXDESC_TU               = 0x00000008 ; Transmit Underrun
TXDESC_LC               = 0x00000004 ; Late Collision
TXDESC_EC               = 0x00000002 ; Excess Collisions
TXDESC_DD               = 0x00000001 ; Descriptor Done

; === RX Descriptor fields ===

; RX Packet Length (word 2)
RXDESC_LEN_MASK         = 0x0000ffff

; RX Descriptor STA field (word 3)
RXDESC_PIF              = 0x00000080 ; Passed In-exact Filter
RXDESC_IPCS             = 0x00000040 ; IP cksum calculated
RXDESC_TCPCS            = 0x00000020 ; TCP cksum calculated
RXDESC_VP               = 0x00000008 ; Packet is 802.1Q
RXDESC_IXSM             = 0x00000004 ; Ignore cksum indication
RXDESC_EOP              = 0x00000002 ; End Of Packet
RXDESC_DD               = 0x00000001 ; Descriptor Done


virtual at ebx
        device:
        ETH_DEVICE

        .mmio_addr      dd ?
        .pci_bus        dd ?
        .pci_dev        dd ?
        .irq_line       db ?

        .cur_tx         dd ?
        .last_tx        dd ?

                        rb 0x100 - (($ - device) and 0xff)
        .rx_desc        rd 256/8

                        rb 0x100 - (($ - device) and 0xff)
        .tx_desc        rd 256/8

        sizeof.device_struct = $ - device

end virtual

section '.flat' code readable align 16


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc START stdcall, state:dword

        cmp [state], 1
        jne .exit

  .entry:

        DEBUGF  2,"Loading %s driver\n", my_service
        stdcall RegService, my_service, service_proc
        ret

  .fail:
  .exit:
        xor eax, eax
        ret

endp




;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [IOCTL.out_size], 4
        jb      .fail
        mov     eax, [IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [IOCTL.inp_size], 3                     ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax, [eax+1]                             ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte [device.pci_bus]
        jne     .next
        cmp     ah, byte [device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
  .next:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device_struct, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.unload], unload
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        movzx   ecx, byte [eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte [eax+2]
        mov     [device.pci_dev], ecx

; Now, it's time to find the base mmio addres of the PCI device

        PCI_find_mmio32

; Create virtual mapping of the physical memory

        push    1Bh             ; PG_SW+PG_NOCACHE
        push    10000h          ; size of the map
        push    eax
        call    MapIoMem
        mov     [device.mmio_addr], eax

; We've found the mmio address, find IRQ now

        PCI_find_irq

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.mmio_addr]:8

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    start_i8254x

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        stdcall KernelFree, ebx

  .fail:
        or      eax, -1
        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


align 4
unload:
        ; TODO: (in this particular order)
        ;
        ; - Stop the device
        ; - Detach int handler
        ; - Remove device from local list (device_list)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax, -1

ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is I8254X)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
probe:

        DEBUGF  1,"Probe\n"

        PCI_make_bus_master

        ; TODO: validate the device

        call    read_mac

        movzx   eax, [device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:


reset_dontstart:
        DEBUGF  1,"Reset\n"

        mov     esi, [device.mmio_addr]

        or      dword [esi + REG_CTRL], CTRL_RST        ; reset device
  .loop:
        push    esi
        xor     esi, esi
        inc     esi
        call    Sleep
        pop     esi
        test    dword [esi + REG_CTRL], CTRL_RST
        jnz     .loop

        mov     dword [esi + REG_IMC], 0xffffffff       ; Disable all interrupt causes
        mov     eax, dword [esi + REG_ICR]              ; Clear any pending interrupts
        mov     dword [esi + REG_ITR], 0                ; Disable interrupt throttling logic

        mov     dword [esi + REG_PBA], 0x00000030       ; PBA: set the RX buffer size to 48KB (TX buffer is calculated as 64-RX buffer)
        mov     dword [esi + REG_RDTR], 0               ; RDTR: set no delay

        mov     dword [esi + REG_TXCW], 0x08008060      ; TXCW: set ANE, TxConfigWord (Half/Full duplex, Next Page Reqest)

        mov     eax, [esi + REG_CTRL]
        or      eax, 1 shl 6 + 1 shl 5
        and     eax, not (1 shl 3 + 1 shl 7 + 1 shl 30 + 1 shl 31)
        mov     dword [esi + REG_CTRL], eax             ; CTRL: clear LRST, set SLU and ASDE, clear RSTPHY, VME, and ILOS

        lea     edi, [esi + 0x5200]                     ; MTA: reset
        mov     eax, 0xffffffff
        stosd
        stosd
        stosd
        stosd

        stdcall KernelAlloc, 48*1024
        mov     dword [device.rx_desc + 16], eax
        GetRealAddr
        mov     dword [device.rx_desc], eax
        mov     dword [device.rx_desc + 4], 0

        lea     eax, [device.rx_desc]
        GetRealAddr
        mov     dword [esi + REG_RDBAL], eax            ; Receive Descriptor Base Address Low
        mov     dword [esi + REG_RDBAH], 0              ; Receive Descriptor Base Address High
        mov     dword [esi + REG_RDLEN], (1 * 128)      ; Receive Descriptor Length
        mov     dword [esi + REG_RDH], 0                ; Receive Descriptor Head
        mov     dword [esi + REG_RDT], 1                ; Receive Descriptor Tail
        mov     dword [esi + REG_RCTL], RCTL_EN or RCTL_SBP or RCTL_BAM or RCTL_SECRC or RCTL_UPE or RCTL_MPE
        ; Receiver Enable, Store Bad Packets, Broadcast Accept Mode, Strip Ethernet CRC from incoming packet, Promiscuous mode

        mov     dword [device.tx_desc], 0
        mov     dword [device.tx_desc + 4], 0
        mov     dword [device.tx_desc + 16], 0

        lea     eax, [device.tx_desc]
        GetRealAddr
        mov     dword [esi + REG_TDBAL], eax            ; Transmit Descriptor Base Address Low
        mov     dword [esi + REG_TDBAH], 0              ; Transmit Descriptor Base Address High
        mov     dword [esi + REG_TDLEN], (1 * 128)      ; Transmit Descriptor Length
        mov     dword [esi + REG_TDH], 0                ; Transmit Descriptor Head
        mov     dword [esi + REG_TDT], 0                ; Transmit Descriptor Tail
        mov     dword [esi + REG_TCTL], 0x010400fa      ; Enabled, Pad Short Packets, 15 retrys, 64-byte COLD, Re-transmit on Late Collision
        mov     dword [esi + REG_TIPG], 0x0060200A      ; IPGT 10, IPGR1 8, IPGR2 6

        xor     eax, eax
        ret

align 4
reset:
        call    reset_dontstart

start_i8254x:

        xor     eax, eax
        mov     [esi + REG_RDTR], eax                   ; Clear the Receive Delay Timer Register
        mov     [esi + REG_RADV], eax                   ; Clear the Receive Interrupt Absolute Delay Timer
        mov     [esi + REG_RSRPD], eax                  ; Clear the Receive Small Packet Detect Interrupt
;        or      eax, 1 shl 0 + 1 shl 7                  ; TXDW + RXT0
        mov     eax, 1+4+16 ;;;; hack!
        mov     [esi + REG_IMS], eax                    ; Enable interrupt types

        mov     [device.mtu], 1514

; Set link state to unknown
        mov     [device.state], ETH_LINK_UNKOWN

        xor     eax, eax
        ret




align 4
read_mac:

        DEBUGF  1,"Read MAC\n"

        mov     esi, [device.mmio_addr]

        mov     eax, [esi+0x5400]                       ; RAL
        test    eax, eax
        jz      .try_eeprom

        mov     dword [device.mac], eax
        mov     eax, [esi+0x5404]                       ; RAH
        mov     word [device.mac+4], ax

        jmp     .mac_ok

  .try_eeprom:
        mov     dword [esi+0x14], 0x00000001
        mov     eax, [esi+0x14]
        shr     eax, 16
        mov     word [device.mac], ax

        mov     dword [esi+0x14], 0x00000101
        mov     eax, [esi+0x14]
        shr     eax, 16
        mov     word [device.mac+2], ax

        mov     dword [esi+0x14], 0x00000201
        mov     eax, [esi+0x14]
        shr     eax, 16
        mov     word [device.mac+4], ax

  .mac_ok:
        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [device.mac+0]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
transmit:
        DEBUGF  2,"\nTransmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
        mov     eax, [esp+4]
        DEBUGF  2,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp + 8], 1514
        ja      .fail
        cmp     dword [esp + 8], 60
        jb      .fail


; Program the descriptor (use legacy mode)
        lea     edi, [device.tx_desc]                   ; Transmit Descriptor Base Address
        mov     dword [edi + 16], eax                   ; Store the data location (for driver)
        GetRealAddr                                     ;
        mov     dword [edi], eax                        ; Real addr (for i8254x)
        mov     dword [edi + 4], 0x00000000             ;

        mov     ecx, [esp + 8]
        or      ecx, 1 shl 24 + 1 shl 25 + 1 shl 27     ; EOP + IFCS + RS
        mov     dword [edi + 8], ecx                    ; Packet size
        mov     dword [edi + 12], 0x00000000

; Tell i8254x wich descriptor(s) we programmed
        mov     edi, [device.mmio_addr]
        mov     dword [edi + REG_TDH], 0                ; TDH - Transmit Descriptor Head
        mov     dword [edi + REG_TDT], 1                ; TDT - Transmit Descriptor Tail

; Update stats
        inc     [device.packets_tx]
        mov     eax, [esp + 8]
        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx + 4], 0

        ret     8

  .fail:
        DEBUGF  1,"Send failed\n"
        ret     8


;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"\n%s int\n", my_service
;-------------------------------------------
; Find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        mov     edi, [device.mmio_addr]
        mov     eax, [edi + REG_ICR]
        test    eax, eax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret

  .got_it:

        DEBUGF  1,"Device: %x Status: %x ", ebx, eax

;---------
; RX done?

        test    eax, ICR_RXDMT0
        jz      .no_rx

        push    eax ebx
        push    .retaddr

; Get last descriptor addr
        lea     esi, [device.rx_desc]

        cmp     byte [esi + 12], 0                      ; Check status field
        je      .retaddr

        movzx   ecx, word [esi + 8]                     ; Get the packet length
        DEBUGF  2,"got %u bytes\n", ecx
        push    ecx
        push    dword [esi + 16]                        ; Get packet pointer

; Update stats
        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0
        inc     dword [device.packets_rx]

; allocate new descriptor
        stdcall KernelAlloc, 48*1024
        mov     dword [esi + 16], eax
        GetRealAddr
        mov     dword [esi], eax

; reset descriptor status
        mov     esi, [device.mmio_addr]
        mov     dword [esi + REG_RDH], 0x00000000       ; Receive Descriptor Head
        mov     dword [esi + REG_RDT], 0x00000001       ; Receive Descriptor Tail

        jmp     Eth_input
  .retaddr:
        pop     ebx eax

  .no_rx:

;--------------
; Link Changed?

        test    eax, ICR_LSC
        jz      .no_link

        DEBUGF  2,"Link Changed\n"

  .no_link:

;---------------
; Transmit done?

        test    eax, ICR_TXDW
        jz      .no_tx

        DEBUGF  2,"Transmit done\n"

        lea     edi, [device.tx_desc]                   ; Transmit Descriptor Base Address
        push    dword [edi + 16]                        ; Store the data location (for driver)
        call    KernelFree

  .no_tx:
  .fail:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret




; End of code

section '.data' data readable writable align 16
align 4

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'I8254X',0                   ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling


