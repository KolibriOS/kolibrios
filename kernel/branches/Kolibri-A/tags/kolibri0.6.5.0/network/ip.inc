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

; IP underlying protocols numbers
PROTOCOL_ICMP     equ      1
PROTOCOL_TCP      equ      6
PROTOCOL_UDP      equ      17

struc IP_PACKET
{  .VersionAndIHL           db   ?  ;+00 - Version[0-3 bits] and IHL(header length)[4-7 bits]
   .TypeOfService           db   ?  ;+01
   .TotalLength             dw   ?  ;+02
   .Identification          dw   ?  ;+04
   .FlagsAndFragmentOffset  dw   ?  ;+06 - Flags[0-2] and FragmentOffset[3-15]
   .TimeToLive              db   ?  ;+08
   .Protocol                db   ?  ;+09
   .HeaderChecksum          dw   ?  ;+10
   .SourceAddress           dd   ?  ;+12
   .DestinationAddress      dd   ?  ;+16
   .DataOrOptional          dd   ?  ;+20
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
	shl     reg, 2
}


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
    je      .exit         ; Exit if no buffer available

    mov     [buffer_number], eax    ;save buffer number

    ; convert buffer pointer eax to the absolute address
    imul    eax, IPBUFFSIZE
    add     eax, IPbuffs

    mov     ebx, eax  ; ebx=pointer to IP_PACKET

    ; Validate the IP checksum
    mov     dx, word[ebx + IP_PACKET.HeaderChecksum]
    xchg    dh,dl          ; Get the checksum in intel format
  
    mov     [ebx + IP_PACKET.HeaderChecksum], word 0  ; clear checksum field - need to when
                                ; recalculating checksum
    ;  this needs two data pointers and two size #.
    ;  2nd pointer can be of length 0

    GET_IHL ecx, ebx + IP_PACKET.VersionAndIHL ;get packet length in ecx
    stdcall checksum_jb, ebx, ecx   ;buf_ptr, buf_size
    cmp     dx, ax

    mov     edx, ebx ; EDX (IP-BUFFER POINTER) WILL BE USED FOR *_rx HANDLERS BELOW!!!
    jnz     .dump  ;if CHECKSUM isn't valid then dump packet

    ; Validate the IP address, if it isn't broadcast
    mov     eax, [stack_ip]
    cmp     dword[ebx + IP_PACKET.DestinationAddress], eax
    je      @f

    ; If the IP address is 255.255.255.255, accept it
    ; - it is a broadcast packet, which we need for dhcp
    cmp     dword[ebx + IP_PACKET.DestinationAddress], 0xffffffff
    jne     .dump

  @@:
    mov     al, [ebx + IP_PACKET.VersionAndIHL]
    and     al, 0x0f  ;get IHL(header length)
    cmp     al, 0x05  ;if IHL!= 5*4(20 bytes)
    jnz     .dump     ;then dump it

    cmp     byte[ebx + IP_PACKET.TimeToLive], byte 0
    je      .dump     ;if TTL==0 then dump it

    mov     ax, word[ebx + IP_PACKET.FlagsAndFragmentOffset]
    and     ax, 0xFFBF   ;get flags
    cmp     ax, 0        ;if some flags was set then we dump this packet
    jnz     .dump        ;the flags should be used for fragmented packets

    ; Check the protocol, and call the appropriate handler
    ; Each handler will re-use or free the queue buffer as appropriate

    mov     al, [ebx + IP_PACKET.Protocol]

    cmp     al , PROTOCOL_TCP
    jne     .not_tcp
    mov     eax, dword[buffer_number]
    call    tcp_rx
    jmp     .exit

  .not_tcp:
    cmp     al, PROTOCOL_UDP
    jne     .not_udp
    mov     eax, dword[buffer_number]
    call    udp_rx
    jmp     .exit

  .not_udp:
    cmp     al , PROTOCOL_ICMP
    jne     .dump              ;protocol ain't supported

    ;GET_IHL ecx, ebx + IP_PACKET.VersionAndIHL ;get packet length in ecx
    mov     eax, dword[buffer_number]
    stdcall icmp_rx,eax,ebx,ecx  ;buffer_number,IPPacketBase,IPHeaderLength
    jmp     .exit


.dump:
    ; No protocol handler available, so
    ; silently dump the packet, freeing up the queue buffer

    inc     dword [dumped_rx_count]

    mov     eax, dword[buffer_number]
    call    freeBuff

  .exit:
    ret
endp

