;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                  ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.     ;;
;; Distributed under terms of the GNU General Public License        ;;
;;                                                                  ;;
;;  AMD PCnet driver for KolibriOS                                  ;;
;;                                                                  ;;
;;  By hidnplayr & clevermouse                                      ;;
;;                                                                  ;;
;;  Based on the PCnet32 driver for MenuetOS, by Jarek Pelczar      ;;
;;                                                                  ;;
;;          GNU GENERAL PUBLIC LICENSE                              ;;
;;             Version 2, June 1991                                 ;;
;;                                                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

; configureable area

        MAX_DEVICES             = 16    ; Maximum number of devices this driver may handle

        __DEBUG__               = 1     ; 1 = on, 0 = off
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

        TX_RING_SIZE            = 32    ; Number of packets in send ring buffer
        RX_RING_SIZE            = 32    ; Number of packets in receive ring buffer

; end configureable area

section '.flat' readable writable executable

include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../fdo.inc'
include '../netdrv.inc'

if (bsr TX_RING_SIZE)>(bsf TX_RING_SIZE)
  display 'TX_RING_SIZE must be a power of two'
  err
end if

if (bsr RX_RING_SIZE)>(bsf RX_RING_SIZE)
  display 'RX_RING_SIZE must be a power of two'
  err
end if

        PORT_AUI                = 0x00
        PORT_10BT               = 0x01
        PORT_GPSI               = 0x02
        PORT_MII                = 0x03
        PORT_PORTSEL            = 0x03
        PORT_ASEL               = 0x04
        PORT_100                = 0x40
        PORT_FD                 = 0x80

        DMA_MASK                = 0xffffffff

        TX_RING_LEN_BITS        = ((bsf TX_RING_SIZE) shl 12)
        RX_RING_LEN_BITS        = ((bsf RX_RING_SIZE) shl 4)

        PKT_BUF_SZ              = 1544

        WIO_RDP                 = 0x10
        WIO_RAP                 = 0x12
        WIO_RESET               = 0x14
        WIO_BDP                 = 0x16

        DWIO_RDP                = 0x10
        DWIO_RAP                = 0x14
        DWIO_RESET              = 0x18
        DWIO_BDP                = 0x1C

; CSR registers

        CSR_CSR                 = 0x00
        CSR_IAB0                = 0x01
        CSR_IAB1                = 0x02
        CSR_IMR                 = 0x03
        CSR_TFEAT               = 0x04
        CSR_EXTCTL1             = 0x05
        CSR_DTBLLEN             = 0x06
        CSR_EXTCTL2             = 0x07
        CSR_MAR0                = 0x08
        CSR_MAR1                = 0x09
        CSR_MAR2                = 0x0A
        CSR_MAR3                = 0x0B
        CSR_PAR0                = 0x0C
        CSR_PAR1                = 0x0D
        CSR_PAR2                = 0x0E
        CSR_MODE                = 0x0F
        CSR_RXADDR0             = 0x18
        CSR_RXADDR1             = 0x19
        CSR_TXADDR0             = 0x1E
        CSR_TXADDR1             = 0x1F
        CSR_TXPOLL              = 0x2F
        CSR_RXPOLL              = 0x31
        CSR_RXRINGLEN           = 0x4C
        CSR_TXRINGLEN           = 0x4E
        CSR_DMACTL              = 0x50
        CSR_BUSTIMER            = 0x52
        CSR_MEMERRTIMEO         = 0x64
        CSR_ONNOWMISC           = 0x74
        CSR_ADVFEAT             = 0x7A
        CSR_MACCFG              = 0x7D
        CSR_CHIPID0             = 0x58
        CSR_CHIPID1             = 0x59

; Control and Status Register (CSR0)

        CSR_INIT                = 0x0001 ;1 shl 0
        CSR_START               = 0x0002 ;1 shl 1
        CSR_STOP                = 0x0004 ;1 shl 2
        CSR_TX                  = 0x0008 ;1 shl 3
        CSR_TXON                = 0x0010 ;1 shl 4
        CSR_RXON                = 0x0020 ;1 shl 5
        CSR_INTEN               = 0x0040 ;1 shl 6
        CSR_INTR                = 0x0080 ;1 shl 7
        CSR_IDONE               = 0x0100 ;1 shl 8
        CSR_TINT                = 0x0200 ;1 shl 9
        CSR_RINT                = 0x0400 ;1 shl 10
        CSR_MERR                = 0x0800 ;1 shl 11
        CSR_MISS                = 0x1000 ;1 shl 12
        CSR_CERR                = 0x2000 ;1 shl 13

; Interrupt masks and deferral control (CSR3)

        IMR_BSWAP               = 0x0004
        IMR_ENMBA               = 0x0008  ; enable modified backoff alg
        IMR_DXMT2PD             = 0x0010
        IMR_LAPPEN              = 0x0020  ; lookahead packet processing enb
        IMR_DXSUFLO             = 0x0040  ; disable TX stop on underflow
        IMR_IDONE               = 0x0100
        IMR_TINT                = 0x0200
        IMR_RINT                = 0x0400
        IMR_MERR                = 0x0800
        IMR_MISS                = 0x1000

        ; Masked interrupts will be disabled.
        IMR                     = 0 ;IMR_IDONE ;or IMR_TINT or IMR_RINT or IMR_MERR or IMR_MISS

