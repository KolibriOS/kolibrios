$Revision: 431 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
init_events:
           stdcall kernel_alloc, 512*EVENT_SIZE
           mov [events], eax
           xor eax, eax
           mov [event_uid], eax
           not eax
           mov edi, event_map
           mov [event_start], edi
           mov ecx, 64/4
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
           inc [event_uid]

           sub ebx, event_map
           lea eax,[eax+ebx*8]

           lea ebx, [eax+eax*4]
           shl eax,5
           lea eax,[eax+ebx*4]   ;eax*=52 (EVENT_SIZE)
           add eax, [events]
           mov ebx, [event_uid]
           popfd
           ret
endp

align 4
free_event:
           sub eax, [events]
           mov ecx, EVENT_SIZE
           mov ebx, event_map
           cdq
           div ecx

           pushfd
           cli
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

EVENT_WATCHED    equ 0x10000000
EVENT_SIGNALED   equ 0x20000000
MANUAL_RESET     equ 0x40000000
MANUAL_DESTROY   equ 0x80000000


; param
;  eax= event data
;  ebx= flags
;
; retval
;  eax= event
;  edx= id

create_event:
           .flags  equ  esp+4
           .data   equ  esp

           push ebx
           push eax

           call alloc_event
           test eax, eax
           jz .fail

           mov [eax+APPOBJ.magic], 'EVNT'
           mov [eax+APPOBJ.destroy], destroy_event.internal
           mov [eax+EVENT.id], ebx

           mov ebx, [CURRENT_TASK]
           shl ebx, 5
           mov ebx, [CURRENT_TASK+ebx+4]
           mov [eax+APPOBJ.pid], ebx
           mov edx, [.flags]
           mov [eax+EVENT.state], edx

           mov esi, [.data]
           test esi, esi
           jz @F
           lea edi, [eax+EVENT.code]
           mov ecx, 6
           cld
           rep movsd
@@:
           mov ecx, [current_slot]
           add ecx, APP_OBJ_OFFSET

           pushfd
           cli
           mov edx, [ecx+APPOBJ.fd]
           mov [eax+APPOBJ.fd], edx
           mov [eax+APPOBJ.bk], ecx
           mov [ecx+APPOBJ.fd], eax
           mov [edx+APPOBJ.bk], eax
           popfd
           mov edx, [eax+EVENT.id]
.fail:
           add esp, 8
           ret

restore .flags
restore .data

; param
;  eax= event
;  ebx= id

destroy_event:

           cmp [eax+APPOBJ.magic], 'EVNT'
           jne .fail
           cmp [eax+EVENT.id], ebx
           jne .fail
.internal:
           mov ebx, [eax+APPOBJ.fd]
           mov ecx, [eax+APPOBJ.bk]
           mov [ebx+APPOBJ.bk], ecx
           mov [ecx+APPOBJ.fd], ebx
.force:
           xor edx, edx             ;clear common header
           mov [eax], edx
           mov [eax+4], edx
           mov [eax+8], edx
           mov [eax+12], edx
           mov [eax+16], edx

           call free_event          ;release object memory
.fail:
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
           cmp [SLOT_BASE+eax+APPDATA.ev_count], 32
           ja .fail

           mov [slot], eax

           call alloc_event
           test eax, eax
           jz .fail

           lea edi, [eax+EVENT.code]
           mov ecx, 6
           mov esi, [event]
           cld
           rep movsd

           mov ecx, [slot]
           add ecx, SLOT_BASE+APP_EV_OFFSET

           mov [eax+APPOBJ.magic], 'EVNT'
           mov [eax+APPOBJ.destroy], destroy_event
           mov ebx, [pid]
           mov [eax+APPOBJ.pid], ebx
           mov [eax+EVENT.state], EVENT_SIGNALED

           pushfd
           cli                         ;insert event into
           mov edx, [ecx+APPOBJ.fd]    ;events list
           mov [eax+APPOBJ.fd], edx    ;and set events flag
           mov [eax+APPOBJ.bk], ecx
           mov [ecx+APPOBJ.fd], eax
           mov [edx+APPOBJ.bk], eax
           inc [ecx+APPDATA.ev_count-APP_EV_OFFSET]
           or  [ecx+APPDATA.event_mask-APP_EV_OFFSET], EVENT_EXTENDED
           popfd
