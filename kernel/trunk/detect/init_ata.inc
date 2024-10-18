;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2014-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struct  IDE_DEVICE
        UDMA_possible_modes  db ?
        UDMA_set_mode        db ?
ends

struct  IDE_CHANNEL     ; sizeof = 2*dword
        base            dw ?   ; BAR0 or BAR2
        ctrl            dw ?   ; (BAR1 or BAR3) + 2
        dev_0           IDE_DEVICE  ; word -1 for not device
        dev_1           IDE_DEVICE
ends

struct  IDE_DATA        ; sizeof = 2*dword + 4*dword + 2*dword
        ProgrammingInterface dd ?
        Interrupt            dw ?
        RegsBaseAddres       dw ?
        channel_1            IDE_CHANNEL
        channel_2            IDE_CHANNEL
        dma_hdd_channel_1    db ?
        dma_hdd_channel_2    db ?
                             dw ?
        pcidev               dd ?       ; pointer to corresponding PCIDEV structure
ends

init_ide:
;-----------------------------------------------------------------------------
; find the IDE controller in the device list
;-----------------------------------------------------------------------------
        and     dword[ide_dev_number], 0


        mov     ecx, IDE_controller_1
        mov     esi, pcidev_list
;--------------------------------------
align 4
.loop:
        mov     esi, [esi + PCIDEV.fd]
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
        push    esi ecx
        mov     eax, [esi + PCIDEV.class]
        DEBUGF  1, 'K : IDE controller programming interface %x\n', eax
        mov     [ecx + IDE_DATA.ProgrammingInterface], eax
        mov     [ecx + IDE_DATA.pcidev], esi

        mov     ah, [esi + PCIDEV.bus]
        mov     al, 2
        mov     bh, [esi + PCIDEV.devfn]
;--------------------------------------
        mov     dx, 0x1F0
        test    byte [esi + PCIDEV.class], 1
        jz      @f
        mov     bl, 0x10
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR0 IDE base addr %x\n', dx
        mov     [ecx + IDE_DATA.channel_1.base], dx
;--------------------------------------
        mov     dx, 0x3F4
        test    byte [esi + PCIDEV.class], 1
        jz      @f
        mov     bl, 0x14
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR1 IDE base addr %x\n', dx
        add     dx, 2
        mov     [ecx + IDE_DATA.channel_1.ctrl], dx
;--------------------------------------
        mov     dx, 0x170
        test    byte [esi + PCIDEV.class], 4
        jz      @f
        mov     bl, 0x18
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR2 IDE base addr %x\n', dx
        mov     [ecx + IDE_DATA.channel_2.base], dx
;--------------------------------------
        mov     dx, 0x374
        test    byte [esi + PCIDEV.class], 4
        jz      @f
        mov     bl, 0x1C
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        mov     edx, eax
        pop     eax
@@:
        DEBUGF  1, 'K : BAR3 IDE base addr %x\n', dx
        add     dx, 2
        mov     [ecx + IDE_DATA.channel_2.ctrl], dx
;--------------------------------------
        mov     bl, 0x20
        push    eax
        call    pci_read_reg
        and     eax, 0xFFFC
        DEBUGF  1, 'K : BAR4 IDE controller register base addr %x\n', ax
        mov     [ecx + IDE_DATA.RegsBaseAddres], ax
        pop     eax
;--------------------------------------
        mov     bl, 0x3C
        push    eax
        call    pci_read_reg
        and     eax, 0xFF
        DEBUGF  1, 'K : IDE Interrupt %x\n', al
        mov     [ecx + IDE_DATA.Interrupt], ax
        pop     eax
;--------------------------------------
        mov     eax, -1 ; set full bit
        mov     dword[ecx + IDE_DATA.channel_1.dev_0], eax ; dev_0 and dev_1
        mov     dword[ecx + IDE_DATA.channel_2.dev_0], eax ; dev_0 and dev_1
;------- Init thish controller and enum all disk on chenals
        mov     esi, boot_disabling_ide
        call    boot_log
