;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;    Based on the code of 4.4BSD                                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Socket states
TCPS_CLOSED		 equ 0
TCPS_LISTEN		 equ 1
TCPS_SYN_SENT		 equ 2
TCPS_SYN_RECEIVED	 equ 3
TCPS_ESTABLISHED	 equ 4
TCPS_CLOSE_WAIT 	 equ 5
TCPS_FIN_WAIT_1 	 equ 6
TCPS_CLOSING		 equ 7
TCPS_LAST_ACK		 equ 8
TCPS_FIN_WAIT_2 	 equ 9
TCPS_TIMED_WAIT 	 equ 10

; Socket Flags
TF_ACKNOW		equ 1 shl 0	; ack peer immediately
TF_DELACK		equ 1 shl 1	; ack, but try to delay it
TF_NODELAY		equ 1 shl 2	; don't delay packets to coalesce
TF_NOOPT		equ 1 shl 3	; don't use tcp options
TF_SENTFIN		equ 1 shl 4	; have sent FIN
TF_REQ_SCALE		equ 1 shl 5	; have/will request window scaling
TF_RCVD_SCALE		equ 1 shl 6	; other side has requested scaling
TF_REQ_TSTMP		equ 1 shl 7	; have/will request timestamps
TF_RCVD_TSTMP		equ 1 shl 8	; a timestamp was received in SYN
TF_SACK_PERMIT		equ 1 shl 9	; other side said I could SACK

; Segment flags
TH_FIN			equ 1 shl 0
TH_SYN			equ 1 shl 1
TH_RST			equ 1 shl 2
TH_PUSH 		equ 1 shl 3
TH_ACK			equ 1 shl 4
TH_URG			equ 1 shl 5

; Segment header options
TCP_OPT_EOL		equ 0		; End of option list.
TCP_OPT_NOP		equ 1		; No-Operation.
TCP_OPT_MAXSEG		equ 2		; Maximum Segment Size.
TCP_OPT_WINDOW		equ 3		; window scale
TCP_OPT_TIMESTAMP	equ 8

; Fundamental timer values
TCP_time_MSL		equ 47		; max segment lifetime (30s)
TCP_time_re_min 	equ 2		; min retransmission (1,28s)
TCP_time_re_max 	equ 100 	; max retransmission (64s)
TCP_time_pers_min	equ 8		; min persist (5,12s)
TCP_time_pers_max	equ 94		; max persist (60,16s)
TCP_time_keep_init	equ 118 	; connectione stablishment (75,52s)
TCP_time_keep_idle	equ 4608	; idle time before 1st probe (2h)
TCP_time_keep_interval	equ 118 	; between probes when no response (75,52s)
TCP_time_rtt_default	equ 5		; default Round Trip Time (3,2s)

; timer constants
TCP_max_rxtshift	equ 12		; max retransmissions waiting for ACK
TCP_max_keepcnt 	equ 8		; max keepalive probes

;
TCP_max_winshift	equ 14
TCP_max_win		equ 65535

TCP_re_xmit_thresh	equ 3

struct	TCP_segment
	.SourcePort		dw ?
	.DestinationPort	dw ?
	.SequenceNumber 	dd ?
	.AckNumber		dd ?
	.DataOffset		db ?	; DataOffset[0-3 bits] and Reserved[4-7]
	.Flags			db ?	; Reserved[0-1 bits]|URG|ACK|PSH|RST|SYN|FIN
	.Window 		dw ?
	.Checksum		dw ?
	.UrgentPointer		dw ?
	.Data:				; ..or options
ends

align 4
uglobal
	TCP_segments_tx 	rd IP_MAX_INTERFACES
	TCP_segments_rx 	rd IP_MAX_INTERFACES
	TCP_bytes_rx		rq IP_MAX_INTERFACES
	TCP_bytes_tx		rq IP_MAX_INTERFACES
	TCP_sequence_num	dd ?
endg


;-----------------------------------------------------------------
;
; TCP_init
;
;  This function resets all TCP variables
;
;-----------------------------------------------------------------
macro	TCP_init {

	xor	eax, eax
	mov	edi, TCP_segments_tx
	mov	ecx, (6*IP_MAX_INTERFACES)
	rep	stosd

	pseudo_random	eax
	mov	[TCP_sequence_num], eax

}


include 'tcp_timer.inc'
include 'tcp_subr.inc'
include 'tcp_input.inc'
include 'tcp_output.inc'


;---------------------------------------------------------------------------
;
; TCP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------
align 4
TCP_API:

	movzx	eax, bh
	shl	eax, 2

	test	bl, bl
	jz	.packets_tx	; 0
	dec	bl
	jz	.packets_rx	; 1

.error:
	mov	eax, -1
	ret

.packets_tx:
	add	eax, TCP_segments_tx
	mov	eax, [eax]
	ret

.packets_rx:
	add	eax, TCP_segments_rx
	mov	eax, [eax]
	ret