.fail:
           ret
endp

; timeout ignored

align 4
proc get_event_ex stdcall, p_ev:dword, timeout:dword

.wait:
           mov edx,[current_slot]
;           cmp [SLOT_BASE+edx+APPDATA.ev_count], 0
;           je .switch

           add edx, APP_EV_OFFSET

           mov eax, [edx+APPOBJ.fd]
           cmp eax, edx
           je .switch

           lea esi, [eax+EVENT.code]
           mov edi, [p_ev]                ;copy event data
           mov ecx, 6
           cld
           rep movsd

           and dword [edi-24], 0xFF00FFFF ;clear priority field
                                         ;
           test [eax+EVENT.state], MANUAL_RESET
           jnz .done

           pushfd
           cli                         ;remove event from events
           mov ebx, [eax+APPOBJ.fd]    ;list (reset event)
           mov ecx, [eax+APPOBJ.bk]    ;and clear events flag
           mov [ebx+APPOBJ.bk], ecx    ;if no active events
           mov [ecx+APPOBJ.fd], ebx

           and [eax+EVENT.state], not (EVENT_SIGNALED+EVENT_WATCHED)

           dec [edx+APPDATA.ev_count-APP_EV_OFFSET]
           jnz @F
           and [edx+APPDATA.event_mask-APP_EV_OFFSET], not EVENT_EXTENDED
@@:
           popfd

           test [eax+EVENT.state], MANUAL_DESTROY
           jz .destroy

           add edx, (APP_OBJ_OFFSET-APP_EV_OFFSET)

           pushfd
           cli
           mov ebx, [edx+APPOBJ.fd]  ;insert event into
           mov [eax+APPOBJ.fd], ebx  ;objects list
           mov [eax+APPOBJ.bk], edx
           mov [edx+APPOBJ.fd], eax
           mov [ebx+APPOBJ.bk], eax
           popfd
.done:
           ret

.destroy:
           call destroy_event.force
           ret
.switch:
           mov eax, [TASK_BASE]
           mov [eax+TASKDATA.state], byte 5
	   call change_task
	   jmp .wait
endp

; param
;  eax= event
;  ebx= id

align 4
wait_event:
           .event equ esp
           push eax
.wait:
           cmp [eax+APPOBJ.magic], 'EVNT'
           jne .done
           cmp [eax+EVENT.id], ebx
           jne .done

           test [eax+EVENT.state], EVENT_SIGNALED
           jz .switch

           test [eax+EVENT.state], MANUAL_RESET
           jnz .done

           mov edx,[current_slot]

           pushfd
           cli                         ;remove event from events
           mov ebx, [eax+APPOBJ.fd]    ;list (reset event)
           mov ecx, [eax+APPOBJ.bk]    ;and clear events flag
           mov [ebx+APPOBJ.bk], ecx    ;if no active events
           mov [ecx+APPOBJ.fd], ebx
           dec [edx+APPDATA.ev_count]
           jnz @F
           and [edx+APPDATA.event_mask], not EVENT_EXTENDED
@@:
           and [eax+EVENT.state], not (EVENT_SIGNALED+EVENT_WATCHED)
           popfd

           test [eax+EVENT.state], MANUAL_DESTROY
           jz .destroy

           add edx, APP_OBJ_OFFSET

           pushfd
           cli
           mov ecx, [edx+APPOBJ.fd]  ;insert event into
           mov [eax+APPOBJ.fd], ecx  ;objects list
           mov [eax+APPOBJ.bk], edx
           mov [edx+APPOBJ.fd], eax
           mov [ecx+APPOBJ.bk], eax
           popfd
.done:
           add esp, 4
           ret
.destroy:
           call destroy_event.force
           add esp, 4
           ret
.switch:
           or [eax+EVENT.state], EVENT_WATCHED
           mov eax, [TASK_BASE]
           mov [eax+TASKDATA.state], byte 5
	   call change_task
           mov eax, [.event]
	   jmp .wait
