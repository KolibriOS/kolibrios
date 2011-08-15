;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2007-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IRQ_RESERVE = 24	; 16 or 24

iglobal
IRQ_COUNT	dd	24
endg

uglobal
APIC:		dd	0
LAPIC_BASE:	dd	0
IOAPIC_base:	dd	0
endg

APIC_ID		equ	0x20
APIC_TPR	equ	0x80
APIC_EOI	equ	0xb0
APIC_LDR	equ	0xd0
APIC_DFR	equ	0xe0
APIC_SVR	equ	0xf0
APIC_ISR	equ	0x100
APIC_ESR	equ	0x280
APIC_ICRL	equ	0x300
APIC_ICRH	equ	0x310
APIC_LVT_LINT0	equ	0x350
APIC_LVT_LINT1	equ	0x360
APIC_LVT_err	equ	0x370

; APIC timer
APIC_LVT_timer	equ	0x320
APIC_timer_div	equ	0x3e0
APIC_timer_init	equ	0x380
APIC_timer_cur	equ	0x390
; IOAPIC
IOAPIC_ID	equ	0x0
IOAPIC_VER	equ	0x1
IOAPIC_ARB	equ	0x2
IOAPIC_REDTBL	equ	0x10

APIC_init:
	mov	dword[APIC], 0
	; jmp	.no_apic		; NO APIC
;	bt	[cpu_caps], CAPS_APIC
;	jnc	.no_apic
	; Check for MP table
	;.....

	call	IRQ_mask_all

	; IOAPIC init
	; 0xfec00000 - may be relocated, see chip reference... & MP table
	stdcall	map_io_mem, 0xfec00000, 0x20, PG_SW
	mov	[IOAPIC_base], eax

	mov	eax, IOAPIC_VER
	call	IOAPIC_read
	shr	eax, 16
	inc	al
	movzx	eax, al
	cmp	al, IRQ_RESERVE
	jbe	@f
	mov	al, IRQ_RESERVE
@@:	mov	[IRQ_COUNT], eax

	; Reroute IOAPIC & mask all interrupts
	xor	ecx, ecx
	mov	eax, IOAPIC_REDTBL
@@:	mov	ebx, eax
	call	IOAPIC_read
	mov	ah, 0x09		; Delivery Mode: Lowest Priority, Destination Mode: Logical
	mov	al, cl
	add	al, 0x20		; vector
	or	eax, 0x10000		; Mask Interrupt
	cmp	ecx, 16
	jb	.set
	or	eax, 0xa000		;<<< level-triggered active-low for IRQ16+
.set:
	xchg	eax, ebx
	call	IOAPIC_write
	inc	eax
	mov	ebx, eax
	call	IOAPIC_read
	or	eax, 0xff000000		; Destination Field
	xchg	eax, ebx
	call	IOAPIC_write
	inc	eax
	inc	ecx
	cmp	ecx, [IRQ_COUNT]
	jb	@b

	call	LAPIC_init

	mov	dword[APIC], 0xffffffff

    mov al, 0x70
    out 0x22, al
    mov al, 1
    out 0x23, al

	; mov	dword[irq_type_to_set], IRQ_TYPE_APIC

;init handlers table

    mov ecx, IRQ_RESERVE
    mov edi, irqh_tab
@@:
    mov eax, edi
    stosd
    stosd
    loop @B

    mov ecx, 48
    mov eax, irqh_array+IRQH.sizeof
    mov [next_irqh], irqh_array

@@:
    mov [eax-IRQH.sizeof], eax
    add eax, IRQH.sizeof
    loop @B

    mov [eax-IRQH.sizeof], dword 0

.no_apic:
	ret

;===========================================================
align 4
LAPIC_init:
	; Check MSR support
	;....
	; Get LAPIC base address
	mov	ecx, 0x1b
	rdmsr				; it may be replaced to
	and	ax, 0xf000		; mov	eax, 0xfee00000

	stdcall	map_io_mem, eax, 0x1000, PG_SW
	mov	[LAPIC_BASE], eax
	mov	esi, eax

	; Program Destination Format Register for Flat mode.
	mov	eax, [esi + APIC_DFR]
	or	eax, 0xf0000000
	mov	[esi + APIC_DFR], eax


	; Program Logical Destination Register.
	mov	eax, [esi + APIC_LDR]
	;and	eax, 0xff000000
	and	eax, 0x00ffffff
	or	eax, 0x01000000	;!!!!!!!!!!!!
	mov	[esi + APIC_LDR], eax

	; Task Priority Register initialization.
	mov	eax, [esi + APIC_TPR]
	and	eax, 0xffffff00
	mov	[esi + APIC_TPR], eax

	; Flush the queue
	mov	edx, 0
