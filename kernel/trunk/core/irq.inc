;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IRQ_RESERVED   equ 16

IRQ_POOL_SIZE  equ 48


macro __list_add new, prev, next
{
    mov [next+LHEAD.prev], new
    mov [new+LHEAD.next], next
    mov [new+LHEAD.prev], prev
    mov [prev+LHEAD.next], new
}

macro list_add new, head
{
    mov eax, [head+LHEAD.next]
    __list_add new, head, eax
}

macro list_add_tail new, head
{
    mov eax, [head+LHEAD.prev]
    __list_add new, eax, head
}

uglobal

align 16
irqh_tab            rd LHEAD.sizeof * IRQ_RESERVED / 4

irqh_pool           rd IRQH.sizeof * IRQ_POOL_SIZE /4
next_irqh           rd 1

irq_active_set      rd 1
irq_failed          rd IRQ_RESERVED

endg

align 4
init_irqs:

    mov ecx, IRQ_RESERVED
    mov edi, irqh_tab
@@:
    mov eax, edi
    stosd
    stosd
    loop @B

    mov ecx, IRQ_POOL_SIZE-1
    mov eax, irqh_pool+IRQH.sizeof
    mov [next_irqh], irqh_pool
@@:
    mov [eax-IRQH.sizeof], eax
    add eax, IRQH.sizeof
    loop @B

    mov [eax-IRQH.sizeof], dword 0
    ret


align 4
proc attach_int_handler stdcall, irq:dword, handler:dword, user_data:dword
         locals
           .irqh dd ?
         endl

         xchg bx, bx

         and [.irqh], 0

         push ebx

         mov  ebx, [irq]            ;irq num
         test ebx, ebx
         jz   .err

         cmp  ebx, IRQ_RESERVED
         jae  .err

         mov  edx, [handler]
         test edx, edx
         jz   .err

         pushfd
         cli

;allocate handler

         mov ecx, [next_irqh]
         test ecx, ecx
         jz .fail

         mov eax, [ecx]
         mov [next_irqh], eax

         mov [.irqh], ecx

         mov eax, [user_data]
         mov [ecx+IRQH.handler], edx
         mov [ecx+IRQH.data],    eax

         lea edx, [irqh_tab+ebx*8]
         list_add_tail ecx, edx     ;clobber eax

         stdcall enable_irq, [irq]

.fail:
         popfd
.err:
         pop ebx
         mov eax, [.irqh]
         ret

endp

if 0
align 4
proc get_int_handler stdcall, irq:dword

        mov eax, [irq]
        cmp eax, 15
        ja .fail
        mov eax, [irq_tab + 4 * eax]
        ret
.fail:
        xor eax, eax
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
    push num
       jmp .main
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

       mov   ebp, [esp + 32]
       mov   bx, app_data  ;os_data
       mov   ds, bx
       mov   es, bx

       cmp   [v86_irqhooks+ebp*8], 0
       jnz   v86_irq

       cmp   bp, 6
       jnz   @f
       push  ebp
       call  [fdc_irq_func]
       pop   ebp
@@:

       cmp   bp, 14
       jnz   @f
       push  ebp
       call  [irq14_func]
       pop   ebp
@@:
       cmp   bp, 15
       jnz   @f
       push  ebp
       call  [irq15_func]
       pop   ebp
@@:
       bts [irq_active_set], ebp

       lea esi, [irqh_tab+ebp*8]        ; esi= list head
       mov ebx, esi
.next:
       mov ebx, [ebx+IRQH.list.next]    ; ebx= irqh pointer
       cmp ebx, esi
       je .done

       push ebx                         ; FIX THIS
       push edi
       push esi

       push [ebx+IRQH.data]
       call [ebx+IRQH.handler]
       add esp, 4

       pop esi
       pop edi
       pop ebx

       test eax, eax
       jz .next

       btr [irq_active_set], ebp
       jmp .next

.done:
       btr [irq_active_set], ebp
       jnc .exit

       inc [irq_failed+ebp*4]
.exit:
       mov  [check_idle_semaphore],5

       mov ecx, ebp
       call irq_eoi

       restore_ring3_context
       add   esp, 4
       iret

align 4
irqD:
        push  eax
        push  ecx
        xor   eax,eax
        out   0xf0,al
        mov   cl, 13
        call  irq_eoi
        pop   ecx
        pop   eax
        iret

