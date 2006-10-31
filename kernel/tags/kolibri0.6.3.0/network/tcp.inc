;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  TCP.INC                                                        ;;
;;                                                                 ;;
;;  TCP Processes for Menuet OS  TCP/IP stack                      ;;
;;                                                                 ;;
;;  Version 0.6  4th July 2004                                       ;;
;;                                                                 ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net               ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;  v0.6 : Added reset handling in the established state           ;;
;;         Added a timer per socket to allow delays when rx window ;;
;;         gets below 1KB                                          ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;*******************************************************************
;   Interface
;
;       tcp_tx_handler      Handles the TCP transmit queue
;       tcp_rx              The protocol handler for received data
;       buildTCPPacket      fills in the packet headers and data
;       tcpStateMachine     Main state machine for received TCP packets
;       tcp_tcb_handler     1s timer, to erase tcb's in TIME_WAIT state
;
;*******************************************************************



;***************************************************************************
;   Function
;      tcp_tcb_handler
;
;   Description
;       Handles sockets in the timewait state, closing them
;       when the TCB timer expires
;
;***************************************************************************
tcp_tcb_handler:
    ; scan through all the sockets, decrementing active timers

    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS

tth1:
    sub     eax, SOCKETBUFFSIZE
    cmp     [eax + sockets + 32], dword 0
    jne     tth2

tth1a:
    cmp     [eax + sockets + 72], dword 0
    jne     tth4

    loop    tth1
    ret

tth2:
    ; decrement it, delete socket if TCB timer = 0 & socket in timewait state
    pusha
    dec     dword [eax + sockets + 32]
    cmp     [eax + sockets + 32], dword 0
    jne     tth3

    cmp     [eax + sockets + 28], dword TCB_TIME_WAIT
    jne     tth3

    ; OK, delete socket
    mov     edi, eax
    add     edi, sockets

    xor     eax, eax
    mov     ecx, SOCKETHEADERSIZE
    cld
    rep     stosb

tth3:
    popa

    jmp     tth1a

    loop    tth1
    ret

    ; TODO - prove it works!
tth4:
    dec     dword [eax + sockets + 72]
    loop    tth1
    ret




tth_exit:
    ret


;***************************************************************************
;   Function
;      tcp_tx_handler
;
;   Description
;       Handles queued TCP data
;       This is a kernel function, called by stack_handler
;
;***************************************************************************
tcp_tx_handler:
    ; decrement all resend buffers timers. If they
    ; expire, queue them for sending, and restart the timer.
    ; If the retries counter reach 0, delete the entry

    mov     esi, resendQ
    mov     ecx, 0

tth001:
    cmp     ecx, NUMRESENDENTRIES
    je      tth003              ; None left
    cmp     [esi], byte 0xFF
    jne      tth002             ; found one
    inc     ecx
    add     esi, 4
    jmp     tth001

tth002:
    ; we have one. decrement it's timer by 1
    dec     word [esi+2]
    mov     ax, [esi+2]
    cmp     ax, 0
    je     tth002a
    inc     ecx
    add     esi, 4
    jmp     tth001              ; Timer not zero, so move on

tth002a:
    mov     bl, 0xff
    ; restart timer, and decrement retries
    ; After the first resend, back of on next, by a factor of 5
    mov     [esi+2], word TCP_TIMEOUT * 5
    dec     byte [esi+1]
    mov     al, [esi+1]
    cmp     al, 0
    jne     tth004

    ; retries now 0, so delete from queue
    xchg     [esi], bl
tth004:

    ; resend packet
    pusha

    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    jne      tth004z

    ; TODO - try again in 10ms.
    cmp     bl, 0xff
    jne     tth004za
    mov     [esi], bl

tth004za:
    ; Mark it to expire in 10ms - 1 tick
    mov     [esi+1], byte 1
    mov     [esi+2], word 1
    jmp     tth005

tth004z:
    ; we have a buffer # in ax

    push    eax
    push    ecx
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    ; we have the buffer address in eax
    mov     edi, eax
    pop     ecx
    ; get resend data address
    inc     ecx
    ; Now get buffer location, and copy buffer across. argh! more copying,,
    mov     esi, resendBuffer - IPBUFFSIZE
