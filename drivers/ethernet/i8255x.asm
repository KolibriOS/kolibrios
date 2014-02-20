;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
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

; TODO: use separate descriptors in memory instead of placing them in front of packets!


format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 16

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1             ; 1 = verbose, 2 = errors only

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv_pe.inc'

; Serial EEPROM

EE_SK           = 1 shl 0      ; serial clock
EE_CS           = 1 shl 1      ; chip select
EE_DI           = 1 shl 2      ; data in
EE_DO           = 1 shl 3      ; data out
EE_MASK         = EE_SK + EE_CS + EE_DI + EE_DO

; opcodes, first bit is start bit and must be 1
EE_READ         = 110b
EE_WRITE        = 101b
EE_ERASE        = 111b

; The SCB accepts the following controls for the Tx and Rx units:

CU_START        = 0x0010
CU_RESUME       = 0x0020
CU_STATSADDR    = 0x0040
CU_SHOWSTATS    = 0x0050        ; Dump statistics counters.
CU_CMD_BASE     = 0x0060        ; Base address to add CU commands.
CU_DUMPSTATS    = 0x0070        ; Dump then reset stats counters.

RX_START        = 0x0001
RX_RESUME       = 0x0002
RX_ABORT        = 0x0004
RX_ADDR_LOAD    = 0x0006
RX_RESUMENR     = 0x0007
INT_MASK        = 0x0100
DRVR_INT        = 0x0200        ; Driver generated interrupt

CmdIASetup      = 0x0001
CmdConfigure    = 0x0002
CmdTx           = 0x0004
CmdTxFlex       = 0x0008
Cmdsuspend      = 0x4000

reg_scb_status  = 0
reg_scb_cmd     = 2
reg_scb_ptr     = 4
reg_port        = 8
reg_eeprom      = 14
reg_mdi_ctrl    = 16

phy_100a        = 0x000003E0
phy_100c        = 0x035002A8
phy_82555_tx    = 0x015002A8
phy_nsc_tx      = 0x5C002000
phy_82562_et    = 0x033002A8
phy_82562_em    = 0x032002A8
phy_82562_ek    = 0x031002A8
phy_82562_eh    = 0x017002A8
phy_82552_v     = 0xd061004d
phy_unknown     = 0xFFFFFFFF

mac_82557_D100_A        = 0
mac_82557_D100_B        = 1     
mac_82557_D100_C        = 2     
mac_82558_D101_A4       = 4     
mac_82558_D101_B0       = 5     
mac_82559_D101M         = 8     
mac_82559_D101S         = 9     
mac_82550_D102          = 12    
mac_82550_D102_C        = 13    
mac_82551_E             = 14    
mac_82551_F             = 15    
mac_82551_10            = 16    
mac_unknown             = 0xFF

struct  rxfd

        status          dw ?
        command         dw ?
        link            dd ?
        rx_buf_addr     dd ?
        count           dw ?
        size            dw ?
        packet          rb 1500

ends

struc   txfd {

        .status         dw ?
        .command        dw ?
        .link           dd ?
        .desc_addr      dd ?
        .count          dd ?

        .buf_addr0      dd ?
        .buf_size0      dd ?

}

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
        last_tx_buffer  dd ?
        ee_bus_width    db ?
        irq_line        db ?

        rb 0x100 - ($ and 0xff) ; align 256
        txfd            txfd

        rb 0x100 - ($ and 0xff) ; align 256
        confcmd         confcmd

        rb 0x100 - ($ and 0xff) ; align 256
        lstats          lstats

ends

macro delay {
        push    eax
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        in      ax, dx
        pop     eax
}

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

        pushf
        cli                     ; disable ints untilm initialisation is done

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        popf

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
        ; TODO: (in this particular order)
        ;
        ; - Stop the device
        ; - Detach int handler
        ; - Remove device from local list (device_list)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax,-1

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
        or      al, PCI_CMD_MASTER
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

        call    ee_get_width
        call    MAC_read_eeprom

        ;;; TODO: detect phy



;----------
;
;  Reset
;
;----------

align 4
reset:

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

;---------------
; reset the card

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_port
        xor     eax, eax        ; Software Reset
        out     dx, eax

        mov     esi, 10
        invoke  Sleep           ; Give the card time to warm up.

;---------------------------------
; Tell device where to store stats

        lea     eax, [ebx + device.lstats.tx_good_frames]      ; lstats
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_scb_ptr
        out     dx, eax

        mov     ax, INT_MASK + CU_STATSADDR
        set_io  [ebx + device.io_addr], reg_scb_cmd
        out     dx, ax
        call    cmd_wait

