;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  STACK.INC                                                   ;;
;;                                                              ;;
;;  TCP/IP stack for Menuet OS                                  ;;
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
  arp_tx_count:
                      dd  0
  arp_rx_count:
                      dd  0
  ip_rx_count:
                      dd  0
  ip_tx_count:
                      dd  0
endg

; socket buffers
SOCKETBUFFSIZE     equ        4096  ; state + config + buffer.
SOCKETHEADERSIZE   equ        SOCKET.rxData ; thus 4096 - SOCKETHEADERSIZE bytes data

;NUM_SOCKETS        equ        16    ; Number of open sockets supported. Was 20

; IPBUFF status values
BUFF_EMPTY         equ     0
BUFF_RX_FULL       equ     1
BUFF_ALLOCATED     equ     2
BUFF_TX_FULL       equ     3

NUM_IPBUFFERS      equ     20    ; buffers allocated for TX/RX

NUMQUEUES          equ        4

EMPTY_QUEUE        equ        0
IPIN_QUEUE         equ        1
IPOUT_QUEUE        equ        2
NET1OUT_QUEUE      equ        3

NO_BUFFER          equ        0xFFFF
IPBUFFSIZE         equ        1500                ; MTU of an ethernet packet
NUMQUEUEENTRIES    equ        NUM_IPBUFFERS
NUMRESENDENTRIES    equ         18              ; Buffers for TCP resend packets

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
stack_config         equ   stack_data

; 32 bit word - IP Address in network format
stack_ip             equ   stack_data + 4

; 1 byte. 0 == inactive, 1 = active
ethernet_active      equ   stack_data + 9


; TODO :: empty memory area

; Address of selected socket
;sktAddr              equ   stack_data + 32
; Parameter to checksum routine - data ptr
checkAdd1            equ   stack_data + 36
; Parameter to checksum routine - 2nd data ptr
checkAdd2            equ   stack_data + 40
; Parameter to checksum routine - data size
checkSize1           equ   stack_data + 44
; Parameter to checksum routine - 2nd data size
checkSize2           equ   stack_data + 46
; result of checksum routine
checkResult          equ   stack_data + 48

; holds the TCP/UDP pseudo header. SA|DA|0|prot|UDP len|
pseudoHeader         equ   stack_data + 50

; receive and transmit IP buffer allocation
;sockets              equ   stack_data + 62
Next_free2 equ stack_data + 62;Next_free2           equ   sockets + (SOCKETBUFFSIZE * NUM_SOCKETS)
; 1560 byte buffer for rx / tx ethernet packets
Ether_buffer         equ   Next_free2
Next_free3           equ   Ether_buffer + 1518
last_1sTick          equ   Next_free3
IPbuffs              equ   Next_free3 + 1
queues               equ   IPbuffs + ( NUM_IPBUFFERS * IPBUFFSIZE )
queueList            equ   queues + (2 * NUMQUEUES)
last_1hsTick         equ   queueList + ( 2 * NUMQUEUEENTRIES )

;resendQ              equ   queueList + ( 2 * NUMQUEUEENTRIES )
;resendBuffer         equ    resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP
;                    equ    resendBuffer + ( IPBUFFSIZE * NUMRESENDENTRIES )



;resendQ             equ     0x770000
;resendBuffer        equ     resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP        ; XTODO: validate size
resendBuffer        equ     resendQ + ( 8 * NUMRESENDENTRIES ) ; for TCP


uglobal
net_sockets rd 2
endg

