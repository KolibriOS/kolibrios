;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
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


; The Queues implemented by these macros form a ring-buffer.
; The data to these queue's always looks like this:
;
; At top, you have the queue struct, which has the size number of currently queued packets, read and write pointers.
; This struct is followed by a number of slots which you can read and write to using the macros.
; How these slots look like is up to you to chose, normally they should have at least a pointer to where the real data is.
; (you can see some examples below)


struct  queue

        size            dd ?    ; number of queued packets in this queue
        w_ptr           dd ?    ; current writing pointer in queue
        r_ptr           dd ?    ; current reading pointer

ends

; The following macros share these inputs:

; ptr           = pointer to where the queue data is located
; size          = number of slots/entrys in the queue
; entry_size    = size of one slot, in bytes
; failaddr      = the address where macro will jump to when there is no data in the queue

; additionally, add_to_queue requires you to set esi to the data which you want to queue
; get_from_queue on the other hand will return a pointer in esi, to the entry you're interested in
; PS: macros WILL destroy ecx and edi

macro add_to_queue ptr, size, entry_size, failaddr {

local .ok, .no_wrap

        spin_lock_irqsave

        cmp     [ptr + queue.size], size        ; Check if queue isn't full
        jb      .ok

        spin_unlock_irqrestore
        jmp     failaddr

  .ok:
        inc     [ptr + queue.size]              ; if not full, queue one more

        mov     edi, [ptr + queue.w_ptr]        ; Current write pointer (FIFO!)
        mov     ecx, entry_size/4               ; Write the queue entry
        rep movsd                               ;

        lea     ecx, [size*entry_size+ptr+sizeof.queue]
        cmp     edi, ecx                        ; entry size
        jb      .no_wrap

        sub     edi, size*entry_size
  .no_wrap:
        mov     [ptr + queue.w_ptr], edi

        spin_unlock_irqrestore

}



macro get_from_queue ptr, size, entry_size,  failaddr {

local .ok, .no_wrap

        spin_lock_irqsave

        cmp     [ptr + queue.size], 0           ; any packets queued?
        ja      .ok

        spin_unlock_irqrestore
        jmp     failaddr

  .ok:
        dec     [ptr + queue.size]              ; if so, dequeue one

        mov     esi, [ptr + queue.r_ptr]
        push    esi

        add     esi, entry_size

        lea     ecx, [size*entry_size+ptr+sizeof.queue]
        cmp     esi, ecx                        ; entry size
        jb      .no_wrap

        sub     esi, size*entry_size

  .no_wrap:
        mov     dword [ptr + queue.r_ptr], esi

        pop     esi

        spin_unlock_irqrestore

}

macro init_queue ptr {

        mov     [ptr + queue.size] , 0
        lea     edi, [ptr + sizeof.queue]
        mov     [ptr + queue.w_ptr], edi
        mov     [ptr + queue.r_ptr], edi

}
