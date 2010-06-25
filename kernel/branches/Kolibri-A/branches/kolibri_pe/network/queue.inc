;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  QUEUE.INC                                                   ;;
;;                                                              ;;
;;  Buffer queue management for Menuet OS TCP/IP Stack          ;;
;;                                                              ;;
;;  Version 0.3  29 August 2002                                 ;;
;;                                                              ;;
;;  Copyright 2002 Mike Hibbett, mikeh@oceanfree.net            ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;*******************************************************************
;   Interface
;
;       queueInit   Configures the queues to empty
;       dequeue     Removes a buffer pointer from a queue
;       queue       Inserts a buffer pointer into a queue
;       freeBuff    Adds the buffer pointer to the list of free buffers
;       queueSize   Returns the number of entries in a queue
;
;      The various defines for queue names can be found in stack.inc
;
;*******************************************************************


;***************************************************************************
;   Function
;      freeBuff
;
;   Description
;       Adds a buffer number to the beginning of the free list.
;       buffer number in eax  ( ms word zeroed )
;       all other registers preserved
;        This always works, so no error returned
;***************************************************************************
uglobal
  freeBuff_cnt dd ?
endg
freeBuff:
;        inc     [freeBuff_cnt]
;        DEBUGF  1, "K : freeBuff (%u)\n", [freeBuff_cnt]
    push    ebx
    push    ecx
    mov     ebx, queues + EMPTY_QUEUE * 2
    cli     ; Ensure that another process does not interfer
    mov     cx, [ebx]
    mov     [ebx], ax
    mov     [queueList + eax * 2], cx
    sti
    pop     ecx
    pop     ebx

    ret


;***************************************************************************
;   Function
;      queueSize
;
;   Description
;       Counts the number of entries in a queue
;       queue number in ebx ( ms word zeroed )
;       Queue size returned in eax
;    This always works, so no error returned
;***************************************************************************
queueSize:
    xor     eax, eax
    shl     ebx, 1
    add     ebx, queues
    movzx   ecx, word [ebx]
    cmp     cx, NO_BUFFER
    je      qs_exit

qs_001:
    inc     eax
    shl     ecx, 1
    add     ecx, queueList
    movzx   ecx, word [ecx]
    cmp     cx, NO_BUFFER
    je      qs_exit
    jmp     qs_001

qs_exit:
    ret


;***************************************************************************
;   Function
;      queue
;
;   Description
;       Adds a buffer number to the *end* of a queue
;       This is quite quick because these queues will be short
;       queue number in eax ( ms word zeroed )
;       buffer number in ebx  ( ms word zeroed )
;       all other registers preserved
;        This always works, so no error returned
;***************************************************************************
uglobal
  queue_cnt dd ?
endg
queue:
;        inc     [queue_cnt]
;        DEBUGF  1, "K : queue (%u)\n", [queue_cnt]
    push    ebx
    shl     ebx, 1
    add     ebx, queueList        ; eax now holds address of queue entry
    mov     [ebx], word NO_BUFFER    ; This buffer will be the last

    cli
    shl     eax, 1
    add     eax, queues            ; eax now holds address of queue
    movzx   ebx, word [eax]

    cmp     bx, NO_BUFFER
    jne     qu_001

    pop     ebx
    ; The list is empty, so add this to the head
    mov     [eax], bx
    jmp     qu_exit

qu_001:
    ; Find the last entry
    shl     ebx, 1
    add     ebx, queueList
    mov     eax, ebx
    movzx   ebx, word [ebx]
    cmp     bx, NO_BUFFER
    jne     qu_001

    mov     ebx, eax
    pop     eax
    mov     [ebx], ax

qu_exit:
    sti
    ret



;***************************************************************************
;   Function
;      dequeue
;
;   Description
;       removes a buffer number from the head of a queue
;       This is fast, as it unlinks the first entry in the list
;       queue number in eax ( ms word zeroed )
;       buffer number returned in eax ( ms word zeroed )
;       all other registers preserved
;
;***************************************************************************
uglobal
  dequeue_cnt dd ?
endg
dequeue:
    push    ebx
    shl     eax, 1
    add     eax, queues            ; eax now holds address of queue
    mov     ebx, eax
    cli
    movzx   eax, word [eax]
    cmp     ax, NO_BUFFER
    je      dq_exit
;        inc     [dequeue_cnt]
;        DEBUGF  1, "K : dequeue (%u)\n", [dequeue_cnt]
    push    eax
    shl     eax, 1
    add     eax, queueList        ; eax now holds address of queue entry
    mov     ax, [eax]
    mov     [ebx], ax
    pop     eax

dq_exit:
    sti
    pop     ebx
    ret


;***************************************************************************
;   Function
;      queueInit
;
;   Description
;       Initialises the queues to empty, and creates the free queue
;       list.
;
;***************************************************************************
queueInit:
    mov     esi, queues
    mov     ecx, NUMQUEUES
    mov     ax, NO_BUFFER

qi001:
    mov     [esi], ax
    inc     esi
    inc     esi
    loop    qi001

    mov     esi, queues + ( 2 * EMPTY_QUEUE )

    ; Initialise empty queue list

    xor     ax, ax
    mov     [esi], ax

    mov     ecx, NUMQUEUEENTRIES - 1
    mov     esi, queueList

qi002:
    inc     ax
    mov     [esi], ax
    inc     esi
    inc     esi
    loop    qi002

    mov     ax, NO_BUFFER
    mov     [esi], ax

    ret
