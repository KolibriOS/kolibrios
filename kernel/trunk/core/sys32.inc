;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  MenuetOS process management, protected ring3                ;;
;;                                                              ;;
;;  Distributed under GPL. See file COPYING for details.        ;;
;;  Copyright 2003 Ville Turjanmaa                              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

align 4 ;3A08
build_interrupt_table:
        mov     edi, idts
        mov     esi, sys_int
        mov     ecx, 0x40
        mov     eax, (10001110b shl 24) + os_code
  @@:
        movsw   ;low word of code-entry
        stosd   ;interrupt gate type : os_code selector
        movsw   ;high word of code-entry
        loop    @b
        movsd   ;copy low  dword of trap gate for int 0x40
        movsd   ;copy high dword of trap gate for int 0x40
        lidt    [esi]
        ret

iglobal
  align 4
  sys_int:
    ;exception handlers addresses (for interrupt gate construction)
        dd      e0,e1,e2,e3,e4,e5,e6,except_7 ; SEE: core/fpu.inc
        dd      e8,e9,e10,e11,e12,e13,page_fault_exc,e15
        dd      e16, e17,e18, e19
        times   12 dd unknown_interrupt ;int_20..int_31

    ;interrupt handlers addresses (for interrupt gate construction)
        ; 0x20 .. 0x2F - IRQ handlers
        dd      irq0, irq_serv.irq_1, irq_serv.irq_2
        dd      irq_serv.irq_3, irq_serv.irq_4
        dd      irq_serv.irq_5,  irq_serv.irq_6,  irq_serv.irq_7
        dd      irq_serv.irq_8,  irq_serv.irq_9,  irq_serv.irq_10
        dd      irq_serv.irq_11, irq_serv.irq_12, irqD, irq_serv.irq_14, irq_serv.irq_15
        dd irq_serv.irq_16
        dd irq_serv.irq_17
        dd irq_serv.irq_18
        dd irq_serv.irq_19
        dd irq_serv.irq_20
        dd irq_serv.irq_21
        dd irq_serv.irq_22
        dd irq_serv.irq_23

    times 32 - IRQ_RESERVED dd unknown_interrupt
    ;int_0x40 gate trap (for directly copied)
        dw      i40 and 0xFFFF, os_code, 11101111b shl 8, i40 shr 16

  idtreg: ; data for LIDT instruction (!!! must be immediately below sys_int data)
        dw      2*($-sys_int-4)-1
        dd      idts ;0x8000B100
        dw      0    ;просто выравнивание

  msg_fault_sel dd  msg_exc_8,msg_exc_u,msg_exc_a,msg_exc_b
                dd  msg_exc_c,msg_exc_d,msg_exc_e

  msg_exc_8     db "Double fault", 0
  msg_exc_u     db "Undefined Exception", 0
  msg_exc_a     db "Invalid TSS", 0
  msg_exc_b     db "Segment not present", 0
  msg_exc_c     db "Stack fault", 0
  msg_exc_d     db "General protection fault", 0
  msg_exc_e     db "Page fault", 0

  if lang eq sp
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

page_fault_exc:                 ; дуракоусточивость: селекторы испорчены...
        pop     [ss:pf_err_code]; действительно до следующего #PF
        save_ring3_context
        mov     bl, 14

