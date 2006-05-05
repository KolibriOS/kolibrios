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

app_code_l:
      dw ((0x80000000-std_application_base_address) shr 12) and 0xffff
      dw 0
      db 0
      dw 11010000b*256+11111010b+256*((0x80000000-std_application_base_address) shr 28)
      db std_application_base_address shr 24

app_data_l:
      dw (0x80000000-std_application_base_address) shr 12 and 0xffff
      dw 0
      db 0
      dw 11010000b*256+11110010b+256*((0x80000000-std_application_base_address) shr 28)
      db std_application_base_address shr 24

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
label idts at 0xB100-8



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
    dd   e0,debug_exc,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15
    dd   e16,e17
    times 14 dd unknown_interrupt

    dd   irq0  ,irq1  ,p_irq2 ,p_irq3 ,p_irq4 ,p_irq5,p_irq6 ,p_irq7
    dd   p_irq8,p_irq9,p_irq10,p_irq11,p_irq12,irqD  ,p_irq14,p_irq15

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

exc_wo_code 0, 1, 2, 3, 4, 5, 6, 9, 15, 16 ; 18, 19
exc_w_code 8, 10, 11, 12, 13, 14, 17

exc_c:
        mov   ax, os_data
        mov   ds, ax
        mov   es, ax

; test if debugging
        cli
        mov   eax, [0x3000]
        shl   eax, 8
        mov   eax, [0x80000+eax+0xAC]
        test  eax, eax
        jnz   .debug
        sti
; not debuggee => say error and terminate
        add   esp, 28h
        movzx eax, bl        
        mov   [error_interrupt], eax
        call  show_error_parameters
        
        mov   edx, [0x3010]
        mov   [edx + 0xA], byte 4
        
        jmp   change_task

.debug:
; we are debugged process, notify debugger and suspend ourself
; eax=debugger PID
        movzx ecx, bl
        push  ecx
        mov   ecx, [0x3010]
        push  dword [ecx+4]    ; PID of current process
        push  12
        pop   ecx
        push  1        ; 1=exception
        call  debugger_notify
        pop   ecx
        pop   ecx
        pop   ecx
        mov   edx, [0x3010]
        mov   byte [edx+0xA], 1        ; suspended
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
        mov   ax, os_data
        mov   ds, ax
        mov   es, ax
        
        mov   eax, [prev_user_of_fpu]
        shl   eax, 8
        add   eax, 0x80000 + 0x10
        fsave [eax]
        
        mov   eax, [0x3000]
        mov   [prev_user_of_fpu], eax
        shl   eax, 8
        add   eax, 0x80000
        cmp   [eax + 0x7f], byte 0
        je    @f
        frstor [eax+0x10]
     @@:
        mov   [eax + 0x7f], byte 1
        restore_ring3_context
        iret
        
iglobal
  prev_user_of_fpu dd 1
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
        mov    eax,[0x3000+4+eax]
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

irqh 2,5,7,8,9,10,11,14,15

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
        add  eax,0x3000+4
        mov  eax,[eax]

        mov  [application_table_status],eax

        pop  eax

        ret


clear_application_table_status:
        push eax

        mov  eax,[0x3000]
        shl  eax, 5
        add  eax,0x3000+4
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
        
        jmp    new_mem_resize ;resize for new type of processes


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
    shr  eax,1
    sub  eax,0x10
    mov  [app_esp],eax
    mov  eax,[0x90000+24]
    mov  [app_i_param],eax
    mov  [app_i_icon],dword 0

    pop  eax
    mov  esi,1
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
    mov  eax,[0x90000+24]
    mov  [app_esp],eax
    mov  eax,[0x90000+28]
    mov  [app_i_param],eax
    mov  eax,[0x90000+32]
    mov  [app_i_icon],eax

    pop  eax
    mov  esi,1
    ret

   no_01_header:

    pop  eax
    mov  esi,0
    ret


start_application_fl:
    jmp new_start_application_fl

;************************************************************************

start_application_floppy:
    jmp  new_start_application_floppy

;********************************************************************

start_application_hd:
    jmp   new_start_application_hd

uglobal
  new_process_place  dd  0x0
  app_start    dd  0x0
  app_i_end    dd  0x0
  app_mem      dd  0x0
  app_esp      dd  0x0
  app_i_param  dd  0x0
  app_i_icon   dd  0x0
  app_mem_pos  dd  0x0
  appl_path        dd 0x0
  appl_path_size   dd 0x0         
endg

