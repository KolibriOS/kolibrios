;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  Shutdown for Menuet                                         ;;
;;                                                              ;;
;;  Distributed under General Public License                    ;;
;;  See file COPYING for details.                               ;;
;;  Copyright 2003 Ville Turjanmaa                              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


align 4
system_shutdown:          ; shut down the system

        cmp     [BOOT.shutdown_type], SYSTEM_SHUTDOWN
        jb      @F
        cmp     [BOOT.shutdown_type], SYSTEM_RESTART
        jbe     .valid
@@:
        ret
.valid:
        call    stop_all_services

yes_shutdown_param:
; Shutdown other CPUs, if initialized
        cmp     [ap_initialized], 0
        jz      .no_shutdown_cpus
        mov     edi, [LAPIC_BASE]
        add     edi, 300h
        mov     esi, smpt+4
        mov     ebx, [cpu_count]
        dec     ebx
.shutdown_cpus_loop:
        lodsd
        push    esi
        xor     esi, esi
        inc     esi
        shl     eax, 24
        mov     [edi+10h], eax
; assert INIT IPI
        mov     dword [edi], 0C500h
        call    delay_ms
@@:
        test    dword [edi], 1000h
        jnz     @b
; deassert INIT IPI
        mov     dword [edi], 8500h
        call    delay_ms
@@:
        test    dword [edi], 1000h
        jnz     @b
; don't send STARTUP IPI: let other CPUs be in wait-for-startup state
        pop     esi
        dec     ebx
        jnz     .shutdown_cpus_loop
.no_shutdown_cpus:

        cli
        call    IRQ_mask_all

        movzx   eax, [BOOT.shutdown_type]
        cmp     al, SYSTEM_RESTART
        jne     @F

; load kernel.mnt to _CLEAN_ZONE
        mov     ebx, kernel_file_load
        pushad
        call    file_system_lfn
        popad
@@:
        mov     esi, OS_BASE+restart_code_start ; move kernel re-starter to 0x5000:0
        mov     edi, OS_BASE+0x50000
        mov     ecx, (restart_code_end - restart_code_start)/4
        rep movsd

        cmp     [BOOT.shutdown_type], SYSTEM_SHUTDOWN
        jne     not_power_off

; system_power_off

        mov     ebx, [acpi_fadt_base]
        test    ebx, ebx
        jz      no_acpi
        cmp     [ebx+ACPI_TABLE.Signature], 'FACP'
        jne     no_acpi
        mov     esi, [acpi_ssdt_base]   ; first SSDT is DSDT
        test    esi, esi
        jz      no_acpi
        cmp     [esi+ACPI_TABLE.Signature], 'DSDT'
        jne     no_acpi
        mov     eax, [esi+ACPI_TABLE.Length]
        sub     eax, 36+4
        jbe     no_acpi
        add     esi, 36
.scan_dsdt:
        cmp     dword [esi], '_S5_'
        jnz     .scan_dsdt_cont
        cmp     byte [esi+4], 12h ; DefPackage opcode
        jnz     .scan_dsdt_cont
        mov     dl, [esi+6]
        cmp     dl, 4 ; _S5_ package must contain 4 bytes
                      ; ...in theory; in practice, VirtualBox has 2 bytes
        ja      .scan_dsdt_cont
        cmp     dl, 1
        jb      .scan_dsdt_cont
        lea     esi, [esi+7]
        xor     ecx, ecx
        cmp     byte [esi], 0 ; 0 means zero byte, 0Ah xx means byte xx
        jz      @f
        cmp     byte [esi], 0xA
        jnz     no_acpi
        inc     esi
        mov     cl, [esi]
@@:
        inc     esi
        cmp     dl, 2
        jb      @f
        cmp     byte [esi], 0
        jz      @f
        cmp     byte [esi], 0xA
        jnz     no_acpi
        inc     esi
        mov     ch, [esi]
@@:
        jmp     do_acpi_power_off
.scan_dsdt_cont:
        inc     esi
        dec     eax
        jnz     .scan_dsdt
        jmp     no_acpi
do_acpi_power_off:
        mov     edx, [ebx+ACPI_FADT.SMI_CMD]
        test    edx, edx
        jz      .nosmi
        mov     al, [ebx+ACPI_FADT.ACPI_ENABLE]
        out     dx, al
        mov     edx, [ebx+ACPI_FADT.PM1a_CNT_BLK]
@@:
        in      ax, dx
        test    al, 1
        jz      @b
.nosmi:
        and     cx, 0x0707
        shl     cx, 2
        or      cx, 0x2020
        mov     edx, [ebx+ACPI_FADT.PM1a_CNT_BLK]
        in      ax, dx
        and     ax, 203h
        or      ah, cl
        out     dx, ax
        mov     edx, [ebx+ACPI_FADT.PM1b_CNT_BLK]
        test    edx, edx
        jz      @f
        in      ax, dx
        and     ax, 203h
        or      ah, ch
        out     dx, ax
@@:
        jmp     no_acpi

not_power_off:
        cmp     [BOOT.shutdown_type], SYSTEM_REBOOT
        jnz     not_reboot
        ; try to reboot via ACPI fixed features
        mov     ebx, [acpi_fadt_base]
        test    ebx, ebx
        jz      no_acpi
        cmp     [ebx+ACPI_TABLE.Signature], 'FACP'
        jne     no_acpi
        cmp     [ebx+ACPI_FADT.Length], ACPI_FADT.RESET_VALUE
        jbe     no_acpi
        test    [ebx+ACPI_FADT.Flags], 1 SHL 10    ; reset_reg_supported
        jz      no_acpi
        cmp     [ebx+ACPI_FADT.RESET_REG.ASID], ASID.SYSTEM_IO
        jnz     no_acpi
        cmp     [ebx+ACPI_FADT.RESET_REG.BitWidth], 8
        jnz     no_acpi
        cmp     [ebx+ACPI_FADT.RESET_REG.BitOffset], 0
        jnz     no_acpi
        cmp     [ebx+ACPI_FADT.RESET_REG.AccessSize], ACCESS_SIZE.BYTE
        ja      no_acpi
        cmp     [ebx+ACPI_FADT.RESET_REG.Address.hi], 0
        jnz     no_acpi
        ; 'enable' ACPI
        mov     edx, [ebx+ACPI_FADT.SMI_CMD]
        test    edx, edx
        jz      .nosmi
        mov     al, [ebx+ACPI_FADT.ACPI_ENABLE]
        out     dx, al
        mov     edx, [ebx+ACPI_FADT.PM1a_CNT_BLK]
@@:
        in      ax, dx
        test    al, 1
        jz      @b
.nosmi:

        mov     edx, [ebx+ACPI_FADT.RESET_REG.Address.lo]
        movzx   eax, [ebx+ACPI_FADT.RESET_VALUE]
        out     dx, al
        jmp     no_acpi

not_reboot:
no_acpi:
        call    create_trampoline_pgmap
        mov     cr3, eax
        jmp     @F
org $-OS_BASE
@@:

;disable paging

        mov     eax, cr0
        and     eax, 0x7FFFFFFF
        mov     cr0, eax
        mov     eax, cr3
        mov     cr3, eax

        jmp     0x50000

align 4
restart_code_start:
org 0x50000

        cmp     [BOOT_LO.shutdown_type], SYSTEM_RESTART
        jne     @F

        mov     esi, _CLEAN_ZONE-OS_BASE
        mov     edi, 0x10000
        mov     ecx, 0x31000/4
        cld
        rep movsd
@@:

        xor     ebx, ebx
        xor     edx, edx
        xor     ecx, ecx
        xor     esi, esi
        xor     edi, edi
        xor     ebp, ebp
        lidt    [.idt]
        lgdt    [.gdt]
        jmp     8:@f
align 8
.gdt:
; selector 0 - not used
        dw      23
        dd      .gdt
        dw      0
; selector 8 - code from 5000:0000 to 1000:FFFF
        dw      0FFFFh
        dw      0
        db      5
        db      10011011b
        db      00000000b
        db      0
; selector 10h - data from 1000:0000 to 1000:FFFF
        dw      0FFFFh
        dw      0
        db      1
        db      10010011b
        db      00000000b
        db      0
.idt:
        dw 256*4
        dd 0
org $ - 0x50000
use16
@@:
        mov     ax, 10h
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        mov     eax, cr0
        and     eax, not 80000001h
        mov     cr0, eax
        jmp     0x5000:.real_mode

align 4
.real_mode:

; setup stack

        mov     ax, (TMP_STACK_TOP and 0xF0000) shr 4
        mov     ss, ax
        mov     esp, TMP_STACK_TOP and 0xFFFF

;remap IRQs
        mov     al, 0x11
        out     0x20, al
        out     0xA0, al

        mov     al, 0x08
        out     0x21, al
        mov     al, 0x70
        out     0xA1, al

        mov     al, 0x04
        out     0x21, al
        mov     al, 0x02
        out     0xA1, al

        mov     al, 0x01
        out     0x21, al
        out     0xA1, al

        mov     al, 0xB8
        out     0x21, al
        mov     al, 0xBD
        out     0xA1, al

        mov     al, 00110100b
        out     43h, al
        mov     al, 0xFF
        out     40h, al
        out     40h, al

        xor     ax, ax
        mov     ds, ax
        mov     al, [BOOT_LO.shutdown_type]
        cmp     al, SYSTEM_RESTART
        je      .restart

        cmp     al, SYSTEM_SHUTDOWN
        je      .APM_PowerOff

        mov     word[0x0472], 0x1234
        jmp     0xF000:0xFFF0

.APM_PowerOff:
        mov     ax, 5304h
        xor     bx, bx
        int     15h
;!!!!!!!!!!!!!!!!!!!!!!!!
        mov     ax, 0x5300
        xor     bx, bx
        int     0x15
        push    ax

        mov     ax, 0x5301
        xor     bx, bx
        int     0x15

        mov     ax, 0x5308
        mov     bx, 1
        mov     cx, bx
        int     0x15

        mov     ax, 0x530E
        xor     bx, bx
        pop     cx
        int     0x15

        mov     ax, 0x530D
        mov     bx, 1
        mov     cx, bx
        int     0x15

        mov     ax, 0x530F
        mov     bx, 1
        mov     cx, bx
        int     0x15

        mov     ax, 0x5307
        mov     bx, 1
        mov     cx, 3
        int     0x15
;!!!!!!!!!!!!!!!!!!!!!!!!
        jmp     $

.restart:

; (hint by Black_mirror)
; We must read data from keyboard port,
; because there may be situation when previous keyboard interrupt is lost
; (due to return to real mode and IRQ reprogramming)
; and next interrupt will not be generated (as keyboard waits for handling)

        mov     cx, 16
@@:
        in      al, 0x64
        test    al, 1
        jz      @F
        in      al, 0x60
        loop    @B
@@:

; bootloader interface
        push    0x1000
        pop     ds
        push    0
        pop     es
        mov     si, [es:BOOT_LO.kernel_restart]
        mov     ax, 'KL'
        jmp     0x1000:0000

align 4
org restart_code_start + $
restart_code_end:

org $+OS_BASE
use32
