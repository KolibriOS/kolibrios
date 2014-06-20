;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; standard driver stuff
format MS COFF

DEBUG = 1

; this is for DEBUGF macro from 'fdo.inc'
__DEBUG__ = 1
__DEBUG_LEVEL__ = 1

include '../../proc32.inc'
include '../../imports.inc'
include '../../fdo.inc'
include '../../struct.inc'

public START
public version

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

;strings
my_driver       db      'usbother',0
nomemory_msg    db      'K : no memory',13,10,0

; Structures
struct ftdi_context
chipType                db      ?
baudrate                dd      ?
bitbangEnabled          db      ?
readBufPtr              dd      ?
readBufOffs             dd      ?
readBufChunkSize        dd      ?
writeBufChunkSize       dd      ?
interface               dd      ?
index                   dd      ?
inEP                    dd      ?
outEP                   dd      ?
nullP                   dd      ? 
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

section '.flat' code readable align 16
; The start procedure.
proc START stdcall, .reason:DWORD

        xor     eax, eax        ; initialize return value
        cmp     [.reason], 1    ; compare the argument
        jnz     .nothing                
        stdcall RegUSBDriver, my_driver, service_proc, usb_functions

.nothing:
        ret
endp


proc AddDevice stdcall uses ebx, .config_pipe:DWORD, .config_descr:DWORD, .interface:DWORD
        
        stdcall USBGetParam, [.config_pipe], 0
        DEBUGF 1,'K : Device detected Vendor: %x\n', [eax+usb_descr.idVendor]
        cmp     word[eax+usb_descr.idVendor], 0x0403
        jnz     .notftdi
        DEBUGF 1,'K : FTDI USB device detected\n'
        movi    eax, sizeof.ftdi_context
        call    Kmalloc
        test    eax, eax
        jnz     @f
        mov     esi, nomemory_msg
        call    SysMsgBoardStr
        xor     eax, eax
        jmp     .nothing
