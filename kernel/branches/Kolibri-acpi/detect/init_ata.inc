;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2014-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 5147 $


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
;        shr     eax, 4
;        cmp     eax, 0x01018
        shr     eax, 7
        cmp     eax, 0x010180 shr 7
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
        mov     dx, 0x1F0
        test    byte [esi+PCIDEV.class], 1
        jz      @f
        mov     bl, 0x10
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR0 IDE base addr %x\n', dx
        mov     [StandardATABases], dx
        mov     [ecx+IDE_DATA.BAR0_val], dx
;--------------------------------------
        mov     dx, 0x3F4
        test    byte [esi+PCIDEV.class], 1
        jz      @f
        mov     bl, 0x14
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR1 IDE base addr %x\n', dx
        mov     [ecx+IDE_DATA.BAR1_val], dx
;--------------------------------------
        mov     dx, 0x170
        test    byte [esi+PCIDEV.class], 4
        jz      @f
        mov     bl, 0x18
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR2 IDE base addr %x\n', dx
        mov     [StandardATABases+2], dx
        mov     [ecx+IDE_DATA.BAR2_val], dx
;--------------------------------------
        mov     dx, 0x374
        test    byte [esi+PCIDEV.class], 4
        jz      @f
        mov     bl, 0x1C
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR3 IDE base addr %x\n', dx
        mov     [ecx+IDE_DATA.BAR3_val], dx
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
IDE_device_1 rd 2
IDE_device_2 rd 2
IDE_device_3 rd 2
;--------------------------------------
endg
;-----------------------------------------------------------------------------
; START of initialisation IDE ATA code
;-----------------------------------------------------------------------------
Init_IDE_ATA_controller:
        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
        jne     @f

        ret
;--------------------------------------
@@:
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
;--------------------------------------
include 'dev_hdcd.inc'
;--------------------------------------
        ret
;-----------------------------------------------------------------------------
Init_IDE_ATA_controller_2:
        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
        jne     @f

        ret
;--------------------------------------
@@:
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
;        push    eax edx
;        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
;        xor     eax, eax
;        add     dx, 2
;        in      al, dx
;        DEBUGF  1, "K : Primary Bus Master IDE Status Register %x\n", eax

;        add     dx, 8
;        in      al, dx
;        DEBUGF  1, "K : Secondary Bus Master IDE Status Register %x\n", eax
;        pop     edx eax

;        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
;        setnz   [ecx+IDE_DATA.dma_hdd]
;-----------------------------------------------------------------------------
; set interrupts for IDE Controller
;-----------------------------------------------------------------------------
        pushfd
        cli
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
        test    ah, 10100000b ; check for ATAPI devices
        jz      @f
;--------------------------------------
.ch1_pio_set_ATAPI:
        DEBUGF  1, "K : IDE CH1 PIO, because ATAPI drive present\n"
        jmp     .ch1_pio_set_for_all
;--------------------------------------
.ch1_pio_set_no_devices:
        DEBUGF  1, "K : IDE CH1 PIO because no devices\n"
        jmp     .ch1_pio_set_for_all
;-------------------------------------
.ch1_pio_set:
        DEBUGF  1, "K : IDE CH1 PIO because device not support UDMA\n"
;-------------------------------------
.ch1_pio_set_for_all:
        mov     [ecx+IDE_DATA.dma_hdd_channel_1], al
        jmp     .ch2_check
;--------------------------------------
@@:
        xor     ebx, ebx
        call    calculate_IDE_device_values_storage

        test    ah, 1010000b
        jz      .ch1_pio_set_no_devices

        test    ah, 1000000b
        jz      @f

        cmp     [ebx+IDE_DEVICE.UDMA_possible_modes], al
        je      .ch1_pio_set

        cmp     [ebx+IDE_DEVICE.UDMA_set_mode], al
        je      .ch1_pio_set
;--------------------------------------
@@:
        test    ah, 10000b
        jz      @f

        add     ebx, 2

        cmp     [ebx+IDE_DEVICE.UDMA_possible_modes], al
        je      .ch1_pio_set

        cmp     [ebx+IDE_DEVICE.UDMA_set_mode], al
        je      .ch1_pio_set
;--------------------------------------
@@:
        mov     dx, [ecx+IDE_DATA.BAR1_val] ;0x3F4
        add     dx, 2 ;0x3F6
        out     dx, al
        DEBUGF  1, "K : IDE CH1 DMA enabled\n"
        mov     [ecx+IDE_DATA.dma_hdd_channel_1], byte 1
;--------------------------------------
.ch2_check:
        test    ah, 1010b ; check for ATAPI devices
        jz      @f
