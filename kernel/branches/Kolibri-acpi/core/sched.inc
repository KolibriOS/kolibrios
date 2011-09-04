;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


align 32
irq0:
        pushad
        Mov     ds, ax, app_data
        mov     es, ax
        inc     [timer_ticks]
        mov     eax, [timer_ticks]
        call    playNote       ; <<<--- Speaker driver
        sub     eax,[next_usage_update]
        cmp     eax,100
        jb      .nocounter
        add     [next_usage_update],100
        call    updatecputimes
  .nocounter:
	mov	al, 0	; send End Of Interrupt signal
	call	IRQ_EOI
        btr     dword[DONT_SWITCH], 0
        jc      .return
        call    find_next_task
        jz      .return  ; if there is only one running process
        call    do_change_task
  .return:
        popad
        iretd

align 4
change_task:
        pushfd
        cli
        pushad
if 0
; \begin{Mario79} ; <- must be refractoried, if used...
        cmp     [dma_task_switched], 1
        jne     .find_next_task
        mov     [dma_task_switched], 0
        mov     ebx, [dma_process]
        cmp     [CURRENT_TASK], ebx
        je      .return
        mov     edi, [dma_slot_ptr]
        mov     [CURRENT_TASK], ebx
        mov     [TASK_BASE], edi
        jmp     @f
.find_next_task:
; \end{Mario79}
end if
        call    find_next_task
        jz      .return  ; the same task -> skip switch
  @@:   mov     byte[DONT_SWITCH], 1
        call    do_change_task
  .return:
        popad
        popfd
        ret

uglobal
align 4
;  far_jump:
;   .offs dd ?
;   .sel  dw ?
   context_counter     dd 0 ;noname & halyavin
   next_usage_update   dd 0
   timer_ticks         dd 0
;  prev_slot           dd ?
;  event_sched         dd ?
endg

align 4
update_counters:
        mov     edi, [TASK_BASE]
        rdtsc
        sub     eax, [edi+TASKDATA.counter_add] ; time stamp counter add
        add     [edi+TASKDATA.counter_sum], eax ; counter sum
        ret
align 4
updatecputimes:
        xor     eax,eax
        xchg    eax,[idleuse]
        mov     [idleusesec],eax
        mov     ecx, [TASK_COUNT]
        mov     edi, TASK_DATA
  .newupdate:
        xor     eax,eax
        xchg    eax,[edi+TASKDATA.counter_sum]
        mov     [edi+TASKDATA.cpu_usage],eax
        add     edi,0x20
        loop    .newupdate
        ret

align 4
find_next_task:
;info:
;   Find next task to execute
;retval:
;   ebx = address of the APPDATA for the selected task (slot-base)
;   esi = previous slot-base ([current_slot] at the begin)
;   edi = address of the TASKDATA for the selected task
;   ZF  = 1  if the task is the same
;warning:
;   [CURRENT_TASK] = bh , [TASK_BASE] = edi -- as result
;   [current_slot] is not set to new value (ebx)!!!
;scratched: eax,ecx
        call    update_counters ; edi := [TASK_BASE]
        Mov     esi, ebx, [current_slot]
  .loop:
        cmp     bh,[TASK_COUNT]
        jb      @f
        xor     bh, bh
        mov     edi,CURRENT_TASK
  @@:   inc     bh       ; ebx += APPDATA.size
        add     edi,0x20 ; edi += TASKDATA.size
        mov     al, [edi+TASKDATA.state]
        test    al, al
        jz      .found   ; state == 0
        cmp     al, 5
        jne     .loop    ; state == 1,2,3,4,9
      ; state == 5
        pushad  ; more freedom for [APPDATA.wait_test]
        call    [ebx+APPDATA.wait_test]
        mov     [esp+28],eax
        popad
        or      eax,eax
        jnz     @f
      ; testing for timeout
        mov     ecx, [timer_ticks]
        sub     ecx, [ebx+APPDATA.wait_begin]
        cmp     ecx, [ebx+APPDATA.wait_timeout]
        jb      .loop
  @@:   mov     [ebx+APPDATA.wait_param], eax  ; retval for wait
        mov     [edi+TASKDATA.state], 0
  .found:
        mov     [CURRENT_TASK],bh
        mov     [TASK_BASE],edi
        rdtsc   ;call  _rdtsc
        mov     [edi+TASKDATA.counter_add],eax ; for next using update_counters
        cmp     ebx, esi ;esi - previous slot-base
        ret
;TODO: Надо бы убрать использование do_change_task из V86...
; и после этого перенести обработку TASKDATA.counter_add/sum в do_change_task

align 4
do_change_task:
;param:
;   ebx = address of the APPDATA for incoming task (new)
;warning:
;   [CURRENT_TASK] and [TASK_BASE] must be changed before (e.g. in find_next_task)
;   [current_slot] is the outcoming (old), and set here to a new value (ebx)
;scratched: eax,ecx,esi
        mov     esi,ebx
        xchg    esi,[current_slot]
; set new stack after saving old
        mov     [esi+APPDATA.saved_esp], esp
        mov     esp, [ebx+APPDATA.saved_esp]
; set new thread io-map
        Mov     dword [page_tabs+((tss._io_map_0 and -4096) shr 10)],eax,[ebx+APPDATA.io_map]
        Mov     dword [page_tabs+((tss._io_map_1 and -4096) shr 10)],eax,[ebx+APPDATA.io_map+4]
; set new thread memory-map
        mov     ecx, APPDATA.dir_table
        mov     eax, [ebx+ecx]      ;offset>0x7F
        cmp     eax, [esi+ecx]      ;offset>0x7F
        je      @f
        mov     cr3, eax
