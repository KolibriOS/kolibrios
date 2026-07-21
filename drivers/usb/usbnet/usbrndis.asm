;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2026. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  usbrndis.asm - USB RNDIS host driver for KolibriOS                ;;
;;                                                                 ;;
;;  Remote NDIS (RNDIS) is the USB networking protocol used by     ;;
;;  Windows-oriented devices: LTE modems in HiLink mode (Huawei    ;;
;;  E3372 and friends), phone USB tethering, etc. The device is    ;;
;;  a self-contained router; the host sees an ethernet link and    ;;
;;  gets its address by DHCP.                                      ;;
;;                                                                 ;;
;;  Binds to control interfaces E0h/01/03 (Wireless-RNDIS) and     ;;
;;  02h/02/FF (RNDIS masquerading as CDC-ACM) through the class    ;;
;;  table of usbother.sys (/sys/settings/usbdrv.dat).              ;;
;;                                                                 ;;
;;  Control model: commands go to the device by                    ;;
;;  SEND_ENCAPSULATED_COMMAND on EP0, the device signals a         ;;
;;  RESPONSE_AVAILABLE notification on the interrupt IN endpoint,  ;;
;;  the host then fetches the reply with                           ;;
;;  GET_ENCAPSULATED_RESPONSE. Data frames travel on the bulk      ;;
;;  endpoints of the companion CDC-Data interface, each frame      ;;
;;  wrapped in a 44-byte REMOTE_NDIS_PACKET_MSG header.            ;;
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

; 1 = print per-second RX/TX rate statistics to the debug board while
; traffic flows (cb/fr = transfers/frames, B = bytes, tx = frames sent,
; wnd = advertised TCP window). Costs a few counters per frame and one
; line of board output per second; invaluable for throughput debugging.
DEBUG_STATS = 0

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

; CDC Union functional descriptor gives the data interface number
CDC_FUNC_UNION          = 06h

; RNDIS control messages
RNDIS_MSG_PACKET        = 0x00000001
RNDIS_MSG_INIT          = 0x00000002
RNDIS_MSG_QUERY         = 0x00000004
RNDIS_MSG_SET           = 0x00000005
RNDIS_MSG_INDICATE      = 0x00000007
RNDIS_MSG_KEEPALIVE     = 0x00000008
RNDIS_MSG_CMPLT         = 0x80000000

OID_802_3_PERMANENT_ADDRESS   = 0x01010101
OID_GEN_CURRENT_PACKET_FILTER = 0x0001010E

; NDIS packet filter: directed | multicast | all-multicast | broadcast
RNDIS_PACKET_FILTER     = 0x0000000F

RNDIS_STATUS_MEDIA_CONNECT    = 0x4001000B
RNDIS_STATUS_MEDIA_DISCONNECT = 0x4001000C

; init chain states
STATE_INIT              = 1     ; INITIALIZE_MSG sent
STATE_MAC               = 2     ; QUERY(PERMANENT_ADDRESS) sent
STATE_FILTER            = 3     ; SET(PACKET_FILTER) sent
STATE_RUN               = 4     ; registered, data path live

; limits
ETH_FRAME_MIN           = 60    ; minimal ethernet frame (without FCS)
ETH_FRAME_MAX           = 1514  ; maximal ethernet frame (without FCS)
RNDIS_HDR_SIZE          = 44    ; REMOTE_NDIS_PACKET_MSG header
RNDIS_RX_BUF            = 8192  ; our max transfer, advertised in INIT
RNDIS_RX_BUFFERS        = 2     ; ping-pong pair for the bulk IN pipe
RNDIS_RX_MAX_ERRORS     = 32    ; consecutive RX errors before receive
                                ; stops (anti retry-storm)
RNDIS_RSP_BUF           = 1024  ; encapsulated response buffer
RNDIS_TX_MAX_PENDING    = 32    ; max messages in flight on bulk OUT
RNDIS_GET_RETRIES       = 200   ; response polls per command (see below)

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
; Note: netdrv.inc prepends 'usbrndis: ' (my_service) to all DEBUGF output.
include '../../netdrv.inc'

; Device context of one RNDIS function. Starts with ETH_DEVICE so that
; the structure can be passed directly to NetRegDev/NetUnRegDev/EthInput.
struct rndis_dev        ETH_DEVICE

        ConfigPipe      dd      ?       ; pipe 0 handle, controls the device
        NotifyPipe      dd      ?       ; interrupt IN pipe (notifications)
        InPipe          dd      ?       ; bulk IN pipe (frames device->host)
        OutPipe         dd      ?       ; bulk OUT pipe (frames host->device)

        CtrlItf         dd      ?       ; bInterfaceNumber of control iface
        DataItf         dd      ?       ; bInterfaceNumber of data iface

        EpNotify        dd      ?       ; notification endpoint address
        EpNotifySize    dd      ?       ;  and max packet size
        EpNotifyIvl     dd      ?       ;  and polling interval
        EpIn            dd      ?       ; bulk IN endpoint address
        EpInSize        dd      ?
        EpOut           dd      ?       ; bulk OUT endpoint address
        EpOutSize       dd      ?

        InitState       dd      ?       ; STATE_* of the init chain
        DevMaxTx        dd      ?       ; device's max transfer (from INIT_C)
        LinkDown        dd      ?       ; a MEDIA_DISCONNECT was indicated

        TxPending       dd      ?       ; messages in flight on bulk OUT
        Dead            dd      ?       ; set on disconnect/fatal error
        Registered      dd      ?       ; set after successful NetRegDev

        RspPending      dd      ?       ; a GET_ENCAPSULATED_RESPONSE pending
        RspAgain        dd      ?       ; a notification arrived while pending
        KaPending       dd      ?       ; a keepalive reply is in flight
        GetRetries      dd      ?       ; polls left for the current command

        RxBufs          rd      RNDIS_RX_BUFFERS ; bulk IN buffers (KernelAlloc)
        RxSize          dd      ?       ; size of one buffer
        RxErrRun        dd      ?       ; consecutive failed RX transfers

        ; per-second statistics, only maintained when DEBUG_STATS = 1
        StatTicks       dd      ?       ; last print time, timer ticks
        StatCb          dd      ?       ; RX callbacks since last print
        StatFrames      dd      ?       ; RX frames since last print
        StatBytes       dd      ?       ; RX bytes since last print
        StatTxPkts      dd      ?       ; packets_tx snapshot at last print
        StatWnd         dd      ?       ; TCP receive window we advertised
                                        ; in the last outgoing TCP frame

        SetupCmd        rb      8       ; setup packet: SEND_ENCAPSULATED_CMD
        SetupRsp        rb      8       ; setup packet: GET_ENCAPSULATED_RSP
        SetupKa         rb      8       ; setup packet: keepalive reply
        CmdBuf          rb      64      ; init chain command messages
        KaBuf           rb      16      ; keepalive reply message
        NotifBuf        rb      16      ; interrupt notification buffer
        RspBuf          rb      RNDIS_RSP_BUF   ; encapsulated responses

