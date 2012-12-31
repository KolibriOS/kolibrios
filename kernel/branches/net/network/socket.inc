;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the TCP/IP network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org,                           ;;
;;     and Clevermouse.                                            ;;
;;                                                                 ;;
;;       Based on code by mike.dld                                 ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


struct  SOCKET

        NextPtr                 dd ? ; pointer to next socket in list
        PrevPtr                 dd ? ; pointer to previous socket in list
        Number                  dd ? ; socket number

        mutex                   MUTEX

        PID                     dd ? ; application process id
        Domain                  dd ? ; INET/UNIX/..
        Type                    dd ? ; RAW/STREAM/DGRAP
        Protocol                dd ? ; ICMP/IPv4/ARP/TCP/UDP
        errorcode               dd ?
        device                  dd ?

        options                 dd ?
        state                   dd ?
        backlog                 dw ? ; how many incomming connections that can be queued

        snd_proc                dd ?
        rcv_proc                dd ?

ends

struct  IP_SOCKET               SOCKET

        LocalIP                 rd 4 ; network byte order
        RemoteIP                rd 4 ; network byte order

ends

struct  TCP_SOCKET              IP_SOCKET

        LocalPort               dw ? ; network byte order
        RemotePort              dw ? ; network byte order

        t_state                 dd ? ; TCB state
        t_rxtshift              db ?
                                rb 3 ; align
        t_rxtcur                dd ?
        t_dupacks               dd ?
        t_maxseg                dd ?
        t_force                 dd ?
        t_flags                 dd ?

;---------------
; RFC783 page 21

; send sequence
        SND_UNA                 dd ? ; sequence number of unack'ed sent Packets
        SND_NXT                 dd ? ; next send sequence number to use
        SND_UP                  dd ? ; urgent pointer
        SND_WL1                 dd ? ; window minus one
        SND_WL2                 dd ? ;
        ISS                     dd ? ; initial send sequence number
        SND_WND                 dd ? ; send window

; receive sequence
        RCV_WND                 dd ? ; receive window
        RCV_NXT                 dd ? ; next receive sequence number to use
        RCV_UP                  dd ? ; urgent pointer
        IRS                     dd ? ; initial receive sequence number

;---------------------
; Additional variables

; receive variables
        RCV_ADV                 dd ?

; retransmit variables
        SND_MAX                 dd ?

; congestion control
        SND_CWND                dd ?
        SND_SSTHRESH            dd ?

;----------------------
; Transmit timing stuff
        t_idle                  dd ?
        t_rtt                   dd ?
        t_rtseq                 dd ?
        t_srtt                  dd ?
        t_rttvar                dd ?
        t_rttmin                dd ?
        max_sndwnd              dd ?

;-----------------
; Out-of-band data
        t_oobflags              dd ?
        t_iobc                  dd ?
        t_softerror             dd ?


;---------
; RFC 1323                              ; the order of next 4 elements may not change

        SND_SCALE               db ?
        RCV_SCALE               db ?
        requested_s_scale       db ?
        request_r_scale         db ?

        ts_recent               dd ?    ; a copy of the most-recent valid timestamp from the other end
        ts_recent_age           dd ?
        last_ack_sent           dd ?


;-------
; Timers
        timer_retransmission    dd ? ; rexmt
        timer_persist           dd ?
        timer_keepalive         dd ? ; keepalive/syn timeout
        timer_timed_wait        dd ? ; also used as 2msl timer

; extra

        ts_ecr                  dd ? ; timestamp echo reply
        ts_val                  dd ?
        temp_bits               db ?

ends

struct  UDP_SOCKET              IP_SOCKET

        LocalPort               dw ? ; network byte order
        RemotePort              dw ? ; network byte order
        firstpacket             db ?

ends


struct  ICMP_SOCKET             IP_SOCKET

        Identifier              dw ?

ends


struct  RING_BUFFER

        start_ptr               dd ? ; Pointer to start of buffer
        end_ptr                 dd ? ; pointer to end of buffer
        read_ptr                dd ? ; Read pointer
        write_ptr               dd ? ; Write pointer
        size                    dd ? ; Number of bytes buffered

ends

struct  STREAM_SOCKET           TCP_SOCKET

        rcv                     RING_BUFFER
        snd                     RING_BUFFER

ends

struct  socket_queue_entry

        data_ptr                dd ?
        buf_ptr                 dd ?
        data_size               dd ?

ends


SOCKETBUFFSIZE          = 4096     ; in bytes

SOCKET_QUEUE_SIZE       = 10       ; maximum number ofincoming packets queued for 1 socket
; the incoming packet queue for sockets is placed in the socket struct itself, at this location from start
SOCKET_QUEUE_LOCATION   = (SOCKETBUFFSIZE - SOCKET_QUEUE_SIZE*sizeof.socket_queue_entry - sizeof.queue)

uglobal
        net_sockets     rd 4
        last_socket_num dd ?
        last_UDP_port   dw ? ; These values give the number of the last used ephemeral port
        last_TCP_port   dw ? ;
endg


;-----------------------------------------------------------------
;
; SOCKET_init
;
;-----------------------------------------------------------------
macro   SOCKET_init {

        xor     eax, eax
        mov     edi, net_sockets
        mov     ecx, 5
        rep     stosd

       @@:
        pseudo_random eax
        cmp     ax, MIN_EPHEMERAL_PORT
        jb      @r
        cmp     ax, MAX_EPHEMERAL_PORT
        ja      @r
        xchg    al, ah
        mov     [last_UDP_port], ax

       @@:
        pseudo_random eax
        cmp     ax, MIN_EPHEMERAL_PORT
        jb      @r
        cmp     ax, MAX_EPHEMERAL_PORT
        ja      @r
        xchg    al, ah
        mov     [last_TCP_port], ax

}


