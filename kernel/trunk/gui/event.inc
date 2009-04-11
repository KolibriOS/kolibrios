;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

uglobal
align 4
  event_start   dd ?
  event_end     dd ?
  event_uid     dd 0
endg
EV_SPACE   = 512
FreeEvents = event_start-EVENT.fd    ; "виртуальный" event, используются только поля:
                                     ;  FreeEvents.fd=event_start и FreeEvents.bk=event_end
align 4
init_events:                                       ;; used from kernel.asm
        stdcall kernel_alloc,EV_SPACE*EVENT.size
        or      eax,eax
        jz      .fail
      ; eax - current event, ebx - previos event below
        mov     ecx,EV_SPACE         ; current - in allocated space
        mov     ebx,FreeEvents       ; previos - начало списка
        push    ebx                  ; оно же и конец потом будет
  @@:   mov     [ebx+EVENT.fd],eax
        mov     [eax+EVENT.bk],ebx
        mov     ebx,eax              ; previos <- current
        add     eax,EVENT.size       ; new current
        loop    @b
        pop     eax                  ; вот оно концом и стало
        mov     [ebx+EVENT.fd],eax
        mov     [eax+EVENT.bk],ebx
.fail:  ret

EVENT_WATCHED    equ 0x10000000 ;бит 28
EVENT_SIGNALED   equ 0x20000000 ;бит 29
MANUAL_RESET     equ 0x40000000 ;бит 30
MANUAL_DESTROY   equ 0x80000000 ;бит 31

align 4
create_event:                                      ;; EXPORT use
;info:
;   Переносим EVENT из списка FreeEvents в список ObjList текущего слота
;   EVENT.state устанавливаем из ecx, EVENT.code косвенно из esi (если esi<>0)
;param:
;   esi - event data
;   ecx - flags
;retval:
;   eax - event (=0 => fail)
;   edx - uid
;scratched: ebx,ecx,esi,edi
        mov     ebx,[current_slot]
        add     ebx,APP_OBJ_OFFSET
        mov     edx,[TASK_BASE]
        mov     edx,[edx+TASKDATA.pid]
        pushfd
        cli

set_event:                                         ;; INTERNAL use !!! don't use for Call
;info:
;   Берем новый event из FreeEvents, заполняем его поля, как указано в ecx,edx,esi
;   и устанавливаем в список, указанный в ebx.
;   Возвращаем сам event (в eax), и его uid (в edx)
;param:
;   ebx - start-chain "virtual" event for entry new event Right of him
;   ecx - flags      (copied to EVENT.state)
;   edx - pid        (copied to EVENT.pid)
;   esi - event data (copied to EVENT.code indirect, =0 => skip)
;retval:
;   eax - event (=0 => fail)
;   edx - uid
;scratched: ebx,ecx,esi,edi
        mov     eax,FreeEvents
        cmp     eax,[eax+EVENT.fd]
        jne     @f  ; not empty ???
        pushad
        call    init_events
        popad
        jz      RemoveEventTo.break ; POPF+RET
  @@:   mov     eax,[eax+EVENT.fd]
        mov     [eax+EVENT.magic],'EVNT'
        mov     [eax+EVENT.destroy],destroy_event.internal
        mov     [eax+EVENT.state],ecx
        mov     [eax+EVENT.pid],edx
        inc     [event_uid]
        Mov     [eax+EVENT.id],edx,[event_uid]
        or      esi,esi
        jz      RemoveEventTo
        lea     edi,[eax+EVENT.code]
        mov     ecx,EVENT.codesize/4
        cld
        rep     movsd

RemoveEventTo:                                     ;; INTERNAL use !!! don't use for Call
;param:
;   eax - указатель на event, КОТОРЫЙ вставляем
;   ebx - указатель на event, ПОСЛЕ которого вставляем
;scratched: ebx,ecx
        mov     ecx,eax              ; ecx=eax=Self,      ebx=NewLeft
        xchg    ecx,[ebx+EVENT.fd]   ; NewLeft.fd=Self,   ecx=NewRight
        cmp     eax,ecx              ; стоп, себе думаю...
        je      .break               ; - а не дурак ли я?
        mov     [ecx+EVENT.bk],eax   ; NewRight.bk=Self
        xchg    ebx,[eax+EVENT.bk]   ; Self.bk=NewLeft,   ebx=OldLeft
        xchg    ecx,[eax+EVENT.fd]   ; Self.fd=NewRight,  ecx=OldRight
        mov     [ebx+EVENT.fd],ecx   ; OldLeft.fd=OldRight
        mov     [ecx+EVENT.bk],ebx   ; OldRight.bk=OldLeft
.break: popfd
        ret

