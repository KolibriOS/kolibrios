;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  IPv4.INC                                                       ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;  Based on the work of [Johnny_B] and [smb]                      ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

MAX_FRAGMENTS	equ 64
MAX_IP		equ MAX_NET_DEVICES
IP_MAX_INTERFACES   equ MAX_IP

struct	IPv4_Packet
	.VersionAndIHL		db  ?  ; Version[0-3 bits] and IHL(header length)[4-7 bits]
	.TypeOfService		db  ?  ; precedence [7-5] minimize delay [4], maximize throughput [3], maximize riliability [2] minimize momentary cost [1] and zero [0]
	.TotalLength		dw  ?
	.Identification 	dw  ?
	.FlagsAndFragmentOffset dw  ?  ; Flags[0-2] and FragmentOffset[3-15]
	.TimeToLive		db  ?  ;
	.Protocol		db  ?
	.HeaderChecksum 	dw  ?
	.SourceAddress		dd  ?
	.DestinationAddress	dd  ?
	.DataOrOptional:
ends

struct	FRAGMENT_slot
	.ttl			dw  ?  ; Time to live for this entry, 0 for empty slot's
	.id			dw  ?  ; Identification field from IP header
	.SrcIP			dd  ?  ; .. from IP header
	.DstIP			dd  ?  ; .. from IP header
	.ptr			dd  ?  ; Pointer to first packet
	.size:
ends

struct	FRAGMENT_entry		       ; This structure will replace the ethernet header in fragmented ip packets
	.PrevPtr		dd  ?  ; Pointer to previous fragment entry  (-1 for first packet)
	.NextPtr		dd  ?  ; Pointer to next fragment entry (-1 for last packet)
	.Owner			dd  ?  ; Pointer to structure of driver
				rb  2  ; to match ethernet header size          ; TODO: fix this hack
	.Data:			       ; Ip header begins here (we will need the IP header to re-construct the complete packet)
ends

align 4
uglobal

	IP_LIST 	rd  MAX_IP
	SUBNET_LIST	rd  MAX_IP
	DNS_LIST	rd  MAX_IP
	GATEWAY_LIST	rd  MAX_IP

	IP_PACKETS_TX	rd  MAX_IP
	IP_PACKETS_RX	rd  MAX_IP

	FRAGMENT_LIST	rb  MAX_FRAGMENTS*FRAGMENT_slot.size
endg


;-----------------------------------------------------------------
;
; IPv4_init
;
;  This function resets all IP variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
IPv4_init:

	or	eax, -1
	mov	edi, IP_LIST
	mov	ecx, 4*MAX_IP
	rep	stosd

	inc	eax
	mov	edi, FRAGMENT_LIST
	mov	ecx, FRAGMENT_slot.size*MAX_FRAGMENTS/4 + 2*MAX_IP
	rep	stosd

	ret



;-----------------------------------------------------------------
;
; IPv4_Handler:
;
;  Will check if IP Packet isnt damaged
;  and call appropriate handler. (TCP/UDP/ICMP/..)
;
;  It will also re-construct fragmented packets
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;       pointer to IP Packet data in edx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
IPv4_handler:	 ; TODO: implement handler for IP options
		 ; TODO2: add code for raw sockets

	DEBUGF	1,"IPv4_Handler, packet from: %u.%u.%u.%u ",\
	[edx + IPv4_Packet.SourceAddress]:1,[edx + IPv4_Packet.SourceAddress + 1]:1,[edx + IPv4_Packet.SourceAddress + 2]:1,[edx + IPv4_Packet.SourceAddress + 3]:1
	DEBUGF	1,"to: %u.%u.%u.%u\n",\
	[edx + IPv4_Packet.DestinationAddress]:1,[edx + IPv4_Packet.DestinationAddress + 1]:1,[edx + IPv4_Packet.DestinationAddress + 2]:1,[edx + IPv4_Packet.DestinationAddress + 3]:1

;-------------------------------------------
; Check if the packet still has time to live

	cmp	byte [edx + IPv4_Packet.TimeToLive], 0
	je	.dump

;--------------------------------------
; First, check if IP packet has options

	movzx	eax, [edx + IPv4_Packet.VersionAndIHL]
	and	al , 0x0f					; get IHL(header length)
	cmp	al , 0x05					; IHL!= 5*4(20 bytes)
	jnz	.has_options

