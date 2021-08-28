;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Realtek 8139 driver for KolibriOS                              ;;
;;                                                                 ;;
;;  based on RTL8139.asm driver for menuetos                       ;;
;;  and realtek8139.asm for SolarOS by Eugen Brasoveanu            ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; TODO: test for RX-overrun

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

; configureable area

        MAX_DEVICES             = 16

        RBLEN                   = 3     ; Receive buffer size: 0==8K 1==16k 2==32k 3==64k

        TXRR                    = 8     ; total retries = 16+(TXRR*16)
        TX_MXDMA                = 6     ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=2048
        ERTXTH                  = 8     ; in unit of 32 bytes e.g:(8*32)=256
        RX_MXDMA                = 7     ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=unlimited
        RXFTH                   = 7     ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=no threshold

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

; end configureable area

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

        REG_IDR0                = 0x00
        REG_MAR0                = 0x08 ; multicast filter register 0
        REG_MAR4                = 0x0c ; multicast filter register 4
        REG_TSD0                = 0x10 ; transmit status of descriptor
        REG_TSAD0               = 0x20 ; transmit start address of descriptor
        REG_RBSTART             = 0x30 ; RxBuffer start address
        REG_COMMAND             = 0x37 ; command register
        REG_CAPR                = 0x38 ; current address of packet read (word) R/W
        REG_IMR                 = 0x3c ; interrupt mask register
        REG_ISR                 = 0x3e ; interrupt status register
        REG_TXCONFIG            = 0x40 ; transmit configuration register
        REG_RXCONFIG            = 0x44 ; receive configuration register 0
        REG_MPC                 = 0x4c ; missed packet counter
        REG_9346CR              = 0x50 ; serial eeprom 93C46 command register
        REG_CONFIG1             = 0x52 ; configuration register 1
        REG_MSR                 = 0x58 ; Media Status register
        REG_CONFIG4             = 0x5a ; configuration register 4
        REG_HLTCLK              = 0x5b ; undocumented halt clock register
        REG_BMCR                = 0x62 ; basic mode control register
        REG_ANAR                = 0x66 ; auto negotiation advertisement register
        REG_9346CR_WE           = 11b shl 6

        BIT_RUNT                = 4 ; total packet length < 64 bytes
        BIT_LONG                = 3 ; total packet length > 4k
        BIT_CRC                 = 2 ; crc error occured
        BIT_FAE                 = 1 ; frame alignment error occured
        BIT_ROK                 = 0 ; received packet is ok

        BIT_RST                 = 4 ; reset bit
        BIT_RE                  = 3 ; receiver enabled
        BIT_TE                  = 2 ; transmitter enabled
        BUFE                    = 1 ; rx buffer is empty, no packet stored

        BIT_ISR_TOK             = 2 ; transmit ok
        BIT_ISR_RER             = 1 ; receive error interrupt
        BIT_ISR_ROK             = 0 ; receive ok

        BIT_TX_MXDMA            = 8 ; Max DMA burst size per Tx DMA burst
        BIT_TXRR                = 4 ; Tx Retry count 16+(TXRR*16)

        BIT_RXFTH               = 13 ; Rx fifo threshold
        BIT_RBLEN               = 11 ; Ring buffer length indicator
        BIT_RX_MXDMA            = 8 ; Max DMA burst size per Rx DMA burst
        BIT_NOWRAP              = 7 ; transfered data wrapping
        BIT_9356SEL             = 6 ; eeprom selector 9346/9356
        BIT_AER                 = 5 ; accept error packets
        BIT_AR                  = 4 ; accept runt packets
        BIT_AB                  = 3 ; accept broadcast packets
        BIT_AM                  = 2 ; accept multicast packets
        BIT_APM                 = 1 ; accept physical match packets
        BIT_AAP                 = 0 ; accept all packets

        BIT_93C46_EEM1          = 7 ; RTL8139 eeprom operating mode1
        BIT_93C46_EEM0          = 6 ; RTL8139 eeprom operating mode0
        BIT_93C46_EECS          = 3 ; chip select
        BIT_93C46_EESK          = 2 ; serial data clock
        BIT_93C46_EEDI          = 1 ; serial data input
        BIT_93C46_EEDO          = 0 ; serial data output

        BIT_LWACT               = 4 ; see REG_CONFIG1
        BIT_SLEEP               = 1 ; sleep bit at older chips
        BIT_PWRDWN              = 0 ; power down bit at older chips
        BIT_PMEn                = 0 ; power management enabled

        BIT_LWPTN               = 2 ; see REG_CONFIG4

        BIT_ERTXTH              = 16 ; early TX threshold
        BIT_TOK                 = 15 ; transmit ok
        BIT_OWN                 = 13 ; tx DMA operation is completed

        BIT_ANE                 = 12 ; auto negotiation enable

        BIT_TXFD                = 8 ; 100base-T full duplex
        BIT_TX                  = 7 ; 100base-T
        BIT_10FD                = 6 ; 10base-T full duplex
        BIT_10                  = 5 ; 10base-T
        BIT_SELECTOR            = 0 ; binary encoded selector CSMA/CD=00001

        BIT_IFG1                = 1 shl 25
        BIT_IFG0                = 1 shl 24

        RX_CONFIG               = (RBLEN shl BIT_RBLEN) or \
                                  (RX_MXDMA shl BIT_RX_MXDMA) or \
                                  (1 shl BIT_NOWRAP) or \
                                  (RXFTH shl BIT_RXFTH) or\
                                  (1 shl BIT_AB) or \                   ; Accept broadcast packets
                                  (1 shl BIT_APM) or \                  ; Accept physical match packets
                                  (1 shl BIT_AER) or \                  ; Accept error packets
                                  (1 shl BIT_AR) or \                   ; Accept Runt packets (smaller then 64 bytes)
                                  (1 shl BIT_AM)                        ; Accept multicast packets

        RX_BUFFER_SIZE          = (8192 shl RBLEN);+16+1500
        NUM_TX_DESC             = 4                                     ; not user selectable

        EE_93C46_REG_ETH_ID     = 7 ; MAC offset
        EE_93C46_READ_CMD       = (6 shl 6) ; 110b + 6bit address
        EE_93C56_READ_CMD       = (6 shl 8) ; 110b + 8bit address
        EE_93C46_CMD_LENGTH     = 9  ; start bit + cmd + 6bit address
        EE_93C56_CMD_LENGTH     = 11 ; start bit + cmd + 8bit ddress

; See chapter "5.7 Transmit Configuration Register" of RTL8139D(L).pdf
        VER_RTL8139             = 1000000b
        VER_RTL8139_K           = 1100000b
        VER_RTL8139A            = 1110000b
        VER_RTL8139A_G          = 1110010b
        VER_RTL8139B            = 1111000b
        VER_RTL8130             = 1111100b
        VER_RTL8139C            = 1110100b
        VER_RTL8100             = 1111010b
        VER_RTL8100_8139D       = 1110101b
        VER_RTL8139CP           = 1110110b
        VER_RTL8101             = 1110111b

        IDX_UNKNOWN             = 0
        IDX_RTL8139             = 1
        IDX_RTL8139_K           = 2
        IDX_RTL8139A            = 3
        IDX_RTL8139A_G          = 4
        IDX_RTL8139B            = 5
        IDX_RTL8130             = 6
        IDX_RTL8139C            = 7
        IDX_RTL8100             = 8
        IDX_RTL8100_8139D       = 9
        IDX_RTL8139CP           = 10
        IDX_RTL8101             = 11

        HW_VERSIONS             = 11

        ISR_SERR                = 1 shl 15
        ISR_TIMEOUT             = 1 shl 14
        ISR_LENCHG              = 1 shl 13
        ISR_FIFOOVW             = 1 shl 6
        ISR_PUN                 = 1 shl 5
        ISR_RXOVW               = 1 shl 4
        ISR_TER                 = 1 shl 3
        ISR_TOK                 = 1 shl 2
        ISR_RER                 = 1 shl 1
        ISR_ROK                 = 1 shl 0

        INTERRUPT_MASK          = ISR_ROK or \
                                  ISR_RER or \
                                  ISR_TOK or \
                                  ISR_TER or \
                                  ISR_RXOVW or \
                                  ISR_PUN or \
                                  ISR_FIFOOVW or \
                                  ISR_LENCHG or \
                                  ISR_TIMEOUT or \
                                  ISR_SERR

        TSR_OWN                 = 1 shl 13
        TSR_TUN                 = 1 shl 14
        TSR_TOK                 = 1 shl 15

        TSR_CDH                 = 1 shl 28
        TSR_OWC                 = 1 shl 29
        TSR_TABT                = 1 shl 30
        TSR_CRS                 = 1 shl 31


struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3    ; align 4

        rx_buffer       dd ?
        rx_data_offset  dd ?
        curr_tx_desc    db ?
        hw_ver_id       db ?
                        rb 2    ; align 4

        TX_DESC         rd NUM_TX_DESC

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

        mov     ax, [eax+1]                             ; get the pci bus and device numbers
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

        DEBUGF  1, "Hooking into device, devfn:%x, bus:%x, irq:%x, I/O addr:%x\n",\
        [ebx + device.pci_dev]:2,[ebx + device.pci_bus]:2,[ebx + device.irq_line]:2,[ebx + device.io_addr]:4

; Allocate the receive buffer

        invoke  CreateRingBuffer, dword (RX_BUFFER_SIZE), dword PG_SW
        test    eax, eax
        jz      .err
        mov     [ebx + device.rx_buffer], eax

; Ok, the eth_device structure is ready, let's probe the device

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    reset
        test    eax, eax
        jnz     .destroy

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1, "Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1, "Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)
  .destroy:
        ; unregister device from device_list
        mov     eax, [devices]
        mov     dword[device_list-4+4*eax], 0
        dec     [devices]

  .err:
        DEBUGF  2, "Fatal error occured, aborting\n"
        invoke  KernelFree, [ebx + device.rx_buffer]
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
        ; - Detach int handler
        ; - Remove device from local list (RTL8139_LIST)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax, -1
        ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is RTL8139)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
        DEBUGF  1, "Probing\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER or PCI_CMD_PIO
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; wake up old chips
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_HLTCLK
        mov     al, 'R'         ; run the clock
        out     dx, al

; get chip version
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_TXCONFIG + 2
        in      ax, dx
        shr     ah, 2
        shr     ax, 6
        and     al, 0x7f
        DEBUGF  1, "Chip version: %x\n", eax:2

; now find it in our array
        mov     ecx, HW_VERSIONS
  @@:
        cmp     al, [hw_ver_array + ecx]
        je      @f
        dec     ecx
        jnz     @r
  @@:
        mov     [ebx + device.hw_ver_id], cl
        mov     ecx, [hw_ver_names+ecx*4]
        mov     [ebx + device.name], ecx
        DEBUGF  1, "Chip version: %s\n", ecx

        cmp     [ebx + device.hw_ver_id], IDX_RTL8139B
        jae     .new_chip

; wake up older chips
  .old_chip:
        DEBUGF  1, "Wake up chip old style\n"
        set_io  [ebx + device.io_addr], REG_CONFIG1
        in      al, dx
        and     al, not ((1 shl BIT_SLEEP) or (1 shl BIT_PWRDWN))
        out     dx, al
        jmp     .done

; set LWAKE pin to active high (default value).
; it is for Wake-On-LAN functionality of some motherboards.
; this signal is used to inform the motherboard to execute a wake-up process.
; only at newer chips.
  .new_chip:
        DEBUGF  1, "Wake up chip new style\n"
; unlock config and BMCR registers
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_9346CR
        mov     al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
        out     dx, al

;
        set_io  [ebx + device.io_addr], REG_CONFIG1
        in      al, dx
        or      al, (1 shl BIT_PMEn)
        and     al, not (1 shl BIT_LWACT)
        out     dx, al

