;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                  ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.     ;;
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

format MS COFF

        API_VERSION             = 0x01000100

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2

        MAX_DEVICES             = 4
        MAX_ETH_FRAME_SIZE      = 1514

        TX_RING_SIZE            = 4
        RX_RING_SIZE            = 4

include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../netdrv.inc'

public START
public service_proc
public version


        PORT_AUI                = 0x00
        PORT_10BT               = 0x01
        PORT_GPSI               = 0x02
        PORT_MII                = 0x03
        PORT_PORTSEL            = 0x03
        PORT_ASEL               = 0x04
        PORT_100                = 0x40
        PORT_FD                 = 0x80

        DMA_MASK                = 0xffffffff

        LOG_TX_BUFFERS          = 2             ; FIXME
        LOG_RX_BUFFERS          = 2

        TX_RING_MOD_MASK        = (TX_RING_SIZE-1)
        TX_RING_LEN_BITS        = (LOG_TX_BUFFERS shl 12)

        RX_RING_MOD_MASK        = (RX_RING_SIZE-1)
        RX_RING_LEN_BITS        = (LOG_RX_BUFFERS shl 4)

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

        CSR_INIT                = 1 shl 0
        CSR_START               = 1 shl 1
        CSR_STOP                = 1 shl 2
        CSR_TX                  = 1 shl 3
        CSR_TXON                = 1 shl 4
        CSR_RXON                = 1 shl 5
        CSR_INTEN               = 1 shl 6
        CSR_INTR                = 1 shl 7
        CSR_IDONE               = 1 shl 8
        CSR_TINT                = 1 shl 9
        CSR_RINT                = 1 shl 10
        CSR_MERR                = 1 shl 11
        CSR_MISS                = 1 shl 12
        CSR_CERR                = 1 shl 13

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

        IMR                     = IMR_IDONE ; IMR_TINT + IMR_RINT + IMR_MERR + IMR_MISS ;+ IMR_IDONE

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

;

        MAX_PHYS                = 32


virtual at ebx

        device:

        ETH_DEVICE

; device specific

                        rb 0x100-(($ - device) and 0xff)        ;        align 256
        .private:
        .mode_          dw ?
        .tlen_rlen      dw ?
        .phys_addr      dp ?
        .reserved       dw ?
        .filter         dq ?
        .rx_ring_phys   dd ?
        .tx_ring_phys   dd ?

                        rb 0x100-(($ - device) and 0xff)        ;        align 256
        .rx_ring        rb RX_RING_SIZE * descriptor.size

                        rb 0x100-(($ - device) and 0xff)        ;        align 256
        .tx_ring        rb TX_RING_SIZE * descriptor.size

        .cur_rx         db ?
        .cur_tx         db ?
        .last_tx        db ?
        .options        dd ?
        .full_duplex    db ?
        .chip_version   dw ?
        .mii            db ?
        .ltint          db ?
        .dxsuflo        db ?
        .fset           db ?
        .fdx            db ?

        .io_addr        dd ?
        .irq_line       db ?
        .pci_bus        dd ?
        .pci_dev        dd ?

        .phy            dw ?

        .read_csr       dd ?
        .write_csr      dd ?
        .read_bcr       dd ?
        .write_bcr      dd ?
        .read_rap       dd ?
        .write_rap      dd ?
        .sw_reset       dd ?

        device_size     = $ - device

end virtual

struc   descriptor {
        .base           dd ?
        .length         dw ?
        .status         dw ?
        .msg_length     dw ?
        .misc           dw ?
        .virtual        dd ?

        .size:
}

virtual at 0
 descriptor descriptor
end virtual




section '.flat' code readable align 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START stdcall, state:dword

        cmp [state], 1
        jne .exit

  .entry:

        DEBUGF 1,"Loading %s driver\n", my_service
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

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

        mov     esi, device_list
;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[device.pci_bus]
        jne     @f
        cmp     ah, byte[device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice

; This device doesnt have its own eth_device structure yet, lets create one

  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, device_size, .fail

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.unload], unload
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device

        PCI_find_io

