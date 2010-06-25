;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; diamond, 2006
sys_debug_services:
        cmp     ebx, 9
        ja      @f
        jmp     dword [sys_debug_services_table+ebx*4]
@@:     ret
iglobal
align 4
sys_debug_services_table:
        dd      debug_set_event_data
        dd      debug_getcontext
        dd      debug_setcontext
        dd      debug_detach
        dd      debug_suspend
        dd      debug_resume
        dd      debug_read_process_memory
        dd      debug_write_process_memory
        dd      debug_terminate
        dd      debug_set_drx
endg
debug_set_event_data:
; in: ecx = pointer
; destroys eax
        mov     eax, [current_slot]
        mov     [eax+APPDATA.dbg_event_mem], ecx
        ret

get_debuggee_slot:
; in: ecx=PID
; out: CF=1 if error
;      CF=0 and eax=slot*0x20 if ok
; out: interrupts disabled
        cli
        mov     eax, ecx
        call    pid_to_slot
        test    eax, eax
        jz      .ret_bad
        shl     eax, 5
        push    ebx
        mov     ebx, [CURRENT_TASK]
        cmp     [SLOT_BASE+eax*8+APPDATA.debugger_slot], ebx
        pop     ebx
        jnz     .ret_bad
;       clc     ; automatically
        ret
.ret_bad:
        stc
        ret

debug_detach:
; in: ecx=pid
; destroys eax,ebx
        call    get_debuggee_slot
        jc      .ret
        and     dword [eax*8+SLOT_BASE+APPDATA.debugger_slot], 0
        call    do_resume
.ret:
        sti
        ret

debug_terminate:
; in: ecx=pid
        call    get_debuggee_slot
        jc      debug_detach.ret
        mov     ecx, eax
        shr     ecx, 5
;        push    2
;        pop     ebx
        mov	edx,esi
        jmp     sysfn_terminate

debug_suspend:
; in: ecx=pid
; destroys eax,ecx
        cli
        mov     eax, ecx
        call    pid_to_slot
        shl     eax, 5
        jz      .ret
        mov     cl, [CURRENT_TASK+eax+TASKDATA.state] ; process state
        test    cl, cl
        jz      .1
        cmp     cl, 5
        jnz     .ret
        mov     cl, 2
.2:     mov     [CURRENT_TASK+eax+TASKDATA.state], cl
.ret:
        sti
        ret
.1:
        inc     ecx
        jmp     .2

do_resume:
        mov     cl, [CURRENT_TASK+eax+TASKDATA.state]
        cmp     cl, 1
        jz      .1
        cmp     cl, 2
        jnz     .ret
        mov     cl, 5
.2:     mov     [CURRENT_TASK+eax+TASKDATA.state], cl
.ret:   ret
.1:     dec     ecx
        jmp     .2

debug_resume:
; in: ecx=pid
; destroys eax,ebx
        cli
        mov     eax, ecx
        call    pid_to_slot
        shl     eax, 5
        jz      .ret
        call    do_resume
.ret:   sti
        ret

debug_getcontext:
; in:
; ecx=pid
; edx=sizeof(CONTEXT)
; esi->CONTEXT
; destroys eax,ecx,edx,esi,edi
        cmp     edx, 28h
        jnz     .ret
;        push    ecx
;        mov     ecx, esi
        call    check_region
;        pop     ecx
        dec     eax
        jnz     .ret
        call    get_debuggee_slot
        jc      .ret
        mov     edi, esi
        mov eax, [eax*8+SLOT_BASE+APPDATA.pl0_stack]
        lea esi, [eax+RING0_STACK_SIZE]

.ring0:
; note that following code assumes that all interrupt/exception handlers
; saves ring-3 context by pushad in this order
; top of ring0 stack: ring3 stack ptr (ss+esp), iret data (cs+eip+eflags), pushad
        sub     esi, 8+12+20h
        lodsd                     ;edi
        mov     [edi+24h], eax
        lodsd                     ;esi
        mov     [edi+20h], eax
        lodsd                     ; ebp
        mov     [edi+1Ch], eax
        lodsd                     ;esp
        lodsd                     ;ebx
        mov     [edi+14h], eax
        lodsd                     ;edx
        mov     [edi+10h], eax
        lodsd                     ;ecx
        mov     [edi+0Ch], eax
        lodsd                     ;eax
        mov     [edi+8], eax
        lodsd                     ;eip
        mov     [edi], eax
        lodsd                     ;cs
        lodsd                     ;eflags
        mov     [edi+4], eax
        lodsd                     ;esp
        mov     [edi+18h], eax
.ret:
        sti
        ret

debug_setcontext:
; in:
; ecx=pid
; edx=sizeof(CONTEXT)
; esi->CONTEXT
; destroys eax,ecx,edx,esi,edi
        cmp     edx, 28h
        jnz     .ret
;        push    ebx
;        mov     ebx, edx
        call    check_region
;        pop     ebx
        dec     eax
        jnz     .ret
        call    get_debuggee_slot
        jc      .stiret
;        mov     esi, edx
        mov eax, [eax*8+SLOT_BASE+APPDATA.pl0_stack]
        lea edi, [eax+RING0_STACK_SIZE]

.ring0:
        sub     edi, 8+12+20h
        mov     eax, [esi+24h]    ;edi
        stosd
        mov     eax, [esi+20h]    ;esi
        stosd
        mov     eax, [esi+1Ch]    ;ebp
        stosd
        scasd
        mov     eax, [esi+14h]    ;ebx
        stosd
        mov     eax, [esi+10h]    ;edx
        stosd
        mov     eax, [esi+0Ch]    ;ecx
        stosd
        mov     eax, [esi+8]      ;eax
        stosd
        mov     eax, [esi]        ;eip
        stosd
        scasd
        mov     eax, [esi+4]      ;eflags
        stosd
        mov     eax, [esi+18h]    ;esp
        stosd