ends

; Scratch structure filled by parse_rndis_function while walking the
; configuration descriptor. AddDevice calls are serialized by the USB
; event thread, so a single static instance is sufficient.
struct rndis_parse

        DataItf         dd      ?       ; -1 until found
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

        DEBUGF  2,"loading (RNDIS host network driver)\n"
        invoke  RegUSBDriver, my_service, service_proc, usb_functions

  .nothing:
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; AddDevice: called by the USB stack (through usbother.sys) for   ;;
;; every interface listed for 'usbrndis' in usbdrv.dat.               ;;
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

; Accept E0h/01/03 (Wireless Controller / RF / RNDIS) ...
        cmp     [esi+interface_descr.bInterfaceClass], 0xE0
        jne     .try_misc
        cmp     [esi+interface_descr.bInterfaceSubClass], 0x01
        jne     .decline
        cmp     [esi+interface_descr.bInterfaceProtocol], 0x03
        jne     .decline
        jmp     .accept
; ... EFh/04/01 (Miscellaneous / RNDIS over Ethernet: phone tethering) ...
  .try_misc:
        cmp     [esi+interface_descr.bInterfaceClass], 0xEF
        jne     .try_acm
        cmp     [esi+interface_descr.bInterfaceSubClass], 0x04
        jne     .decline
        cmp     [esi+interface_descr.bInterfaceProtocol], 0x01
        jne     .decline
        jmp     .accept
; ... and 02h/02/FF (RNDIS masquerading as CDC-ACM).
  .try_acm:
        cmp     [esi+interface_descr.bInterfaceClass], 0x02
        jne     .decline
        cmp     [esi+interface_descr.bInterfaceSubClass], 0x02
        jne     .decline
        cmp     [esi+interface_descr.bInterfaceProtocol], 0xFF
        jne     .decline
  .accept:

; 1. Parse the descriptors of this function.
        mov     edi, parse_scratch
        mov     ecx, sizeof.rndis_parse/4
        xor     eax, eax
        rep stosd
        mov     edi, parse_scratch
        mov     [edi+rndis_parse.DataItf], -1

        stdcall parse_rndis_function, [.config], [.interface]
        test    eax, eax
        jz      .no_parse
        cmp     [edi+rndis_parse.EpNotify], 0   ; notify EP is mandatory
        je      .no_parse

; 2. Allocate and clear the device context.
        mov     eax, sizeof.rndis_dev
        invoke  Kmalloc
        test    eax, eax
        jz      .no_memory
        mov     ebx, eax

        push    edi
        mov     edi, ebx
        mov     ecx, sizeof.rndis_dev/4
        xor     eax, eax
        rep stosd
        pop     edi

        mov     eax, [.pipe0]
        mov     [ebx+rndis_dev.ConfigPipe], eax
        mov     esi, [.interface]
        movzx   eax, [esi+interface_descr.bInterfaceNumber]
        mov     [ebx+rndis_dev.CtrlItf], eax
        mov     eax, [edi+rndis_parse.DataItf]
        mov     [ebx+rndis_dev.DataItf], eax
        mov     eax, [edi+rndis_parse.EpNotify]
        mov     [ebx+rndis_dev.EpNotify], eax
        mov     eax, [edi+rndis_parse.EpNotifySize]
        mov     [ebx+rndis_dev.EpNotifySize], eax
        mov     eax, [edi+rndis_parse.EpNotifyIvl]
        mov     [ebx+rndis_dev.EpNotifyIvl], eax
        mov     eax, [edi+rndis_parse.EpIn]
        mov     [ebx+rndis_dev.EpIn], eax
        mov     eax, [edi+rndis_parse.EpInSize]
        mov     [ebx+rndis_dev.EpInSize], eax
        mov     eax, [edi+rndis_parse.EpOut]
        mov     [ebx+rndis_dev.EpOut], eax
        mov     eax, [edi+rndis_parse.EpOutSize]
        mov     [ebx+rndis_dev.EpOutSize], eax

; 3. Fill the NET_DEVICE part.
        mov     [ebx+rndis_dev.type], NET_TYPE_ETH
        mov     [ebx+rndis_dev.mtu], 1514
        mov     [ebx+rndis_dev.name], netdev_name
        mov     [ebx+rndis_dev.unload], rndis_unload
        mov     [ebx+rndis_dev.reset], rndis_reset
        mov     [ebx+rndis_dev.transmit], rndis_transmit
        mov     [ebx+rndis_dev.state], ETH_LINK_DOWN

        DEBUGF  2,"ctrl itf %u, data itf %u, EP in %x out %x notify %x\n",\
                [ebx+rndis_dev.CtrlItf], [ebx+rndis_dev.DataItf],\
                [ebx+rndis_dev.EpIn], [ebx+rndis_dev.EpOut], [ebx+rndis_dev.EpNotify]

