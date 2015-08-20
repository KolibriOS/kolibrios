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

; Extract port number from server address
        mov     esi, serveraddr
  @@:
        lodsb
        test    al, al
        jz      .port_done
        cmp     al, ':'
        jne     @r
        mov     byte[esi-1], 0          ; replace colon with 0 byte, we dont want to upset getaddrinfo
        xor     eax, eax
        xor     ebx, ebx                ; port number
  @@:
        lodsb
        test    al, al
        jz      @f
        sub     al, '0'
        jb      err_dns
        cmp     al, 9
        ja      err_dns
        lea     ebx, [ebx*4+ebx]
        lea     ebx, [ebx*2+eax]
        jmp     @b
  @@:
        xchg    bl, bh
        mov     [sockaddr1.port], bx
  .port_done:

; Resolve hostname
        push    esp                     ; reserve stack place
        invoke  getaddrinfo, serveraddr, 0, 0, esp
        pop     esi
        test    eax, eax
        jnz     err_dns

        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        invoke  freeaddrinfo, esi

        DEBUGF  1, "Connecting to %u.%u.%u.%u:%u\n", \
        [sockaddr1.ip]:1, [sockaddr1.ip+1]:1, [sockaddr1.ip+2]:1, [sockaddr1.ip+3]:1, \
        [sockaddr1.port]:2

; Open socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        je      err_sock
        mov     [socketnum], eax

; Connect to the server
        mcall   connect, [socketnum], sockaddr1, 18
        cmp     eax, -1
        je      err_connect

; Verify handshake from server
        call    read_data
        cmp     eax, 12
        jb      err_proto
        cmp     dword[esi], "RFB "
        jne     err_proto
        add     esi, 12

; Did we get an error message already?
        cmp     eax, 16
        jb      @f
        lodsd
        test    eax, eax
        je      err_handshake
  @@:

; Reply to handshake
        DEBUGF  1, "Sending handshake\n"
        mcall   send, [socketnum], HandShake, 12, 0

; VNC 3.3 protocol: server decides security type
        call    read_data
        cmp     eax, 4
        jb      err_proto
        lodsd
        cmp     eax, 0x00000000
        je      err_handshake
        cmp     eax, 0x01000000         ; no security
        je      initialize
        cmp     eax, 0x02000000         ; VNC security
        je      vnc_security
        jmp     err_proto

vnc_security:

        lea     eax, [esi+8]
        cmp     [datapointer], eax
        jb      err_proto

        push    esi     ; pointer to message

        mov     dword[password], 0
        mov     dword[password+4], 0

        and     [USERbox.flags], not ed_focus
        or      [USERbox.flags], ed_disabled
        or      [PASSbox.flags], ed_focus

        mov     [status], STATUS_REQ_LOGIN
        or      [work], WORK_GUI
  @@:
        mcall   5, 10
        cmp     [status], STATUS_LOGIN
        je      @f
        cmp     [status], STATUS_REQ_LOGIN
        je      @r
        mcall   -1
  @@:
        DEBUGF  1, "VNC authentication\n"

; Bit reverse the password and create DES keys
        mov     ebx, dword[password]
        mov     edx, ebx
        and     ebx, 0xf0f0f0f0
        shr     ebx, 4
        and     edx, 0x0f0f0f0f
        shl     edx, 4
        or      ebx, edx
        mov     edx, ebx
        and     ebx, 0xCCCCCCCC
        shr     ebx, 2
        and     edx, 0x33333333
        shl     edx, 2
        or      ebx, edx
        mov     edx, ebx
        and     ebx, 0xAAAAAAAA
        shr     ebx, 1
        and     edx, 0x55555555
        shl     edx, 1
        or      ebx, edx
        bswap   ebx

        mov     eax, dword[password+4]
        mov     edx, eax
        and     eax, 0xf0f0f0f0
        shr     eax, 4
        and     edx, 0x0f0f0f0f
        shl     edx, 4
        or      eax, edx
        mov     edx, eax
        and     eax, 0xCCCCCCCC
        shr     eax, 2
        and     edx, 0x33333333
        shl     edx, 2
        or      eax, edx
        mov     edx, eax
        and     eax, 0xAAAAAAAA
        shr     eax, 1
        and     edx, 0x55555555
        shl     edx, 1
        or      edx, eax
        bswap   edx

        mov     edi, keys
        call    DES_create_keys

; Encrypt message with DES
        mov     esi, [esp]
        mov     ebx, dword[esi+0]
        mov     edx, dword[esi+4]
        call    encrypt_DES
        mov     esi, [esp]
        mov     dword[esi+0], ebx
        mov     dword[esi+4], edx

        mov     ebx, dword[esi+8]
        mov     edx, dword[esi+12]
        call    encrypt_DES
        mov     esi, [esp]
        mov     dword[esi+8], ebx
        mov     dword[esi+12], edx

; Blank out the password and key fields in RAM
        mov     edi, password
        mov     ecx, 384/4
        xor     eax, eax
        rep     stosd

; Send the authentication response to server
        pop     edx
        mcall   send, [socketnum], , 16, 0

securityresult:
; Wait for SecurityResult from server
        call    read_data
        cmp     eax, 4
        jb      err_proto
        cmp     dword[esi], 0           ; OK
        jne     err_login

initialize:
        DEBUGF  1, "Sending ClientInit\n"
        mcall   send, [socketnum], ClientInit, 1, 0

        call    read_data               ; now the server should send init message
        cmp     eax, ServerInit.name
        jb      err_proto

        DEBUGF  2, "Serverinit: bpp: %u depth: %u bigendian: %u truecolor: %u\n", \
        [esi+ServerInit.pixelformat.bpp]:1, \
        [esi+ServerInit.pixelformat.depth]:1, \
        [esi+ServerInit.pixelformat.big_endian]:1, \
        [esi+ServerInit.pixelformat.true_color]:1

        mov     eax, dword[esi+ServerInit.width]
        mov     dword[FramebufferUpdateRequest.width], eax
        bswap   eax
        mov     dword[screen], eax
        DEBUGF  1, "Screen width=%u, height=%u\n", [screen.width]:2, [screen.height]:2

; Set main window caption to servername
        mov     ecx, dword[esi+ServerInit.name_length]
        bswap   ecx
        add     esi, ServerInit.name
        lea     eax, [esi+ecx]
        cmp     [datapointer], eax
        jb      err_proto
        cmp     ecx, 64         ; Limit name length to 64 chars
        jbe     @f
        mov     ecx, 64
  @@:
        mov     edi, servername
        rep movsb
        mov     byte[edi], 0
        mov     [name.dash], "-"

        DEBUGF  1, "Sending pixel format\n"
        mcall   send, [socketnum], SetPixelFormat, 20, 0

        DEBUGF  1, "Sending encoding info\n"
        mcall   send, [socketnum], SetEncodings, SetEncodings.length, 0

; Tell the main thread we are ready for business!
        mov     [status], STATUS_CONNECTED

; Request initial framebuffer update from server
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

        DEBUGF  2, "Unknown server command: %u\n", al
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
        bswap   eax
        DEBUGF  1, "Rectangle: x=%u y=%u width=%u height=%u encoding: ",\
        [rectangle.x]:2, [rectangle.y]:2, [rectangle.width]:2, [rectangle.height]:2

        cmp     eax, 0
        je      encoding_raw
        cmp     eax, 1
        je      encoding_CopyRect
        cmp     eax, 2
        je      encoding_RRE
        cmp     eax, 15
        je      encoding_TRLE
        cmp     eax, 16
        je      encoding_ZRLE
        cmp     eax, 0xffffff11
        je      encoding_cursor

        DEBUGF  2, "unknown encoding: %u\n", eax
        jmp     thread_loop

next_rectangle:
        or      [work], WORK_FRAMEBUFFER
        dec     [rectangles]
        jnz     rectangle_loop
        jmp     request_fbu


setcolourmapentries:

        DEBUGF  1, "Server sent SetColourMapEntries message\n"

  @@:
        lea     eax, [esi+5]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        inc     esi             ; padding

        xor     eax, eax
        lodsw                   ; first color (just ignore for now)

        lodsw                   ; number of colors (use to find end of message)
        xchg    al, ah
        lea     eax, [eax*2+eax]
        shl     eax, 1
  @@:
        push    eax
        add     eax, esi
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        pop     eax
        jmp     @b
  @@:
        pop     eax

        add     esi, eax        ; Just skip it for now.
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
        neg     esi
        add     esi, receive_buffer + RECEIVE_BUFFER_SIZE
        jz      .buffer_end_reached
        xor     edi, edi
        mcall   recv, [socketnum], [datapointer]
        pop     edi esi edx ecx ebx
        cmp     eax, -1
        je      err_sock
        test    eax, eax
        jz      err_disconnected
        add     [datapointer], eax
        ret

  .buffer_end_reached:
        DEBUGF  1, "end of buffer reached, re-organizing\n"
        pop     edi esi edx ecx ebx
        ; Buffer is full, first needed data by program is pointed to by esi.
        ; Move all usefull data to begin of buffer
        cmp     esi, receive_buffer
        je      err_proto
        mov     ecx, [datapointer]
        sub     ecx, esi
        mov     edi, receive_buffer
        rep movsb
        mov     [datapointer], edi      ; new end of data
        mov     esi, receive_buffer     ; new start of data
        jmp     .more


err_disconnected:
        mov     [status], STATUS_DISCONNECTED
        or      [work], WORK_GUI
        mcall   -1

err_dns:
        mov     [status], STATUS_DNS_ERR
        or      [work], WORK_GUI
        mcall   -1

err_sock:
; TODO: distinguish between different socket errors!
        DEBUGF  2, "Socket error: %u\n", ebx
        mov     [status], STATUS_SOCK_ERR
        or      [work], WORK_GUI
        mcall   -1

err_connect:
        mov     [status], STATUS_CONNECT_ERR
        or      [work], WORK_GUI
        mcall   -1
        ret

err_proto:
        mov     [status], STATUS_PROTO_ERR
        or      [work], WORK_GUI
        mcall   -1
        ret

err_handshake:
        mov     [status], STATUS_SECURITY_ERR

        lodsd                   ; Custom message from server?
        test    eax, eax
        jz      .no_msg
        bswap   eax
        mov     ecx, eax
        cmp     ecx, 512
        jb      @f
        mov     ecx, 512
  @@:
        mov     edi, sz_err_security_c
        rep movsb
        mov     byte[edi], 0
        mov     [status], STATUS_SECURITY_ERR_C
  .no_msg:

        or      [work], WORK_GUI
        mcall   -1
        ret

err_login:
        mov     [status], STATUS_LOGIN_FAILED
        or      [work], WORK_GUI
        mcall   -1
        ret
