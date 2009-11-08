;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  queue.inc                                                   ;;
;;                                                              ;;
;;    Written by hidnplayr@kolibrios.org                        ;;
;;                                                              ;;
;;          GNU GENERAL PUBLIC LICENSE                          ;;
;;             Version 2, June 1991                             ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

struct	queue
	.size		dd ?	; number of queued packets in thsi queue
	.w_ptr		dd ?	; current writing pointer in queue
	.r_ptr		dd ?	; current reading pointer
	.data:
ends

struct	eth_queue_entry
	.owner		dd ?
	.data_ptr	dd ?
	.data_size	dd ?
	.size:
ends

struct	tcp_in_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.offset 	dd ?
	.size:
ends

struct	tcp_out_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.ttl		dd ?
	.retries	dd ?
	.owner		dd ?
	.sendproc	dd ?
	.seq_num	dd ?
	.size:
ends

struct	socket_queue_entry
	.data_ptr	dd ?
	.data_size	dd ?
	.offset 	dd ?
	.size:
ends

macro add_to_queue ptr, size, entry_size, failaddr {

	cmp	[ptr + queue.size], size  ; Check if queue isnt full
	jge	failaddr

	inc	[ptr + queue.size]

	mov	edi, [ptr + queue.w_ptr]  ; Current write pointer (FIFO!)
	mov	ecx, entry_size/4		; Write the queue entry
	rep	movsd				;

	lea	ecx, [size*entry_size+ptr+queue.data]
	cmp	edi, ecx     ; entry size
	jl	.no_wrap

	sub	edi, size*entry_size

  .no_wrap:
	mov	[ptr + queue.w_ptr], edi

}



macro get_from_queue ptr, size, entry_size,  failaddr {

	cmp	[ptr + queue.size], 0		  ; any packets queued?
	je	failaddr

	dec	[ptr + queue.size]

	mov	esi, [ptr + queue.r_ptr]
	push	esi

	add	esi, entry_size

	lea	ecx, [size*entry_size+ptr+queue.data]
	cmp	esi, ecx	  ; entry size
	jl	.no_wrap

	sub	esi, size*entry_size

  .no_wrap:
	mov	dword [ptr + queue.r_ptr], esi

	pop	esi

}

macro init_queue queue_ptr {

	mov	[queue_ptr + queue.size] , 0
	lea	esi, [queue_ptr + queue.data]
	mov	[queue_ptr + queue.w_ptr], esi
	mov	[queue_ptr + queue.r_ptr], esi
}