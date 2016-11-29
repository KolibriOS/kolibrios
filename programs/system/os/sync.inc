; High-level synchronization primitives.

; Mutex: stands for MUTual EXclusion.
; Allows to enforce that only one thread executes some code at a time.
; mutex_lock acquires the given mutex, mutex_unlock releases it;
; if thread 1 holds the mutex and thread 2 calls mutex_lock,
; thread 2 is blocked until thread 1 calls mutex_unlock.
; Several threads can wait for the same mutex; when the owner
; releases the mutex, one of waiting threads grabs the released mutex,
; but it is unspecified which one.

; If there is no contention, i.e. no one calls mutex_lock
; while somebody is holding the mutex, then
; mutex_lock and mutex_unlock use just a few instructions.
; This is the fast path.
; Otherwise, mutex_lock and mutex_unlock require a syscall
; to enter waiting state and wake someone up correspondingly.

; Implementation. We use one dword for status and
; kernel handle for underlying futex to be able to sleep/wake.
; Bit 31, the highest bit of status dword,
; is set if someone holds the mutex and clear otherwise.
; Bits 0-30 form the number of threads waiting in mutex_lock.
; All modifications of status dword should be atomic.

struct MUTEX
status  dd      ?
handle  dd      ?
ends

; Initialization. Set status dword to zero and
; open the underlying futex.
; in: ecx -> MUTEX
proc mutex_init
        mov     [ecx+MUTEX.status], 0
        push    ebx
        mov     eax, 77
        xor     ebx, ebx
        call    FS_SYSCALL_PTR
        pop     ebx
        mov     [ecx+MUTEX.handle], eax
        ret
endp

; Finalization. Close the underlying futex.
; in: ecx = MUTEX handle
proc mutex_destroy
        push    ebx
        mov     eax, 77
        mov     ebx, 1
        call    FS_SYSCALL_PTR
        pop     ebx
        ret
endp

; Acquire the mutex.
macro mutex_lock mutex
{
local .done
; Atomically set the locked status bit and get the previous value.
        lock bts [mutex+MUTEX.status], 31
; Fast path: the mutex was not locked. If so, we are done.
        jnc     .done
if ~(mutex eq ecx)
        mov     ecx, mutex
end if
        call    mutex_lock_slow_path
.done:
}

; Acquire the mutex, slow path.
; Someone holds the mutex... or has held it a moment ago.
; in: ecx -> MUTEX
proc mutex_lock_slow_path
; Atomically increment number of waiters.
        lock inc [ecx+MUTEX.status]
; When the mutex owner will release the mutex and wake us up,
; another thread can sneak in and grab the mutex before us.
; So, the following actions are potentially repeated in a loop.
.wait_loop:
        mov     edx, [ecx+MUTEX.status]
; The owner could have unlocked the mutex in parallel with us.
; If so, don't sleep: nobody would wake us up.
        test    edx, edx
        jns     .skip_wait
; Pass the fetched value to the kernel along with futex handle.
; If the owner unlocks the mutex while we are here,
; the kernel will detect mismatch and exit without sleeping.
; Otherwise, the owner will wake us up explicitly.
        push    ebx ecx esi
        mov     eax, 77
        mov     ebx, 2
        mov     ecx, [ecx+MUTEX.handle]
        xor     esi, esi
        call    FS_SYSCALL_PTR
        pop     esi ecx ebx
.skip_wait:
; We have woken up.
; Or we didn't even sleep because status dword has been changed beneath us.
; Anyway, something may have changed, re-evaluate the situation.
; Atomically set the locked status bit and get the previous value.
        lock bts [ecx+MUTEX.status], 31
; If the mutex was locked, someone has grabbed the mutex before us.
; Repeat the loop.
        jc      .wait_loop
; The mutex was unlocked and we have just managed to lock it.
; Our status has changed from a waiter to the owner.
; Decrease number of waiters and exit.
        lock dec [ecx+MUTEX.status]
        ret
endp

; Release the mutex.
macro mutex_unlock mutex
{
local .done
; Atomically clear the locked status bit and check whether someone is waiting.
        lock and [mutex+MUTEX.status], 0x7FFFFFFF
; Fast path: nobody is waiting.
        jz      .done
        mov     ecx, [mutex+MUTEX.handle]
        call    mutex_unlock_slow_path
.done:
}

; Release the mutex, slow path.
; Someone is sleeping in the kernel, or preparing for the sleep.
; Wake one of waiters.
; in: ecx = MUTEX handle
proc mutex_unlock_slow_path
        push    ebx
        mov     eax, 77
        mov     ebx, 3
        mov     edx, 1
        call    FS_SYSCALL_PTR
        pop     ebx
        ret
endp
