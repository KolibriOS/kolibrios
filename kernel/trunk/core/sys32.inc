;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                               ;;
;;  MenuetOS process management, protected ring3                 ;;
;;                                                               ;;
;;  Distributed under GPL. See file COPYING for details.         ;;
;;  Copyright 2003 Ville Turjanmaa                               ;;
;;                                                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32

; GDT TABLE

gdts:

        dw     gdte-$-1
        dd     gdts
        dw     0

int_code_l:
os_code_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

int_data_l:
os_data_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00

; --------------- APM ---------------------
apm_code_32:
        dw     0x10        ; limit 64kb
        db     0, 0, 0
        dw     11011111b *256 +10011010b
        db     0x00
apm_code_16:
        dw     0x10
        db     0, 0, 0
        dw     10011111b *256 +10011010b
        db     0x00
apm_data_16:
        dw     0x10
        db     0, 0, 0
        dw     10011111b *256 +10010010b
        db     0x00
; -----------------------------------------

app_code_l:
      dw 0xFFFF;((0x80000000-std_application_base_address) shr 12) and 0xffff
      dw 0
      db 0x40
      db cpl3
      dw G32+D32+0x6000+0x7;

app_data_l:
      dw 0xFFFF;(0x80000000-std_application_base_address) shr 12 and 0xffff
      dw 0
      db 0x40
      db drw3
      dw G32+D32+0x6000+0x7;

graph_data_l:

        dw     0x3ff
        dw     0x0000
        db     0x00
        dw     11010000b *256 +11110010b
        db     0x00

tss0_l:
      times (max_processes+10) dd 0,0

gdte:


idtreg:
     dw   8*0x41-1
     dd   idts+8
;label idts at 0xB100-8



uglobal
 tss_sceleton:
  l.back   dw 0,0
  l.esp0   dd 0
  l.ss0    dw 0,0
  l.esp1   dd 0
  l.ss1    dw 0,0
  l.esp2   dd 0
  l.ss2    dw 0,0
  l.cr3    dd 0
  l.eip    dd 0
  l.eflags dd 0
  l.eax    dd 0
  l.ecx    dd 0
  l.edx    dd 0
  l.ebx    dd 0
  l.esp    dd 0
  l.ebp    dd 0
  l.esi    dd 0
  l.edi    dd 0
  l.es     dw 0,0
  l.cs     dw 0,0
  l.ss     dw 0,0
  l.ds     dw 0,0
  l.fs     dw 0,0
  l.gs     dw 0,0
  l.ldt    dw 0,0
  l.trap   dw 0
  l.io     dw 0
endg