; 4. Open the three pipes. The data interface of an RNDIS function has
; its bulk endpoints in alt setting 0, no SET_INTERFACE is needed.
        invoke  USBOpenPipe, [ebx+rndis_dev.ConfigPipe], [ebx+rndis_dev.EpNotify], \
                [ebx+rndis_dev.EpNotifySize], INTERRUPT_PIPE, [ebx+rndis_dev.EpNotifyIvl]
        test    eax, eax
        jz      .free_ctx
        mov     [ebx+rndis_dev.NotifyPipe], eax

        invoke  USBOpenPipe, [ebx+rndis_dev.ConfigPipe], [ebx+rndis_dev.EpIn], \
                [ebx+rndis_dev.EpInSize], BULK_PIPE, 0
        test    eax, eax
        jz      .free_ctx
        mov     [ebx+rndis_dev.InPipe], eax

        invoke  USBOpenPipe, [ebx+rndis_dev.ConfigPipe], [ebx+rndis_dev.EpOut], \
                [ebx+rndis_dev.EpOutSize], BULK_PIPE, 0
        test    eax, eax
        jz      .free_ctx
        mov     [ebx+rndis_dev.OutPipe], eax

; 5. Prebuild the constant setup packets.
        lea     esi, [ebx+rndis_dev.SetupRsp]
        mov     byte [esi], 0A1h                ; IN, class, interface
        mov     byte [esi+1], 1                 ; GET_ENCAPSULATED_RESPONSE
        mov     word [esi+2], 0
        mov     eax, [ebx+rndis_dev.CtrlItf]
        mov     word [esi+4], ax
        mov     word [esi+6], RNDIS_RSP_BUF

        lea     esi, [ebx+rndis_dev.SetupKa]
        mov     byte [esi], 21h                 ; OUT, class, interface
        mov     byte [esi+1], 0                 ; SEND_ENCAPSULATED_COMMAND
        mov     word [esi+2], 0
        mov     word [esi+4], ax
        mov     word [esi+6], 16

; 6. Arm the notification pipe: the whole control protocol hangs off it.
        lea     edx, [ebx+rndis_dev.NotifBuf]
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.NotifyPipe], edx, 16, \
                rndis_notify_callback, ebx, 1
        test    eax, eax
        jz      .fail_alive

; 7. Send REMOTE_NDIS_INITIALIZE_MSG.
        lea     edi, [ebx+rndis_dev.CmdBuf]
        mov     dword [edi], RNDIS_MSG_INIT
        mov     dword [edi+4], 24               ; MessageLength
        mov     dword [edi+8], 1                ; RequestID
        mov     dword [edi+12], 1               ; MajorVersion
        mov     dword [edi+16], 0               ; MinorVersion
        mov     dword [edi+20], RNDIS_RX_BUF    ; our MaxTransferSize
        mov     [ebx+rndis_dev.InitState], STATE_INIT
        mov     eax, 24
        call    rndis_send_command
        test    eax, eax
        jz      .fail_alive

; Return the context. Even if the init chain fails later, the context
; stays alive so that DeviceDisconnected can clean up properly.
        mov     eax, ebx
        ret

; Something failed after transfers could already reference the context:
; keep it alive for DeviceDisconnected, just mark it dead.
  .fail_alive:
        DEBUGF  2,"init submission failed\n"
        mov     [ebx+rndis_dev.Dead], 1
        mov     eax, ebx
        ret

  .free_ctx:
        DEBUGF  2,"opening pipes failed\n"
        mov     eax, ebx
        invoke  Kfree
        xor     eax, eax
        ret

  .no_parse:
        DEBUGF  2,"incomplete descriptors, ignoring device\n"
        xor     eax, eax
        ret

  .no_memory:
        DEBUGF  2,"out of memory\n"
        xor     eax, eax
        ret

  .decline:
        DEBUGF  2,"interface declined\n"
        xor     eax, eax
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; parse_rndis_function: walk the configuration descriptor from    ;;
;; the control interface: pick up the interrupt IN endpoint, find  ;;
;; the data interface (by CDC Union descriptor if present, else    ;;
;; the first CDC-Data (0Ah) interface that follows) and collect    ;;
;; its bulk endpoints.                                             ;;
;;                                                                 ;;
;; IN: edi -> rndis_parse (zeroed, DataItf = -1)                   ;;
;; OUT: eax = 1 if data iface and both bulk endpoints found        ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc parse_rndis_function stdcall uses ebx esi, .config:dword, .interface:dword

locals
        in_ctrl         dd      ?       ; still inside the control interface
        in_data         dd      ?       ; inside the data interface
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

; the Union functional descriptor names the data interface
  .cs_interface:
        cmp     [in_ctrl], 0
        je      .next
        cmp     [esi+usb_descr.bLength], 5
        jb      .next
        movzx   ecx, byte [esi+2]       ; bDescriptorSubtype
        cmp     ecx, CDC_FUNC_UNION
        jne     .next
        movzx   ecx, byte [esi+4]       ; bSubordinateInterface0
        mov     [edi+rndis_parse.DataItf], ecx
        jmp     .next

  .itf_descr:
        mov     [in_ctrl], 0
        mov     [in_data], 0
        cmp     [esi+interface_descr.bAlternateSetting], 0
        jne     .next
        movzx   ecx, [esi+interface_descr.bInterfaceNumber]
        cmp     ecx, [edi+rndis_parse.DataItf]
        je      .data_itf
