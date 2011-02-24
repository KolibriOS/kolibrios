;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  STACK.INC                                                      ;;
;;                                                                 ;;
;;  TCP/IP stack for KolibriOS                                     ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;     Some parts of code are based on the work of:                ;;
;;      Mike Hibbett (menuetos network stack)                      ;;
;;      Eugen Brasoveanu (solar os network stack and drivers)      ;;
;;      mike.dld (kolibrios socket code)                           ;;
;;                                                                 ;;
;;     TCP part is based on 4.4BSD                                 ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

__DEBUG_LEVEL_OLD__	equ __DEBUG_LEVEL__	; use seperate debug level for network part of kernel
__DEBUG_LEVEL__ 	equ 1

uglobal
	net_10ms	dd ?
	net_tmr_count	dw ?
endg

MAX_NET_DEVICES 	equ 16

MIN_EPHEMERAL_PORT	equ 49152
MAX_EPHEMERAL_PORT	equ 61000

; Ethernet protocol numbers
ETHER_ARP		equ 0x0608
ETHER_IPv4		equ 0x0008
ETHER_PPP_DISCOVERY	equ 0x6388
ETHER_PPP_SESSION	equ 0x6488

;Protocol family
AF_UNSPEC		equ 0
AF_UNIX 		equ 1
AF_INET4		equ 2
AF_INET6		equ 10

; Internet protocol numbers
IP_PROTO_IP		equ 0
IP_PROTO_ICMP		equ 1
IP_PROTO_TCP		equ 6
IP_PROTO_UDP		equ 17

; Socket types
SOCK_STREAM		equ 1
SOCK_DGRAM		equ 2
SOCK_RAW		equ 3

; Socket options
SO_ACCEPTCON		equ 1 shl 0
SO_BROADCAST		equ 1 shl 1
SO_DEBUG		equ 1 shl 2
SO_DONTROUTE		equ 1 shl 3
SO_KEEPALIVE		equ 1 shl 4
SO_OOBINLINE		equ 1 shl 5
SO_REUSEADDR		equ 1 shl 6
SO_REUSEPORT		equ 1 shl 7
SO_USELOOPBACK		equ 1 shl 8


; Socket States
SS_NOFDREF		equ 0x001	; no file table ref any more
SS_ISCONNECTED		equ 0x002	; socket connected to a peer
SS_ISCONNECTING 	equ 0x004	; in process of connecting to peer
SS_ISDISCONNECTING	equ 0x008	; in process of disconnecting
SS_CANTSENDMORE 	equ 0x010	; can't send more data to peer
SS_CANTRCVMORE		equ 0x020	; can't receive more data from peer
SS_RCVATMARK		equ 0x040	; at mark on input
SS_ISABORTING		equ 0x080	; aborting fd references - close()
SS_RESTARTSYS		equ 0x100	; restart blocked system calls
SS_ISDISCONNECTED	equ 0x800	; socket disconnected from peer

SS_ASYNC		equ 0x100	; async i/o notify
SS_ISCONFIRMING 	equ 0x200	; deciding to accept connection req
SS_MORETOCOME		equ 0x400


SOCKET_MAXDATA		equ 4096*32	; must be 4096*(power of 2) where 'power of 2' is at least 8

; Network driver types
NET_TYPE_ETH		equ 1
NET_TYPE_SLIP		equ 2

MAX_backlog		equ 20		; maximum backlog for stream sockets

; Error Codes
ENOBUFS 		equ 55
ECONNREFUSED		equ 61
ECONNRESET		equ 52
ETIMEDOUT		equ 60
ECONNABORTED		equ 53



virtual at 0

	NET_DEVICE:

	.type		dd ?	; Type field
	.mtu		dd ?	; Maximal Transmission Unit
	.name		dd ?	; Ptr to 0 terminated string

	.unload 	dd ?	; Ptrs to driver functions
	.reset		dd ?	;
	.transmit	dd ?	;

	.bytes_tx	dq ?	; Statistics, updated by the driver
	.bytes_rx	dq ?	;
	.packets_tx	dd ?	;
	.packets_rx	dd ?	;

;       .hwacc          dd ?    ; bitmask stating available hardware accelerations (offload engines)

	.end:

end virtual


; Exactly as it says..
macro pseudo_random reg {
	add	reg, [esp]
	rol	reg, 5
	xor	reg, [timer_ticks]
	add	reg, [CPU_FREQ]
	imul	reg, 214013
	xor	reg, 0xdeadbeef
	rol	reg, 9
}

macro ntohd reg {

	rol	word reg, 8
	rol	dword reg, 16
	rol	word reg , 8

}

macro ntohw reg {

	rol	word reg, 8

}

include "queue.inc"

include "ethernet.inc"

;include "slip.inc"
;include "pppoe.inc"

include "ARP.inc"
include "IPv4.inc"

include "icmp.inc"
include "udp.inc"
include "tcp.inc"

include "socket.inc"



align 4
uglobal

	NET_RUNNING	dd  ?
	NET_DRV_LIST	rd  MAX_NET_DEVICES

endg


;-----------------------------------------------------------------
;
; stack_init
;
;  This function calls all network init procedures
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
stack_init:

