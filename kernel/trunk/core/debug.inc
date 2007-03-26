$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; diamond, 2006
sys_debug_services:
	cmp	eax, 9
	ja	@f
	jmp	dword [sys_debug_services_table+eax*4]
@@:	ret
sys_debug_services_table:
	dd	debug_set_event_data
	dd	debug_getcontext
	dd	debug_setcontext
	dd	debug_detach
	dd	debug_suspend
	dd	debug_resume
	dd	debug_read_process_memory
	dd	debug_write_process_memory
	dd	debug_terminate
	dd	debug_set_drx

debug_set_event_data:
; in: ebx = pointer
; destroys eax
        mov     eax, [CURRENT_TASK]
	shl	eax, 8
        mov     [eax+SLOT_BASE+APPDATA.dbg_event_mem], ebx
	ret

get_debuggee_slot:
; in: ebx=PID
; out: CF=1 if error
;      CF=0 and eax=slot*0x20 if ok
; out: interrupts disabled
	cli
	mov	eax, ebx
	call	pid_to_slot
	test	eax, eax
	jz	.ret_bad
	shl	eax, 5
	push	ebx
        mov     ebx, [CURRENT_TASK]
        cmp     [SLOT_BASE+eax*8+APPDATA.debugger_slot], ebx
	pop	ebx
	jnz	.ret_bad
;	clc	; automatically
	ret
.ret_bad:
	stc
	ret

debug_detach:
; in: ebx=pid
; destroys eax,ebx
	call	get_debuggee_slot
	jc	.ret
        and     dword [eax*8+SLOT_BASE+APPDATA.debugger_slot], 0
	call	do_resume
.ret:
	sti
	ret

debug_terminate:
; in: ebx=pid
	call	get_debuggee_slot
	jc	debug_detach.ret
	mov	ebx, eax
	shr	ebx, 5
	push	2
	pop	eax
	jmp	sys_system

debug_suspend:
; in: ebx=pid
; destroys eax,ebx
	call	get_debuggee_slot
	jc	.ret
        mov     bl, [CURRENT_TASK+eax+TASKDATA.state] ; process state
	test	bl, bl
	jz	.1
	cmp	bl, 5
	jnz	.ret
	mov	bl, 2
.2:     mov     [CURRENT_TASK+eax+TASKDATA.state], bl
.ret:
	sti
	ret
.1:
	inc	ebx
	jmp	.2

do_resume:
        mov     bl, [CURRENT_TASK+eax+TASKDATA.state]
	cmp	bl, 1
	jz	.1
	cmp	bl, 2
	jnz	.ret
	mov	bl, 5
.2:     mov     [CURRENT_TASK+eax+TASKDATA.state], bl
.ret:	ret
.1:	dec	ebx
	jmp	.2

debug_resume:
; in: ebx=pid
; destroys eax,ebx
	call	get_debuggee_slot
	jc	.ret
	call	do_resume
.ret:	sti
	ret

debug_getcontext:
; in:
; ebx=pid
; ecx=sizeof(CONTEXT)
; edx->CONTEXT
; destroys eax,ecx,edx,esi,edi
	cmp	ecx, 28h
	jnz	.ret
	add	edx, std_application_base_address
	push	ebx
	mov	ebx, edx
	call	check_region
	pop	ebx
	dec	eax
	jnz	.ret
	call	get_debuggee_slot
	jc	.ret
	imul	eax, tss_step/32
	add	eax, tss_data
	mov	edi, edx
        cmp     [eax+TSS._cs], app_code
	jnz	.ring0
        lea     esi, [eax+TSS._eip]
	shr	ecx, 2
	rep	movsd
	jmp	.ret
.ring0:
; note that following code assumes that all interrupt/exception handlers
; saves ring-3 context by push ds es, pushad in this order
        mov     esi, [eax+TSS._esp0]
; top of ring0 stack: ring3 stack ptr (ss+esp), iret data (cs+eip+eflags), ds, es, pushad
	sub	esi, 8+12+8+20h
	lodsd
	mov	[edi+24h], eax
	lodsd
	mov	[edi+20h], eax
	lodsd
	mov	[edi+1Ch], eax
	lodsd
	lodsd
	mov	[edi+14h], eax
	lodsd
	mov	[edi+10h], eax
	lodsd
	mov	[edi+0Ch], eax
	lodsd
	mov	[edi+8], eax
	add	esi, 8
	lodsd
	mov	[edi], eax
	lodsd
	lodsd
	mov	[edi+4], eax
	lodsd
	mov	[edi+18h], eax