iglobal
  hd_app_string      db  'HDAPP       '
  process_loading    db 'K : Process - loading ',13,10,0
  process_running    db 'K : Process - done',13,10,0
  first_gdt_search   dd 0x2
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
    call   dispose_app_cr3_table

    cmp   [prev_user_of_fpu],esi   ; if user fpu last -> fpu user = 1
    jne   fpu_ok_1
    mov   [prev_user_of_fpu],1
  fpu_ok_1:

    mov   [0xf400],byte 0           ; empty keyboard buffer
    mov   [0xf500],byte 0           ; empty button buffer


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
    mov   ax,[esi+0]
    mov   word [dlx],ax
    mov   bx,[esi+8]
    add   ax,bx
    mov   word [dlxe],ax
    mov   ax,[esi+4]
    mov   word [dly],ax
    mov   bx,[esi+12]
    add   ax,bx
    mov   word [dlye],ax
    mov   [esi+0],word 0
    mov   [esi+8],word 5
    mov   ax,[0xFE04]
    mov   [esi+4],ax
    mov   [esi+12],word 5
    xor   eax, eax
    mov   [esi+16],eax;dword 0
    mov   [esi+20],eax;dword 0
    mov   [esi+24],eax;dword 0
    mov   [esi+28],eax;dword 0
    popa

    pusha
    mov   edi,esi
    shl   edi,5
    add   edi,window_data
    mov   ecx,32/4
    xor   eax, eax
  ;  cld
    rep   stosd

    mov   eax,[0xFE04]      ; set window to start from maxy+1
    add   eax,2

    mov   edi,esi
    shl   edi,5
    add   edi,window_data
    mov   [edi+4],eax

    popa

    pusha
    mov   edi,esi
    shl   edi,5
    add   edi,draw_data
    mov   ecx,32/4
    xor   eax, eax
  ;  cld
    rep   stosd
    popa

; debuggee test
    pushad
    mov  edi, esi
    shl  edi, 5
    mov  eax, [0x80000+edi*8+0xAC]
    test eax, eax
    jz   .nodebug
    push 8
    pop  ecx
    push dword [0x3000+edi+0x4]   ; PID
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
  ;  cld
    rep   stosd
    popa

    pusha          ; name to spaces
    mov   edi,esi
    shl   edi,8
    add   edi,0x80000
    mov   ecx,11
    mov   eax,' '
  ;  cld
    rep   stosb
    popa

    pusha                ; C000 --> C400
    mov   eax, 0xc000
    mov   esi, 0
  nlc40:
    add   eax, 2
    inc   esi
    cmp   esi, [0x3004]
    jae   nlc41
    movzx ecx, word [eax]
    mov   [0xC400 + ecx*2], si
    jmp   nlc40
  nlc41:
    popa

    pusha ; remove hd1 reservation
    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]
    cmp   [hd1_status],edx
    jne   no_hd1_s_remove
    mov   [hd1_status],0
  no_hd1_s_remove:
    popa

    pusha ; remove all irq reservations
    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]
    mov   edi,irq_owner
    mov   ecx,16
  newirqfree:
    cmp   [edi],edx
    jne   nofreeirq
    mov   [edi],dword 0
  nofreeirq:
    add    edi,4
    loop   newirqfree
    popa


    pusha                     ; remove all port reservations
    mov   [deleted_process],esi
    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]

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
    mov  [edi+0x300A],byte 9
; debugger test - terminate all debuggees
    mov  eax, 2
    mov  ecx, 0x80000+2*0x100+0xAC
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

;    movzx eax,word [dlx]
;    movzx ebx,word [dly]
;    movzx ecx,word [dlxe]
;    movzx edx,word [dlye]
    call  calculatescreen
    cli
    mov  eax,[deleted_process]
    cmp  eax,[active_process]
    jne  no_activate_process
    call read_active_process_stack
  no_activate_process:
    sti
    xor   eax, eax
    xor   esi, esi
    call  redrawscreen

    mov   [0xfff4],byte 0  ; no mouse background
    mov   [0xfff5],byte 0  ; draw mouse

    mov   [application_table_status],0
    mov   esi,process_terminated
    call  sys_msg_board_str
    ret

save_active_process_stack:
    cmp   [active_proc_stack_coun],0xa400+0x400-4
    jne   @f
    mov   [active_proc_stack_coun],0xa400-4
  @@:
    push  eax
    push  ebx
    mov   eax,[active_process]
    shl   eax,5
    add   eax,0x3000
    mov   [eax-twdw+31],byte 1
    add   [active_proc_stack_coun],4
    mov   eax,[active_process]
    mov   ebx,[active_proc_stack_coun]
    mov   [ebx],eax
    pop   ebx
    pop   eax
    ret

read_active_process_stack:
    cmp   [active_proc_stack_coun],0xa400-4
    jne   @f
    mov   [active_proc_stack_coun],0xa400+0x400-4
  @@:
    push  eax
    push  ebx
    mov   ebx,[active_proc_stack_coun]
    mov   eax,[ebx]
    mov   [0xff01],eax     ; activate
    sub   [active_proc_stack_coun],4
    mov   [active_process_flag],1
    pop   ebx
    pop   eax
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

