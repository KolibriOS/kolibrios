;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
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

$Revision: 3515 $

MAX_FRAGMENTS                   = 64

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

struct  FRAGMENT_slot

        ttl                     dw ?    ; Time to live for this entry, 0 for empty slot's
        id                      dw ?    ; Identification field from IP header
        SrcIP                   dd ?    ; .. from IP header
        DstIP                   dd ?    ; .. from IP header
        ptr                     dd ?    ; Pointer to first packet

ends

struct  FRAGMENT_entry                  ; This structure will replace the ethernet header in fragmented ip packets

        PrevPtr                 dd ?    ; Pointer to previous fragment entry  (-1 for first packet)
        NextPtr                 dd ?    ; Pointer to next fragment entry (-1 for last packet)
        Owner                   dd ?    ; Pointer to structure of driver
                                rb 2    ; to match ethernet header size         ;;; FIXME
                                        ; Ip header begins here (we will need the IP header to re-construct the complete packet)
ends


uglobal
align 4

        IP_LIST                 rd NET_DEVICES_MAX
        SUBNET_LIST             rd NET_DEVICES_MAX
        DNS_LIST                rd NET_DEVICES_MAX
        GATEWAY_LIST            rd NET_DEVICES_MAX
        BROADCAST_LIST          rd NET_DEVICES_MAX

        IP_packets_tx           rd NET_DEVICES_MAX
        IP_packets_rx           rd NET_DEVICES_MAX
        IP_packets_dumped       rd NET_DEVICES_MAX

        FRAGMENT_LIST           rb MAX_FRAGMENTS * sizeof.FRAGMENT_slot

endg


;-----------------------------------------------------------------
;
; IPv4_init
;
;  This function resets all IP variables
;
;-----------------------------------------------------------------
macro   IPv4_init {

        xor     eax, eax
        mov     edi, IP_LIST
        mov     ecx, 7*NET_DEVICES_MAX + (sizeof.FRAGMENT_slot*MAX_FRAGMENTS)/4
        rep stosd

}


;-----------------------------------------------------------------
;
; Decrease TimeToLive of all fragment slots
;
;-----------------------------------------------------------------
macro IPv4_decrease_fragment_ttls {

local   .loop, .next

        mov     esi, FRAGMENT_LIST
        mov     ecx, MAX_FRAGMENTS
  .loop:
        cmp     [esi + FRAGMENT_slot.ttl], 0
        je      .next
        dec     [esi + FRAGMENT_slot.ttl]
        jz      .died
  .next:
        add     esi, sizeof.FRAGMENT_slot
        dec     ecx
        jnz     .loop
        jmp     .done

  .died:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4 Fragment slot timed-out!\n"
;;; TODO: clear all entry's of timed-out slot
        jmp     .next

  .done:
}



