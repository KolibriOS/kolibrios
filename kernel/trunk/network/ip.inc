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


;*******************************************************************
;   Interface
;
;       ip_rx       processes all packets received by the network layer
;                   It calls the appropriate protocol handler
;
;
;
;*******************************************************************


;***************************************************************************
;   Function
;      ip_rx
;
;   Description
;       Handles received IP packets
;       This is a kernel function, called by stack_handler
;
;***************************************************************************
ip_rx:
    ; Look for a buffer to tx
    mov     eax, IPIN_QUEUE
    call    dequeue
    cmp     ax, NO_BUFFER
    je      ipr_exit         ; Exit if no buffer available

    push    eax

    ; convert buffer pointer eax to the absolute address
    mov     ecx, IPBUFFSIZE
    mul     ecx
    add     eax, IPbuffs

    mov     edx, eax  ; Save the address in  edx for use by future processes

    ; Validate the IP checksum
    mov     ebx, edx
    mov     ah, [ebx + 10]
    mov     al, [ebx + 11]      ; Get the checksum in intel format
    mov     [ebx + 10], word 0  ; clear checksum field - need to when
                                    ; recalculating checksum

    ;  this needs two data pointers and two size #.
    ;  2nd pointer can be of length 0
    mov     ebx, edx
    mov     [checkAdd1], ebx
    mov     [checkSize1], word 20
    mov     [checkAdd2], dword 0
    mov     [checkSize2], word 0

    call    checksum           ; Recalculate IP checksum
    cmp     ax, [checkResult]
    jnz     ipr_dump

    ; If the IP address is 255.255.255.255, accept it
    ; - it is a broadcast packet, which we need for dhcp
    mov     eax, [edx + 16]
    cmp     eax, 0xffffffff
    je      ipr_p0

    ; Validate the IP address, if it isn't broadcast
    cmp     eax, [stack_ip]
    jnz     ipr_dump

ipr_p0:
    mov     al, [edx]
    and     al, 0x0f
    cmp     al, 0x05
    jnz     ipr_dump

    cmp     [edx+8], byte 0
    jz      ipr_dump

    mov     ax, [edx + 6]
    and     ax, 0xFFBF
    cmp     ax, 0
    jnz     ipr_dump

    ; Check the protocol, and call the appropriate handler
    ; Each handler will re-use or free the queue buffer as appropriate
    mov     al, [edx + 9]
    cmp     al , PROTOCOL_ICMP
    jnz     ipr_p1
    pop     eax
    call    icmp_rx
    jmp     ipr_exit

ipr_p1:
    cmp     al , PROTOCOL_TCP
    jnz     ipr_p2
    pop     eax
    call    tcp_rx
    jmp     ipr_exit

ipr_p2:
    cmp     al , PROTOCOL_UDP
    jnz     ipr_dump
    pop     eax
    call    udp_rx
    jmp     ipr_exit

ipr_dump:
    ; No protocol handler available, so
    ; silently dump the packet, freeing up the queue buffer

;    inc     dword [dumped_rx_count]

    pop     eax
    call    freeBuff

ipr_exit:
    ret



;***************************************************************************
;   Function
;      icmp_rx
;
;   Description
;       ICMP protocol handler
;       This is a kernel function, called by ip_rx
;       edx contains the address of the buffer in use.
;       This buffer must be reused or marked as empty afterwards
;
;***************************************************************************
icmp_rx:
    cmp     [edx + 20], byte 8 ; Is this an echo request? discard if not
    jz      icmp_echo

    call    freeBuff
    jmp     icmp_exit

icmp_echo:
    push    eax
    mov     [edx + 10], word 0  ; I think this was already done by IP rx

    ; swap the source and destination addresses
    mov     ecx, [edx + 16]
    mov     eax, [edx + 12]
    mov     [edx + 16], eax
    mov     [edx + 12], ecx

    ; recaluculate the IP header checksum

    mov     ebx, edx
    mov     [checkAdd1], ebx
    mov     [checkSize1], word 20
    mov     [checkAdd2], dword 0
    mov     [checkSize2], word 0

    call    checksum
    mov     ax, [checkResult]
    mov     [edx + 10], ah
    mov     [edx + 11], al      ; ?? correct byte order?

    mov     [edx + 20], byte 0  ; change the request to a response
    mov     [edx + 22], word 0  ; clear ICMP checksum prior to re-calc

    ; Calculate the length of the ICMP data ( IP payload)
    mov     ah, [edx + 2]
    mov     al, [edx + 3]
    sub     ax, 20

    mov     [checkSize1], ax
    mov     ebx, edx
    add     ebx, 20

    mov     [checkAdd1], ebx
    mov     [checkAdd2], dword 0
    mov     [checkSize2], word 0

    call    checksum

    mov     ax, [checkResult]
    mov     [edx + 22], ah
    mov     [edx + 23], al

    ; Queue packet for transmission

    pop     ebx
    mov     eax, NET1OUT_QUEUE
    call    queue

icmp_exit:
    ret