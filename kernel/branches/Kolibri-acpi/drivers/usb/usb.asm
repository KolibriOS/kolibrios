;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format MS COFF

API_VERSION     equ 0  ;debug

include '../proc32.inc'
include '../imports.inc'
include 'urb.inc'

struc UHCI
{
   .bus                dd ?
   .devfn              dd ?
   .io_base            dd ?
   .mm_base            dd ?
   .irq                dd ?
   .flags              dd ?
   .reset              dd ?
   .start              dd ?
   .stop               dd ?

   .port_c_suspend     dd ?
   .resuming_ports     dd ?
   .rh_state           dd ?
   .rh_numports        dd ?
   .is_stopped         dd ?
   .dead               dd ?

   .sizeof:
}

virtual at 0
  UHCI UHCI
end virtual

struc IOCTL
{  .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

struc TD   ;transfer descriptor
{
   .link        dd ?
   .status      dd ?
   .token       dd ?
   .buffer      dd ?

   .addr        dd ?
   .frame       dd ?
   .fd          dd ?
   .bk          dd ?
   .sizeof:
}

virtual at 0
  TD TD
end virtual

public START
public service_proc
public version

DEBUG        equ 1

DRV_ENTRY    equ 1
DRV_EXIT     equ -1
STRIDE       equ 4      ;size of row in devices table

SRV_GETVERSION  equ 0

section '.flat' code readable align 16

proc START stdcall, state:dword

        cmp     [state], 1
        jne     .exit
.entry:

     if DEBUG
        mov     esi, msgInit
        call    SysMsgBoardStr
     end if

        call    init

        stdcall RegService, my_service, service_proc
        ret
.fail:
.exit:
        xor     eax, eax
        ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

        mov     ebx, [ioctl]
        mov     eax, [ebx+io_code]
        cmp     eax, SRV_GETVERSION
        jne     @F

        mov     eax, [ebx+output]
        cmp     [ebx+out_size], 4
        jne     .fail
        mov     [eax], dword API_VERSION
        xor     eax, eax
        ret
@@:
.fail:
        or      eax, -1
        ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

align 4
proc detect
           locals
            last_bus   dd ?
            bus        dd ?
            devfn      dd ?
           endl

        xor     eax, eax
        mov     [bus], eax
        inc     eax
        call    PciApi
        cmp     eax, -1
        je      .err

        mov     [last_bus], eax

.next_bus:
        and     [devfn], 0
.next_dev:
        stdcall PciRead32, [bus], [devfn], dword 0
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        mov     edi, devices
@@:
        mov     ebx, [edi]
        test    ebx, ebx
        jz      .next

        cmp     eax, ebx
        je      .found

        add     edi, STRIDE
        jmp     @B
.next:
        inc     [devfn]
        cmp     [devfn], 256
        jb      .next_dev
        mov     eax, [bus]
        inc     eax
        mov     [bus], eax
        cmp     eax, [last_bus]
        jna     .next_bus
        xor     eax, eax
        ret
.found:
        mov     eax, UHCI.sizeof
        call    Kmalloc
        test    eax, eax
        jz      .mem_fail

        mov     ebx, [bus]
        mov     [eax+UHCI.bus], ebx

        mov     ecx, [devfn]
        mov     [eax+UHCI.devfn], ecx
        ret
.mem_fail:
     if DEBUG
        mov     esi, msgMemFail
        call    SysMsgBoardStr
     end if
.err:
        xor     eax, eax
        ret
endp

PCI_BASE    equ 0x20
USB_LEGKEY  equ 0xC0

align 4
proc init
           locals
            uhci       dd ?
           endl

        call    detect
        test    eax, eax
        jz      .fail

        mov     [uhci], eax

        stdcall PciRead32, [eax+UHCI.bus], [eax+UHCI.devfn], PCI_BASE
        and     eax, 0xFFC0
        mov     esi, [uhci]
        mov     [esi+UHCI.io_base], eax

        stdcall uhci_reset, esi

        stdcall finish_reset, [uhci]

.fail:
     if DEBUG
        mov     esi, msgDevNotFound
        call    SysMsgBoardStr
     end if
        ret
endp

UHCI_USBINTR            equ  4             ; interrupt register

UHCI_USBLEGSUP_RWC      equ  0x8f00        ; the R/WC bits
UHCI_USBLEGSUP_RO       equ  0x5040        ; R/O and reserved bits

UHCI_USBCMD_RUN         equ  0x0001        ; RUN/STOP bit
UHCI_USBCMD_HCRESET     equ  0x0002        ; Host Controller reset
UHCI_USBCMD_EGSM        equ  0x0008        ; Global Suspend Mode
UHCI_USBCMD_CONFIGURE   equ  0x0040        ; Config Flag
UHCI_USBINTR_RESUME     equ  0x0002        ; Resume interrupt enable

PORTSC0                 equ  0x10
PORTSC1                 equ  0x12


UHCI_RH_RESET           equ  0
UHCI_RH_SUSPENDED       equ  1
UHCI_RH_AUTO_STOPPED    equ  2
UHCI_RH_RESUMING        equ  3

; In this state the HC changes from running to halted
; so it can legally appear either way.
UHCI_RH_SUSPENDING      equ  4

; In the following states it's an error if the HC is halted.
; These two must come last.
UHCI_RH_RUNNING         equ 5  ; The normal state
UHCI_RH_RUNNING_NODEVS  equ 6  ; Running with no devices

UHCI_IS_STOPPED         equ 9999

align 4
proc uhci_reset stdcall, uhci:dword
        mov     esi, [uhci]
        stdcall PciRead16, [esi+UHCI.bus], [esi+UHCI.devfn], USB_LEGKEY
        test    eax, not (UHCI_USBLEGSUP_RO or UHCI_USBLEGSUP_RWC)
        jnz     .reset

        mov     edx, [esi+UHCI.io_base]
        in      ax, dx
        test    ax, UHCI_USBCMD_RUN
        jnz     .reset

        test    ax, UHCI_USBCMD_CONFIGURE
        jz      .reset

        test    ax, UHCI_USBCMD_EGSM
        jz      .reset

        add     edx, UHCI_USBINTR
        in      ax, dx
        test    ax, not UHCI_USBINTR_RESUME
        jnz     .reset
        ret
.reset:
        stdcall PciWrite16, [esi+UHCI.bus], [esi+UHCI.devfn], USB_LEGKEY, UHCI_USBLEGSUP_RWC

        mov     edx, [esi+UHCI.io_base]
        mov     ax, UHCI_USBCMD_HCRESET
        out     dx, ax

        xor     eax, eax
        out     dx, ax
        add     edx, UHCI_USBINTR
        out     dx, ax
        ret
endp

proc finish_reset stdcall, uhci:dword

        mov     esi, [uhci]
        mov     edx, [esi+UHCI.io_base]
        add     edx, PORTSC0
        xor     eax, eax
        out     dx, ax
        add     edx, (PORTSC1-PORTSC0)
        out     dx, ax

        mov     [esi+UHCI.port_c_suspend], eax
        mov     [esi+UHCI.resuming_ports], eax
        mov     [esi+UHCI.rh_state], UHCI_RH_RESET
        mov     [esi+UHCI.rh_numports], 2

        mov     [esi+UHCI.is_stopped], UHCI_IS_STOPPED
     ;      mov [ uhci_to_hcd(uhci)->state = HC_STATE_HALT;
     ;      uhci_to_hcd(uhci)->poll_rh = 0;

        mov     [esi+UHCI.dead], eax ; Full reset resurrects the controller

        ret
endp

proc insert_td stdcall, td:dword, frame:dword

        mov     edi, [td]
        mov     eax, [frame]
        and     eax, -1024
        mov     [edi+TD.frame], eax

        mov     ebx, [framelist]
        mov     edx, [dma_framelist]
        shl     eax, 5

        mov     ecx, [eax+ebx]
        test    ecx, ecx
        jz      .empty

        mov     ecx, [ecx+TD.bk]              ;last TD

        mov     edx, [ecx+TD.fd]
        mov     [edi+TD.fd], edx
        mov     [edi+TD.bk], ecx
        mov     [ecx+TD.fd], edi
        mov     [edx+TD.bk], edi

        mov     eax, [ecx+TD.link]
        mov     [edi+TD.link], eax
        mov     ebx, [edi+TD.addr]
        mov     [ecx+TD.link], ebx
        ret
.empty:
        mov     ecx, [eax+edx]
        mov     [edi+TD.link], ecx
        mov     [ebx+eax], edi
        mov     ecx, [edi+TD.addr]
        mov     [eax+edx], ecx
        ret
endp


align 4
proc usb_get_descriptor stdcall, dev:dword, type:dword, index:dword,\
                                 buf:dword, size:dword

           locals
             count        dd ?
           endl

        mov     esi, [buf]
        mov     ecx, [size]
        xor     eax, eax
        cld
        rep stosb

        mov     [count], 3
@@:
        mov     eax, [type]
        shl     eax, 8
        add     eax, [index]
        stdcall usb_control_msg, [dev], pipe, USB_REQ_GET_DESCRIPTOR, \
                USB_DIR_IN, eax,0,[buf], [size],\
                USB_CTRL_GET_TIMEOUT
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next
           jmp. ok
.next:
        dec     [count]
        jnz     @B
        mov     eax, -1
.ok:
        ret
endp

DEVICE_ID    equ  0x24D2     ;  pci device id
VENDOR_ID    equ  0x8086     ;  device vendor id
QEMU_USB     equ  0x7020

;all initialized data place here

align 4
devices         dd (DEVICE_ID shl 16)+VENDOR_ID
                dd (QEMU_USB  shl 16)+VENDOR_ID
                dd 0      ;terminator

version         dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service      db 'UHCI',0  ;max 16 chars include zero

msgInit         db 'detect hardware...',13,10,0
msgPCI          db 'PCI accsess not supported',13,10,0
msgDevNotFound  db 'device not found',13,10,0
msgMemFail      db 'Kmalloc failed', 10,10,0
;msgFail         db 'device not found',13,10,0

section '.data' data readable writable align 16

;all uninitialized data place here