; Init the network drivers list
	xor	eax, eax
	mov	edi, NET_RUNNING
	mov	ecx, MAX_NET_DEVICES + 1
	rep	stosd

	ETH_init
;        SLIP_init
;        PPPOE_init

	IPv4_init
	ICMP_init

	ARP_init
	UDP_init
	TCP_init

	SOCKET_init

	mov	[net_tmr_count], 0

	ret


;-----------------------------------------------------------------
;
; stack_handler
;
;  This function is called in kernel loop
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
stack_handler:

	cmp	[NET_RUNNING], 0
	je	.exit

	; Test for 10ms tick
	mov	eax, [timer_ticks]
	cmp	eax, [net_10ms]
	je	.exit
	mov	[net_10ms], eax

	test	[net_10ms], 0x0f	; 160ms
	jnz	.exit

	TCP_timer_160ms

	test	[net_10ms], 0x3f	; 640ms
	jnz	.exit

	TCP_timer_640ms
	ARP_decrease_entry_ttls
	IPv4_decrease_fragment_ttls

  .exit:
	ret



;-----------------------------------------------------------------
;
; NET_add_Device:
;
;  This function is called by the network drivers,
;  to register each running NIC to the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: Device num in eax, -1 on error
;
;-----------------------------------------------------------------
align 4
NET_add_device:

	DEBUGF	1,"NET_Add_Device: %x\n", ebx

	mov	eax, [NET_RUNNING]
	cmp	eax, MAX_NET_DEVICES
	jge	.error

;----------------------------------
; Check if device is already listed
	mov	eax, ebx
	mov	ecx, MAX_NET_DEVICES	  ; We need to check whole list because a device may be removed without re-organizing list
	mov	edi, NET_DRV_LIST

	repne	scasd			  ; See if device is already in the list
	jz	.error

;----------------------------
; Find empty slot in the list
	xor	eax, eax
	mov	ecx, MAX_NET_DEVICES
	mov	edi, NET_DRV_LIST

	repne	scasd
	jnz	.error

	sub	edi, 4

	cmp	[ebx + NET_DEVICE.type], NET_TYPE_ETH
	je	.ethernet

	cmp	[ebx + NET_DEVICE.type], NET_TYPE_SLIP
	je	.slip

	DEBUGF	1,"Unknown network device type: %u\n", [ebx + NET_DEVICE.type]
	jmp	.error

  .ethernet:
	DEBUGF	1,"Trying to add an ethernet device\n"

	inc	[ETH_RUNNING]		  ; Indicate that one more ethernet device is up and running
	jmp	.add_it

  .slip:
	DEBUGF	1,"Trying to add a slip device\n"
	;;;;
	jmp	.error


  .add_it:

;-----------------------------
; Add device to the found slot
	mov	[edi], ebx		  ; add device to list

	sub	edi, NET_DRV_LIST	  ; Calculate device number in eax
	mov	eax, edi		  ;
	shr	eax, 2

	inc	[NET_RUNNING]		  ; Indicate that one more network device is up and running

	DEBUGF	1,"Device number: %u\n",eax
	ret

  .error:
	or	eax, -1
	DEBUGF	2,"Adding network device failed\n"
	ret



;-----------------------------------------------------------------
;
; NET_Remove_Device:
;
;  This function is called by etwork drivers,
;  to unregister network devices from the kernel
;
;  IN:  Pointer to device structure in ebx
;  OUT: eax: -1 on error
;
;-----------------------------------------------------------------
align 4
NET_remove_device:

	cmp	[NET_RUNNING], 0
	je	.error

;----------------------------
; Find the driver in the list

	mov	eax, ebx
	mov	ecx, MAX_NET_DEVICES
	mov	edi, NET_DRV_LIST

	repne	scasd
	jnz	.error

;------------------------
; Remove it from the list

	xor	eax, eax
	mov	dword [edi-4], eax

	dec	[NET_RUNNING]
	ret

  .error:
	or	eax, -1
	ret



;-----------------------------------------------------------------
;
; NET_ptr_to_num
;
; IN:  ebx = ptr to device struct
; OUT: edi = -1 on error, device number otherwise
;
;-----------------------------------------------------------------
align 4
NET_ptr_to_num:
	push	ecx

	mov	ecx, MAX_NET_DEVICES
	mov	edi, NET_DRV_LIST

  .loop:
	cmp	ebx, [edi]
	jz	.found
	add	edi, 4
	dec	ecx
	jnz	.loop

	; repnz  scasd could work too if eax is used instead of ebx!

	or	edi, -1

	pop	ecx
	ret

  .found:
	sub	edi, NET_DRV_LIST
	shr	edi, 2

	pop	ecx
	ret

