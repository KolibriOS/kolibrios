;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  RTL8029/ne2000 driver for KolibriOS                            ;;
;;                                                                 ;;
;;  based on RTL8029.asm driver for menuetos                       ;;
;;  and realtek8029.asm for SolarOS by Eugen Brasoveanu            ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;     with help from CleverMouse                                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        MAX_DEVICES             = 16

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2             ; 1 = verbose, 2 = errors only

include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../netdrv.inc'

virtual at ebx

        device:

        ETH_DEVICE

        .io_addr        dd ?
        .irq_line       db ?
        .pci_bus        dd ?
        .pci_dev        dd ?

        .flags          db ?
        .vendor         db ?

        .memsize        db ?
        .rx_start       db ?
        .tx_start       db ?
        .bmem           dd ?
        .rmem           dd ?

        .size = $ - device

end virtual


public START
public service_proc
public version

        P0_COMMAND              = 0x00
        P0_PSTART               = 0x01
        P0_PSTOP                = 0x02
        P0_BOUND                = 0x03
        P0_TSR                  = 0x04
        P0_TPSR                 = 0x04
        P0_TBCR0                = 0x05
        P0_TBCR1                = 0x06
        P0_ISR                  = 0x07
        P0_RSAR0                = 0x08
        P0_RSAR1                = 0x09
        P0_RBCR0                = 0x0A
        P0_RBCR1                = 0x0B
        P0_RSR                  = 0x0C
        P0_RCR                  = 0x0C
        P0_TCR                  = 0x0D
        P0_DCR                  = 0x0E
        P0_IMR                  = 0x0F

        P1_COMMAND              = 0x00
        P1_PAR0                 = 0x01
        P1_PAR1                 = 0x02
        P1_PAR2                 = 0x03
        P1_PAR3                 = 0x04
        P1_PAR4                 = 0x05
        P1_PAR5                 = 0x06
        P1_CURR                 = 0x07
        P1_MAR0                 = 0x08

        CMD_PS0                 = 0x00          ; Page 0 select
        CMD_PS1                 = 0x40          ; Page 1 select
        CMD_PS2                 = 0x80          ; Page 2 select
        CMD_RD2                 = 0x20          ; Remote DMA control
        CMD_RD1                 = 0x10
        CMD_RD0                 = 0x08
        CMD_TXP                 = 0x04          ; transmit packet
        CMD_STA                 = 0x02          ; start
        CMD_STP                 = 0x01          ; stop

        CMD_RDMA_READ           = 001b shl 3
        CMD_RDMA_WRITE          = 010b shl 3
        CMD_RDMA_SEND_PACKET    = 011b shl 3
        CMD_RDMA_ABORT          = 100b shl 3    ; really is 1xx, Abort/Complete Remote DMA
;        RDMA_MASK               = 111b shl 3    ; internal, mask

        RCR_MON                 = 0x20          ; monitor mode

        DCR_FT1                 = 0x40
        DCR_LS                  = 0x08          ; Loopback select
        DCR_WTS                 = 0x01          ; Word transfer select

        ISR_PRX                 = 0x01          ; successful recv
        ISR_PTX                 = 0x02          ; successful xmit
        ISR_RXE                 = 0x04          ; receive error
        ISR_TXE                 = 0x08          ; transmit error
        ISR_OVW                 = 0x10          ; Overflow
        ISR_CNT                 = 0x20          ; Counter overflow
        ISR_RDC                 = 0x40          ; Remote DMA complete
        ISR_RST                 = 0x80          ; reset

        IRQ_MASK                = ISR_PRX ;+ ISR_PTX ;+ ISR_RDC + ISR_PTX + ISR_TXE

        RSTAT_PRX               = 1 shl 0       ; successful recv
        RSTAT_CRC               = 1 shl 1       ; CRC error
        RSTAT_FAE               = 1 shl 2       ; Frame alignment error
        RSTAT_OVER              = 1 shl 3       ; FIFO overrun

        TXBUF_SIZE              = 6
        RXBUF_END               = 32
        PAGE_SIZE               = 256

        ETH_ZLEN                = 60
        ETH_FRAME_LEN           = 1514

        FLAG_PIO                = 1 shl 0
        FLAG_16BIT              = 1 shl 1

        VENDOR_NONE             = 0
        VENDOR_WD               = 1
        VENDOR_NOVELL           = 2
        VENDOR_3COM             = 3

        NE_ASIC                 = 0x10
        NE_RESET                = 0x0F          ; Used to reset card
        NE_DATA                 = 0x00          ; Used to read/write NIC mem

        MEM_8k                  = 32
        MEM_16k                 = 64
        MEM_32k                 = 128

        ISA_MAX_ADDR            = 0x400