tth004a:
    add     esi, IPBUFFSIZE
    loop    tth004a

    ; we have resend buffer location in esi
    mov     ecx, IPBUFFSIZE

    ; copy data across
    cld
    rep     movsb

    ; queue packet



    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ edi + 16 ]
    cmp     edx, ecx
    jne     tth004b
    mov     eax, IPIN_QUEUE

tth004b:
    pop     ebx

    call    queue


tth005:
    popa

    inc     ecx
    add     esi, 4
    jmp     tth001

tth003:
    ret




;***************************************************************************
;   Function
;      tcp_rx
;
;   Description
;       TCP protocol handler
;       This is a kernel function, called by ip_rx
;       IP buffer address given in edx
;          IP buffer number in eax
;          Free up (or re-use) IP buffer when finished
;
;***************************************************************************
tcp_rx:
    ; The process is as follows.
    ; Look for a socket with matching remote IP, remote port, local port
    ; if not found, then
    ; look for remote IP + local port match ( where sockets remote port = 0)
    ; if not found, then
    ; look for a socket where local socket port == IP packets remote port
    ; where sockets remote port, remote IP = 0
    ; discard if not found
    ; Call sockets tcbStateMachine, with pointer to packet.
    ; the state machine will not delete the packet, so do that here.

    push        eax

    ; Look for a socket where
    ; IP Packet TCP Destination Port = local Port
    ; IP Packet SA = Remote IP
    ; IP Packet TCP Source Port = remote Port

    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS
ss1:
    sub     eax, SOCKETBUFFSIZE
    movzx   ebx, word [edx + 22]     ; get the dest. port from the TCP hdr
    cmp     [eax + sockets + 12], bx ; compare with socket's local port
    jnz     nxttst1                        ; different - try next socket

    movzx   ebx, word [edx + 20]       ; get the source port from the TCP hdr
    cmp     [eax + sockets + 20], bx ; compare with socket's remote port
    jnz     nxttst1                        ; different - try next socket


    mov     ebx, [edx + 12]           ; get the source IP Addr from the IP hdr
    cmp     [eax + sockets + 16], ebx ; compare with socket's remote IP
    jnz     nxttst1                        ; different - try next socket

    ; We have a complete match - use this socket
    jmp     tcprx_001

nxttst1:
    loop    ss1                     ; Return back if no match

    ; If we got here, there was no match
    ; Look for a socket where
    ; IP Packet TCP Destination Port = local Port
    ; IP Packet SA = Remote IP
    ; socket remote Port = 0

    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS

ss2:
    sub     eax, SOCKETBUFFSIZE

    movzx   ebx, word [edx + 22]     ; get the dest. port from the TCP hdr
    cmp     [eax + sockets + 12], bx ; compare with socket's local port
    jnz     nxttst2                        ; different - try next socket

    mov     ebx, [edx + 12]          ; get the source IP Addr from the IP hdr
    cmp     [eax + sockets + 16], ebx ; compare with socket's remote IP
    jnz     nxttst2                        ; different - try next socket

    mov     ebx, 0
    cmp     [eax + sockets + 20], bx ; only match a remote socket of 0
    jnz     nxttst2                        ; different - try next socket

    ; We have a complete match - use this socket
    jmp     tcprx_001

nxttst2:
    loop    ss2                     ; Return back if no match

    ; If we got here, there was no match
    ; Look for a socket where
    ; IP Packet TCP Destination Port = local Port
    ; socket Remote IP = 0
    ; socket remote Port = 0

    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS

ss3:
    sub     eax, SOCKETBUFFSIZE

    movzx   ebx, word [edx + 22]     ; get destination port from the TCP hdr
    cmp     [eax + sockets + 12], bx ; compare with socket's local port
    jnz     nxttst3                        ; different - try next socket

    mov     ebx, 0
    cmp     [eax + sockets + 20], bx ; only match a remote socket of 0
    jnz     nxttst3                        ; different - try next socket

    mov     ebx, 0
    cmp     [eax + sockets + 16], ebx ; only match a socket remote IP of 0
    jnz     nxttst3                        ; different - try next socket

    ; We have a complete match - use this socket
    jmp     tcprx_001