;-----------------
; setup RX

        set_io  [ebx + device.io_addr], reg_scb_ptr
        xor     eax, eax
        out     dx, eax

        set_io  [ebx + device.io_addr], reg_scb_cmd
        mov     ax, INT_MASK + RX_ADDR_LOAD
        out     dx, ax
        call    cmd_wait

;-----------------------------
; Create RX and TX descriptors

        call    create_ring

; RX start

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_scb_ptr
        mov     eax, [ebx + device.rx_desc]
        invoke  GetPhysAddr
        out     dx, eax

        mov     ax, INT_MASK + RX_START
        set_io  [ebx + device.io_addr], reg_scb_cmd
        out     dx, ax
        call    cmd_wait

; Set-up TX

        set_io  [ebx + device.io_addr], reg_scb_ptr
        xor     eax, eax
        out     dx, eax

        set_io  [ebx + device.io_addr], reg_scb_cmd
        mov     ax, INT_MASK + CU_CMD_BASE
        out     dx, ax
        call    cmd_wait

;  --------------------

        mov     [ebx + device.confcmd.command], CmdConfigure + Cmdsuspend
        mov     [ebx + device.confcmd.status], 0
        lea     eax, [ebx + device.txfd.status]
        invoke  GetPhysAddr
        mov     [ebx + device.confcmd.link], eax

        mov     esi, confcmd_data
        lea     edi, [ebx + device.confcmd.data]
        mov     ecx, 22
        rep     movsb

        mov     byte[ebx + device.confcmd.data + 1], 0x88  ; fifo of 8 each
        mov     byte[ebx + device.confcmd.data + 4], 0
        mov     byte[ebx + device.confcmd.data + 5], 0x80
        mov     byte[ebx + device.confcmd.data + 15], 0x48
        mov     byte[ebx + device.confcmd.data + 19], 0x80
        mov     byte[ebx + device.confcmd.data + 21], 0x05

        mov     [ebx + device.txfd.command], CmdIASetup
        mov     [ebx + device.txfd.status], 0
        lea     eax, [ebx + device.confcmd.status]
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.link], eax

;;; copy in our MAC

        lea     edi, [ebx + device.txfd.desc_addr]
        lea     esi, [ebx + device.mac]
        movsd
        movsw

        set_io  [ebx + device.io_addr], reg_scb_ptr
        lea     eax, [ebx + device.txfd.status]
        invoke  GetPhysAddr
        out     dx, eax

; Start CU & enable ints

        set_io  [ebx + device.io_addr], reg_scb_cmd
        mov     ax, CU_START
        out     dx, ax
        call    cmd_wait

;-----------------------
; build txfd structure (again!)

        lea     eax, [ebx + device.txfd.status]
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.link], eax
        mov     [ebx + device.txfd.count], 0x02208000
        lea     eax, [ebx + device.txfd.buf_addr0]
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.desc_addr], eax

; Indicate that we have successfully reset the card

        DEBUGF  1,"Reset complete\n"

        mov     [ebx + device.mtu], 1514

; Set link state to unknown
        mov     [ebx + device.state], ETH_LINK_UNKNOWN

        xor     eax, eax        ; indicate that we have successfully reset the card
        ret


align 4
create_ring:

        DEBUGF  1,"Creating ring\n"

;---------------------
; build rxfd structure

        invoke  KernelAlloc, 2000
        mov     [ebx + device.rx_desc], eax
        mov     esi, eax
        invoke  GetPhysAddr
        mov     [esi + rxfd.status], 0x0000
        mov     [esi + rxfd.command], 0x0000
        mov     [esi + rxfd.link], eax
        mov     [esi + rxfd.count], 0
        mov     [esi + rxfd.size], 1528

