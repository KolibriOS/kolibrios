;-----------------------------------------------------------------------------
; find the IDE controller in the device list
;-----------------------------------------------------------------------------
        mov     esi, pcidev_list
.loop:
        mov     esi, [esi+PCIDEV.fd]
        cmp     esi, pcidev_list
        jz      .done
        mov     eax, [esi+PCIDEV.class]
        shr     eax, 4
        cmp     eax, 0x01018
        jnz     .loop
.found:
        mov     eax, [esi+PCIDEV.class]
        DEBUGF  1, 'K : IDE controller programming interface %x\n', eax
        mov     [IDEContrProgrammingInterface], eax

        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        mov     bh, [esi+PCIDEV.devfn]
;-----------------------------------------------------------------------------
        mov     bl, 0x10
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .show_BAR0
@@:
        mov     ax, 0x1F0
.show_BAR0:
        DEBUGF  1, 'K : BAR0 IDE base addr %x\n', ax
        mov     [StandardATABases], ax
        mov     [hd_address_table], eax
        mov     [hd_address_table+8], eax
        mov     [IDE_BAR0_val], ax
        pop     eax
;-----------------------------------------------------------------------------
        mov     bl, 0x14
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .show_BAR1
@@:
        mov     ax, 0x3F4
.show_BAR1:
        DEBUGF  1, 'K : BAR1 IDE base addr %x\n', ax
        mov     [IDE_BAR1_val], ax
        pop     eax
;-----------------------------------------------------------------------------
        mov     bl, 0x18
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .show_BAR2
@@:
        mov     ax, 0x170
.show_BAR2:
        DEBUGF  1, 'K : BAR2 IDE base addr %x\n', ax
        mov     [StandardATABases+2], ax
        mov     [hd_address_table+16], eax
        mov     [hd_address_table+24], eax
        mov     [IDE_BAR2_val], ax
        pop     eax
;-----------------------------------------------------------------------------
        mov     bl, 0x1C
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .show_BAR3
@@:
        mov     ax, 0x374
.show_BAR3:
        DEBUGF  1, 'K : BAR3 IDE base addr %x\n', ax
        mov     [IDE_BAR3_val], ax
        pop     eax
;-----------------------------------------------------------------------------
        mov     bl, 0x20
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        DEBUGF  1, 'K : BAR4 IDE controller register base addr %x\n', ax
        mov     [IDEContrRegsBaseAddr], ax
        pop     eax
;-----------------------------------------------------------------------------
        mov     bl, 0x3C
        push    eax
        call    pci_read_reg
        and     eax, 0xFF
        DEBUGF  1, 'K : IDE Interrupt %x\n', al
        mov     [IDE_Interrupt], ax
        pop     eax
;-----------------------------------------------------------------------------
.done:
;-----------------------------------------------------------------------------
; START of initialisation IDE ATA code
;-----------------------------------------------------------------------------
        cmp     [IDEContrProgrammingInterface], 0
        je      set_interrupts_for_IDE_controllers.continue

        mov     esi, boot_disabling_ide
        call    boot_log
;--------------------------------------
; Disable IDE interrupts, because the search
; for IDE partitions is in the PIO mode.
;--------------------------------------
.disable_IDE_interrupt:
; Disable interrupts in IDE controller for PIO
        mov     al, 2
        mov     dx, [IDE_BAR1_val] ;0x3F4
        add     dx, 2 ;0x3F6
        out     dx, al
        mov     dx, [IDE_BAR3_val] ;0x374
        add     dx, 2 ;0x376
        out     dx, al
@@:
; show base variables of IDE controller 
;        DEBUGF  1, "K : BAR0 %x \n", [IDE_BAR0_val]:4
;        DEBUGF  1, "K : BAR1 %x \n", [IDE_BAR1_val]:4
;        DEBUGF  1, "K : BAR2 %x \n", [IDE_BAR2_val]:4
;        DEBUGF  1, "K : BAR3 %x \n", [IDE_BAR3_val]:4
;        DEBUGF  1, "K : BAR4 %x \n", [IDEContrRegsBaseAddr]:4
;        DEBUGF  1, "K : IDEContrProgrammingInterface %x \n", [IDEContrProgrammingInterface]:4
;        DEBUGF  1, "K : IDE_Interrupt %x \n", [IDE_Interrupt]:4
;-----------------------------------------------------------------------------
        mov     esi, boot_detecthdcd
        call    boot_log
include 'detect/dev_hdcd.inc'
        mov     esi, boot_getcache
        call    boot_log
include 'detect/getcache.inc'
        mov     esi, boot_detectpart
        call    boot_log
include 'detect/sear_par.inc'
;-----------------------------------------------------------------------------
        mov     dx, [IDEContrRegsBaseAddr]
; test whether it is our interrupt?
        add     dx, 2
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
@@:
        add     dx, 8
; test whether it is our interrupt?
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
@@:
; read status register and remove the interrupt request
        mov     dx, [IDE_BAR0_val] ;0x1F0
        add     dx, 0x7 ;0x1F7
        in      al, dx
        mov     dx, [IDE_BAR2_val] ;0x170
        add     dx, 0x7 ;0x177
        in      al, dx
;-----------------------------------------------------------------------------
        push    eax edx
        mov     dx, [IDEContrRegsBaseAddr]
        xor     eax, eax
        add     dx, 2
        in      al, dx
        DEBUGF  1, "K : Primary Bus Master IDE Status Register %x\n", eax

        add     dx, 8
        in      al, dx
        DEBUGF  1, "K : Secondary Bus Master IDE Status Register %x\n", eax
        pop     edx eax

        cmp     [IDEContrRegsBaseAddr], 0
        setnz   [dma_hdd]
;-----------------------------------------------------------------------------
; set interrupts for IDE Controller
;-----------------------------------------------------------------------------
        mov     esi, boot_set_int_IDE
        call    boot_log
set_interrupts_for_IDE_controllers:
        mov     eax, [IDEContrProgrammingInterface]
        cmp     ax, 0x0180
        je      .pata_ide

        cmp     ax, 0x018a
        jne     .sata_ide
;--------------------------------------
.pata_ide:
        cmp     [IDEContrRegsBaseAddr], 0
        je      .end_set_interrupts

        stdcall attach_int_handler, 14, IDE_irq_14_handler, 0
        DEBUGF  1, "K : Set IDE IRQ14 return code %x\n", eax
        stdcall attach_int_handler, 15, IDE_irq_15_handler, 0
        DEBUGF  1, "K : Set IDE IRQ15 return code %x\n", eax
        jmp     .enable_IDE_interrupt
;--------------------------------------
.sata_ide:
        cmp     ax, 0x0185
        je      .sata_ide_1

        cmp     ax, 0x018f
        jne     .end_set_interrupts
;--------------------------------------
.sata_ide_1:
        cmp     [IDEContrRegsBaseAddr], 0
        je      .end_set_interrupts

        mov     ax, [IDE_Interrupt]
        movzx   eax, al
        stdcall attach_int_handler, eax, IDE_common_irq_handler, 0
        DEBUGF  1, "K : Set IDE IRQ%d return code %x\n", [IDE_Interrupt]:1, eax
;--------------------------------------
.enable_IDE_interrupt:
        mov     esi, boot_enabling_ide
        call    boot_log
; Enable interrupts in IDE controller for DMA
        mov     al, 0
        mov     ah, [DRIVE_DATA+1]
        test    ah, 10100000b
        jz      @f

        DEBUGF  1, "K : IDE CH1 PIO, because ATAPI drive present\n"
        jmp     .ch2_check
@@:
        mov     dx, [IDE_BAR1_val] ;0x3F4
        add     dx, 2 ;0x3F6
        out     dx, al
        DEBUGF  1, "K : IDE CH1 DMA enabled\n"
.ch2_check:
        test    ah, 1010b
        jz      @f

        DEBUGF  1, "K : IDE CH2 PIO, because ATAPI drive present\n"
        jmp     .end_set_interrupts
@@:
        mov     dx, [IDE_BAR3_val] ;0x374
        add     dx, 2 ;0x376
        out     dx, al
        DEBUGF  1, "K : IDE CH2 DMA enabled\n"
;--------------------------------------
.end_set_interrupts:
;-----------------------------------------------------------------------------
        cmp     [dma_hdd], 0
        je      .print_pio
.print_dma:
        DEBUGF  1, "K : IDE DMA mode\n"
        jmp     .continue

.print_pio:
        DEBUGF  1, "K : IDE PIO mode\n"
.continue:
;-----------------------------------------------------------------------------
; END of initialisation IDE ATA code
;-----------------------------------------------------------------------------
