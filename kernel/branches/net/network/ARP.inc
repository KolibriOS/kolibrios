;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  ARP.INC                                                        ;;
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


$Revision: 983 $


ARP_NO_ENTRY		equ  0
ARP_VALID_MAPPING	equ  1
ARP_AWAITING_RESPONSE	equ  2
ARP_RESPONSE_TIMEOUT	equ  3

ARP_REQUEST_TTL 	= 20 ; in seconds
ARP_ENTRY_TTL		= 30 ; in seconds

ETHER_ARP		equ  0x0608

ARP_REQ_OPCODE		equ  0x0100  ; request
ARP_REP_OPCODE		equ  0x0200  ; reply

ARP_TABLE_SIZE		equ  20      ; Size of table

struct ARP_ENTRY
       .IP		dd  ?
       .MAC		dp  ?
       .Status		dw  ?
       .TTL		dw  ?  ; in seconds
       .size:
ends

struct ARP_Packet
       .HardwareType	dw  ?
       .ProtocolType	dw  ?
       .HardwareSize	db  ?
       .ProtocolSize	db  ?
       .Opcode		dw  ?
       .SenderMAC	dp  ?
       .SenderIP	dd  ?
       .TargetMAC	dp  ?
       .TargetIP	dd  ?
ends


; The TTL field is decremented every second, and is deleted when it
; reaches 0. It is refreshed every time a packet is received
; If the TTL field is 0xFFFF it is a static entry and is never deleted
; The status field can be the following values:
; 0x0000  entry not used
; 0x0001  entry holds a valid mapping
; 0x0002  entry contains an IP address, awaiting ARP response
; 0x0003  No response received to ARP request.
; The last status value is provided to allow the network layer to delete
; a packet that is queued awaiting an ARP response

align 4
uglobal

	NumARP		dd ?
	ARPTable	rb ARP_ENTRY.size * ARP_TABLE_SIZE

	ARP_PACKETS_TX	rd  MAX_NET_DEVICES
	ARP_PACKETS_RX	rd  MAX_NET_DEVICES


endg



;-----------------------------------------------------------------
;
; ARP_init
;
;  This function resets all ARP variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------

align 4
ARP_init:

	xor	eax, eax

	mov	[NumARP], eax

	mov	edi, ARP_PACKETS_TX
	mov	ecx, 2*MAX_NET_DEVICES
	rep	stosd

	ret


;-----------------------------------------------------------------
;
; ARP_IP_to_MAC
;
;  This function resets all ARP variables
;
;  IN: eax = IPv4 address
;  OUT: eax = -1 on error, else eax = first two bytes of mac
;                   ( high 16 bits are zero)
;       ebx = last four bytes of mac                                  ; TODO: special eax value for 'request send'
;
;-----------------------------------------------------------------

align 4
ARP_IP_to_MAC:

	DEBUGF 1,"ARP_IP_to_MAC\n"

    ; first, check destination IP to see if it is on 'this' network.
    ; The test is:
    ; if ( destIP & subnet_mask == stack_ip & subnet_mask )
    ;   destination is local
    ; else
    ;  destination is remote, so pass to gateway

	xor	edx, edx ; TODO: find device num in edx

	mov	ebx, [IP_LIST+edx]
	and	ebx, [SUBNET_LIST+edx]
	mov	ecx, eax
	and	ecx, [SUBNET_LIST+edx]
	cmp	ecx, ebx
	je	.local

	mov	eax, [GATEWAY_LIST+edx]
	DEBUGF 1,"requested IP is not on subnet, using gateway\n"

  .local:
   ; try to find it on the list
	mov	ecx, [NumARP]
	jz	.not_in_list
	mov	esi, ARPTable + ARP_ENTRY.IP
  .scan_loop:
	scasd
	jz	.found_it
	add	esi, ARP_ENTRY.size - 4
	loop	.scan_loop
  .not_in_list:

	DEBUGF 1,"IP not found on list, preparing for ARP request\n"

   ; if not, reserve an entry in list and send an ARP request packet

	push	eax

	push	word ARP_REQUEST_TTL
	push	word ARP_AWAITING_RESPONSE
	push	dword 0
	push	word 0
	push	eax
	call	ARP_add_entry

	cmp	eax, -1
	je	.full

	pop	eax
	call	ARP_create_request

	ret

  .found_it:
	DEBUGF 1,"Found MAC! (%u-%u-%u-%u-%u-%u)\n",[esi+0]:2,[esi+1]:2,[esi+2]:2,[esi+3]:2,[esi+4]:2,[esi+5]:2
	movzx  eax, word [esi]
	mov    ebx, [esi+2]

	ret

  .full:
	add	esp, 4
	mov	eax, -1
	ret


;---------------------------------------------------------------------------
;
; ARP_create_packet
;
; IN:  ip in eax
;
; OUT: /
;
;---------------------------------------------------------------------------


