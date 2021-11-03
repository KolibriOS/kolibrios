;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; i8255x (Intel eepro 100) driver for KolibriOS                   ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;; Some parts of this driver are based on the code of eepro100.c   ;;
;;  from linux.                                                    ;;
;;                                                                 ;;
;; Intel's programming manual for i8255x:                          ;;
;; http://www.intel.com/design/network/manuals/8255x_opensdm.htm   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;TODO: use more RX buffers

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

; configureable area

        MAX_DEVICES             = 16            ; Maximum number of devices this driver may handle

        TX_RING_SIZE            = 16            ; Number of packets in send ring buffer
        RX_RING_SIZE            = 1             ; Number of packets in receive ring buffer

        __DEBUG__               = 1             ; 1 = on, 0 = off
        __DEBUG_LEVEL__         = 2             ; 1 = verbose, 2 = errors only

; end configureable area

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

if (bsr TX_RING_SIZE)>(bsf TX_RING_SIZE)
  display 'TX_RING_SIZE must be a power of two'
  err
end if

if (RX_RING_SIZE)<>(1)
  display 'RX_RING_SIZE must be 1'
  err
end if

; I/O registers
REG_SCB_STATUS          = 0
REG_SCB_CMD             = 2
REG_SCB_PTR             = 4
REG_PORT                = 8
REG_EEPROM              = 14
REG_MDI_CTRL            = 16

; Port commands
PORT_SOFT_RESET         = 0x0
PORT_SELF_TEST          = 0x1
PORT_SELECTIVE_RESET    = 0x2
PORT_DUMP               = 0x3
PORT_DUMP_WAKEUP        = 0x7
PORT_PTR_MASK           = 0xfffffff0

; Serial EEPROM
EE_SK                   = 1 shl 0       ; serial clock
EE_CS                   = 1 shl 1       ; chip select
EE_DI                   = 1 shl 2       ; data in
EE_DO                   = 1 shl 3       ; data out
EE_MASK                 = EE_SK or EE_CS or EE_DI or EE_DO
; opcodes, first bit is start bit and must be 1
EE_READ                 = 110b
EE_WRITE                = 101b
EE_ERASE                = 111b

; The SCB accepts the following controls for the Tx and Rx units:
CU_START                = 0x0010
CU_RESUME               = 0x0020
CU_STATSADDR            = 0x0040
CU_SHOWSTATS            = 0x0050        ; Dump statistics counters.
CU_CMD_BASE             = 0x0060        ; Base address to add CU commands.
CU_DUMPSTATS            = 0x0070        ; Dump then reset stats counters.

RX_START                = 0x0001
RX_RESUME               = 0x0002
RX_ABORT                = 0x0004
RX_ADDR_LOAD            = 0x0006
RX_RESUMENR             = 0x0007
INT_MASK                = 0x0100
DRVR_INT                = 0x0200        ; Driver generated interrupt

PHY_100a                = 0x000003E0
PHY_100c                = 0x035002A8
PHY_82555_tx            = 0x015002A8
PHY_nsc_tx              = 0x5C002000
PHY_82562_et            = 0x033002A8
PHY_82562_em            = 0x032002A8
PHY_82562_ek            = 0x031002A8
PHY_82562_eh            = 0x017002A8
PHY_82552_v             = 0xd061004d
PHY_unknown             = 0xFFFFFFFF

MAC_82557_D100_A        = 0
MAC_82557_D100_B        = 1
MAC_82557_D100_C        = 2
MAC_82558_D101_A4       = 4
MAC_82558_D101_B0       = 5
MAC_82559_D101M         = 8
MAC_82559_D101S         = 9
MAC_82550_D102          = 12
MAC_82550_D102_C        = 13
MAC_82551_E             = 14
MAC_82551_F             = 15
MAC_82551_10            = 16
MAC_unknown             = 0xFF