;-------------------------------
; Now, re-calculate the checksum

	push	edx ebx
	mov	esi, edx
	call	IPv4_checksum
	pop	ebx edx

	cmp	[edx + IPv4_Packet.HeaderChecksum], 0
	jne	.dump						; if checksum isn't valid then dump packet

	DEBUGF	1,"IPv4 Checksum is correct\n"

;-----------------------------------
; Check if destination IP is correct

	call	NET_ptr_to_num
	shl	edi, 2

	; check if it matches local ip

	mov	eax, dword[IP_LIST+edi]
	cmp	[edx + IPv4_Packet.DestinationAddress], eax
	je	.ip_ok

	; check for broadcast

	mov	eax, dword[SUBNET_LIST+edi]
	not	eax
	or	eax, dword[IP_LIST+edi]
	cmp	[edx + IPv4_Packet.DestinationAddress], eax
	je	.ip_ok

	; or a special broadcast

	cmp	[edx + IPv4_Packet.DestinationAddress], -1
	je	.ip_ok

;        ; maybe it's a multicast then
;
;        mov     eax, [edx + IPv4_Packet.DestinationAddress]
;        and     eax, 0xff000000
;        cmp     eax, 224 shl 24
;        je      .ip_ok

	; or a loopback address

	cmp	eax, 127 shl 24
	je	.ip_ok

	; or it's not meant for us..

	DEBUGF	2,"Destination address does not match!\n"
	jmp	.dump

;------------------------
; Now we can update stats

  .ip_ok:
	inc	[IP_PACKETS_RX+edi]

;----------------------------------
; Check if the packet is fragmented

	test	[edx + IPv4_Packet.FlagsAndFragmentOffset], 1 shl 5	; Is 'more fragments' flag set ?
	jnz	.has_fragments						; If so, we definately have a fragmented packet

	test	[edx + IPv4_Packet.FlagsAndFragmentOffset], 0xff1f	; If flag is not set, but there is a fragment offset, the packet is last in series of fragmented packets
	jnz	.is_last_fragment

;-------------------------------------------------------------------
; No, it's just a regular IP packet, pass it to the higher protocols

  .handle_it:							; We reach here if packet hasnt been fragmented, or when it already has been re-constructed
	movzx	eax, byte [edx + IPv4_Packet.VersionAndIHL]	; Calculate Header length by using IHL field
	and	eax, 0x0000000f 				;
	shl	eax, 2						;
	movzx	ecx, word [edx + IPv4_Packet.TotalLength]	; Calculate length of encapsulated Packet
	xchg	cl , ch 					;
	sub	ecx, eax					;

	add	eax, edx
	push	eax

	mov	esi, [edx + IPv4_Packet.SourceAddress]		; These values might be of interest to the higher protocols
	mov	edi, [edx + IPv4_Packet.DestinationAddress]	;
	mov	al , [edx + IPv4_Packet.Protocol]
	pop	edx						; Offset to data (tcp/udp/icmp/.. Packet)

	cmp	al , IP_PROTO_TCP
	je	TCP_input

	cmp	al , IP_PROTO_UDP
	je	UDP_input

	cmp	al , IP_PROTO_ICMP
	je	ICMP_input

	DEBUGF	2,"unknown Internet protocol: %u\n", al

  .dump:
	DEBUGF	2,"IP_Handler - dumping\n"
;        inc     [dumped_rx_count]
	call	kernel_free
	add	esp, 4						; pop (balance stack)
	ret


;---------------------------
; Fragmented packet handler


  .has_fragments:
	movzx	eax, [edx + IPv4_Packet.FlagsAndFragmentOffset]
	xchg	al , ah
	shl	ax , 3

	DEBUGF	1,"Fragmented packet, offset:%u, id:%x\n", ax, [edx + IPv4_Packet.Identification]:4

	test	ax , ax 					; Is this the first packet of the fragment?
	jz	.is_first_fragment