section '.flat' code readable align 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; proc START
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc START stdcall, state:dword

        cmp     [state], 1
        jne     .exit
  .entry:
        DEBUGF  1, "Loading driver\n"
        stdcall RegService, my_service, service_proc
        ret
  .fail:
  .exit:
        xor     eax, eax
        ret

endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; proc SERVICE_PROC
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------
                       ;---------------
        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F     ;---------------

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:                  ;---------
        cmp     eax, 1 ;SRV_HOOK
        jne     @F     ;---------

        DEBUGF  1, "Checking if device is already listed..\n"

        mov     eax, [edx + IOCTL.input]

        cmp     [edx + IOCTL.inp_size], 3
        jb      .fail
        cmp     byte [eax], 1
        je      .pci

        cmp     [edx + IOCTL.inp_size], 4
        jb      .fail
        cmp     byte [eax], 0
        je      .isa

        jmp     .fail

  .pci:

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice_pci

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

  .firstdevice_pci:
        call    create_new_struct

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device

        PCI_find_io

; We've found the io address, find IRQ now

        PCI_find_irq

        jmp     .hook

  .isa:

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice_isa
        mov     al, [eax+3]
        movzx   edi, word [eax+1]
  .nextdevice_isa:
        mov     ebx, [esi]
        cmp     edi, [device.io_addr]
        jne     .maybenext
        cmp     al, [device.irq_line]
        je      find_device_num
  .maybenext:
        add     esi, 4
        loop    .nextdevice_isa



  .firstdevice_isa:
        call    create_new_struct

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, word [eax+1]
        mov     [device.io_addr], ecx
        mov     cl, [eax+3]
        mov     [device.irq_line], cl

  .hook:

        DEBUGF  1, "Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]
        mov     [device_list+4*eax], ebx
        inc     [devices]

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        jz      .err
        ret


; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1, "Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1, "Kernel says: %u\n", eax
        ret

  .err:
        DEBUGF  2, "Failed, removing device structure\n"
        stdcall KernelFree, ebx

        jmp     .fail

;------------------------------------------------------
  @@:
.fail:
        or      eax, -1
        ret

;------------------------------------------------------
endp


create_new_struct:

        cmp     [devices], MAX_DEVICES
        jae     .fail

        allocate_and_clear ebx, device.size, .fail      ; Allocate the buffer for device structure

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.unload], unload
        mov     [device.name], my_service

        ret

  .fail:
        add     esp, 4                                  ; return to caller of 'hook'
        or      eax, -1
        ret

find_device_num:

        DEBUGF  1, "Trying to find device number of already registered device\n"
        mov     ebx, eax
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1, "Kernel says: %u\n", eax
        ret


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


unload:   ; TODO
        or      eax, -1
        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device and clears the rx buffer
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

