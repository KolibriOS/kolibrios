$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  SOCKET.INC                                                  ;;
;;                                                              ;;
;;  Sockets constants, structures and functions                 ;;
;;                                                              ;;
;;  Last revision: 11.11.2006                                   ;;
;;                                                              ;;
;;  This file contains the following:                           ;;
;;    is_localport_unused                                       ;;
;;    get_free_socket                                           ;;
;;    socket_open                                               ;;
;;    socket_open_tcp                                           ;;
;;    socket_close                                              ;;
;;    socket_close_tcp                                          ;;
;;    socket_poll                                               ;;
;;    socket_status                                             ;;
;;    socket_read                                               ;;
;;    socket_write                                              ;;
;;    socket_write_tcp                                          ;;
;;                                                              ;;
;;                                                              ;;
;;  Changes history:                                            ;;
;;   22.09.2003 - [Mike Hibbett] : mikeh@oceanfree.net          ;;
;;   11.11.2006 - [Johnny_B] and [smb]                          ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
;  Socket Descriptor + Buffer
;
;    0                   1                   2                   3
;    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
;
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  0|                    Status ( of this buffer )                  |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  4|  Application Process ID                                       |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;  8|                  Local IP Address                             |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 12| Local IP Port                 | Unused ( set to 0 )           |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 16|                  Remote IP Address                            |
;   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
; 20| Remote IP Port                | Unused ( set to 0 )           |
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
; 76|      RX offset from
; 76|       RX Data                                                 |
;   +-+-+-..........                                               -+


; so, define struct
struc SOCKET
{  .Status		    dd	 ?  ;+00 - Status ( of this buffer )
   .PID 		    dd	 ?  ;+04 - Application Process ID
   .LocalIP		    dd	 ?  ;+08 -  Local IP Address
   .LocalPort		    dw	 ?  ;+12 - Local Port
   .UnusedL		    dw	 ?  ;+14 - may be removed in future
   .RemoteIP		    dd	 ?  ;+16 - Remote IP Address
   .RemotePort		    dw	 ?  ;+20 - Remote Port
   .UnusedR		    dw	 ?  ;+22 - may be removed in future
   .rxDataCount 	    dd	 ?  ;+24 - Rx Data Count
   .TCBState		    dd	 ?  ;+28 - TCB STATE
   .TCBTimer		    dd	 ?  ;+32 - TCB Timer (seconds)
   .ISS 		    dd	 ?  ;+36 - Initial Send Sequence
   .IRS 		    dd	 ?  ;+40 - Initial Receive Sequence
   .SND_UNA		    dd	 ?  ;+44 - Sequence number of unack'ed sent packets
   .SND_NXT		    dd	 ?  ;+48 - Next send sequence number to use
   .SND_WND		    dd	 ?  ;+52 - Send window
   .RCV_NXT		    dd	 ?  ;+56 - Next receive sequence number to use
   .RCV_WND		    dd	 ?  ;+60 - Receive window
   .SEG_LEN		    dd	 ?  ;+64 - Segment length
   .SEG_WND		    dd	 ?  ;+68 - Segment window
   .wndsizeTimer	    dd	 ?  ;+72 - Retransmit queue # NOW WINDOW SIZE TIMER
   .rxData		    dd	 ?  ;+76 - receive data buffer here
}

virtual at 0
  SOCKET SOCKET
end virtual

; simple macro calcing real memory address of SOCKET struct by socket's
macro	Index2RealAddr reg
{
	shl	reg, 12
    add     reg, sockets
}

;Constants
; current socket statuses
SOCK_EMPTY	   equ	   0	    ; socket not in use
SOCK_OPEN	   equ	   1	    ; open issued, but no data sent

; TCP opening modes
SOCKET_PASSIVE	   equ	   0
SOCKET_ACTIVE	   equ	   1

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
    mov     eax, 0		      ; Assume the return value is 'in use'

ilu1:
    sub     edx, SOCKETBUFFSIZE
    cmp     [edx + sockets + SOCKET.LocalPort], bx
    loopnz  ilu1		  ; Return back if the socket is occupied

    jz	    ilu_exit
    inc     eax 			; return port not in use

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
    cmp     [eax + sockets + SOCKET.Status], dword SOCK_EMPTY
    loopnz  gfs1		  ; Return back if the socket is occupied
    mov     eax, ecx
    pop     ecx
    jz	    gfs_exit
    mov     eax, 0xFFFFFFFF

gfs_exit:
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
    jz	    so_exit

    ; ax holds the socket number that is free. Get real address
    push    eax
    Index2RealAddr eax

    mov     [eax + SOCKET.Status], dword SOCK_OPEN

    xchg    bh, bl
    mov     [eax + SOCKET.LocalPort], bx
    xchg    ch, cl
    mov     [eax + SOCKET.RemotePort], cx

    mov     ebx, [stack_ip]
    mov     [eax + SOCKET.LocalIP], ebx
    mov     [eax + SOCKET.RemoteIP], edx
    mov     [eax + SOCKET.rxDataCount], dword 0      ; recieved data count

    mov     esi, [TASK_BASE]
    mov     ebx, [esi+TASKDATA.pid]
    mov     [eax + SOCKET.PID], ebx	    ; save the process ID
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
    jz	    so_exit

    ; ax holds the socket number that is free. Get real address
    push eax
    Index2RealAddr eax

    mov     [sktAddr], eax
    mov     [eax], dword SOCK_OPEN

    ; TODO - check this works!
    mov     [eax + SOCKET.wndsizeTimer], dword 0     ; Reset the window timer.

	xchg	bh, bl
	mov	[eax + SOCKET.LocalPort], bx
;    mov     [eax + 12], byte bh      ; Local port ( LS 16 bits )
;    mov     [eax + 13], byte bl      ; Local port ( LS 16 bits )

	xchg	ch, cl
	mov	[eax + SOCKET.RemotePort], cx
;    mov     [eax + 20], ch         ; Remote Port ( LS 16 bits )
;    mov     [eax + 21], cl         ; Remote Port ( LS 16 bits )

    mov     ebx, [stack_ip]
    mov     [eax + SOCKET.LocalIP], ebx
    mov     [eax + SOCKET.RemoteIP], edx
    mov     [eax + SOCKET.rxDataCount], dword 0

    ; Now fill in TCB state
    mov     ebx, TCB_LISTEN
    cmp     esi, SOCKET_PASSIVE
    jz	    sot_001
    mov     ebx, TCB_SYN_SENT

sot_001:
    mov     [eax + SOCKET.TCBState], ebx	    ; Indicate the state of the TCB

    mov     esi, [TASK_BASE]
    mov     ecx, [esi+TASKDATA.pid]
    mov     [eax + SOCKET.PID], ecx	    ; save the process ID

    cmp     ebx, TCB_LISTEN
    je	    sot_done

    ; Now, if we are in active mode, then we have to send a SYN to the specified remote port
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    sot_done

    push    eax

    mov     bl, 0x02	    ; SYN
    mov     ecx, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [sktAddr ]
    mov     ecx, [ecx + 16]
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
    Index2RealAddr ebx
    mov     eax, 0xFFFFFFFF	    ; assume this operation will fail..
    cmp     [ebx + SOCKET.Status], dword SOCK_EMPTY
    jz	    sc_exit

    ; Clear the socket varaibles
    xor     eax, eax
    mov     edi, ebx
    mov     ecx, SOCKETHEADERSIZE
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
    je	    sct003		; None left
    cmp     [esi], bl
    je	    sct002		; found one
    inc     ecx
    add     esi, 4
    jmp     sct001

sct002:

    mov     [esi], byte 0xFF
    jmp     sct001

sct003:
    popa

    Index2RealAddr ebx
    mov     [sktAddr], ebx
    mov     eax, 0xFFFFFFFF	    ; assume this operation will fail..
    cmp     [ebx + SOCKET.Status], dword SOCK_EMPTY
    jz	    sct_exit

    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    stl_exit

    push    eax

    mov     bl, 0x11	    ; FIN + ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     ebx, [sktAddr]

    ; increament SND.NXT in socket
    mov     esi, 48
    add     esi, ebx
    call    inc_inet_esi


    ; Get the socket state
    mov     eax, [ebx + SOCKET.TCBState]
    cmp     eax, TCB_LISTEN
    je	    destroyTCB
    cmp     eax, TCB_SYN_SENT
    je	    destroyTCB
    cmp     eax, TCB_SYN_RECEIVED
    je	    sct_finwait1
    cmp     eax, TCB_ESTABLISHED
    je	    sct_finwait1

    ; assume CLOSE WAIT
    ; Send a fin, then enter last-ack state
    mov     eax, TCB_LAST_ACK
    mov     [ebx + SOCKET.TCBState], eax
    xor     eax, eax
    jmp     sct_send

sct_finwait1:
    ; Send a fin, then enter finwait2 state
    mov     eax, TCB_FIN_WAIT_1
    mov     [ebx + SOCKET.TCBState], eax
    xor     eax, eax

sct_send:
    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [sktAddr ]
    mov     ecx, [ecx + 16]
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
    mov     edi, ebx
    mov     ecx, SOCKETHEADERSIZE
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
    Index2RealAddr ebx
    mov     eax, [ebx + SOCKET.rxDataCount]

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
    Index2RealAddr ebx
    mov     eax, [ebx + SOCKET.TCBState]

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
    Index2RealAddr ebx
    mov     eax, [ebx + SOCKET.rxDataCount]	    ; get count of bytes
    mov     ecx, 1
    test    eax, eax
    jz	    sr2

    dec     eax
    mov     esi, ebx		; esi is address of socket
    mov     [ebx + SOCKET.rxDataCount], eax	    ; store new count
    ;movzx   ebx, byte [ebx + SOCKET.rxData]  ; get the byte
    movzx   ebx, byte [ebx + SOCKETHEADERSIZE]	; get the byte
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
;      socket_read_packet
;
;   Description
;       socket # in ebx
;       datapointer # in ecx
;       buffer size in edx
;       returns # of bytes copied in eax
;
;***************************************************************************
socket_read_packet:
    Index2RealAddr ebx					   ; get real socket address
    mov     eax, [ebx + SOCKET.rxDataCount]		   ; get count of bytes
    test    eax, eax					   ; if count of bytes is zero..
    jz	    .exit					   ; exit function (eax will be zero)

    test    edx, edx					   ; if buffer size is zero, copy all data
    jz	    .copyallbytes
    cmp     edx, eax					   ; if buffer size is larger then the bytes of data, copy all data
    jge     .copyallbytes

    sub     eax, edx					   ; store new count (data bytes in buffer - bytes we're about to copy)
    mov     [ebx + SOCKET.rxDataCount], eax		   ;
    push    eax
    mov     eax, edx					   ; number of bytes we want to copy must be in eax
    call    .startcopy					   ; copy to the application

    mov     esi, ebx					   ; now we're going to copy the remaining bytes to the beginning
    add     esi, SOCKETHEADERSIZE			   ; we dont need to copy the header
    mov     edi, esi					   ; edi is where we're going to copy to
    add     esi, edx					   ; esi is from where we copy
    pop     ecx 					   ; count of bytes we have left
    push    ecx 					   ; push it again so we can re-use it later
    shr     ecx, 2					   ; divide eax by 4
    cld
    rep     movsd					   ; copy all full dwords
    pop     ecx
    and     ecx, 3
    rep     movsb					   ; copy remaining bytes

    ret 						   ; at last, exit

.copyallbytes:
    xor     esi, esi
    mov     [ebx + SOCKET.rxDataCount], esi		   ; store new count (zero)

.startcopy:
    mov     edi, ecx					   ;
    add     edi, std_application_base_address		   ; get data pointer to buffer in application

    mov     esi, ebx					   ;
    add     esi, SOCKETHEADERSIZE			   ; we dont need to copy the header
    mov     ecx, eax					   ; eax is count of bytes
    push    ecx
    shr     ecx, 2					   ; divide eax by 4
    cld 						   ; copy all full dwords
    rep     movsd					   ;
    pop     ecx
    and     ecx, 3
    rep     movsb					   ; copy the rest bytes

.exit:
    ret 						   ; exit, or go back to shift remaining bytes if any




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
    Index2RealAddr ebx

    mov     eax, 0xFFFFFFFF
    ; If the socket is invalid, return with an error code
    cmp     [ebx], dword SOCK_EMPTY
    je	    sw_exit


    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    sw_exit

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
    mov     [edx + 12], eax	 ; source IP
    mov     eax, [ebx + 16]
    mov     [edx + 16], eax	 ; Destination IP

    mov     al, 0x45
    mov     [edx], al	      ; Version, IHL
    xor     al, al
    mov     [edx + 1], al     ; Type of service

    pop     eax 		  ; Get the UDP data length
    push    eax

    add     eax, 20 + 8 	  ; add IP header and UDP header lengths
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

    pop     ecx 		 ; count of bytes to send
    mov     ebx, ecx		; need the length later
    pop     eax 		 ; get callers ptr to data to send

    ; Get the address of the callers data
    mov     edi, [TASK_BASE]
    add     edi, TASKDATA.mem_start
    add     eax, [edi]
    mov     esi, eax

    mov     edi, edx
    add     edi, 28
    cld
    rep     movsb		; copy the data across

    ; we have edx as IPbuffer ptr.
    ; Fill in the UDP checksum
    ; First, fill in pseudoheader
    mov     eax, [edx + 12]
    mov     [pseudoHeader], eax
    mov     eax, [edx + 16]
    mov     [pseudoHeader+4], eax
    mov     ax, 0x1100		  ; 0 + protocol
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
    mov     [checkSize2], ax	  ; was eax!! mjh 8/7/02

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
    GET_IHL ecx,edx		 ; get IP-Header length
    stdcall checksum_jb,edx,ecx  ; buf_ptr, buf_size

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
	Index2RealAddr ebx

    mov     [sktAddr], ebx

    mov     eax, 0xFFFFFFFF
    ; If the socket is invalid, return with an error code
    cmp     [ebx], dword SOCK_EMPTY
    je	    swt_exit

    ; If the sockets window timer is nonzero, do not queue packet
    ; TODO - done
    cmp     [ebx + SOCKET.wndsizeTimer], dword 0
    jne     swt_exit

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je	    swt_exit

    push    eax

    mov     bl, 0x10	    ; ACK

    ; Get the address of the callers data
    mov     edi, [TASK_BASE]
    add     edi, TASKDATA.mem_start
    add     edx, [edi]
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
    mov     ecx, [sktAddr ]
    mov     ecx, [ecx + 16]
    cmp     edx, ecx
    jne     swt_notlocal
    mov     eax, IPIN_QUEUE

swt_notlocal:
    pop     ecx

    push    ebx 		; save ipbuffer number

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
    je	    swt001		; None found
    cmp     [esi], byte 0xFF
    je	    swt002		; found one
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
    shr     eax, 12		; get skt #
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

swt001:
    xor     eax, eax

swt_exit:
    ret

