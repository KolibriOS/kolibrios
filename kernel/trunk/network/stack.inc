;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  STACK.INC                                                   ;;
;;                                                              ;;
;;  TCP/IP stack for Menuet OS                                  ;;
;;                                                              ;;
;;  Version 0.7  4th July 2004                                  ;;
;;                                                              ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net            ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;; Version 0.7                                                  ;;
;;      Added a timer per socket to allow delays when rx window ;;
;;      gets below 1KB                                          ;;
;;                                                              ;;
;;10.01.2007 Bugfix for checksum function from Paolo Franchetti ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;*******************************************************************
;   Interface
;      The interfaces defined in ETHERNET.INC plus:
;      stack_init
;      stack_handler
;      app_stack_handler
;      app_socket_handler
;      checksum
;
;*******************************************************************

uglobal
StackCounters:
  dumped_rx_count     dd  0
  arp_tx_count:       dd  0
  arp_rx_count:       dd  0
  ip_rx_count:	      dd  0
  ip_tx_count:	      dd  0
endg

; socket buffers
SOCKETBUFFSIZE	   equ	      4096  ; state + config + buffer.
SOCKETHEADERSIZE   equ	      76+8+8  ; thus 4096 - SOCKETHEADERSIZE bytes data

;NUM_SOCKETS        equ        16    ; Number of open sockets supported. Was 20

; IPBUFF status values
BUFF_EMPTY	   equ	   0
BUFF_RX_FULL	   equ	   1
BUFF_ALLOCATED	   equ	   2
BUFF_TX_FULL	   equ	   3

NUM_IPBUFFERS	   equ	   20	 ; buffers allocated for TX/RX

NUMQUEUES	   equ	      4

EMPTY_QUEUE	   equ	      0
IPIN_QUEUE	   equ	      1
IPOUT_QUEUE	   equ	      2
NET1OUT_QUEUE	   equ	      3

NO_BUFFER	   equ	      0xFFFF
IPBUFFSIZE	   equ	      1500		  ; MTU of an ethernet packet
NUMQUEUEENTRIES    equ	      NUM_IPBUFFERS
NUMRESENDENTRIES    equ 	18		; Buffers for TCP resend packets

; These are the 0x40 function codes for application access to the stack
STACK_DRIVER_STATUS  equ   52
SOCKET_INTERFACE     equ   53


; 128KB allocated for the stack and network driver buffers and other
; data requirements
;stack_data_start     equ   0x700000
;eth_data_start       equ   0x700000
;stack_data           equ   0x704000
;stack_data_end       equ   0x71ffff

; 32 bit word
stack_config	     equ   stack_data

; 32 bit word - IP Address in network format
stack_ip	     equ   stack_data + 4

; 1 byte. 0 == inactive, 1 = active
ethernet_active      equ   stack_data + 9


; TODO :: empty memory area

; Address of selected socket
;sktAddr              equ   stack_data + 32
; Parameter to checksum routine - data ptr
checkAdd1	     equ   stack_data + 36
; Parameter to checksum routine - 2nd data ptr
checkAdd2	     equ   stack_data + 40
; Parameter to checksum routine - data size
checkSize1	     equ   stack_data + 44
; Parameter to checksum routine - 2nd data size
checkSize2	     equ   stack_data + 46
; result of checksum routine
checkResult	     equ   stack_data + 48

; holds the TCP/UDP pseudo header. SA|DA|0|prot|UDP len|
pseudoHeader	     equ   stack_data + 50

; receive and transmit IP buffer allocation
;sockets              equ   stack_data + 62
Next_free2 equ stack_data + 62;Next_free2           equ   sockets + (SOCKETBUFFSIZE * NUM_SOCKETS)
; 1560 byte buffer for rx / tx ethernet packets
Ether_buffer	     equ   Next_free2
Next_free3	     equ   Ether_buffer + 1518
last_1sTick	     equ   Next_free3
IPbuffs 	     equ   Next_free3 + 1
queues		     equ   IPbuffs + ( NUM_IPBUFFERS * IPBUFFSIZE )
queueList	     equ   queues + (2 * NUMQUEUES)
last_1hsTick	     equ   queueList + ( 2 * NUMQUEUEENTRIES )

;resendQ              equ   queueList + ( 2 * NUMQUEUEENTRIES )
;resendBuffer         equ    resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP
;                    equ    resendBuffer + ( IPBUFFSIZE * NUMRESENDENTRIES )



;resendQ             equ     0x770000
;resendBuffer        equ     resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP        ; XTODO: validate size
resendBuffer	    equ     resendQ + ( 8 * NUMRESENDENTRIES ) ; for TCP


uglobal
net_sockets rd 2
endg

; simple macro for memory set operation
macro _memset_dw adr,value,amount
{
	mov	edi, adr
	mov	ecx, amount
	if value = 0
		xor	eax, eax
	else
		mov	eax, value
	end if
	cld
	rep	stosd
}


; Below, the main network layer source code is included
;
include "queue.inc"
include "eth_drv/ethernet.inc"
include "ip.inc"
include "socket.inc"

;***************************************************************************
;   Function
;      stack_init
;
;   Description
;      Clear all allocated memory to zero. This ensures that
;       on startup, the stack is inactive, and consumes no resources
;       This is a kernel function, called prior to the OS main loop
;       in set_variables
;
;***************************************************************************

stack_init:
	; Init two address spaces with default values
	_memset_dw	stack_data_start, 0, 0x20000/4
	;_memset_dw      resendQ, 0xFFFFFFFF, NUMRESENDENTRIES   ; XTODO: validate size
	_memset_dw	resendQ, 0xFFFFFFFF, NUMRESENDENTRIES * 2

	mov	[net_sockets], 0
	mov	[net_sockets + 4], 0

	; Queries initialization
	call	queueInit

	; The following block sets up the 1s timer
	mov	al, 0x0
	out	0x70, al
	in	al, 0x71
	mov	[last_1sTick], al
ret



;***************************************************************************
;   Function
;      stack_handler
;
;   Description
;       The kernel loop routine for the stack
;       This is a kernel function, called in the main loop
;
;***************************************************************************
stack_handler:

    call    ethernet_driver
    call    ip_rx


    ; Test for 10ms tick, call tcp timer
    mov     eax, [timer_ticks] ;[0xfdf0]
    cmp     eax, [last_1hsTick]
    je	    sh_001

    mov     [last_1hsTick], eax
    call    tcp_tx_handler

sh_001:

    ; Test for 1 second event, call 1s timer functions
    mov     al, 0x0   ;second
    out     0x70, al
    in	    al, 0x71
    cmp     al, [last_1sTick]
    je	    sh_exit

    mov     [last_1sTick], al

    stdcall arp_table_manager, ARP_TABLE_TIMER, 0, 0
    call    tcp_tcb_handler

sh_exit:
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
    lodsw   ;eax=word[esi],esi=esi+2
    xchg    ah,al    ;cause must be a net byte-order
    add     ebx, eax
    loop    .loop

    mov     eax, ebx
    shr     eax, 16
    add     ax, bx
    not     ax

    ret
endp

;***************************************************************************
;   Function
;      checksum
;
;   Description
;       checkAdd1,checkAdd2, checkSize1, checkSize2, checkResult
;       Dont break anything; Most registers are used by the caller
;       This code is derived from the 'C' source, cksum.c, in the book
;       Internetworking with TCP/IP Volume II by D.E. Comer
;
;***************************************************************************


checksum:
    pusha
    mov     eax, [checkAdd1]
    xor     edx, edx		      ; edx is the accumulative checksum
    xor     ebx, ebx
    mov     cx, [checkSize1]
    shr     cx, 1
    jz	    cs1_1

cs1:
    mov     bh, [eax]
    mov     bl, [eax + 1]

    add     eax, 2
    add     edx, ebx

    loopw   cs1

cs1_1:
    and     word [checkSize1], 0x01
    jz	    cs_test2

    mov     bh, [eax]
    xor     bl, bl

    add     edx, ebx

cs_test2:
    mov     cx, [checkSize2]
    cmp     cx, 0
    jz	    cs_exit			; Finished if no 2nd buffer

    mov     eax, [checkAdd2]

    shr     cx, 1
    jz	    cs2_1

cs2:
    mov     bh, [eax]
    mov     bl, [eax + 1]

    add     eax, 2
    add     edx, ebx

    loopw   cs2

cs2_1:
    and     word [checkSize2], 0x01
    jz	    cs_exit

    mov     bh, [eax]
    xor     bl, bl

    add     edx, ebx

cs_exit:
    mov     ebx, edx

    shr     ebx, 16
    and     edx, 0xffff
    add     edx, ebx
    mov     eax, edx
    shr     eax, 16
    add     edx, eax
    not     dx

    mov     [checkResult], dx
    popa
    ret