nxttst3:
    loop    ss3                     ; Return back if no match

    ; If we got here, we need to reject the packet
    inc     dword [dumped_rx_count]
    jmp     tcprx_exit

tcprx_001:
    ; We have a valid socket/TCB, so call the TCB State Machine for that skt.
    ; socket is pointed to by [eax + sockets]
    ; IP packet is pointed to by [edx]
    ; IP buffer number is on stack ( it will be popped at the end)
    call    tcpStateMachine

tcprx_exit:
    pop     eax
    call    freeBuff

    ret



;***************************************************************************
;   Function
;      buildTCPPacket
;
;   Description
;       builds an IP Packet with TCP data fully populated for transmission
;       You may destroy any and all registers
;          TCP control flags specified in bl
;          This TCB is in [sktAddr]
;          User data pointed to by esi
;       Data length in ecx
;          Transmit buffer number in eax
;
;***************************************************************************
buildTCPPacket:
    push    ecx                        ; Save data length

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edx, eax

    mov     [edx + 33], bl            ; TCP flags

    mov     ebx, [sktAddr]

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

    pop     eax                   ; Get the TCP data length
    push    eax

    add     eax, 20 + 20           ; add IP header and TCP header lengths
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
    mov     al, 6                         ; TCP protocol
    mov     [edx + 9], al

    ; Checksum left unfilled
    xor     ax, ax
    mov     [edx + 10], ax

    ; Fill in the TCP header ( some data is in the socket descriptor)
    mov     ax, [ebx + 12]
    mov     [edx + 20], ax        ; Local Port

    mov     ax, [ebx + 20]
    mov     [edx + 20 + 2], ax    ; desitination Port

    ; Checksum left unfilled
    xor     ax, ax
    mov     [edx + 20 + 16], ax

    ; sequence number
    mov     eax, [ebx + 48]
    mov     [edx + 20 + 4], eax

    ; ack number
    mov     eax, [ebx + 56]
    mov     [edx + 20 + 8], eax

    ; window ( 0x2000 is default ).I could accept 4KB, fa0, ( skt buffer size)
    ; 768 bytes seems better
    mov     ax, 0x0003
    mov     [edx + 20 + 14], ax

    ; Urgent pointer (0)
    mov     ax, 0
    mov     [edx + 20 + 18], ax

    ; data offset ( 0x50 )
    mov     al, 0x50
    mov     [edx + 20 + 12], al

    pop     ecx                  ; count of bytes to send
    mov     ebx, ecx            ; need the length later

    cmp     ebx, 0
    jz      btp_001

    mov     edi, edx
    add     edi, 40
    cld
    rep     movsb               ; copy the data across

btp_001:
    ; we have edx as IPbuffer ptr.
    ; Fill in the TCP checksum
    ; First, fill in pseudoheader
    mov     eax, [edx + 12]
    mov     [pseudoHeader], eax
    mov     eax, [edx + 16]
    mov     [pseudoHeader+4], eax
    mov     ax, 0x0600            ; 0 + protocol
    mov     [pseudoHeader+8], ax
    add     ebx, 20
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
    mov     [checkSize2], ax

    call    checksum

    ; store it in the TCP checksum ( in the correct order! )
    mov     ax, [checkResult]

    mov     [edx + 20 + 16], ah
    mov     [edx + 20 + 17], al

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

    ret


; Increments the 32 bit value pointed to by esi in internet order
inc_inet_esi:
    push    eax
    add     esi, 3
    mov     al, byte[esi]
    inc     al
    mov     byte[esi], al
    cmp     al, 0
    jnz     iie_exit
    dec     esi
    mov     al, byte[esi]
    inc     al
    mov     byte[esi], al
    cmp     al, 0
    jnz     iie_exit
    dec     esi
    mov     al, byte[esi]
    inc     al
    mov     byte[esi], al
    cmp     al, 0
    jnz     iie_exit
    dec     esi
    mov     al, byte[esi]
    inc     al
    mov     byte[esi], al

iie_exit:
    pop     eax
    ret


; Increments the 32 bit value pointed to by esi in internet order
; by the value in ecx
add_inet_esi:
    push    eax

    mov     al, [esi]
    shl     eax, 8
    inc     esi
    mov     al, [esi]
    shl     eax, 8
    inc     esi
    mov     al, [esi]
    shl     eax, 8
    inc     esi
    mov     al, [esi]
    add     eax, ecx
    mov     [esi], al
    dec     esi
    shr     eax, 8
    mov     [esi], al
    dec     esi
    shr     eax, 8
    mov     [esi], al
    dec     esi
    shr     eax, 8
    mov     [esi], al
    pop     eax
    ret


iglobal
  TCBStateHandler:
    dd      stateTCB_LISTEN
    dd      stateTCB_SYN_SENT
    dd      stateTCB_SYN_RECEIVED
    dd      stateTCB_ESTABLISHED
    dd      stateTCB_FIN_WAIT_1
    dd      stateTCB_FIN_WAIT_2
    dd      stateTCB_CLOSE_WAIT
    dd      stateTCB_CLOSING
    dd      stateTCB_LAST_ACK
    dd      stateTCB_TIME_WAIT
    dd      stateTCB_CLOSED
endg

;***************************************************************************
;   Function
;      tcpStateMachine
;
;   Description
;       TCP state machine
;       This is a kernel function, called by tcp_rx
;
;       IP buffer address given in edx
;          Socket/TCB address in [eax + sockets]
;
;       The IP buffer will be released by the caller
;***************************************************************************
tcpStateMachine:
    mov     ebx, sockets
    add     ebx, eax
    mov     [sktAddr], ebx

    ; as a packet has been received, update the TCB timer
    mov     ecx, TWOMSL
    mov     [ebx + 32], ecx

    ; If the received packet has an ACK bit set,
    ; remove any packets in the resend queue that this
    ; received packet acknowledges
    pusha
    mov     cl, [edx + 33]
    and     cl, 0x10
    cmp     cl, 0x10
    jne     tsm001                      ; No ACK, so no data yet


    ; get skt number in al
    shr     eax, 12

    ; The ack number is in [edx + 28], inet format
    ; skt in al

    mov     esi, resendQ
    mov     ecx, 0

t001:
    cmp     ecx, NUMRESENDENTRIES
    je      t003              ; None left
    cmp     [esi], al
    je      t002              ; found one
    inc     ecx
    add     esi, 4
    jmp     t001

t002:                   ; Can we delete this buffer?

                        ; If yes, goto t004. No, goto t001
    ; Get packet data address

    push    ecx
    inc     ecx
    ; Now get buffer location, and copy buffer across. argh! more copying,,
    mov     edi, resendBuffer - IPBUFFSIZE
t002a:
    add     edi, IPBUFFSIZE
    loop    t002a

    ; we have dest buffer location in edi. incoming packet in edx.
    ; Get this packets sequence number
    ; preserve al, ecx, esi, edx

    mov     cl, [edi + 24]
    shl     ecx, 8
    mov     cl, [edi + 25]
    shl     ecx, 8
    mov     cl, [edi + 26]
    shl     ecx, 8
    mov     cl, [edi + 27]
    movzx   ebx, byte [edi + 3]
    mov     bh, [edi + 2]
    sub     ebx, 40
    add     ecx, ebx          ; ecx is now seq# of last byte +1, intel format

    ; get recievd ack #, in intel format
    mov     bl, [edx + 28]
    shl     ebx, 8
    mov     bl, [edx + 29]
    shl     ebx, 8
    mov     bl, [edx + 30]
    shl     ebx, 8
    mov     bl, [edx + 31]

    cmp     ebx, ecx        ; Finally. ecx = rx'ed ack. ebx = last byte in que
                            ; DANGER! need to handle case that we have just
                            ; passed the 2**32, and wrapped round!
    pop     ecx

    jae     t004             ; if rx > old, delete old
    inc     ecx
    add     esi, 4
    jmp     t001


