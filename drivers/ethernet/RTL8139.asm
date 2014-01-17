;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
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

format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        MAX_DEVICES             = 16

        RBLEN                   = 3 ; Receive buffer size: 0==8K 1==16k 2==32k 3==64k

        TXRR                    = 8 ; total retries = 16+(TXRR*16)
        TX_MXDMA                = 6 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=2048
        ERTXTH                  = 8 ; in unit of 32 bytes e.g:(8*32)=256
        RX_MXDMA                = 7 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=unlimited
        RXFTH                   = 7 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=no threshold

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2             ; 1 = verbose, 2 = errors only

include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../netdrv.inc'

public START
public service_proc
public version

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
        REG_MSR                 = 0x58
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

        BIT_IFG1                = 25
        BIT_IFG0                = 24

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
        MAX_ETH_FRAME_SIZE      = 1514
        NUM_TX_DESC             = 4                                     ; not user selectable

        EE_93C46_REG_ETH_ID     = 7 ; MAC offset
        EE_93C46_READ_CMD       = (6 shl 6) ; 110b + 6bit address
        EE_93C56_READ_CMD       = (6 shl 8) ; 110b + 8bit address
        EE_93C46_CMD_LENGTH     = 9  ; start bit + cmd + 6bit address
        EE_93C56_CMD_LENGTH     = 11 ; start bit + cmd + 8bit ddress

        VER_RTL8139             = 1100000b
        VER_RTL8139A            = 1110000b
        VER_RTL8139AG           = 1110100b
        VER_RTL8139B            = 1111000b
        VER_RTL8130             = VER_RTL8139B
        VER_RTL8139C            = 1110100b
        VER_RTL8100             = 1111010b
        VER_RTL8100B            = 1110101b
        VER_RTL8139D            = VER_RTL8100B
        VER_RTL8139CP           = 1110110b
        VER_RTL8101             = 1110111b

        IDX_RTL8139             = 0
        IDX_RTL8139A            = 1
        IDX_RTL8139B            = 2
        IDX_RTL8139C            = 3
        IDX_RTL8100             = 4
        IDX_RTL8139D            = 5
        IDX_RTL8139D            = 6
        IDX_RTL8101             = 7

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


virtual at ebx

        device:

        ETH_DEVICE

        .rx_buffer      dd ?

        .rx_data_offset dd ?
        .io_addr        dd ?

        .curr_tx_desc   db ?
        .pci_bus        dd ?
        .pci_dev        dd ?
        .irq_line       db ?
        .hw_ver_id      db ?

                        db ?    ; align 4
        .TX_DESC        rd NUM_TX_DESC

        .size = $ - device

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

        cmp     [state], 1
        jne     .exit

  .entry:

        DEBUGF  2, "Loading driver\n"
        stdcall RegService, my_service, service_proc
        ret

  .fail:
  .exit:
        xor     eax, eax
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

        mov     ax, [eax+1]                             ; get the pci bus and device numbers
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

        allocate_and_clear ebx, device.size, .fail      ; Allocate the buffer for device structure

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

        DEBUGF  1, "Hooking into device, dev:%x, bus:%x, irq:%x, I/O addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

; Allocate the receive buffer

        stdcall CreateRingBuffer, dword (RX_BUFFER_SIZE), dword PG_SW
        test    eax, eax
        jz      .err
        mov     [device.rx_buffer], eax

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

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1, "Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1, "Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: unregister device from device_list
        ; todo: reset device into virgin state

  .err:
        DEBUGF  2, "Error, removing all data !\n"
        stdcall KernelFree, [device.rx_buffer]
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

        PCI_make_bus_master

; get chip version
        set_io  0
        set_io  REG_TXCONFIG + 2
        in      ax, dx
        shr     ah, 2
        shr     ax, 6
        and     al, 01111111b

; now find it in our array
        mov     ecx, HW_VER_ARRAY_SIZE-1
  .chip_ver_loop:
        cmp     al, [hw_ver_array + ecx]
        je      .chip_ver_found
        dec     ecx
        jns     .chip_ver_loop
  .unknown:
        mov     ecx, 8
  .chip_ver_found:
        cmp     ecx, 8
        ja      .unknown

        mov     [device.hw_ver_id], cl

        mov     ecx, [crosslist+ecx*4]
        mov     [device.name], ecx

        DEBUGF  1, "Chip version: %s\n", ecx

; wake up the chip
        set_io  0
        set_io  REG_HLTCLK
        mov     al, 'R'         ; run the clock
        out     dx, al

; unlock config and BMCR registers
        set_io  REG_9346CR
        mov     al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
        out     dx, al

; enable power management
        set_io  REG_CONFIG1
        in      al, dx
        cmp     [device.hw_ver_id], IDX_RTL8139B
        jae     .new_chip
; wake up older chips
        and     al, not ((1 shl BIT_SLEEP) or (1 shl BIT_PWRDWN))
        out     dx, al
        jmp     .finish_wake_up

; set LWAKE pin to active high (default value).
; it is for Wake-On-LAN functionality of some motherboards.
; this signal is used to inform the motherboard to execute a wake-up process.
; only at newer chips.
  .new_chip:
        or      al, (1 shl BIT_PMEn)
        and     al, not (1 shl BIT_LWACT)
        out     dx, al

        set_io  REG_CONFIG4
        in      al, dx
        and     al, not (1 shl BIT_LWPTN)
        out     dx, al

; lock config and BMCR registers
  .finish_wake_up:
        xor     al, al
        set_io  0
        set_io  REG_9346CR
        out     dx, al
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

; attach int handler
        movzx   eax, [device.irq_line]
        DEBUGF  1, "Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  2, "Could not attach int handler!\n"
;        or      eax, -1
;        ret
       @@:

; reset chip
        DEBUGF  1, "Resetting chip\n"
        set_io  0
        set_io  REG_COMMAND
        mov     al, 1 shl BIT_RST
        out     dx, al
        mov     cx, 1000                ; wait no longer for the reset
  .wait_for_reset:
        in      al, dx
        test    al, 1 shl BIT_RST
        jz      .reset_completed        ; RST remains 1 during reset
        dec     cx
        jns     .wait_for_reset
        DEBUGF  2, "Reset timeout!\n"
  .reset_completed:

; Read MAC address
        call    read_mac

; unlock config and BMCR registers
        set_io  0
        set_io  REG_9346CR
        mov     al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
        out     dx, al

; initialize multicast registers (no filtering)
        mov     eax, 0xffffffff
        set_io  REG_MAR0
        out     dx, eax
        set_io  REG_MAR4
        out     dx, eax

; enable Rx/Tx
        mov     al, (1 shl BIT_RE) or (1 shl BIT_TE)
        set_io  REG_COMMAND
        out     dx, al

; Rxbuffer size, unlimited dma burst, no wrapping, no rx threshold
; accept broadcast packets, accept physical match packets
        mov     eax, RX_CONFIG
        set_io  REG_RXCONFIG
        out     dx, eax

; 1024 bytes DMA burst, total retries = 16 + 8 * 16 = 144
        mov     eax, (TX_MXDMA shl BIT_TX_MXDMA) or (TXRR shl BIT_TXRR) or BIT_IFG1 or BIT_IFG0
        set_io  REG_TXCONFIG
        out     dx, eax

; enable auto negotiation
        set_io  REG_BMCR
        in      ax, dx
        or      ax, (1 shl BIT_ANE)
        out     dx, ax

; set auto negotiation advertisement
        set_io  REG_ANAR
        in      ax, dx
        or      ax, (1 shl BIT_SELECTOR) or (1 shl BIT_10) or (1 shl BIT_10FD) or (1 shl BIT_TX) or (1 shl BIT_TXFD)
        out     dx, ax

; lock config and BMCR registers
        xor     eax, eax
        set_io  REG_9346CR
        out     dx, al

; init RX/TX pointers
        mov     [device.rx_data_offset], eax
        mov     [device.curr_tx_desc], al
;        set_io  REG_CAPR
;        out     dx, ax

; clear packet/byte counters
        lea     edi, [device.bytes_tx]
        mov     ecx, 6
        rep     stosd

; clear missing packet counter
        set_io  REG_MPC
        out     dx, eax

; set RxBuffer address, init RX buffer offset
        mov     eax, [device.rx_buffer]
        mov     dword[eax], 0                   ; clear receive flags for first packet (really needed??)
        DEBUGF  1, "RX buffer virtual addr=0x%x\n", eax
        GetRealAddr
        DEBUGF  1, "RX buffer physical addr=0x%x\n", eax
        set_io  REG_RBSTART
        out     dx, eax

; enable interrupts
        set_io  0
        set_io  REG_IMR
        mov     ax, INTERRUPT_MASK
        out     dx, ax

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], 1514

        call    cable

; Indicate that we have successfully reset the card
        xor     eax, eax
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
        DEBUGF  1, "Transmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
        mov     eax, [esp+4]
        DEBUGF  1, "To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp+8], MAX_ETH_FRAME_SIZE
        ja      .fail
        cmp     dword [esp+8], 60
        jb      .fail

