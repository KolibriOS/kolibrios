;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
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


align 4
idtreg:
     dw   8*0x41-1
     dd   idts+8

build_interrupt_table:

	mov    edi, idts+8
	mov    esi, sys_int
	mov    ecx, 0x40
     @@:
	lodsd
	mov    [edi],	ax	     ; lower part of offset
	mov    [edi+2], word os_code ; segment selector
	mov	ax, word 10001110b shl 8	; type: interrupt gate
	mov	[edi+4], eax
	add    edi, 8
	loop	@b

	;mov    edi,8*0x40+idts+8
	mov	dword [edi], (i40 and 0xFFFF) or (os_code shl 16)
	mov	dword [edi+4], (11101111b shl 8) or (i40 and 0xFFFF0000)
						; type: trap gate
	ret

iglobal

  msg_sel_ker	db "kernel", 0
  msg_sel_app	db "application", 0

  sys_int:
    dd e0,debug_exc,e2,e3
    dd e4,e5,e6,e7
    dd e8,e9,e10,e11
    dd e12,e13,page_fault_handler,e15

    dd except_16, e17,e18, except_19
    times 12 dd unknown_interrupt

    dd	 irq0, irq_serv.irq_1, irq_serv.irq_2
if USE_COM_IRQ
    dd	 irq_serv.irq_3, irq_serv.irq_4
else
    dd	 p_irq3, p_irq4
end if
    dd	 irq_serv.irq_5,  p_irq6,	  irq_serv.irq_7
    dd	 irq_serv.irq_8,  irq_serv.irq_9, irq_serv.irq_10
    dd	 irq_serv.irq_11, irq_serv.irq_12,irqD ,p_irq14,p_irq15

    times 16 dd unknown_interrupt

    dd	 i40
endg

