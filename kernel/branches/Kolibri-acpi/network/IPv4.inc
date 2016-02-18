;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  IPv4.INC                                                       ;;
;;                                                                 ;;
;;  Part of the TCP/IP network stack for KolibriOS                 ;;
;;                                                                 ;;
;;  Based on the work of [Johnny_B] and [smb]                      ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6011 $

IPv4_MAX_FRAGMENTS              = 64
IPv4_MAX_ROUTES                 = 64

IPv4_ROUTE_FLAG_UP              = 1 shl 0
IPv4_ROUTE_FLAG_GATEWAY         = 1 shl 1
IPv4_ROUTE_FLAG_HOST            = 1 shl 2
IPv4_ROUTE_FLAG_D               = 1 shl 3       ; Route was created by a redirect
IPv4_ROUTE_FLAG_M               = 1 shl 4       ; Route was modified by a redirect

struct  IPv4_header

        VersionAndIHL           db ?    ; Version[0-3 bits] and IHL(header length)[4-7 bits]
        TypeOfService           db ?    ; precedence [7-5] minimize delay [4], maximize throughput [3], maximize riliability [2] minimize momentary cost [1] and zero [0]
        TotalLength             dw ?
        Identification          dw ?
        FlagsAndFragmentOffset  dw ?    ; Flags[0-2] and FragmentOffset[3-15]
        TimeToLive              db ?    ;
        Protocol                db ?
        HeaderChecksum          dw ?
        SourceAddress           dd ?
        DestinationAddress      dd ?

ends

struct  IPv4_FRAGMENT_slot

        ttl                     dw ?    ; Time to live for this entry, 0 for empty slot's
        id                      dw ?    ; Identification field from IP header
        SrcIP                   dd ?    ; .. from IP header
        DstIP                   dd ?    ; .. from IP header
        ptr                     dd ?    ; Pointer to first packet

ends

struct  IPv4_FRAGMENT_entry             ; This structure will replace the ethernet header in fragmented ip packets

        PrevPtr                 dd ?    ; Pointer to previous fragment entry  (-1 for first packet)
        NextPtr                 dd ?    ; Pointer to next fragment entry (-1 for last packet)
        Owner                   dd ?    ; Pointer to structure of driver
                                rb 2    ; to match ethernet header size         ;;; FIXME
                                        ; Ip header begins here (we will need the IP header to re-construct the complete packet)
ends

struct  IPv4_ROUTE

        Destination             dd ?
        Gateway                 dd ?
        Flags                   dd ?
        Use                     dd ?
        Interface               dd ?

ends


uglobal
align 4

        IP_LIST                 rd NET_DEVICES_MAX
        SUBNET_LIST             rd NET_DEVICES_MAX
        DNS_LIST                rd NET_DEVICES_MAX
        GATEWAY_LIST            rd NET_DEVICES_MAX
        BROADCAST_LIST          rd NET_DEVICES_MAX

        IPv4_packets_tx         rd NET_DEVICES_MAX
        IPv4_packets_rx         rd NET_DEVICES_MAX
        IPv4_packets_dumped     rd NET_DEVICES_MAX

        IPv4_FRAGMENT_LIST      rb IPv4_MAX_FRAGMENTS * sizeof.IPv4_FRAGMENT_slot

        IPv4_ROUTES             rd IPv4_MAX_ROUTES * sizeof.IPv4_ROUTE

endg


;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_init: Resets all IPv4 variables                            ;
;                                                                 ;
;-----------------------------------------------------------------;
macro   ipv4_init {

        xor     eax, eax
        mov     edi, IP_LIST
        mov     ecx, 7*NET_DEVICES_MAX + (sizeof.IPv4_FRAGMENT_slot*IPv4_MAX_FRAGMENTS)/4
        rep stosd

}


;-----------------------------------------------------------------;
;                                                                 ;
; Decrease TimeToLive of all fragment slots                       ;
;                                                                 ;
;-----------------------------------------------------------------;
macro ipv4_decrease_fragment_ttls {

local   .loop, .next

        mov     esi, IPv4_FRAGMENT_LIST
        mov     ecx, IPv4_MAX_FRAGMENTS
  .loop:
        cmp     [esi + IPv4_FRAGMENT_slot.ttl], 0
        je      .next
        dec     [esi + IPv4_FRAGMENT_slot.ttl]
        jz      .died
  .next:
        add     esi, sizeof.IPv4_FRAGMENT_slot
        dec     ecx
        jnz     .loop
        jmp     .done

  .died:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4 Fragment slot timed-out!\n"
;;; TODO: clear all entry's of timed-out slot
        jmp     .next

  .done:
}



