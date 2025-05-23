;*****************************************************************************;
;    Copyright (C) 2025, Mikhail Frolov aka Doczom . All rights reserved.     ;
;            Distributed under terms of the 3-Clause BSD License.             ;
;                                                                             ;
; usbother is a driver for loading USB drivers of a certain class and vendor. ;
;                                                                             ;
;                         Version 0.1.1, 24 May 2025                          ;
;                                                                             ;
;*****************************************************************************;
format PE native 0.05
entry  START
; const

DRV_VERSION     = 0x0101 ; 0.1.1


; struct
include '../../struct.inc'

; USB device descriptor
struct DEVICE_DESCR
        bLength                 db      ?
        bDescriptorType         db      ?
        bcdUSB                  dw      ?
        bDeviceClass            db      ?
        bDeviceSubClass         db      ?
        bDeviceProtocol         db      ?
        bMaxPacketSize0         db      ?
        idVendor                dw      ?
        idProduct               dw      ?
        bcdDevice               dw      ?
        iManufacturer           db      ?
        iProduct                db      ?
        iSerialNumber           db      ?
        bNumConfigurations      db      ?
ends

struct INTERFACE_DESCR
        bLength                 db      ?
        bDescriptorType         db      ?
        bInterfaceNumber        db      ?
        bAlternateSetting       db      ?
        bNumEndpoints           db      ?
        bInterfaceClass         db      ?
        bInterfaceSubClass      db      ?
        bInterfaceProtocol      db      ?
        iInterface              db      ?
ends


struct  SRV
        srv_name        rb 16    ;ASCIIZ string
        magic           dd ?     ;+0x10 ;'SRV '
        size            dd ?     ;+0x14 ;size of structure SRV
        fd              dd ?     ;+0x18 ;next SRV descriptor
        bk              dd ?     ;+0x1C ;prev SRV descriptor
        base            dd ?     ;+0x20 ;service base address
        entry           dd ?     ;+0x24 ;service START function
        srv_proc        dd ?     ;+0x28 ;user mode service handler
        srv_proc_ex     dd ?     ;+0x2C ;kernel mode service handler
ends

struct USBSRV
        srv             SRV
        usb_func        dd ?
ends

struct USBFUNC
        strucsize       dd ?
        add_device      dd ?
        device_disconnect dd ?
ends

USBDRV_TYPE_NOLOCK      = 0 ; usb device not controlled (native driver
                            ;  not found and IOCTL not opened device)
USBDRV_TYPE_NATIVE      = 1 ; native PE kernel driver for usb
USBDRV_TYPE_IOCTL       = 2 ; usb device is controlled by IOCTL service
                            ; (driver or userspace process/threads)



struct DRV_CONTEXT
        next            dd ?
        prev            dd ?
        drv_hand        dd ?
        drv_pdata       dd ?
        flags           dd ?

        config_pipe     dd ?
        config_descr    dd ?
        interface_descr dd ?
ends


section '.flat' code readable writable executable

include '../../proc32.inc'
include '../../peimport.inc'
include '../../macros.inc'


proc    START c, state:dword, cmdline:dword

        cmp     [state], DRV_ENTRY
        jne     .end
        ; init

        mov     ecx, drv_list_lock
        invoke  MutexInit

        ; load drv_list
        stdcall load_drv_list, default_list
        test    eax, eax
        jnz     .end

        ; reg driver
        invoke  RegUSBDriver, drv_name, service_proc, usb_functions
        ret
.end:
        xor     eax, eax
        ret
endp

proc    load_drv_list stdcall, .path:dword

        push    ebx
        mov     ecx, drv_list_lock
        invoke  MutexLock
        ; load file
        invoke  LoadFile, [.path]
        test    eax, eax
        push    eax
        jnz     @f
        mov     dword[esp], -1
        jmp     .exit
@@:
        cmp     [drv_list], 0
        jz      @f

        invoke  KernelFree, [drv_list]
@@:
        mov     eax,[esp]
        mov     [drv_list], eax
        mov     dword[esp], 0
.exit:
        mov     ecx, drv_list_lock
        invoke  MutexUnlock
        pop     eax
        pop     ebx
        ret
endp


proc    service_proc stdcall, .ioctl:dword

        push    esi
        or      eax, -1
        mov     esi, [.ioctl]

        mov     ecx, [esi + IOCTL.io_code]
        cmp     ecx, .count_ioctl_codes
        jae     .fail

        jmp     dword[.table_subfunction + ecx]

