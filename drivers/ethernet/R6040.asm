;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  R6040 driver for KolibriOS                                     ;;
;;                                                                 ;;
;;  based on R6040.c from linux                                    ;;
;;                                                                 ;;
;;    Written by Asper (asper.85@mail.ru)                          ;;
;;            and hidnplayr (hidnplayr@gmail.com)                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 16

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2

        W_MAX_TIMEOUT           = 0x0FFF        ; max time out delay time

        TX_TIMEOUT              = 6000          ; Time before concluding the transmitter is hung, in ms

        TX_RING_SIZE            = 4             ; RING sizes must be a power of 2
        RX_RING_SIZE            = 4

        RX_BUF_LEN_IDX          = 3             ; 0==8K, 1==16K, 2==32K, 3==64K

; Threshold is bytes transferred to chip before transmission starts.

        TX_FIFO_THRESH          = 256           ; In bytes, rounded down to 32 byte units.

; The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.

        RX_FIFO_THRESH          = 4             ; Rx buffer level before first PCI xfer.
        RX_DMA_BURST            = 4             ; Maximum PCI burst, '4' is 256 bytes
        TX_DMA_BURST            = 4

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

; Operational parameters that usually are not changed.

PHY1_ADDR       = 1                     ; For MAC1
PHY2_ADDR       = 3                     ; For MAC2
PHY_MODE        = 0x3100                ; PHY CHIP Register 0
PHY_CAP         = 0x01E1                ; PHY CHIP Register 4

;**************************************************************************
; RDC R6040 Register Definitions
;**************************************************************************

MCR0            = 0x00                  ; Control register 0
MCR0_RCVEN      = 0x0002                ; Receive enable
MCR0_PROMISC    = 0x0020                ; Promiscuous mode
MCR0_HASH_EN    = 0x0100                ; Enable multicast hash table function
MCR0_XMTEN      = 0x1000                ; Transmission enable
MCR0_FD         = 0x8000                ; Full/Half Duplex mode

MCR1            = 0x01                  ; Control register 1
MAC_RST         = 0x0001                ; Reset the MAC

MBCR            = 0x08                  ; Bus control
MT_ICR          = 0x0C                  ; TX interrupt control
MR_ICR          = 0x10                  ; RX interrupt control
MTPR            = 0x14                  ; TX poll command register
MR_BSR          = 0x18                  ; RX buffer size
MR_DCR          = 0x1A                  ; RX descriptor control
MLSR            = 0x1C                  ; Last status

MMDIO           = 0x20                  ; MDIO control register
MDIO_WRITE      = 0x4000                ; MDIO write
MDIO_READ       = 0x2000                ; MDIO read
MMRD            = 0x24                  ; MDIO read data register
MMWD            = 0x28                  ; MDIO write data register

MTD_SA0         = 0x2C                  ; TX descriptor start address 0
MTD_SA1         = 0x30                  ; TX descriptor start address 1
MRD_SA0         = 0x34                  ; RX descriptor start address 0
MRD_SA1         = 0x38                  ; RX descriptor start address 1

MISR            = 0x3C                  ; Status register
MIER            = 0x40                  ; INT enable register
MSK_INT         = 0x0000                ; Mask off interrupts
RX_FINISH       = 0x0001                ; RX finished
RX_NO_DESC      = 0x0002                ; No RX descriptor available
RX_FIFO_FULL    = 0x0004                ; RX FIFO full
RX_EARLY        = 0x0008                ; RX early
TX_FINISH       = 0x0010                ; TX finished
TX_EARLY        = 0x0080                ; TX early
EVENT_OVRFL     = 0x0100                ; Event counter overflow
LINK_CHANGED    = 0x0200                ; PHY link changed

