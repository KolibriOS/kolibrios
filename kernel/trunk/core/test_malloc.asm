;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2009-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Tests of malloc()/free() from the kernel heap.
; This file is not included in the kernel, it is just test application.
use32
        db      'MENUET01'
        dd      1, start, i_end, mem, mem, 0, 0

start:
; Zero-initialize uglobals (as in kernel at boot)
        mov     ecx, (zeroend - zerostart + 3) / 4
        xor     eax, eax
        mov     edi, zerostart
        rep stosd
; Initialize small heap (as in kernel at boot)
        call    init_malloc
; Run tests
        call    run_test1
        call    run_test2
        call    run_test3
; All is OK, return
        or      eax, -1
        int     0x40

run_test1:
; basic test
        mov     eax, 1
        call    malloc_with_test
        mov     byte [eax], 0xDD
        mov     esi, eax
        mov     eax, 1
        call    malloc_with_test
        cmp     byte [esi], 0xDD
        jnz     memory_destroyed
        mov     byte [eax], 0xEE
        xchg    eax, esi
        call    free
        cmp     byte [esi], 0xEE
        jnz     memory_destroyed
        xchg    eax, esi
        call    free
        ret

run_test2:
        ret

run_test3:
; 1024 times run random operation.
; Randomly select malloc(random size from 1 to 1023)
; or free(random of previously allocated areas)
        mov     edi, 0x12345678
        xor     esi, esi        ; 0 areas allocated
        mov     ebx, 1024
.loop:
        imul    edi, 1103515245
        add     edi, 12345
        mov     eax, edi
        shr     eax, 16
        test    ebx, 64
        jz      .prefer_free
.prefer_malloc:
        test    eax, 3
        jz      .free
        jmp     @f
.prefer_free:
        test    eax, 3
        jnz     .free
@@:
        shr     eax, 2
        and     eax, 1023
        jz      .loop
        push    ebx
        push    eax
;       mov     ecx, [saved_state_num]
;       mov     [saved_state+ecx*8], eax
        push    edi
        call    malloc_with_test
        pop     ecx
        cmp     ecx, edi
        jnz     edi_destroyed
;       mov     ecx, [saved_state_num]
;       mov     [saved_state+ecx*8+4], eax
;       inc     [saved_state_num]
        pop     ecx
        pop     ebx
        inc     esi
        push    ecx eax
        push    edi
        mov     edi, eax
        mov     eax, esi
        rep stosb
        pop     edi
        jmp     .common
.free:
        test    esi, esi
        jz      .loop
        xor     edx, edx
        div     esi
        sub     edx, esi
        neg     edx
        dec     edx
        mov     eax, [esp+edx*8]
;       mov     ecx, [saved_state_num]
;       mov     [saved_state+ecx*8], -1
;       mov     [saved_state+ecx*8+4], eax
;       inc     [saved_state_num]
        mov     ecx, [esp+edx*8+4]
        push    edi eax
        mov     edi, eax
        mov     al, [edi]
        repz scasb
        jnz     memory_destroyed
        pop     eax edi
        push    ebx edx
        push    edi
        call    free
        pop     ecx
        cmp     ecx, edi
        jnz     edi_destroyed
        pop     edx ebx
        dec     esi
        pop     eax ecx
        push    edi
        lea     edi, [esp+4]
@@:
        dec     edx
        js      @f
        xchg    eax, [edi]
        xchg    ecx, [edi+4]
        add     edi, 8
        jmp     @b
@@:
        pop     edi
.common:
        dec     ebx
        jnz     .loop
@@:
        dec     esi
        js      @f
        pop     eax ecx
        call    free
        jmp     @b
@@:
        ret

malloc_with_test:
; calls malloc() and checks returned value
        call    malloc
        test    eax, eax
        jz      generic_malloc_fail
        call    check_mutex
        call    check_range
        ret

; Stubs for kernel procedures used by heap code
mutex_init:
        and     dword [ecx], 0
        ret
mutex_lock:
        inc     dword [ecx]
        ret
mutex_unlock:
        dec     dword [ecx]
        ret

kernel_alloc:
        cmp     dword [esp+4], bufsize
        jnz     error1
        mov     eax, buffer
        ret     4

macro $Revision [args]
{
}

; Error handlers
error1:
        mov     eax, 1
        jmp     error_with_code

generic_malloc_fail:
        mov     eax, 2
        jmp     error_with_code

check_mutex:
        cmp     dword [mst.mutex], 0
        jnz     @f
        ret
@@:
        mov     eax, 3
        jmp     error_with_code

check_range:
        cmp     eax, buffer
        jb      @f
        cmp     eax, buffer+bufsize
        jae     @f
        ret
@@:
        mov     eax, 4
        jmp     error_with_code

memory_destroyed:
        mov     eax, 5
        jmp     error_with_code

edi_destroyed:
        mov     eax, 6
        jmp     error_with_code

error_with_code:
        mov     edx, saved_state_num
; eax = error code
; 1 signals error in testing code (wrong bufsize)
; 2 = malloc() returned NULL
; 3 = mutex not released
; 4 = weird returned value from malloc()
; 5 = memory destroyed by malloc() or free()
        int3    ; simplest way to report error
        jmp     $-1     ; just in case

; Include main heap code
include '../macros.inc'
include '../proc32.inc'
include '../struct.inc'
include '../const.inc'
include 'malloc.inc'

i_end:

align 4
zerostart:
mst     MEM_STATE

align 16
bufsize = 0x40000       ; change if malloc.inc changes
buffer  rb      bufsize
zeroend:

saved_state_num dd      ?
saved_state     rd      0x10000

align 4
        rb      0x10000 ; for stack
mem:
