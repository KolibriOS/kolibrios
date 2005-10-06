sys_getevent:

     call   get_event_for_app
     mov    [esp+36],eax
     ret


align 4

sys_wait_event_timeout:

     mov   ebx,[timer_ticks];[0xfdf0]
     add   ebx,eax
     cmp   ebx,[timer_ticks];[0xfdf0]
     jna   .swfet2
   .swfet1:
     call  get_event_for_app
     test  eax,eax
     jne   .eventoccur_time
     call  change_task
     cmp   ebx,[timer_ticks];[0xfdf0]
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

     call  change_task
     call  get_event_for_app
     test  eax,eax
     je    newwait

   eventoccur:
     mov   [esp+36],eax
     ret


get_event_for_app:

     pushad

     mov   edi,[0x3010]              ; WINDOW REDRAW
     test  [edi],dword 1
     jz    no_eventoccur1
     mov   edi,[0x3010]
     cmp   [edi-twdw+31],byte 0
     je    no_eventoccur1
     popad
     mov   eax,1
     ret
   no_eventoccur1:

     mov   edi,[0x3010]              ; KEY IN BUFFER
     test  [edi],dword 2
     jz    no_eventoccur2
     mov   ecx, [0x3000]
     movzx edx,word [0xC000+ecx*2]
     mov   eax, [0x3004]
     cmp   eax,edx
     jne   no_eventoccur2
     cmp   [0xf400],byte 0
     je    no_eventoccur2
     popad
     mov   eax,2
     ret
   no_eventoccur2:

     mov   edi,[0x3010]              ; BUTTON IN BUFFER
     test  [edi],dword 4
     jz    no_eventoccur3
     mov   ecx, [0x3000]
     movzx edx, word [0xC000+ecx*2]
     mov   eax, [0x3004]
     cmp   eax,edx
     jnz   no_eventoccur3
     cmp   [0xf500],byte 0
     je    no_eventoccur3
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

     mov   edi,[0x3010]              ; DESKTOP BACKGROUND REDRAW
     test  [edi],dword 16
     jz    no_eventoccur5
     cmp   [0xfff0],byte 2
     jnz   no_eventoccur5
     popad
     mov   eax,5
     ret
   no_eventoccur5:

     mov   edi,[0x3010]              ; mouse event
     test  [edi],dword 00100000b
     jz    no_mouse_event
     mov   edi,[0x3000]
     shl   edi,8
     test  [edi+0x80000+0xA8],dword 00100000b
     jz    no_mouse_event
     and   [edi+0x80000+0xA8],dword 0xffffffff-00100000b
     popad
     mov   eax,6
     ret
   no_mouse_event:

     mov   edi,[0x3010]              ; IPC
     test  [edi],dword 01000000b
     jz    no_ipc
     mov   edi,[0x3000]
     shl   edi,8
     test  [edi+0x80000+0xA8],dword 01000000b
     jz    no_ipc
     and   [edi+0x80000+0xA8],dword 0xffffffff-01000000b
     popad
     mov   eax,7
     ret
   no_ipc:


     mov   edi,[0x3010]              ; STACK
     test  [edi],dword 10000000b
     jz    no_stack_event
     mov   edi,[0x3000]
     shl   edi,8
     test  [edi+0x80000+0xA8],dword 10000000b
     jz    no_stack_event
     and   [edi+0x80000+0xA8],dword 0xffffffff-10000000b
     popad
     mov   eax,7
     ret
   no_stack_event:


     mov   esi,0x2e0000              ; IRQ'S AND DATA
     mov   ebx,0x00010000
     xor   ecx, ecx
   irq_event_test:
     mov   edi,[0x3010]
     test  [edi],ebx
     jz    no_irq_event
     mov   edi,ecx
     shl   edi,2
     add   edi,irq_owner
     mov   edx,[edi]
     mov   eax,[0x3010]
     mov   eax,[eax+0x4]
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

     popad
     xor   eax, eax
     ret