;--------------------------------------
.ch2_pio_set_ATAPI:
        DEBUGF  1, "K : IDE CH2 PIO, because ATAPI drive present\n"
        jmp     .ch2_pio_set_for_all
;--------------------------------------
.ch2_pio_set_no_devices:
        DEBUGF  1, "K : IDE CH2 PIO because no devices\n"
        jmp     .ch2_pio_set_for_all
;--------------------------------------
.ch2_pio_set:
        DEBUGF  1, "K : IDE CH2 PIO because device not support UDMA\n"
;--------------------------------------
.ch2_pio_set_for_all:
        mov     [ecx+IDE_DATA.dma_hdd_channel_2], al
        jmp     .set_interrupts_for_IDE_controllers
;--------------------------------------
@@:
        mov     ebx, 4
        call    calculate_IDE_device_values_storage

        test    ah, 101b
        jz      .ch2_pio_set_no_devices

        test    ah, 100b
        jz      @f

        cmp     [ebx+IDE_DEVICE.UDMA_possible_modes], al
        je      .ch2_pio_set

        cmp     [ebx+IDE_DEVICE.UDMA_set_mode], al
        je      .ch2_pio_set
;--------------------------------------
@@:
        test    ah, 1b
        jz      @f

        add     ebx, 2

        cmp     [ebx+IDE_DEVICE.UDMA_possible_modes], al
        je      .ch2_pio_set

        cmp     [ebx+IDE_DEVICE.UDMA_set_mode], al
        je      .ch2_pio_set
;--------------------------------------
@@:
        mov     dx, [ecx+IDE_DATA.BAR3_val] ;0x374
        add     dx, 2 ;0x376
        out     dx, al
        DEBUGF  1, "K : IDE CH2 DMA enabled\n"
        mov     [ecx+IDE_DATA.dma_hdd_channel_2], byte 1
;--------------------------------------
.set_interrupts_for_IDE_controllers:
        mov     esi, boot_set_int_IDE
        call    boot_log
;--------------------------------------
        mov     eax, [ecx+IDE_DATA.ProgrammingInterface]
;        cmp     ax, 0x0180
;        je      .pata_ide

;        cmp     ax, 0x018a
;        jne     .sata_ide

        test    al, 1 ; 0 - legacy PCI mode, 1 - native PCI mode
        jnz     .sata_ide
;--------------------------------------
.pata_ide:
        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
        je      .end_set_interrupts

        push    ecx
        stdcall attach_int_handler, 14, IDE_irq_14_handler, ecx
        pop     ecx
        DEBUGF  1, "K : Set IDE IRQ14 return code %x\n", eax
        push    ecx
        stdcall attach_int_handler, 15, IDE_irq_15_handler, ecx
        DEBUGF  1, "K : Set IDE IRQ15 return code %x\n", eax
        pop     ecx

        jmp     .end_set_interrupts
;--------------------------------------
.sata_ide:
;        cmp     ax, 0x0185
;        je      .sata_ide_1

;        cmp     ax, 0x018f
;        jne     .end_set_interrupts
;--------------------------------------
;.sata_ide_1:
; Some weird controllers generate an interrupt even if IDE interrupts
; are disabled and no IDE devices. For example, notebook ASUS K72F -
; IDE controller 010185 generates false interrupt when we work with
; the IDE controller 01018f. For this reason, the interrupt handler
; does not need to be installed if both channel IDE controller
; running in PIO mode.

; ...unfortunately, PCI interrupt can be shared with other devices
; which could enable it without consulting IDE code.
; So install the handler anyways and try to process
; even those interrupts which we are not expecting.
        cmp     [ecx+IDE_DATA.RegsBaseAddres], 0
        je      .end_set_interrupts

        mov     ax, [ecx+IDE_DATA.Interrupt]
        movzx   eax, al
        push    ecx
        stdcall attach_int_handler, eax, IDE_common_irq_handler, ecx
        pop     ecx
        DEBUGF  1, "K : Set IDE IRQ%d return code %x\n", [ecx+IDE_DATA.Interrupt]:1, eax
;--------------------------------------
.end_set_interrupts:
        popfd
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
        mov     esi, boot_init_sys
        call    boot_log
        call    Parser_params

if ~ defined extended_primary_loader
; ramdisk image should be loaded by extended primary loader if it exists
; READ RAMDISK IMAGE FROM HD
include '../boot/rdload.inc'
end if
;-----------------------------------------------------------------------------
        mov     ecx, IDE_controller_1
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller_2
        mov     ecx, IDE_controller_2
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller_2
        mov     ecx, IDE_controller_3
        mov     [IDE_controller_pointer], ecx
        call    Init_IDE_ATA_controller_2
;-----------------------------------------------------------------------------