probe:
        mov     [device.vendor], VENDOR_NONE
        mov     [device.bmem], 0

        DEBUGF  1, "Trying 16-bit mode\n"

        mov     [device.flags], FLAG_16BIT + FLAG_PIO
        mov     [device.memsize], MEM_32k
        mov     [device.tx_start], 64
        mov     [device.rx_start], TXBUF_SIZE + 64

        set_io  0
        set_io  P0_DCR
        mov     al, DCR_WTS + DCR_FT1 + DCR_LS  ; word transfer select +
        out     dx, al

        set_io  P0_PSTART
        mov     al, MEM_16k
        out     dx, al

        set_io  P0_PSTOP
        mov     al, MEM_32k
        out     dx, al

        mov     esi, my_service
        mov     di, 16384
        mov     cx, 14
        call    PIO_write

        mov     si, 16384
        mov     cx, 14
        sub     esp, 16
        mov     edi, esp
        call    PIO_read

        mov     esi, esp
        add     esp, 16
        mov     edi, my_service
        mov     ecx, 13
        repe    cmpsb
        je      ep_set_vendor

        DEBUGF  1, "16-bit mode failed\n"
        DEBUGF  1, "Trying 8-bit mode\n"

        mov     [device.flags], FLAG_PIO
        mov     [device.memsize], MEM_16k
        mov     [device.tx_start], 32
        mov     [device.rx_start], TXBUF_SIZE + 32

        set_io  NE_ASIC + NE_RESET
        in      al, dx
        out     dx, al

        in      al, 0x84

        set_io  P0_COMMAND
        mov     al, CMD_RD2 + CMD_STP
        out     dx, al

        set_io  P0_RCR
        mov     al, RCR_MON
        out     dx, al

        set_io  P0_DCR
        mov     al, DCR_FT1 + DCR_LS
        out     dx, al

        set_io  P0_PSTART
        mov     al, MEM_8k
        out     dx, al

        set_io  P0_PSTOP
        mov     al, MEM_16k
        out     dx, al

        mov     esi, my_service
        mov     di, 8192
        mov     cx, 14
        call    PIO_write

        mov     si, 8192
        mov     cx, 14
        sub     esp, 16
        mov     edi, esp
        call    PIO_read

        mov     esi, my_service
        mov     edi, esp
        add     esp, 16
        mov     ecx, 13
        repe    cmpsb
        je      ep_set_vendor

        DEBUGF  2, "This is not a valid ne2000 device!\n"
        or      eax, -1
        ret


ep_set_vendor:

        DEBUGF  1, "Mode ok\n"

        cmp     [device.io_addr], ISA_MAX_ADDR
        jbe     .isa

        DEBUGF  1, "Card is using PCI bus\n"

        mov     [device.vendor], VENDOR_NOVELL  ;;; FIXME
        jmp     ep_check_have_vendor

  .isa:
        DEBUGF  1, "Card is using ISA bus\n"

        mov     [device.vendor], VENDOR_NOVELL

ep_check_have_vendor:


        mov     al, [device.vendor]
        cmp     al, VENDOR_NONE
;        je      exit

        cmp     al, VENDOR_3COM
        je      reset

        mov     eax, [device.bmem]
        mov     [device.rmem], eax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   reset: Place the chip into a virgin state
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

reset:
        DEBUGF  1, "Resetting device\n"

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

; Stop card + DMA
        set_io  0
;        set_io  P0_COMMAND
        mov     al, CMD_PS0 + CMD_RDMA_ABORT + CMD_STP
        out     dx, al

; initialize DCR
        set_io  P0_DCR
        mov     al, DCR_FT1 + DCR_LS
        test    [device.flags], FLAG_16BIT
        jz      @f
        or      al, DCR_WTS                     ; word transfer select
      @@:
        out     dx, al

; clear remote bytes count
        set_io  P0_RBCR0
        xor     al, al
        out     dx, al

        set_io  P0_RBCR1
        out     dx, al

; initialize Receive configuration register (until all init is done)
        set_io  P0_RCR
        mov     al, 0x20        ; monitor mode
        out     dx, al

; transmit configuration register to monitor mode (until all ini is done)
        set_io  P0_TCR
        mov     al, 2           ; internal loopback
        out     dx, al

; clear interupt status
        set_io  P0_ISR
        mov     al, 0xff
        out     dx, al

; clear IRQ mask                        ;;;;; CHECKME ;;;;;
        set_io  P0_IMR
        xor     al, al
        out     dx, al

; set transmit pointer
        set_io  P0_TPSR
        mov     al, [device.tx_start]
        out     dx, al

; set pagestart pointer
        set_io  P0_PSTART
        mov     al, [device.rx_start]
        out     dx, al

; set pagestop pointer
        set_io  P0_PSTOP
        mov     al, [device.memsize]
        out     dx, al

; set boundary pointer
        set_io  P0_BOUND
        mov     al, [device.memsize]
        dec     al
        out     dx, al

