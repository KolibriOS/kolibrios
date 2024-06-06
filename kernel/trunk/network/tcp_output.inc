;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the TCP/IP network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;    Based on the code of 4.4BSD                                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


TCP_BIT_SENDALOT        = 1 shl 0

;-----------------------------------------------------------------;
;                                                                 ;
; tcp_output                                                      ;
;                                                                 ;
;  IN:  eax = socket pointer                                      ;
;                                                                 ;
; OUT:  eax = 0 on success/errorcode                              ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
proc tcp_output

locals
        temp_bits       db ?
        rcv_window      dd ?
endl

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: socket=%x state=%u\n", eax, [eax + TCP_SOCKET.t_state]

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        pop     eax

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: socket locked\n"

; We'll detect the length of the data to be transmitted, and flags to be used
; If there is some data, or any critical controls to send (SYN / RST), then transmit
; Otherwise, investigate further

        mov     ebx, [eax + TCP_SOCKET.SND_MAX]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        jbe     .not_idle

        mov     ebx, [eax + TCP_SOCKET.t_idle]
        cmp     ebx, [eax + TCP_SOCKET.t_rxtcur]
        jbe     .not_idle

; We have been idle for a while and no ACKS are expected to clock out any data we send..
; Slow start to get ack "clock" running again.

        mov     ebx, [eax + TCP_SOCKET.t_maxseg]
        mov     [eax + TCP_SOCKET.SND_CWND], ebx

  .not_idle:
  .again:
        mov     [temp_bits], 0

; Calculate offset

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        sub     ebx, [eax + TCP_SOCKET.SND_UNA]

; Determine window

        mov     ecx, [eax + TCP_SOCKET.SND_WND]
        cmp     ecx, [eax + TCP_SOCKET.SND_CWND]
        jb      @f
        mov     ecx, [eax + TCP_SOCKET.SND_CWND]
       @@:

; get flags in dl

        call    tcp_outflags

;------------------------
; data being forced out ?

; If in persist timeout with window of 0, send 1 byte.
; Otherwise, if window is small but nonzero, and timer expired,
; we will send what we can and go to transmit state

        test    [eax + TCP_SOCKET.t_flags], TF_FORCE
        jz      .no_force

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: forcing data out\n"

        test    ecx, ecx
        jnz     .no_zero_window

        cmp     ebx, [eax + STREAM_SOCKET.snd.size]
        jae     @f
        and     dl, not (TH_FIN)
       @@:

        inc     ecx
        jmp     .no_force

  .no_zero_window:
        and     [eax + TCP_SOCKET.timer_flags], not timer_flag_persist
        mov     [eax + TCP_SOCKET.t_rxtshift], 0

  .no_force:

;--------------------------------
; Calculate how much data to send

        mov     esi, [eax + STREAM_SOCKET.snd.size]
        cmp     esi, ecx
        jb      @f
        mov     esi, ecx
       @@:
        sub     esi, ebx

;------------------------
; check for window shrink

; If FIN has been sent, but not ACKed, but we havent been called to retransmit, esi will be -1
; Otherwise, window shrank after we sent into it.

        jge     .not_persist

; enter persist state

        xor     esi, esi

; If window shrank to 0

        test    ecx, ecx
        jnz     @f

; cancel pending retransmit

        and     [eax + TCP_SOCKET.timer_flags], not timer_flag_retransmission

; pull SND_NXT back to (closed) window, We will enter persist state below.

        push    [eax + TCP_SOCKET.SND_UNA]
        pop     [eax + TCP_SOCKET.SND_NXT]
       @@:

; If window didn't close completely, just wait for an ACK

  .not_persist:

;---------------------------
; Send one segment at a time

        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        jbe     @f
        mov     esi, [eax + TCP_SOCKET.t_maxseg]
        or      [temp_bits], TCP_BIT_SENDALOT
       @@:

;--------------------------------------------
; Turn of FIN flag if send buffer not emptied

        mov     edi, [eax + TCP_SOCKET.SND_NXT]
        add     edi, esi
        sub     edi, [eax + TCP_SOCKET.SND_UNA]
        cmp     edi, [eax + STREAM_SOCKET.snd.size]
        jae     @f
        and     dl, not (TH_FIN)
       @@:

;-------------------------------
; calculate window advertisement

        xor     ecx, ecx
        test    [eax + SOCKET.state], SS_CANTRCVMORE
        jnz     @f
        mov     ecx, SOCKET_BUFFER_SIZE
        sub     ecx, [eax + STREAM_SOCKET.rcv.size]
  @@:

;------------------------------
; Sender silly window avoidance

        test    esi, esi
        jz      .len_zero

        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        je      .send

        add     ebx, esi                                ; offset + length
        cmp     ebx, [eax + STREAM_SOCKET.snd.size]
        jb      @f

        test    [eax + TCP_SOCKET.t_flags], TF_NODELAY
        jnz     .send

        mov     ebx, [eax + TCP_SOCKET.SND_MAX]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        je      .send
       @@:

        test    [eax + TCP_SOCKET.t_flags], TF_FORCE
        jnz     .send

        mov     ebx, [eax + TCP_SOCKET.max_sndwnd]
        shr     ebx, 1
        cmp     esi, ebx
        jae     .send

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_MAX]
        jb      .send

  .len_zero:

;----------------------------------------
; Check if a window update should be sent

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: window=%d\n", ecx

; Compare available window to amount of window known to peer (as advertised window less next expected input)
; If the difference is at least two max size segments, or at least 50% of the maximum possible window,
; Then we want to send a window update to the peer.

        test    ecx, ecx
        jz      .no_window

        push    ecx
        mov     cl, [eax + TCP_SOCKET.RCV_SCALE]
        mov     ebx, TCP_max_win
        shl     ebx, cl
        pop     ecx
        sub     ebx, [eax + TCP_SOCKET.RCV_ADV]
        add     ebx, [eax + TCP_SOCKET.RCV_NXT]

        cmp     ebx, ecx
        jl      @f
        mov     ebx, ecx
       @@:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: we can increase window by %d bytes\n", ebx

        mov     edi, [eax + TCP_SOCKET.t_maxseg]
        shl     edi, 1
        cmp     ebx, edi
        jae     .send

        cmp     ebx, SOCKET_BUFFER_SIZE/2
        jae     .send

  .no_window:

;--------------------------
; Should a segment be sent?

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: Should a segment be sent?\n"

        test    [eax + TCP_SOCKET.t_flags], TF_ACKNOW   ; we need to ACK
        jnz     .send

        test    dl, TH_SYN + TH_RST                     ; we need to send a SYN or RST
        jnz     .send

        mov     ebx, [eax + TCP_SOCKET.SND_UP]          ; when urgent pointer is beyond start of send bufer
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        ja      .send

; Do we need to send a FIN according to our state?

        test    dl, TH_FIN
        jz      .enter_persist                          ; no reason to send, enter persist state

; Do so if we didnt do it already

        test    [eax + TCP_SOCKET.t_flags], TF_SENTFIN
        jz      .send

; Or when we need to retransmit the FIN

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        je      .send

;--------------------
; Enter persist state

  .enter_persist:

        cmp     [eax + STREAM_SOCKET.snd.size], 0                       ; Data ready to send?
        je      @f
        test    [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission or timer_flag_persist
        jnz     @f

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: Entering persist state\n"

        mov     [eax + TCP_SOCKET.t_rxtshift], 0
        call    tcp_set_persist
       @@:

;----------------------------
; No reason to send a segment

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_output: No reason to send a segment\n"

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

        xor     eax, eax
        ret


;-----------------------------------------------
;
; Send a segment
;
; eax = socket pointer
; esi = data len
;  dl = flags
;
;-----------------------------------------------
  .send:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: socket=%x length=%u flags=%x\n", eax, esi, dl

        push    eax                     ; save socket ptr
        push    esi                     ; and data length too
        mov     edi, sizeof.TCP_header  ; edi will contain headersize

;------------------------------------
; Send options with first SYN segment

        test    dl, TH_SYN
        jz      .options_done

        push    [eax + TCP_SOCKET.ISS]
        pop     [eax + TCP_SOCKET.SND_NXT]

        test    [eax + TCP_SOCKET.t_flags], TF_NOOPT
        jnz     .options_done

        mov     ecx, 1460                              ;;;; FIXME: use routing blablabla to determine MSS
        or      ecx, TCP_OPT_MAXSEG shl 24 + 4 shl 16
        bswap   ecx
        push    ecx
        add     di, 4

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: added maxseg option\n"

        test    [eax + TCP_SOCKET.t_flags], TF_REQ_SCALE
        jz      .no_scale

        test    dl, TH_ACK
        jz      .scale_opt

        test    [eax + TCP_SOCKET.t_flags], TF_RCVD_SCALE
        jz      .no_scale

  .scale_opt:
        mov     cl, [eax + TCP_SOCKET.request_r_scale]
        mov     ch, TCP_OPT_NOP
        pushw   cx
        pushw   TCP_OPT_WINDOW + 3 shl 8
        add     di, 4

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: added scale option\n"

  .no_scale:
  .no_syn:

;------------------------------------
; Make the timestamp option if needed

        test    [eax + TCP_SOCKET.t_flags], TF_REQ_TSTMP
        jz      .no_timestamp

        test    dl, TH_RST
        jnz     .no_timestamp

        test    dl, TH_ACK
        jz      .timestamp

        test    [eax + TCP_SOCKET.t_flags], TF_RCVD_TSTMP
        jz      .no_timestamp

  .timestamp:
        pushd   0
        pushd   [timer_ticks]
        pushd   TCP_OPT_NOP + TCP_OPT_NOP shl 8 + TCP_OPT_TIMESTAMP shl 16 + 10 shl 24
        add     di, 12

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: added timestamp\n"

  .no_timestamp:

        ; <Add additional options here>

  .options_done:

; eax = socket ptr
; edx = flags
; edi = header size
; esi = data len

;---------------------------------------------
; check if we don't exceed the max segment size

        add     esi, edi                        ; total TCP segment size
        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        jbe     .no_overflow

        mov     esi, [eax + TCP_SOCKET.t_maxseg]
        or      [temp_bits], TCP_BIT_SENDALOT
  .no_overflow:

; Update stats

        test    esi, esi
        jz      .zero_data

        test    [eax + TCP_SOCKET.t_flags], TF_FORCE
        jz      @f
        cmp     esi, 1
        jne     @f
        inc     [TCPS_sndprobe]
        jmp     .eos
  @@:

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_MAX]
        jae     @f
        inc     [TCPS_sndrexmitpack]
        add     [TCPS_sndrexmitbyte], esi
        jmp     .eos
  @@:
        inc     [TCPS_sndpack]
        add     [TCPS_sndbyte], esi
        jmp     .eos

  .zero_data:
        test    [eax + TCP_SOCKET.t_flags], TF_ACKNOW
        jz      @f
        inc     [TCPS_sndacks]
        jmp     .eos
  @@:
        test    dl, TH_SYN + TH_FIN + TH_RST
        jz      @f
        inc     [TCPS_sndctrl]
        jmp     .eos
  @@:
        mov     ebx, [eax + TCP_SOCKET.SND_UP]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        jb      @f
        inc     [TCPS_sndurg]
        jmp     .eos
  @@:
        inc     [TCPS_sndwinup]

  .eos:

;---------------------------------------------------
; Don't increase sequence number when resending a FIN

        test    dl, TH_FIN
        jz      .no_fin_retransmit

        test    [eax + TCP_SOCKET.t_flags], TF_SENTFIN
        jz      .no_fin_retransmit

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_MAX]
        jne     .no_fin_retransmit

        dec     [eax + TCP_SOCKET.SND_NXT]

  .no_fin_retransmit:

;----------------------------------------------------
; Calculate the receive window.
; Don't shrink window, but avoid silly window syndrome

        xor     ebx, ebx
        test    [eax + SOCKET.state], SS_CANTRCVMORE
        jnz     @f
        mov     ebx, SOCKET_BUFFER_SIZE
        sub     ebx, [eax + STREAM_SOCKET.rcv.size]

        cmp     ebx, SOCKET_BUFFER_SIZE/4
        jge     @f
        cmp     ebx, [eax + TCP_SOCKET.t_maxseg]
        jge     @f
        xor     ebx, ebx
  @@:


        mov     cl, [eax + TCP_SOCKET.RCV_SCALE]
        push    eax
        mov     eax, TCP_max_win
        shl     eax, cl
        cmp     ebx, eax
        jle     @f
        mov     ebx, eax
  @@:
        pop     eax


        mov     ecx, [eax + TCP_SOCKET.RCV_ADV]
        sub     ecx, [eax + TCP_SOCKET.RCV_NXT]
        cmp     ebx, ecx
        jg      @f
        mov     ebx, ecx
  @@:

;; TODO URGENT POINTER

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: window=%u\n", ebx
        mov     [rcv_window], ebx

        mov     cl, [eax + TCP_SOCKET.RCV_SCALE]
        shr     ebx, cl
        xchg    bl, bh

;-----------------------------------------------------------------
; Start by pushing all TCP header values in reverse order on stack
; (essentially, creating the tcp header on the stack!)

        pushw   0                               ; UrgentPointer
        pushw   0                               ; Checksum
        pushw   bx                              ; Window
        shl     edi, 2                          ; DataOffset
        shl     dx, 8
        or      dx, di                          ; Flags
        pushw   dx
        shr     edi, 2                          ; DataOffset

        push    [eax + TCP_SOCKET.RCV_NXT]      ; AckNumber
        ntohd   [esp]

        push    [eax + TCP_SOCKET.SND_NXT]      ; SequenceNumber
        ntohd   [esp]

        push    [eax + TCP_SOCKET.RemotePort]   ; DestinationPort
        push    [eax + TCP_SOCKET.LocalPort]    ; SourcePort

        push    edi                             ; header size

;---------------------
; Create the IP packet

        mov     ecx, esi
        mov     ebx, [eax + IP_SOCKET.device]
        mov     edx, [eax + IP_SOCKET.LocalIP]  ; source ip
        mov     edi, [eax + IP_SOCKET.RemoteIP] ; dest ip
        mov     al, [eax + IP_SOCKET.ttl]
        mov     ah, IP_PROTO_TCP
        call    ipv4_output
        jz      .ip_error

;------------------------------------------
; Move TCP header from stack to TCP segment

        push    ecx
        mov     ecx, [esp + 4]
        lea     esi, [esp + 8]
        shr     ecx, 2                          ; count is in bytes, we will work with dwords
        rep movsd
        pop     ecx                             ; full TCP packet size

        pop     esi                             ; headersize
        add     esp, esi                        ; remove it from stack

        push    eax                             ; packet ptr for send proc

        mov     edx, edi                        ; begin of data
        sub     edx, esi                        ; begin of packet (edi = begin of data)
        push    ecx
        sub     ecx, esi                        ; data size

;--------------
; Copy the data

; eax = ptr to ring struct
; ecx = buffer size
; edi = ptr to buffer

        mov     eax, [esp + 12]                 ; get socket ptr

        push    edx
        push    [eax + TCP_SOCKET.SND_NXT]      ; we'll need this for timing the transmission
        test    ecx, ecx
        jz      .nodata
        mov     edx, [eax + TCP_SOCKET.SND_NXT]
        add     [eax + TCP_SOCKET.SND_NXT], ecx ; update sequence number
        sub     edx, [eax + TCP_SOCKET.SND_UNA] ; offset
        add     eax, STREAM_SOCKET.snd
        call    socket_ring_read
  .nodata:
        pop     edi
        pop     esi                             ; begin of data
        pop     ecx                             ; full packet size
        mov     eax, [esp + 8]                  ; socket ptr

;----------------------------
; initialize retransmit timer

;TODO: check t_force and persist

        test    [esi + TCP_header.Flags], TH_SYN + TH_FIN       ; syn and fin take a sequence number
        jz      @f
        inc     [eax + TCP_SOCKET.SND_NXT]
        test    [esi + TCP_header.Flags], TH_FIN
        jz      @f
        or      [eax + TCP_SOCKET.t_flags], TF_SENTFIN          ; if we sent a fin, set the sentfin flag
       @@:

        mov     edx, [eax + TCP_SOCKET.SND_NXT]
        cmp     edx, [eax + TCP_SOCKET.SND_MAX]                 ; is this a retransmission?
        jbe     @f
        mov     [eax + TCP_SOCKET.SND_MAX], edx                 ; [eax + TCP_SOCKET.SND_NXT] from before we updated it

        cmp     [eax + TCP_SOCKET.t_rtt], 0                     ; are we currently timing anything?
        je      @f
        mov     [eax + TCP_SOCKET.t_rtt], 1                     ; nope, start transmission timer
        mov     [eax + TCP_SOCKET.t_rtseq], edi
        inc     [TCPS_segstimed]
       @@:

; set retransmission timer if not already set, and not doing an ACK or keepalive probe

        test    [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission
        jnz     .retransmit_set

        cmp     edx, [eax + TCP_SOCKET.SND_UNA]                 ; edx is still [eax + TCP_SOCKET.SND_NXT]
        je      .retransmit_set

        mov     edx, [eax + TCP_SOCKET.t_rxtcur]
        mov     [eax + TCP_SOCKET.timer_retransmission], edx
        or      [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission

        test    [eax + TCP_SOCKET.timer_flags], timer_flag_persist
        jz      .retransmit_set
        and     [eax + TCP_SOCKET.timer_flags], not timer_flag_persist
        mov     [eax + TCP_SOCKET.t_rxtshift], 0
  .retransmit_set:

;--------------------
; Create the checksum

        xor     dx, dx
        test    [ebx + NET_DEVICE.hwacc], NET_HWACC_TCP_IPv4_OUT
        jnz     .checksum_ok

        tcp_checksum (eax + IP_SOCKET.LocalIP), (eax + IP_SOCKET.RemoteIP)

  .checksum_ok:
        mov     [esi + TCP_header.Checksum], dx

;----------------
; Send the packet

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: Sending with device %x\n", ebx
        call    [ebx + NET_DEVICE.transmit]
        jnz     .send_error

;---------------
; Ok, data sent!

        pop     ecx
        pop     eax

        call    net_ptr_to_num4
        inc     [TCP_segments_tx + edi]
        inc     [TCPS_sndtotal]

; update advertised receive window

        mov     ecx, [rcv_window]
        test    ecx, ecx
        jz      @f
        add     ecx, [eax + TCP_SOCKET.RCV_NXT]
        cmp     ecx, [eax + TCP_SOCKET.RCV_ADV]
        jbe     @f
        mov     [eax + TCP_SOCKET.RCV_ADV], ecx
       @@:

; update last ack sent

        push    [eax + TCP_SOCKET.RCV_NXT]
        pop     [eax + TCP_SOCKET.last_ack_sent]

; clear the ACK flags

        and     [eax + TCP_SOCKET.t_flags], not (TF_ACKNOW + TF_DELACK)

;--------------
; unlock socket

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: unlocking socket 0x%x\n", eax

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

;-----------------------------
; Check if we need more output

        test    [temp_bits], TCP_BIT_SENDALOT
        jnz     tcp_output.again

        DEBUGF  DEBUG_NETWORK_VERBOSE, "TCP_send: success!\n"

        xor     eax, eax
        ret


  .ip_error:
        pop     ecx
        add     esp, ecx
        add     esp, 4
        pop     eax

        mov     [eax + TCP_SOCKET.timer_retransmission], TCP_time_re_min
        or      [eax + TCP_SOCKET.timer_flags], timer_flag_retransmission

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        DEBUGF  DEBUG_NETWORK_ERROR, "TCP_send: IP error\n"

        or      eax, -1
        ret


  .send_error:
        add     esp, 4
        pop     eax

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        DEBUGF  DEBUG_NETWORK_ERROR, "TCP_send: sending failed\n"

        or      eax, -2
        ret


endp
