;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  FTDI chips driver for KolibriOS                             ;;
;;                                                              ;;
;;   Written by gtament@gmail.com                               ;;
;;                                                              ;;
;;         GNU GENERAL PUBLIC LICENSE                           ;;
;;          Version 2, June 1991                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

DEBUG = 1

__DEBUG__ = 1
__DEBUG_LEVEL__ = 1

node equ ftdi_context
node.next equ ftdi_context.next_context

include '../../proc32.inc'
include '../../peimport.inc'
include '../../fdo.inc'
include '../../struct.inc'

; USB constants
DEVICE_DESCR_TYPE           = 1
CONFIG_DESCR_TYPE           = 2
STRING_DESCR_TYPE           = 3
INTERFACE_DESCR_TYPE        = 4
ENDPOINT_DESCR_TYPE         = 5
DEVICE_QUALIFIER_DESCR_TYPE = 6

CONTROL_PIPE     = 0
ISOCHRONOUS_PIPE = 1
BULK_PIPE        = 2
INTERRUPT_PIPE   = 3

; USB HID constants
HID_DESCR_TYPE      = 21h
REPORT_DESCR_TYPE   = 22h
PHYSICAL_DESCR_TYPE = 23h

; LibUSB constatnts
LIBUSB_REQUEST_TYPE_STANDARD = (0x00 shl 5)
LIBUSB_REQUEST_TYPE_CLASS = (0x01 shl 5)
LIBUSB_REQUEST_TYPE_VENDOR = (0x02 shl 5)
LIBUSB_REQUEST_TYPE_RESERVED = (0x03 shl 5)

LIBUSB_RECIPIENT_DEVICE = 0x00
LIBUSB_RECIPIENT_INTERFACE = 0x01
LIBUSB_RECIPIENT_ENDPOINT = 0x02
LIBUSB_RECIPIENT_OTHER = 0x03

LIBUSB_ENDPOINT_IN = 0x80
LIBUSB_ENDPOINT_OUT = 0x00

; FTDI Constants
FTDI_DEVICE_OUT_REQTYPE = (LIBUSB_REQUEST_TYPE_VENDOR or LIBUSB_RECIPIENT_DEVICE or LIBUSB_ENDPOINT_OUT)
FTDI_DEVICE_IN_REQTYPE = (LIBUSB_REQUEST_TYPE_VENDOR or LIBUSB_RECIPIENT_DEVICE or LIBUSB_ENDPOINT_IN)

; Requests
;Definitions for flow control
SIO_RESET         =0  ;Reset the port
SIO_MODEM_CTRL    =1  ;Set the modem control register 
SIO_SET_FLOW_CTRL =2  ;Set flow control register
SIO_SET_BAUD_RATE =3  ;Set baud rate
SIO_SET_DATA      =4  ;Set the data characteristics of the port

SIO_RESET_REQUEST            =SIO_RESET
SIO_SET_BAUDRATE_REQUEST     =SIO_SET_BAUD_RATE
SIO_SET_DATA_REQUEST         =SIO_SET_DATA
SIO_SET_FLOW_CTRL_REQUEST    =SIO_SET_FLOW_CTRL
SIO_SET_MODEM_CTRL_REQUEST   =SIO_MODEM_CTRL
SIO_POLL_MODEM_STATUS_REQUEST=0x05
SIO_SET_EVENT_CHAR_REQUEST   =0x06
SIO_SET_ERROR_CHAR_REQUEST   =0x07
SIO_SET_LATENCY_TIMER_REQUEST=0x09
SIO_GET_LATENCY_TIMER_REQUEST=0x0A
SIO_SET_BITMODE_REQUEST      =0x0B
SIO_READ_PINS_REQUEST        =0x0C
SIO_READ_EEPROM_REQUEST      =0x90
SIO_WRITE_EEPROM_REQUEST     =0x91
SIO_ERASE_EEPROM_REQUEST     =0x92


SIO_RESET_SIO=0
SIO_RESET_PURGE_RX=1
SIO_RESET_PURGE_TX=2

SIO_DISABLE_FLOW_CTRL=0x0
SIO_RTS_CTS_HS =(0x1 shl 8)
SIO_DTR_DSR_HS =(0x2 shl 8)
SIO_XON_XOFF_HS=(0x4 shl 8)