SCB_STATUS_RUS          = 111100b       ; RU Status
RU_STATUS_IDLE          = 0000b shl 2
RU_STATUS_SUSPENDED     = 0001b shl 2
RU_STATUS_NO_RESOURCES  = 0010b shl 2
RU_STATUS_READY         = 0100b shl 2
SCB_STATUS_FCP          = 1 shl 8       ; Flow Control Pause
SCB_STATUS_SWI          = 1 shl 10      ; Software Interrupt
SCB_STATUS_MDI          = 1 shl 11      ; MDI read/write complete
SCB_STATUS_RNR          = 1 shl 12      ; Receiver Not Ready
SCB_STATUS_CNA          = 1 shl 13      ; Command unit Not Active
SCB_STATUS_FR           = 1 shl 14      ; Frame received
SCB_STATUS_CX_TNO       = 1 shl 15      ; Command finished / Transmit Not Okay

struct  rxfd

        status          dw ?
        command         dw ?
        link            dd ?
        rx_buf_addr     dd ?
        count           dw ?
        size            dw ?
        packet          rb 1500

ends

RXFD_STATUS_RC          = 1 shl 0       ; Receive collision
RXFD_STATUS_IA          = 1 shl 1       ; IA mismatch
RXFD_STATUS_NA          = 1 shl 2       ; No address match
RXFD_STATUS_RE          = 1 shl 4       ; Receive Error
RXFD_STATUS_TL          = 1 shl 5       ; Type/length
RXFD_STATUS_FS          = 1 shl 7       ; Frame too short
RXFD_STATUS_DMA_FAIL    = 1 shl 8       ; DMA overrun failure
RXFD_STATUS_NR          = 1 shl 9       ; Out of buffer space; no resources
RXFD_STATUS_MISA        = 1 shl 10      ; Alignment error
RXFD_STATUS_CRC_ERR     = 1 shl 11      ; CRC error in aligned frame
RXFD_STATUS_OK          = 1 shl 13      ; Frame received and stored
RXFD_STATUS_C           = 1 shl 15      ; Completion of frame reception

RXFD_CMD_SF             = 1 shl 3
RXFD_CMD_H              = 1 shl 4       ; Header RFD
RXFD_CMD_SUSPEND        = 1 shl 14      ; Suspends RU after receiving the frame
RXFD_CMD_EL             = 1 shl 15      ; Last RFD in RFA

struct  txfd

        status          dw ?
        command         dw ?
        link            dd ?
        desc_addr       dd ?
        count           dd ?

        buf_addr        dd ?
        buf_size        dd ?
        virt_addr       dd ?
                        dd ?            ; alignment

ends

TXFD_CMD_IA             = 1 shl 0
TXFD_CMD_CFG            = 1 shl 1
TXFD_CMD_TX             = 1 shl 2
TXFD_CMD_TX_FLEX        = 1 shl 3
TXFD_CMD_SUSPEND        = 1 shl 14

struc   confcmd {

        .status         dw ?
        .command        dw ?
        .link           dd ?
        .data           rb 64

}

struc   lstats {

        .tx_good_frames         dd ?
        .tx_coll16_errs         dd ?
        .tx_late_colls          dd ?
        .tx_underruns           dd ?
        .tx_lost_carrier        dd ?
        .tx_deferred            dd ?
        .tx_one_colls           dd ?
        .tx_multi_colls         dd ?
        .tx_total_colls         dd ?

        .rx_good_frames         dd ?
        .rx_crc_errs            dd ?
        .rx_align_errs          dd ?
        .rx_resource_errs       dd ?
        .rx_overrun_errs        dd ?
        .rx_colls_errs          dd ?
        .rx_runt_errs           dd ?

}

struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        rx_desc         dd ?
        cur_tx          dd ?
        last_tx         dd ?
        link_timer      dd ?
        ee_bus_width    db ?
        irq_line        db ?

        rb 0x100 - ($ and 0xff) ; align 256
        tx_ring         rb TX_RING_SIZE*sizeof.txfd

        rb 0x100 - ($ and 0xff) ; align 256
        confcmd         confcmd

        rb 0x100 - ($ and 0xff) ; align 256
        lstats          lstats

