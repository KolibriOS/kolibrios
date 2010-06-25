;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  IP.INC                                                         ;;
;;                                                                 ;;
;;  IP Processes for Menuet OS  TCP/IP stack                       ;;
;;                                                                 ;;
;;  Version 0.3  29 August 2002                                    ;;
;;                                                                 ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net               ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; IP underlying protocols numbers
PROTOCOL_ICMP	  equ	   1
PROTOCOL_TCP	  equ	   6
PROTOCOL_UDP	  equ	   17

struc IP_PACKET
{  .VersionAndIHL	    db	 ?  ;+00 - Version[0-3 bits] and IHL(header length)[4-7 bits]
   .TypeOfService	    db	 ?  ;+01
   .TotalLength 	    dw	 ?  ;+02
   .Identification	    dw	 ?  ;+04
   .FlagsAndFragmentOffset  dw	 ?  ;+06 - Flags[0-2] and FragmentOffset[3-15]
   .TimeToLive		    db	 ?  ;+08
   .Protocol		    db	 ?  ;+09
   .HeaderChecksum	    dw	 ?  ;+10
   .SourceAddress	    dd	 ?  ;+12
   .DestinationAddress	    dd	 ?  ;+16
   .DataOrOptional	    dd	 ?  ;+20
}

virtual at 0
  IP_PACKET IP_PACKET
end virtual


;*******************************************************************
;   Interface
;
;       ip_rx       processes all packets received by the network layer
;                   It calls the appropriate protocol handler
;
;
;
;*******************************************************************


;
;   IP Packet after reception - Normal IP packet format
;
;           0               1               2               3
;    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;0  |Version|  IHL  |Type of Service|       Total Length            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;4  |         Identification        |Flags|      Fragment Offset    |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;8  |  Time to Live |    Protocol   |         Header Checksum       |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;12 |                       Source Address                          |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;16 |                    Destination Address                        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;20 |      Data                                                     |
;   +-+-+-..........                                               -+
;
;
;    [smb] attention! according to RFC 791 IP packet may have 'options' sections,
; so we can't simply think, that data have offset 20. We must calculate offset from
; IHL field
;
macro	GET_IHL reg, header_addr
{
	movzx	reg, byte [header_addr]
	
	; we need 4-7 bits, so....
	and	reg, 0x0000000F
	
	; IHL keeps number of octets, so we need to << 2 'reg'
	shl	reg, 2
}


include "tcp.inc"
include "udp.inc"
include "icmp.inc"

;***************************************************************************
;   Function
;      ip_rx
;
;   Description
;       This is a kernel function, called by stack_handler
;       Processes all IP-packets received by the network layer
;       It calls the appropriate protocol handler
;
;***************************************************************************
proc ip_rx stdcall
local buffer_number dd ?

    ; Look for a buffer to tx
    mov     eax, IPIN_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    .exit	  ; Exit if no buffer available

    mov     [buffer_number], eax    ;save buffer number

    ; convert buffer pointer eax to the absolute address
    imul    eax, IPBUFFSIZE
    add     eax, IPbuffs

    mov     ebx, eax  ; ebx=pointer to IP_PACKET

;        DEBUGF  1, "K : ip_rx - proto: %u\n", [ebx + IP_PACKET.Protocol]:1

    ; Validate the IP checksum
    mov     dx, word[ebx + IP_PACKET.HeaderChecksum]
    xchg    dh,dl	   ; Get the checksum in intel format
  
    mov     [ebx + IP_PACKET.HeaderChecksum], 0  ; clear checksum field - need to when
				; recalculating checksum
    ;  this needs two data pointers and two size #.
    ;  2nd pointer can be of length 0

    GET_IHL ecx, ebx + IP_PACKET.VersionAndIHL ;get packet length in ecx
    stdcall checksum_jb, ebx, ecx   ;buf_ptr, buf_size
    cmp     dx, ax

;        DEBUGF  1, "K : ip_rx - checksums: %x - %x\n", dx, ax

    jnz     .dump.1  ;if CHECKSUM isn't valid then dump packet
    mov     edx, ebx ; EDX (IP-BUFFER POINTER) WILL BE USED FOR *_rx HANDLERS BELOW!!!