SIO_SET_DTR_MASK=0x1
SIO_SET_DTR_HIGH=( 1 or ( SIO_SET_DTR_MASK  shl 8))
SIO_SET_DTR_LOW =( 0 or ( SIO_SET_DTR_MASK  shl 8))
SIO_SET_RTS_MASK=0x2
SIO_SET_RTS_HIGH=( 2 or ( SIO_SET_RTS_MASK shl 8 ))
SIO_SET_RTS_LOW =( 0 or ( SIO_SET_RTS_MASK shl 8 ))

SIO_RTS_CTS_HS =(0x1 shl 8)

;FTDI chip type
TYPE_AM=0
TYPE_BM=1
TYPE_2232C=2
TYPE_R=3
TYPE_2232H=4
TYPE_4232H=5
TYPE_232H=6
TYPE_230X=7

;strings
my_driver       db      'usbother',0
nomemory_msg    db      'K : no memory',13,10,0

; Structures
struct ftdi_context
chipType                db      ?
baudrate                dd      ?
bitbangEnabled          db      ?
readBufChunkSize        dd      ?
writeBufChunkSize       dd      ?
readBufPtr              dd      ?
writeBufPtr             dd      ?
readBufSize             dd      ?
writeBufSize            dd      ?
maxPacketSize           dd      ?
interface               dd      ?
index                   dd      ?
inEP                    dd      ?
outEP                   dd      ?
nullP                   dd      ? 
lockPID                 dd      ?
next_context            dd      ?
ends

struct IOCTL
handle                  dd      ?
io_code                 dd      ?
input                   dd      ?
inp_size                dd      ?
output                  dd      ?
out_size                dd      ?
ends

struct usb_descr
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

struct conf_packet
bmRequestType           db      ?
bRequest                db      ?
wValue                  dw      ?
wIndex                  dw      ?
wLength                 dw      ?
ends

section '.flat' code readable executable
; The start procedure.
proc START c, .reason:DWORD, .cmdline:DWORD

        xor     eax, eax        ; initialize return value
        cmp     [.reason], 1    ; compare the argument
        jnz     .nothing                
        invoke  RegUSBDriver, my_driver, service_proc, usb_functions

.nothing:
        ret
endp


proc AddDevice stdcall uses ebx esi edi, .config_pipe:DWORD, .config_descr:DWORD, .interface:DWORD
        
        invoke  USBGetParam, [.config_pipe], 0
        mov     edx, eax
        DEBUGF 2,'K : Detected device vendor: 0x%x\n', [eax+usb_descr.idVendor]
        cmp     word[eax+usb_descr.idVendor], 0x0403
        jnz     .notftdi
        mov     eax, sizeof.ftdi_context
        invoke  Kmalloc
        test    eax, eax
        jnz     @f
        mov     esi, nomemory_msg
        invoke  SysMsgBoardStr
        jmp     .nothing
  @@:
        DEBUGF 2,'K : Adding struct to list 0x%x\n', eax
        call    linkedlist_add
        
        mov     ebx, [.config_pipe]
        mov     [eax + ftdi_context.nullP], ebx
        mov     [eax + ftdi_context.index], 0
        mov     [eax + ftdi_context.lockPID], 0
        mov     [eax + ftdi_context.maxPacketSize], 64       
        mov     [eax + ftdi_context.readBufChunkSize], 64
        mov     [eax + ftdi_context.writeBufChunkSize], 64

        mov     [eax + ftdi_context.chipType], TYPE_R
        jmp     .slow
        
        mov     cx, [edx+usb_descr.bcdDevice]
        DEBUGF 2, 'K : Chip type 0x%x\n', ecx
        cmp     cx, 0x400
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_BM
        jmp     .slow
  @@:      
        cmp     cx, 0x200
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_AM
        jmp     .slow
  @@:     
        cmp     cx, 0x500
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_2232C 
        jmp     .slow
  @@:      
        cmp     cx, 0x600
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_R
        jmp     .slow
  @@:      
        cmp     cx, 0x700
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_2232H
        jmp     .fast
  @@:      
        cmp     cx, 0x900
        jnz     @f
        mov     [eax + ftdi_context.chipType], TYPE_232H
        jmp     .fast
  @@:      
        cmp     cx, 0x1000
        jnz     .slow
        mov     [eax + ftdi_context.chipType], TYPE_230X
        jmp     .fast
        
  .fast:
        add     [eax + ftdi_context.maxPacketSize], 512-64
  .slow:    
        mov     ebx, eax
        invoke  USBOpenPipe, [.config_pipe], 0x81, \
                            [ebx + ftdi_context.maxPacketSize],  BULK_PIPE, 0
        test    eax, eax
        jz      .nothing
        mov     [ebx + ftdi_context.outEP], eax
        invoke  USBOpenPipe, [.config_pipe], 0x02, \
                            [ebx + ftdi_context.maxPacketSize],  BULK_PIPE, 0
        test    eax, eax
        jz      .nothing
        mov     [ebx + ftdi_context.inEP], eax
        mov     eax, ebx
        ret        
           
  .notftdi:
        DEBUGF 1,'K : Skipping not FTDI device\n'
  .nothing:        
        xor     eax, eax
        ret
