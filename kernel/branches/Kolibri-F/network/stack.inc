;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  STACK.INC                                                      ;;
;;                                                                 ;;
;;  TCP/IP stack for KolibriOS                                     ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;     Some parts of code are based on the work of:                ;;
;;      Mike Hibbett (menuetos network stack)                      ;;
;;      Eugen Brasoveanu (solar os network stack and drivers)      ;;
;;      mike.dld (kolibrios socket code)                           ;;
;;                                                                 ;;
;;     TCP part is based on 4.4BSD                                 ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

uglobal
        net_10ms        dd ?
        net_tmr_count   dw ?
endg

DEBUG_NETWORK_ERROR     = 1
DEBUG_NETWORK_VERBOSE   = 0

NET_DEVICES_MAX         = 16
NET_BUFFERS             = 512
NET_BUFFER_SIZE         = 2048
ARP_BLOCK               = 1             ; true or false

EPHEMERAL_PORT_MIN      = 49152
EPHEMERAL_PORT_MAX      = 61000
MIN_EPHEMERAL_PORT_N    = 0x00C0        ; same in Network byte order (FIXME)
MAX_EPHEMERAL_PORT_N    = 0x48EE        ; same in Network byte order (FIXME)

; Ethernet protocol numbers
ETHER_PROTO_ARP                 = 0x0608
ETHER_PROTO_IPv4                = 0x0008
ETHER_PROTO_IPv6                = 0xDD86
ETHER_PROTO_PPP_DISCOVERY       = 0x6388
ETHER_PROTO_PPP_SESSION         = 0x6488

; Internet protocol numbers
IP_PROTO_IP             = 0
IP_PROTO_ICMP           = 1
IP_PROTO_TCP            = 6
IP_PROTO_UDP            = 17
IP_PROTO_RAW            = 255

; IP options
IP_TOS                  = 1
IP_TTL                  = 2
IP_HDRINCL              = 3

; PPP protocol numbers
PPP_PROTO_IPv4          = 0x2100
PPP_PROTO_IPV6          = 0x5780
PPP_PROTO_ETHERNET      = 666           ; FIXME

;Protocol family
AF_UNSPEC               = 0
AF_LOCAL                = 1
AF_INET4                = 2
AF_INET6                = 10
AF_PPP                  = 777           ; FIXME

; Socket types
SOCK_STREAM             = 1
SOCK_DGRAM              = 2
SOCK_RAW                = 3

; Socket level
SOL_SOCKET              = 0xffff

; Socket options
SO_ACCEPTCON            = 1 shl 0
SO_BROADCAST            = 1 shl 1
SO_DEBUG                = 1 shl 2
SO_DONTROUTE            = 1 shl 3
SO_KEEPALIVE            = 1 shl 4
SO_OOBINLINE            = 1 shl 5
SO_REUSEADDR            = 1 shl 6
SO_REUSEPORT            = 1 shl 7
SO_USELOOPBACK          = 1 shl 8
SO_BINDTODEVICE         = 1 shl 9
SO_LINGER               = 1 shl 10

SO_NONBLOCK             = 1 shl 31

; Socket flags for user calls
MSG_PEEK                = 0x02
MSG_DONTWAIT            = 0x40

; Socket States
SS_NOFDREF              = 0x0001        ; no file table ref any more
SS_ISCONNECTED          = 0x0002        ; socket connected to a peer
SS_ISCONNECTING         = 0x0004        ; in process of connecting to peer
SS_ISDISCONNECTING      = 0x0008        ; in process of disconnecting
SS_CANTSENDMORE         = 0x0010        ; can't send more data to peer
SS_CANTRCVMORE          = 0x0020        ; can't receive more data from peer
SS_RCVATMARK            = 0x0040        ; at mark on input
SS_ISABORTING           = 0x0080        ; aborting fd references - close()
SS_RESTARTSYS           = 0x0100        ; restart blocked system calls
SS_ISDISCONNECTED       = 0x0800        ; socket disconnected from peer

SS_ASYNC                = 0x1000        ; async i/o notify
SS_ISCONFIRMING         = 0x2000        ; deciding to accept connection req
SS_MORETOCOME           = 0x4000

SS_BLOCKED              = 0x8000


SOCKET_BUFFER_SIZE      = 4096*8        ; must be 4096*(power of 2) where 'power of 2' is at least 8
MAX_backlog             = 20            ; maximum backlog for stream sockets

; Error Codes
ENOBUFS                 = 1
EINPROGRESS             = 2
EOPNOTSUPP              = 4
EWOULDBLOCK             = 6
ENOTCONN                = 9
EALREADY                = 10
EINVAL                  = 11
EMSGSIZE                = 12
ENOMEM                  = 18
EADDRINUSE              = 20
EADDRNOTAVAIL           = 21
ECONNRESET              = 52
ECONNABORTED            = 53
EISCONN                 = 56
ETIMEDOUT               = 60
ECONNREFUSED            = 61

; Api protocol numbers
API_ETH                 = 0
API_IPv4                = 1
API_ICMP                = 2
API_UDP                 = 3
API_TCP                 = 4
API_ARP                 = 5
API_PPPOE               = 6
API_IPv6                = 7

; Network device types
NET_DEVICE_LOOPBACK     = 0
NET_DEVICE_ETH          = 1
NET_DEVICE_SLIP         = 2

; Network link types (link protocols)
NET_LINK_LOOPBACK       = 0
NET_LINK_MAC            = 1     ; Media access control (ethernet, isdn, ...)
NET_LINK_PPP            = 2     ; Point to Point Protocol (PPPoE, ...)
NET_LINK_IEEE802.11     = 3     ; IEEE 802.11 (WiFi)

; Hardware acceleration bits
NET_HWACC_TCP_IPv4_IN   = 1 shl 0
NET_HWACC_TCP_IPv4_OUT  = 1 shl 1

; Network frame types
NET_BUFF_LOOPBACK       = 0
NET_BUFF_ETH            = 1

struct  NET_DEVICE

        device_type     dd ?    ; Type field
        mtu             dd ?    ; Maximal Transmission Unit
        name            dd ?    ; Ptr to 0 terminated string

        unload          dd ?    ; Ptrs to driver functions
        reset           dd ?    ;
        transmit        dd ?    ;

        link_state      dd ?    ; link state (0 = no link)
        hwacc           dd ?    ; bitmask stating enabled HW accelerations (offload engines)

        bytes_tx        dq ?    ; Statistics, updated by the driver
        bytes_rx        dq ?    ;

        packets_tx      dd ?    ;
        packets_tx_err  dd ?    ; CRC errors, too long or too short frames
        packets_tx_drop dd ?    ;
        packets_tx_ovr  dd ?    ; FIFO overrun

        packets_rx      dd ?    ;
        packets_rx_err  dd ?    ; CRC errors, too long or too short frames
        packets_rx_drop dd ?    ;
        packets_rx_ovr  dd ?    ; FIFO overrun

ends

struct  NET_BUFF

        NextPtr         dd ?    ; pointer to next frame in list
        PrevPtr         dd ?    ; pointer to previous frame in list
        device          dd ?    ; ptr to NET_DEVICE structure
        type            dd ?    ; encapsulation type: e.g. Ethernet
        length          dd ?    ; size of encapsulated data
        offset          dd ?    ; offset to actual data (24 bytes for default frame)
        data            rb 0

ends


; Exactly as it says..
macro pseudo_random reg {
        add     reg, [esp]
        rol     reg, 5
        xor     reg, [timer_ticks]
;        add     reg, [CPU_FREQ]
        imul    reg, 214013
        xor     reg, 0xdeadbeef
        rol     reg, 9
}

; Network to Hardware byte order (dword)
macro ntohd reg {

        rol     word reg, 8
        rol     dword reg, 16
        rol     word reg , 8

}

; Network to Hardware byte order (word)
macro ntohw reg {

        rol     word reg, 8

}


include "queue.inc"

include "loopback.inc"
include "ethernet.inc"

include "PPPoE.inc"

include "ARP.inc"
include "IPv4.inc"
include "IPv6.inc"

include "icmp.inc"
include "udp.inc"
include "tcp.inc"

include "socket.inc"



uglobal
align 4

        net_device_count        dd ?
        net_device_list         rd NET_DEVICES_MAX

        net_buffs_free          rd NET_BUFFERS
        .current                dd ?

endg


;-----------------------------------------------------------------;
;                                                                 ;
; stack_init: Initialize all network variables                    ;
;                                                                 ;
;  IN:  /                                                         ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
stack_init:

; allocate network buffers
        stdcall kernel_alloc, NET_BUFFER_SIZE*NET_BUFFERS
        test    eax, eax
        jz      .fail

        mov     edi, net_buffs_free
        mov     ecx, NET_BUFFERS
        cld
  .loop:
        stosd
        add     eax, NET_BUFFER_SIZE
        dec     ecx
        jnz     .loop

        mov     eax, net_buffs_free
        stosd

; Init the network drivers list
        xor     eax, eax
        mov     edi, net_device_count
        mov     ecx, (NET_DEVICES_MAX + 1)
        rep stosd

        eth_init

        pppoe_init

        ipv4_init
;        ipv6_init
        icmp_init

        arp_init
        udp_init
        tcp_init

        socket_init

        loop_init

        mov     [net_tmr_count], 0
        ret

  .fail:
        DEBUGF  DEBUG_NETWORK_ERROR, "Stack init failed!\n"
        ret



; Wakeup every tick.
proc stack_handler_has_work?

        mov     eax, [timer_ticks]
        cmp     eax, [net_10ms]

        ret
endp


;-----------------------------------------------------------------;
;                                                                 ;
; stack_handler: Network handlers called from os_loop.            ;
;                                                                 ;
;  IN:  /                                                         ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
stack_handler:

        ; Test for 10ms tick
        mov     eax, [timer_ticks]
        cmp     eax, [net_10ms]
        je      .exit
        mov     [net_10ms], eax

        cmp     [net_device_count], 0
        je      .exit

        test    [net_10ms], 0x0f        ; 160ms
        jnz     .exit

        tcp_timer_160ms

        test    [net_10ms], 0x3f        ; 640ms
        jnz     .exit

        arp_decrease_entry_ttls
        ipv4_decrease_fragment_ttls

        xor     edx, edx
        mov     eax, [TCP_timer1_event]
        mov     ebx, [eax + EVENT.id]
        xor     esi, esi
        call    raise_event

  .exit:
        ret


align 4
proc net_buff_alloc stdcall, buffersize

        cmp     [buffersize], NET_BUFFER_SIZE
        ja      .too_large

        spin_lock_irqsave

        mov     eax, [net_buffs_free.current]
        cmp     eax, net_buffs_free+NET_BUFFERS*4
        jae     .out_of_mem
        mov     eax, [eax]
        add     [net_buffs_free.current], 4

        spin_unlock_irqrestore

        DEBUGF  DEBUG_NETWORK_VERBOSE, "net_buff_alloc: 0x%x\n", eax
        ret

  .out_of_mem:
        spin_unlock_irqrestore

        xor     eax, eax
        DEBUGF  DEBUG_NETWORK_ERROR, "net_buff_alloc: out of mem!\n"
        ret

  .too_large:
        xor     eax, eax
        DEBUGF  DEBUG_NETWORK_ERROR, "net_buff_alloc: too large!\n"
        ret
endp


align 4
proc net_buff_free stdcall, buffer

        DEBUGF  DEBUG_NETWORK_VERBOSE, "net_buff_free: 0x%x\n", [buffer]

        spin_lock_irqsave

        sub     [net_buffs_free.current], 4             ; move pointer backwards
        mov     eax, [net_buffs_free.current]           ; place free'd buffer pointer on the list
        push    [buffer]
        pop     dword[eax]

        spin_unlock_irqrestore

        ret
endp


align 4
net_link_changed:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "net_link_changed device=0x%x status=0x%x\n", ebx, [ebx + NET_DEVICE.link_state]

align 4
net_send_event:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "net_send_event\n"

; Send event to all applications
        push    edi ecx
        mov     edi, SLOT_BASE
        mov     ecx, [thread_count]
  .loop:
        add     edi, sizeof.APPDATA
        or      [edi + APPDATA.occurred_events], EVENT_NETWORK2
        loop    .loop
        pop     ecx edi

        ret



;-----------------------------------------------------------------;
;                                                                 ;
; net_add_device: Called by network driver to register interface. ;
;                                                                 ;
;  IN:  ebx = ptr to device structure                             ;
;                                                                 ;
;  OUT: eax = device num on success                               ;
;       eax = -1 on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
net_add_device:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "net_add_device: %x\n", ebx   ;;; TODO: use mutex to lock net device list

        cmp     [net_device_count], NET_DEVICES_MAX
        jae     .error

;----------------------------------
; Check if device is already listed
        mov     eax, ebx
        mov     ecx, NET_DEVICES_MAX    ; We need to check whole list because a device may be removed without re-organizing list
        mov     edi, net_device_list

        repne scasd                     ; See if device is already in the list
        jz      .error

;----------------------------
; Find empty slot in the list
        xor     eax, eax
        mov     ecx, NET_DEVICES_MAX
        mov     edi, net_device_list

        repne scasd
        jnz     .error

        sub     edi, 4

