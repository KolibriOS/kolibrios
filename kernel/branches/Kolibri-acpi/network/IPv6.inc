;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2012-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  IPv6.INC                                                       ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 3251 $


struct  IPv6_header

        VersionTrafficFlow      dd ?    ; Version[0-3], Traffic class[4-11], Flow Label [12-31]
        PayloadLength           dw ?    ; 16 bits, unsigned length of payload (extension headers are part of this)
        NextHeader              db ?    ; Values are same as in IPv4 'Protocol' field
        HopLimit                db ?    ; Decremented by every node, packet is discarded when it reaches 0
        SourceAddress           rd 4    ; 128-bit addresses
        DestinationAddress      rd 4    ;
        Payload                 rb 0

ends


align 4
uglobal

        IPv6:
        .addresses      rd 4*MAX_NET_DEVICES
        .subnet         rd 4*MAX_NET_DEVICES
        .dns            rd 4*MAX_NET_DEVICES
        .gateway        rd 4*MAX_NET_DEVICES

        .packets_tx     rd MAX_NET_DEVICES
        .packets_rx     rd MAX_NET_DEVICES

endg


;-----------------------------------------------------------------
;
; IPv6_init
;
;  This function resets all IP variables
;
;-----------------------------------------------------------------
macro   IPv6_init {

        xor     eax, eax
        mov     edi, IPv6
        mov     ecx, (4*4*4+2*4)MAX_IP
        rep     stosd

}



;-----------------------------------------------------------------
;
; IPv6_input:
;
;  Will check if IPv6 Packet isnt damaged
;  and call appropriate handler. (TCP/UDP/ICMP/..)
;
;  It will also re-construct fragmented packets
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to IPv6 header in edx
;       size of IPv6 packet in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
IPv6_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input from: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",\
        [edx + IPv6_header.SourceAddress + 0]:2,[edx + IPv6_header.SourceAddress + 1]:2,\
        [edx + IPv6_header.SourceAddress + 2]:2,[edx + IPv6_header.SourceAddress + 3]:2,\
        [edx + IPv6_header.SourceAddress + 4]:2,[edx + IPv6_header.SourceAddress + 5]:2,\
        [edx + IPv6_header.SourceAddress + 6]:2,[edx + IPv6_header.SourceAddress + 7]:2,\
        [edx + IPv6_header.SourceAddress + 8]:2,[edx + IPv6_header.SourceAddress + 9]:2,\
        [edx + IPv6_header.SourceAddress + 10]:2,[edx + IPv6_header.SourceAddress + 11]:2,\
        [edx + IPv6_header.SourceAddress + 12]:2,[edx + IPv6_header.SourceAddress + 13]:2,\
        [edx + IPv6_header.SourceAddress + 14]:2,[edx + IPv6_header.SourceAddress + 15]:2

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input to: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",\
        [edx + IPv6_header.DestinationAddress + 0]:2,[edx + IPv6_header.DestinationAddress + 1]:2,\
        [edx + IPv6_header.DestinationAddress + 2]:2,[edx + IPv6_header.DestinationAddress + 3]:2,\
        [edx + IPv6_header.DestinationAddress + 4]:2,[edx + IPv6_header.DestinationAddress + 5]:2,\
        [edx + IPv6_header.DestinationAddress + 6]:2,[edx + IPv6_header.DestinationAddress + 7]:2,\
        [edx + IPv6_header.DestinationAddress + 8]:2,[edx + IPv6_header.DestinationAddress + 9]:2,\
        [edx + IPv6_header.DestinationAddress + 10]:2,[edx + IPv6_header.DestinationAddress + 11]:2,\
        [edx + IPv6_header.DestinationAddress + 12]:2,[edx + IPv6_header.DestinationAddress + 13]:2,\
        [edx + IPv6_header.DestinationAddress + 14]:2,[edx + IPv6_header.DestinationAddress + 15]:2

        sub     ecx, sizeof.IPv6_header
        jb      .dump

        cmp     cx, [edx + IPv6_header.PayloadLength]
        jb      .dump