;-----------------------------------------------------------------
;
; SOCKET_block
;
;-----------------------------------------------------------------
macro   SOCKET_block socket, loop, done {

        test    [socket + SOCKET.options], SO_BLOCK     ; Is this a blocking socket?
        jz      done                                    ; No, return immediately

        pusha
        mov     eax, EVENT_NETWORK
        mov     ebx, 1337                   ; UID: ????
        call    wait_event
        popa

        jmp     loop

}


;-----------------------------------------------------------------
;
; Socket API (function 74)
;
;-----------------------------------------------------------------
align 4
sys_socket:

        cmp     ebx, 255
        jz      SOCKET_debug

        cmp     ebx, .number
        ja      s_error
        jmp     dword [.table + 4*ebx]

  .table:
        dd      SOCKET_open     ; 0
        dd      SOCKET_close    ; 1
        dd      SOCKET_bind     ; 2
        dd      SOCKET_listen   ; 3
        dd      SOCKET_connect  ; 4
        dd      SOCKET_accept   ; 5
        dd      SOCKET_send     ; 6
        dd      SOCKET_receive  ; 7
        dd      SOCKET_set_opt  ; 8
        dd      SOCKET_get_opt  ; 9
  .number = ($ - .table) / 4 - 1

s_error:
        DEBUGF  1,"socket error\n"
        mov     dword [esp+32], -1

        ret



;-----------------------------------------------------------------
;
; SOCKET_open
;
;  IN:  domain in ecx
;       type in edx
;       protocol in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_open:

        DEBUGF  1,"SOCKET_open: domain=%u type=%u protocol=%x\n", ecx, edx, esi

        push    ecx edx esi
        call    SOCKET_alloc
        pop     esi edx ecx
        jz      s_error

        mov     [esp+32], edi                   ; return socketnumber

        mov     [eax + SOCKET.Domain], ecx
        mov     [eax + SOCKET.Type], edx
        mov     [eax + SOCKET.Protocol], esi

        cmp     ecx, AF_INET4
        jne     .no_inet4

        cmp     edx, SOCK_DGRAM
        je      .udp

        cmp     edx, SOCK_STREAM
        je      .tcp

        cmp     edx, SOCK_RAW
        je      .raw

  .no_inet4:
        cmp     ecx, AF_PPP
        jne     .no_ppp

        cmp     esi, PPP_PROTO_ETHERNET
        je      .pppoe

  .no_ppp:
        ret

align 4
  .raw:
        test    esi, esi       ; IP_PROTO_IP
        jz      .ip

        cmp     esi, IP_PROTO_ICMP
        je      .icmp

        cmp     esi, IP_PROTO_UDP
        je      .udp

        cmp     esi, IP_PROTO_TCP
        je      .tcp

        ret

align 4
  .udp:
        mov     [eax + SOCKET.Protocol], IP_PROTO_UDP
        mov     [eax + SOCKET.snd_proc], SOCKET_send_udp
        mov     [eax + SOCKET.rcv_proc], SOCKET_receive_dgram
        ret

align 4
  .tcp:
        mov     [eax + SOCKET.Protocol], IP_PROTO_TCP
        mov     [eax + SOCKET.snd_proc], SOCKET_send_tcp
        mov     [eax + SOCKET.rcv_proc], SOCKET_receive_tcp

        TCP_init_socket eax
        ret


align 4
  .ip:
        mov     [eax + SOCKET.snd_proc], SOCKET_send_ip
        mov     [eax + SOCKET.rcv_proc], SOCKET_receive_dgram
        ret


align 4
  .icmp:
        mov     [eax + SOCKET.snd_proc], SOCKET_send_icmp
        mov     [eax + SOCKET.rcv_proc], SOCKET_receive_dgram
        ret

align 4
  .pppoe:
        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        mov     [eax + SOCKET.snd_proc], SOCKET_send_pppoe
        mov     [eax + SOCKET.rcv_proc], SOCKET_receive_dgram
        ret


;-----------------------------------------------------------------
;
; SOCKET_bind
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------------------------
align 4
SOCKET_bind:

        DEBUGF  1,"SOCKET_bind: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     esi, 2
        jb      s_error

        cmp     word [edx], AF_INET4
        je      .af_inet4

        cmp     word [edx], AF_UNIX
        je      .af_unix

        jmp     s_error

  .af_unix:
        ; TODO: write code here

        mov     dword [esp+32], 0
        ret

  .af_inet4:

        cmp     esi, 6
        jb      s_error

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .udp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        jmp     s_error

  .tcp:
  .udp:

        mov     ebx, [edx + 4]                  ; First, fill in the IP
        test    ebx, ebx                        ; If IP is 0, use default
        jnz     @f
        mov     ebx, [NET_DEFAULT]
        mov     ebx, [IP_LIST + 4*ebx]
       @@:
        mov     [eax + IP_SOCKET.LocalIP], ebx

        mov     bx, [edx + 2]                   ; Now fill in the local port if it's still available
        call    SOCKET_check_port
        jz      s_error                         ; ZF is set by socket_check_port, on error

        DEBUGF  1,"SOCKET_bind: local ip=%u.%u.%u.%u\n",\
        [eax + IP_SOCKET.LocalIP + 0]:1,[eax + IP_SOCKET.LocalIP + 1]:1,\
        [eax + IP_SOCKET.LocalIP + 2]:1,[eax + IP_SOCKET.LocalIP + 3]:1

        mov     dword [esp+32], 0
        ret




