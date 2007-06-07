$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  UDP.INC                                                     ;;
;;                                                              ;;
;;  UDP Processes for Menuet OS  TCP/IP stack                   ;;
;;                                                              ;;
;;  Version 0.3  29 August 2002                                 ;;
;;                                                              ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net            ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;*******************************************************************
;   Interface
;
;       udp_rx      Handles received IP packets with the UDP protocol
;
;*******************************************************************


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

struc UDP_PACKET
{  .SourcePort       dw  ?  ;+00
   .DestinationPort  dw  ?  ;+02
   .Length           dw  ?  ;+04 - Length of (UDP Header + Data)
   .Checksum         dw  ?  ;+06
   .Data             db  ?  ;+08
}

virtual at 0
  UDP_PACKET UDP_PACKET
end virtual


;***************************************************************************
;   Function
;      udp_rx  [by Johnny_B]
;
;   Description
;       UDP protocol handler
;       This is a kernel function, called by ip_rx
;       IP buffer address given in edx
;          IP buffer number in eax
;          Free up (or re-use) IP buffer when finished
;
;***************************************************************************
udp_rx:
    push    eax

    ; First validate the header & checksum. Discard buffer if error

    ; Look for a socket where
    ; IP Packet UDP Destination Port = local Port
    ; IP Packet SA = Remote IP

    movzx   ebx, word [edx + 22]   ; get the local port from
                                  ; the IP packet's UDP header
    mov     eax, SOCKETBUFFSIZE * NUM_SOCKETS
    mov     ecx, NUM_SOCKETS

fs1:
    sub     eax, SOCKETBUFFSIZE
    cmp     [eax + sockets + 12], bx ; bx will hold the 'wrong' value,
                                    ; but the comparision is correct
    loopnz  fs1                     ; Return back if no match
    jz      fs_done

    ; No match, so exit
    jmp     udprx_001

fs_done:
    ; For dhcp, we must allow any remote server to respond.
    ; I will accept the first incoming response to be the one
    ; I bind to, if the socket is opened with a destination IP address of
    ; 255.255.255.255
    mov     ebx, [eax + sockets + 16]
    cmp     ebx, 0xffffffff
    je      udprx_002

    mov     ebx, [edx + 12]    ; get the Source address from the IP packet
    cmp     [eax + sockets + 16], ebx
    jne     udprx_001          ; Quit if the source IP is not valid

udprx_002:
    ; OK - we have a valid UDP packet for this socket.
    ; First, update the sockets remote port number with the incoming msg
    ; - it will have changed
    ; from the original ( 69 normally ) to allow further connects
    movzx   ebx, word [edx + 20]      ; get the UDP source port
                                     ; ( was 69, now new )
    mov     [eax + sockets + 20], bx

    ; Now, copy data to socket. We have socket address as [eax + sockets].
    ; We have IP packet in edx

    ; get # of bytes in ecx
    movzx   ecx, byte [edx + 3]  ; total length of IP packet. Subtract
    mov     ch, byte [edx + 2]   ; 20 + 8 gives data length
    sub     ecx, 28

    mov     ebx, eax
    add     ebx, sockets         ; ebx = address of actual socket

    mov     eax, [ebx+ 4]       ; get socket owner PID
    push    eax

    mov     eax, [ebx + 24]      ; get # of bytes already in buffer
    add     [ebx + 24], ecx      ; increment the count of bytes in buffer

    ; point to the location to store the data
    add     ebx, eax
    add     ebx, SOCKETHEADERSIZE

    ; ebx = location for first byte, ecx has count,
    ; edx points to data

    add     edx, 28        ; edx now points to the data
    mov     edi, ebx
    mov     esi, edx

    cld
    rep     movsb          ; copy the data across

    ; flag an event to the application
    pop     eax
    mov     ecx,1
    mov     esi,TASK_DATA+TASKDATA.pid

newsearch:
    cmp     [esi],eax
    je      foundPID
    inc     ecx
    add     esi,0x20
    cmp     ecx,[TASK_COUNT]
    jbe     newsearch

foundPID:
    shl     ecx,8
    or      dword [ecx+SLOT_BASE+APPDATA.event_mask],dword 10000000b ; stack event

    mov     [check_idle_semaphore],200

udprx_001:
    pop     eax
    call    freeBuff    ; Discard the packet
    ret