; Test and features control (CSR4)

        TFEAT_TXSTRTMASK        = 0x0004
        TFEAT_TXSTRT            = 0x0008
        TFEAT_RXCCOFLOWM        = 0x0010  ; Rx collision counter oflow
        TFEAT_RXCCOFLOW         = 0x0020
        TFEAT_UINT              = 0x0040
        TFEAT_UINTREQ           = 0x0080
        TFEAT_MISSOFLOWM        = 0x0100
        TFEAT_MISSOFLOW         = 0x0200
        TFEAT_STRIP_FCS         = 0x0400
        TFEAT_PAD_TX            = 0x0800
        TFEAT_TXDPOLL           = 0x1000
        TFEAT_DMAPLUS           = 0x4000

; Extended control and interrupt 1 (CSR5)

        EXTCTL1_SPND            = 0x0001  ; suspend
        EXTCTL1_MPMODE          = 0x0002  ; magic packet mode
        EXTCTL1_MPENB           = 0x0004  ; magic packet enable
        EXTCTL1_MPINTEN         = 0x0008  ; magic packet interrupt enable
        EXTCTL1_MPINT           = 0x0010  ; magic packet interrupt
        EXTCTL1_MPPLBA          = 0x0020  ; magic packet phys. logical bcast
        EXTCTL1_EXDEFEN         = 0x0040  ; excessive deferral interrupt enb.
        EXTCTL1_EXDEF           = 0x0080  ; excessive deferral interrupt
        EXTCTL1_SINTEN          = 0x0400  ; system interrupt enable
        EXTCTL1_SINT            = 0x0800  ; system interrupt
        EXTCTL1_LTINTEN         = 0x4000  ; last TX interrupt enb
        EXTCTL1_TXOKINTD        = 0x8000  ; TX OK interrupt disable

; RX/TX descriptor len (CSR6)

        DTBLLEN_RLEN            = 0x0F00
        DTBLLEN_TLEN            = 0xF000

; Extended control and interrupt 2 (CSR7)

        EXTCTL2_MIIPDTINTE      = 0x0001
        EXTCTL2_MIIPDTINT       = 0x0002
        EXTCTL2_MCCIINTE        = 0x0004
        EXTCTL2_MCCIINT         = 0x0008
        EXTCTL2_MCCINTE         = 0x0010
        EXTCTL2_MCCINT          = 0x0020
        EXTCTL2_MAPINTE         = 0x0040
        EXTCTL2_MAPINT          = 0x0080
        EXTCTL2_MREINTE         = 0x0100
        EXTCTL2_MREINT          = 0x0200
        EXTCTL2_STINTE          = 0x0400
        EXTCTL2_STINT           = 0x0800
        EXTCTL2_RXDPOLL         = 0x1000
        EXTCTL2_RDMD            = 0x2000
        EXTCTL2_RXFRTG          = 0x4000
        EXTCTL2_FASTSPNDE       = 0x8000

; Mode (CSR15)

        MODE_RXD                = 0x0001  ; RX disable
        MODE_TXD                = 0x0002  ; TX disable
        MODE_LOOP               = 0x0004  ; loopback enable
        MODE_TXCRCD             = 0x0008
        MODE_FORCECOLL          = 0x0010
        MODE_RETRYD             = 0x0020
        MODE_INTLOOP            = 0x0040
        MODE_PORTSEL            = 0x0180
        MODE_RXVPAD             = 0x2000
        MODE_RXNOBROAD          = 0x4000
        MODE_PROMISC            = 0x8000

; BCR (Bus Control Registers)

        BCR_MMRA                = 0x00    ; Master Mode Read Active
        BCR_MMW                 = 0x01    ; Master Mode Write Active
        BCR_MISCCFG             = 0x02
        BCR_LED0                = 0x04
        BCR_LED1                = 0x05
        BCR_LED2                = 0x06
        BCR_LED3                = 0x07
        BCR_DUPLEX              = 0x09
        BCR_BUSCTL              = 0x12
        BCR_EECTL               = 0x13
        BCR_SSTYLE              = 0x14
        BCR_PCILAT              = 0x16
        BCR_PCISUBVENID         = 0x17
        BCR_PCISUBSYSID         = 0x18
        BCR_SRAMSIZE            = 0x19
        BCR_SRAMBOUND           = 0x1A
        BCR_SRAMCTL             = 0x1B
        BCR_MIICTL              = 0x20
        BCR_MIIADDR             = 0x21
        BCR_MIIDATA             = 0x22
        BCR_PCIVENID            = 0x23
        BCR_PCIPCAP             = 0x24
        BCR_DATA0               = 0x25
        BCR_DATA1               = 0x26
        BCR_DATA2               = 0x27
        BCR_DATA3               = 0x28
        BCR_DATA4               = 0x29
        BCR_DATA5               = 0x2A
        BCR_DATA6               = 0x2B
        BCR_DATA7               = 0x2C
        BCR_ONNOWPAT0           = 0x2D
        BCR_ONNOWPAT1           = 0x2E
        BCR_ONNOWPAT2           = 0x2F
        BCR_PHYSEL              = 0x31

