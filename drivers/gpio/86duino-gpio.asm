;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2015-2017. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1             ; 1 = verbose, 2 = errors only


        API_VERSION             = 0  ;debug

        STRIDE                  = 4      ;size of row in devices table

        SRV_GETVERSION          = 0

section '.flat' code readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../peimport.inc'
include '../fdo.inc'

GPIO_PORT_CONFIG_ADDR = 0xF100
GPIO_DATA_ADDR = 0xF200
ADC_ADDR = 0xFE00

proc START c, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .exit
.entry:

        push    esi
        DEBUGF  1,"Loading vortex86EX GPIO driver\n"
        call    detect
        pop     esi
        test    eax, eax
        jz      .fail

; Set crossbar base address register in southbridge
        invoke  PciWrite16, [bus], [dev], 0x64, 0x0A00 or 1

; Set GPIO base address register in southbridge
        invoke  PciWrite16, [bus], [dev], 0x62, GPIO_PORT_CONFIG_ADDR or 1

        DEBUGF  1,"Setting up ADC\n"

; Enable ADC
        invoke  PciRead32, [bus], [dev], 0xBC
        and     eax, not (1 shl 28)
        invoke  PciWrite32, [bus], [dev], 0xBC, eax

; Set ADC base address
        mov     ebx, [dev]
        inc     ebx
        invoke  PciRead16, [bus], ebx, 0xDE
        or      ax, 0x02
        invoke  PciWrite16, [bus], ebx, 0xDE, eax

        invoke  PciWrite32, [bus], ebx, 0xE0, 0x00500000 or ADC_ADDR

; set up ADC
        mov     dx, ADC_ADDR + 1
        xor     al, al
        out     dx, al

; Empty FIFO
  @@:
        mov     dx, ADC_ADDR + 2        ; Status register
        in      al, dx
        test    al, 0x01                ; FIFO ready
        jz      @f
        mov     dx, ADC_ADDR + 4
        in      ax, dx
        jmp     @r
  @@:

; Enable GPIO0-9
        mov     dx, GPIO_PORT_CONFIG_ADDR + 0  ; General-Purpose I/O Data & Direction Decode Enable
        mov     eax, 0x000001ff
        out     dx, eax

        mov     ecx, 10                 ; 10 GPIO ports total
        mov     dx, GPIO_PORT_CONFIG_ADDR + 4  ; General-Purpose I/O Port0 Data & Direction Decode Address
        mov     ax, GPIO_DATA_ADDR
  .gpio_init:
; Set GPIO data port base address
        out     dx, ax
        add     ax, 2
        add     dx, 2
; Set GPIO direction base address
        out     dx, ax
        add     ax, 2
        add     dx, 2
; loop
        dec     ecx
        jnz     .gpio_init

; Set GPIO0 pin 0-7 as output
        mov     al, 0xff
        mov     dx, GPIO_DATA_ADDR + 0*4 + 2
        out     dx, al

        invoke  RegService, my_service, service_proc
        ret
  .fail:
  .exit:
        xor     eax, eax
        ret
endp

proc service_proc stdcall, ioctl:dword

        mov     ebx, [ioctl]
        mov     eax, [ebx+IOCTL.io_code]
        cmp     eax, SRV_GETVERSION
        jne     @F

        mov     eax, [ebx+IOCTL.output]
        cmp     [ebx+IOCTL.out_size], 4
        jne     .fail
        mov     dword [eax], API_VERSION
        xor     eax, eax
        ret
  @@:
        cmp     eax, 1  ; read GPIO P0
        jne     .no_gpioread
        mov     dx, GPIO_DATA_ADDR + 0x00
        in      al, dx
        ret
  .no_gpioread:
        cmp     eax, 2  ; write GPIO P0
        jne     .no_gpiowrite

        mov     eax, [ebx + IOCTL.input]
        mov     dx, GPIO_DATA_ADDR + 0x00
        out     dx, al
        xor     eax, eax
        ret
  .no_gpiowrite:
        cmp     eax, 3  ; read single ADC channel
        jne     .no_adcread

        mov     ecx, [ebx + IOCTL.input]
        cmp     ecx, 8
        jae     .fail

        mov     dx, ADC_ADDR + 1
        mov     al, 1 shl 3             ; Power down ADC
        out     dx, al

        mov     dx, ADC_ADDR + 0        ; AUX channel select register
        mov     al, 1
        shl     ax, cl
        out     dx, al

        mov     dx, ADC_ADDR + 1
        mov     al, 1 shl 0             ; Single shot, no interrupts, start
        out     dx, al

        mov     dx, ADC_ADDR + 2
  @@:
        in      al, dx
        test    al, 1 shl 0             ; data ready?
        jz      @r

        mov     dx, ADC_ADDR + 4
        in      ax, dx                  ; read the data from the FIFO and return to user call
        ret
  .no_adcread:
  .fail:
        or      eax, -1
        ret
endp


proc detect
        push    ebx
        invoke  GetPCIList
        mov     ebx, eax
  .next_dev:
        mov     eax, [eax+PCIDEV.fd]
        cmp     eax, ebx
        jz      .err
        mov     edx, [eax+PCIDEV.vendor_device_id]

        mov     esi, devices
  @@:
        cmp     dword [esi], 0
        jz      .next_dev
        cmp     edx, [esi]
        jz      .found

        add     esi, STRIDE
        jmp     @B

  .found:
        movzx   ebx, [eax+PCIDEV.devfn]
        mov     [dev], ebx
        movzx   ebx, [eax+PCIDEV.bus]
        mov     [bus], ebx
        xor     eax, eax
        inc     eax
        pop     ebx
        ret
  .err:
        DEBUGF  1,"Could not find vortex86EX south bridge!\n"
        xor     eax, eax
        pop     ebx
        ret
endp

DEVICE_ID    = 6011h
VENDOR_ID    = 17F3h

;all initialized data place here

align 4
devices      dd (DEVICE_ID shl 16)+VENDOR_ID
             dd 0    ;terminator

my_service   db '86DUINO-GPIO',0  ;max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

dev     dd ?
bus     dd ?

align 4
data fixups
end data
