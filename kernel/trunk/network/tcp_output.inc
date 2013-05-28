;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
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

$Revision: 3289 $

;-----------------------------------------------------------------
;
; TCP_output
;
; IN:  eax = socket pointer
;
; OUT: /
;
;-----------------------------------------------------------------
align 4
TCP_output:

        DEBUGF  1,"TCP_output: socket=%x\n", eax

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        pop     eax

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
        mov     [eax + TCP_SOCKET.temp_bits], 0

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]         ; calculate offset (71)
        sub     ebx, [eax + TCP_SOCKET.SND_UNA]         ;

        mov     ecx, [eax + TCP_SOCKET.SND_WND]         ; determine window
        cmp     ecx, [eax + TCP_SOCKET.SND_CWND]        ;
        jb      @f                                      ;
        mov     ecx, [eax + TCP_SOCKET.SND_CWND]        ;
       @@:                                              ;

        call    TCP_outflags                            ; flags in dl

;------------------------
; data being forced out ?

; If in persist timeout with window of 0, send 1 byte.
; Otherwise, if window is small but nonzero, and timer expired,
; we will send what we can and go to transmit state

        cmp     [eax + TCP_SOCKET.t_force], 0
        je      .no_force

        DEBUGF 1,"TCP_output: forcing data out\n"

        test    ecx, ecx
        jnz     .no_zero_window

        cmp     ebx, [eax + STREAM_SOCKET.snd.size]
        jae     @f

        and     dl, not (TH_FIN)

       @@:
        inc     ecx
        jmp     .no_force

  .no_zero_window:
        mov     [eax + TCP_SOCKET.timer_persist], 0
        mov     [eax + TCP_SOCKET.t_rxtshift], 0

  .no_force:

;--------------------------------
; Calculate how much data to send (106)

        mov     esi, [eax + STREAM_SOCKET.snd.size]
        cmp     esi, ecx
        jb      @f
        mov     esi, ecx
       @@:
        sub     esi, ebx


;------------------------
; check for window shrink (107)

; If FIN has been set, but not ACKed, but we havent been called to retransmit, esi will be -1
; Otherwise, window shrank after we sent into it.

        jae     .not_persist

; enter persist state
        xor     esi, esi

; If window shrank to 0
        test    ecx, ecx
        jnz     @f

; cancel pending retransmit
        mov     [eax + TCP_SOCKET.timer_retransmission], 0

; pull SND_NXT back to (closed) window, We will enter persist state below.
        push    [eax + TCP_SOCKET.SND_UNA]
        pop     [eax + TCP_SOCKET.SND_NXT]
       @@:

; If window didn't close completely, just wait for an ACK

  .not_persist:

;---------------------------
; Send one segment at a time (124)

        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        jbe     @f

        mov     esi, [eax + TCP_SOCKET.t_maxseg]
        or      [eax + TCP_SOCKET.temp_bits], TCP_BIT_SENDALOT
       @@:

;--------------------------------------------
; Turn of FIN flag if send buffer not emptied (128)

        mov     edi, [eax + TCP_SOCKET.SND_NXT]
        add     edi, esi
        sub     edi, [eax + TCP_SOCKET.SND_UNA]
        cmp     edi, [eax + STREAM_SOCKET.snd.size]
        jae     @f
        and     dl, not (TH_FIN)

       @@:

;-------------------------------
; calculate window advertisement (130)

        mov     ecx, SOCKET_MAXDATA
        sub     ecx, [eax + STREAM_SOCKET.rcv.size]

;------------------------------
; Sender silly window avoidance (131)

        test    esi, esi
        jz      .len_zero

        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        je      TCP_send

        add     ebx, esi                                ; offset + length
        cmp     ebx, [eax + STREAM_SOCKET.snd.size]
        jb      @f

        test    [eax + TCP_SOCKET.t_flags], TF_NODELAY
        jnz     TCP_send

        mov     ebx, [eax + TCP_SOCKET.SND_MAX]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        je      TCP_send
       @@:

        test    [eax + TCP_SOCKET.t_force], -1  ;;;
        jnz     TCP_send

        mov     ebx, [eax + TCP_SOCKET.max_sndwnd]
        shr     ebx, 1
        cmp     esi, ebx
        jae     TCP_send

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_MAX]
        jb      TCP_send

  .len_zero:

;----------------------------------------
; Check if a window update should be sent (154)

        DEBUGF  1,"TCP_output: window=%d\n", ecx

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
        cmp     ebx, ecx
        jb      @f
        mov     ebx, ecx
       @@:
        sub     ebx, [eax + TCP_SOCKET.RCV_ADV]
        add     ebx, [eax + TCP_SOCKET.RCV_NXT]

        mov     edi, [eax + TCP_SOCKET.t_maxseg]
        shl     edi, 1

;        cmp     ebx, edi
;        jae     TCP_send

;        cmp     ebx, [eax + TCP_SOCKET.]    ;;; TODO: check with receive buffer high water mark
;        jae     TCP_send

  .no_window:

;--------------------------
; Should a segment be sent? (174)

        DEBUGF  1,"TCP_output: 174\n"

        test    [eax + TCP_SOCKET.t_flags], TF_ACKNOW   ; we need to ACK
        jnz     TCP_send

        test    dl, TH_SYN + TH_RST                     ; we need to send a SYN or RST
        jnz     TCP_send

        mov     ebx, [eax + TCP_SOCKET.SND_UP]          ; when urgent pointer is beyond start of send bufer
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        ja      TCP_send

        test    dl, TH_FIN
        jz      .enter_persist  ; no reason to send, enter persist state

; FIN was set, only send if not already sent, or on retransmit

        test    [eax + TCP_SOCKET.t_flags], TF_SENTFIN
        jz      TCP_send

        mov     ebx, [eax + TCP_SOCKET.SND_NXT]
        cmp     ebx, [eax + TCP_SOCKET.SND_UNA]
        je      TCP_send

;--------------------
; Enter persist state (191)

  .enter_persist:

        cmp     [eax + STREAM_SOCKET.snd.size], 0       ; Data ready to send?
        jne     @f
        cmp     [eax + TCP_SOCKET.timer_retransmission], 0
        jne     @f
        cmp     [eax + TCP_SOCKET.timer_persist], 0     ; Persist timer already expired?
        jne     @f

        DEBUGF  1,"TCP_output: Entering persist state\n"

        mov     [eax + TCP_SOCKET.t_rxtshift], 0
        call    TCP_set_persist
       @@:

;----------------------------
; No reason to send a segment (219)

        DEBUGF  1,"TCP_output: No reason to send a segment\n"

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        popa

; Fixme: returnvalue?

        ret









;-----------------------------------------------
;
; Send a segment (222)
;
; eax = socket pointer
; esi = data len
;  dl = flags
;
;-----------------------------------------------
align 4
TCP_send:

        DEBUGF  1,"TCP_send: socket=%x length=%u flags=%x\n", eax, esi, dl

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

        DEBUGF  1,"TCP_send: added maxseg option\n"

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

        DEBUGF  1,"TCP_send: added scale option\n"

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

        DEBUGF  1,"TCP_send: added timestamp\n"

  .no_timestamp:

        ; <Add additional options here>

  .options_done:

; eax = socket ptr
; edx = flags
; edi = header size
; esi = data len

;---------------------------------------------
; check if we dont exceed the max segment size (270)

        add     esi, edi                        ; total TCP segment size
        cmp     esi, [eax + TCP_SOCKET.t_maxseg]
        jbe     .no_overflow

        mov     esi, [eax + TCP_SOCKET.t_maxseg]
        or      [eax + TCP_SOCKET.temp_bits], TCP_BIT_SENDALOT
  .no_overflow:

;-----------------------------------------------------------------
; Start by pushing all TCP header values in reverse order on stack
; (essentially, creating the tcp header on the stack!)

        pushw   0       ;        .UrgentPointer          dw ?
        pushw   0       ;        .Checksum               dw ?
        pushw   0x00a0  ;        .Window                 dw ?    ;;;;;;; FIXME (370)
        shl     edi, 2  ;        .DataOffset             db ?  only 4 left-most bits
        shl     dx, 8
        or      dx, di  ;        .Flags                  db ?
        pushw   dx
        shr     edi, 2  ;        .DataOffset             db ?

        push    [eax + TCP_SOCKET.RCV_NXT]      ;        .AckNumber              dd ?
        ntohd   [esp]

        push    [eax + TCP_SOCKET.SND_NXT]      ;        .SequenceNumber         dd ?
        ntohd   [esp]

        push    [eax + TCP_SOCKET.RemotePort]   ;        .DestinationPort        dw ?
        push    [eax + TCP_SOCKET.LocalPort]    ;        .SourcePort             dw ?

        push    edi                     ; header size

;---------------------
; Create the IP packet

        mov     ecx, esi

        mov     ebx, [eax + SOCKET.device]
        mov     edx, [eax + IP_SOCKET.LocalIP]  ; source ip
        mov     eax, [eax + IP_SOCKET.RemoteIP] ; dest ip
        mov     di, IP_PROTO_TCP shl 8 + 128
        call    IPv4_output
        jz      .ip_error

;-----------------------------------------
; Move TCP header from stack to TCP packet

        push    ecx
        mov     ecx, [esp + 4]
        lea     esi, [esp + 8]
        shr     ecx, 2                  ; count is in bytes, we will work with dwords
        rep     movsd
        pop     ecx                     ; full TCP packet size

        pop     esi                     ; headersize
        add     esp, esi                ; remove it from stack

        push    edx                     ; packet size for send proc
        push    eax                     ; packet ptr for send proc

        mov     edx, edi                ; begin of data
        sub     edx, esi                ; begin of packet (edi = begin of data)
        push    ecx
        sub     ecx, esi                ; data size

;--------------
; Copy the data

; eax = ptr to ring struct
; ecx = buffer size
; edi = ptr to buffer

        mov     eax, [esp + 16]                 ; get socket ptr

        push    edx
        push    [eax + TCP_SOCKET.SND_NXT]      ; we'll need this for timing the transmission
        test    ecx, ecx
        jz      .nodata
        mov     edx, [eax + TCP_SOCKET.SND_NXT]
        add     [eax + TCP_SOCKET.SND_NXT], ecx ; update sequence number <<< CHECKME
        sub     edx, [eax + TCP_SOCKET.SND_UNA] ; offset
        add     eax, STREAM_SOCKET.snd
        call    SOCKET_ring_read
  .nodata:
        pop     edi
        pop     esi                             ; begin of data
        pop     ecx                             ; full packet size
        mov     eax, [esp + 12]                 ; socket ptr

;----------------------------------
; initialize retransmit timer (400)

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
;TODO: update stats
       @@:

; set retransmission timer if not already set, and not doing an ACK or keepalive probe

        cmp     [eax + TCP_SOCKET.timer_retransmission], 0 ;;;; FIXME
        ja      .retransmit_set

        cmp     edx, [eax + TCP_SOCKET.SND_UNA]                 ; edx is still [eax + TCP_SOCKET.SND_NXT]
        je      .retransmit_set

        mov     edx, [eax + TCP_SOCKET.t_rxtcur]
        mov     [eax + TCP_SOCKET.timer_retransmission], edx

        cmp     [eax + TCP_SOCKET.timer_persist], 0
        jne     .retransmit_set
        mov     [eax + TCP_SOCKET.timer_persist], 0
        mov     [eax + TCP_SOCKET.t_rxtshift], 0

  .retransmit_set:

;--------------------
; Create the checksum

        TCP_checksum (eax + IP_SOCKET.LocalIP), (eax + IP_SOCKET.RemoteIP)
        mov     [esi + TCP_header.Checksum], dx

;----------------
; Send the packet

        DEBUGF  1,"TCP_send: Sending with device %x\n", ebx
        call    [ebx + NET_DEVICE.transmit]
        jnz     .send_error

;---------------
; Ok, data sent!

        pop     ecx
        pop     eax

        inc     [TCP_segments_tx]       ; FIXME: correct interface?

; update advertised receive window
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

; and flags
        and     [eax + TCP_SOCKET.t_flags], not (TF_ACKNOW + TF_DELACK)

;--------------
; unlock socket

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

;-----------------------------
; Check if we need more output

        test    [eax + TCP_SOCKET.temp_bits], TCP_BIT_SENDALOT
        jnz     TCP_output.again

        DEBUGF 1,"TCP_send: success!\n"

        xor     eax, eax
        ret


  .ip_error:
        pop     ecx
        add     esp, ecx
        add     esp, 4
        pop     eax

        mov     [eax + TCP_SOCKET.timer_retransmission], TCP_time_re_min

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        DEBUGF 1,"TCP_send: IP error\n"

        or      eax, -1
        ret


  .send_error:
        add     esp, 8

        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock

        DEBUGF 1,"TCP_send: sending failed\n"

        or      eax, -2
        ret