;***************************************************************************
;   Function
;      app_stack_handler
;
;   Description
;       This is an application service, called by int 0x40, function 52
;       It provides application access to the network interface layer
;
;***************************************************************************
app_stack_handler:
    cmp     eax, 0
    jnz     not0
    ; Read the configuration word
    mov     eax, [stack_config]
    ret

not0:
    cmp     eax, 1
    jnz     not1
    ; read the IP address

    mov     eax, [stack_ip]
    ret

not1:
    cmp     eax, 2
    jnz     not2

    ; write the configuration word
    mov     [stack_config], ebx

    ; <Slip shouldn't be active anyway - thats an operational issue.>
    ; If ethernet now enabled, probe for the card, reset it and empty
    ; the packet buffer
    ; If all successfull, enable the card.
    ; If ethernet now disabled, set it as disabled. Should really
    ; empty the tcpip data area too.

    ; ethernet interface is '3' in ls 7 bits
    and     bl, 0x7f
    cmp     bl, 3

    je	     ash_eth_enable
    ; Ethernet isn't enabled, so make sure that the card is disabled
    mov     [ethernet_active], byte 0

    ret

ash_eth_enable:
    ; Probe for the card. This will reset it and enable the interface
    ; if found
    call    eth_probe
    cmp     eax, 0
    je	    ash_eth_done	    ; Abort if no hardware found

    mov     [ethernet_active], byte 1

ash_eth_done:
    ret

not2:
    cmp     eax, 3
    jnz     not3
    ; write the IP Address
    mov     [stack_ip], ebx
    ret

;old functions was deleted

not3:
    cmp     eax, 6
    jnz     not6

    ; Insert an IP packet into the stacks received packet queue
    call    stack_insert_packet
    ret

not6:
    cmp     eax, 7
    jnz     not7

    ; Test for any packets queued for transmission over the network

not7:
    cmp     eax, 8
    jnz     not8

    call    stack_get_packet
    ; Extract a packet queued for transmission by the network
    ret

not8:
    cmp     eax, 9
    jnz     not9

    ; read the gateway IP address

    mov     eax, [gateway_ip]
    ret

not9:
    cmp     eax, 10
    jnz     not10

    ; read the subnet mask

    mov     eax, [subnet_mask]
    ret

not10:
    cmp     eax, 11
    jnz     not11

    ; write the gateway IP Address
    mov     [gateway_ip], ebx

    ret

not11:
    cmp     eax, 12
    jnz     not12

    ; write the subnet mask
    mov     [subnet_mask], ebx


not12:
    cmp     eax, 13
    jnz     not13

    ; read the dns

    mov     eax, [dns_ip]
    ret

not13:
    cmp     eax, 14
    jnz     not14

    ; write the dns IP Address
    mov     [dns_ip], ebx

    ret

;<added by Frank Sommer>
not14:
	cmp	eax, 15
	jnz	not15

    ; in ebx we need 4 to read the last 2 bytes
	cmp	ebx, dword 4
	je	read

    ; or we need 0 to read the first 4 bytes
	cmp	ebx, dword 0
	jnz	param_error

    ; read MAC, returned (in mirrored byte order) in eax
read:
	mov	eax, [node_addr + ebx]
	jmp	@f

param_error:
	mov	eax, -1     ; params not accepted
@@:
	ret


; 0 -> arp_probe
; 1 -> arp_announce
; 2 -> arp_responce (not supported yet)

not15:					; ARP stuff
	cmp	eax, 16
	jnz	not16

	cmp	ebx, 0
	je	a_probe

	cmp	ebx, 1
	je	a_ann			; arp announce

;       cmp     ebx,2
;       jne     a_resp                  ; arp response

	jmp	param15_error


; arp probe, sender IP must be set to 0.0.0.0, target IP is set to address being probed
; ecx: pointer to target MAC, MAC should set to 0 by application
; edx: target IP
a_probe:
	push	dword [stack_ip]

	mov	edx, [stack_ip]
	mov	[stack_ip], dword 0
	mov	esi, ecx		; pointer to target MAC address
	call	arp_request

	pop	dword [stack_ip]
	jmp	@f

; arp announce, sender IP must be set to target IP
; ecx: pointer to target MAC
a_ann:
	mov	edx, [stack_ip]
	mov	esi, ecx		; pointer to target MAC address
	call	arp_request
	jmp	@f

param15_error:
	mov	eax, -1

@@:
	ret
;</added by Frank Sommer>
; modified by [smb]

;<added by Johnny_B>
; ARPTable manager interface
not16:
    cmp     eax, 17
    jnz     stack_driver_end

    ;see "proc arp_table_manager" for more details
    stdcall arp_table_manager,ebx,ecx,edx  ;Opcode,Index,Extra

    ret
