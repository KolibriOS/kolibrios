;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2009-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Clevermouse & hidnplayr                                      ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


struct  PPPoE_header

        VersionAndType  db ?
        Code            db ?
        SessionID       dw ?
        Length          dw ?            ; Length of payload, does NOT include the length PPPoE header.

ends

struct  PPPoE_connection

        next            dd ?            ; pointer to next connection
        prev            dd ?            ; pointer to previous connection

        pid             dd ?            ; identifier of base application

        datalen         dd ?            ; length of received data
        recvbuf         rb 1500         ; buffer for received data
        sendbuf         rb 1500         ; buffer for data to send

ends

iglobal
align 4
        PPPoE.head              dd PPPoE.head
        PPPoE.tail              dd PPPoE.head
endg

uglobal
        PPPoE.cur_receiver      dd ?
        PPPoE.cur_receiver_ptr  dd ?
        PPPoE.cur_receiver_len  dd ?
endg


; Allocates internal structure for future PPPoE actions.
align 4
PPPoE_alloc_connection:

; 1. Allocate memory in the kernel area.
        stdcall kernel_alloc, sizeof.PPPoE_connection

; 1a. If memory allocation failed, return NULL.
        test    eax, eax
        jz      .nothing

; 2. Copy PID of caller to the structure.
        mov     edx, [CURRENT_TASK]
        mov     [eax + PPPoE_connection.pid], edx

; 3. Insert the structure to the list of all connections.
        mov     [eax + PPPoE_connection.next], PPPoE.head
        mov     edx, [PPPoE.tail]
        mov     [eax + PPPoE_connection.prev], edx
        mov     [edx + PPPoE_connection.next], eax
        mov     [PPPoE.tail], eax

  .nothing:
        ret


align 4
PPPoE_free_connection:

; 1. Check that the caller is the owner of this connection.
        mov     eax, [CURRENT_TASK]
        cmp     [ebx+PPPoE_connection.pid], eax
        jnz     .nothing

; 2. Delete the structure from the list of all connections.
        mov     eax, [ebx+PPPoE_connection.next]
        mov     edx, [ebx+PPPoE_connection.prev]
        mov     [eax+PPPoE_connection.prev], edx
        mov     [edx+PPPoE_connection.next], eax

; 3. Free the memory.
        stdcall kernel_free, ebx

  .nothing:
        ret


; Send PADI packet

; ebx (ecx in app) = size of buffer for PPPoE offers, must be at least 1514
; ecx (edx in app) = size of tags, 0 means "use default"
; edx (esi in app) = pointer to buffer for PPPoE offers
; esi (edi in app) = pointer to tags, ignored if 'size of tags' == 0
align 4
PPPoE_send_init:

; 1. Check length.
        cmp     ebi, 1514
        jb      .bad

; RFC2516: An entire PADI packet (including the PPPoE header) MUST NOT
; exceed 1484 octets.
; PPPoE header is 6 bytes long, so maximum length of tags is 1478.
        cmp     ecx, 1478
        ja      .bad

; 2. Check that no one listen for offers.
        cmp     [PPPoE.cur_receiver], 0
        jnz     .bad

; 3. Remember PID and data pointer of current listener.
        push    [CURRENT_TASK]
        pop     [PPPoE.cur_receiver]
        mov     [PPPoE.cur_receiver_ptr], edx
        mov     [PPPoE.cur_receiver_len], ebx
        and     dword [edx], 0 ; no offers yet

; 4. Create packet.
        test    ecx, ecx
        jnz     @f
        mov     esi, .default_payload
        mov     ecx, .default_payload_length
       @@:

        mov     edx, [NET_DRV_LIST]     ;;;; FIXME
        lea     eax, [ebx + ETH_DEVICE.mac]     ; Source Address
        mov     edx, ETH_BROADCAST              ; Destination Address
        add     ecx, sizeof.PPPoE_header        ; Data size
        mov     di, ETHER_PPP_DISCOVERY         ; Protocol
        call    ETH_output
        jz      .eth_error

        push    edx eax

; 4b. Set ver=1, type=1 (=> first byte 0x11), code=9 (PADI packet), session=0
        mov     dword [edi], (0x09 shl 8) + 0x11

; 4c. Set payload length.
        mov     [edi+4], ch
        mov     [edi+5], cl

; 4e. Copy given tags.
        rep     movsb

; 5. Send packet.
        call    [ebx + NET_DEVICE.transmit]
; 6. Return.
        xor     eax, eax
        ret

  .bad:
        or      eax, -1
        ret

  .default_payload:
; Service-Name tag with zero length
        dw      0x0101, 0x0000
  .default_payload_length = $ - .default_payload


; Stop receiving PADO packets
align 4
PPPoE_stop_offers:

; Only the listener can stop listen.    ;;; TODO: make sure this function is called when process gets terminated
        mov     eax, [CURRENT_TASK]
        cmp     [PPPoE.cur_receiver], eax
        jnz     .bad
        xor     eax, eax
        mov     [PPPoE.cur_receiver_ptr], eax
        mov     [PPPoE.cur_receiver], eax
        ret

  .bad:
        or      eax, -1
        ret

; Send PPPoE data in Discovery stage
align 4
PPPoE_send_discovery:
        ret

; Receive PPPoE data in Discovery stage
align 4
PPPoE_receive_discovery:
        ret



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

; 1. Minimum 6 bytes for PPPoE header.
        cmp     ecx, sizeof.PPPoE_header
        jb      .bad

; 1. Ignore packets with ver<>1 and/or type<>1.
        cmp     [edx + PPPoE_header.VersionAndType], 0x11
        jnz     .bad

; 2. Code must be either 7 for Offer,
; or 0x65 for Session-Confirmation, or 0xa7 for Terminate.
; Because only Initiation/Offers are supported, we expect only value 7.
        cmp     [edx + PPPoE_header.Code], 7
        jnz     .bad

; 3. Session ID must be zero for Offers.
        cmp     [edx + PPPoE_header.SessionID], 0
        jnz     .bad

; 4. Payload length
        rol     [edx + PPPoE_header.Length], 8          ; Convert INET byte order to intel

; 5. Ignore packet if nobody is listening.
        cmp     [PPPoE.cur_receiver], 0
        jz      .bad

; 6. Good, now copy the received packet to the buffer of listener.

        ;;; TODO

  .bad:
        DEBUGF 1,'K : PPPoE - dumped\n'
        call    kernel_free
        add     esp, 4                                  ; pop (balance stack)
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
        dd      PPPoE_send_init         ; 0
        dd      PPPoE_stop_offers       ; 1
        dd      PPPoE_alloc_connection  ; 3
        dd      PPPoE_free_connection   ; 4
        dd      PPPoE_send_discovery    ; 5
        dd      PPPoE_receive_discovery ; 6
  .number = ($ - .table) / 4 - 1

  .error:
        mov     eax, -1
        ret