;-------------------------------------------------------------------
; No, it's just a regular IP packet, pass it to the higher protocols

  .handle_it:
        movzx   ecx, [edx + IPv6_header.PayloadLength]
        lea     edi, [edx + IPv6_header.SourceAddress]          ; make edi ptr to source and dest IPv6 address
        lea     esi, [edx + IPv6_header.Payload]                ; make esi ptr to data
        mov     al, [edx + IPv6_header.NextHeader]

  .scan:
        cmp     al, 59  ; no next
        je      .dump

        cmp     al, 0
        je      .hop_by_hop

        cmp     al, 43
        je      .routing

        cmp     al, 44
        je      .fragment

        cmp     al, 60
        je      .dest_opts

;        cmp     al, IP_PROTO_TCP
;        je      TCP_input

;        cmp     al, IP_PROTO_UDP
;        je      UDP_input

;        cmp     al, 58
;        je      ICMP6_input

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - unknown protocol: %u\n", al

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - dumping\n"
        call    kernel_free
        add     esp, 4

        ret

  .dump_options:
        add     esp, 2+4+4
        jmp     .dump

  .nextheader:
        pop     esi
        pop     ecx
        pop     ax
        jmp     .scan

;-------------------------
; Hop-by-Hop

  .hop_by_hop:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - hop by hop\n"
        pushw   [esi]                   ; 8 bit identifier for option type
        movzx   eax, byte[esi + 1]      ; Hdr Ext Len
        inc     eax                     ; first 8 octets not counted
        shl     eax, 3                  ; * 8
        sub     ecx, eax
        push    ecx
        add     eax, esi
        push    eax
        inc     esi
        inc     esi

        mov     al, [esi]

        cmp     al, 0
        je      .pad_1

        cmp     al, 1
        je      .pad_n

        ; TODO: check with other known options

; unknown option.. discard packet or not?
; check highest two bits
        test    al, 0xc0        ; discard packet
        jnz     .dump_options

  .pad_n:
        movzx   eax, byte[esi + 1]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - pad %u\n", eax
        inc     esi
        inc     esi
        add     esi, eax
        sub     ecx, eax
        jmp     .hop_by_hop

  .pad_1:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - pad 1\n"
        inc     esi
        dec     ecx
        jmp     .hop_by_hop



  .dest_opts:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - dest opts\n"
        jmp     .nextheader

  .routing:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - routing\n"
        pushw   [esi]                   ; 8 bit identifier for option type
        movzx   eax, byte[esi + 1]      ; Hdr Ext Len
        inc     eax                     ; first 8 octets not counted
        shl     eax, 3                  ; * 8
        sub     ecx, eax
        push    ecx
        add     eax, esi
        push    eax
        inc     esi
        inc     esi

        cmp     al, 0
        je      .pad_1

        cmp     al, 1
        je      .pad_n

        mov     al, [esi]       ; routing type

        jmp     .nextheader

  .fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv6_input - fragment\n"

        jmp     .nextheader






;---------------------------------------------------------------------------
;
; IPv6_API
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
IPv6_api:

        movzx   eax, bh
        shl     eax, 2

        and     ebx, 0x000000ff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .packets_tx     ; 0
        dd      .packets_rx     ; 1
;        dd      .read_ip        ; 2
;        dd      .write_ip       ; 3
;        dd      .read_dns       ; 4
;        dd      .write_dns      ; 5
;        dd      .read_subnet    ; 6
;        dd      .write_subnet   ; 7
;        dd      .read_gateway   ; 8
;        dd      .write_gateway  ; 9
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret

  .packets_tx:
        mov     eax, [IPv6.packets_tx + eax]
        ret

  .packets_rx:
        mov     eax, [IPv6.packets_rx + eax]
        ret