.nxt2:	mov	ecx, 32
	mov	eax, [esi + APIC_ISR + edx]
.nxt:	shr	eax, 1
	jnc	@f
	mov	dword [esi + APIC_EOI], 0	; EOI
@@:	loop	.nxt
	add	edx, 0x10
	cmp	edx, 0x170
	jbe	.nxt2

	; Spurious-Interrupt Vector Register initialization.
	mov	eax, [esi + APIC_SVR]
	or	eax, 0x1ff
	and	eax, 0xfffffdff
	mov	[esi + APIC_SVR], eax

	; Initialize LVT LINT0 register. (INTR)
	mov	eax, 0x00700
	; mov	eax, 0x10700
	mov	[esi + APIC_LVT_LINT0], eax

	; Initialize LVT LINT1 register. (NMI)
	mov	eax, 0x00400
	mov	[esi + APIC_LVT_LINT1], eax

	; Initialize LVT Error register.
	mov	eax, [esi + APIC_LVT_err]
	or	eax, 0x10000		; bit 16
	mov	[esi + APIC_LVT_err], eax

	; LAPIC timer
	; pre init
	mov	dword[esi + APIC_timer_div], 1011b	; 1
	mov	dword[esi + APIC_timer_init], 0xffffffff	; max val
	push	esi
	mov	esi, 640				; wait 0.64 sec
	call	delay_ms
	pop	esi
	mov	eax, [esi + APIC_timer_cur]		; read current tick couner
	xor	eax, 0xffffffff				; eax = 0xffffffff - eax
	shr	eax, 6					; eax /= 64; APIC ticks per 0.01 sec

	; Start (every 0.01 sec)
	mov	dword[esi + APIC_LVT_timer], 0x30020	; periodic int 0x20
	mov	dword[esi + APIC_timer_init], eax

	ret
;===========================================================
; IOAPIC implementation
align 4
IOAPIC_read:
; in : EAX - IOAPIC register
; out: EAX - readed value
	push	esi
	mov	esi, [IOAPIC_base]
	mov	[esi], eax
	mov	eax, [esi + 0x10]
	pop	esi
	ret
align 4
IOAPIC_write:
; in :	EAX - IOAPIC register
;	EBX - value
; out:	none
	push	esi
	mov	esi, [IOAPIC_base]
	mov	[esi], eax
	mov	[esi + 0x10], ebx
	pop	esi
	ret
;===========================================================
; Remap all IRQ to 0x20+ Vectors
; IRQ0 to vector 0x20, IRQ1 to vector 0x21....
PIC_init:
	cli
	mov	al,0x11 	;  icw4, edge triggered
	out	0x20,al
	call	pic_delay
	out	0xA0,al
	call	pic_delay

	mov	al,0x20 	;  generate 0x20 +
	out	0x21,al
	call	pic_delay
	mov	al,0x28 	;  generate 0x28 +
	out	0xA1,al
	call	pic_delay

	mov	al,0x04 	;  slave at irq2
	out	0x21,al
	call	pic_delay
	mov	al,0x02 	;  at irq9
	out	0xA1,al
	call	pic_delay

	mov	al,0x01 	;  8086 mode
	out	0x21,al
	call	pic_delay
	out	0xA1,al
	call	pic_delay

	call	IRQ_mask_all
	cli
	; mov	dword[irq_type_to_set], IRQ_TYPE_PIC
	ret

; -----------------------------------------
pic_delay:
	jmp	pdl1
pdl1:	ret

; -----------------------------------------
; TIMER SET TO 1/100 S
PIT_init:
	mov   al,0x34		   ; set to 100Hz
	out   0x43,al
	mov   al,0x9b		   ; lsb    1193180 / 1193
	out   0x40,al
	mov   al,0x2e		   ; msb
	out   0x40,al
	ret