; no Union descriptor: take the first CDC-Data interface that follows
        cmp     [edi+rndis_parse.DataItf], -1
        jne     .next
        cmp     [esi+interface_descr.bInterfaceClass], 0x0A
        jne     .next
        mov     [edi+rndis_parse.DataItf], ecx
  .data_itf:
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
        mov     [edi+rndis_parse.EpIn], ecx
        mov     [edi+rndis_parse.EpInSize], edx
        jmp     .ep_have_both
  .ep_out:
        mov     [edi+rndis_parse.EpOut], ecx
        mov     [edi+rndis_parse.EpOutSize], edx
  .ep_have_both:
        cmp     [edi+rndis_parse.EpIn], 0
        je      .next
        cmp     [edi+rndis_parse.EpOut], 0
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
        mov     [edi+rndis_parse.EpNotify], ecx
        movzx   ecx, [esi+endpoint_descr.wMaxPacketSize]
        and     ecx, 0x7FF
        mov     [edi+rndis_parse.EpNotifySize], ecx
        movzx   ecx, [esi+endpoint_descr.bInterval]
        mov     [edi+rndis_parse.EpNotifyIvl], ecx
        jmp     .next

  .done:
        xor     eax, eax
        cmp     [edi+rndis_parse.DataItf], -1
        je      .ret
        cmp     [edi+rndis_parse.EpIn], 0
        je      .ret
        cmp     [edi+rndis_parse.EpOut], 0
        je      .ret
        inc     eax
  .ret:
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;                      RNDIS control channel                      ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Send the message prepared in CmdBuf with SEND_ENCAPSULATED_COMMAND.
; IN: ebx = device context, eax = message length
; OUT: eax = 0 if submission failed
proc rndis_send_command

        push    esi edx ecx
        mov     [ebx+rndis_dev.GetRetries], RNDIS_GET_RETRIES
        lea     esi, [ebx+rndis_dev.SetupCmd]
        mov     byte [esi], 21h                 ; OUT, class, interface
        mov     byte [esi+1], 0                 ; SEND_ENCAPSULATED_COMMAND
        mov     word [esi+2], 0
        mov     edx, [ebx+rndis_dev.CtrlItf]
        mov     word [esi+4], dx
        mov     word [esi+6], ax
        movzx   ecx, ax
        lea     edx, [ebx+rndis_dev.CmdBuf]
        invoke  USBControlTransferAsync, [ebx+rndis_dev.ConfigPipe], esi, edx, ecx, \
                rndis_cmd_callback, ebx, 0
        pop     ecx edx esi
        ret

endp

; Submit a GET_ENCAPSULATED_RESPONSE control read.
; IN: ebx = device context. OUT: eax = 0 if submission failed
proc rndis_submit_get

        push    esi edx
        lea     esi, [ebx+rndis_dev.SetupRsp]
        lea     edx, [ebx+rndis_dev.RspBuf]
        invoke  USBControlTransferAsync, [ebx+rndis_dev.ConfigPipe], esi, edx, RNDIS_RSP_BUF, \
                rndis_response_callback, ebx, 1
        pop     edx esi
        ret

endp

; SEND_ENCAPSULATED_COMMAND completion: a failure during the init chain
; is fatal, later command failures (keepalive replies) are just logged.
; On success during the init chain, start polling for the response right
; away: notifications are unreliable (VirtualBox USB passthrough loses
; interrupt IN transfers; Linux rndis_host polls for the same reason).
; The notification path stays as a shortcut and for runtime indications.
proc rndis_cmd_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [.status], 0
        je      .sent
        cmp     [.status], USB_STATUS_CLOSED
        je      .ret
        DEBUGF  2,"command transfer error %u\n", [.status]
        cmp     [ebx+rndis_dev.InitState], STATE_RUN
        jae     .ret
        mov     [ebx+rndis_dev.Dead], 1
        ret

  .sent:
        cmp     [ebx+rndis_dev.InitState], STATE_RUN
        jae     .ret                            ; keepalive reply: no response
        mov     eax, 1
        xchg    eax, [ebx+rndis_dev.RspPending] ; atomic test-and-set
        test    eax, eax
        jnz     .ret                            ; a fetch is already in flight
        call    rndis_submit_get
        test    eax, eax
        jnz     .ret
        mov     [ebx+rndis_dev.RspPending], 0   ; submission failed
  .ret:
        ret

endp

; Keepalive reply completion: allow the next one.
proc rndis_ka_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        mov     [ebx+rndis_dev.KaPending], 0
        ret

endp

; Interrupt endpoint callback: RESPONSE_AVAILABLE. Fetch the response
; unless a fetch is already in flight (then remember to fetch again:
; the device may queue several responses but we have only one buffer).
proc rndis_notify_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+rndis_dev.Dead], 0
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

        DEBUGF  1,"notification\n"
        mov     eax, 1
        xchg    eax, [ebx+rndis_dev.RspPending] ; atomic test-and-set
        test    eax, eax
        jz      .fetch
        mov     [ebx+rndis_dev.RspAgain], 1     ; fetch again when done
        jmp     .rearm
  .fetch:
        call    rndis_submit_get
        test    eax, eax
        jnz     .rearm
        mov     [ebx+rndis_dev.RspPending], 0   ; submission failed

  .rearm:
        lea     edx, [ebx+rndis_dev.NotifBuf]
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.NotifyPipe], edx, 16, \
                rndis_notify_callback, ebx, 1
  .ret:
        ret

endp

; GET_ENCAPSULATED_RESPONSE completion: dispatch on the message type.
; Completions drive the init chain forward; keepalives and status
; indications are handled in any state.
proc rndis_response_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     ebx, [.calldata]
        cmp     [ebx+rndis_dev.Dead], 0
        jne     .ret
        cmp     [.status], 0
        jne     .error

        mov     ecx, [.length]
        sub     ecx, 8                          ; minus the setup packet
        cmp     ecx, 16
        jb      .empty                          ; nothing ready yet

        lea     esi, [ebx+rndis_dev.RspBuf]
        mov     eax, [esi]                      ; MessageType
        DEBUGF  1,"response type %x len %u\n", eax, ecx
        cmp     eax, RNDIS_MSG_INIT or RNDIS_MSG_CMPLT
        je      .init_c
        cmp     eax, RNDIS_MSG_QUERY or RNDIS_MSG_CMPLT
        je      .query_c
        cmp     eax, RNDIS_MSG_SET or RNDIS_MSG_CMPLT
        je      .set_c
        cmp     eax, RNDIS_MSG_KEEPALIVE
        je      .keepalive
        cmp     eax, RNDIS_MSG_INDICATE
        je      .indicate
        jmp     .done                           ; RESET_CMPLT etc: ignore

;--------------------------------- REMOTE_NDIS_INITIALIZE_CMPLT
  .init_c:
        cmp     [ebx+rndis_dev.InitState], STATE_INIT
        jne     .done
        cmp     ecx, 52
        jb      .fail
        cmp     dword [esi+12], 0               ; Status
        jne     .fail
        mov     eax, [esi+36]                   ; device's MaxTransferSize
        mov     [ebx+rndis_dev.DevMaxTx], eax
        DEBUGF  2,"initialized, device max transfer %u\n", eax

