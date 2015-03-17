;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  MTD80x driver for KolibriOS                                    ;;
;;                                                                 ;;
;;  Based on mtd80x.c from the etherboot project                   ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
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

        NUM_TX_DESC             = 6
        NUM_RX_DESC             = 12

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

; for different PHY

    MysonPHY            = 1
    AhdocPHY            = 2
    SeeqPHY             = 3
    MarvellPHY          = 4
    Myson981            = 5
    LevelOnePHY         = 6
    OtherPHY            = 10

; Offsets to the Command and Status Registers.

    PAR0                = 0x0           ; physical address 0-3
    PAR1                = 0x04          ; physical address 4-5
    MAR0                = 0x08          ; multicast address 0-3
    MAR1                = 0x0C          ; multicast address 4-7
    FAR0                = 0x10          ; flow-control address 0-3
    FAR1                = 0x14          ; flow-control address 4-5
    TCRRCR              = 0x18          ; receive & transmit configuration
    BCR                 = 0x1C          ; bus command
    TXPDR               = 0x20          ; transmit polling demand
    RXPDR               = 0x24          ; receive polling demand
    RXCWP               = 0x28          ; receive current word pointer
    TXLBA               = 0x2C          ; transmit list base address
    RXLBA               = 0x30          ; receive list base address
    ISR                 = 0x34          ; interrupt status
    IMR                 = 0x38          ; interrupt mask
    FTH                 = 0x3C          ; flow control high/low threshold
    MANAGEMENT          = 0x40          ; bootrom/eeprom and mii management
    TALLY               = 0x44          ; tally counters for crc and mpa
    TSR                 = 0x48          ; tally counter for transmit status
    BMCRSR              = 0x4c          ; basic mode control and status
    PHYIDENTIFIER       = 0x50          ; phy identifier
    ANARANLPAR          = 0x54          ; auto-negotiation advertisement and link partner ability
    ANEROCR             = 0x58          ; auto-negotiation expansion and pci conf.
    BPREMRPSR           = 0x5c          ; bypass & receive error mask and phy status

; Bits in the interrupt status/enable registers.

    RFCON               = 0x00020000    ; receive flow control xon packet
    RFCOFF              = 0x00010000    ; receive flow control xoff packet
    LSCStatus           = 0x00008000    ; link status change
    ANCStatus           = 0x00004000    ; autonegotiation completed
    FBE                 = 0x00002000    ; fatal bus error
    FBEMask             = 0x00001800    ; mask bit12-11
    ParityErr           = 0x00000000    ; parity error
    TargetErr           = 0x00001000    ; target abort
    MasterErr           = 0x00000800    ; master error
    TUNF                = 0x00000400    ; transmit underflow
    ROVF                = 0x00000200    ; receive overflow
    ETI                 = 0x00000100    ; transmit early int
    ERI                 = 0x00000080    ; receive early int
    CNTOVF              = 0x00000040    ; counter overflow
    RBU                 = 0x00000020    ; receive buffer unavailable
    TBU                 = 0x00000010    ; transmit buffer unavilable
    TI                  = 0x00000008    ; transmit interrupt
    RI                  = 0x00000004    ; receive interrupt
    RxErr               = 0x00000002    ; receive error

; Bits in the NetworkConfig register.

    RxModeMask          = 0xe0
    AcceptAllPhys       = 0x80          ; promiscuous mode
    AcceptBroadcast     = 0x40          ; accept broadcast
    AcceptMulticast     = 0x20          ; accept mutlicast
    AcceptRunt          = 0x08          ; receive runt pkt
    ALP                 = 0x04          ; receive long pkt
    AcceptErr           = 0x02          ; receive error pkt

    AcceptMyPhys        = 0x00000000
    RxEnable            = 0x00000001
    RxFlowCtrl          = 0x00002000
    TxEnable            = 0x00040000
    TxModeFDX           = 0x00100000
    TxThreshold         = 0x00e00000

    PS1000              = 0x00010000
    PS10                = 0x00080000
    FD                  = 0x00100000