; RX status register

        RXSTAT_BPE              = 0x0080        ; bus parity error
        RXSTAT_ENP              = 0x0100        ; end of packet
        RXSTAT_STP              = 0x0200        ; start of packet
        RXSTAT_BUFF             = 0x0400        ; buffer error
        RXSTAT_CRC              = 0x0800        ; CRC error
        RXSTAT_OFLOW            = 0x1000        ; rx overrun
        RXSTAT_FRAM             = 0x2000        ; framing error
        RXSTAT_ERR              = 0x4000        ; error summary
        RXSTAT_OWN              = 0x8000

; TX status register

        TXSTAT_TRC              = 0x0000000F    ; transmit retries
        TXSTAT_RTRY             = 0x04000000    ; retry
        TXSTAT_LCAR             = 0x08000000    ; lost carrier
        TXSTAT_LCOL             = 0x10000000    ; late collision
        TXSTAT_EXDEF            = 0x20000000    ; excessive deferrals
        TXSTAT_UFLOW            = 0x40000000    ; transmit underrun
        TXSTAT_BUFF             = 0x80000000    ; buffer error

        TXCTL_OWN               = 0x8000
        TXCTL_ERR               = 0x4000        ; error summary
        TXCTL_ADD_FCS           = 0x2000        ; add FCS to pkt
        TXCTL_MORE_LTINT        = 0x1000
        TXCTL_ONE               = 0x0800
        TXCTL_DEF               = 0x0400
        TXCTL_STP               = 0x0200
        TXCTL_ENP               = 0x0100
        TXCTL_BPE               = 0x0080

        TXCTL_MBO               = 0x0000F000
        TXCTL_BUFSZ             = 0x00000FFF

        MAX_PHYS                = 32


; Pcnet configuration structure
struct  pcnet_init_block

        mode            dw ?
        tlen_rlen       dw ?
        phys_addr       dp ?
        reserved        dw ?
        filter          dq ?
        rx_ring_phys    dd ?
        tx_ring_phys    dd ?
ends


struct  device          ETH_DEVICE

        rb 0x100-($ and 0xff)   ; align 256
        init_block      pcnet_init_block

        rb 0x100-($ and 0xff)   ; align 256
        rx_ring         rb RX_RING_SIZE * sizeof.descriptor

        rb 0x100-($ and 0xff)   ; align 256
        tx_ring         rb TX_RING_SIZE * sizeof.descriptor

        cur_rx          dd ?
        cur_tx          dd ?
        last_tx         dd ?

        options         dd ?
        full_duplex     db ?
        chip_version    dw ?
        mii             db ?
        ltint           db ?
        dxsuflo         db ?
        fset            db ?
        fdx             db ?

        io_addr         dd ?
        irq_line        db ?
        pci_bus         dd ?
        pci_dev         dd ?

        phy             dw ?

        link_timer      dd ?

        rb 0x100-($ and 0xff)   ; align 256

        read_csr        dd ?
        write_csr       dd ?
        read_bcr        dd ?
        write_bcr       dd ?
        read_rap        dd ?
        write_rap       dd ?
        sw_reset        dd ?

ends

struct  descriptor

        base            dd ?
        length          dw ?
        status          dw ?
        msg_length      dw ?
        misc            dw ?
        virtual         dd ?

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
        cmp     byte[eax], 1                            ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

        mov     esi, device_list
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
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .destroy                                                ; If an error occured, exit

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev
        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state
        add     eax, NET_BUFF.data
        dec     [devices]
  .err:
        DEBUGF  2,"Error, removing all data !\n"
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

        cmp     [ebx + device.link_timer], 0
        je      @f
        invoke  CancelTimerHS, [ebx + device.link_timer]
  @@:

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
;;  probe: enables the device (if it really is a PCnet device)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:

        mov     edx, [ebx + device.io_addr]

        call    wio_reset

        xor     ecx, ecx
        call    wio_read_csr
        cmp     eax, 4
        jne     .try_dwio

        ; Try Word I/O
        mov     ax, 88
        add     edx, WIO_RAP
        out     dx, ax
        nop
        nop
        in      ax, dx
        sub     edx, WIO_RAP
        cmp     ax, 88
        jne     .try_dwio

        call    switch_to_wio

        jmp     .L1

  .try_dwio:
        call    dwio_reset

        xor     ecx, ecx
        call    dwio_read_csr
        cmp     eax, 4
        jne     .no_dev

        ; Try Dword I/O
        add     edx, DWIO_RAP
        mov     eax, 88
        out     dx, eax
        nop
        nop
        in      eax, dx
        sub     edx, DWIO_RAP
        and     eax, 0xffff
        cmp     eax, 88
        jne     .no_dev

        call    switch_to_dwio

        jmp     .L1

  .no_dev:
        DEBUGF  2,"device not found!\n"
        mov     eax, 1
        ret

  .L1:
        mov     ecx, CSR_CHIPID0
        call    [ebx + device.read_csr]

        mov     esi, eax
        shr     esi, 12

        and     ax, 0xfff
        cmp     ax, 3
        jne     .no_dev

        mov     ecx, CSR_CHIPID1
        call    [ebx + device.read_csr]
        shl     eax, 4
        or      eax, esi
        mov     [ebx + device.chip_version], ax

        mov     [ebx + device.fdx], 0
        mov     [ebx + device.mii], 0
        mov     [ebx + device.fset], 0
        mov     [ebx + device.dxsuflo], 0
        mov     [ebx + device.ltint], 0

        cmp     ax, 0x2420
        je      .L2
        cmp     ax, 0x2430
        je      .L2

        mov     [ebx + device.fdx], 1

        cmp     ax, 0x2621
        je      .L4
        cmp     ax, 0x2623
        je      .L5
        cmp     ax, 0x2624
        je      .L6
        cmp     ax, 0x2625
        je      .L7
        cmp     ax, 0x2626
        je      .L8
        cmp     ax, 0x2627
        je      .L9

        DEBUGF  2,"Invalid chip rev\n"
        jmp     .no_dev
  .L2:
        mov     [ebx + device.name], device_l2
        jmp     .L10
  .L4:
        mov     [ebx + device.name], device_l4
