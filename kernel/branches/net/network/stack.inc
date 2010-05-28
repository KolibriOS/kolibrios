;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  STACK.INC                                                      ;;
;;                                                                 ;;
;;  BASIC TCP/IP stack for KolibriOS                               ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;     based on the work of Mike Hibbett, mikeh@oceanfree.net      ;;
;;     but also Paolo Franchetti                                   ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

__DEBUG_LEVEL_OLD__  equ __DEBUG_LEVEL__
__DEBUG_LEVEL__ equ 1			; this sets the debug level for network part of kernel

uglobal
	last_1sTick	db ?
	last_1hsTick	dd ?
endg

MAX_NET_DEVICES 	equ 16
QUEUE_BEFORE_SENDING	equ 0		; 1 or 0 (enable or disable) currently only affects ethernet

MIN_EPHEMERAL_PORT equ 49152
MAX_EPHEMERAL_PORT equ 61000

ETHER		equ 1337		; TODO: find another value for this (how does it work in posix ?)
ETHER_ARP	equ 0x0608

AF_UNSPEC	equ 0
AF_UNIX 	equ 1
AF_INET4	equ 2
;AF_AX25         equ 3
;AF_IPX          equ 4
;AF_APPLETALK    equ 5
;AF_NETROM       equ 6
;AF_BRIDGE       equ 7
;AF_AAL5         equ 8
;AF_X25          equ 9
;AF_INET6        equ 10
;AF_MAX          equ 12

IP_PROTO_IP	equ 0
IP_PROTO_ICMP	equ 1
IP_PROTO_TCP	equ 6
IP_PROTO_UDP	equ 17

; Socket types
SOCK_STREAM	= 1
SOCK_DGRAM	= 2
SOCK_RAW	= 3

TCB_LISTEN		equ 1
TCB_SYN_SENT		equ 2
TCB_SYN_RECEIVED	equ 3
TCB_ESTABLISHED 	equ 4
TCB_FIN_WAIT_1		equ 5
TCB_FIN_WAIT_2		equ 6
TCB_CLOSE_WAIT		equ 7
TCB_CLOSING		equ 8
TCB_LAST_ACK		equ 9
TCB_TIMED_WAIT		equ 10
TCB_CLOSED		equ 11

TH_FIN			equ 1 shl 0
TH_SYN			equ 1 shl 1
TH_RST			equ 1 shl 2
TH_PUSH 		equ 1 shl 3
TH_ACK			equ 1 shl 4
TH_URG			equ 1 shl 5


macro inc_INET reg {

	add	byte [reg + 3], 1
	adc	byte [reg + 2], 0
	adc	byte [reg + 1], 0
	adc	byte [reg], 0

}


macro add_INET reg {
	add	byte [reg + 3], cl
	adc	byte [reg + 2], ch
	adc	byte [reg + 1], 0
	adc	byte [reg], 0
	rol	ecx, 16
	add	byte [reg + 1], cl
	adc	byte [reg], ch
	rol	ecx, 16
}


macro pseudo_random reg {

	add	reg, [esp]
	rol	reg, 5
	xor	reg, [timer_ticks]
	imul	reg, 214013
	xor	reg, 0xdeadbeef
	rol	reg, 9

	pushd	reg
	mov	word [esp], 0x8080 ; kernel heap start addr      (os_stack)
	xor	reg, [esp]
	add	esp, 4

}

include "queue.inc"
include "ARP.inc"
include "IPv4.inc"
include "ethernet.inc"
include "socket.inc"
include "tcp.inc"
include "udp.inc"
include "icmp.inc"

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

	call	ETH_init
	call	IPv4_init
	call	ARP_init
	call	UDP_init
	call	TCP_init
	call	ICMP_init
	call	socket_init

	mov	al, 0		      ; set up 1s timer
	out	0x70, al
	in	al, 0x71
	mov	[last_1sTick], al

	ret



;-----------------------------------------------------------------
;
; stack_handler
;
;  This function calls all network init procedures
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------------------------
align 4
stack_handler:

	cmp	[ETH_RUNNING], 0
	je	.exit

	; Test for 10ms tick
	mov	eax, [timer_ticks]
	cmp	eax, [last_1hsTick]
	je	.exit

	mov	[last_1hsTick], eax

	call	ETH_handler		    ; handle all queued ethernet packets
