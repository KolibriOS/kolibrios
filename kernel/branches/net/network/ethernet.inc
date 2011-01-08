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

struct	ETH_FRAME
	.DstMAC 	dp  ?  ; destination MAC-address
	.SrcMAC 	dp  ?  ; source MAC-address
	.Type		dw  ?  ; type of the upper-layer protocol
	.Data:		       ; data (46-1500 bytes for a normal packet)
ends

virtual at NET_DEVICE.end

	ETH_DEVICE:

	.set_mode	dd ?
	.get_mode	dd ?

	.set_MAC	dd ?
	.get_MAC	dd ?

	.mode		dd ?
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
;-----------------------------------------------------------------
macro	ETH_init {

	mov	[ETH_RUNNING], 0

}


;-----------------------------------------------------------------
;
; ETH_input
;
;  This function is called by ethernet drivers,
;  It pushes the received ethernet packets onto the eth_in_queue
;
;  IN:   [esp]  = Pointer to buffer
;       [esp+4] = size of buffer
;         ebx   = pointer to eth_device
;  OUT: /
;
;-----------------------------------------------------------------
align 4
ETH_input:
	mov	eax, [esp]
	mov	ecx, [esp+4]

	DEBUGF	1,"ETH_input - size: %u\n", ecx
	cmp	ecx, 60    ; check packet length
	jl	.dump
	sub	ecx, ETH_FRAME.Data

	lea	edx, [eax + ETH_FRAME.Data]
	mov	ax , [eax + ETH_FRAME.Type]

	cmp	ax, ETHER_IPv4
	je	IPv4_input

	cmp	ax, ETHER_ARP
	je	ARP_input

;        cmp     ax, ETHER_PPP_DISCOVERY
;        je      PPPOE_discovery

	DEBUGF	2,"Unknown ethernet packet type %x\n", ax

  .dump:
	DEBUGF	2,"ETH_input - dumping\n"
	call	kernel_free
	add	esp, 4
	ret

;-----------------------------------------------------------------
;
; ETH_output
;
; IN: eax = pointer to source mac
;     ebx = device ptr
;     ecx = packet size
;     edx = pointer to destination mac
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
ETH_output:

	DEBUGF	1,"ETH_output: size=%u device:%x\n", ecx, ebx

	cmp	ecx, [ebx + NET_DEVICE.mtu]
	jg	.exit

	push	ecx			; << 1
	push	di eax edx		; << 2
	add	ecx, ETH_FRAME.Data

	push	ecx			; << 3

	push	ecx			; << 4
	call	kernel_alloc		; >> 4
	test	eax, eax
	jz	.out_of_ram
	mov	edi, eax

	pop	ecx			; >> 3

	pop	esi			; >> 2
	movsd
	movsw
	pop	esi			; >> 2
	movsd
	movsw
	pop	ax			; >> 2
	stosw

	lea	eax, [edi - ETH_FRAME.Data]  ; Set eax to buffer start
	mov	edx, ecx		     ; Set edx to complete buffer size

	pop	ecx			; >> 1

	cmp	edx, 60-1		; minimum ethernet packet size
	jle	.adjust_size
	DEBUGF	1,"ETH_output: done: %x total size: %u\n", eax, edx
	ret

  .adjust_size:
	mov	edx, 60
	test	edx, edx	; clear zero flag
	ret

  .out_of_ram:
	DEBUGF	2,"ETH_output: Out of ram space!!\n"
	add	esp, 3*4+2+4
	sub	edi, edi
	ret

  .exit:
	DEBUGF	2,"ETH_output: Packet too large!\n"
	sub	edi, edi
;;;        dec     edi
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
	mov	eax, dword [eax + NET_DEVICE.packets_tx]

	ret

.packets_rx:
	mov	eax, dword [eax + NET_DEVICE.packets_rx]
	ret

.bytes_tx:
	mov	ebx, dword [eax + NET_DEVICE.bytes_tx + 4]
	mov	eax, dword [eax + NET_DEVICE.bytes_tx]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret

.bytes_rx:
	mov	ebx, dword [eax + NET_DEVICE.bytes_rx + 4]
	mov	eax, dword [eax + NET_DEVICE.bytes_rx]
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
	call	[eax + ETH_DEVICE.set_MAC]
	ret