ME_CISR         = 0x44                  ; Event counter INT status
ME_CIER         = 0x48                  ; Event counter INT enable
MR_CNT          = 0x50                  ; Successfully received packet counter
ME_CNT0         = 0x52                  ; Event counter 0
ME_CNT1         = 0x54                  ; Event counter 1
ME_CNT2         = 0x56                  ; Event counter 2
ME_CNT3         = 0x58                  ; Event counter 3
MT_CNT          = 0x5A                  ; Successfully transmit packet counter
ME_CNT4         = 0x5C                  ; Event counter 4
MP_CNT          = 0x5E                  ; Pause frame counter register
MAR0            = 0x60                  ; Hash table 0
MAR1            = 0x62                  ; Hash table 1
MAR2            = 0x64                  ; Hash table 2
MAR3            = 0x66                  ; Hash table 3
MID_0L          = 0x68                  ; Multicast address MID0 Low
MID_0M          = 0x6A                  ; Multicast address MID0 Medium
MID_0H          = 0x6C                  ; Multicast address MID0 High
MID_1L          = 0x70                  ; MID1 Low
MID_1M          = 0x72                  ; MID1 Medium
MID_1H          = 0x74                  ; MID1 High
MID_2L          = 0x78                  ; MID2 Low
MID_2M          = 0x7A                  ; MID2 Medium
MID_2H          = 0x7C                  ; MID2 High
MID_3L          = 0x80                  ; MID3 Low
MID_3M          = 0x82                  ; MID3 Medium
MID_3H          = 0x84                  ; MID3 High
PHY_CC          = 0x88                  ; PHY status change configuration register
PHY_ST          = 0x8A                  ; PHY status register
MAC_SM          = 0xAC                  ; MAC status machine
MAC_ID          = 0xBE                  ; Identifier register

MAX_BUF_SIZE    = 1514

MBCR_DEFAULT    = 0x012A                ; MAC Bus Control Register
MCAST_MAX       = 3                     ; Max number multicast addresses to filter

;Descriptor status
DSC_OWNER_MAC   = 0x8000                ; MAC is the owner of this descriptor
DSC_RX_OK       = 0x4000                ; RX was successfull
DSC_RX_ERR      = 0x0800                ; RX PHY error
DSC_RX_ERR_DRI  = 0x0400                ; RX dribble packet
DSC_RX_ERR_BUF  = 0x0200                ; RX length exceeds buffer size
DSC_RX_ERR_LONG = 0x0100                ; RX length > maximum packet length
DSC_RX_ERR_RUNT = 0x0080                ; RX packet length < 64 byte
DSC_RX_ERR_CRC  = 0x0040                ; RX CRC error
DSC_RX_BCAST    = 0x0020                ; RX broadcast (no error)
DSC_RX_MCAST    = 0x0010                ; RX multicast (no error)
DSC_RX_MCH_HIT  = 0x0008                ; RX multicast hit in hash table (no error)
DSC_RX_MIDH_HIT = 0x0004                ; RX MID table hit (no error)
DSC_RX_IDX_MID_MASK  = 3                ; RX mask for the index of matched MIDx

;PHY settings
ICPLUS_PHY_ID   = 0x0243

RX_INTS         = RX_FIFO_FULL or RX_NO_DESC or RX_FINISH
TX_INTS         = TX_FINISH
INT_MASK        = RX_INTS or TX_INTS

RX_BUF_LEN      equ (8192 << RX_BUF_LEN_IDX)    ; Size of the in-memory receive ring.

IO_SIZE         = 256       ; RDC MAC I/O Size
MAX_MAC         = 2         ; MAX RDC MAC

struct  x_head

        status          dw ?   ;0-1
        len             dw ?   ;2-3
        buf             dd ?   ;4-7
        ndesc           dd ?   ;8-B
        rev1            dd ?   ;C-F
        vbufp           dd ?   ;10-13
        vndescp         dd ?   ;14-17
        skb_ptr         dd ?   ;18-1B
        rev2            dd ?   ;1C-1F

ends


struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3    ; align 4

        cur_rx          dw ?
        cur_tx          dw ?
        last_tx         dw ?
        phy_addr        dd ?
        phy_mode        dw ?
        mcr0            dw ?
        mcr1            dw ?
        switch_sig      dw ?

        rb 0x100 - ($ and 0xff) ; align 256
        tx_ring         rb ((TX_RING_SIZE*sizeof.x_head+32) and 0xfffffff0)
        rx_ring         rb ((RX_RING_SIZE*sizeof.x_head+32) and 0xfffffff0)

ends



;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  2,"Loading driver\n"
        invoke  RegService, my_service, service_proc
        ret

  .fail:
        xor     eax, eax
        ret

endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [edx + IOCTL.input]               ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     @f
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device, .fail    ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], unload
        mov     [ebx + device.name], my_service

