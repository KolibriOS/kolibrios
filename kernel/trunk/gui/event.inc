;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

WINDOW_MOVE_AND_RESIZE_FLAGS = \
  mouse.WINDOW_RESIZE_N_FLAG + \
  mouse.WINDOW_RESIZE_W_FLAG + \
  mouse.WINDOW_RESIZE_S_FLAG + \
  mouse.WINDOW_RESIZE_E_FLAG + \
  mouse.WINDOW_MOVE_FLAG

uglobal
align 4
  event_start   dd ?
  event_end     dd ?
  event_uid     dd 0
endg
EV_SPACE   = 512
FreeEvents = event_start-EVENT.fd    ; "virtual" event, only following field used:
                                     ;  FreeEvents.fd=event_start and FreeEvents.bk=event_end
;-----------------------------------------------------------------------------
align 4
init_events:                                       ;; used from kernel.asm
        stdcall kernel_alloc, EV_SPACE*sizeof.EVENT
        or      eax, eax
        jz      .fail
      ; eax - current event, ebx - previous event below
        mov     ecx, EV_SPACE        ; current - in allocated space
        mov     ebx, FreeEvents      ; previous - list beginning
        push    ebx                  ; it will be the end later
;--------------------------------------
align 4
@@:
        mov     [ebx+EVENT.fd], eax
        mov     [eax+EVENT.bk], ebx
        mov     ebx, eax             ; previos <- current
        add     eax, sizeof.EVENT    ; new current
        loop    @b
        pop     eax                  ; here it became the end
        mov     [ebx+EVENT.fd], eax
        mov     [eax+EVENT.bk], ebx
;--------------------------------------
align 4
.fail:
        ret
;-----------------------------------------------------------------------------
EVENT_WATCHED    = 0x10000000 ; bit 28
EVENT_SIGNALED   = 0x20000000 ; bit 29
MANUAL_RESET     = 0x40000000 ; bit 30
MANUAL_DESTROY   = 0x80000000 ; bit 31
;-----------------------------------------------------------------------------
align 4
create_event:                                      ;; EXPORT use
;info:
;   Move EVENT from the FreeEvents list to the ObjList list of the current slot;
;   EVENT.state is set from ecx, EVENT.code indirectly from esi (if esi <> 0)
;param:
;   esi - event data
;   ecx - flags
;retval:
;   eax - event (=0 => fail)
;   edx - uid
;scratched: ebx,ecx,esi,edi
        mov     ebx, [current_slot]
        add     ebx, APP_OBJ_OFFSET
        mov     edx, [TASK_BASE]
        mov     edx, [edx+TASKDATA.pid]
        pushfd
        cli
;--------------------------------------
align 4
set_event:                                         ;; INTERNAL use !!! don't use for Call
;info:
;   We take a new event from FreeEvents, fill its fields from ecx, edx, esi
;   and add it to the list, which specified in ebx.
;   Return event (to eax), and it's uid (to edx)
;param:
;   ebx - start-chain "virtual" event for entry new event Right of him
;   ecx - flags      (copied to EVENT.state)
;   edx - pid        (copied to EVENT.pid)
;   esi - event data (copied to EVENT.code indirect, =0 => skip)
;retval:
;   eax - event (=0 => fail)
;   edx - uid
;scratched: ebx,ecx,esi,edi
        mov     eax, FreeEvents
        cmp     eax, [eax+EVENT.fd]
        jne     @f  ; not empty ???
        pushad
        call    init_events
        popad
        jz      RemoveEventTo.break ; POPF+RET
;--------------------------------------
align 4
@@:
        mov     eax, [eax+EVENT.fd]
        mov     [eax+EVENT.magic], 'EVNT'
        mov     [eax+EVENT.destroy], destroy_event.internal
        mov     [eax+EVENT.state], ecx
        mov     [eax+EVENT.pid], edx
        inc     [event_uid]
        mov     edx, [event_uid]
        mov     [eax+EVENT.id], edx
        or      esi, esi
        jz      RemoveEventTo
        lea     edi, [eax+EVENT.code]
        mov     ecx, (sizeof.EVENT -EVENT.code)/4
        cld
        rep movsd