; Bits in network_desc.status

    RXOWN               = 0x80000000    ; own bit
    FLNGMASK            = 0x0fff0000    ; frame length
    FLNGShift           = 16
    MARSTATUS           = 0x00004000    ; multicast address received
    BARSTATUS           = 0x00002000    ; broadcast address received
    PHYSTATUS           = 0x00001000    ; physical address received
    RXFSD               = 0x00000800    ; first descriptor
    RXLSD               = 0x00000400    ; last descriptor
    ErrorSummary        = 0x80          ; error summary
    RUNT                = 0x40          ; runt packet received
    LONG                = 0x20          ; long packet received
    FAE                 = 0x10          ; frame align error
    CRC                 = 0x08          ; crc error
    RXER                = 0x04          ; receive error

; rx_desc_control_bits

    RXIC                = 0x00800000    ; interrupt control
    RBSShift            = 0

; tx_desc_status_bits

    TXOWN               = 0x80000000    ; own bit
    JABTO               = 0x00004000    ; jabber timeout
    CSL                 = 0x00002000    ; carrier sense lost
    LC                  = 0x00001000    ; late collision
    EC                  = 0x00000800    ; excessive collision
    UDF                 = 0x00000400    ; fifo underflow
    DFR                 = 0x00000200    ; deferred
    HF                  = 0x00000100    ; heartbeat fail
    NCRMask             = 0x000000ff    ; collision retry count
    NCRShift            = 0

; tx_desc_control_bits

    TXIC                = 0x80000000    ; interrupt control
    ETIControl          = 0x40000000    ; early transmit interrupt
    TXLD                = 0x20000000    ; last descriptor
    TXFD                = 0x10000000    ; first descriptor
    CRCEnable           = 0x08000000    ; crc control
    PADEnable           = 0x04000000    ; padding control
    RetryTxLC           = 0x02000000    ; retry late collision
    PKTSMask            = 0x3ff800      ; packet size bit21-11
    PKTSShift           = 11
    TBSMask             = 0x000007ff    ; transmit buffer bit 10-0
    TBSShift            = 0

; BootROM/EEPROM/MII Management Register

    MASK_MIIR_MII_READ  = 0x00000000
    MASK_MIIR_MII_WRITE = 0x00000008
    MASK_MIIR_MII_MDO   = 0x00000004
    MASK_MIIR_MII_MDI   = 0x00000002
    MASK_MIIR_MII_MDC   = 0x00000001

; ST+OP+PHYAD+REGAD+TA

    OP_READ             = 0x6000        ; ST:01+OP:10+PHYAD+REGAD+TA:Z0
    OP_WRITE            = 0x5002        ; ST:01+OP:01+PHYAD+REGAD+TA:10

; -------------------------------------------------------------------------
;      Constants for Myson PHY
; -------------------------------------------------------------------------

    MysonPHYID          = 0xd0000302
    MysonPHYID0         = 0x0302
    StatusRegister      = 18
    SPEED100            = 0x0400        ; bit10
    FULLMODE            = 0x0800        ; bit11

; -------------------------------------------------------------------------
;      Constants for Seeq 80225 PHY
; -------------------------------------------------------------------------

    SeeqPHYID0          = 0x0016
    MIIRegister18       = 18
    SPD_DET_100         = 0x80
    DPLX_DET_FULL       = 0x40

; -------------------------------------------------------------------------
;      Constants for Ahdoc 101 PHY
; -------------------------------------------------------------------------

    AhdocPHYID0         = 0x0022
    DiagnosticReg       = 18
    DPLX_FULL           = 0x0800
    Speed_100           = 0x0400

; --------------------------------------------------------------------------
;      Constants
; --------------------------------------------------------------------------

    MarvellPHYID0               = 0x0141
    LevelOnePHYID0              = 0x0013

    MII1000BaseTControlReg      = 9
    MII1000BaseTStatusReg       = 10
    SpecificReg                 = 17

; for 1000BaseT Control Register

    PHYAbletoPerform1000FullDuplex = 0x0200
    PHYAbletoPerform1000HalfDuplex = 0x0100
    PHY1000AbilityMask             = 0x300