;-----------------------
; build txfd structure

        lea     eax, [ebx + device.txfd.status]
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.link], eax
        mov     [ebx + device.txfd.count], 0x02208000
        lea     eax, [ebx + device.txfd.buf_addr0]
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.desc_addr], eax

        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc transmit stdcall bufferptr, buffersize

        pushf
        cli

        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [buffersize]
        mov     eax, [bufferptr]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [buffersize], 1514
        ja      .fail
        cmp     [buffersize], 60
        jb      .fail

        ;;; TODO: check if current descriptor is in use
        ; fill in buffer address and size
        mov     [ebx + device.last_tx_buffer], eax
        invoke  GetPhysAddr
        mov     [ebx + device.txfd.buf_addr0], eax
        mov     ecx, [buffersize]
        mov     [ebx + device.txfd.buf_size0], ecx

        mov     [ebx + device.txfd.status], 0
        mov     [ebx + device.txfd.command], Cmdsuspend + CmdTx + CmdTxFlex + 1 shl 15 ;;; EL bit
 ;       mov     [txfd.count], 0x02208000   ;;;;;;;;;;;

        ; Inform device of the new/updated transmit descriptor
        lea     eax, [ebx + device.txfd.status]
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_scb_ptr
        out     dx, eax

        ; Start the transmit
        mov     ax, CU_START
        set_io  [ebx + device.io_addr], reg_scb_cmd
        out     dx, ax
        call    cmd_wait

; Update stats
        inc     [ebx + device.packets_tx]
        mov     ecx, [buffersize]
        add     dword[ebx + device.bytes_tx], ecx
        adc     dword[ebx + device.bytes_tx + 4], 0

        DEBUGF  1,"Transmit OK\n"
        popf
        xor     eax, eax
        ret

  .fail:
        invoke  KernelFree, [bufferptr]
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

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

;        set_io  [ebx + device.io_addr], 0              ; reg_scb_status = 0
        set_io  [ebx + device.io_addr], reg_scb_status
        in      ax, dx
        out     dx, ax                              ; send it back to ACK
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                                         ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:

        DEBUGF  1,"Device: %x Status: %x\n", ebx, ax

        test    ax, 1 shl 14    ; did we receive a frame?
        jz      .no_rx

        push    ax

        DEBUGF  1,"Receiving\n"

        push    ebx
  .rx_loop:
        pop     ebx

        mov     esi, [ebx + device.rx_desc]
        cmp     [esi + rxfd.status], 0        ; we could also check bits C and OK (bit 15 and 13)
        je      .nodata

        DEBUGF  1,"rxfd status=0x%x\n", [esi + rxfd.status]:4

        movzx   ecx, [esi + rxfd.count]
        and     ecx, 0x3fff

        push    ebx
        push    .rx_loop
        push    ecx
        add     esi, rxfd.packet
        push    esi

; Update stats
        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0
        inc     dword [ebx + device.packets_rx]

; allocate new descriptor

        invoke  KernelAlloc, 2000
        mov     [ebx + device.rx_desc], eax
        mov     esi, eax
        invoke  GetPhysAddr
        mov     [esi + rxfd.status], 0x0000
        mov     [esi + rxfd.command], 0xc000    ; End of list + Suspend
        mov     [esi + rxfd.link], eax
        mov     [esi + rxfd.count], 0
        mov     [esi + rxfd.size], 1528

; restart RX

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_scb_ptr
;        lea     eax, [ebx + device.rx_desc]
;        invoke  GetPhysAddr
        out     dx, eax

        set_io  [ebx + device.io_addr], reg_scb_cmd
        mov     ax, RX_START
        out     dx, ax
        call    cmd_wait

; And give packet to kernel
        jmp     [Eth_input]

  .nodata:
        DEBUGF  1, "no more data\n"
        pop     ax

  .no_rx:

; Cleanup after TX
        cmp     [ebx + device.txfd.status], 0
        je      .done
        cmp     [ebx + device.last_tx_buffer], 0
        je      .done
        push    ax
        DEBUGF  1, "Removing packet 0x%x from RAM!\n", [ebx + device.last_tx_buffer]
        invoke  KernelFree, [ebx + device.last_tx_buffer]
        mov     [ebx + device.last_tx_buffer], 0
        pop     ax

  .done:
        and     ax, 00111100b
        cmp     ax, 00001000b
        jne     .fail

        DEBUGF  1, "out of resources!\n"
; Restart the RX

; allocate new descriptor

        invoke  KernelAlloc, 2000
        mov     [ebx + device.rx_desc], eax
        mov     esi, eax
        invoke  GetPhysAddr
        mov     [esi + rxfd.status], 0x0000
        mov     [esi + rxfd.command], 0xc000    ; End of list + Suspend
        mov     [esi + rxfd.link], eax
        mov     [esi + rxfd.count], 0
        mov     [esi + rxfd.size], 1528