macro save_ring3_context
{
    pushad
}
macro restore_ring3_context
{
    popad
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

exc_wo_code 0, 2, 3, 4, 5, 6, 9, 15, 18
exc_w_code 8, 10, 11, 12, 13, 14, 17

exc_c:
	mov   ax, app_data  ;исключение
	mov   ds, ax	    ;загрузим правильные значени
	mov   es, ax	    ;в регистры

; redirect to V86 manager? (EFLAGS & 0x20000) != 0?
	test	byte [esp+20h+8+2], 2
	jnz	v86_exc_c

; test if debugging
	cli
	mov   eax, [current_slot]
	mov   eax, [eax+APPDATA.debugger_slot]
	test  eax, eax
	jnz   .debug
	sti
; not debuggee => say error and terminate
	movzx eax, bl
	mov   [error_interrupt], eax
	call  show_error_parameters
	add   esp, 0x20
	mov   edx, [TASK_BASE]
	mov   [edx + TASKDATA.state], byte 4

	jmp   change_task

.debug:
; we are debugged process, notify debugger and suspend ourself
; eax=debugger PID
	cld
	movzx ecx, bl
	push  ecx
	mov   ecx, [TASK_BASE]
	push  dword [ecx+TASKDATA.pid]	  ; PID of current process
	push  12
	pop   ecx
	push  1        ; 1=exception
	call  debugger_notify
	pop   ecx
	pop   ecx
	pop   ecx
	mov   edx, [TASK_BASE]
	mov   byte [edx+TASKDATA.state], 1	  ; suspended
	call  change_task
	restore_ring3_context
	iretd

iglobal
	hexletters	db '0123456789ABCDEF'
	error_interrupt dd  -1
endg

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
show_error_parameters:
    mov eax,[CURRENT_TASK]
    shl eax, 5
    DEBUGF  1, "K : Process - forced terminate PID: %x\n", [CURRENT_TASK + TASKDATA.pid + eax]
    mov eax, [error_interrupt]
    cmp al, 0x08
    jne @f
    DEBUGF  1, "K : Double fault\n"
    jmp defined_error
@@:
    cmp al, 0x0a
    jne @f
    DEBUGF  1, "K : Invalid TSS\n"
    jmp defined_error
@@:
    cmp al, 0x0b
    jne @f
    DEBUGF  1, "K : Segment not present\n"
    jmp defined_error
@@:
    cmp al, 0x0c
    jne @f
    DEBUGF  1, "K : Stack fault\n"
    jmp defined_error
@@:
    cmp al, 0x0d
    jne @f
    DEBUGF  1, "K : General protection fault\n"
    jmp defined_error
@@:
    cmp al, 0x0e
    jne @f
    DEBUGF  1, "K : Page fault\n"
    jmp defined_error
@@:
    DEBUGF  1, "K : Undefined Exception\n"
defined_error:
    DEBUGF  1, "K : EAX : %x EBX : %x ECX : %x\n", [esp + 0x20], [esp - 12 + 0x20], [esp - 4 + 0x20]
    DEBUGF  1, "K : EDX : %x ESI : %x EDI : %x\n", [esp - 8 + 0x20], [esp - 24 + 0x20], [esp - 28 + 0x20]
    DEBUGF  1, "K : EBP : %x EIP : %x ", [esp - 20 + 0x20], [esp + 4 + 0x20]

    mov eax, [esp + 8 + 0x20]
    mov edi, msg_sel_app
    mov ebx, [esp + 16 + 0x20]
    cmp eax, app_code
    je	@f
    mov edi, msg_sel_ker
    mov ebx, [esp - 16 + 0x20]
@@:
    DEBUGF  1, "ESP : %x\nK : Flags : %x CS : %x (%s)\n", ebx, [esp + 12 + 0x20], eax, edi
    ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


; irq1  ->  hid/keyboard.inc
macro irqh [num]
{
  forward
  p_irq#num :
     mov   edi, num
     jmp   irqhandler
}

irqh 2,3,4,5,7,8,9,10,11


p_irq6:
     save_ring3_context
     mov   ax, app_data  ;os_data
     mov   ds, ax
     mov   es, ax
     call  fdc_irq
     call  ready_for_next_irq
     restore_ring3_context
     iret


p_irq14:
	save_ring3_context
	mov	ax, app_data  ;os_data
	mov	ds, ax
	mov	es, ax
	mov	edi, 14
	cmp	[v86_irqhooks+edi*8], 0
	jnz	v86_irq2
;	mov	byte [BOOT_VAR + 0x48E], 0xFF
	call	[irq14_func]
	call	ready_for_next_irq_1
	restore_ring3_context
	iret
p_irq15:
	save_ring3_context
	mov	ax, app_data  ;os_data
	mov	ds, ax
	mov	es, ax
	mov	edi, 15
	cmp	[v86_irqhooks+edi*8], 0
	jnz	v86_irq2
;	mov	byte [BOOT_VAR + 0x48E], 0xFF
	call	[irq15_func]
	call	ready_for_next_irq_1
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
     mov   ax, app_data  ;os_data
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

     mov    esi,edi	     ; 1
     shl    esi,6	     ; 1
     add    esi,irq00read    ; 1
     shl    edi,12	     ; 1
     add    edi,IRQ_SAVE
     mov    ecx,16

   irqnewread:
     dec    ecx
     js     irqover

     movzx  edx, word [esi]	   ; 2+

     test   edx, edx		   ; 1
     jz     irqover


     mov    ebx, [edi]		   ; address of begin of buffer in edi      ; + 0x0 dword - data size
     mov    eax, 4000							    ; + 0x4 dword - data begin offset
     cmp    ebx, eax
     je     irqfull
     add    ebx, [edi + 0x4]	   ; add data size to data begin offset
     cmp    ebx, eax		   ; if end of buffer, begin cycle again
     jb     @f

     xor    ebx, ebx

  @@:
     add    ebx, edi
     movzx  eax, byte[esi + 3]	   ; get type of data being received 1 - byte, 2 - word
     dec    eax
     jz     irqbyte
     dec    eax
     jnz    noirqword

     in     ax,dx
     cmp    ebx, 3999		   ; check for address odd in the end of buffer
     jne    .odd
     mov    [ebx + 0x10], ax
     jmp    .add_size
  .odd:
     mov    [ebx + 0x10], al	   ; I could make mistake here :)
     mov    [edi + 0x10], ah
  .add_size:
     add    dword [edi], 2
     jmp    nextport


  irqbyte:
     in     al,dx
     mov    [ebx + 0x10],al
     inc    dword [edi]
  nextport:
     add    esi,4
     jmp    irqnewread


   noirqword:
   irqfull:
   irqover:

     ret



set_application_table_status:
	push eax

	mov  eax,[CURRENT_TASK]
	shl  eax, 5
	add  eax,CURRENT_TASK+TASKDATA.pid
	mov  eax,[eax]

	mov  [application_table_status],eax

	pop  eax

	ret


clear_application_table_status:
	push eax

	mov  eax,[CURRENT_TASK]
	shl  eax, 5
	add  eax,CURRENT_TASK+TASKDATA.pid
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

sys_threads:

; eax=1 create thread
;
;   ebx=thread start
;   ecx=thread stack value
;
; on return : eax = pid
jmp new_sys_threads

iglobal
  process_terminating	db 'K : Process - terminating',13,10,0
  process_terminated	db 'K : Process - done',13,10,0
  msg_obj_destroy	db 'K : destroy app object',13,10,0
