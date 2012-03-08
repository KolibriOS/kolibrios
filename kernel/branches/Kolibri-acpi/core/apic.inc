;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


iglobal
IRQ_COUNT    dd 24
endg

uglobal
irq_mode     rd  1
IOAPIC_base  rd  1
LAPIC_BASE   rd  1
endg

APIC_ID         equ 0x20
APIC_TPR        equ 0x80
APIC_EOI        equ 0xb0
APIC_LDR        equ 0xd0
APIC_DFR        equ 0xe0
APIC_SVR        equ 0xf0
APIC_ISR        equ 0x100
APIC_ESR        equ 0x280
APIC_ICRL       equ 0x300
APIC_ICRH       equ 0x310
APIC_LVT_LINT0	equ	0x350
APIC_LVT_LINT1	equ	0x360
APIC_LVT_err    equ 0x370

; APIC timer
APIC_LVT_timer	equ	0x320
APIC_timer_div	equ	0x3e0
APIC_timer_init	equ	0x380
APIC_timer_cur	equ	0x390
; IOAPIC
IOAPIC_ID               equ  0x0
IOAPIC_VER              equ  0x1
IOAPIC_ARB              equ  0x2
IOAPIC_REDTBL           equ  0x10

IPI_INIT                equ  (0x6 shl 8)
IPI_LEVEL_ASSERT        equ  (0x1 shl 14)
SHORTHAND_ALL_EXCL      equ  (0x3 shl 18)

CMD_IPI_INIT            equ  (IPI_INIT+SHORTHAND_ALL_EXCL)



align 4
APIC_init:
        mov     [irq_mode], IRQ_PIC

        cmp     [acpi_ioapic_base], 0
        jz      .no_apic

        cmp     [acpi_lapic_base], 0
        jz      .no_apic

        stdcall load_file, dev_data_path
        test    eax, eax
        jz      .no_apic

        mov     [acpi_dev_data], eax
        mov     [acpi_dev_size], ebx

        call    IRQ_mask_all

; IOAPIC init
        stdcall map_io_mem, [acpi_ioapic_base], 0x20, PG_SW
        mov     [IOAPIC_base], eax

        mov eax, IOAPIC_VER
        call    IOAPIC_read
        shr     eax, 16
        inc     al
        movzx eax, al
        cmp     al, IRQ_RESERVED
        jbe     @f

        mov     al, IRQ_RESERVED
@@:
        mov     [IRQ_COUNT], eax

	; Reroute IOAPIC & mask all interrupts
        xor     ecx, ecx
        mov     eax, IOAPIC_REDTBL
@@:
        mov     ebx, eax
        call    IOAPIC_read
        mov     ah, 0x09; Delivery Mode: Lowest Priority, Destination Mode: Logical
        mov     al, cl
        add     al, 0x20; vector
        or      eax, 0x10000; Mask Interrupt
        cmp     ecx, 16
        jb      .set

        or      eax, 0xa000;<<< level-triggered active-low for IRQ16+
.set:
        xchg    eax, ebx
        call    IOAPIC_write
        inc     eax
        mov     ebx, eax
        call    IOAPIC_read
        or      eax, 0xff000000; Destination Field
        xchg    eax, ebx
        call    IOAPIC_write
        inc     eax
        inc     ecx
        cmp     ecx, [IRQ_COUNT]
        jb      @b

        call    LAPIC_init

        mov     [irq_mode], IRQ_APIC

        mov     al, 0x70
        out     0x22, al
        mov     al, 1
        out     0x23, al

        call    pci_irq_fixup
.no_apic:

        ret