;        mov     [ebx + device.fdx], 1
        jmp     .L10
  .L5:
        mov     [ebx + device.name], device_l5
;        mov     [ebx + device.fdx], 1
        mov     [ebx + device.mii], 1
        mov     [ebx + device.fset], 1
        mov     [ebx + device.ltint], 1
        jmp     .L10
  .L6:
        mov     [ebx + device.name], device_l6
;        mov     [ebx + device.fdx], 1
        mov     [ebx + device.mii], 1
        mov     [ebx + device.fset], 1
        jmp     .L10
  .L7:
        mov     [ebx + device.name], device_l7
;        mov     [ebx + device.fdx], 1
        mov     [ebx + device.mii], 1
        jmp     .L10
  .L8:
        mov     [ebx + device.name], device_l8
;        mov     [ebx + device.fdx], 1
        mov     ecx, CSR_RXPOLL
        call    dword [ebx + device.read_bcr]
        call    dword [ebx + device.write_bcr]
        jmp     .L10
  .L9:
        mov     [ebx + device.name], device_l9
;        mov     [ebx + device.fdx], 1
        mov     [ebx + device.mii], 1
  .L10:
        DEBUGF  1,"device name: %s\n", [ebx + device.name]

        cmp     [ebx + device.fset], 1
        jne     .L11
        mov     ecx, BCR_BUSCTL
        call    [ebx + device.read_bcr]
        or      eax, 0x800
        call    [ebx + device.write_bcr]

        mov     ecx, CSR_DMACTL
;        call    [ebx + device.read_csr]
        mov     eax, 0xc00
        call    [ebx + device.write_csr]

        mov     [ebx + device.dxsuflo],1
        mov     [ebx + device.ltint],1
  .L11:

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

        mov     [ebx + device.options], PORT_ASEL
        mov     [ebx + device.init_block.mode], MODE_RXD + MODE_TXD     ; disable receive and transmit
        mov     [ebx + device.init_block.tlen_rlen], (TX_RING_LEN_BITS or RX_RING_LEN_BITS)

        mov     dword[ebx + device.init_block.filter], 0
        mov     dword[ebx + device.init_block.filter+4], 0

align 4
reset:

; Stop link check timer if it was already running
        cmp     [ebx + device.link_timer], 0
        je      @f
        invoke  CancelTimerHS, [ebx + device.link_timer]
  @@:

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

        mov     edx, [ebx + device.io_addr]
        call    [ebx + device.sw_reset]

        ; Switch pcnet32 to 32bit mode
        mov     ecx, BCR_SSTYLE
        mov     eax, 2
        call    [ebx + device.write_bcr]

        ; set/reset autoselect bit
        mov     ecx, BCR_MISCCFG
        call    [ebx + device.read_bcr]
        and     eax, not 2
        test    [ebx + device.options], PORT_ASEL
        jz      @f
        or      eax, 2
  @@:
        call    [ebx + device.write_bcr]

        ; Handle full duplex setting
        cmp     byte [ebx + device.full_duplex], 0
        je      .duplex_ok
        mov     ecx, BCR_DUPLEX
        call    [ebx + device.read_bcr]
        and     eax, not 3
        test    [ebx + device.options], PORT_FD
        jz      @f
        or      eax, 1
        cmp     [ebx + device.options], PORT_FD or PORT_AUI
        jne     .set_duplex
        or      eax, 2
        jmp     .set_duplex
  @@:
        test    [ebx + device.options], PORT_ASEL
        jz      .set_duplex
        cmp     [ebx + device.chip_version], 0x2627
        jne     .set_duplex
        or      eax, 3
  .set_duplex:
        mov     ecx, BCR_DUPLEX
        call    [ebx + device.write_bcr]
  .duplex_ok:

        ; set/reset GPSI bit in test register
        mov     ecx, 124
        call    [ebx + device.read_csr]
        mov     ecx, [ebx + device.options]
        and     ecx, PORT_PORTSEL
        cmp     ecx, PORT_GPSI
        jne     @f
        or      eax, 0x10
  @@:
        call    [ebx + device.write_csr]
        cmp     [ebx + device.mii], 0
        je      .L6
        test    [ebx + device.options], PORT_ASEL
        jnz     .L6
        mov     ecx, BCR_MIICTL
        call    [ebx + device.read_bcr]
        and     eax, not 0x38
        test    [ebx + device.options], PORT_FD
        jz      @f
        or      eax, 0x10
  @@:
        test    [ebx + device.options], PORT_100
        jz      @f
        or      eax, 0x08
  @@:
        call    [ebx + device.write_bcr]
        jmp     .L9
  .L6:
        test    [ebx + device.options], PORT_ASEL
        jz      .L9
        mov     ecx, BCR_MIICTL
        DEBUGF  1,"ASEL, enable auto-negotiation\n"
        call    [ebx + device.read_bcr]
        and     eax, not 0x98
        or      eax, 0x20
        call    [ebx + device.write_bcr]
  .L9:
        cmp     [ebx + device.ltint], 0
        je      @f
        mov     ecx, 5
        call    [ebx + device.read_csr]
        or      eax, (1 shl 14)
        call    [ebx + device.write_csr]
  @@:
        mov     eax, [ebx + device.options]
        and     eax, PORT_PORTSEL
        shl     eax, 7
        mov     [ebx + device.init_block.mode], ax
        mov     dword [ebx + device.init_block.filter], -1
        mov     dword [ebx + device.init_block.filter+4], -1