;-----------------------------------------------------------------
;
; SOCKET_connect
;
;  IN:  socket number in ecx
;       pointer to sockaddr struct in edx
;       length of that struct in esi
;  OUT: 0 on success
;
;-----------------------------------------------------------------
align 4
SOCKET_connect:

        DEBUGF  1,"SOCKET_connect: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     esi, 8
        jb      s_error

        cmp     word [edx], AF_INET4
        je      .af_inet4

        jmp     s_error

  .af_inet4:
        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST]
        pop     [eax + IP_SOCKET.LocalIP]
       @@:

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .udp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_IP
        je      .ip

        cmp     [eax + SOCKET.Protocol], IP_PROTO_ICMP
        je      .ip

        jmp     s_error

align 4
  .udp:
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        pushw   [edx + 2]
        pop     [eax + UDP_SOCKET.RemotePort]

        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

        cmp     [eax + UDP_SOCKET.LocalPort], 0
        jne     @f
        call    SOCKET_find_port
       @@:

        mov     [eax + UDP_SOCKET.firstpacket], 0

        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        mov     dword [esp+32], 0
        ret

align 4
  .tcp:
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        pushw   [edx + 2]
        pop     [eax + TCP_SOCKET.RemotePort]

        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

        cmp     [eax + TCP_SOCKET.LocalPort], 0
        jne     @f
        call    SOCKET_find_port
       @@:

        mov     [eax + TCP_SOCKET.timer_persist], 0
        mov     [eax + TCP_SOCKET.t_state], TCPS_SYN_SENT

        push    [TCP_sequence_num]
        add     [TCP_sequence_num], 6400
        pop     [eax + TCP_SOCKET.ISS]
        mov     [eax + TCP_SOCKET.timer_keepalive], TCP_time_keep_init


        TCP_sendseqinit eax

;        mov     [ebx + TCP_SOCKET.timer_retransmission],   ;; todo: create macro to set retransmission timer

        mov     ebx, eax

        lea     eax, [ebx + STREAM_SOCKET.snd]
        call    SOCKET_ring_create

        lea     eax, [ebx + STREAM_SOCKET.rcv]
        call    SOCKET_ring_create

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa

        mov     eax, ebx
        call    TCP_output

;;; TODO: wait for successfull connection if blocking socket

        mov     dword [esp+32], 0
        ret

align 4
  .ip:
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)        ; Set up data receiving queue
        pop     eax

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        mov     dword [esp+32], 0
        ret


;-----------------------------------------------------------------
;
; SOCKET_listen
;
;  IN:  socket number in ecx
;       backlog in edx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_listen:

        DEBUGF  1,"SOCKET_listen: socknum=%u backlog=%u\n", ecx, edx

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     s_error

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     s_error

        cmp     [eax + TCP_SOCKET.LocalPort], 0
        je      s_error

        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST]
        pop     [eax + IP_SOCKET.LocalIP]
       @@:

        cmp     edx, MAX_backlog
        jbe     @f
        mov     edx, MAX_backlog
       @@:

        mov     [eax + SOCKET.backlog], dx
        or      [eax + SOCKET.options], SO_ACCEPTCON
        mov     [eax + TCP_SOCKET.t_state], TCPS_LISTEN
        mov     [eax + TCP_SOCKET.timer_keepalive], 0           ; disable keepalive timer

        push    eax
        init_queue (eax + SOCKET_QUEUE_LOCATION)                ; Set up sockets queue
        pop     eax

        mov     dword [esp+32], 0

        ret


;-----------------------------------------------------------------
;
; SOCKET_accept
;
;  IN:  socket number in ecx
;       addr in edx
;       addrlen in esi
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_accept:

        DEBUGF  1,"SOCKET_accept: socknum=%u sockaddr=%x length=%u\n", ecx, edx, esi

        call    SOCKET_num_to_ptr
        jz      s_error

        test    [eax + SOCKET.options], SO_ACCEPTCON
        jz      s_error

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     s_error

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jne     s_error

  .loop:
        get_from_queue (eax + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, .block

; Ok, we got a socket ptr
        mov     eax, [esi]

; Change PID to that of the current process
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.PID], ebx

; Convert it to a socket number
        call    SOCKET_ptr_to_num
        jz      s_error
; and return it to caller
        mov     [esp+32], eax
        ret

  .block:
        SOCKET_block    eax, .loop, s_error

;-----------------------------------------------------------------
;
; SOCKET_close
;
;  IN:  socket number in ecx
;  OUT: eax is socket num, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_close:

        DEBUGF  1,"SOCKET_close: %u\n", ecx

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     s_error

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .free

        cmp     [eax + SOCKET.Protocol], IP_PROTO_ICMP
        je      .free

        cmp     [eax + SOCKET.Protocol], IP_PROTO_IP
        je      .free

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        jmp     s_error

  .tcp:
        cmp     [eax + TCP_SOCKET.t_state], TCPS_SYN_RECEIVED    ; state must be LISTEN, SYN_SENT or CLOSED
        jb      .free

        call    TCP_usrclosed
        call    TCP_output      ;;;; Fixme: is this nescessary??

;;; TODO: wait for successfull termination if blocking socket
        mov     dword [esp+32], 0

        ret

  .free:
        call    SOCKET_free
        mov     dword [esp+32], 0

        ret


;-----------------------------------------------------------------
;
; SOCKET_receive
;
;  IN:  socket number in ecx
;       addr to buffer in edx
;       length of buffer in esi
;       flags in edi
;  OUT: eax is number of bytes copied, -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_receive:

        DEBUGF  1,"SOCKET_receive: socknum=%u bufaddr=%x buflength=%u flags=%x\n", ecx, edx, esi, edi

        call    SOCKET_num_to_ptr
        jz      s_error

        jmp     [eax + SOCKET.rcv_proc]


