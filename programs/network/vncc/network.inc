;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  VNC client for KolibriOS                                       ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

thread_start:

        mcall   40, 0                   ; disable all events for this thread

; resolve name
        push    esp                     ; reserve stack place
        invoke  getaddrinfo, serveraddr, 0, 0, esp
        pop     esi
; test for error
        test    eax, eax
        jnz     err_dns

        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax

        DEBUGF  1, "Connecting to %u.%u.%u.%u:%u\n", \
                [sockaddr1.ip]:1, [sockaddr1.ip+1]:1, \
                [sockaddr1.ip+2]:1, [sockaddr1.ip+3]:1, \
                [sockaddr1.port]:2

        invoke  freeaddrinfo, esi

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      err_sock

        mov     [socketnum], eax
        mcall   connect, [socketnum], sockaddr1, 18
        cmp     eax, -1
        je      err_connect

        ; TODO: implement timeout
        call    wait_for_data

        cmp     dword[receive_buffer], "RFB "
        jne     err_proto
        DEBUGF  1, "received: %s\n", receive_buffer
        mcall   send, [socketnum], HandShake, 12, 0
        DEBUGF  1, "Sending handshake: protocol version\n"

        call    wait_for_data

        cmp     dword[receive_buffer], 0x01000000
        je      no_security
        cmp     dword[receive_buffer], 0x02000000
        je      vnc_security
        jmp     err_security

vnc_security:
        mov     [status], STATUS_LOGIN
        call    draw_gui

no_security:
        mcall   send, [socketnum], ClientInit, 1, 0
        DEBUGF  1, "ClientInit sent\n"

        call    wait_for_data       ; now the server should send init message

        DEBUGF  1, "Serverinit: bpp: %u depth: %u bigendian: %u truecolor: %u\n", \
        [receive_buffer+framebuffer.pixelformat.bpp]:1, \
        [receive_buffer+framebuffer.pixelformat.depth]:1, \
        [receive_buffer+framebuffer.pixelformat.big_endian]:1, \
        [receive_buffer+framebuffer.pixelformat.true_color]:1

        mov     eax, dword[receive_buffer+framebuffer.width]
        mov     dword[FramebufferUpdateRequest.width], eax
        bswap   eax
        mov     dword[screen], eax
        DEBUGF  1, "Screen width=%u, height=%u\n", [screen.width]:2, [screen.height]:2

        mcall   send, [socketnum], SetPixelFormat8, 20, 0
        DEBUGF  1, "Sending pixel format\n"

        mcall   send, [socketnum], SetEncodings, 12, 0
        DEBUGF  1, "Sending encoding info\n"

; Set main window caption to servername
        mov     ecx, dword[receive_buffer+framebuffer.name_length]
        bswap   ecx
        cmp     ecx, 64
        jbe     @f
        mov     ecx, 64
  @@:
        lea     esi, [receive_buffer+framebuffer.name]
        mov     edi, servername
        rep movsb
        mov     byte[edi], 0
        mov     [name.dash], "-"

; Tell the main thread we are ready for business!
        mov     [status], STATUS_CONNECTED

; Request initial update
        mov     [FramebufferUpdateRequest.inc], 0

request_fbu:
        DEBUGF  1, "Requesting framebuffer update\n"
        mcall   send, [socketnum], FramebufferUpdateRequest, 10, 0
        mov     [FramebufferUpdateRequest.inc], 1

thread_loop:
        call    read_data              ; Read the data into the buffer

        lodsb
        cmp     al, 0
        je      framebufferupdate
        cmp     al, 1
        je      setcolourmapentries
        cmp     al, 2
        je      bell
        cmp     al, 3
        je      servercuttext


        DEBUGF  1, "Unknown server command: %u\n", al
        jmp     thread_loop

framebufferupdate:

  @@:
        lea     eax, [esi+6]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        inc     esi     ; padding
        lodsw
        xchg    al, ah
        mov     [rectangles], ax
        DEBUGF  1, "Framebufferupdate: %u rectangles\n", ax

rectangle_loop:

  @@:
        lea     eax, [esi+12]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        xor     eax, eax
        lodsw
        xchg    al, ah
        mov     [rectangle.x], eax
        lodsw
        xchg    al, ah
        mov     [rectangle.y], eax
        lodsw
        xchg    al, ah
        mov     [rectangle.width], eax
        lodsw
        xchg    al, ah
        mov     [rectangle.height], eax

        lodsd                           ; encoding
        DEBUGF  1, "rectangle: width=%u height=%u x=%u y=%u encoding: ",\
        [rectangle.width]:2, [rectangle.height]:2, [rectangle.x]:2, [rectangle.y]:2

        cmp     eax, 0
        je      encoding_raw
        cmp     eax, 1
        je      encoding_CopyRect
        cmp     eax, 2
        je      encoding_RRE
;        cmp     eax, 5
;        je      encoding_hextile
;        cmp     eax, 15
;        je      encoding_TRLE
;        cmp     eax, 16
;        je      encoding_ZRLE

        DEBUGF  1, "\nunknown encoding: %u\n", eax
        jmp     thread_loop

next_rectangle:
        push    esi
        call    drawbuffer
        pop     esi

        dec     [rectangles]
        jnz     rectangle_loop
        jmp     request_fbu


setcolourmapentries:

        DEBUGF  1, "Server sent SetColourMapEntries message\n"

        ; TODO

        jmp     thread_loop


bell:
        mcall   55, 55, , , beep
        jmp     thread_loop


servercuttext:

        DEBUGF  1, "Server cut text\n"

  @@:
        lea     eax, [esi+7]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        add     esi, 3
        lodsd
        bswap   eax
        mov     ecx, eax

  @@:
        lea     eax, [esi+ecx]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        ; TODO: paste text to clipboard

        DEBUGF  1, "%u bytes of text\n", ecx
        add     esi, ecx
        jmp     thread_loop


read_data:
        mov     [datapointer], receive_buffer
        mov     esi, receive_buffer
  .more:
        push    ebx ecx edx esi edi
        mcall   recv, [socketnum], [datapointer], 4096, 0
        pop     edi esi edx ecx ebx
        cmp     eax, -1
        je      err_disconnected
        add     [datapointer], eax
; Check for buffer overflow
        cmp     [datapointer], receive_buffer + RECEIVE_BUFFER_SIZE
        jbe     @f
        ; Buffer is full, first needed data by program is pointed to by esi.
        ; Move all remaining data, starting from esi, to begin of buffer
        cmp     esi, receive_buffer
        je      err_proto
        mov     ecx, [datapointer]
        sub     ecx, esi
        mov     edi, receive_buffer
        rep movsb
        mov     [datapointer], edi
        mov     esi, receive_buffer
  @@:
        cmp     eax, 4096
        je      .more
        ret


wait_for_data:  ; FIXME: add timeout
        mcall   recv, [socketnum], receive_buffer, 4096, 0
        cmp     eax, -1
        je      err_disconnected
        test    eax, eax
        jz      wait_for_data
        ret


err_disconnected:
        mov     [status], STATUS_DISCONNECTED
        inc     [update_gui]
        mcall   -1

err_dns:
        mov     [status], STATUS_DNS_ERR
        inc     [update_gui]
        mcall   -1

err_sock:
        mov     [status], STATUS_SOCK_ERR
        inc     [update_gui]
        mcall   -1

err_connect:
        mov     [status], STATUS_CONNECT_ERR
        inc     [update_gui]
        mcall   -1
        ret

err_proto:
        mov     [status], STATUS_PROTO_ERR
        inc     [update_gui]
        mcall   -1
        ret

err_security:
        mov     [status], STATUS_SECURITY_ERR
        inc     [update_gui]
        mcall   -1
        ret