;===========================================================
align 4
LAPIC_init:
	; Check MSR support
	;....
	; Get LAPIC base address
 ;mov ecx, 0x1b
 ;rdmsr       ; it may be replaced to
 ;and ax, 0xf000    ; mov eax, 0xfee00000

        mov [acpi_lapic_base], 0xfee00000
 ;       xchg bx, bx

        stdcall map_io_mem, [acpi_lapic_base], 0x1000, PG_SW
        mov     [LAPIC_BASE], eax
        mov     esi, eax

	; Program Destination Format Register for Flat mode.
        mov     eax, [esi + APIC_DFR]
        or      eax, 0xf0000000
        mov     [esi + APIC_DFR], eax

	; Program Logical Destination Register.
        mov     eax, [esi + APIC_LDR]
        ;and  eax, 0xff000000
        and     eax, 0x00ffffff
        or      eax, 0x01000000;!!!!!!!!!!!!
        mov     [esi + APIC_LDR], eax

	; Task Priority Register initialization.
        mov     eax, [esi + APIC_TPR]
        and     eax, 0xffffff00
        mov     [esi + APIC_TPR], eax

	; Flush the queue
        mov     edx, 0
.nxt2:
        mov     ecx, 32
        mov     eax, [esi + APIC_ISR + edx]
.nxt:
        shr     eax, 1
        jnc      @f
        mov     dword [esi + APIC_EOI], 0; EOI
@@:
        loop    .nxt

        add     edx, 0x10
        cmp     edx, 0x170
        jbe     .nxt2

	; Spurious-Interrupt Vector Register initialization.
        mov     eax, [esi + APIC_SVR]
        or      eax, 0x1ff
        and     eax, 0xfffffdff
        mov     [esi + APIC_SVR], eax

	; Initialize LVT LINT0 register. (INTR)
        mov     eax, 0x00700
        ; mov eax, 0x10700
        mov     [esi + APIC_LVT_LINT0], eax

	; Initialize LVT LINT1 register. (NMI)
        mov     eax, 0x00400
        mov     [esi + APIC_LVT_LINT1], eax

	; Initialize LVT Error register.
        mov     eax, [esi + APIC_LVT_err]
        or      eax, 0x10000; bit 16
        mov     [esi + APIC_LVT_err], eax

	; LAPIC timer
	; pre init
;        mov     dword[esi + APIC_timer_div], 1011b; 1
;        mov     dword[esi + APIC_timer_init], 0xffffffff; max val
;        push  esi
;        mov esi, 640        ; wait 0.64 sec
;        call  delay_ms
;        pop esi
;        mov     eax, [esi + APIC_timer_cur]; read current tick couner
;        xor eax, 0xffffffff       ; eax = 0xffffffff - eax
;        shr eax, 6          ; eax /= 64; APIC ticks per 0.01 sec

	; Start (every 0.01 sec)
;        mov     dword[esi + APIC_LVT_timer], 0x30020; periodic int 0x20
;        mov dword[esi + APIC_timer_init], eax
        xchg bx, bx

;        mov     al, 0x0F
;        out     0x70, al
;        mov     al, 0x0A
;        out     0x71, al

;        mov     [OS_BASE+0x469], word (__ap_start_16) shr 4
;        mov     [OS_BASE+0x469], word 0

        mov     [esi+APIC_ICRH], dword 0
        mov     [esi+APIC_ICRL], dword 0xc4500

        mov ecx, 1000
@@:
        loop @B

        mov     [esi+APIC_ICRH], dword 0
        mov     [esi+APIC_ICRL], dword 0xC4600+((0x10000+__ap_start_16) shr 12)
;        mov     [esi+APIC_ICRL], dword 0xC4612

;        mov     [esi+APIC_ICRH], dword 0
;        mov     [esi+APIC_ICRL], dword CMD_IPI_INIT+IPI_LEVEL_ASSERT+16



        ret

;===========================================================
; IOAPIC implementation
align 4
IOAPIC_read:
; in : EAX - IOAPIC register
; out: EAX - readed value
        push  esi
        mov esi, [IOAPIC_base]
        mov [esi], eax
        mov eax, [esi + 0x10]
        pop esi
        ret

align 4
IOAPIC_write:
; in :	EAX - IOAPIC register
;	EBX - value
; out:	none
        push  esi
        mov esi, [IOAPIC_base]
        mov [esi], eax
        mov [esi + 0x10], ebx
        pop esi
        ret