if QUEUE_BEFORE_SENDING
	call	ETH_send_queued
end if
	call	TCP_send_queued

  .sec_tick:

	; Test for 1 second tick
	mov	al, 0
	out	0x70, al
	in	al, 0x71
	cmp	al, [last_1sTick]
	je	.exit

	mov	[last_1sTick], al

	call	ARP_decrease_entry_ttls
	call	IPv4_decrease_fragment_ttls
	call	TCP_decrease_socket_ttls

  .exit:
	ret


;-----------------------------------------------------------------
;
; checksum_1
;
;  This is the first of two functions needed to calculate the TCP checksum.
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

	inc	ecx
	inc	ecx

  .no_2:
	popf
	jnc	.end

	add	dh, [esi+0]
	adc	edx, 0
  .end:
	ret





;IN: 12 bytes of pseudoheader pushed onto the stack
;    edx = start offset
;
; OUT: pseudochecksum in edx


align 4
checksum_pseudoheader:

	add	dl, [esp+5]
	adc	dh, [esp+4]

	adc	dl, [esp+7]
	adc	dh, [esp+6]

	adc	dl, [esp+9]
	adc	dh, [esp+8]

	adc	dl, [esp+11]
	adc	dh, [esp+10]

	adc	dl, [esp+13]
	adc	dh, [esp+12]

	adc	dl, [esp+15]
	adc	dh, [esp+14]

	adc	edx,0
	ret	12





align 4
checksum_ip_header:

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

	neg	word [esi+10]		; zero will stay zero so we jsut get the checksum
	add	word [esi+10], dx	;  , else we will get (new checksum - old checksum) in the end, wich should be 0 :)

	ret








align 4
checksum_udp:

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

	neg	word [esi+10]		; zero will stay zero so we jsut get the checksum
	add	word [esi+10], dx	;  , else we will get (new checksum - old checksum) in the end, wich should be 0 :)

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
	mov	eax, edx
	shr	eax, 16
	add	edx, eax

	not	dx
	jnz	.not_zero
	dec	dx
  .not_zero:
	xchg	dl, dh

	DEBUGF 1,"Checksum: %x\n",dx

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

	mov	eax, [ETH_RUNNING]
	jmp	.return

   @@:
	cmp	bh, MAX_NET_DEVICES		 ; Check if device number exists
	jge	.doesnt_exist

	mov	esi, ebx
	and	esi, 0x0000ff00
	shr	esi, 6

	cmp	dword [esi + ETH_DRV_LIST], 0 ; check if driver is running
	je	.doesnt_exist

	test	bl, bl			; 0 = Get device type (ethernet/token ring/...)
	jnz	@f
					 ; todo
	xor	eax, eax
	jmp	.return


  @@:
	dec	bl			; 1 = Get device name
	jnz	@f

	mov	esi, [esi + ETH_DRV_LIST]
	mov	esi, [esi + ETH_DEVICE.name]
	mov	edi, ecx

	mov	ecx, 64 ; max length
	repnz	movsb

	xor	eax, eax
	jmp	.return

  @@:

	dec	bl			; 2 = Reset the device
	jnz	@f

	mov	esi, [esi + ETH_DRV_LIST]
	call	[esi + ETH_DEVICE.reset]
	jmp	.return

  @@:

	dec	bl			; 3 = Stop driver for this device
	jnz	@f

	mov	esi, [esi + ETH_DRV_LIST]
	call	[esi + ETH_DEVICE.unload]
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
;  System Function To work with Protocols  (75)
;
;----------------------------------------------------------------
align 4
sys_protocols:
	cmp	bh, MAX_NET_DEVICES		; Check if device number exists
	jge	.doesnt_exist

	mov	esi, ebx
	and	esi, 0x0000ff00
	shr	esi, 6
	cmp	dword [esi + ETH_DRV_LIST], 0	; check if driver is running TODO: check other lists too
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

	cmp	ax , ETHER
	je	ETH_API

	add	esp, 4				 ; if we reached here, no function was called, so we need to balance stack

  .doesnt_exist:
	DEBUGF	1,"sys_protocols: protocol %u doesnt exist on device %u!\n",ax, bh
	mov	eax, -1

  .return:
	mov	[esp+28+4], eax
	ret


__DEBUG_LEVEL__ equ __DEBUG_LEVEL_OLD__