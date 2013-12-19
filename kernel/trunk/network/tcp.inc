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

$Revision: 3406 $

; Socket states
TCPS_CLOSED             = 0
TCPS_LISTEN             = 1
TCPS_SYN_SENT           = 2
TCPS_SYN_RECEIVED       = 3
TCPS_ESTABLISHED        = 4
TCPS_CLOSE_WAIT         = 5
TCPS_FIN_WAIT_1         = 6
TCPS_CLOSING            = 7
TCPS_LAST_ACK           = 8
TCPS_FIN_WAIT_2         = 9
TCPS_TIMED_WAIT         = 10

; Socket Flags
TF_ACKNOW               = 1 shl 0     ; ack peer immediately
TF_DELACK               = 1 shl 1     ; ack, but try to delay it
TF_NODELAY              = 1 shl 2     ; don't delay packets to coalesce
TF_NOOPT                = 1 shl 3     ; don't use tcp options
TF_SENTFIN              = 1 shl 4     ; have sent FIN
TF_REQ_SCALE            = 1 shl 5     ; have/will request window scaling
TF_RCVD_SCALE           = 1 shl 6     ; other side has requested scaling
TF_REQ_TSTMP            = 1 shl 7     ; have/will request timestamps
TF_RCVD_TSTMP           = 1 shl 8     ; a timestamp was received in SYN
TF_SACK_PERMIT          = 1 shl 9     ; other side said I could SACK

; Segment flags
TH_FIN                  = 1 shl 0
TH_SYN                  = 1 shl 1
TH_RST                  = 1 shl 2
TH_PUSH                 = 1 shl 3
TH_ACK                  = 1 shl 4
TH_URG                  = 1 shl 5

; Segment header options
TCP_OPT_EOL             = 0           ; End of option list.
TCP_OPT_NOP             = 1           ; No-Operation.
TCP_OPT_MAXSEG          = 2           ; Maximum Segment Size.
TCP_OPT_WINDOW          = 3           ; window scale
TCP_OPT_SACK_PERMIT     = 4           ; Selective Acknowledgement
TCP_OPT_SACK            = 5
TCP_OPT_TIMESTAMP       = 8

; Fundamental timer values
TCP_time_MSL            = 47          ; max segment lifetime (30s)
TCP_time_re_min         = 2           ; min retransmission (1,28s)
TCP_time_re_max         = 100         ; max retransmission (64s)
TCP_time_pers_min       = 8           ; min persist (5,12s)
TCP_time_pers_max       = 94          ; max persist (60,16s)
TCP_time_keep_init      = 118         ; connection establishment (75,52s)
TCP_time_keep_idle      = 4608        ; idle time before 1st probe (2h)
TCP_time_keep_interval  = 118         ; between probes when no response (75,52s)
TCP_time_rtt_default    = 5           ; default Round Trip Time (3,2s)
TCP_time_srtt_default   = 0           ;
TCP_time_max_idle       = 8*TCP_time_keep_interval      ; FIXME

TCP_time_connect        = 300         ; in 1/100s (default=3s)

; timer constants
TCP_max_rxtshift        = 12          ; max retransmissions waiting for ACK
TCP_max_keepcnt         = 8           ; max keepalive probes

;
TCP_max_winshift        = 14
TCP_max_win             = 65535

TCP_re_xmit_thresh      = 3

TCP_mss_default         = 1480        ; default max segment size

; smoothed round trip time and estimated variance are stored as fixed point numbers,
; shifted by the value below.
; With these scales, srtt has 3 bits to the right of the binary point, and thus an "alpha"
; of .875. rttvar has 2 bits to the right and thus "alpha" of 0.75
TCP_RTT_SHIFT           = 3
TCP_RTTVAR_SHIFT        = 2

; bits used by tcp_input and tcp_output
TCP_BIT_NEEDOUTPUT      = 1 shl 0
TCP_BIT_TIMESTAMP       = 1 shl 1
TCP_BIT_DROPSOCKET      = 1 shl 2

TCP_BIT_SENDALOT        = 1 shl 0

TCP_PAWS_IDLE           = 24*24*60*60*100       ; 24 days, in 1/100 seconds

TCP_QUEUE_SIZE          = 50

struct  TCP_header

        SourcePort              dw ?
        DestinationPort         dw ?
        SequenceNumber          dd ?
        AckNumber               dd ?
        DataOffset              db ?    ; DataOffset[0-3 bits] and Reserved[4-7]
        Flags                   db ?    ; Reserved[0-1 bits]|URG|ACK|PSH|RST|SYN|FIN
        Window                  dw ?
        Checksum                dw ?
        UrgentPointer           dw ?

ends

struct  TCP_queue_entry

        ip_ptr          dd ?
        segment_ptr     dd ?
        segment_size    dd ?
        device_ptr      dd ?

        buffer_ptr      dd ?
        timestamp       dd ?

ends

uglobal
align 4

        TCP_segments_tx         rd NET_DEVICES_MAX
        TCP_segments_rx         rd NET_DEVICES_MAX
        TCP_segments_missed     rd NET_DEVICES_MAX
        TCP_segments_dumped     rd NET_DEVICES_MAX
;        TCP_bytes_rx            rq NET_DEVICES_MAX
;        TCP_bytes_tx            rq NET_DEVICES_MAX
        TCP_sequence_num        dd ?
        TCP_queue               rd (TCP_QUEUE_SIZE*sizeof.TCP_queue_entry + sizeof.queue)/4
        TCP_input_event         dd ?
endg

uglobal
align 4

        TCPS_accepts            dd ?    ; #SYNs received in LISTEN state
        TCPS_closed             dd ?    ; #connections closed (includes drops)
        TCPS_connattempt        dd ?    ; #connections initiated (calls to connect)
        TCPS_conndrops          dd ?    ; #embryonic connections dropped (before SYN received)
        TCPS_connects           dd ?    ; #connections established actively or passively
        TCPS_delack             dd ?    ; #delayed ACKs sent
        TCPS_drops              dd ?    ; #connections dropped (after SYN received)
        TCPS_keepdrops          dd ?    ; #connections dropped in keepalive (established or awaiting SYN)
        TCPS_keepprobe          dd ?    ; #keepalive probes sent
        TCPS_keeptimeo          dd ?    ; #times keepalive timer or connections-establishment timer expire
        TCPS_pawsdrop           dd ?    ; #segments dropped due to PAWS
        TCPS_pcbcachemiss       dd ?    ; #times PCB cache comparison fails
        TCPS_persisttimeo       dd ?    ; #times persist timer expires
        TCPS_predack            dd ?    ; #times header prediction correct for ACKs
        TCPS_preddat            dd ?    ; #times header prediction correct for data packets
        TCPS_rcvackbyte         dd ?    ; #bytes ACKed by received ACKs
        TCPS_rcvackpack         dd ?    ; #received ACK packets
        TCPS_rcvacktoomuch      dd ?    ; #received ACKs for unsent data
        TCPS_rcvafterclose      dd ?    ; #packets received after connection closed
        TCPS_rcvbadoff          dd ?    ; #packets received with invalid header length
        TCPS_rcvbadsum          dd ?    ; #packets received with checksum errors
        TCPS_rcvbyte            dd ?    ; #bytes received in sequence
        TCPS_rcvbyteafterwin    dd ?    ; #bytes received beyond advertised window
        TCPS_rcvdupack          dd ?    ; #duplicate ACKs received
        TCPS_rcvdupbyte         dd ?    ; #bytes receivedin completely duplicate packets
        TCPS_rcvduppack         dd ?    ; #packets received with completely duplicate bytes
        TCPS_rcvoobyte          dd ?    ; #out-of-order bytes received
        TCPS_rcvoopack          dd ?    ; #out-of-order packets received
        TCPS_rcvpack            dd ?    ; #packets received in sequence
        TCPS_rcvpackafterwin    dd ?    ; #packets with some data beyond advertised window
        TCPS_rcvpartdupbyte     dd ?    ; #duplicate bytes in part-duplicate packets
        TCPS_rcvpartduppack     dd ?    ; #packets with some duplicate data
        TCPS_rcvshort           dd ?    ; #packets received too short
        TCPS_rcvtotal           dd ?    ; #total packets received
        TCPS_rcvwinprobe        dd ?    ; #window probe packets received
        TCPS_rcvwinupd          dd ?    ; #received window update packets
        TCPS_rexmttimeo         dd ?    ; #retransmission timeouts
        TCPS_rttupdated         dd ?    ; #times RTT estimators updated
        TCPS_segstimed          dd ?    ; #segments for which TCP tried to measure RTT
        TCPS_sndacks            dd ?    ; #ACK-only packets sent (data length = 0)
        TCPS_sndbyte            dd ?    ; #data bytes sent
        TCPS_sndctrl            dd ?    ; #control (SYN, FIN, RST) packets sent (data length = 0)
        TCPS_sndpack            dd ?    ; #data packets sent (data length > 0)
        TCPS_sndprobe           dd ?    ; #window probes sent (1 byte of data forced by persist timer)
        TCPS_sndrexmitbyte      dd ?    ; #data bytes retransmitted
        TCPS_sndrexmitpack      dd ?    ; #data packets retransmitted
        TCPS_sndtotal           dd ?    ; total #packets sent
        TCPS_sndurg             dd ?    ; #packets sent with URG-only (data length=0)
        TCPS_sndwinup           dd ?    ; #window update-only packets sent (data length=0)
        TCPS_timeoutdrop        dd ?    ; #connections dropped in retransmission timeout

endg


;-----------------------------------------------------------------
;
; TCP_init
;
;  This function resets all TCP variables
;
;-----------------------------------------------------------------
macro   TCP_init {

        xor     eax, eax
        mov     edi, TCP_segments_tx
        mov     ecx, (6*NET_DEVICES_MAX)
        rep stosd

        pseudo_random   eax
        mov     [TCP_sequence_num], eax

        init_queue TCP_queue

        movi    ebx, 1
        mov     ecx, TCP_process_input
        call    new_sys_threads
        test    eax, eax
        jns     @f
        DEBUGF  DEBUG_NETWORK_ERROR,'K : cannot create kernel thread for TCP, error %d\n', eax
  @@:

}


include 'tcp_timer.inc'
include 'tcp_subr.inc'
include 'tcp_usreq.inc'
include 'tcp_input.inc'
include 'tcp_output.inc'


;---------------------------------------------------------------------------
;
; TCP_API
;
; This function is called by system function 76
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------
align 4
TCP_api:

        movzx   eax, bh
        shl     eax, 2

        test    bl, bl
        jz      .packets_tx     ; 0
        dec     bl
        jz      .packets_rx     ; 1
        dec     bl
        jz      .packets_missed ; 2
        dec     bl
        jz      .packets_dumped ; 3

  .error:
        mov     eax, -1
        ret

  .packets_tx:
        mov     eax, [TCP_segments_tx + eax]
        ret

  .packets_rx:
        mov     eax, [TCP_segments_rx + eax]
        ret

  .packets_missed:
        mov     eax, [TCP_segments_missed + eax]
        ret

  .packets_dumped:
        mov     eax, [TCP_segments_dumped + eax]
        ret