;-------------------------------------------------------
; We have a fragmented IP packet, but it's not the first

	DEBUGF	1,"Middle fragmented packet received!\n"

	call	IPv4_find_fragment_slot
	cmp	esi, -1
	je	.dump

	mov	word [esi + FRAGMENT_slot.ttl], 15		; Reset the ttl
	mov	esi, [esi + FRAGMENT_slot.ptr]
	or	edi, -1
  .find_last_entry:						; The following routine will try to find the last entry
	cmp	edi, [esi + FRAGMENT_entry.PrevPtr]
	jne	.destroy_slot					; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
	mov	edi, esi
	mov	esi, [esi + FRAGMENT_entry.NextPtr]
	cmp	esi, -1
	jne	.find_last_entry
								; We found the last entry (pointer is now in edi)
								; We are going to overwrite the ethernet header in received packet with a FRAGMENT_entry structure

	pop	eax						; pointer to packet
	mov	[edi + FRAGMENT_entry.NextPtr], eax		; update pointer of previous entry to the new entry
	mov	[eax + FRAGMENT_entry.NextPtr], -1
	mov	[eax + FRAGMENT_entry.PrevPtr], edi
	mov	[eax + FRAGMENT_entry.Owner], ebx

	add	esp, 4
	ret


;------------------------------------
; We have received the first fragment

  .is_first_fragment:
	DEBUGF	1,"First fragmented packet received!\n"
								; try to locate a free slot..
	mov	ecx, MAX_FRAGMENTS
	mov	esi, FRAGMENT_LIST
  .find_free_slot:
	cmp	word [esi + FRAGMENT_slot.ttl], 0
	je	.found_free_slot
	add	esi, FRAGMENT_slot.size
	loop	.find_free_slot
	jmp	.dump						; If no free slot was found, dump the packet

  .found_free_slot:						; We found a free slot, let's fill in the FRAGMENT_slot structure
	mov	word [esi + FRAGMENT_slot.ttl], 15		; RFC recommends 15 secs as ttl
	mov	ax , word [edx + IPv4_Packet.Identification]
	mov	word [esi + FRAGMENT_slot.id], ax
	mov	eax, dword [edx + IPv4_Packet.SourceAddress]
	mov	dword [esi + FRAGMENT_slot.SrcIP], eax
	mov	eax, dword [edx + IPv4_Packet.DestinationAddress]
	mov	dword [esi + FRAGMENT_slot.DstIP], eax
	pop	eax
	mov	dword [esi + FRAGMENT_slot.ptr], eax
								; Now, replace ethernet header in original buffer with a FRAGMENT_entry structure
	mov	[eax + FRAGMENT_entry.NextPtr], -1
	mov	[eax + FRAGMENT_entry.PrevPtr], -1
	mov	[eax + FRAGMENT_entry.Owner], ebx

	add	esp, 4						; balance stack and exit
	ret


