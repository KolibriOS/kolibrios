;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2012-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  PPPoE.INC                                                   ;;
;;                                                              ;;
;;  Part of the tcp/ip network stack for KolibriOS              ;;
;;                                                              ;;
;;    Written by hidnplayr@kolibrios.org                        ;;
;;                                                              ;;
;;          GNU GENERAL PUBLIC LICENSE                          ;;
;;             Version 2, June 1991                             ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6011 $


struct  PPPoE_frame
        VersionAndType  db ?
        Code            db ?
        SessionID       dw ?
        Length          dw ?            ; Length of payload, does NOT include the length PPPoE header.
        Payload         rb 0
ends

uglobal
align 4

        PPPoE_SID       dw ?
        PPPoE_MAC       dp ?

endg

;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_init: Reset all pppoe variables                           ;
;                                                                 ;
;-----------------------------------------------------------------;
macro   pppoe_init {

        call    pppoe_stop_connection

}


;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_discovery_input                                           ;
;                                                                 ;
;  IN:  [esp] = ptr to buffer                                     ;
;       [esp+4] = size of buffer                                  ;
;       ebx = ptr to device struct                                ;
;       ecx = size of PPP packet                                  ;
;       edx = ptr to PPP header                                   ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
pppoe_discovery_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_discovery_input\n"

; First, find open PPPoE socket

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        mov     eax, net_sockets

  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .dump

        cmp     [eax + SOCKET.Domain], AF_PPP
        jne     .next_socket

        cmp     [eax + SOCKET.Protocol], PPP_PROTO_ETHERNET
        jne     .next_socket

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

; Now, send it to the this socket

        mov     ecx, [esp + 4]
        mov     esi, [esp]

        jmp     socket_input

  .dump:
        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, 'PPPoE_discovery_input: dumping\n'
        call    net_buff_free
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_discovery_output                                          ;
;                                                                 ;
;  IN:  eax = socket pointer                                      ;
;       ecx = number of bytes to send                             ;
;       esi = pointer to data                                     ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
pppoe_discovery_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_discovery_output: socket=%x buffer=%x size=%d\n", eax, esi, ecx

; RFC2516: An entire PADI packet (including the PPPoE header) MUST NOT
; exceed 1484 octets.
        cmp     ecx, 1484 + 14
        ja      .bad

; Check that device exists and is ethernet device
        mov     ebx, [eax + SOCKET.device]

        cmp     ebx, NET_DEVICES_MAX
        ja      .bad

        mov     ebx, [NET_DRV_LIST + 4*ebx]
        test    ebx, ebx
        jz      .bad

        cmp     [ebx + NET_DEVICE.device_type], NET_DEVICE_ETH
        jne     .bad

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_discovery_output: device=%x\n", ebx

; Create packet.
        push    ecx esi
;;;; FIXME        stdcall kernel_alloc, 1500
        pop     esi ecx
        test    eax, eax
        jz      .bad

        mov     edx, ecx
        mov     edi, eax
        rep movsb

        cmp     edx, 60         ; Min ETH size
        ja      @f
        mov     edx, 60
       @@:

        push    edx eax         ; size and packet ptr for driver send proc

; Overwrite source MAC and protocol type
        lea     edi, [eax + ETH_header.SrcMAC]
        lea     esi, [ebx + ETH_DEVICE.mac]
        movsd
        movsw
        cmp     word[edi], ETHER_PROTO_PPP_SESSION      ; Allow only PPP_discovery, or LCP
        je      @f
        mov     ax, ETHER_PROTO_PPP_DISCOVERY
        stosw
       @@:

; And send the packet
        call    [ebx + NET_DEVICE.transmit]

        xor     eax, eax
        ret

  .bad:
        or      eax, -1
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_session_input                                             ;
;                                                                 ;
;  IN:  [esp] = ptr to buffer                                     ;
;       [esp+4] = size of buffer                                  ;
;       ebx = ptr to device struct                                ;
;       edx = ptr to PPP header                                   ;
;       ecx = size of PPP packet                                  ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
pppoe_session_input:

        cmp     [edx + PPPoE_frame.VersionAndType], 0x11
        jne     .dump

        cmp     [edx + PPPoE_frame.Code], 0x00
        jne     .dump

        movzx   ecx, [edx + PPPoE_frame.Length]
        xchg    cl, ch

        mov     ax, [edx + PPPoE_frame.SessionID]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_input: session ID=%x, length=%u\n", ax, cx
        cmp     ax, [PPPoE_SID]
        jne     .dump

        mov     ax, word [edx + PPPoE_frame.Payload]
        add     edx, PPPoE_frame.Payload + 2

        cmp     ax, PPP_PROTO_IPv4
        je      ipv4_input

;        cmp     ax, PPP_PROTO_IPv6
;        je      ipv6_input

        jmp     pppoe_discovery_input   ; Send LCP,CHAP,CBCP,... packets to the PPP dialer
        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_input: Unknown protocol=%x\n", ax

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_input: dumping\n"
        call    net_buff_free
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_output                                                    ;
;                                                                 ;
;  IN:  ax = protocol                                             ;
;       ebx = device ptr                                          ;
;       ecx = packet size                                         ;
;                                                                 ;
; OUT:  eax = buffer start                                        ;
;       eax = 0 on error                                          ;
;       ebx = device ptr                                          ;
;       ecx = packet size                                         ;
;       edx = size of complete buffer                             ;
;       edi = start of PPP payload                                ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
pppoe_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_output: size=%u device=%x\n", ecx, ebx

        pushw   ax
        pushw   [PPPoE_SID]

        mov     ax, ETHER_PROTO_PPP_SESSION
        add     ecx, PPPoE_frame.Payload + 2
        lea     edx, [PPPoE_MAC]
        call    eth_output
        jz      .eth_error

        sub     ecx, PPPoE_frame.Payload
        mov     [edi + PPPoE_frame.VersionAndType], 0x11
        mov     [edi + PPPoE_frame.Code], 0
        popw    [edi + PPPoE_frame.SessionID]
        xchg    cl, ch
        mov     [edi + PPPoE_frame.Length], cx
        xchg    cl, ch

        pop     word [edi + PPPoE_frame.Payload]

        sub     ecx, 2
        add     edi, PPPoE_frame.Payload + 2

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_output: success!\n"
        ret


  .eth_error:
        add     esp, 4
        xor     eax, eax
        ret

align 4
pppoe_start_connection:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_start_connection: %x\n", cx

        cmp     [PPPoE_SID], 0
        jne     .fail

        mov     [PPPoE_SID], cx
        mov     dword [PPPoE_MAC], edx
        mov     word [PPPoE_MAC + 4], si

        xor     eax, eax
        ret

  .fail:
        or      eax, -1
        ret


align 4
pppoe_stop_connection:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "PPPoE_stop_connection\n"

        xor     eax, eax
        mov     [PPPoE_SID], ax
        mov     dword [PPPoE_MAC], eax
        mov     word [PPPoE_MAC + 4], ax

        ret


;-----------------------------------------------------------------;
;                                                                 ;
; pppoe_api: Part of system function 76                           ;
;                                                                 ;
; IN:  subfunction number in bl                                   ;
;      device number in bh                                        ;
;      ecx, edx, .. depends on subfunction                        ;
;                                                                 ;
; OUT:                                                            ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
pppoe_api:

        movzx   eax, bh
        shl     eax, 2

        and     ebx, 0xff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      pppoe_start_connection  ; 0
        dd      pppoe_stop_connection   ; 1
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret
