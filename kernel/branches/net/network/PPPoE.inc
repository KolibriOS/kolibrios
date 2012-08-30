;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2012. All rights reserved.      ;;
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

struct  PPPoE_frame
        VersionAndType  db ?
        Code            db ?
        SessionID       dw ?
        Length          dw ?            ; Length of payload, does NOT include the length PPPoE header.
        Payload         rb 0
ends

struct  LCP_frame
        Code            db ?
        Identifier      db ?
        Length          dw ?
        Data            rb 0
ends

uglobal
        PPPoE_SID       dw ?
        PPPoE_MAC       dp ?
endg

LCP_config_request      = 1
LCP_config_ack          = 2
LCP_config_nak          = 3
LCP_config_reject       = 4
LCP_terminate_request   = 5
LCP_terminate_ack       = 6
LCP_code_reject         = 7
LCP_protocol_reject     = 8
LCP_echo_request        = 9
LCP_echo_reply          = 10
LCP_discard_request     = 11


;-----------------------------------------------------------------
;
; IPv4_init
;
;  This function resets all IP variables
;
;-----------------------------------------------------------------
macro   PPPoE_init {

        call    PPPoE_stop_connection

}


;-----------------------------------------------------------------
;
; PPPoE discovery input
;
; Handler of received Ethernet packet with type = Discovery
;
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to PPP header in edx
;       size of PPP packet in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
PPPoE_discovery_input:

        DEBUGF  2,"PPPoE_discovery_input\n"

; First, find open PPPoE socket

        mov     eax, net_sockets

  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .dump

        cmp     [eax + SOCKET.Domain], AF_PPP
        jne     .next_socket

        cmp     [eax + SOCKET.Protocol], PPP_PROTO_ETHERNET
        jne     .next_socket

; Now, send it to the this socket

        mov     ecx, [esp + 4]
        mov     esi, [esp]

        jmp     SOCKET_input

  .dump:
        DEBUGF 1,'PPPoE_discovery_input: dumping\n'
        call    kernel_free
        add     esp, 4
        ret


;--------------------------------------
;
; Send discovery packet
;
; IN: eax = socket pointer
;     ecx = number of bytes to send
;     esi = pointer to data
;
;--------------------------------------

align 4
PPPoE_discovery_output:

        DEBUGF  2,"PPPoE_discovery_output: socket=%x buffer=%x size=%d\n", eax, esi, ecx

; RFC2516: An entire PADI packet (including the PPPoE header) MUST NOT
; exceed 1484 octets.
        cmp     ecx, 1484 + 14
        ja      .bad

; Check that device exists and is ethernet device
        mov     ebx, [eax + SOCKET.device]

        cmp     ebx, MAX_NET_DEVICES
        ja      .bad

        mov     ebx, [NET_DRV_LIST + 4*ebx]
        test    ebx, ebx
        jz      .bad

        cmp     [ebx + NET_DEVICE.type], NET_TYPE_ETH
        jne     .bad

        DEBUGF  2,"PPPoE_discovery_output: device=%x\n", ebx

; Create packet.
        push    ecx esi
        stdcall kernel_alloc, 1500
        pop     esi ecx
        test    eax, eax
        jz      .bad

        mov     edx, ecx
        mov     edi, eax
        rep     movsb

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
        mov     ax, ETHER_PPP_DISCOVERY
        stosw

; And send the packet
        call    [ebx + NET_DEVICE.transmit]

        xor     eax, eax
        ret

  .bad:
        or      eax, -1
        ret


;-----------------------------------------------------------------
;
; PPPoE session input
;
; Handler of received Ethernet packet with type = Session
;
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to PPP header in edx
;       size of PPP packet in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
PPPoE_session_input:

        cmp     [edx + PPPoE_frame.VersionAndType], 0x11
        jne     .dump

        cmp     [edx + PPPoE_frame.Code], 0x00
        jne     .dump

        movzx   ecx, [edx + PPPoE_frame.Length]
        xchg    cl, ch

        mov     ax, [edx + PPPoE_frame.SessionID]
        DEBUGF  2,"PPPoE_input: session ID=%x, length=%u\n", ax, cx
        cmp     ax, [PPPoE_SID]
        jne     .dump

        mov     ax, word [edx + PPPoE_frame.Payload]
        add     edx, PPPoE_frame.Payload + 2

        cmp     ax, PPP_IPv4
        je      IPv4_input

        cmp     ax, PPP_LCP
        je      LCP_input

        DEBUGF  2,"PPPoE_input: Unknown protocol=%x\n", ax

  .dump:
        DEBUGF  2,"PPPoE_input: dumping\n"
        call    kernel_free
        add     esp, 4
        ret




;-----------------------------------------------------------------
;
; PPPoE_output
;
; IN:
;     ebx = device ptr
;     ecx = packet size
;
;      di = protocol
;
; OUT: edi = 0 on error, pointer to buffer otherwise
;      eax = buffer start
;      ebx = to device structure
;      ecx = unchanged (packet size of embedded data)
;      edx = size of complete buffer
;
;-----------------------------------------------------------------
align 4
PPPoE_output:

        DEBUGF  1,"PPPoE_output: size=%u device=%x\n", ecx, ebx

        pushw   di
        pushw   [PPPoE_SID]

        lea     eax, [ebx + ETH_DEVICE.mac]
        lea     edx, [PPPoE_MAC]
        add     ecx, PPPoE_frame.Payload + 2
        mov     di, ETHER_PPP_SESSION
        call    ETH_output
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

        DEBUGF  1,"PPPoE_output: success!\n"
        ret


  .eth_error:
        add     esp, 4
        xor     edi, edi

        ret




;-----------------------------------------------------------------
;
; LCP_input:
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to LCP header in edx
;       size of LCP packet in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
LCP_input:

        DEBUGF  1,"LCP_input\n"

        cmp     [edx + LCP_frame.Code], LCP_echo_request
        je      .echo

        jmp     .dump

  .echo:
        mov     [edx + LCP_frame.Code], LCP_echo_reply
        mov     esi, [esp]

        push    dword [esi + ETH_header.DstMAC]
        push    dword [esi + ETH_header.SrcMAC]
        pop     dword [esi + ETH_header.DstMAC]
        pop     dword [esi + ETH_header.SrcMAC]
        push    word [esi + ETH_header.DstMAC + 4]
        push    word [esi + ETH_header.SrcMAC + 4]
        pop     word [esi + ETH_header.DstMAC + 4]
        pop     word [esi + ETH_header.SrcMAC + 4]

        call    [ebx + NET_DEVICE.transmit]
        ret

  .dump:
        DEBUGF  2,"LCP_input: dumping\n"
        call    kernel_free
        add     esp, 4
        ret




align 4
PPPoE_start_connection:

        DEBUGF  2,"PPPoE_start_connection: %x\n", cx

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
PPPoE_stop_connection:

        DEBUGF  2,"PPPoE_stop_connection\n"

        xor     eax, eax
        mov     [PPPoE_SID], ax
        mov     dword [PPPoE_MAC], eax
        mov     word [PPPoE_MAC + 4], ax

        ret


;---------------------------------------------------------------------------
;
; PPPoE API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------
align 4
PPPoE_api:

        movzx   eax, bh
        shl     eax, 2

        and     ebx, 0xff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      PPPoE_start_connection  ; 0
        dd      PPPoE_stop_connection   ; 1
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret
