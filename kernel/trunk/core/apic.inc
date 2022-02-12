;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

MAX_IOAPICS = 2

uglobal
IRQ_COUNT    rd  MAX_IOAPICS
irq_mode     rd  1              ; PIC/(IO)APIC
IOAPIC_base  rd  MAX_IOAPICS
ioapic_gsi_base rd MAX_IOAPICS  ; zero-based, i.e. not vector
ioapic_cnt   dd  ?              ; from MADT aka APIC table
ioapic_cur   dd  ?
LAPIC_BASE   rd  1
endg

APIC_ID         = 0x20
APIC_TPR        = 0x80
APIC_EOI        = 0xb0
APIC_LDR        = 0xd0
APIC_DFR        = 0xe0
APIC_SVR        = 0xf0
APIC_ISR        = 0x100
APIC_ESR        = 0x280
APIC_ICRL       = 0x300
APIC_ICRH       = 0x310
APIC_LVT_LINT0  = 0x350
APIC_LVT_LINT1  = 0x360
APIC_LVT_err    = 0x370

; APIC timer
APIC_LVT_timer  = 0x320
APIC_timer_div  = 0x3e0
APIC_timer_init = 0x380
APIC_timer_cur  = 0x390
; IOAPIC
IOAPIC_ID       = 0x0
IOAPIC_VER      = 0x1
IOAPIC_ARB      = 0x2
IOAPIC_REDTBL   = 0x10

align 4
APIC_init:
        push    ebx
        mov     [irq_mode], IRQ_PIC

        cmp     [acpi_ioapic_base], 0
        jz      .no_apic

        cmp     [acpi_lapic_base], 0
        jz      .no_apic

; non-UEFI loaders don't load DEVICES.DAT and don't initialize [acpi_dev_size]
if defined UEFI
        cmp     [acpi_dev_size], 0
        jz      @f
        stdcall map_io_mem, [acpi_dev_data], [acpi_dev_size], PG_SWR
        mov     [acpi_dev_data], eax
        jmp     .loaded
@@:
end if

        stdcall load_file, dev_data_path
        test    eax, eax
        jz      .no_apic

        mov     [acpi_dev_data], eax
        mov     [acpi_dev_size], ebx
.loaded:

; IOAPIC init
        xor     ebx, ebx
@@:
        stdcall map_io_mem, [acpi_ioapic_base + ebx*4], 0x20, PG_GLOBAL + PG_NOCACHE + PG_SWR
        mov     [IOAPIC_base + ebx*4], eax
        inc     ebx
        cmp     ebx, [ioapic_cnt]
        jnz     @b

        call    IRQ_mask_all
        mov     [ioapic_cur], 0
.next_ioapic:
        mov     edx, [ioapic_cur]
        mov     eax, IOAPIC_VER
        call    IOAPIC_read
        shr     eax, 16
        inc     al
        movzx   eax, al
        mov     ecx, [ioapic_gsi_base + edx*4]
        cmp     ecx, IRQ_RESERVED
        jae     .lapic_init
        add     ecx, eax
        sub     ecx, IRQ_RESERVED
        jbe     @f
        sub     eax, ecx
@@:
        mov     [IRQ_COUNT + edx*4], eax

        ; Reroute IOAPIC & mask all interrupts
        xor     ecx, ecx
        mov     eax, IOAPIC_REDTBL
.next_irq:
        mov     ebx, eax
        call    IOAPIC_read
        mov     ah, 0x08; Delivery Mode: Fixed, Destination Mode: Logical
        mov     al, cl
        add     al, 0x20; vector
        add     eax, [ioapic_gsi_base + edx*4]
        or      eax, 0x1a000; Mask Interrupt, level-triggered active-low
        cmp     [ioapic_cur], 0
        jnz     @f
        cmp     ecx, 16
        jae     @f
        and     eax, NOT 0xa000 ; edge-triggered active-high for IRQ0-15
@@:
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
        cmp     ecx, [IRQ_COUNT + edx*4]
        jb      .next_irq

        inc     [ioapic_cur]
        inc     edx
        cmp     edx, [ioapic_cnt]
        jnz     .next_ioapic

.lapic_init:
        call    LAPIC_init

        mov     [irq_mode], IRQ_APIC

        mov     al, 0x70
        out     0x22, al
        mov     al, 1
        out     0x23, al

        call    pci_irq_fixup
.no_apic:
        pop     ebx
        ret

;===========================================================
align 4
LAPIC_init:

        mov     eax, [LAPIC_BASE]
        test    eax, eax
        jnz     @f
        stdcall map_io_mem, [acpi_lapic_base], 0x1000, PG_GLOBAL + PG_NOCACHE + PG_SWR
        mov     [LAPIC_BASE], eax
@@:
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
        jnc     @f
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
        mov     dword[esi + APIC_timer_div], 1011b; 1
        mov     dword[esi + APIC_timer_init], 0xffffffff; max val
        push    esi
        mov     esi, 640    ; wait 0.64 sec
        call    delay_ms
        pop     esi
        mov     eax, [esi + APIC_timer_cur]; read current tick couner
        xor     eax, 0xffffffff   ; eax = 0xffffffff - eax
        shr     eax, 6      ; eax /= 64; APIC ticks per 0.01 sec

        ; Start (every 0.01 sec)
        mov     dword[esi + APIC_LVT_timer], 0x30020; periodic int 0x20
        mov     dword[esi + APIC_timer_init], eax

.done:
        ret

;===========================================================
; IOAPIC implementation
align 4
IOAPIC_read:
; in : EAX - IOAPIC register
; out: EAX - readed value
        push    esi
        mov     esi, [ioapic_cur]
        mov     esi, [IOAPIC_base + esi*4]
        mov     [esi], eax
        mov     eax, [esi + 0x10]
        pop     esi
        ret

align 4
IOAPIC_write:
; in :  EAX - IOAPIC register
;       EBX - value
; out:  none
        push    esi
        mov     esi, [ioapic_cur]
        mov     esi, [IOAPIC_base + esi*4]
        mov     [esi], eax
        mov     [esi + 0x10], ebx
        pop     esi
        ret
;===========================================================
; Remap all IRQ to 0x20+ Vectors
; IRQ0 to vector 0x20, IRQ1 to vector 0x21....
align 4
PIC_init:
        mov     [IRQ_COUNT], 16
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

        call    IRQ_mask_all
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
        cmp     [irq_mode], IRQ_APIC
        je      @f

        stdcall enable_irq, 0
        ret
@@:
        ; use PIT
        ; in some systems PIT no connected to IOAPIC
        ; mov   eax, 0x14
        ; call  IOAPIC_read
        ; mov   ah, 0x09                ; Delivery Mode: Lowest Priority, Destination Mode: Logical
        ; mov   al, 0x20
        ; or    eax, 0x10000            ; Mask Interrupt
        ; mov   ebx, eax
        ; mov   eax, 0x14
        ; call  IOAPIC_write
        ; stdcall enable_irq, 2
        ; ret

        ; use LAPIC timer
        mov     esi, [LAPIC_BASE]
        mov     eax, [esi + APIC_LVT_timer]
        and     eax, 0xfffeffff
        mov     [esi + APIC_LVT_timer], eax
        ret

; -----------------------------------------
; Disable all IRQ
align 4
IRQ_mask_all:
        cmp     [irq_mode], IRQ_APIC
        je      .APIC

        mov     al, 0xFF
        out     0x21, al
        out     0xA1, al
        mov     ecx, 0x1000
        ret
.APIC:
        cmp     [IOAPIC_base], 0
        jz      .done
        mov     [ioapic_cur], 0
.next_ioapic:
        mov     edx, [ioapic_cur]
        mov     ecx, [IRQ_COUNT + edx*4]
        mov     eax, 0x10
@@:
        mov     ebx, eax
        call    IOAPIC_read
        or      eax, 0x10000; bit 16
        xchg    eax, ebx
        call    IOAPIC_write
        inc     eax
        inc     eax
        loop    @b

        inc     [ioapic_cur]
        inc     edx
        cmp     edx, [ioapic_cnt]
        jnz     .next_ioapic
.done:
        ret

; -----------------------------------------
; End Of Interrupt
; cl - IRQ number
align 4
irq_eoi:         ; __fastcall
        cmp     [irq_mode], IRQ_APIC
        je      .APIC

        cmp     cl, 8
        mov     al, 0x20
        jb      @f
        out     0xa0, al
@@:
        out     0x20, al
        ret

.APIC:
        mov     eax, [LAPIC_BASE]
        mov     dword [eax + APIC_EOI], 0; EOI
        ret

; -----------------------------------------
; from dll.inc
align 4
proc enable_irq stdcall, irq_line:dword
        mov     ebx, [irq_line]
        cmp     [irq_mode], IRQ_APIC
        je      .APIC

        mov     edx, 0x21
        cmp     ebx, 8
        jb      @F

        mov     edx, 0xA1
        sub     ebx, 8
@@:
        in      al, dx
        btr     eax, ebx
        out     dx, al
        ret
.APIC:
        push    [ioapic_cur]
        xor     eax, eax
.next_ioapic:
        mov     ecx, [ioapic_gsi_base+eax*4]
        add     ecx, [IRQ_COUNT+eax*4]
        cmp     ebx, ecx
        jb      .found
        inc     eax
        cmp     eax, [ioapic_cnt]
        jnz     .next_ioapic
        jmp     .done
.found:
        mov     [ioapic_cur], eax
        sub     ebx, [ioapic_gsi_base+eax*4]
        shl     ebx, 1
        add     ebx, 0x10
        mov     eax, ebx
        call    IOAPIC_read
        and     eax, 0xfffeffff; bit 16
        xchg    eax, ebx
        call    IOAPIC_write
.done:
        pop     [ioapic_cur]
        ret
endp

proc disable_irq stdcall, irq_line:dword
        mov     ebx, [irq_line]
        cmp     [irq_mode], IRQ_APIC
        je      .APIC

        mov     edx, 0x21
        cmp     ebx, 8
        jb      @F

        mov     edx, 0xA1
        sub     ebx, 8
@@:
        in      al, dx
        bts     eax, ebx
        out     dx, al
        ret
.APIC:
        push    [ioapic_cur]
        xor     eax, eax
.next_ioapic:
        mov     ecx, [ioapic_gsi_base + eax*4]
        add     ecx, [IRQ_COUNT + eax*4]
        cmp     ebx, ecx
        jae     .found
        inc     eax
        cmp     eax, [ioapic_cnt]
        jnz     .next_ioapic
        jmp     .done
.found:
        mov     [ioapic_cur], eax
        sub     ebx, [ioapic_gsi_base+eax*4]
        shl     ebx, 1
        add     ebx, 0x10
        mov     eax, ebx
        call    IOAPIC_read
        or      eax, 0x10000; bit 16
        xchg    eax, ebx
        call    IOAPIC_write
.done:
        pop     [ioapic_cur]
        ret
endp

align 4
pci_irq_fixup:

        push    ebp

        mov     esi, [acpi_dev_data]
        mov     ebx, [acpi_dev_size]

        lea     edi, [esi + ebx]

.iterate:

        cmp     esi, edi
        jae     .done

        mov     eax, [esi]

        cmp     eax, -1
        je      .done

        movzx   ebx, al
        movzx   ebp, ah

        stdcall pci_read32, ebp, ebx, 0

        cmp     eax, [esi+4]
        jne     .skip

        mov     eax, [esi+8]
        stdcall pci_write8, ebp, ebx, 0x3C, eax
.skip:
        add     esi, 16
        jmp     .iterate

.done:
.fail:
        pop     ebp
        ret

align 4
get_clock_ns:

        mov     eax, [hpet_base]
        test    eax, eax
        jz      .old_tics

        push    ebx
        push    esi
        pushfd
        cli

        mov     ebx, eax
@@:
        mov     edx, [ebx+0xF4]
        mov     eax, [ebx+0xF0]
        mov     ecx, [ebx+0xF4]
        cmp     ecx, edx
        jne     @B
        popfd

;96-bit arithmetic
;ebx - low dword
;esi - medium dword
;edx - high dword

        mul     [hpet_period]
        mov     ebx, eax
        mov     esi, edx

        mov     eax, ecx
        mul     [hpet_period]
        add     esi, eax
        adc     edx, 0
        mov     eax, ebx
        mov     ebx, esi
        shrd    eax, ebx, 10
        shrd    esi, edx, 10
        mov     edx, esi

        pop     esi
        pop     ebx
        ret

.old_tics:
        mov     eax, [timer_ticks]
        mov     edx, 10000000
        mul     edx
        ret