; for phy specific status register, marvell phy.

    SpeedMask      = 0x0c000
    Speed_1000M    = 0x08000
    Speed_100M     = 0x4000
    Speed_10M      = 0
    Full_Duplex    = 0x2000

; for phy specific status register, levelone phy

    LXT1000_100M   = 0x08000
    LXT1000_1000M  = 0x0c000
    LXT1000_Full   = 0x200

; for PHY

    LinkIsUp       = 0x0004
    LinkIsUp2      = 0x00040000



struct  descriptor
        status                  dd ?
        control                 dd ?
        buffer                  dd ?
        next_desc               dd ?

        next_desc_logical       dd ?
        skbuff                  dd ?
        reserved1               dd ?
        reserved2               dd ?
ends


struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
        dev_id          dw ?
        flags           dd ?
        crvalue         dd ?
        bcrvalue        dd ?
        cur_rx          dd ?
        cur_tx          dd ?
        default_port    dd ?
        PHYType         dd ?

; MII transceiver section.
        mii_cnt         dd ?    ; MII device addresses.
        phys            db ?    ; MII device addresses.

; descriptors
        rb 0x100 - ($ and 0xff) ; align 256
        tx_desc         rb NUM_TX_DESC*sizeof.descriptor
        rx_desc         rb NUM_RX_DESC*sizeof.descriptor

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

        DEBUGF  1,"Loading driver\n"
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

;        mov     eax, [edx + IOCTL.input]                ; get the pci bus and device numbers
        mov     ax, [eax+1]                             ;
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

        allocate_and_clear ebx, sizeof.device, .fail

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
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2                                                   ; If an error occured, exit

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
        DEBUGF  2,"removing device structure\n"
        invoke  KernelFree, ebx
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

;    /* Disable Tx Rx*/
;    outl( mtdx.crvalue & (~TxEnable) & (~RxEnable), mtdx.ioaddr + TCRRCR );
;
;    /* Reset the chip to erase previous misconfiguration. */
;    mtd_reset(nic);

        ; - Detach int handler
        ; - Remove device from local list (device_list)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax, -1
        ret


;-------
;
; PROBE
;
;-------
align 4
probe:

        DEBUGF  1,"Probing\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; Check vendor/device id's
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], 0
        cmp     ax, 0x1516
        jne     .notfound
        shr     eax, 16
        mov     [ebx + device.dev_id], ax
        cmp     ax, 0x0800
        je      .mtd800
        cmp     ax, 0x0803
        je      .mtd803
        cmp     ax, 0x0891
        je      .mtd891

  .notfound:
        DEBUGF  2,"Device not supported!\n"
        xor     eax, eax
        dec     eax
        ret

  .mtd803:
        mov     [ebx + device.name], sz_mtd803
        DEBUGF  1,"Device has chip xcvr\n"
        jmp     .xcvr_set

  .mtd800:
        DEBUGF  1,"Device has mii xcvr\n"
        mov     [ebx + device.name], sz_mtd800
        jmp     .xcvr_set

  .mtd891:
        DEBUGF  1,"Device has mii xcvr\n"
        mov     [ebx + device.name], sz_mtd800

  .xcvr_set:
        call    read_mac

; Reset the chip to erase previous misconfiguration.
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], BCR
        xor     eax, eax
        inc     eax
        out     dx, eax

; find the connected MII xcvrs
        cmp     [ebx + device.dev_id], 0x0803
        je      .is_803