endp


handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

proc service_proc stdcall uses ebx esi edi, ioctl:DWORD
locals
ConfPacket  rb  10
EventData   rd  3
endl
        mov     edi, [ioctl]
        mov     eax, [edi+io_code]
        DEBUGF 1,'K : FTDI got the request: %d\n', eax
        test    eax, eax            ;0
        jz      .version
        dec     eax                 ;1
        jz      .ftdi_get_list
        
        push    eax
        mov     esi, [edi+input]
        mov     eax, [esi+4]
        call    linkedlist_isvalid
        test    eax, eax
        pop     eax        
        jnz     .endswitch
        
        dec     eax
        jz      .ftdi_lock      ;2
        dec     eax
        jz      .ftdi_unlock    ;3
        
        mov     ebx, [esi+4]
        mov     ecx, [ebx + ftdi_context.lockPID]
        cmp     dword[esi], 0
        jz      .error
        cmp     ecx, [esi]
        jz      .pid_ok
        mov     esi, [edi+output]
        mov     dword[esi], 'LCKD'
        jmp     .endswitch
        
  .pid_ok:
        dec     eax
        jz      .ftdi_get_wchunksize    ;4
        dec     eax
        jz      .ftdi_get_rchunksize    ;5        
        
        mov     edi, [edi+input]
        
        dec     eax
        jz      .ftdi_set_rchunksize    ;6
        dec     eax
        jz      .ftdi_set_wchunksize    ;7
  
        push    eax edi
        mov     ecx, 0x80000000
        cmp     eax, 8-7
        je      .bulkevent
        cmp     eax, 9-7
        je      .bulkevent
        xor     ecx, ecx
  .bulkevent:                
        xor     esi, esi
        invoke  CreateEvent        
        mov     [EventData], eax
        mov     [EventData+4], edx
        pop     edi eax
        
        dec     eax                 ;8
        jz      .ftdi_write_data
        dec     eax                 ;9
        jz      .ftdi_read_data
        dec     eax                 ;10
        jz      .ftdi_set_baudrate
        dec     eax                 ;11
        jz      .ftdi_set_bitmode
        dec     eax                 ;12
        jz      .ftdi_setrtshigh   
        dec     eax                 ;13
        jz      .ftdi_setrtslow  
        dec     eax                 ;14
        jz      .ftdi_setdtrhigh
        dec     eax                 ;15
        jz      .ftdi_setdtrlow
        dec     eax                 ;16
        jz      .ftdi_usb_reset
        dec     eax                 ;17
        jz      .ftdi_setflowctrl
        dec     eax                 ;18
        jz      .ftdi_set_event_char
        dec     eax                 ;19
        jz      .ftdi_set_error_char
        dec     eax                 ;20
        jz      .ftdi_set_latency_timer
        dec     eax                 ;21
        jz      .ftdi_get_latency_timer
        dec     eax                 ;22
        jz      .ftdi_read_pins
        dec     eax                 ;23
        jz      .ftdi_poll_modem_status        
        dec     eax                 ;24
        jz      .ftdi_set_line_property
        dec     eax                 ;25
        jz      .ftdi_purge_rx_buf
        dec     eax                 ;26
        jz      .ftdi_purge_tx_buf
        jmp     .error
                  
  .version:     
        jmp     .endswitch
  .error:
        DEBUGF 1, 'K : FTDI error occured! %d\n', eax
        ;mov     esi, [edi+output]
        ;mov     [esi], dword 'ERR0'
        ;or      [esi], eax
        ret
  .endswitch:
        xor     eax, eax
        ret
        
  .eventdestroy:
        ;---Dirty hack begin
        test    eax, eax
        jz      @f
        mov     eax, dword[ConfPacket]
        invoke  Kfree
  @@:
        ;---Dirty hack end
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  DestroyEvent
        jmp     .endswitch
        
  .ftdi_out_control_transfer_withinp:               
        mov     dx, word[edi+8]
        mov     word[ConfPacket+2], dx 
  .ftdi_out_control_transfer_noinp:
        mov     ebx, [edi+4]
        mov     cx, word[ebx + ftdi_context.index]
        mov     word[ConfPacket+4], cx
        xor     cx, cx
        mov     word[ConfPacket+6], cx
  .own_index:
        mov     ebx, [edi+4]
        DEBUGF 2,'K : ConfPacket 0x%x 0x%x\n', [ConfPacket], [ConfPacket+4]          
        lea     esi, [ConfPacket]
        lea     edi, [EventData]        
        invoke  USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, 0,\
                                            0, control_callback, edi, 0
        test    eax, eax        
        jz      .error
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  WaitEvent
        mov     eax, [EventData+8]
        test    eax, eax
        jz      .endswitch
        jmp     .error
        
  .ftdi_setrtshigh:
        DEBUGF 2,'K : FTDI Setting RTS pin HIGH PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_SET_MODEM_CTRL_REQUEST shl 8) \
                                 + (SIO_SET_RTS_HIGH shl 16)
        jmp     .ftdi_out_control_transfer_noinp         

  .ftdi_setrtslow:
        DEBUGF 2,'K : FTDI Setting RTS pin LOW PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_SET_MODEM_CTRL_REQUEST shl 8) \
                                 + (SIO_SET_RTS_LOW shl 16)
        jmp     .ftdi_out_control_transfer_noinp           
        
  .ftdi_setdtrhigh:
        DEBUGF 2,'K : FTDI Setting DTR pin HIGH PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_SET_MODEM_CTRL_REQUEST shl 8) \
                                 + (SIO_SET_DTR_HIGH shl 16)
        jmp     .ftdi_out_control_transfer_noinp           

  .ftdi_setdtrlow:
        DEBUGF 2,'K : FTDI Setting DTR pin LOW PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_SET_MODEM_CTRL_REQUEST shl 8) \
                                 + (SIO_SET_DTR_LOW shl 16)
        jmp     .ftdi_out_control_transfer_noinp           
        
  .ftdi_usb_reset:
        DEBUGF 2,'K : FTDI Reseting PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_RESET_REQUEST shl 8) \
                                 + (SIO_RESET_SIO shl 16)
        jmp     .ftdi_out_control_transfer_noinp  
        
  .ftdi_purge_rx_buf:
        DEBUGF 2, 'K : FTDI Purge TX buffer PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_RESET_REQUEST shl 8) \
                                 + (SIO_RESET_PURGE_RX shl 16)
        jmp     .ftdi_out_control_transfer_noinp  
        
  .ftdi_purge_tx_buf:
        DEBUGF 2, 'K : FTDI Purge RX buffer PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_RESET_REQUEST shl 8) \
                                 + (SIO_RESET_PURGE_TX shl 16)
        jmp     .ftdi_out_control_transfer_noinp  
        
  .ftdi_set_bitmode:
        DEBUGF 2, 'K : FTDI Set bitmode 0x%x, bitmask 0x%x %d PID: %d Dev handler 0x0x%x\n', \
                                               [edi+8]:2,[edi+10]:2,[edi],[edi+4]
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_BITMODE_REQUEST shl 8)                                       
        jmp     .ftdi_out_control_transfer_withinp        
     
  .ftdi_set_line_property:
        DEBUGF 2, 'K : FTDI Set line property 0x%x PID: %d Dev handler 0x0x%x\n', \
                                                          [edi+8]:4,[edi],[edi+4]
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_DATA_REQUEST shl 8)
        jmp     .ftdi_out_control_transfer_withinp
        
  .ftdi_set_latency_timer:
        DEBUGF 2, 'K : FTDI Set latency %d PID: %d Dev handler 0x0x%x\n', \
                                                          [edi+8],[edi],[edi+4]
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_LATENCY_TIMER_REQUEST shl 8)        
        jmp     .ftdi_out_control_transfer_withinp
        
  .ftdi_set_event_char:
        DEBUGF 2, 'K : FTDI Set event char %c PID: %d Dev handler 0x0x%x\n', \
                                                          [edi+8],[edi],[edi+4]
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_EVENT_CHAR_REQUEST shl 8)
        jmp     .ftdi_out_control_transfer_withinp

  .ftdi_set_error_char:
        DEBUGF 2, 'K : FTDI Set error char %c PID: %d Dev handler 0x0x%x\n', \
                                                          [edi+8],[edi],[edi+4]
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_ERROR_CHAR_REQUEST shl 8)
        jmp     .ftdi_out_control_transfer_withinp 
        
  .ftdi_setflowctrl:
        DEBUGF 2, 'K : FTDI Set flow control PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                 + (SIO_SET_FLOW_CTRL_REQUEST shl 8) + (0 shl 16)
        mov     ebx, [edi+4]
        mov     cx, word[edi+8]   
        or      ecx, [ebx + ftdi_context.index]
        mov     word[ConfPacket+4], cx
        xor     cx, cx
        mov     word[ConfPacket+6], cx
        jmp     .own_index

  .ftdi_read_pins:
        DEBUGF 2, 'K : FTDI Read pins PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                     [edi+4]
        mov     ebx, [edi+4] 
        mov     dword[ConfPacket], (FTDI_DEVICE_IN_REQTYPE) \
                                 + (SIO_READ_PINS_REQUEST shl 8) + (0 shl 16)
        mov     ecx, [ebx + ftdi_context.index]
        mov     word[ConfPacket+4], cx
        mov     word[ConfPacket+6], 1
        lea     esi, [ConfPacket]
        lea     edi, [EventData]
        mov     ecx, esi
        add     ecx, 8
        mov     word[ConfPacket+8], 0
        invoke  USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, ecx, \
                                            1, control_callback, edi, 0
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  WaitEvent
        xor     ebx, ebx
        mov     bx, word[ConfPacket+8]
        mov     ecx, [ioctl]
        mov     ecx, [ecx+output]
        mov     [ecx], ebx
        mov     eax, [EventData+8]
        test    eax, eax
        jz      .endswitch
        jmp     .error
        
  .ftdi_set_wchunksize:
        DEBUGF 2, 'K : FTDI Set write chunksize %d bytes PID: %d Dev handler 0x0x%x\n', \
                                                    [edi+8], [edi], [edi+4]
        mov     ebx, [edi+4]
        mov     ecx, [edi+8] 
        cmp     [ebx + ftdi_context.maxPacketSize], ecx
        jg      .error
        mov     [ebx + ftdi_context.writeBufChunkSize], ecx
        jmp     .endswitch
   
  .ftdi_get_wchunksize:
        DEBUGF 2, 'K : FTDI Get write chunksize PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     esi, [edi+output]
        mov     edi, [edi+input]
        mov     ebx, [edi+4]
        mov     ecx, [ebx + ftdi_context.writeBufChunkSize]
        mov     [esi], ecx
        jmp     .endswitch

  .ftdi_set_rchunksize:
        DEBUGF 2, 'K : FTDI Set read chunksize %d bytes PID: %d Dev handler 0x0x%x\n', \
                                                    [edi+8], [edi], [edi+4]
        mov     ebx, [edi+4]
        mov     ecx, [edi+8]
        cmp     [ebx + ftdi_context.maxPacketSize], ecx
        jg      .error
        mov     [ebx + ftdi_context.readBufChunkSize], ecx
        jmp     .endswitch

  .ftdi_get_rchunksize:
        DEBUGF 2, 'K : FTDI Get read chunksize PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4]
        mov     esi, [edi+output]
        mov     edi, [edi+input]
        mov     ebx, [edi+4]
        mov     ecx, [ebx + ftdi_context.readBufChunkSize]
        mov     [esi], ecx
        jmp     .endswitch        

  .ftdi_write_data:
        DEBUGF 2, 'K : FTDI Write %d bytes PID: %d Dev handler 0x%x\n', [edi+8],\
                                                            [edi], [edi+4]
        mov     esi, edi
        add     esi, 12        
        mov     ebx, [edi+4]
        ;---Dirty hack begin
        mov     eax, [edi+8]
        invoke  Kmalloc
        test    eax, eax
        jnz     @f
        mov     esi, nomemory_msg
        invoke  SysMsgBoardStr
        jmp     .eventdestroy
  @@:
        mov     dword[ConfPacket], eax
        mov     ecx, [edi+8]
        push    edi
        mov     edi, eax
        rep movsb
        pop     edi
        mov     esi, dword[ConfPacket]
        ;---Dirty hack end        
        xor     ecx, ecx        ; ecx - offset      
  .write_loop:
        mov     edx, [edi+8]    ; edx - write_size
        sub     edx, ecx
        cmp     edx, [ebx + ftdi_context.writeBufChunkSize]
        jle     .lessthanchunk_write
        mov     edx, [ebx + ftdi_context.writeBufChunkSize]
  .lessthanchunk_write:
        add     esi, ecx
        lea     eax, [EventData]
        push    ecx ebx esi edi
        invoke USBNormalTransferAsync, [ebx + ftdi_context.inEP], esi, edx, \
                                            bulk_callback, eax, 1
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  WaitEvent
        pop     edi esi ebx ecx
        cmp     [EventData+8], -1
        jz      .error
        add     ecx, [EventData+8]
        cmp     ecx, [edi+8]
        jge     .eventdestroy
        jmp     .write_loop
        
  .ftdi_read_data:
        DEBUGF 2, 'K : FTDI Read %d bytes PID: %d Dev handler 0x%x\n', [edi+8],\
                                                            [edi], [edi+4]
        mov     edi, [ioctl]
        mov     esi, [edi+input]
        mov     edi, [edi+output]
        mov     ebx, [esi+4]     
        ;---Dirty hack begin
        mov     eax, [esi+8]
        invoke  Kmalloc
        test    eax, eax
        jnz     @f
        mov     esi, nomemory_msg
        invoke  SysMsgBoardStr
        jmp     .eventdestroy
  @@:
        mov     edi, eax
        ; push    edi eax
        ; mov     ecx, [esi+8]
        ; xor     eax, eax
        ; rep     stosb
        ; pop     eax edi
        mov     dword[ConfPacket], eax ; Store in ConfPacket ptr to allocated memory
        ;---Dirty hack end        
        xor     ecx, ecx
  .read_loop:
        mov     edx, [esi+8]
        cmp     ecx, edx
        jge     .read_end;jge     .eventdestroy ;part of Dirty hack
        sub     edx, ecx
        cmp     edx, [ebx + ftdi_context.readBufChunkSize]
        jl      .lessthanchunk_read
        mov     edx, [ebx + ftdi_context.readBufChunkSize]     
  .lessthanchunk_read:
        lea     eax, [EventData]
        add     edi, ecx
        push    esi edi ecx ebx
        invoke  USBNormalTransferAsync, [ebx + ftdi_context.outEP], edi, edx, \
                                            bulk_callback, eax, 1        
        mov     eax, [EventData]
        mov     ebx, [EventData+4]        
        invoke  WaitEvent
        pop     ebx ecx edi esi
        cmp     [EventData+8], -1        
        jz      .error
        add     ecx, [EventData+8]
        jmp     .read_loop 
        ;---Dirty hack begin        
  .read_end:
        mov     esi, dword[ConfPacket]
        mov     edi, [ioctl]
        mov     ecx, [edi+input]
        mov     ecx, [ecx+8]
        mov     edi, [edi+output]
        rep     movsb
        jmp     .eventdestroy
        ;---Dirty hack end
        
  .ftdi_poll_modem_status:
        DEBUGF 2, 'K : FTDI Poll modem status PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4] 
        mov     ebx, [edi+4] 
        mov     dword[ConfPacket], (FTDI_DEVICE_IN_REQTYPE) \
                                 + (SIO_POLL_MODEM_STATUS_REQUEST shl 8) + (0 shl 16)
        mov     ecx, [ebx + ftdi_context.index]
        mov     word[ConfPacket+4], cx
        mov     word[ConfPacket+6], 1
        lea     esi, [ConfPacket]
        lea     edi, [EventData]
        mov     ecx, [ioctl]
        mov     ecx, [ecx+output]
        invoke  USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, ecx, \
                                            2, control_callback, edi, 0
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  WaitEvent
        mov     ax, word[ecx]
        xchg    ah, al
        and     ah, 0xFF
        mov     word[ecx], ax
        jmp     .endswitch
        
  .ftdi_get_latency_timer:
        DEBUGF 2, 'K : FTDI Get latency timer PID: %d Dev handler 0x0x%x\n', [edi],\
                                                                        [edi+4] 
        mov     ebx, [edi+4] 
        mov     dword[ConfPacket], FTDI_DEVICE_IN_REQTYPE \
                                + (SIO_GET_LATENCY_TIMER_REQUEST shl 8) + (0 shl 16)
        mov     ecx, [ebx + ftdi_context.index]
        mov     word[ConfPacket+4], cx
        mov     word[ConfPacket+6], 1
        lea     esi, [ConfPacket]
        lea     edi, [EventData]
        mov     ecx, [ioctl]
        mov     ecx, [ecx+output]
        invoke  USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, ecx, \
                                            2, control_callback, edi, 0
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        invoke  WaitEvent
        jmp     .endswitch
                
  .ftdi_get_list:
        DEBUGF 2, 'K : FTDI devices list request\n'  
        mov     edi, [edi+output]
        xor     ecx, ecx
        call    linkedlist_gethead
        test    eax, eax
        jz      .emptylist
        push    edi
        add     edi, 4         
  .nextdev:
        inc     ecx
        cmp     [eax + ftdi_context.lockPID], 0
        jnz      .dev_is_locked        
        mov     dword[edi], 'NLKD'
        jmp     .nextfields
  .dev_is_locked:
        mov     dword[edi], 'LCKD'
  .nextfields:
        mov     bl, [eax + ftdi_context.chipType]
        mov     [edi+4], ebx
        mov     [edi+8], eax
        add     edi, 12
        mov     eax, [eax + ftdi_context.next_context]
        test    eax, eax
        jnz     .nextdev
        pop     edi
  .emptylist:
        mov     [edi], ecx
        jmp     .endswitch

  .ftdi_lock:
        DEBUGF 2, 'K : FTDI Lock PID: %d Dev handler 0x0x%x\n', [edi], [edi+4]  
        mov     esi, [edi+input]
        mov     ebx, [esi+4]
        mov     eax, [ebx + ftdi_context.lockPID]
        test    eax, eax
        jnz     .lockedby
        mov     eax, [esi]
        mov     [ebx + ftdi_context.lockPID], eax
  .lockedby:
        mov     edi, [edi+output]
        mov     [edi], eax
        jmp     .endswitch
        
  .ftdi_unlock:
        DEBUGF 2, 'K : FTDI Unlock PID: %d Dev handler 0x0x%x\n', [edi], [edi+4]
        mov     esi, [edi+input]
        mov     edi, [edi+output]
        mov     ebx, [esi+4]
        mov     eax, [ebx + ftdi_context.lockPID]
        cmp     eax, [esi]
        jnz     .unlockimp
        mov     [ebx + ftdi_context.lockPID], 0
        mov     dword[edi], 0
        jmp     .endswitch   
  .unlockimp:     
        mov     [edi], eax
        jmp     .endswitch 