; set curr pointer
        set_io  P0_COMMAND
        mov     al, CMD_PS1 ;+ CMD_RD2 + CMD_STP ; page 1, stop mode
        out     dx, al

        set_io  P1_CURR
        mov     al, [device.rx_start]
        out     dx, al

        set_io  P0_COMMAND
        mov     al, CMD_PS0 ;+ CMD_RD2 + CMD_STA ; go to page 0, start mode
        out     dx, al

; Read MAC address and set it to registers
        call    read_mac
        push    .macret
        sub     esp, 6
        lea     esi, [device.mac]
        mov     edi, esp
        movsd
        movsw
        jmp     write_mac
  .macret:

; set IRQ mask
        set_io  0
        set_io  P0_IMR
        mov     al, IRQ_MASK
        out     dx, al

; start mode
        set_io  P0_COMMAND
        mov     al, CMD_STA
        out     dx, al

; clear transmit control register
        set_io  P0_TCR
        xor     al, al                  ; no loopback
        out     dx, al

; set receive control register ;;;;
        set_io  P0_RCR
        mov     al, 4                   ; accept broadcast
        out     dx, al

; clear packet/byte counters
        xor     eax, eax
        lea     edi, [device.bytes_tx]
        mov     ecx, 6
        rep     stosd

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], ETH_FRAME_LEN

; Set link state to unknown
        mov     [device.state], ETH_LINK_UNKOWN

; Indicate that we have successfully reset the card
        xor     eax, eax
        DEBUGF  1, "Done!\n"

        ret



;***************************************************************************
;   Function
;      transmit
; buffer in [esp+4], size in [esp+8], pointer to device struct in ebx
;***************************************************************************

align 4
transmit:

        mov     esi, [esp + 4]
        mov     ecx, [esp + 8]
        DEBUGF  1, "Transmitting packet, buffer:%x, size:%u\n",esi, ecx
        DEBUGF  1, "To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [esi+0]:2,[esi+1]:2,[esi+2]:2,[esi+3]:2,[esi+4]:2,[esi+5]:2,[esi+6]:2,[esi+7]:2,[esi+8]:2,[esi+9]:2,[esi+10]:2,[esi+11]:2,[esi+13]:2,[esi+12]:2

        cmp     ecx, ETH_FRAME_LEN
        ja      .err ; packet is too long
        cmp     ecx, ETH_ZLEN
        jb      .err ; packet is too short

        movzx   edi, [device.tx_start]
        shl     edi, 8
        push    cx
        call    PIO_write
        pop     cx

        set_io  0
;        set_io  P0_COMMAND
        mov     al, CMD_PS0 + CMD_RD2 + CMD_STA
        out     dx, al

        set_io  P0_TPSR
        mov     al, [device.tx_start]
        out     dx, al

        set_io  P0_TBCR0
        mov     al, cl
        out     dx, al

        set_io  P0_TBCR1
        mov     al, ch
        out     dx, al

        set_io  P0_COMMAND
        mov     al, CMD_PS0 + CMD_TXP + CMD_RD2 + CMD_STA
        out     dx, al

        DEBUGF  1, "Packet Sent!\n"

        inc     [device.packets_tx]
        mov     eax, [esp + 8]                   ; Get packet size in eax

        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx + 4], 0

        stdcall KernelFree, [esp+4]
        xor     eax, eax
        ret     8

  .err:
        DEBUGF  2, "Transmit error!\n"
        stdcall KernelFree, [esp+4]
        or      eax, -1
        ret     8



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Interrupt handler
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1, "INT\n"

; find pointer of device wich made INT occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  0
;        set_io  P0_COMMAND
        mov     al, CMD_PS0
        out     dx, al

        set_io  P0_ISR
        in      al, dx
        test    al, al
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

        DEBUGF  1, "Device=%x status=%x\n", ebx, eax:2

        push    ebx

        test    al, ISR_PRX     ; packet received ok ?
        jz      .no_rx

        test    [device.flags], FLAG_PIO
        jz      .no_rx          ; FIXME: Only PIO mode supported for now

;

        pushd   .no_rx

; allocate a buffer

        stdcall KernelAlloc, ETH_FRAME_LEN
        test    eax, eax
        jz      .fail_2
        pushd   0
        push    eax