; save the pci bus and device numbers

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [ebx + device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [ebx + device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device

        stdcall PCI_find_io, [ebx + device.pci_bus], [ebx + device.pci_dev]
        mov     [ebx + device.io_addr], eax

; We've found the io address, find IRQ now

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        mov     [ebx + device.irq_line], al

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.io_addr]:8

; Ok, the eth_device structure is ready, let's probe the device

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2

        DEBUGF  2,"Initialised OK\n"

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  2,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  2,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err2:
        dec     [devices]
  .err:
        invoke  KernelFree, ebx
  .fail:
        DEBUGF  2, "Failed to load\n"
        or      eax, -1
        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


;mdio_read:
;        stdcall phy_read, [ebx + device.io_addr], [ebx + device.phy_addr], ecx

;        ret

;mdio_write:
;        stdcall phy_write, [ebx + device.io_addr], [ebx + device.phy_addr], ecx, eax

;        ret


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
;;  probe: enables the device (if it really is R6040)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
        DEBUGF  1,"Probing\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

        ; If PHY status change register is still set to zero
        ; it means the bootloader didn't initialize it

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], PHY_CC
        in      ax, dx
        test    ax, ax
        jnz     @f
        mov     ax, 0x9F07
        out     dx, ax
  @@:

        call    read_mac

        ; Some bootloaders/BIOSes do not initialize MAC address, warn about that
        and     eax, 0xFF
        or      eax, dword [ebx + device.mac]
        test    eax, eax
        jnz     @f
        DEBUGF  2, "MAC address not initialized!\n"

  @@:
        ; Init RDC private data
        mov     [ebx + device.mcr0], MCR0_XMTEN or MCR0_RCVEN
        mov     [ebx + device.phy_addr], PHY1_ADDR
        mov     [ebx + device.switch_sig], 0

        ; Check the vendor ID on the PHY, if 0xFFFF assume none attached
        stdcall phy_read, [ebx + device.phy_addr], 2
        cmp     ax, 0xFFFF
        jne     @f
        DEBUGF  2, "Failed to detect an attached PHY!\n"
  .err:
        mov     eax, -1
        ret
  @@:

        ; Set MAC address
        call    init_mac_regs

        ; Initialize and alloc RX/TX buffers
        call    init_txbufs
        call    init_rxbufs
        test    eax, eax
        jnz     .err

        ; Read the PHY ID
        mov     [ebx + device.phy_mode], MCR0_FD
        stdcall phy_read, 0, 2
        mov     [ebx + device.switch_sig], ax
        cmp     ax, ICPLUS_PHY_ID
        jne     @f
        stdcall phy_write, 29, 31, 0x175C ; Enable registers
        jmp     .phy_readen
  @@:

        ; PHY Mode Check
        stdcall phy_write, [ebx + device.phy_addr], 4, PHY_CAP
        stdcall phy_write, [ebx + device.phy_addr], 0, PHY_MODE

      if PHY_MODE = 0x3100
        call    phy_mode_chk
        mov     [ebx + device.phy_mode], ax
        jmp     .phy_readen
      end if

      if not (PHY_MODE and 0x0100)
        mov     [ebx + device.phy_mode], 0
      end if

  .phy_readen:

        ; Set duplex mode
        mov     ax, [ebx + device.phy_mode]
        or      [ebx + device.mcr0], ax

        ; improve performance (by RDC guys)
        stdcall phy_read, 30, 17
        or      ax, 0x4000
        stdcall phy_write, 30, 17, eax

        stdcall phy_read, 30, 17
        and     ax, not 0x2000
        stdcall phy_write, 30, 17, eax

        stdcall phy_write, 0, 19, 0x0000
        stdcall phy_write, 0, 30, 0x01F0

        ; Initialize all Mac registers
        call    init_mac_regs


align 4
reset:

        DEBUGF  1,"Resetting\n"

        ; Mask off Interrupt
        xor     ax, ax
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MIER
        out     dx, ax

