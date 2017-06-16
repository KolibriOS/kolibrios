;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2017. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6917 $

O_CLOEXEC           equ 0x40000
PIPE_BUFFER_SIZE    equ 4096


iglobal
align 4
pipe_file_ops:
        dd pipe_close       ;0
        dd pipe_read        ;1
        dd pipe_write       ;2
endg

;int pipe2(int pipefd[2], int flags);
;ecx pipefd
;edx flags

align 4
sys_pipe2:
.pipeflags equ  esp+16
.pipefd    equ  esp+12
.fdread    equ  esp+8
.fdwrite   equ  esp+4
.intpipe   equ  esp

        push    ebp
        test    ecx, ecx
        mov     ebp, -EFAULT
        js      .fail

        test    edx, not O_CLOEXEC
        mov     ebp, -EINVAL
        jnz     .fail

        push    ecx
        push    edx
        sub     esp, (5-2)*4

        mov     ecx, sizeof.FILED
        call    create_object
        mov     [.fdread], eax
        test    eax, eax
        mov     ebp, -EMFILE
        jz      .err_0

        mov     ecx, sizeof.FILED
        call    create_object
        mov     [.fdwrite], eax
        test    eax, eax
        jz      .err_1

        mov     eax, sizeof.PIPE
        call    malloc
        test    eax, eax
        mov     ebp, -ENFILE
        jz      .err_2

        mov     ebp, eax

        stdcall create_ring_buffer, PIPE_BUFFER_SIZE, PG_SWR
        test    eax, eax
        jz      .err_3

        mov     [ebp+PIPE.pipe_ops], pipe_file_ops
        mov     [ebp+PIPE.buffer], eax

        xor     eax, eax
        mov     [ebp+PIPE.count], eax
        mov     [ebp+PIPE.read_end], eax
        mov     [ebp+PIPE.write_end], eax

        inc     eax
        mov     [ebp+PIPE.readers], eax
        mov     [ebp+PIPE.writers], eax

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_init

        lea     ecx, [ebp+PIPE.rlist]
        list_init ecx

        lea     ecx, [ebp+PIPE.wlist]
        list_init ecx

        mov     eax, [.fdread]
        mov     edx, [.fdwrite]
        mov     ecx, [.pipefd]

        mov     [eax+FILED.magic], 'PIPE'
        mov     [eax+FILED.destroy], 0
        mov     [eax+FILED.file], ebp

        mov     [edx+FILED.magic], 'PIPE'
        mov     [edx+FILED.destroy], 0
        mov     [edx+FILED.file], ebp

        mov     [ecx], eax
        mov     [ecx+4], edx
        add     esp, 5*4
        pop     ebp
        xor     eax, eax
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.err_3:
        mov     eax, ebp
        call    free
        mov     ebp, -ENFILE
.err_2:
        mov     ecx, [.fdwrite]
        call    destroy_object
.err_1:
        mov     ecx, [.fdread]
        call    destroy_object
.err_0:
        add     esp, 5*4
.fail:
        mov     [esp+SYSCALL_STACK._eax], ebp
        pop     ebp
        ret

purge .pipeflags
purge .filefd
purge .fdread
purge .fdwrite
purge .intpipe


; edx buf
; esi count
; ebp pipe

align 4
pipe_read:
        mov     edi, edx

.again:
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock

        xor     eax, eax
        cmp     eax, [ebp+PIPE.writers]
        je      .eof

        mov     ecx, [ebp+PIPE.count]
        test    ecx, ecx
        jz      .wait

.check_count:
        cmp     ecx, esi
        jb      .read
        mov     ecx, esi
.read:
        mov     esi, [ebp+PIPE.buffer]
        add     esi, [ebp+PIPE.read_end]
        mov     [esp+SYSCALL_STACK._eax], ecx
        sub     [ebp+PIPE.count], ecx
        cld
        rep movsb
        and     esi, 0xFFF
        mov     [ebp+PIPE.read_end], esi

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

.wait:
        sub     esp, sizeof.MUTEX_WAITER
        mov     ebx, [TASK_BASE]
        mov     [esp+MUTEX_WAITER.task], ebx
        lea     edx, [ebp+PIPE.rlist]

        list_add_tail esp, edx      ;esp= new waiter, edx= list head

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        mov     [ebx+TASKDATA.state], 1
        call    change_task

        list_del esp
        add     esp, sizeof.MUTEX_WAITER
        jmp     .again

.eof:
        mov     [esp+SYSCALL_STACK._eax], eax
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

; edx buf
; esi count
; ebp pipe

align 4
pipe_write:
        mov     edi, edx

.again:
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock

        xor     eax, eax
        cmp     eax, [ebp+PIPE.readers]
        je      .epipe

        mov     ecx, [ebp+PIPE.count]
        sub     ecx, 4096
        jz      .wait

.check_count:
        cmp     ecx, esi
        jb      .write
        mov     ecx, esi
.write:
        mov     esi, [ebp+PIPE.buffer]
        add     esi, [ebp+PIPE.write_end]
        mov     [esp+SYSCALL_STACK._eax], ecx
        add     [ebp+PIPE.count], ecx
        cld
        rep movsb
        and     esi, 0xFFF
        mov     [ebp+PIPE.write_end], esi

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

.wait:
        sub     esp, sizeof.MUTEX_WAITER
        mov     ebx, [TASK_BASE]
        mov     [esp+MUTEX_WAITER.task], ebx
        lea     edx, [ebp+PIPE.wlist]

        list_add_tail esp, edx      ;esp= new waiter, edx= list head

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        mov     [ebx+TASKDATA.state], 1
        call    change_task

        list_del esp
        add     esp, sizeof.MUTEX_WAITER
        jmp     .again

.epipe:
        mov     [esp+SYSCALL_STACK._eax], -EPIPE
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

align 4
pipe_close:
        mov     [esp+SYSCALL_STACK._eax], -EBADF
        ret