;-----------------------------
; Add device to the found slot
        mov     [edi], ebx              ; add device to list

        mov     eax, edi                ; Calculate device number in eax
        sub     eax, net_device_list
        shr     eax, 2

        inc     [net_device_count]      ; Indicate that one more network device is up and running

        call    net_send_event

        DEBUGF  DEBUG_NETWORK_VERBOSE, "Device number: %u\n", eax
        ret

  .error:
        or      eax, -1
        DEBUGF  DEBUG_NETWORK_ERROR, "Adding network device failed\n"
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; net_remove_device: Called by network driver to unregister dev.  ;
;                                                                 ;
;  IN:  ebx = ptr to device                                       ;
;                                                                 ;
;  OUT: eax: -1 on error                                          ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
net_remove_device:

        cmp     [net_device_count], 0
        je      .error

;----------------------------
; Find the driver in the list

        mov     eax, ebx
        mov     ecx, NET_DEVICES_MAX
        mov     edi, net_device_list

        repne scasd
        jnz     .error

;------------------------
; Remove it from the list

        xor     eax, eax
        mov     dword [edi-4], eax
        dec     [net_device_count]

        call    net_send_event

        xor     eax, eax
        ret

  .error:
        or      eax, -1
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; net_ptr_to_num                                                  ;
;                                                                 ;
;  IN:  ebx = ptr to device struct                                ;
;                                                                 ;
;  OUT: edi = device number                                       ;
;       edi = -1 on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
net_ptr_to_num:

        call    net_ptr_to_num4
        ror     edi, 2          ; If -1, stay -1
                                ; valid device numbers have last two bits 0, so do just shr

        ret