macro ipv4_checksum ptr {

; This is the fast procedure to create or check an IP header without options
; To create a new checksum, the checksum field must be set to 0 before computation
; To check an existing checksum, leave the checksum as is, and it will be 0 after this procedure, if it was correct

        push    ebx
        xor     ebx, ebx
        add     bl, [ptr+1]
        adc     bh, [ptr+0]

        adc     bl, [ptr+3]
        adc     bh, [ptr+2]

        adc     bl, [ptr+5]
        adc     bh, [ptr+4]

        adc     bl, [ptr+7]
        adc     bh, [ptr+6]

        adc     bl, [ptr+9]
        adc     bh, [ptr+8]

; we skip 11th and 12th byte, they are the checksum bytes and should be 0 for re-calculation

        adc     bl, [ptr+13]
        adc     bh, [ptr+12]

        adc     bl, [ptr+15]
        adc     bh, [ptr+14]

        adc     bl, [ptr+17]
        adc     bh, [ptr+16]

        adc     bl, [ptr+19]
        adc     bh, [ptr+18]

        adc     ebx, 0

        push    ecx
        mov     ecx, ebx
        shr     ecx, 16
        and     ebx, 0xffff
        add     ebx, ecx

        mov     ecx, ebx
        shr     ecx, 16
        add     ebx, ecx

        not     bx
        jnz     .not_zero
        dec     bx
  .not_zero:
        xchg    bl, bh
        pop     ecx

        neg     word [ptr+10]           ; zero will stay zero so we just get the checksum
        add     word [ptr+10], bx       ;  , else we will get (new checksum - old checksum) in the end, wich should be 0 :)
        pop     ebx

}



;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_input: Check if IPv4 Packet isnt damaged and call          ;
; appropriate handler. (TCP/UDP/ICMP/..)                          ;
; We will also re-construct fragmented packets.                   ;
;                                                                 ;
;  IN:  Pointer to buffer in [esp]                                ;
;       pointer to device struct in ebx                           ;
;       pointer to IPv4 header in edx                             ;
;       size of IPv4 packet in ecx                                ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: packet from %u.%u.%u.%u ",\
        [edx + IPv4_header.SourceAddress + 0]:1,[edx + IPv4_header.SourceAddress + 1]:1,\
        [edx + IPv4_header.SourceAddress + 2]:1,[edx + IPv4_header.SourceAddress + 3]:1
        DEBUGF  DEBUG_NETWORK_VERBOSE, "to %u.%u.%u.%u\n",\
        [edx + IPv4_header.DestinationAddress + 0]:1,[edx + IPv4_header.DestinationAddress + 1]:1,\
        [edx + IPv4_header.DestinationAddress + 2]:1,[edx + IPv4_header.DestinationAddress + 3]:1

        call    net_ptr_to_num4
        cmp     edi, -1
        je      .invalid_device

;-------------------------------
; re-calculate the checksum

        ipv4_checksum edx
        jnz     .dump                                           ; if checksum isn't valid then dump packet

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Checksum ok\n"

;--------------------------------
; Check if destination IP matches

; local ip (Using RFC1122 strong end system model)
        mov     eax, [edx + IPv4_header.DestinationAddress]
        cmp     eax, [IP_LIST + edi]
        je      .ip_ok

; network layer broadcast
        cmp     eax, [BROADCAST_LIST + edi]
        je      .ip_ok

; physical layer broadcast (255.255.255.255)
        cmp     eax, 0xffffffff
        je      .ip_ok

; multicast (224.0.0.0/4 = 224.0.0.0 to 239.255.255.255)
        and     eax, 0x0fffffff
        cmp     eax, 224
        je      .ip_ok

; maybe we just dont have an IP yet and should accept everything on the IP level
        cmp     [IP_LIST + edi], 0
        je      .ip_ok

; or it's just not meant for us.. :(
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Destination address does not match!\n"
        jmp     .dump

;------------------------
; Now we can update stats

  .ip_ok:
        inc     [IPv4_packets_rx + edi]