;        int     phy, phy_idx =   0;
;
;        for (phy =   1; phy < 32 && phy_idx < 1; phy++) {
;            int mii_status =   mdio_read(nic, phy, 1);
;
;            if (mii_status !=   0xffff && mii_status !=   0x0000) {
;                mtdx.phys[phy_idx] =   phy;
;
;                DBG ( "%s: MII PHY found at address %d, status "
;                      "0x%4.4x.\n", mtdx.nic_name, phy, mii_status );
;                /* get phy type */
;                {
;                    unsigned int data;
;
;                    data =   mdio_read(nic, mtdx.phys[phy_idx], 2);
;                    if (data equ=   SeeqPHYID0)
;                        mtdx.PHYType =   SeeqPHY;
;                    else if (data equ=   AhdocPHYID0)
;                        mtdx.PHYType =   AhdocPHY;
;                    else if (data equ=   MarvellPHYID0)
;                        mtdx.PHYType =   MarvellPHY;
;                    else if (data equ=   MysonPHYID0)
;                        mtdx.PHYType =   Myson981;
;                    else if (data equ=   LevelOnePHYID0)
;                        mtdx.PHYType =   LevelOnePHY;
;                    else
;                        mtdx.PHYType =   OtherPHY;
;                }
;                phy_idx++;
;            }
;        }
;
;        mtdx.mii_cnt =   phy_idx;
;        if (phy_idx equ=   0) {
;            printf("%s: MII PHY not found -- this device may "
;                   "not operate correctly.\n", mtdx.nic_name);
;        }

        jmp     .no_803

  .is_803:
        mov     [ebx + device.phys], 32

; get phy type
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], PHYIDENTIFIER
        in      eax, dx

        cmp     eax, MysonPHYID
        jne     @f
        mov     [ebx + device.PHYType], MysonPHY
        DEBUGF  1,"Myson PHY\n"
        jmp     .no_803
       @@:

        mov     [ebx + device.PHYType], OtherPHY
        DEBUGF  1,"Other PHY\n"
  .no_803:

;-------
;
; RESET
;
;-------
align 4
reset:

        DEBUGF  1,"Resetting\n"

; attach irq handler
        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

; Reset the chip to erase previous misconfiguration.
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], BCR
        xor     eax, eax
        inc     eax
        out     dx, eax

        call    init_ring
        test    eax, eax
        jnz     .err

; Initialize other registers.
; Configure the PCI bus bursts and FIFO thresholds.
        mov     [ebx + device.bcrvalue], 0x10         ; little-endian, 8 burst length
        mov     [ebx + device.crvalue], 0xa00         ; 128 burst length

        cmp     [ebx + device.dev_id], 0x891
        jne     @f
        or      [ebx + device.bcrvalue], 0x200       ; set PROG bit
        or      [ebx + device.crvalue], 0x02000000   ; set enhanced bit
  @@:
        or      [ebx + device.crvalue], RxEnable + TxThreshold + TxEnable

        call    set_rx_mode

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], BCR
        mov     eax, [ebx + device.bcrvalue]
        out     dx, eax

        set_io  [ebx + device.io_addr], TCRRCR
        mov     eax, [ebx + device.crvalue]
        out     dx, eax

        call    getlinkstatus

; Restart Rx engine if stopped.
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], RXPDR
        xor     eax, eax
        out     dx, eax

; Enable interrupts
        set_io  [ebx + device.io_addr], ISR
        mov     eax, FBE or TUNF or CNTOVF or RBU or TI or RI
        out     dx, eax
        set_io  [ebx + device.io_addr], IMR
        out     dx, eax

; clear packet/byte counters
        xor     eax, eax
        lea     edi, [ebx + device.bytes_tx]
        mov     ecx, 6
        rep     stosd

        mov     [ebx + device.mtu], 1514
        xor     eax, eax
        ret

  .err:
        DEBUGF  2, "Error!\n"
        or      eax, -1
        ret




align 4
init_ring:

        DEBUGF  1, "initializing rx and tx ring\n"

; Initialize all Rx descriptors
        lea     esi, [ebx + device.rx_desc]
        mov     [ebx + device.cur_rx], esi
        mov     ecx, NUM_RX_DESC
  .rx_desc_loop:
        mov     [esi + descriptor.status], RXOWN
        mov     [esi + descriptor.control], 1514 shl RBSShift

        lea     eax, [esi + sizeof.descriptor]
        mov     [esi + descriptor.next_desc_logical], eax
        push    ecx esi
        invoke  GetPhysAddr
        mov     [esi + descriptor.next_desc], eax

        invoke  NetAlloc, 1514+NET_BUFF.data
        pop     esi ecx
        test    eax, eax
        jz      .out_of_mem
        push    ecx esi
        mov     [esi + descriptor.skbuff], eax
        invoke  GetPgAddr
        add     eax, NET_BUFF.data
        pop     esi ecx
        mov     [esi + descriptor.buffer], eax

        add     esi, sizeof.descriptor
        loop    .rx_desc_loop