;--------------------------------------
; Disable IDE interrupts, because the search
; for IDE partitions is in the PIO mode.
;--------------------------------------
; Disable interrupts in IDE controller for PIO
        mov     al, 2
        mov     dx, [ecx + IDE_DATA.channel_1.ctrl]
        out     dx, al
        mov     dx, [ecx + IDE_DATA.channel_2.ctrl]
        out     dx, al
;------- Find ide devices
        ;ide_dev_number = ((1/0 channal) shl 1) + (1/0 device on channal)

        mov     ebp, esp

        sub     esp, 16
        call    ide_check_device

        inc     dword[ide_dev_number]
        sub     esp, 16
        call    ide_check_device

        inc     dword[ide_dev_number]
        sub     esp, 16
        call    ide_check_device

        inc     dword[ide_dev_number]
        sub     esp, 16
        call    ide_check_device

        inc     dword[ide_dev_number]   ; ide_dev_number + 4
;--------------------------------------

        mov     dx, [ecx + IDE_DATA.RegsBaseAddres]
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
        mov     dx, [ecx + IDE_DATA.channel_1.base]
        add     dx, 0x7 ;0x1F7
        in      al, dx
        mov     dx, [ecx + IDE_DATA.channel_2.base]
        add     dx, 0x7 ;0x177
        in      al, dx
;-----------------------------------------------------------------------------
; set interrupts for IDE Controller
;-----------------------------------------------------------------------------
        spin_lock_irqsave
.enable_IDE_interrupt:
        mov     esi, boot_enabling_ide
        call    boot_log

;------- Set DMA mode
        xor     eax, eax
        cmp     dword[ecx + IDE_DATA.channel_1.dev_0], -1
        jne     @f
;--------------------------------------
.ch1_pio_set_no_devices:
        DEBUGF  1, "K : IDE CH1 PIO because no devices\n"
        jmp     .ch1_pio_set_for_all
;-------------------------------------
.ch1_pio_set:
        DEBUGF  1, "K : IDE CH1 PIO because device not support UDMA\n"
.ch1_pio_set_for_all:
        mov     [ecx + IDE_DATA.dma_hdd_channel_1], al
        jmp     .ch2_check

@@:
        cmp     [ecx + IDE_DATA.channel_1.dev_0.UDMA_possible_modes], al
        je      .ch1_pio_set

        cmp     [ecx + IDE_DATA.channel_1.dev_0.UDMA_set_mode], al
        je      .ch1_pio_set

        cmp     [ecx + IDE_DATA.channel_1.dev_1.UDMA_possible_modes], al
        je      .ch1_pio_set

        cmp     [ecx + IDE_DATA.channel_1.dev_1.UDMA_set_mode], al
        je      .ch1_pio_set

        mov     dx, [ecx + IDE_DATA.channel_1.base]
        add     dx, 2
        out     dx, al
        call    set_pci_command_bus_master
        DEBUGF  1, "K : IDE CH1 DMA enabled\n"
        mov     [ecx + IDE_DATA.dma_hdd_channel_1], byte 1

.ch2_check:
        cmp     dword[ecx + IDE_DATA.channel_2.dev_0], -1
        jne     @f

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

@@:
        cmp     [ecx + IDE_DATA.channel_2.dev_0.UDMA_possible_modes], al
        je      .ch2_pio_set

        cmp     [ecx + IDE_DATA.channel_2.dev_0.UDMA_set_mode], al
        je      .ch2_pio_set

        cmp     [ecx + IDE_DATA.channel_2.dev_1.UDMA_possible_modes], al
        je      .ch2_pio_set

        cmp     [ecx + IDE_DATA.channel_2.dev_1.UDMA_set_mode], al
        je      .ch2_pio_set

        mov     dx, [ecx + IDE_DATA.channel_2.base]
        add     dx, 2
        out     dx, al
        call    set_pci_command_bus_master
        DEBUGF  1, "K : IDE CH2 DMA enabled\n"
        mov     [ecx+IDE_DATA.dma_hdd_channel_2], byte 1

        ; set irq handlers
.set_interrupts_for_IDE_controllers:
        mov     esi, boot_set_int_IDE
        call    boot_log
;--------------------------------------
        mov     eax, [ecx + IDE_DATA.ProgrammingInterface]

        test    al, 1 ; 0 - legacy PCI mode, 1 - native PCI mode
        jnz     .sata_ide