; We've found the io address, find IRQ now

        PCI_find_irq

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .destroy                                                ; If an error occured, exit

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

        dec     [devices]
  .err:
        DEBUGF  1,"Error, removing all data !\n"
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
        ; - Remove device from local list (RTL8139_LIST)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax,-1

ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is a PCnet device)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:

        mov     edx, [device.io_addr]

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
        DEBUGF 1,"PCnet device not found!\n"
        mov     eax, 1
        ret

  .L1:
        mov     ecx, CSR_CHIPID0
        call    [device.read_csr]

        mov     esi, eax
        shr     esi, 12

        and     ax, 0xfff
        cmp     ax, 3
        jne     .no_dev

        mov     ecx, CSR_CHIPID1
        call    [device.read_csr]
        shl     eax, 4
        or      eax, esi
        mov     [device.chip_version], ax

        mov     [device.fdx], 0
        mov     [device.mii], 0
        mov     [device.fset], 0
        mov     [device.dxsuflo], 0
        mov     [device.ltint], 0

        cmp     ax, 0x2420
        je      .L2
        cmp     ax, 0x2430
        je      .L2

        mov     [device.fdx], 1

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

        DEBUGF 1,"Invalid chip rev\n"
        jmp     .no_dev
  .L2:
        mov     [device.name], device_l2
        jmp     .L10
  .L4:
        mov     [device.name], device_l4
;        mov     [device.fdx], 1
        jmp     .L10
  .L5:
        mov     [device.name], device_l5
;        mov     [device.fdx], 1
        mov     [device.mii], 1
        mov     [device.fset], 1
        mov     [device.ltint], 1
        jmp     .L10
  .L6:
        mov     [device.name], device_l6
;        mov     [device.fdx], 1
        mov     [device.mii], 1
        mov     [device.fset], 1
        jmp     .L10
  .L7:
        mov     [device.name], device_l7
;        mov     [device.fdx], 1
        mov     [device.mii], 1
        jmp     .L10
  .L8:
        mov     [device.name], device_l8
;        mov     [device.fdx], 1
        mov     ecx, CSR_RXPOLL
        call    dword [device.read_bcr]
        call    dword [device.write_bcr]
        jmp     .L10
  .L9:
        mov     [device.name], device_l9
;        mov     [device.fdx], 1
        mov     [device.mii], 1
  .L10:
        DEBUGF 1,"device name: %s\n", [device.name]

        cmp     [device.fset], 1
        jne     .L11
        mov     ecx, BCR_BUSCTL
        call    [device.read_bcr]
        or      eax, 0x800
        call    [device.write_bcr]

        mov     ecx, CSR_DMACTL
        call    [device.read_csr]
;        and     eax, 0xc00
;        or      eax, 0xc00
        mov     eax, 0xc00
        call    [device.write_csr]

        mov     [device.dxsuflo],1
        mov     [device.ltint],1
  .L11:

        PCI_make_bus_master

        mov     [device.options], PORT_ASEL
        mov     [device.mode_], MODE_RXD + MODE_TXD     ; disable receive and transmit
        mov     [device.tlen_rlen], (TX_RING_LEN_BITS or RX_RING_LEN_BITS)

        mov     dword [device.filter], 0
        mov     dword [device.filter+4], 0

align 4
reset:

; attach int handler

        movzx   eax, [device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

        mov     edx, [device.io_addr]

        call    [device.sw_reset]

        ; Switch pcnet32 to 32bit mode
        mov     ecx, BCR_SSTYLE
        mov     eax, 2
        call    [device.write_bcr]

        ; set/reset autoselect bit
        mov     ecx, BCR_MISCCFG
        call    [device.read_bcr]
        and     eax, not 2
        test    [device.options], PORT_ASEL
        jz      @f
        or      eax, 2
  @@:
        call    [device.write_bcr]

        ; Handle full duplex setting
        cmp     byte [device.full_duplex], 0
        je      .duplex_ok
        mov     ecx, BCR_DUPLEX
        call    [device.read_bcr]
        and     eax, not 3
        test    [device.options], PORT_FD
        jz      @f
        or      eax, 1
        cmp     [device.options], PORT_FD or PORT_AUI
        jne     .set_duplex
        or      eax, 2
        jmp     .set_duplex
  @@:
        test    [device.options], PORT_ASEL
        jz      .set_duplex
        cmp     [device.chip_version], 0x2627
        jne     .set_duplex
        or      eax, 3
  .set_duplex:
        mov     ecx, BCR_DUPLEX
        call    [device.write_bcr]
  .duplex_ok:

        ; set/reset GPSI bit in test register
        mov     ecx, 124
        call    [device.read_csr]
        mov     ecx, [device.options]
        and     ecx, PORT_PORTSEL
        cmp     ecx, PORT_GPSI
        jne     @f
        or      eax, 0x10
  @@:
        call    [device.write_csr]
        cmp     [device.mii], 0
        je      .L6
        test    [device.options], PORT_ASEL
        jnz     .L6
        mov     ecx, BCR_MIICTL
        call    [device.read_bcr]
        and     eax, not 0x38
        test    [device.options], PORT_FD
        jz      @f
        or      eax, 0x10
  @@:
        test    [device.options], PORT_100
        jz      @f
        or      eax, 0x08
  @@:
        call    [device.write_bcr]
        jmp     .L9
  .L6:
        test    [device.options], PORT_ASEL
        jz      .L9
        mov     ecx, BCR_MIICTL
        DEBUGF 1,"ASEL, enable auto-negotiation\n"
        call    [device.read_bcr]
        and     eax, not 0x98
        or      eax, 0x20
        call    [device.write_bcr]
  .L9:
        cmp     [device.ltint], 0
        je      @f
        mov     ecx, 5
        call    [device.read_csr]
        or      eax, (1 shl 14)
        call    [device.write_csr]
  @@:
        mov     eax, [device.options]
        and     eax, PORT_PORTSEL
        shl     eax, 7
        mov     [device.mode_], ax
        mov     dword [device.filter], -1
        mov     dword [device.filter+4], -1



;-----------------------------

        test    [device.mii], 1
        jz      .no_mii

        mov     [device.phy], 0

  .mii_loop:
        mov     ecx, MII_PHYSID1
        call    mdio_read
        cmp     ax, 0xffff
        je      .next

        DEBUGF  1, "0x%x\n", ax

        mov     ecx, MII_PHYSID2
        call    mdio_read
        cmp     ax, 0xffff
        je      .next

        DEBUGF  1, "0x%x\n", ax

        jmp     .got_phy

        cmp     [device.phy], 31
        jne     .next
        mov     ax, [device.chip_version]
        inc     ax
        and     ax, 0xfffe
        cmp     ax, 0x2624              ; 79c971 & 79c972 have phantom phy at id 31
        je      .got_phy

  .next:
        inc     [device.phy]
        cmp     [device.phy], MAX_PHYS
        jb      .mii_loop

        DEBUGF  1, "No PHY found!\n"

        or      eax, -1
        ret

  .got_phy:
        DEBUGF  1, "Found PHY at 0x%x\n", [device.phy]:4

  .no_mii:

;-----------------------------------------------

        call    read_mac

        lea     esi, [device.mac]
        lea     edi, [device.phys_addr]
        movsd
        movsw

        call    init_ring

        mov     edx, [device.io_addr]   ; init ring destroys edx

        lea     eax, [device.private]
        GetRealAddr
        push    eax
        and     eax, 0xffff
        mov     ecx, 1
        call    [device.write_csr]
        pop     eax
        shr     eax, 16
        mov     ecx, 2
        call    [device.write_csr]

        mov     ecx, 4
        mov     eax, 0x0915
        call    [device.write_csr]

; Set the interrupt mask
        mov     ecx, CSR_IMR
        mov     eax, IMR
        call    [device.write_csr]

; Initialise the device
        xor     ecx, ecx
        mov     eax, CSR_INIT
        call    [device.write_csr]

        mov     esi, 100
;        xor     ecx, ecx
  @@:
        call    [device.read_csr]
        test    ax, CSR_IDONE
        jnz     @f

        dec     esi
        jnz     @r
        DEBUGF 1,"Initialize timeout!\n"
  @@:

; Start the device and enable interrupts
        xor     ecx, ecx
        mov     eax, CSR_START + CSR_INTEN
        call    [device.write_csr]

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], 1514

; get link status
        mov     [device.state], ETH_LINK_UNKOWN

        call    check_media

        DEBUGF 1,"reset complete\n"
        xor     eax, eax
        ret


align 4
init_ring:

        DEBUGF 1,"init ring\n"

        lea     edi, [device.rx_ring]
        mov     eax, edi
        GetRealAddr
        mov     [device.rx_ring_phys], eax
        mov     ecx, RX_RING_SIZE
  .rx_init:
        push    ecx
        stdcall KernelAlloc, PKT_BUF_SZ
        pop     ecx
        mov     [edi + descriptor.virtual], eax
        GetRealAddr
        mov     [edi + descriptor.base], eax
        mov     [edi + descriptor.length], - PKT_BUF_SZ
        mov     [edi + descriptor.status], RXSTAT_OWN
        mov     dword [edi + descriptor.msg_length], 0    ; also clears misc field
        add     edi, descriptor.size
        dec     ecx
        jnz     .rx_init

        lea     edi, [device.tx_ring]
        mov     eax, edi
        GetRealAddr
        mov     [device.tx_ring_phys], eax
        mov     ecx, TX_RING_SIZE
  .tx_init:
        mov     [edi + descriptor.status], 0
        add     edi, descriptor.size
        dec     ecx
        jnz     .tx_init

        mov     [device.tlen_rlen], (TX_RING_LEN_BITS or RX_RING_LEN_BITS)

        mov     [device.cur_tx], 0
        mov     [device.last_tx], 0
        mov     [device.cur_rx], 0

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
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
        mov     eax, [esp+4]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp+8], 1514
        ja      .nospace                        ; packet is too long
        cmp     dword [esp+8], 60
        jb      .nospace                        ; packet is too short

; check descriptor
        lea     edi, [device.tx_ring]
        movzx   eax, [device.cur_tx]
        shl     eax, 4
        add     edi, eax

        test    [edi + descriptor.status], TXCTL_OWN
        jnz     .nospace
; descriptor is free, use it
        mov     eax, [esp+4]
        mov     [edi + descriptor.virtual], eax
        GetRealAddr
        mov     [edi + descriptor.base], eax
; set length
        mov     eax, [esp+8]
        neg     eax
        mov     [edi + descriptor.length], ax
; put to transfer queue
        mov     [edi + descriptor.status], TXCTL_OWN + TXCTL_STP + TXCTL_ENP

; trigger an immediate send
        mov     edx, [device.io_addr]
        xor     ecx, ecx                        ; CSR0
        call    [device.read_csr]
        or      eax, CSR_TX
        call    [device.write_csr]

; get next descriptor 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, ...
        inc     [device.cur_tx]
        and     [device.cur_tx], TX_RING_SIZE - 1
        DEBUGF  2," - Packet Sent! "

; Update stats
        inc     [device.packets_tx]
        mov     eax, [esp+8]
        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx + 4], 0

  .finish:
        DEBUGF  2," - Done!\n"
        xor     eax, eax
        ret     8

  .nospace:
        DEBUGF  1, 'ERROR: no free transmit descriptors\n'
        stdcall KernelFree, [esp+4]
        or      eax, -1
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

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        mov     edx, [device.io_addr]
        push    ecx
        xor     ecx, ecx                        ; CSR0
        call    [device.read_csr]               ; get IRQ reason
        call    [device.write_csr]              ; write it back to ACK
        pop     ecx
        and     ax, CSR_RINT or CSR_TINT
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
        DEBUGF  1,"Device: %x status: %x\n", ebx, eax:2

        push    ax
        test    ax, CSR_RINT
        jz      .not_receive

        push    ebx
  .rx_loop:
        pop     ebx
        movzx   eax, [device.cur_rx]
        shl     eax, 4
        lea     edi, [device.rx_ring]
        add     edi, eax                        ; edi now points to current rx ring entry

        mov     ax, [edi + descriptor.status]
        DEBUGF  1,"RX packet status: %x\n", eax:4

        test    ax, RXSTAT_OWN                  ; If this bit is set, the controller OWN's the packet, if not, we do
        jnz     .not_receive

        test    ax, RXSTAT_ENP
        jz      .not_receive

        test    ax, RXSTAT_STP
        jz      .not_receive

        movzx   ecx, [edi + descriptor.msg_length]      ; get packet length in ecx
        sub     ecx, 4                                  ;

; Set pointers for ETH_input
        push    ebx

        push    .rx_loop                                ; return address
        push    ecx                                     ; packet size
        push    [edi + descriptor.virtual]              ; packet address

; Update stats
        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0
        inc     [device.packets_rx]

; now allocate a new buffer
        stdcall KernelAlloc, PKT_BUF_SZ                 ; Allocate a buffer for the next packet
        mov     [edi + descriptor.virtual], eax         ; set virtual address
        GetRealAddr
        mov     [edi + descriptor.base], eax            ; and real address

