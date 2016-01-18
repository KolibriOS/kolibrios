;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  loopback.inc                                                   ;;
;;                                                                 ;;
;;  LoopBack device for KolibriOS                                  ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6011 $

iglobal
align 4

LOOPBACK_DEVICE:

        .device_type    dd NET_DEVICE_LOOPBACK
        .mtu            dd 4096
        .name           dd .namestr

        .unload         dd .dummy_fn
        .reset          dd .dummy_fn
        .transmit       dd loop_input

        .bytes_tx       dq 0
        .bytes_rx       dq 0
        .packets_tx     dd 0
        .packets_rx     dd 0

        .link_state     dd -1
        .hwacc          dd NET_HWACC_TCP_IPv4_IN + NET_HWACC_TCP_IPv4_OUT

        .namestr        db 'loopback', 0

        .dummy_fn:
        ret

endg


macro   loop_init {
local   .fail

        mov     ebx, LOOPBACK_DEVICE
        call    net_add_device

        cmp     eax, -1
        je      .fail

        mov     [IP_LIST], 127 + 1 shl 24
        mov     [SUBNET_LIST], 255
        mov     [BROADCAST_LIST], 0xffffff00 + 127

  .fail:
}

;-----------------------------------------------------------------;
;                                                                 ;
; loop_input                                                      ;
;                                                                 ;
;   IN: [esp+4] = Pointer to buffer                               ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
loop_input:

        mov     eax, [esp+4]

; Update stats
        inc     [LOOPBACK_DEVICE.packets_rx]

        mov     ecx, [eax + NET_BUFF.length]
        add     dword[LOOPBACK_DEVICE.bytes_rx], ecx
        adc     dword[LOOPBACK_DEVICE.bytes_rx + 4], 0

        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_input: ptr=%x size=%u\n", eax, ecx

; Reverse buffptr and returnaddr on stack
        pop     edx edi
        push    edx edi

; Set registers for protocol handlers
        lea     edx, [eax + NET_BUFF.data]
        mov     ebx, [eax + NET_BUFF.device]
        mov     eax, [eax + NET_BUFF.type]

; Place protocol handlers here
        cmp     eax, AF_INET4
        je      ipv4_input

        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_input: Unknown packet type=%x\n", eax

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_input: dumping\n"
        call    net_buff_free
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; loop_output                                                     ;
;                                                                 ;
;  IN:  ecx = packet size                                         ;
;       edi = address family                                      ;
;                                                                 ;
; OUT:  eax = start of net frame / 0 on error                     ;
;       ebx = to device structure                                 ;
;       ecx = unchanged (packet size of embedded data)            ;
;       edi = start of payload                                    ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
loop_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_output\n"

        cmp     ecx, [LOOPBACK_DEVICE.mtu]
        ja      .too_large

        push    ecx edi
        add     ecx, NET_BUFF.data
        stdcall net_buff_alloc, ecx
        test    eax, eax
        jz      .out_of_ram

        pop     edi
        mov     [eax + NET_BUFF.type], edi
        mov     ebx, LOOPBACK_DEVICE
        mov     [eax + NET_BUFF.device], ebx
        pop     ecx
        mov     [eax + NET_BUFF.length], ecx
        lea     edi, [eax + NET_BUFF.data]

        inc     [LOOPBACK_DEVICE.packets_tx]
        add     dword[LOOPBACK_DEVICE.bytes_tx], ecx
        adc     dword[LOOPBACK_DEVICE.bytes_tx + 4], 0

        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_output: ptr=%x size=%u\n", eax, ecx
        ret

  .too_large:
        DEBUGF  DEBUG_NETWORK_ERROR, "LOOP_output: packet is too large\n"
        xor     eax, eax
        ret

  .out_of_ram:
        DEBUGF  DEBUG_NETWORK_ERROR, "LOOP_output: out of memory\n"
        add     esp, 4+4
        xor     eax, eax
        ret


