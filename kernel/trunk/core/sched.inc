label  next_usage_update   dword   at  0xB008
label  timer_ticks         dword   at  0xFDF0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
irq0:
	pushad
	push  ds es
	mov   ax, os_data
	mov   ds, ax
	mov   es, ax

        mov   edi,[0x3000]
        shl   edi, 3
                         ; fields of TSS descriptor:
        mov   [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

        inc   dword [timer_ticks]

        mov   eax, [timer_ticks]
        cmp   eax,[next_usage_update]
        jb    .nocounter
        add   eax,100
        mov   [next_usage_update],eax
        call  updatecputimes
     .nocounter:

        call  playNote           ; <<<--- Speaker driver

        mov   edi, [0x3010]

        mov   ebx, [edi+0x18] ; time stamp counter add
        call  _rdtsc
        sub   eax, ebx
        add   eax, [edi+0x14] ; counter sum
        mov   [edi+0x14], eax

        mov   ebx,[0x3000]

        xor   esi,esi           ;esi - should we change task?
        cmp   [0xffff], byte 1
        je    .do_not_change_task

      .waiting_for_termination:
      .waiting_for_reuse:
        add   edi,0x20
        inc   ebx

        mov   al, byte [edi+0xA]
        cmp   al, 3
        je    .waiting_for_termination
        cmp   al, 4
        je    .waiting_for_termination
        cmp   al, 9
        je    .waiting_for_reuse

        cmp   ebx,[0x3004]
        jbe   @f
        mov   ebx,1
        mov   edi,0x3020
       @@:

        mov   [0x3000],ebx
        mov   [0x3010],edi

        jmp   @f
     .do_not_change_task:
        inc   esi          ;set don't change task flag
       @@:

        call  _rdtsc
        mov   [edi+0x18],eax

        cmp   [0xffff],byte 0
        je    @f
        dec   byte [0xffff]
       @@:


        shl   ebx, 3
        xor   eax, eax
        add   ebx, tss0
        mov   word  [0xB004], bx   ; selector
        mov   dword [0xB000], eax  ; offset

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al

        test  esi,esi   ; skip jmp to TSS if we aren't going to switch tasks
        jnz    @f
        
.switch:
        inc   [context_counter] ;noname & halyavin
        jmp   pword [0xB000]

       @@:
        pop   es ds
        popad
        iret



uglobal
context_counter dd 0 ;noname & halyavin
endg


align 4
change_task:

        mov   [0xffff],byte 2

        dec   dword [timer_ticks]  ; because irq0 will increase it

        int   0x20   ; irq0 handler

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
