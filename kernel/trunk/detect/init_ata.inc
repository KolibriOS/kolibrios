;-----------------------------------------------------------------------------
; find the IDE controller in the device list
;-----------------------------------------------------------------------------
        mov     ecx, IDE_controller_1
        mov     esi, pcidev_list
;--------------------------------------
align 4
.loop:
        mov     esi, [esi+PCIDEV.fd]
        cmp     esi, pcidev_list
        jz      find_IDE_controller_done

        mov     eax, [esi+PCIDEV.class]
        shr     eax, 4
        cmp     eax, 0x01018
        jnz     .loop
;--------------------------------------
.found:
        mov     eax, [esi+PCIDEV.class]
        DEBUGF  1, 'K : IDE controller programming interface %x\n', eax
        mov     [ecx+IDE_DATA.ProgrammingInterface], eax

        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        mov     bh, [esi+PCIDEV.devfn]
;--------------------------------------
        mov     bl, 0x10
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f

        cmp     ax, 1
        jne     .show_BAR0
;--------------------------------------
@@:
        mov     ax, 0x1F0
;--------------------------------------
.show_BAR0:
        DEBUGF  1, 'K : BAR0 IDE base addr %x\n', ax
        mov     [StandardATABases], ax
        mov     [ecx+IDE_DATA.BAR0_val], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x14
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f

        cmp     ax, 1
        jne     .show_BAR1
;--------------------------------------
@@:
        mov     ax, 0x3F4
;--------------------------------------
.show_BAR1:
        DEBUGF  1, 'K : BAR1 IDE base addr %x\n', ax
        mov     [ecx+IDE_DATA.BAR1_val], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x18
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f

        cmp     ax, 1
        jne     .show_BAR2
;--------------------------------------
@@:
        mov     ax, 0x170
;--------------------------------------
.show_BAR2:
        DEBUGF  1, 'K : BAR2 IDE base addr %x\n', ax
        mov     [StandardATABases+2], ax
        mov     [ecx+IDE_DATA.BAR2_val], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x1C
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        cmp     ax, 0
        je      @f

        cmp     ax, 1
        jne     .show_BAR3
;--------------------------------------
@@:
        mov     ax, 0x374
;--------------------------------------
.show_BAR3:
        DEBUGF  1, 'K : BAR3 IDE base addr %x\n', ax
        mov     [ecx+IDE_DATA.BAR3_val], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x20
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        DEBUGF  1, 'K : BAR4 IDE controller register base addr %x\n', ax
        mov     [ecx+IDE_DATA.RegsBaseAddres], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x3C
        push    eax
        call    pci_read_reg
        and     eax, 0xFF
        DEBUGF  1, 'K : IDE Interrupt %x\n', al
        mov     [ecx+IDE_DATA.Interrupt], ax
        pop     eax

        add     ecx, sizeof.IDE_DATA
;--------------------------------------
        jmp     .loop
;-----------------------------------------------------------------------------
uglobal
align 4
;--------------------------------------
IDE_controller_pointer dd ?
;--------------------------------------
IDE_controller_1 IDE_DATA
IDE_controller_2 IDE_DATA
IDE_controller_3 IDE_DATA
;--------------------------------------
cache_ide0  IDE_CACHE
cache_ide1  IDE_CACHE
cache_ide2  IDE_CACHE
cache_ide3  IDE_CACHE
cache_ide4  IDE_CACHE
cache_ide5  IDE_CACHE
cache_ide6  IDE_CACHE
cache_ide7  IDE_CACHE
cache_ide8  IDE_CACHE
cache_ide9  IDE_CACHE
cache_ide10 IDE_CACHE
cache_ide11 IDE_CACHE
;--------------------------------------
endg
;-----------------------------------------------------------------------------
; START of initialisation IDE ATA code
;-----------------------------------------------------------------------------
Init_IDE_ATA_controller:
        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
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
        mov     dx, [ecx+IDE_DATA.BAR1_val] ;0x3F4
        add     dx, 2 ;0x3F6
        out     dx, al
        mov     dx, [ecx+IDE_DATA.BAR3_val] ;0x374
        add     dx, 2 ;0x376
        out     dx, al
;-----------------------------------------------------------------------------
; set current ata bases
@@:
        mov     ax, [ecx+IDE_DATA.BAR0_val]
        mov     [StandardATABases], ax
        mov     ax, [ecx+IDE_DATA.BAR2_val]
        mov     [StandardATABases+2], ax

        mov     esi, boot_detecthdcd
        call    boot_log
include 'dev_hdcd.inc'
;-----------------------------------------------------------------------------
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
; test whether it is our interrupt?
        add     dx, 2
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
;--------------------------------------
@@:
        add     dx, 8