H_CLK = 120000000
C_CLK = 48000000        
  .ftdi_set_baudrate:
        DEBUGF 2, 'K : FTDI Set baudrate to %d PID: %d Dev handle: 0x%x\n',\
                   [edi+8], [edi], [edi+4]
        mov     ebx, [edi+4]
        cmp     [ebx + ftdi_context.chipType], TYPE_2232H
        jl      .c_clk        
        imul    eax, [edi+8], 10
        cmp     eax, H_CLK / 0x3FFF        
        jle     .c_clk
  .h_clk:
        cmp     dword[edi+8], H_CLK/10
        jl      .h_nextbaud1
        xor     edx, edx
        mov     ecx, H_CLK/10
        jmp     .calcend
        
  .c_clk:
        cmp     dword[edi+8], C_CLK/16
        jl      .c_nextbaud1
        xor     edx, edx
        mov     ecx, C_CLK/16
        jmp     .calcend  
  
  .h_nextbaud1:
        cmp     dword[edi+8], H_CLK/(10 + 10/2)
        jl      .h_nextbaud2
        mov     edx, 1
        mov     ecx, H_CLK/(10 + 10/2)
        jmp     .calcend
        
  .c_nextbaud1:
        cmp     dword[edi+8], C_CLK/(16 + 16/2)
        jl      .c_nextbaud2
        mov     edx, 1
        mov     ecx, C_CLK/(16 + 16/2)
        jmp     .calcend        
        
  .h_nextbaud2:
        cmp     dword[edi+8], H_CLK/(2*10)
        jl      .h_nextbaud3
        mov     edx, 2
        mov     ecx, H_CLK/(2*10)
        jmp     .calcend
        
  .c_nextbaud2:  
        cmp     dword[edi+8], C_CLK/(2*16)
        jl      .c_nextbaud3
        mov     edx, 2
        mov     ecx, C_CLK/(2*16)
        jmp     .calcend        
        
  .h_nextbaud3:
        mov     eax, H_CLK*16/10  ; eax - best_divisor
        xor     edx, edx
        div     dword[edi+8]      ; [edi+8] - baudrate
        push    eax
        and     eax, 1
        pop     eax
        shr     eax, 1
        jz      .h_rounddowndiv     ; jump by result of and eax, 1
        inc     eax
  .h_rounddowndiv:
        cmp     eax, 0x20000
        jle     .h_best_divok
        mov     eax, 0x1FFFF
  .h_best_divok:
        mov     ecx, eax        
        mov     eax, H_CLK*16/10
        xor     edx, edx
        div     ecx
        xchg    ecx, eax            ; ecx - best_baud
        push    ecx
        and     ecx, 1
        pop     ecx
        shr     ecx, 1
        jz      .rounddownbaud
        inc     ecx
        jmp     .rounddownbaud
        
  .c_nextbaud3:
        mov     eax, C_CLK        ; eax - best_divisor
        xor     edx, edx
        div     dword[edi+8]      ; [edi+8] - baudrate
        push    eax
        and     eax, 1
        pop     eax
        shr     eax, 1
        jnz     .c_rounddowndiv     ; jump by result of and eax, 1
        inc     eax
  .c_rounddowndiv:
        cmp     eax, 0x20000
        jle     .c_best_divok
        mov     eax, 0x1FFFF
  .c_best_divok:
        mov     ecx, eax        
        mov     eax, C_CLK
        xor     edx, edx
        div     ecx
        xchg    ecx, eax            ; ecx - best_baud
        push    ecx
        and     ecx, 1
        pop     ecx
        shr     ecx, 1
        jnz     .rounddownbaud
        inc     ecx
        
  .rounddownbaud:
        mov     edx, eax            ; edx - encoded_divisor
        shr     edx, 3
        and     eax, 0x7      
        push    7 6 5 1 4 2 3 0               
        mov     eax, [esp+eax*4]
        shl     eax, 14
        or      edx, eax        
        add     esp, 32
        
  .calcend:
        mov     eax, edx        ; eax - *value
        mov     ecx, edx        ; ecx - *index
        and     eax, 0xFFFF
        cmp     [ebx + ftdi_context.chipType], TYPE_2232H
        jge     .foxyindex        
        shr     ecx, 16
        jmp     .preparepacket
  .foxyindex:
        shr     ecx, 8
        and     ecx, 0xFF00
        or      ecx, [ebx + ftdi_context.index]
        
  .preparepacket:
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) \
                                + (SIO_SET_BAUDRATE_REQUEST shl 8)
        mov     word[ConfPacket+2], ax
        mov     word[ConfPacket+4], cx
        mov     word[ConfPacket+6], 0
        jmp     .own_index        
        