;--------------------------------------
.pata_ide:
        cmp     [ecx + IDE_DATA.RegsBaseAddres], 0
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

.sata_ide:
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
        cmp     [ecx + IDE_DATA.RegsBaseAddres], 0
        je      .end_set_interrupts

        mov     ax, [ecx + IDE_DATA.Interrupt]
        movzx   eax, al
        push    ecx
        stdcall attach_int_handler, eax, IDE_common_irq_handler, ecx
        pop     ecx
        DEBUGF  1, "K : Set IDE IRQ%d return code %x\n", [ecx + IDE_DATA.Interrupt]:1, eax
;--------------------------------------
.end_set_interrupts:
        spin_unlock_irqrestore
        ; added disks in kernel list
        ; [esp + 12] - pdata(HD_DATA/CD_DATA)
        ; [esp + 8] - name \ ASCII name 'XXX'
        ; [esp + 4] - name /            'hdXX'
        ; [esp] - ptr to ide_disk_add/ide_atapi_add

        ; in ebp end of array params
@@:
        pop     eax
        call    eax
        cmp     esp, ebp
        jnz     @b
.end_init:
;--------------------------------------
        pop     ecx esi
        add     ecx, sizeof.IDE_DATA
;--------------------------------------
        jmp     .loop
;-----------------------------------------------------------------------------

ide_disk_add:
        lea     eax, [esp + 4]
        mov     edx, [esp + 12]
        stdcall disk_add, ide_callbacks, eax, edx, 0
        test    eax, eax
        jz      @f

        stdcall disk_media_changed, eax, 1
@@:
        ret     12

ide_atapi_add:
        lea     eax, [esp + 4]
        mov     edi, [esp + 12]
        ;stdcall disk_add, cd_callbacks, eax, edi, 0
        test    eax, eax
        jz      @f

        ;mov     [edi + CD_DATA.disk], eax

        ; check loading media

        ; mount media
        ;stdcall disk_media_changed, eax, 1
@@:
        ret     12

;-----------------------------------------------------------------------------
; [esp + 16] - pdata(HD_DATA/CD_DATA)
; [esp + 12] - name \ ASCII name 'XXX'
; [esp + 8]  - name /            'hdXX'
; [esp + 4]  - ptr to ide_disk_add/ide_atapi_add
;  ecx - IDE_DATA
ide_check_device:
        push    ecx ebx ebp

        mov     ebx, [ide_dev_number]
        shr     ebx, 1
        and     ebx, 1

        lea     ebp, [IDE_DATA.channel_1 + ebx*sizeof.IDE_CHANNEL]
        add     ebp, ecx

        DEBUGF  1, "K : Channel %d ",ebx
        DEBUGF  1, "Disk %d\n",[ide_dev_number]:1

        mov     ebx, [ide_dev_number]
        imul    ebx, sizeof.HD_DATA
        add     ebx, hd0_data
        mov     [esp + 3*4 + 16], ebx  ; ebx - HD_DATA

        mov     ax, [ebp + IDE_CHANNEL.base]
        mov     [ebx + HD_DATA.hdbase], ax

        ; ebp - IDE_CHANNEL
        ; ebx - HD_DATA
        stdcall kernel_alloc, 512 ; allocate 512 byte(in real allocated 4096)
        test    eax, eax
        jz      .err_memory

        mov     edi, eax; buffer

        call    ide_ata_identify
        cmp     eax, 7
        je      .end_mem
        test    eax, eax
        jnz     .FindCD

        cmp     word[edi + 6], 16
        ja      .FindCD
        cmp     word[edi + 12], 255
        ja      .FindCD
.FindHDD:
        mov     word[esp + 3*4 + 8], 'hd'
        mov     dword[esp + 3*4 + 4], ide_disk_add

        mov     ecx, [edi + 120]
        mov     dword[ebx + HD_DATA.sectors], ecx
        and     dword[ebx + HD_DATA.sectors+4], 0
        bt      word [edi + 166], 10
        jnc     .dev_found

        mov     [ebx + HD_DATA.hd48], 1
        mov     ecx, [edi + 200]
        mov     dword[ebx + HD_DATA.sectors], ecx
        mov     ecx, [edi + 204]
        mov     dword[ebx + HD_DATA.sectors+4], ecx
        jmp     .dev_found
