;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
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
;;             Version 2, June- 1991                               ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

ARP_NO_ENTRY		equ 0
ARP_VALID_MAPPING	equ 1
ARP_AWAITING_RESPONSE	equ 2
ARP_RESPONSE_TIMEOUT	equ 3

ARP_REQUEST_TTL 	equ 31		; 20 s
ARP_ENTRY_TTL		equ 937 	; 600 s

ARP_REQ_OPCODE		equ 0x0100	; request
ARP_REP_OPCODE		equ 0x0200	; reply

ARP_TABLE_SIZE		equ 20		; Size of table

struct ARP_ENTRY
       .IP		dd  ?
       .MAC		dp  ?
       .Status		dw  ?
       .TTL		dw  ?
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
       .size:
ends


align 4
uglobal

	NumARP		dd ?

	ARP_table	rb ARP_ENTRY.size * ARP_TABLE_SIZE

	ARP_PACKETS_TX	rd MAX_NET_DEVICES
	ARP_PACKETS_RX	rd MAX_NET_DEVICES


endg



;-----------------------------------------------------------------
;
; ARP_init
;
;  This function resets all ARP variables
;
;-----------------------------------------------------------------
macro ARP_init {

	xor	eax, eax
	mov	[NumARP], eax

	mov	edi, ARP_PACKETS_TX
	mov	ecx, 2*MAX_NET_DEVICES
	rep	stosd

}

;---------------------------------------------------------------------------
;
; ARP_decrease_entry_ttls
;
;---------------------------------------------------------------------------

macro ARP_decrease_entry_ttls {

local	.loop
local	.exit

; The TTL field is decremented every second, and is deleted when it reaches 0.
; It is refreshed every time a packet is received.
; If the TTL field is 0xFFFF it is a static entry and is never deleted.
; The status field can be the following values:
; 0x0000  entry not used
; 0x0001  entry holds a valid mapping
; 0x0002  entry contains an IP address, awaiting ARP response
; 0x0003  No response received to ARP request.
; The last status value is provided to allow the network layer to delete
; a packet that is queued awaiting an ARP response

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.exit

	mov	esi, ARP_table
  .loop:
	cmp	[esi + ARP_ENTRY.TTL], 0xffff	; 0xffff = static entry
	je	.next

	dec	[esi + ARP_ENTRY.TTL]
	jz	.time_out

  .next:
	add	esi, ARP_ENTRY.size
	loop	.loop
	jmp	.exit

  .time_out:
	cmp	[esi + ARP_ENTRY.Status], ARP_AWAITING_RESPONSE
	jz	.response_timeout

	push	esi ecx
	call	ARP_del_entry
	pop	ecx esi

	jmp	.next

  .response_timeout:
	mov	[esi + ARP_ENTRY.Status], ARP_RESPONSE_TIMEOUT
	mov	[esi + ARP_ENTRY.TTL], 10

	jmp	.next

  .exit:

}


;-----------------------------------------------------------------
;
; ARP_input
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       packet size (without ethernet header) in ecx
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ARP_input:

	DEBUGF	1,"ARP_Handler - start\n"
	cmp	ecx, 28
	jl	.exit

;---------------------
; Handle Reply packets

	cmp	word [edx + ARP_Packet.Opcode], ARP_REP_OPCODE
	jne	.maybe_request

	DEBUGF	1,"ARP_Handler - it's a reply packet from %u.%u.%u.%u\n",\
	[edx + ARP_Packet.SenderIP]:1,[edx + ARP_Packet.SenderIP+1]:1,[edx + ARP_Packet.SenderIP+2]:1,[edx + ARP_Packet.SenderIP+3]:1,

	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.exit

	mov	eax, [edx + ARP_Packet.SenderIP]
	mov	esi, ARP_table

  .loop:
	cmp	[esi + ARP_ENTRY.IP], eax
	je	.gotit
	add	esi, ARP_ENTRY.size
	loop	.loop

	jmp	.exit

  .gotit:
	DEBUGF	1,"ARP_Handler - found matching entry\n"

	cmp	[esi+ARP_ENTRY.TTL], 0xffff		; if it is a static entry, dont touch it
	je	.exit

	DEBUGF	1,"ARP_Handler - updating entry\n"

	mov	[esi+ARP_ENTRY.Status], ARP_VALID_MAPPING
	mov	[esi+ARP_ENTRY.TTL], ARP_ENTRY_TTL

	mov	eax, dword [edx + ARP_Packet.SenderMAC]
	mov	dword [esi+ARP_ENTRY.MAC], eax
	mov	ax , word [edx + ARP_Packet.SenderMAC + 4]
	mov	word [esi+ARP_ENTRY.MAC+4], ax

	jmp	.exit


