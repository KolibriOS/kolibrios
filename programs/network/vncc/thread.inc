
thread_start:

        DEBUGF  1, 'I am the thread!\n'

        mcall   40, 0

; resolve name
        push    esp     ; reserve stack place
        invoke  getaddrinfo, first, 0, 0, esp
        pop     esi
; test for error
        test    eax, eax
        jnz     exit

        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax

;        DEBUGF  1, 'Connecting to %u.%u.%u.%u:%u\n', \
;                [server_ip]:1, [server_ip+1]:1, \
;                [server_ip+2]:1, [server_ip+3]:1, \
;                [server_port]:2

        mcall   socket, AF_INET4, SOCK_STREAM, 0
        mov     [socketnum], eax
        mcall   connect, [socketnum], sockaddr1, 18

        call    wait_for_data

        cmp     dword [receive_buffer], 'RFB '
        jne     no_rfb
        DEBUGF  1, 'received: %s\n', receive_buffer
        mcall   send, [socketnum], handshake, 12, 0
        DEBUGF  1, 'Sending handshake: protocol version\n'

        call    wait_for_data

        cmp     dword [receive_buffer], 0x00000000
        je      invalid_security

        cmp     dword [receive_buffer], 0x01000000
        je      no_security

        cmp     dword [receive_buffer], 0x02000000
        je      vnc_security

        jmp     exit

vnc_security:
        mov     byte[mode], 1
        call    red_logon

no_security:
        mcall   send, [socketnum], shared, 1, 0
        DEBUGF  1, 'Sending handshake: shared session?\n'

        call    wait_for_data       ; now the server should send init message

        DEBUGF  1, 'Serverinit: bpp: %u depth: %u bigendian: %u truecolor: %u\n', \
                [receive_buffer+framebuffer.pixelformat.bpp]:1, \
                [receive_buffer+framebuffer.pixelformat.depth]:1, \
                [receive_buffer+framebuffer.pixelformat.big_endian]:1, \
                [receive_buffer+framebuffer.pixelformat.true_color]:1

        mov     eax, dword [receive_buffer]
        mov     dword [fbur.width], eax
        bswap   eax
        mov     dword [screen], eax

        mcall   send, [socketnum], pixel_format8, 20, 0
        DEBUGF  1, 'Sending pixel format\n'
        call    read_data

;    eth.write_tcp [socket],8,encodings
;    DEBUGF 1,'Sending encoding info\n'
;    call    read_data

        mov     byte [thread_ready], 1

request_rfb:
        mov     [fbur.inc], 2
        mcall   send, [socketnum], fbur, 10, 0

thread_loop:
        call    read_data              ; Read the data into the buffer

;        cmp     eax, 2
;        jb      mainloop

        DEBUGF 1,'Data received, %u bytes\n', eax

        cmp     byte [receive_buffer],0
        je      framebufferupdate

        cmp     byte [receive_buffer],1
        je      setcolourmapentries

        cmp     byte [receive_buffer],2
        je      bell

        cmp     byte [receive_buffer],3
        je      servercuttext

        jmp     thread_loop

align 4
framebufferupdate:

        mov     ax, word [receive_buffer+2]
        xchg    al, ah
        mov     di, ax
        DEBUGF  1, 'Framebufferupdate: %u frames\n', di
        mov     esi, receive_buffer+4
        jmp     rectangle_loop


next_rectangle:
        call    drawbuffer

        dec     di
        test    di, di
        jz      request_rfb

rectangle_loop:

        mov     edx, [esi]
        bswap   edx
        mov     ebx, edx
        shr     edx, 16
        mov     [frame.x], dx
        mov     [frame.y], bx
        add     esi, 4
        mov     ecx, [esi]
        bswap   ecx
        mov     eax, ecx
        shr     ecx, 16
        mov     [frame.width], cx
        mov     [frame.height], ax
        add     esi, 4
        mov     eax, [esi]
        add     esi, 4

        mov     ebx, esi
        sub     ebx, receive_buffer+12
        DEBUGF  1, 'frame: width=%u height=%u x=%u y=%u offset:%u encoding:',\
                [frame.width]:2, [frame.height]:2, [frame.x]:2, [frame.y]:2, ebx

        cmp     eax, 0
        je      encoding_raw
;        cmp     eax, 1
;        je      encoding_copyrect
        cmp     eax, 2
        je      encoding_RRE
        cmp     eax, 5
        je      encoding_hextile
        cmp     eax, 16
        je      encoding_ZRLE

        mov     ebx, esi
        sub     ebx, receive_buffer+8
        DEBUGF  1, '\nunknown encoding: %u (offset %u)\n', eax, ebx
        jmp     bell
        jmp     thread_loop

encoding_RRE:

        DEBUGF  1, 'RRE\n'

        jmp     next_rectangle

encoding_hextile:

        DEBUGF  1, 'hextile\n'

        jmp     next_rectangle

encoding_ZRLE:

        DEBUGF  1, 'ZRLE\n'

        jmp     next_rectangle

setcolourmapentries:

        DEBUGF  1, 'Server sent SetColourMapEntries message\n'

        jmp     thread_loop


bell:
        mcall   55, 55, , , beep

        jmp     thread_loop


servercuttext:

        DEBUGF  1, 'Server cut text\n'

        jmp     thread_loop


read_data:
        mov     [datapointer], receive_buffer
  .loop:
        mcall   recv, [socketnum], [datapointer], 4096, 0
        cmp     eax, -1
        je      .done

        add     [datapointer], eax

        cmp     eax, 4096
        je      .loop

  .done:
        mov     eax, [datapointer]
        sub     eax, receive_buffer
        ret



wait_for_data:
        mcall   recv, [socketnum], receive_buffer, 4096, 0
        cmp     eax, -1
        je      wait_for_data
        test    eax, eax
        jz      wait_for_data

        ret