macro IPv4_checksum ptr {

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



;-----------------------------------------------------------------
;
; IPv4_input:
;
;  Will check if IPv4 Packet isnt damaged
;  and call appropriate handler. (TCP/UDP/ICMP/..)
;
;  It will also re-construct fragmented packets
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to IPv4 header in edx
;       size of IPv4 packet in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
IPv4_input:                                                     ; TODO: add IPv4 raw sockets support

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input, packet from: %u.%u.%u.%u ",\
        [edx + IPv4_header.SourceAddress + 0]:1,[edx + IPv4_header.SourceAddress + 1]:1,\
        [edx + IPv4_header.SourceAddress + 2]:1,[edx + IPv4_header.SourceAddress + 3]:1
        DEBUGF  DEBUG_NETWORK_VERBOSE, "to: %u.%u.%u.%u\n",\
        [edx + IPv4_header.DestinationAddress + 0]:1,[edx + IPv4_header.DestinationAddress + 1]:1,\
        [edx + IPv4_header.DestinationAddress + 2]:1,[edx + IPv4_header.DestinationAddress + 3]:1

;-------------------------------
; re-calculate the checksum

        IPv4_checksum edx
        jnz     .dump                                           ; if checksum isn't valid then dump packet

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Checksum ok\n"

;-----------------------------------
; Check if destination IP is correct

        call    NET_ptr_to_num4

        ; check if it matches local ip (Using RFC1122 strong end system model)

        mov     eax, [edx + IPv4_header.DestinationAddress]
        cmp     eax, [IP_LIST + edi]
        je      .ip_ok

        ; check for broadcast (IP or (not SUBNET))

        cmp     eax, [BROADCAST_LIST + edi]
        je      .ip_ok

        ; or a special broadcast (255.255.255.255)

        cmp     eax, 0xffffffff
        je      .ip_ok

        ; maybe it's a multicast (224.0.0.0/4)

        and     eax, 0x0fffffff
        cmp     eax, 224
        je      .ip_ok

        ; or a loopback address (127.0.0.0/8)

        and     eax, 0x00ffffff
        cmp     eax, 127
        je      .ip_ok

        ; or it's just not meant for us.. :(

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Destination address does not match!\n"
        jmp     .dump

;------------------------
; Now we can update stats

  .ip_ok:
        inc     [IP_packets_rx + edi]

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

        lea     edi, [edx + IPv4_header.SourceAddress]          ; make edi ptr to source and dest IPv4 address
        mov     al, [edx + IPv4_header.Protocol]
        add     esi, edx                                        ; make esi ptr to data

        cmp     al, IP_PROTO_TCP
        je      TCP_input

        cmp     al, IP_PROTO_UDP
        je      UDP_input

        cmp     al, IP_PROTO_ICMP
        je      ICMP_input

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: unknown protocol %u\n", al

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: dumping\n"
        inc     [IP_packets_dumped]                             ; FIXME: use correct interface
        call    NET_packet_free
        add     esp, 4                                          ; pop (balance stack)
        ret


;---------------------------
; Fragmented packet handler


  .has_fragments:
        movzx   eax, [edx + IPv4_header.FlagsAndFragmentOffset]
        xchg    al, ah
        shl     ax, 3

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: fragmented packet offset=%u id=%x\n", ax, [edx + IPv4_header.Identification]:4

        test    ax, ax                                          ; Is this the first packet of the fragment?
        jz      .is_first_fragment


;-------------------------------------------------------
; We have a fragmented IP packet, but it's not the first

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Middle fragment packet received!\n"

        call    IPv4_find_fragment_slot
        cmp     esi, -1
        je      .dump

        mov     [esi + FRAGMENT_slot.ttl], 15                   ; Reset the ttl
        mov     esi, [esi + FRAGMENT_slot.ptr]
        or      edi, -1
  .find_last_entry:                                             ; The following routine will try to find the last entry
        cmp     edi, [esi + FRAGMENT_entry.PrevPtr]
        jne     .destroy_slot                                   ; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
        mov     edi, esi
        mov     esi, [esi + FRAGMENT_entry.NextPtr]
        cmp     esi, -1
        jne     .find_last_entry
                                                                ; We found the last entry (pointer is now in edi)
                                                                ; We are going to overwrite the ethernet header in received packet with a FRAGMENT_entry structure

        pop     eax                                             ; pointer to packet
        mov     [edi + FRAGMENT_entry.NextPtr], eax             ; update pointer of previous entry to the new entry
        mov     [eax + FRAGMENT_entry.NextPtr], -1
        mov     [eax + FRAGMENT_entry.PrevPtr], edi
        mov     [eax + FRAGMENT_entry.Owner], ebx

        add     esp, 4
        ret


;------------------------------------
; We have received the first fragment

  .is_first_fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: First fragment packet received!\n"
                                                                ; try to locate a free slot..
        mov     ecx, MAX_FRAGMENTS
        mov     esi, FRAGMENT_LIST
  .find_free_slot:
        cmp     word [esi + FRAGMENT_slot.ttl], 0
        je      .found_free_slot
        add     esi, sizeof.FRAGMENT_slot
        loop    .find_free_slot
        jmp     .dump                                           ; If no free slot was found, dump the packet

  .found_free_slot:                                             ; We found a free slot, let's fill in the FRAGMENT_slot structure
        mov     [esi + FRAGMENT_slot.ttl], 15                   ; RFC recommends 15 secs as ttl
        mov     ax, [edx + IPv4_header.Identification]
        mov     [esi + FRAGMENT_slot.id], ax
        mov     eax, [edx + IPv4_header.SourceAddress]
        mov     [esi + FRAGMENT_slot.SrcIP], eax
        mov     eax, [edx + IPv4_header.DestinationAddress]
        mov     [esi + FRAGMENT_slot.DstIP], eax
        pop     eax
        mov     [esi + FRAGMENT_slot.ptr], eax
                                                                ; Now, replace ethernet header in original buffer with a FRAGMENT_entry structure
        mov     [eax + FRAGMENT_entry.NextPtr], -1
        mov     [eax + FRAGMENT_entry.PrevPtr], -1
        mov     [eax + FRAGMENT_entry.Owner], ebx

        add     esp, 4                                          ; balance stack and exit
        ret


;-----------------------------------
; We have received the last fragment

  .is_last_fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Last fragment packet received!\n"

        call    IPv4_find_fragment_slot
        cmp     esi, -1
        je      .dump

        mov     esi, [esi + FRAGMENT_slot.ptr]                  ; We found the first entry, let's calculate total size of the packet in eax, so we can allocate a buffer
        push    esi
        xor     eax, eax
        or      edi, -1

  .count_bytes:
        cmp     [esi + FRAGMENT_entry.PrevPtr], edi
        jne     .destroy_slot_pop                                               ; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
        mov     cx, [esi + sizeof.FRAGMENT_entry + IPv4_header.TotalLength]     ; Add total length
        xchg    cl, ch
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Packet size=%u\n", cx
        add     ax, cx
        movzx   cx, [esi + sizeof.FRAGMENT_entry + IPv4_header.VersionAndIHL]   ; Sub Header length
        and     cx, 0x000F
        shl     cx, 2
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Header size=%u\n", cx
        sub     ax, cx
        mov     edi, esi
        mov     esi, [esi + FRAGMENT_entry.NextPtr]
        cmp     esi, -1
        jne     .count_bytes

        mov     esi, [esp+4]
        mov     [edi + FRAGMENT_entry.NextPtr], esi                             ; Add this packet to the chain, this simplifies the following code
        mov     [esi + FRAGMENT_entry.NextPtr], -1
        mov     [esi + FRAGMENT_entry.PrevPtr], edi
        mov     [esi + FRAGMENT_entry.Owner], ebx

        mov     cx, [edx + IPv4_header.TotalLength]                             ; Note: This time we dont substract Header length
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
        movzx   ecx, [edx + sizeof.FRAGMENT_entry + IPv4_header.FlagsAndFragmentOffset] ; Calculate the fragment offset
        xchg    cl, ch                                                                  ;  intel byte order
        shl     cx, 3                                                                   ;   multiply by 8 and clear first 3 bits
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Fragment offset=%u\n", cx

        lea     edi, [eax + ecx]                                                        ; Notice that edi will be equal to eax for first fragment
        movzx   ebx, [edx + sizeof.FRAGMENT_entry + IPv4_header.VersionAndIHL]          ; Find header size (in ebx) of fragment
        and     bx, 0x000F                                                              ;
        shl     bx, 2                                                                   ;

        lea     esi, [edx + sizeof.FRAGMENT_entry]                                      ; Set esi to the correct begin of fragment
        movzx   ecx, [edx + sizeof.FRAGMENT_entry + IPv4_header.TotalLength]            ; Calculate total length of fragment
        xchg    cl, ch                                                                  ;  intel byte order

        cmp     edi, eax                                                                ; Is this packet the first fragment ?
        je      .first_fragment
        sub     cx, bx                                                                  ; If not, dont copy the header
        add     esi, ebx                                                                ;
  .first_fragment:

        push    cx                                                                      ; First copy dword-wise, then byte-wise
        shr     cx, 2                                                                   ;
        rep movsd                                                                       ;
        pop     cx                                                                      ;
        and     cx, 3                                                                   ;
        rep movsb                                                                       ;

        push    eax
        push    edx                                                                     ; Push pointer to fragment onto stack
        mov     ebx, [edx + FRAGMENT_entry.Owner]                                       ; we need to remeber the owner, in case this is the last packet
        mov     edx, [edx + FRAGMENT_entry.NextPtr]                                     ; Set edx to the next pointer
        call    NET_packet_free                                                         ; free the previous fragment buffer (this uses the value from stack)
        pop     eax
        cmp     edx, -1                                                                 ; Check if it is last fragment in chain
        jne     .rebuild_packet_loop

        pop     ecx
        xchg    cl, ch
        mov     edx, eax
        mov     [edx + IPv4_header.TotalLength], cx
        add     esp, 8
        xchg    cl, ch
        push    ecx

        push    eax
        jmp     .handle_it          ; edx = buf ptr, ecx = size, [esp] buf ptr, [esp+4], total size, ebx=device ptr

  .destroy_slot_pop:
        add     esp, 4
  .destroy_slot:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_input: Destroy fragment slot!\n"
        ; TODO!
        jmp     .dump





;-----------------------------------------------------------------
;
; find fragment slot
;
; IN: pointer to fragmented packet in edx
; OUT: pointer to slot in esi, -1 on error
;
;-----------------------------------------------------------------
align 4
IPv4_find_fragment_slot:

;;; TODO: the RFC says we should check protocol number too

        push    eax ebx ecx edx
        mov     ax, [edx + IPv4_header.Identification]
        mov     ecx, MAX_FRAGMENTS
        mov     esi, FRAGMENT_LIST
        mov     ebx, [edx + IPv4_header.SourceAddress]
        mov     edx, [edx + IPv4_header.DestinationAddress]
  .find_slot:
        cmp     [esi + FRAGMENT_slot.id], ax
        jne     .try_next
        cmp     [esi + FRAGMENT_slot.SrcIP], ebx
        jne     .try_next
        cmp     [esi + FRAGMENT_slot.DstIP], edx
        je      .found_slot
  .try_next:
        add     esi, sizeof.FRAGMENT_slot
        loop    .find_slot

        or      esi, -1
  .found_slot:
        pop     edx ecx ebx eax
        ret


;------------------------------------------------------------------
;
; IPv4_output
;
; IN: eax = dest ip
;     ebx = output device ptr/0 for automatic choice
;     ecx = data length
;     edx = source ip
;     di  = TTL shl 8 + protocol
;
; OUT: eax = pointer to buffer start
;      ebx = pointer to device struct (needed for sending procedure)
;      ecx = unchanged (packet size of embedded data)
;      edx = size of complete buffer
;      edi = pointer to start of data (0 on error)
;
;------------------------------------------------------------------
align 4
IPv4_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output: size=%u ip=0x%x\n", ecx, eax

        cmp     ecx, 65500              ; Max IPv4 packet size
        ja      .too_large

        push    ecx eax edx di

        call    IPv4_route              ; outputs device number in edi, dest ip in eax

        test    edi, edi
        jz      .loopback

        call    ARP_IP_to_MAC
        test    eax, 0xffff0000         ; error bits
        jnz     .arp_error
        push    ebx                     ; push the mac onto the stack
        push    ax

        inc     [IP_packets_tx + edi]   ; update stats

        mov     ebx, [NET_DRV_LIST + edi]
        lea     eax, [ebx + ETH_DEVICE.mac]
        mov     edx, esp
        mov     ecx, [esp + 10 + 6]
        add     ecx, sizeof.IPv4_header
        mov     di, ETHER_PROTO_IPv4
        call    ETH_output
        jz      .eth_error
        add     esp, 6                  ; pop the mac out of the stack

  .continue:
        xchg    cl, ch                                  ; internet byte order
        mov     [edi + IPv4_header.VersionAndIHL], 0x45 ; IPv4, normal length (no Optional header)
        mov     [edi + IPv4_header.TypeOfService], 0    ; nothing special, just plain ip packet
        mov     [edi + IPv4_header.TotalLength], cx
        mov     [edi + IPv4_header.Identification], 0   ; fragment id: FIXME
        mov     [edi + IPv4_header.FlagsAndFragmentOffset], 0
        pop     word [edi + IPv4_header.TimeToLive]     ; ttl shl 8 + protocol
;               [edi + IPv4_header.Protocol]
        mov     [edi + IPv4_header.HeaderChecksum], 0
        popd    [edi + IPv4_header.SourceAddress]
        popd    [edi + IPv4_header.DestinationAddress]

        pop     ecx

        IPv4_checksum edi
        add     edi, sizeof.IPv4_header
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output: success!\n"
        ret

  .eth_error:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: ethernet error\n"
        add     esp, 3*4+2+6
        xor     edi, edi
        ret

  .arp_error:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: ARP error=%x\n", eax
        add     esp, 3*4+2
        xor     edi, edi
        ret

  .too_large:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output: Packet too large!\n"
        xor     edi, edi
        ret

  .loopback:
        mov     dword [esp + 2], eax            ; change source IP to dest IP
        mov     ecx, [esp + 10]
        add     ecx, sizeof.IPv4_header
        mov     edi, AF_INET4
        call    LOOP_output
        jmp     .continue




;------------------------------------------------------------------
;
; IPv4_output_raw
;
; IN: eax = socket ptr
;     ecx = data length
;     esi = data ptr
;
; OUT: /
;
;------------------------------------------------------------------
align 4
IPv4_output_raw:

        DEBUGF 1,"IPv4_output_raw: size=%u ptr=%x socket=%x\n", ecx, esi, eax

        cmp     ecx, 1480               ;;;;; FIXME
        ja      .too_large

        sub     esp, 8
        push    esi eax

        call    IPv4_route
        call    ARP_IP_to_MAC

        test    eax, 0xffff0000         ; error bits
        jnz     .arp_error

        push    ebx                     ; push the mac
        push    ax

        inc     [IP_packets_tx + 4*edi]
        mov     ebx, [NET_DRV_LIST + 4*edi]
        lea     eax, [ebx + ETH_DEVICE.mac]
        mov     edx, esp
        mov     ecx, [esp + 6 + 4]
        add     ecx, sizeof.IPv4_header
        mov     di, ETHER_PROTO_IPv4
        call    ETH_output
        jz      .error

        add     esp, 6  ; pop the mac

        mov     dword[esp+4+4], edx
        mov     dword[esp+4+4+4], eax

        pop     eax esi
;; todo: check socket options if we should add header, or just compute checksum

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

        IPv4_checksum edi                       ;;;; todo: checksum for IP packet with options!
        add     edi, sizeof.IPv4_header
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_output_raw: device=%x\n", ebx
        call    [ebx + NET_DEVICE.transmit]
        ret

  .error:
        add     esp, 6
  .arp_error:
        add     esp, 8+4+4
  .too_large:
        DEBUGF  DEBUG_NETWORK_ERROR, "IPv4_output_raw: Failed\n"
        sub     edi, edi
        ret


;--------------------------------------------------------
;
;
; IN: dword [esp] = pointer to buffer containing ipv4 packet to be fragmented
;     dword [esp+4] = buffer size
;     esi = pointer to ip header in that buffer
;     ecx = max size of fragments
;
; OUT: /
;
;--------------------------------------------------------

align 4
IPv4_fragment:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_fragment\n"

        and     ecx, not 111b   ; align 4

        cmp     ecx, sizeof.IPv4_header + 8     ; must be able to put at least 8 bytes
        jb      .err2

        push    esi ecx
        mov     eax, [esi + IPv4_header.DestinationAddress]
        call    ARP_IP_to_MAC
        pop     ecx esi
        cmp     eax, -1
        jz      .err2

        push    ebx
        push    ax

        mov     ebx, [NET_DRV_LIST]
        lea     eax, [ebx + ETH_DEVICE.mac]
        push    eax


        push    esi                             ; ptr to ip header
        sub     ecx, sizeof.IPv4_header         ; substract header size
        push    ecx                             ; max data size
        push    dword 0                         ; offset

  .new_fragment:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "Ipv4_fragment: new fragment"


        mov     eax, [esp + 3*4]
        lea     ebx, [esp + 4*4]
        mov     di , ETHER_PROTO_IPv4
        call    ETH_output

        cmp     edi, -1
        jz      .err

; copy header
        mov     esi, [esp + 2*4]
        mov     ecx, 5  ; 5 dwords: TODO: use IHL field of the header!
        rep movsd

; copy data
        mov     esi, [esp + 2*4]
        add     esi, sizeof.IPv4_header
        add     esi, [esp]      ; offset

        mov     ecx, [esp + 1*4]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_fragment: copying %u bytes\n", ecx
        rep movsb

; now, correct header
        mov     ecx, [esp + 1*4]
        add     ecx, sizeof.IPv4_header
        xchg    cl, ch
        mov     [edi + IPv4_header.TotalLength], cx

        mov     ecx, [esp]              ; offset
        xchg    cl, ch

;        cmp     dword[esp + 4*4], 0     ; last fragment?;<<<<<<
;        je      .last_fragment
        or      cx, 1 shl 2             ; more fragments
;  .last_fragment:
        mov     [edi + IPv4_header.FlagsAndFragmentOffset], cx

        mov     [edi + IPv4_header.HeaderChecksum], 0

        ;<<<<<<<<<<<<<<<<<<<<<<<<<<<<< send the packet
        mov     ecx, [esp + 1*4]

        push    edx eax
        IPv4_checksum edi

        call    [ebx + NET_DEVICE.transmit]
        ;<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        mov     ecx, [esp+4]
        add     [esp], ecx

        mov     ecx, [esp+3*4+6+4]      ; ptr to begin of buff
        add     ecx, [esp+3*4+6+4+4]    ; buff size
        sub     ecx, [esp+2*4]          ; ptr to ip header
        add     ecx, [esp]              ; offset

        DEBUGF  DEBUG_NETWORK_VERBOSE, "Ipv4_fragment: %u bytes remaining\n", ecx

        cmp     ecx, [esp+1*4]
        jae     .new_fragment

        mov     [esp+4], ecx            ; set fragment size to remaining packet size
        jmp     .new_fragment

      .err:
        DEBUGF  DEBUG_NETWORK_ERROR, "Ipv4_fragment: failed\n"
      .done:
        add     esp, 12 + 4 + 6
      .err2:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "Ipv4_fragment: dumping\n"
        call    NET_packet_free
        add     esp, 4

        ret



;---------------------------------------------------------------------------
;
; IPv4_route
;
; IN:   eax = Destination IP
; OUT:  edi = device number*4
;       eax = ip of gateway if nescessary, unchanged otherwise
;
;---------------------------------------------------------------------------
align 4
IPv4_route:

        cmp     eax, 0xffffffff
        je      .broadcast

        xor     edi, edi
        mov     ecx, NET_DEVICES_MAX
  .loop:
        mov     ebx, [IP_LIST+edi]
        and     ebx, [SUBNET_LIST+edi]
        jz      .next
        mov     edx, eax
        and     edx, [SUBNET_LIST+edi]

        cmp     ebx, edx
        jne     .next

        DEBUGF  DEBUG_NETWORK_VERBOSE, "IPv4_route: %u\n", edi
        ret

  .next:
        add     edi, 4
        dec     ecx
        jnz     .loop

  .invalid:
        mov     eax, [GATEWAY_LIST+4]       ;;; FIXME
  .broadcast:
        mov     edi, 4                  ; if none found, use device 1 as default     ;;;; FIXME
        ret



;---------------------------------------------------------------------------
;
; IPv4_get_frgmnt_num
;
; IN: /
; OUT: fragment number in ax
;
;---------------------------------------------------------------------------
align 4
IPv4_get_frgmnt_num:
        xor     ax, ax  ;;; TODO: replace this with real code

        ret


;-----------------------------------------------------------------
;
; IPv4_connect
;
;   IN: eax = socket pointer
;  OUT: eax = 0 ok / -1 error
;       ebx = error code
;
;-------------------------
align 4
IPv4_connect:

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

; Fill in local IP
        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST + 4]                                   ; FIXME: use correct local IP
        pop     [eax + IP_SOCKET.LocalIP]

; Fill in remote IP
        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)                ; Set up data receiving queue
        pop     eax

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        xor     eax, eax
        ret


;---------------------------------------------------------------------------
;
; IPv4_API
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
IPv4_api:

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
        mov     eax, [IP_packets_tx + eax]
        ret

  .packets_rx:
        mov     eax, [IP_packets_rx + eax]
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
        call    ARP_output_request              ; now send a gratuitous ARP

        call    NET_send_event
        xor     eax, eax
        ret

  .read_dns:
        mov     eax, [DNS_LIST + eax]
        ret

  .write_dns:
        mov     [DNS_LIST + eax], ecx
        call    NET_send_event
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

        call    NET_send_event
        xor     eax, eax
        ret

  .read_gateway:
        mov     eax, [GATEWAY_LIST + eax]
        ret

  .write_gateway:
        mov     [GATEWAY_LIST + eax], ecx

        call    NET_send_event
        xor     eax, eax
        ret