endg

; param
;  esi= slot

terminate: ; terminate application

	   .slot equ esp   ;locals

	   push   esi	   ;save .slot

	   shl esi, 8
	   cmp [SLOT_BASE+esi+APPDATA.dir_table], 0
	   jne @F
	   pop	  esi
	   shl	  esi, 5
	   mov	  [CURRENT_TASK+esi+TASKDATA.state], 9
	   ret
@@:
	   ;mov    esi,process_terminating
	   ;call   sys_msg_board_str
	   DEBUGF 1,"%s",process_terminating
@@:
	   cli
	   cmp	 [application_table_status],0
	   je	 term9
	   sti
	   call  change_task
	   jmp	 @b
term9:
	   call  set_application_table_status

; if the process is in V86 mode...
	mov	eax, [.slot]
	shl	eax, 8
	mov	esi, [eax+SLOT_BASE+APPDATA.pl0_stack]
	add	esi, RING0_STACK_SIZE
	cmp	[eax+SLOT_BASE+APPDATA.saved_esp0], esi
	jz	.nov86
; ...it has page directory for V86 mode
	mov	esi, [eax+SLOT_BASE+APPDATA.saved_esp0]
	mov	ecx, [esi+4]
	mov	[eax+SLOT_BASE+APPDATA.dir_table], ecx
; ...and I/O permission map for V86 mode
	mov	ecx, [esi+12]
	mov	[eax+SLOT_BASE+APPDATA.io_map], ecx
	mov	ecx, [esi+8]
	mov	[eax+SLOT_BASE+APPDATA.io_map+4], ecx
.nov86:

	   mov esi, [.slot]
	   shl esi,8
	   add esi, SLOT_BASE+APP_OBJ_OFFSET
@@:
	   mov eax, [esi+APPOBJ.fd]
	   test eax, eax
	   jz @F

	   cmp eax, esi
	   je @F

	   push esi
	   call [eax+APPOBJ.destroy]
	   DEBUGF 1,"%s",msg_obj_destroy
	   pop esi
	   jmp @B
@@:
	   mov eax, [.slot]
	   shl eax, 8
	   mov eax,[SLOT_BASE+eax+APPDATA.dir_table]
	   stdcall destroy_app_space, eax

	   mov esi, [.slot]
	   cmp [fpu_owner],esi	 ; if user fpu last -> fpu user = 1
	   jne @F

	   mov [fpu_owner],1
	   mov eax, [256+SLOT_BASE+APPDATA.fpu_state]
	   clts
	   bt [cpu_caps], CAPS_SSE
	   jnc .no_SSE
	   fxrstor [eax]
	   jmp @F
.no_SSE:
	   fnclex
	   frstor [eax]
@@:

    mov   [KEY_COUNT],byte 0	       ; empty keyboard buffer
    mov   [BTN_COUNT],byte 0	       ; empty button buffer


; remove defined hotkeys
	mov	eax, hotkey_list
.loop:
	cmp	[eax+8], esi
	jnz	.cont
	mov	ecx, [eax]
	jecxz	@f
	push	dword [eax+12]
	pop	dword [ecx+12]
@@:
	mov	ecx, [eax+12]
	push	dword [eax]
	pop	dword [ecx]
	xor	ecx, ecx
	mov	[eax], ecx
	mov	[eax+4], ecx
	mov	[eax+8], ecx
	mov	[eax+12], ecx
.cont:
	add	eax, 16
	cmp	eax, hotkey_list+256*16
	jb	.loop
; remove hotkeys in buffer
	mov	eax, hotkey_buffer
.loop2:
	cmp	[eax], esi
	jnz	.cont2
	and	dword [eax+4], 0
	and	dword [eax], 0
.cont2:
	add	eax, 8
	cmp	eax, hotkey_buffer+120*8
	jb	.loop2

    mov   ecx,esi		  ; remove buttons
  bnewba2:
    mov   edi,[BTN_ADDR]
    mov   eax,edi
    cld
    movzx ebx,word [edi]
    inc   bx
  bnewba:
    dec   bx
    jz	  bnmba
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
    mov  eax, [SLOT_BASE+edi*8+APPDATA.debugger_slot]
    test eax, eax
    jz	 .nodebug
    push 8
    pop  ecx
    push dword [CURRENT_TASK+edi+TASKDATA.pid]	 ; PID
    push 2
    call debugger_notify
    pop  ecx
    pop  ecx