; check if we own the current discriptor
        set_io  0
        set_io  REG_TSD0
        movzx   ecx, [device.curr_tx_desc]
        shl     ecx, 2
        add     edx, ecx
        in      eax, dx
        test    eax, (1 shl BIT_OWN)
        jz      .wait_to_send

  .send_packet:
; get next descriptor
        inc     [device.curr_tx_desc]
        and     [device.curr_tx_desc], NUM_TX_DESC-1

; Update stats
        inc     [device.packets_tx]
        mov     eax, [esp+8]
        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx+4], 0

; Set the buffer address
        set_io  REG_TSAD0
        mov     eax, [esp+4]
        mov     [device.TX_DESC+ecx], eax
        GetRealAddr
        out     dx, eax

; And the size of the buffer
        set_io  REG_TSD0
        mov     eax, [esp+8]
        or      eax, (ERTXTH shl BIT_ERTXTH)    ; Early threshold
        out     dx, eax

        DEBUGF  1, "Packet Sent!\n"
        xor     eax, eax
        ret     8

  .wait_to_send:
        DEBUGF  1, "Waiting for timeout\n"

        push    edx
        mov     esi, 30
        stdcall Sleep
        pop     edx

        in      ax, dx
        test    ax, (1 shl BIT_OWN)
        jnz     .send_packet

        pusha
        call    reset                            ; if chip hung, reset it
        popa

        jmp     .send_packet

  .fail:
        DEBUGF  2, "transmit failed!\n"
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

        DEBUGF  1, "INT\n"

; find pointer of device wich made IRQ occur
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  0
        set_io  REG_ISR
        in      ax, dx                          ; Get interrupt status
        out     dx, ax                          ; send it back to ACK
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                                     ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:

        DEBUGF  1, "Device: %x Status: %x\n", ebx, ax

;----------------------------------------------------
; Received packet ok?

        test    ax, ISR_ROK
        jz      @f
        push    ax

  .receive:
        set_io  0
        set_io  REG_COMMAND
        in      al, dx
        test    al, BUFE                        ; test if RX buffer is empty
        jnz     .finish

        DEBUGF  1, "RX:\n"

        mov     eax, [device.rx_buffer]
        add     eax, [device.rx_data_offset]
        test    byte [eax], (1 shl BIT_ROK)     ; check if packet is ok
        jz      .reset_rx

; packet is ok, copy it
        movzx   ecx, word [eax+2]               ; packet length
        sub     cx, 4                           ; don't copy CRC

; Update stats
        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0
        inc     [device.packets_rx]

        DEBUGF  1, "Received %u bytes\n", ecx

        push    ebx eax ecx
        stdcall KernelAlloc, ecx                ; Allocate a buffer to put packet into
        pop     ecx
        test    eax, eax                        ; Test if we allocated succesfully
        jz      .abort

        mov     edi, eax                        ; Where we will copy too

        mov     esi, [esp]                      ; The buffer we will copy from
        add     esi, 4                          ; Dont copy CRC

        push    dword .abort                    ; Kernel will return to this address after EthReceiver
        push    ecx edi                         ; Save buffer pointer and size, to pass to kernel

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

        jmp     Eth_input                       ; Send it to kernel

  .abort:
        pop     eax ebx
                                                ; update eth_data_start_offset
        movzx   eax, word [eax+2]               ; packet length
        add     eax, [device.rx_data_offset]
        add     eax, 4+3                        ; packet header is 4 bytes long + dword alignment
        and     eax, not 3                      ; dword alignment

        cmp     eax, RX_BUFFER_SIZE
        jb      .no_wrap
        DEBUGF  1, "Wrapping\n"
        sub     eax, RX_BUFFER_SIZE
  .no_wrap:
        mov     [device.rx_data_offset], eax
        DEBUGF  1, "New RX ptr: %d\n", eax

        set_io  0
        set_io  REG_CAPR                        ; update 'Current Address of Packet Read register'
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

;----------------------------------------------------
; Transmit ok / Transmit error
  @@:
        test    ax, ISR_TOK + ISR_TER
        jz      @f

        DEBUGF  1, "Transmit done!\n"

        push    ax
        mov     ecx, (NUM_TX_DESC-1)*4
  .txdescloop:
        set_io  0
        set_io  REG_TSD0
        add     edx, ecx
        in      eax, dx

        test    eax, TSR_OWN                    ; DMA operation completed
        jz      .notthisone

        cmp     [device.TX_DESC+ecx], 0
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
        DEBUGF  1, "free transmit buffer 0x%x\n", [device.TX_DESC+ecx]:8
        push    ecx ebx
        stdcall KernelFree, [device.TX_DESC+ecx]
        pop     ebx ecx
        mov     [device.TX_DESC+ecx], 0

  .notthisone:
        sub     ecx, 4
        jae     .txdescloop
        pop     ax