; Mark the last entry as wrapping the ring.
        lea     eax, [ebx + device.rx_desc]
        mov     [esi - sizeof.descriptor + descriptor.next_desc_logical], eax
        push    esi
        invoke  GetPhysAddr
        pop     esi
        mov     [esi - sizeof.descriptor + descriptor.next_desc], eax

        set_io  [ebx + device.io_addr],   0
        set_io  [ebx + device.io_addr],   RXLBA
        out     dx, eax

; Initialize all Tx descriptors
        lea     esi, [ebx + device.tx_desc]
        mov     [ebx + device.cur_tx], esi
        mov     ecx, NUM_TX_DESC
  .tx_desc_loop:
        mov     [esi + descriptor.status], 0

        lea     eax, [esi + sizeof.descriptor]
        mov     [esi + descriptor.next_desc_logical], eax
        push    ecx esi
        invoke  GetPhysAddr
        pop     esi ecx
        mov     [esi + descriptor.next_desc], eax
        mov     [esi + descriptor.skbuff], 0
        add     esi, sizeof.descriptor
        loop    .tx_desc_loop

; Mark the last entry as wrapping the ring.
        lea     eax, [ebx + device.tx_desc]
        mov     [esi - sizeof.descriptor + descriptor.next_desc_logical], eax
        push    esi
        invoke  GetPhysAddr
        pop     esi
        mov     [esi - sizeof.descriptor + descriptor.next_desc], eax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], TXLBA
        out     dx, eax

        xor     eax, eax
        ret

  .out_of_mem:
        or      eax, -1
        ret


align 4
set_rx_mode:

        DEBUGF  1,"Setting RX mode\n"

; Too many to match, or accept all multicasts.
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], MAR0
        xor     eax, eax
        not     eax
        out     dx, eax
        set_io  [ebx + device.io_addr], MAR1
        out     dx, eax

        and     [ebx + device.crvalue], not (RxModeMask)
        or      [ebx + device.crvalue], AcceptBroadcast + AcceptMulticast + AcceptMyPhys

        ret


align 4
getlinkstatus:

        DEBUGF  1,"Getting link status\n"

        mov     [ebx + device.state], ETH_LINK_DOWN     ; assume link is dead

        cmp     [ebx + device.PHYType], MysonPHY
        jne     .no_myson_phy
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], BMCRSR
        in      eax, dx
        test    eax, LinkIsUp2
        jnz     getlinktype
        ret

  .no_myson_phy:
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], BMCRSR
        in      eax, dx
        test    eax, LinkIsUp
        jnz     getlinktype
        ret

getlinktype:

        DEBUGF  1,"Getting link type\n"
        cmp     [ebx + device.PHYType], MysonPHY
        jne     .no_myson_phy

        DEBUGF  1,"myson PHY\n"
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], TCRRCR
        in      eax, dx
        test    eax, FD
        jz      @f
        DEBUGF  1,"full duplex\n"
        or      [ebx + device.state], ETH_LINK_FD
       @@:
        test    eax, PS10
        jnz     @f
        DEBUGF  1,"100mbit\n"
        or      [ebx + device.state], ETH_LINK_100M
        ret
       @@:
        DEBUGF  1,"10mbit\n"
        or      [ebx + device.state], ETH_LINK_10M
        ret

  .no_myson_phy:
        DEBUGF  1,"not a myson PHY\n"
        mov     [ebx + device.state], ETH_LINK_UNKNOWN

;        if (mtdx.PHYType equ=   SeeqPHY) { /* this PHY is SEEQ 80225 */
;            unsigned int data;
;
;            data =   mdio_read(dev, mtdx.phys[0], MIIRegister18);
;            if (data & SPD_DET_100)
;                mtdx.line_speed =   2; /* 100M */
;            else
;                mtdx.line_speed =   1; /* 10M */
;            if (data & DPLX_DET_FULL)
;                mtdx.duplexmode =   2; /* full duplex mode */
;            else
;                mtdx.duplexmode =   1; /* half duplex mode */
;        } else if (mtdx.PHYType equ=   AhdocPHY) {
;            unsigned int data;
;
;            data =   mdio_read(dev, mtdx.phys[0], DiagnosticReg);
;            if (data & Speed_100)
;                mtdx.line_speed =   2; /* 100M */
;            else
;                mtdx.line_speed =   1; /* 10M */
;            if (data & DPLX_FULL)
;                mtdx.duplexmode =   2; /* full duplex mode */
;            else
;                mtdx.duplexmode =   1; /* half duplex mode */
;        }
;        else if (mtdx.PHYType equ=   MarvellPHY) {
;            unsigned int data;
;
;            data =   mdio_read(dev, mtdx.phys[0], SpecificReg);
;            if (data & Full_Duplex)
;                mtdx.duplexmode =   2; /* full duplex mode */
;            else
;                mtdx.duplexmode =   1; /* half duplex mode */
;            data &=   SpeedMask;
;            if (data equ=   Speed_1000M)
;                mtdx.line_speed =   3; /* 1000M */
;            else if (data equ=   Speed_100M)
;                mtdx.line_speed =   2; /* 100M */
;            else
;                mtdx.line_speed =   1; /* 10M */
;        }
;        else if (mtdx.PHYType equ=   Myson981) {
;            unsigned int data;
;
;            data =   mdio_read(dev, mtdx.phys[0], StatusRegister);
;
;            if (data & SPEED100)
;                mtdx.line_speed =   2;
;            else
;                mtdx.line_speed =   1;
;
;            if (data & FULLMODE)
;                mtdx.duplexmode =   2;
;            else
;                mtdx.duplexmode =   1;
;        }
;        else if (mtdx.PHYType equ=   LevelOnePHY) {
;            unsigned int data;
;
;            data =   mdio_read(dev, mtdx.phys[0], SpecificReg);
;            if (data & LXT1000_Full)
;                mtdx.duplexmode =   2; /* full duplex mode */
;            else
;                mtdx.duplexmode =   1; /* half duplex mode */
;            data &=   SpeedMask;
;            if (data equ=   LXT1000_1000M)
;                mtdx.line_speed =   3; /* 1000M */
;            else if (data equ=   LXT1000_100M)
;                mtdx.line_speed =   2; /* 100M */
;            else
 ;               mtdx.line_speed =   1; /* 10M */
  ;      }

;        // chage crvalue
;        // mtdx.crvalue&equ(~PS10)&(~FD);
;        mtdx.crvalue &=   (~PS10) & (~FD) & (~PS1000);
;        if (mtdx.line_speed equ=   1)
;            mtdx.crvalue |=   PS10;
;        else if (mtdx.line_speed equ=   3)
;            mtdx.crvalue |=   PS1000;
;        if (mtdx.duplexmode equ=   2)
;            mtdx.crvalue |=   FD;

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

        mov     esi, [ebx + device.cur_tx]
        test    [esi + descriptor.status], TXOWN
        jnz     .fail

        push    [esi + descriptor.next_desc_logical]
        pop     [ebx + device.cur_tx]

        mov     eax, [bufferptr]
        mov     [esi + descriptor.skbuff], eax
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [esi + descriptor.buffer], eax

        mov     eax, [bufferptr]
        mov     eax, [eax + NET_BUFF.length]
        mov     ecx, eax
        shl     eax, PKTSShift               ; packet size
        shl     ecx, TBSShift
        or      eax, ecx
        or      eax, TXIC + TXLD + TXFD + CRCEnable + PADEnable
        mov     [esi + descriptor.control], eax
        mov     [esi + descriptor.status], TXOWN