align 4
net_ptr_to_num4:                ; Todo, place number in device structure so we only need to verify?

        test    ebx, ebx
        jz      .fail

        push    ecx
        mov     ecx, NET_DEVICES_MAX
        mov     edi, net_device_list
  .loop:
        cmp     ebx, [edi]
        je      .found
        add     edi, 4
        dec     ecx
        jnz     .loop

        pop     ecx
  .fail:
        or      edi, -1
        ret

  .found:
        sub     edi, net_device_list
        pop     ecx
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; checksum_1: Calculate semi-checksum for network packets.        ;
;                                                                 ;
;  IN:  edx = start offset for semi-checksum                      ;
;       esi = pointer to data                                     ;
;       ecx = data size                                           ;
;                                                                 ;
;  OUT: edx = semi-checksum                                       ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
checksum_1:

        shr     ecx, 1
        pushf
        jz      .no_2

        shr     ecx, 1
        pushf
        jz      .no_4

        shr     ecx, 1
        pushf
        jz      .no_8

  .loop:
        add     dl, [esi+1]
        adc     dh, [esi+0]

        adc     dl, [esi+3]
        adc     dh, [esi+2]

        adc     dl, [esi+5]
        adc     dh, [esi+4]

        adc     dl, [esi+7]
        adc     dh, [esi+6]

        adc     edx, 0
        add     esi, 8

        dec     ecx
        jnz     .loop

        adc     edx, 0

  .no_8:
        popf
        jnc     .no_4

        add     dl, [esi+1]
        adc     dh, [esi+0]

        adc     dl, [esi+3]
        adc     dh, [esi+2]

        adc     edx, 0
        add     esi, 4

  .no_4:
        popf
        jnc     .no_2

        add     dl, [esi+1]
        adc     dh, [esi+0]

        adc     edx, 0
        inc     esi
        inc     esi

  .no_2:
        popf
        jnc     .end

        add     dh, [esi+0]
        adc     edx, 0
  .end:
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; checksum_2: Calculate the final ip/tcp/udp checksum.            ;
;                                                                 ;
;   IN: edx = semi-checksum                                       ;
;                                                                 ;
;  OUT: dx = checksum (in INET byte order)                        ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
checksum_2:

        mov     ecx, edx
        shr     ecx, 16
        and     edx, 0xffff
        add     edx, ecx

        mov     ecx, edx
        shr     ecx, 16
        add     dx, cx
        test    dx, dx          ; it seems that ZF is not set when CF is set :(
        not     dx
        jnz     .not_zero
        dec     dx
  .not_zero:
        xchg    dl, dh

        DEBUGF  DEBUG_NETWORK_VERBOSE, "Checksum: %x\n", dx

        ret



;-----------------------------------------------------------------;
;                                                                 ;
;  System function 74: Low level access to network devices.       ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
sys_network:

        cmp     bl, 255
        jne     @f

        mov     eax, [net_device_count]
        mov     [esp+32], eax
        ret

   @@:
        cmp     bh, NET_DEVICES_MAX                     ; Check if device number exists
        jae     .doesnt_exist

        mov     esi, ebx
        and     esi, 0x0000ff00
        shr     esi, 6

        cmp     dword[esi + net_device_list], 0         ; check if device is running
        je      .doesnt_exist

        mov     eax, [esi + net_device_list]

        and     ebx, 0x000000ff
        cmp     ebx, .number
        ja      .doesnt_exist
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .get_type               ; 0
        dd      .get_dev_name           ; 1
        dd      .reset                  ; 2
        dd      .stop                   ; 3
        dd      .get_ptr                ; 4
        dd      .get_drv_name           ; 5

        dd      .packets_tx             ; 6
        dd      .packets_rx             ; 7
        dd      .bytes_tx               ; 8
        dd      .bytes_rx               ; 9
        dd      .state                  ; 10
  .number = ($ - .table) / 4 - 1

  .get_type:
        mov     eax, [eax + NET_DEVICE.device_type]
        mov     [esp+32], eax
        ret

  .get_dev_name:
; { Patch by Coldy, sanity check
        mov     ebx, eax ; eax will used for is_region_userspace return 
        stdcall is_region_userspace, ecx, 64
        jz      .bad_buffer      
        mov     esi, [ebx + NET_DEVICE.name] ;mov     esi, [eax + NET_DEVICE.name]
; } End patch by Coldy, sanity check
        mov     edi, ecx

        mov     ecx, 64/4 ; max length
        rep movsd

        xor     eax, eax
        mov     [esp+32], eax
        ret

  .reset:
        call    [eax + NET_DEVICE.reset]
        mov     [esp+32], eax
        ret

  .stop:
        call    [eax + NET_DEVICE.unload]
        mov     [esp+32], eax
        ret


  .get_ptr:
        mov     [esp+32], eax
        ret


  .get_drv_name:
        xor     eax, eax
        mov     [esp+32], eax
        ret

  .packets_tx:
        mov     eax, [eax + NET_DEVICE.packets_tx]
        mov     [esp+32], eax
        ret

  .packets_rx:
        mov     eax, [eax + NET_DEVICE.packets_rx]
        mov     [esp+32], eax
        ret

  .bytes_tx:
        mov     ebx, dword[eax + NET_DEVICE.bytes_tx + 4]
        mov     [esp+20], ebx
        mov     eax, dword[eax + NET_DEVICE.bytes_tx]
        mov     [esp+32], eax
        ret

  .bytes_rx:
        mov     ebx, dword[eax + NET_DEVICE.bytes_rx + 4]
        mov     [esp+20], ebx
        mov     eax, dword[eax + NET_DEVICE.bytes_rx]
        mov     [esp+32], eax
        ret

  .state:
        mov     eax, [eax + NET_DEVICE.link_state]
        mov     [esp+32], eax
        ret


  .doesnt_exist:
  .bad_buffer: ; Sanity check failed, exit
        mov     dword[esp+32], -1
        ret



;-----------------------------------------------------------------;
;                                                                 ;
;  System function 76: Low level access to protocol handlers.     ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
sys_protocols:
        cmp     bh, NET_DEVICES_MAX             ; Check if device number exists
        jae     .doesnt_exist

        mov     eax, ebx
        and     eax, 0x0000ff00
        shr     eax, 6                          ; now we have the device num * 4 in eax
        cmp     [eax + net_device_list], 0      ; check if device is running
        je      .doesnt_exist

        push    .return                         ; return address (we will be using jumps instead of calls)

        mov     eax, ebx                        ; set ax to protocol number
        shr     eax, 16                         ;

        cmp     ax, API_ETH
        je      eth_api

        cmp     ax, API_IPv4
        je      ipv4_api

        cmp     ax, API_ICMP
        je      icmp_api

        cmp     ax, API_UDP
        je      udp_api

        cmp     ax, API_TCP
        je      tcp_api

        cmp     ax, API_ARP
        je      arp_api

        cmp     ax, API_PPPOE
        je      pppoe_api

        cmp     ax, API_IPv6
        je      ipv6_api

        add     esp, 4                           ; if we reached here, no function was called, so we need to balance stack

  .doesnt_exist:
        mov     eax, -1

  .return:
        mov     [esp+28+4], eax                 ; return eax value to the program
        ret