;-----------------------------

        test    [ebx + device.mii], 1
        jz      .no_mii

        mov     [ebx + device.phy], 0

  .mii_loop:
        mov     ecx, MII_PHYSID1
        call    mdio_read
        cmp     ax, 0xffff
        je      .next

        DEBUGF  1, "PHY ID1: 0x%x\n", ax

        mov     ecx, MII_PHYSID2
        call    mdio_read
        cmp     ax, 0xffff
        je      .next

        DEBUGF  1, "PHY ID2: 0x%x\n", ax

        jmp     .got_phy

        cmp     [ebx + device.phy], 31
        jne     .next
        mov     ax, [ebx + device.chip_version]
        inc     ax
        and     ax, 0xfffe
        cmp     ax, 0x2624              ; 79c971 & 79c972 have phantom phy at id 31
        je      .got_phy

  .next:
        inc     [ebx + device.phy]
        cmp     [ebx + device.phy], MAX_PHYS
        jb      .mii_loop

        DEBUGF  2, "No PHY found!\n"
        or      eax, -1
        ret

  .got_phy:
        DEBUGF  1, "Found PHY at 0x%x\n", [ebx + device.phy]:4

  .no_mii:

;-----------------------------------------------

        call    read_mac

        lea     esi, [ebx + device.mac]
        lea     edi, [ebx + device.init_block.phys_addr]
        movsd
        movsw

        call    init_ring
        test    eax, eax
        jnz     .fail

        mov     edx, [ebx + device.io_addr]     ; init ring destroys edx

        lea     eax, [ebx + device.init_block]
        invoke  GetPhysAddr
        push    eax
        and     eax, 0xffff
        mov     ecx, CSR_IAB0
        call    [ebx + device.write_csr]
        pop     eax
        shr     eax, 16
        mov     ecx, CSR_IAB1
        call    [ebx + device.write_csr]

        mov     ecx, CSR_TFEAT
        mov     eax, 0x0915                     ; Auto TX PAD ?
        call    [ebx + device.write_csr]

; Set the interrupt mask
        mov     ecx, CSR_IMR
        mov     eax, IMR
        call    [ebx + device.write_csr]

; Initialise the device
        xor     ecx, ecx
        mov     eax, CSR_INIT
        call    [ebx + device.write_csr]

        mov     esi, 100
;        xor     ecx, ecx
  @@:
        call    [ebx + device.read_csr]
        test    ax, CSR_IDONE
        jnz     @f

        dec     esi
        jnz     @r
        DEBUGF  2,"Initialize timeout!\n"
  @@:

; Start the device and enable interrupts
        xor     ecx, ecx
        mov     eax, CSR_START + CSR_INTEN
        call    [ebx + device.write_csr]

; Set the MTU
        mov     [ebx + device.mtu], 1514

        mov     [ebx + device.state], ETH_LINK_UNKNOWN
; Start media check timer
        cmp     [ebx + device.mii], 0
        je      @f
        mov     [ebx + device.state], ETH_LINK_DOWN
        invoke  TimerHS, 0, 50, check_media_mii, ebx
        mov     [ebx + device.link_timer], eax
  @@:

        DEBUGF  1,"reset complete\n"
        xor     eax, eax
  .fail:
        ret


align 4
init_ring:

        DEBUGF  1,"init ring\n"

        lea     edi, [ebx + device.rx_ring]
        mov     eax, edi
        invoke  GetPhysAddr
        mov     [ebx + device.init_block.rx_ring_phys], eax
        mov     ecx, RX_RING_SIZE
  .rx_init:
        push    ecx
        invoke  NetAlloc, PKT_BUF_SZ+NET_BUFF.data
        pop     ecx
        test    eax, eax
        jz      .out_of_mem
        mov     [edi + descriptor.virtual], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [edi + descriptor.base], eax
        mov     [edi + descriptor.length], - PKT_BUF_SZ
        mov     dword[edi + descriptor.msg_length], 0    ; also clears misc field
        mov     [edi + descriptor.status], RXSTAT_OWN
        add     edi, sizeof.descriptor
        dec     ecx
        jnz     .rx_init

        lea     edi, [ebx + device.tx_ring]
        mov     eax, edi
        invoke  GetPhysAddr
        mov     [ebx + device.init_block.tx_ring_phys], eax
        mov     ecx, TX_RING_SIZE
  .tx_init:
        mov     [edi + descriptor.status], 0
        add     edi, sizeof.descriptor
        dec     ecx
        jnz     .tx_init

        mov     [ebx + device.init_block.tlen_rlen], (TX_RING_LEN_BITS or RX_RING_LEN_BITS)

        mov     [ebx + device.cur_tx], 0
        mov     [ebx + device.last_tx], 0
        mov     [ebx + device.cur_rx], 0

        xor     eax, eax
        ret

  .out_of_mem:
        DEBUGF  2,"Out of memory!\n"

        or      eax, -1
        ret




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc transmit stdcall bufferptr

        spin_lock_irqsave

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .error
        cmp     [esi + NET_BUFF.length], 60
        jb      .error

