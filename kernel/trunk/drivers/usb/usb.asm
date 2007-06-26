;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format MS COFF

API_VERSION     equ 0  ;debug

include '../proc32.inc'
include '../imports.inc'

OS_BASE         equ 0x80000000
SLOT_BASE       equ OS_BASE+0x0080000

struc UHCI
{
   .bus         dd ?
   .devfn       dd ?
   .io_base     dd ?
   .mm_base     dd ?
   .irq         dd ?
   .flags       dd ?
   .reset       dd ?
   .start       dd ?
   .stop        dd ?
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

           cmp [state], 1
           jne .exit
.entry:

     if DEBUG
           mov esi, msgInit
           call SysMsgBoardStr
     end if

           call init

           stdcall RegService, my_service, service_proc
           ret
.fail:
.exit:
           xor eax, eax
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

           mov ebx, [ioctl]
           mov eax, [ebx+io_code]
           cmp eax, SRV_GETVERSION
           jne @F

           mov eax, [ebx+output]
           cmp [ebx+out_size], 4
           jne .fail
           mov [eax], dword API_VERSION
           xor eax, eax
           ret
@@:
.fail:
           or eax, -1
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

           xor eax, eax
           mov [bus], eax
           inc eax
           call PciApi
           cmp eax, -1
           je .err

           mov [last_bus], eax

.next_bus:
           and [devfn], 0
.next_dev:
           stdcall PciRead32, [bus], [devfn], dword 0
           test eax, eax
           jz .next
           cmp eax, -1
           je .next

           mov edi, devices
@@:
           mov ebx, [edi]
           test ebx, ebx
           jz .next

           cmp eax, ebx
           je .found

           add edi, STRIDE
           jmp @B
.next:
           inc [devfn]
           cmp [devfn], 256
           jb  .next_dev
           mov eax, [bus]
           inc eax
           mov [bus], eax
           cmp eax, [last_bus]
           jna .next_bus
           xor eax, eax
           ret
.found:
           mov eax, UHCI.sizeof
           call Kmalloc
           test eax, eax
           jz .mem_fail

           mov ebx, [bus]
           mov [eax+UHCI.bus], ebx

           mov ecx, [devfn]
           mov [eax+UHCI.devfn], ecx
           ret
.mem_fail:
     if DEBUG
           mov esi, msgMemFail
           call SysMsgBoardStr
     end if
.err:
           xor eax, eax
           ret
endp

PCI_BASE     equ 0x20
USB_LEGKEY  equ 0xC0

align 4
proc init
           locals
            uhci       dd ?
           endl

           call detect
           test eax, eax
           jz .fail

           mov [uhci], eax

           stdcall PciRead32, [eax+UHCI.bus], [eax+UHCI.devfn], PCI_BASE
           and eax, 0xFFC0
           mov esi, [uhci]
           mov [esi+UHCI.io_base], eax

           stdcall uhci_reset, esi


.fail:
     if DEBUG
           mov esi, msgDevNotFound
           call SysMsgBoardStr
     end if
           ret
endp

UHCI_USBINTR          equ  4             ; interrupt register

UHCI_USBLEGSUP_RWC    equ  0x8f00        ; the R/WC bits
UHCI_USBLEGSUP_RO     equ  0x5040        ; R/O and reserved bits

UHCI_USBCMD_RUN       equ  0x0001        ; RUN/STOP bit
UHCI_USBCMD_HCRESET   equ  0x0002        ; Host Controller reset
UHCI_USBCMD_EGSM      equ  0x0008        ; Global Suspend Mode
UHCI_USBCMD_CONFIGURE equ  0x0040        ; Config Flag
UHCI_USBINTR_RESUME   equ  0x0002        ; Resume interrupt enable


align 4
proc uhci_reset stdcall, uhci:dword

           mov esi, [uhci]
           stdcall PciRead16, [esi+UHCI.bus], [esi+UHCI.devfn], USB_LEGKEY
           test eax, not (UHCI_USBLEGSUP_RO or UHCI_USBLEGSUP_RWC)
           jnz .reset

           mov edx, [esi+UHCI.io_base]
           in ax, dx
           test ax, UHCI_USBCMD_RUN
           jnz .reset

           test ax, UHCI_USBCMD_CONFIGURE
           jz .reset

           test ax, UHCI_USBCMD_EGSM
           jz .reset
           ret
.reset:
           stdcall PciWrite16, [esi+UHCI.bus], [esi+UHCI.devfn], USB_LEGKEY, UHCI_USBLEGSUP_RWC

           mov edx, [esi+UHCI.io_base]
           mov ax, UHCI_USBCMD_HCRESET
           out dx, ax

           xor eax, eax
           out dx, ax
           add edx, UHCI_USBINTR
           out dx, ax
           ret
endp


DEVICE_ID    equ  0x8086;  pci device id
VENDOR_ID    equ  0x24D4;  device vendor id


;all initialized data place here

align 4
devices         dd (DEVICE_ID shl 16)+VENDOR_ID
                dd 0    ;terminator

version         dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service      db 'UHCI',0  ;max 16 chars include zero

msgInit         db 'detect hardware...',13,10,0
msgPCI          db 'PCI accsess not supported',13,10,0
msgDevNotFound  db 'device not found',13,10,0
msgMemFail      db 'Kmalloc failed', 10,10,0
;msgFail         db 'device not found',13,10,0

section '.data' data readable writable align 16

;all uninitialized data place here