t004:
    dec     dword [arp_rx_count] ; ************ TEST ONLY!

    mov     [esi], byte 0xFF
    inc     ecx
    add     esi, 4
    jmp     t001

t003:

tsm001:
    popa

    ; Call handler for given TCB state
    mov     ebx, [eax + sockets+28]
    cmp     ebx, TCB_LISTEN
    jb      tsm_exit
    cmp     ebx, TCB_CLOSED
    ja      tsm_exit

    dec     ebx
    call    dword [TCBStateHandler+ebx*4]

tsm_exit:
    ret



stateTCB_LISTEN:
    ; In this case, we are expecting a SYN packet
    ; For now, if the packet is a SYN, process it, and send a response
    ; If not, ignore it

    ; Look at control flags
    mov     bl, [edx + 33]
    and     bl, 0x02
    cmp     bl, 0x02
    jnz     stl_exit

    ; We have a SYN. update the socket with this IP packets details,
    ; And send a response

    mov     ebx, [edx + 12] ; IP source address
    mov     [eax + sockets + 16], ebx
    mov     bx, [edx + 20] ; IP source port
    mov     [eax + sockets + 20], bx
    mov     ebx, [edx + 24] ; IRS
    mov     [eax + sockets + 40], ebx
    mov     [eax + sockets + 56], ebx
    mov     esi, sockets
    add     esi, eax
    add     esi, 56
    call    inc_inet_esi ; RCV.NXT
    mov     ebx, [eax + sockets + 36]    ; ISS
    mov     [eax + sockets + 48], ebx    ; SND.NXT

    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      stl_exit

    push    eax
    mov     bl, 0x12        ; SYN + ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE
    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     stl_notlocal
    mov     eax, IPIN_QUEUE

stl_notlocal:
       ; Send it.
    pop     ebx
    call    queue


    mov     ebx, TCB_SYN_RECEIVED
    mov     esi, [sktAddr]
    mov     [esi + 28], ebx

    ; increament SND.NXT in socket
    add     esi, 48
    call    inc_inet_esi

stl_exit:
    ret



stateTCB_SYN_SENT:
    ; We are awaiting an ACK to our SYN, with a SYM
    ; Look at control flags - expecting an ACK
    mov     bl, [edx + 33]
    and     bl, 0x12
    cmp     bl, 0x12
    jnz     stss_exit

    mov     ebx, TCB_ESTABLISHED
    mov     esi, [sktAddr]
    mov     [esi + 28], ebx

    ; Store the recv.nxt field
    mov     eax, [edx + 24]

    ; Update our recv.nxt field
    mov     esi, [sktAddr]
    add     esi, 56
    mov     [esi], eax
    call    inc_inet_esi

    ; Send an ACK
    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      stss_exit

    push    eax

    mov     bl, 0x10        ; ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     stss_notlocal
    mov     eax, IPIN_QUEUE

stss_notlocal:
       ; Send it.
    pop     ebx
    call    queue

stss_exit:
    ret



stateTCB_SYN_RECEIVED:
    ; In this case, we are expecting an ACK packet
    ; For now, if the packet is an ACK, process it,
    ; If not, ignore it

    ; Look at control flags - expecting an ACK
    mov     bl, [edx + 33]
    and     bl, 0x10
    cmp     bl, 0x10
    jnz     stsr_exit

    mov     ebx, TCB_ESTABLISHED
    mov     esi, [sktAddr]
    mov     [esi + 28], ebx

stsr_exit:
    ret



stateTCB_ESTABLISHED:
    ; Here we are expecting data, or a request to close
    ; OR both...

    ; Did we receive a FIN or RST?
    mov     bl, [edx + 33]
    and     bl, 0x05
    cmp     bl, 0
    je      ste_chkack

    ; It was a fin or reset.

    ; Remove resend entries from the queue  - I dont want to send any more data
    pusha

    mov     ebx, [sktAddr]
    sub     ebx, sockets
    shr     ebx, 12             ; get skt #

    mov     esi, resendQ
    mov     ecx, 0

ste001:
    cmp     ecx, NUMRESENDENTRIES
    je      ste003              ; None left
    cmp     [esi], bl
    je      ste002              ; found one
    inc     ecx
    add     esi, 4
    jmp     ste001

ste002:
    dec     dword [arp_rx_count] ; ************ TEST ONLY!

    mov     [esi], byte 0xFF
    jmp     ste001

ste003:
    popa

    ; was it a reset?
    mov     bl, [edx + 33]
    and     bl, 0x04
    cmp     bl, 0x04
    jne     ste003a

    mov     esi, [sktAddr]
    mov     ebx, TCB_CLOSED
    mov     [esi + 28], ebx
    jmp     ste_exit

ste003a:
    ; Send an ACK to that fin, and enter closewait state

    mov     esi, [sktAddr]
    mov     ebx, TCB_CLOSE_WAIT
    mov     [esi + 28], ebx
    add     esi, 56
    mov     eax, [esi]              ; save original
    call    inc_inet_esi
    ;; jmp    ste_ack - NO, there may be data

ste_chkack:
    ; Check that we received an ACK
    mov     bl, [edx + 33]
    and     bl, 0x10
    cmp     bl, 0x10
    jnz     ste_exit


    ; TODO - done, I think!
    ; First, look at the incoming window. If this is less than or equal to 1024,
    ; Set the socket window timer to 1. This will stop an additional packets being
    ; queued.
    ; ** I may need to tweak this value, since I do not know how many packets are already queued
    mov     ch, [edx + 34]
    mov     cl, [edx + 35]
    cmp     cx, 1024
    ja      ste004

    mov     ecx, [sktAddr]
    mov     [ecx+72], dword 1

ste004:

    ; OK, here is the deal
    ; My recv.nct field holds the seq of the expected next rec byte
    ; if the recevied sequence number is not equal to this, do not
    ; increment the recv.nxt field, do not copy data - just send a
    ; repeat ack.

    ; recv.nxt is in dword [edx+24], in inext format
    ; recv seq is in [sktAddr]+56, in inet format
    ; just do a comparision
    mov     ecx, [sktAddr]
    add     ecx, 56

    cmp     [ecx - 56 + 28], dword TCB_CLOSE_WAIT
    mov     ecx, [ecx]
    jne     stenofin
    mov     ecx, eax

stenofin:
    cmp     ecx, [edx+24]
    jne     ste_ack


    ; Read the data bytes, store in socket buffer
    xor     ecx, ecx
    mov     ch, [edx + 2]
    mov     cl, [edx + 3]
    sub     ecx, 40                    ; Discard 40 bytes of header

    cmp     ecx, 0
    jnz     ste_data                ; Read data, if any

    ; If we had received a fin, we need to ACK it.
    mov     esi, [sktAddr]
    mov     ebx, [esi + 28]
    cmp     ebx, TCB_CLOSE_WAIT
    jz      ste_ack
    jnz     ste_exit

ste_data:
    push    ecx
    mov     esi, [sktAddr]

    add     [esi + 24], ecx      ; increment the count of bytes in buffer

    mov     eax, [esi + 4]       ; get socket owner PID
    push    eax

    mov     eax, [esi + 24]      ; get # of bytes already in buffer

    ; point to the location to store the data
    add     esi, eax
    sub     esi, ecx
    add     esi, SOCKETHEADERSIZE

    add     edx, 40        ; edx now points to the data
    mov     edi, esi
    mov     esi, edx

    cld
    rep     movsb          ; copy the data across

    ; flag an event to the application
    pop     eax
    mov     ecx,1
    mov     esi,0x3020+TASKDATA.pid

news:
    cmp     [esi],eax
    je      foundPID1
    inc     ecx
    add     esi,0x20
    cmp     ecx,[0x3004]
    jbe     news

foundPID1:
    shl     ecx,8
    or      dword [ecx+0x80000+APPDATA.event_mask],dword 10000000b ; stack event

    pop     ecx

    ; Update our recv.nxt field
    mov     esi, [sktAddr]
    add     esi, 56
    call    add_inet_esi