;
        set_io  [ebx + device.io_addr], REG_CONFIG4
        in      al, dx
        and     al, not (1 shl BIT_LWPTN)
        out     dx, al

; lock config and BMCR registers
        xor     al, al
        set_io  [ebx + device.io_addr], REG_9346CR
        out     dx, al

  .done:
        DEBUGF  1, "probing done!\n"
        xor     eax, eax
        ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   reset: Set up all registers and descriptors, clear some values
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

reset:
        DEBUGF  1, "Reset\n"

; reset chip
        DEBUGF  1, "Resetting chip\n"
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        mov     al, 1 shl BIT_RST
        out     dx, al
        mov     cx, 1000                ; wait no longer for the reset
  @@:
        in      al, dx
        test    al, 1 shl BIT_RST
        jz      @f                      ; RST remains 1 during reset
        dec     cx
        jnz     @r
        DEBUGF  2, "Reset timeout!\n"
        or      eax, -1
        ret
  @@:

; Read MAC address
        call    read_mac

; attach int handler
        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1, "Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2, "Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

; unlock config and BMCR registers
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_9346CR
        mov     al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
        out     dx, al

; initialize multicast registers (no filtering)
        mov     eax, 0xffffffff
        set_io  [ebx + device.io_addr], REG_MAR0
        out     dx, eax
        set_io  [ebx + device.io_addr], REG_MAR4
        out     dx, eax

; enable Rx/Tx
        mov     al, (1 shl BIT_RE) or (1 shl BIT_TE)
        set_io  [ebx + device.io_addr], REG_COMMAND
        out     dx, al

; Rxbuffer size, unlimited dma burst, no wrapping, no rx threshold
; accept broadcast packets, accept physical match packets
        mov     eax, RX_CONFIG
        set_io  [ebx + device.io_addr], REG_RXCONFIG
        out     dx, eax

; 1024 bytes DMA burst, total retries = 16 + 8 * 16 = 144
        mov     eax, (TX_MXDMA shl BIT_TX_MXDMA) or (TXRR shl BIT_TXRR) or BIT_IFG1 or BIT_IFG0
        set_io  [ebx + device.io_addr], REG_TXCONFIG
        out     dx, eax

; enable auto negotiation
        set_io  [ebx + device.io_addr], REG_BMCR
        in      ax, dx
        or      ax, (1 shl BIT_ANE)
        out     dx, ax

; set auto negotiation advertisement
        set_io  [ebx + device.io_addr], REG_ANAR
        in      ax, dx
        or      ax, (1 shl BIT_SELECTOR) or (1 shl BIT_10) or (1 shl BIT_10FD) or (1 shl BIT_TX) or (1 shl BIT_TXFD)
        out     dx, ax

; lock config and BMCR registers
        xor     eax, eax
        set_io  [ebx + device.io_addr], REG_9346CR
        out     dx, al

; init RX/TX pointers
        mov     [ebx + device.rx_data_offset], eax
        mov     [ebx + device.curr_tx_desc], al
;        set_io  [ebx + device.io_addr], REG_CAPR
;        out     dx, ax

; clear packet/byte counters
        lea     edi, [ebx + device.bytes_tx]
        mov     ecx, 6
        rep     stosd

; clear missing packet counter
        set_io  [ebx + device.io_addr], REG_MPC
        out     dx, eax

; set RxBuffer address, init RX buffer offset
        mov     eax, [ebx + device.rx_buffer]
        mov     dword[eax], 0                   ; clear receive flags for first packet (really needed??)
        DEBUGF  1, "RX buffer virtual addr=0x%x\n", eax
        invoke  GetPhysAddr
        DEBUGF  1, "RX buffer physical addr=0x%x\n", eax
        set_io  [ebx + device.io_addr], REG_RBSTART
        out     dx, eax

; enable interrupts
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_IMR
        mov     ax, INTERRUPT_MASK
        out     dx, ax

