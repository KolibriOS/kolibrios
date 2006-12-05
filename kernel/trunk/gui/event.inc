struc EVENT
{  .code    rd 1
            rd 5
   .next    rd 1   ;+24
   .prev    rd 1   ;+28
}
EVENT_SIZE  equ 32

virtual at 0
  EVENT EVENT
end virtual

align 4
init_events:
           stdcall kernel_alloc, 1024*EVENT_SIZE
           mov [events], eax
           xor eax, eax
           not eax
           mov edi, event_map
           mov [event_start], edi
           mov ecx, 128/4
           cld
           rep stosd
           mov [event_end], edi
           ret

align 4
proc alloc_event

           pushfd
           cli
           mov ebx, [event_start]
           mov ecx, [event_end]
.l1:
           bsf eax,[ebx]
           jnz .found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           popfd
           xor eax,eax
           ret
.found:
           btr [ebx], eax
           mov [event_start],ebx
           sub ebx, event_map
           shl ebx, 3
           add eax,ebx
           shl eax,5
           add eax, [events]
           popfd
           xor ebx, ebx
           mov [eax+EVENT.next], ebx
           mov [eax+EVENT.prev], ebx
           ret
endp

align 4
free_event:
           pushfd
           cli
           sub eax, [events]
           shr eax, 5
           mov ebx, event_map
           bts [ebx], eax
           shr eax, 3
           and eax, not 3
           add eax, ebx
           cmp [event_start], eax
           ja @f
           popfd
           ret
@@:
           mov [event_start], eax
           popfd
           ret

align 4
proc send_event stdcall pid:dword, event:dword
           locals
             slot     dd ?
           endl

           mov eax, [pid]
           call pid_to_slot
           test eax, eax
           jz .fail

           shl eax, 8
           cmp [PROC_BASE+eax+APPDATA.ev_count], 32
           ja .fail

           mov [slot], eax
           call alloc_event
           test eax, eax
           jz .fail

           mov edi, eax
           mov ecx, 6
           mov esi, [event]
           cld
           rep movsd

           mov esi, eax
           mov eax, [slot]
           mov edi, [PROC_BASE+eax+APPDATA.ev_last]
           mov [esi+EVENT.prev], edi
           test edi, edi
           jz .set_last
           mov [edi+EVENT.next], esi
.set_last:
           mov edx, [PROC_BASE+eax+APPDATA.ev_first]
           test edx, edx
           jnz @F
           mov [PROC_BASE+eax+APPDATA.ev_first], esi
@@:
           mov [PROC_BASE+eax+APPDATA.ev_last], esi
           inc [PROC_BASE+eax+APPDATA.ev_count]
           or  [PROC_BASE+eax+APPDATA.event_mask], EVENT_EXTENDED
.fail:
           ret
endp

align 4
proc get_event_ex stdcall, p_ev:dword, timeout:dword

.wait:
           mov ebx,[CURRENT_TASK]
           shl ebx,8
           cmp [PROC_BASE+ebx+APPDATA.ev_count], 0
           je .switch

           mov esi, [PROC_BASE+ebx+APPDATA.ev_first]
           mov edx, [esi+EVENT.next]
           mov [PROC_BASE+ebx+APPDATA.ev_first], edx
           test edx, edx
           jz @F
           mov [edx+EVENT.prev], 0
@@:
           jnz @F
           mov [PROC_BASE+ebx+APPDATA.ev_last], edx
           and dword [PROC_BASE+ebx+APPDATA.event_mask], not EVENT_EXTENDED
@@:
           dec [PROC_BASE+ebx+APPDATA.ev_count]

           mov eax, esi
           and dword [esi], 0xFF00FFFF
           mov edi, [p_ev]
           mov ecx, 6
           cld
           rep movsd
           call free_event
           ret
.switch:
           mov eax, [0x3010]
           mov [eax+TASKDATA.state], byte 5
	   call change_task
	   jmp .wait
endp

sys_getevent:

     call   get_event_for_app
     mov    [esp+36],eax
     ret


align 4
sys_wait_event_timeout:

     mov   ebx,[timer_ticks]
     add   ebx,eax
     cmp   ebx,[timer_ticks]
     jna   .swfet2
.swfet1:
     call  get_event_for_app
     test  eax,eax
     jne   .eventoccur_time
     call  change_task
     cmp   ebx,[timer_ticks]
     jg    .swfet1
.swfet2:
     xor   eax,eax
.eventoccur_time:
     mov   [esp+36],eax
     ret


align 4

sys_waitforevent:

     call  get_event_for_app
     test  eax,eax
     jne   eventoccur
   newwait:

     mov   eax, [0x3010]
     mov   [eax+TASKDATA.state], byte 5
     call  change_task

     mov eax, [event_sched]

   eventoccur:
     mov   [esp+36],eax
     ret

get_event_for_app:

     pushad

     mov   edi,[0x3010]              ; WINDOW REDRAW
     test  [edi+TASKDATA.event_mask],dword 1
     jz    no_eventoccur1
     ;mov   edi,[0x3010]
     cmp   [edi-twdw+WDATA.fl_redraw],byte 0
     je    no_eventoccur1
     popad
     mov   eax,1
     ret
   no_eventoccur1:

     ;mov   edi,[0x3010]              ; KEY IN BUFFER
     test  [edi+TASKDATA.event_mask],dword 2
     jz    no_eventoccur2
     mov   ecx, [0x3000]
     movzx edx,word [0xC000+ecx*2]
     mov   eax, [0x3004]
     cmp   eax,edx
     jne   no_eventoccur2x
     cmp   [0xf400],byte 0
     je    no_eventoccur2x
   eventoccur2:
     popad
     mov   eax,2
     ret
   no_eventoccur2x:
        mov     eax, hotkey_buffer
@@:
        cmp     [eax], ecx
        jz      eventoccur2
        add     eax, 8
        cmp     eax, hotkey_buffer+120*8
        jb      @b
   no_eventoccur2:

     ;mov   edi,[0x3010]              ; BUTTON IN BUFFER
     test  [edi+TASKDATA.event_mask],dword 4
     jz    no_eventoccur3
     cmp   [0xf500],byte 0
     je    no_eventoccur3
     mov   ecx, [0x3000]
     movzx edx, word [0xC000+ecx*2]
     mov   eax, [0x3004]
     cmp   eax,edx
     jnz   no_eventoccur3
     popad
     mov   eax,[0xf501]
     cmp   eax,65535
     je    no_event_1
     mov   eax,3
     ret

    no_event_1:
     mov   [window_minimize],1
     mov   [0xf500],byte 0
     xor   eax, eax
     ret

   no_eventoccur3:


     ;mov   edi,[0x3010]              ; mouse event
     test  [edi+TASKDATA.event_mask],dword 00100000b
     jz    no_mouse_event
     mov   eax,[0x3000]
     shl   eax,8
     test  [eax+0x80000+APPDATA.event_mask],dword 00100000b
     jz    no_mouse_event
     and   [eax+0x80000+APPDATA.event_mask],dword 0xffffffff-00100000b
     popad
     mov   eax,6
     ret
   no_mouse_event:


     ;mov   edi,[0x3010]              ; DESKTOP BACKGROUND REDRAW
     test  [edi+TASKDATA.event_mask],dword 16
     jz    no_eventoccur5
     cmp   [0xfff0],byte 2
     jnz   no_eventoccur5
     popad
     mov   eax,5
     ret
   no_eventoccur5:

     ;mov   edi,[0x3010]              ; IPC
     test  [edi+TASKDATA.event_mask],dword 01000000b
     jz    no_ipc
     mov   eax,[0x3000]
     shl   eax,8
     test  [eax+0x80000+APPDATA.event_mask],dword 01000000b
     jz    no_ipc
     and   [eax+0x80000+APPDATA.event_mask],dword 0xffffffff-01000000b
     popad
     mov   eax,7
     ret
   no_ipc:


     ;mov   edi,[0x3010]              ; STACK
     test  [edi+TASKDATA.event_mask],dword 10000000b
     jz    no_stack_event
     mov   eax,[0x3000]
     shl   eax,8
     test  [eax+0x80000+APPDATA.event_mask],dword 10000000b
     jz    no_stack_event
     and   [eax+0x80000+APPDATA.event_mask],dword 0xffffffff-10000000b
     popad
     mov   eax,8
     ret
   no_stack_event:

     test  byte [edi+TASKDATA.event_mask+1], 1		; DEBUG
     jz    .test_ext
     mov   eax, [0x3000]
     shl   eax, 8
     test  byte [eax+0x80000+APPDATA.event_mask+1], byte 1
     jz    .test_ext
     and   byte [eax+0x80000+APPDATA.event_mask+1], not 1
     popad
     mov   eax, 9
     ret

.test_ext:
     mov   eax, [0x3000]
     shl   eax, 8
     test  dword [eax+0x80000+APPDATA.event_mask], EVENT_EXTENDED
     jz .test_IRQ
     mov eax, 10
     ret

.test_IRQ:


     cmp   dword [edi+TASKDATA.event_mask], 0xFFFF
     jbe   no_events

     mov   esi,0x2e0000              ; IRQ'S AND DATA
     mov   ebx,0x00010000
     xor   ecx, ecx
   irq_event_test:
     mov   edi,[0x3010]
     test  [edi+TASKDATA.event_mask],ebx
     jz    no_irq_event
     mov   edi,ecx
     shl   edi,2
     add   edi,irq_owner
     mov   edx,[edi]
     mov   eax,[0x3010]
     mov   eax,[eax+TASKDATA.pid]
     cmp   edx,eax
     jne   no_irq_event
     cmp   [esi],dword 0
     jz    no_irq_event
     mov   eax,ecx
     add   eax,16
     mov   [esp+28],eax
     popad
     ret
    no_irq_event:
     add   esi,0x1000
     shl   ebx,1
     inc   ecx
     cmp   ecx,16
     jb    irq_event_test

   no_events:
     popad
     xor   eax, eax
     ret



