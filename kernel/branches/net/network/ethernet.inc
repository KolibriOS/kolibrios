;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
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

ETH_QUEUE_SIZE		equ 16

struct	ETH_FRAME
	.DstMAC 	dp  ?  ; destination MAC-address
	.SrcMAC 	dp  ?  ; source MAC-address
	.Type		dw  ?  ; type of the upper-layer protocol
	.Data:		       ; data (46-1500 bytes for a normal packet)
ends

virtual at NET_DEVICE.end

	ETH_DEVICE:
	.unload 	dd ?
	.reset		dd ?
	.transmit	dd ?
	.set_MAC	dd ?
	.get_MAC	dd ?
	.set_mode	dd ?
	.get_mode	dd ?

	.bytes_tx	dq ?
	.bytes_rx	dq ?
	.packets_tx	dd ?
	.packets_rx	dd ?
	.mode		dd ?
	.name		dd ?
	.mac		dp ?

end virtual

align 4
iglobal

	ETH_BROADCAST	dp  0xffffffffffff
endg

align 4
uglobal
	ETH_RUNNING	dd  ?
endg


;-----------------------------------------------------------------
;
; ETH_init
;
;  This function resets all ethernet variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ETH_init:

	mov	[ETH_RUNNING], 0

	ret


;-----------------------------------------------------------------
;
; ETH_Receiver:
;
;  This function is called by ethernet drivers,
;  It pushes the received ethernet packets onto the eth_in_queue
;
;  IN:   [esp]  = Pointer to buffer
;       [esp-4] = size of buffer
;         ebx   = pointer to eth_device
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ETH_receiver:
	mov	eax, [esp]
	mov	ecx, [esp+4]

	DEBUGF	1,"ETH_Handler - size: %u\n", ecx
	cmp	ecx, 60    ; check packet length
	jl	.dump
	sub	ecx, ETH_FRAME.Data

	lea	edx, [eax + ETH_FRAME.Data]
	mov	ax , [eax + ETH_FRAME.Type]

	cmp	ax, ETHER_IPv4
	je	IPv4_handler

	cmp	ax, ETHER_ARP
	je	ARP_handler

	DEBUGF	2,"Unknown ethernet packet type %x\n", ax

  .dump:
	DEBUGF	2,"ETH_Handler - dumping\n"
	call	kernel_free
	add	esp, 4
	ret

;-----------------------------------------------------------------
;
; ETH_create_packet
;
; IN: eax = pointer to source mac
;     ebx = pointer to destination mac
;     ecx = packet size
;     edx = device number
;      di = protocol
;
; OUT: edi = 0 on error, pointer to buffer otherwise
;      eax = buffer start
;      ebx = to device structure
;      ecx = unchanged (packet size of embedded data)
;      edx = size of complete buffer
;
;-----------------------------------------------------------------
align 4
ETH_create_packet:

	DEBUGF	1,"Creating Ethernet Packet (size=%u): \n", ecx

	cmp	ecx, 1500 ;;;
	jg	.exit

	push	ecx di eax ebx edx

	add	ecx, ETH_FRAME.Data
	push	ecx
	push	ecx
	call	kernel_alloc
	mov	edi, eax
	test	edi, edi
	jz	.pop_exit

	pop	ecx
	pop	edx

	pop	esi
	movsd
	movsw
	pop	esi
	movsd
	movsw
	pop	ax
	stosw

	lea	eax, [edi - ETH_FRAME.Data]  ; Set eax to buffer start
	mov	edx, ecx		     ; Set ebx to complete buffer size
	pop	ecx

	xor	ebx, ebx			;;;; TODO: Fixme
	mov	ebx, [NET_DRV_LIST + ebx]

	cmp	edx, 46 + ETH_FRAME.Data    ; If data size is less then 46, add padding bytes
	jg	.continue
	mov	edx, 46 + ETH_FRAME.Data
       .continue:

	DEBUGF	1,"done: %x size:%u device:%x\n", eax, edx, ebx
	ret

  .pop_exit:
	DEBUGF	2,"Out of ram space!!\n"
	add	esp, 18
	and	edi, 0
	ret

  .exit:
	DEBUGF	2,"Packet too large!\n"
	and	edi, 0
	ret



;-----------------------------------------------------------------
;
; ETH_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;-----------------------------------------------------------------
align 4
ETH_API:

	cmp	bh, MAX_NET_DEVICES
	jg	.error
	movzx	eax, bh
	shl	eax, 2

	cmp	bl, 7
	jz	.out_queue
	cmp	bl, 6
	jz	.in_queue

	mov	eax, dword [NET_DRV_LIST + eax]
	cmp	[eax + NET_DEVICE.type], NET_TYPE_ETH
	jne	.error

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1
	dec	bl
	jz	.bytes_tx	; 2
	dec	bl
	jz	.bytes_rx	; 3
	dec	bl
	jz	.read_mac	; 4
	dec	bl
	jz	.write_mac	; 5

  .error:
	DEBUGF	2,"Device is not ethernet type\n"
	or	eax, -1
	ret

.packets_tx:
	mov	eax, dword [eax + ETH_DEVICE.packets_tx]

	ret

.packets_rx:
	mov	eax, dword [eax + ETH_DEVICE.packets_rx]
	ret

.bytes_tx:
	mov	ebx, dword [eax + ETH_DEVICE.bytes_tx + 4]
	mov	eax, dword [eax + ETH_DEVICE.bytes_tx]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret

.bytes_rx:
	mov	ebx, dword [eax + ETH_DEVICE.bytes_rx + 4]
	mov	eax, dword [eax + ETH_DEVICE.bytes_rx]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret


.read_mac:
	movzx	ebx, word [eax + ETH_DEVICE.mac]
	mov	eax, dword [eax + ETH_DEVICE.mac + 2]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret

.write_mac:
	push	ecx
	push	dx
	mov	eax, [eax + ETH_DEVICE.set_MAC]
	call	eax
	ret

.in_queue:
   if ETH_QUEUE
	add	eax, ETH_IN_QUEUE
	mov	eax, [eax + queue.size]
   else
	or	eax, -1
   end if
	ret

.out_queue:
   if ETH_QUEUE
	add	eax, ETH_OUT_QUEUE
	mov	eax, [eax + queue.size]
   else
	or	eax, -1
   end if
	ret