.nodebug:
    popad

	   mov ebx, [.slot]
	   shl ebx, 8
	   push ebx
	   mov ebx,[SLOT_BASE+ebx+APPDATA.pl0_stack]

	   stdcall kernel_free, ebx

	   pop ebx
	   mov ebx,[SLOT_BASE+ebx+APPDATA.cur_dir]
	   stdcall kernel_free, ebx

	   mov edi, [.slot]
	   shl edi,8
	   add edi,SLOT_BASE

	   mov eax, [edi+APPDATA.io_map]
	   cmp eax, (tss._io_map_0-OS_BASE+PG_MAP)
	   je @F
	   call free_page
@@:
	   mov eax, [edi+APPDATA.io_map+4]
	   cmp eax, (tss._io_map_1-OS_BASE+PG_MAP)
	   je @F
	   call free_page
@@:
	   mov eax, 0x20202020
	   stosd
	   stosd
	   stosd
	   mov ecx,244/4
	   xor eax, eax
	   rep stosd

  ; activate window
	movzx  eax, word [WIN_STACK + esi*2]
	cmp    eax, [TASK_COUNT]
	jne    .dont_activate
	pushad
 .check_next_window:
	dec    eax
	cmp    eax, 1
	jbe    .nothing_to_activate
	lea    esi, [WIN_POS+eax*2]
	movzx  edi, word [esi]		     ; edi = process
	shl    edi, 5
	cmp    [CURRENT_TASK + edi + TASKDATA.state], byte 9  ; skip dead slots
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

	push	esi	; remove hd1 & cd & flp reservation
	shl	esi, 5
	mov	esi, [esi+CURRENT_TASK+TASKDATA.pid]
	cmp	[hd1_status], esi
	jnz	@f
	call	free_hd_channel
	mov	[hd1_status], 0
@@:
	cmp	[cd_status], esi
	jnz	@f
	call	free_cd_channel
	mov	[cd_status], 0
@@:
	cmp	[flp_status], esi
	jnz	@f
	mov	[flp_status], 0
@@:
	pop	esi
	cmp	[bgrlockpid], esi
	jnz	@f
	and	[bgrlockpid], 0
	mov	[bgrlock], 0
@@:

    pusha ; remove all irq reservations
    mov   eax,esi
    shl   eax, 5
    mov   eax,[eax+CURRENT_TASK+TASKDATA.pid]
    mov   edi,irq_owner
    xor   ebx, ebx
    xor   edx, edx
  newirqfree:
    cmp   [edi + 4 * ebx], eax
    jne   nofreeirq
    mov   [edi + 4 * ebx], edx				; remove irq reservation
    mov   [irq_tab + 4 * ebx], edx			; remove irq handler
    mov   [irq_rights + 4 * ebx], edx			; set access rights to full access
  nofreeirq:
    inc   ebx
    cmp   ebx, 16
    jb	  newirqfree
    popa

    pusha		      ; remove all port reservations
    mov   edx,esi
    shl   edx, 5
    add   edx,CURRENT_TASK
    mov   edx,[edx+TASKDATA.pid]

  rmpr0:

    mov   esi,[RESERVED_PORTS]

    cmp   esi,0
    je	  rmpr9

  rmpr3:

    mov   edi,esi
    shl   edi,4
    add   edi,RESERVED_PORTS

    cmp   edx,[edi]
    je	  rmpr4

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

    dec   dword [RESERVED_PORTS]

    jmp   rmpr0

  rmpr9:

    popa
    mov  edi,esi	 ; do not run this process slot
    shl  edi, 5
    mov  [edi+CURRENT_TASK + TASKDATA.state],byte 9
; debugger test - terminate all debuggees
    mov  eax, 2
    mov  ecx, SLOT_BASE+2*0x100+APPDATA.debugger_slot
.xd0:
    cmp  eax, [TASK_COUNT]
    ja	 .xd1
    cmp  dword [ecx], esi
    jnz  @f
    and  dword [ecx], 0
    pushad
    xchg eax, ecx
    mov  ebx, 2
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

    mov   [MOUSE_BACKGROUND],byte 0  ; no mouse background
    mov   [DONT_DRAW_MOUSE],byte 0  ; draw mouse

    mov   [application_table_status],0
    ;mov   esi,process_terminated
    ;call  sys_msg_board_str
    DEBUGF 1,"%s",process_terminated
    add esp, 4
    ret
restore .slot

iglobal
  boot_sched_1	  db   'Building gdt tss pointer',0
  boot_sched_2	  db   'Building IDT table',0
endg


build_scheduler:

	mov    esi,boot_sched_1
	call   boot_log
  ;      call   build_process_gdt_tss_pointer

  ;      mov    esi,boot_sched_2
  ;      call   boot_log

	ret
