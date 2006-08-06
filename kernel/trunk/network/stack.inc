;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  STACK.INC                                                      ;;
;;                                                                 ;;
;;  TCP/IP stack for Menuet OS                                     ;;
;;                                                                 ;;
;;  Version 0.7  4th July 2004                                     ;;
;;                                                                 ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net               ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;; Version 0.7                                                     ;;
;;         Added a timer per socket to allow delays when rx window ;;
;;         gets below 1KB                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


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



;
;   IP Packet after reception - Normal IP packet format
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
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
;   |      Data                                                     |
;   +-+-+-..........                                               -+


;   TCP Payload ( Data field in IP datagram )
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;20 |          Source Port          |       Destination Port        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;24 |                        Sequence Number                        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;28 |                    Acknowledgment Number                      |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;32 |  Data |           |U|A|P|R|S|F|                               |
;   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
;   |       |           |G|K|H|T|N|N|                               |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;36 |           Checksum            |         Urgent Pointer        |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;40 |                    Options                    |    Padding    |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                             data


;
;   UDP Payload ( Data field in IP datagram )
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |       Source Port             |      Destination Port         |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | Length ( UDP Header + Data )  |           Checksum            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |       UDP Data                                                |
;   +-+-+-..........                                               -+
;


;
;  Socket Descriptor + Buffer
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                    Status ( of this buffer )                  |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |  Application Process ID                                       |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                  Local IP Address                             |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | Local IP Port                 | Unused ( set to 0 )           |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   |                  Remote IP Address                            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;   | Remote IP Port                | Unused ( set to 0 )           |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 24|   Rx Data Count                                   INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 28|                 TCB STATE                         INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 32|   TCB Timer (seconds)                             INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 36| ISS (Inital Sequence # used by this connection )   INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 40| IRS ( Inital Receive Sequence # )                  INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 44| SND.UNA  Seq # of unack'ed sent packets            INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 48| SND.NXT  Next send seq # to use                    INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 52| SND.WND  Send window                               INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 56| RCV.NXT  Next expected receive sequence #          INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 60| RCV.WND  Receive window                            INET format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 64| SEG.LEN  Segment length                           INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 68| SEG.WND  Segment window                           INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 72| Retransmit queue # NOW WINDOW SIZE TIMER          INTEL format|
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 76|       RX Data                                                 |
;   +-+-+-..........                                               -+



; IP protocol numbers
PROTOCOL_ICMP     equ      1
PROTOCOL_TCP      equ      6
PROTOCOL_UDP      equ      17


; TIPBUFF status values
BUFF_EMPTY         equ     0
BUFF_RX_FULL       equ     1
BUFF_ALLOCATED     equ     2
BUFF_TX_FULL       equ     3

NUM_IPBUFFERS      equ     20    ; buffers allocated for TX/RX

SOCK_EMPTY         equ     0        ; socket not in use
SOCK_OPEN          equ     1        ; open issued, but no data sent

; TCP opening modes
SOCKET_PASSIVE     equ     0
SOCKET_ACTIVE      equ     1

; TCP TCB states
TCB_LISTEN         equ        1
TCB_SYN_SENT       equ        2
TCB_SYN_RECEIVED   equ        3
TCB_ESTABLISHED    equ        4
TCB_FIN_WAIT_1     equ        5
TCB_FIN_WAIT_2     equ        6
TCB_CLOSE_WAIT     equ        7
TCB_CLOSING        equ        8
TCB_LAST_ACK       equ        9
TCB_TIME_WAIT      equ        10
TCB_CLOSED         equ        11

TWOMSL              equ     10      ; # of secs to wait before closing socket

; socket buffers
SOCKETBUFFSIZE     equ        4096  ; state + config + buffer.
SOCKETHEADERSIZE   equ        76    ; thus 4096 - SOCKETHEADERSIZE bytes data

NUM_SOCKETS        equ        16    ; Number of open sockets supported. Was 20


NUMQUEUES          equ        4
EMPTY_QUEUE        equ        0
IPIN_QUEUE         equ        1
IPOUT_QUEUE        equ        2
NET1OUT_QUEUE      equ        3

NO_BUFFER          equ        0xFFFF
IPBUFFSIZE         equ        1500                ; MTU of an ethernet packet
NUMQUEUEENTRIES    equ        NUM_IPBUFFERS
NUMRESENDENTRIES    equ         18              ; Buffers for TCP resend packets
TCP_RETRIES         equ         5               ; Number of times to resend a packet
TCP_TIMEOUT         equ         10              ; resend if not replied to in x hs

; These are the 0x40 function codes for application access to the stack
STACK_DRIVER_STATUS  equ   52
SOCKET_INTERFACE     equ   53


; 128KB allocated for the stack and network driver buffers and other
; data requirements
stack_data_start     equ   0x700000
eth_data_start       equ   0x700000
stack_data           equ   0x704000
stack_data_end       equ   0x71ffff

; 32 bit word
stack_config         equ   stack_data
; 32 bit word - IP Address in network format
stack_ip             equ   stack_data + 4
; 1 byte. 0 == inactive, 1 = active
slip_active          equ   stack_data + 8       ; no longer used
; 1 byte. 0 == inactive, 1 = active
ethernet_active      equ   stack_data + 9
unused               equ   stack_data + 10
;  word. Buffer number, -1 if none
rx_buff_ptr          equ   stack_data + 12
; dword. Buffer number, -1 if none
tx_buff_ptr          equ   stack_data + 16
; byte.
slip_rx_state        equ   stack_data + 20      ; no longer used
; byte
slip_tx_state        equ   stack_data + 21      ; no longer used
; dword. Index into data
rx_data_ptr          equ   stack_data + 22
; dword. Index into data
tx_data_ptr          equ   stack_data + 26
; word. Count of bytes to send
tx_msg_len           equ   stack_data + 30
; Address of selected socket
sktAddr              equ   stack_data + 32
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
sockets              equ   stack_data + 62
Next_free2           equ   sockets + (SOCKETBUFFSIZE * NUM_SOCKETS)
; 1560 byte buffer for rx / tx ethernet packets
Ether_buffer         equ   Next_free2
Next_free3           equ   Ether_buffer + 1560
last_1sTick          equ   Next_free3
IPbuffs              equ   Next_free3 + 1
queues               equ   IPbuffs + ( NUM_IPBUFFERS * IPBUFFSIZE )
queueList            equ   queues + (2 * NUMQUEUES)
last_1hsTick         equ   queueList + ( 2 * NUMQUEUEENTRIES )

;resendQ              equ   queueList + ( 2 * NUMQUEUEENTRIES )
;resendBuffer         equ    resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP
;                    equ    resendBuffer + ( IPBUFFSIZE * NUMRESENDENTRIES )



resendQ             equ     0x770000
resendBuffer        equ     resendQ + ( 4 * NUMRESENDENTRIES ) ; for TCP


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
    xor     eax,eax
    mov     edi,stack_data_start
    mov     ecx,0x20000 / 4  ; Assume that we have 128KB of data
    cld
    rep     stosd

    ; Initialise TCP resend queue data structures
    mov     eax, 0xFFFFFFFF
    mov     edi, resendQ
    mov     ecx, NUMRESENDENTRIES  ; 1 dword per entry
    cld
    rep     stosd


    mov     eax, 0xFFFFFFFF
    mov     [rx_buff_ptr], eax
    mov     [tx_buff_ptr], eax

    ; Put in some defaults : slip, 0x3f8, 4, ip=192.168.1.22
    ; Saves me entering them each boot up when debugging
    mov     eax, 0x03f80401
    mov     [stack_config], eax
    mov     eax, 0xc801a8c0
    mov     [stack_ip], eax

    call    queueInit

    ; The following block sets up the 1s timer
    mov     al,0x0
    out     0x70,al
    in      al,0x71
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
stack_handler:

    call    ethernet_driver
    call    ip_rx


    ; Test for 10ms tick, call tcp timer
    mov     eax, [timer_ticks] ;[0xfdf0]
    cmp     eax, [last_1hsTick]
    je      sh_001

    mov     [last_1hsTick], eax
    call    tcp_tx_handler

sh_001:

    ; Test for 1 second event, call 1s timer functions
    mov     al,0x0   ;second
    out     0x70,al
    in      al,0x71
    cmp     al, [last_1sTick]
    je      sh_exit

    mov     [last_1sTick], al

    call    arp_timer
    call    tcp_tcb_handler

sh_exit:
    ret




;***************************************************************************
;   Function
;      is_localport_unused
;
;   Description
;         scans through all the active sockets , looking to see if the
;      port number specified in bx is in use as a localport number.
;      This is useful when you want a to generate a unique local port
;      number.
;          On return, eax = 1 for free, 0 for in use
;
;***************************************************************************
is_localport_unused:
    mov     al, bh
    mov     ah, bl
    mov     bx, ax

    mov     edx, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS
    mov     eax, 0                    ; Assume the return value is 'in use'

ilu1:
    sub     edx, SOCKETBUFFSIZE
    cmp     [edx + sockets + 12], bx
    loopnz  ilu1                  ; Return back if the socket is occupied

    jz      ilu_exit
    inc     eax                         ; return port not in use

ilu_exit:
    ret



;***************************************************************************
;   Function
;      get_free_socket
;
;   Description
;
;***************************************************************************
get_free_socket:
    push    ecx
    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS

gfs1:
    sub     eax, SOCKETBUFFSIZE
    cmp     [eax + sockets], dword SOCK_EMPTY
    loopnz  gfs1                  ; Return back if the socket is occupied
    mov     eax, ecx
    pop     ecx
    jz      gfs_exit
    mov     eax, 0xFFFFFFFF

gfs_exit:
    ret



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

    xor     edx, edx                  ; edx is the accumulative checksum
    xor     ebx, ebx
    mov     cx, [checkSize1]
    shr     cx, 1
    jz      cs1_1

    mov     eax, [checkAdd1]

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
    jz      cs_exit                     ; Finished if no 2nd buffer

    shr     cx, 1
    jz      cs2_1

    mov     eax, [checkAdd2]

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
;       This is an application service, called by int 0x40 fn 52
;       It provides application access to the network interface layer
;
;***************************************************************************
app_stack_handler:
    cmp     eax, 0
    jnz     not0
    ; Read the configuartion word
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

    je       ash_eth_enable
    ; Ethernet isn't enabled, so make sure that the card is disabled
    mov     [ethernet_active], byte 0

    ret

ash_eth_enable:
    ; Probe for the card. This will reset it and enable the interface
    ; if found
    call    eth_probe
    cmp     eax, 0
    je      ash_eth_done            ; Abort if no hardware found

    mov     [ethernet_active], byte 1

ash_eth_done:
    ret

not2:
    cmp     eax, 3
    jnz     not3
    ; write the IP Address
    mov     [stack_ip], ebx
    ret

not3:
    cmp     eax, 4
    jnz     not4
    ; Enabled the slip driver on the comm port
    ; slip removed
    ret

not4:
    cmp     eax, 5
    jnz     not5
    ; Disable the slip driver on the comm port
    ; slip removed

not5:
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
    jnz     stack_driver_end

    ; write the dns IP Address
    mov     [dns_ip], ebx

    ret

stack_driver_end:
    ret



;***************************************************************************
;   Function
;      app_socket_handler
;
;   Description
;       This is an application service, called by int 0x40
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
    je      sgp_non_exit            ; Exit if no buffer available

    push    eax                     ; Save buffer number for freeing at end

    push    edx
    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs
    pop     edx

    push    eax                     ; save address of IP data

    ; Get the address of the callers data
    mov     edi,[0x3010]
    add     edi,TASKDATA.mem_start
    add     edx,[edi]
    mov     edi, edx

    pop     eax

    mov     ecx, 1500           ; should get the actual number of bytes to write
    mov     esi, eax
    cld
    rep     movsb               ; copy the data across

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

    pop     ecx                     ; count of bytes to send
    mov     ebx, ecx                ; need the length later
    pop     eax                     ; get callers ptr to data to send

    ; Get the address of the callers data
    mov     edi,[0x3010]
    add     edi,TASKDATA.mem_start
    add     eax,[edi]
    mov     esi, eax

    mov     edi, edx
    cld
    rep     movsb               ; copy the data across

    pop     ebx

    mov     eax, IPIN_QUEUE
    call    queue

    inc     dword [ip_rx_count]

    mov     eax, 0
    ret

sip_err_exit:
    mov     eax, 0xFFFFFFFF
    ret



;***************************************************************************
;   Function
;      socket_open
;
;   Description
;       find a free socket
;       local port in ebx
;       remote port in ecx
;       remote ip in edx
;       return socket # in eax, -1 if none available
;
;***************************************************************************
socket_open:
    call    get_free_socket

    cmp     eax, 0xFFFFFFFF
    jz      so_exit

    ; ax holds the socket number that is free. Get real address
    push    eax
    shl     eax, 12
    add     eax, sockets

    mov     [eax], dword SOCK_OPEN

    mov     [eax + 12], byte bh      ; Local port ( LS 16 bits )
    mov     [eax + 13], byte bl      ; Local port ( LS 16 bits )
    mov     ebx, [stack_ip]
    mov     [eax + 8], ebx         ; Local IP
    mov     [eax + 20], ch         ; Remote Port ( LS 16 bits )
    mov     [eax + 21], cl         ; Remote Port ( LS 16 bits )
    mov     [eax + 16], edx         ; Remote IP ( in Internet order )
    mov     [eax + 24], dword 0      ; recieved data count

    mov     esi, [0x3010]
    mov     ebx, [esi+TASKDATA.pid]
    mov     [eax + 4], ebx         ; save the process ID
    pop     eax      ; Get the socket number back, so we can return it

so_exit:
    ret



;***************************************************************************
;   Function
;      socket_open_tcp
;
;   Description
;       Opens a TCP socket in PASSIVE or ACTIVE mode
;       find a free socket
;       local port in ebx ( intel format )
;       remote port in ecx ( intel format )
;       remote ip in edx ( in Internet byte order )
;       Socket open mode in esi  ( SOCKET_PASSIVE or SOCKET_ACTIVE )
;       return socket # in eax, -1 if none available
;
;***************************************************************************
socket_open_tcp:
    call    get_free_socket

    cmp     eax, 0xFFFFFFFF
    jz      so_exit

    ; ax holds the socket number that is free. Get real address
    push    eax
    shl     eax, 12
    add     eax, sockets

    mov     [sktAddr], eax
    mov     [eax], dword SOCK_OPEN

    ; TODO - check this works!
    mov     [eax + 72], dword 0     ; Reset the window timer.

    mov     [eax + 12], byte bh      ; Local port ( LS 16 bits )
    mov     [eax + 13], byte bl      ; Local port ( LS 16 bits )
    mov     ebx, [stack_ip]
    mov     [eax + 8], ebx         ; Local IP
    mov     [eax + 20], ch         ; Remote Port ( LS 16 bits )
    mov     [eax + 21], cl         ; Remote Port ( LS 16 bits )
    mov     [eax + 16], edx         ; Remote IP ( in Internet order )
    mov     [eax + 24], dword 0      ; recieved data count

    ; Now fill in TCB state
    mov     ebx, TCB_LISTEN
    cmp     esi, SOCKET_PASSIVE
    jz      sot_001
    mov     ebx, TCB_SYN_SENT

sot_001:
    mov     [eax + 28], ebx            ; Indicate the state of the TCB

    mov     esi, [0x3010]
    mov     ecx, [esi+TASKDATA.pid]
    mov     [eax + 4], ecx         ; save the process ID

    cmp     ebx, TCB_LISTEN
    je      sot_done

    ; Now, if we are in active mode, then we have to send a SYN to the specified remote port


    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      sot_done

    push    eax

    mov     bl, 0x02        ; SYN
    mov     ecx, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     sot_notlocal
    mov     eax, IPIN_QUEUE

sot_notlocal:
       ; Send it.
    pop     ebx
    call    queue

    mov     esi, [sktAddr]

    ; increment SND.NXT in socket
    add     esi, 48
    call    inc_inet_esi

sot_done:
    pop     eax      ; Get the socket number back, so we can return it

sot_exit:
    ret



;***************************************************************************
;   Function
;      socket_close
;
;   Description
;       socket # in ebx
;       returns 0 for ok, -1 for socket not open (fail)
;
;***************************************************************************
socket_close:
    shl     ebx, 12
    add     ebx, sockets
    mov     eax, 0xFFFFFFFF         ; assume this operation will fail..
    cmp     [ebx], dword SOCK_EMPTY
    jz      sc_exit

    ; Clear the socket varaibles
    xor     eax, eax
    mov     edi,ebx
    mov     ecx,SOCKETHEADERSIZE
    cld
    rep     stosb

sc_exit:
    ret



;***************************************************************************
;   Function
;      socket_close_tcp
;
;   Description
;       socket # in ebx
;       returns 0 for ok, -1 for socket not open (fail)
;
;***************************************************************************
socket_close_tcp:
    ; first, remove any resend entries
    pusha

    mov     esi, resendQ
    mov     ecx, 0

sct001:
    cmp     ecx, NUMRESENDENTRIES
    je      sct003              ; None left
    cmp     [esi], bl
    je      sct002              ; found one
    inc     ecx
    add     esi, 4
    jmp     sct001

sct002:
    dec     dword [arp_rx_count] ; ************ TEST ONLY!

    mov     [esi], byte 0xFF
    jmp     sct001

sct003:
    popa

    shl     ebx, 12
    add     ebx, sockets
    mov     [sktAddr], ebx
    mov     eax, 0xFFFFFFFF         ; assume this operation will fail..
    cmp     [ebx], dword SOCK_EMPTY
    jz      sct_exit

    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      stl_exit

    push    eax

    mov     bl, 0x11        ; FIN + ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     ebx, [sktAddr]

    ; increament SND.NXT in socket
    mov     esi, 48
    add     esi, ebx
    call    inc_inet_esi


    ; Get the socket state
    mov     eax, [ebx + 28]
    cmp     eax, TCB_LISTEN
    je      destroyTCB
    cmp     eax, TCB_SYN_SENT
    je      destroyTCB
    cmp     eax, TCB_SYN_RECEIVED
    je      sct_finwait1
    cmp     eax, TCB_ESTABLISHED
    je      sct_finwait1

    ; assume CLOSE WAIT
    ; Send a fin, then enter last-ack state
    mov     eax, TCB_LAST_ACK
    mov     [ebx + 28], eax
    xor     eax, eax
    jmp     sct_send

sct_finwait1:
    ; Send a fin, then enter finwait2 state
    mov     eax, TCB_FIN_WAIT_1
    mov     [ebx + 28], eax
    xor     eax, eax

sct_send:
    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     sct_notlocal
    mov     eax, IPIN_QUEUE

sct_notlocal:
       ; Send it.
    pop     ebx
    call    queue
    jmp     sct_exit

destroyTCB:
    pop     eax
    ; Clear the socket varaibles
    xor     eax, eax
    mov     edi,ebx
    mov     ecx,SOCKETHEADERSIZE
    cld
    rep     stosb

sct_exit:
    ret



;***************************************************************************
;   Function
;      socket_poll
;
;   Description
;       socket # in ebx
;       returns count in eax.
;
;***************************************************************************
socket_poll:
    shl     ebx, 12
    add     ebx, sockets
    mov     eax, [ebx + 24]

    ret



;***************************************************************************
;   Function
;      socket_status
;
;   Description
;       socket # in ebx
;       returns TCB state in eax.
;
;***************************************************************************
socket_status:
    shl     ebx, 12
    add     ebx, sockets
    mov     eax, [ebx + 28]

    ret



;***************************************************************************
;   Function
;      socket_read
;
;   Description
;       socket # in ebx
;       returns # of bytes remaining in eax, data in bl
;
;***************************************************************************
socket_read:
    shl     ebx, 12
    add     ebx, sockets
    mov     eax, [ebx + 24]         ; get count of bytes
    mov     ecx,1
    test    eax, eax
    jz      sr2

    dec     eax
    mov     esi, ebx            ; esi is address of socket
    mov     [ebx + 24], eax         ; store new count
    movzx   ebx, byte [ebx + SOCKETHEADERSIZE]  ; get the byte
    add     esi, SOCKETHEADERSIZE
    mov     edi, esi
    inc     esi

    mov     ecx, (SOCKETBUFFSIZE - SOCKETHEADERSIZE) / 4
    cld
    rep     movsd
    xor     ecx, ecx

sr1:
    jmp     sor_exit

sr2:
    xor     bl, bl

sor_exit:
    ret



;***************************************************************************
;   Function
;      socket_write
;
;   Description
;       socket in ebx
;       # of bytes to write in ecx
;       pointer to data in edx
;       returns 0 in eax ok, -1 == failed ( invalid socket, or
;       could not queue IP packet )
;
;***************************************************************************
socket_write:
    ; First, find the address of the socket descriptor
    shl     ebx, 12
    add     ebx, sockets         ; ebx = address of actual socket

    mov     eax, 0xFFFFFFFF
    ; If the socket is invalid, return with an error code
    cmp     [ebx], dword SOCK_EMPTY
    je      sw_exit


    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      sw_exit

    ; Save the queue entry number
    push    eax

    ; save the pointers to the data buffer & size
    push    edx
    push    ecx

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edx, eax

    ; So, ebx holds the socket ptr, edx holds the IPbuffer ptr

    ; Fill in the IP header ( some data is in the socket descriptor)
    mov     eax, [ebx + 8]
    mov     [edx + 12], eax      ; source IP
    mov     eax, [ebx + 16]
    mov     [edx + 16], eax      ; Destination IP

    mov     al, 0x45
    mov     [edx], al         ; Version, IHL
    xor     al, al
    mov     [edx + 1], al     ; Type of service

    pop     eax                   ; Get the UDP data length
    push    eax

    add     eax, 20 + 8           ; add IP header and UDP header lengths
    mov     [edx + 2], ah
    mov     [edx + 3], al
    xor     al, al
    mov     [edx + 4], al
    mov     [edx + 5], al
    mov     al, 0x40
    mov     [edx + 6], al
    xor     al, al
    mov     [edx + 7], al
    mov     al, 0x20
    mov     [edx + 8], al
    mov     al, 17
    mov     [edx + 9], al

    ; Checksum left unfilled
    xor     ax, ax
    mov     [edx + 10], ax

    ; Fill in the UDP header ( some data is in the socket descriptor)
    mov     ax, [ebx + 12]
    mov     [edx + 20], ax

    mov     ax, [ebx + 20]
    mov     [edx + 20 + 2], ax

    pop     eax
    push    eax

    add     eax, 8
    mov     [edx + 20 + 4], ah
    mov     [edx + 20 + 5], al

    ; Checksum left unfilled
    xor     ax, ax
    mov     [edx + 20 + 6], ax

    pop     ecx                  ; count of bytes to send
    mov     ebx, ecx            ; need the length later
    pop     eax                  ; get callers ptr to data to send

    ; Get the address of the callers data
    mov     edi,[0x3010]
    add     edi,TASKDATA.mem_start
    add     eax,[edi]
    mov     esi, eax

    mov     edi, edx
    add     edi, 28
    cld
    rep     movsb               ; copy the data across

    ; we have edx as IPbuffer ptr.
    ; Fill in the UDP checksum
    ; First, fill in pseudoheader
    mov     eax, [edx + 12]
    mov     [pseudoHeader], eax
    mov     eax, [edx + 16]
    mov     [pseudoHeader+4], eax
    mov     ax, 0x1100            ; 0 + protocol
    mov     [pseudoHeader+8], ax
    add     ebx, 8
    mov     eax, ebx
    mov     [pseudoHeader+10], ah
    mov     [pseudoHeader+11], al

    mov     eax, pseudoHeader
    mov     [checkAdd1], eax
    mov     [checkSize1], word 12
    mov     eax, edx
    add     eax, 20
    mov     [checkAdd2], eax
    mov     eax, ebx
    mov     [checkSize2], ax      ; was eax!! mjh 8/7/02

    call    checksum

    ; store it in the UDP checksum ( in the correct order! )
    mov     ax, [checkResult]

    ; If the UDP checksum computes to 0, we must make it 0xffff
    ; (0 is reserved for 'not used')
    cmp     ax, 0
    jne     sw_001
    mov     ax, 0xffff

sw_001:
    mov     [edx + 20 + 6], ah
    mov     [edx + 20 + 7], al

    ; Fill in the IP header checksum
    mov     eax, edx
    mov     [checkAdd1], eax
    mov     [checkSize1], word 20
    mov     [checkAdd2], dword 0
    mov     [checkSize2], word 0

    call    checksum

    mov     ax, [checkResult]
    mov     [edx + 10], ah
    mov     [edx + 11], al

    ; Check destination IP address.
    ; If it is the local host IP, route it back to IP_RX

    pop     ebx
    mov     eax, NET1OUT_QUEUE

    mov     ecx, [ edx + 16]
    mov     edx, [stack_ip]
    cmp     edx, ecx
    jne     sw_notlocal
    mov     eax, IPIN_QUEUE

sw_notlocal:
    ; Send it.
    call    queue

    xor     eax, eax

sw_exit:
    ret



;***************************************************************************
;   Function
;      socket_write_tcp
;
;   Description
;       socket in ebx
;       # of bytes to write in ecx
;       pointer to data in edx
;       returns 0 in eax ok, -1 == failed ( invalid socket, or
;       could not queue IP packet )
;
;***************************************************************************
socket_write_tcp:
    ; First, find the address of the socket descriptor
    shl     ebx, 12
    add     ebx, sockets         ; ebx = address of actual socket

    mov     [sktAddr], ebx

    mov     eax, 0xFFFFFFFF
    ; If the socket is invalid, return with an error code
    cmp     [ebx], dword SOCK_EMPTY
    je      swt_exit

    ; If the sockets window timer is nonzero, do not queue packet
    ; TODO - done
    cmp     [ebx + 72], dword 0
    jne     swt_exit

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      swt_exit

    push    eax

    mov     bl, 0x10        ; ACK

    ; Get the address of the callers data
    mov     edi,[0x3010]
    add     edi,TASKDATA.mem_start
    add     edx,[edi]
    mov     esi, edx

    pop     eax
    push    eax

    push    ecx
    call    buildTCPPacket
    pop     ecx

    ; Check destination IP address.
    ; If it is the local host IP, route it back to IP_RX

    pop     ebx
    push    ecx
    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     swt_notlocal
    mov     eax, IPIN_QUEUE

swt_notlocal:
    pop     ecx

    push    ebx                 ; save ipbuffer number

    call    queue

    mov     esi, [sktAddr]

    ; increament SND.NXT in socket
    ; Amount to increment by is in ecx
    add     esi, 48
    call    add_inet_esi

    pop     ebx

    ; Copy the IP buffer to a resend queue
    ; If there isn't one, dont worry about it for now
    mov     esi, resendQ
    mov     ecx, 0

swt003:
    cmp     ecx, NUMRESENDENTRIES
    je      swt001              ; None found
    cmp     [esi], byte 0xFF
    je      swt002              ; found one
    inc     ecx
    add     esi, 4
    jmp     swt003

swt002:
    push    ebx

    ; OK, we have a buffer descriptor ptr in esi.
    ; resend entry # in ecx
    ;  Populate it
    ;  socket #
    ;  retries count
    ;  retry time
    ;  fill IP buffer associated with this descriptor

    mov     eax, [sktAddr]
    sub     eax, sockets
    shr     eax, 12             ; get skt #
    mov     [esi], al
    mov     [esi + 1], byte TCP_RETRIES
    mov     [esi + 2], word TCP_TIMEOUT

    inc     ecx
    ; Now get buffer location, and copy buffer across. argh! more copying,,
    mov     edi, resendBuffer - IPBUFFSIZE
swt002a:
    add     edi, IPBUFFSIZE
    loop    swt002a

    ; we have dest buffer location in edi
    pop     eax
    ; convert source buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs
    mov     esi, eax

    ; do copy
    mov     ecx, IPBUFFSIZE
    cld
    rep     movsb

    inc     dword [arp_rx_count] ; ************ TEST ONLY!

swt001:
    xor     eax, eax

swt_exit:
    ret



; Below, the main network layer source code is included
;

include "queue.inc"
include "ip.inc"
include "tcp.inc"
include "udp.inc"
include "eth_drv/ethernet.inc"