;----------------------------------
; Check if the packet is fragmented

        test    [edx + IPv4_header.FlagsAndFragmentOffset], 1 shl 5     ; Is 'more fragments' flag set ?
        jnz     .has_fragments                                          ; If so, we definately have a fragmented packet

        test    [edx + IPv4_header.FlagsAndFragmentOffset], 0xff1f      ; If flag is not set, but there is a fragment offset, the packet is last in series of fragmented packets
        jnz     .is_last_fragment

;-------------------------------------------------------------------
; No, it's just a regular IP packet, pass it to the higher protocols

  .handle_it:                                                   ; We reach here if packet hasnt been fragmented, or when it already has been re-constructed

        movzx   esi, [edx + IPv4_header.VersionAndIHL]          ; Calculate Header length by using IHL field
        and     esi, 0x0000000f                                 ;
        shl     esi, 2                                          ;

        movzx   ecx, [edx + IPv4_header.TotalLength]            ; Calculate length of encapsulated Packet
        xchg    cl, ch                                          ;
        sub     ecx, esi                                        ;

        mov     al, [edx + IPv4_header.Protocol]
        add     esi, edx                                        ; make esi ptr to data

        cmp     al, IP_PROTO_TCP
        je      tcp_input

        cmp     al, IP_PROTO_UDP
        je      udp_input

        cmp     al, IP_PROTO_ICMP
        je      icmp_input

;-------------------------------
; Look for a matching RAW socket
        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        add     ecx, esi
        sub     ecx, edx
        mov     esi, edx
        movzx   edx, al
        mov     eax, net_sockets
  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .dump_unlock

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .next_socket

        cmp     [eax + SOCKET.Protocol], edx
        jne     .next_socket

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: found matching RAW socket: 0x%x\n", eax

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        jmp     socket_input

  .dump_unlock:

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: unknown protocol %u\n", al

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: dumping\n"
        inc     [IPv4_packets_dumped + edi]
        call    net_buff_free
        ret

  .invalid_device:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_input: packet originated from invalid device\n"
        call    net_buff_free
        ret


;---------------------------
; Fragmented packet handler


  .has_fragments:
        movzx   eax, [edx + IPv4_header.FlagsAndFragmentOffset]
        xchg    al, ah
        shl     ax, 3

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: fragmented packet offset=%u id=%x ptr=0x%x\n", ax, [edx + IPv4_header.Identification]:4, edx

        test    ax, ax                                          ; Is this the first packet of the fragment?
        jz      .is_first_fragment


;-------------------------------------------------------
; We have a fragmented IP packet, but it's not the first

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Middle fragment packet received!\n"

        call    ipv4_find_fragment_slot
        cmp     esi, -1
        je      .dump

        mov     [esi + IPv4_FRAGMENT_slot.ttl], 15              ; Reset the ttl
        mov     esi, [esi + IPv4_FRAGMENT_slot.ptr]
        or      edi, -1
  .find_last_entry:                                             ; The following routine will try to find the last entry
        cmp     edi, [esi + IPv4_FRAGMENT_entry.PrevPtr]
        jne     .destroy_slot                                   ; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
        mov     edi, esi
        mov     esi, [esi + IPv4_FRAGMENT_entry.NextPtr]
        cmp     esi, -1
        jne     .find_last_entry
                                                                ; We found the last entry (pointer is now in edi)
                                                                ; We are going to overwrite the ethernet header in received packet with a FRAGMENT_entry structure

        pop     eax                                             ; pointer to packet
        mov     [edi + IPv4_FRAGMENT_entry.NextPtr], eax        ; update pointer of previous entry to the new entry
        mov     [eax + IPv4_FRAGMENT_entry.NextPtr], -1
        mov     [eax + IPv4_FRAGMENT_entry.PrevPtr], edi
        mov     [eax + IPv4_FRAGMENT_entry.Owner], ebx

        ret