exc_c:                          ; исключения (все, кроме 7-го - #NM)
; Фрэйм стека при исключении/прерывании из 3-го кольца + pushad (т.е., именно здесь)
  reg_ss        equ esp+0x30
  reg_esp3      equ esp+0x2C
  reg_eflags    equ esp+0x28
  reg_cs3       equ esp+0x24
  reg_eip       equ esp+0x20
 ; это фрэйм от pushad
  reg_eax       equ esp+0x1C
  reg_ecx       equ esp+0x18
  reg_edx       equ esp+0x14
  reg_ebx       equ esp+0x10
  reg_esp0      equ esp+0x0C
  reg_ebp       equ esp+0x08
  reg_esi       equ esp+0x04
  reg_edi       equ esp+0x00

        mov     ax, app_data        ;исключение
        mov     ds, ax                  ;загрузим правильные значения
        mov     es, ax                  ;в регистры
        cld                     ; и приводим DF к стандарту
        movzx   ebx, bl
; redirect to V86 manager? (EFLAGS & 0x20000) != 0?
        test    byte[reg_eflags+2], 2
        jnz     v86_exc_c
        cmp     bl, 14          ; #PF
        jne     @f
        call    page_fault_handler ; SEE: core/memory.inc
  @@:
        mov     esi, [current_slot]
        btr     [esi+APPDATA.except_mask], ebx
        jnc     @f
        mov     eax, [esi+APPDATA.exc_handler]
        test    eax, eax
        jnz     IRetToUserHook
  @@:
        cli
        mov     eax, [esi+APPDATA.debugger_slot]
        test    eax, eax
        jnz     .debug
        sti
; not debuggee => say error and terminate
        call    show_error_parameters ;; only ONE using, inline ???
       ;mov     edx, [TASK_BASE]
        mov     [edx + TASKDATA.state], byte 4 ; terminate
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
        mov     ebx, [TASK_BASE]
        push    [ebx+TASKDATA.pid] ; PID
        push    ecx             ; debug_message code ((here: ecx==1/3))
        mov     cl, 12          ; debug_message size
        call    debugger_notify ;; only ONE using, inline ??? SEE: core/debug.inc
        add     esp, 12
        mov     edx, [TASK_BASE]
        mov     byte [edx+TASKDATA.state], 1 ; suspended
        call    change_task     ; SEE: core/shed.inc
        restore_ring3_context
        iretd

IRetToUserHook:
        xchg    eax, [reg_eip]
        sub     dword[reg_esp3], 8
        mov     edi, [reg_esp3]
        stosd
        mov     [edi], ebx
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
        mov     edx, [TASK_BASE];not scratched below
        if lang eq sp
        DEBUGF  1, "K : Proceso - terminado forzado PID: %x [%s]\n", [edx+TASKDATA.pid], [current_slot]
        else
        DEBUGF  1, "K : Process - forced terminate PID: %x [%s]\n", [edx+TASKDATA.pid], [current_slot]
        end if
        cmp     bl, 0x08
        jb      .l0
        cmp     bl, 0x0e
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
        DEBUGF  1, "K : [ESP+00]: %x [ESP+04]: %x [ESP+08]: %x\n", [ebx], [ebx+4], [ebx+8]
        DEBUGF  1, "K : [ESP+12]: %x [ESP+16]: %x [ESP+20]: %x\n", [ebx+12], [ebx+16], [ebx+20]
        DEBUGF  1, "K : [ESP+24]: %x [ESP+28]: %x [ESP+32]: %x\n", [ebx+24], [ebx+28], [ebx+32]
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

        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        add     eax, CURRENT_TASK+TASKDATA.pid
        mov     eax, [eax]

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

;  * eax = 64 - номер функции
;  * ebx = 1 - единственная подфункция
;  * ecx = новый размер памяти
;Возвращаемое значение:
;  * eax = 0 - успешно
;  * eax = 1 - недостаточно памяти

align 4
sys_resize_app_memory:
        ; ebx = 1 - resize
        ; ecx = new amount of memory

;        cmp    eax,1
        dec     ebx
        jnz     .no_application_mem_resize
        stdcall new_mem_resize, ecx
        mov     [esp+32], eax
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

           .slot equ esp   ;locals

        push    esi        ;save .slot

        shl     esi, 8
        cmp     [SLOT_BASE+esi+APPDATA.dir_table], 0
        jne     @F
        pop     esi
        shl     esi, 5
        mov     [CURRENT_TASK+esi+TASKDATA.state], 9
        ret
@@:
        lea     edx, [SLOT_BASE+esi]
        call    scheduler_remove_thread
           ;mov    esi,process_terminating
           ;call   sys_msg_board_str
        call    lock_application_table

; if the process is in V86 mode...
        mov     eax, [.slot]
        shl     eax, 8
        mov     esi, [eax+SLOT_BASE+APPDATA.pl0_stack]
        add     esi, RING0_STACK_SIZE
        cmp     [eax+SLOT_BASE+APPDATA.saved_esp0], esi
        jz      .nov86
; ...it has page directory for V86 mode
        mov     esi, [eax+SLOT_BASE+APPDATA.saved_esp0]
        mov     ecx, [esi+4]
        mov     [eax+SLOT_BASE+APPDATA.dir_table], ecx
; ...and I/O permission map for V86 mode
        mov     ecx, [esi+12]
        mov     [eax+SLOT_BASE+APPDATA.io_map], ecx
        mov     ecx, [esi+8]
        mov     [eax+SLOT_BASE+APPDATA.io_map+4], ecx
.nov86:

        mov     esi, [.slot]
        shl     esi, 8
        add     esi, SLOT_BASE+APP_OBJ_OFFSET
@@:
        mov     eax, [esi+APPOBJ.fd]
        test    eax, eax
        jz      @F

        cmp     eax, esi
        je      @F

        push    esi
        call    [eax+APPOBJ.destroy]
           DEBUGF 1,"%s",msg_obj_destroy
        pop     esi
        jmp     @B
@@:

        mov     eax, [.slot]
        shl     eax, 8
        stdcall destroy_app_space, [SLOT_BASE+eax+APPDATA.dir_table], [SLOT_BASE+eax+APPDATA.dlls_list_ptr]

        mov     esi, [.slot]
        cmp     [fpu_owner], esi ; if user fpu last -> fpu user = 2
        jne     @F

        mov     [fpu_owner], 2
        mov     eax, [256*2+SLOT_BASE+APPDATA.fpu_state]
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
        shl     eax, 5
        mov     eax, [eax+CURRENT_TASK+TASKDATA.pid]
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
  bnewba2:
        mov     edi, [BTN_ADDR]
        mov     eax, edi
        cld
        movzx   ebx, word [edi]
        inc     bx
  bnewba:
        dec     bx
        jz      bnmba
        add     eax, 0x10
        cmp     cx, [eax]
        jnz     bnewba
        pusha
        mov     ecx, ebx
        inc     ecx
        shl     ecx, 4
        mov     ebx, eax
        add     eax, 0x10
        call    memmove
        dec     dword [edi]
        popa
        jmp     bnewba2
  bnmba:

        pusha   ; save window coordinates for window restoring
        cld
        shl     esi, 5
        add     esi, window_data
        mov     eax, [esi+WDATA.box.left]
        mov     [draw_limits.left], eax
        add     eax, [esi+WDATA.box.width]
        mov     [draw_limits.right], eax
        mov     eax, [esi+WDATA.box.top]
        mov     [draw_limits.top], eax
        add     eax, [esi+WDATA.box.height]
        mov     [draw_limits.bottom], eax

        xor     eax, eax
        mov     [esi+WDATA.box.left], eax
        mov     [esi+WDATA.box.width], eax
        mov     [esi+WDATA.box.top], eax
        mov     [esi+WDATA.box.height], eax
        mov     [esi+WDATA.cl_workarea], eax
        mov     [esi+WDATA.cl_titlebar], eax
        mov     [esi+WDATA.cl_frames], eax
        mov     dword [esi+WDATA.reserved], eax; clear all flags: wstate, redraw, wdrawn
        lea     edi, [esi-window_data+draw_data]
        mov     ecx, 32/4
        rep stosd
        popa

; debuggee test
        pushad
        mov     edi, esi
        shl     edi, 5
        mov     eax, [SLOT_BASE+edi*8+APPDATA.debugger_slot]
        test    eax, eax
        jz      .nodebug
        movi    ecx, 8
        push    dword [CURRENT_TASK+edi+TASKDATA.pid]; PID
        push    2
        call    debugger_notify
        pop     ecx
        pop     ecx
.nodebug:
        popad

        mov     ebx, [.slot]
        shl     ebx, 8
        push    ebx
        mov     ebx, [SLOT_BASE+ebx+APPDATA.pl0_stack]

        stdcall kernel_free, ebx

        pop     ebx
        mov     ebx, [SLOT_BASE+ebx+APPDATA.cur_dir]
        stdcall kernel_free, ebx

        mov     edi, [.slot]
        shl     edi, 8
        add     edi, SLOT_BASE

        mov     eax, [edi+APPDATA.io_map]
        cmp     eax, [SLOT_BASE+256+APPDATA.io_map]
        je      @F
        call    free_page
@@:
        mov     eax, [edi+APPDATA.io_map+4]
        cmp     eax, [SLOT_BASE+256+APPDATA.io_map+4]
        je      @F
        call    free_page
@@:
        mov     eax, 0x20202020
        stosd
        stosd
        stosd
        mov     ecx, 244/4
        xor     eax, eax
        rep stosd

  ; activate window
        movzx   eax, word [WIN_STACK + esi*2]
        cmp     eax, [TASK_COUNT]
        jne     .dont_activate
        pushad
 .check_next_window:
        dec     eax
        cmp     eax, 1
        jbe     .nothing_to_activate
        lea     esi, [WIN_POS+eax*2]
        movzx   edi, word [esi]              ; edi = process
        shl     edi, 5
        cmp     [CURRENT_TASK + edi + TASKDATA.state], byte 9 ; skip dead slots
        je      .check_next_window
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
        shl     esi, 5
        mov     esi, [esi+CURRENT_TASK+TASKDATA.pid]
        cmp     [cd_status], esi
        jnz     @f
        call    free_cd_channel
        and     [cd_status], 0
@@:
        cmp     [flp_status], esi
        jnz     @f
        and     [flp_status], 0
@@:
        pop     esi
        cmp     [bgrlockpid], esi
        jnz     @f
        and     [bgrlockpid], 0
        and     [bgrlock], 0
@@:

        pusha                 ; remove all port reservations
        mov     edx, esi
        shl     edx, 5
        add     edx, CURRENT_TASK
        mov     edx, [edx+TASKDATA.pid]

  rmpr0:

        mov     esi, [RESERVED_PORTS]

        test    esi, esi
        jz      rmpr9

  rmpr3:

        mov     edi, esi
        shl     edi, 4
        add     edi, RESERVED_PORTS

        cmp     edx, [edi]
        je      rmpr4

        dec     esi
        jnz     rmpr3

        jmp     rmpr9

  rmpr4:

        mov     ecx, 256
        sub     ecx, esi
        shl     ecx, 4

        mov     esi, edi
        add     esi, 16
        cld
        rep movsb

        dec     dword [RESERVED_PORTS]

        jmp     rmpr0

  rmpr9:

        popa
        mov     edi, esi ; do not run this process slot
        shl     edi, 5
        mov     [edi+CURRENT_TASK + TASKDATA.state], byte 9
; debugger test - terminate all debuggees
        mov     eax, 2
        mov     ecx, SLOT_BASE+2*0x100+APPDATA.debugger_slot
.xd0:
        cmp     eax, [TASK_COUNT]
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
        add     ecx, 0x100
        jmp     .xd0
.xd1:
;    call  systest
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
        add     esp, 4
        ret
restore .slot

;build_scheduler:
;        mov     esi, boot_sched_1
;        call    boot_log
;        call   build_process_gdt_tss_pointer

;        mov    esi,boot_sched_2
;        call   boot_log
;        ret

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
        lock xadd [edx+APPDATA.terminate_protection], eax
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
        lock dec [edx+APPDATA.terminate_protection]
; If the result of decrement is zero, somebody has requested termination,
; but at that moment we were inside a critical area; terminate now.
        jz      sys_end
; Otherwise, return to the caller.
        ret
endp

; Request termination of thread identified by edx = SLOT_BASE + slot*256.
; Called by anyone.
proc request_terminate
        xor     eax, eax        ; set return value
; Atomically clear the upper bit. If it was already zero, then
; somebody has requested termination before us, so just exit.
        lock btr [edx+APPDATA.terminate_protection], 31
        jnc     .unsafe
; Atomically decrement depth of critical areas.
        lock dec [edx+APPDATA.terminate_protection]
; If the result of decrement is nonzero, the target thread is inside a
; critical area; leave termination to leaving that area.
        jnz     .unsafe
; Otherwise, it is safe to kill the target now and the caller is responsible
; for this. Return eax=1.
        inc     eax
.unsafe:
        ret
endp