; Update stats
        inc     [ebx + device.packets_tx]
        mov     eax, [bufferptr]
        mov     ecx, [eax + NET_BUFF.length]
        add     dword[ebx + device.bytes_tx], ecx
        adc     dword[ebx + device.bytes_tx + 4], 0

; TX Poll
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], TXPDR
        xor     eax, eax
        out     dx, eax

        DEBUGF  1,"Transmit OK\n"
        popf
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Transmit failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp



align 4
read_mac:

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], PAR0
        lea     edi, [ebx + device.mac]
        insd
        set_io  [ebx + device.io_addr], PAR1
        insw
        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,[ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret

align 4
write_mac:

        ret



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], ISR
        in      eax, dx
        out     dx, eax                                 ; send it back to ACK
        test    eax, eax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                                             ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:

        DEBUGF  1,"Device: %x Status: %x\n", ebx, ax

        test    ax, RI  ; receive interrupt
        jz      .no_rx
        push    ax
  .rx_loop:
        mov     esi, [ebx + device.cur_rx]
        test    [esi + descriptor.status], RXOWN
        jnz     .rx_done

        push    ebx
        push    .rx_complete

        mov     ecx, [esi + descriptor.status]
        shr     ecx, FLNGShift
        sub     ecx, 4                  ; we dont need CRC
        DEBUGF  1,"Received %u bytes\n", ecx
        mov     eax, [esi + descriptor.skbuff]
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

; Update stats
        add     dword[ebx + device.bytes_rx], ecx
        adc     dword[ebx + device.bytes_rx + 4], 0
        inc     [ebx + device.packets_rx]

        jmp     [EthInput]

  .rx_complete:
        pop     ebx
        mov     esi, [ebx + device.cur_rx]
        mov     [esi + descriptor.control], 1514 shl RBSShift
        push    esi
        invoke  NetAlloc, 1514+NET_BUFF.data
        pop     esi
;        test    eax, eax
;        jz      .rx_loop
        mov     [esi + descriptor.skbuff], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + descriptor.buffer], eax
        mov     [esi + descriptor.status], RXOWN

        push    [esi + descriptor.next_desc_logical]
        pop     [ebx + device.cur_rx]

        jmp     .rx_loop

  .rx_done:
        DEBUGF  1,"RX done\n"

; Restart Rx engine if stopped.
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], RXPDR
        xor     eax, eax
        out     dx, eax

        pop     ax
  .no_rx:

        test    ax, TI ; transmit interrupt
        jz      .no_tx
        DEBUGF  1,"TX\n"
        push    ax
        lea     esi, [ebx + device.tx_desc]
        mov     ecx, NUM_TX_DESC
  .tx_loop:
        test    [esi + descriptor.status], TXOWN
        jnz     .skip_this_one
        mov     eax, [esi + descriptor.skbuff]
        test    eax, eax
        je      .skip_this_one
        mov     [esi + descriptor.skbuff], 0
        DEBUGF  1,"freeing buffer: 0x%x\n", eax
        invoke  NetFree, eax
  .skip_this_one:
        mov     esi, [esi + descriptor.next_desc_logical]
        loop    .tx_loop
        pop     ax
  .no_tx:

        test    ax, LSCStatus
        jz      .no_link_change
        push    ax
        call    getlinkstatus
        pop     ax
  .no_link_change:

;        test    ax, TBU
;        jz      .no_tbu
;        DEBUGF  2,"Transmit buffer unavailable!\n"
;  .no_tbu:

  .fail:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret


; End of code


data fixups
end data

include '../peimport.inc'

my_service      db 'MTD80X',0                   ; max 16 chars include zero

sz_mtd800       db "Myson MTD800", 0
sz_mtd803       db "Surecom EP-320X", 0
sz_mtd891       db "Myson MTD891", 0


include_debug_strings                           ; All data wich FDO uses will be included here

align 4
devices       dd 0
device_list   rd MAX_DEVICES                    ; This list contains all pointers to device structures the driver is handling