align 4
ARP_create_request:

	DEBUGF 1,"Create ARP Packet\n"

	call	IPv4_dest_to_dev

	push	eax						; DestIP
	mov	eax, [IP_LIST+4*edi]				; senderIP
	push	eax

	mov	edi, [ETH_DRV_LIST + 4*edi]
	lea	eax, [edi + ETH_DEVICE.mac]
	mov	ebx, ETH_BROADCAST
	mov	ecx, 60 ; minimum packet size
	mov	edx, edi ;;;
	mov	di , ETHER_ARP
	call	ETH_create_Packet
	cmp	edi, -1
	je	.exit

	mov	word [edi + ARP_Packet.HardwareType], 0x0100 ;Ethernet
	mov	word [edi + ARP_Packet.ProtocolType], 0x0008 ;IP
	mov	byte [edi + ARP_Packet.HardwareSize], 6   ;MAC-addr length
	mov	byte [edi + ARP_Packet.ProtocolSize], 4   ;IP-addr length
	mov	word [edi + ARP_Packet.Opcode], ARP_REQ_OPCODE	    ;Request

	add	edi, ARP_Packet.SenderMAC			; sendermac
	lea	esi, [edx + ETH_DEVICE.mac]			;
	movsw							;
	movsd							;

	pop	eax
	stosd							;

	xor	eax, eax					; destmac
	movsw							;
	movsw							;

	pop	eax
	movsd							;

	DEBUGF 1,"ARP Packet for device %x created successfully\n", edx

	call	esi

	inc	[ARP_PACKETS_TX+4*edi]

	ret

  .exit:
	add	esp, 8
	DEBUGF 1,"Create ARP Packet - failed\n"
	mov	eax, -1
	ret



;---------------------------------------------------------------------------
;
; ARP_decrease_entry_ttls
;
; IN: /
; OUT: /
;
;---------------------------------------------------------------------------

align 4
ARP_decrease_entry_ttls:

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.exit

	mov	ebx, ARPTable