restore .event

; param
;  eax= event
;  ebx= id
;  ecx= flags
;  edx= event data

raise_event:
           .event equ esp
           push eax

           cmp [eax+APPOBJ.magic], 'EVNT'
           jne .fail
           cmp [eax+EVENT.id], ebx
           jne .fail

           mov eax, [eax+APPOBJ.pid]
           call pid_to_slot
           test eax, eax
           jz .fail

           mov esi, edx
           test esi, esi
           mov edx, [.event]
           jz @F

           push ecx
           lea edi, [edx+EVENT.code]
           mov ecx, 6
           cld
           rep movsd
           pop ecx
@@:
           test [edx+EVENT.state], EVENT_SIGNALED
           jnz .done

           test ecx, EVENT_WATCHED
           jz @F
           test [edx+EVENT.state], EVENT_WATCHED
           jz .done
@@:
           shl eax, 8
           add eax, SLOT_BASE+APP_EV_OFFSET

           pushfd
           cli
           mov ebx, [edx+APPOBJ.fd]
           mov ecx, [edx+APPOBJ.bk]
           mov [ebx+APPOBJ.bk], ecx
           mov [ecx+APPOBJ.fd], ebx

           mov ecx, [eax+APPOBJ.fd]
           mov [edx+APPOBJ.fd], ecx
           mov [edx+APPOBJ.bk], eax
           mov [eax+APPOBJ.fd], edx
           mov [ecx+APPOBJ.bk], edx
           or [edx+EVENT.state], EVENT_SIGNALED

           inc [eax+APPDATA.ev_count-APP_EV_OFFSET]
           or  [eax+APPDATA.event_mask-APP_EV_OFFSET], EVENT_EXTENDED
           popfd
.fail:
.done:
           add esp, 4
           ret
restore .event

; param
;  eax= event
;  ebx= id
align 4
clear_event:
           .event equ esp
           push eax

           cmp [eax+APPOBJ.magic], 'EVNT'
           jne .fail
           cmp [eax+EVENT.id], ebx
           jne .fail

           mov eax, [eax+APPOBJ.pid]
           call pid_to_slot
           test eax, eax
           jz .fail

           shl eax, 8
           add eax, SLOT_BASE+APP_EV_OFFSET
           mov edx, [.event]
           pushfd
           cli                         ;remove event from events
           mov ebx, [edx+APPOBJ.fd]    ;list (reset event)
           mov ecx, [edx+APPOBJ.bk]    ;and clear events flag
           mov [ebx+APPOBJ.bk], ecx    ;if no active events
           mov [ecx+APPOBJ.fd], ebx

           and [edx+EVENT.state], not (EVENT_SIGNALED+EVENT_WATCHED)

           dec [eax+APPDATA.ev_count-APP_EV_OFFSET]
           jnz @F
           and [eax+APPDATA.event_mask-APP_EV_OFFSET], not EVENT_EXTENDED
@@:
           add eax, (APP_OBJ_OFFSET-APP_EV_OFFSET)

           mov ecx, [eax+APPOBJ.fd]  ;insert event into
           mov [edx+APPOBJ.fd], ecx  ;objects list
           mov [edx+APPOBJ.bk], eax
           mov [eax+APPOBJ.fd], edx
           mov [ecx+APPOBJ.bk], edx
           popfd
.fail:
.done:
           add esp, 4
           ret
restore .event

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

     mov   eax, [TASK_BASE]
     mov   [eax+TASKDATA.state], byte 5
     call  change_task

     mov eax, [event_sched]

   eventoccur:
     mov   [esp+36],eax
     ret

