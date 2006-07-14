;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
irq0:
        save_ring3_context
	mov   ax, os_data
	mov   ds, ax
	mov   es, ax

        mov   edi,[0x3000]
        shl   edi, 3
                         ; fields of TSS descriptor:
;        mov   [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

        inc   dword [timer_ticks]

        mov   eax, [timer_ticks]
        call  playNote           ; <<<--- Speaker driver

        cmp   eax,[next_usage_update]
        jb    .nocounter
        add   eax,100
        mov   [next_usage_update],eax
        call  updatecputimes
     .nocounter:

        mov   edi, [0x3010]

        mov   ebx, [edi+0x18] ; time stamp counter add
        call  _rdtsc
        sub   eax, ebx
        add   eax, [edi+0x14] ; counter sum
        mov   [edi+0x14], eax

        mov   ebx,[0x3000]
        mov   [prev_slot], ebx

        cmp   [0xffff], byte 1
        je    .do_not_change_task


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

        mov   al, byte [edi+0xA]
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
        mov   [edi+0xA], byte 0
      .noevents:
        cmp   ebx, [prev_slot]
        sete  [0xffff]


     .do_not_change_task:

        call  _rdtsc
        mov   [edi+0x18],eax

        shl   ebx, 3
        xor   eax, eax
        add   ebx, tss0
        mov   word  [far_jump.sel],  bx   ; selector
        mov   dword [far_jump.offs], eax  ; offset
        
        cmp   [irq0needeoi],byte 0
        mov   [irq0needeoi],byte 1          
        jz    .noeoi

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al
.noeoi:   

        cmp   [0xffff],byte 0
        je    .switch
        dec   byte [0xffff]
        jz    @f
     .switch:
        jmp   pword [far_jump]
        inc   [context_counter] ;noname & halyavin
       @@:
        
        restore_ring3_context
        iret


uglobal
   align 4
   far_jump:
    .offs dd ?
    .sel  dw ?
   context_counter     dd ? ;noname & halyavin
   next_usage_update   dd ?
   timer_ticks         dd ?
   prev_slot dd ?
   event_sched dd ?
endg
iglobal
   irq0needeoi         db 1   
endg


align 4
change_task:

        pushfd
        cli
        mov   [0xffff],byte 2

        mov   [irq0needeoi],byte 0
        dec   dword [timer_ticks]  ; because irq0 will increase it

        int   0x20   ; irq0 handler
        popfd

        ret



align 4
updatecputimes:

        mov  eax,[idleuse]
        mov  [idleusesec],eax
        mov  [idleuse],dword 0
        mov  ecx, [0x3004]
        mov  edi, 0x3020
      .newupdate:
        mov  ebx,[edi+0x14]
        mov  [edi+0x1c],ebx
        mov  [edi+0x14],dword 0
        add  edi,0x20
        dec  ecx
        jnz  .newupdate

        ret
