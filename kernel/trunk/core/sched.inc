label  next_usage_update   dword   at  0xB008
label  timer_ticks         dword   at  0xFDF0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



align 32
irq0:
        cmp   [error_interrupt],-1
        je    no_error_in_previous_process

        mov   [0xffff],byte 0

        mov   [error_interrupt],-1

     no_error_in_previous_process:

        mov   edi,[0x3000]
        shl   edi, 3
                         ; fields of TSS descriptor:
        mov   [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

        inc   dword [timer_ticks]

        mov   eax, [timer_ticks]
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        call  playNote           ; <<<--- INSERT THIS LINE !!!!!!!!!!
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

        cmp   [0xffff], byte 1   ;1
        je   do_not_change_task ;je

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
        jbe   nsched0
        mov   ebx,1
        mov   edi,0x3020

      nsched0:

        mov   [0x3000],ebx
        mov   [0x3010],edi

      do_not_change_task:

        call  _rdtsc
        mov   [edi+0x18],eax

        cmp   [0xffff],byte 0
        je    nodecffff
        dec   byte [0xffff]
      nodecffff:


        shl   ebx, 3
        xor   eax, eax
        add   ebx, tss0
        mov   word  [0xB004], bx   ; selector    ;mov   [tss_s],bx
        mov   dword [0xB000], eax  ; offset

        mov   al,0x20   ; send End Of Interrupt signal
        mov   dx,0x20
        out   dx,al
.switch:
        jmp   pword [0xB000]
        inc [context_counter] ;noname & halyavin
        
        jmp   irq0

iglobal
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