get_event_for_app:

     pushad

     mov   edi,[TASK_BASE]              ; WINDOW REDRAW
     test  [edi+TASKDATA.event_mask],dword 1
     jz    no_eventoccur1
     ;mov   edi,[TASK_BASE]
     cmp   [edi-twdw+WDATA.fl_redraw],byte 0
     je    no_eventoccur1
     popad
     mov   eax,1
     ret
   no_eventoccur1:

     ;mov   edi,[TASK_BASE]              ; KEY IN BUFFER
     test  [edi+TASKDATA.event_mask],dword 2
     jz    no_eventoccur2
     mov   ecx, [CURRENT_TASK]
     movzx edx,word [WIN_STACK+ecx*2]
     mov   eax, [TASK_COUNT]
     cmp   eax,edx
     jne   no_eventoccur2x
     cmp   [KEY_COUNT],byte 0
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

     ;mov   edi,[TASK_BASE]              ; BUTTON IN BUFFER
     test  [edi+TASKDATA.event_mask],dword 4
     jz    no_eventoccur3
     cmp   [BTN_COUNT],byte 0
     je    no_eventoccur3
     mov   ecx, [CURRENT_TASK]
     movzx edx, word [WIN_STACK+ecx*2]
     mov   eax, [TASK_COUNT]
     cmp   eax,edx
     jnz   no_eventoccur3
     popad
     mov   eax,[BTN_BUFF]
     cmp   eax,65535
     je    no_event_1
     mov   eax,3
     ret

    no_event_1:
     mov   [window_minimize],1
     mov   [BTN_COUNT],byte 0
     xor   eax, eax
     ret

no_eventoccur3:

     ;mov   edi,[TASK_BASE]              ; mouse event
     test  [edi+TASKDATA.event_mask],dword 00100000b
     jz    no_mouse_event

     mov eax, [CURRENT_TASK]
     shl eax, 8
     test  [eax+SLOT_BASE+APPDATA.event_mask],dword 00100000b
     jz    no_mouse_event
     and   [eax+SLOT_BASE+APPDATA.event_mask],dword (not 00100000b)
     popad
     mov   eax,6
     ret
no_mouse_event:

     ;mov   edi,[TASK_BASE]              ; DESKTOP BACKGROUND REDRAW
     test  [edi+TASKDATA.event_mask],dword 16
     jz    no_eventoccur5
     cmp   [REDRAW_BACKGROUND],byte 2
     jnz   no_eventoccur5
     popad
     mov   eax,5
     ret
no_eventoccur5:

     ;mov   edi,[TASK_BASE]              ; IPC
     test  [edi+TASKDATA.event_mask],dword 01000000b
     jz    no_ipc
     mov   eax,[current_slot]
     test  [eax+APPDATA.event_mask],dword 01000000b
     jz    no_ipc
     and   [eax+APPDATA.event_mask],dword 0xffffffff-01000000b
     popad
     mov   eax,7
     ret
no_ipc:


     ;mov   edi,[TASK_BASE]              ; STACK
     test  [edi+TASKDATA.event_mask],dword 10000000b
     jz    no_stack_event
     mov   eax,[current_slot]
     test  [eax+APPDATA.event_mask],dword 10000000b
     jz    no_stack_event
     and   [eax+APPDATA.event_mask],dword 0xffffffff-10000000b
     popad
     mov   eax,8
     ret
no_stack_event:

     test  byte [edi+TASKDATA.event_mask+1], 1		; DEBUG
     jz    .test_IRQ
     mov   eax, [current_slot]
     test  byte [eax+APPDATA.event_mask+1], byte 1
     jz    .test_IRQ
     and   byte [eax+APPDATA.event_mask+1], not 1
     popad
     mov   eax, 9
     ret

;.test_ext:
;     mov   eax, [CURRENT_TASK]
;     shl   eax, 8
;     test  dword [eax+SLOT_BASE+APPDATA.event_mask], EVENT_EXTENDED
;     jz .test_IRQ
;     popad
;     mov eax, 10
;     ret

.test_IRQ:
     cmp   dword [edi+TASKDATA.event_mask], 0xFFFF
     jbe   no_events

     mov   esi,IRQ_SAVE              ; IRQ'S AND DATA
     mov   ebx,0x00010000
     xor   ecx, ecx
   irq_event_test:
     mov   edi,[TASK_BASE]
     test  [edi+TASKDATA.event_mask],ebx
     jz    no_irq_event
     mov   edi,ecx
     shl   edi,2
     add   edi,irq_owner
     mov   edx,[edi]
     mov   eax,[TASK_BASE]
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