; attach int handler

        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

        ;Reset RDC MAC
        mov     eax, MAC_RST
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MCR1
        out     dx, ax

        mov     ecx, 2048 ;limit
  .read:
        in      ax, dx
        test    ax, 0x1
        jnz      @f
        dec     ecx
        test    ecx, ecx
        jnz     .read
  @@:
        ;Reset internal state machine
        mov     ax,  2
        set_io  [ebx + device.io_addr], MAC_SM
        out     dx, ax

        xor     ax, ax
        out     dx, ax

        mov     esi, 5
        invoke  Sleep

        ;MAC Bus Control Register
        mov     ax, MBCR_DEFAULT
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MBCR
        out     dx, ax

        ;Buffer Size Register
        mov     ax, MAX_BUF_SIZE
        set_io  [ebx + device.io_addr], MR_BSR
        out     dx, ax

        ;Write TX ring start address
        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], MTD_SA0
        out     dx, ax
        shr     eax, 16
        set_io  [ebx + device.io_addr], MTD_SA1
        out     dx, ax

        ;Write RX ring start address
        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], MRD_SA0
        out     dx, ax
        shr     eax, 16
        set_io  [ebx + device.io_addr], MRD_SA1
        out     dx, ax

        ;Set interrupt waiting time and packet numbers
        xor     ax, ax
        set_io  [ebx + device.io_addr], MT_ICR
        out     dx, ax

        ;Enable interrupts
        mov     ax, INT_MASK
        set_io  [ebx + device.io_addr], MIER
        out     dx, ax

        ;Enable RX
        mov     ax, [ebx + device.mcr0]
        or      ax, MCR0_RCVEN
        set_io  [ebx + device.io_addr], 0
        out     dx, ax

        ;Let TX poll the descriptors
        ;we may got called by tx_timeout which has left some unset tx buffers
        xor     ax, ax
        inc     ax
        set_io  [ebx + device.io_addr], MTPR
        out     dx, ax

; Set the mtu, kernel will be able to send now
        mov     [ebx + device.mtu], 1514

; Set link state to unknown
        mov     [ebx + device.state], ETH_LINK_UNKNOWN

        DEBUGF  1,"Reset ok\n"
        xor     eax, eax
        ret



align 4
init_txbufs:

        DEBUGF  1,"Init TxBufs\n"

        lea     esi, [ebx + device.tx_ring]
        lea     eax, [ebx + device.tx_ring + sizeof.x_head]
        invoke  GetPhysAddr
        mov     ecx, TX_RING_SIZE

  .next_desc:
        mov     [esi + x_head.ndesc], eax
        mov     [esi + x_head.skb_ptr], 0
        mov     [esi + x_head.status], DSC_OWNER_MAC
        add     eax, sizeof.x_head
        add     esi, sizeof.x_head
        dec     ecx
        jnz     .next_desc

        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        mov     dword[ebx + device.tx_ring + sizeof.x_head*(TX_RING_SIZE-1) + x_head.ndesc], eax

        ret



align 4
init_rxbufs:

        DEBUGF  1,"Init RxBufs\n"

        lea     esi, [ebx + device.rx_ring]
        lea     eax, [ebx + device.rx_ring + sizeof.x_head]
        invoke  GetPhysAddr
        mov     edx, eax
        mov     ecx, RX_RING_SIZE

  .next_desc:
        mov     [esi + x_head.ndesc], edx
        push    esi ecx edx
        invoke  NetAlloc, MAX_BUF_SIZE+NET_BUFF.data
        pop     edx ecx esi
        test    eax, eax
        jz      .out_of_mem
        mov     [esi + x_head.skb_ptr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + x_head.buf], eax
        mov     [esi + x_head.status], DSC_OWNER_MAC

        add     edx, sizeof.x_head
        add     esi, sizeof.x_head
        dec     ecx
        jnz     .next_desc

; complete the ring by linking the last to the first
        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        mov     dword[ebx + device.rx_ring + sizeof.x_head*(RX_RING_SIZE-1) + x_head.ndesc], eax

        xor     eax, eax
        ret

  .out_of_mem:
        or      eax, -1
        ret