.table_subfunction:
        dd      .get_version
        dd      .update_list
        dd      .get_array_dev
        dd      .get_full_dev_data
        dd      .open_dev
        dd      .close_dev
        dd      .control_transfer
        dd      .bulk_transfer
        dd      .interrupt_transfer
        ;dd      .control_transfer_async
        ;dd      .bulk_transfer_async
        ;dd      .interrupt_transfer_async
.count_ioctl_codes = ($ - .table_subfunction)/4

.get_version:
        mov     eax, [esi + IOCTL.output]
        cmp     [esi + IOCTL.out_size], 4
        jne     .fail

        mov     dword[eax], DRV_VERSION
        xor     eax, eax
        jmp     .exit

.update_list:
        ; update list
        mov     ecx, [esi + IOCTL.input]
        cmp     [esi + IOCTL.inp_size], 0
        jnz     @f

        mov     ecx, default_list
@@:
        stdcall load_drv_list, ecx

.exit:
        pop     esi
        ret

.get_array_dev:
.get_full_dev_data:
.open_dev:
.close_dev:
.control_transfer:
.bulk_transfer:
.interrupt_transfer:

.fail:
        or      eax, -1
        jmp     .exit
endp


proc    AddDevice stdcall, .config_pipe:dword, \
                           .config_descr:dword,\
                           .interface:dword

        push    esi edi
        mov     eax, sizeof.DRV_CONTEXT
        invoke  Kmalloc
        test    eax, eax
        jz      .err_init

        mov     esi, eax

;        mov     ecx, interface_list_lock
;        invoke  MutexLock
;
;        mov     edx, [usb_interface_list]
;        test    edx, edx
;        jz      @f
;        mov     [edx + DRV_CONTEXT.prev], esi
;@@:
;        mov     [esi + DRV_CONTEXT.next], edx
;        mov     [esi + DRV_CONTEXT.prev], 0
;        mov     [usb_interface_list], esi
;
;        mov     ecx, interface_list_lock
;        invoke  MutexUnlock

        and     [esi + DRV_CONTEXT.drv_hand], 0
        mov     [esi + DRV_CONTEXT.flags], 0

        ; lock mutex
        mov     ecx, drv_list_lock
        invoke  MutexLock

        ; save device context data
        mov     eax, [.config_pipe]
        mov     [esi + DRV_CONTEXT.config_pipe], eax
        mov     eax, [.config_descr]
        mov     [esi + DRV_CONTEXT.config_descr], eax
        mov     eax, [.interface]
        mov     [esi + DRV_CONTEXT.interface_descr], eax

        ; get pointer to list
        mov     edx, [drv_list]
        test    edx, edx
        jz      .err_exit

        mov     edi, edx
        add     edi, [edx]

        ; get in ecx VID:PID code
        invoke  USBGetParam, [.config_pipe], 0
        mov     ecx, dword[eax + DEVICE_DESCR.idVendor]

.loop_id_drv:
        cmp     dword[edi], 0
        jz      .end_loop_id_drv

        mov     edx, [drv_list]
        add     edx, [edi] ; ID_TABLE
.loop_id:
        cmp     dword[edx], 0
        jz      .end_loop_id

        ; check id
        mov     eax, ecx
        test    word[edx + 2], 0xffff
        jne     @f
        ; driver for all devices of VID
        and     eax, 0xffff
@@:
        cmp     [edx], eax ; check VID:PID
        je      @f

        add     edx, 4
        jmp     .loop_id
@@:     ; found
        call    .load_drv
        jnz     .exit

        add     edx, 4
        jmp     .loop_id
.end_loop_id:
        add     edi, 8
        jmp     .loop_id_drv

.end_loop_id_drv:


        push    esi
        mov     esi, str_1
        invoke  SysMsgBoardStr
        pop     esi


        ; get in ecx class code
        mov     eax, [.interface]
        mov     ecx, dword[eax + INTERFACE_DESCR.bInterfaceClass]  ; 24-31bits

        mov     edi, [drv_list]
        add     edi, [edi + 4]
.loop_class:
        cmp     dword[edi], 0
        jz      .end_loop_class

        ; check class
        movzx   eax, byte[edi]  ; length
        and     eax, 11b ; protect - max length = 3
        lea     eax, [eax*8] ; 1 = 8;  2 = 16; 3 = 24
        xor     edx, edx
        bts     edx, eax
        dec     edx  ; bitmask

        mov     eax, [edi]
        shr     eax, 8
        and     eax, edx    ; good class in list

        and     edx, ecx    ; good class of device

        cmp     eax, edx
        je      @f

        add     edi, 8
        jmp     .loop_class
@@:     ; found
        call    .load_drv
        jnz     .exit

        add     edi, 8
        jmp     .loop_class