; read offset for current packet from device

        set_io  0
        set_io  P0_BOUND        ; boundary ptr is offset to next packet we need to read.
        in      al, dx
        inc     al

        cmp     al, [device.memsize]
        jb      @f
        mov     al, [device.rx_start]
       @@:
        mov     ch, al

        set_io  P0_COMMAND
        mov     al, CMD_PS1
        out     dx, al

        set_io  P1_CURR
        in      al, dx          ; get current page in cl
        mov     cl, al

        set_io  P1_COMMAND
        mov     al, CMD_PS0
        out     dx, al

        cmp     cl, [device.memsize]
        jb      @f
        mov     cl, [device.rx_start]
       @@:

        cmp     cl, ch
        je      .fail

        movzx   esi, ch                         ; we are using 256 byte pages
        shl     esi, 8                          ; esi now holds the offset for current packet

; Get packet header in eax

        sub     esp, 4                          ; reserve 4 bytes on stack to put packet header in
        mov     edi, esp
        mov     cx, 4
        call    PIO_read

        mov     ecx, [esp]                      ; ecx now contains packet header

; check if packet is ok

        test    ecx, RSTAT_PRX
        jz      .fail_3

; calculate packet length in ecx

        shr     ecx, 16
        sub     ecx, 4                          ; CRC doesnt count as data byte
        mov     [esp + 4 + 4], ecx

; check if packet size is ok

        cmp     ecx, ETH_ZLEN
        jb      .fail_3
        cmp     ecx, ETH_FRAME_LEN
        ja      .fail_3

; update stats

        DEBUGF  1, "Received %u bytes\n", ecx

        add     dword[device.bytes_rx], ecx
        adc     dword[device.bytes_rx + 4], 0
        inc     [device.packets_rx]

; update read and write pointers

        add     esi, 4
        mov     edi, [esp + 4]

; now check if we can read all data at once (if we cross the end boundary, we need to wrap back to the beginning)

        xor     eax, eax
        mov     ah, [device.memsize]
        sub     eax, esi
        cmp     ecx, eax                ; eax = number of bytes till end of buffer, ecx = bytes we need to read
        jbe     .no_wrap

; Read first part

        sub     ecx, eax
        push    ecx
        mov     ecx, eax

        call    PIO_read                ; Read the data

; update pointers

        add     edi, ecx
        pop     ecx

        movzx   esi, [device.rx_start]
        shl     esi, 8

; now read second part (or only part)

  .no_wrap:
        call    PIO_read                ; Read the data

; update boundary pointer

        pop     eax
        mov     al, ah
        cmp     al, [device.rx_start]
        jne     @f
        mov     al, [device.memsize]
       @@:

        set_io  0
        set_io  P0_BOUND
        dec     al
        out     dx, al

; now send the data to the kernel

        jmp     Eth_input

  .fail_3:
        add     esp, 4
  .fail:
        add     esp, 8
  .fail_2:


  .no_rx:
        pop     ebx
        DEBUGF  1, "done\n"

        set_io  0
        set_io  P0_ISR
        mov     al, 0xff
        out     dx, al

        pop     edi esi ebx

        ret





;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:      ; in: mac on stack (6 bytes)

        DEBUGF  1, "Writing MAC\n"

        set_io  0
        mov     al, CMD_PS1; + CMD_RD2 + CMD_STP
        out     dx, al

        set_io  P1_PAR0
        mov     esi, esp
        mov     cx, 6
 @@:
        lodsb
        out     dx, al
        inc     dx
        loopw   @r

        add     esp, 6

; Notice this procedure does not ret, but continues to read_mac instead.

;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;

read_mac:

        DEBUGF  1, "Reading MAC\n"

        xor     esi, esi
        mov     cx, 16
        sub     esp, 16
        mov     edi, esp
        call    PIO_read

        mov     esi, esp
        add     esp, 16
        lea     edi, [device.mac]
        mov     ecx, 6
  .loop:
        movsb
        test    [device.flags], FLAG_16BIT
        jz      .8bit
        inc     esi
  .8bit:
        loop    .loop

        DEBUGF  1, "MAC=%x-%x-%x-%x-%x-%x\n",\
        [device.mac]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        ret