align 4
phy_mode_chk:

        DEBUGF  1,"Checking PHY mode\n"

        ; PHY Link Status Check
        stdcall phy_read, [ebx + device.phy_addr], MII_BMSR
        test    ax, BMSR_LSTATUS
        jz      .ret_0x8000

        ; PHY Chip Auto-Negotiation Status
        test    ax, BMSR_ANEGCOMPLETE
        jnz     .auto_nego

        ; Force Mode
        stdcall phy_read, [ebx + device.phy_addr], MII_BMCR
        test    ax, BMCR_FULLDPLX
        jnz     .ret_0x8000

  .auto_nego:
        ; Auto Negotiation Mode
        stdcall phy_read, [ebx + device.phy_addr], MII_LPA
        mov     cx, ax
        stdcall phy_read, [ebx + device.phy_addr], MII_ADVERTISE
        and     ax, cx
        test    ax, ADVERTISE_10FULL + ADVERTISE_100FULL
        jnz     .ret_0x8000

        xor     eax, eax
        ret

  .ret_0x8000:
        mov     eax, 0x8000
        ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc transmit stdcall bufferptr

        pushf
        cli

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .fail
        cmp     [esi + NET_BUFF.length], 60
        jb      .fail

        movzx   edi, [ebx + device.cur_tx]
        shl     edi, 5
        add     edi, ebx
        add     edi, device.tx_ring

        DEBUGF  1,"TX buffer status: 0x%x\n", [edi + x_head.status]:4

        test    [edi + x_head.status], DSC_OWNER_MAC    ; check if buffer is available
        jnz     .wait_to_send

  .do_send:
        DEBUGF  1,"Sending now\n"

        mov     [edi + x_head.skb_ptr], esi
        mov     eax, esi
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + x_head.buf], eax
        mov     ecx, [esi + NET_BUFF.length]
        mov     [edi + x_head.len], cx
        mov     [edi + x_head.status], DSC_OWNER_MAC

        ; Trigger the MAC to check the TX descriptor
        mov     ax, 0x01
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MTPR
        out     dx, ax

        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], TX_RING_SIZE - 1

; Update stats
        inc     [ebx + device.packets_tx]
        mov     eax, [esi + NET_BUFF.length]
        add     dword[ebx + device.bytes_tx], eax
        adc     dword[ebx + device.bytes_tx + 4], 0

        popf
        xor     eax, eax
        ret

  .wait_to_send:
        DEBUGF  1,"Waiting for TX buffer\n"
        invoke  GetTimerTicks           ; returns in eax
        lea     edx, [eax + 100]
  .l2:
        mov     esi, [bufferptr]
        test    [edi + x_head.status], DSC_OWNER_MAC
        jz      .do_send
        popf
        mov     esi, 10
        invoke  Sleep
        invoke  GetTimerTicks
        pushf
        cli
        cmp     edx, eax
        jb      .l2

        DEBUGF  2,"Send timeout\n"
  .fail:
        DEBUGF  2,"Send failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"int\n"

; Find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MISR
        in      ax, dx
        out     dx, ax                  ; send it back to ACK
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                             ; If no device was found, abort