.ret:
	sti
	ret

debug_setcontext:
; in:
; ebx=pid
; ecx=sizeof(CONTEXT)
; edx->CONTEXT
; destroys eax,ecx,edx,esi,edi
	cmp	ecx, 28h
	jnz	.ret
	add	edx, std_application_base_address
	push	ebx
	mov	ebx, edx
	call	check_region
	pop	ebx
	dec	eax
	jnz	.ret
	call	get_debuggee_slot
	jc	.stiret
	imul	eax, tss_step/32
	add	eax, tss_data
	mov	esi, edx
        cmp     [eax+TSS._cs], app_code
	jnz	.ring0
        lea     edi, [eax+TSS._eip]
	shr	ecx, 2
	rep	movsd
	jmp	.stiret
.ring0:
        mov     edi, [eax+TSS._esp0]
	sub	edi, 8+12+8+20h
	mov	eax, [esi+24h]
	stosd
	mov	eax, [esi+20h]
	stosd
	mov	eax, [esi+1Ch]
	stosd
	scasd
	mov	eax, [esi+14h]
	stosd
	mov	eax, [esi+10h]
	stosd
	mov	eax, [esi+0Ch]
	stosd
	mov	eax, [esi+8]
	stosd
	add	edi, 8
	mov	eax, [esi]
	stosd
	scasd
	mov	eax, [esi+4]
	stosd
	mov	eax, [esi+18h]
	stosd
.stiret:
	sti
.ret:
	ret

debug_set_drx:
	call	get_debuggee_slot
	jc	.errret
	mov	ebp, eax
        lea     eax, [eax*8+SLOT_BASE+APPDATA.dbg_regs]
; [eax]=dr0, [eax+4]=dr1, [eax+8]=dr2, [eax+C]=dr3
; [eax+10]=dr7
	add	edx, std_application_base_address
	jc	.errret
	cmp	cl, 3
	ja	.errret
	mov	ebx, dr7
	shr	ebx, cl
	shr	ebx, cl
	test	ebx, 2		; bit 1+2*index = G0..G3, global break enable
	jnz	.errret2
	test	ch, ch
	jns	.new
; clear breakpoint
	movzx	ecx, cl
	add	ecx, ecx
	and	dword [eax+ecx*2], 0	; clear DR<i>
	btr	dword [eax+10h], ecx	; clear L<i> bit
	test	byte [eax+10h], 55h
	jnz	.okret
	imul	eax, ebp, tss_step/32
        and     byte [eax + tss_data + TSS._trap], not 1
.okret:
	and	dword [esp+36], 0
	sti
	ret
.errret:
	sti
	mov	dword [esp+36], 1
	ret
.errret2:
	sti
	mov	dword [esp+36], 2
	ret
.new:
; add new breakpoint
; cl=index; ch=flags; edx=address
	test	ch, 0xF0
	jnz	.errret
	mov	bl, ch
	and	bl, 3
	cmp	bl, 2
	jz	.errret
	mov	bl, ch
	shr	bl, 2
	cmp	bl, 2
	jz	.errret
	test	dl, bl
	jnz	.errret
	or	byte [eax+10h+1], 3	; set GE and LE flags
	movzx	ebx, ch
	movzx	ecx, cl
	add	ecx, ecx
	bts	dword [eax+10h], ecx	; set L<i> flag
	add	ecx, ecx
	mov	[eax+ecx], edx		; set DR<i>
	shl	ebx, cl
	mov	edx, 0xF
	shl	edx, cl
	not	edx
	and	[eax+10h+2], dx
	or	[eax+10h+2], bx		; set R/W and LEN fields
	imul	eax, ebp, tss_step/32
        or      byte [eax + tss_data + TSS._trap], 1
	jmp	.okret

debug_read_process_memory:
; in:
; ebx=pid
; ecx=length
; esi->buffer in debugger
; edx=address in debuggee
; out: [esp+36]=sizeof(read)
; destroys all
	add	esi, std_application_base_address
	push	ebx
	mov	ebx, esi
	call	check_region
	pop	ebx
	dec	eax
	jnz	.err
	call	get_debuggee_slot
	jc	.err
	shr	eax, 5
	mov	ebx, esi
	call	read_process_memory
	sti
	mov	dword [esp+36], eax
	ret
.err:
	or	dword [esp+36], -1
	ret