;</added by Johnny_B>

stack_driver_end:
	ret



;***************************************************************************
;   Function
;      app_socket_handler
;
;   Description
;       This is an application service, called by int 0x40, function 53
;       It provides application access to stack socket services
;       such as opening sockets
;
;***************************************************************************
app_socket_handler:
    cmp     eax, 0
    jnz     nots0

    call    socket_open
    ret

nots0:
    cmp     eax, 1
    jnz     nots1

    call    socket_close
    ret

nots1:
    cmp     eax, 2
    jnz     nots2

    call    socket_poll
    ret

nots2:
    cmp     eax, 3
    jnz     nots3

    call    socket_read
    ret

nots3:
    cmp     eax, 4
    jnz     nots4

    call    socket_write
    ret

nots4:
    cmp     eax, 5
    jnz     nots5

    call    socket_open_tcp
    ret

nots5:
    cmp     eax, 6
    jnz     nots6

    call    socket_status
    ret

nots6:
    cmp     eax, 7
    jnz     nots7

    call    socket_write_tcp
    ret

nots7:
    cmp     eax, 8
    jnz     nots8

    call    socket_close_tcp
    ret

nots8:
    cmp     eax, 9
    jnz     nots9

    call    is_localport_unused
    ret

nots9:
    cmp     eax, 10
    jnz     nots10

    mov     eax,dword[drvr_cable]
    test    eax,eax
    jnz     @f		      ; if function is not implented, return -1
    mov     al,-1
    ret

   @@:
    call    dword[drvr_cable]
    ret


nots10:
    cmp     eax, 11
    jnz     nots11

    call    socket_read_packet
    ret

nots11:
    cmp     eax, 254
    jnz     notdump

    ret

