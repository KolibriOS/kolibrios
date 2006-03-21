; this code uses following additions to system structures:
; in additional app info at 80000..8FFFF:
; AC (dword)	0 or debugger slot
; BC (dword)	address of debug event memory
; new process slot state:
; 1 = suspended
; 2 = suspended waiting for event
; new event: 8 (and new possible bitflag for A8 in additional app info)
; diamond, 2006
sys_debug_services:
	cmp	eax, 8
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

debug_set_event_data:
; in: ebx = pointer
; destroys eax
	mov	eax, [0x3000]
	shl	eax, 8
	mov	[eax+0x80000+0xBC], ebx
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
	mov	ebx, [0x3000]
	cmp	[0x80000+eax*8+0xAC], ebx
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
	and	dword [eax*8+0x80000+0xAC], 0
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
	mov	bl, [0x3000+eax+0xA]	; process state
	test	bl, bl
	jz	.1
	cmp	bl, 5
	jnz	.ret
	mov	bl, 2
.2:	mov	[0x3000+eax+0xA], bl
.ret:
	sti
	ret
.1:
	inc	ebx
	jmp	.2

do_resume:
	mov	bl, [0x3000+eax+0xA]
	cmp	bl, 1
	jz	.1
	cmp	bl, 2
	jnz	.ret
	mov	bl, 5
.2:	mov	[0x3000+eax+0xA], bl
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
	cmp	[l.cs - tss_sceleton + eax], app_code
	jnz	.ring0
	lea	esi, [l.eip - tss_sceleton + eax]
	shr	ecx, 2
	rep	movsd
	jmp	.ret
.ring0:
; note that following code assumes that all interrupt/exception handlers
; saves ring-3 context by push ds es, pushad in this order
	mov	esi, [l.esp0 - tss_sceleton + eax]
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
	cmp	[l.cs - tss_sceleton + eax], app_code
	jnz	.ring0
	lea	edi, [l.eip - tss_sceleton + eax]
	shr	ecx, 2
	rep	movsd
	jmp	.stiret
.ring0:
	mov	edi, [l.esp0 - tss_sceleton + eax]
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
	mov	ebp, eax
	shl	eax, 8
	mov	edi, [timer_ticks]
	add	edi, 500	; 5 sec timeout
.1:
	mov	edx, [0x80000+eax+0xBC]
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
	cmp	[timer_ticks], edi
	jae	.ret
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
	or	byte [0x80000+eax+0xA8+1], 1	; set flag 100h
.ret:
	ret