; -----------------------------------------
unmask_timer:
	test	dword[APIC], 0xffffffff
	jnz	@f
	stdcall enable_irq, 0
	ret
@@:
	; use PIT
	; in some systems PIT no connected to IOAPIC
	; mov	eax, 0x14
	; call	IOAPIC_read
	; mov	ah, 0x09		; Delivery Mode: Lowest Priority, Destination Mode: Logical
	; mov	al, 0x20
	; or	eax, 0x10000		; Mask Interrupt
	; mov	ebx, eax
	; mov	eax, 0x14
	; call	IOAPIC_write
	; stdcall enable_irq, 2
	; ret

	; use LAPIC timer
	mov	esi, [LAPIC_BASE]
	mov	eax, [esi + APIC_LVT_timer]
	and	eax, 0xfffeffff
	mov	[esi + APIC_LVT_timer], eax
	ret

; -----------------------------------------
; Disable all IRQ
IRQ_mask_all:
	test	dword[APIC], 0xffffffff
	jnz	.APIC
	mov	al, 0xFF
	out	0x21, al
	out	0xA1, al
	mov	ecx,0x1000
	cld
@@:	call	pic_delay
	loop	@b
	ret
.APIC:
	mov	ecx, [IRQ_COUNT]
	mov	eax, 0x10
@@:	mov	ebx, eax
	call	IOAPIC_read
	or	eax, 0x10000		; bit 16
	xchg	eax, ebx
	call	IOAPIC_write
	inc	eax
	inc	eax
	loop	@b
	ret
; -----------------------------------------
; End Of Interrupt
; al - IRQ number
align 16
IRQ_EOI:
	test	dword[APIC], 0xffffffff
	jnz	.APIC
	cmp	al, 8
	mov	al, 0x20
	jb	@f
	out	0xa0, al
@@:	out	0x20, al
	ret
.APIC:
	mov	eax, [LAPIC_BASE]
	mov	dword [eax + APIC_EOI], 0	; EOI
	ret

; -----------------------------------------
; from dll.inc
align 4
proc enable_irq stdcall, irq_line:dword
	mov	ebx, [irq_line]
	test	dword[APIC], 0xffffffff
	jnz	.APIC
	mov	edx, 0x21
	cmp	ebx, 8
	jb	@F
	mov	edx, 0xA1
	sub	ebx,8
@@:
	in	al,dx
	btr	eax, ebx
	out	dx, al
	ret
.APIC:
	shl	ebx, 1
	add	ebx, 0x10
	mov	eax, ebx
	call	IOAPIC_read
	and	eax, 0xfffeffff		; bit 16
	xchg	eax, ebx
	call	IOAPIC_write
	ret
endp



; IRQ_TYPE_DISABLE	equ	0
; IRQ_TYPE_PIC		equ	1
; IRQ_TYPE_APIC		equ	2

; uglobal
; irq_type_to_set	rd	1
; irq_types	rd	IRQ_RESERVE
; endg

; -----------------------------------------
; End Of Interrupt
; al - IRQ number
; align 16
; IRQ_EOI:
	; movzx	eax, al
	; cmp	dword[irq_types + eax * 4], IRQ_TYPE_APIC
	; jne	@f
	; mov	eax, [LAPIC_BASE]
	; mov	dword [eax + APIC_EOI], 0	; EOI
	; ret
; @@:
	; cmp	al, 8
	; mov	al, 0x20
	; jb	@f
	; out	0xa0, al
; @@:	out	0x20, al
	; ret

; align 4
; proc enable_irq stdcall, irq_line:dword
	; cmp	dword[irq_type_to_set], IRQ_TYPE_APIC
	; jne	@f
	; stdcall	APIC_enable_irq, [irq_line]
	; ret
; @@:	stdcall	PIC_enable_irq, [irq_line]
	; ret
; endp

; align 4
; proc disable_irq stdcall, irq_line:dword
	; push	eax
	; mov	eax, [irq_line]
	; cmp	dword[irq_types + eax * 4], IRQ_TYPE_APIC
	; jne	@f
	; stdcall	APIC_disable_irq, eax
	; pop	eax
	; ret
; @@:	stdcall	PIC_disable_irq, eax
	; pop	eax
	; ret
; endp