;--------------------------------------
align 4
RemoveEventTo:                                     ;; INTERNAL use !!! don't use for Call
;param:
;   eax - pointer to event, WHICH we will insert
;   ebx - pointer to event, AFTER which we will insert
;scratched: ebx,ecx
        mov     ecx, eax             ; ecx=eax=Self,      ebx=NewLeft
        xchg    ecx, [ebx+EVENT.fd]  ; NewLeft.fd=Self,   ecx=NewRight
        cmp     eax, ecx             ; stop, I think...
        je      .break               ; - am I not a fool?
        mov     [ecx+EVENT.bk], eax  ; NewRight.bk=Self
        xchg    ebx, [eax+EVENT.bk]  ; Self.bk=NewLeft,   ebx=OldLeft
        xchg    ecx, [eax+EVENT.fd]  ; Self.fd=NewRight,  ecx=OldRight
        mov     [ebx+EVENT.fd], ecx  ; OldLeft.fd=OldRight
        mov     [ecx+EVENT.bk], ebx  ; OldRight.bk=OldLeft
;--------------------------------------
align 4
.break:
        popfd
        ret
;-----------------------------------------------------------------------------
align 4
NotDummyTest:                                      ;; INTERNAL use (not returned for fail !!!)
        pop     edi
        call    DummyTest ; not returned for fail !!!
        mov     ebx, eax
        mov     eax, [ebx+EVENT.pid]
        push    edi
;--------------------------------------
align 4
.small: ; somehow ugly...
        pop     edi
        pushfd
        cli
        call    pid_to_slot ; saved all registers (eax - retval)
        shl     eax, 8
        jz      RemoveEventTo.break ; POPF+RET
        jmp     edi ; normal return
;-----------------------------------------------------------------------------
align 4
raise_event:                                       ;; EXPORT use
;info:
;   Setting up EVENT.code data
;   If is has flag EVENT_SIGNALED activated - nothing else
;   Otherwise: activate this flag, except when the EVENT_WATCHED flag is present in edx
;   In this case EVENT_SIGNALED will activated only if EVENT_WATCHED presents in the event itself
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;   edx - flags
;   esi - event data (=0 => skip)
;scratched: ebx,ecx,esi,edi
        call    NotDummyTest ; not returned for fail !!!
        or      esi, esi
        jz      @f
        lea     edi, [ebx+EVENT.code]
        mov     ecx, (sizeof.EVENT -EVENT.code)/4
        cld
        rep movsd
;--------------------------------------
align 4
@@:
        test    byte[ebx+EVENT.state+3], EVENT_SIGNALED shr 24
        jnz     RemoveEventTo.break  ; POPF+RET
        bt      edx, 28 ;EVENT_WATCHED
        jnc     @f
        test    byte[ebx+EVENT.state+3], EVENT_WATCHED shr 24
        jz      RemoveEventTo.break  ; POPF+RET
;--------------------------------------
align 4
@@:
        or      byte[ebx+EVENT.state+3], EVENT_SIGNALED shr 24
        add     eax, SLOT_BASE+APP_EV_OFFSET
        xchg    eax, ebx
        jmp     RemoveEventTo
;-----------------------------------------------------------------------------
align 4
clear_event:                                       ;; EXPORT use
;info:
;
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;scratched: ebx,ecx
        call    NotDummyTest ; not returned for fail !!!
        add     eax, SLOT_BASE+APP_OBJ_OFFSET
        and     byte[ebx+EVENT.state+3], not((EVENT_SIGNALED+EVENT_WATCHED)shr 24)
        xchg    eax, ebx
        jmp     RemoveEventTo
