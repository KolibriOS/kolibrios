;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2017. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6917 $

F_READ              = 0x0001  ; file opened for reading
F_WRITE             = 0x0002  ; file opened for writing

O_CLOEXEC           = 0x40000
PIPE_BUFFER_SIZE    = 4096


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

        push    edx
        push    ecx
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
        mov     [eax+FILED.mode], F_READ
        mov     [eax+FILED.file], ebp

        mov     [edx+FILED.magic], 'PIPE'
        mov     [edx+FILED.destroy], 0
        mov     [edx+FILED.mode], F_WRITE
        mov     [edx+FILED.file], ebp

        mov     eax, [eax+FILED.handle]
        mov     edx, [edx+FILED.handle]

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
        mov     eax, ebp
        pop     ebp
        mov     [esp+SYSCALL_STACK._eax], eax
        ret

purge .pipeflags
purge .filefd
purge .fdread
purge .fdwrite
purge .intpipe


; edx dst_buf
; esi read count
; ebp pipe

align 4
pipe_read:

        mov     edi, edx

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock
.again:
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

        lea     ecx, [ebp+PIPE.wlist]
        cmp     ecx, [ebp+PIPE.wlist.next]
        je      @F

        mov     ecx, [ecx+MUTEX_WAITER.task]
        mov     [ecx+TASKDATA.state], 0         ;activate writer task
@@:
        cmp     [ebp+PIPE.count], 0
        je      @F

        lea     eax, [ebp+PIPE.rlist]
        cmp     eax, [ebp+PIPE.rlist.next]
        je      @F

        mov     eax, [eax+MUTEX_WAITER.task]
        mov     [eax+TASKDATA.state], 0         ;activate reader task
@@:
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

.wait:
        pushfd
        cli

        sub     esp, sizeof.MUTEX_WAITER
        mov     ebx, [TASK_BASE]
        mov     [esp+MUTEX_WAITER.task], ebx
        lea     edx, [ebp+PIPE.rlist]

        list_add_tail esp, edx                  ;esp= new waiter, edx= list head

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        mov     [ebx+TASKDATA.state], 1
        call    change_task

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock

        list_del esp
        add     esp, sizeof.MUTEX_WAITER
        popfd
        jmp     .again

.eof:
        mov     [esp+SYSCALL_STACK._eax], eax
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock
        ret

; edx src_buf
; esi write count
; ebp pipe

align 4
pipe_write:

.written    equ esp

        push    0                               ;written
        mov     ebx, esi                        ;ebx = write count
        mov     esi, edx                        ;esi = src

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock
.again:
        xor     eax, eax
        cmp     eax, [ebp+PIPE.readers]
        je      .epipe

        mov     ecx, 4096
        sub     ecx, [ebp+PIPE.count]
        jz      .wait                           ;wait if buffer full

.check_count:
        cmp     ecx, ebx
        jb      .write
        mov     ecx, ebx
.write:
        mov     edi, [ebp+PIPE.buffer]
        add     edi, [ebp+PIPE.write_end]
        add     [.written], ecx
        sub     ebx, ecx
        add     [ebp+PIPE.count], ecx

        cld
        rep movsb
        and     edi, 0xFFF
        mov     [ebp+PIPE.write_end], edi

        pushfd
        cli

        lea     eax, [ebp+PIPE.rlist]
        cmp     eax, [ebp+PIPE.rlist.next]
        je      @F

        mov     eax, [eax+MUTEX_WAITER.task]
        mov     [eax+TASKDATA.state], 0         ;activate reader task
@@:
        cmp     [ebp+PIPE.count], 4096
        je      @F

        lea     ecx, [ebp+PIPE.wlist]
        cmp     ecx, [ebp+PIPE.wlist.next]
        je      @F

        mov     ecx, [eax+MUTEX_WAITER.task]
        mov     [ecx+TASKDATA.state], 0         ;activate writer task
@@:
        popfd

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        test    ebx, ebx
        jnz     .again

        pop     eax                             ; written
        mov     [esp+SYSCALL_STACK._eax], eax
        ret

.wait:
        pushfd
        cli

        sub     esp, sizeof.MUTEX_WAITER
        mov     ecx, [TASK_BASE]
        mov     [esp+MUTEX_WAITER.task], ecx
        lea     edx, [ebp+PIPE.wlist]

        list_add_tail esp, edx                  ;esp= new waiter, edx= list head

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        mov     [ecx+TASKDATA.state], 1
        call    change_task

        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_lock

        list_del esp
        add     esp, sizeof.MUTEX_WAITER
        popfd
        jmp     .again

.epipe:
        lea     ecx, [ebp+PIPE.pipe_lock]
        call    mutex_unlock

        add     esp, 4
        mov     [esp+SYSCALL_STACK._eax], -EPIPE
        ret

align 4
pipe_close:
        mov     [esp+SYSCALL_STACK._eax], -EBADF
        ret
