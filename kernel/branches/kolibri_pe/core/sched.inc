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
        mov   ax, app_data  ;
        mov   ds, ax
        mov   es, ax

        inc   dword [timer_ticks]

        mov   eax, [timer_ticks]
        call  playNote           ; <<<--- Speaker driver

        cmp   eax,[next_usage_update]
        jb    .nocounter
        add   eax,100
        mov   [next_usage_update],eax
        call  updatecputimes
.nocounter:
        cmp   [DONT_SWITCH], byte 1
        jne   .change_task

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al

        mov   [DONT_SWITCH], byte 0

        popad
        iretd

.change_task:
        call  update_counters

        call  find_next_task
        mov   ecx, eax

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al

	test  ecx, ecx  ; if there is only one running process
        jnz   .return

        call  do_change_task

.return:
        popad
 ;       popfd
        iretd


align 4
change_task:

        pushfd
        cli
        pushad

        call  update_counters

if 0

; \begin{Mario79}
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

        call  find_next_task
        test  eax, eax    ; the same task -> skip switch
        jnz    .return
@@:
        mov   [DONT_SWITCH],byte 1
        call  do_change_task

.return:
        popad
        popfd
        ret


uglobal
   align 4
   far_jump:
    .offs dd ?
    .sel  dw ?
   context_counter     dd ? ;noname & halyavin
   next_usage_update   dd ?
   timer_ticks         dd ?
   prev_slot           dd ?
   event_sched         dd ?
endg


update_counters:
        mov   edi, [TASK_BASE]
        mov   ebx, [edi+TASKDATA.counter_add] ; time stamp counter add
        rdtsc
        sub   eax, ebx
        add   eax, [edi+TASKDATA.counter_sum] ; counter sum
        mov   [edi+TASKDATA.counter_sum], eax
ret


; Find next task to execute
; result: ebx = number of the selected task
;         eax = 1  if the task is the same
;         edi = address of the data for the task in ebx
;         [0x3000] = ebx and [0x3010] = edi
;         corrupts other regs
find_next_task:
        mov   ebx, [CURRENT_TASK]
        mov   edi, [TASK_BASE]
        mov   [prev_slot], ebx

.waiting_for_termination:
.waiting_for_reuse:
.waiting_for_event:
.suspended:
        cmp   ebx, [TASK_COUNT]
        jb    @f
        mov   edi, CURRENT_TASK
        xor   ebx, ebx
@@:

        add   edi,0x20
        inc   ebx

        mov   al, byte [edi+TASKDATA.state]
        test  al, al
        jz    .found
	cmp   al, 1
	jz    .suspended
	cmp   al, 2
	jz    .suspended
        cmp   al, 3
        je    .waiting_for_termination
        cmp   al, 4
        je    .waiting_for_termination
        cmp   al, 9
        je    .waiting_for_reuse

        mov   [CURRENT_TASK],ebx
        mov   [TASK_BASE],edi

        cmp   al, 5
        jne   .noevents
        call  get_event_for_app
        test  eax, eax
        jnz   @f
        mov   eax, ebx
        shl   eax, 8
        mov   eax, [SLOT_BASE + APPDATA.wait_timeout + eax]
        cmp   eax, [timer_ticks]
        jae   .waiting_for_event
        xor   eax, eax
@@:
        mov   [event_sched], eax
        mov   [edi+TASKDATA.state], byte 0
.noevents:
.found:
        mov   [CURRENT_TASK],ebx
        mov   [TASK_BASE],edi
        rdtsc                     ;call  _rdtsc
        mov   [edi+TASKDATA.counter_add],eax

        mov esi, [prev_slot]
	xor   eax, eax
        cmp   ebx, esi
        sete  al
ret

; param
;  ebx = incoming task
;  esi = outcomig task

do_change_task:

        shl ebx, 8
        add ebx, SLOT_BASE
        mov [current_slot], ebx

        shl esi, 8
        add esi, SLOT_BASE

        mov [esi+APPDATA.saved_esp], esp
        mov esp, [ebx+APPDATA.saved_esp]

; set thread io map

     ;   mov ecx, [ebx+APPDATA.io_map]
     ;   mov edx, [ebx+APPDATA.io_map+4]
     ;   mov dword [page_tabs+((tss._io_map_0 and -4096) shr 10)], ecx
     ;   mov dword [page_tabs+((tss._io_map_1 and -4096) shr 10)], edx

        mov eax, [ebx+APPDATA.dir_table]
        cmp eax, [esi+APPDATA.dir_table]
        je @F
        mov cr3, eax
@@:
        mov     eax, [ebx+APPDATA.saved_esp0]
        mov     [tss._esp0], eax
        mov ax, graph_data
        mov gs, ax

        mov eax, [CURRENT_TASK]
        cmp eax, [fpu_owner]
        clts                             ;clear a task switch flag
        je @F
                                         ;and set it again if the owner
        mov ecx, cr0                     ;of a fpu has changed
        or ecx, CR0_TS
        mov cr0, ecx
@@:
        inc   [context_counter] ;noname & halyavin
        test [ebx+APPDATA.dbg_state], 1
        jnz @F
        ret
@@:
        mov eax, [ebx+APPDATA.dbg_regs.dr0]
        mov dr0, eax
        mov eax, [ebx+APPDATA.dbg_regs.dr1]
        mov dr1, eax
        mov eax, [ebx+APPDATA.dbg_regs.dr2]
        mov dr2, eax
        mov eax, [ebx+APPDATA.dbg_regs.dr3]
        mov dr3, eax
        xor eax, eax
        mov dr6, eax
        mov eax, [ebx+APPDATA.dbg_regs.dr7]
        mov dr7, eax
        ret

align 4
updatecputimes:

        mov  eax,[idleuse]
        mov  [idleusesec],eax
        mov  [idleuse],dword 0
        mov  ecx, [TASK_COUNT]
        mov  edi, TASK_DATA
.newupdate:
        mov  ebx,[edi+TASKDATA.counter_sum]
        mov  [edi+TASKDATA.cpu_usage],ebx
        mov  [edi+TASKDATA.counter_sum],dword 0
        add  edi,0x20
        dec  ecx
        jnz  .newupdate

        ret

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

rdy_head   rd 16


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