;-----------------------
; Handle Request packets

  .maybe_request:
	cmp	word [edx + ARP_Packet.Opcode], ARP_REQ_OPCODE
	jne	.exit

	call	NET_ptr_to_num
	cmp	edi, -1
	jz	.exit
	DEBUGF	1,"ARP Request packet through device: %u\n", edi
	inc	[ARP_PACKETS_RX+4*edi]

	mov	eax, [IP_LIST+4*edi]
	cmp	eax, [edx + ARP_Packet.TargetIP]		; Is it looking for my IP address?
	jne	.exit						; TODO: instead of quitting, update local entrys with matching IP's ?

	push	eax
	push	edi

; OK, it is a request for one of our MAC addresses.
; Build the frame and send it. We can reuse the buffer.  (faster then using ARP_create_packet)

	lea	esi, [edx + ARP_Packet.SenderMAC]
	lea	edi, [edx + ARP_Packet.TargetMAC]
	movsd							; Move Sender Mac to Dest MAC
	movsw							;
	movsd							; Move sender IP to Dest IP

	pop	esi
	mov	esi, [NET_DRV_LIST + 4*esi]
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

	DEBUGF	1,"ARP_Handler - Sending reply \n"

	call	[ebx + NET_DEVICE.transmit]
	ret

     .exit:
	call	kernel_free
	add	esp, 4						; pop (balance stack)

	DEBUGF	1,"ARP_Handler - exiting\n"
	ret


;---------------------------------------------------------------------------
;
; ARP_output_request
;
; IN:  ip in eax
; OUT: /
;
;---------------------------------------------------------------------------
align 4
ARP_output_request:

	DEBUGF 1,"Create ARP Packet\n"

	call	IPv4_dest_to_dev
	push	eax				; DestIP
	pushd	[IP_LIST+edi]			; SenderIP

	mov	ebx, [NET_DRV_LIST+edi] 	; device ptr

	lea	eax, [ebx + ETH_DEVICE.mac]	; local device mac
	mov	edx, ETH_BROADCAST		; broadcast mac
	mov	ecx, ARP_Packet.size
	mov	di, ETHER_ARP
	call	ETH_output
	jz	.exit

	mov	ecx, eax

	mov	[edi + ARP_Packet.HardwareType], 0x0100 	; Ethernet
	mov	[edi + ARP_Packet.ProtocolType], 0x0008 	; IP
	mov	[edi + ARP_Packet.HardwareSize], 6		; MAC-addr length
	mov	[edi + ARP_Packet.ProtocolSize], 4		; IP-addr length
	mov	[edi + ARP_Packet.Opcode], ARP_REQ_OPCODE	; Request

	add	edi, ARP_Packet.SenderMAC

	lea	esi, [ebx + ETH_DEVICE.mac]	; SenderMac
	movsw					;
	movsd					;
	pop	eax				; SenderIP
	stosd					;

	mov	eax, -1 			; DestMac
	stosd					;
	stosw					;
	pop	eax				; DestIP
	stosd					;

	DEBUGF 1,"ARP Packet for device %x created successfully\n", ebx

	push	edx ecx
	call	[ebx + NET_DEVICE.transmit]
	ret

  .exit:
	add	esp, 4+4
	DEBUGF	1,"Create ARP Packet - failed\n"
	sub	eax, eax
	ret


;-----------------------------------------------------------------
;
; ARP_add_entry (or update)
;
; IN:  esi = ptr to entry (can easily be made on the stack)
; OUT: eax = entry #, -1 on error
;
;-----------------------------------------------------------------   ; TODO: use a mutex
align 4
ARP_add_entry:

	DEBUGF 1,"ARP add entry: "

	mov	ecx, [NumARP]
	test	ecx, ecx		; first entry?
	jz	.add
	cmp	ecx, ARP_TABLE_SIZE	; list full ?
	jge	.error

	mov	eax, dword[esi + ARP_ENTRY.MAC]
	mov	bx , word[esi + ARP_ENTRY.MAC + 4]
	mov	edi, ARP_table

  .loop:
	cmp	dword [edi + ARP_ENTRY.MAC], eax	; Check for duplicate MAC's
	jne	.maybe_next				;
	cmp	word [edi + ARP_ENTRY.MAC + 4], bx	;
	jne	.maybe_next				;

	cmp	dword[edi + ARP_ENTRY.TTL], 0xFFFF	; static entry
	jne	.notstatic
	cmp	dword[esi + ARP_ENTRY.TTL], 0xFFFF
	jne	.error
  .notstatic:

	neg	ecx
	add	ecx, [NumARP]
	jmp	.add

  .maybe_next:
	add	esi, ARP_ENTRY.size
	loop	.loop

	mov	ecx, [NumARP]
  .add:
	push	ecx
	imul	ecx, ARP_ENTRY.size
	lea	edi, [ecx + ARP_table]
	mov	ecx, ARP_ENTRY.size/2
	rep	movsw

	lea	esi, [edi - ARP_ENTRY.size]
	inc	[NumARP]
	pop	eax
	DEBUGF 1,"New entry created: %u\n", eax

  .exit:
	DEBUGF 1,"Exiting\n"
	ret

  .error:
	DEBUGF 1,"error! \n"
	mov	eax, -1
	ret