;-----------------------------------
; We have received the last fragment

  .is_last_fragment:
	DEBUGF	1,"Last fragmented packet received!\n"

	call	IPv4_find_fragment_slot
	cmp	esi, -1
	je	.dump

	mov	esi, [esi + FRAGMENT_slot.ptr]			; We found the first entry, let's calculate total size of the packet in eax, so we can allocate a buffer
	push	esi
	xor	eax, eax
	or	edi, -1

  .count_bytes:
	cmp	[esi + FRAGMENT_entry.PrevPtr], edi
	jne	.destroy_slot_pop						  ; Damn, something screwed up, remove the whole slot (and free buffers too if possible!)
	mov	cx, word [esi + FRAGMENT_entry.Data + IPv4_Packet.TotalLength]	  ; Add total length
	xchg	cl, ch
	DEBUGF	1,"Packet size: %u\n", cx
	add	ax, cx
	movzx	cx, byte [esi + FRAGMENT_entry.Data + IPv4_Packet.VersionAndIHL]  ; Sub Header length
	and	cx, 0x000F
	shl	cx, 2
	DEBUGF	1,"Header size: %u\n", cx
	sub	ax, cx
	mov	edi, esi
	mov	esi, [esi + FRAGMENT_entry.NextPtr]
	cmp	esi, -1
	jne	.count_bytes

	mov	esi, [esp+4]
	mov	[edi + FRAGMENT_entry.NextPtr], esi			       ; Add this packet to the chain, this simplifies the following code
	mov	[esi + FRAGMENT_entry.NextPtr], -1
	mov	[esi + FRAGMENT_entry.PrevPtr], edi
	mov	[esi + FRAGMENT_entry.Owner], ebx

	mov	cx, [edx + IPv4_Packet.TotalLength]			       ; Note: This time we dont substract Header length
	xchg	cl , ch
	DEBUGF	1,"Packet size: %u\n", cx
	add	ax , cx
	DEBUGF	1,"Total Received data size: %u\n", eax

	push	eax
	mov	ax , [edx + IPv4_Packet.FlagsAndFragmentOffset]
	xchg	al , ah
	shl	ax , 3
	add	cx , ax
	pop	eax
	DEBUGF	1,"Total Fragment size: %u\n", ecx

	cmp	ax, cx
	jne	.destroy_slot_pop

	push	eax
	push	eax
	call	kernel_alloc
	test	eax, eax
	je	.destroy_slot_pop							; If we dont have enough space to allocate the buffer, discard all packets in slot
	mov	edx, [esp+4]								; Get pointer to first fragment entry back in edx

  .rebuild_packet_loop:
	movzx	ecx, word [edx + FRAGMENT_entry.Data + IPv4_Packet.FlagsAndFragmentOffset]	; Calculate the fragment offset
	xchg	cl , ch 									;  intel byte order
	shl	cx , 3										;   multiply by 8 and clear first 3 bits
	DEBUGF	1,"Fragment offset: %u\n", cx

	lea	edi, [eax + ecx]								; Notice that edi will be equal to eax for first fragment
	movzx	ebx, byte [edx + FRAGMENT_entry.Data + IPv4_Packet.VersionAndIHL]		; Find header size (in ebx) of fragment
	and	bx , 0x000F									;
	shl	bx , 2										;

	lea	esi, [edx + FRAGMENT_entry.Data]						; Set esi to the correct begin of fragment
	movzx	ecx, word [edx + FRAGMENT_entry.Data + IPv4_Packet.TotalLength] 		; Calculate total length of fragment
	xchg	cl, ch										;  intel byte order

	cmp	edi, eax									; Is this packet the first fragment ?
	je	.first_fragment
	sub	cx, bx										; If not, dont copy the header
	add	esi, ebx									;
  .first_fragment:

	push	cx										; First copy dword-wise, then byte-wise
	shr	cx, 2										;
	rep	movsd										;
	pop	cx										;
	and	cx, 3										;
	rep	movsb										;

	push	eax
	push	edx										; Push pointer to fragment onto stack
	mov	ebx, [edx + FRAGMENT_entry.Owner]						; we need to remeber the owner, in case this is the last packet
	mov	edx, [edx + FRAGMENT_entry.NextPtr]						; Set edx to the next pointer
	call	kernel_free									; free the previous fragment buffer (this uses the value from stack)
	pop	eax
	cmp	edx, -1 									; Check if it is last fragment in chain
	jne	.rebuild_packet_loop

	pop	ecx										;
	xchg	cl, ch
	mov	edx, eax
	mov	word [edx + IPv4_Packet.TotalLength], cx
	add	esp, 8

	xchg	cl, ch		    ;

	push	ecx		   ;;;;
	push	eax		   ;;;;

;        mov     esi, edx           ;  This prints the IP packet to the debug board (usefull when using serial output debug..)
;                                   ;
;       @@:                         ;
;        lodsb                      ;
;        DEBUGF  1,"%x ", eax:2     ;
;        loop    @r                 ;

	jmp	.handle_it	    ; edx = buf ptr, ecx = size, [esp] buf ptr, [esp+4], total size, ebx=device ptr

  .destroy_slot_pop:
	add	esp, 4
  .destroy_slot:
	DEBUGF	1,"Destroy fragment slot!\n"
	; TODO!
	jmp	.dump




;-----------------------------------
; The IP packet has some options

  .has_options:
	jmp	.dump






;-----------------------------------------------------------------
;
; find fragment slot
;
; IN: pointer to fragmented packet in edx
; OUT: pointer to slot in edi, -1 on error
;
;-----------------------------------------------------------------
align 4
IPv4_find_fragment_slot:

;;; TODO: the RFC says we should check protocol number too

	push	eax ebx ecx edx
	mov	ax , word [edx + IPv4_Packet.Identification]
	mov	ecx, MAX_FRAGMENTS
	mov	esi, FRAGMENT_LIST
	mov	ebx, dword [edx + IPv4_Packet.SourceAddress]
	mov	edx, dword [edx + IPv4_Packet.DestinationAddress]
  .find_slot:
	cmp	word [esi + FRAGMENT_slot.id], ax
	jne	.try_next
	cmp	dword [esi + FRAGMENT_slot.SrcIP], ebx
	jne	.try_next
	cmp	dword [esi + FRAGMENT_slot.DstIP], edx
	je	.found_slot
  .try_next:
	add	esi, FRAGMENT_slot.size
	loop	.find_slot
;        pop     edx ebx
	or	esi, -1
;        ret

  .found_slot:
	pop	edx ecx ebx eax
	ret


;-----------------------------------------------------------------
;
; Decrease TimeToLive of all fragment slots
;
; IN: /
; OUT: /
;
;-----------------------------------------------------------------
align 4
IPv4_decrease_fragment_ttls:

	mov	esi, FRAGMENT_LIST
	mov	ecx, MAX_FRAGMENTS
  .loop:
	cmp	[esi + FRAGMENT_slot.ttl], 0
	je	.try_next
	dec	[esi + FRAGMENT_slot.ttl]
	jnz	.try_next
	DEBUGF 1,"Fragment slot timed-out!\n"
;;; TODO: clear all entry's of timed-out slot
  .try_next:
	add	esi, 4
	loop	.loop
	ret





;------------------------------------------------------------------
;
;
; IN: dword [esp] = pointer to packet to be fragmented
;     dword [esp+4] = buffer size
;     edx = pointer to IPv4 header in that packet
;     ecx = data length
;     ebx = device structure
;
; OUT: /
;
;------------------------------------------------------------------
align 4
IPv4_fragment:

;;; TODO: write code here


	call	kernel_free
	add	esp, 4

	ret




;------------------------------------------------------------------
;
; Create_IPv4_Packet
;
; IN: eax = dest ip
;     ebx = source ip
;     ecx = data length
;     dx  = fragment id  ;;;;
;     di  = protocol
;
; OUT: eax = pointer to buffer start
;      ebx = pointer to device struct (needed for sending procedure)
;      ecx = unchanged (packet size of embedded data)
;      edx = size of complete buffer
;      edi = pointer to start of data (0 on error)
;
;------------------------------------------------------------------
align 4
IPv4_create_packet:

	DEBUGF 1,"Create IPv4 Packet (size=%u)\n", ecx

	cmp	ecx, 65500	      ; Max IPv4 packet size
	jg	.exit_

	test	ebx, ebx	      ; if source ip = 0
	jnz	.ip_ok		      ; and local ip is valid
				      ; use local ip instead
	cmp	[IP_LIST],0xffffffff  ;
	je	.ip_ok		      ; TODO: find solution to send broadcast
				      ;  on device other then device 0
	mov	ebx, [IP_LIST]	      ;
				      ;
  .ip_ok:			      ;

	push	ecx eax ebx dx di

	cmp	eax, -1
	je	.broadcast		; If it is broadcast, just send

	call	ARP_IP_to_MAC

	cmp	eax, -1
	je	.not_found

	push	ebx
	push	ax

	jmp	.send

  .broadcast:
	push	word -1
	push	dword -1

  .send:
	call	IPv4_dest_to_dev
	inc	[IP_PACKETS_TX+4*edi]
	mov	edx, [NET_DRV_LIST + 4*edi]
	lea	eax, [edx + ETH_DEVICE.mac]
	mov	ebx, esp
	mov	ecx, [esp+18]	 ;; 18 or  22 ??
	add	ecx, IPv4_Packet.DataOrOptional
	mov	di , ETHER_IPv4
	;;; TODO: detect if packet is too large for ethernet, if so, call IPv4_fragment
	call	ETH_create_packet		   ;;; TODO: figure out a way to make this work with other protocols too
	add	esp, 6
	test	edi, edi
	jz	.exit

	mov	[edi + IPv4_Packet.VersionAndIHL], 0x45   ; IPv4, normal length (no Optional header)
	mov	[edi + IPv4_Packet.TypeOfService], 0
	xchg	ch, cl
	mov	[edi + IPv4_Packet.TotalLength], cx
	mov	[edi + IPv4_Packet.FlagsAndFragmentOffset], 0x0000
	mov	[edi + IPv4_Packet.TimeToLive], 128
	mov	[edi + IPv4_Packet.HeaderChecksum], 0
	pop	cx
	mov	[edi + IPv4_Packet.Protocol], cl
	pop	cx
	mov	[edi + IPv4_Packet.Identification], cx
	pop	ecx
	mov	[edi + IPv4_Packet.SourceAddress], ecx
	pop	ecx
	mov	[edi + IPv4_Packet.DestinationAddress], ecx

	push	eax edx esi
	mov	esi, edi
	call	IPv4_checksum
	pop	esi edx eax ecx
	add	edi, IPv4_Packet.DataOrOptional

	DEBUGF	1,"IPv4 Packet for device %x created successfully\n", ebx

	ret


  .not_found:
	DEBUGF	1,"Create IPv4 Packet - ARP entry not found!\n"
	;;;;;;
  .exit:
	add	esp, 16
  .exit_:
	DEBUGF	1,"Create IPv4 Packet - failed\n"
	and	edi, 0
	ret



