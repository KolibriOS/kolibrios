;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
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

MAX_NET_DEVICES         = 16
ARP_BLOCK               = 1             ; true or false

MIN_EPHEMERAL_PORT      = 49152
MIN_EPHEMERAL_PORT_N    = 0x00C0        ; same in Network byte order (FIXME)
MAX_EPHEMERAL_PORT      = 61000
MAX_EPHEMERAL_PORT_N    = 0x48EE        ; same in Network byte order (FIXME)

; Ethernet protocol numbers
ETHER_ARP               = 0x0608
ETHER_IPv4              = 0x0008
ETHER_IPv6              = 0xDD86
ETHER_PPP_DISCOVERY     = 0x6388
ETHER_PPP_SESSION       = 0x6488

; PPP protocol numbers
PPP_IPv4                = 0x2100
PPP_IPV6                = 0x5780

;Protocol family
AF_UNSPEC               = 0
AF_LOCAL                = 1
AF_INET4                = 2
AF_INET6                = 10
AF_PPP                  = 777

; Internet protocol numbers
IP_PROTO_IP             = 0
IP_PROTO_ICMP           = 1
IP_PROTO_TCP            = 6
IP_PROTO_UDP            = 17

; PPP protocol number
PPP_PROTO_ETHERNET      = 666

; Socket types
SOCK_STREAM             = 1
SOCK_DGRAM              = 2
SOCK_RAW                = 3

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

SO_BLOCK                = 1 shl 10    ; TO BE REMOVED
SO_NONBLOCK             = 1 shl 31

; Socket flags for user calls
MSG_PEEK                = 0x02
MSG_DONTWAIT            = 0x40

; Socket level
SOL_SOCKET              = 0


; Socket States
SS_NOFDREF              = 0x0001      ; no file table ref any more
SS_ISCONNECTED          = 0x0002      ; socket connected to a peer
SS_ISCONNECTING         = 0x0004      ; in process of connecting to peer
SS_ISDISCONNECTING      = 0x0008      ; in process of disconnecting
SS_CANTSENDMORE         = 0x0010      ; can't send more data to peer
SS_CANTRCVMORE          = 0x0020      ; can't receive more data from peer
SS_RCVATMARK            = 0x0040      ; at mark on input
SS_ISABORTING           = 0x0080      ; aborting fd references - close()
SS_RESTARTSYS           = 0x0100      ; restart blocked system calls
SS_ISDISCONNECTED       = 0x0800      ; socket disconnected from peer

SS_ASYNC                = 0x0100      ; async i/o notify
SS_ISCONFIRMING         = 0x0200      ; deciding to accept connection req
SS_MORETOCOME           = 0x0400

SS_BLOCKED              = 0x8000


SOCKET_MAXDATA          = 4096*32     ; must be 4096*(power of 2) where 'power of 2' is at least 8

; Network driver types
NET_TYPE_LOOPBACK       = 0
NET_TYPE_ETH            = 1
NET_TYPE_SLIP           = 2

MAX_backlog             = 20          ; maximum backlog for stream sockets

; Error Codes
ENOBUFS                 = 55
ECONNREFUSED            = 61
ECONNRESET              = 52
ETIMEDOUT               = 60
ECONNABORTED            = 53

; Api protocol numbers
API_ETH                 = 0
API_IPv4                = 1
API_ICMP                = 2
API_UDP                 = 3
API_TCP                 = 4
API_ARP                 = 5
API_PPPOE               = 6
API_IPv6                = 7

HWACC_TCP_IPv4          = 1 shl 0

struct  NET_DEVICE

        type            dd ?    ; Type field
        mtu             dd ?    ; Maximal Transmission Unit
        name            dd ?    ; Ptr to 0 terminated string

        unload          dd ?    ; Ptrs to driver functions
        reset           dd ?    ;
        transmit        dd ?    ;

        bytes_tx        dq ?    ; Statistics, updated by the driver
        bytes_rx        dq ?    ;
        packets_tx      dd ?    ;
        packets_rx      dd ?    ;

        state           dd ?    ; link state (0 = no link)
        hwacc           dd ?    ; bitmask stating enabled HW accelerations (offload engines)

ends