.stiret:
        sti
.ret:
        ret

debug_set_drx:
        call    get_debuggee_slot
        jc      .errret
        mov     ebp, eax
        lea     eax, [eax*8+SLOT_BASE+APPDATA.dbg_regs]
; [eax]=dr0, [eax+4]=dr1, [eax+8]=dr2, [eax+C]=dr3
; [eax+10]=dr7
        cmp     esi, OS_BASE
        jae      .errret
        cmp     dl, 3
        ja      .errret
        mov     ecx, dr7
;fix me
	xchg	ecx,edx
        shr     edx, cl
        shr     edx, cl
	xchg	ecx,edx

        test    ecx, 2          ; bit 1+2*index = G0..G3, global break enable
        jnz     .errret2
        test    dh, dh
        jns     .new
; clear breakpoint
        movzx   edx, dl
        add     edx, edx
        and     dword [eax+edx*2], 0    ; clear DR<i>
        btr     dword [eax+10h], edx    ; clear L<i> bit
        test    byte [eax+10h], 55h
        jnz     .okret
;        imul    eax, ebp, tss_step/32
;        and     byte [eax + tss_data + TSS._trap], not 1
        and [ebp*8 + SLOT_BASE+APPDATA.dbg_state], not 1
.okret:
        and     dword [esp+32], 0
        sti
        ret
.errret:
        sti
        mov     dword [esp+32], 1
        ret
.errret2:
        sti
        mov     dword [esp+32], 2
        ret
.new:
; add new breakpoint
; dl=index; dh=flags; esi=address
        test    dh, 0xF0
        jnz     .errret
        mov     cl, dh
        and     cl, 3
        cmp     cl, 2
        jz      .errret
        mov     cl, dh
        shr     cl, 2
        cmp     cl, 2
        jz      .errret

        mov	ebx,esi
        test    bl, dl

        jnz     .errret
        or      byte [eax+10h+1], 3     ; set GE and LE flags

        movzx   edx, dh
        movzx   ecx, dl
        add     ecx, ecx
        bts     dword [eax+10h], ecx    ; set L<i> flag
        add     ecx, ecx
        mov     [eax+ecx], ebx;esi      ; set DR<i>
        shl     edx, cl
        mov     ebx, 0xF
        shl     ebx, cl
        not     ebx
        and     [eax+10h+2], bx
        or      [eax+10h+2], dx         ; set R/W and LEN fields
;        imul    eax, ebp, tss_step/32
;        or      byte [eax + tss_data + TSS._trap], 1
        or [ebp*8 + SLOT_BASE+APPDATA.dbg_state], 1
        jmp     .okret

debug_read_process_memory:
; in:
; ecx=pid
; edx=length
; edi->buffer in debugger
; esi=address in debuggee
; out: [esp+36]=sizeof(read)
; destroys all
;        push    ebx
;        mov     ebx, esi
        call    check_region
;        pop     ebx
        dec     eax
        jnz     .err
        call    get_debuggee_slot
        jc      .err
        shr     eax, 5
;        mov     ebx, esi
        call    read_process_memory
        sti
        mov     dword [esp+32], eax
        ret
.err:
        or      dword [esp+32], -1
        ret

debug_write_process_memory:
; in:
; ecx=pid
; edx=length
; edi->buffer in debugger
; esi=address in debuggee
; out: [esp+36]=sizeof(write)
; destroys all
;        push    ebx
;        mov     ebx, esi
        call    check_region
;        pop     ebx
        dec     eax
        jnz     debug_read_process_memory.err
        call    get_debuggee_slot
        jc      debug_read_process_memory.err
        shr     eax, 5
;        mov     ebx, esi
        call    write_process_memory
        sti
        mov     [esp+32], eax
        ret

debugger_notify:
; in: eax=debugger slot
;     ecx=size of debug message
;     [esp+4]..[esp+4+ecx]=message
; interrupts must be disabled!
; destroys all general registers
; interrupts remain disabled
        xchg    ebp, eax
        mov     edi, [timer_ticks]
        add     edi, 500        ; 5 sec timeout
.1:
        mov     eax, ebp
        shl     eax, 8
        mov     edx, [SLOT_BASE+eax+APPDATA.dbg_event_mem]
        test    edx, edx
        jz      .ret
; read buffer header
        push    ecx
        push    eax
        push    eax
        mov     eax, ebp
        mov     ebx, esp
        mov     ecx, 8
        call    read_process_memory
        cmp     eax, ecx
        jz      @f
        add     esp, 12
        jmp     .ret
@@:
        cmp     dword [ebx], 0
        jg      @f
.2:
        pop     ecx
        pop     ecx
        pop     ecx
        cmp     dword [CURRENT_TASK], 1
        jnz     .notos
        cmp     [timer_ticks], edi
        jae     .ret
.notos:
        sti
        call    change_task
        cli
        jmp     .1
@@:
        mov     ecx, [ebx+8]
        add     ecx, [ebx+4]
        cmp     ecx, [ebx]
        ja      .2
; advance buffer position
        push    ecx
        mov     ecx, 4
        sub     ebx, ecx
        mov     eax, ebp
        add     edx, ecx
        call    write_process_memory
        pop     eax
; write message
        mov     eax, ebp
        add     edx, ecx
        add     edx, [ebx+8]
        add     ebx, 20
        pop     ecx
        pop     ecx
        pop     ecx
        call    write_process_memory
; new debug event
        mov     eax, ebp
        shl     eax, 8
        or      byte [SLOT_BASE+eax+APPDATA.event_mask+1], 1      ; set flag 100h
.ret:
        ret