; query the permanent MAC address
        lea     edi, [ebx+rndis_dev.CmdBuf]
        mov     dword [edi], RNDIS_MSG_QUERY
        mov     dword [edi+4], 28               ; MessageLength
        mov     dword [edi+8], 2                ; RequestID
        mov     dword [edi+12], OID_802_3_PERMANENT_ADDRESS
        mov     dword [edi+16], 0               ; InformationBufferLength
        mov     dword [edi+20], 20              ; InformationBufferOffset
        mov     dword [edi+24], 0               ; DeviceVcHandle
        mov     [ebx+rndis_dev.InitState], STATE_MAC
        mov     eax, 28
        call    rndis_send_command
        test    eax, eax
        jz      .fail
        jmp     .done

;--------------------------------- REMOTE_NDIS_QUERY_CMPLT
  .query_c:
        cmp     [ebx+rndis_dev.InitState], STATE_MAC
        jne     .done
        cmp     ecx, 24
        jb      .fail
        cmp     dword [esi+12], 0               ; Status
        jne     .fail
        cmp     dword [esi+16], 6               ; InformationBufferLength
        jb      .fail
        mov     edx, [esi+20]                   ; InformationBufferOffset
        lea     eax, [edx+8+6]                  ; buffer offsets count from
        cmp     eax, ecx                        ; the RequestID field (+8)
        ja      .fail
        push    esi
        lea     eax, [esi+edx+8]
        mov     esi, eax
        lea     edi, [ebx+rndis_dev.mac]
        movsd
        movsw
        pop     esi
        DEBUGF  2,"MAC %x-%x-%x-%x-%x-%x\n",\
                [ebx+rndis_dev.mac+0]:2,[ebx+rndis_dev.mac+1]:2,[ebx+rndis_dev.mac+2]:2,\
                [ebx+rndis_dev.mac+3]:2,[ebx+rndis_dev.mac+4]:2,[ebx+rndis_dev.mac+5]:2

; enable reception: set the packet filter
        lea     edi, [ebx+rndis_dev.CmdBuf]
        mov     dword [edi], RNDIS_MSG_SET
        mov     dword [edi+4], 32               ; MessageLength
        mov     dword [edi+8], 3                ; RequestID
        mov     dword [edi+12], OID_GEN_CURRENT_PACKET_FILTER
        mov     dword [edi+16], 4               ; InformationBufferLength
        mov     dword [edi+20], 20              ; InformationBufferOffset
        mov     dword [edi+24], 0               ; DeviceVcHandle
        mov     dword [edi+28], RNDIS_PACKET_FILTER
        mov     [ebx+rndis_dev.InitState], STATE_FILTER
        mov     eax, 32
        call    rndis_send_command
        test    eax, eax
        jz      .fail
        jmp     .done

;--------------------------------- REMOTE_NDIS_SET_CMPLT
  .set_c:
        cmp     [ebx+rndis_dev.InitState], STATE_FILTER
        jne     .done
        cmp     ecx, 16
        jb      .fail
        cmp     dword [esi+12], 0               ; Status
        jne     .fail
        call    rndis_start
        jmp     .done

;--------------------------------- REMOTE_NDIS_KEEPALIVE_MSG
; the device checks that the host is alive; reply with KEEPALIVE_CMPLT
  .keepalive:
        DEBUGF  1,"keepalive\n"
        mov     eax, 1
        xchg    eax, [ebx+rndis_dev.KaPending]  ; atomic test-and-set
        test    eax, eax
        jnz     .done                           ; previous reply still in flight
        lea     edi, [ebx+rndis_dev.KaBuf]
        mov     dword [edi], RNDIS_MSG_KEEPALIVE or RNDIS_MSG_CMPLT
        mov     dword [edi+4], 16               ; MessageLength
        mov     eax, [esi+8]                    ; echo the RequestID
        mov     [edi+8], eax
        mov     dword [edi+12], 0               ; RNDIS_STATUS_SUCCESS
        lea     edx, [ebx+rndis_dev.SetupKa]
        invoke  USBControlTransferAsync, [ebx+rndis_dev.ConfigPipe], edx, edi, 16, \
                rndis_ka_callback, ebx, 0
        test    eax, eax
        jnz     .done
        mov     [ebx+rndis_dev.KaPending], 0
        jmp     .done

;--------------------------------- REMOTE_NDIS_INDICATE_STATUS_MSG
  .indicate:
        cmp     ecx, 12
        jb      .done
        mov     eax, [esi+8]                    ; Status
        DEBUGF  1,"indication %x\n", eax
        cmp     eax, RNDIS_STATUS_MEDIA_CONNECT
        je      .media_up
        cmp     eax, RNDIS_STATUS_MEDIA_DISCONNECT
        jne     .done
        mov     [ebx+rndis_dev.LinkDown], 1
        mov     [ebx+rndis_dev.state], ETH_LINK_DOWN
        DEBUGF  2,"link down\n"
        jmp     .media_changed
  .media_up:
        mov     [ebx+rndis_dev.LinkDown], 0
        mov     [ebx+rndis_dev.state], ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
        DEBUGF  2,"link up\n"
  .media_changed:
        cmp     [ebx+rndis_dev.Registered], 0
        je      .done
        push    ebx
        invoke  NetLinkChanged
        pop     ebx
        jmp     .done

;---------------------------------
; the device had no response ready; while a command of the init chain
; is outstanding, keep polling (bounded) instead of trusting the
; notification to arrive
  .empty:
        cmp     [ebx+rndis_dev.InitState], STATE_RUN
        jae     .done
        cmp     [ebx+rndis_dev.GetRetries], 0
        je      .poll_dry
        dec     [ebx+rndis_dev.GetRetries]
        call    rndis_submit_get
        test    eax, eax
        jnz     .ret                            ; RspPending stays set
        jmp     .release
  .poll_dry:
        DEBUGF  2,"no response after polling (state %u)\n", [ebx+rndis_dev.InitState]
        mov     [ebx+rndis_dev.Dead], 1
        ret