; Set the mtu, kernel will be able to send now
        mov     [ebx + device.mtu], 1514

; Detect current link status
        call    link

; Indicate that we have successfully reset the card
        xor     eax, eax
        ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: pointer to device structure in ebx  ;;
;; Out: eax = 0 on success                 ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 16
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

; check if we own the current discriptor
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_TSD0
        movzx   ecx, [ebx + device.curr_tx_desc]
        shl     ecx, 2
        add     edx, ecx
        in      eax, dx
        test    eax, (1 shl BIT_OWN)
        jz      .overrun

; Set the buffer address
        set_io  [ebx + device.io_addr], REG_TSAD0
        mov     [ebx + device.TX_DESC+ecx], esi
        mov     eax, esi
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        out     dx, eax

; And the size of the buffer
        set_io  [ebx + device.io_addr], REG_TSD0
        mov     eax, [esi + NET_BUFF.length]
        or      eax, (ERTXTH shl BIT_ERTXTH)    ; Early threshold
        out     dx, eax

; get next descriptor
        inc     [ebx + device.curr_tx_desc]
        and     [ebx + device.curr_tx_desc], NUM_TX_DESC-1

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
align 16
int_handler:

        push    ebx esi edi

        mov     ebx, [esp+4*4]
        DEBUGF  1,"INT for 0x%x\n", ebx

; TODO? if we are paranoid, we can check that the value from ebx is present in the current device_list

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_ISR
        in      ax, dx                          ; Get interrupt status
        test    ax, ax
        jz      .nothing

        out     dx, ax                          ; ACK interrupt
        DEBUGF  1, "Status: %x\n", ax

;----------------------------------------------------
; Received packet ok?

        test    ax, ISR_ROK
        jz      @f
        push    ax

  .receive:
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_COMMAND
        in      al, dx
        test    al, BUFE                        ; test if RX buffer is empty
        jnz     .finish

        DEBUGF  1, "RX:\n"

        mov     eax, [ebx + device.rx_buffer]
        add     eax, [ebx + device.rx_data_offset]
        test    byte [eax], (1 shl BIT_ROK)     ; check if packet is ok
        jz      .reset_rx

; packet is ok, copy it
        movzx   ecx, word [eax+2]               ; packet length
        sub     cx, 4                           ; don't copy CRC

; Update stats
        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0
        inc     [ebx + device.packets_rx]

        DEBUGF  1, "Received %u bytes\n", ecx

        push    ebx eax ecx
        add     ecx, NET_BUFF.data
        invoke  NetAlloc, ecx                   ; Allocate a buffer to put packet into
        pop     ecx
        test    eax, eax                        ; Test if we allocated succesfully
        jz      .abort
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

        lea     edi, [eax + NET_BUFF.data]      ; Where we will copy too
        mov     esi, [esp]                      ; The buffer we will copy from
        add     esi, 4                          ; Dont copy CRC

        push    .abort                          ; return addr for Eth_input
        push    eax                             ; buffer ptr for Eth_input

  .copy:
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        jz      .nd
        rep     movsd
  .nd:

        jmp     [EthInput]                      ; Send it to kernel

  .abort:
        pop     eax ebx
                                                ; update eth_data_start_offset
        movzx   eax, word [eax+2]               ; packet length
        add     eax, [ebx + device.rx_data_offset]
        add     eax, 4+3                        ; packet header is 4 bytes long + dword alignment
        and     eax, not 3                      ; dword alignment

        cmp     eax, RX_BUFFER_SIZE
        jb      .no_wrap
        DEBUGF  1, "Wrapping\n"
        sub     eax, RX_BUFFER_SIZE
  .no_wrap:
        mov     [ebx + device.rx_data_offset], eax
        DEBUGF  1, "New RX ptr: %d\n", eax

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_CAPR                        ; update 'Current Address of Packet Read register'
        sub     eax, 0x10                       ; value 0x10 is a constant for CAPR
        out     dx, ax

        jmp     .receive                        ; check for multiple packets

  .reset_rx:
        test    byte [eax], (1 shl BIT_CRC)
        jz      .no_crc_error
        DEBUGF  2, "RX: CRC error!\n"

  .no_crc_error:
        test    byte [eax], (1 shl BIT_FAE)
        jz      .no_fae_error
        DEBUGF  2, "RX: Frame alignment error!\n"

  .no_fae_error:
        DEBUGF  1, "Reset RX\n"
        in      al, dx                          ; read command register
        push    ax
        and     al, not (1 shl BIT_RE)          ; Clear the RE bit
        out     dx, al
        pop     ax
        out     dx, al                          ; write original command back

        add     edx, REG_RXCONFIG - REG_COMMAND         ; Restore RX configuration
        mov     ax, RX_CONFIG
        out     dx, ax

  .finish:
        pop     ax
  @@:

;----------------------------------------------------
; Transmit ok / Transmit error
        test    ax, ISR_TOK + ISR_TER
        jz      @f

        DEBUGF  1, "Transmit done!\n"

        push    ax
        mov     ecx, (NUM_TX_DESC-1)*4
  .txdescloop:
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_TSD0
        add     edx, ecx
        in      eax, dx

        test    eax, TSR_OWN                    ; DMA operation completed
        jz      .notthisone

        cmp     [ebx + device.TX_DESC+ecx], 0
        je      .notthisone

        DEBUGF  1, "TSD: 0x%x\n", eax

        test    eax, TSR_TUN
        jz      .no_bun
        DEBUGF  2, "TX: FIFO Buffer underrun!\n"

  .no_bun:
        test    eax, TSR_OWC
        jz      .no_owc
        DEBUGF  2, "TX: OWC!\n"

  .no_owc:
        test    eax, TSR_TABT
        jz      .no_tabt
        DEBUGF  2, "TX: TABT!\n"

  .no_tabt:
        test    eax, TSR_CRS
        jz      .no_csl
        DEBUGF  2, "TX: Carrier Sense Lost!\n"

  .no_csl:
        test    eax, TSR_TOK
        jz      .no_tok
        DEBUGF  1, "TX: Transmit OK!\n"

  .no_tok:
        DEBUGF  1, "free transmit buffer 0x%x\n", [ebx + device.TX_DESC+ecx]:8
        push    ecx ebx
        invoke  NetFree, [ebx + device.TX_DESC+ecx]
        pop     ebx ecx
        mov     [ebx + device.TX_DESC+ecx], 0

  .notthisone:
        sub     ecx, 4
        jae     .txdescloop
        pop     ax
  @@:

;----------------------------------------------------
; Rx buffer overflow ?
        test    ax, ISR_RXOVW
        jz      @f

        push    ax
        DEBUGF  2, "RX:buffer overflow!\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_ISR
        mov     ax, ISR_FIFOOVW or ISR_RXOVW or ISR_ROK
        out     dx, ax
        pop     ax
  @@:

;----------------------------------------------------
; Packet underrun?
        test    ax, ISR_PUN
        jz      @f

        DEBUGF  1, "Packet underrun or link changed!\n"

        call    link
  @@:

;----------------------------------------------------
; Receive FIFO overflow ?
        test    ax, ISR_FIFOOVW
        jz      @f

        push    ax
        DEBUGF  2, "RX fifo overflow!\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_ISR
        mov     ax, ISR_FIFOOVW or ISR_RXOVW or ISR_ROK
        out     dx, ax
        pop     ax
  @@:

;----------------------------------------------------
; cable length changed ?
        test    ax, ISR_LENCHG
        jz      @f

        DEBUGF  2, "Cable length changed!\n"

        call    link
  @@:

        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret

  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Check link status ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