endp 
restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size 


proc control_callback stdcall uses ebx edi esi, .pipe:DWORD, .status:DWORD, \
                               .buffer:DWORD, .length:DWORD, .calldata:DWORD   
   
        DEBUGF 1, 'K : status is %d\n', [.status] 
        mov     ecx, [.calldata]
        mov     eax, [ecx]
        mov     ebx, [ecx+4]
        mov     edx, [.status]
        mov     [ecx+8], edx
        xor     esi, esi
        xor     edx, edx
        invoke  RaiseEvent              
        ret
endp

proc bulk_callback stdcall uses ebx edi esi, .pipe:DWORD, .status:DWORD, \
                            .buffer:DWORD, .length:DWORD, .calldata:DWORD

        DEBUGF 1, 'K : status is %d\n', [.status]        
        mov     ecx, [.calldata]
        mov     eax, [ecx]
        mov     ebx, [ecx+4]
        cmp     [.status], 0
        jz      .normal
        cmp     [.status], 9
        jne     .error
  .normal:
        mov     edx, [.length]
        mov     [ecx+8], edx
        jmp     .ok
  .error:
        mov     [ecx+8], dword -1
  .ok:    
        xor     esi, esi
        xor     edx, edx
        invoke  RaiseEvent            
        ret
endp

proc DeviceDisconnected stdcall uses  ebx esi edi, .device_data:DWORD

        DEBUGF 1, 'K : FTDI deleting device data 0x%x\n', [.device_data]
        mov     eax, [.device_data] 
        call    linkedlist_unlink
        invoke  Kfree
        ret           
endp

include 'linkedlist.inc'

align 4

; Structure with callback functions.
usb_functions:
        dd      12
        dd      AddDevice
        dd      DeviceDisconnected

data fixups
end data        
        
;for DEBUGF macro
include_debug_strings