;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ETHERNET.INC                                                   ;;
;;                                                                 ;;
;;  Ethernet network layer for KolibriOS                           ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 3346 $

ETH_FRAME_MINIMUM       = 60

struct  ETH_header

        DstMAC          dp  ?  ; destination MAC-address
        SrcMAC          dp  ?  ; source MAC-address
        Type            dw  ?  ; type of the upper-layer protocol

ends

struct  ETH_DEVICE      NET_DEVICE

        mac             dp ?

ends

align 4
iglobal

        ETH_BROADCAST   dp  0xffffffffffff
endg

;-----------------------------------------------------------------
;
; ETH_input
;
;  This function is called by ethernet drivers,
;  It pushes the received ethernet packets onto the eth_in_queue
;
;  IN:   [esp]  = Pointer to buffer
;       [esp+4] = size of buffer
;         ebx   = pointer to eth_device
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ETH_input:
        mov     eax, [esp]
        mov     ecx, [esp+4]

        DEBUGF  1,"ETH_input: size=%u\n", ecx
        cmp     ecx, ETH_FRAME_MINIMUM
        jb      .dump
        sub     ecx, sizeof.ETH_header

        lea     edx, [eax + sizeof.ETH_header]
        mov     ax, [eax + ETH_header.Type]

        cmp     ax, ETHER_IPv4
        je      IPv4_input

        cmp     ax, ETHER_ARP
        je      ARP_input

        cmp     ax, ETHER_IPv6
        je      IPv6_input

        cmp     ax, ETHER_PPP_DISCOVERY
        je      PPPoE_discovery_input

        cmp     ax, ETHER_PPP_SESSION
        je      PPPoE_session_input

        DEBUGF  2,"ETH_input: Unknown packet type=%x\n", ax

  .dump:
        DEBUGF  2,"ETH_input: dumping\n"
        call    kernel_free
        add     esp, 4
        ret

;-----------------------------------------------------------------
;
; ETH_output
;
; IN: eax = pointer to source mac
;     ebx = device ptr
;     ecx = packet size
;     edx = pointer to destination mac
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
ETH_output:

        DEBUGF  1,"ETH_output: size=%u device=%x\n", ecx, ebx

        cmp     ecx, [ebx + NET_DEVICE.mtu]
        ja      .exit

        push    ecx
        push    di eax edx

        add     ecx, sizeof.ETH_header
        stdcall kernel_alloc, ecx
        test    eax, eax
        jz      .out_of_ram
        mov     edi, eax

        pop     esi
        movsd
        movsw
        pop     esi
        movsd
        movsw
        pop     ax
        stosw

        lea     eax, [edi - sizeof.ETH_header]  ; Set eax to buffer start
        pop     ecx
        lea     edx, [ecx + sizeof.ETH_header]  ; Set edx to complete buffer size

        cmp     edx, ETH_FRAME_MINIMUM
        jbe     .adjust_size
  .done:
        DEBUGF  1,"ETH_output: ptr=%x size=%u\n", eax, edx
        ret

  .adjust_size:
        mov     edx, ETH_FRAME_MINIMUM
        test    edx, edx        ; clear zero flag
        jmp     .done

  .out_of_ram:
        DEBUGF  2,"ETH_output: Out of ram!\n"
        add     esp, 4+4+2+4
        sub     edi, edi
        ret

  .exit:
        DEBUGF  2,"ETH_output: Packet too large!\n"
        sub     edi, edi
        ret



;-----------------------------------------------------------------
;
; ETH_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;-----------------------------------------------------------------
align 4
ETH_api:

        cmp     bh, MAX_NET_DEVICES
        ja      .error
        movzx   eax, bh
        mov     eax, dword [NET_DRV_LIST + 4*eax]
        cmp     [eax + NET_DEVICE.type], NET_TYPE_ETH
        jne     .error

        and     ebx, 0xff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .packets_tx     ; 0
        dd      .packets_rx     ; 1
        dd      .bytes_tx       ; 2
        dd      .bytes_rx       ; 3
        dd      .read_mac       ; 4
        dd      .state          ; 5
  .number = ($ - .table) / 4 - 1

  .error:
        or      eax, -1
        ret

  .packets_tx:
        mov     eax, [eax + NET_DEVICE.packets_tx]

        ret

  .packets_rx:
        mov     eax, [eax + NET_DEVICE.packets_rx]
        ret

  .bytes_tx:
        mov     ebx, dword [eax + NET_DEVICE.bytes_tx + 4]
        mov     eax, dword [eax + NET_DEVICE.bytes_tx]
        mov     [esp+20+4], ebx                         ; TODO: fix this ugly code
        ret

  .bytes_rx:
        mov     ebx, dword [eax + NET_DEVICE.bytes_rx + 4]
        mov     eax, dword [eax + NET_DEVICE.bytes_rx]
        mov     [esp+20+4], ebx                         ; TODO: fix this ugly code
        ret


  .read_mac:
        movzx   ebx, word [eax + ETH_DEVICE.mac]
        mov     eax, dword [eax + ETH_DEVICE.mac + 2]
        mov     [esp+20+4], ebx                         ; TODO: fix this ugly code
        ret

  .state:
        mov     eax, [eax + NET_DEVICE.state]
        ret

