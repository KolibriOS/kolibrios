;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


align 32
irq0:
        save_ring3_context
	mov   ax, os_data
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

        cmp   [0xffff], byte 1
        jne   .change_task

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al

        mov   [0xffff], byte 0

        restore_ring3_context
        iret

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
        restore_ring3_context
        iret


align 4
change_task:

        pushfd
        cli
        pushad

        call  update_counters
        call  find_next_task
        test  eax, eax    ; the same task -> skip switch
        jnz    .return

        mov   [0xffff],byte 1
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
        mov   edi, [0x3010]
        mov   ebx, [edi+TASKDATA.counter_add] ; time stamp counter add
        call  _rdtsc
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
        mov   ebx, [0x3000]
        mov   edi, [0x3010]
        mov   [prev_slot], ebx

      .waiting_for_termination:
      .waiting_for_reuse:
      .waiting_for_event:
      .suspended:
        cmp   ebx, [0x3004]
        jb    @f
        mov   edi, 0x3000
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

        mov   [0x3000],ebx
        mov   [0x3010],edi

        cmp   al, 5
        jne   .noevents
        call  get_event_for_app
        test  eax, eax
        jz    .waiting_for_event
        mov   [event_sched], eax
        mov   [edi+TASKDATA.state], byte 0
      .noevents:
      .found:
        mov   [0x3000],ebx
        mov   [0x3010],edi
        call  _rdtsc
        mov   [edi+TASKDATA.counter_add],eax

	xor   eax, eax
        cmp   ebx, [prev_slot]
        sete  al
ret

; in: ebx = TSS selector index
do_change_task:
        shl   ebx, 3
        xor   eax, eax
        add   ebx, tss0
        mov   [far_jump.sel],  bx   ; selector
        mov   [far_jump.offs], eax  ; offset
        jmp   pword [far_jump]
        inc   [context_counter] ;noname & halyavin
ret



align 4
updatecputimes:

        mov  eax,[idleuse]
        mov  [idleusesec],eax
        mov  [idleuse],dword 0
        mov  ecx, [0x3004]
        mov  edi, 0x3020
      .newupdate:
        mov  ebx,[edi+TASKDATA.counter_sum]
        mov  [edi+TASKDATA.cpu_usage],ebx
        mov  [edi+TASKDATA.counter_sum],dword 0
        add  edi,0x20
        dec  ecx
        jnz  .newupdate

        ret