;---------------------------------
; if another notification arrived while this response was in flight,
; fetch the queued response; otherwise release the buffer
  .done:
        xor     eax, eax
        xchg    eax, [ebx+rndis_dev.RspAgain]
        test    eax, eax
        jz      .release
        call    rndis_submit_get
        test    eax, eax
        jnz     .ret                            ; RspPending stays set
  .release:
        mov     [ebx+rndis_dev.RspPending], 0
  .ret:
        ret

  .error:
        DEBUGF  2,"response transfer error %u\n", [.status]
        jmp     .release

  .fail:
        DEBUGF  2,"init chain failed (state %u)\n", [ebx+rndis_dev.InitState]
        mov     [ebx+rndis_dev.Dead], 1
        ret

endp

; Init chain complete: allocate the receive buffers, register in the
; network stack, bring the link up and start receiving.
; IN: ebx = device context
proc rndis_start

        mov     [ebx+rndis_dev.InitState], STATE_RUN
        mov     [ebx+rndis_dev.RxSize], RNDIS_RX_BUF

        xor     edi, edi
  .alloc_loop:
        invoke  KernelAlloc, RNDIS_RX_BUF
        test    eax, eax
        jz      .fail
        mov     [ebx+rndis_dev.RxBufs+edi*4], eax
        inc     edi
        cmp     edi, RNDIS_RX_BUFFERS
        jb      .alloc_loop

        push    ebx
        invoke  NetRegDev
        pop     ebx
        cmp     eax, -1
        je      .fail
        mov     [ebx+rndis_dev.Registered], 1
        DEBUGF  2,"registered as network device %u\n", eax

; An LTE router bridge is up unless the device indicated otherwise
; before we finished initializing.
        cmp     [ebx+rndis_dev.LinkDown], 0
        jne     @f
        mov     [ebx+rndis_dev.state], ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
  @@:
        push    ebx
        invoke  NetLinkChanged
        pop     ebx

; Arm the first receive. From here on the RX callback ping-pongs between
; the two buffers: it re-arms the pipe with the other buffer BEFORE
; parsing the completed one, so the device streams the next frames while
; the host walks the current buffer. Exactly one transfer is outstanding
; at any time - queueing several short-packet-terminated bulk IN
; transfers breaks the EHCI stack (see the note in usbcdc.asm).
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.InPipe], \
                [ebx+rndis_dev.RxBufs], [ebx+rndis_dev.RxSize], \
                rndis_rx_callback, ebx, 1
        test    eax, eax
        jz      .fail
        ret

  .fail:
        DEBUGF  2,"starting the device failed\n"
        mov     [ebx+rndis_dev.Dead], 1
        ret

endp

if DEBUG_STATS
; Print transfer-rate statistics to the debug board about once a second
; while traffic flows (called from the RX callback). Shows where the
; bottleneck is: cb/fr = USB transfers and frames per interval (RX),
; tx = frames sent per interval (the TCP ACK clock on downloads).
; IN: ebx = device context
proc rndis_stats

        push    eax ecx edx
        invoke  GetTimerTicks
        mov     ecx, [ebx+rndis_dev.StatTicks]
        test    ecx, ecx
        jz      .reset
        mov     edx, eax
        sub     edx, ecx
        cmp     edx, 100                        ; print every 100 ticks = 1 s
        jb      .done
        mov     ecx, [ebx+rndis_dev.packets_tx]
        sub     ecx, [ebx+rndis_dev.StatTxPkts]
        DEBUGF  2,"stat %ut: cb %u fr %u B %u tx %u wnd %u txovr %u rxdrop %u\n", \
                edx, [ebx+rndis_dev.StatCb], [ebx+rndis_dev.StatFrames], \
                [ebx+rndis_dev.StatBytes], ecx, [ebx+rndis_dev.StatWnd], \
                [ebx+rndis_dev.packets_tx_ovr], [ebx+rndis_dev.packets_rx_drop]
        mov     [ebx+rndis_dev.StatCb], 0
        mov     [ebx+rndis_dev.StatFrames], 0
        mov     [ebx+rndis_dev.StatBytes], 0
        mov     ecx, [ebx+rndis_dev.packets_tx]
        mov     [ebx+rndis_dev.StatTxPkts], ecx
  .reset:
        mov     [ebx+rndis_dev.StatTicks], eax
  .done:
        pop     edx ecx eax
        ret

endp
end if ; DEBUG_STATS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;                        RNDIS data path                          ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Bulk IN callback: one transfer may carry several PACKET_MSGs back to
; back. Unwrap each, hand the frames to the network stack, re-arm.
proc rndis_rx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

locals
        msg_off         dd      ?       ; offset of the current message
        rearmed         dd      ?       ; the other buffer is already armed
endl

        mov     ebx, [.calldata]
        DEBUGF  1,"RX cb st %u len %u\n", [.status], [.length]
        cmp     [ebx+rndis_dev.Dead], 0
        jne     .ret
        cmp     [.status], USB_STATUS_CLOSED
        je      .ret
        cmp     [.status], USB_STATUS_CANCELLED
        je      .ret
        cmp     [.status], USB_STATUS_STALL
        je      .stalled
if DEBUG_STATS
        inc     [ebx+rndis_dev.StatCb]
end if

; Transfer error: nothing to parse, retry with the same buffer. A run
; of consecutive failures stops the receive altogether (the HC driver
; logs every failed TD, so an endless retry storm starves the whole
; single-core system). Replugging the device recovers.
        cmp     [.status], 0
        je      .status_ok
        inc     [ebx+rndis_dev.RxErrRun]
        cmp     [ebx+rndis_dev.RxErrRun], RNDIS_RX_MAX_ERRORS
        jae     .rx_dead
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.InPipe], [.buffer], \
                [ebx+rndis_dev.RxSize], rndis_rx_callback, ebx, 1
        jmp     .ret
  .status_ok:
        mov     [ebx+rndis_dev.RxErrRun], 0     ; the error run is broken