build_process_gdt_tss_pointer:

        mov    ecx,tss_data
        mov    edi,0
      setgdtl2:
        mov    [edi+gdts+ tss0 +0], word tss_step
        mov    [edi+gdts+ tss0 +2], cx
        mov    eax,ecx
        shr    eax,16
        mov    [edi+gdts+ tss0 +4], al
        mov    [edi+gdts+ tss0 +7], ah
        mov    [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b
        add    ecx,tss_step
        add    edi,8
        cmp    edi,8*(max_processes+5)
        jbe    setgdtl2

        ret


build_interrupt_table:

        mov    edi, idts+8
        mov    esi, sys_int
        mov    ecx, 0x40
     @@:
        mov    eax, [esi]
        mov    [edi],   ax           ; lower part of offset
        mov    [edi+2], word os_code ; segment selector
        shr    eax, 16
        mov    [edi+4], word 10001110b shl 8 ; interrupt descriptor
        mov    [edi+6], ax
        add    esi, 4
        add    edi, 8
        dec    ecx
        jnz    @b

        ;mov    edi,8*0x40+idts+8
        mov    [edi + 0], word (i40 and ((1 shl 16)-1))
        mov    [edi + 2], word os_code
        mov    [edi + 4], word 11101110b*256
        mov    [edi + 6], word (i40 shr 16)

        ret

iglobal
  sys_int:
    dd e0,debug_exc,e2,e3
    dd e4,e5,e6,e7
    dd e8,e9,e10,e11
    dd e12,e13,page_fault_handler,e15

    dd except_16, e17,e18, except_19
    times 12 dd unknown_interrupt

    dd   irq0  , irq_serv.irq_1, p_irq2 ,irq_serv.irq_3
    dd   p_irq4 ,irq_serv.irq_5,p_irq6,irq_serv.irq_7
    dd   irq_serv.irq_8, irq_serv.irq_9, irq_serv.irq_10
    dd   irq_serv.irq_11,p_irq12,irqD ,p_irq14,p_irq15

    times 16 dd unknown_interrupt

    dd   i40
endg

macro save_ring3_context
{
    push    ds es
    pushad
}
macro restore_ring3_context
{
    popad
    pop    es ds
}

; simply return control to interrupted process
unknown_interrupt:
     iret

macro exc_wo_code [num]
{
  forward
  e#num :
      save_ring3_context
      mov bl, num
      jmp exc_c
}

macro exc_w_code [num]
{
  forward
  e#num :
      add esp, 4
      save_ring3_context
      mov bl, num
      jmp exc_c
}

exc_wo_code 0, 1, 2, 3, 4, 5, 6, 9, 15, 18
exc_w_code 8, 10, 11, 12, 13, 14, 17

exc_c:
        mov   ax, os_data
        mov   ds, ax
        mov   es, ax

; test if debugging
        cli
        mov   eax, [0x3000]
        shl   eax, 8
        mov   eax, [0x80000+eax+APPDATA.debugger_slot]
        test  eax, eax
        jnz   .debug
        sti
; not debuggee => say error and terminate
        add   esp, 28h
        movzx eax, bl
        mov   [error_interrupt], eax
        call  show_error_parameters

        mov   edx, [0x3010]
        mov   [edx + TASKDATA.state], byte 4

        jmp   change_task

.debug:
; we are debugged process, notify debugger and suspend ourself
; eax=debugger PID
        cld
        movzx ecx, bl
        push  ecx
        mov   ecx, [0x3010]
        push  dword [ecx+TASKDATA.pid]    ; PID of current process
        push  12
        pop   ecx
        push  1        ; 1=exception
        call  debugger_notify
        pop   ecx
        pop   ecx
        pop   ecx
        mov   edx, [0x3010]
        mov   byte [edx+TASKDATA.state], 1        ; suspended
        call  change_task
        restore_ring3_context
        iretd

;;;;;;;;;;;;;;;;;;;;;;;
;; FPU ERROR HANDLER ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
e7:
        save_ring3_context
        clts
        mov ax, os_data
        mov ds, ax
        mov es, ax

        mov ebx, [fpu_owner]
        cmp ebx, [CURRENT_TASK]
        je .exit

        shl ebx, 8
        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        bt [cpu_caps], CAPS_FXSR
        jnc .no_SSE

        fxsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        cmp dword [ebx+PROC_BASE+APPDATA.fpu_init], 0
        je .init
        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        fxrstor [eax]
        restore_ring3_context
        iret
.init:
        fninit                      ;нам не нужны немаскированные исключения
        mov dword [ebx+PROC_BASE+APPDATA.fpu_init], 1
.exit:
        restore_ring3_context
        iret
.no_SSE:
        fnsave [eax]
        mov ebx, [CURRENT_TASK]
        mov [fpu_owner], ebx
        shl ebx, 8
        cmp dword [ebx+PROC_BASE+APPDATA.fpu_init], 0
        je .init

        mov eax, [ebx+PROC_BASE+APPDATA.fpu_state]
        frstor [eax]
        restore_ring3_context
        iret

iglobal
  fpu_owner dd 1
 endg


writehex:
      pusha

      mov  edi, [write_error_to]
      mov  esi, 8
    @@:
      mov  ecx, eax
      and  ecx, 0xf

      mov  cl,[ecx+hexletters]
      mov  [edi],cl
      dec  edi

      shr  eax,4
      dec  esi
      jnz  @b

      popa
      ret

iglobal
  hexletters  db '0123456789ABCDEF'

  error_interrupt         dd  -1

  process_error  db 'K : Process - forced terminate INT: 00000000',13,10,0
  process_pid    db 'K : Process - forced terminate PID: 00000000',13,10,0
  process_eip    db 'K : Process - forced terminate EIP: 00000000',13,10,0
  system_error   db 'K : Kernel error',13,10,0
endg

uglobal
  write_error_to  dd  0x0
endg

show_error_parameters:

        mov    [write_error_to],process_pid+43
        mov    eax,[0x3000]
        shl    eax, 5
        mov    eax,[0x3000+TASKDATA.pid+eax]
        call   writehex

        mov    [write_error_to],process_error+43
        mov    eax,[error_interrupt]
        call   writehex

        cmp    dword [esp+4+4], os_code ; CS
        jnz    @f
        mov    esi,system_error
        call   sys_msg_board_str
      @@:
        mov    eax, [esp+4] ; EIP

        mov    [write_error_to],process_eip+43
        call   writehex

        mov    esi,process_error
        call   sys_msg_board_str

        mov    esi,process_pid
        call   sys_msg_board_str

        mov    esi,process_eip
        call   sys_msg_board_str

        ret



; irq1  ->  hid/keyboard.inc


macro irqh [num]
{
  forward
  p_irq#num :
     save_ring3_context
     mov   edi, num
     jmp   irq_c
}

irqh 2,5,7,8,9,10,11

 irq_c:
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax
     call  irqhandler
     restore_ring3_context
     iret

p_irq6:
     save_ring3_context
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax
     call  fdc_irq
     call  ready_for_next_irq
     restore_ring3_context
     iret

p_irq3:
     save_ring3_context
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax
     cmp   [com2_mouse_detected],0
     je    old_irq3_handler
     call  check_mouse_data_com2
     jmp   p_irq3_1
 old_irq3_handler:
     mov   edi,3
     call  irqhandler
  p_irq3_1:
     restore_ring3_context
     iret

p_irq4:
     save_ring3_context
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax
     cmp   [com1_mouse_detected],0
     je    old_irq4_handler
     call  check_mouse_data_com1
     jmp   p_irq4_1
 old_irq4_handler:
     mov   edi,4
     call  irqhandler
  p_irq4_1:
     restore_ring3_context
     iret

p_irq12:
     save_ring3_context
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax
     call  check_mouse_data_ps2
     restore_ring3_context
     iret

p_irq14:
        save_ring3_context
        mov     ax, os_data
        mov     ds, ax
        mov     es, ax
        call    [irq14_func]
        call    ready_for_next_irq_1
        restore_ring3_context
        iret
p_irq15:
        save_ring3_context
        mov     ax, os_data
        mov     ds, ax
        mov     es, ax
        call    [irq15_func]
        call    ready_for_next_irq_1
        restore_ring3_context
        iret

ready_for_next_irq:
     mov    [check_idle_semaphore],5
     mov   al, 0x20
     out   0x20, al
     ret

ready_for_next_irq_1:
     mov    [check_idle_semaphore],5
     mov   al, 0x20
     out    0xa0,al
     out   0x20, al
     ret

irqD:
     save_ring3_context
     mov   ax, os_data
     mov   ds, ax
     mov   es, ax

     mov   dx,0xf0
     mov   al,0
     out   dx,al

     mov   dx,0xa0
     mov   al,0x20
     out   dx,al
     mov   dx,0x20
     out   dx,al

     restore_ring3_context

     iret


irqhandler:

     push   edi

     mov    esi,edi          ; 1
     shl    esi,6            ; 1
     add    esi,irq00read    ; 1
     shl    edi,12           ; 1
     add    edi,0x2E0000
     mov    ecx,16

     mov    [check_idle_semaphore],5

   irqnewread:
     dec    ecx
     js     irqover

     mov    dx,[esi]         ; 2+

     cmp    dx,0             ; 1
     jz     irqover
     cmp    [esi+3],byte 1   ; 2     ; byte read
     jne    noirqbyte        ; 4-11

     in     al,dx

     mov    edx,[edi]
     cmp    edx,4000
     je     irqfull
     mov    ebx,edi
     add    ebx,0x10
     add    ebx,edx
     mov    [ebx],al
     inc    edx
     mov    [edi],edx

     add    esi,4
     jmp    irqnewread

   noirqbyte:


     cmp    [esi+3],byte 2     ; word read
     jne    noirqword

     in     ax,dx

     mov    edx,[edi]
     cmp    edx,4000
     je     irqfull
     mov    ebx,edi
     add    ebx,0x10
     add    ebx,edx
     mov    [ebx],ax
     add    edx,2
     mov    [edi],edx
     add    esi,4
     jmp    irqnewread

   noirqword:
   irqfull:
   irqover:

     mov    al,0x20            ; ready for next irq
     out    0x20,al

     pop    ebx
     cmp    ebx,7
     jbe    noa0
     out    0xa0,al
   noa0:

     ret



set_application_table_status:
        push eax

        mov  eax,[0x3000]
        shl  eax, 5
        add  eax,0x3000+TASKDATA.pid
        mov  eax,[eax]

        mov  [application_table_status],eax

        pop  eax

        ret


clear_application_table_status:
        push eax

        mov  eax,[0x3000]
        shl  eax, 5
        add  eax,0x3000+TASKDATA.pid
        mov  eax,[eax]

        cmp  eax,[application_table_status]
        jne  apptsl1
        mov  [application_table_status],0
      apptsl1:

        pop  eax

        ret



sys_resize_app_memory:
        ; eax = 1 - resize
        ;     ebx = new amount of memory

        cmp    eax,1
        jne    .no_application_mem_resize

        stdcall new_mem_resize, ebx
        mov [esp+36], eax
        ret

.no_application_mem_resize:
        ret



get_app_params:

    push eax

    cmp  [0x90000+6],word '00'
    jne  no_00_header

    mov  eax,[0x90000+12]
    mov  [app_start],eax
    mov  eax,[0x90000+16]
    mov  [app_i_end],eax
    mov  eax,[0x90000+20]
    mov  [app_mem],eax
; \begin{diamond}[20.08.2006]
; sanity check (functions 19,58 load app_i_end bytes and that must
; fit in allocated memory to prevent kernel faults)
    cmp  eax,[app_i_end]
    jb   no_01_header
; \end{diamond}[20.08.2006]
    shr  eax,1
    sub  eax,0x10
    mov  [app_esp],eax
    mov  eax,[0x90000+24]
    mov  [app_i_param],eax
    mov  [app_i_icon],dword 0

    pop  eax
    clc
    ret

  no_00_header:


    cmp  [0x90000+6],word '01'
    jne  no_01_header

    mov  eax,[0x90000+12]
    mov  [app_start],eax
    mov  eax,[0x90000+16]
    mov  [app_i_end],eax
    mov  eax,[0x90000+20]
    mov  [app_mem],eax
; \begin{diamond}[20.08.2006]
    cmp  eax,[app_i_end]
    jb   no_01_header
; \end{diamond}[20.08.2006]
    mov  eax,[0x90000+24]
    mov  [app_esp],eax
    mov  eax,[0x90000+28]
    mov  [app_i_param],eax
    mov  eax,[0x90000+32]
    mov  [app_i_icon],eax

    pop  eax
    clc
    ret

   no_01_header:

    pop  eax
    stc
    ret


uglobal
  new_process_place  dd  0x0
  app_start    dd  0x0
  app_i_end    dd  0x0
  app_mem      dd  0x0
  app_esp      dd  0x0
  app_i_param  dd  0x0
  app_i_icon   dd  0x0
;  app_mem_pos  dd  0x0
  appl_path        dd 0x0
  appl_path_size   dd 0x0
endg


sys_threads:

; eax=1 create thread
;
;   ebx=thread start
;   ecx=thread stack value
;
; on return : eax = pid
jmp new_sys_threads

iglobal
  process_terminating   db 'K : Process - terminating',13,10,0
  process_terminated    db 'K : Process - done',13,10,0
endg


terminate: ; terminate application
    push   esi
    mov    esi,process_terminating
    call   sys_msg_board_str
    pop    esi

@@:
    cli
    cmp   [application_table_status],0
    je    term9
    sti
    call  change_task
    jmp   @b
  term9:

    call  set_application_table_status

    mov    eax,esi

    pushad
    shl   eax,8
    mov   eax,[PROC_BASE+eax+0xB8]
    stdcall destroy_app_space, eax
    popad

    cmp   [fpu_owner],esi   ; if user fpu last -> fpu user = 1
    jne   fpu_ok_1
    mov   [fpu_owner],1
  fpu_ok_1:

    mov   [0xf400],byte 0           ; empty keyboard buffer
    mov   [0xf500],byte 0           ; empty button buffer


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

    mov   ecx,esi                 ; remove buttons
  bnewba2:
    mov   edi,[0xfe88]
    mov   eax,edi
    cld
    movzx ebx,word [edi]
    inc   bx
  bnewba:
    dec   bx
    jz    bnmba
    add   eax,0x10
    cmp   cx,[eax]
    jnz   bnewba
    pusha
    mov   ecx,ebx
    inc   ecx
    shl   ecx,4
    mov   ebx,eax
    add   eax,0x10
    call  memmove
    dec   dword [edi]
    popa
    jmp   bnewba2
  bnmba:

    pusha     ; save window coordinates for window restoring
    cld
    shl   esi,5
    add   esi,window_data
    mov   eax,[esi+WDATA.box.left]
    mov   [dlx],eax
    add   eax,[esi+WDATA.box.width]
    mov   [dlxe],eax
    mov   eax,[esi+WDATA.box.top]
    mov   [dly],eax
    add   eax,[esi+WDATA.box.height]
    mov   [dlye],eax

    xor   eax, eax
    mov   [esi+WDATA.box.left],eax
    mov   [esi+WDATA.box.width],eax
    mov   [esi+WDATA.box.top],eax
    mov   [esi+WDATA.box.height],eax
    mov   [esi+WDATA.cl_workarea],eax
    mov   [esi+WDATA.cl_titlebar],eax
    mov   [esi+WDATA.cl_frames],eax
    mov   dword [esi+WDATA.reserved],eax ; clear all flags: wstate, redraw, wdrawn
    lea   edi, [esi-window_data+draw_data]
    mov   ecx,32/4
    rep   stosd
    popa

; debuggee test
    pushad
    mov  edi, esi
    shl  edi, 5
    mov  eax, [0x80000+edi*8+APPDATA.debugger_slot]
    test eax, eax
    jz   .nodebug
    push 8
    pop  ecx
    push dword [0x3000+edi+TASKDATA.pid]   ; PID
    push 2
    call debugger_notify
    pop  ecx
    pop  ecx
.nodebug:
    popad

    pusha         ; at 0x80000+
    mov   edi,esi
    shl   edi,8
    add   edi,0x80000
    mov   ecx,256/4
    xor   eax, eax
    rep   stosd
    popa

    pusha          ; name to spaces
    mov   edi,esi
    shl   edi,8
    add   edi,0x80000+APPDATA.app_name
    mov   ecx,11
    mov   eax,' '
    rep   stosb
    popa


  ; activate window
        movzx  eax, word [0xC000 + esi*2]
        cmp    eax, [0x3004]
        jne    .dont_activate
        pushad
 .check_next_window:
        dec    eax
        cmp    eax, 1
        jbe    .nothing_to_activate
        lea    esi, [0xc400+eax*2]
        movzx  edi, word [esi]               ; edi = process
        shl    edi, 5
        cmp    [0x3000 + edi + TASKDATA.state], byte 9  ; skip dead slots
        je     .check_next_window
        add    edi, window_data
; \begin{diamond}[19.09.2006]
; skip minimized windows
        test   [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz    .check_next_window
; \end{diamond}
        call   waredraw
 .nothing_to_activate:
        popad
 .dont_activate:

        push    esi     ; remove hd1 & cd & flp reservation
        shl     esi, 5
        mov     esi, [esi+0x3000+TASKDATA.pid]
        cmp     [hd1_status], esi
        jnz     @f
        mov     [hd1_status], 0
@@:
        cmp     [cd_status], esi
        jnz     @f
        mov     [cd_status], 0
@@:
        cmp     [flp_status], esi
        jnz     @f
        mov     [flp_status], 0
@@:
        pop     esi

    pusha ; remove all irq reservations
    mov   eax,esi
    shl   eax, 5
    mov   eax,[eax+0x3000+TASKDATA.pid]
    mov   edi,irq_owner
    mov   ecx,16
  newirqfree:
    scasd
    jne   nofreeirq
    mov   [edi-4],dword 0
  nofreeirq:
    loop   newirqfree
    popa


    pusha                     ; remove all port reservations
    mov   edx,esi
    shl   edx, 5
    add   edx,0x3000
    mov   edx,[edx+TASKDATA.pid]

  rmpr0:

    mov   esi,[0x2d0000]

    cmp   esi,0
    je    rmpr9

  rmpr3:

    mov   edi,esi
    shl   edi,4
    add   edi,0x2d0000

    cmp   edx,[edi]
    je    rmpr4

    dec   esi
    jnz   rmpr3

    jmp   rmpr9

  rmpr4:

    mov   ecx,256
    sub   ecx,esi
    shl   ecx,4

    mov   esi,edi
    add   esi,16
    cld
    rep   movsb

    dec   dword [0x2d0000]

    jmp   rmpr0

  rmpr9:

    popa
    mov  edi,esi         ; do not run this process slot
    shl  edi, 5
    mov  [edi+0x3000 + TASKDATA.state],byte 9
; debugger test - terminate all debuggees
    mov  eax, 2
    mov  ecx, 0x80000+2*0x100+APPDATA.debugger_slot
.xd0:
    cmp  eax, [0x3004]
    ja   .xd1
    cmp  dword [ecx], esi
    jnz  @f
    and  dword [ecx], 0
    pushad
    xchg eax, ebx
    mov  eax, 2
    call sys_system
    popad
@@:
    inc  eax
    add  ecx, 0x100
    jmp  .xd0
.xd1:
;    call  systest
    sti  ; .. and life goes on

    mov   eax, [dlx]
    mov   ebx, [dly]
    mov   ecx, [dlxe]
    mov   edx, [dlye]
    call  calculatescreen
    xor   eax, eax
    xor   esi, esi
    call  redrawscreen

    mov   [0xfff4],byte 0  ; no mouse background
    mov   [0xfff5],byte 0  ; draw mouse

    mov   [application_table_status],0
    mov   esi,process_terminated
    call  sys_msg_board_str

    ret

iglobal
  boot_sched_1    db   'Building gdt tss pointer',0
  boot_sched_2    db   'Building IDT table',0
endg


build_scheduler:

        mov    esi,boot_sched_1
        call   boot_log
        call   build_process_gdt_tss_pointer

        mov    esi,boot_sched_2
        call   boot_log
        call   build_interrupt_table

        ret