link:
        DEBUGF  1, "Checking link status:\n"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_MSR
        in      ax, dx

        test    al, 1 shl 2             ; 0 = link ok 1 = link fail
        jnz     .notconnected

        mov     ecx, ETH_LINK_SPEED_10M
        test    al, 1 shl 3             ; 0 = 100 Mbps 1 = 10 Mbps
        jnz     @f
        mov     ecx, ETH_LINK_SPEED_100M
  @@:

        set_io  [ebx + device.io_addr], REG_BMCR
        in      ax, dx
        test    ax, 1 shl 8             ; Duplex mode
        jz      @f
        or      ecx, ETH_LINK_FULL_DUPLEX
  @@:

        mov     [ebx + device.state], ecx
        invoke  NetLinkChanged
        DEBUGF  2, "link is up\n"
        ret

  .notconnected:
        mov     [ebx + device.state], ETH_LINK_DOWN
        invoke  NetLinkChanged
        DEBUGF  2, "link is down\n"
        ret



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:      ; in: mac pushed onto stack (as 3 words)

        DEBUGF  1, "Writing MAC\n"

; disable all in command registers
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_9346CR
        xor     eax, eax
        out     dx, al

        set_io  [ebx + device.io_addr], REG_IMR
        xor     eax, eax
        out     dx, ax

        set_io  [ebx + device.io_addr], REG_ISR
        mov     eax, -1
        out     dx, ax

; enable writing
        set_io  [ebx + device.io_addr], REG_9346CR
        mov     eax, REG_9346CR_WE
        out     dx, al

 ; write the mac ...
        set_io  [ebx + device.io_addr], REG_IDR0
        pop     eax
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_IDR0+4
        xor     eax, eax
        pop     ax
        out     dx, eax

; disable writing
        set_io  [ebx + device.io_addr], REG_9346CR
        xor     eax, eax
        out     dx, al

        DEBUGF  1, "MAC write ok!\n"

; Notice this procedure does not ret, but continues to read_mac instead.


;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;

read_mac:
        DEBUGF  1, "Reading MAC:\n"

        set_io  [ebx + device.io_addr], 0
        lea     edi, [ebx + device.mac]
        in      eax, dx
        stosd
        add     edx, 4
        in      ax, dx
        stosw

        DEBUGF  1, "%x-%x-%x-%x-%x-%x\n",[edi-6]:2,[edi-5]:2,[edi-4]:2,[edi-3]:2,[edi-2]:2,[edi-1]:2

        ret


; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'RTL8139',0                    ; max 16 chars include zero

sz_unknown              db 'Unknown RTL8139 clone', 0
sz_RTL8139              db 'Realtek 8139',0
sz_RTL8139_K            db 'Realtek 8139 rev K',0
sz_RTL8139A             db 'Realtek 8139A',0
sz_RTL8139A_G           db 'Realtek 8139A rev G',0
sz_RTL8139B             db 'Realtek 8139B',0
sz_RTL8130              db 'Realtek 8130',0
sz_RTL8139C             db 'Realtek 8139C',0
sz_RTL8100              db 'Realtek 8100',0
sz_RTL8100_8139D        db 'Realtek 8100B / 8139D',0
sz_RTL8139CP            db 'Realtek 8139CP', 0
sz_RTL8101              db 'Realtek 8101',0

hw_ver_names:
        dd sz_unknown
        dd sz_RTL8139
        dd sz_RTL8139_K
        dd sz_RTL8139A
        dd sz_RTL8139A_G
        dd sz_RTL8139B
        dd sz_RTL8130
        dd sz_RTL8139C
        dd sz_RTL8100
        dd sz_RTL8100_8139D
        dd sz_RTL8139CP
        dd sz_RTL8101

hw_ver_array:                   ; This array is used by the probe routine to find out wich version of the RTL8139 we are working with
        db 0
        db VER_RTL8139
        db VER_RTL8139_K
        db VER_RTL8139A
        db VER_RTL8139A_G
        db VER_RTL8139B
        db VER_RTL8130
        db VER_RTL8139C
        db VER_RTL8100
        db VER_RTL8100_8139D
        db VER_RTL8139CP
        db VER_RTL8101

include_debug_strings           ; All data wich FDO uses will be included here

align 4
devices         dd 0
device_list     rd MAX_DEVICES  ; This list contains all pointers to device structures the driver is handling