; At this point, test for all possible reasons, and handle accordingly

  .got_it:

        DEBUGF  1,"Device: %x Status: %x\n", ebx, ax

        push ax

        test    word[esp], RX_FINISH
        jz      .no_RX

        push    ebx
  .more_RX:
        pop     ebx

        ; Find the current RX descriptor
        movzx   edx, [ebx + device.cur_rx]
        shl     edx, 5
        lea     edx, [ebx + device.rx_ring + edx]

        ; Check the descriptor status
        mov     cx, [edx + x_head.status]
        test    cx, DSC_OWNER_MAC
        jnz     .no_RX

        DEBUGF  1,"packet status=0x%x\n", cx

        test    cx, DSC_RX_ERR          ; Global error status set
        jnz     .no_RX

        ; Packet successfully received
        movzx   ecx, [edx + x_head.len]
        and     ecx, 0xFFF
        sub     ecx, 4                  ; Do not count the CRC

        DEBUGF  1,"packet ptr=0x%x size=%u\n", [edx + x_head.skb_ptr], ecx

        ; Update stats
        add     dword[ebx + device.bytes_rx], ecx
        adc     dword[ebx + device.bytes_rx + 4], 0
        inc     dword[ebx + device.packets_rx]

        push    ebx
        ; Push packet ptr and return addr for Eth_input
        push    .more_RX
        mov     eax, [edx + x_head.skb_ptr]
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

        ; reset the RX descriptor (alloc new buffer)
        push    edx
        invoke  NetAlloc, MAX_BUF_SIZE+NET_BUFF.data
        pop     edx
        mov     [edx + x_head.skb_ptr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [edx + x_head.buf], eax
        mov     [edx + x_head.status], DSC_OWNER_MAC

        ; Use next descriptor next time
        inc     [ebx + device.cur_rx]
        and     [ebx + device.cur_rx], RX_RING_SIZE - 1

        ; At last, send packet to kernel
        jmp     [EthInput]


  .no_RX:
        test    word[esp], TX_FINISH
        jz      .no_TX

  .loop_tx:
        movzx   edi, [ebx + device.last_tx]
        shl     edi, 5
        lea     edi, [ebx + device.tx_ring + edi]

        test    [edi + x_head.status], DSC_OWNER_MAC
        jnz     .no_TX

        cmp     [edi + x_head.skb_ptr], 0
        je      .no_TX

        DEBUGF  1,"Freeing buffer 0x%x\n", [edi + x_head.skb_ptr]

        push    [edi + x_head.skb_ptr]
        mov     [edi + x_head.skb_ptr], 0
        invoke  NetFree

        inc     [ebx + device.last_tx]
        and     [ebx + device.last_tx], TX_RING_SIZE - 1

        jmp     .loop_tx

  .no_TX:
        test    word[esp], RX_NO_DESC
        jz      .no_rxdesc

        DEBUGF  2, "No more RX descriptors!\n"

  .no_rxdesc:
        test    word[esp], RX_FIFO_FULL
        jz      .no_rxfifo

        DEBUGF  2, "RX FIFO full!\n"

  .no_rxfifo:
        test    word[esp], RX_EARLY
        jz      .no_rxearly

        DEBUGF  2, "RX early\n"

  .no_rxearly:
        test    word[esp], TX_EARLY
        jz      .no_txearly

        DEBUGF  2, "TX early\n"

  .no_txearly:
        test    word[esp], EVENT_OVRFL
        jz      .no_ovrfl

        DEBUGF  2, "Event counter overflow!\n"

  .no_ovrfl:
        test    word[esp], LINK_CHANGED
        jz      .no_link

        DEBUGF  2, "Link changed\n"

  .no_link:
        pop     ax

        pop     edi esi ebx

        ret




align 4
init_mac_regs:

        DEBUGF  1,"initializing MAC regs\n"

        ; MAC operation register
        mov     ax, 1
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MCR1
        out     dx, ax
        ; Reset MAC
        mov     ax, 2
        set_io  [ebx + device.io_addr], MAC_SM
        out     dx, ax
        ; Reset internal state machine
        xor     ax, ax
        out     dx, ax
        mov     esi, 5
        invoke  Sleep

        call    read_mac

        ret




; Read a word data from PHY Chip

align 4
proc  phy_read stdcall, phy_addr:dword, reg:dword

        DEBUGF  1,"PHY read, addr=0x%x reg=0x%x\n", [phy_addr]:8, [reg]:8

        mov     eax, [phy_addr]
        shl     eax, 8
        add     eax, [reg]
        add     eax, MDIO_READ
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MMDIO
        out     dx, ax

        ;Wait for the read bit to be cleared.
        mov     ecx, 2048 ;limit
  .read:
        in      ax, dx
        test    ax, MDIO_READ
        jz      @f
        dec     ecx
        jnz     .read
  @@:

        set_io  [ebx + device.io_addr],   MMRD
        in      ax, dx
        and     eax, 0xFFFF

        DEBUGF  1,"PHY read, val=0x%x\n", eax:4

        ret

endp




; Write a word data to PHY Chip

align 4
proc  phy_write stdcall, phy_addr:dword, reg:dword, val:dword

        DEBUGF  1,"PHY write, addr=0x%x reg=0x%x val=0x%x\n", \
        [phy_addr]:8, [reg]:8, [val]:8

        mov     eax, [val]
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MMWD
        out     dx, ax

        ;Write the command to the MDIO bus
        mov     eax, [phy_addr]
        shl     eax, 8
        add     eax, [reg]
        add     eax, MDIO_WRITE
        set_io  [ebx + device.io_addr], MMDIO
        out     dx, ax

        ;Wait for the write bit to be cleared.
        mov     ecx, 2048 ;limit
  .write:
        in      ax, dx
        test    ax, MDIO_WRITE
        jz      @f
        dec     ecx
        jnz     .write
  @@:

        DEBUGF  1,"PHY write ok\n"

        ret
endp



align 4
read_mac:

        DEBUGF  1,"Reading MAC:\n"

        mov     cx, 3
        lea     edi, [ebx + device.mac]
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MID_0L
  .loop:
        in      ax, dx
        stosw
        inc     dx
        inc     dx
        dec     cx
        jnz     .loop

        DEBUGF  1,"%x-%x-%x-%x-%x-%x\n",\
        [edi-6]:2, [edi-5]:2, [edi-4]:2, [edi-3]:2, [edi-2]:2, [edi-1]:2

        ret




; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'R6040',0                    ; max 16 chars include zero

include_debug_strings

align 4
devices         dd 0
device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling

