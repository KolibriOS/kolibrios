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

$Revision: 983 $

struct	queue
	.size		dd ?
	.w_ptr		dd ?
	.r_ptr		dd ?
	.data:
ends

struct	queue_entry
	.owner		dd ?
	.data_ptr	dd ?
	.data_size	dd ?
	.size:
ends


macro add_to_queue ptr, size, returnaddr {

	cmp	dword [ptr + queue.size], size	; Check if queue isnt full
	jge	.fail

	DEBUGF	1,"Queuing packet for device %x\n",ebx

	inc	dword [ptr + queue.size]

	mov	edi, dword [ptr + queue.w_ptr]	; Current write pointer (FIFO!)

	mov	eax, ebx
	stosd
	pop	eax
	stosd
	pop	eax
	stosd

	cmp	edi, size*queue_entry.size+ptr+queue.data     ; entry size
	jl	.no_wrap

	sub	edi, size*queue_entry.size

  .no_wrap:
	mov	dword [ptr + queue.w_ptr], edi
	jmp	returnaddr

  .fail:
	DEBUGF	1,"queuing failed\n"

	call	kernel_free
	add	esp, 4
	ret

}


macro get_from_queue ptr, size, returnaddr {

  .start_of_code:
	cmp	dword [ptr + queue.size], 0		; any packets queued?
	je	returnaddr

	DEBUGF	1,"Dequeuing packet"

	dec	dword [ptr + queue.size]

	push	dword .start_of_code			; return address for call's

	mov	esi, [ptr + queue.r_ptr]
	lodsd
	mov	ebx, eax
	lodsd
	mov	ecx, eax
	lodsd
	push	eax
	push	ecx
	xchg	eax, ecx

	DEBUGF	1," for device %x\n", ebx

	cmp	esi, size*queue_entry.size+ptr+queue.data	   ; entry size
	jl	.no_wrap

	sub	esi, size*queue_entry.size

  .no_wrap:
	mov	dword [ptr + queue.r_ptr], esi

}