; IN: edi - item list of driver
;     esi - DRV_CONTEXT
; OUT: ZF - not found zF - found
; function save drv handl in DRV_CONTEXT.drv_hand
;          and pdata in DRV_CONTEXT.drv_pdata
.load_drv:
        push    ecx edx
        ; load driver
        push    esi
        mov     esi, str_2
        invoke  SysMsgBoardStr
        pop     esi

        mov     ecx, [drv_list]
        add     ecx, [edi + 4]

        pusha
        mov     esi, ecx
        invoke  SysMsgBoardStr
        mov     esi, str_newline
        invoke  SysMsgBoardStr
        popa

        invoke  GetService, ecx
        test    eax, eax
        jz      @f

        mov     [esi + DRV_CONTEXT.drv_hand], eax
        ; get function list
        mov     ecx, [eax + USBSRV.usb_func]

        ; call AddDevice of driver
        stdcall [ecx + USBFUNC.add_device], [.config_pipe], \
                                            [.config_descr],\
                                            [.interface]
        mov     [esi + DRV_CONTEXT.drv_pdata], eax
        test    eax, eax
        jnz     .load_drv.good

        push    esi
        mov     esi, str_4
        invoke  SysMsgBoardStr
        pop     esi

        and     [esi + DRV_CONTEXT.drv_hand], 0
@@:
        pushf
        push    esi
        mov     esi, str_5
        invoke  SysMsgBoardStr
        pop     esi
        popf

        pop     edx ecx
        retn
.load_drv.good:
        pushf
        push    esi
        mov     esi, str_3
        invoke  SysMsgBoardStr
        pop     esi
        popf

        mov     [esi + DRV_CONTEXT.flags], USBDRV_TYPE_NATIVE
        pop     edx ecx
        retn


.err_exit:
        mov     eax, esi
        invoke  Kfree
        xor     esi, esi
.end_loop_class:
.exit:
        ; driver not found - Added libusb driver

        ; unlock mutex
        mov     ecx, drv_list_lock
        invoke  MutexUnlock

        mov     eax, esi
        pop     edi esi
        ret
.err_init:
        xor     eax, eax
        pop     edi esi
        ret
endp


proc    DeviceDisconnected stdcall, .pdata:dword

        mov     eax, [.pdata]

        test    [eax + DRV_CONTEXT.flags], USBDRV_TYPE_NATIVE
        jz      .no_native

        cmp     [eax + DRV_CONTEXT.drv_hand], 0
        jz      .free

        ; call device disconnected
        mov     ecx, [eax + DRV_CONTEXT.drv_hand]

        mov     edx, [ecx + USBSRV.usb_func]
        cmp     dword[edx], USBFUNC.device_disconnect
        jbe     .free                                ; TODO: check

        stdcall [edx + USBFUNC.device_disconnect], [eax + DRV_CONTEXT.drv_pdata]
.free:
        ; clear list of DRV_CONTENT
;        mov     ecx, interface_list_lock
;        invoke  MutexLock
;
;        mov     eax, [.pdata]
;        mov     edx, [eax + DRV_CONTEXT.prev]
;        mov     ecx, [eax + DRV_CONTEXT.next]
;        test    edx, edx
;        jz      @f
;        mov     [edx + DRV_CONTEXT.next], ecx
;@@:
;        test    ecx, ecx
;        jz      @f
;        mov     [ecx + DRV_CONTEXT.prev], edx
;@@:
;        cmp     [usb_interface_list], eax
;        jne     @f
;        mov     [usb_interface_list], ecx
;@@:
;        mov     ecx, interface_list_lock
;        invoke  MutexUnlock

        ; free context
        mov    eax, [.pdata]
        invoke Kfree
        ret
.no_native:
        test    [eax + DRV_CONTEXT.flags], USBDRV_TYPE_IOCTL
        jz      .free

        jmp     .free
endp

; data
drv_list_lock   MUTEX

drv_list        dd ?

interface_list_lock     MUTEX
usb_interface_list:
        dd      ?
        dd      ?


usb_functions:
        dd      .end - usb_functions
        dd      AddDevice
        dd      DeviceDisconnected
.end:

drv_name        db 'usbother', 0

default_list:   db '/sys/settings/usbdrv.dat', 0

str_1: db 'USBOTHER: Not found ID driver', 13, 10, 0
str_2: db 'USBOTHER: Check found driver: ', 0, 13, 10, 0
str_3: db 'USBOTHER: Device driver good', 13, 10, 0
str_4: db 'USBOTHER: Device driver fail prob', 13, 10, 0
str_5: db 'USBOTHER: Load device driver error', 13, 10, 0
str_newline: db 13,10,0

data fixups
end data