.FindCD:
        call    ide_device_reset
        test    eax, eax
        jnz     .end_mem

        call    ide_atapi_identify
        test    eax, eax
        jnz     .end_mem


        mov     word[esp + 3*4 + 8], 'cd'
        mov     dword[esp + 3*4 + 4], ide_atapi_add

.dev_found:
        add     ebp, IDE_CHANNEL.dev_0
        test    byte[ide_dev_number], 1b
        je      @f
        add     ebp, sizeof.IDE_DEVICE
@@:
        ; ebp - IDE_DEVICE

.copy_dev_name:

        push    edi
        lea     esi, [edi + 27*2] ; Model number (40 ASCII characters)
        mov     edi, esi
        mov     ecx, 20
        cld
;--------------------------------------
@@:
        lodsw
        xchg    ah, al
        stosw
        loop    @b

        xor     ecx, ecx
        mov     edx, [esi]
        mov     eax, [esp]
        mov     [esi], ecx
        add     eax, 27*2
        DEBUGF 1, "K : Dev: %s \n", eax
        mov    [esi], edx

        pop     edi

        ; set flags DMA mode of selected disk
        xor     eax, eax
        mov     edx, eax
        mov     ax, [edi + 64*2]
        DEBUGF  1, "K : PIO possible modes %x\n", al

        mov     ax, [edi + 51*2]
        mov     al, ah

        xor     ah, ah
        bsf     dx, ax
        jnz     @f
        xor     edx, edx
@@:
        DEBUGF  1, "K : PIO set mode %x\n", dl

        mov     ax, [edi + 63*2]
        DEBUGF  1, "K : Multiword DMA possible modes %x\n", al
        mov     al, ah

        xor     ah, ah
        bsf     dx, ax
        jnz     @f
        xor     edx, edx
@@:
        DEBUGF  1, "K : Multiword DMA set mode %x\n", dl

        mov     ax, [edi + 88*2]
        DEBUGF  1, "K : Ultra DMA possible modes %x\n", al
        mov     [ebp + IDE_DEVICE.UDMA_possible_modes], al
        mov     al, ah

        xor     ah, ah
        bsf     dx, ax
        jnz     @f
        xor     edx, edx
@@:
        DEBUGF  1, "K : Ultra DMA set mode %x\n", ah
        mov     [ebp + IDE_DEVICE.UDMA_set_mode], ah


        ; calc and set number of disk
        mov     eax, [ide_dev_number] ; max 12 - 3 ide controllers
        mov     edx, '0'
        add     edx, eax
        cmp     eax, 10
        jb      @f

        mov     edx, '10'
        sub     eax, 10
        add     dh, al
@@:
        mov     [esp + 3*4 + 10], edx

        ; fixed PIO mode for ATAPI device
        cmp     word[esp + 3*4 + 8], 'cd'
        jne     @f

        mov     word[ebp], 0   ; clear DMA flags for set PIO mode
@@:
        stdcall kernel_free, edi
        pop     ebp ebx ecx
        ret

.end_mem:
        stdcall kernel_free, edi
.end:
.err_memory:
        DEBUGF  1, "K : Device not found\n"
        pop     ebp ebx ecx
        ret     16



uglobal
align 4
; number use for found HD_DATA/CD_DATA, IDE_DATA and IDE_CHANNAL
ide_dev_number  rd 1  ; number of device on ide controller
; (ide_controller_num shl 2) + (ide_channel shl 1) + dev_num
;--------------------------------------
IDE_controller_pointer dd ?
;--------------------------------------
IDE_controller_1 IDE_DATA
IDE_controller_2 IDE_DATA
IDE_controller_3 IDE_DATA
endg

;--------------------------------------
; set Bus Master bit of Command PCI register
;--------------------------------------
set_pci_command_bus_master:
PCI_COMMAND_BUS_MASTER = 0x0004
        push    eax ecx

        mov     ecx, [ecx+IDE_DATA.pcidev]
        mov     ah, [ecx+PCIDEV.bus]
        mov     al, 1   ; word
        mov     bh, [ecx+PCIDEV.devfn]
        mov     bl, 0x4 ; Command register
        push    eax
        call    pci_read_reg
        mov     ecx, eax
        pop     eax
        test    ecx, PCI_COMMAND_BUS_MASTER     ; already set?
        jnz     @f
        or      ecx, PCI_COMMAND_BUS_MASTER
        call    pci_write_reg
@@:
        pop     ecx eax
        ret


virtual at esp
ide_send_cmd_args:
.ATAFeatures            db ?
.ATASectorCount         db ?
.ATASectorNumber        db ?
.ATACylinder            dw ?
.ATAHead                db ?
.ATACommand             db ?
.size = $ - ide_send_cmd_args
end virtual

ATA_ADDR_MODE_CHS       = 0
ATA_ADDR_MODE_LBA       = 1 shl 6
ATA_DISK_NUM_OFFSET     = 4

ATA_HEAD_REG_VALUE      = 10100000b

;-----------------------------------------------------------------------------
; IN:  ebp - IDE_CHANNEL
;      [ide_dev_number] AND 1b  - disk number
;      edi - buffer for data(512 bytes)
; Saved edi, ebx and ebp registers
ide_ata_identify:
        sub     esp, ide_send_cmd_args.size
        mov     dword[ide_send_cmd_args.ATASectorCount], 0
        ; send device identification command
        mov     [ide_send_cmd_args.ATAFeatures], 0
        mov     [ide_send_cmd_args.ATAHead], 0 + ATA_ADDR_MODE_CHS
        mov     [ide_send_cmd_args.ATACommand], 0xEC

        jmp     ide_atapi_identify.send_command
;-----------------------------------------------------------------------------
; IN:  ebp - IDE_CHANNEL
;      [ide_dev_number] AND 1b  - disk number
;      edi - buffer for data(512 bytes)
; Saved edi, ebx and ebp registers
ide_atapi_identify:
        sub     esp, ide_send_cmd_args.size
        ; Send command for device identification
        mov     [ide_send_cmd_args.ATAFeatures], 0
        mov     [ide_send_cmd_args.ATASectorCount], 0
        mov     [ide_send_cmd_args.ATASectorNumber], 0
        mov     [ide_send_cmd_args.ATACylinder], 0
        mov     [ide_send_cmd_args.ATAHead], 0 + ATA_ADDR_MODE_CHS
        mov     [ide_send_cmd_args.ATACommand], 0xA1

.send_command:
        ; Waiting for HDD ready to receive a command
        ; Choose desired disk
        mov     dx, [ebp + IDE_CHANNEL.base]
        add     dx, 6   ; address of the heads register
        mov     al, byte[ide_dev_number]
        and     al, 1b  ; al = disk number (0 or 1)

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Waiting for disk ready
        inc     dx
        mov     ecx, 0xfff
.wait_ready:
        ; Check waiting time
        dec     ecx
        jz      .err_timeout
        ; Read the state register
        in      al, dx
        ; Check the state of BSY signal
        test    al, 80h
        jnz     .wait_ready
        ; Check the state of DRQ signal
        test    al, 08h
        jnz     .wait_ready

        ; load command to controller's registers
        cli
        mov     dx, [ebp + IDE_CHANNEL.base]
        inc     dx      ; "features" register
        mov     al, [ide_send_cmd_args.ATAFeatures]
        out     dx, al
        inc     dx      ; sector counter
        mov     al, [ide_send_cmd_args.ATASectorCount]
        out     dx, al
        inc     dx      ; sector number register
        mov     al, [ide_send_cmd_args.ATASectorNumber]
        out     dx, al
        inc     dx      ; cylinder number (low byte)
        mov     ax, [ide_send_cmd_args.ATACylinder]
        out     dx, al
        inc     dx      ; cylinder number (high byte)
        mov     al, ah
        out     dx, al
        inc     dx      ; head number / disk number

        mov     al, byte[ide_dev_number]
        and     al, 1b  ; al = disk number (0 or 1)
        shl     al, 4

        or      al, [ide_send_cmd_args.ATAHead]
        or      al, 10100000b
        out     dx, al
        ; Send command
        mov     al, [ide_send_cmd_args.ATACommand]
        inc     dx      ; command register
        out     dx, al
        sti

        ; Wait for HDD data ready
        mov     dx, [ebp + IDE_CHANNEL.base]
        add     dx, 7    ; address of state register
        mov     ecx, 0xffff
