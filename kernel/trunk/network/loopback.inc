;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
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


iglobal
align 4

LOOPBACK_DEVICE:

        .device_type     dd NET_DEVICE_LOOPBACK
        .mtu             dd NET_BUFFER_SIZE - NET_BUFF.data
        .name            dd .namestr

        .unload          dd loop_dummy
        .reset           dd loop_dummy
        .transmit        dd loop_input

        .link_state      dd -1
        .hwacc           dd NET_HWACC_TCP_IPv4_IN + NET_HWACC_TCP_IPv4_OUT

        .bytes_tx        dq ?
        .bytes_rx        dq ?

        .packets_tx      dd ?
        .packets_tx_err  dd ?
        .packets_tx_drop dd ?
        .packets_tx_ovr  dd ?

        .packets_rx      dd ?
        .packets_rx_err  dd ?
        .packets_rx_drop dd ?
        .packets_rx_ovr  dd ?

        .namestr         db 'loopback', 0

endg


macro   loop_init {
local   .fail

        mov     ebx, LOOPBACK_DEVICE
        call    net_add_device

        cmp     eax, -1
        je      .fail

        mov     [IPv4_address], 127 + 1 shl 24
        mov     [IPv4_subnet], 255
        mov     [IPv4_broadcast], 0xffffff00 + 127

  .fail:
}

;-----------------------------------------------------------------;
;                                                                 ;
; loop_dummy                                                      ;
;                                                                 ;
;   IN: /                                                         ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
loop_dummy:
        ret

;-----------------------------------------------------------------;
;                                                                 ;
; loop_input                                                      ;
;                                                                 ;
;   IN: [esp+4] = Pointer to buffer                               ;
;                                                                 ;
;  OUT: eax = 0 on success, errorcode otherwise                   ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
loop_input:

        mov     eax, [esp+4]

; Update stats
        inc     [LOOPBACK_DEVICE.packets_tx]
        inc     [LOOPBACK_DEVICE.packets_rx]

        mov     ecx, [eax + NET_BUFF.length]
        add     dword[LOOPBACK_DEVICE.bytes_rx], ecx
        adc     dword[LOOPBACK_DEVICE.bytes_rx + 4], 0
        add     dword[LOOPBACK_DEVICE.bytes_tx], ecx
        adc     dword[LOOPBACK_DEVICE.bytes_tx + 4], 0

        DEBUGF  DEBUG_NETWORK_VERBOSE, "LOOP_input: ptr=%x size=%u\n", eax, ecx

; Reverse buffptr and returnaddr on stack
        pop     edx edi
        push    edx .done edi

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

        or      eax, -1
        ret

  .done:
        xor     eax, eax
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


