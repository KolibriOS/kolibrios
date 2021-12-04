;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved.    ;;
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

$Revision$

ETH_FRAME_MINIMUM       = 60
ETH_QUEUE_SIZE          = 255

struct  ETH_header

        DstMAC          dp  ?  ; destination MAC-address
        SrcMAC          dp  ?  ; source MAC-address
        Type            dw  ?  ; type of the upper-layer protocol

ends

struct  ETH_DEVICE      NET_DEVICE

        mac             dp ?

ends

iglobal
align 4

        ETH_BROADCAST   dp 0xffffffffffff

        ETH_frame_queued        dd 0    ; Number of queued frames

        ETH_frame_head          dd ETH_frame_head       ; Pointer to next frame in the linked list
        ETH_frame_tail          dd ETH_frame_head       ; Pointer to last frame in the linked list

endg

uglobal
align 4
        ETH_input_event dd ?
endg

macro   eth_init {

        movi    ebx, 1
        mov     ecx, eth_process_input
        call    new_sys_threads
        test    eax, eax
        jns     @f
        DEBUGF  DEBUG_NETWORK_ERROR,'K : cannot create kernel thread for ethernet, error %d\n', eax
  @@:

}

align 4
; This function is called by ethernet drivers.
; Push the received ethernet packet onto the ethernet input queue.
;
;  Input:
;    [esp + 4] = Pointer to buffer
;    [esp + 8] = Return address (yes, really)
;
;  Example:
;    push .retaddr
;    push buf_addr
;    jmp eth_input
eth_input:

        pop     eax

if defined NETWORK_SANITY_CHECKS
        cmp     eax, [net_buffs_low]
        jb      .assert_mbuff
        cmp     eax, [net_buffs_high]
        ja      .assert_mbuff
        test    eax, 0x7ff
        jnz     .assert_mbuff
end if

        spin_lock_irqsave

        cmp     [ETH_frame_queued], ETH_QUEUE_SIZE
        jae     .full
        inc     [ETH_frame_queued]

; Add frame to the end of the linked list
        mov     [eax + NET_BUFF.NextPtr], ETH_frame_head

        mov     ebx, [ETH_frame_tail]
        mov     [eax + NET_BUFF.PrevPtr], ebx

        mov     [ETH_frame_tail], eax
        mov     [ebx + NET_BUFF.NextPtr], eax

        spin_unlock_irqrestore

; Mark it as being an Ethernet Frame
        mov     [eax + NET_BUFF.type], NET_BUFF_ETH

; Now queue an event to process it
        xor     edx, edx
        mov     eax, [ETH_input_event]
        mov     ebx, [eax + EVENT.id]
        xor     esi, esi
        call    raise_event

        ret

  .full:
        mov     ebx, [eax + NET_BUFF.device]
        inc     [ebx + NET_DEVICE.packets_rx_ovr]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH incoming queue is full, discarding packet!\n"
        spin_unlock_irqrestore
        stdcall net_buff_free, eax
        ret

if defined NETWORK_SANITY_CHECKS
  .assert_mbuff:
        DEBUGF  DEBUG_NETWORK_ERROR, "eth_input: invalid buffer 0x%x\n", eax
        DEBUGF  DEBUG_NETWORK_ERROR, "eth_input: caller=0x%x\n", [esp+4]
        xor     eax, eax
        ret
end if



;-----------------------------------------------------------------;
;                                                                 ;
; eth_process_input: Process packets from ethernet input queue.   ;
;                                                                 ;
;  IN:  /                                                         ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
eth_process_input:

        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        call    create_event
        mov     [ETH_input_event], eax
        pushf
  .wait:
        popf
        mov     eax, [ETH_input_event]
        mov     ebx, [eax + EVENT.id]
        call    wait_event

  .loop:
        pushf
        cli
        cmp     [ETH_frame_queued], 0
        je      .wait

        dec     [ETH_frame_queued]

        mov     esi, [ETH_frame_head]
        mov     ebx, [esi + NET_BUFF.NextPtr]

        mov     [ETH_frame_head], ebx
        mov     [ebx + NET_BUFF.PrevPtr], ETH_frame_head

        popf

        mov     eax, [esi + NET_BUFF.offset]
        add     eax, esi
        mov     ecx, [esi + NET_BUFF.length]
        mov     ebx, [esi + NET_BUFF.device]

        pushd   .loop           ; return address for protocol handler
        push    esi             ; keep pointer to NET_BUFF on stack

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_input: size=%u\n", ecx
        sub     ecx, sizeof.ETH_header
        jb      .err