; restart RX

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_scb_ptr
;        lea     eax, [ebx + device.rx_desc]
;        invoke  GetPhysAddr
        out     dx, eax

        set_io  [ebx + device.io_addr], reg_scb_cmd
        mov     ax, RX_START
        out     dx, ax
        call    cmd_wait

  .fail:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret




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
        set_io  [ebx + device.io_addr], reg_eeprom

;-----------------------------------------------------
; Prepend start bit + read opcode to the address field
; and shift it to the very left bits of esi

        mov     cl, 29
        sub     cl, [ebx + device.ee_bus_width]
        shl     esi, cl
        or      esi, EE_READ shl 29

        movzx   ecx, [ebx + device.ee_bus_width]
        add     ecx, 3

        mov     al, EE_CS
        out     dx, al
        delay

;-----------------------
; Write this to the chip

  .loop:
        mov     al, EE_CS + EE_SK
        shl     esi, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        delay

        and     al, not EE_SK
        out     dx, al
        delay

        loop    .loop

;------------------------------
; Now read the data from eeprom

        xor     esi, esi
        mov     ecx, 16

  .loop2:
        shl     esi, 1
        mov     al, EE_CS + EE_SK
        out     dx, al
        delay

        in      al, dx
        test    al, EE_DO
        jz      @f
        inc     esi
       @@:

        mov     al, EE_CS
        out     dx, al
        delay

        loop    .loop2

;-----------------------
; de-activate the eeprom

        xor     ax, ax
        out     dx, ax


        DEBUGF  1,"0x%x\n", esi:4
        ret



align 4
ee_write:       ; esi = address to write to, di = data

        DEBUGF  1,"Eeprom write 0x%x to 0x%x\n", di, esi

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_eeprom

;-----------------------------------------------------
; Prepend start bit + write opcode to the address field
; and shift it to the very left bits of esi

        mov     cl, 29
        sub     cl, [ebx + device.ee_bus_width]
        shl     esi, cl
        or      esi, EE_WRITE shl 29

        movzx   ecx, [ebx + device.ee_bus_width]
        add     ecx, 3

        mov     al, EE_CS       ; enable chip
        out     dx, al

;-----------------------
; Write this to the chip

  .loop:
        mov     al, EE_CS + EE_SK
        shl     esi, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        delay

        and     al, not EE_SK
        out     dx, al
        delay

        loop    .loop

;-----------------------------
; Now write the data to eeprom

        mov     ecx, 16

  .loop2:
        mov     al, EE_CS + EE_SK
        shl     di, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        delay

        and     al, not EE_SK
        out     dx, al
        delay

        loop    .loop2

;-----------------------
; de-activate the eeprom

        xor     al, al
        out     dx, al


        ret



align 4
ee_get_width:

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_eeprom

        mov     al, EE_CS      ; activate eeprom
        out     dx, al
        delay

        mov     si, EE_READ shl 13
        xor     ecx, ecx
  .loop:
        mov     al, EE_CS + EE_SK
        shl     si, 1
        jnc     @f
        or      al, EE_DI
       @@:
        out     dx, al
        delay

        and     al, not EE_SK
        out     dx, al
        delay

        inc     ecx

        cmp     ecx, 15
        jae     .give_up

        in      al, dx
        test    al, EE_DO
        jnz     .loop

        xor     al, al
        out     dx, al          ; de-activate eeprom

        sub     cl, 3           ; dont count the opcode bits
        mov     [ebx + device.ee_bus_width], cl
        DEBUGF  1, "Eeprom width=%u bit\n", ecx

        ret

  .give_up:
        DEBUGF  2, "Eeprom not found!\n"

        xor     al, al
        out     dx, al          ; de-activate eeprom

        ret



; cx = phy addr
; dx = phy reg addr

; ax = data

align 4
mdio_read:

        DEBUGF  1,"MDIO read\n"

        shl     ecx, 21                 ; PHY addr
        shl     edx, 16                 ; PHY reg addr

        mov     eax, ecx
        or      eax, edx
        or      eax, 10b shl 26         ; read opcode

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], reg_mdi_ctrl
        out     dx, eax

  .wait:
        delay
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
        set_io  [ebx + device.io_addr], reg_mdi_ctrl
        out     dx, eax

  .wait:
        delay
        in      eax, dx
        test    eax, 1 shl 28           ; ready bit
        jz      .wait

        ret

read_mac:

        ret



align 4
MAC_read_eeprom:

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


align 4
MAC_write:

;;;;

        ret




; End of code


data fixups
end data

include '../peimport.inc'

my_service      db 'i8255x', 0                    ; max 16 chars include zero
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

