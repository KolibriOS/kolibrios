;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IRQ_RESERVED   equ 24

IRQ_POOL_SIZE  equ 48

uglobal

align 16
irqh_tab            rd sizeof.LHEAD * IRQ_RESERVED / 4

irqh_pool           rd sizeof.IRQH * IRQ_POOL_SIZE /4
next_irqh           rd 1

irq_active_set      rd 1
irq_failed          rd IRQ_RESERVED

endg

align 4
init_irqs:

        mov     ecx, IRQ_RESERVED
        mov     edi, irqh_tab
@@:
        mov     eax, edi
        stosd
        stosd
        loop    @B

        mov     ecx, IRQ_POOL_SIZE-1
        mov     eax, irqh_pool+sizeof.IRQH
        mov     [next_irqh], irqh_pool
@@:
        mov     [eax-sizeof.IRQH], eax
        add     eax, sizeof.IRQH
        loop    @B

        mov     [eax-sizeof.IRQH], dword 0
        ret


align 4
proc attach_int_handler stdcall, irq:dword, handler:dword, user_data:dword
         locals
           .irqh dd ?
         endl

        DEBUGF  1, "K : Attach Interrupt %d Handler %x\n", [irq], [handler]

        and     [.irqh], 0

        push    ebx

        mov     ebx, [irq]          ;irq num
        test    ebx, ebx
        jz      .err

        cmp     ebx, IRQ_RESERVED
        jae     .err

        mov     edx, [handler]
        test    edx, edx
        jz      .err

        spin_lock_irqsave IrqsList

;allocate handler

        mov     ecx, [next_irqh]
        test    ecx, ecx
        jz      .fail

        mov     eax, [ecx]
        mov     [next_irqh], eax
        mov     [.irqh], ecx

        mov     [irq_failed+ebx*4], 0;clear counter

        mov     eax, [user_data]
        mov     [ecx+IRQH.handler], edx
        mov     [ecx+IRQH.data], eax
        and     [ecx+IRQH.num_ints], 0

        lea     edx, [irqh_tab+ebx*8]
        list_add_tail ecx, edx     ;clobber eax
        stdcall enable_irq, ebx

.fail:
        spin_unlock_irqrestore IrqsList
.err:
        pop     ebx
        mov     eax, [.irqh]
        ret

endp

if 0
align 4
proc get_int_handler stdcall, irq:dword

        mov     eax, [irq]
        cmp     eax, 15
        ja      .fail
        mov     eax, [irq_tab + 4 * eax]
        ret
.fail:
        xor     eax, eax
        ret
endp
end if


align 4
proc  detach_int_handler

        ret
endp


macro irq_serv_h [num] {
    forward
align 4
  .irq_#num :
        push    num
        jmp     .main
}

align 16
irq_serv:

; .irq_1:
;      push 1
;      jmp .main
; etc...

irq_serv_h 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15
irq_serv_h 16, 17, 18, 19, 20, 21, 22, 23

purge irq_serv_h

align 16
.main:
        save_ring3_context

        mov     ebp, [esp + 32]
        mov     bx, app_data;os_data
        mov     ds, bx
        mov     es, bx

        cmp     [v86_irqhooks+ebp*8], 0
        jnz     v86_irq

        bts     [irq_active_set], ebp

        lea     esi, [irqh_tab+ebp*8]   ; esi= list head
        mov     ebx, esi
.next:
        mov     ebx, [ebx+IRQH.list.next]; ebx= irqh pointer
        cmp     ebx, esi
        je      .done

        push    ebx                     ; FIX THIS
        push    edi
        push    esi

        push    [ebx+IRQH.data]
        call    [ebx+IRQH.handler]
        pop     ecx

        pop     esi
        pop     edi
        pop     ebx

        test    eax, eax
        jz      .next

        inc     [ebx+IRQH.num_ints]
        btr     [irq_active_set], ebp
        jmp     .next

.done:
        btr     [irq_active_set], ebp
        jnc     .exit

; There is at least one configuration with one device which generates IRQ
; that is not the same as it should be according to PCI config space.
; For that device, the handler is registered at wrong IRQ.
; As a workaround, when nobody acknowledges the generated IRQ,
; try to ask all other registered handlers; if some handler acknowledges
; the IRQ this time, relink it to the current IRQ list.
; To make this more reliable, for every handler keep number of times
; that it has acknowledged an IRQ, and assume that handlers with at least one
; acknowledged IRQ are registered properly.
; Note: this still isn't 100% correct, because two IRQs can fire simultaneously,
; the better way would be to find the correct IRQ, but I don't know how to do
; this in that case.
        push    ebp
        xor     ebp, ebp
.try_other_irqs:
        cmp     ebp, [esp]
        jz      .try_next_irq
        cmp     ebp, 1
        jz      .try_next_irq
        cmp     ebp, 6
        jz      .try_next_irq
        cmp     ebp, 12
        jz      .try_next_irq
        cmp     ebp, 14
        jz      .try_next_irq
        cmp     ebp, 15
        jz      .try_next_irq
        lea     esi, [irqh_tab+ebp*8]
        mov     ebx, esi
.try_next_handler:
        mov     ebx, [ebx+IRQH.list.next]
        cmp     ebx, esi
        je      .try_next_irq
        cmp     [ebx+IRQH.num_ints], 0
        jne     .try_next_handler
; keyboard handler acknowledges everything
        push    [ebx+IRQH.data]
        call    [ebx+IRQH.handler]
        pop     ecx
        test    eax, eax
        jz      .try_next_handler

.found_in_wrong_list:
        DEBUGF 1,'K : warning: relinking handler from IRQ%d to IRQ%d\n',\
                ebp, [esp]
        pop     ebp
        spin_lock_irqsave IrqsList
        list_del ebx
        lea     edx, [irqh_tab+ebp*8]
        list_add_tail ebx, edx
        spin_unlock_irqrestore IrqsList
        jmp     .exit

.try_next_irq:
        inc     ebp
        cmp     ebp, 16
        jb      .try_other_irqs
        pop     ebp

.fail:
        inc     [irq_failed+ebp*4]
.exit:

        mov     ecx, ebp
        call    irq_eoi

; IRQ handler could make some kernel thread ready; reschedule
        mov     bl, SCHEDULE_HIGHER_PRIORITY
        call    find_next_task
        jz      .return  ; if there is only one running process
        call    do_change_task
  .return:
        restore_ring3_context
        add     esp, 4
        iret

align 4
irqD:
        push    eax
        push    ecx
        xor     eax, eax
        out     0xf0, al
        mov     cl, 13
        call    irq_eoi
        pop     ecx
        pop     eax
        iret