; simple macro for memory set operation
macro _memset_dw adr,value,amount
{
        mov     edi, adr
        mov     ecx, amount
        if value = 0
        xor     eax, eax
        else
        mov     eax, value
        end if
        cld
        rep stosd
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
        _memset_dw      stack_data_start, 0, 0x20000/4
        _memset_dw      resendQ, 0, NUMRESENDENTRIES * 2

        mov     [net_sockets], 0
        mov     [net_sockets + 4], 0

        ; Queries initialization
        call    queueInit

        ; The following block sets up the 1s timer
        mov     al, 0x0
        out     0x70, al
        in      al, 0x71
        mov     [last_1sTick], al
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
align 4
stack_handler:

        call    ethernet_driver
        call    ip_rx


    ; Test for 10ms tick, call tcp timer
        mov     eax, [timer_ticks];[0xfdf0]
        cmp     eax, [last_1hsTick]
        je      sh_001

        mov     [last_1hsTick], eax
        call    tcp_tx_handler

sh_001:

    ; Test for 1 second event, call 1s timer functions
        mov     al, 0x0;second
        out     0x70, al
        in      al, 0x71
        cmp     al, [last_1sTick]
        je      sh_exit

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
        xor     ebx, ebx;accumulator
        mov     esi, dword[buf_ptr]
        mov     ecx, dword[buf_size]
        shr     ecx, 1; ecx=ecx/2
        jnc     @f  ; if CF==0 then size is even number
        mov     bh, byte[esi + ecx*2]
  @@:
        cld

  .loop:
        lodsw   ;eax=word[esi],esi=esi+2
        xchg    ah, al;cause must be a net byte-order
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
        xor     edx, edx              ; edx is the accumulative checksum
        xor     ebx, ebx
        mov     cx, [checkSize1]
        shr     cx, 1
        jz      cs1_1

cs1:
        mov     bh, [eax]
        mov     bl, [eax + 1]

        add     eax, 2
        add     edx, ebx

        loopw   cs1

cs1_1:
        and     word [checkSize1], 0x01
        jz      cs_test2

        mov     bh, [eax]
        xor     bl, bl

        add     edx, ebx

cs_test2:
        mov     cx, [checkSize2]
        cmp     cx, 0
        jz      cs_exit                 ; Finished if no 2nd buffer

        mov     eax, [checkAdd2]

        shr     cx, 1
        jz      cs2_1

cs2:
        mov     bh, [eax]
        mov     bl, [eax + 1]

        add     eax, 2
        add     edx, ebx

        loopw   cs2

cs2_1:
        and     word [checkSize2], 0x01
        jz      cs_exit

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
iglobal
align 4
f52call:
        dd      app_stack_handler.00
        dd      app_stack_handler.01
        dd      app_stack_handler.02
        dd      app_stack_handler.03
        dd      app_stack_handler.fail  ;04
        dd      app_stack_handler.fail  ;05
        dd      stack_insert_packet     ;app_stack_handler.06
        dd      app_stack_handler.fail  ;07
        dd      stack_get_packet        ;app_stack_handler.08
        dd      app_stack_handler.09
        dd      app_stack_handler.10
        dd      app_stack_handler.11
        dd      app_stack_handler.12
        dd      app_stack_handler.13
        dd      app_stack_handler.14
        dd      app_stack_handler.15
endg
app_stack_handler:
;in ebx,ecx
;out eax
        cmp     ebx, 15
        ja      .fail                   ;if more than 15 then exit

        jmp     dword [f52call+ebx*4]


.00:
; Read the configuration word
        mov     eax, [stack_config]
        ret

.01:
; read the IP address
        mov     eax, [stack_ip]
        ret

.02:
; write the configuration word
        mov     [stack_config], ecx

; <Slip shouldn't be active anyway - thats an operational issue.>
; If ethernet now enabled, probe for the card, reset it and empty
; the packet buffer
; If all successfull, enable the card.
; If ethernet now disabled, set it as disabled. Should really
; empty the tcpip data area too.

; ethernet interface is '3' in ls 7 bits
        and     cl, 0x7f
        cmp     cl, 3
        je      ash_eth_enable
; Ethernet isn't enabled, so make sure that the card is disabled
        mov     [ethernet_active], byte 0
        ret

.03:
; write the IP Address
        mov     [stack_ip], ecx
        ret
;old functions was deleted
;.06:
; Insert an IP packet into the stacks received packet queue
;       call    stack_insert_packet
;       ret

; Test for any packets queued for transmission over the network

;.08:
;       call    stack_get_packet
; Extract a packet queued for transmission by the network
;       ret

.09:
; read the gateway IP address
        mov     eax, [gateway_ip]
        ret

.10:
; read the subnet mask
        mov     eax, [subnet_mask]
        ret
.11:
; write the gateway IP Address
        mov     [gateway_ip], ecx
        ret

.12:
; write the subnet mask
        mov     [subnet_mask], ecx
        ret

.13:
; read the dns
        mov     eax, [dns_ip]
        ret

.14:
; write the dns IP Address
        mov     [dns_ip], ecx
        ret

.15:
;<added by Frank Sommer>
; in ecx we need 4 to read the last 2 bytes
; or we need 0 to read the first 4 bytes
        cmp     ecx, 4
        ja      .param_error

; read MAC, returned (in mirrored byte order) in eax
        mov     eax, [node_addr + ecx]
        ret

.param_error:
        or      eax, -1         ; params not accepted
        ret

.16:
; 0 -> arp_probe
; 1 -> arp_announce
; 2 -> arp_responce (not supported yet)
        test    ecx, ecx
        je      a_probe

        dec     ebx
        jz      a_ann                   ; arp announce
.fail:
        or      eax, -1
        ret

;       cmp     ebx,2
;       jne     a_resp                  ; arp response

; arp probe, sender IP must be set to 0.0.0.0, target IP is set to address being probed
; ecx: pointer to target MAC, MAC should set to 0 by application
; edx: target IP
a_probe:
        push    dword [stack_ip]

        mov     edx, [stack_ip]
        and     [stack_ip], dword 0
        mov     esi, ecx                ; pointer to target MAC address
        call    arp_request

        pop     dword [stack_ip]
        ret

; arp announce, sender IP must be set to target IP
; ecx: pointer to target MAC
a_ann:
        mov     edx, [stack_ip]
        mov     esi, ecx                ; pointer to target MAC address
        call    arp_request
        ret

.17:
;</added by Frank Sommer>
; modified by [smb]

;<added by Johnny_B>
; ARPTable manager interface
    ;see "proc arp_table_manager" for more details
        stdcall arp_table_manager, ecx, edx, esi;Opcode,Index,Extra
        ret
;</added by Johnny_B>

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ash_eth_enable:
; Probe for the card. This will reset it and enable the interface
; if found
        call    eth_probe
        test    eax, eax
        jz      ash_eth_done            ; Abort if no hardware found

        mov     [ethernet_active], byte 1
ash_eth_done:
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
iglobal
align 4
f53call:
        dd      socket_open                     ;00
        dd      socket_close                    ;01
        dd      socket_poll                     ;02
        dd      socket_read                     ;03
        dd      socket_write                    ;04
        dd      socket_open_tcp                 ;05
        dd      socket_status                   ;06
        dd      socket_write_tcp                ;07
        dd      socket_close_tcp                ;08
        dd      is_localport_unused             ;09
        dd      app_socket_handler.10
        dd      socket_read_packet              ;11
endg

app_socket_handler:
;in ebx,ecx,edx,wsi
;out eax
        cmp     eax, 255
        je      stack_internal_status

        cmp     eax, 11
        ja      .fail                   ;if more than 15 then exit

        jmp     dword [f53call+eax*4]

.10:
        mov     eax, dword[drvr_cable]
        test    eax, eax
        jnz     @f                      ; if function is not implented, return -1
        or      al, -1
        ret
@@:
        jmp     dword[drvr_cable]

.fail:
        or      eax, -1
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
; This sub function allows access to debugging information on the stack
; ecx holds the request:
; 100 : return length of empty queue
; 101 : return length of IPOUT QUEUE
; 102 : return length of IPIN QUEUE
; 103 : return length of NET1OUT QUEUE
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
;  ( 0 == not active, FFFFFFFF = successful )


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
        mov     eax, 20; ARP_TABLE_SIZE
        ret

notsis201:
        cmp     ebx, 202
        jnz     notsis202

    ; 202 - read the requested table entry
    ; into a temporary buffer
    ; ecx holds the entry number

        mov     eax, ecx
        mov     ecx, 14; ARP_ENTRY_SIZE
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
        je      sgp_non_exit        ; Exit if no buffer available

        push    eax                 ; Save buffer number for freeing at end

        push    edx
    ; convert buffer pointer eax to the absolute address
        mov     ecx, IPBUFFSIZE
        mul     ecx
        add     eax, IPbuffs
        pop     edx

        push    eax                 ; save address of IP data
    ; Get the address of the callers data
        mov     edi, [TASK_BASE]
        add     edi, TASKDATA.mem_start
        add     edx, [edi]
        mov     edi, edx
        pop     eax

        mov     ecx, 1500       ; should get the actual number of bytes to write
        mov     esi, eax
        cld
        rep movsb               ; copy the data across

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
        je      sip_err_exit

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

        pop     ecx                 ; count of bytes to send
        mov     ebx, ecx            ; need the length later
        pop     eax                 ; get callers ptr to data to send

    ; Get the address of the callers data
        mov     edi, [TASK_BASE]
        add     edi, TASKDATA.mem_start
        add     eax, [edi]
        mov     esi, eax

        mov     edi, edx
        cld
        rep movsb               ; copy the data across

        pop     ebx

        mov     eax, IPIN_QUEUE
        call    queue

        inc     dword [ip_rx_count]

        mov     eax, 0
        ret

sip_err_exit:
        mov     eax, 0xFFFFFFFF
        ret

