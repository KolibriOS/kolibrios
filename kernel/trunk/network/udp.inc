;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  UDP.INC                                                     ;;
;;                                                              ;;
;;  UDP Processes for Menuet OS  TCP/IP stack                   ;;
;;                                                              ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net            ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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

proc udp_rx stdcall
        push    eax

        ; First validate the header & checksum. Discard buffer if error

        ; Look for a socket where
        ; IP Packet UDP Destination Port = local Port
        ; IP Packet SA = Remote IP

        mov     ax, [edx + 20 + UDP_PACKET.DestinationPort]   ; get the local port from
                                       ; the IP packet's UDP header

        mov     ebx, net_sockets

  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .exit                                   ; No match, so exit
        cmp     [ebx + SOCKET.LocalPort], ax            ; ax will hold the 'wrong' value,
                                                        ; but the comparision is correct
        jne     .next_socket                            ; Return back if no match

        ; For dhcp, we must allow any remote server to respond.
        ; I will accept the first incoming response to be the one
        ; I bind to, if the socket is opened with a destination IP address of
        ; 255.255.255.255
        cmp     [ebx + SOCKET.RemoteIP], 0xffffffff
        je      @f

        mov     eax, [edx + IP_PACKET.SourceAddress]    ; get the Source address from the IP packet
        cmp     [ebx + SOCKET.RemoteIP], eax
        jne     .exit              ; Quit if the source IP is not valid

    @@: ; OK - we have a valid UDP packet for this socket.
        ; First, update the sockets remote port number with the incoming msg
        ; - it will have changed
        ; from the original ( 69 normally ) to allow further connects
        mov     ax, [edx + 20 + UDP_PACKET.SourcePort]          ; get the UDP source port
                                                                ; ( was 69, now new )
        mov     [ebx + SOCKET.RemotePort], ax

        ; Now, copy data to socket. We have socket address as [eax + sockets].
        ; We have IP packet in edx

        ; get # of bytes in ecx
        movzx   ecx, [edx + IP_PACKET.TotalLength]      ; total length of IP packet. Subtract
        xchg    cl, ch                                  ; 20 + 8 gives data length
        sub     ecx, 28

        mov     eax, [ebx + SOCKET.rxDataCount]         ; get # of bytes already in buffer
        add     [ebx + SOCKET.rxDataCount], ecx         ; increment the count of bytes in buffer

        ; ecx has count, edx points to data

        add     edx, 28        ; edx now points to the data
        lea     edi, [ebx + eax + SOCKETHEADERSIZE]
        mov     esi, edx

        cld
        rep movsb              ; copy the data across

        ; flag an event to the application
        mov     eax, [ebx + SOCKET.PID]                 ; get socket owner PID
        mov     ecx, 1
        mov     esi, TASK_DATA + TASKDATA.pid

  .next_pid:
        cmp     [esi], eax
        je      .found_pid
        inc     ecx
        add     esi, 0x20
        cmp     ecx, [TASK_COUNT]
        jbe     .next_pid

        jmp     .exit

  .found_pid:
        shl     ecx, 8
        or      [ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK ; stack event

        mov     [check_idle_semaphore], 200

  .exit:
        pop     eax
        call    freeBuff    ; Discard the packet
        ret
endp
