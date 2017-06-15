;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2017. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 6917 $

align 4
;struct futex*  __fastcall create_futex(int *ptr)
create_futex:
        push    ecx
        mov     ecx, sizeof.FUTEX
        call    create_object
        pop     ecx
        test    eax, eax
        jz      .fail

        mov     [eax+FUTEX.magic], 'FUTX'
        mov     [eax+FUTEX.destroy], 0
        mov     [eax+FUTEX.pointer], ecx
        lea     ecx, [eax+FUTEX.wait_list]
        list_init ecx
        mov     [eax+FUTEX.flags], 0
.fail:
        ret

align 4
;int __fastcall destroy_futex(struct futex *futex)
destroy_futex:
        push    esi
        mov     esi, [current_process]
        mov     edx, [ecx+FUTEX.handle]

        pushfd
        cli

        lea     eax, [ecx+FUTEX.wait_list]
        cmp     eax, [eax+LHEAD.next]
        jne     .fail

        mov     eax, [esi+PROC.ht_next]
        mov     [esi+PROC.htab+edx*4], eax
        mov     [esi+PROC.ht_next], edx
        inc     [esi+PROC.ht_free]

        popfd
        pop     esi

        mov     eax, ecx
        call    free
        xor     eax, eax
        ret

.fail:
        popfd
        pop     esi
        mov     eax, -1
        ret

align 4
sys_futex:
        cmp     ecx, STDERR_FILENO
        jbe     .fail
        cmp     ecx, (PROC.pdt_0 - PROC.htab)/4
        jae     .fail

        mov     edi, [current_process]
        mov     ebp, [edi+PROC.htab+ecx*4]

        cmp     [ebp+FUTEX.magic], 'FUTX'
        jne     .fail
        cmp     [ebp+FUTEX.handle], ecx
        jne     .fail

        jmp     dword [sys_futex_call+ebx*4-4]

.fail:
.requeue:
.cmp_requeue:
.wait_bitset:
.wake_bitset:
        mov     [esp+SYSCALL_STACK._eax], -1
        ret

align 4
.init:
        call    create_futex
        test    eax, eax
        jz      @F
        mov     eax, [eax+FUTEX.handle]
@@:
        mov     [esp+SYSCALL_STACK._eax], eax
        ret

align 4
;ecx futex handle
;edi current process
;ebp futex object
.destroy:
        mov     ecx, ebp
        call    destroy_futex
        mov     [esp+SYSCALL_STACK._eax], eax
        ret

align 4
;ecx futex handle
;edx control value
;esi timeout
;edi current process
;ebp futex object
.wait:
        test    esi, esi
        jnz     .wait_timeout
        mov     ecx, [ebp+FUTEX.pointer]
        mov     eax, edx
        lock cmpxchg [ecx], edx
        je      .wait_slow

        mov     [esp+SYSCALL_STACK._eax], -2
        ret

.wait_slow:
        pushfd
        cli

        sub     esp, sizeof.MUTEX_WAITER
        mov     ebx, [TASK_BASE]
        mov     [esp+MUTEX_WAITER.task], ebx
        lea     esi, [ebp+FUTEX.wait_list]

        list_add_tail esp, esi      ;esp= new waiter, esi= list head
        mov     eax, edx
.again:
        mov     [ebx+TASKDATA.state], 1
        call    change_task

        lock cmpxchg [ecx], edx
        je      .again

        list_del esp
        add     esp, sizeof.MUTEX_WAITER

        popfd
        mov     [esp+SYSCALL_STACK._eax], 0
        ret

align 4
;ecx futex handle
;edx control value
;esi timeout
;edi current process
;ebp futex object

.wait_timeout:
        mov     ecx, [ebp+FUTEX.pointer]
        mov     eax, edx
        lock cmpxchg [ecx], edx         ;wait until old_value == new_value
        je      .wait_slow_timeout

        mov     [esp+SYSCALL_STACK._eax], -2
        ret

align 4
.wait_test:
        xor     eax, eax
        ret

.wait_slow_timeout:
        pushfd
        cli

        sub     esp, sizeof.MUTEX_WAITER

        mov     ebx, [current_slot]
        mov     [ebx+APPDATA.wait_test], sys_futex.wait_test
        mov     [ebx+APPDATA.wait_timeout], esi
        mov     [ebx+APPDATA.wait_param], ebp
        mov     eax, [timer_ticks]
        mov     [ebx+APPDATA.wait_begin], eax
        mov     eax, [TASK_BASE]
        mov     [eax+TASKDATA.state], 5

        mov     [esp+MUTEX_WAITER.task], eax
        lea     esi, [ebp+FUTEX.wait_list]

        list_add_tail esp, esi      ;esp= new waiter, esi= list head

.again_timeout:
        call    change_task
        mov     eax, [ebx+APPDATA.wait_param]
        test    eax, eax
        jz      .timeout

        mov     eax, edx
        lock cmpxchg [ecx], edx
        jz      .again_timeout
@@:
        list_del esp
        add     esp, sizeof.MUTEX_WAITER

        popfd
        mov     [esp+SYSCALL_STACK._eax], 0
        ret

.timeout:
        list_del esp
        add     esp, sizeof.MUTEX_WAITER

        popfd
        mov     [esp+SYSCALL_STACK._eax], -1
        ret


align 4
;ecx futex handle
;edx number of threads
;edi current process
;ebp futex object
.wake:

        xor     ecx, ecx

        pushfd
        cli

        lea     ebx, [ebp+FUTEX.wait_list]
        mov     esi, [ebx+LHEAD.next]
.again_wake:
        cmp     esi, ebx
        je      .done

        mov     eax, [esi+MUTEX_WAITER.task]
        mov     [eax+TASKDATA.state], 0

        mov     esi, [esi+MUTEX_WAITER.list.next]
        inc     ecx
        cmp     ecx, edx
        jb      .again_wake
.done:
        popfd
        mov     [esp+SYSCALL_STACK._eax], ecx
        ret