;-----------------------------------------------------------------
;
; checksum_1
;
;  This is the first of two functions needed to calculate a checksum.
;
;  IN:  edx = start offset for semi-checksum
;       esi = pointer to data
;       ecx = data size
;  OUT: edx = semi-checksum
;
;
; Code was optimized by diamond
;
;-----------------------------------------------------------------
align 4
checksum_1:

	shr	ecx, 1
	pushf
	jz	.no_2

	shr	ecx, 1
	pushf
	jz	.no_4

	shr	ecx, 1
	pushf
	jz	.no_8

  .loop:
	add	dl, [esi+1]
	adc	dh, [esi+0]

	adc	dl, [esi+3]
	adc	dh, [esi+2]

	adc	dl, [esi+5]
	adc	dh, [esi+4]

	adc	dl, [esi+7]
	adc	dh, [esi+6]

	adc	edx, 0
	add	esi, 8

	dec	ecx
	jnz	.loop

	adc	edx, 0

  .no_8:
	popf
	jnc	.no_4

	add	dl, [esi+1]
	adc	dh, [esi+0]

	adc	dl, [esi+3]
	adc	dh, [esi+2]

	adc	edx, 0
	add	esi, 4

  .no_4:
	popf
	jnc	.no_2

	add	dl, [esi+1]
	adc	dh, [esi+0]

	adc	edx, 0
	inc	esi
	inc	esi

  .no_2:
	popf
	jnc	.end

	add	dh, [esi+0]
	adc	edx, 0
  .end:
	ret

;-----------------------------------------------------------------
;
; checksum_2
;
;  This function calculates the final ip/tcp/udp checksum for you
;
;  IN:  edx = semi-checksum
;  OUT: dx = checksum (in INET byte order)
;
;-----------------------------------------------------------------
align 4
checksum_2:

	mov	ecx, edx
	shr	ecx, 16
	and	edx, 0xffff
	add	edx, ecx

	mov	ecx, edx
	shr	ecx, 16
	add	dx, cx
	test	dx, dx		; it seems that ZF is not set when CF is set :(
	not	dx
	jnz	.not_zero
	dec	dx
  .not_zero:
	xchg	dl, dh

	DEBUGF 1,"Checksum: %x\n", dx

	ret



;----------------------------------------------------------------
;
;  System function to work with network devices (73)
;
;----------------------------------------------------------------
align 4
sys_network:

	cmp	ebx, -1
	jne	@f

	mov	eax, [NET_RUNNING]
	jmp	.return

   @@:
	cmp	bh, MAX_NET_DEVICES		 ; Check if device number exists
	jge	.doesnt_exist

	mov	esi, ebx
	and	esi, 0x0000ff00
	shr	esi, 6

	cmp	dword [esi + NET_DRV_LIST], 0 ; check if driver is running
	je	.doesnt_exist

	test	bl, bl			 ; 0 = Get device type (ethernet/token ring/...)
	jnz	@f

	xor	eax, eax
	jmp	.return


  @@:
	dec	bl			; 1 = Get device name
	jnz	@f

	mov	esi, [esi + NET_DRV_LIST]
	mov	esi, [esi + NET_DEVICE.name]
	mov	edi, ecx

	mov	ecx, 64 ; max length
	repnz	movsb

	xor	eax, eax
	jmp	.return

  @@:

	dec	bl			; 2 = Reset the device
	jnz	@f

	mov	esi, [esi + NET_DRV_LIST]
	call	[esi + NET_DEVICE.reset]
	jmp	.return

  @@:

	dec	bl			; 3 = Stop driver for this device
	jnz	@f

	mov	esi, [esi + NET_DRV_LIST]
	call	[esi + NET_DEVICE.unload]
	jmp	.return

  @@:
	dec	bl			; 4 = Get driver pointer
	jnz	@f

	; ..;


  @@:
;  ...                                   ; 5 Get driver name

  .doesnt_exist:
	DEBUGF	1,"sys_network: invalid device/function specified!\n"
	mov	eax, -1

  .return:
	mov	[esp+28+4], eax
	ret


;----------------------------------------------------------------
;
;  System function to work with protocols  (75)
;
;----------------------------------------------------------------
align 4
sys_protocols:
	cmp	bh, MAX_NET_DEVICES		; Check if device number exists
	jge	.doesnt_exist

	mov	esi, ebx
	and	esi, 0x0000ff00
	shr	esi, 6				; now we have the device num * 4 in esi
	cmp	dword [esi + NET_DRV_LIST], 0	; check if driver is running
	je	.doesnt_exist

	push	.return 			; return address (we will be using jumps instead of calls)

	mov	eax, ebx			; set ax to protocol number
	shr	eax, 16 			;

	cmp	ax , IP_PROTO_IP
	je	IPv4_API

	cmp	ax , IP_PROTO_ICMP
	je	ICMP_API

	cmp	ax , IP_PROTO_UDP
	je	UDP_API

	cmp	ax , IP_PROTO_TCP
	je	TCP_API

	cmp	ax , ETHER_ARP
	je	ARP_API

	cmp	ax , 1337  ;;;;;
	je	ETH_API

	add	esp, 4				 ; if we reached here, no function was called, so we need to balance stack

  .doesnt_exist:
	DEBUGF	1,"sys_protocols: protocol %u doesnt exist on device %u!\n", ax, bh
	mov	eax, -1

  .return:
	mov	[esp+28+4], eax
	ret


__DEBUG_LEVEL__ equ __DEBUG_LEVEL_OLD__