; Exactly as it says..
macro pseudo_random reg {
        add     reg, [esp]
        rol     reg, 5
        xor     reg, [timer_ticks]
        add     reg, [CPU_FREQ]
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



align 4
uglobal

        NET_RUNNING     dd  ?
        NET_DEFAULT     dd  ?
        NET_DRV_LIST    rd  MAX_NET_DEVICES

endg


;-----------------------------------------------------------------
;
; stack_init
;
;  This function calls all network init procedures
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
stack_init:

; Init the network drivers list
        xor     eax, eax
        mov     edi, NET_RUNNING
        mov     ecx, (MAX_NET_DEVICES + 2)
        rep     stosd

        PPPoE_init

        IPv4_init
;        IPv6_init
        ICMP_init

        ARP_init
        UDP_init
        TCP_init

        SOCKET_init

        mov     [net_tmr_count], 0

        ret


;-----------------------------------------------------------------
;
; stack_handler
;
;  This function is called in kernel loop
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
stack_handler:

        cmp     [NET_RUNNING], 0
        je      .exit


        ; Test for 10ms tick
        mov     eax, [timer_ticks]
        cmp     eax, [net_10ms]
        je      .exit
        mov     [net_10ms], eax

        test    [net_10ms], 0x0f        ; 160ms
        jnz     .exit

        TCP_timer_160ms

        test    [net_10ms], 0x3f        ; 640ms
        jnz     .exit

        TCP_timer_640ms
        ARP_decrease_entry_ttls
        IPv4_decrease_fragment_ttls

  .exit:
        ret



align 4
NET_link_changed:

        DEBUGF  1,"NET_link_changed device=0x%x status=0x%x\n", ebx, [ebx + NET_DEVICE.state]

align 4
NET_send_event:

        DEBUGF  1,"NET_send_event\n"

; Send event to all applications
        push    edi ecx
        mov     edi, SLOT_BASE
        mov     ecx, [TASK_COUNT]
  .loop:
        add     edi, 256
        or      [edi + APPDATA.event_mask], EVENT_NETWORK2
        loop    .loop
        pop     ecx edi

        call    change_task

        ret



;-----------------------------------------------------------------
;
; NET_add_device:
;
;  This function is called by the network drivers,
;  to register each running NIC to the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: Device num in eax, -1 on error
;
;-----------------------------------------------------------------
align 4
NET_add_device:

        DEBUGF  1,"NET_Add_Device: %x\n", ebx   ;;; TODO: use mutex to lock net device list

        cmp     [NET_RUNNING], MAX_NET_DEVICES
        jae     .error

;----------------------------------
; Check if device is already listed
        mov     eax, ebx
        mov     ecx, MAX_NET_DEVICES    ; We need to check whole list because a device may be removed without re-organizing list
        mov     edi, NET_DRV_LIST

        repne   scasd                   ; See if device is already in the list
        jz      .error

;----------------------------
; Find empty slot in the list
        xor     eax, eax
        mov     ecx, MAX_NET_DEVICES
        mov     edi, NET_DRV_LIST

        repne   scasd
        jnz     .error

        sub     edi, 4

;-----------------------------
; Add device to the found slot
        mov     [edi], ebx              ; add device to list

        mov     eax, edi                ; Calculate device number in eax
        sub     eax, NET_DRV_LIST
        shr     eax, 2

        inc     [NET_RUNNING]           ; Indicate that one more network device is up and running

        cmp     eax, 1                  ; If it's the first network device, try to set it as default
        jne     @f
        push    eax
        call    NET_set_default
        pop     eax
       @@:

        call    NET_send_event

        DEBUGF  1,"Device number: %u\n", eax
        ret

  .error:
        or      eax, -1
        DEBUGF  2,"Adding network device failed\n"
        ret



;-----------------------------------------------------------------
;
; NET_set_default
;
;  API to set the default interface
;
;  IN:  Device num in eax
;  OUT: Device num in eax, -1 on error
;
;-----------------------------------------------------------------
align 4
NET_set_default:

        DEBUGF  1,"NET_set_default: device=%x\n", eax

        cmp     eax, MAX_NET_DEVICES
        jae     .error

        cmp     [NET_DRV_LIST+eax*4], 0
        je      .error

        mov     [NET_DEFAULT], eax

        DEBUGF  1,"NET_set_default: succes\n"
        ret

  .error:
        or      eax, -1
        DEBUGF  1,"NET_set_default: failed\n"
        ret


;-----------------------------------------------------------------
;
; NET_Remove_Device:
;
;  This function is called by etwork drivers,
;  to unregister network devices from the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: eax: -1 on error
;
;-----------------------------------------------------------------
align 4
NET_remove_device:

        cmp     [NET_RUNNING], 0
        je      .error

        cmp     [NET_DRV_LIST], ebx
        jne     @f
        mov     [NET_DRV_LIST], 0
        cmp     [NET_RUNNING], 1
        je      @f
        ; there are still active devices, find one and make it default
        xor     eax, eax
        mov     ecx, MAX_NET_DEVICES
        mov     edi, NET_DRV_LIST
        repe    scasd
        je      @f
        shr     edi, 2
        dec     edi
        mov     [NET_DEFAULT], edi
       @@:

;----------------------------
; Find the driver in the list

        mov     eax, ebx
        mov     ecx, MAX_NET_DEVICES
        mov     edi, NET_DRV_LIST+4

        repne   scasd
        jnz     .error

;------------------------
; Remove it from the list

        xor     eax, eax
        mov     dword [edi-4], eax

        call    NET_send_event

        dec     [NET_RUNNING]
        ret

  .error:
        or      eax, -1
        ret



;-----------------------------------------------------------------
;
; NET_ptr_to_num
;
; IN:  ebx = ptr to device struct
; OUT: edi = -1 on error, device number otherwise
;
;-----------------------------------------------------------------
align 4
NET_ptr_to_num:
        push    ecx

        mov     ecx, MAX_NET_DEVICES
        mov     edi, NET_DRV_LIST

  .loop:
        cmp     ebx, [edi]
        jz      .found
        add     edi, 4
        dec     ecx
        jnz     .loop

        ; repnz  scasd could work too if eax is used instead of ebx!

        or      edi, -1

        pop     ecx
        ret

  .found:
        sub     edi, NET_DRV_LIST
        shr     edi, 2

        pop     ecx
        ret

;-----------------------------------------------------------------
;
; checksum_1
;
;  This is the first of two functions needed to calculate a checksum.
;
;  IN:  edx = start offset for semi-checksum
;       esi = pointer to data
;       ecx = data size
;  OUT: edx = semi-checksum
;
;
; Code was optimized by diamond
;
;-----------------------------------------------------------------
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

;-----------------------------------------------------------------
;
; checksum_2
;
;  This function calculates the final ip/tcp/udp checksum for you
;
;  IN:  edx = semi-checksum
;  OUT: dx = checksum (in INET byte order)
;
;-----------------------------------------------------------------
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

        DEBUGF  1,"Checksum: %x\n", dx

        ret



;----------------------------------------------------------------
;
;  System function to work with network devices (75)
;
;----------------------------------------------------------------
align 4
sys_network:                    ; FIXME: make default device easily accessible

        cmp     ebx, -1
        jne     @f

        mov     eax, [NET_RUNNING]
        jmp     .return

   @@:
        cmp     bh, MAX_NET_DEVICES             ; Check if device number exists
        jae     .doesnt_exist

        mov     esi, ebx
        and     esi, 0x0000ff00
        shr     esi, 6

        cmp     dword [esi + NET_DRV_LIST], 0   ; check if driver is running
        je      .doesnt_exist

        mov     eax, [esi + NET_DRV_LIST]

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
        dd      .set_default            ; 6
  .number = ($ - .table) / 4 - 1

  .get_type:                            ; 0 = Get device type (ethernet/token ring/...)

        mov     eax, [eax + NET_DEVICE.type]
        jmp     .return


  .get_dev_name:                        ; 1 = Get device name

        mov     esi, [eax + NET_DEVICE.name]
        mov     edi, ecx

        mov     ecx, 64/4 ; max length
        rep     movsd

        xor     eax, eax
        jmp     .return

  .reset:                               ; 2 = Reset the device

        call    [eax + NET_DEVICE.reset]
        jmp     .return

  .stop:                                ; 3 = Stop driver for this device

        call    [eax + NET_DEVICE.unload]
        jmp     .return


  .get_ptr:                             ; 4 = Get driver pointer

        jmp     .return


  .get_drv_name:                        ; 5 = Get driver name

        xor     eax, eax
        jmp     .return


  .set_default:                         ; 6 = Set default device

        call    NET_set_default
        jmp     .return

  .doesnt_exist:
        mov     eax, -1

  .return:
        mov     [esp+32], eax
        ret


;----------------------------------------------------------------
;
;  System function to work with protocols  (76)
;
;----------------------------------------------------------------
align 4
sys_protocols:
        cmp     bh, MAX_NET_DEVICES             ; Check if device number exists
        jae     .doesnt_exist

        mov     esi, ebx
        and     esi, 0x0000ff00
        shr     esi, 6                          ; now we have the device num * 4 in esi
        cmp     [esi + NET_DRV_LIST], 0         ; check if driver is running
        je      .doesnt_exist

        push    .return                         ; return address (we will be using jumps instead of calls)

        mov     eax, ebx                        ; set ax to protocol number
        shr     eax, 16                         ;

        cmp     ax, API_ETH
        je      ETH_api

        cmp     ax, API_IPv4
        je      IPv4_api

        cmp     ax, API_ICMP
        je      ICMP_api

        cmp     ax, API_UDP
        je      UDP_api

        cmp     ax, API_TCP
        je      TCP_api

        cmp     ax, API_ARP
        je      ARP_api

        cmp     ax, API_PPPOE
        je      PPPoE_api

        cmp     ax, API_IPv6
        je      IPv6_api

        add     esp, 4                           ; if we reached here, no function was called, so we need to balance stack

  .doesnt_exist:
        mov     eax, -1

  .return:
        mov     [esp+28+4], eax                 ; return eax value to the program
        ret