;===========================================================
; Remap all IRQ to 0x20+ Vectors
; IRQ0 to vector 0x20, IRQ1 to vector 0x21....
align 4
PIC_init:
        cli
        mov     al, 0x11        ;  icw4, edge triggered
        out     0x20, al
        out     0xA0, al

        mov     al, 0x20        ;  generate 0x20 +
        out     0x21, al
        mov     al, 0x28        ;  generate 0x28 +
        out     0xA1, al

        mov     al, 0x04        ;  slave at irq2
        out     0x21, al
        mov     al, 0x02        ;  at irq9
        out     0xA1, al

        mov     al, 0x01        ;  8086 mode
        out     0x21, al
        out     0xA1, al

        call  IRQ_mask_all
	; mov	dword[irq_type_to_set], IRQ_TYPE_PIC
        ret

; -----------------------------------------
; TIMER SET TO 1/100 S
align 4
PIT_init:
        mov     al, 0x34   ; set to 100Hz
        out     0x43, al
        mov     al, 0x9b   ; lsb    1193180 / 1193
        out     0x40, al
        mov     al, 0x2e   ; msb
        out     0x40, al
        ret

; -----------------------------------------
align 4
unmask_timer:
        cmp  [irq_mode], IRQ_APIC
        je @f

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
        mov esi, [LAPIC_BASE]
        mov eax, [esi + APIC_LVT_timer]
        and eax, 0xfffeffff
        mov [esi + APIC_LVT_timer], eax
        ret

; -----------------------------------------
; Disable all IRQ
align 4
IRQ_mask_all:
        cmp [irq_mode], IRQ_APIC
        je .APIC

        mov al, 0xFF
        out 0x21, al
        out 0xA1, al
        mov     ecx, 0x1000
        ret
.APIC:
        mov ecx, [IRQ_COUNT]
        mov eax, 0x10
@@:
        mov ebx, eax
        call  IOAPIC_read
        or      eax, 0x10000; bit 16
        xchg  eax, ebx
        call  IOAPIC_write
        inc eax
        inc eax
        loop  @b
        ret

; -----------------------------------------
; End Of Interrupt
; cl - IRQ number
align 4
irq_eoi:         ; __fastcall
        cmp [irq_mode], IRQ_APIC
        je .APIC

        cmp cl, 8
        mov al, 0x20
        jb  @f
        out 0xa0, al
@@:
        out 0x20, al
        ret

.APIC:
        mov eax, [LAPIC_BASE]
        mov     dword [eax + APIC_EOI], 0; EOI
        ret

; -----------------------------------------
; from dll.inc
align 4
proc enable_irq stdcall, irq_line:dword
        mov ebx, [irq_line]
        cmp [irq_mode], IRQ_APIC
        je .APIC

        mov edx, 0x21
        cmp ebx, 8
        jb  @F

        mov edx, 0xA1
        sub     ebx, 8
@@:
        in      al, dx
        btr eax, ebx
        out dx, al
        ret
.APIC:
        shl ebx, 1
        add ebx, 0x10
        mov eax, ebx
        call  IOAPIC_read
        and     eax, 0xfffeffff; bit 16
        xchg  eax, ebx
        call  IOAPIC_write
        ret
endp

align 4
pci_irq_fixup:

        push ebp

        mov esi, [acpi_dev_data]
        mov ebx, [acpi_dev_size]

        lea edi, [esi+ebx]

.iterate:

        cmp esi, edi
        jae .done

        mov eax, [esi]

        cmp eax, -1
        je .done

        movzx ebx, al
        movzx ebp, ah

        stdcall pci_read32, ebp, ebx, 0

        cmp eax, [esi+4]
        jne .skip

        mov eax, [esi+8]
        stdcall pci_write8, ebp, ebx, 0x3C, eax
.skip:
        add esi, 16
        jmp .iterate

.done:
.fail:
        pop ebp
        ret