;------------------------------------
; We have received the first fragment

  .is_first_fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: First fragment packet received!\n"
                                                                ; try to locate a free slot..
        mov     ecx, IPv4_MAX_FRAGMENTS
        mov     esi, IPv4_FRAGMENT_LIST
  .find_free_slot:
        cmp     word [esi + IPv4_FRAGMENT_slot.ttl], 0
        je      .found_free_slot
        add     esi, sizeof.IPv4_FRAGMENT_slot
        loop    .find_free_slot
        jmp     .dump                                           ; If no free slot was found, dump the packet

  .found_free_slot:                                             ; We found a free slot, let's fill in the FRAGMENT_slot structure
        mov     [esi + IPv4_FRAGMENT_slot.ttl], 15              ; RFC recommends 15 secs as ttl
        mov     ax, [edx + IPv4_header.Identification]
        mov     [esi + IPv4_FRAGMENT_slot.id], ax
        mov     eax, [edx + IPv4_header.SourceAddress]
        mov     [esi + IPv4_FRAGMENT_slot.SrcIP], eax
        mov     eax, [edx + IPv4_header.DestinationAddress]
        mov     [esi + IPv4_FRAGMENT_slot.DstIP], eax
        pop     eax
        mov     [esi + IPv4_FRAGMENT_slot.ptr], eax
                                                                ; Now, replace ethernet header in original buffer with a FRAGMENT_entry structure
        mov     [eax + IPv4_FRAGMENT_entry.NextPtr], -1
        mov     [eax + IPv4_FRAGMENT_entry.PrevPtr], -1
        mov     [eax + IPv4_FRAGMENT_entry.Owner], ebx

        ret


;-----------------------------------
; We have received the last fragment

  .is_last_fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Last fragment packet received!\n"

        call    ipv4_find_fragment_slot
        cmp     esi, -1
        je      .dump

        mov     esi, [esi + IPv4_FRAGMENT_slot.ptr]                     ; We found the first entry, let's calculate total size of the packet in eax, so we can allocate a buffer
        push    esi
        xor     eax, eax
        or      edi, -1

  .count_bytes:
        cmp     [esi + IPv4_FRAGMENT_entry.PrevPtr], edi
        jne     .destroy_slot_pop                                                       ; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
        mov     cx, [esi + sizeof.IPv4_FRAGMENT_entry + IPv4_header.TotalLength]        ; Add total length
        xchg    cl, ch
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Packet size=%u\n", cx
        add     ax, cx
        movzx   cx, [esi + sizeof.IPv4_FRAGMENT_entry + IPv4_header.VersionAndIHL]      ; Sub Header length
        and     cx, 0x000F
        shl     cx, 2
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Header size=%u\n", cx
        sub     ax, cx
        mov     edi, esi
        mov     esi, [esi + IPv4_FRAGMENT_entry.NextPtr]
        cmp     esi, -1
        jne     .count_bytes

        mov     esi, [esp+4]
        mov     [edi + IPv4_FRAGMENT_entry.NextPtr], esi                                ; Add this packet to the chain, this simplifies the following code
        mov     [esi + IPv4_FRAGMENT_entry.NextPtr], -1
        mov     [esi + IPv4_FRAGMENT_entry.PrevPtr], edi
        mov     [esi + IPv4_FRAGMENT_entry.Owner], ebx

        mov     cx, [edx + IPv4_header.TotalLength]                                     ; Note: This time we dont substract Header length
        xchg    cl, ch
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Packet size=%u\n", cx
        add     ax, cx
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Total Received data size=%u\n", eax

        push    eax
        mov     ax, [edx + IPv4_header.FlagsAndFragmentOffset]
        xchg    al, ah
        shl     ax, 3
        add     cx, ax
        pop     eax
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Total Fragment size=%u\n", ecx

        cmp     ax, cx
        jne     .destroy_slot_pop

        push    eax
        push    eax
        call    kernel_alloc
        test    eax, eax
        je      .destroy_slot_pop                                                       ; If we dont have enough space to allocate the buffer, discard all packets in slot
        mov     edx, [esp+4]                                                            ; Get pointer to first fragment entry back in edx

  .rebuild_packet_loop:
        movzx   ecx, [edx + sizeof.IPv4_FRAGMENT_entry + IPv4_header.FlagsAndFragmentOffset] ; Calculate the fragment offset
        xchg    cl, ch                                                                  ;  intel byte order
        shl     cx, 3                                                                   ;   multiply by 8 and clear first 3 bits
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Fragment offset=%u\n", cx

        lea     edi, [eax + ecx]                                                        ; Notice that edi will be equal to eax for first fragment
        movzx   ebx, [edx + sizeof.IPv4_FRAGMENT_entry + IPv4_header.VersionAndIHL]     ; Find header size (in ebx) of fragment
        and     bx, 0x000F                                                              ;
        shl     bx, 2                                                                   ;

        lea     esi, [edx + sizeof.IPv4_FRAGMENT_entry]                                 ; Set esi to the correct begin of fragment
        movzx   ecx, [edx + sizeof.IPv4_FRAGMENT_entry + IPv4_header.TotalLength]       ; Calculate total length of fragment
        xchg    cl, ch                                                                  ;  intel byte order

        cmp     edi, eax                                                                ; Is this packet the first fragment ?
        je      .first_fragment
        sub     cx, bx                                                                  ; If not, dont copy the header
        add     esi, ebx                                                                ;
  .first_fragment:


        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Copying %u bytes from 0x%x to 0x%x\n", ecx, esi, edi
        push    cx                                                                      ; First copy dword-wise, then byte-wise
        shr     cx, 2                                                                   ;
        rep movsd                                                                       ;
        pop     cx                                                                      ;
        and     cx, 3                                                                   ;
        rep movsb                                                                       ;

        push    eax
        push    [edx + IPv4_FRAGMENT_entry.Owner]                                       ; we need to remeber the owner, in case this is the last packet
        push    [edx + IPv4_FRAGMENT_entry.NextPtr]                                     ; Set edx to the next pointer
        push    edx                                                                     ; Push pointer to fragment onto stack
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Next Fragment: 0x%x\n", edx
        call    net_buff_free                                                          ; free the previous fragment buffer (this uses the value from stack)
        pop     edx ebx eax
        cmp     edx, -1                                                                 ; Check if it is last fragment in chain
        jne     .rebuild_packet_loop

        pop     ecx
        xchg    cl, ch
        mov     edx, eax
        mov     [edx + IPv4_header.TotalLength], cx
        add     esp, 12
        xchg    cl, ch
        push    ecx edx                 ; size and pointer
        jmp     .handle_it              ; edx = buf ptr, ecx = size, [esp] buf ptr, [esp+4], total size, ebx=device ptr

  .destroy_slot_pop:
        add     esp, 4
  .destroy_slot:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Destroy fragment slot!\n"
        ; TODO!
        jmp     .dump