; Set registers for protocol handlers
        lea     edx, [eax + sizeof.ETH_header]
        mov     ax, [eax + ETH_header.Type]

; Place protocol handlers here
        cmp     ax, ETHER_PROTO_IPv4
        je      ipv4_input

        cmp     ax, ETHER_PROTO_ARP
        je      arp_input

;        cmp     ax, ETHER_PROTO_IPv6
;        je      ipv6_input

;        cmp     ax, ETHER_PROTO_PPP_DISCOVERY
;        je      pppoe_discovery_input

;        cmp     ax, ETHER_PROTO_PPP_SESSION
;        je      pppoe_session_input

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_input: Unknown packet type=%x\n", ax

  .drop:
        mov     eax, [esp]
        mov     eax, [eax + NET_BUFF.device]
        inc     [eax + NET_DEVICE.packets_rx_drop]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_input: dropping\n"
        call    net_buff_free
        ret

  .err:
        mov     eax, [esp]
        mov     eax, [eax + NET_BUFF.device]
        inc     [eax + NET_DEVICE.packets_rx_err]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_input: invalid frame received\n"
        call    net_buff_free
        ret




;-----------------------------------------------------------------;
;                                                                 ;
; eth_output                                                      ;
;                                                                 ;
;  IN:  ax = protocol                                             ;
;       ebx = device ptr                                          ;
;       ecx = payload size                                        ;
;       edx = pointer to destination mac                          ;
;                                                                 ;
;  OUT: eax = start of net frame / 0 on error                     ;
;       ebx = device ptr                                          ;
;       ecx = payload size                                        ;
;       edi = start of payload                                    ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
eth_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_output: size=%u device=%x\n", ecx, ebx

        cmp     ecx, [ebx + ETH_DEVICE.mtu]
        ja      .too_large

        push    ecx
        push    ax edx

        add     ecx, sizeof.ETH_header + NET_BUFF.data
        stdcall net_buff_alloc, ecx
        test    eax, eax
        jz      .out_of_ram

        mov     [eax + NET_BUFF.type], NET_BUFF_ETH
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data
        lea     edi, [eax + NET_BUFF.data]

        pop     esi
        movsd
        movsw
        lea     esi, [ebx + ETH_DEVICE.mac]
        movsd
        movsw
        pop     ax
        stosw

        lea     eax, [edi - sizeof.ETH_header - NET_BUFF.data]  ; Set eax to buffer start
        pop     ecx

        lea     edx, [ecx + sizeof.ETH_header]  ; Set edx to complete buffer size
        cmp     edx, ETH_FRAME_MINIMUM
        jbe     .adjust_size
  .done:
        mov     [eax + NET_BUFF.length], edx
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_output: ptr=%x size=%u\n", eax, edx
        ret

  .adjust_size:
        mov     edx, ETH_FRAME_MINIMUM
        test    edx, edx                        ; clear zero flag
        jmp     .done

  .out_of_ram:
        inc     [ebx + NET_DEVICE.packets_tx_drop]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_output: Out of ram!\n"
        add     esp, 4+2
        pop     ecx
        xor     eax, eax
        ret

  .too_large:
        inc     [eax + NET_DEVICE.packets_tx_err]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "ETH_output: Packet too large!\n"
        xor     eax, eax
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; eth_api: Part of system function 76.                            ;
;                                                                 ;
;  IN:  bl = subfunction number                                   ;
;       bh = device number                                        ;
;       ecx, edx, .. depends on subfunction                       ;
;                                                                 ;
; OUT:  depends on subfunction                                    ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
eth_api:

        cmp     bh, NET_DEVICES_MAX
        ja      .error
        movzx   eax, bh
        mov     eax, dword [net_device_list + 4*eax]
        cmp     [eax + NET_DEVICE.device_type], NET_DEVICE_ETH
        jne     .error

        and     ebx, 0xff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .read_mac       ; 0
  .number = ($ - .table) / 4 - 1

  .error:
        or      eax, -1
        ret


  .read_mac:
        movzx   ebx, word [eax + ETH_DEVICE.mac]
        mov     eax, dword [eax + ETH_DEVICE.mac + 2]
        mov     [esp+20+4], ebx                         ; FIXME
        ret