.timer_loop:

	movsx	esi, word [ebx + ARP_ENTRY.TTL]
	cmp	esi, 0xFFFFFFFF
	je	.timer_loop_end  ;if TTL==0xFFFF then it's static entry

	test	esi, esi
	jnz	.timer_loop_end_with_dec  ;if TTL!=0

	; Ok, TTL is 0
	;if Status==AWAITING_RESPONSE and TTL==0
	;then we have to change it to ARP_RESPONSE_TIMEOUT
	cmp	word [ebx + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
	jne	@f

	mov	word [ebx + ARP_ENTRY.Status], ARP_RESPONSE_TIMEOUT
	mov	word [ebx + ARP_ENTRY.TTL], word 0x000A   ;10 sec
	jmp	.timer_loop_end

  @@:
	;if TTL==0 and Status==VALID_MAPPING, we have to delete it
	;if TTL==0 and Status==RESPONSE_TIMEOUT, delete too
	mov	esi, dword[NumARP]
	sub	esi, ecx	  ;esi=index of entry, will be deleted

	call	ARP_del_entry

	jmp	.timer_loop_end


.timer_loop_end_with_dec:

	dec	word [ebx + ARP_ENTRY.TTL]  ;decrease TTL

.timer_loop_end:

	add	ebx, ARP_ENTRY.size
	loop	.timer_loop

.exit:

	ret

;---------------------------------------------------------------------------
;
; ARP_add_entry (or update)
;
; IN: arp entry in stack: esp     .IP
;                         esp+4   .MAC
;                         esp+10  .Status
;                         esp+12  .TTL
;                         esp+14
;
; OUT: eax = entry #, -1 on error
;
;---------------------------------------------------------------------------

; TODO: use a mutex

align 4
ARP_add_entry:

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.add

	mov	eax, dword[esp + ARP_ENTRY.MAC]
	mov	bx , word[esp + ARP_ENTRY.MAC + 4]
	mov	esi, ARPTable

.loop:
	cmp	dword [esi + ARP_ENTRY.MAC], eax
	jne	.maybe_next
	cmp	word [esi + ARP_ENTRY.MAC + 4], bx
	jne	.maybe_next

	cmp	dword[esi + ARP_ENTRY.TTL], 0xFFFF ; static entry
	jne	.notstatic
	cmp	dword[esp + ARP_ENTRY.TTL], 0xFFFF
	jne	.exit
.notstatic:

	mov	ebx, [NumARP]
	xchg	ebx, ecx
	sub	ecx, ebx
	jmp	.add

.maybe_next:
	add	esi, ARP_ENTRY.size
	loop	.loop

	mov	ecx, [NumARP]
	cmp	ecx, ARP_TABLE_SIZE
	jge	.full

.add:

	push	ecx
	imul	ecx, ARP_ENTRY.size
	lea	edi, [ecx + ARPTable]
	lea	esi, [esp + 4]
	mov	ecx, ARP_ENTRY.size/2
	repz	movsw

	inc	[NumARP]
	pop	eax

.exit:

	add	esp, 14
	ret

.full:

	mov	eax, -1
	jmp	.exit



;---------------------------------------------------------------------------
;
; ARP_del_entry
;
; IN: entry # in esi
; OUT: /
;
;---------------------------------------------------------------------------

align 4
ARP_del_entry:

	imul	esi, ARP_ENTRY.size

	mov	ecx, (ARP_TABLE_SIZE - 1) * ARP_ENTRY.size
	sub	ecx, esi

	lea	edi, [ebx + esi]	    ;edi=ptr to entry that should be deleted
	lea	esi, [edi + ARP_ENTRY.size] ;esi=ptr to next entry

	shr	ecx,1	   ;ecx/2 => ARP_ENTRY_SIZE MUST BE EVEN NUMBER!
	cld
	rep	movsw

	dec	[NumARP] ;decrease arp-entries counter
	ret




;-----------------------------------------------------
;
; ARP_Handler:
;
;  This function handles ARP protocol over ethernet
;  (other protocols may follow in the future)
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       packet size (without ethernet header) in ecx
;  OUT: /
;
;-----------------------------------------------------

align 4
ARP_handler:

	DEBUGF	1,"ARP_Handler - start\n"
	cmp	ecx, 28
	jl	.exit

	cmp	word [edx + ARP_Packet.Opcode], ARP_REP_OPCODE	; Is this a reply packet?
	jne	.maybe_request

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.exit

	mov	eax, [esp]
	mov	eax, [eax + ARP_Packet.SenderIP]
	mov	esi, ARPTable+ARP_ENTRY.IP

  .loop:
	scasd
	jz	.gotit
	add	esi, ARP_ENTRY.size-4
	loop	.loop

	jmp	.exit

  .gotit:
	cmp	[esi-4+ARP_ENTRY.Status], 0x0300   ;if it is a static entry, dont touch it
	je	.exit

	mov	[esi-4+ARP_ENTRY.Status], ARP_VALID_MAPPING
	mov	[esi+ARP_ENTRY.TTL-4], ARP_ENTRY_TTL

	mov	ebx, [esp]
	mov	eax, dword [ebx + ARP_Packet.SenderMAC]
	mov	dword [esi+ARP_ENTRY.MAC-4], eax
	mov	ax , word [ebx + ARP_Packet.SenderMAC + 4]
	mov	word [esi+ARP_ENTRY.MAC-4+4], ax

	jmp	.exit


;------


  .maybe_request:
	cmp	word [edx + ARP_Packet.Opcode], ARP_REQ_OPCODE	; Is this a request packet?
	jne	.exit

	call	ETH_struc2dev
	DEBUGF	1,"ARP Packet came from device: %u\n", edi
	inc	[ARP_PACKETS_RX+4*edi]
	cmp	edi, -1
	jz	.exit

	mov	eax, edi
	shl	eax, 2
	add	eax, IP_LIST
	mov	eax, [eax]
	cmp	eax, [edx + ARP_Packet.TargetIP]		; Is it looking for my IP address?
	jnz	.exit
	push	eax
	push	edi

; OK, it is a request for one of our MAC addresses. Build the frame and send it
; We can reuse the buffer.  (faster then using ARP_create_packet)

	cld
	lea	esi, [edx + ARP_Packet.SenderMAC]
	lea	edi, [edx + ARP_Packet.TargetMAC]
	movsd							; Move Sender Mac to Dest MAC
	movsw							;
	movsd							; Move sender IP to Dest IP

	pop	esi
	mov	esi, [ETH_DRV_LIST + 4*esi]
	lea	esi, [esi + ETH_DEVICE.mac]
	lea	edi, [edx + ARP_Packet.SenderMAC]
	movsd							; Copy MAC address from in MAC_LIST
	movsw							;
	pop	eax
	stosd							; Write our IP

	mov	word [edx + ARP_Packet.Opcode], ARP_REP_OPCODE

; Now, Fill in ETHERNET header

	mov	edi, [esp]
	lea	esi, [edx + ARP_Packet.TargetMAC]
	movsd
	movsw
	lea	esi, [edx + ARP_Packet.SenderMAC]
	movsd
	movsw
;        mov     ax , ETHER_ARP
;        stosw

	jmp	ETH_Sender					; And send it!

     .exit:
	call	kernel_free
	add	esp, 4						; pop (balance stack)

	DEBUGF 1,"ARP_Handler - fail\n"
	ret




;---------------------------------------------------------------------------
;
; ARP_API
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
ARP_API:

	movzx	eax, bh
	shl	eax, 2

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1
	dec	bl
	jz	.entries	; 2
	dec	bl
	jz	.read		; 3
	dec	bl
	jz	.write		; 4
	dec	bl
	jz	.remove 	; 5
	dec	bl

.error:
	mov	eax, -1
	ret

.packets_tx:
	add	eax, ARP_PACKETS_TX
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, ARP_PACKETS_RX
	mov	eax, [eax]
	ret

.entries:
	mov	eax, [NumARP]
	ret

.read:
	; TODO: write code
	ret

.write:
	; TODO: write code
	; call    ARP_write_entry
	ret

.remove:
	mov	esi, eax
	call	ARP_del_entry
	ret