;        mov     word [edi + descriptor.length], - PKT_BUF_SZ
        mov     [edi + descriptor.status], RXSTAT_OWN   ; give it back to PCnet controller

        inc     [device.cur_rx]                         ; set next receive descriptor
        and     [device.cur_rx], RX_RING_SIZE - 1

        jmp     Eth_input

  .not_receive:
        pop     ax

        test    ax, CSR_TINT
        jz      .not_transmit

  .tx_loop:
        lea     edi, [device.tx_ring]
        movzx   eax, [device.last_tx]
        shl     eax, 4
        add     edi, eax

        test    [edi + descriptor.status], TXCTL_OWN
        jnz     .not_transmit

        mov     eax, [edi + descriptor.virtual]
        test    eax, eax
        jz      .not_transmit

        mov     [edi + descriptor.virtual], 0

        DEBUGF  1,"Removing packet %x from memory\n", eax

        stdcall KernelFree, eax

        inc     [device.last_tx]
        and     [device.last_tx], TX_RING_SIZE - 1
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

        DEBUGF  1,"Writing MAC: %x-%x-%x-%x-%x-%x",[esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

        mov     edx, [device.io_addr]
        add     dx, 2
        xor     eax, eax

        mov     ecx, CSR_PAR0
       @@:
        pop     ax
        call    [device.write_csr]
        DEBUGF  1,"."
        inc     ecx
        cmp     ecx, CSR_PAR2
        jb      @r

        DEBUGF  1,"\n"

; Notice this procedure does not ret, but continues to read_mac instead.

;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;
align 4
read_mac:
        DEBUGF  1,"Reading MAC"

        mov     edx, [device.io_addr]
        add     dx, 6
       @@:
        dec     dx
        dec     dx
        in      ax, dx
        push    ax
        DEBUGF  1,"."
        cmp     edx, [device.io_addr]
        ja      @r

        DEBUGF  1," %x-%x-%x-%x-%x-%x\n",[esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

        lea     edi, [device.mac]
        pop     ax
        stosw
        pop     ax
        stosw
        pop     ax
        stosw

        ret

align 4
switch_to_wio:

        DEBUGF  1,"Switching to 16-bit mode\n"

        mov     [device.read_csr], wio_read_csr
        mov     [device.write_csr], wio_write_csr
        mov     [device.read_bcr], wio_read_bcr
        mov     [device.write_bcr], wio_write_bcr
        mov     [device.read_rap], wio_read_rap
        mov     [device.write_rap], wio_write_rap
        mov     [device.sw_reset], wio_reset

        ret

align 4
switch_to_dwio:

        DEBUGF  1,"Switching to 32-bit mode\n"

        mov     [device.read_csr], dwio_read_csr
        mov     [device.write_csr], dwio_write_csr
        mov     [device.read_bcr], dwio_read_bcr
        mov     [device.write_bcr], dwio_write_bcr
        mov     [device.read_rap], dwio_read_rap
        mov     [device.write_rap], dwio_write_rap
        mov     [device.sw_reset], dwio_reset

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
        mov     ax, [device.phy]
        and     ax, 0x1f
        shl     ax, 5
        or      ax, cx

        mov     ecx, BCR_MIIADDR
        call    [device.write_bcr]

        mov     ecx, BCR_MIIDATA
        call    [device.read_bcr]

        ret


align 4
mdio_write:

        push    eax
        and     ecx, 0x1f
        mov     ax, [device.phy]
        and     ax, 0x1f
        shl     ax, 5
        or      ax, cx

        mov     ecx, BCR_MIIADDR
        call    [device.write_bcr]

        pop     eax
        mov     ecx, BCR_MIIDATA
        call    [device.write_bcr]

        ret


align 4
check_media:

        DEBUGF  1, "check_media\n"

        test    [device.mii], 1
        jnz      mii_link_ok

        mov     ecx, BCR_LED0
        call    [device.read_bcr]
        cmp     eax, 0xc0

        DEBUGF  1, "link status=0x%x\n", ax

        ret



; End of code

align 4                                       ; Place all initialised data here

devices     dd 0
version       dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'PCnet',0                    ; max 16 chars include zero

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

section '.data' data readable writable align 16         ; place all uninitialized data place here

device_list rd MAX_DEVICES                              ; This list contains all pointers to device structures the driver is handling