;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_find_fragment_slot                                         ;
;                                                                 ;
; IN: pointer to fragmented packet in edx                         ;
;                                                                 ;
; OUT: pointer to slot in esi, -1 on error                        ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_find_fragment_slot:

;;; TODO: the RFC says we should check protocol number too

        push    eax ebx ecx edx
        mov     ax, [edx + IPv4_header.Identification]
        mov     ecx, IPv4_MAX_FRAGMENTS
        mov     esi, IPv4_FRAGMENT_LIST
        mov     ebx, [edx + IPv4_header.SourceAddress]
        mov     edx, [edx + IPv4_header.DestinationAddress]
  .find_slot:
        cmp     [esi + IPv4_FRAGMENT_slot.id], ax
        jne     .try_next
        cmp     [esi + IPv4_FRAGMENT_slot.SrcIP], ebx
        jne     .try_next
        cmp     [esi + IPv4_FRAGMENT_slot.DstIP], edx
        je      .found_slot
  .try_next:
        add     esi, sizeof.IPv4_FRAGMENT_slot
        loop    .find_slot

        or      esi, -1
  .found_slot:
        pop     edx ecx ebx eax
        ret


;------------------------------------------------------------------;
;                                                                  ;
; ipv4_output                                                      ;
;                                                                  ;
;  IN:  al = protocol                                              ;
;       ah = TTL                                                   ;
;       ebx = device ptr (or 0 to let IP layer decide)             ;
;       ecx = data length                                          ;
;       edx = Source IP                                            ;
;       edi = Destination IP                                       ;
;                                                                  ;
; OUT:  eax = pointer to buffer start                              ;
;       eax = 0 on error                                           ;
;       ebx = device ptr (send packet through this device)         ;
;       ecx = data length                                          ;
;       edx = size of complete frame                               ;
;       edi = start of IPv4 payload                                ;
;                                                                  ;
;------------------------------------------------------------------;
align 4
ipv4_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output: size=%u ip=0x%x\n", ecx, eax

        cmp     ecx, 65500              ; Max IPv4 packet size
        ja      .too_large

        push    ecx ax edi
        mov     eax, edi
        call    ipv4_route              ; outputs device number in edi, dest ip in eax, source IP in edx
        test    eax, eax
        jz      .no_route
        push    edx
        test    edi, edi
        jz      .loopback

        call    arp_ip_to_mac
        test    eax, 0xffff0000         ; error bits
        jnz     .arp_error
        push    ebx                     ; push the mac onto the stack
        push    ax

        inc     [IPv4_packets_tx + edi] ; update stats

        mov     ax, ETHER_PROTO_IPv4
        mov     ebx, [NET_DRV_LIST + edi]
        mov     ecx, [esp + 6 + 8 + 2]
        add     ecx, sizeof.IPv4_header
        mov     edx, esp
        call    eth_output
        jz      .eth_error
        add     esp, 6                  ; pop the mac out of the stack

  .continue:
        xchg    cl, ch                                  ; internet byte order
        mov     [edi + IPv4_header.VersionAndIHL], 0x45 ; IPv4, normal length (no Optional header)
        mov     [edi + IPv4_header.TypeOfService], 0    ; nothing special, just plain ip packet
        mov     [edi + IPv4_header.TotalLength], cx
        mov     [edi + IPv4_header.Identification], 0   ; fragment id: FIXME
        mov     [edi + IPv4_header.FlagsAndFragmentOffset], 0

        mov     [edi + IPv4_header.HeaderChecksum], 0
        popd    [edi + IPv4_header.SourceAddress]
        popd    [edi + IPv4_header.DestinationAddress]

        pop     word[edi + IPv4_header.TimeToLive]      ; ttl shl 8 + protocol