ste_ack:
    ; Send an ACK
    ; Now construct the response, and queue for sending by IP
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      ste_exit

    push    eax

    mov     bl, 0x10        ; ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     ste_notlocal
    mov     eax, IPIN_QUEUE
ste_notlocal:

       ; Send it.
    pop     ebx
    call    queue

ste_exit:
    ret



stateTCB_FIN_WAIT_1:
    ; We can either receive an ACK of a fin, or a fin
    mov     bl, [edx + 33]
    and     bl, 0x10
    cmp     bl, 0x10
    jnz     stfw1_001

    ; It was an ACK
    mov     esi, [sktAddr]
    mov     ebx, TCB_FIN_WAIT_2
    mov     [esi + 28], ebx
    jmp     stfw1_exit

stfw1_001:
    ; It must be a fin then
    mov     esi, [sktAddr]
    mov     ebx, TCB_CLOSING
    mov     [esi + 28], ebx
    add     esi, 56
    call    inc_inet_esi

    ; Send an ACK
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      stfw1_exit

    push    eax

    mov     bl, 0x10        ; ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket
    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     stfw1_notlocal
    mov     eax, IPIN_QUEUE

stfw1_notlocal:
    ; Send it.
    pop     ebx
    call    queue

stfw1_exit:
    ret



stateTCB_FIN_WAIT_2:
    mov     esi, [sktAddr]

    ; Get data length
    xor     ecx, ecx
    mov     ch, [edx+2]
    mov     cl, [edx+3]
    sub     ecx, 40

    mov     bl, [edx + 33]
    and     bl, 0x01
    cmp     bl, 0x01
    jne     stfw2001

    ; Change state, as we have a fin
    mov     ebx, TCB_TIME_WAIT
    mov     [esi + 28], ebx

    inc     ecx                     ; FIN is part of the sequence space

stfw2001:
    add     esi, 56
    call    add_inet_esi

    ; Send an ACK
    mov     eax, EMPTY_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      stfw2_exit

    push    eax

    mov     bl, 0x10        ; ACK
    mov     ecx, 0
    mov     esi, 0

    call    buildTCPPacket

    mov     eax, NET1OUT_QUEUE

    mov     edx, [stack_ip]
    mov     ecx, [ sktAddr ]
    mov     ecx, [ ecx + 16 ]
    cmp     edx, ecx
    jne     stfw2_notlocal
    mov     eax, IPIN_QUEUE

stfw2_notlocal:
       ; Send it.
    pop     ebx
    call    queue

    ; Only delete the socket if we received the FIN

    mov     bl, [edx + 33]
    and     bl, 0x01
    cmp     bl, 0x01
    jne     stfw2_exit

;    mov     edi, [sktAddr]

    ; delete the socket. Should really wait for 2MSL
;    xor     eax, eax
;    mov     ecx,SOCKETHEADERSIZE
;    cld
;    rep     stosb

stfw2_exit:
    ret



stateTCB_CLOSE_WAIT:
    ; Intentionally left empty
    ; socket_close_tcp handles this
    ret



stateTCB_CLOSING:
    ; We can either receive an ACK of a fin, or a fin
    mov     bl, [edx + 33]
    and     bl, 0x10
    cmp     bl, 0x10
    jnz     stc_exit

    ; It was an ACK

    mov     edi, [sktAddr]

    ; delete the socket
    xor     eax, eax
    mov     ecx,SOCKETHEADERSIZE
    cld
    rep     stosb

stc_exit:
    ret



stateTCB_LAST_ACK:
    ; Look at control flags - expecting an ACK
    mov     bl, [edx + 33]
    and     bl, 0x10
    cmp     bl, 0x10
    jnz     stla_exit

    mov     edi, [sktAddr]

    ; delete the socket
    xor     eax, eax
    mov     ecx,SOCKETHEADERSIZE
    cld
    rep     stosb

stla_exit:
    ret



stateTCB_TIME_WAIT:
    ret



stateTCB_CLOSED:
    ret
