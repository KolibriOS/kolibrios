;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 2381 $

; Simple implementation of timers. All timers are organized in a double-linked
; list, and the OS loop after every timer tick processes the list.

; This structure describes a timer for the kernel.
struct  TIMER
        Next            dd      ?
        Prev            dd      ?
; These fields organize a double-linked list of all timers.
        TimerFunc       dd      ?
; Function to be called when the timer is activated.
        UserData        dd      ?
; The value that is passed as is to .TimerFunc.
        Time            dd      ?
; Time at which the timer should be activated.
        Interval        dd      ?
; Interval between activations of the timer, in 0.01s.
ends

iglobal
align 4
; The head of timer list.
timer_list:
        dd      timer_list
        dd      timer_list
endg
uglobal
; These two variables are used to synchronize access to the global list.
; Logically, they form an recursive mutex. Physically, the first variable holds
; the slot number of the current owner or 0, the second variable holds the
; recursion count.
; The mutex should be recursive to allow a timer function to add/delete other
; timers or itself.
timer_list_owner        dd      0
timer_list_numlocks     dd      0
; A timer function can delete any timer, including itself and the next timer in
; the chain. To handle such situation correctly, we keep the next timer in a
; global variable, so the removing operation can update it.
timer_next      dd      0
endg

; This internal function acquires the lock for the global list.
lock_timer_list:
        mov     edx, [CURRENT_TASK]
@@:
        xor     eax, eax
        lock cmpxchg [timer_list_owner], edx
        jz      @f
        cmp     eax, edx
        jz      @f
        call    change_task
        jmp     @b
@@:
        inc     [timer_list_numlocks]
        ret

; This internal function releases the lock for the global list.
unlock_timer_list:
        dec     [timer_list_numlocks]
        jnz     .nothing
        mov     [timer_list_owner], 0
.nothing:
        ret

; This function adds a timer.
; If deltaStart is nonzero, the timer is activated after deltaStart hundredths
; of seconds starting from the current time. If interval is nonzero, the timer
; is activated every deltaWork hundredths of seconds starting from the first
; activation. The activated timer calls timerFunc as stdcall function with one
; argument userData.
; Return value is NULL if something has failed or some value which is opaque
; for the caller. Later this value can be used for cancel_timer_hs.
proc timer_hs stdcall uses ebx, deltaStart:dword, interval:dword, \
        timerFunc:dword, userData:dword
; 1. Allocate memory for the TIMER structure.
; 1a. Call the allocator.
        push    sizeof.TIMER
        pop     eax
        call    malloc
; 1b. If allocation failed, return (go to 5) with eax = 0.
        test    eax, eax
        jz      .nothing
; 2. Setup the TIMER structure.
        xchg    ebx, eax
; 2a. Copy values from the arguments.
        mov     ecx, [interval]
        mov     [ebx+TIMER.Interval], ecx
        mov     ecx, [timerFunc]
        mov     [ebx+TIMER.TimerFunc], ecx
        mov     ecx, [userData]
        mov     [ebx+TIMER.UserData], ecx
; 2b. Get time of the next activation.
        mov     ecx, [deltaStart]
        test    ecx, ecx
        jnz     @f
        mov     ecx, [interval]
@@:
        add     ecx, [timer_ticks]
        mov     [ebx+TIMER.Time], ecx
; 3. Insert the TIMER structure to the global list.
; 3a. Acquire the lock.
        call    lock_timer_list
; 3b. Insert an item at ebx to the tail of the timer_list.
        mov     eax, timer_list
        mov     ecx, [eax+TIMER.Prev]
        mov     [ebx+TIMER.Next], eax
        mov     [ebx+TIMER.Prev], ecx
        mov     [eax+TIMER.Prev], ebx
        mov     [ecx+TIMER.Next], ebx
; 3c. Release the lock.
        call    unlock_timer_list
; 4. Return with eax = pointer to TIMER structure.
        xchg    ebx, eax
.nothing:
; 5. Returning.
        ret
endp

; This function removes a timer.
; The only argument is [esp+4] = the value which was returned from timer_hs.
cancel_timer_hs:
        push    ebx     ; save used register to be stdcall
; 1. Remove the TIMER structure from the global list.
; 1a. Acquire the lock.
        call    lock_timer_list
        mov     ebx, [esp+4+4]
; 1b. Delete an item at ebx from the double-linked list.
        mov     eax, [ebx+TIMER.Next]
        mov     ecx, [ebx+TIMER.Prev]
        mov     [eax+TIMER.Prev], ecx
        mov     [ecx+TIMER.Next], eax
; 1c. If we are removing the next timer in currently processing chain,
; the next timer for this timer becomes new next timer.
        cmp     ebx, [timer_next]
        jnz     @f
        mov     [timer_next], eax
@@:
; 1d. Release the lock.
        call    unlock_timer_list
; 2. Free the TIMER structure.
        xchg    eax, ebx
        call    free
; 3. Return.
        pop     ebx     ; restore used register to be stdcall
        ret     4       ; purge one dword argument to be stdcall

; This function is regularly called from osloop. It processes the global list
; and activates the corresponding timers.
check_timers:
; 1. Acquire the lock.
        call    lock_timer_list
; 2. Loop over all registered timers, checking time.
; 2a. Get the first item.
        mov     eax, [timer_list+TIMER.Next]
        mov     [timer_next], eax
.loop:
; 2b. Check for end of list.
        cmp     eax, timer_list
        jz      .done
; 2c. Get and store the next timer.
        mov     edx, [eax+TIMER.Next]
        mov     [timer_next], edx
; 2d. Check time for timer activation.
; We can't just compare [timer_ticks] and [TIMER.Time], since overflows are
; possible: if the current time is 0FFFFFFFFh ticks and timer should be
; activated in 3 ticks, the simple comparison will produce incorrect result.
; So we calculate the difference [timer_ticks] - [TIMER.Time]; if it is
; non-negative, the time is over; if it is negative, then either the time is
; not over or we have not processed this timer for 2^31 ticks, what is very
; unlikely.
        mov     edx, [timer_ticks]
        sub     edx, [eax+TIMER.Time]
        js      .next
; The timer should be activated now.
; 2e. Store the timer data in the stack. This is required since 2f can delete
; the timer, invalidating the content.
        push    [eax+TIMER.UserData]    ; parameter for TimerFunc
        push    [eax+TIMER.TimerFunc]   ; to be restored in 2g
; 2f. Calculate time of next activation or delete the timer if it is one-shot.
        mov     ecx, [eax+TIMER.Interval]
        add     [eax+TIMER.Time], ecx
        test    ecx, ecx
        jnz     .nodelete
        stdcall cancel_timer_hs, eax
.nodelete:
; 2g. Activate timer, using data from the stack.
        pop     eax
        call    eax
.next:
; 2h. Advance to the next timer and continue the loop.
        mov     eax, [timer_next]
        jmp     .loop
.done:
; 3. Release the lock.
        call    unlock_timer_list
; 4. Return.
        ret