; test whether it is our interrupt?
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
;--------------------------------------
@@:
; read status register and remove the interrupt request
        mov     dx, [ecx+IDE_DATA.BAR0_val] ;0x1F0
        add     dx, 0x7 ;0x1F7
        in      al, dx
        mov     dx, [ecx+IDE_DATA.BAR2_val] ;0x170
        add     dx, 0x7 ;0x177
        in      al, dx
;-----------------------------------------------------------------------------
        push    eax edx
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
        xor     eax, eax
        add     dx, 2
        in      al, dx
;        DEBUGF  1, "K : Primary Bus Master IDE Status Register %x\n", eax

        add     dx, 8
        in      al, dx
;        DEBUGF  1, "K : Secondary Bus Master IDE Status Register %x\n", eax
        pop     edx eax

        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
        setnz   [ecx+IDE_DATA.dma_hdd]
;-----------------------------------------------------------------------------
; set interrupts for IDE Controller
;-----------------------------------------------------------------------------
        mov     esi, boot_set_int_IDE
        call    boot_log
set_interrupts_for_IDE_controllers:
        mov     eax, [ecx+IDE_DATA.ProgrammingInterface]
        cmp     ax, 0x0180
        je      .pata_ide

        cmp     ax, 0x018a
        jne     .sata_ide
;--------------------------------------
.pata_ide:
        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
        je      .end_set_interrupts

        push    ecx
        stdcall attach_int_handler, 14, IDE_irq_14_handler, 0
        DEBUGF  1, "K : Set IDE IRQ14 return code %x\n", eax
        stdcall attach_int_handler, 15, IDE_irq_15_handler, 0
        DEBUGF  1, "K : Set IDE IRQ15 return code %x\n", eax
        pop     ecx

        jmp     .enable_IDE_interrupt
;--------------------------------------
.sata_ide:
        cmp     ax, 0x0185
        je      .sata_ide_1

        cmp     ax, 0x018f
        jne     .end_set_interrupts
;--------------------------------------
.sata_ide_1:
        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
        je      .end_set_interrupts

        mov     ax, [ecx+IDE_DATA.Interrupt]
        movzx   eax, al
        push    ecx
        stdcall attach_int_handler, eax, IDE_common_irq_handler, 0
        pop     ecx
        DEBUGF  1, "K : Set IDE IRQ%d return code %x\n", [ecx+IDE_DATA.Interrupt]:1, eax
;--------------------------------------
.enable_IDE_interrupt:
        mov     esi, boot_enabling_ide
        call    boot_log
; Enable interrupts in IDE controller for DMA
        xor     ebx, ebx
        cmp     ecx, IDE_controller_2
        jne     @f

        add     ebx, 5
        jmp     .check_DRIVE_DATA
;--------------------------------------
@@:
        cmp     ecx, IDE_controller_3
        jne     .check_DRIVE_DATA

        add     ebx, 10
;--------------------------------------
.check_DRIVE_DATA:
        mov     al, 0
        mov     ah, [ebx+DRIVE_DATA+1]
        test    ah, 10100000b
        jz      @f

        DEBUGF  1, "K : IDE CH1 PIO, because ATAPI drive present\n"
        jmp     .ch2_check
;--------------------------------------
@@:
        mov     dx, [ecx+IDE_DATA.BAR1_val] ;0x3F4
        add     dx, 2 ;0x3F6
        out     dx, al
        DEBUGF  1, "K : IDE CH1 DMA enabled\n"
;--------------------------------------
.ch2_check:
        test    ah, 1010b
        jz      @f

        DEBUGF  1, "K : IDE CH2 PIO, because ATAPI drive present\n"
        jmp     .end_set_interrupts
;--------------------------------------
@@:
        mov     dx, [ecx+IDE_DATA.BAR3_val] ;0x374
        add     dx, 2 ;0x376
        out     dx, al
        DEBUGF  1, "K : IDE CH2 DMA enabled\n"
;--------------------------------------
.end_set_interrupts:
;-----------------------------------------------------------------------------
        cmp     [ecx+IDE_DATA.dma_hdd], 0
        je      .print_pio
;--------------------------------------
.print_dma:
        DEBUGF  1, "K : IDE DMA mode\n"
        jmp     .continue
;--------------------------------------
.print_pio:
        DEBUGF  1, "K : IDE PIO mode\n"
.continue:
        ret
;-----------------------------------------------------------------------------
; END of initialisation IDE ATA code
;-----------------------------------------------------------------------------
find_IDE_controller_done:
        mov     ecx, IDE_controller_1
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller
        mov     ecx, IDE_controller_2
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller
        mov     ecx, IDE_controller_3
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller
;-----------------------------------------------------------------------------
        mov     esi, boot_getcache
        call    boot_log
include 'getcache.inc'
;-----------------------------------------------------------------------------
        mov     esi, boot_detectpart
        call    boot_log
include 'sear_par.inc'
;-----------------------------------------------------------------------------
