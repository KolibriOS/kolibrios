;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
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

$Revision: 983 $

MAX_ETH_DEVICES 	equ MAX_NET_DEVICES
ETH_QUEUE_SIZE		equ 16

struct	ETH_FRAME
	.DstMAC 	dp  ?  ; destination MAC-address [6 bytes]
	.SrcMAC 	dp  ?  ; source MAC-address [6 bytes]
	.Type		dw  ?  ; type of the upper-layer protocol [2 bytes]
	.Data:		       ; data [46-1500 bytes]
ends

struct	ETH_DEVICE
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
	.mode		dd ?  ; This dword contains cable status (10mbit/100mbit, full/half duplex, auto negotiation or not,..)
	.name		dd ?
	.mac		dp ?
ends			      ; the rest of the device struct depends on the type of device


align 4
uglobal

	ETH_RUNNING	dd  ?
	ETH_DRV_LIST	rd  MAX_ETH_DEVICES
	ETH_IN_QUEUE	rd  3*ETH_QUEUE_SIZE+3
	ETH_OUT_QUEUE	rd  3*ETH_QUEUE_SIZE+3
endg


;-----------------------------------------------
;
; ETH_init
;
;  This function resets all ethernet variables
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------

align 4
ETH_init:

	xor	eax, eax
	mov	edi, ETH_RUNNING
	mov	ecx, (1+MAX_ETH_DEVICES)
	rep	stosd

	mov	dword [ETH_IN_QUEUE], ETH_QUEUE_SIZE
	mov	dword [ETH_IN_QUEUE+4], ETH_IN_QUEUE + queue.data
	mov	dword [ETH_IN_QUEUE+8], ETH_IN_QUEUE + queue.data

	mov	dword [ETH_OUT_QUEUE], ETH_QUEUE_SIZE
	mov	dword [ETH_OUT_QUEUE+4], ETH_OUT_QUEUE + queue.data
	mov	dword [ETH_OUT_QUEUE+8], ETH_OUT_QUEUE + queue.data

	ret


;---------------------------------------------------------
;
; ETH_Add_Device:
;
;  This function is called by ethernet drivers,
;  to register each running ethernet device to the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: Device num in eax, -1 on error
;
;---------------------------------------------------------

align 4
ETH_Add_Device:

	DEBUGF	1,"ETH_Add_Device: %x ", ebx

	mov	eax, [ETH_RUNNING]
	cmp	eax, MAX_ETH_DEVICES
	jge	.error

	test	eax, eax
	jnz	.notfirst
	mov	dword [ETH_IN_QUEUE], eax
	mov	dword [ETH_OUT_QUEUE], eax
      .notfirst:

	mov	eax, ebx
	mov	ecx, MAX_ETH_DEVICES	  ; We need to check whole list because a device may be removed without re-organizing list
	mov	edi, ETH_DRV_LIST

	cld
	repne	scasd			  ; See if device is already in the list
	jz	.error

	xor	eax, eax
	mov	ecx, MAX_ETH_DEVICES
	mov	edi, ETH_DRV_LIST

	repne	scasd			  ; Find empty spot in the list
	jnz	.error

	sub	edi, 4
	mov	[edi], ebx		  ; add device to list

	sub	edi, ETH_DRV_LIST	  ; edi = 4*device num       Calculate device number in eax
	mov	eax, edi		  ; edx = 4*device num
	shr	eax, 2

	inc	[ETH_RUNNING]		  ; Indicate that one more ethernet device is up and running

	DEBUGF	1,"- succes: %u\n",eax
	ret

       .error:
	or	eax, -1
	DEBUGF	1,"- fail\n"

	ret




;--------------------------------
;
; ETH_Remove_Device:
;
;  This function is called by ethernet drivers,
;  to unregister ethernet devices from the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: eax: -1 on error
;
;--------------------------------