;               [edi + IPv4_header.Protocol]

        pop     ecx

        ipv4_checksum edi
        add     edi, sizeof.IPv4_header
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output: success!\n"
        ret

  .eth_error:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: ethernet error\n"
        add     esp, 3*4+2+6
        xor     eax, eax
        ret

  .no_route:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: No route to host!\n"
        add     esp, 2*4+2
        xor     eax, eax
        ret

  .arp_error:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: ARP error=%x\n", eax
        add     esp, 4
        pop     eax
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: ip=0x%x\n", eax
        add     esp, 4+2
        xor     eax, eax
        ret

  .too_large:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: Packet too large!\n"
        xor     eax, eax
        ret

  .loopback:
        mov     dword [esp], eax                        ; set source IP to dest IP
        mov     ecx, [esp + 10]
        add     ecx, sizeof.IPv4_header
        mov     edi, AF_INET4
        call    loop_output
        jmp     .continue




;------------------------------------------------------------------;
;                                                                  ;
; ipv4_output_raw                                                  ;
;                                                                  ;
;  IN: eax = socket ptr                                            ;
;      ecx = data length                                           ;
;      esi = data ptr                                              ;
;                                                                  ;
; OUT: eax = -1 on error                                           ;
;                                                                  ;
;------------------------------------------------------------------;
align 4
ipv4_output_raw:

        DEBUGF 1,"IPv4_output_raw: size=%u ptr=%x socket=%x\n", ecx, esi, eax

        sub     esp, 8
        push    esi eax

        call    ipv4_route
        call    arp_ip_to_mac

        test    eax, 0xffff0000         ; error bits
        jnz     .arp_error

        push    ebx                     ; push the mac
        push    ax

        inc     [IPv4_packets_tx + 4*edi]
        mov     ax, ETHER_PROTO_IPv4
        mov     ebx, [NET_DRV_LIST + 4*edi]
        mov     ecx, [esp + 6 + 4]
        add     ecx, sizeof.IPv4_header
        mov     edx, esp
        call    eth_output
        jz      .error
        add     esp, 6  ; pop the mac

        mov     dword[esp+4+4], edx
        mov     dword[esp+4+4+4], eax

        pop     eax esi
;; TODO: check socket options if we should add header, or just compute checksum

        push    edi ecx
        rep movsb
        pop     ecx edi

;        [edi + IPv4_header.VersionAndIHL]              ; IPv4, normal length (no Optional header)
;        [edi + IPv4_header.TypeOfService]              ; nothing special, just plain ip packet
;        [edi + IPv4_header.TotalLength]
;        [edi + IPv4_header.TotalLength]                ; internet byte order
;        [edi + IPv4_header.FlagsAndFragmentOffset]

        mov     [edi + IPv4_header.HeaderChecksum], 0