.wait:
        ; Check command execution time
        dec     ecx
        jz      .err_timeout_limit ; timeout error
        ; Check if ready or not
        in      al, dx
        test    al, 80h  ; BSY signal state
        jnz     .wait

        test    al, 1    ; ERR signal state
        jnz     .err_signal

        test    al, 08h  ; DRQ signal state
        jz      .wait
        ; Receive data block from controller
        push    edi
        mov     dx, [ebp + IDE_CHANNEL.base]
        mov     cx, 512/2  ; words read count
        rep insw
        pop     edi
        xor     eax, eax   ; err code - not error
        add     esp, ide_send_cmd_args.size
        ret
; write error code
.err_timeout_limit:
        mov     eax, 1  ; 1 - waiting time limit exceed
        add     esp, ide_send_cmd_args.size
        ret
.err_timeout:
        mov     eax, 7  ; 7 - time out when choosing channel
        add     esp, ide_send_cmd_args.size
        ret
.err_signal:
        mov     eax, 6  ; 6 - command execution error
        add     esp, ide_send_cmd_args.size
        ret

;-----------------------------------------------------------------------------
; IN:  ebp - IDE_CHANNEL
;      [ide_dev_number] AND 1b  - disk number
; OUT: eax - 0 good
;          - 1 err timeout
ide_device_reset:
        ; Set base address
        mov     dx, [ebp + IDE_CHANNEL.base]
        ; Choose desired disk
        add     dx, 6   ; address of heads register
        mov     al, byte[ide_dev_number]
        and     al, 1b  ; al = disk number (0 or 1)

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Send the "Reset" command
        mov     al, ATA_CMD_DEVICE_RESET
        inc     dx      ; command register
        out     dx, al
        mov     ecx, 0x80000
.wait_ready:
        ; Check waiting time
        dec     ecx
        je      .err_timeout ; time out error
        ; read the state register
        in      al, dx
        ; Check the state of BSY signal
        test    al, ATA_SIGNAL_BSY
        jnz     .wait_ready

        ; reset the error sign
        mov     eax, 0
        ret

.err_timeout:
        mov     eax, 1
        ret

;-----------------------------------------------------------------------------
; END of initialisation IDE ATA code
;-----------------------------------------------------------------------------
find_IDE_controller_done:
; 3. Notify the system about /bd* disks.
; 3a. Check whether there are BIOS disks. If no, skip step 3.
        xor     esi, esi
        cmp     esi, [NumBiosDisks]
        jz      .nobd
; Loop over all disks.
        push    0
        push    'bd'
.bdloop:
; 3b. Get the drive number for using in /bd* name.
        lea     eax, [esi*4]
        movzx   eax, [BiosDisksData+eax*4+BiosDiskData.DriveNumber]
        sub     al, 80h
; 3c. Convert eax to decimal and store starting with [esp+3].
; First 2 bytes in [esp] are "bd".
        lea     edi, [esp+2]
; store digits in the stack, ending with -'0'
        push    -'0'
@@:
        xor     edx, edx
iglobal
align 4
_10     dd      10
endg
        div     [_10]
        push    edx
        test    eax, eax
        jnz     @b
; restore digits from the stack, this reverses the order;
; add '0', stop, when zero is reached
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
; 3e. Call the API with userdata = 80h + ecx.
        mov     eax, esp
        lea     edx, [esi+80h]
        stdcall disk_add, bd_callbacks, eax, edx, 0
        test    eax, eax
        jz      @f
        stdcall disk_media_changed, eax, 1
@@:
; 3f. Continue the loop.
        inc     esi
        cmp     esi, [NumBiosDisks]
        jnz     .bdloop
        pop     ecx ecx ; restore stack after name
.nobd:
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