align 4
NotDummyTest:                                      ;; INTERNAL use (not returned for fail !!!)
        pop     edi
        call    DummyTest ; not returned for fail !!!
        mov     ebx,eax
        mov     eax,[ebx+EVENT.pid]
        push    edi
.small: ; криво как-то...
        pop     edi
        pushfd
        cli
        call    pid_to_slot ; saved all registers (eax - retval)
        shl     eax,8
        jz      RemoveEventTo.break ; POPF+RET
        jmp     edi ; штатный возврат

align 4
raise_event:                                       ;; EXPORT use
;info:
;   Устанавливаем данные EVENT.code
;   Если там флаг EVENT_SIGNALED уже активен - больше ничего
;   Иначе: этот флаг взводится, за исключением случая наличия флага EVENT_WATCHED в edx
;   В этом случае EVENT_SIGNALED взводится лишь при наличие EVENT_WATCHED в самом событии
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;   edx - flags
;   esi - event data (=0 => skip)
;scratched: ebx,ecx,esi,edi
        call    NotDummyTest ; not returned for fail !!!
        or      esi,esi
        jz      @f
        lea     edi,[ebx+EVENT.code]
        mov     ecx,EVENT.codesize/4
        cld
        rep     movsd
  @@:
        test    byte[ebx+EVENT.state+3], EVENT_SIGNALED shr 24
        jnz     RemoveEventTo.break  ; POPF+RET
        bt      edx, 28 ;EVENT_WATCHED
        jnc     @f
        test    byte[ebx+EVENT.state+3], EVENT_WATCHED  shr 24
        jz      RemoveEventTo.break  ; POPF+RET
  @@:
        or      byte[ebx+EVENT.state+3], EVENT_SIGNALED shr 24
        add     eax,SLOT_BASE+APP_EV_OFFSET
        xchg    eax,ebx
        jmp     RemoveEventTo

align 4
clear_event:                                       ;; EXPORT use
;info:
;
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;scratched: ebx,ecx
        call    NotDummyTest ; not returned for fail !!!
        add     eax,SLOT_BASE+APP_OBJ_OFFSET
        and     byte[ebx+EVENT.state+3], not((EVENT_SIGNALED+EVENT_WATCHED)shr 24)
        xchg    eax,ebx
        jmp     RemoveEventTo

align 4
send_event:                                        ;; EXPORT use
;info:
;   Создает новый EVENT (вытаскивает из списка FreeEvents) в списке EventList
;   целевого слота (eax=pid), с данными из esi косвенно, и state=EVENT_SIGNALED
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
        mov     edx,eax
        call    NotDummyTest.small ; not returned for fail !!!
        lea     ebx,[eax+SLOT_BASE+APP_EV_OFFSET]
        mov     ecx,EVENT_SIGNALED
        jmp     set_event

align 4
DummyTest:                                         ;; INTERNAL use (not returned for fail !!!)
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
        cmp     [eax+EVENT.magic],'EVNT'
        jne     @f
        cmp     [eax+EVENT.id],ebx
        je      .ret
  @@:   pop     eax
        xor     eax,eax
.ret:   ret


align 4
Wait_events:
        or      ebx,-1 ; infinite timeout
Wait_events_ex:
;info:
;   Ожидание "абстрактного" события через перевод слота в 5-ю позицию.
;   Абстрактность заключена в том, что факт события определяется функцией APPDATA.wait_test,
;   которая задается клиентом и может быть фактически любой.
;   Это позволяет shed-у надежно определить факт события, и не совершать "холостых" переключений,
;   предназначенных для разборок типа "свой/чужой" внутри задачи.
;param:
;   edx - wait_test, клиентская ф-я тестирования (адрес кода)
;   ecx - wait_param, дополнительный параметр, возможно необходимый для [wait_test]
;   ebx - wait_timeout
;retval:
;   eax - результат вызова [wait_test] (=0 => timeout)
;scratched: esi
        mov     esi,[current_slot]
        mov     [esi+APPDATA.wait_param],ecx
        pushad
        mov     ebx,esi;пока это вопрос, чего куды сувать..........
        pushfd  ; это следствие общей концепции: пусть ф-я тестирования имеет
        cli     ; право рассчитывать на закрытые прерывания, как при вызове из shed
        call    edx
        popfd
        mov     [esp+28],eax
        popad
        or      eax,eax
        jnz     @f   ;RET
        mov     [esi+APPDATA.wait_test],edx
        mov     [esi+APPDATA.wait_timeout],ebx
        Mov     [esi+APPDATA.wait_begin],eax,[timer_ticks]
        mov     eax,[TASK_BASE]
        mov     [eax+TASKDATA.state], 5
        call    change_task
        mov     eax,[esi+APPDATA.wait_param]
  @@:   ret

