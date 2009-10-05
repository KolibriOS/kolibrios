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

$Revision: 983 $

uglobal
	last_1sTick	db ?
	last_1hsTick	dd ?
endg

MAX_NET_DEVICES equ 16

MIN_EPHEMERAL_PORT equ 49152
MAX_EPHEMERAL_PORT equ 61000

ETHER		equ 1337
ETHER_ARP	equ 0x0608

;AF_UNSPEC       equ 0
;AF_UNIX         equ 1
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

; TCP opening modes
SOCKET_PASSIVE	equ 0
SOCKET_ACTIVE	equ 1

include "queue.inc"
include "ARP.inc"
include "IPv4.inc"
include "ethernet.inc"
include "socket.inc"
;include "tcp.inc"
include "udp.inc"
include "icmp.inc"

;-----------------------------------------------
;
; stack_init
;
;  This function calls all network init procedures
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------

align 4
stack_init:

	call	ETH_init
	call	IPv4_init
	call	ARP_init
	call	UDP_init
	call	ICMP_init
	call	socket_init

	mov	al, 0x0 		; set up 1s timer
	out	0x70, al
	in	al, 0x71
	mov	[last_1sTick], al

	ret



;-----------------------------------------------
;
; stack_handler
;
;  This function calls all network init procedures
;
;  IN:  /
;  OUT: /
;
;-----------------------------------------------

align 4
stack_handler:

    cmp     [ETH_RUNNING], 0
    je	    .exit

    call    ETH_handler 		; handle all queued ethernet packets
    call    ETH_send_queued

    ; Test for 10ms tick, call tcp timer
    mov     eax, [timer_ticks]
    cmp     eax, [last_1hsTick]
    je	    .exit

    mov     [last_1hsTick], eax
;    call    tcp_tx_handler

  .sec_tick:

    ; Test for 1 second event, call 1s timer functions
    mov     al, 0x0   ;second
    out     0x70, al
    in	    al, 0x71
    cmp     al, [last_1sTick]
    je	    .exit

    mov     [last_1sTick], al

    call    ARP_decrease_entry_ttls
    call    IPv4_decrease_fragment_ttls
;    call    tcp_tcb_handler

  .exit:
    ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Checksum [by Johnny_B]
;;  IN:
;;    buf_ptr=POINTER to buffer
;;    buf_size=SIZE of buffer
;;  OUT:
;;    AX=16-bit checksum
;;              Saves all used registers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
proc checksum_jb stdcall uses ebx esi ecx,\
     buf_ptr:DWORD, buf_size:DWORD

    xor     eax, eax
    xor     ebx, ebx  ;accumulator
    mov     esi, dword[buf_ptr]
    mov     ecx, dword[buf_size]
    shr     ecx, 1  ; ecx=ecx/2
    jnc     @f	    ; if CF==0 then size is even number
    mov     bh, byte[esi + ecx*2]
  @@:
    cld

  .loop:
    lodsw		;eax=word[esi],esi=esi+2
    xchg    ah,al	;cause must be a net byte-order
    add     ebx, eax
    loop    .loop

    mov     eax, ebx
    shr     eax, 16
    add     ax, bx
    not     ax

    ret
endp



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
;        je      TCP_API

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