align 4
IPv4_checksum:

; This is the fast procedure to create or check a IP header without options
;
; To create a new checksum, the checksum field must be set to 0 before computation
;
; To check an existing checksum, leave the checksum as is, and it will be 0 after this procedure, if it was correct

	xor	edx, edx

	add	dl, [esi+1]
	adc	dh, [esi+0]

	adc	dl, [esi+3]
	adc	dh, [esi+2]

	adc	dl, [esi+5]
	adc	dh, [esi+4]

	adc	dl, [esi+7]
	adc	dh, [esi+6]

	adc	dl, [esi+9]
	adc	dh, [esi+8]

; we skip 11th and 12th byte, they are the checksum bytes and should be 0 for re-calculation

	adc	dl, [esi+13]
	adc	dh, [esi+12]

	adc	dl, [esi+15]
	adc	dh, [esi+14]

	adc	dl, [esi+17]
	adc	dh, [esi+16]

	adc	dl, [esi+19]
	adc	dh, [esi+18]

	adc	edx, 0

	call	checksum_2

	neg	word [esi+10]		; zero will stay zero so we just get the checksum
	add	word [esi+10], dx	;  , else we will get (new checksum - old checksum) in the end, wich should be 0 :)

	ret





;---------------------------------------------------------------------------
;
; IPv4_dest_to_dev
;
; IN: Destination IP in eax
; OUT: device id in edi
;
;---------------------------------------------------------------------------
align 4
IPv4_dest_to_dev:

	DEBUGF 1,"IPv4 destination to device: "

	xor	edi, edi
	mov	ecx, MAX_IP

  .loop:
	mov	ebx, [IP_LIST+edi]		; we dont need to worry about non exisiting ip interfaces
	and	ebx, [SUBNET_LIST+edi]		; they have IP and SUBNET set to all one's, so they will have no match except 255.255.255.255
						; (only a moron would insert that ip into this function..)
	mov	edx, eax
	and	edx, [SUBNET_LIST+edi]

	cmp	ebx, edx
	je	.found_it

	add	edi, 4
	loop	.loop

	xor	edi, edi	; if none found, use device 0 as default device

  .found_it:
	shr	edi, 2

	DEBUGF	1,"%u\n",edi

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
	xor	ax, ax	;;; TODO: replace this with real code

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
IPv4_API:

	movzx	eax, bh
	shl	eax, 2

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1
	dec	bl
	jz	.read_ip	; 2
	dec	bl
	jz	.write_ip	; 3
	dec	bl
	jz	.read_dns	; 4
	dec	bl
	jz	.write_dns	; 5
	dec	bl
	jz	.read_subnet	; 6
	dec	bl
	jz	.write_subnet	; 7
	dec	bl
	jz	.read_gateway	; 8
	dec	bl
	jz	.write_gateway	; 9

  .error:
	mov	eax, -1
	ret

  .packets_tx:
	add	eax, IP_PACKETS_TX
	mov	eax, [eax]
	ret

  .packets_rx:
	add	eax, IP_PACKETS_RX
	mov	eax, [eax]
	ret

  .read_ip:
	add	eax, IP_LIST
	mov	eax, [eax]
	ret

  .write_ip:
	add	eax, IP_LIST
	mov	[eax], ecx
	xor	eax, eax
	ret

  .read_dns:
	add	eax, DNS_LIST
	mov	eax, [eax]
	ret

  .write_dns:
	add	eax, DNS_LIST
	mov	[eax], ecx
	xor	eax, eax
	ret

  .read_subnet:
	add	eax, SUBNET_LIST
	mov	eax, [eax]
	ret

  .write_subnet:
	add	eax, SUBNET_LIST
	mov	[eax], ecx
	xor	eax, eax
	ret

  .read_gateway:
	add	eax, GATEWAY_LIST
	mov	eax, [eax]
	ret

  .write_gateway:
	add	eax, GATEWAY_LIST
	mov	[eax], ecx
	xor	eax, eax
	ret




