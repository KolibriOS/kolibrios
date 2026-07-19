;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2026. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  usbcdc.asm - USB CDC class driver for KolibriOS                ;;
;;                                                                 ;;
;;  Supports:                                                      ;;
;;   * CDC-NCM  (subclass 0Dh) - USB network adapters; registers   ;;
;;     an ethernet device in the network stack. Written for        ;;
;;     pico-usb-wifi (Raspberry Pi Pico W driverless USB Wi-Fi     ;;
;;     adapter, https://gitlab.com/baiyibai/pico-usb-wifi), works  ;;
;;     with any spec-conforming NTB16 CDC-NCM device.              ;;
;;   * CDC-ACM  (subclass 02h) - USB serial ports, exposed to      ;;
;;     userspace through the IOCTL interface of service 'usbcdc'   ;;
;;     (see IOCTL codes below). Used for the pico-usb-wifi         ;;
;;     management/debug consoles and generic CDC serial adapters.  ;;
;;                                                                 ;;
;;  The driver is loaded by usbother.sys according to the class    ;;
;;  table in /sys/settings/usbdrv.dat (class 02h -> 'usbcdc').     ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

        API_VERSION             = 0x00010001

DEBUG = 1
; for DEBUGF macro from 'fdo.inc'
__DEBUG__ = 1
__DEBUG_LEVEL__ = 2             ; 1 = verbose, 2 = errors/events only

include '../../struct.inc'

; USB descriptor types
DEVICE_DESCR_TYPE       = 1
CONFIG_DESCR_TYPE       = 2
STRING_DESCR_TYPE       = 3
INTERFACE_DESCR_TYPE    = 4
ENDPOINT_DESCR_TYPE     = 5
IAD_DESCR_TYPE          = 11
CS_INTERFACE_DESCR_TYPE = 24h

; pipe types for USBOpenPipe
CONTROL_PIPE            = 0
ISOCHRONOUS_PIPE        = 1
BULK_PIPE               = 2
INTERRUPT_PIPE          = 3

; transfer callback status codes
USB_STATUS_OK           = 0
USB_STATUS_STALL        = 4
USB_STATUS_CLOSED       = 16
USB_STATUS_CANCELLED    = 17

; CDC subclasses of the Communications interface class (02h)
CDC_SUBCLASS_ACM        = 02h
CDC_SUBCLASS_ECM        = 06h
CDC_SUBCLASS_NCM        = 0Dh

; CDC class-specific functional descriptor subtypes
CDC_FUNC_UNION          = 06h
CDC_FUNC_ETHERNET       = 0Fh
CDC_FUNC_NCM            = 1Ah

; CDC class-specific requests
REQ_SET_CONTROL_LINE_STATE = 22h
REQ_GET_NTB_PARAMETERS     = 80h

; CDC notifications
NOTIFY_NETWORK_CONNECTION  = 00h
NOTIFY_SPEED_CHANGE        = 2Ah

; NCM Transfer Block signatures
NTH16_SIGNATURE         = 'NCMH'
NDP16_SIGNATURE_NOCRC   = 'NCM0'
NDP16_SIGNATURE_CRC     = 'NCM1'

; limits
ETH_FRAME_MIN           = 60    ; minimal ethernet frame (without FCS)
ETH_FRAME_MAX           = 1514  ; maximal ethernet frame (without FCS)
NCM_TX_MAX_PENDING      = 8     ; max NTBs in flight on the bulk OUT pipe
NCM_NTB_SANE_MAX        = 65536 ; refuse devices asking for larger NTBs
NCM_MAX_NDP_PER_NTB     = 16    ; guard against corrupt NDP chains
ACM_TX_MAX_PENDING      = 16    ; max writes in flight per ACM port
ACM_TX_MAX_CHUNK        = 1024  ; max bytes per single WRITE ioctl
ACM_RING_SIZE           = 8192  ; RX ring buffer per ACM port, power of 2
ACM_RX_CHUNK            = 512   ; single bulk IN transfer size
ACM_MAX_PORTS           = 8     ; max simultaneously connected ACM ports

ACM_MAGIC               = 'ACM0'; acm_dev.type tag; distinguishes acm_dev
                                ; from ncm_dev (whose .type = NET_TYPE_ETH)

; IOCTL codes of the 'usbcdc' service (ACM serial ports):
; 0 GETVERSION  out: dd API_VERSION
; 1 PORT_COUNT  out: dd number_of_ports_present
; 2 PORT_OPEN   in: dd port_index; asserts DTR+RTS, clears the RX ring
; 3 PORT_CLOSE  in: dd port_index; deasserts DTR+RTS
; 4 PORT_READ   in: dd port_index; out: dd bytes_returned, then data
; 5 PORT_WRITE  in: dd port_index, then data
; 6 PORT_STATUS in: dd port_index; out: dd present, dd rx_bytes_available
; all return eax = 0 on success, -1 on failure

; USB structures
struct usb_descr
bLength                 db      ?
bDescriptorType         db      ?
ends

struct config_descr
bLength                 db      ?
bDescriptorType         db      ?
wTotalLength            dw      ?
bNumInterfaces          db      ?
bConfigurationValue     db      ?
iConfiguration          db      ?
bmAttributes            db      ?
bMaxPower               db      ?
ends

struct interface_descr
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

struct endpoint_descr
bLength                 db      ?
bDescriptorType         db      ?
bEndpointAddress        db      ?
bmAttributes            db      ?
wMaxPacketSize          dw      ?
bInterval               db      ?
ends

section '.flat' code readable writable executable

include '../../proc32.inc'
include '../../peimport.inc'
include '../../fdo.inc'
include '../../macros.inc'
; NET_DEVICE/ETH_DEVICE/NET_BUFF structures and link state constants.
; Note: netdrv.inc prepends 'usbcdc: ' (my_service) to all DEBUGF output.
include '../../netdrv.inc'

; Device context of one CDC-NCM function (network adapter).
; Starts with ETH_DEVICE so that the structure can be passed
; directly to NetRegDev/NetUnRegDev/EthInput (in ebx).
struct ncm_dev          ETH_DEVICE

        ConfigPipe      dd      ?       ; pipe 0 handle, controls the device
        NotifyPipe      dd      ?       ; interrupt IN pipe (notifications)
        InPipe          dd      ?       ; bulk IN pipe (NTBs device->host)
        OutPipe         dd      ?       ; bulk OUT pipe (NTBs host->device)

        CtrlItf         dd      ?       ; bInterfaceNumber of control iface
        DataItf         dd      ?       ; bInterfaceNumber of data iface

        EpNotify        dd      ?       ; notification endpoint address
        EpNotifySize    dd      ?       ;  and max packet size
        EpNotifyIvl     dd      ?       ;  and polling interval
        EpIn            dd      ?       ; bulk IN endpoint address
        EpInSize        dd      ?
        EpOut           dd      ?       ; bulk OUT endpoint address
        EpOutSize       dd      ?

        iMacString      dd      ?       ; string descriptor index of the MAC
        MaxSegment      dd      ?       ; wMaxSegmentSize from ETH func descr
        NtbInMax        dd      ?       ; dwNtbInMaxSize from NTB parameters
        NtbOutMax       dd      ?       ; dwNtbOutMaxSize from NTB parameters

        TxSeq           dd      ?       ; NTH16 wSequence counter
        TxPending       dd      ?       ; NTBs currently in flight

        Dead            dd      ?       ; set on disconnect/fatal error
        Registered      dd      ?       ; set after successful NetRegDev

        RxBuf           dd      ?       ; NTB receive buffer (KernelAlloc)
        RxSize          dd      ?       ; its size, NtbInMax rounded up to 64

        Setup           rb      8       ; setup packet for init requests
        CtrlBuf         rb      64      ; buffer for control request data
        NotifBuf        rb      16      ; buffer for interrupt notifications

ends

; Device context of one CDC-ACM function (serial port).
struct acm_dev

        type            dd      ?       ; = ACM_MAGIC

        ConfigPipe      dd      ?
        InPipe          dd      ?
        OutPipe         dd      ?

        CtrlItf         dd      ?
        DataItf         dd      ?

        EpIn            dd      ?
        EpInSize        dd      ?
        EpOut           dd      ?
        EpOutSize       dd      ?

        PortIdx         dd      ?       ; index in the ports table
        Opened          dd      ?       ; DTR asserted by an application
        Dead            dd      ?
        TxPending       dd      ?

        RxRing          dd      ?       ; ring buffer (KernelAlloc)
        RxHead          dd      ?       ; write position (producer = USB)
        RxTail          dd      ?       ; read position (consumer = IOCTL)
        RxDrops         dd      ?       ; bytes dropped on ring overflow
        RxArmed         dd      ?       ; a bulk IN read is currently pending

        RxChunk         rb      ACM_RX_CHUNK    ; bulk IN transfer buffer

ends

; Scratch structure filled by parse_cdc_function while walking the
; configuration descriptor. AddDevice calls are serialized by the USB
; event thread, so a single static instance is sufficient.
struct cdc_parse

        DataItf         dd      ?       ; -1 if no union descriptor found
        WantedAlt       dd      ?       ; data iface alt setting to look for
        iMacString      dd      ?
        MaxSegment      dd      ?
        EpNotify        dd      ?
        EpNotifySize    dd      ?
        EpNotifyIvl     dd      ?
        EpIn            dd      ?
        EpInSize        dd      ?
        EpOut           dd      ?
        EpOutSize       dd      ?

ends

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        xor     eax, eax
        cmp     [reason], DRV_ENTRY
        jne     .nothing

        mov     ecx, ports_mutex
        invoke  MutexInit

        DEBUGF  2,"loading (CDC-NCM network + CDC-ACM serial)\n"
        invoke  RegUSBDriver, my_service, service_proc, usb_functions

  .nothing:
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; AddDevice: called by the USB stack (through usbother.sys) for   ;;
;; every interface of class 02h. Dispatches on the subclass.       ;;
;;                                                                 ;;
;; IN: pipe0 = config pipe handle, config -> config descriptor,    ;;
;;     interface -> interface descriptor inside the config data    ;;
;; OUT: eax = device context or 0 on failure                       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc AddDevice stdcall uses ebx esi edi, .pipe0:dword, .config:dword, .interface:dword

        mov     esi, [.interface]
        DEBUGF  2,"AddDevice: class %x subclass %x protocol %x\n",\
                [esi+interface_descr.bInterfaceClass]:2,\
                [esi+interface_descr.bInterfaceSubClass]:2,\
                [esi+interface_descr.bInterfaceProtocol]:2

        cmp     [esi+interface_descr.bInterfaceClass], 2
        jne     .decline

        cmp     [esi+interface_descr.bInterfaceSubClass], CDC_SUBCLASS_NCM
        je      .ncm
        cmp     [esi+interface_descr.bInterfaceSubClass], CDC_SUBCLASS_ACM
        jne     .decline
; RNDIS masquerades as ACM with bInterfaceProtocol = FFh; real ACM uses 0/1/2.
        cmp     [esi+interface_descr.bInterfaceProtocol], 2
        ja      .decline

        stdcall acm_add_device, [.pipe0], [.config], esi
        ret

  .ncm:
        stdcall ncm_add_device, [.pipe0], [.config], esi
        ret

  .decline:
        DEBUGF  2,"interface declined\n"
        xor     eax, eax
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; parse_cdc_function: walk the configuration descriptor starting  ;;
;; from the control interface, collecting the CDC functional       ;;
;; descriptors, the notification endpoint and the bulk endpoints   ;;
;; of the data interface (alt setting cdc_parse.WantedAlt).        ;;
;;                                                                 ;;
;; IN: edi -> cdc_parse (WantedAlt/DataItf preset, rest zeroed)    ;;
;; OUT: eax = 1 if data iface and both bulk endpoints found        ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc parse_cdc_function stdcall uses ebx esi, .config:dword, .interface:dword

locals
        in_ctrl         dd      ?       ; still inside the control interface
        in_data         dd      ?       ; inside data iface with wanted alt
endl

        mov     [in_ctrl], 1
        mov     [in_data], 0

; compute the end of configuration data
        mov     ebx, [.config]
        movzx   ecx, [ebx+config_descr.wTotalLength]
        add     ebx, ecx                ; ebx = end pointer

        mov     esi, [.interface]       ; start from the control interface
  .next:
        movzx   ecx, [esi+usb_descr.bLength]
        test    ecx, ecx
        jz      .done                   ; malformed descriptor, stop
        add     esi, ecx
        lea     ecx, [esi+2]
        cmp     ecx, ebx
        ja      .done                   ; no space even for a header

        movzx   ecx, [esi+usb_descr.bLength]
        lea     ecx, [esi+ecx]
        cmp     ecx, ebx
        ja      .done                   ; descriptor sticks out of the config

        movzx   ecx, [esi+usb_descr.bDescriptorType]
        cmp     ecx, CS_INTERFACE_DESCR_TYPE
        je      .cs_interface
        cmp     ecx, INTERFACE_DESCR_TYPE
        je      .itf_descr
        cmp     ecx, ENDPOINT_DESCR_TYPE
        je      .endpoint
        jmp     .next                   ; IADs and others: skip

; class-specific descriptors are only interesting inside the control iface
  .cs_interface:
        cmp     [in_ctrl], 0
        je      .next
        cmp     [esi+usb_descr.bLength], 4
        jb      .next
        movzx   ecx, byte [esi+2]       ; bDescriptorSubtype
        cmp     ecx, CDC_FUNC_UNION
        je      .union
        cmp     ecx, CDC_FUNC_ETHERNET
        je      .ethernet
        jmp     .next
  .union:
        cmp     [esi+usb_descr.bLength], 5
        jb      .next
        movzx   ecx, byte [esi+4]       ; bSubordinateInterface0
        mov     [edi+cdc_parse.DataItf], ecx
        jmp     .next
  .ethernet:
        cmp     [esi+usb_descr.bLength], 13
        jb      .next
        movzx   ecx, byte [esi+3]       ; iMACAddress
        mov     [edi+cdc_parse.iMacString], ecx
        movzx   ecx, word [esi+8]       ; wMaxSegmentSize
        mov     [edi+cdc_parse.MaxSegment], ecx
        jmp     .next

  .itf_descr:
        mov     [in_ctrl], 0
        mov     [in_data], 0
        movzx   ecx, [esi+interface_descr.bInterfaceNumber]
        cmp     ecx, [edi+cdc_parse.DataItf]
        jne     .next
        movzx   ecx, [esi+interface_descr.bAlternateSetting]
        cmp     ecx, [edi+cdc_parse.WantedAlt]
        jne     .next
        mov     [in_data], 1
        jmp     .next

  .endpoint:
        cmp     [esi+usb_descr.bLength], 7
        jb      .next
        cmp     [in_ctrl], 0
        jne     .ep_ctrl
        cmp     [in_data], 0
        je      .next
; bulk endpoint of the data interface
        mov     cl, [esi+endpoint_descr.bmAttributes]
        and     cl, 3
        cmp     cl, BULK_PIPE
        jne     .next
        movzx   ecx, [esi+endpoint_descr.bEndpointAddress]
        movzx   edx, [esi+endpoint_descr.wMaxPacketSize]
        and     edx, 0x7FF
        test    cl, 80h
        jz      .ep_out
        mov     [edi+cdc_parse.EpIn], ecx
        mov     [edi+cdc_parse.EpInSize], edx
        jmp     .ep_have_both
  .ep_out:
        mov     [edi+cdc_parse.EpOut], ecx
        mov     [edi+cdc_parse.EpOutSize], edx
  .ep_have_both:
        cmp     [edi+cdc_parse.EpIn], 0
        je      .next
        cmp     [edi+cdc_parse.EpOut], 0
        je      .next
        jmp     .done                   ; found everything we came for
; interrupt IN endpoint of the control interface
  .ep_ctrl:
        mov     cl, [esi+endpoint_descr.bmAttributes]
        and     cl, 3
        cmp     cl, INTERRUPT_PIPE
        jne     .next
        movzx   ecx, [esi+endpoint_descr.bEndpointAddress]
        test    cl, 80h
        jz      .next
        mov     [edi+cdc_parse.EpNotify], ecx
        movzx   ecx, [esi+endpoint_descr.wMaxPacketSize]
        and     ecx, 0x7FF
        mov     [edi+cdc_parse.EpNotifySize], ecx
        movzx   ecx, [esi+endpoint_descr.bInterval]
        mov     [edi+cdc_parse.EpNotifyIvl], ecx
        jmp     .next

  .done:
        xor     eax, eax
        cmp     [edi+cdc_parse.DataItf], -1
        je      .ret
        cmp     [edi+cdc_parse.EpIn], 0
        je      .ret
        cmp     [edi+cdc_parse.EpOut], 0
        je      .ret
        inc     eax
  .ret:
        ret

endp

; Prepare the static parse scratch: zero it, DataItf = -1, WantedAlt = eax.
; OUT: edi -> cdc_parse
proc parse_prepare
        push    eax ecx
        mov     edi, parse_scratch
        mov     ecx, sizeof.cdc_parse/4
        xor     eax, eax
        rep stosd
        mov     edi, parse_scratch
        mov     [edi+cdc_parse.DataItf], -1
        pop     ecx eax
        mov     [edi+cdc_parse.WantedAlt], eax
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;                      CDC-NCM network device                     ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc ncm_add_device stdcall uses ebx esi edi, .pipe0:dword, .config:dword, .interface:dword

; 1. Parse the descriptors of this CDC function.
        mov     eax, 1                  ; NCM data lives in alt setting 1
        call    parse_prepare
        stdcall parse_cdc_function, [.config], [.interface]
        test    eax, eax
        jz      .no_parse
        cmp     [edi+cdc_parse.iMacString], 0   ; MAC string is mandatory
        je      .no_parse
        cmp     [edi+cdc_parse.EpNotify], 0     ; notify EP is mandatory
        je      .no_parse

; 2. Allocate and clear the device context.
        mov     eax, sizeof.ncm_dev
        invoke  Kmalloc
        test    eax, eax
        jz      .no_memory
        mov     ebx, eax

        push    edi
        mov     edi, ebx
        mov     ecx, sizeof.ncm_dev/4
        xor     eax, eax
        rep stosd
        pop     edi

        mov     eax, [.pipe0]
        mov     [ebx+ncm_dev.ConfigPipe], eax
        mov     esi, [.interface]
        movzx   eax, [esi+interface_descr.bInterfaceNumber]
        mov     [ebx+ncm_dev.CtrlItf], eax
        mov     eax, [edi+cdc_parse.DataItf]
        mov     [ebx+ncm_dev.DataItf], eax
        mov     eax, [edi+cdc_parse.iMacString]
        mov     [ebx+ncm_dev.iMacString], eax
        mov     eax, [edi+cdc_parse.MaxSegment]
        mov     [ebx+ncm_dev.MaxSegment], eax
        mov     eax, [edi+cdc_parse.EpNotify]
        mov     [ebx+ncm_dev.EpNotify], eax
        mov     eax, [edi+cdc_parse.EpNotifySize]
        mov     [ebx+ncm_dev.EpNotifySize], eax
        mov     eax, [edi+cdc_parse.EpNotifyIvl]
        mov     [ebx+ncm_dev.EpNotifyIvl], eax
        mov     eax, [edi+cdc_parse.EpIn]
        mov     [ebx+ncm_dev.EpIn], eax
        mov     eax, [edi+cdc_parse.EpInSize]
        mov     [ebx+ncm_dev.EpInSize], eax
        mov     eax, [edi+cdc_parse.EpOut]
        mov     [ebx+ncm_dev.EpOut], eax
        mov     eax, [edi+cdc_parse.EpOutSize]
        mov     [ebx+ncm_dev.EpOutSize], eax

; 3. Fill the NET_DEVICE part.
        mov     [ebx+ncm_dev.type], NET_TYPE_ETH
        mov     [ebx+ncm_dev.mtu], 1514
        mov     [ebx+ncm_dev.name], netdev_name
        mov     [ebx+ncm_dev.unload], ncm_unload
        mov     [ebx+ncm_dev.reset], ncm_reset
        mov     [ebx+ncm_dev.transmit], ncm_transmit
        mov     [ebx+ncm_dev.state], ETH_LINK_DOWN

        DEBUGF  2,"NCM: ctrl itf %u, data itf %u, EP in %x out %x notify %x\n",\
                [ebx+ncm_dev.CtrlItf], [ebx+ncm_dev.DataItf],\
                [ebx+ncm_dev.EpIn], [ebx+ncm_dev.EpOut], [ebx+ncm_dev.EpNotify]

; 4. Start the asynchronous init chain: read the MAC address string
; descriptor (12 UTF-16 hex digits; per the ECM/NCM specs the host
; network interface adopts this MAC).
        lea     esi, [ebx+ncm_dev.Setup]
        mov     byte [esi], 80h                 ; IN, standard, device
        mov     byte [esi+1], 6                 ; GET_DESCRIPTOR
        mov     eax, [ebx+ncm_dev.iMacString]
        mov     byte [esi+2], al                ; wValue low = string index
        mov     byte [esi+3], STRING_DESCR_TYPE ; wValue high = type
        mov     word [esi+4], 0x0409            ; wIndex = langid en-US
        mov     word [esi+6], 64                ; wLength

        lea     edx, [ebx+ncm_dev.CtrlBuf]
        invoke  USBControlTransferAsync, [ebx+ncm_dev.ConfigPipe], esi, edx, 64, \
                ncm_mac_callback, ebx, 1
        test    eax, eax
        jz      .transfer_failed

; Return the context. Even if the init chain fails later, the context
; stays alive so that DeviceDisconnected can clean up properly.
        mov     eax, ebx
        ret

  .transfer_failed:
        DEBUGF  2,"NCM: control transfer submission failed\n"
        mov     eax, ebx
        invoke  Kfree
        xor     eax, eax
        ret

  .no_parse:
        DEBUGF  2,"NCM: incomplete descriptors, ignoring device\n"
        xor     eax, eax
        ret

  .no_memory:
        DEBUGF  2,"NCM: out of memory\n"
        xor     eax, eax
        ret

endp

; Convert an ASCII hex digit in al to a nibble; sets CF on error.
proc hexdigit
        cmp     al, '0'
        jb      .bad
        cmp     al, '9'
        jbe     .digit
        or      al, 20h                 ; tolower
        cmp     al, 'a'
        jb      .bad
        cmp     al, 'f'
        ja      .bad
        sub     al, 'a' - 10
        clc
        ret
  .digit:
        sub     al, '0'
        clc
        ret
  .bad:
        stc
        ret
endp

; Init chain step 1 done: MAC string descriptor has been read.
proc ncm_mac_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .ret
        cmp     [.status], 0
        jne     .fail

; The callback length includes the 8 setup bytes; the string descriptor
; must contain at least a 2-byte header + 12 UTF-16 hex digits.
        cmp     [.length], 8 + 2 + 12*2
        jb      .fail

        lea     esi, [ebx+ncm_dev.CtrlBuf]
        cmp     byte [esi+1], STRING_DESCR_TYPE
        jne     .fail
        movzx   ecx, byte [esi]                 ; bLength
        cmp     ecx, 2 + 12*2
        jb      .fail

; Parse 12 hex UTF-16LE characters into 6 MAC bytes.
        add     esi, 2
        lea     edi, [ebx+ncm_dev.mac]
        mov     ecx, 6
  .mac_loop:
        mov     ax, word [esi]
        test    ah, ah
        jnz     .fail
        call    hexdigit
        jc      .fail
        shl     al, 4
        mov     dl, al
        mov     ax, word [esi+2]
        test    ah, ah
        jnz     .fail
        call    hexdigit
        jc      .fail
        or      dl, al
        mov     [edi], dl
        add     esi, 4
        inc     edi
        dec     ecx
        jnz     .mac_loop

        DEBUGF  2,"NCM: MAC %x-%x-%x-%x-%x-%x\n",\
                [ebx+ncm_dev.mac+0]:2,[ebx+ncm_dev.mac+1]:2,[ebx+ncm_dev.mac+2]:2,\
                [ebx+ncm_dev.mac+3]:2,[ebx+ncm_dev.mac+4]:2,[ebx+ncm_dev.mac+5]:2

; Init chain step 2: GET_NTB_PARAMETERS.
        lea     esi, [ebx+ncm_dev.Setup]
        mov     byte [esi], 0A1h                ; IN, class, interface
        mov     byte [esi+1], REQ_GET_NTB_PARAMETERS
        mov     word [esi+2], 0                 ; wValue
        mov     eax, [ebx+ncm_dev.CtrlItf]
        mov     word [esi+4], ax                ; wIndex = control interface
        mov     word [esi+6], 28                ; wLength

        lea     edx, [ebx+ncm_dev.CtrlBuf]
        invoke  USBControlTransferAsync, [ebx+ncm_dev.ConfigPipe], esi, edx, 28, \
                ncm_ntbparams_callback, ebx, 1
        test    eax, eax
        jz      .fail
  .ret:
        ret

  .fail:
        DEBUGF  2,"NCM: reading MAC address failed\n"
        mov     [ebx+ncm_dev.Dead], 1
        ret

endp

; Init chain step 2 done: NTB parameters have been read.
proc ncm_ntbparams_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .ret
        cmp     [.status], 0
        jne     .fail
        cmp     [.length], 8 + 28               ; setup + NTB_PARAMETERS
        jb      .fail

        lea     esi, [ebx+ncm_dev.CtrlBuf]
        test    word [esi+2], 1                 ; bmNtbFormatsSupported: NTB16
        jz      .fail

        mov     eax, [esi+4]                    ; dwNtbInMaxSize
        cmp     eax, ETH_FRAME_MAX + 28
        jb      .fail
        cmp     eax, NCM_NTB_SANE_MAX
        ja      .fail
        mov     [ebx+ncm_dev.NtbInMax], eax

        mov     eax, [esi+16]                   ; dwNtbOutMaxSize
        cmp     eax, ETH_FRAME_MAX + 32
        jb      .fail
        mov     [ebx+ncm_dev.NtbOutMax], eax

        DEBUGF  2,"NCM: NTB sizes: in %u, out %u\n", [ebx+ncm_dev.NtbInMax], [ebx+ncm_dev.NtbOutMax]

; Init chain step 3: SET_INTERFACE(data interface, alt 1) enables the
; data endpoints; per the NCM spec alt setting 0 has no endpoints.
        lea     esi, [ebx+ncm_dev.Setup]
        mov     byte [esi], 01h                 ; OUT, standard, interface
        mov     byte [esi+1], 0Bh               ; SET_INTERFACE
        mov     word [esi+2], 1                 ; wValue = alt setting 1
        mov     eax, [ebx+ncm_dev.DataItf]
        mov     word [esi+4], ax                ; wIndex = data interface
        mov     word [esi+6], 0                 ; wLength

        invoke  USBControlTransferAsync, [ebx+ncm_dev.ConfigPipe], esi, 0, 0, \
                ncm_altset_callback, ebx, 0
        test    eax, eax
        jz      .fail
  .ret:
        ret

  .fail:
        DEBUGF  2,"NCM: GET_NTB_PARAMETERS failed\n"
        mov     [ebx+ncm_dev.Dead], 1
        ret

endp

; Init chain step 3 done: data interface switched to alt setting 1.
; Open the pipes, register the network device, start receiving.
proc ncm_altset_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .ret
        cmp     [.status], 0
        jne     .fail

; 1. Open the three pipes.
        invoke  USBOpenPipe, [ebx+ncm_dev.ConfigPipe], [ebx+ncm_dev.EpNotify], \
                [ebx+ncm_dev.EpNotifySize], INTERRUPT_PIPE, [ebx+ncm_dev.EpNotifyIvl]
        test    eax, eax
        jz      .fail
        mov     [ebx+ncm_dev.NotifyPipe], eax

        invoke  USBOpenPipe, [ebx+ncm_dev.ConfigPipe], [ebx+ncm_dev.EpIn], \
                [ebx+ncm_dev.EpInSize], BULK_PIPE, 0
        test    eax, eax
        jz      .fail
        mov     [ebx+ncm_dev.InPipe], eax

        invoke  USBOpenPipe, [ebx+ncm_dev.ConfigPipe], [ebx+ncm_dev.EpOut], \
                [ebx+ncm_dev.EpOutSize], BULK_PIPE, 0
        test    eax, eax
        jz      .fail
        mov     [ebx+ncm_dev.OutPipe], eax

; 2. Allocate the NTB receive buffer (rounded up to whole packets).
        mov     eax, [ebx+ncm_dev.NtbInMax]
        add     eax, 63
        and     eax, not 63
        mov     [ebx+ncm_dev.RxSize], eax
        invoke  KernelAlloc, eax
        test    eax, eax
        jz      .fail
        mov     [ebx+ncm_dev.RxBuf], eax

; 3. Register in the network stack.
        push    ebx
        invoke  NetRegDev
        pop     ebx
        cmp     eax, -1
        je      .fail_freebuf
        mov     [ebx+ncm_dev.Registered], 1
        DEBUGF  2,"NCM: registered as network device %u\n", eax

; 4. Arm the notification pipe and the NTB receive pipe.
        lea     edx, [ebx+ncm_dev.NotifBuf]
        invoke  USBNormalTransferAsync, [ebx+ncm_dev.NotifyPipe], edx, 16, \
                ncm_notify_callback, ebx, 1
        test    eax, eax
        jnz     @f
        DEBUGF  2,"NCM: failed to arm the notification pipe\n"
  @@:
        invoke  USBNormalTransferAsync, [ebx+ncm_dev.InPipe], [ebx+ncm_dev.RxBuf], \
                [ebx+ncm_dev.RxSize], ncm_rx_callback, ebx, 1
        test    eax, eax
        jz      .fail
  .ret:
        ret

  .fail_freebuf:
        invoke  KernelFree, [ebx+ncm_dev.RxBuf]
        mov     [ebx+ncm_dev.RxBuf], 0
  .fail:
        DEBUGF  2,"NCM: device init failed\n"
        mov     [ebx+ncm_dev.Dead], 1
        ret

endp

; Interrupt endpoint callback: connection notifications.
proc ncm_notify_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .ret
        cmp     [.status], USB_STATUS_CLOSED
        je      .ret
        cmp     [.status], USB_STATUS_CANCELLED
        je      .ret
        cmp     [.status], USB_STATUS_STALL
        je      .ret                            ; don't hammer a stalled pipe
        cmp     [.status], 0
        jne     .rearm                          ; transient error: retry

        cmp     [.length], 8
        jb      .rearm

        lea     esi, [ebx+ncm_dev.NotifBuf]
        cmp     byte [esi+1], NOTIFY_NETWORK_CONNECTION
        jne     .rearm

        movzx   eax, word [esi+2]               ; wValue: 1 = connected
        test    eax, eax
        jz      .link_down
        mov     [ebx+ncm_dev.state], ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
        DEBUGF  2,"NCM: link up\n"
        jmp     .link_changed
  .link_down:
        mov     [ebx+ncm_dev.state], ETH_LINK_DOWN
        DEBUGF  2,"NCM: link down\n"
  .link_changed:
        push    ebx
        invoke  NetLinkChanged
        pop     ebx

  .rearm:
        lea     edx, [ebx+ncm_dev.NotifBuf]
        invoke  USBNormalTransferAsync, [ebx+ncm_dev.NotifyPipe], edx, 16, \
                ncm_notify_callback, ebx, 1
  .ret:
        ret

endp

; Bulk IN callback: one NCM Transfer Block received. Unpack all
; datagrams, hand them to the network stack, re-arm the transfer.
proc ncm_rx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

locals
        ndp_off         dd      ?       ; offset of the current NDP16
        ndp_count       dd      ?       ; NDPs walked, guards corrupt chains
        entry_ptr       dd      ?       ; pointer to current datagram entry
        entries_left    dd      ?
endl

        mov     ebx, [.calldata]
        DEBUGF  1,"NCM: RX cb st %u len %u\n", [.status], [.length]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .ret
        cmp     [.status], USB_STATUS_CLOSED
        je      .ret
        cmp     [.status], USB_STATUS_CANCELLED
        je      .ret
        cmp     [.status], USB_STATUS_STALL
        je      .stalled
        cmp     [.status], 0
        jne     .rearm                          ; transient error: re-arm

; Validate the NTH16 header.
        mov     esi, [ebx+ncm_dev.RxBuf]
        cmp     [.length], 12
        jb      .rearm
        cmp     dword [esi], NTH16_SIGNATURE
        jne     .bad_ntb
        cmp     word [esi+4], 12                ; wHeaderLength
        jne     .bad_ntb
        movzx   eax, word [esi+8]               ; wBlockLength
        cmp     eax, [.length]
        ja      .bad_ntb                        ; block larger than transfer
        movzx   eax, word [esi+10]              ; wNdpIndex
        mov     [ndp_off], eax
        mov     [ndp_count], 0

; Walk the NDP16 chain.
  .ndp_loop:
        mov     eax, [ndp_off]
        test    eax, eax
        jz      .rearm                          ; end of chain
        inc     [ndp_count]
        cmp     [ndp_count], NCM_MAX_NDP_PER_NTB
        ja      .rearm
        lea     edx, [eax+8]
        cmp     edx, [.length]
        ja      .rearm                          ; NDP header out of bounds
        cmp     dword [esi+eax], NDP16_SIGNATURE_NOCRC
        je      .ndp_ok
        cmp     dword [esi+eax], NDP16_SIGNATURE_CRC
        jne     .rearm
  .ndp_ok:
        movzx   ecx, word [esi+eax+4]           ; wLength of the NDP
        cmp     ecx, 16
        jb      .rearm
        lea     edx, [eax+ecx]
        cmp     edx, [.length]
        ja      .rearm                          ; NDP sticks out of transfer
        sub     ecx, 8
        shr     ecx, 2                          ; number of 4-byte entries
        mov     [entries_left], ecx
        lea     edx, [esi+eax+8]
        mov     [entry_ptr], edx
        movzx   eax, word [esi+eax+6]           ; wNextNdpIndex
        mov     [ndp_off], eax

  .entry_loop:
        cmp     [entries_left], 0
        je      .ndp_loop
        dec     [entries_left]
        mov     edx, [entry_ptr]
        movzx   eax, word [edx]                 ; wDatagramIndex
        movzx   ecx, word [edx+2]               ; wDatagramLength
        add     edx, 4
        mov     [entry_ptr], edx
        test    eax, eax
        jz      .ndp_loop                       ; terminating entry
        test    ecx, ecx
        jz      .ndp_loop
; bounds check: the datagram must lie inside the transfer
        lea     edx, [eax+ecx]
        cmp     edx, [.length]
        ja      .entry_loop
        cmp     ecx, 14
        jb      .entry_loop                     ; runt, ignore
        cmp     ecx, ETH_FRAME_MAX + 4
        ja      .entry_loop                     ; oversized, ignore

; Allocate a network buffer and copy the datagram into it.
        push    eax ecx
        add     ecx, NET_BUFF.data
        invoke  NetAlloc, ecx
        pop     ecx edx                         ; ecx = length, edx = offset
        test    eax, eax
        jz      .rx_oom
        mov     edi, eax
        mov     [edi+NET_BUFF.device], ebx
        mov     [edi+NET_BUFF.length], ecx
        mov     [edi+NET_BUFF.offset], NET_BUFF.data

        push    esi edi
        add     esi, edx                        ; esi = datagram start
        lea     edi, [edi+NET_BUFF.data]
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        pop     edi esi

; Update statistics.
        mov     ecx, [edi+NET_BUFF.length]
        add     dword [ebx+ncm_dev.bytes_rx], ecx
        adc     dword [ebx+ncm_dev.bytes_rx+4], 0
        inc     [ebx+ncm_dev.packets_rx]
        DEBUGF  1,"NCM: ->host datagram %u bytes\n", [edi+NET_BUFF.length]

; Hand the buffer to the network stack. EthInput consumes
; [esp] = buffer and returns to the address at [esp+4].
        push    ebp ebx esi
        pushd   .eth_done
        push    edi
        jmp     [EthInput]
  .eth_done:
        pop     esi ebx ebp
        jmp     .entry_loop

  .rx_oom:
        inc     [ebx+ncm_dev.packets_rx_drop]
        jmp     .entry_loop

  .bad_ntb:
        DEBUGF  1,"NCM: malformed NTB dropped\n"
  .rearm:
        invoke  USBNormalTransferAsync, [ebx+ncm_dev.InPipe], [ebx+ncm_dev.RxBuf], \
                [ebx+ncm_dev.RxSize], ncm_rx_callback, ebx, 1
        test    eax, eax
        jnz     .ret
        DEBUGF  2,"NCM: failed to re-arm RX\n"
  .ret:
        ret

  .stalled:
        DEBUGF  2,"NCM: bulk IN stalled, receive stopped\n"
        ret

endp

; Transmit callback: free the NTB, decrement the in-flight counter.
; calldata points to the allocation: [context ptr][NTB data...]
proc ncm_tx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     eax, [.calldata]
        mov     ebx, [eax]                      ; device context
        lock dec [ebx+ncm_dev.TxPending]
        cmp     [.status], 0
        je      @f
        cmp     [.status], USB_STATUS_CLOSED
        je      @f
        inc     [ebx+ncm_dev.packets_tx_err]
        DEBUGF  1,"NCM: TX error %u\n", [.status]
  @@:
        mov     eax, [.calldata]
        invoke  Kfree
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; ncm_transmit: called by the network stack to send a frame.      ;;
;;                                                                 ;;
;; IN: ebx = device context, [bufferptr] = NET_BUFF                ;;
;; OUT: eax = 0 on success                                         ;;
;;                                                                 ;;
;; The frame is wrapped into a minimal NTB16 with one datagram:    ;;
;;   NTH16 (12 bytes) + NDP16 (16 bytes) + frame at offset 28.     ;;
;; If the resulting block length is a multiple of the endpoint     ;;
;; packet size, two padding bytes are added inside the block so    ;;
;; that the transfer is terminated by a short packet.              ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc ncm_transmit stdcall bufferptr

        push    esi edi

        mov     esi, [bufferptr]
        cmp     [ebx+ncm_dev.Dead], 0
        jne     .drop
        cmp     [ebx+ncm_dev.Registered], 0
        je      .drop
        mov     ecx, [esi+NET_BUFF.length]
        cmp     ecx, 14
        jb      .err
        cmp     ecx, ETH_FRAME_MAX
        ja      .err
        DEBUGF  1,"NCM: host-> frame %u bytes\n", ecx

; Limit the number of NTBs in flight.
        lock inc [ebx+ncm_dev.TxPending]
        cmp     [ebx+ncm_dev.TxPending], NCM_TX_MAX_PENDING
        ja      .overrun

; Padded payload size (short frames are padded up to 60 bytes).
        mov     edx, ecx
        cmp     edx, ETH_FRAME_MIN
        jae     @f
        mov     edx, ETH_FRAME_MIN
  @@:
; Total block size; avoid multiples of the endpoint packet size so
; that the transfer ends in a short packet on the device side.
        lea     eax, [edx+28]
        mov     edi, [ebx+ncm_dev.EpOutSize]
        dec     edi
        test    eax, edi
        jnz     @f
        add     eax, 2
  @@:
; Allocate [context dword][NTB].
        push    ecx edx eax                     ; frame len, payload len, block len
        add     eax, 4
        invoke  Kmalloc
        pop     edx                             ; edx = block length
        test    eax, eax
        jz      .no_memory
        mov     edi, eax
        mov     [edi], ebx                      ; context for the callback
        add     edi, 4                          ; edi = NTB start

; Build the NTH16.
        mov     dword [edi], NTH16_SIGNATURE
        mov     word [edi+4], 12                ; wHeaderLength
        mov     eax, 1
        lock xadd [ebx+ncm_dev.TxSeq], eax
        mov     word [edi+6], ax                ; wSequence
        mov     word [edi+8], dx                ; wBlockLength
        mov     word [edi+10], 12               ; wNdpIndex

; Build the NDP16 with a single datagram entry.
        pop     ecx                             ; ecx = padded payload size
        mov     dword [edi+12], NDP16_SIGNATURE_NOCRC
        mov     word [edi+16], 16               ; wLength
        mov     word [edi+18], 0                ; wNextNdpIndex
        mov     word [edi+20], 28               ; wDatagramIndex
        mov     word [edi+22], cx               ; wDatagramLength
        mov     dword [edi+24], 0               ; terminating entry

; Copy the frame and zero the padding after it.
        push    esi edi
        mov     eax, [esi+NET_BUFF.length]
        add     esi, [esi+NET_BUFF.offset]
        add     edi, 28
        sub     edx, 28                         ; edx = block len - headers
        mov     ecx, eax
        rep movsb
        sub     edx, eax                        ; bytes of padding to zero
        mov     ecx, edx
        xor     eax, eax
        rep stosb
        pop     edi esi

; Submit the transfer: buffer = NTB, calldata = allocation start.
        movzx   edx, word [edi+8]               ; wBlockLength
        lea     eax, [edi-4]
        invoke  USBNormalTransferAsync, [ebx+ncm_dev.OutPipe], edi, edx, \
                ncm_tx_callback, eax, 0
        test    eax, eax
        jz      .submit_failed

; Update statistics, free the NET_BUFF, report success.
        pop     ecx                             ; original frame length
        add     dword [ebx+ncm_dev.bytes_tx], ecx
        adc     dword [ebx+ncm_dev.bytes_tx+4], 0
        inc     [ebx+ncm_dev.packets_tx]
        invoke  NetFree, [bufferptr]
        xor     eax, eax
        pop     edi esi
        ret

  .submit_failed:
        lea     eax, [edi-4]
        invoke  Kfree
        pop     ecx
        lock dec [ebx+ncm_dev.TxPending]
        jmp     .err

  .no_memory:
        pop     eax ecx                         ; drop saved payload/frame sizes
        lock dec [ebx+ncm_dev.TxPending]
        jmp     .err

  .overrun:
        lock dec [ebx+ncm_dev.TxPending]
        inc     [ebx+ncm_dev.packets_tx_ovr]
        jmp     .drop

  .err:
        inc     [ebx+ncm_dev.packets_tx_err]
  .drop:
        DEBUGF  1,"NCM: frame dropped\n"
        invoke  NetFree, [bufferptr]
        or      eax, -1
        pop     edi esi
        ret

endp

; The network stack may call these through NET_DEVICE.
align 4
ncm_unload:
        or      eax, -1
        ret

align 4
ncm_reset:
        xor     eax, eax
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;                      CDC-ACM serial port                        ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; (Re-)arm the bulk IN read for a port. Idempotent and safe against a
; concurrent completion callback: RxArmed is claimed with an atomic xchg,
; so at most one read is ever pending on RxChunk. Does nothing if the port
; is dead or a read is already armed.
; IN: ebx = acm context. Clobbers eax, edx.
proc acm_arm_rx
        cmp     [ebx+acm_dev.Dead], 0
        jne     .no
        mov     eax, 1
        xchg    eax, [ebx+acm_dev.RxArmed]      ; atomic test-and-set
        test    eax, eax
        jnz     .no                             ; already armed
        lea     edx, [ebx+acm_dev.RxChunk]
        invoke  USBNormalTransferAsync, [ebx+acm_dev.InPipe], edx, ACM_RX_CHUNK, \
                acm_rx_callback, ebx, 1
        test    eax, eax
        jnz     .no
        mov     [ebx+acm_dev.RxArmed], 0        ; submission failed, allow retry
  .no:
        ret
endp

proc acm_add_device stdcall uses ebx esi edi, .pipe0:dword, .config:dword, .interface:dword

; 1. Parse the descriptors (the ACM data interface is alt setting 0).
        xor     eax, eax
        call    parse_prepare
        stdcall parse_cdc_function, [.config], [.interface]
        test    eax, eax
        jz      .no_parse

; 2. Allocate and clear the context.
        mov     eax, sizeof.acm_dev
        invoke  Kmalloc
        test    eax, eax
        jz      .no_memory
        mov     ebx, eax

        push    edi
        mov     edi, ebx
        mov     ecx, sizeof.acm_dev/4
        xor     eax, eax
        rep stosd
        pop     edi

        mov     [ebx+acm_dev.type], ACM_MAGIC
        mov     eax, [.pipe0]
        mov     [ebx+acm_dev.ConfigPipe], eax
        mov     esi, [.interface]
        movzx   eax, [esi+interface_descr.bInterfaceNumber]
        mov     [ebx+acm_dev.CtrlItf], eax
        mov     eax, [edi+cdc_parse.DataItf]
        mov     [ebx+acm_dev.DataItf], eax
        mov     eax, [edi+cdc_parse.EpIn]
        mov     [ebx+acm_dev.EpIn], eax
        mov     eax, [edi+cdc_parse.EpInSize]
        mov     [ebx+acm_dev.EpInSize], eax
        mov     eax, [edi+cdc_parse.EpOut]
        mov     [ebx+acm_dev.EpOut], eax
        mov     eax, [edi+cdc_parse.EpOutSize]
        mov     [ebx+acm_dev.EpOutSize], eax

; 3. Allocate the RX ring.
        invoke  KernelAlloc, ACM_RING_SIZE
        test    eax, eax
        jz      .free_ctx
        mov     [ebx+acm_dev.RxRing], eax

; 4. Open the bulk pipes.
        invoke  USBOpenPipe, [ebx+acm_dev.ConfigPipe], [ebx+acm_dev.EpIn], \
                [ebx+acm_dev.EpInSize], BULK_PIPE, 0
        test    eax, eax
        jz      .free_ring
        mov     [ebx+acm_dev.InPipe], eax

        invoke  USBOpenPipe, [ebx+acm_dev.ConfigPipe], [ebx+acm_dev.EpOut], \
                [ebx+acm_dev.EpOutSize], BULK_PIPE, 0
        test    eax, eax
        jz      .free_ring
        mov     [ebx+acm_dev.OutPipe], eax

; 5. Take a slot in the ports table.
        mov     ecx, ports_mutex
        invoke  MutexLock
        xor     ecx, ecx
  .find_slot:
        cmp     [ports+ecx*4], 0
        je      .slot_found
        inc     ecx
        cmp     ecx, ACM_MAX_PORTS
        jb      .find_slot
        mov     ecx, ports_mutex
        invoke  MutexUnlock
        jmp     .free_ring                      ; no free slots
  .slot_found:
        mov     [ports+ecx*4], ebx
        mov     [ebx+acm_dev.PortIdx], ecx
        mov     ecx, ports_mutex
        invoke  MutexUnlock

; 6. Start receiving into the ring (armed for the port's lifetime).
        call    acm_arm_rx

        DEBUGF  2,"ACM: serial port %u ready (ctrl itf %u)\n",\
                [ebx+acm_dev.PortIdx], [ebx+acm_dev.CtrlItf]
        mov     eax, ebx
        ret

  .free_ring:
        invoke  KernelFree, [ebx+acm_dev.RxRing]
  .free_ctx:
        mov     eax, ebx
        invoke  Kfree
        xor     eax, eax
        ret

  .no_parse:
        DEBUGF  2,"ACM: incomplete descriptors, ignoring interface\n"
        xor     eax, eax
        ret

  .no_memory:
        xor     eax, eax
        ret

endp

; Bulk IN callback: append the received bytes to the RX ring
; (single producer; data that does not fit is dropped), re-arm.
proc acm_rx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        DEBUGF  1,"ACM: RX cb st %u len %u\n", [.status], [.length]
; Keep RxArmed = 1 until the RxChunk copy below is finished, so a concurrent
; PORT_OPEN (which also calls acm_arm_rx) cannot start a second read into
; RxChunk while this one is still being drained.
        cmp     [ebx+acm_dev.Dead], 0
        jne     .dead
        cmp     [.status], USB_STATUS_CLOSED
        je      .dead
        cmp     [.status], USB_STATUS_CANCELLED
        je      .dead
        cmp     [.status], USB_STATUS_STALL
        je      .dead
        cmp     [.status], 0
        jne     .rearm

        mov     ecx, [.length]
        test    ecx, ecx
        jz      .rearm

; used = (head - tail) and (SIZE-1); free = SIZE - 1 - used
        mov     eax, [ebx+acm_dev.RxHead]
        mov     edx, [ebx+acm_dev.RxTail]
        sub     eax, edx
        and     eax, ACM_RING_SIZE - 1
        mov     edx, ACM_RING_SIZE - 1
        sub     edx, eax                        ; edx = free bytes
        cmp     ecx, edx
        jbe     @f
        sub     ecx, edx
        add     [ebx+acm_dev.RxDrops], ecx
        mov     ecx, edx                        ; store only what fits
  @@:
        test    ecx, ecx
        jz      .rearm

        lea     esi, [ebx+acm_dev.RxChunk]
        mov     edi, [ebx+acm_dev.RxHead]
        mov     edx, [ebx+acm_dev.RxRing]
  .copy_loop:
        mov     al, [esi]
        mov     [edx+edi], al
        inc     esi
        inc     edi
        and     edi, ACM_RING_SIZE - 1
        dec     ecx
        jnz     .copy_loop
        mov     [ebx+acm_dev.RxHead], edi

  .rearm:
        mov     [ebx+acm_dev.RxArmed], 0        ; this read is done (RxChunk free)
        call    acm_arm_rx
        ret
  .dead:
        mov     [ebx+acm_dev.RxArmed], 0
        ret

endp

; Bulk OUT callback: free the write buffer.
; calldata points to the allocation: [context ptr][data...]
proc acm_tx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        DEBUGF  1,"ACM: TX cb st %u len %u\n", [.status], [.length]
        mov     eax, [.calldata]
        mov     edx, [eax]
        lock dec [edx+acm_dev.TxPending]
        invoke  Kfree
        ret

endp

; Control transfer callback: free the 8-byte setup packet allocation.
proc acm_ctrl_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        DEBUGF  1,"ACM: DTR ctrl cb st %u len %u\n", [.status], [.length]
        mov     eax, [.calldata]
        invoke  Kfree
        ret

endp

; Send SET_CONTROL_LINE_STATE with a freshly allocated setup packet
; (the packet must stay valid until the transfer completes).
; IN: ebx = acm context, eax = line state (bit 0 = DTR, bit 1 = RTS)
proc acm_set_line_state

        push    esi edx ecx eax
        mov     eax, 8
        invoke  Kmalloc
        test    eax, eax
        jz      .no_memory
        mov     esi, eax
        pop     eax
        mov     byte [esi], 21h                 ; OUT, class, interface
        mov     byte [esi+1], REQ_SET_CONTROL_LINE_STATE
        mov     word [esi+2], ax                ; wValue = line state
        mov     eax, [ebx+acm_dev.CtrlItf]
        mov     word [esi+4], ax                ; wIndex
        mov     word [esi+6], 0                 ; wLength
        invoke  USBControlTransferAsync, [ebx+acm_dev.ConfigPipe], esi, 0, 0, \
                acm_ctrl_callback, esi, 0
        DEBUGF  1,"ACM: DTR itf %u submit %x\n", [ebx+acm_dev.CtrlItf], eax
        test    eax, eax
        jnz     .done
        mov     eax, esi                        ; submission failed: free now
        invoke  Kfree
  .done:
        pop     ecx edx esi
        ret

  .no_memory:
        pop     eax ecx edx esi
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; DeviceDisconnected: called by the USB stack after all transfer  ;;
;; callbacks have completed with USB_STATUS_CLOSED.                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc DeviceDisconnected stdcall uses ebx esi edi, .devdata:dword

        mov     ebx, [.devdata]
        cmp     dword [ebx], ACM_MAGIC
        je      .acm

; --- NCM network device ---
        DEBUGF  2,"NCM: device disconnected\n"
        mov     [ebx+ncm_dev.Dead], 1
        cmp     [ebx+ncm_dev.Registered], 0
        je      @f
        mov     [ebx+ncm_dev.state], ETH_LINK_DOWN
        push    ebx
        invoke  NetLinkChanged
        pop     ebx
        push    ebx
        invoke  NetUnRegDev
        pop     ebx
        mov     [ebx+ncm_dev.Registered], 0
  @@:
        cmp     [ebx+ncm_dev.RxBuf], 0
        je      @f
        invoke  KernelFree, [ebx+ncm_dev.RxBuf]
  @@:
        mov     eax, ebx
        invoke  Kfree
        ret

; --- ACM serial port ---
  .acm:
        DEBUGF  2,"ACM: port %u disconnected\n", [ebx+acm_dev.PortIdx]
        mov     ecx, ports_mutex
        invoke  MutexLock
        mov     [ebx+acm_dev.Dead], 1
        mov     ecx, [ebx+acm_dev.PortIdx]
        cmp     [ports+ecx*4], ebx
        jne     @f
        mov     dword [ports+ecx*4], 0
  @@:
        mov     ecx, ports_mutex
        invoke  MutexUnlock
        invoke  KernelFree, [ebx+acm_dev.RxRing]
        mov     eax, ebx
        invoke  Kfree
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; service_proc: IOCTL interface for userspace (ACM serial ports). ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc service_proc stdcall uses ebx esi edi, ioctl:dword

        mov     esi, [ioctl]
        mov     eax, [esi+IOCTL.io_code]

        cmp     eax, 0                          ; GETVERSION
        jne     @f
        cmp     [esi+IOCTL.out_size], 4
        jb      .fail
        mov     eax, [esi+IOCTL.output]
        mov     dword [eax], API_VERSION
        jmp     .ok
  @@:
        cmp     eax, 1                          ; PORT_COUNT
        jne     @f
        cmp     [esi+IOCTL.out_size], 4
        jb      .fail
        mov     ecx, ports_mutex
        invoke  MutexLock
        xor     eax, eax
        xor     ecx, ecx
  .count_loop:
        cmp     [ports+ecx*4], 0
        je      .count_next
        inc     eax
  .count_next:
        inc     ecx
        cmp     ecx, ACM_MAX_PORTS
        jb      .count_loop
        mov     edx, [esi+IOCTL.output]
        mov     [edx], eax
        mov     ecx, ports_mutex
        invoke  MutexUnlock
        jmp     .ok
  @@:
        cmp     eax, 2                          ; PORT_OPEN
        je      .open
        cmp     eax, 3                          ; PORT_CLOSE
        je      .close
        cmp     eax, 4                          ; PORT_READ
        je      .read
        cmp     eax, 5                          ; PORT_WRITE
        je      .write
        cmp     eax, 6                          ; PORT_STATUS
        je      .status
        jmp     .fail

;----------------------------------------
  .open:
        call    .get_port                       ; -> ebx, ports_mutex held
        DEBUGF  1,"ACM: PORT_OPEN port %u\n", [ebx+acm_dev.PortIdx]
; a fresh session starts with an empty ring
        mov     eax, [ebx+acm_dev.RxHead]
        mov     [ebx+acm_dev.RxTail], eax
        mov     [ebx+acm_dev.Opened], 1
; make sure a bulk IN read is pending (in case the initial one never armed
; or a stall stopped it), then assert DTR+RTS so the firmware's console
; output (gated on DTR) is enabled.
        call    acm_arm_rx
        mov     eax, 3                          ; DTR + RTS
        call    acm_set_line_state
        jmp     .ok_unlock

;----------------------------------------
  .close:
        call    .get_port
        mov     [ebx+acm_dev.Opened], 0
        xor     eax, eax                        ; drop DTR + RTS
        call    acm_set_line_state
        jmp     .ok_unlock

;----------------------------------------
  .read:
        call    .get_port
        cmp     [esi+IOCTL.out_size], 5
        jb      .fail_unlock
        mov     edi, [esi+IOCTL.output]
        mov     ecx, [esi+IOCTL.out_size]
        sub     ecx, 4                          ; space for the data
; available = (head - tail) and (SIZE-1)
        mov     eax, [ebx+acm_dev.RxHead]
        mov     edx, [ebx+acm_dev.RxTail]
        sub     eax, edx
        and     eax, ACM_RING_SIZE - 1
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax                        ; clamp to available
  @@:
        mov     [edi], ecx                      ; bytes returned
        add     edi, 4
        test    ecx, ecx
        jz      .read_done
        mov     edx, [ebx+acm_dev.RxTail]
        push    esi
        mov     esi, [ebx+acm_dev.RxRing]
  .read_loop:
        mov     al, [esi+edx]
        mov     [edi], al
        inc     edi
        inc     edx
        and     edx, ACM_RING_SIZE - 1
        dec     ecx
        jnz     .read_loop
        pop     esi
        mov     [ebx+acm_dev.RxTail], edx
  .read_done:
        jmp     .ok_unlock

;----------------------------------------
  .write:
        call    .get_port
        cmp     [ebx+acm_dev.TxPending], ACM_TX_MAX_PENDING
        jae     .fail_unlock
        mov     ecx, [esi+IOCTL.inp_size]
        sub     ecx, 4                          ; minus the port index dword
        jbe     .fail_unlock
        cmp     ecx, ACM_TX_MAX_CHUNK
        ja      .fail_unlock
; allocate [context ptr][data]
        push    ecx
        lea     eax, [ecx+4]
        invoke  Kmalloc
        pop     ecx
        test    eax, eax
        jz      .fail_unlock
        mov     [eax], ebx
        push    ecx esi
        lea     edi, [eax+4]
        mov     edx, [esi+IOCTL.input]
        lea     esi, [edx+4]
        push    eax
        rep movsb
        pop     eax
        pop     esi ecx
        lock inc [ebx+acm_dev.TxPending]
        lea     edx, [eax+4]
        push    eax ecx
        invoke  USBNormalTransferAsync, [ebx+acm_dev.OutPipe], edx, ecx, \
                acm_tx_callback, eax, 0
        pop     ecx
        DEBUGF  1,"ACM: PORT_WRITE %u bytes, submit %x\n", ecx, eax
        pop     edx
        test    eax, eax
        jnz     .ok_unlock
        lock dec [ebx+acm_dev.TxPending]
        mov     eax, edx
        invoke  Kfree
        jmp     .fail_unlock

;----------------------------------------
  .status:
        call    .get_port
        cmp     [esi+IOCTL.out_size], 8
        jb      .fail_unlock
        mov     edi, [esi+IOCTL.output]
        mov     dword [edi], 1                  ; present
        mov     eax, [ebx+acm_dev.RxHead]
        mov     edx, [ebx+acm_dev.RxTail]
        sub     eax, edx
        and     eax, ACM_RING_SIZE - 1
        mov     [edi+4], eax                    ; rx bytes available
        jmp     .ok_unlock

;----------------------------------------
; helper: read dd port_index from the input buffer, lock the ports
; table and return the port context in ebx. On error discards the
; return address and jumps to the failure path.
  .get_port:
        cmp     [esi+IOCTL.inp_size], 4
        jb      .bad_port_nolock
        mov     eax, [esi+IOCTL.input]
        mov     eax, [eax]                      ; port index
        cmp     eax, ACM_MAX_PORTS
        jae     .bad_port_nolock
        push    eax
        mov     ecx, ports_mutex
        invoke  MutexLock
        pop     eax
        mov     ebx, [ports+eax*4]
        test    ebx, ebx
        jz      .bad_port
        cmp     [ebx+acm_dev.Dead], 0
        jne     .bad_port
        retn
  .bad_port:
        add     esp, 4                          ; discard the return address
        jmp     .fail_unlock
  .bad_port_nolock:
        add     esp, 4                          ; discard the return address
        jmp     .fail

;----------------------------------------
  .ok_unlock:
        mov     ecx, ports_mutex
        invoke  MutexUnlock
  .ok:
        xor     eax, eax
        ret

  .fail_unlock:
        mov     ecx, ports_mutex
        invoke  MutexUnlock
  .fail:
        or      eax, -1
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; strings and static data
my_service      db      'usbcdc', 0
netdev_name     db      'USB CDC-NCM', 0

align 4
usb_functions:
        dd      usb_functions_end - usb_functions
        dd      AddDevice
        dd      DeviceDisconnected
usb_functions_end:

data fixups
end data

align 4
ports_mutex     rd      3
ports           rd      ACM_MAX_PORTS
parse_scratch   cdc_parse

include_debug_strings
