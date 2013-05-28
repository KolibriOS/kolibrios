;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Broadcom NetXtreme 57xx driver for KolibriOS                   ;;
;;                                                                 ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;; Broadcom's programmers's manual for the BCM57xx                 ;;
;; http://www.broadcom.com/collateral/pg/57XX-PG105-R.pdf          ;;
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


include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../netdrv.inc'

public START
public service_proc
public version


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





align 4
reset:

        DEBUGF  1,"Reset\n"

        movzx   eax, [device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

        call    read_mac

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], 1514

; Set link state to unknown
        mov     [device.state], ETH_LINK_UNKOWN

        ret




align 4
read_mac:

        DEBUGF  1,"Read MAC\n"

        mov     esi, [device.mmio_addr]
        lea     edi, [device.mac]
        movsd
        movsw

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

;        mov     edi, [device.mmio_addr]
;        mov     eax, [edi + REG_ICR]
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

        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret




; End of code

section '.data' data readable writable align 16
align 4

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'BCM57XX',0                   ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling


