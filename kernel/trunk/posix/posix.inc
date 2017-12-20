;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2017. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6917 $

ENOENT  =      2
EBADF   =      9
EFAULT  =     14
;EINVAL  =     22      11   defined in stack.inc
ENFILE  =     23
EMFILE  =     24
EPIPE   =     32

FILEOP_CLOSE    =     0
FILEOP_READ     =     1
FILEOP_WRITE    =     2


include "futex.inc"

iglobal
align 4
sys_futex_call:
        dd sys_futex.destroy        ;1
        dd sys_futex.wait           ;2
        dd sys_futex.wake           ;3
        dd sys_futex.requeue        ;4
        dd sys_futex.cmp_requeue    ;5
        dd sys_futex.wait_bitset    ;6
        dd sys_futex.wake_bitset    ;7
endg

include "pipe.inc"

iglobal
align 4
sys_posix_call:
        dd sys_futex.init   ;0     futex_init
        dd sys_futex        ;1     futex_destroy
        dd sys_futex        ;2     futex_wait
        dd sys_futex        ;3     futex_wake
        dd sys_futex        ;4     reserved
        dd sys_futex        ;5     reserved
        dd sys_futex        ;6     reserved
        dd sys_futex        ;7     reserved
        dd sys_posix.fail   ;8     sys_open
        dd sys_posix.fail   ;9     sys_close
        dd sys_read         ;10    read()
        dd sys_write        ;11    write()
        dd sys_posix.fail   ;12    sys_dup3
        dd sys_pipe2        ;13
.end:
endg

align 4
sys_posix:
        cmp     ebx, (sys_posix_call.end-sys_posix_call)/4
        jae     .fail

        jmp     dword [sys_posix_call+ebx*4]

.fail:
        mov     [esp+SYSCALL_STACK._eax], -EBADF
        ret


;ssize_t read(int fd, void *buf, size_t count);
; ecx fd
; edx buf
; esi count

align 4
sys_read:
        cmp     ecx, STDERR_FILENO
        jbe     .fail
        cmp     ecx, (PROC.pdt_0 - PROC.htab)/4
        jae     .fail

        mov     edi, [current_process]
        mov     ebp, [edi+PROC.htab+ecx*4]

        cmp     [ebp+FILED.magic], 'PIPE'
        jne     .fail
        cmp     [ebp+FILED.handle], ecx
        jne     .fail

        test    [ebp+FILED.mode], F_READ
        jz      .fail

        mov     ebp, [ebp+FILED.file]
        mov     eax, [ebp]
        jmp     dword [eax+FILEOP_READ*4]
.fail:
        mov     [esp+SYSCALL_STACK._eax], -EBADF
        ret

;ssize_t write(int fd, const void *buf, size_t count);
; ecx fd
; edx buf
; esi count

align 4
sys_write:
        cmp     ecx, STDERR_FILENO
        jbe     .fail
        cmp     ecx, (PROC.pdt_0 - PROC.htab)/4
        jae     .fail

        mov     edi, [current_process]
        mov     ebp, [edi+PROC.htab+ecx*4]

        cmp     [ebp+FILED.magic], 'PIPE'
        jne     .fail
        cmp     [ebp+FILED.handle], ecx
        jne     .fail
        test    [ebp+FILED.mode], F_WRITE
        jz      .fail

        mov     ebp, [ebp+FILED.file]
        mov     eax, [ebp]
        jmp     dword [eax+FILEOP_WRITE*4]
.fail:
        mov     [esp+SYSCALL_STACK._eax], -EBADF
        ret