@@:
        DEBUGF 1,'K : Adding struct to list %x\n', eax
        call    linkedlist.add
        
        mov     ebx, [.config_pipe]
        mov     [eax + ftdi_context.nullP], ebx
        
        DEBUGF 1,'K : Open first pipe\n'
        mov     ebx, eax
        stdcall USBOpenPipe, [.config_pipe],  0x81,  0x40,  BULK_PIPE, 0
        mov     [ebx + ftdi_context.inEP], eax
        DEBUGF 1,'K : Open second pipe\n'
        stdcall USBOpenPipe, [.config_pipe],  0x02,  0x40,  BULK_PIPE, 0
        mov     [ebx + ftdi_context.outEP], eax        

  .nothing:
        ret            
  .notftdi:
        DEBUGF 1,'K : Skipping not FTDI device\n'
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
proc service_proc stdcall uses ebx esi edi, ioctl:DWORD
locals
ConfPacket  rb  8
EventData   rd  2
endl
        mov     edi, [ioctl]
        mov     eax, [edi + io_code]
        DEBUGF 1,'K : FTDI got the request: %d\n', eax
        test    eax, eax           ;0
        jz      .version
        dec     eax                 ;1
        jz      .ftdi_get_list
        
        push    eax edi
        xor     ecx, ecx
        xor     esi, esi
        call    CreateEvent        
        mov     [EventData], eax
        mov     [EventData+4], edx
        pop     edi eax 
        
        dec     eax                 ;2
        jz      .ftdi_set_bitmode
        dec     eax                 ;3
        jz      .ftdi_setrtshigh   
        dec     eax                 ;4
        jz      .ftdi_setrtslow     
        
  .version:     
  .endswitch:
        xor     eax, eax
	    ret 
        
  .ftdi_out_control_transfer:
        DEBUGF 1,'K : ConfPacket %x %x\n', [ConfPacket], [ConfPacket+4]  
        mov     ecx, [edi+input]
        mov     ebx, [ecx] 
        lea     esi, [ConfPacket]
        lea     edi, [EventData]        
        stdcall USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, 0, 0, control_callback, edi, 0
        DEBUGF 1, 'K : Returned value is %d\n', eax
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        call    WaitEvent     
        jmp     .endswitch
        
  .ftdi_set_bitmode:
        DEBUGF 1,'K : FTDI Seting bitmode\n'                   
        mov     word[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_BITMODE_REQUEST shl 8)
        mov     edi, [edi+input]        
        mov     dx, word[edi+4]                
        mov     word[ConfPacket+2], dx
        mov     dword[ConfPacket+4], 0                                         
        jmp     .ftdi_out_control_transfer     
        jmp     .endswitch

  .ftdi_setrtshigh:
        DEBUGF 1,'K : FTDI Setting RTS pin HIGH\n'                     
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_MODEM_CTRL_REQUEST shl 8) + (SIO_SET_RTS_HIGH shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch

  .ftdi_setrtslow:
        DEBUGF 1,'K : FTDI Setting RTS pin LOW\n'             
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_MODEM_CTRL_REQUEST shl 8) + (SIO_SET_RTS_LOW shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_setdtrhigh:
        DEBUGF 1,'K : FTDI Setting DTR pin HIGH\n'                     
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_MODEM_CTRL_REQUEST shl 8) + (SIO_SET_DTR_HIGH shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch

  .ftdi_setdtrlow:
        DEBUGF 1,'K : FTDI Setting DTR pin LOW\n'             
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_MODEM_CTRL_REQUEST shl 8) + (SIO_SET_DTR_LOW shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_usb_reset:
        DEBUGF 1,'K : FTDI Reseting\n'
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_RESET_REQUEST shl 8) + (SIO_RESET_SIO shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_purge_rx_buf:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_RESET_REQUEST shl 8) + (SIO_RESET_PURGE_RX shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_purge_tx_buf:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_RESET_REQUEST shl 8) + (SIO_RESET_PURGE_TX shl 16)
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_set_baudrate:
        ;!!!!!!!!!!!!!!!!!!!!
        ;jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_set_line_property:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_DATA_REQUEST shl 8)
        mov     edi, [edi+input]        
        mov     dx, word[edi+4]                
        mov     word[ConfPacket+2], dx
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch  
        
  .ftdi_set_latency_timer:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_LATENCY_TIMER_REQUEST shl 8)
        mov     edi, [edi+input]        
        mov     dx, word[edi+4]                
        mov     word[ConfPacket+2], dx
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch
        
  .ftdi_set_event_char:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_EVENT_CHAR_REQUEST shl 8)
        mov     edi, [edi+input]        
        mov     dx, word[edi+4]                
        mov     word[ConfPacket+2], dx
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch

  .ftdi_set_error_char:
        mov     dword[ConfPacket], (FTDI_DEVICE_OUT_REQTYPE) + (SIO_SET_ERROR_CHAR_REQUEST shl 8)
        mov     edi, [edi+input]        
        mov     dx, word[edi+4]                
        mov     word[ConfPacket+2], dx
        mov     dword[ConfPacket+4], 0
        jmp     .ftdi_out_control_transfer         
        jmp     .endswitch           
        

  .ftdi_read_pins:
        DEBUGF 1,'K : FTDI Reading pins\n'
        mov     dword[ConfPacket], FTDI_DEVICE_IN_REQTYPE + (SIO_READ_PINS_REQUEST shl 8) + (0 shl 16)
        mov     dword[ConfPacket+4], 0x00000001
        mov     edi, [edi+input]                                  
        mov     ebx, [edi] 
        lea     esi, [ConfPacket]
        lea     edi, [EventData]
        mov     ecx, [ioctl]
        mov     ecx, [ecx+output]
        stdcall USBControlTransferAsync, [ebx + ftdi_context.nullP],  esi, ecx, 1, control_callback, edi, 0
        DEBUGF 1, 'K : Returned value is %d\n', eax
        mov     eax, [EventData]
        mov     ebx, [EventData+4]
        call    WaitEvent     
        jmp     .endswitch
        
   .ftdi_read_data:
        ;stdcall USBNormalTransferAsync, [ebx + ftdi_context.inEP], [ebx + ftdi_context.readBufPtr], [ebx + 1]

   .ftdi_get_list:
        call    linkedlist.gethead                        
        mov     edi, [edi+output]
        mov     [edi], eax
        DEBUGF 1, 'K : FTDI Device pointer %x\n', [edi]
        mov     eax, 4        
        mov     [edi+out_size], eax
        jmp     .endswitch
        	   
endp 
restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size 


align 4
proc control_callback stdcall uses ebx edi esi, .pipe:DWORD, .status:DWORD, .buffer:DWORD, .length:DWORD, .calldata:DWORD   
   
        DEBUGF 1, 'K : status is %d\n', [.status] 
        mov     ecx, [.calldata]
        mov     eax, [ecx]
        mov     ebx, [ecx+4]
        xor     edx, edx
        call    RaiseEvent
              
        ret
endp


proc DeviceDisconnected stdcall uses  ebx esi edi, .device_data:DWORD

        DEBUGF 1, 'K : FTDI deleting device data\n'
        mov     eax, [.device_data] 
        call    linkedlist.delete
        ret           
endp

include 'linkedlist.inc'
        
; Exported variable: kernel API version.
align 4
version dd      50005h
; Structure with callback functions.
usb_functions:
        dd      12
        dd      AddDevice
        dd      DeviceDisconnected

;for DEBUGF macro
include_debug_strings




; for uninitialized data
;section '.data' data readable writable align 16