@@:
; set tss.esp0

        Mov     [tss._esp0],eax,[ebx+APPDATA.saved_esp0]

        mov edx, [ebx+APPDATA.tls_base]
        cmp edx, [esi+APPDATA.tls_base]
        je @f

        mov     [tls_data_l+2],dx
        shr     edx,16
        mov     [tls_data_l+4],dl
        mov     [tls_data_l+7],dh

        mov dx, app_tls
        mov fs, dx
@@:
; set gs selector unconditionally
        Mov     gs,ax,graph_data
      ; set CR0.TS
        cmp     bh, byte[fpu_owner] ;bh == incoming task (new)
        clts                        ;clear a task switch flag
        je      @f
        mov     eax, cr0            ;and set it again if the owner
        or      eax, CR0_TS         ;of a fpu has changed
        mov     cr0, eax
  @@: ; set context_counter (only for user pleasure ???)
        inc     [context_counter]   ;noname & halyavin
      ; set debug-registers, if it's necessary
        test    byte[ebx+APPDATA.dbg_state], 1
        jz      @f
        xor     eax, eax
        mov     dr6, eax
        lea     esi,[ebx+ecx+APPDATA.dbg_regs-APPDATA.dir_table] ;offset>0x7F
        cld
  macro lodsReg [reg] {
        lodsd
        mov     reg,eax
  }     lodsReg dr0, dr1, dr2, dr3, dr7
  purge lodsReg
  @@:   ret
;end.



struc MUTEX_WAITER
{
    .list   LHEAD
    .task   rd 1
    .sizeof:
};

virtual at 0
 MUTEX_WAITER MUTEX_WAITER
end virtual

;void  __fastcall mutex_init(struct mutex *lock)

align 4
mutex_init:
        mov [ecx+MUTEX.wait.next], ecx
        mov [ecx+MUTEX.wait.prev], ecx
        mov [ecx+MUTEX.count],1
        ret


;void  __fastcall mutex_lock(struct mutex *lock)

align 4
mutex_lock:

        dec [ecx+MUTEX.count]
        jns .done

        pushfd
        cli

        sub esp, MUTEX_WAITER.sizeof

        list_add_tail esp, ecx      ;esp= new waiter, ecx= list head

        mov edx, [TASK_BASE]
        mov [esp+MUTEX_WAITER.task], edx

.forever:

        mov eax, -1
        xchg eax, [ecx+MUTEX.count]
        dec eax
        jz @F

        mov [edx+TASKDATA.state], 1
        call change_task
        jmp .forever
@@:
        mov edx, [esp+MUTEX_WAITER.list.next]
        mov eax, [esp+MUTEX_WAITER.list.prev]

        mov [eax+MUTEX_WAITER.list.next], edx
        mov [edx+MUTEX_WAITER.list.prev], eax
        cmp [ecx+MUTEX.wait.next], ecx
        jne @F

        mov [ecx+MUTEX.count], 0
@@:
        add  esp, MUTEX_WAITER.sizeof

        popfd
.done:
        ret

;void  __fastcall mutex_unlock(struct mutex *lock)

align 4
mutex_unlock:

        pushfd
        cli

        mov eax, [ecx+MUTEX.wait.next]
        cmp eax, ecx
        mov [ecx+MUTEX.count], 1
        je @F

        mov eax, [eax+MUTEX_WAITER.task]
        mov [eax+TASKDATA.state], 0
@@:
        popfd
        ret


purge MUTEX_WAITER

if 0

struc TIMER
{
  .next      dd ?
  .exp_time  dd ?
  .func      dd ?
  .arg       dd ?
}


MAX_PROIRITY         0   ; highest, used for kernel tasks
MAX_USER_PRIORITY    0   ; highest priority for user processes
USER_PRIORITY        7   ; default (should correspond to nice 0)
MIN_USER_PRIORITY   14   ; minimum priority for user processes
IDLE_PRIORITY       15   ; lowest, only IDLE process goes here
NR_SCHED_QUEUES     16   ; MUST equal IDLE_PRIORYTY + 1

uglobal
rdy_head   rd 16
endg

align 4
pick_task:

       xor eax, eax
  .pick:
       mov ebx, [rdy_head+eax*4]
       test ebx, ebx
       jz .next

       mov [next_task], ebx
       test [ebx+flags.billable]
       jz @F
       mov [bill_task], ebx
  @@:
       ret
  .next:
       inc eax
       jmp .pick

; param
;  eax= task
;
; retval
;  eax= task
;  ebx= queue
;  ecx= front if 1 or back if 0
align 4
shed:
       cmp [eax+.tics_left], 0 ;signed compare
       mov ebx, [eax+.priority]
       setg ecx
       jg @F

       mov edx, [eax+.tics_quantum]
       mov [eax+.ticks_left], edx
       cmp ebx, (IDLE_PRIORITY-1)
       je @F
       inc ebx
  @@:
       ret

; param
;  eax= task
align 4
enqueue:
      call shed  ;eax
      cmp [rdy_head+ebx*4],0
      jnz @F

      mov [rdy_head+ebx*4], eax
      mov [rdy_tail+ebx*4], eax
      mov [eax+.next_ready], 0
      jmp .pick
  @@:
      test ecx, ecx
      jz .back

      mov ecx, [rdy_head+ebx*4]
      mov [eax+.next_ready], ecx
      mov [rdy_head+ebx*4], eax
      jmp .pick
  .back:
      mov ecx, [rdy_tail+ebx*4]
      mov [ecx+.next_ready], eax
      mov [rdy_tail+ebx*4], eax
      mov [eax+.next_ready], 0
  .pick:
      call pick_proc     ;select next task
      ret

end if
