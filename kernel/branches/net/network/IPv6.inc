;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2012. All rights reserved.         ;;
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

$Revision$


struct  IPv6_header

        VersionTrafficFlow      dd ?    ; Version[0-3], Traffic class[4-11], Flow Label [12-31]
        PayloadLength           dw ?    ; 16 bits, unsigned length of payload (extension headers are part of this)
        NextHeader              db ?    ; Values are same as in IPv4 'Protocol' field
        HopLimit                db ?    ; Decremented by every node, packet is discarded when it reaches 0
        SourceAddress           rd 4    ; 128-bit addresses
        DestinationAddress      rd 4    ;

ends





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

        DEBUGF  1,"IPv6_input\nfrom: %x:%x:%x:%x:%x:%x:%x:%x\n",\
        [edx + IPv6_header.SourceAddress + 0]:4,[edx + IPv6_header.SourceAddress + 2]:4,\
        [edx + IPv6_header.SourceAddress + 4]:4,[edx + IPv6_header.SourceAddress + 6]:4,\
        [edx + IPv6_header.SourceAddress + 8]:4,[edx + IPv6_header.SourceAddress + 10]:4,\
        [edx + IPv6_header.SourceAddress + 12]:4,[edx + IPv6_header.SourceAddress + 14]:4
        DEBUGF  1,"to: %x:%x:%x:%x:%x:%x:%x:%x\n",\
        [edx + IPv6_header.DestinationAddress + 0]:4,[edx + IPv6_header.DestinationAddress + 2]:4,\
        [edx + IPv6_header.DestinationAddress + 4]:4,[edx + IPv6_header.DestinationAddress + 6]:4,\
        [edx + IPv6_header.DestinationAddress + 8]:4,[edx + IPv6_header.DestinationAddress + 10]:4,\
        [edx + IPv6_header.DestinationAddress + 12]:4,[edx + IPv6_header.DestinationAddress + 14]:4

        sub     ecx, sizeof.IPv6_header
        jb      .dump

        movzx   eax, [edx + IPv6.PayloadLength]
        xchg    al, ah
        cmp     eax, ecx
        jb      .dump



;-------------------------------------------------------------------
; No, it's just a regular IP packet, pass it to the higher protocols

  .handle_it:

        movzx   esi, [edx + IPv6_header.VersionAndIHL]          ; Calculate Header length by using IHL field
        and     esi, 0x0000000f                                 ;
        shl     esi, 2                                          ;

        movzx   ecx, [edx + IPv6_header.TotalLength]            ; Calculate length of encapsulated Packet
        xchg    cl, ch                                          ;
        sub     ecx, esi                                        ;

        lea     edi, [edx + IPv6_header.SourceAddress]          ; make edi ptr to source and dest IPv6 address
        mov     al, [edx + IPv6_header.Protocol]
        add     esi, edx                                        ; make esi ptr to data

;        cmp     al, IP_PROTO_TCP
;        je      TCP_input

;        cmp     al, IP_PROTO_UDP
;        je      UDP_input

;        cmp     al, IP_PROTO_ICMP
;        je      ICMP_input

        DEBUGF  2,"IPv6_input - unknown protocol: %u\n", al

  .dump:
        DEBUGF  2,"IPv6_input - dumping\n"

        add     esp, 4
        call    KernelFree
        ret