debug_write_process_memory:
; in:
; ebx=pid
; ecx=length
; esi->buffer in debugger
; edx=address in debuggee
; out: [esp+36]=sizeof(write)
; destroys all
	add	esi, std_application_base_address
	push	ebx
	mov	ebx, esi
	call	check_region
	pop	ebx
	dec	eax
	jnz	debug_read_process_memory.err
	call	get_debuggee_slot
	jc	debug_read_process_memory.err
	shr	eax, 5
	mov	ebx, esi
	call	write_process_memory
	sti
	mov	[esp+36], eax
	ret

debugger_notify:
; in: eax=debugger slot
;     ecx=size of debug message
;     [esp+4]..[esp+4+ecx]=message
; interrupts must be disabled!
; destroys all general registers
; interrupts remain disabled
	xchg	ebp, eax
	mov	edi, [timer_ticks]
	add	edi, 500	; 5 sec timeout
.1:
	mov	eax, ebp
	shl	eax, 8
        mov     edx, [SLOT_BASE+eax+APPDATA.dbg_event_mem]
	test	edx, edx
	jz	.ret
; read buffer header
	push	ecx
	push	eax
	push	eax
	mov	eax, ebp
	mov	ebx, esp
	mov	ecx, 8
	call	read_process_memory
	cmp	eax, ecx
	jz	@f
	add	esp, 12
	jmp	.ret
@@:
	cmp	dword [ebx], 0
	jg	@f
.2:
	pop	ecx
	pop	ecx
	pop	ecx
        cmp     dword [CURRENT_TASK], 1
	jnz	.notos
	cmp	[timer_ticks], edi
	jae	.ret
.notos:
	sti
	call	change_task
	cli
	jmp	.1
@@:
	mov	ecx, [ebx+8]
	add	ecx, [ebx+4]
	cmp	ecx, [ebx]
	ja	.2
; advance buffer position
	push	ecx
	mov	ecx, 4
	sub	ebx, ecx
	mov	eax, ebp
	add	edx, ecx
	call	write_process_memory
	pop	eax
; write message
	mov	eax, ebp
	add	edx, ecx
	add	edx, [ebx+8]
	add	ebx, 20
	pop	ecx
	pop	ecx
	pop	ecx
	call	write_process_memory
; new debug event
	mov	eax, ebp
	shl	eax, 8
        or      byte [SLOT_BASE+eax+APPDATA.event_mask+1], 1      ; set flag 100h
.ret:
	ret

debug_exc:
; int 1 = #DB
	save_ring3_context
	cld
	mov	ax, os_data
	mov	ds, ax
	mov	es, ax
	mov	eax, dr6
	test	ax, ax
	jns	@f
; this is exception from task switch
; set DRx registers for task and continue
        mov     eax, [CURRENT_TASK]
	shl	eax, 8
        add     eax, SLOT_BASE+APPDATA.dbg_regs
	mov	ecx, [eax+0]
	mov	dr0, ecx
	mov	ecx, [eax+4]
	mov	dr1, ecx
	mov	ecx, [eax+8]
	mov	dr2, ecx
	mov	ecx, [eax+0Ch]
	mov	dr3, ecx
	xor	ecx, ecx
	mov	dr6, ecx
	mov	ecx, [eax+10h]
	mov	dr7, ecx
	restore_ring3_context
	iretd
@@:
	push	eax
	xor	eax, eax
	mov	dr6, eax
; test if debugging
	cli
        mov     eax, [CURRENT_TASK]
	shl	eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.debugger_slot]
	test	eax, eax
	jnz	.debug
	sti
; not debuggee => say error and terminate
	add	esp, 28h+4
	mov	[error_interrupt], 1
	call	show_error_parameters
        mov     edx, [TASK_BASE]
	mov	byte [edx+TASKDATA.state], 4
	jmp	change_task
.debug:
; we are debugged process, notify debugger and suspend ourself
; eax=debugger PID
	pop	edx
	mov	ebx, dr7
	mov	cl, not 1
.l1:
	test	bl, 1
	jnz	@f
	and	dl, cl
@@:
	shr	ebx, 2
	add	cl, cl
	inc	ecx
	cmp	cl, not 10h
	jnz	.l1
	push	edx	; DR6 image
        mov     ecx, [TASK_BASE]
	push	dword [ecx+TASKDATA.pid]	; PID
	push	12
	pop	ecx
	push	3	; 3 = debug exception
	call	debugger_notify
	pop	ecx
	pop	ecx
	pop	ecx
        mov     edx, [TASK_BASE]
	mov	byte [edx+TASKDATA.state], 1	; suspended
	call	change_task
	restore_ring3_context
	iretd