ends

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, state:dword

        cmp [state], 1
        jne .exit

  .entry:

        DEBUGF 1,"Loading driver\n"
        invoke  RegService, my_service, service_proc
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

        allocate_and_clear ebx, sizeof.device, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], unload
        mov     [ebx + device.name], devicename

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

        DEBUGF  1,"Hooking into device, devfn:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:2,[ebx + device.pci_bus]:2,[ebx + device.irq_line]:2,[ebx + device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                 ; If an error occured, exit

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .err

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
  .err:
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


;-------------
;
; Probe
;
;-------------

align 4
probe:

        DEBUGF  1,"Probing\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER or PCI_CMD_PIO
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

;---------------------------
; First, identify the device

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.vendor_id ; get device/vendor id

        DEBUGF  1,"Vendor_id=0x%x\n", ax
        cmp     ax, 0x8086
        jne     .notfound
        shr     eax, 16

        DEBUGF  1,"Device_id=0x%x\n", ax
        mov     ecx, DEVICE_IDs
        mov     edi, device_id_list
        repne   scasw
        jne     .notfound
        jmp     .found

  .notfound:
        DEBUGF  2,"Unsupported device!\n"
        or      eax, -1
        ret

  .found:



;----------
;
;  Reset
;
;----------

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

        DEBUGF  1,"Resetting\n"

;----------------
; Selective reset

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_EEPROM
        mov     eax, PORT_SELECTIVE_RESET
        out     dx, eax

        mov     esi, 10
        invoke  Sleep

;-----------
; Soft reset

        set_io  [ebx + device.io_addr], REG_PORT
        mov     eax, PORT_SOFT_RESET
        out     dx, eax

        mov     esi, 10
        invoke  Sleep

;-------------
; Read PHY IDs

        mov     cx, 1
        mov     dx, MII_PHYSID1
        call    mdio_read
        DEBUGF  1, "PHY ID1: 0x%x\n", ax

        mov     cx, 1
        mov     dx, MII_PHYSID2
        call    mdio_read
        DEBUGF  1, "PHY ID2: 0x%x\n", ax

;---------------------
; Read MAC from eeprom

        call    ee_get_width
        call    mac_read_eeprom

;---------------------------------
; Tell device where to store stats

        lea     eax, [ebx + device.lstats.tx_good_frames]      ; lstats
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_SCB_PTR
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, CU_STATSADDR or INT_MASK
        out     dx, ax
        call    cmd_wait

;------------------------
; setup RX base addr to 0

        set_io  [ebx + device.io_addr], REG_SCB_PTR
        xor     eax, eax
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, RX_ADDR_LOAD or INT_MASK
        out     dx, ax
        call    cmd_wait

;-----------------------------
; Create RX and TX descriptors

        call    init_rx_ring
        test    eax, eax
        jz      .error

        call    init_tx_ring


;---------
; Start RX

        DEBUGF  1, "Starting RX"

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_SCB_PTR
        mov     eax, [ebx + device.rx_desc]
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, RX_START or INT_MASK
        out     dx, ax
        call    cmd_wait

;----------
; Set-up TX

        set_io  [ebx + device.io_addr], REG_SCB_PTR
        xor     eax, eax
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, CU_CMD_BASE or INT_MASK
        out     dx, ax
        call    cmd_wait

;-------------------------
; Individual address setup

        mov     [ebx + device.confcmd.command], TXFD_CMD_IA or TXFD_CMD_SUSPEND
        mov     [ebx + device.confcmd.status], 0
        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        mov     [ebx + device.confcmd.link], eax
        lea     edi, [ebx + device.confcmd.data]
        lea     esi, [ebx + device.mac]
        movsd
        movsw

        set_io  [ebx + device.io_addr], REG_SCB_PTR
        lea     eax, [ebx + device.confcmd.status]
        invoke  GetPhysAddr
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, CU_START or INT_MASK
        out     dx, ax
        call    cmd_wait

;-------------
; Configure CU

        mov     [ebx + device.confcmd.command], TXFD_CMD_CFG or TXFD_CMD_SUSPEND
        mov     [ebx + device.confcmd.status], 0
        lea     eax, [ebx + device.confcmd.status]
        invoke  GetPhysAddr
        mov     [ebx + device.confcmd.link], eax

        mov     esi, confcmd_data
        lea     edi, [ebx + device.confcmd.data]
        mov     ecx, 22
        rep     movsb

        set_io  [ebx + device.io_addr], REG_SCB_PTR
        lea     eax, [ebx + device.confcmd.status]
        invoke  GetPhysAddr
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, CU_START                            ; expect Interrupts from now on
        out     dx, ax
        call    cmd_wait

; Start media check timer
        mov     [ebx + device.state], ETH_LINK_DOWN
        invoke  TimerHS, 0, 50, check_media_mii, ebx
        mov     [ebx + device.link_timer], eax

        mov     [ebx + device.mtu], 1514

        DEBUGF  1,"Reset complete\n"

        xor     eax, eax        ; indicate that we have successfully reset the card
        ret

  .error:
        or      eax, -1
        ret


align 4
init_rx_ring:

        DEBUGF  1,"Creating ring\n"

;---------------------
; build rxfd structure

        invoke  NetAlloc, 2000
        test    eax, eax
        jz      .out_of_mem
        mov     [ebx + device.rx_desc], eax
        mov     esi, eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + sizeof.NET_BUFF + rxfd.status], 0
        mov     [esi + sizeof.NET_BUFF + rxfd.command], RXFD_CMD_EL or RXFD_CMD_SUSPEND
        mov     [esi + sizeof.NET_BUFF + rxfd.link], eax
        mov     [esi + sizeof.NET_BUFF + rxfd.count], 0
        mov     [esi + sizeof.NET_BUFF + rxfd.size], 1528

        ret

  .out_of_mem:
        ret




align 4
init_tx_ring:

        DEBUGF  1,"Creating TX ring\n"

        lea     esi, [ebx + device.tx_ring]
        mov     eax, esi
        invoke  GetPhysAddr
        mov     ecx, TX_RING_SIZE
  .next_desc:
        mov     [esi + txfd.status], 0
        mov     [esi + txfd.command], 0
        lea     edx, [eax + txfd.buf_addr]
        mov     [esi + txfd.desc_addr], edx
        add     eax, sizeof.txfd
        mov     [esi + txfd.link], eax
        mov     [esi + txfd.count], 0x01208000          ; One buffer, 0x20 bytes of transmit threshold, end of frame
        add     esi, sizeof.txfd
        dec     ecx
        jnz     .next_desc

        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        mov     dword[ebx + device.tx_ring + sizeof.txfd*(TX_RING_SIZE-1) + txfd.link], eax

        mov     [ebx + device.cur_tx], 0
        mov     [ebx + device.last_tx], 0

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

        ; Get current TX descriptor
        mov     edi, [ebx + device.cur_tx]
        mov     eax, sizeof.txfd
        mul     edi
        lea     edi, [ebx + device.tx_ring + eax]

        ; Check if current descriptor is free or still in use
        cmp     [edi + txfd.status], 0
        jne     .overrun

        ; Fill in status and command values
        mov     [edi + txfd.status], 0
        mov     [edi + txfd.command], TXFD_CMD_SUSPEND or TXFD_CMD_TX or TXFD_CMD_TX_FLEX
        mov     [edi + txfd.count], 0x01208000

        ; Fill in buffer address and size
        mov     [edi + txfd.virt_addr], esi
        mov     eax, esi
        add     eax, [esi + NET_BUFF.offset]
        push    edi
        invoke  GetPhysAddr
        pop     edi
        mov     [edi + txfd.buf_addr], eax
        mov     ecx, [esi + NET_BUFF.length]
        mov     [edi + txfd.buf_size], ecx

        ; Inform device of the new/updated transmit descriptor
        mov     eax, edi
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_SCB_PTR
        out     dx, eax

        ; Start the transmit
        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, CU_START
        out     dx, ax

        ; Update stats
        inc     [ebx + device.packets_tx]
        add     dword[ebx + device.bytes_tx], ecx
        adc     dword[ebx + device.bytes_tx + 4], 0

        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], TX_RING_SIZE - 1

        ; Wait for command to complete
        call    cmd_wait

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

;        set_io  [ebx + device.io_addr], 0      ; REG_SCB_STATUS = 0
        set_io  [ebx + device.io_addr], REG_SCB_STATUS
        in      ax, dx
        test    ax, ax
        jz      .nothing
        out     dx, ax                          ; send it back to ACK

        DEBUGF  1,"Status: %x\n", ax

        test    ax, SCB_STATUS_FR               ; did we receive a frame?
        jz      .no_rx

        push    ax

        DEBUGF  1,"Receiving\n"

        push    ebx
  .rx_loop:
        pop     ebx

        mov     esi, [ebx + device.rx_desc]
        test    [esi + sizeof.NET_BUFF + rxfd.status], RXFD_STATUS_C            ; Completed?
        jz      .no_rx_
        test    [esi + sizeof.NET_BUFF + rxfd.status], RXFD_STATUS_OK           ; OK?
        jz      .not_ok

        DEBUGF  1,"rxfd status=0x%x\n", [esi + sizeof.NET_BUFF + rxfd.status]:4

        movzx   ecx, [esi + sizeof.NET_BUFF + rxfd.count]
        and     ecx, 0x3fff

        push    ebx
        push    .rx_loop
        push    esi
        mov     [esi + NET_BUFF.length], ecx
        mov     [esi + NET_BUFF.device], ebx
        mov     [esi + NET_BUFF.offset], NET_BUFF.data + rxfd.packet

; Update stats
        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0
        inc     dword [ebx + device.packets_rx]

; allocate new descriptor

        invoke  NetAlloc, 2000
        test    eax, eax
        jz      .out_of_mem
        mov     [ebx + device.rx_desc], eax
        mov     esi, eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + sizeof.NET_BUFF + rxfd.status], 0
        mov     [esi + sizeof.NET_BUFF + rxfd.command], RXFD_CMD_EL or RXFD_CMD_SUSPEND
        mov     [esi + sizeof.NET_BUFF + rxfd.link], eax
        mov     [esi + sizeof.NET_BUFF + rxfd.count], 0
        mov     [esi + sizeof.NET_BUFF + rxfd.size], 1528

; restart RX

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_SCB_PTR
;        mov     eax, [ebx + device.rx_desc]
;        invoke  GetPhysAddr
;        add     eax, NET_BUFF.data
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, RX_START
        out     dx, ax
        call    cmd_wait
  .out_of_mem:

; Hand the frame over to the kernel
        jmp     [EthInput]

  .not_ok:
; Reset the FD
        mov     [esi + sizeof.NET_BUFF + rxfd.status], 0
        mov     [esi + sizeof.NET_BUFF + rxfd.command], RXFD_CMD_EL or RXFD_CMD_SUSPEND
        mov     [esi + sizeof.NET_BUFF + rxfd.count], 0

; Restart RX
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_SCB_PTR
        mov     eax, esi
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_SCB_CMD
        mov     ax, RX_START
        out     dx, ax
        call    cmd_wait

        push    ebx
        jmp     .rx_loop

  .no_rx_:
        DEBUGF  1, "no more data\n"
        pop     ax

  .no_rx:

        test    ax, SCB_STATUS_CNA
        jz      .no_tx
        DEBUGF  1, "Command completed\n"

        push    eax
  .loop_tx:
        mov     edi, [ebx + device.last_tx]
        mov     eax, sizeof.txfd
        mul     eax
        lea     edi, [ebx + device.tx_ring + eax]

        cmp     [edi + txfd.status], 0
        je      .tx_done

        cmp     [edi + txfd.virt_addr], 0
        je      .tx_done

        DEBUGF  1,"Freeing buffer 0x%x\n", [edi + txfd.virt_addr]

        push    [edi + txfd.virt_addr]
        mov     [edi + txfd.virt_addr], 0
        invoke  NetFree

        inc     [ebx + device.last_tx]
        and     [ebx + device.last_tx], TX_RING_SIZE - 1

        jmp     .loop_tx
  .tx_done:
        pop     eax
  .no_tx:

        test    ax, RU_STATUS_NO_RESOURCES
        jz      .not_out_of_resources

        DEBUGF  2, "Out of resources!\n"

  .not_out_of_resources:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret

  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret



align 16
proc check_media_mii stdcall dev:dword

        spin_lock_irqsave

        mov     ebx, [dev]

        mov     ecx, 1  ;;;
        mov     edx, MII_BMSR
        call    mdio_read

        mov     ecx, 1  ;;;
        mov     edx, MII_BMSR
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

        mov     ecx, 1  ;;;
        mov     edx, MII_ADVERTISE
        call    mdio_read
        mov     esi, eax

        mov     ecx, 1  ;;;
        mov     edx, MII_LPA
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


align 4
cmd_wait:

        in      al, dx
        test    al, al
        jnz     cmd_wait

        ret



align 4
ee_read:        ; esi = address to read

        DEBUGF  1,"Eeprom read from 0x%x\n", esi

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_EEPROM

;-----------------------------------------------------
; Prepend start bit + read opcode to the address field
; and shift it to the very left bits of esi

        mov     cl, 29
        sub     cl, [ebx + device.ee_bus_width]
        shl     esi, cl
        or      esi, EE_READ shl 29

        movzx   ecx, [ebx + device.ee_bus_width]
        add     ecx, 3

        mov     ax, 0x4800 + EE_CS
        out     dx, ax
        call    udelay

;-----------------------
; Write this to the chip

  .loop:
        mov     al, EE_CS
        shl     esi, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        call    udelay

        or      al, EE_SK
        out     dx, al
        call    udelay

        loop    .loop

;------------------------------
; Now read the data from eeprom

        xor     esi, esi
        mov     ecx, 16

  .loop2:
        shl     esi, 1
        mov     al, EE_CS
        out     dx, al
        call    udelay

        or      al, EE_SK
        out     dx, al
        call    udelay

        in      al, dx
        test    al, EE_DO
        jz      @f
        inc     esi
       @@:

        loop    .loop2

;-----------------------
; de-activate the eeprom

        xor     al, al
        out     dx, al

        DEBUGF  1,"0x%x\n", esi:4
        ret



align 4
ee_write:       ; esi = address to write to, di = data

        DEBUGF  1,"Eeprom write 0x%x to 0x%x\n", di, esi

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_EEPROM

;-----------------------------------------------------
; Prepend start bit + write opcode to the address field
; and shift it to the very left bits of esi

        mov     cl, 29
        sub     cl, [ebx + device.ee_bus_width]
        shl     esi, cl
        or      esi, EE_WRITE shl 29

        movzx   ecx, [ebx + device.ee_bus_width]
        add     ecx, 3

        mov     ax, 0x4800 + EE_CS       ; enable chip
        out     dx, ax

;-----------------------
; Write this to the chip

  .loop:
        mov     al, EE_CS
        shl     esi, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        call    udelay

        or      al, EE_SK
        out     dx, al
        call    udelay

        loop    .loop

;-----------------------------
; Now write the data to eeprom

        mov     ecx, 16

  .loop2:
        mov     al, EE_CS
        shl     di, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        call    udelay

        or      al, EE_SK
        out     dx, al
        call    udelay

        loop    .loop2

;-----------------------
; de-activate the eeprom

        xor     al, al
        out     dx, al


        ret