align 4
SOCKET_receive_dgram:

        DEBUGF  1,"SOCKET_receive: DGRAM\n"

        mov     ebx, esi
        mov     edi, edx                                        ; addr to buffer

  .loop:
        get_from_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, sizeof.socket_queue_entry, .block      ; destroys esi and ecx

        mov     ecx, [esi + socket_queue_entry.data_size]
        DEBUGF  1,"SOCKET_receive: %u bytes data\n", ecx

        cmp     ecx, ebx
        ja      .too_small

        push    [esi + socket_queue_entry.buf_ptr]              ; save the buffer addr so we can clear it later
        mov     esi, [esi + socket_queue_entry.data_ptr]
        DEBUGF  1,"SOCKET_receive: Source buffer=%x real addr=%x\n", [esp], esi
        mov     [esp+32+4], ecx                                 ; return number of bytes copied

; copy the data
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep     movsd
  .nd:

        call    kernel_free                                     ; remove the packet
        ret

  .too_small:

        DEBUGF  1,"SOCKET_receive: Buffer too small\n"
        jmp     s_error

  .block:
        SOCKET_block    eax, .loop, s_error


align 4
SOCKET_receive_tcp:

        DEBUGF  1,"SOCKET_receive: TCP\n"

        mov     ecx, esi
        mov     edi, edx
        xor     edx, edx
        add     eax, STREAM_SOCKET.rcv
  .loop:
        cmp     [eax + RING_BUFFER.size], 0
        je      .block
        call    SOCKET_ring_read
        call    SOCKET_ring_free

        mov     [esp+32], ecx                                   ; return number of bytes copied
        ret

  .block:
        SOCKET_block    (eax - STREAM_SOCKET.rcv), .loop, s_error


;-----------------------------------------------------------------
;
; SOCKET_send
;
;
;  IN:  socket number in ecx
;       pointer to data in edx
;       datalength in esi
;       flags in edi
;  OUT: -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_send:

        DEBUGF  1,"SOCKET_send: socknum=%u data ptr=%x length=%u flags=%x\n", ecx, edx, esi, edi

        call    SOCKET_num_to_ptr
        jz      s_error

        mov     ecx, esi
        mov     esi, edx

        jmp     [eax + SOCKET.snd_proc]


align 4
SOCKET_send_udp:

        DEBUGF  1,"SOCKET_send: UDP\n"

        mov     [esp+32], ecx
        call    UDP_output
        cmp     eax, -1
        je      s_error
        ret


align 4
SOCKET_send_tcp:

        DEBUGF  1,"SOCKET_send: TCP\n"

        push    eax
        add     eax, STREAM_SOCKET.snd
        call    SOCKET_ring_write
        pop     eax

        mov     [esp+32], ecx

        call    TCP_output
        ret


align 4
SOCKET_send_ip:

        DEBUGF  1,"SOCKET_send: IPv4\n"

        mov     [esp+32], ecx
        call    IPv4_output_raw
        cmp     eax, -1
        je      s_error
        ret


align 4
SOCKET_send_icmp:

        DEBUGF  1,"SOCKET_send: ICMP\n"

        mov     [esp+32], ecx
        call    ICMP_output_raw
        cmp     eax, -1
        je      s_error
        ret


align 4
SOCKET_send_pppoe:

        DEBUGF  1,"SOCKET_send: PPPoE\n"

        mov     [esp+32], ecx
        mov     ebx, [eax + SOCKET.device]

        call    PPPoE_discovery_output
        cmp     eax, -1
        je      s_error
        ret




;-----------------------------------------------------------------
;
; SOCKET_get_options
;
;  IN:  ecx = socket number
;       edx = pointer to the options:
;               dd      level, optname, optval, optlen
;  OUT: -1 on error
;
; At moment, uses only pseudo-optname -2 for get last_ack_number for TCP.
; TODO: find best way to notify that send()'ed data were acknowledged
; Also pseudo-optname -3 is valid and returns socket state, one of TCPS_*.
;
;-----------------------------------------------------------------
align 4
SOCKET_get_opt:

        DEBUGF  1,"SOCKET_get_opt\n"

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     dword [edx], IP_PROTO_TCP
        jne     s_error
        cmp     dword [edx+4], -2
        je      @f
        cmp     dword [edx+4], -3
        jne     s_error
@@:
;        mov     eax, [edx+12]
;        test    eax, eax
;        jz      .fail
;        cmp     dword [eax], 4
;        mov     dword [eax], 4
;        jb      .fail
;        stdcall net_socket_num_to_addr, ecx
;        test    eax, eax
;        jz      .fail
;        ; todo: check that eax is really TCP socket
;        mov     ecx, [eax + TCP_SOCKET.last_ack_number]
;        cmp     dword [edx+4], -2
;        jz      @f
;        mov     ecx, [eax + TCP_SOCKET.state]
@@:
        mov     eax, [edx+8]
        test    eax, eax
        jz      @f
        mov     [eax], ecx
@@:
        mov     dword [esp+32], 0
        ret



;-----------------------------------------------------------------
;
; SOCKET_set_options
;
;  IN:  ecx = socket number
;       edx = pointer to the options:
;               dd      level, optname, optlen, optval
;  OUT: -1 on error
;
;-----------------------------------------------------------------
align 4
SOCKET_set_opt:

        DEBUGF  1,"SOCKET_set_opt\n"

        call    SOCKET_num_to_ptr
        jz      s_error

        cmp     dword [edx], SOL_SOCKET
        jne     s_error

        cmp     dword [edx+4], SO_BINDTODEVICE
        je      .bind

        cmp     dword [edx+4], SO_BLOCK
        je      .block

        jmp     s_error

  .bind:
        cmp     dword [edx+8], 0
        je      .unbind

        movzx   edx, byte [edx + 9]
        cmp     edx, MAX_NET_DEVICES
        ja      s_error

        mov     edx, [NET_DRV_LIST + 4*edx]
        test    edx, edx
        jz      s_error
        mov     [eax + SOCKET.device], edx

        DEBUGF  1,"SOCKET_set_opt: Bound socket %x to device %x\n",eax, edx

        mov     dword [esp+32], 0       ; success!
        ret

  .unbind:
        mov     [eax + SOCKET.device], 0

        mov     dword [esp+32], 0       ; success!
        ret

  .block:
        cmp     dword [edx+8], 0
        je      .unblock

        or      [eax + SOCKET.options], SO_BLOCK

        mov     dword [esp+32], 0       ; success!
        ret

  .unblock:
        and     [eax + SOCKET.options], not SO_BLOCK

        mov     dword [esp+32], 0       ; success!
        ret





;-----------------------------------------------------------------
;
; SOCKET_debug
;
;  Copies socket variables to application buffer
;
;  IN:  ecx = socket number
;       edx = pointer to buffer
;
;  OUT: -1 on error
;-----------------------------------------------------------------
align 4
SOCKET_debug:

;        DEBUGF  1,"SOCKET_debug\n"

        mov     edi, edx

        test    ecx, ecx
        jz      .returnall

        call    SOCKET_num_to_ptr
        jz      s_error

        mov     esi, eax
        mov     ecx, SOCKETBUFFSIZE/4
        rep     movsd

        mov     dword [esp+32], 0
        ret

  .returnall:
        mov     ebx, net_sockets
  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        test    ebx, ebx
        jz      .done
        mov     eax, [ebx + SOCKET.Number]
        stosd
        jmp     .next_socket
  .done:
        xor     eax, eax
        stosd

        mov     dword [esp+32], 0
        ret


;-----------------------------------------------------------------
;
; SOCKET_find_port
;
; Fills in the local port number for TCP and UDP sockets
; This procedure always works because the number of sockets is
; limited to a smaller number then the number of possible ports
;
;  IN:  eax = socket pointer
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_find_port:

        DEBUGF  1,"SOCKET_find_port\n"

        push    ebx esi ecx

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        je      .udp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

        pop     ecx esi ebx
        ret

  .udp:
        mov     bx, [last_UDP_port]
        call    .findit
        mov     [last_UDP_port], bx

        pop     ecx esi ebx
        ret

  .tcp:
        mov     bx, [last_TCP_port]
        call    .findit
        mov     [last_TCP_port], bx

        pop     ecx esi ebx
        ret


  .restart:
        mov     bx, MIN_EPHEMERAL_PORT_N
  .findit:
        cmp     bx, MAX_EPHEMERAL_PORT_N
        je      .restart

        add     bh, 1
        adc     bl, 0

        call    SOCKET_check_port
        jz      .findit
        ret



;-----------------------------------------------------------------
;
; SOCKET_check_port (to be used with AF_INET only!)
;
; Checks if a local port number is unused
; If the proposed port number is unused, it is filled in in the socket structure
;
;  IN:  eax = socket ptr (to find out if its a TCP/UDP socket)
;        bx = proposed socket number (network byte order)
;
;  OUT:  ZF = set on error
;
;-----------------------------------------------------------------
align 4
SOCKET_check_port:

        DEBUGF  1,"SOCKET_check_port: "

        mov     ecx, [eax + SOCKET.Protocol]
        mov     edx, [eax + IP_SOCKET.LocalIP]
        mov     esi, net_sockets

  .next_socket:
        mov     esi, [esi + SOCKET.NextPtr]
        or      esi, esi
        jz      .port_ok

        cmp     [esi + SOCKET.Protocol], ecx
        jne     .next_socket

        cmp     [esi + IP_SOCKET.LocalIP], edx
        jne     .next_socket

        cmp     [esi + UDP_SOCKET.LocalPort], bx
        jne     .next_socket

        DEBUGF  1,"local port %x already in use\n", bx  ; FIXME: find a way to print big endian values with debugf
        ret

  .port_ok:
        DEBUGF  1,"local port %x is free\n", bx         ; FIXME: find a way to print big endian values with debugf
        mov     [eax + UDP_SOCKET.LocalPort], bx
        or      bx, bx                                  ; clear the zero-flag
        ret



;-----------------------------------------------------------------
;
; SOCKET_input
;
; Updates a (stateless) socket with received data
;
; Note: the mutex should already be set !
;
;  IN:  eax = socket ptr
;       ecx = data size
;       esi = ptr to data
;       [esp] = ptr to buf
;       [esp + 4] = buf size
;
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_input:

        DEBUGF  1,"SOCKET_input: socket=%x, data=%x size=%u\n", eax, esi, ecx

        mov     [esp+4], ecx
        push    esi
        mov     esi, esp

        add_to_queue (eax + SOCKET_QUEUE_LOCATION), SOCKET_QUEUE_SIZE, sizeof.socket_queue_entry, SOCKET_input.full

        DEBUGF  1,"SOCKET_input: queued packet successfully\n"
        add     esp, sizeof.socket_queue_entry

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

        jmp     SOCKET_notify_owner

  .full:
        DEBUGF  2,"SOCKET_input: socket %x is full!\n", eax

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

        call    kernel_free
        add     esp, 8

        ret


;--------------------------
;
; eax = ptr to ring struct (just a buffer of the right size)
;
align 4
SOCKET_ring_create:

        push    esi
        mov     esi, eax

        push    edx
        stdcall create_ring_buffer, SOCKET_MAXDATA, PG_SW
        pop     edx

        DEBUGF  1,"SOCKET_ring_created: %x\n", eax
        mov     [esi + RING_BUFFER.start_ptr], eax
        mov     [esi + RING_BUFFER.write_ptr], eax
        mov     [esi + RING_BUFFER.read_ptr], eax
        mov     [esi + RING_BUFFER.size], 0
        add     eax, SOCKET_MAXDATA
        mov     [esi + RING_BUFFER.end_ptr], eax
        mov     eax, esi
        pop     esi

        ret

;-----------------------------------------------------------------
;
; SOCKET_ring_write
;
; Adds data to a stream socket, and updates write pointer and size
;
;  IN:  eax = ptr to ring struct
;       ecx = data size
;       esi = ptr to data
;
;  OUT: ecx = number of bytes stored
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_write:

        DEBUGF  1,"SOCKET_ring_write: ringbuff=%x ptr=%x size=%u\n", eax, esi, ecx

        add     [eax + RING_BUFFER.size], ecx
        jc      .way_too_large
        cmp     [eax + RING_BUFFER.size], SOCKET_MAXDATA
        ja      .too_large

  .copy:
        mov     edi, [eax + RING_BUFFER.write_ptr]
        DEBUGF  2,"SOCKET_ring_write: %u bytes from %x to %x\n", ecx, esi, edi

        push    ecx
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep     movsd
  .nd:
        pop     ecx

        cmp     edi, [eax + RING_BUFFER.end_ptr]
        jae     .wrap
        mov     [eax + RING_BUFFER.write_ptr], edi

        ret

  .wrap:
        sub     edi, SOCKET_MAXDATA
        mov     [eax + RING_BUFFER.write_ptr], edi

        ret

  .too_large:                                                           ; update size, we will fill buffer completely
        sub     [eax + RING_BUFFER.size], SOCKET_MAXDATA
        sub     ecx, [eax + RING_BUFFER.size]
        mov     [eax + RING_BUFFER.size], SOCKET_MAXDATA
        ja      .copy

  .full:
        DEBUGF  2,"SOCKET_ring_write: ring buffer is full!\n"
        xor     ecx, ecx
        ret

  .way_too_large:
        sub     [eax + RING_BUFFER.size], ecx
        mov     ecx, SOCKET_MAXDATA
        sub     ecx, [eax + RING_BUFFER.size]
        ja      .copy
        jmp     .full



;-----------------------------------------------------------------
;
; SOCKET_ring_read
;
;  IN:  eax = ring struct ptr
;       ecx = bytes to read
;       edx = offset
;       edi = ptr to buffer start
;
;  OUT: eax = unchanged
;       ecx = number of bytes read (0 on error)
;       edx = destroyed
;       esi = destroyed
;       edi = ptr to buffer end
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_read:

        DEBUGF  1,"SOCKET_ring_read: ringbuff=%x ptr=%x size=%u offset=%x\n", eax, edi, ecx, edx

        mov     esi, [eax + RING_BUFFER.read_ptr]
        add     esi, edx                                        ; esi = start_ptr + offset

        neg     edx
        add     edx, [eax + RING_BUFFER.size]                   ; edx = snd.size - offset
        jle     .no_data_at_all

        cmp     ecx, edx
        ja      .less_data

  .copy:
        DEBUGF  2,"SOCKET_ring_read: %u bytes from %x to %x\n", ecx, esi, edi
        push    ecx
        shr     ecx, 1
        jnc     .nb
        movsb
  .nb:
        shr     ecx, 1
        jnc     .nw
        movsw
  .nw:
        test    ecx, ecx
        jz      .nd
        rep     movsd
  .nd:
        pop     ecx
        ret

  .no_data_at_all:
        DEBUGF  1,"SOCKET_ring_read: no data at all!\n"
        xor     ecx, ecx
        ret

  .less_data:
        mov     ecx, edx
        jmp     .copy


;-----------------------------------------------------------------
;
; SOCKET_ring_free
;
; Free's some bytes from the ringbuffer
;
;  IN:  eax = ptr to ring struct
;       ecx = data size
;
;  OUT: ecx = number of bytes free-ed
;
;-----------------------------------------------------------------
align 4
SOCKET_ring_free:

        DEBUGF  1,"SOCKET_ring_free: %u bytes from ring %x\n", ecx, eax

        sub     [eax + RING_BUFFER.size], ecx
        jb      .error
        add     [eax + RING_BUFFER.read_ptr], ecx

        mov     edx, [eax + RING_BUFFER.end_ptr]
        cmp     [eax + RING_BUFFER.read_ptr], edx
        jb      @f
        sub     [eax + RING_BUFFER.read_ptr], SOCKET_MAXDATA
       @@:
        ret

  .error:       ; we could free all available bytes, but that would be stupid, i guess..
        DEBUGF  1,"SOCKET_ring_free: buffer=%x error!\n", eax
        add     [eax + RING_BUFFER.size], ecx
        xor     ecx, ecx
        ret


;-----------------------------------------------------------------
;
; SOCKET_notify_owner
;
; notify's the owner of a socket that something happened
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------
align 4
SOCKET_notify_owner:

        DEBUGF  1,"SOCKET_notify_owner: %x\n", eax

        call    SOCKET_check
        jz      .error

        push    eax ecx esi

; socket exists, now try to flag an event to the application

        mov     eax, [eax + SOCKET.PID]
        test    eax, eax
        jz      .error2
        mov     ecx, 1
        mov     esi, TASK_DATA + TASKDATA.pid

       .next_pid:
        cmp     [esi], eax
        je      .found_pid
        inc     ecx
        add     esi, 0x20
        cmp     ecx, [TASK_COUNT]
        jbe     .next_pid

; PID not found, TODO: close socket!

        jmp     .error2

       .found_pid:
        shl     ecx, 8
        or      [ecx + SLOT_BASE + APPDATA.event_mask], EVENT_NETWORK
        mov     [check_idle_semaphore], 200

        DEBUGF  1,"SOCKET_notify_owner: succes!\n"

  .error2:
        pop     esi ecx eax

  .error:

        ret


;--------------------------------------------------------------------
;
; SOCKET_alloc
;
; Allocate memory for socket data and put new socket into the list
; Newly created socket is initialized with calling PID and number and
; put into beginning of list (which is a fastest way).
;
; IN:  /
; OUT: eax = 0 on error, socket ptr otherwise
;      edi = socket number
;       ZF = cleared on error
;
;--------------------------------------------------------------------
align 4
SOCKET_alloc:

        push    ebx

        stdcall kernel_alloc, SOCKETBUFFSIZE
        DEBUGF  1, "SOCKET_alloc: ptr=%x\n", eax
        or      eax, eax
        jz      .exit

; zero-initialize allocated memory
        push    eax
        mov     edi, eax
        mov     ecx, SOCKETBUFFSIZE / 4
        xor     eax, eax
        rep     stosd
        pop     eax

; set send-and receive procedures to return -1
        mov     [eax + SOCKET.snd_proc], s_error
        mov     [eax + SOCKET.rcv_proc], s_error

; find first free socket number and use it
        mov     edi, [last_socket_num]
  .next_socket_number:
        inc     edi
        jz      .next_socket_number     ; avoid socket nr 0
        cmp     edi, -1
        je      .next_socket_number     ; avoid socket nr -1
        mov     ebx, net_sockets
  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        test    ebx, ebx
        jz      .last_socket

        cmp     [ebx + SOCKET.Number], edi
        jne     .next_socket
        jmp     .next_socket_number

  .last_socket:
        mov     [last_socket_num], edi
        mov     [eax + SOCKET.Number], edi
        DEBUGF  1, "SOCKET_alloc: number=%u\n", edi

; Fill in PID
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx + TASKDATA.pid]
        mov     [eax + SOCKET.PID], ebx

; init mutex
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_init
        popa

; add socket to the list by re-arranging some pointers
        mov     ebx, [net_sockets + SOCKET.NextPtr]

        mov     [eax + SOCKET.PrevPtr], net_sockets
        mov     [eax + SOCKET.NextPtr], ebx

        test    ebx, ebx
        jz      @f

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_lock
        popa

        mov     [ebx + SOCKET.PrevPtr], eax

        pusha
        lea     ecx, [ebx + SOCKET.mutex]
        call    mutex_unlock
        popa
       @@:

        mov     [net_sockets + SOCKET.NextPtr], eax
        or      eax, eax                ; used to clear zero flag
  .exit:
        pop     ebx

        ret


;----------------------------------------------------
;
; SOCKET_free
;
; Free socket data memory and remove socket from the list
;
; IN:  eax = socket ptr
; OUT: /
;
;----------------------------------------------------
align 4
SOCKET_free:

        DEBUGF  1, "SOCKET_free: %x\n", eax

        call    SOCKET_check
        jz      .error

        push    ebx

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        cmp     [eax + SOCKET.Domain], AF_INET4
        jnz     .no_tcp

        cmp     [eax + SOCKET.Protocol], IP_PROTO_TCP
        jnz     .no_tcp

        mov     ebx, eax
        stdcall kernel_free, [ebx + STREAM_SOCKET.rcv.start_ptr]
        stdcall kernel_free, [ebx + STREAM_SOCKET.snd.start_ptr]
        mov     eax, ebx
  .no_tcp:

        push    eax                             ; this will be passed to kernel_free
        mov     ebx, [eax + SOCKET.NextPtr]
        mov     eax, [eax + SOCKET.PrevPtr]

        DEBUGF  1, "SOCKET_free: linking socket %x to socket %x\n", eax, ebx

        test    eax, eax
        jz      @f
        mov     [eax + SOCKET.NextPtr], ebx
       @@:

        test    ebx, ebx
        jz      @f
        mov     [ebx + SOCKET.PrevPtr], eax
       @@:

        call    kernel_free
        pop     ebx

        DEBUGF  1, "SOCKET_free: success!\n"

  .error:
        ret

;------------------------------------
;
; SOCKET_fork
;
; Create a child socket
;
; IN:  socket nr in ebx
; OUT: child socket nr in eax
;
;-----------------------------------
align 4
SOCKET_fork:

        DEBUGF  1,"SOCKET_fork: %x\n", ebx

; Exit if backlog queue is full
        mov     eax, [ebx + SOCKET_QUEUE_LOCATION + queue.size]
        cmp     ax, [ebx + SOCKET.backlog]
        jae     .fail

; Allocate new socket
        push    ebx
        call    SOCKET_alloc
        pop     ebx
        jz      .fail

        push    eax
        mov     esi, esp
        add_to_queue (ebx + SOCKET_QUEUE_LOCATION), MAX_backlog, 4, .fail2
        pop     eax

; Copy structure from current socket to new
; We start at PID to preserve the socket num, and the 2 pointers at beginning of socket
        lea     esi, [ebx + SOCKET.PID]
        lea     edi, [eax + SOCKET.PID]
        mov     ecx, (SOCKET_QUEUE_LOCATION - SOCKET.PID + 3)/4
        rep     movsd

        and     [eax + SOCKET.options], not SO_ACCEPTCON

        ret

  .fail2:
        add     esp, 4+4+4
  .fail:
        DEBUGF  1,"SOCKET_fork: failed\n"
        xor     eax, eax
        ret


;---------------------------------------------------
;
; SOCKET_num_to_ptr
;
; Get socket structure address by its number
;
; IN:  ecx = socket number
; OUT: eax = 0 on error, socket ptr otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_num_to_ptr:

        DEBUGF  1,"SOCKET_num_to_ptr: num=%u ", ecx

        mov     eax, net_sockets

  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .error
        cmp     [eax + SOCKET.Number], ecx
        jne     .next_socket

        test    eax, eax

        DEBUGF  1,"ptr=%x\n", eax
        ret

  .error:
        DEBUGF  1,"not found\n", eax
        ret


;---------------------------------------------------
;
; SOCKET_ptr_to_num
;
; Get socket number by its address
;
; IN:  eax = socket ptr
; OUT: eax = 0 on error, socket num otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_ptr_to_num:

        DEBUGF  1,"SOCKET_ptr_to_num: ptr=%x ", eax

        call    SOCKET_check
        jz      .error

        mov     eax, [eax + SOCKET.Number]

        DEBUGF  1,"num=%u\n", eax
        ret

  .error:
        DEBUGF  1,"not found\n", eax
        ret


;---------------------------------------------------
;
; SOCKET_check
;
; checks if the given value is really a socket ptr
;
; IN:  eax = socket ptr
; OUT: eax = 0 on error, unchanged otherwise
;       ZF = set on error
;
;---------------------------------------------------
align 4
SOCKET_check:

        DEBUGF  1,"SOCKET_check: %x\n", eax

        push    ebx
        mov     ebx, net_sockets

  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
        or      ebx, ebx
        jz      .done
        cmp     ebx, eax
        jnz     .next_socket

  .done:
        mov     eax, ebx
        test    eax, eax
        pop     ebx

        ret



;---------------------------------------------------
;
; SOCKET_check_owner
;
; checks if the caller application owns the socket
;
; IN:  eax = socket ptr
; OUT:  ZF = true/false
;
;---------------------------------------------------
align 4
SOCKET_check_owner:

        DEBUGF  1,"SOCKET_check_owner: %x\n", eax

        push    ebx
        mov     ebx, [TASK_BASE]
        mov     ebx, [ecx + TASKDATA.pid]
        cmp     [eax + SOCKET.PID], ebx
        pop      ebx

        ret




;------------------------------------------------------
;
; SOCKET_process_end
;
; Kernel calls this function when a certain process ends
; This function will check if the process had any open sockets
; And update them accordingly
;
; IN:  edx = pid
; OUT: /
;
;------------------------------------------------------
align 4
SOCKET_process_end:

        DEBUGF  1,"SOCKET_process_end: %x\n", edx

        push    ebx
        mov     ebx, net_sockets

  .next_socket:
        mov     ebx, [ebx + SOCKET.NextPtr]
  .test_socket:
        test    ebx, ebx
        jz      .done

        cmp     [ebx + SOCKET.PID], edx
        jne     .next_socket

        DEBUGF  1,"SOCKET_process_end: killing socket %x\n", ebx

        mov     [ebx + SOCKET.PID], 0

        cmp     [ebx + SOCKET.Protocol], IP_PROTO_TCP
        je      .tcp

; The socket is stateless, just kill it right away!

        mov     eax, ebx
        mov     ebx, [ebx + SOCKET.NextPtr]
        call    SOCKET_free
        jmp     .test_socket

  .tcp:
        push    [ebx + SOCKET.NextPtr]
        mov     eax, ebx
        call    TCP_disconnect
        pop     ebx
        jmp     .test_socket

  .done:
        pop     ebx

        ret




;-----------------------------------------------------------------
;
; SOCKET_is_connecting
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_connecting:

        DEBUGF  1,"SOCKET_is_connecting: %x\n", eax

        and     [eax + SOCKET.options], not (SS_ISCONNECTED + SS_ISDISCONNECTING + SS_ISCONFIRMING)
        or      [eax + SOCKET.options], SS_ISCONNECTING

        jmp     SOCKET_notify_owner



;-----------------------------------------------------------------
;
; SOCKET_is_connected
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_connected:

        DEBUGF  1,"SOCKET_is_connected: %x\n", eax

        and     [eax + SOCKET.options], not (SS_ISCONNECTING + SS_ISDISCONNECTING + SS_ISCONFIRMING)
        or      [eax + SOCKET.options], SS_ISCONNECTED

        jmp     SOCKET_notify_owner




;-----------------------------------------------------------------
;
; SOCKET_is_disconnecting
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_disconnecting:

        DEBUGF  1,"SOCKET_is_disconnecting: %x\n", eax

        and     [eax + SOCKET.options], not (SS_ISCONNECTING)
        or      [eax + SOCKET.options], SS_ISDISCONNECTING + SS_CANTRCVMORE + SS_CANTSENDMORE

        jmp     SOCKET_notify_owner



;-----------------------------------------------------------------
;
; SOCKET_is_disconnected
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_is_disconnected:

        DEBUGF  1,"SOCKET_is_disconnected: %x\n", eax

        and     [eax + SOCKET.options], not (SS_ISCONNECTING + SS_ISCONNECTED + SS_ISDISCONNECTING)
        or      [eax + SOCKET.options], SS_CANTRCVMORE + SS_CANTSENDMORE

        jmp     SOCKET_notify_owner


;-----------------------------------------------------------------
;
; SOCKET_cant_recv_more
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_cant_recv_more:

        DEBUGF  1,"SOCKET_cant_recv_more: %x\n", eax

        or      [eax + SOCKET.options], SS_CANTRCVMORE

        ret



;-----------------------------------------------------------------
;
; SOCKET_cant_send_more
;
;  IN:  eax = socket ptr
;  OUT: /
;
;-----------------------------------------------------------------

align 4
SOCKET_cant_send_more:

        DEBUGF  1,"SOCKET_cant_send_more: %x\n", eax

        or      [eax + SOCKET.options], SS_CANTSENDMORE

        ret