align 4
ETH_Remove_Device:

	cmp	[ETH_RUNNING], 0
	je	.error

	mov	eax, ebx
	mov	ecx, MAX_ETH_DEVICES
	mov	edi, ETH_DRV_LIST

	repne	scasd
	jnz	.error

	xor	eax, eax
	mov	dword [edi-4], eax

	dec	[ETH_RUNNING]
	jnz	.notlast

	mov	dword [ETH_IN_QUEUE], ETH_QUEUE_SIZE
	mov	dword [ETH_OUT_QUEUE], ETH_QUEUE_SIZE

       .notlast:

	ret

       .error:
	or	eax, -1

	ret



;-------------------------------------------------------------
;
; ETH_Receiver:
;
;  This function is called by ethernet drivers,
;  It pushes the received ethernet packets onto the eth_in_queue
;
;  IN:  Pointer to buffer in [esp], size of buffer in [esp-4], pointer to eth_device in ebx
;  OUT: /
;
;-------------------------------------------------------------

align 4
ETH_Receiver:
	DEBUGF	1,"ETH_Receiver \n"

	add_to_queue ETH_IN_QUEUE, ETH_QUEUE_SIZE, .gohome

  .gohome:
	ret





;-------------------------------------------------------------
;
; ETH_Handler:
;
;  Handles all queued eth packets (called from kernel's main_loop)
;
;  IN:  /
;  OUT: /
;
;-------------------------------------------------------------

align 4
ETH_Handler:

	get_from_queue ETH_IN_QUEUE, ETH_QUEUE_SIZE, .gohome

	DEBUGF	1,"ETH_Handler - size: %u\n", ecx
	cmp	ecx, 60    ; check packet length
	jl	.dump
	sub	ecx, ETH_FRAME.Data

	lea	edx, [eax + ETH_FRAME.Data]
	mov	ax , [eax + ETH_FRAME.Type]

	cmp	ax, ETHER_IPv4
	je	IPv4_Handler

	cmp	ax, ETHER_ARP
	je	ARP_Handler

	DEBUGF	1,"Unknown ethernet packet type %x\n", ax

  .dump:
	DEBUGF	1,"Dumping packet\n"
	call	kernel_free
	add	esp, 4

  .gohome:
	ret				; return 1. to get more from queue / 2. to caller



;-----------------------------------------------------------------
;
; ETH_Sender:
;
;  This function sends an ethernet packet to the correct driver.
;
;  IN:  Pointer to buffer in [esp]
;       size of buffer in [esp+4]
;       pointer to device struct in ebx
;  OUT: /
;
;-----------------------------------------------------------------

align 4
ETH_Sender:
	DEBUGF	1,"ETH_Sender \n"

	add_to_queue ETH_OUT_QUEUE, ETH_QUEUE_SIZE, .gohome

  .gohome:
	ret


align 4
ETH_send_queued:

	get_from_queue ETH_OUT_QUEUE, ETH_QUEUE_SIZE, .gohome

	call	ETH_struc2dev		 ; convert struct ptr to device num (this way we know if driver is still mounted)
	cmp	edi, -1
	je	.fail

	DEBUGF 1,"ETH_Sender - device: %u\n", edi

	jmp	[ebx+ETH_DEVICE.transmit]

     .fail:
	call	kernel_free
	add	esp, 4 ; pop (balance stack)
	DEBUGF 1,"ETH_Sender - fail\n"
     .gohome:
	ret

;---------------------------------------------------------------------------
;
; ETH_struc2dev
;
; IN: pointer to device struct in ebx
;
; OUT: edi is -1 on error, device number otherwise
;
;---------------------------------------------------------------------------

align 4
ETH_struc2dev:
	push	eax ecx

	mov	eax, ebx
	mov	ecx, MAX_ETH_DEVICES
	mov	edi, ETH_DRV_LIST

	repne	scasd
	jnz	.error

	sub	edi, ETH_DRV_LIST+4
	shr	edi, 2

	pop	ecx eax
	ret
  .error:
	or	edi, -1
	pop	ecx eax

	ret


;---------------------------------------------------------------------------
;
; ETH_create_Packet
;
; IN: pointer to source mac in eax
;     pointer to destination mac in ebx
;     packet size in ecx
;     device number in edx
;     protocol in di
;
; OUT: edi is -1 on error, pointer to buffer otherwise                    ;; TODO: XCHG EDX AND EBX output parameters
;      eax points to buffer start
;      ebx is size of complete buffer
;      ecx is unchanged (packet size of embedded data)
;      edx is pointer to device structure
;      esi points to procedure wich needs to be called to send packet
;
;---------------------------------------------------------------------------

align 4
ETH_create_Packet:

	DEBUGF 1,"Creating Ethernet Packet:\n"

	cmp	ecx, 60-ETH_FRAME.Data
	jl	.exit
	cmp	ecx, 1514-ETH_FRAME.Data
	jg	.exit

	push	ecx di eax ebx edx

	add	ecx, ETH_FRAME.Data
	push	ecx
	push	ecx
	call	kernel_alloc
	test	eax, eax
	jz	.pop_exit

	pop	ecx
	pop	edx

	DEBUGF 1,"1"
	mov	edi, eax
	pop	esi
	movsd
	movsw
	DEBUGF 1,"2"
	pop	esi
	movsd
	movsw
	DEBUGF 1,"3"
	pop	ax
	stosw
	DEBUGF 1,"4"

	lea	eax, [edi - ETH_FRAME.Data]  ; Set eax to buffer start
	mov	ebx, ecx		     ; Set ebx to complete buffer size
	pop	ecx
	mov	esi, ETH_Sender

	xor	edx, edx ;;;; TODO: Fixme
	mov	edx, [ETH_DRV_LIST + edx]

	DEBUGF 1,"done: %x size:%u device:%x\n", eax, ebx, edx
	ret

  .pop_exit:
	add	esp, 18
  .exit:
	or	edi, -1
	ret




;---------------------------------------------------------------------------
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
;---------------------------------------------------------------------------

align 4
ETH_API:

	movzx	eax, bh
	shl	eax, 2

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
	dec	bl
	jz	.in_queue	; 6
	dec	bl
	jz	.out_queue	; 7

.error:
	mov	eax, -1
	ret

.packets_tx:
	add	eax, ETH_DRV_LIST
	mov	eax, dword [eax]
	mov	eax, dword [eax + ETH_DEVICE.packets_tx]

	ret

.packets_rx:
	add	eax, ETH_DRV_LIST
	mov	eax, dword [eax]
	mov	eax, dword [eax + ETH_DEVICE.packets_rx]
	ret

.bytes_tx:
	add	eax, ETH_DRV_LIST
	mov	eax, dword [eax]
	mov	eax, dword [eax + ETH_DEVICE.bytes_tx + 4]
	mov	eax, dword [eax + ETH_DEVICE.bytes_tx]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret

.bytes_rx:
	add	eax, ETH_DRV_LIST
	mov	eax, dword [eax]
	mov	ebx, dword [eax + ETH_DEVICE.bytes_rx + 4]
	mov	eax, dword [eax + ETH_DEVICE.bytes_rx]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret


.read_mac:
	add	eax, ETH_DRV_LIST
	mov	eax, [eax]
;        push    eax
;        call    dword [eax + ETH_DEVICE.get_MAC]
;        pop     eax
	movzx	ebx, word [eax + ETH_DEVICE.mac]
	mov	eax, dword [eax + ETH_DEVICE.mac + 2]
	mov	[esp+20+4], ebx 			; TODO: fix this ugly code
	ret

.write_mac:
	push	ecx
	push	dx
	add	eax, ETH_DRV_LIST
	mov	eax, [eax]
	mov	eax, dword [eax + ETH_DEVICE.set_MAC]
	call	eax
	ret

.in_queue:
	add	eax, ETH_IN_QUEUE
	mov	eax, [eax + queue.size]
	ret

.out_queue:
	add	eax, ETH_OUT_QUEUE
	mov	eax, [eax + queue.size]
	ret