; Re-arm the pipe with the other buffer BEFORE parsing this one: the
; device streams the next frames while the host walks the current
; buffer. The completed transfer is already off the queue, so still at
; most one transfer is outstanding (a hard requirement of the EHCI
; stack, see the note in usbcdc.asm).
        mov     [rearmed], 0
        mov     eax, [ebx+rndis_dev.RxBufs]
        cmp     eax, [.buffer]
        jne     @f
        mov     eax, [ebx+rndis_dev.RxBufs+4]
  @@:
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.InPipe], eax, \
                [ebx+rndis_dev.RxSize], rndis_rx_callback, ebx, 1
        test    eax, eax
        jz      @f
        mov     [rearmed], 1
  @@:

        mov     [msg_off], 0

  .msg_loop:
        mov     esi, [.buffer]
        mov     eax, [msg_off]
        lea     edx, [eax+RNDIS_HDR_SIZE]
        cmp     edx, [.length]
        ja      .rearm                          ; no room for another header
        add     esi, eax                        ; esi -> current message
        cmp     dword [esi], RNDIS_MSG_PACKET
        jne     .rearm                          ; not a data message: stop
        mov     ecx, [esi+4]                    ; MessageLength
        cmp     ecx, RNDIS_HDR_SIZE
        jb      .rearm                          ; guarantees forward progress
        mov     edx, [msg_off]
        add     edx, ecx
        jc      .rearm
        cmp     edx, [.length]
        ja      .rearm                          ; message sticks out
        mov     [msg_off], edx                  ; next message

; extract the frame: [esi+8] = DataOffset (from the byte at offset 8),
; [esi+12] = DataLength
        mov     edx, [esi+12]
        cmp     edx, 14
        jb      .msg_loop                       ; runt, ignore
        cmp     edx, ETH_FRAME_MAX + 4
        ja      .msg_loop                       ; oversized, ignore
        mov     eax, [esi+8]
        cmp     eax, ecx
        ja      .msg_loop                       ; nonsense offset
        add     eax, 8                          ; frame offset in the message
        push    eax
        add     eax, edx
        cmp     eax, ecx                        ; frame must fit in the message
        pop     eax
        ja      .msg_loop

; allocate a network buffer and copy the frame into it
        push    eax edx
        mov     ecx, edx
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
        add     esi, edx                        ; esi -> frame start
        lea     edi, [edi+NET_BUFF.data]
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        pop     edi esi

; update statistics
        mov     ecx, [edi+NET_BUFF.length]
        add     dword [ebx+rndis_dev.bytes_rx], ecx
        adc     dword [ebx+rndis_dev.bytes_rx+4], 0
        inc     [ebx+rndis_dev.packets_rx]
if DEBUG_STATS
        inc     [ebx+rndis_dev.StatFrames]
        add     [ebx+rndis_dev.StatBytes], ecx
end if
        DEBUGF  1,"->host frame %u bytes\n", [edi+NET_BUFF.length]

; hand the buffer to the network stack. EthInput consumes
; [esp] = buffer and returns to the address at [esp+4].
        push    ebp ebx esi
        pushd   .eth_done
        push    edi
        jmp     [EthInput]
  .eth_done:
        pop     esi ebx ebp
        jmp     .msg_loop

  .rx_oom:
        inc     [ebx+rndis_dev.packets_rx_drop]
        jmp     .msg_loop

; End of processing. If arming the other buffer failed at entry, fall
; back to re-submitting the just-parsed buffer so receive does not stop.
  .rearm:
if DEBUG_STATS
        call    rndis_stats
end if
        cmp     [rearmed], 0
        jne     .ret
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.InPipe], [.buffer], \
                [ebx+rndis_dev.RxSize], rndis_rx_callback, ebx, 1
        test    eax, eax
        jnz     .ret
        DEBUGF  2,"failed to re-arm RX\n"
  .ret:
        ret

  .stalled:
        DEBUGF  2,"bulk IN stalled, receive stopped\n"
        ret

  .rx_dead:
        DEBUGF  2,"RX stopped after repeated transfer errors\n"
        mov     [ebx+rndis_dev.state], ETH_LINK_DOWN
        push    ebx
        invoke  NetLinkChanged
        pop     ebx
        ret

endp

; Transmit callback: free the message, decrement the in-flight counter.
; calldata points to the allocation: [context ptr][message...]
proc rndis_tx_callback stdcall uses ebx esi edi, .pipe:dword, .status:dword, .buffer:dword, .length:dword, .calldata:dword

        mov     eax, [.calldata]
        mov     ebx, [eax]                      ; device context
        lock dec [ebx+rndis_dev.TxPending]
        cmp     [.status], 0
        je      @f
        cmp     [.status], USB_STATUS_CLOSED
        je      @f
        inc     [ebx+rndis_dev.packets_tx_err]
        DEBUGF  1,"TX error %u\n", [.status]
  @@:
        mov     eax, [.calldata]
        invoke  Kfree
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; rndis_transmit: called by the network stack to send a frame.    ;;
;;                                                                 ;;
;; IN: ebx = device context, [bufferptr] = NET_BUFF                ;;
;; OUT: eax = 0 on success                                         ;;
;;                                                                 ;;
;; The frame is prepended with a 44-byte REMOTE_NDIS_PACKET_MSG.   ;;
;; If the transfer would be a multiple of the endpoint packet      ;;
;; size, one extra zero byte is sent (not counted in the message   ;;
;; length) so that the transfer ends in a short packet.            ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc rndis_transmit stdcall bufferptr

locals
        paylen          dd      ?       ; payload padded to ethernet minimum
        msglen          dd      ?       ; RNDIS message length
        xferlen         dd      ?       ; USB transfer length
        alloc           dd      ?       ; [context ptr][message]
endl

        push    esi edi

        mov     esi, [bufferptr]
        cmp     [ebx+rndis_dev.Dead], 0
        jne     .drop
        cmp     [ebx+rndis_dev.Registered], 0
        je      .drop
        mov     ecx, [esi+NET_BUFF.length]
        cmp     ecx, 14
        jb      .err
        cmp     ecx, ETH_FRAME_MAX
        ja      .err
        DEBUGF  1,"host-> frame %u bytes\n", ecx

; limit the number of messages in flight
        lock inc [ebx+rndis_dev.TxPending]
        cmp     [ebx+rndis_dev.TxPending], RNDIS_TX_MAX_PENDING
        ja      .overrun

; short frames are padded up to the ethernet minimum
        mov     edx, ecx
        cmp     edx, ETH_FRAME_MIN
        jae     @f
        mov     edx, ETH_FRAME_MIN
  @@:
        mov     [paylen], edx
        lea     eax, [edx+RNDIS_HDR_SIZE]
        mov     [msglen], eax
