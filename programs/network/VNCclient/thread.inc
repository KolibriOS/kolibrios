   thread_start:
    DEBUGF 1,'I am the thread!\n'

    mov     eax,40		       ; Report events
    mov     ebx,10000000b	       ; Only Stack
    mcall

    resolve first,[server_ip]	       ; the input window putted the server @ 'first', resolve it into a real ip
    mov     [server_port],5900	       ; no port input for now, only standard port 5900

    DEBUGF  1,'connecting to %u.%u.%u.%u:%u\n',1[server_ip],1[server_ip+1],1[server_ip+2],1[server_ip+3],4[server_port]
    eth.search_port 1000,edx					  ; Find a free port starting from 1001 and store in edx
    eth.open_tcp edx,[server_port],[server_ip],1,[socket]	  ; open socket
    DEBUGF 1,'Socket opened: %u (port %u)\n',[socket],ecx

    call    read_data
    cmp     dword[receive_buffer+1],'RFB '
    jne     no_rfb
    eth.write_tcp [socket],12,handshake
    DEBUGF 1,'Sending handshake: protocol version\n'

    call    read_data
    mov     eax,receive_buffer+1
    mov     eax,[eax]
    bswap   eax
    cmp     eax,0
    je	    invalid_security
    cmp     eax,1
    je	    no_security
    cmp     eax,2
    je	    vnc_security

    jmp     close

   vnc_security:
    mov     byte[mode],1
    call    red_logon

   no_security:
    eth.write_tcp [socket],1,shared
    DEBUGF 1,'Sending handshake: shared session?\n'

    eth.wait_for_data [socket],TIMEOUT*10,close
    eth.read_data [socket],framebuffer,[datapointer],IM_END-receive_buffer ; now the server should send init message
    DEBUGF 1,'Serverinit: bpp:%u depth:%u bigendian:%u truecolor:%u\n',1[pixelformat.bpp],1[pixelformat.depth],1[pixelformat.big_endian],1[pixelformat.true_color]
    mov     eax,dword[framebuffer]
    bswap   eax
    mov     dword[screen],eax

    eth.write_tcp [socket],20,pixel_format8
    DEBUGF 1,'Sending pixel format\n'
    call    read_data

;    eth.write_tcp [socket],8,encodings
;    DEBUGF 1,'Sending encoding info\n'
;    call    read_data

    mov     eax,dword[framebuffer.width]
    mov     dword[fbur.width],eax

    mov     byte[thread_ready],1

   request_rfb:
    mov     byte[fbur.inc],2	     ;;;;;;;;
    eth.write_tcp [socket],10,fbur   ;;;;;;;;;

   thread_loop:
    eth.wait_for_data [socket],1000,thread_loop

    call    read_data		   ; Read the data into the buffer

    mov     eax,[datapointer]	   ; at least 2 bytes should be received
    sub     eax,receive_buffer
    cmp     eax,1
    jle     mainloop

    DEBUGF 1,'Data received, %u bytes\n',eax

    cmp     byte[receive_buffer],0
    je	    framebufferupdate

    cmp     byte[receive_buffer],1
    je	    setcolourmapentries

    cmp     byte[receive_buffer],2
    je	    bell

    cmp     byte[receive_buffer],3
    je	    servercuttext

    jmp     thread_loop


   framebufferupdate:
    mov     ax,word[receive_buffer+2]
    xchg    al,ah
    mov     di,ax
    DEBUGF 1,'Framebufferupdate: %u frames\n',di
    mov     esi,receive_buffer+4
    jmp     rectangle_loop

   next_rectangle:
    call    drawbuffer

    dec     di
    test    di,di
    jz	    request_rfb

   rectangle_loop:
    mov     edx,[esi]
    bswap   edx
    mov     ebx,edx
    shr     edx,16
    mov     [frame.x],dx
    mov     [frame.y],bx
    add     esi,4
    mov     ecx,[esi]
    bswap   ecx
    mov     eax,ecx
    shr     ecx,16
    mov     [frame.width],cx
    mov     [frame.height],ax
    add     esi,4
    mov     eax,[esi]
    add     esi,4

    mov     ebx,esi
    sub     ebx,receive_buffer+12
    DEBUGF 1,'frame: width=%u height=%u x=%u y=%u offset:%u encoding:',2[frame.width],2[frame.height],2[frame.x],2[frame.y],ebx

    cmp     eax,0
    je	    encoding_raw
    cmp     eax,1
    je	    encoding_copyrect
    cmp     eax,2
    je	    encoding_RRE
    cmp     eax,5
    je	    encoding_hextile
    cmp     eax,16
    je	    encoding_ZRLE

    mov     ebx,esi
    sub     ebx,receive_buffer+8
    DEBUGF 1,'\nunknown encoding: %u (offset %u)\n',eax,ebx
    jmp     bell
    jmp     thread_loop

  encoding_RRE:
    DEBUGF 1,'RRE\n'

    jmp     next_rectangle

  encoding_hextile:
    DEBUGF 1,'hextile\n'

    jmp     next_rectangle

  encoding_ZRLE:
    DEBUGF 1,'ZRLE\n'

    jmp     next_rectangle


  setcolourmapentries:
    DEBUGF 1,'Server sended SetColourMapEntries message\n'

    jmp     thread_loop


  bell:
    mov     eax,55
    mov     ebx,eax
    mov     esi,beep
    mcall

    jmp     thread_loop


  servercuttext:
    DEBUGF 1,'Server cut text\n'

    jmp thread_loop



read_data:
    eth.read_data [socket],receive_buffer,[datapointer],IM_END-receive_buffer
ret