;-----------------------------------------------------------------
;
; ARP_del_entry
;
; IN:  esi = ptr to arp entry
; OUT: /
;
;-----------------------------------------------------------------
align 4
ARP_del_entry:

	DEBUGF 1,"ARP del entry %x, total entrys: %u\n", esi, [NumARP]

	mov	ecx, ARP_table + (ARP_TABLE_SIZE - 1) * ARP_ENTRY.size
	sub	ecx, esi
	shr	ecx, 1

	mov	edi, esi
	lea	esi, [edi + ARP_ENTRY.size]
	rep	movsw

	dec	[NumARP]
	DEBUGF 1,"ARP entry deleted\n"

	ret





;-----------------------------------------------------------------
;
; ARP_IP_to_MAC
;
;  This function translates an IP address to a MAC address
;
;  IN:  eax = IPv4 address
;  OUT: eax = -1 on error, -2 means request send
;      else, ax = first two bytes of mac (high 16 bits of eax will be 0)
;       ebx = last four bytes of mac
;
;-----------------------------------------------------------------
align 4
ARP_IP_to_MAC:

	DEBUGF 1,"ARP_IP_to_MAC\n"

	cmp	eax, 0xffffffff
	je	.broadcast

; if ((Remote IP & subnet_mask) == (local IP & subnet_mask ))
; destination is on same subnet
; else, destination is remote and must use a gateway

	call	IPv4_dest_to_dev
	mov	ebx, [IP_LIST + edi]
	and	ebx, [SUBNET_LIST + edi]

	mov	ecx, eax
	and	ecx, [SUBNET_LIST + edi]

	cmp	ecx, ebx
	je	.local

	mov	eax, [GATEWAY_LIST + edi]
	DEBUGF	1,"requested IP is not on subnet, using default gateway\n"

;--------------------------------
; Try to find the IP in ARP_table

  .local:
	mov	ecx, [NumARP]
	test	ecx, ecx
	jz	.not_in_list
	mov	esi, ARP_table + ARP_ENTRY.IP
  .scan_loop:
	cmp	[esi], eax
	je	.found_it
	add	esi, ARP_ENTRY.size
	loop	.scan_loop

  .not_in_list:
	DEBUGF 1,"IP not found on list, preparing for ARP request\n"

;--------------------
; Send an ARP request

	push	eax

	pushw	ARP_REQUEST_TTL
	pushw	ARP_AWAITING_RESPONSE
	pushd	0
	pushw	0
	pushd	eax
	mov	esi, esp
	call	ARP_add_entry
	add	esp, ARP_ENTRY.size

	cmp	eax, -1
	je	.full

	mov	ecx, eax
	pop	eax
	push	ecx
	call	ARP_output_request

;; TODO: check if driver could transmit packet

	pop	esi
	imul	esi, ARP_ENTRY.size
	add	esi, ARP_table

	mov	ecx, 25
  .wait_loop:
	cmp	[esi + ARP_ENTRY.Status], 1
	je	.got_it
	push	esi
	mov	esi, 10
	call	delay_ms
	pop	esi
	loop	.wait_loop

	mov	eax, -2 	; request send
	ret

  .found_it:
	DEBUGF	1,"found IP in ARPTable\n"
	cmp	[esi + ARP_ENTRY.Status], 1
	jne	.invalid

  .got_it:
	movzx	eax, word [esi+ARP_ENTRY.MAC]
	mov	ebx, dword[esi+ARP_ENTRY.MAC+2]
	ret

  .invalid:
	mov	eax, -1
	ret

  .full:
	DEBUGF	1,"ARP table is full!\n"
	pop	eax
	mov	eax, -1
	ret

  .broadcast:
	mov	eax, 0x0000ffff
	mov	ebx, 0xffffffff
	ret


;-----------------------------------------------------------------
;
; ARP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:  ?
;
;-----------------------------------------------------------------
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
	cmp	ecx, [NumARP]
	jge	.error
	; edi = pointer to buffer
	; ecx = # entry
	imul	ecx, ARP_ENTRY.size
	add	ecx, ARP_table
	mov	esi, ecx
	mov	ecx, ARP_ENTRY.size/2
	rep	movsw

	xor	eax, eax
	ret

.write:
	; esi = pointer to buffer
	call	ARP_add_entry	     ;out: eax = entry number, -1 on error
	ret

.remove:
	; ecx = # entry
	cmp	ecx, [NumARP]
	jge	.error
	imul	ecx, ARP_ENTRY.size
	lea	esi, [ARP_table + ecx]
	call	ARP_del_entry
	ret