;***************************************************************************
;
;   PIO_read
;
;   Description
;       Read a frame from the ethernet card via Programmed I/O
;      src in si
;      cnt in cx
;       dst in edi
;***************************************************************************
PIO_read:

        DEBUGF  1, "PIO Read from %x to %x, %u bytes ", si, edi, cx

; start DMA
        set_io  0
;        set_io  P0_COMMAND
        mov     al, CMD_RD2 + CMD_STA
        out     dx, al

; set length of data we're interested in
        set_io  P0_RBCR0
        mov     al, cl
        out     dx, al

        set_io  P0_RBCR1
        mov     al, ch
        out     dx, al

; set offset of what we want to read
        set_io  P0_RSAR0
        mov     ax, si
        out     dx, al

        set_io  P0_RSAR1
        shr     ax, 8
        out     dx, al

; start DMA read
        set_io  P0_COMMAND
        mov     al, CMD_RD0 + CMD_STA
        out     dx, al

        set_io  NE_ASIC

        test    [device.flags], FLAG_16BIT
        jz      .8bits

        DEBUGF  1, "(16-bit mode)\n"

        shr     cx, 1   ; note that if the number was odd, carry flag will be set
        pushf

  .16bits:
        in      ax, dx
        stosw
        loopw   .16bits

        inc     cx
        popf
        jnc     .done
        jmp     .8bits_

  .8bits:
        DEBUGF  1, "(8-bit mode)\n"

  .8bits_:
        in      al, dx
        stosb
        loopw   .8bits_


  .done:
;        set_io  0
;        set_io  P0_ISR
;
;  .dmawait:                             ; Wait for Remote DMA Complete
;        in      al, dx
;        test    al, ISR_RDC
;        jz      .dmawait
;        and     al, not ISR_RDC
;        out     dx, al                  ; clear the bit

        ret




;***************************************************************************
;
;   PIO_write
;
;   Description
;       writes a frame to the ethernet card via Programmed I/O
;      dst in di
;      cnt in cx
;       src in esi
;***************************************************************************
PIO_write:

        DEBUGF  1, "Eth PIO Write from %x to %x, %u bytes ", esi, di, cx

        set_io  0
;        set_io  P0_COMMAND
        mov     al, CMD_RD2 + CMD_STA
        out     dx, al

        set_io  P0_ISR
        mov     al, ISR_RDC
        out     dx, al

        set_io  P0_RBCR0
        mov     al, cl
        out     dx, al

        set_io  P0_RBCR1
        mov     al, ch
        out     dx, al

        mov     ax, di
        set_io  P0_RSAR0
        out     dx, al
        shr     ax, 8
        set_io  P0_RSAR1
        out     dx, al

        set_io  P0_COMMAND
        mov     al, CMD_RD1 + CMD_STA
        out     dx, al

        set_io  NE_ASIC
        test    [device.flags], FLAG_16BIT
        jz      .8_bit

        DEBUGF  1, "(16-bit mode)\n"

        shr     cx, 1   ; note that if the number was odd, carry flag will be set
        pushf           ; save the flags for later

  .16bit:
        lodsw
        out     dx, ax
        loopw   .16bit

        popf
        jnc     .done
        inc     cx
        jmp     .8_bit_

  .8_bit:

        DEBUGF  1, "(8-bit mode)\n"

  .8_bit_:
        lodsb
        out     dx, al
        loopw   .8_bit_

  .done:
;        set_io  0
;        set_io  P0_ISR
;  .dmawait:                             ; Wait for Remote DMA Complete
;        in      al, dx
;        test    al, ISR_RDC
;        jz      .dmawait
;        and     al, not ISR_RDC
;        out     dx, al                  ; clear the bit

        ret



;all initialized data place here
align 4

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'RTL8029/ne2000',0  ;max 16 chars include zero

;device_1        db 'Realtek 8029',0
;device_2        db 'Realtek 8019',0
;device_3        db 'Realtek 8019AS',0
;device_4        db 'ne2000',0
;device_5        db 'DP8390',0

include_debug_strings

section '.data' data readable writable align 16  ;place all uninitialized data place here

device_list     rd MAX_DEVICES





