label  next_usage_update   dword   at  0xB008
label  timer_ticks         dword   at  0xFDF0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; IRQ0 HANDLER (TIMER INTERRUPT) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32
irq0:

        cmp   [error_interrupt],-1
        je    no_error_in_previous_process

        mov   edi,[error_interrupt]
        shl   edi, 3
        mov   [edi+tss0i_l +5], word 01010000b *256 +11101001b

        mov   edi,[error_interrupt]
        shl   edi,7
        add   edi,0x290000
        mov   esi,[error_interrupt_entry]
        mov   [edi+l.eip-tss_sceleton],esi
        mov   [edi+l.eflags-tss_sceleton],dword 0x11002

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
      .waiting_on_queue:
        add   edi,0x20
        inc   ebx

        mov   al, byte [edi+0xA]
        cmp   al, 3
        je    .waiting_for_termination
        cmp   al, 4
        je    .waiting_for_termination
        cmp   al, 9
        je    .waiting_for_reuse
        cmp   al, 16
        je    .waiting_on_queue
        cmp   al, 17
        je    .waiting_on_queue

        cmp   ebx,[0x3004]
        jbe   nsched0
        mov   ebx,1
        mov   edi,0x3020

      nsched0:

        mov   [0x3000],ebx
        mov   [0x3010],edi

      do_not_change_task:

        mov   edx,[0x3000]
        lea   edx,[tss0sys+8*edx]
        ;mov   [8*0x40+idts+8+0], word 0
        ;mov   [8*0x40+idts+8+2],dx
        ;mov   [8*0x40+idts+8+4],word 11100101b*256
        ;mov   [8*0x40+idts+8+6], word 0

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



;
; Wait queue is 16 bytes
; dd return code                +12
; dd pointer to process         +8
; dd prev                       +4
; dd next                       +0
;
; eax - pointer to pointer to the wait queue
; return:
; ecx - return code
sleep_on_queue:
    sub esp,16          ; reserve space for wait node
    mov ecx,esp         ; ecx=this_node, [eax]=queue

    pusha

    mov ebx,[0x3010]    ; get pointer to the current process
    mov [ecx+8],ebx

    pushf
    cli                 ; adding element to the wait queue must be atomic

    mov edi,[eax]       ; edi=queue
    and edi,edi         ; check if queue is empty
    jz .is_empty

    ; add element at the end of wait queue

    mov edx,[edi+4]     ; get pointer to prev edx=queue->prev
    mov [ecx+4],edx     ; this_node->prev=queue->prev
    mov [ecx+0],edi     ; this_node->next=queue
    mov [edx+0],ecx     ; this_node->prev->next=this_node
    mov [edi+4],ecx     ; queue->prev=this_node
    jmp .added_ok
.is_empty:
    ; set this element as first in the queue
    mov [ecx+0],ecx     ; this_node->next=this_node
    mov [ecx+4],ecx     ; this_node->prev=this_node
    mov [eax],ecx       ; [queue]=this_node
.added_ok:

    popf                ; we can safely restore interrupts

    mov [ebx+0xa],byte 17 ; set current task state as sleeping
    call change_task    ; schedule new thread

    ; someone has called wake_up_queue

    pushf               ; disable interrupts
    cli

    mov edx,[ecx+0]     ; edx=this_node->next
    mov esi,[ecx+4]     ; esi=this_node->prev

    ; check if we need to remove this node from head
    cmp [eax],ecx
    jne .no_head

    cmp [ecx+0],ecx ; check if queue is empty
    jne .not_empty

    mov [eax],dword 0
    jmp .no_head

.not_empty:
    mov [eax],edx

    ; remove our node from the queue (this must be atomic)
.no_head:
    mov [edx+4],esi     ; this_node->next->prev=this_node->prev
    mov [esi+0],edx     ; this_node->prev->next=this_node->next

    popf
    popa
    add esp,12
    pop ecx
    ret

; eax - pointer to the wait queue
; ebx - wake up all (1=yes, 0=no)
; ecx - return code
; return:
; ebx - number of processes woken
wake_up_queue:
    and eax,eax
    jnz .nz
    ret
.nz:
    push eax
    push ebx
    push ecx
    push edx
    push esi

    pushf
    cli

    xor ebx,ebx
    mov edx,eax
.wake_loop:

    mov [edx+12],ecx
    mov esi,[edx+8]
    mov byte [esi+0xa],0
    inc ebx

    mov edx,[edx+0]
    cmp edx,eax
    jne .wake_loop

    and ebx,ebx
    jz .wake_up_1

.return_it:
    popf
    pop esi
    pop edx
    pop ecx
    add esp,4
    pop eax
    ret
.wake_up_1:
    mov [eax+12],ecx
    mov ecx,[eax+8]
    mov byte [ecx+0xa],0
    jmp .return_it