;        DEBUGF  1, "K : ip_rx - dest: %x - %x\n", [ebx + IP_PACKET.DestinationAddress], [stack_ip]

    ; Validate the IP address, if it isn't broadcast
    mov     eax, [stack_ip]
    cmp     dword[ebx + IP_PACKET.DestinationAddress], eax
    je	    @f

    ; If the IP address is 255.255.255.255, accept it
    ; - it is a broadcast packet, which we need for dhcp

	mov	eax, [ebx + IP_PACKET.DestinationAddress]
	cmp	eax, 0xffffffff
	je	@f
	mov	ecx, [stack_ip]
	and	eax, [subnet_mask]
	and	ecx, [subnet_mask]
	cmp	eax, ecx
	jne	.dump.2
	mov	eax, [ebx + IP_PACKET.DestinationAddress]
	or	eax, [subnet_mask]
	cmp	eax, 0xffffffff
	jne	.dump.2

  @@:
    mov     al, [ebx + IP_PACKET.VersionAndIHL]
    and     al, 0x0f  ;get IHL(header length)
    cmp     al, 0x05  ;if IHL!= 5*4(20 bytes)
;        DEBUGF  1, "K : ip_rx - ihl: %x - 05\n", al
    jnz     .dump.3	;then dump it

;        DEBUGF  1, "K : ip_rx - ttl: %x - 00\n", [ebx + IP_PACKET.TimeToLive]:2

    cmp     [ebx + IP_PACKET.TimeToLive], 0
    je	    .dump.4	;if TTL==0 then dump it

    mov     ax, [ebx + IP_PACKET.FlagsAndFragmentOffset]
    and     ax, 0xFFBF	 ;get flags
;        DEBUGF  1, "K : ip_rx - flags: %x - 0000\n", ax
    cmp     ax, 0	 ;if some flags was set then we dump this packet
    jnz     .dump.5	   ;the flags should be used for fragmented packets

    ; Check the protocol, and call the appropriate handler
    ; Each handler will re-use or free the queue buffer as appropriate

    mov     al, [ebx + IP_PACKET.Protocol]

    cmp     al , PROTOCOL_TCP
    jne     .not_tcp
;    DEBUGF  1,"K : ip_rx - TCP packet\n"
    mov     eax, dword[buffer_number]
    call    tcp_rx
    jmp     .exit

  .not_tcp:
    cmp     al, PROTOCOL_UDP
    jne     .not_udp
;    DEBUGF  1,"K : ip_rx - UDP packet\n"
    mov     eax, dword[buffer_number]
    call    udp_rx
    jmp     .exit

  .not_udp:
    cmp     al, PROTOCOL_ICMP
    jne     .dump.6		 ;protocol ain't supported

;    DEBUGF  1,"K : ip_rx - ICMP packet\n"
    ;GET_IHL ecx, ebx + IP_PACKET.VersionAndIHL ;get packet length in ecx
    mov     eax, dword[buffer_number]
    stdcall icmp_rx,eax,ebx,ecx  ;buffer_number,IPPacketBase,IPHeaderLength
    jmp     .exit


  .dump.1:
	DEBUGF	1, "K : ip_rx - dumped (checksum: 0x%x-0x%x)\n", dx, ax
	jmp	.dump.x

  .dump.2:
	DEBUGF	1, "K : ip_rx - dumped (ip: %u.%u.%u.%u)\n", [ebx + IP_PACKET.DestinationAddress + 0]:1, [ebx + IP_PACKET.DestinationAddress + 1]:1, [ebx + IP_PACKET.DestinationAddress + 2]:1, [ebx + IP_PACKET.DestinationAddress + 3]:1
	jmp	.dump.x

  .dump.3:
	DEBUGF	1, "K : ip_rx - dumped (ihl: %u)\n", al
	jmp	.dump.x

  .dump.4:
	DEBUGF	1, "K : ip_rx - dumped (ttl: %u)\n", [ebx + IP_PACKET.TimeToLive]
	jmp	.dump.x

  .dump.5:
	DEBUGF	1, "K : ip_rx - dumped (flags: 0x%x)\n", ax
	jmp	.dump.x

  .dump.6:
	DEBUGF	1, "K : ip_rx - dumped (proto: %u)\n", [ebx + IP_PACKET.Protocol]:1

  .dump.x:
    inc     dword[dumped_rx_count]
    mov     eax, [buffer_number]
    call    freeBuff

  .exit:
    ret
endp