;        [edi + IPv4_header.TimeToLive]                 ; ttl shl 8 + protocol
;        [edi + IPv4_header.Protocol]
;        [edi + IPv4_header.Identification]             ; fragment id
;        [edi + IPv4_header.SourceAddress]
;        [edi + IPv4_header.DestinationAddress]

        ipv4_checksum edi                       ;;;; todo: checksum for IP packet with options!
        add     edi, sizeof.IPv4_header
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output_raw: device=%x\n", ebx
        call    [ebx + NET_DEVICE.transmit]
        ret

  .error:
        add     esp, 6+8+4+4
        mov     ebx, ENOBUFS            ; FIXME: NOBUFS or MSGSIZE error
        or      eax, -1
        ret

  .arp_error:
        add     esp, 8+4+4
        mov     ebx, ENOTCONN
        or      eax, -1
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_fragment                                                   ;
;                                                                 ;
;  IN:  [esp] = ptr to packet buffer to fragment                  ;
;       edi = ptrr to ip header in that buffer                    ;
;       ebx = device ptr                                          ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
proc ipv4_fragment stdcall buffer

locals
        offset          dd ?
        headerlength    dd ?
        headerptr       dd ?
        dataptr         dd ?
        remaining       dd ?
        segmentsize     dd ?
endl

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_fragment\n"

; We must be able to put at least 8 bytes per segment
        movzx   eax, byte[edi]          ; IHL
        and     eax, 0xf
        shl     eax, 2
        mov     [headerlength], eax
        add     eax, 8
        mov     ecx, [ebx + NET_DEVICE.mtu]
        and     ecx, not 11b
        cmp     ecx, eax
        jb      .fail

        mov     [edi + IPv4_header.HeaderChecksum], 0

        mov     [segmentsize], ecx
        mov     [headerptr], edi
        movzx   ecx, [edi + IPv4_header.TotalLength]
        xchg    cl, ch
        sub     ecx, [headerlength]
        mov     [remaining], ecx
        mov     [offset], 0

        add     edi, [headerlength]
        mov     [dataptr], edi

  .loop:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "Ipv4_fragment: new fragment"

        mov     ecx, [segmentsize]
        cmp     ecx, [remaining]
        jbe     @f
        mov     ecx, [remaining]
  @@:

        mov     ax, ETHER_PROTO_IPv4
        mov     edx, [esp]
        add     edx, [edx + NET_BUFF.offset]
;        add     edx, ETH_header.DstMAC         ; = 0
        call    ETH_output
        jz      .fail

        push    edi
        mov     edx, ecx

; copy header
        mov     esi, [headerptr]
        mov     ecx, [headerlength]
        shr     ecx, 2
        rep movsd

; copy data
        mov     esi, [dataptr]
        add     esi, [offset]
        mov     ecx, edx
        sub     ecx, [headerlength]
        shr     ecx, 2
        rep movsd
        pop     edi

; now, correct header
; packet length
        mov     ax, dx
        xchg    al, ah
        mov     [edi + IPv4_header.TotalLength], ax

; offset
        mov     eax, [offset]
        xchg    al, ah

        sub     edx, [headerlength]
        sub     [remaining], edx
        je      @f
        jb      .fail
        or      ah, 1 shl 2             ; more fragments
        add     [offset], edx
  @@:
        mov     [edi + IPv4_header.FlagsAndFragmentOffset], ax

; Send the fragment
        IPv4_checksum edi
        call    [ebx + NET_DEVICE.transmit]

        cmp     [remaining], 0
        jne     .loop

        call    NET_BUFF_free
        ret

      .fail:
        DEBUGF  DEBUG_NETWORK_ERROR, "Ipv4_fragment: failed\n"
        call    NET_BUFF_free
        ret

endp



;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_route                                                      ;
;                                                                 ;
; IN:   eax = Destination IP                                      ;
;       ebx = outgoing device / 0                                 ;
;       edx = Source IP                                           ;
;                                                                 ;
; OUT:  eax = Destination IP (may be gateway), 0 on error         ;
;       edx = Source IP                                           ;
;       edi = device number*4                                     ;
;                                                                 ;
; DESTROYED:                                                      ;
;       ecx                                                       ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_route:

        test    ebx, ebx
        jnz     .got_device