align 4
ee_get_width:

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_EEPROM

        mov     ax, 0x4800 + EE_CS      ; activate eeprom
        out     dx, ax
        call    udelay

        mov     si, EE_READ shl 13
        xor     ecx, ecx
  .loop:
        mov     al, EE_CS
        shl     si, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, ax
        call    udelay

        or      al, EE_SK
        out     dx, ax
        call    udelay

        inc     ecx

        cmp     ecx, 15
        jae     .give_up

        in      al, dx
        test    al, EE_DO
        jnz     .loop

        xor     al, al
        out     dx, al                  ; de-activate eeprom

        sub     cl, 3                   ; dont count the opcode bits
        mov     [ebx + device.ee_bus_width], cl
        DEBUGF  1, "Eeprom width=%u bit\n", ecx

        ret

  .give_up:
        DEBUGF  2, "Eeprom not found!\n"
        xor     al, al
        out     dx, al                  ; de-activate eeprom

        ret


; Wait a minimum of 2µs
udelay:
        pusha
        mov     esi, 1
        invoke  Sleep
        popa

        ret



; cx = phy addr
; dx = phy reg addr

; ax = data

align 4
mdio_read:

        DEBUGF  1,"MDIO read\n"

        shl     ecx, 21                 ; PHY addr
        mov     eax, ecx
        shl     edx, 16                 ; PHY reg addr
        or      eax, edx
        or      eax, 10b shl 26         ; read opcode

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_MDI_CTRL
        out     dx, eax

  .wait:
        call    udelay
        in      eax, dx
        test    eax, 1 shl 28           ; ready bit
        jz      .wait

        ret



; ax = data
; cx = phy addr
; dx = phy reg addr

; ax = data

align 4
mdio_write:

        DEBUGF  1,"MDIO write\n"

        and     eax, 0xffff

        shl     ecx, 21                 ; PHY addr
        shl     edx, 16                 ; PHY reg addr

        or      eax, ecx
        or      eax, edx
        or      eax, 01b shl 26         ; write opcode

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_MDI_CTRL
        out     dx, eax

  .wait:
        call    udelay
        in      eax, dx
        test    eax, 1 shl 28           ; ready bit
        jz      .wait

        ret


align 4
mac_read_eeprom:

        mov     esi, 0
        call    ee_read
        mov     word[ebx + device.mac], si

        mov     esi, 1
        call    ee_read
        mov     word[ebx + device.mac+2], si

        mov     esi, 2
        call    ee_read
        mov     word[ebx + device.mac+4], si


        ret


; End of code


data fixups
end data

include '../peimport.inc'

my_service      db 'I8255X', 0                    ; max 16 chars include zero
devicename      db 'Intel Etherexpress pro/100', 0

confcmd_data    db 22, 0x08, 0, 0, 0, 0x80, 0x32, 0x03, 1
                db 0, 0x2e, 0, 0x60, 0, 0xf2, 0x48, 0, 0x40, 0xf2
                db 0x80, 0x3f, 0x05                                     ; 22 bytes total


device_id_list:
dw 0x1029
dw 0x1030
dw 0x1031
dw 0x1032
dw 0x1033
dw 0x1034
dw 0x1038
dw 0x1039
dw 0x103A
dw 0x103B
dw 0x103C
dw 0x103D
dw 0x103E
dw 0x1050
dw 0x1051
dw 0x1052
dw 0x1053
dw 0x1054
dw 0x1055
dw 0x1056
dw 0x1057
dw 0x1059
dw 0x1064
dw 0x1065
dw 0x1066
dw 0x1067
dw 0x1068
dw 0x1069
dw 0x106A
dw 0x106B
dw 0x1091
dw 0x1092
dw 0x1093
dw 0x1094
dw 0x1095
dw 0x10fe
dw 0x1209
dw 0x1229
dw 0x2449
dw 0x2459
dw 0x245D
dw 0x27DC

DEVICE_IDs = ($ - device_id_list) / 2

include_debug_strings                           ; All data wich FDO uses will be included here

align 4
devices         dd 0                              ; number of currently running devices
device_list     rd MAX_DEVICES                    ; This list contains all pointers to device structures the driver is handling