;----------------------------------------------------
; Rx buffer overflow ?
  @@:
        test    ax, ISR_RXOVW
        jz      @f

        push    ax
        DEBUGF  2, "RX:buffer overflow!\n"

        set_io  0
        set_io  REG_ISR
        mov     ax, ISR_FIFOOVW or ISR_RXOVW
        out     dx, ax
        pop     ax

;----------------------------------------------------
; Packet underrun?
  @@:
        test    ax, ISR_PUN
        jz      @f

        DEBUGF  1, "Packet underrun or link changed!\n"

        call    cable

;----------------------------------------------------
; Receive FIFO overflow ?
  @@:
        test    ax, ISR_FIFOOVW
        jz      @f

        push    ax
        DEBUGF  2, "RX fifo overflow!\n"

        set_io  0
        set_io  REG_ISR
        mov     ax, ISR_FIFOOVW or ISR_RXOVW
        out     dx, ax
        pop     ax

;----------------------------------------------------
; cable length changed ?
  @@:
        test    ax, ISR_LENCHG
        jz      .fail

        DEBUGF  2, "Cable length changed!\n"

        call    cable

  .fail:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret




;;;;;;;;;;;;;;;;;;;;;;;;;
;;                     ;;
;; Update Cable status ;;
;;                     ;;
;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
cable:
        DEBUGF  1, "Checking link status:\n"

        set_io  0
        set_io  REG_MSR
        in      al, dx

        test    al, 1 shl 2             ; 0 = link ok 1 = link fail
        jnz     .notconnected

        test    al, 1 shl 3             ; 0 = 100 Mbps 1 = 10 Mbps
        jnz     .10mbps

  .100mbps:
        mov     [device.state], ETH_LINK_100M
        call    NetLinkChanged
        DEBUGF  2, "link changed to 100 mbit\n"

        ret

  .10mbps:
        mov     [device.state], ETH_LINK_10M
        call    NetLinkChanged
        DEBUGF  2, "link changed to 10 mbit\n"

        ret

  .notconnected:
        mov     [device.state], ETH_LINK_DOWN
        call    NetLinkChanged
        DEBUGF  2, "no link\n"

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
        set_io  0
        set_io  REG_9346CR
        xor     eax, eax
        out     dx, al

        set_io  REG_IMR
        xor     eax, eax
        out     dx, ax

        set_io  REG_ISR
        mov     eax, -1
        out     dx, ax

; enable writing
        set_io  REG_9346CR
        mov     eax, REG_9346CR_WE
        out     dx, al

 ; write the mac ...
        set_io  REG_IDR0
        pop     eax
        out     dx, eax

        set_io  REG_IDR0+4
        xor     eax, eax
        pop     ax
        out     dx, eax

; disable writing
        set_io  REG_9346CR
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

        set_io  0
        lea     edi, [device.mac]
        in      eax, dx
        stosd
        add     edx, 4
        in      ax, dx
        stosw

        DEBUGF  1, "%x-%x-%x-%x-%x-%x\n",[edi-6]:2,[edi-5]:2,[edi-4]:2,[edi-3]:2,[edi-2]:2,[edi-1]:2

        ret


; End of code

section '.data' data readable writable align 16 ; place all uninitialized data place here
align 4                                         ; Place all initialised data here

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'RTL8139',0                    ; max 16 chars include zero

device_1        db 'Realtek 8139',0
device_2        db 'Realtek 8139A',0
device_3        db 'Realtek 8139B',0
device_4        db 'Realtek 8139C',0
device_5        db 'Realtek 8100',0
device_6        db 'Realtek 8139D',0
device_7        db 'Realtek 8139CP',0
device_8        db 'Realtek 8101',0
device_unknown  db 'Unknown RTL8139 clone', 0

crosslist:
        dd device_1
        dd device_2
        dd device_3
        dd device_4
        dd device_5
        dd device_6
        dd device_7
        dd device_8
        dd device_unknown

hw_ver_array:                    ; This array is used by the probe routine to find out wich version of the RTL8139 we are working with
        db VER_RTL8139
        db VER_RTL8139A
        db VER_RTL8139B
        db VER_RTL8139C
        db VER_RTL8100
        db VER_RTL8139D
        db VER_RTL8139CP
        db VER_RTL8101
        db 0

HW_VER_ARRAY_SIZE = $-hw_ver_array

include_debug_strings                           ; All data wich FDO uses will be included here

device_list     rd MAX_DEVICES                   ; This list contains all pointers to device structures the driver is handling

