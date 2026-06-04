struct  LHEAD
        next            dd ?   ;next object in list
        prev            dd ?   ;prev object in list
ends

struct  MUTEX
        lhead   LHEAD
        count   dd ?
ends


struct  SOCKET

        NextPtr                 dd ?    ; pointer to next socket in list
        PrevPtr                 dd ?    ; pointer to previous socket in list
        Number                  dd ?    ; socket number

        mutex                   MUTEX

        PID                     dd ?    ; process ID
        TID                     dd ?    ; thread ID
        Domain                  dd ?    ; INET4/INET6/LOCAL/..
        Type                    dd ?    ; RAW/STREAM/DGRAM
        Protocol                dd ?    ; UDP/TCP/ARP/ICMP
        errorcode               dd ?
        device                  dd ?    ; device pointer, paired socket pointer if it's a local socket

        options                 dd ?
        state                   dd ?
        backlog                 dw ?    ; number of incoming connections that can be queued

        snd_proc                dd ?
        rcv_proc                dd ?
        connect_proc            dd ?
        ;sndto_proc              dd ?
        ;rcvfrom_proc            dd ?

ends

struct  IP_SOCKET               SOCKET

        LocalIP                 rd 4    ; network byte order
        RemoteIP                rd 4    ; network byte order
        ttl                     db ?
                                rb 3    ; align

ends

struct  TCP_SOCKET              IP_SOCKET

        LocalPort               dw ?    ; network byte order
        RemotePort              dw ?    ; network byte order

        t_state                 dd ?    ; TCB state
        t_rxtshift              db ?
                                rb 3    ; align
        t_rxtcur                dd ?
        t_dupacks               dd ?
        t_maxseg                dd ?
        t_flags                 dd ?

;---------------
; RFC783 page 21

; send sequence
        SND_UNA                 dd ?    ; sequence number of unack'ed sent Packets
        SND_NXT                 dd ?    ; next send sequence number to use
        SND_UP                  dd ?    ; urgent pointer
        SND_WL1                 dd ?    ; the sequence number of the last segment used to update the send window
        SND_WL2                 dd ?    ; the acknowledgment number of the last segment used to update the send window
        ISS                     dd ?    ; initial send sequence number
        SND_WND                 dd ?    ; send window

; receive sequence
        RCV_WND                 dd ?    ; receive window
        RCV_NXT                 dd ?    ; next receive sequence number to use
        RCV_UP                  dd ?    ; urgent pointer
        IRS                     dd ?    ; initial receive sequence number

;---------------------
; Additional variables

; receive variables
        RCV_ADV                 dd ?

; retransmit variables
        SND_MAX                 dd ?

; congestion control
        SND_CWND                dd ?    ; congestion window
        SND_SSTHRESH            dd ?    ; slow start threshold

;----------------------
; Transmit timing stuff
        t_idle                  dd ?
        t_rtt                   dd ?    ; round trip time
        t_rtseq                 dd ?
        t_srtt                  dd ?    ; smoothed round trip time
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
        timer_flags             dd ?
        timer_retransmission    dd ?    ; rexmt
        timer_persist           dd ?
        timer_keepalive         dd ?    ; keepalive/syn timeout
        timer_timed_wait        dd ?    ; also used as 2msl timer
        timer_connect           dd ?

; extra

        ts_ecr                  dd ?    ; timestamp echo reply
        ts_val                  dd ?

        seg_next                dd ?    ; re-assembly queue

ends

struct  UDP_SOCKET              IP_SOCKET

        LocalPort               dw ?    ; in network byte order
        RemotePort              dw ?    ; in network byte order

ends

struct  RING_BUFFER

        mutex                   MUTEX
        start_ptr               dd ?    ; Pointer to start of buffer
        end_ptr                 dd ?    ; pointer to end of buffer
        read_ptr                dd ?    ; Read pointer
        write_ptr               dd ?    ; Write pointer
        size                    dd ?    ; Number of bytes buffered

ends

struct  STREAM_SOCKET           TCP_SOCKET

        rcv                     RING_BUFFER
        snd                     RING_BUFFER

ends