; check descriptor
        lea     edi, [ebx + device.tx_ring]
        mov     ecx, [ebx + device.cur_tx]
        shl     ecx, 4
        add     edi, ecx

        test    [edi + descriptor.status], TXCTL_OWN
        jnz     .overrun
; descriptor is free, use it
        mov     [edi + descriptor.virtual], esi
        mov     eax, esi
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + descriptor.base], eax
; set length
        mov     eax, [esi + NET_BUFF.length]
        neg     eax
        mov     [edi + descriptor.length], ax
; put to transfer queue
        mov     [edi + descriptor.status], TXCTL_OWN + TXCTL_STP + TXCTL_ENP

; trigger an immediate send
        mov     edx, [ebx + device.io_addr]
        xor     ecx, ecx                        ; CSR0
        call    [ebx + device.read_csr]
        or      eax, CSR_TX
        call    [ebx + device.write_csr]

; get next descriptor
        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], TX_RING_SIZE - 1

; Update stats
        inc     [ebx + device.packets_tx]
        mov     eax, [esi + NET_BUFF.length]
        add     dword[ebx + device.bytes_tx], eax
        adc     dword[ebx + device.bytes_tx + 4], 0

        spin_unlock_irqrestore
        xor     eax, eax
        ret

  .error:
        DEBUGF  2, "TX packet error\n"
        inc     [ebx + device.packets_tx_err]
        invoke  NetFree, [bufferptr]

        spin_unlock_irqrestore
        or      eax, -1
        ret

  .overrun:
        DEBUGF  2, "TX overrun\n"
        inc     [ebx + device.packets_tx_ovr]
        invoke  NetFree, [bufferptr]

        spin_unlock_irqrestore
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

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]
        mov     edx, [ebx + device.io_addr]
        push    ecx
        xor     ecx, ecx                        ; CSR0
        call    [ebx + device.read_csr]         ; get IRQ reason
        call    [ebx + device.write_csr]        ; write it back to ACK
        pop     ecx
        test    ax, CSR_RINT or CSR_TINT
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
        DEBUGF  1,"Device: %x status: %x\n", ebx, eax:4

        push    ax
        test    ax, CSR_RINT
        jz      .not_receive

        push    ebx
  .rx_loop:
        pop     ebx
        push    ebx
        mov     eax, [ebx + device.cur_rx]
        shl     eax, 4
        lea     edi, [ebx + device.rx_ring]
        add     edi, eax                        ; edi now points to current rx ring entry

        mov     ax, [edi + descriptor.status]
        DEBUGF  1,"RX packet status: %x\n", eax:4

        test    ax, RXSTAT_OWN                  ; If this bit is set, the controller OWN's the packet, if not, we do
        jnz     .rx_done
; Both Start of packet and End of packet bits should be set, we dont support multi frame packets
        test    ax, RXSTAT_ENP
        jz      .rx_drop
        test    ax, RXSTAT_STP
        jz      .rx_drop

        movzx   ecx, [edi + descriptor.msg_length]      ; get packet length in ecx
        sub     ecx, 4                                  ; We dont need the CRC
        DEBUGF  1,"Got %u bytes\n", ecx

; Set pointers for ETH_input
        push    .rx_loop                                ; return address
        mov     eax, [edi + descriptor.virtual]
        push    eax                                     ; packet address
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

; Update stats
        add     dword[ebx + device.bytes_rx], ecx
        adc     dword[ebx + device.bytes_rx + 4], 0
        inc     [ebx + device.packets_rx]

; now allocate a new buffer
        invoke  NetAlloc, PKT_BUF_SZ+NET_BUFF.data      ; Allocate a buffer for the next packet
        test    eax, eax
        jz      .rx_overrun
        mov     [edi + descriptor.virtual], eax         ; set virtual address
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [edi + descriptor.base], eax            ; and physical address
        mov     [edi + descriptor.status], RXSTAT_OWN   ; give it back to PCnet controller

        inc     [ebx + device.cur_rx]                   ; set next receive descriptor
        and     [ebx + device.cur_rx], RX_RING_SIZE - 1

        jmp     [EthInput]

  .rx_overrun:
        add     esp, 4+4
        DEBUGF  2,"RX FIFO overrun\n"
        inc     [ebx + device.packets_rx_ovr]
        jmp     .rx_next

  .rx_drop:
        DEBUGF  2,"Dropping incoming packet\n"
        inc     [ebx + device.packets_rx_drop]

  .rx_next:
        mov     [edi + descriptor.status], RXSTAT_OWN   ; give it back to PCnet controller

        inc     [ebx + device.cur_rx]                   ; set next receive descriptor
        and     [ebx + device.cur_rx], RX_RING_SIZE - 1
        jmp     .rx_loop

  .rx_done:
        pop     ebx

  .not_receive:
        pop     ax

        test    ax, CSR_TINT
        jz      .not_transmit

  .tx_loop:
        lea     edi, [ebx + device.tx_ring]
        mov     eax, [ebx + device.last_tx]
        shl     eax, 4
        add     edi, eax

        test    [edi + descriptor.status], TXCTL_OWN
        jnz     .not_transmit

        mov     eax, [edi + descriptor.virtual]
        test    eax, eax
        jz      .not_transmit

        mov     [edi + descriptor.virtual], 0

        DEBUGF  1,"Removing packet %x from memory\n", eax

        invoke  NetFree, eax

        inc     [ebx + device.last_tx]
        and     [ebx + device.last_tx], TX_RING_SIZE - 1
        jmp     .tx_loop

  .not_transmit:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret




;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:      ; in: mac pushed onto stack (as 3 words)

        DEBUGF  1,"Writing MAC: %x-%x-%x-%x-%x-%x\n",\
        [esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

        mov     edx, [ebx + device.io_addr]
        add     dx, 2
        xor     eax, eax

        mov     ecx, CSR_PAR0
       @@:
        pop     ax
        call    [ebx + device.write_csr]
        inc     ecx
        cmp     ecx, CSR_PAR2
        jb      @r

; Notice this procedure does not ret, but continues to read_mac instead.

;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;
align 4
read_mac:
        DEBUGF  1,"Reading MAC\n"

        mov     edx, [ebx + device.io_addr]
        lea     edi, [ebx + device.mac]
        in      ax, dx
        stosw

        inc     dx
        inc     dx
        in      ax, dx
        stosw

        inc     dx
        inc     dx
        in      ax, dx
        stosw

        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,\
        [ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret

align 4
switch_to_wio:

        DEBUGF  1,"Switching to 16-bit mode\n"

        mov     [ebx + device.read_csr], wio_read_csr
        mov     [ebx + device.write_csr], wio_write_csr
        mov     [ebx + device.read_bcr], wio_read_bcr
        mov     [ebx + device.write_bcr], wio_write_bcr
        mov     [ebx + device.read_rap], wio_read_rap
        mov     [ebx + device.write_rap], wio_write_rap
        mov     [ebx + device.sw_reset], wio_reset

        ret

align 4
switch_to_dwio:

        DEBUGF  1,"Switching to 32-bit mode\n"

        mov     [ebx + device.read_csr], dwio_read_csr
        mov     [ebx + device.write_csr], dwio_write_csr
        mov     [ebx + device.read_bcr], dwio_read_bcr
        mov     [ebx + device.write_bcr], dwio_write_bcr
        mov     [ebx + device.read_rap], dwio_read_rap
        mov     [ebx + device.write_rap], dwio_write_rap
        mov     [ebx + device.sw_reset], dwio_reset

        ret


; ecx - index
; return:
; eax - data
align 4
wio_read_csr:

        add     edx, WIO_RAP
        mov     ax, cx
        out     dx, ax
        add     edx, WIO_RDP - WIO_RAP
        in      ax, dx
        and     eax, 0xffff
        sub     edx, WIO_RDP

        ret


; eax - data
; ecx - index
align 4
wio_write_csr:

        add     edx, WIO_RAP
        xchg    eax, ecx
        out     dx, ax
        xchg    eax, ecx
        add     edx, WIO_RDP - WIO_RAP
        out     dx, ax
        sub     edx, WIO_RDP

        ret


; ecx - index
; return:
; eax - data
align 4
wio_read_bcr:

        add     edx, WIO_RAP
        mov     ax, cx
        out     dx, ax
        add     edx, WIO_BDP - WIO_RAP
        in      ax, dx
        and     eax, 0xffff
        sub     edx, WIO_BDP

        ret


; eax - data
; ecx - index
align 4
wio_write_bcr:

        add     edx, WIO_RAP
        xchg    eax, ecx
        out     dx, ax
        xchg    eax, ecx
        add     edx, WIO_BDP - WIO_RAP
        out     dx, ax
        sub     edx, WIO_BDP

        ret

align 4
wio_read_rap:

        add     edx, WIO_RAP
        in      ax, dx
        and     eax, 0xffff
        sub     edx, WIO_RAP

        ret

; eax - val
align 4
wio_write_rap:

        add     edx, WIO_RAP
        out     dx, ax
        sub     edx, WIO_RAP

        ret

align 4
wio_reset:

        push    eax
        add     edx, WIO_RESET
        in      ax, dx
        pop     eax
        sub     edx, WIO_RESET

        ret



; ecx - index
; return:
; eax - data
align 4
dwio_read_csr:

        add     edx, DWIO_RAP
        mov     eax, ecx
        out     dx, eax
        add     edx, DWIO_RDP - DWIO_RAP
        in      eax, dx
        and     eax, 0xffff
        sub     edx, DWIO_RDP

        ret


; ecx - index
; eax - data
align 4
dwio_write_csr:

        add     edx, DWIO_RAP
        xchg    eax, ecx
        out     dx, eax
        add     edx, DWIO_RDP - DWIO_RAP
        xchg    eax, ecx
        out     dx, eax
        sub     edx, DWIO_RDP

        ret

; ecx - index
; return:
; eax - data
align 4
dwio_read_bcr:

        add     edx, DWIO_RAP
        mov     eax, ecx
        out     dx, eax
        add     edx, DWIO_BDP - DWIO_RAP
        in      eax, dx
        and     eax, 0xffff
        sub     edx, DWIO_BDP

        ret


; ecx - index
; eax - data
align 4
dwio_write_bcr:

        add     edx, DWIO_RAP
        xchg    eax, ecx
        out     dx, eax
        add     edx, DWIO_BDP - DWIO_RAP
        xchg    eax, ecx
        out     dx, eax
        sub     edx, DWIO_BDP

        ret

align 4
dwio_read_rap:

        add     edx, DWIO_RAP
        in      eax, dx
        and     eax, 0xffff
        sub     edx, DWIO_RAP

        ret


; eax - val
align 4
dwio_write_rap:

        add     edx, DWIO_RAP
        out     dx, eax
        sub     edx, DWIO_RAP

        ret

align 4
dwio_reset:

        push    eax
        add     edx, DWIO_RESET
        in      eax, dx
        pop     eax
        sub     edx, DWIO_RESET

        ret


align 4
mdio_read:

        and     ecx, 0x1f
        mov     ax, [ebx + device.phy]
        and     ax, 0x1f
        shl     ax, 5
        or      ax, cx

        mov     ecx, BCR_MIIADDR
        call    [ebx + device.write_bcr]

        mov     ecx, BCR_MIIDATA
        call    [ebx + device.read_bcr]

        ret


align 4
mdio_write:

        push    eax
        and     ecx, 0x1f
        mov     ax, [ebx + device.phy]
        and     ax, 0x1f
        shl     ax, 5
        or      ax, cx

        mov     ecx, BCR_MIIADDR
        call    [ebx + device.write_bcr]

        pop     eax
        mov     ecx, BCR_MIIDATA
        call    [ebx + device.write_bcr]

        ret




proc check_media_mii stdcall dev:dword

        spin_lock_irqsave

        mov     ebx, [dev]
        mov     edx, [ebx + device.io_addr]

        mov     ecx, MII_BMSR
        call    mdio_read

        mov     ecx, MII_BMSR
        call    mdio_read

        mov     ecx, eax
        and     eax, BMSR_LSTATUS
        shr     eax, 2
        cmp     eax, [ebx + device.state]
        jne     .changed

        spin_unlock_irqrestore
        ret

  .changed:
        test    eax, eax
        jz      .update

        test    ecx, BMSR_ANEGCOMPLETE
        jz      .update

        mov     ecx, MII_ADVERTISE
        call    mdio_read
        mov     esi, eax

        mov     ecx, MII_LPA
        call    mdio_read
        and     eax, esi

        test    eax, LPA_100FULL
        jz      @f
        mov     eax, ETH_LINK_SPEED_100M or ETH_LINK_FULL_DUPLEX
        jmp     .update
  @@:

        test    eax, LPA_100HALF
        jz      @f
        mov     eax, ETH_LINK_SPEED_100M
        jmp     .update
  @@:

        test    eax, LPA_10FULL
        jz      @f
        mov     eax, ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
        jmp     .update
  @@:

        test    eax, LPA_10HALF
        jz      @f
        mov     eax, ETH_LINK_SPEED_10M
        jmp     .update
  @@:

        mov     eax, ETH_LINK_UNKNOWN

  .update:
        mov     [ebx + device.state], eax
        invoke  NetLinkChanged


        spin_unlock_irqrestore
        ret


endp


; End of code


data fixups
end data

include '../peimport.inc'

my_service    db 'PCNET32',0            ; max 16 chars include zero

device_l2     db "PCnet/PCI 79C970",0
device_l4     db "PCnet/PCI II 79C970A",0
device_l5     db "PCnet/FAST 79C971",0
device_l6     db "PCnet/FAST+ 79C972",0
device_l7     db "PCnet/FAST III 79C973",0
device_l8     db "PCnet/Home 79C978",0
device_l9     db "PCnet/FAST III 79C975",0

options_mapping:
dd PORT_ASEL                            ; 0 Auto-select
dd PORT_AUI                             ; 1 BNC/AUI
dd PORT_AUI                             ; 2 AUI/BNC
dd PORT_ASEL                            ; 3 not supported
dd PORT_10BT or PORT_FD                 ; 4 10baseT-FD
dd PORT_ASEL                            ; 5 not supported
dd PORT_ASEL                            ; 6 not supported
dd PORT_ASEL                            ; 7 not supported
dd PORT_ASEL                            ; 8 not supported
dd PORT_MII                             ; 9 MII 10baseT
dd PORT_MII or PORT_FD                  ; 10 MII 10baseT-FD
dd PORT_MII                             ; 11 MII (autosel)
dd PORT_10BT                            ; 12 10BaseT
dd PORT_MII or PORT_100                 ; 13 MII 100BaseTx
dd PORT_MII or PORT_100 or PORT_FD      ; 14 MII 100BaseTx-FD
dd PORT_ASEL                            ; 15 not supported

include_debug_strings                                   ; All data wich FDO uses will be included here


align 4
devices     dd 0
device_list rd MAX_DEVICES                              ; This list contains all pointers to device structures the driver is handling