; avoid transfers that are a multiple of the endpoint packet size
        mov     edx, [ebx+rndis_dev.EpOutSize]
        dec     edx
        test    eax, edx
        jnz     @f
        inc     eax
  @@:
        mov     [xferlen], eax
; never exceed what the device declared it can receive
        mov     edx, [ebx+rndis_dev.DevMaxTx]
        test    edx, edx
        jz      @f
        cmp     eax, edx
        ja      .toobig
  @@:
; allocate [context dword][message]
        add     eax, 4
        invoke  Kmalloc
        test    eax, eax
        jz      .no_memory
        mov     [alloc], eax
        mov     [eax], ebx                      ; context for the callback
        lea     edi, [eax+4]

; build the REMOTE_NDIS_PACKET_MSG header
        mov     eax, RNDIS_MSG_PACKET
        stosd
        mov     eax, [msglen]
        stosd
        mov     eax, RNDIS_HDR_SIZE - 8         ; DataOffset
        stosd
        mov     eax, [paylen]                   ; DataLength
        stosd
        xor     eax, eax
        mov     ecx, 7                          ; OOB/PPI/VcHandle/Reserved
        rep stosd

; copy the frame, zero the padding after it
        push    esi
        mov     ecx, [esi+NET_BUFF.length]
        mov     edx, ecx
        add     esi, [esi+NET_BUFF.offset]
        rep movsb
        pop     esi
        mov     ecx, [xferlen]
        sub     ecx, RNDIS_HDR_SIZE
        sub     ecx, edx                        ; padding bytes to zero
        xor     eax, eax
        rep stosb

if DEBUG_STATS
; stats: remember the TCP receive window we advertise (rwnd health on
; downloads; parsed from the outgoing frame: eth+IPv4+TCP)
        cmp     edx, 54                         ; eth+min IP+min TCP
        jb      @f
        mov     eax, [alloc]
        lea     eax, [eax+4+RNDIS_HDR_SIZE]     ; frame start
        cmp     word [eax+12], 0x0008           ; EtherType IPv4 (BE 08 00)
        jne     @f
        cmp     byte [eax+23], 6                ; IP protocol TCP
        jne     @f
        movzx   ecx, byte [eax+14]
        and     ecx, 15                         ; IHL, dwords
        movzx   edx, word [eax+ecx*4+14+14]     ; TCP.Window, big-endian
        xchg    dl, dh
        mov     [ebx+rndis_dev.StatWnd], edx
  @@:
end if

; submit the transfer: buffer = message, calldata = allocation
        mov     eax, [alloc]
        lea     edx, [eax+4]
        invoke  USBNormalTransferAsync, [ebx+rndis_dev.OutPipe], edx, [xferlen], \
                rndis_tx_callback, eax, 0
        test    eax, eax
        jz      .submit_failed

; update statistics, free the NET_BUFF, report success
        mov     ecx, [esi+NET_BUFF.length]
        add     dword [ebx+rndis_dev.bytes_tx], ecx
        adc     dword [ebx+rndis_dev.bytes_tx+4], 0
        inc     [ebx+rndis_dev.packets_tx]
        invoke  NetFree, [bufferptr]
        xor     eax, eax
        pop     edi esi
        ret

  .submit_failed:
        mov     eax, [alloc]
        invoke  Kfree
        lock dec [ebx+rndis_dev.TxPending]
        jmp     .err

  .no_memory:
        lock dec [ebx+rndis_dev.TxPending]
        jmp     .err

  .toobig:
        lock dec [ebx+rndis_dev.TxPending]
        jmp     .err

  .overrun:
        lock dec [ebx+rndis_dev.TxPending]
        inc     [ebx+rndis_dev.packets_tx_ovr]
        jmp     .drop

  .err:
        inc     [ebx+rndis_dev.packets_tx_err]
  .drop:
        DEBUGF  1,"frame dropped\n"
        invoke  NetFree, [bufferptr]
        or      eax, -1
        pop     edi esi
        ret

endp

; The network stack may call these through NET_DEVICE.
align 4
rndis_unload:
        or      eax, -1
        ret

align 4
rndis_reset:
        xor     eax, eax
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; DeviceDisconnected: called by the USB stack after all transfer  ;;
;; callbacks have completed with USB_STATUS_CLOSED.                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc DeviceDisconnected stdcall uses ebx esi edi, .devdata:dword

        mov     ebx, [.devdata]
        DEBUGF  2,"device disconnected\n"
        mov     [ebx+rndis_dev.Dead], 1
        cmp     [ebx+rndis_dev.Registered], 0
        je      @f
        mov     [ebx+rndis_dev.state], ETH_LINK_DOWN
        push    ebx
        invoke  NetLinkChanged
        pop     ebx
        push    ebx
        invoke  NetUnRegDev
        pop     ebx
        mov     [ebx+rndis_dev.Registered], 0
  @@:
        xor     esi, esi
  .free_loop:
        mov     eax, [ebx+rndis_dev.RxBufs+esi*4]
        test    eax, eax
        jz      @f
        invoke  KernelFree, eax
  @@:
        inc     esi
        cmp     esi, RNDIS_RX_BUFFERS
        jb      .free_loop
        mov     eax, ebx
        invoke  Kfree
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; service_proc: nothing to control from userspace yet.            ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc service_proc stdcall uses ebx esi edi, ioctl:dword

        mov     esi, [ioctl]
        mov     eax, [esi+IOCTL.io_code]
        test    eax, eax                        ; 0 = GETVERSION
        jnz     .fail
        cmp     [esi+IOCTL.out_size], 4
        jb      .fail
        mov     eax, [esi+IOCTL.output]
        mov     dword [eax], API_VERSION
        xor     eax, eax
        ret

  .fail:
        or      eax, -1
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; strings and static data
my_service      db      'usbrndis', 0
netdev_name     db      'USB RNDIS', 0

align 4
usb_functions:
        dd      usb_functions_end - usb_functions
        dd      AddDevice
        dd      DeviceDisconnected
usb_functions_end:

data fixups
end data

align 4
parse_scratch   rndis_parse

include_debug_strings