notdump:
    cmp     eax, 255
    jnz     notsdebug

    ; This sub function allows access to debugging information on the stack
    ; ebx holds the request:
    ;  100 : return length of empty queue
    ;  101 : return length of IPOUT QUEUE
    ;  102 : return length of IPIN QUEUE
    ;  103 : return length of NET1OUT QUEUE
    ; 200 : return # of ARP entries
    ; 201 : return size of ARP table ( max # entries )
    ; 202 : select ARP table entry #
    ; 203 : return IP of selected table entry
    ; 204 : return High 4 bytes of MAC address of selected table entry
    ; 205 : return low  2 bytes of MAC address of selected table entry
    ; 206 : return status word of selected table entry
    ; 207 : return Time to live of selected table entry


    ;  2 : return number of IP packets received
    ;  3 : return number of packets transmitted
    ;  4 : return number of received packets dumped
    ;  5 : return number of arp packets received
    ;  6 : return status of packet driver
    ;      ( 0 == not active, FFFFFFFF = successful )

    call    stack_internal_status
    ret

notsdebug:
    ; Invalid Option
    ret


uglobal
  ARPTmp:
  times 14 db 0
endg

;***************************************************************************
;   Function
;      stack_internal_status
;
;   Description
;       Returns information about the internal status of the stack
;       This is only useful for debugging
;       It works with the ethernet driver
;       sub function in ebx
;       return requested data in eax
;
;***************************************************************************
stack_internal_status:
    cmp     ebx, 100
    jnz     notsis100

    ;  100 : return length of EMPTY QUEUE
    mov     ebx, EMPTY_QUEUE
    call    queueSize
    ret

notsis100:
    cmp     ebx, 101
    jnz     notsis101

    ;  101 : return length of IPOUT QUEUE
    mov     ebx, IPOUT_QUEUE
    call    queueSize
    ret

notsis101:
    cmp     ebx, 102
    jnz     notsis102

    ;  102 : return length of IPIN QUEUE
    mov     ebx, IPIN_QUEUE
    call    queueSize
    ret

notsis102:
    cmp     ebx, 103
    jnz     notsis103

    ;  103 : return length of NET1OUT QUEUE
    mov     ebx, NET1OUT_QUEUE
    call    queueSize
    ret

notsis103:
    cmp     ebx, 200
    jnz     notsis200

    ; 200 : return num entries in arp table
    movzx   eax, byte [NumARP]
    ret

notsis200:
    cmp     ebx, 201
    jnz     notsis201

    ; 201 : return arp table size
    mov     eax, 20 ; ARP_TABLE_SIZE
    ret

notsis201:
    cmp     ebx, 202
    jnz     notsis202

    ; 202 - read the requested table entry
    ; into a temporary buffer
    ; ecx holds the entry number

    mov     eax, ecx
    mov     ecx, 14 ; ARP_ENTRY_SIZE
    mul     ecx

    mov     ecx, [eax + ARPTable]
    mov     [ARPTmp], ecx
    mov     ecx, [eax + ARPTable+4]
    mov     [ARPTmp+4], ecx
    mov     ecx, [eax + ARPTable+8]
    mov     [ARPTmp+8], ecx
    mov     cx, [eax + ARPTable+12]
    mov     [ARPTmp+12], cx
    ret

notsis202:
    cmp     ebx, 203
    jnz     notsis203

    ; 203 - return IP address
    mov     eax, [ARPTmp]
    ret

notsis203:
    cmp     ebx, 204
    jnz     notsis204

    ; 204 - return MAC high dword
    mov     eax, [ARPTmp+4]
    ret

notsis204:
    cmp     ebx, 205
    jnz     notsis205

    ; 205 - return MAC ls word
    movzx   eax, word [ARPTmp+8]
    ret

notsis205:
    cmp     ebx, 206
    jnz     notsis206

    ; 206 - return status word
    movzx   eax, word [ARPTmp+10]
    ret

notsis206:
    cmp     ebx, 207
    jnz     notsis207

    ; 207 - return ttl word
    movzx   eax, word [ARPTmp+12]
    ret

notsis207:
    cmp     ebx, 2
    jnz     notsis2

    ;  2 : return number of IP packets received
    mov     eax, [ip_rx_count]
    ret

notsis2:
    cmp     ebx, 3
    jnz     notsis3

    ;  3 : return number of packets transmitted
    mov     eax, [ip_tx_count]
    ret

notsis3:
    cmp     ebx, 4
    jnz     notsis4

    ;  4 : return number of received packets dumped
    mov     eax, [dumped_rx_count]
    ret

notsis4:
    cmp     ebx, 5
    jnz     notsis5

    ;  5 : return number of arp packets received
    mov     eax, [arp_rx_count]
    ret

notsis5:
    cmp     ebx, 6
    jnz     notsis6

    ;  6 : return status of packet driver
    ;  ( 0 == not active, FFFFFFFF = successful )
    mov     eax, [eth_status]
    ret

notsis6:
    xor     eax, eax
    ret



;***************************************************************************
;   Function
;      stack_get_packet
;
;   Description
;       extracts an IP packet from the NET1 output queue
;       and sends the data to the calling process
;       pointer to data in edx
;       returns number of bytes read in eax
;
;***************************************************************************
stack_get_packet:
    ; Look for a buffer to tx
    mov     eax, NET1OUT_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    sgp_non_exit	    ; Exit if no buffer available

    push    eax 		    ; Save buffer number for freeing at end

    push    edx
    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs
    pop     edx

    push    eax 		    ; save address of IP data
    ; Get the address of the callers data
    mov     edi,[TASK_BASE]
    add     edi,TASKDATA.mem_start
    add     edx,[edi]
    mov     edi, edx
    pop     eax

    mov     ecx, 1500		; should get the actual number of bytes to write
    mov     esi, eax
    cld
    rep     movsb		; copy the data across

    ; And finally, return the buffer to the free queue
    pop     eax
    call    freeBuff

    mov     eax, 1500
    ret

sgp_non_exit:
    xor     eax, eax
    ret



;***************************************************************************
;   Function
;      stack_insert_packet
;
;   Description
;       writes an IP packet into the stacks receive queue
;       # of bytes to write in ecx
;       pointer to data in edx
;       returns 0 in eax ok, -1 == failed
;
;***************************************************************************
stack_insert_packet:

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    sip_err_exit

    push    eax

    ; save the pointers to the data buffer & size
    push    edx
    push    ecx

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edx, eax

    ; So, edx holds the IPbuffer ptr

    pop     ecx 		    ; count of bytes to send
    mov     ebx, ecx		    ; need the length later
    pop     eax 		    ; get callers ptr to data to send

    ; Get the address of the callers data
    mov     edi,[TASK_BASE]
    add     edi,TASKDATA.mem_start
    add     eax,[edi]
    mov     esi, eax

    mov     edi, edx
    cld
    rep     movsb		; copy the data across

    pop     ebx

    mov     eax, IPIN_QUEUE
    call    queue

    inc     dword [ip_rx_count]

    mov     eax, 0
    ret

sip_err_exit:
    mov     eax, 0xFFFFFFFF
    ret

