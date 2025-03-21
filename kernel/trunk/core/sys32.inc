;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


struct EXCEPT_STACK
    RegEIP     dd ?
    ExcCode    dd ? ; only exception 12 overflow stack
    OldESP     dd ?
    RegCR2     dd ?
    LockAccess dd ?
ends

align 4 ;3A08
build_interrupt_table:
        mov     edi, idts
        mov     esi, sys_int
        mov     ecx, 0x40
        mov     eax, (10001110b shl 24) + os_code
  @@:
        movsw   ; low word of code-entry
        stosd   ; interrupt gate type : os_code selector
        movsw   ; high word of code-entry
        loop    @b
        movsd   ; copy low  dword of trap gate for int 0x40
        movsd   ; copy high dword of trap gate for int 0x40
        mov     ecx, 23
        mov     eax, (10001110b shl 24) + os_code
  @@:
        movsw   ; low word of code-entry
        stosd   ; interrupt gate type : os_code selector
        movsw   ; high word of code-entry
        loop    @b
        lidt    [esi]
        ret

iglobal
  align 4
  sys_int:
    ; exception handlers addresses (for interrupt gate construction)
        dd      e0,e1,e2,e3,e4,e5,e6,except_7 ; SEE: core/fpu.inc
        dd      e8,e9,e10,e11,e12,e13,page_fault_exc,e15
        dd      e16, e17,e18, e19
        times   12 dd unknown_interrupt ;int_20..int_31

    ; interrupt handlers addresses (for interrupt gate construction)
        ; 0x20+ are IRQ handlers
        dd irq0
        rept 12 irqn:1  \{dd irq_serv.irq_\#irqn\}
        dd irqD
        rept 18 irqn:14 \{dd irq_serv.irq_\#irqn\}

        ; int_0x40 gate trap (for directly copied)
        dw i40 and 0xFFFF, os_code, 11101111b shl 8, i40 shr 16

        rept 23 irqn:33 \{dd irq_serv.irq_\#irqn\}

  idtreg: ; data for LIDT instruction (!!! must be immediately below sys_int data)
        dw      2*($-sys_int-4)-1
        dd      idts ; 0x8000B100
        dw      0    ; alignment

  msg_fault_sel dd  msg_exc_8,msg_exc_u,msg_exc_a,msg_exc_b
                dd  msg_exc_c,msg_exc_d,msg_exc_e,msg_exc_u
                dd  msg_exc_u,msg_exc_11

  msg_exc_8     db "Double fault", 0
  msg_exc_u     db "Undefined Exception", 0
  msg_exc_a     db "Invalid TSS", 0
  msg_exc_b     db "Segment not present", 0
  msg_exc_c     db "Stack fault", 0
  msg_exc_d     db "General protection fault", 0
  msg_exc_e     db "Page fault", 0
  msg_exc_11    db "Alignment Check", 0

  if lang eq es_ES
    include 'core/sys32-sp.inc'
  else
    msg_sel_ker   db "kernel", 0
    msg_sel_app   db "application", 0
  end if

endg

macro save_ring3_context {
        pushad
}
macro restore_ring3_context {
        popad
}
macro exc_wo_code [num] {
  e#num :
        save_ring3_context
        mov     bl, num
        jmp     exc_c
} exc_wo_code   0,1,2,3,4,5,6,15,16,19

macro exc_w_code [num] {
  e#num :
        add     esp, 4
        save_ring3_context
        mov     bl, num
        jmp     exc_c
} exc_w_code    8,9,10,11,12,13,17,18


uglobal
  pf_err_code   dd ?
endg

page_fault_exc:                   ; foolproof: selectors are clobbered ...
        pop     [ss:pf_err_code]  ; actually, until the next #PF

        cmp     edi, CONTROL_EXCEPTION ; equ 'EXPT'
        jne     .no_ctrl_exc
        bt      dword [esp], 31
        jc      .setret
        test    esi, esi
        jl      .no_ctrl_exc
.setret:
        mov     [esp], esi
        iret

.no_ctrl_exc:
        save_ring3_context
        mov     bl, 14

exc_c:                            ; exceptions (all but 7th - #NM)
 ; stack frame when exception/interrupt from ring3 + pushad (i.e right here)
  reg_ss        equ esp+0x30
  reg_esp3      equ esp+0x2C
  reg_eflags    equ esp+0x28
  reg_cs3       equ esp+0x24
  reg_eip       equ esp+0x20
 ; this if frame from pushad
  reg_eax       equ esp+0x1C
  reg_ecx       equ esp+0x18
  reg_edx       equ esp+0x14
  reg_ebx       equ esp+0x10
  reg_esp0      equ esp+0x0C
  reg_ebp       equ esp+0x08
  reg_esi       equ esp+0x04
  reg_edi       equ esp+0x00

        mov     ax, app_data       ; exception
        mov     ds, ax             ; load proper values
        mov     es, ax             ; to registers
        cld                        ; clear the direction flag
        movzx   ebx, bl
; redirect to V86 manager? (EFLAGS & 0x20000) != 0?
        test    byte[reg_eflags+2], 2
        jnz     v86_exc_c
        cmp     bl, 14             ; #PF
        jne     @f
        call    page_fault_handler ; SEE: core/memory.inc
  @@:
        mov     esi, [current_slot]
        btr     [esi + APPDATA.except_mask], ebx
        jnc     @f
        mov     eax, [esi + APPDATA.exc_handler]
        test    eax, eax
        jnz     IRetToUserHook
  @@:
        cli
        mov     eax, [esi + APPDATA.debugger_slot]
        test    eax, eax
        jnz     .debug
; not debuggee => say error and terminate
        call    show_error_parameters  ; this function output in edx = current_slot
        sti
        mov     [edx + APPDATA.state], TSTATE_TERMINATING
        call    wakeup_osloop
        call    change_task
; If we're here, then the main OS thread has crashed before initializing IDLE thread.
; Or they both have crashed. Anyway, things are hopelessly broken.
        hlt
        jmp     $-1
.debug:
; we are debugged process, notify debugger and suspend ourself
; eax=debugger PID
        mov     ecx, 1          ; debug_message code=other_exception
        cmp     bl, 1           ; #DB
        jne     .notify         ; notify debugger and suspend ourself
        mov     ebx, dr6        ; debug_message data=DR6_image
        xor     edx, edx
        mov     dr6, edx
        mov     edx, dr7
        mov     cl, not 8
  .l1:
        shl     dl, 2
        jc      @f
        and     bl, cl
  @@:
        sar     cl, 1
        jc      .l1
        mov     cl, 3           ; debug_message code=debug_exception
.notify:
        push    ebx             ; debug_message data
        mov     ebx, [current_slot]
        push    [ebx + APPDATA.tid] ; PID
        push    ecx             ; debug_message code ((here: ecx==1/3))
        mov     cl, 12          ; debug_message size
        call    debugger_notify ;; only ONE using, inline ??? SEE: core/debug.inc
        add     esp, 12
        mov     edx, [current_slot]
        mov     [edx + APPDATA.state], TSTATE_RUN_SUSPENDED
        call    change_task     ; SEE: core/shed.inc
        restore_ring3_context
        iretd

IRetToUserHook:
        cmp     ebx, 12
        je      .ex_stack
        cmp     ebx, 14
        jne     .nostack
        mov     ecx, cr2
        sub     ecx, [reg_esp3]
        jg      .nostack
        add     ecx, 1000h
        jl      .nostack
.ex_stack:
        mov     ecx, [esi+APPDATA.exc_reserve_stack]
        test    ecx, ecx
        jle     .nostack
        xchg    edi, eax
        sub     ecx, sizeof.EXCEPT_STACK
        push    ebx
        push    1
        pop     ebx
.lock:
        lock bts [ecx+EXCEPT_STACK.LockAccess], 0
        jnc     .lock1
        call    delay_hs_unprotected
        jmp     .lock
.lock1:
        pop     ebx
        cmp     ebx, 14
        jne     .ex12
        btr     [esi+APPDATA.except_mask], 12
        jc      .ex_stack1
        xchg    eax, edi
        jmp     .nostack
.ex_stack1:
        bts     [esi+APPDATA.except_mask], ebx
        dec     ebx
        dec     ebx
.ex12:
        mov     [ecx+EXCEPT_STACK.ExcCode], ebx
        mov     eax, ecx
        xchg    [reg_esp3], eax
        mov     [ecx+EXCEPT_STACK.OldESP], eax
        mov     eax, cr2
        mov     [ecx+EXCEPT_STACK.RegCR2], eax
        xchg    edi, [reg_eip]
        mov     [ecx+EXCEPT_STACK.RegEIP], edi
        jmp     .end
.nostack:
        xchg    eax, [reg_eip]
        sub     dword[reg_esp3], 8
        mov     edi, [reg_esp3]
        stosd
        mov     [edi], ebx
.end:
        restore_ring3_context
; simply return control to interrupted process
unknown_interrupt:
        iretd

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; bl - error vector
show_error_parameters:
        cmp     bl, 0x06
        jnz     .no_ud
        push    ebx
        mov     ebx, ud_user_message
        mov     ebp, notifyapp
        call    fs_execute_from_sysdir_param
        pop     ebx
.no_ud:
        mov     edx, [current_slot];not scratched below
        if lang eq es_ES
        DEBUGF  1, "K : Proceso - terminado forzado PID: %x [%s]\n", [edx + APPDATA.tid], [current_slot]
        else
        DEBUGF  1, "K : Process - forced terminate PID: %x [%s]\n", [edx + APPDATA.tid], [current_slot]
        end if
        cmp     bl, 0x08
        jb      .l0
        cmp     bl, 0x11
        jbe     .l1
  .l0:
        mov     bl, 0x09
  .l1:
        mov     eax, [msg_fault_sel+ebx*4 - 0x08*4]
        DEBUGF  1, "K : %s\n", eax
        mov     eax, [reg_cs3+4]
        mov     edi, msg_sel_app
        mov     ebx, [reg_esp3+4]
        cmp     eax, app_code
        je      @f
        mov     edi, msg_sel_ker
        mov     ebx, [reg_esp0+4]
    @@:
        DEBUGF  1, "K : EAX : %x EBX : %x ECX : %x\n", [reg_eax+4], [reg_ebx+4], [reg_ecx+4]
        DEBUGF  1, "K : EDX : %x ESI : %x EDI : %x\n", [reg_edx+4], [reg_esi+4], [reg_edi+4]
        DEBUGF  1, "K : EBP : %x EIP : %x ESP : %x\n", [reg_ebp+4], [reg_eip+4], ebx
        DEBUGF  1, "K : Flags : %x CS : %x (%s)\n", [reg_eflags+4], eax, edi

        DEBUGF  1, "K : Stack dump:\n"
        push    eax ebx ecx edx
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, "K : [ESP+00]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+04]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+08]: %x\n",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, "K : [ESP+12]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+16]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+20]: %x\n",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, "K : [ESP+24]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+28]: %x",[ebx]
        add     ebx, 4
        call    .check_ESP
        test    eax, eax
        jnz     .error_ESP
        DEBUGF  1, " [ESP+32]: %x\n",[ebx]
        pop     edx ecx ebx eax
        ret
.error_ESP:
        pop     edx ecx ebx eax
        DEBUGF  1, "\n"
        DEBUGF  1, "K : Unexpected end of the stack\n"
        ret
;--------------------------------------
.check_ESP:
        push    ebx
        shr     ebx, 12
        mov     ecx, ebx
        shr     ecx, 10
        mov     edx, [master_tab + ecx*4]
        test    edx, PG_READ
        jz      .fail             ; page table is not created
                                  ; incorrect address in the program

        mov     eax, [page_tabs + ebx*4]
        test    eax, 2
        jz      .fail             ; address not reserved for use. error

        pop     ebx
        xor     eax, eax
        ret

.fail:
        pop     ebx
        xor     eax, eax
        dec     eax
        ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  restore  reg_ss
  restore  reg_esp3
  restore  reg_eflags
  restore  reg_cs
  restore  reg_eip
  restore  reg_eax
  restore  reg_ecx
  restore  reg_edx
  restore  reg_ebx
  restore  reg_esp0
  restore  reg_ebp
  restore  reg_esi
  restore  reg_edi


align 4
lock_application_table:
        push    eax ecx edx
        mov     ecx, application_table_mutex
        call    mutex_lock

        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.tid]

        mov     [application_table_owner], eax

        pop     edx ecx eax

        ret

align 4
unlock_application_table:
        push    eax ecx edx

        mov     [application_table_owner], 0
        mov     ecx, application_table_mutex
        call    mutex_unlock

        pop     edx ecx eax

        ret

; sysfn 64 implementation
align 4
sys_resize_app_memory:
; in:   eax = 64 - function number
;       ebx = 1 - number of its only subfunction
;       ecx = new amount of memory
; out:
;       eax = 0 - success
;       eax = 1 - out of memory

;        cmp    eax,1
        dec     ebx
        jnz     .no_application_mem_resize

        mov     eax, [pg_data.pages_free]
        shl     eax, 12
        cmp     eax, ecx
        jae     @f

        xor     eax, eax
        inc     eax
        jmp     .store_result
@@:
        stdcall new_mem_resize, ecx
.store_result:
        mov     [esp + SYSCALL_STACK.eax], eax
.no_application_mem_resize:
        ret

iglobal
;  process_terminating  db 'K : Process - terminating',13,10,0
;  process_terminated   db 'K : Process - done',13,10,0
  msg_obj_destroy       db 'K : destroy app object',13,10,0
endg

; param
;  esi= slot

align 4
terminate: ; terminate application
destroy_thread:

        .slot     equ esp+4             ;locals
        .process  equ esp               ;ptr to parent process


        push    esi        ;save .slot

        shl     esi, BSF sizeof.APPDATA
        mov     edx, [SLOT_BASE + esi + APPDATA.process]
        test    edx, edx
        jnz     @F
        mov     [SLOT_BASE + esi + APPDATA.state], TSTATE_FREE
        pop     esi
        ret
@@:
        push    edx                     ;save .process
        lea     edx, [SLOT_BASE + esi]
        call    scheduler_remove_thread
        call    lock_application_table

; if the process is in V86 mode...
        mov     eax, [.slot]
        shl     eax, BSF sizeof.APPDATA
        mov     esi, [SLOT_BASE + eax + APPDATA.pl0_stack]
        add     esi, RING0_STACK_SIZE
        cmp     [SLOT_BASE + eax + APPDATA.saved_esp0], esi
        jz      .nov86
; ...it has page directory for V86 mode
        mov     esi, [SLOT_BASE + eax + APPDATA.saved_esp0]
        mov     ecx, [esi+4]
        mov     [SLOT_BASE + eax + APPDATA.process], ecx
; ...and I/O permission map for V86 mode
        mov     ecx, [esi+12]
        mov     [SLOT_BASE + eax + APPDATA.io_map], ecx
        mov     ecx, [esi+8]
        mov     [SLOT_BASE + eax + APPDATA.io_map+4], ecx
.nov86:
; destroy per-thread kernel objects
        mov     esi, [.slot]
        shl     esi, BSF sizeof.APPDATA
        add     esi, SLOT_BASE + APP_OBJ_OFFSET
@@:
        mov     eax, [esi + APPOBJ.fd]
        test    eax, eax
        jz      @F

        cmp     eax, esi
        je      @F

        push    esi
        call    [eax + APPOBJ.destroy]
           DEBUGF 1,"%s",msg_obj_destroy
        pop     esi
        jmp     @B
@@:
        mov     esi, [.slot]
        cmp     [fpu_owner], esi ; if user fpu last -> fpu user = 2
        jne     @F

        mov     [fpu_owner], 2
        mov     eax, [SLOT_BASE + sizeof.APPDATA*2 + APPDATA.fpu_state]
        clts
        bt      [cpu_caps], CAPS_SSE
        jnc     .no_SSE
        fxrstor [eax]
        jmp     @F
.no_SSE:
        fnclex
        frstor  [eax]
@@:

        mov     [KEY_COUNT], byte 0    ; empty keyboard buffer
        mov     [BTN_COUNT], byte 0    ; empty button buffer


; remove defined hotkeys
        mov     eax, hotkey_list
.loop:
        cmp     [eax+8], esi
        jnz     .cont
        mov     ecx, [eax]
        jecxz   @f
        push    dword [eax+12]
        pop     dword [ecx+12]
@@:
        mov     ecx, [eax+12]
        push    dword [eax]
        pop     dword [ecx]
        xor     ecx, ecx
        mov     [eax], ecx
        mov     [eax+4], ecx
        mov     [eax+8], ecx
        mov     [eax+12], ecx
.cont:
        add     eax, 16
        cmp     eax, hotkey_list+256*16
        jb      .loop
; get process PID
        mov     eax, esi
        shl     eax, BSF sizeof.APPDATA
        mov     eax, [eax + SLOT_BASE + APPDATA.tid]
; compare current lock input with process PID
        cmp     eax, [PID_lock_input]
        jne     @f

        xor     eax, eax
        mov     [PID_lock_input], eax
@@:
; remove hotkeys in buffer
        mov     eax, hotkey_buffer
.loop2:
        cmp     [eax], esi
        jnz     .cont2
        and     dword [eax+4], 0
        and     dword [eax], 0
.cont2:
        add     eax, 8
        cmp     eax, hotkey_buffer+120*8
        jb      .loop2

        mov     ecx, esi          ; remove buttons
  .bnewba2:
        mov     edi, [BTN_ADDR]
        mov     eax, edi
        cld
        movzx   ebx, word [edi]
        inc     bx
  .bnewba:
        dec     bx
        jz      .bnmba
        add     eax, 0x10
        cmp     cx, [eax]
        jnz     .bnewba
        pusha
        mov     ecx, ebx
        inc     ecx
        shl     ecx, 4
        mov     ebx, eax
        add     eax, 0x10
        call    memmove
        dec     dword [edi]
        popa
        jmp     .bnewba2
  .bnmba:

        pusha   ; save window coordinates for window restoring
        cld
        shl     esi, BSF sizeof.WDATA
        add     esi, window_data
        mov     eax, [esi + WDATA.box.left]
        mov     [draw_limits.left], eax
        add     eax, [esi + WDATA.box.width]
        mov     [draw_limits.right], eax
        mov     eax, [esi + WDATA.box.top]
        mov     [draw_limits.top], eax
        add     eax, [esi + WDATA.box.height]
        mov     [draw_limits.bottom], eax

        xor     eax, eax
        mov     edi, esi
        mov     ecx, sizeof.WDATA/4
        rep stosd
        popa

; debuggee test
        pushad
        mov     edi, esi
        shl     edi, BSF sizeof.APPDATA
        mov     eax, [SLOT_BASE + edi + APPDATA.debugger_slot]
        test    eax, eax
        jz      .nodebug
        movi    ecx, 8
        push    dword [SLOT_BASE + edi + APPDATA.tid]; PID
        push    2
        call    debugger_notify
        pop     ecx
        pop     ecx
.nodebug:
        popad

        mov     ebx, [.slot]
        shl     ebx, BSF sizeof.APPDATA
        push    ebx
        mov     ebx, [SLOT_BASE + ebx + APPDATA.pl0_stack]

        stdcall kernel_free, ebx

        pop     ebx
        mov     ebx, [SLOT_BASE + ebx + APPDATA.cur_dir]
        stdcall kernel_free, ebx

        mov     edi, [.slot]
        shl     edi, BSF sizeof.APPDATA
        add     edi, SLOT_BASE

        mov     eax, [edi + APPDATA.io_map]
        cmp     eax, [SLOT_BASE + sizeof.APPDATA+APPDATA.io_map]
        je      @F
        call    free_page
@@:
        mov     eax, [edi+APPDATA.io_map+4]
        cmp     eax, [SLOT_BASE+sizeof.APPDATA+APPDATA.io_map+4]
        je      @F
        call    free_page
@@:
        lea     ebx, [edi + APPDATA.list]
        list_del ebx                    ;destroys edx, ecx

  ; activate window
        movzx   eax, word [WIN_STACK + esi*2]
        cmp     eax, [thread_count]
        jne     .dont_activate
        pushad
 .check_next_window:
        dec     eax
        cmp     eax, 1
        jbe     .nothing_to_activate
        lea     esi, [WIN_POS + eax*2]
        movzx   edi, word [esi]              ; edi = process
        shl     edi, BSF sizeof.APPDATA
        cmp     [SLOT_BASE + edi + APPDATA.state], TSTATE_FREE ; skip free slots
        je      .check_next_window
        shr     edi, (BSF sizeof.APPDATA - BSF sizeof.WDATA)
        add     edi, window_data
; \begin{diamond}[19.09.2006]
; skip minimized windows
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .check_next_window
; \end{diamond}
        call    waredraw
 .nothing_to_activate:
        popad
 .dont_activate:

        push    esi     ; remove hd1 & cd & flp reservation
        shl     esi, BSF sizeof.APPDATA
        mov     esi, [SLOT_BASE + esi + APPDATA.tid]
        cmp     [cd_status], esi
        jnz     @f
        call    free_cd_channel
        and     [cd_status], 0
@@:
        pop     esi
        cmp     [bgrlockpid], esi
        jnz     @f
        and     [bgrlockpid], 0
        and     [bgrlock], 0
@@:

        pusha                 ; remove all port reservations
        mov     edx, esi
        shl     edx, BSF sizeof.APPDATA
        mov     edx, [SLOT_BASE + edx + APPDATA.tid]

  .rmpr0:
        mov     esi, [RESERVED_PORTS]

        test    esi, esi
        jz      .rmpr9

  .rmpr3:

        mov     edi, esi
        shl     edi, 4
        add     edi, RESERVED_PORTS

        cmp     edx, [edi]
        je      .rmpr4

        dec     esi
        jnz     .rmpr3

        jmp     .rmpr9

  .rmpr4:

        mov     ecx, 256
        sub     ecx, esi
        shl     ecx, 4

        mov     esi, edi
        add     esi, 16
        cld
        rep movsb

        dec     dword [RESERVED_PORTS]

        jmp     .rmpr0

  .rmpr9:
        popa

; clearing APPDATA structure this thread
        pushad
        mov     edi, esi
        shl     edi, BSF sizeof.APPDATA
        add     edi, SLOT_BASE
        mov     eax, 0x20202020
        stosd
        stosd
        stosd
        mov     ecx, 244/4
        xor     eax, eax
        rep stosd
        popad

        mov     edi, esi ; do not run this process slot
        shl     edi, BSF sizeof.APPDATA
        mov     [SLOT_BASE + edi + APPDATA.state], TSTATE_FREE
; debugger test - terminate all debuggees
        mov     eax, 2
        mov     ecx, SLOT_BASE + 2*sizeof.APPDATA + APPDATA.debugger_slot
.xd0:
        cmp     eax, [thread_count]
        ja      .xd1
        cmp     dword [ecx], esi
        jnz     @f
        and     dword [ecx], 0
        pushad
        xchg    eax, ecx
        mov     ebx, 2
        call    sys_system
        popad
@@:
        inc     eax
        add     ecx, sizeof.APPDATA
        jmp     .xd0
.xd1:
;release slot

        bts     [thr_slot_map], esi

        mov     ecx, [.process]
        lea     eax, [ecx + PROC.thr_list]
        cmp     eax, [eax + LHEAD.next]
        jne     @F

        call    destroy_process.internal
@@:
        sti     ; .. and life goes on

        mov     eax, [draw_limits.left]
        mov     ebx, [draw_limits.top]
        mov     ecx, [draw_limits.right]
        mov     edx, [draw_limits.bottom]
        call    calculatescreen
        xor     eax, eax
        xor     esi, esi
        call    redrawscreen

        call    unlock_application_table
    ;mov   esi,process_terminated
    ;call  sys_msg_board_str
        add     esp, 8
        ret
restore .slot
restore .process

; Three following procedures are used to guarantee that
; some part of kernel code will not be terminated from outside
; while it is running.
; Note: they do not protect a thread from terminating due to errors inside
; the thread; accessing a nonexisting memory would still terminate it.

; First two procedures must be used in pair by thread-to-be-protected
; to signal the beginning and the end of an important part.
; It is OK to have nested areas.

; The last procedure must be used by outside wanna-be-terminators;
; if it is safe to terminate the given thread immediately, it returns eax=1;
; otherwise, it returns eax=0 and notifies the target thread that it should
; terminate itself when leaving a critical area (the last critical area if
; they are nested).

; Implementation. Those procedures use one dword in APPDATA for the thread,
; APPDATA.terminate_protection.
; * The upper bit is 1 during normal operations and 0 when terminate is requested.
; * Other bits form a number = depth of critical regions,
;   plus 1 if the upper bit is 1.
; * When this dword goes to zero, the thread should be destructed,
;   and the procedure in which it happened becomes responsible for destruction.

; Enter critical area. Called by thread which wants to be protected.
proc protect_from_terminate
        mov     edx, [current_slot]
; Atomically increment depth of critical areas and get the old value.
        mov     eax, 1
        lock xadd [edx + APPDATA.terminate_protection], eax
; If the old value was zero, somebody has started to terminate us,
; so we are destructing and cannot do anything protected.
; Otherwise, return to the caller.
        test    eax, eax
        jz      @f
        ret
@@:
; Wait for somebody to finish us.
        call    change_task
        jmp     @b
endp

; Leave critical area. Called by thread which wants to be protected.
proc unprotect_from_terminate
        mov     edx, [current_slot]
; Atomically decrement depth of critical areas.
        lock dec [edx + APPDATA.terminate_protection]
; If the result of decrement is zero, somebody has requested termination,
; but at that moment we were inside a critical area; terminate now.
        jz      sys_end
; Otherwise, return to the caller.
        ret
endp

; Request termination of thread identified by edx = SLOT_BASE + slot*sizeof.APPDATA.
; Called by anyone.
proc request_terminate
        xor     eax, eax        ; set return value
; Atomically clear the upper bit. If it was already zero, then
; somebody has requested termination before us, so just exit.
        lock btr [edx + APPDATA.terminate_protection], 31
        jnc     .unsafe
; Atomically decrement depth of critical areas.
        lock dec [edx + APPDATA.terminate_protection]
; If the result of decrement is nonzero, the target thread is inside a
; critical area; leave termination to leaving that area.
        jnz     .unsafe
; Otherwise, it is safe to kill the target now and the caller is responsible
; for this. Return eax=1.
        inc     eax
.unsafe:
        ret
endp