align 4
wait_event:                                        ;; EXPORT use
;info:
;   Ожидание флага EVENT_SIGNALED в совершенно конкретном Event
;   (устанавливаемого, надо полагать, через raise_event)
;   При активном флаге MANUAL_RESET - больше ничего
;   Иначе: флаги EVENT_SIGNALED и EVENT_WATCHED у полученного события сбрасываются,
;   и, при активном MANUAL_DESTROY - перемещается в список ObjList текущего слота,
;   а при не активном - уничтожается штатно (destroy_event.internal)
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;scratched: ecx,edx,esi
        call    DummyTest
        mov     ecx,eax              ; wait_param
        mov     edx, get_event_alone ; wait_test
        call    Wait_events          ; timeout ignored
        jmp     wait_finish

align 4
get_event_ex:                                      ;; f68:14
;info:
;   Ожидание любого события в очереди EventList текущего слота
;   Данные события code - копируются в память приложения (косвенно по edi)
;   При активном флаге MANUAL_RESET - больше ничего
;   Иначе: флаги EVENT_SIGNALED и EVENT_WATCHED у полученного события сбрасываются,
;   и, при активном MANUAL_DESTROY - перемещается в список ObjList текущего слота,
;   а при не активном - уничтожается штатно (destroy_event.internal)
;param:
;   edi - адрес в коде приложения для копирования данных из EVENT.code
;retval:
;   eax - собственно EVENT (будем называть это его хэндлом)
;scratched: ebx,ecx,edx,esi,edi
        mov     edx, get_event_queue ; wait_test
        call    Wait_events          ; timeout ignored
        lea     esi,[eax+EVENT.code]
        mov     ecx,EVENT.codesize/4
        cld
        rep     movsd
        mov     [edi-EVENT.codesize+2],cl ;clear priority field
wait_finish:
        test    byte[eax+EVENT.state+3], MANUAL_RESET shr 24
        jnz     get_event_queue.ret  ; RET
        and     byte[eax+EVENT.state+3], not((EVENT_SIGNALED+EVENT_WATCHED)shr 24)
        test    byte[eax+EVENT.state+3], MANUAL_DESTROY shr 24
        jz      destroy_event.internal
        mov     ebx,[current_slot]
        add     ebx,APP_OBJ_OFFSET
        pushfd
        cli
        jmp     RemoveEventTo

align 4
destroy_event:                                     ;; EXPORT use
;info:
;   Переносим EVENT в список FreeEvents, чистим поля magic,destroy,pid,id
;param:
;   eax - event
;   ebx - uid (for Dummy testing)
;retval:
;   eax - адрес объекта EVENT (=0 => fail)
;scratched: ebx,ecx
        call    DummyTest ; not returned for fail !!!
.internal:
        xor     ecx,ecx   ; clear common header
        pushfd
        cli
        mov     [eax+EVENT.magic],ecx
        mov     [eax+EVENT.destroy],ecx
        mov     [eax+EVENT.pid],ecx
        mov     [eax+EVENT.id],ecx
        mov     ebx,FreeEvents
        jmp     RemoveEventTo

align 4
get_event_queue:
;info:
;   клиентская ф-я тестирования для get_event_ex
;warning:
;  -don't use [TASK_BASE],[current_slot],[CURRENT_TASK] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - адрес APPDATA слота тестирования
;retval:
;   eax - адрес объекта EVENT (=0 => fail)
        add     ebx,APP_EV_OFFSET
        mov     eax,[ebx+APPOBJ.bk]  ; выбираем с конца, по принципу FIFO
        cmp     eax,ebx  ; empty ???
        je      get_event_alone.ret0
.ret:   ret

align 4
get_event_alone:
;info:
;   клиентская ф-я тестирования для wait_event
;warning:
;  -don't use [TASK_BASE],[current_slot],[CURRENT_TASK] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - адрес APPDATA слота тестирования
;retval:
;   eax - адрес объекта EVENT (=0 => fail)
        mov     eax,[ebx+APPDATA.wait_param]
        test    byte[eax+EVENT.state+3], EVENT_SIGNALED shr 24
        jnz     .ret
        or      byte[eax+EVENT.state+3], EVENT_WATCHED shr 24
.ret0:  xor     eax,eax ; NO event!!!
.ret:   ret

align 4
sys_sendwindowmsg:                                 ;; f72
        dec     ebx
        jnz     .ret ;subfunction==1 ?
        pushfd
        cli
        sub     ecx,2
        je      .sendkey
        loop    .retf
.sendbtn:
        cmp     byte[BTN_COUNT],1
        jae     .result ;overflow
        inc     byte[BTN_COUNT]
        mov     [BTN_BUFF],edx
        jmp     .result
.sendkey:
        movzx   eax,byte[KEY_COUNT]
        cmp     al,120
        jae     .result ;overflow
        inc     byte[KEY_COUNT]
        mov     [KEY_COUNT+1+eax],dl
.result:
        setae   byte[esp+36] ;считаем, что исходно: dword[esp+36]==72
.retf:  popfd
.ret:   ret

align 4
sys_getevent:                                      ;; f11
        mov     ebx,[current_slot] ;пока это вопрос, чего куды сувать..........
        pushfd  ; это следствие общей концепции: пусть ф-я тестирования имеет
        cli     ; право рассчитывать на закрытые прерывания, как при вызове из shed
        call    get_event_for_app
        popfd
        mov     [esp+32],eax
        ret

align 4
sys_waitforevent:                                  ;; f10
        or      ebx,-1 ; infinite timeout
sys_wait_event_timeout:                            ;; f23
        mov     edx,get_event_for_app ; wait_test
        call    Wait_events_ex        ; ebx - timeout
        mov     [esp+32],eax
        ret

align 4
get_event_for_app:                                 ;; used from f10,f11,f23
;info:
;   клиентская ф-я тестирования для приложений (f10,f23)
;warning:
;  -don't use [TASK_BASE],[current_slot],[CURRENT_TASK] - it is not for your slot
;  -may be assumed, that interrupt are disabled
;  -it is not restriction for scratched registers
;param:
;   ebx - адрес APPDATA слота тестирования
;retval:
;   eax - номер события (=0 => no events)
        movzx   edi,bh                ; bh  is assumed as [CURRENT_TASK]
        shl     edi,5
        add     edi,CURRENT_TASK      ; edi is assumed as [TASK_BASE]
        mov     ecx,[edi+TASKDATA.event_mask]
.loop: ; пока не исчерпаем все биты маски
        bsr     eax,ecx        ; находим ненулевой бит маски (31 -> 0)
        jz      .no_events     ; исчерпали все биты маски, но ничего не нашли ???
        btr     ecx,eax        ; сбрасываем проверяемый бит маски
       ; переходим на обработчик этого (eax) бита
        cmp     eax,16
        jae     .IRQ           ; eax=[16..31]=retvals, events irq0..irq15
        cmp     eax,9
        jae     .loop          ; eax=[9..15], ignored
        cmp     eax,3
        je      .loop          ; eax=3, ignored
        ja      .FlagAutoReset ; eax=[4..8], retvals=eax+1
        cmp     eax,1
        jae     .BtKy          ; eax=[1,2],  retvals=eax+1
.WndRedraw:                    ; eax=0, retval WndRedraw=1
        cmp     [edi-twdw+WDATA.fl_redraw],al ;al==0
        jne     .result
        jmp     .loop
  .no_events:
        xor     eax,eax
        ret
.IRQ:
;TODO: сделать так же, как и для FlagAutoReset (BgrRedraw,Mouse,IPC,Stack,Debug)
        mov     edx,[irq_owner+eax*4-64] ; eax==16+irq
        cmp     edx,[edi+TASKDATA.pid]
        jne     .loop
        mov     edx,eax
        shl     edx,12
        cmp     dword[IRQ_SAVE+edx-0x10000],0 ; edx==(16+irq)*0x1000
        je      .loop          ; empty ???
        ret     ; retval = eax
.FlagAutoReset: ; retvals: BgrRedraw=5, Mouse=6, IPC=7, Stack=8, Debug=9
        btr     [ebx+APPDATA.event_mask],eax
        jnc     .loop
  .result:      ; retval = eax+1
        inc     eax
        ret
  .BtKy:
        movzx   edx,bh
        movzx   edx, word[WIN_STACK+edx*2]
        je      .Keys          ; eax=1, retval Keys=2
.Buttons:                      ; eax=2, retval Buttons=3
        cmp     byte[BTN_COUNT],0
        je      .loop          ; empty ???
        cmp     edx,[TASK_COUNT]
        jne     .loop          ; not Top ???
        cmp     dword[BTN_BUFF],0xFFFF ;-ID for Minimize-Button of Form
        jne     .result
        mov     [window_minimize],1
        dec     byte[BTN_COUNT]
        jmp     .loop
.Keys:    ; eax==1
        cmp     edx,[TASK_COUNT]
        jne     @f             ; not Top ???
        cmp     [KEY_COUNT],al ; al==1
        jae     .result        ; not empty ???
  @@:   mov     edx, hotkey_buffer
  @@:   cmp     [edx],bh       ; bh - slot for testing
        je      .result
        add     edx,8
        cmp     edx, hotkey_buffer+120*8
        jb      @b
        jmp     .loop
;end.
