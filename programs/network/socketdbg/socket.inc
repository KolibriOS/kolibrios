struct  LHEAD
        next            dd ?   ;next object in list
        prev            dd ?   ;prev object in list
ends

struct  MUTEX
        lhead   LHEAD
        count   dd ?
ends


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