;-----------------------------------------------------------------------------
align 4
send_event:                                        ;; EXPORT use
;info:
;   Creates a new EVENT (pulls from the FreeEvents list) in the EventList list
;   of target slot (eax=pid), with data from esi indirectly, and state=EVENT_SIGNALED
;param:
;   eax - slots pid, to sending new event
;   esi - pointer to sending data (in code field of new event)
;retval:
;   eax - event (=0 => fail)
;   edx - uid
;warning:
;   may be used as CDECL with such prefix...
;       mov     esi,[esp+8]
;       mov     eax,[esp+4]
;   but not as STDCALL :(
;scratched: ebx,ecx,esi,edi
        mov     edx, eax
        call    NotDummyTest.small ; not returned for fail !!!
        lea     ebx, [eax+SLOT_BASE+APP_EV_OFFSET]
        mov     ecx, EVENT_SIGNALED
        jmp     set_event
;-----------------------------------------------------------------------------
align 4
DummyTest:                                         ;; INTERNAL use (not returned for fail !!!)
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
        cmp     [eax+EVENT.magic], 'EVNT'
        jne     @f
        cmp     [eax+EVENT.id], ebx
        je      .ret
;--------------------------------------
align 4
@@:
        pop     eax
        xor     eax, eax
;--------------------------------------
align 4
.ret:
        ret
;-----------------------------------------------------------------------------
align 4
Wait_events:
        or      ebx, -1; infinite timeout
;--------------------------------------
align 4
Wait_events_ex:
;info:
;   Waiting for an "abstract" event by moving the slot to the 5th position.
;   Abstractness lies in the fact, that the fact of an event is determined by the APPDATA.wait_test function,
;   which is set by the client and can be actually anything.
;   This allows the shed to reliably determine the fact of the event, and not make "idle" switches,
;   intended for showdowns like "friend / foe" within the problem.
;param:
;   edx - wait_test, client testing function (code address)
;   ecx - wait_param, additional parameter, possibly needed for [wait_test]
;   ebx - wait_timeout
;retval:
;   eax - call result [wait_test] (=0 => timeout)
;scratched: esi
        mov     esi, [current_slot]
        mov     [esi+APPDATA.wait_param], ecx
        pushad
        mov     ebx, esi  ;now this is a question, what where to put..........
        pushfd  ; this is a consequence of the general concept: let the test function have
        cli     ; the right to hope to disable interrupts, as when called from shed
        call    edx
        popfd
        mov     [esp+28], eax
        popad
        or      eax, eax
        jnz     @f   ;RET
        mov     [esi+APPDATA.wait_test], edx
        mov     [esi+APPDATA.wait_timeout], ebx
        mov     eax, [timer_ticks]
        mov     [esi+APPDATA.wait_begin], eax
        mov     eax, [TASK_BASE]
        mov     [eax+TASKDATA.state], 5
        call    change_task
        mov     eax, [esi+APPDATA.wait_param]
;--------------------------------------
align 4
@@:
        ret
;-----------------------------------------------------------------------------
align 4
wait_event:                                        ;; EXPORT use
;info:
;   Waiting for the EVENT_SIGNALED flag in a very specific Event
;   (set, presumably, via raise_event)
;   When the MANUAL_RESET flag is active, nothing else
;   Otherwise: the flags EVENT_SIGNALED and EVENT_WATCHED for the received event are cleared,
;   and, if MANUAL_DESTROY is active, it moves to the ObjList list of the current slot,
;   and if not active, it is destroyed normally (destroy_event.internal)
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;scratched: ecx,edx,esi
        call    DummyTest
        mov     ecx, eax             ; wait_param
        mov     edx, get_event_alone ; wait_test
        call    Wait_events          ; timeout ignored
        jmp     wait_finish
;-----------------------------------------------------------------------------
align 4
wait_event_timeout:
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;   ecx - timeout in timer ticks
;retval:
;   eax - EVENT handle or 0 if timeout
        call    DummyTest
        mov     ebx, ecx
        mov     ecx, eax             ; wait_param
        mov     edx, get_event_alone ; wait_test
        call    Wait_events_ex
        test    eax, eax
        jnz     wait_finish
        ret
;-----------------------------------------------------------------------------
align 4
get_event_ex:                                      ;; f68:14
;info:
;   Waiting for any event in the EventList of the current slot
;   Code event data - copied to application memory (indirectly by edi)
;   When the MANUAL_RESET flag is active, nothing else
;   Otherwise: the flags EVENT_SIGNALED and EVENT_WATCHED for the received event are cleared,
;   and, if MANUAL_DESTROY is active, it moves to the ObjList list of the current slot,
;   and if not active, it is destroyed normally (destroy_event.internal)
;param:
;   edi - address in the application code to copy data from EVENT.code
;retval:
;   eax - EVENT itself (we will call it a handle)
;scratched: ebx,ecx,edx,esi,edi
        mov     edx, get_event_queue ; wait_test
        call    Wait_events          ; timeout ignored
        lea     esi, [eax+EVENT.code]
        mov     ecx, (sizeof.EVENT-EVENT.code)/4
        cld
        rep movsd
        mov     byte[edi-(sizeof.EVENT-EVENT.code)+2], cl;clear priority field
;--------------------------------------
align 4
wait_finish:
        test    byte[eax+EVENT.state+3], MANUAL_RESET shr 24
        jnz     get_event_queue.ret  ; RET
        and     byte[eax+EVENT.state+3], not((EVENT_SIGNALED+EVENT_WATCHED)shr 24)
        test    byte[eax+EVENT.state+3], MANUAL_DESTROY shr 24
        jz      destroy_event.internal
        mov     ebx, [current_slot]
        add     ebx, APP_OBJ_OFFSET
        pushfd
        cli
        jmp     RemoveEventTo
;-----------------------------------------------------------------------------
align 4
destroy_event:                                     ;; EXPORT use
;info:
;   Move EVENT to the FreeEvents list, clear the magic, destroy, pid, id fields
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;retval:
;   eax - address of EVENT object (=0 => fail)
;scratched: ebx,ecx
        call    DummyTest ; not returned for fail !!!
;--------------------------------------
align 4
.internal:
        xor     ecx, ecx  ; clear common header
        pushfd
        cli
        mov     [eax+EVENT.magic], ecx
        mov     [eax+EVENT.destroy], ecx
        mov     [eax+EVENT.pid], ecx
        mov     [eax+EVENT.id], ecx
        mov     ebx, FreeEvents
        jmp     RemoveEventTo
;-----------------------------------------------------------------------------
align 4
get_event_queue:
;info:
;   client testing function for get_event_ex
;warning:
;  -don't use [TASK_BASE],[current_slot],[current_slot_idx] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - APPDATA address of testing slot
;retval:
;   eax - address of EVENT object (=0 => fail)
        add     ebx, APP_EV_OFFSET
        mov     eax, [ebx+APPOBJ.bk] ; we choose from the end, according to the FIFO principle
        cmp     eax, ebx ; empty ???
        je      get_event_alone.ret0
;--------------------------------------
align 4
.ret:
        ret
;-----------------------------------------------------------------------------
align 4
get_event_alone:
;info:
;   client testing function for wait_event
;warning:
;  -don't use [TASK_BASE],[current_slot],[current_slot_idx] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - APPDATA address of testing slot
;retval:
;   eax - address of EVENT object (=0 => fail)
        mov     eax, [ebx+APPDATA.wait_param]
        test    byte[eax+EVENT.state+3], EVENT_SIGNALED shr 24
        jnz     .ret
        or      byte[eax+EVENT.state+3], EVENT_WATCHED shr 24
;--------------------------------------
align 4
.ret0:
        xor     eax, eax; NO event!!!
;--------------------------------------
align 4
.ret:
        ret
;-----------------------------------------------------------------------------
align 4
sys_sendwindowmsg:                                 ;; f72
        dec     ebx
        jnz     .ret ;subfunction==1 ?
        pushfd
        cli
        sub     ecx, 2
        je      .sendkey
        dec     ecx
        jnz     .retf
;--------------------------------------
align 4
.sendbtn:
        cmp     byte[BTN_COUNT], 1
        jae     .result ;overflow
        inc     byte[BTN_COUNT]
        shl     edx, 8
        mov     [BTN_BUFF], edx
        jmp     .result
;--------------------------------------
align 4
.sendkey:
        movzx   eax, byte[KEY_COUNT]
        cmp     al, 120
        jae     .result ;overflow
        inc     byte[KEY_COUNT]
        mov     [KEY_BUFF+eax], dl
; store empty scancode
        add     eax, 120+2
        mov     [KEY_BUFF+eax], byte 0
        sub     eax, 120+2
;--------------------------------------
align 4
.result:
        setae   byte[esp+32+4] ;we consider that initially was: dword[esp+32+4]==72
;--------------------------------------
align 4
.retf:
        popfd
;--------------------------------------
align 4
.ret:
        ret
;-----------------------------------------------------------------------------
align 4
sys_getevent:                                      ;; f11
        mov     ebx, [current_slot]  ;now this is a question, what where to put......
        pushfd  ; this is a consequence of the general concept: let the test function have
        cli     ; the right to hope to disable interrupts, as when called from shed
        call    get_event_for_app
        popfd
        mov     [esp+32], eax
        ret
;-----------------------------------------------------------------------------
align 4
sys_waitforevent:                                  ;; f10
        or      ebx, -1; infinite timeout
;--------------------------------------
align 4
sys_wait_event_timeout:                            ;; f23
        call    unprotect_from_terminate
        mov     edx, get_event_for_app; wait_test
        call    Wait_events_ex        ; ebx - timeout
        mov     [esp+32], eax
        call    protect_from_terminate
        ret
;-----------------------------------------------------------------------------
align 4
get_event_for_app:                                 ;; used from f10,f11,f23
;info:
;   client testing function for applications (f10,f23)
;warning:
;  -don't use [TASK_BASE],[current_slot],[current_slot_idx] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - APPDATA address of testing slot
;retval:
;   eax - event number (=0 => no events)
        movzx   edi, bh               ; bh  is assumed as [current_slot_idx]
        shl     edi, 5
        add     edi, TASK_TABLE     ; edi is assumed as [TASK_BASE]
        mov     ecx, [edi+TASKDATA.event_mask]
        and     ecx, 0x7FFFFFFF
;--------------------------------------
align 4
.loop: ; until we run out all the bits of the mask
        bsr     eax, ecx       ; find a non-zero bit of the mask (31 -> 0)
        jz      .no_events     ; ran out all the bits of the mask but found nothing ???
        btr     ecx, eax       ; clear the current checking bit of the mask
       ; go to the handler of this (eax) bit
        cmp     eax, 10
        jae     .loop          ; eax=[10..31], ignored (event 11...32)

        cmp     eax, 3
        je      .loop          ; eax=3, ignored (event 4)

        cmp     eax, 4
        je      .FlagAutoReset  ; eax=4, retvals=eax+1 (event 5)

        cmp     eax, 5
        je      .mouse_check  ; eax=5, retvals=eax+1 (event 6)

        ja      .FlagAutoReset ; eax=[6..9], retvals=eax+1 (event 7...10)

        cmp     eax, 1
        jae     .BtKy          ; eax=[1,2],  retvals=eax+1 (event 2,3)
;--------------------------------------
align 4
.WndRedraw:                    ; eax=0, retval WndRedraw=1
        cmp     [edi-twdw+WDATA.fl_redraw], al;al==0
        jne     .result
        jmp     .loop
;--------------------------------------
align 4
.no_events:
        xor     eax, eax
        ret
;--------------------------------------
align 4
.mouse_check:    ; Mouse 5+1=6
        push    eax
        mov     eax, [TASK_BASE]
        mov     eax, [eax + TASKDATA.event_mask]
        test    eax, 0x80000000 ; bit 31: active/inactive filter f.40
        jz      @f
        pop     eax
        jmp     .FlagAutoReset
;--------------------------------------
align 4
@@:
; If the window is captured and moved by the user, then no mouse events!!!
        mov     al, [mouse.active_sys_window.action]
        and     al, WINDOW_MOVE_AND_RESIZE_FLAGS
        test    al, al
        pop     eax
        jnz     .loop
;--------------------------------------
align 4
.FlagAutoReset: ; retvals: BgrRedraw=5, IPC=7, Stack=8, Debug=9
        btr     [ebx+APPDATA.occurred_events], eax
        jnc     .loop
;--------------------------------------
align 4
.result:      ; retval = eax+1
        inc     eax
        ret
;--------------------------------------
align 4
.BtKy:
        movzx   edx, bh
        movzx   edx, word[WIN_STACK+edx*2]
        je      .Keys          ; eax=1, retval Keys=2
;--------------------------------------
align 4
.Buttons:                      ; eax=2, retval Buttons=3
        cmp     byte[BTN_COUNT], 0
        je      .loop          ; empty ???
        cmp     edx, [thread_count]
        jne     .loop          ; not Top ???
        mov     edx, [BTN_BUFF]
        shr     edx, 8
        cmp     edx, 0xFFFF    ;-ID for Minimize-Button of Form
        jne     .result
        mov     [window_minimize], 1
        call    wakeup_osloop
        dec     byte[BTN_COUNT]
        jmp     .loop
;--------------------------------------
align 4
.Keys:    ; eax==1
        cmp     edx, [thread_count]
        jne     @f             ; not Top ???
        cmp     [KEY_COUNT], al; al==1
        jae     .result        ; not empty ???
;--------------------------------------
align 4
@@:
        mov     edx, hotkey_buffer
;--------------------------------------
align 4
@@:
        cmp     [edx], bh      ; bh - slot for testing
        je      .result
        add     edx, 8
        cmp     edx, hotkey_buffer+120*8
        jb      @b
        jmp     .loop
;end.
;-----------------------------------------------------------------------------