; Broadcast does not need gateway
        cmp     eax, 0xffffffff
        je      .broadcast

        xor     edi, edi
  .loop:
        mov     ebx, [IP_LIST + edi]
        and     ebx, [SUBNET_LIST + edi]
        jz      .next
        mov     ecx, eax
        and     ecx, [SUBNET_LIST + edi]
        cmp     ebx, ecx
        je      .got_it
  .next:
        add     edi, 4
        cmp     edi, 4*NET_DEVICES_MAX
        jb      .loop

        mov     eax, [GATEWAY_LIST + 4]         ; TODO: let user (or a user space daemon) configure default route
  .broadcast:
        mov     edi, 4                          ; TODO: same as above
  .got_it:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_route: %u\n", edi
        test    edx, edx
        jnz     @f
        mov     edx, [IP_LIST + edi]
  @@:

        ret

  .got_device:
; Validate device ptr and convert to device number
        call    net_ptr_to_num4
        cmp     edi, -1
        je      .fail

        mov     edx, [IP_LIST + edi]            ; Source IP

; Broadcast does not need gateway
        cmp     eax, 0xffffffff
        je      @f

; Check if we should route to gateway or not
        mov     ebx, [IP_LIST + edi]
        and     ebx, [SUBNET_LIST + edi]
        mov     ecx, eax
        and     ecx, [SUBNET_LIST + edi]
        je      @f
        mov     eax, [GATEWAY_LIST + edi]
  @@:
        ret

  .fail:
        xor     eax, eax
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_get_frgmnt_num                                             ;
;                                                                 ;
;  IN: /                                                          ;
;                                                                 ;
; OUT: ax = fragment number                                       ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_get_frgmnt_num:
        xor     ax, ax  ;;; TODO: replace this with real code

        ret


;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_connect                                                    ;
;                                                                 ;
;   IN: eax = socket pointer                                      ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = error code on error                                 ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_connect:

        push    eax edx
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        pop     edx eax

; Fill in local IP
        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST + 4]                                   ; FIXME: use correct local IP
        pop     [eax + IP_SOCKET.LocalIP]

; Fill in remote IP
        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        xor     eax, eax
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; ipv4_API: Part of system function 76.                           ;
;                                                                 ;
;  IN:  bl = subfunction number                                   ;
;       bh = device number                                        ;
;       ecx, edx, .. depends on subfunction                       ;
;                                                                 ;
; OUT:  depends on subfunction                                    ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
ipv4_api:

        movzx   eax, bh
        shl     eax, 2

        and     ebx, 0x000000ff
        cmp     ebx, .number
        ja      .error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      .packets_tx     ; 0
        dd      .packets_rx     ; 1
        dd      .read_ip        ; 2
        dd      .write_ip       ; 3
        dd      .read_dns       ; 4
        dd      .write_dns      ; 5
        dd      .read_subnet    ; 6
        dd      .write_subnet   ; 7
        dd      .read_gateway   ; 8
        dd      .write_gateway  ; 9
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret

  .packets_tx:
        mov     eax, [IPv4_packets_tx + eax]
        ret

  .packets_rx:
        mov     eax, [IPv4_packets_rx + eax]
        ret

  .read_ip:
        mov     eax, [IP_LIST + eax]
        ret

  .write_ip:
        mov     [IP_LIST + eax], ecx
        mov     edi, eax                        ; device number, we'll need it for ARP

        ; pre-calculate the local broadcast address
        mov     ebx, [SUBNET_LIST + eax]
        not     ebx
        or      ebx, ecx
        mov     [BROADCAST_LIST + eax], ebx

        mov     ebx, [NET_DRV_LIST + eax]
        mov     eax, [IP_LIST + eax]
        call    arp_output_request              ; now send a gratuitous ARP

        call    net_send_event
        xor     eax, eax
        ret

  .read_dns:
        mov     eax, [DNS_LIST + eax]
        ret

  .write_dns:
        mov     [DNS_LIST + eax], ecx
        call    net_send_event
        xor     eax, eax
        ret

  .read_subnet:
        mov     eax, [SUBNET_LIST + eax]
        ret

  .write_subnet:
        mov     [SUBNET_LIST + eax], ecx

        ; pre-calculate the local broadcast address
        mov     ebx, [IP_LIST + eax]
        not     ecx
        or      ecx, ebx
        mov     [BROADCAST_LIST + eax], ecx

        call    net_send_event
        xor     eax, eax
        ret

  .read_gateway:
        mov     eax, [GATEWAY_LIST + eax]
        ret

  .write_gateway:
        mov     [GATEWAY_LIST + eax], ecx

        call    net_send_event
        xor     eax, eax
        ret