; align 4
; proc PIC_enable_irq stdcall, irq_line:dword
	; pusha
	; mov	ebx, [irq_line]
	; mov	eax, [irq_types + ebx * 4]
	; cmp	eax, IRQ_TYPE_DISABLE
	; je	@f
	; cmp	eax, IRQ_TYPE_PIC
	; je	@f
	; stdcall	disable_irq, ebx
; @@:	mov	dword[irq_types + ebx * 4], IRQ_TYPE_PIC
	; mov	edx, 0x21
	; cmp	ebx, 8
	; jb	@F
	; mov	edx, 0xA1
	; sub	ebx,8
; @@:	in	al,dx
	; btr	eax, ebx
	; out	dx, al
	; popa
	; ret
; endp

; align 4
; proc PIC_disable_irq stdcall, irq_line:dword
	; pusha
	; mov	ebx, [irq_line]
	; mov	dword[irq_types + ebx * 4], IRQ_TYPE_DISABLE
	; mov	edx, 0x21
	; cmp	ebx, 8
	; jb	@F
	; mov	edx, 0xA1
	; sub	ebx,8
; @@:	in	al,dx
	; bts	eax, ebx
	; out	dx, al
	; popa
	; ret
; endp

; align 4
; proc APIC_enable_irq stdcall, irq_line:dword
	; pusha
	; mov	ebx, [irq_line]
	; mov	eax, [irq_types + ebx * 4]
	; cmp	eax, IRQ_TYPE_DISABLE
	; je	@f
	; cmp	eax, IRQ_TYPE_APIC
	; je	@f
	; stdcall	disable_irq, ebx
; @@:	mov	dword[irq_types + ebx * 4], IRQ_TYPE_APIC
	; shl	ebx, 1
	; add	ebx, 0x10
	; mov	eax, ebx
	; call	IOAPIC_read
	; and	eax, 0xfffeffff		; bit 16
	; xchg	eax, ebx
	; call	IOAPIC_write
	; popa
	; ret
; endp

; align 4
; proc APIC_disable_irq stdcall, irq_line:dword
	; pusha
	; mov	ebx, [irq_line]
	; mov	dword[irq_types + ebx * 4], IRQ_TYPE_DISABLE
	; shl	ebx, 1
	; add	ebx, 0x10
	; mov	eax, ebx
	; call	IOAPIC_read
	; or	eax, 0x10000
	; xchg	eax, ebx
	; call	IOAPIC_write
	; popa
	; ret
; endp


macro __list_add new, prev, next
{
    mov [next+LHEAD.prev], new
    mov [new+LHEAD.next], next
    mov [new+LHEAD.prev], prev
    mov [prev+LHEAD.next], new
}

macro list_add new, head
{
    mov eax, [head+LHEAD.next]
    __list_add new, head, eax
}

macro list_add_tail new, head
{
    mov eax, [head+LHEAD.prev]
    __list_add new, eax, head
}


align 4
proc attach_int_handler_ex stdcall, irq:dword, handler:dword, user_data:dword
         locals
           .irqh dd ?
         endl

         and [.irqh], 0

         push ebx

         mov  ebx, [irq]            ;irq num
         test ebx, ebx
         jz   .err

         cmp  ebx, IRQ_RESERVE
         jae  .err

         mov  edx, [handler]
         test edx, edx
         jz   .err

         pushfd
         cli

;allocate handler

         mov ecx, [next_irqh]
         test ecx, ecx
         jz .fail

         mov eax, [ecx]
         mov [next_irqh], eax

         mov [.irqh], ecx

         mov eax, [user_data]
         mov [ecx+IRQH.handler], edx
         mov [ecx+IRQH.data],    eax

         mov eax, [irqh_set]
         bt [pci_irq_set], ebx      ;check irq type
         jc .pci_irq

.isa_irq:
         bts eax, ebx               ;check for installed handler
         jc .fail

.set_handler:
         mov [irqh_set], eax

         lea edx, [irqh_tab+ebx*8]
         list_add_tail ecx, edx     ;clobber eax

         stdcall enable_irq, [irq]

.fail:
         popfd
.err:
         pop ebx
         mov eax, [.irqh]
         ret

.pci_irq:
         bts ecx, ebx               ;check for installed handler
         jmp .set_handler

endp

